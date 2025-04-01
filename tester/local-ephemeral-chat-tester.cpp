/*
 * copyright (c) 2010-2023 belledonne communications sarl.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "conference/conference.h"
#include "conference/participant.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/chat.h"
#include "local-conference-tester-functions.h"

namespace LinphoneTest {

static void group_chat_room_server_admin_managed_messages_unencrypted(void) {
	group_chat_room_server_admin_managed_messages_base(FALSE);
}

static void group_chat_room_server_admin_managed_messages_ephemeral_enabled_after_creation(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		stats chloe_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());

		linphone_chat_room_params_enable_group(params, FALSE);
		linphone_chat_room_params_enable_encryption(params, FALSE);
		linphone_chat_room_params_set_ephemeral_mode(params, adminMode);
		linphone_chat_room_params_set_ephemeral_lifetime(params, 0);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);

		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_params(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, params);
		linphone_chat_room_params_unref(params);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat = pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyReceived,
		                             pauline_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(50), [&pauline] {
			for (auto chatRoom : pauline.getCore().getChatRooms()) {
				if (!chatRoom->ephemeralEnabled() || (chatRoom->getEphemeralLifetime() != 10)) {
					return false;
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =
		    linphone_address_clone(linphone_proxy_config_get_contact(marie.getDefaultProxyConfig()));
		marieCr = marie.searchChatRoom(marieDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline}).wait([&marie] { return checkChatroomCreation(marie, 1); }));

		if (marieCr) {
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");

			marie_stat = marie.getStats();
			pauline_stat = pauline.getStats();

			linphone_chat_room_set_ephemeral_lifetime(marieCr, 5);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");

			sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Back online ", noMsg);
		}

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(marieDeviceAddr);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_ephemeral_disabled_after_creation(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 1);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		stats chloe_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoom *marieCr = create_chat_room_client_side(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, FALSE, adminMode);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 1, int, "%d");

		pauline_stat = pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 0);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageDisabled,
		                             pauline_stat.number_of_LinphoneChatRoomEphemeralMessageDisabled + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(50), [&pauline] {
			for (auto chatRoom : pauline.getCore().getChatRooms()) {
				if (chatRoom->ephemeralEnabled()) {
					return false;
				}
			}
			return true;
		}));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();

		constexpr int noMsg = 10;
		LinphoneChatMessage *message[noMsg];
		// Marie sends messages
		for (int i = 0; i < noMsg; i++) {
			const std::string text = std::string("Hello ") + std::to_string(i);
			message[i] = _send_message_ephemeral(marieCr, text.c_str(), TRUE);
		}

		bctbx_list_t *marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), noMsg, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
		                             pauline_stat.number_of_LinphoneMessageReceived + noMsg, 11000));

		// Check that the message has been delivered to Pauline
		for (int i = 0; i < noMsg; i++) {
			const auto msg = message[i];
			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([msg] {
				return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
			}));
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&, paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == noMsg;
		}));

		bctbx_list_t *paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), noMsg, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
		                             marie_stat.number_of_LinphoneMessageDeliveredToUser + noMsg,
		                             liblinphone_tester_sip_timeout));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             marie_stat.number_of_LinphoneMessageDisplayed + noMsg,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), noMsg, int, "%i");
		marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), noMsg, int, "%i");

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		for (int i = 0; i < noMsg; i++) {
			linphone_chat_message_unref(message[i]);
		}

		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =
		    linphone_address_clone(linphone_proxy_config_get_contact(marie.getDefaultProxyConfig()));
		marieCr = marie.searchChatRoom(marieDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline}).wait([&marie] { return checkChatroomCreation(marie, 1); }));

		if (marieCr) {
			BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));

			marie_stat = marie.getStats();
			pauline_stat = pauline.getStats();

			int lifetime = 5;
			char *conference_address_str = linphone_address_as_string(confAddr);
			ms_message("%s enables ephemeral capabilities with lifetime set to %0d seconds in chatroom %s",
			           linphone_core_get_identity(marie.getLc()), lifetime, conference_address_str);
			ms_free(conference_address_str);
			linphone_chat_room_set_ephemeral_lifetime(marieCr, lifetime);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageEnabled,
			    pauline_stat.number_of_LinphoneChatRoomEphemeralMessageEnabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,
			    pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

			sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Back online ", noMsg);
		}

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(marieDeviceAddr);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_ephemeral_lifetime_update(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 5);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoom *marieCr = create_chat_room_client_side(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, FALSE, adminMode);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =
		    linphone_address_clone(linphone_proxy_config_get_contact(marie.getDefaultProxyConfig()));
		marieCr = marie.searchChatRoom(marieDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline}).wait([&marie] { return checkChatroomCreation(marie, 1); }));

		if (marieCr) {
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");

			marie_stat = marie.getStats();
			pauline_stat = pauline.getStats();
			linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,
			    pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

			constexpr int noMsg = 10;
			sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);
		}

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(marieDeviceAddr);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_ephemeral_lifetime_toggle_using_different_methods(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 5);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		stats chloe_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoom *marieCr = create_chat_room_client_side(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, FALSE, adminMode);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		pauline_stat = pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,
		                             pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

		chloe_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		pauline_stat = pauline.getStats();
		// Disable ephemeral
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 0);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageDisabled,
		                             pauline_stat.number_of_LinphoneChatRoomEphemeralMessageDisabled + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 0, int, "%d");

		chloe_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();

		LinphoneChatMessage *non_ephemeral_message;
		// Marie sends messages
		const std::string non_ephemeral_text = std::string("Not an ephemeral message");
		non_ephemeral_message = _send_message_ephemeral(marieCr, non_ephemeral_text.c_str(), TRUE);

		auto marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), 1, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
		                             pauline_stat.number_of_LinphoneMessageReceived + 1, 11000));

		// Check that the message has been delivered to Pauline
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([non_ephemeral_message] {
			return (linphone_chat_message_get_state(non_ephemeral_message) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&, paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));

		auto paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), 1, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
		                             marie_stat.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             marie_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		pauline_stat = pauline.getStats();
		linphone_chat_room_enable_ephemeral(marieCr, TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageEnabled,
		                             pauline_stat.number_of_LinphoneChatRoomEphemeralMessageEnabled + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr),
		                linphone_core_get_default_ephemeral_lifetime(marie.getLc()), int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr),
		                linphone_core_get_default_ephemeral_lifetime(marie.getLc()), int, "%d");

		chloe_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();

		constexpr int noShortMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Test ephemeral message #",
		                                noShortMsg);

		linphone_chat_message_unref(non_ephemeral_message);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_ephemeral_mode_changed(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		stats chloe_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());

		linphone_chat_room_params_enable_group(params, FALSE);
		linphone_chat_room_params_enable_encryption(params, FALSE);
		linphone_chat_room_params_set_ephemeral_mode(params, adminMode);
		linphone_chat_room_params_set_ephemeral_lifetime(params, 0);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);

		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_params(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, params);
		linphone_chat_room_params_unref(params);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat = pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyReceived,
		                             pauline_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(50), [&pauline] {
			for (auto chatRoom : pauline.getCore().getChatRooms()) {
				if (!chatRoom->ephemeralEnabled() || (chatRoom->getEphemeralLifetime() != 10)) {
					return false;
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		pauline_stat = pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 0);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyReceived,
		                             pauline_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat = pauline.getStats();
		const LinphoneChatRoomEphemeralMode deviceMode = LinphoneChatRoomEphemeralModeDeviceManaged;
		linphone_chat_room_set_ephemeral_mode(marieCr, deviceMode);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyReceived,
		                             pauline_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		pauline_stat = pauline.getStats();
		marie_stat = marie.getStats();

		linphone_chat_room_enable_ephemeral(paulineCr, TRUE);
		linphone_chat_room_set_ephemeral_lifetime(paulineCr, 5);

		wait_for_list(coresList, NULL, 1, 2000);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), deviceMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), deviceMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		LinphoneChatMessage *nonEphemeralMessage = _send_message(marieCr, "I have disabled ephemeral messages");

		auto marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), 1, int, "%i");
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
		                             pauline_stat.number_of_LinphoneMessageReceived + 1, 11000));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([nonEphemeralMessage] {
			return (linphone_chat_message_get_state(nonEphemeralMessage) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&, paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));

		auto paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), 1, int, "%i");
		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
		                             marie_stat.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             marie_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		sendEphemeralMessageInAdminMode(focus, pauline, marie, paulineCr, marieCr, "Test ephemeral message ", noMsg);

		if (nonEphemeralMessage) {
			linphone_chat_message_unref(nonEphemeralMessage);
		}

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_ephemeral_chat_tests[] = {
    TEST_NO_TAG("Unencrypted group chat server chat room with admin managed ephemeral messages",
                LinphoneTest::group_chat_room_server_admin_managed_messages_unencrypted),
    TEST_ONE_TAG("Group chat Server chat room with admin managed ephemeral messages disabled after creation",
                 LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_disabled_after_creation,
                 "LeaksMemory"), /* because of coreMgr restart*/
    TEST_ONE_TAG("Group chat Server chat room with admin managed ephemeral messages enabled after creation",
                 LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_enabled_after_creation,
                 "LeaksMemory"), /* because of coreMgr restart*/
    TEST_ONE_TAG("Group chat Server chat room with admin managed ephemeral messages with lifetime update",
                 LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_lifetime_update,
                 "LeaksMemory"), /* because of coreMgr restart*/
    TEST_NO_TAG(
        "Group chat Server chat room with admin managed ephemeral messages with lifetime toggle",
        LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_lifetime_toggle_using_different_methods),
    TEST_NO_TAG("Group chat Server chat room with ephemeral message mode changed",
                LinphoneTest::group_chat_room_server_ephemeral_mode_changed)};

test_suite_t local_conference_test_suite_ephemeral_chat = {"Local conference tester (Ephemeral Chat)",
                                                           NULL,
                                                           NULL,
                                                           liblinphone_tester_before_each,
                                                           liblinphone_tester_after_each,
                                                           sizeof(local_conference_ephemeral_chat_tests) /
                                                               sizeof(local_conference_ephemeral_chat_tests[0]),
                                                           local_conference_ephemeral_chat_tests,
                                                           0};
