/*
 * copyright (c) 2010-2022 belledonne communications sarl.
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

#include "chat/encryption/encryption-engine.h"
#include "conference/conference.h"
#include "conference/participant.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/api/c-participant-imdn-state.h"
#include "linphone/api/c-participant.h"
#include "linphone/chat.h"
#include "local-conference-tester-functions.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace LinphoneTest {

void sendEphemeralMessageInAdminMode(Focus &focus,
                                     ClientConference &sender,
                                     ClientConference &recipient,
                                     LinphoneChatRoom *senderCr,
                                     LinphoneChatRoom *recipientCr,
                                     const std::string basicText,
                                     const int noMsg) {

	bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, sender.getLc());
	coresList = bctbx_list_append(coresList, recipient.getLc());

	bctbx_list_t *senderHistory = linphone_chat_room_get_history(senderCr, 0);
	auto initialSenderMessages = (int)bctbx_list_size(senderHistory);
	bctbx_list_free_with_data(senderHistory, (bctbx_list_free_func)linphone_chat_message_unref);

	bctbx_list_t *recipientHistory = linphone_chat_room_get_history(recipientCr, 0);
	auto initialRecipientMessages = (int)bctbx_list_size(recipientHistory);
	bctbx_list_free_with_data(recipientHistory, (bctbx_list_free_func)linphone_chat_message_unref);

	int initialUnreadMessages = linphone_chat_room_get_unread_messages_count(recipientCr);

	auto sender_stat = sender.getStats();
	auto recipient_stat = recipient.getStats();

	std::list<LinphoneChatMessage *> messages;
	// Marie sends messages
	for (int i = 0; i < noMsg; i++) {
		const std::string text = basicText + std::to_string(i);
		messages.push_back(_send_message_ephemeral(senderCr, text.c_str(), TRUE));
	}

	senderHistory = linphone_chat_room_get_history(senderCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(senderHistory), (noMsg + initialSenderMessages), int, "%i");
	set_ephemeral_cbs(senderHistory);

	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageReceived,
	                             recipient_stat.number_of_LinphoneMessageReceived + noMsg, 11000));

	// Check that the message has been delivered to Pauline
	for (const auto &msg : messages) {
		BC_ASSERT_TRUE(CoreManagerAssert({focus, sender, recipient}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
	}

	BC_ASSERT_TRUE(CoreManagerAssert({focus, sender, recipient}).wait([&, recipientCr] {
		return linphone_chat_room_get_unread_messages_count(recipientCr) == (noMsg + initialUnreadMessages);
	}));

	recipientHistory = linphone_chat_room_get_history(recipientCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(recipientHistory), (noMsg + initialRecipientMessages), int, "%i");
	set_ephemeral_cbs(recipientHistory);

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageDeliveredToUser,
	                             sender_stat.number_of_LinphoneMessageDeliveredToUser + noMsg,
	                             liblinphone_tester_sip_timeout));

	// Pauline marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(recipientCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageDisplayed,
	                             sender_stat.number_of_LinphoneMessageDisplayed + noMsg,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             sender_stat.number_of_LinphoneChatRoomEphemeralTimerStarted + noMsg,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             recipient_stat.number_of_LinphoneChatRoomEphemeralTimerStarted + noMsg,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageEphemeralTimerStarted,
	                             sender_stat.number_of_LinphoneMessageEphemeralTimerStarted + noMsg,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageEphemeralTimerStarted,
	                             recipient_stat.number_of_LinphoneMessageEphemeralTimerStarted + noMsg,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneChatRoomEphemeralDeleted,
	                             sender_stat.number_of_LinphoneChatRoomEphemeralDeleted + noMsg, 15000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneChatRoomEphemeralDeleted,
	                             recipient_stat.number_of_LinphoneChatRoomEphemeralDeleted + noMsg, 15000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageEphemeralDeleted,
	                             sender_stat.number_of_LinphoneMessageEphemeralDeleted + noMsg, 15000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageEphemeralDeleted,
	                             recipient_stat.number_of_LinphoneMessageEphemeralDeleted + noMsg, 15000));

	bctbx_list_free_with_data(recipientHistory, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(senderHistory, (bctbx_list_free_func)linphone_chat_message_unref);

	// wait a bit longer to detect side effect if any
	CoreManagerAssert({focus, sender, recipient}).waitUntil(chrono::seconds(2), [] { return false; });

	recipientHistory = linphone_chat_room_get_history(recipientCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(recipientHistory), initialRecipientMessages, int, "%i");
	senderHistory = linphone_chat_room_get_history(senderCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(senderHistory), initialSenderMessages, int, "%i");

	for (auto &msg : messages) {
		linphone_chat_message_unref(msg);
	}

	bctbx_list_free_with_data(recipientHistory, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(senderHistory, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free(coresList);
}

bool checkChatroomCreation(const ConfCoreManager &core,
                           const int nbChatRooms,
                           const int participantNumber,
                           const std::string subject) {
	auto &chatRooms = core.getCore().getChatRooms();
	if (chatRooms.size() != static_cast<size_t>(nbChatRooms)) {
		return false;
	}
	for (auto chatRoom : chatRooms) {
		if (chatRoom->getState() != ConferenceInterface::State::Created) {
			return false;
		}
		if ((participantNumber >= 0) && (chatRoom->getConference()->getParticipantCount() != participantNumber)) {
			return false;
		}
		if (!subject.empty() && (chatRoom->getSubject().compare(subject) != 0)) {
			return false;
		}
	}
	return true;
}

bool checkChatroom(Focus &focus, const ConfCoreManager &core, const time_t baseJoiningTime) {
	const auto &chatRooms = core.getCore().getChatRooms();
	if (chatRooms.size() < 1) {
		return false;
	}

	for (const auto &chatRoom : chatRooms) {
		auto participants = chatRoom->getParticipants();
		if (focus.getLc() != core.getLc()) {
			participants.push_back(chatRoom->getMe());
		}
		for (const auto &participant : participants) {
			for (const auto &device : participant->getDevices()) {
				if (device->getState() != ParticipantDevice::State::Present) {
					return false;
				}
				if (device->isInConference() == false) {
					return false;
				}
				if ((baseJoiningTime >= 0) && (device->getTimeOfJoining() >= baseJoiningTime)) {
					return false;
				}
			}
		}
	}
	return true;
}

void group_chat_room_server_admin_managed_messages_base(bool_t encrypted) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 25);

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
		linphone_chat_room_params_set_ephemeral_lifetime(params, 5);
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
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

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

void group_chat_room_with_client_restart_base(bool encrypted) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(berthe);

		linphone_core_set_add_admin_information_to_contact(marie.getLc(), TRUE);
		linphone_core_set_add_admin_information_to_contact(laure.getLc(), TRUE);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, berthe.getLc());
		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address bertheAddr = berthe.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(bertheAddr.toC()));

		stats initialMarieStats = marie.getStats();
		stats initialMichelleStats = michelle.getStats();
		stats initialBertheStats = berthe.getStats();
		stats initialLaureStats = laure.getStats();

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
		}

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
		    coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, 2, encrypted,
		    LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Michelle's side and that the participants are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 2, FALSE);

		LinphoneChatRoom *bertheCr = check_creation_chat_room_client_side(
		    coresList, berthe.getCMgr(), &initialBertheStats, confAddr, initialSubject, 2, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, laure, berthe}).wait([&focus] {
			for (const auto &chatRoom : focus.getCore().getChatRooms()) {
				for (const auto &participant : chatRoom->getParticipants()) {
					for (const auto &device : participant->getDevices()) {
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
					}
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialBertheStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed,
		                             initialMarieStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed,
		                             initialMichelleStats.number_of_subject_changed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_subject_changed,
		                             initialBertheStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(michelleCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(bertheCr), newSubject);

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(bertheCr), 2, int, "%d");

		const std::initializer_list<std::reference_wrapper<ConfCoreManager>> cores2{focus, marie, michelle, berthe};
		for (const ConfCoreManager &core : cores2) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, berthe, laure})
			        .waitUntil(chrono::seconds(10), [&focus, &core] { return checkChatroom(focus, core, -1); }));
		};

		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		stats initialMichelle2Stats = michelle2.getStats();
		coresList = bctbx_list_append(coresList, michelle2.getLc());
		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));
		}

		focus.registerAsParticipantDevice(michelle2);

		LinphoneAddress *michelle2Contact = linphone_address_clone(
		    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(michelle2.getLc())));
		char *michelle2ContactString = linphone_address_as_string(michelle2Contact);
		ms_message("%s is adding device %s", linphone_core_get_identity(focus.getLc()), michelle2ContactString);
		ms_free(michelle2ContactString);
		focus.registerAsParticipantDevice(michelle2);

		// Notify chat room that a participant has registered
		bctbx_list_t *devices = NULL;
		const LinphoneAddress *deviceAddr = linphone_proxy_config_get_contact(michelle.getDefaultProxyConfig());
		LinphoneParticipantDeviceIdentity *identity =
		    linphone_factory_create_participant_device_identity(linphone_factory_get(), deviceAddr, "");
		bctbx_list_t *specs = linphone_core_get_linphone_specs_list(michelle.getLc());
		linphone_participant_device_identity_set_capability_descriptor_2(identity, specs);
		bctbx_list_free_with_data(specs, ms_free);
		devices = bctbx_list_append(devices, identity);

		deviceAddr = linphone_proxy_config_get_contact(michelle2.getDefaultProxyConfig());
		identity = linphone_factory_create_participant_device_identity(linphone_factory_get(), deviceAddr, "");
		specs = linphone_core_get_linphone_specs_list(michelle2.getLc());
		linphone_participant_device_identity_set_capability_descriptor_2(identity, specs);
		bctbx_list_free_with_data(specs, ms_free);
		devices = bctbx_list_append(devices, identity);

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			linphone_chat_room_set_participant_devices(chatRoom->toC(), michelle.getCMgr()->identity, devices);
		}
		bctbx_list_free_with_data(devices, (bctbx_list_free_func)belle_sip_object_unref);

		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &initialMichelle2Stats, confAddr, newSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelle2Cr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		const std::initializer_list<std::reference_wrapper<ConfCoreManager>> cores3{focus, marie, michelle, michelle2,
		                                                                            berthe};
		for (const ConfCoreManager &core : cores3) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure})
			        .waitUntil(chrono::seconds(10), [&focus, &core] { return checkChatroom(focus, core, -1); }));

			const bctbx_list_t *chat_rooms = linphone_core_get_chat_rooms(core.getLc());
			for (const bctbx_list_t *chat_room_it = chat_rooms; chat_room_it != NULL;
			     chat_room_it = chat_room_it->next) {
				const LinphoneChatRoom *chat_room =
				    static_cast<const LinphoneChatRoom *>(bctbx_list_get_data(chat_room_it));
				BC_ASSERT_PTR_NOT_NULL(chat_room);
				if (chat_room) {
					const char *chat_room_identifier = linphone_chat_room_get_identifier(chat_room);
					LinphoneChatRoom *found_chat_room =
					    linphone_core_search_chat_room_by_identifier(core.getLc(), chat_room_identifier);
					BC_ASSERT_PTR_NOT_NULL(found_chat_room);
					BC_ASSERT_PTR_EQUAL(chat_room, found_chat_room);
				}
			}
		};

		// Test invalid peer address
		BC_ASSERT_PTR_NULL(linphone_core_search_chat_room_by_identifier(
		    marie.getLc(), "==sip:toto@sip.conference.org##sip:me@sip.local.org"));
		// Test inexistent chat room identifier
		BC_ASSERT_PTR_NULL(linphone_core_search_chat_room_by_identifier(
		    marie.getLc(), "sip:toto@sip.conference.org##sip:me@sip.local.org"));

		initialMarieStats = marie.getStats();
		initialMichelleStats = michelle.getStats();
		initialBertheStats = berthe.getStats();
		initialLaureStats = laure.getStats();

		Address laureAddr = laure.getIdentity();
		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(laureAddr.toC()));
		ms_message("%s is adding participant %s", linphone_core_get_identity(focus.getLc()),
		           linphone_core_get_identity(laure.getLc()));
		linphone_chat_room_add_participants(marieCr, participantsAddresses);
		bctbx_list_free(participantsAddresses);

		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats,
		                                                                 confAddr, newSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreationPending,
		                             initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialLaureStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneChatRoomConferenceJoined,
		                             initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_chat_room_participants_added,
		                             initialMarieStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_chat_room_participants_added,
		                             initialBertheStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_chat_room_participants_added,
		                             initialMichelleStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_chat_room_participants_added,
		                             initialMichelle2Stats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_chat_room_participant_devices_added,
		                             initialMarieStats.number_of_chat_room_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_chat_room_participant_devices_added,
		                             initialBertheStats.number_of_chat_room_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_chat_room_participant_devices_added,
		                             initialMichelleStats.number_of_chat_room_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_chat_room_participant_devices_added,
		                             initialMichelle2Stats.number_of_chat_room_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(bertheCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelle2Cr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

		const std::initializer_list<std::reference_wrapper<ConfCoreManager>> cores{focus,     marie, michelle,
		                                                                           michelle2, laure, berthe};
		for (const ConfCoreManager &core : cores) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe})
			        .waitUntil(chrono::seconds(10), [&focus, &core] { return checkChatroom(focus, core, -1); }));
			for (auto chatRoom : core.getCore().getChatRooms()) {
				BC_ASSERT_EQUAL(chatRoom->getParticipants().size(), ((focus.getLc() == core.getLc())) ? 4 : 3, size_t,
				                "%zu");
				BC_ASSERT_STRING_EQUAL(chatRoom->getSubject().c_str(), newSubject);
			}
		};

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).waitUntil(chrono::seconds(1), [] {
			return false;
		});

		time_t participantAddedTime = ms_time(nullptr);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).waitUntil(chrono::seconds(10), [] {
			return false;
		});

		ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
		coresList = bctbx_list_remove(coresList, focus.getLc());
		// Restart flexisip
		focus.reStart();
		coresList = bctbx_list_append(coresList, focus.getLc());
		BC_ASSERT_EQUAL(focus.getCore().getChatRooms().size(), 1, size_t, "%zu");
		for (auto chatRoom : focus.getCore().getChatRooms()) {
			BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom->toC()), 4, int, "%d");
		}

		// Michelle2 mutes all chat rooms
		for (auto chatRoom : michelle2.getCore().getChatRooms()) {
			chatRoom->setIsMuted(true);
		}
		michelle2ContactString = linphone_address_as_string(michelle2Contact);
		ms_message("%s is restarting its core", michelle2ContactString);
		ms_free(michelle2ContactString);
		coresList = bctbx_list_remove(coresList, michelle2.getLc());
		// Restart michelle
		michelle2.reStart();
		coresList = bctbx_list_append(coresList, michelle2.getLc());

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, laure, berthe, michelle, michelle2}).wait([&michelle2] {
			return checkChatroomCreation(michelle2, 1);
		}));

		LinphoneAddress *michelleDeviceAddr =
		    linphone_address_clone(linphone_proxy_config_get_contact(michelle2.getDefaultProxyConfig()));
		michelle2Cr = michelle2.searchChatRoom(michelleDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(michelle2Cr);
		for (const ConfCoreManager &core : cores) {
			BC_ASSERT_TRUE(checkChatroom(focus, core, participantAddedTime));
			for (auto chatRoom : core.getCore().getChatRooms()) {
				BC_ASSERT_EQUAL(chatRoom->getParticipants().size(), ((focus.getLc() == core.getLc())) ? 4 : 3, size_t,
				                "%zu");
				BC_ASSERT_STRING_EQUAL(chatRoom->getSubject().c_str(), newSubject);
			}
		}

		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(michelle2Cr, "back with you");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		linphone_chat_message_unref(msg);
		msg = NULL;

		msg = linphone_chat_room_create_message_from_utf8(marieCr, "welcome back");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([michelle2Cr] {
			return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		}));
		linphone_chat_message_unref(msg);
		msg = NULL;

		msg = linphone_chat_room_create_message_from_utf8(michelleCr, "message blabla");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 2;
		}));
		linphone_chat_message_unref(msg);

		CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).waitUntil(std::chrono::seconds(2), [] {
			return false;
		});

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, michelle, michelle2, laure, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(michelle2Contact);
		linphone_address_unref(michelleDeviceAddr);
		bctbx_list_free(coresList);
	}
}

static void chat_room_participant_added_sip_error(LinphoneChatRoom *cr,
                                                  BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	bctbx_list_t *participants = linphone_chat_room_get_participants(cr);
	if (bctbx_list_size(participants) == 2) {
		LinphoneCoreManager *initiator = (LinphoneCoreManager *)linphone_chat_room_get_user_data(cr);
		ms_message("Turning off network for core %s", linphone_core_get_identity(initiator->lc));
		linphone_core_set_network_reachable(initiator->lc, FALSE);
	}
	if (participants) {
		bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);
	}
}

static void
server_core_chat_room_state_changed_sip_error(LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	Focus *focus = (Focus *)(((LinphoneCoreManager *)linphone_core_get_user_data(core))->user_info);
	switch (state) {
		case LinphoneChatRoomStateInstantiated: {
			LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
			linphone_chat_room_cbs_set_participant_added(cbs, chat_room_participant_added_sip_error);
			linphone_chat_room_add_callbacks(cr, cbs);
			linphone_chat_room_cbs_set_user_data(cbs, focus);
			linphone_chat_room_cbs_unref(cbs);
			break;
		}
		default:
			break;
	}
}

void group_chat_room_with_sip_errors_base(bool invite_error, bool subscribe_error, bool encrypted) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);

		stats initialFocusStats = focus.getStats();
		stats initialMarieStats = marie.getStats();
		stats initialMichelleStats = michelle.getStats();
		stats initialMichelle2Stats = michelle2.getStats();
		stats initialLaureStats = laure.getStats();
		stats initialBertheStats = berthe.getStats();
		stats initialPaulineStats = pauline.getStats();

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, berthe.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		linphone_core_set_network_reachable(marie.getLc(), FALSE);
		linphone_core_set_network_reachable(berthe.getLc(), FALSE);

		char *spec = bctbx_strdup_printf("groupchat/1.1");
		linphone_core_remove_linphone_spec(marie.getLc(), "groupchat");
		linphone_core_add_linphone_spec(marie.getLc(), spec);
		linphone_core_remove_linphone_spec(berthe.getLc(), "groupchat");
		linphone_core_add_linphone_spec(berthe.getLc(), spec);
		bctbx_free(spec);

		linphone_core_set_network_reachable(marie.getLc(), TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneRegistrationOk,
		                             initialMarieStats.number_of_LinphoneRegistrationOk + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_core_set_network_reachable(berthe.getLc(), TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneRegistrationOk,
		                             initialBertheStats.number_of_LinphoneRegistrationOk + 1,
		                             liblinphone_tester_sip_timeout));

		initialMarieStats = marie.getStats();
		initialBertheStats = berthe.getStats();

		std::list<LinphoneCoreManager *> shutdownNetworkClients;
		std::list<stats> initialStatsList;
		if (invite_error) {
			shutdownNetworkClients.push_back(michelle2.getCMgr());
			initialStatsList.push_back(michelle2.getStats());
			shutdownNetworkClients.push_back(berthe.getCMgr());
			initialStatsList.push_back(berthe.getStats());
		} else if (subscribe_error) {
			shutdownNetworkClients.push_back(marie.getCMgr());
			initialStatsList.push_back(marie.getStats());
			shutdownNetworkClients.push_back(michelle2.getCMgr());
			initialStatsList.push_back(michelle2.getStats());
			shutdownNetworkClients.push_back(berthe.getCMgr());
			initialStatsList.push_back(berthe.getStats());
		}

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(berthe);

		if (invite_error) {
			LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
			linphone_core_cbs_set_chat_room_state_changed(cbs, server_core_chat_room_state_changed_sip_error);
			linphone_core_add_callbacks(focus.getLc(), cbs);
			linphone_core_cbs_unref(cbs);
		}

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(berthe.getLc()));

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));
		Address bertheAddr = berthe.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(bertheAddr.toC()));
		Address paulineAddr = pauline.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));

		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		int participantsAddressesSize = (int)bctbx_list_size(participantsAddresses);
		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());

		linphone_chat_room_params_enable_encryption(params, encrypted);
		linphone_chat_room_params_set_ephemeral_mode(params, LinphoneChatRoomEphemeralModeDeviceManaged);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
		linphone_chat_room_params_enable_group(params, participantsAddressesSize > 1 ? TRUE : FALSE);
		// Marie creates a new group chat room
		LinphoneChatRoom *marieCr =
		    linphone_core_create_chat_room_2(marie.getLc(), params, initialSubject, participantsAddresses);
		linphone_chat_room_params_unref(params);
		if (marieCr) linphone_chat_room_unref(marieCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 1;
		}));

		LinphoneAddress *confAddr = NULL;
		for (auto chatRoom : focus.getCore().getChatRooms()) {
			linphone_chat_room_set_user_data(chatRoom->toC(), marie.getCMgr());
			confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(chatRoom->toC()));
		}

		for (const auto &client : shutdownNetworkClients) {
			stats &initialStats = initialStatsList.front();
			BC_ASSERT_TRUE(wait_for_list(coresList, &client->stat.number_of_LinphoneConferenceStateCreated,
			                             initialStats.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			char *proxy_contact_str = linphone_address_as_string(
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc)));
			ms_message("Disabling network of core %s (contact %s)", linphone_core_get_identity(client->lc),
			           proxy_contact_str);
			ms_free(proxy_contact_str);
			linphone_core_set_network_reachable(client->lc, FALSE);
			initialStatsList.pop_front();
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_added,
		                             initialFocusStats.number_of_chat_room_participants_added + 4, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participant_devices_added,
		                             initialFocusStats.number_of_chat_room_participant_devices_added + 5, 5000));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe})
		    .waitUntil(chrono::seconds(60), [] { return false; });

		check_create_chat_room_client_side(coresList, marie.getCMgr(), marieCr, &initialMarieStats,
		                                   participantsAddresses, initialSubject, 2);
		// Check that the chat room is correctly created on Pauline's and Michelle's side and that the participants are
		// added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 3, FALSE);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &initialMichelle2Stats, confAddr, initialSubject, 3, FALSE);
		LinphoneChatRoom *bertheCr = check_creation_chat_room_client_side(
		    coresList, berthe.getCMgr(), &initialBertheStats, confAddr, initialSubject, 3, FALSE);
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(
		    coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 3, FALSE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialBertheStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		std::string msg_text = "message michelle blabla";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(michelleCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg] {
			return (msg && (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered));
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));
		LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);
		if (paulineLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(paulineCr);

		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageDisplayed,
		                              initialMichelleStats.number_of_LinphoneMessageDisplayed + 1, 3000));

		if (invite_error || subscribe_error) {
			char *marie_proxy_contact_str = linphone_address_as_string(
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
			ms_message("Enabling network of core %s (contact %s)", linphone_core_get_identity(marie.getLc()),
			           marie_proxy_contact_str);
			ms_free(marie_proxy_contact_str);
			if (invite_error) {
				initialMarieStats = marie.getStats();
			}
			linphone_core_set_network_reachable(marie.getLc(), TRUE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneRegistrationOk,
			                             initialMarieStats.number_of_LinphoneRegistrationOk + 1,
			                             liblinphone_tester_sip_timeout));
			marieCr = check_creation_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, confAddr,
			                                               initialSubject, 3, TRUE);
			BC_ASSERT_PTR_NOT_NULL(marieCr);
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomSessionUpdating, 1, 1000));
			BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneChatRoomSessionUpdating, 0, int, "%0d");
		}

		focus.registerAsParticipantDevice(laure);
		Address laureAddr = laure.getIdentity();
		linphone_chat_room_add_participant(marieCr, linphone_address_ref(laureAddr.toC()));
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(
		    coresList, laure.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 4, FALSE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_added,
		                             initialFocusStats.number_of_chat_room_participants_added + 4, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participant_devices_added,
		                             initialFocusStats.number_of_chat_room_participant_devices_added + 5, 5000));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		LinphoneChatMessage *marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		msg_text = "message laure blabla";
		LinphoneChatMessage *msg2 = ClientConference::sendTextMsg(laureCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg2] {
			return (linphone_chat_message_get_state(msg2) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));
		paulineLastMsg = pauline.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);
		if (paulineLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 2;
		}));
		marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([michelleCr] {
			    return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		    }));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(paulineCr);
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(michelleCr);

		BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageDisplayed,
		                              initialLaureStats.number_of_LinphoneMessageDisplayed + 1, 3000));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg] {
			bctbx_list_t *displayed_list =
			    linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateDisplayed);
			size_t displayed = bctbx_list_size(displayed_list);
			bctbx_list_free_with_data(displayed_list, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
			return (displayed == 2);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg2] {
			bctbx_list_t *displayed_list =
			    linphone_chat_message_get_participants_by_imdn_state(msg2, LinphoneChatMessageStateDisplayed);
			size_t displayed = bctbx_list_size(displayed_list);
			bctbx_list_free_with_data(displayed_list, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
			return (displayed == 3);
		}));

		if (invite_error || subscribe_error) {
			char *michelle2_proxy_contact_str = linphone_address_as_string(
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(michelle2.getLc())));
			ms_message("Enabling network of core %s (contact %s)", linphone_core_get_identity(michelle2.getLc()),
			           michelle2_proxy_contact_str);
			ms_free(michelle2_proxy_contact_str);
			initialMichelle2Stats = michelle2.getStats();
			linphone_core_set_network_reachable(michelle2.getLc(), TRUE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneRegistrationOk,
			                             initialMichelle2Stats.number_of_LinphoneRegistrationOk + 1,
			                             liblinphone_tester_sip_timeout));
			michelle2Cr = check_creation_chat_room_client_side(coresList, michelle2.getCMgr(), &initialMichelle2Stats,
			                                                   confAddr, initialSubject, 4, FALSE);
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneChatRoomSessionUpdating, 1, 1000));
			BC_ASSERT_EQUAL(michelle2.getStats().number_of_LinphoneChatRoomSessionUpdating, 0, int, "%0d");

			char *berthe_proxy_contact_str = linphone_address_as_string(
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(berthe.getLc())));
			ms_message("Enabling network of core %s (contact %s)", linphone_core_get_identity(berthe.getLc()),
			           berthe_proxy_contact_str);
			ms_free(berthe_proxy_contact_str);
			initialBertheStats = berthe.getStats();
			linphone_core_set_network_reachable(berthe.getLc(), TRUE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneRegistrationOk,
			                             initialBertheStats.number_of_LinphoneRegistrationOk + 1,
			                             liblinphone_tester_sip_timeout));
			bertheCr = check_creation_chat_room_client_side(coresList, berthe.getCMgr(), &initialBertheStats, confAddr,
			                                                initialSubject, 4, FALSE);
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &berthe.getStats().number_of_LinphoneChatRoomSessionUpdating, 1, 1000));
			BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneChatRoomSessionUpdating, 0, int, "%0d");
		}

		LinphoneChatMessage *michelle2LastMsg = NULL;
		if (!invite_error) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([michelle2Cr] {
				    return linphone_chat_room_get_history_size(michelle2Cr) == 2;
			    }));
			michelle2LastMsg = michelle2.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
			if (michelle2LastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
			}
			linphone_chat_room_mark_as_read(michelle2Cr);
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([bertheCr] {
			return linphone_chat_room_get_history_size(bertheCr) == 2;
		}));

		LinphoneChatMessage *bertheLastMsg = berthe.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
		if (bertheLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(bertheCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMichelleStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageDisplayed,
		                             initialLaureStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
		msg = nullptr;
		linphone_chat_message_unref(msg2);
		msg2 = nullptr;

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed,
		                             initialMarieStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed,
		                             initialMichelleStats.number_of_subject_changed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_subject_changed,
		                             initialMichelle2Stats.number_of_subject_changed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed,
		                             initialPaulineStats.number_of_subject_changed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed,
		                             initialLaureStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_subject_changed,
		                             initialBertheStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(michelleCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(michelle2Cr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(bertheCr), newSubject);

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelle2Cr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(bertheCr), 4, int, "%d");

		msg_text = "message marie blabla";
		msg = ClientConference::sendTextMsg(marieCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));
		paulineLastMsg = pauline.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);
		if (paulineLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([laureCr] {
			return linphone_chat_room_get_unread_messages_count(laureCr) == 1;
		}));
		LinphoneChatMessage *laureLastMsg = laure.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(laureLastMsg);
		if (laureLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([michelleCr] {
			    return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		    }));
		michelleLastMsg = michelle.getStats().last_received_chat_message;
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([michelle2Cr] {
			    return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		    }));
		michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([bertheCr] {
			return linphone_chat_room_get_unread_messages_count(bertheCr) == 1;
		}));
		bertheLastMsg = berthe.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
		if (bertheLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(michelle2Cr);
		linphone_chat_room_mark_as_read(paulineCr);
		linphone_chat_room_mark_as_read(laureCr);
		linphone_chat_room_mark_as_read(bertheCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		msg_text = "message michelle2 blabla";
		msg = ClientConference::sendTextMsg(michelle2Cr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));
		paulineLastMsg = pauline.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);
		if (paulineLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([bertheCr] {
			return linphone_chat_room_get_unread_messages_count(bertheCr) == 1;
		}));
		bertheLastMsg = berthe.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
		if (bertheLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([laureCr] {
			return linphone_chat_room_get_unread_messages_count(laureCr) == 1;
		}));
		laureLastMsg = laure.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(laureLastMsg);
		if (laureLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), msg_text.c_str());
		}

		michelleLastMsg = michelle.getStats().last_received_chat_message;
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(paulineCr);
		linphone_chat_room_mark_as_read(laureCr);
		linphone_chat_room_mark_as_read(bertheCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMichelle2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		msg_text = "message pauline blabla";
		msg = ClientConference::sendTextMsg(paulineCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([michelleCr] {
			    return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		    }));
		michelleLastMsg = michelle.getStats().last_received_chat_message;
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([bertheCr] {
			return linphone_chat_room_get_unread_messages_count(bertheCr) == 1;
		}));
		bertheLastMsg = berthe.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
		if (bertheLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([laureCr] {
			return linphone_chat_room_get_unread_messages_count(laureCr) == 1;
		}));
		laureLastMsg = laure.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(laureLastMsg);
		if (laureLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([michelle2Cr] {
			    return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		    }));
		michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(michelle2Cr);
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(laureCr);
		linphone_chat_room_mark_as_read(bertheCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageDisplayed,
		                             initialPaulineStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		CoreManagerAssert({focus, marie}).waitUntil(std::chrono::seconds(1), [] { return false; });

		CoreManagerAssert({focus, marie}).waitUntil(std::chrono::seconds(2), [] { return false; });

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure, berthe})
		    .waitUntil(chrono::seconds(2), [] { return false; });

		linphone_address_unref(confAddr);

		// to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

void group_chat_room_lime_server_message(bool encrypted) {
	Focus focus("chloe_rc");
	LinphoneChatMessage *msg;
	{ // to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		if (encrypted) {
			auto rawEncryptionSuccess = 0;

			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));

			// Test the raw encryption/decryption
			auto marieEncryptionEngine = L_GET_CPP_PTR_FROM_C_OBJECT(marie.getCMgr()->lc)->getEncryptionEngine();
			char *deviceId = linphone_address_as_string_uri_only(
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
			std::string marieAddressString{deviceId};
			bctbx_free(deviceId);
			auto paulineEncryptionEngine = L_GET_CPP_PTR_FROM_C_OBJECT(pauline.getCMgr()->lc)->getEncryptionEngine();
			deviceId = linphone_address_as_string_uri_only(
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline.getLc())));
			std::string paulineAddressString{deviceId};
			bctbx_free(deviceId);

			std::string messageString = "This is my message to you Rudy";
			std::string ADString = "These are my AD to you Rudy";
			std::vector<uint8_t> message(messageString.cbegin(), messageString.cend());
			std::vector<uint8_t> AD(ADString.cbegin(), ADString.cend());
			std::vector<uint8_t> cipherText{};

			marieEncryptionEngine->rawEncrypt(
			    marieAddressString, std::list<std::string>{paulineAddressString}, message, AD,
			    [&rawEncryptionSuccess, &cipherText, paulineAddressString](
			        const bool status, std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts) {
				    auto search = cipherTexts.find(paulineAddressString);
				    if (status && search != cipherTexts.end()) {
					    rawEncryptionSuccess++;
					    cipherText = cipherTexts[paulineAddressString];
				    }
			    });

			BC_ASSERT_TRUE(wait_for_list(coresList, &rawEncryptionSuccess, 1, x3dhServer_creationTimeout));
			if (rawEncryptionSuccess == 1) {
				// try to decrypt only if encryption was a success
				std::vector<uint8_t> plainText{};
				BC_ASSERT_TRUE(paulineEncryptionEngine->rawDecrypt(paulineAddressString, marieAddressString, AD,
				                                                   cipherText, plainText));
				std::string plainTextString{plainText.cbegin(), plainText.cend()};
				BC_ASSERT_TRUE(plainTextString == messageString);
			}
		}

		Address paulineAddr = pauline.getIdentity();
		Address laureAddr = laure.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr =
		    create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject,
		                                 encrypted, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat,
		                                                                 confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		if (paulineCr && laureCr) {
			// Marie sends the message
			const char *marieMessage = "Hey ! What's up ?";
			msg = _send_message_ephemeral(marieCr, marieMessage, FALSE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                             pauline_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *laureLastMsg = laure.getStats().last_received_chat_message;
			linphone_chat_message_unref(msg);
			if (paulineLastMsg && laureLastMsg) {
				// Check that the message was correctly decrypted if encrypted
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
				LinphoneAddress *marieAddr = marie.getCMgr()->identity;
				BC_ASSERT_TRUE(
				    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
				BC_ASSERT_TRUE(
				    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			}
		}

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

void one_to_one_group_chat_room_deletion_by_server_client_base(bool encrypted) {
	Focus focus("chloe_rc");
	LinphoneChatMessage *msg;
	{ // to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		}

		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr =
		    create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject,
		                                 encrypted, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 1, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);

		if (paulineCr && marieCr) {
			// Marie sends the message
			const char *marieMessage = "Hey ! What's up ?";
			msg = _send_message(marieCr, marieMessage);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                             pauline_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
			linphone_chat_message_unref(msg);
			BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);

			// Check that the message was correctly decrypted if encrypted
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
			LinphoneAddress *marieAddr = marie.getCMgr()->identity;
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

			LinphoneAddress *paulineLocalAddr = linphone_address_clone(linphone_chat_room_get_local_address(paulineCr));
			LinphoneAddress *paulinePeerAddr = linphone_address_clone(linphone_chat_room_get_peer_address(paulineCr));

			// Restart pauline so that she has to send an INVITE and BYE it to exit the chatroom
			coresList = bctbx_list_remove(coresList, pauline.getLc());
			pauline.reStart();
			coresList = bctbx_list_append(coresList, pauline.getLc());
			paulineCr = linphone_core_search_chat_room(pauline.getLc(), NULL, paulineLocalAddr, paulinePeerAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(paulineCr);

			LinphoneChatRoom *focusCr =
			    linphone_core_search_chat_room(focus.getLc(), NULL, NULL, paulinePeerAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(focusCr);

			LinphoneParticipant *paulineParticipant = NULL;
			LinphoneParticipantDevice *paulineDevice = NULL;

			if (focusCr) {
				paulineParticipant = linphone_chat_room_find_participant(focusCr, paulineLocalAddr);
				BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
				if (paulineParticipant) {
					paulineDevice = linphone_participant_find_device(paulineParticipant, paulineLocalAddr);
					BC_ASSERT_PTR_NOT_NULL(paulineDevice);
				}
			}

			linphone_address_unref(paulineLocalAddr);
			linphone_address_unref(paulinePeerAddr);

			OrtpNetworkSimulatorParams simparams = {0};
			simparams.mode = OrtpNetworkSimulatorOutbound;
			simparams.enabled = TRUE;
			simparams.max_bandwidth = 430000; /*we first limit to 430 kbit/s*/
			simparams.max_buffer_size = (int)simparams.max_bandwidth;
			simparams.latency = 60;
			simparams.loss_rate = 90;
			linphone_core_set_network_simulator_params(pauline.getLc(), &simparams);
			linphone_core_set_network_simulator_params(focus.getLc(), &simparams);

			linphone_core_delete_chat_room(marie.getLc(), marieCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted,
			                             marie_stat.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));

			// wait until chatroom is deleted server side
			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&paulineDevice] {
				return (paulineDevice) ? (linphone_participant_device_get_state(paulineDevice) ==
				                          LinphoneParticipantDeviceStateLeaving)
				                       : false;
			}));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneChatRoomSessionReleased, 1,
			                             liblinphone_tester_sip_timeout));

			linphone_core_delete_chat_room(pauline.getLc(), paulineCr);
			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&pauline] {
				auto &chatRooms = pauline.getCore().getChatRooms();
				for (auto chatRoom : chatRooms) {
					if (chatRoom->getState() != ConferenceInterface::State::Deleted) {
						return false;
					}
				}
				return true;
			}));
		}
	}
}

} // namespace LinphoneTest
