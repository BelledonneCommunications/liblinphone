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
#include "linphone/api/c-participant-imdn-state.h"
#include "local-conference-tester-functions.h"

namespace LinphoneTest {

static void group_chat_room_with_imdn_base(bool_t core_goes_offline) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t encrypted = FALSE;
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference marie2("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline2("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(marie2);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(pauline2);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline2.getLc()));

		stats marie_stat = marie.getStats();
		stats marie2_stat = marie.getStats();
		stats michelle_stat = michelle.getStats();
		stats michelle2_stat = michelle2.getStats();
		stats pauline_stat = pauline.getStats();
		stats pauline2_stat = pauline2.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, marie2.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, pauline2.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_X3dhUserCreationSuccess,
			                             marie_stat.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_X3dhUserCreationSuccess,
			                             marie2_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_X3dhUserCreationSuccess,
			                             michelle_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_X3dhUserCreationSuccess,
			                             michelle2_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_X3dhUserCreationSuccess,
			                             pauline_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_X3dhUserCreationSuccess,
			                             pauline2_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));

			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2.getLc()));
		}

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));
		Address paulineAddr = pauline.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));
		Address pauline2Addr = pauline2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(pauline2Addr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, 2, encrypted,
		    LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat,
		                                                                  confAddr, initialSubject, 2, TRUE);
		BC_ASSERT_PTR_NOT_NULL(marie2Cr);

		// Check that the chat room is correctly created on Michelle's side and that the participants are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &michelle_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &michelle2_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelle2Cr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *pauline2Cr = check_creation_chat_room_client_side(
		    coresList, pauline2.getCMgr(), &pauline2_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(pauline2Cr);

		if (!!core_goes_offline) {
			const std::initializer_list<std::reference_wrapper<ClientConference>> cores2{pauline, michelle2};
			for (const ClientConference &core : cores2) {
				Address contactAddress = *Address::toCpp(
				    linphone_account_get_contact_address(linphone_core_get_default_account(core.getLc())));
				ms_message("%s (contact %s) goes offline", linphone_core_get_identity(core.getLc()),
				           contactAddress.toString().c_str());
				linphone_core_set_network_reachable(pauline.getLc(), FALSE);
			}
		}

		// Marie sends the message
		const char *marieMessage = "Hey ! What's up ?";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(marieCr, marieMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneMessageReceived,
		                             marie2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *marie2LastMsg = marie2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marie2LastMsg);
		if (marie2LastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(marie2LastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageReceived,
		                             michelle_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(michelleLastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageReceived,
		                             pauline2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *pauline2LastMsg = pauline2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(pauline2LastMsg);
		if (pauline2LastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(pauline2LastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}

		if (!!core_goes_offline) {
			// wait a bit longer to detect side effect if any
			CoreManagerAssert({focus, marie, marie2, michelle, michelle2, pauline, pauline2})
			    .waitUntil(chrono::seconds(2), [] { return false; });

			const std::initializer_list<std::reference_wrapper<ClientConference>> cores2{pauline, michelle2};
			for (const ClientConference &core : cores2) {
				Address contactAddress = *Address::toCpp(
				    linphone_account_get_contact_address(linphone_core_get_default_account(core.getLc())));
				ms_message("%s (contact %s) comes back online", linphone_core_get_identity(core.getLc()),
				           contactAddress.toString().c_str());
				linphone_core_set_network_reachable(pauline.getLc(), TRUE);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
		                             pauline_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);
		if (paulineLastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(paulineLastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageReceived,
		                             michelle2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(michelle2LastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2, pauline, pauline2})
		    .waitUntil(chrono::seconds(2), [] { return false; });

		const std::initializer_list<std::reference_wrapper<ClientConference>> cores2{pauline, michelle};
		for (const ClientConference &core : cores2) {
			Address contactAddress =
			    *Address::toCpp(linphone_account_get_contact_address(linphone_core_get_default_account(core.getLc())));
			ms_message("%s (contact %s) marks message as read", linphone_core_get_identity(core.getLc()),
			           contactAddress.toString().c_str());
			LinphoneChatRoom *cr =
			    linphone_core_search_chat_room(core.getLc(), NULL, contactAddress.toC(), confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(cr);
			if (cr) {
				linphone_chat_room_mark_as_read(cr);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             marie_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneMessageDisplayed,
		                             marie2_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageDisplayed,
		                             michelle_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             michelle2_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageDisplayed,
		                             pauline_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageDisplayed,
		                             pauline2_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
		msg = nullptr;

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2, pauline, pauline2}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2, pauline, pauline2})
		    .waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_with_imdn(void) {
	group_chat_room_with_imdn_base(FALSE);
}

static void group_chat_room_with_imdn_and_core_restarts(void) {
	group_chat_room_with_imdn_base(TRUE);
}

static void
group_chat_room_with_client_idmn_after_restart_base(bool_t encrypted, bool_t add_participant, bool_t stop_core) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(pauline);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		stats michelle_stat = michelle.getStats();
		stats michelle2_stat = michelle2.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));
		}

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));
		Address laureAddr = laure.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));
		Address paulineAddr = pauline.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, 3, encrypted,
		    LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);
		char *conference_address = linphone_address_as_string(confAddr);

		// Check that the chat room is correctly created on Michelle's side and that the participants are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &michelle_stat, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &michelle2_stat, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelle2Cr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);

		// Check that the chat room is correctly created on Laure's side and that the participants are added
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat,
		                                                                 confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure}).wait([&focus] {
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             michelle_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             michelle2_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreated,
		                             laure_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated,
		                             pauline_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		ms_message("%s goes offline", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), FALSE);

		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), encrypted);
		focus.registerAsParticipantDevice(berthe);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(berthe.getLc()));
		stats berthe_stat = berthe.getStats();
		coresList = bctbx_list_append(coresList, berthe.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();
		michelle_stat = michelle.getStats();
		michelle2_stat = michelle2.getStats();
		LinphoneChatRoom *bertheCr = NULL;
		if (add_participant) {
			Address bertheAddr = berthe.getIdentity();
			// Add and remove Berthe in order to create a gap between Laure's last notify and the actual notify of the
			// chatroom on the server side so that a full state is triggered
			for (int cnt = 0; cnt <= 6; cnt++) {
				marie_stat = marie.getStats();
				berthe_stat = berthe.getStats();
				pauline_stat = pauline.getStats();
				michelle_stat = michelle.getStats();
				michelle2_stat = michelle2.getStats();
				ms_message("Try #%0d - %s adds %s to chatroom %s", cnt, linphone_core_get_identity(marie.getLc()),
				           linphone_core_get_identity(berthe.getLc()), conference_address);
				linphone_chat_room_add_participant(marieCr, bertheAddr.toC());
				bertheCr = check_creation_chat_room_client_side(coresList, berthe.getCMgr(), &berthe_stat, confAddr,
				                                                initialSubject, 4, FALSE);
				// wait until chatroom is deleted from Berthe's standpoint
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&berthe] {
					return berthe.getCore().getChatRooms().size() == 1;
				}));
				BC_ASSERT_PTR_NOT_NULL(bertheCr);
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
				                             pauline_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
				                             pauline_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
				                             michelle_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
				                             michelle_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participants_added,
				                             michelle2_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participant_devices_added,
				                             michelle2_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&focus] {
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
				ms_message("Try #%0d - %s removes %s to chatroom %s", cnt, linphone_core_get_identity(marie.getLc()),
				           linphone_core_get_identity(berthe.getLc()), conference_address);
				LinphoneParticipant *bertheParticipant = linphone_chat_room_find_participant(marieCr, bertheAddr.toC());
				BC_ASSERT_PTR_NOT_NULL(bertheParticipant);
				if (bertheParticipant) {
					linphone_chat_room_remove_participant(marieCr, bertheParticipant);
				}
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&berthe] {
					for (auto chatRoom : berthe.getCore().getChatRooms()) {
						if (chatRoom->getState() != ConferenceInterface::State::Terminated) {
							return false;
						}
					}
					return true;
				}));
				if (bertheCr) {
					linphone_core_delete_chat_room(berthe.getLc(), bertheCr);
				}
				// wait until chatroom is deleted from Berthe's standpoint
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&berthe] {
					return berthe.getCore().getChatRooms().size() == 0;
				}));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
				                             pauline_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed,
				                             michelle_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participants_removed,
				                             michelle2_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}
			marie_stat = marie.getStats();
			berthe_stat = berthe.getStats();
			pauline_stat = pauline.getStats();
			michelle_stat = michelle.getStats();
			michelle2_stat = michelle2.getStats();
			ms_message("%s adds %s to chatroom %s one more time", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(berthe.getLc()), conference_address);
			linphone_chat_room_add_participant(marieCr, bertheAddr.toC());
			bertheCr = check_creation_chat_room_client_side(coresList, berthe.getCMgr(), &berthe_stat, confAddr,
			                                                initialSubject, 4, FALSE);
			BC_ASSERT_PTR_NOT_NULL(bertheCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
			                             pauline_stat.number_of_participants_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
			                             pauline_stat.number_of_participant_devices_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
			                             michelle_stat.number_of_participants_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
			                             michelle_stat.number_of_participant_devices_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participants_added,
			                             michelle2_stat.number_of_participants_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participant_devices_added,
			                             michelle2_stat.number_of_participant_devices_added + 1, 5000));
		}

		std::string msg_text = "message pauline blabla";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(paulineCr, msg_text);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		LinphoneChatMessage *marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([michelleCr] {
			    return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		    }));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([michelle2Cr] {
			    return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		    }));
		LinphoneChatMessage *michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}

		if (bertheCr) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([bertheCr] {
				    return linphone_chat_room_get_unread_messages_count(bertheCr) == 1;
			    }));
			LinphoneChatMessage *bertheLastMsg = berthe.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
			if (bertheLastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
			}
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(michelle2Cr);
		if (bertheCr) {
			linphone_chat_room_mark_as_read(bertheCr);
		}
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(paulineCr);

		for (const auto client : {marie.getCMgr(), michelle.getCMgr(), michelle2.getCMgr(), berthe.getCMgr(),
		                          laure.getCMgr(), pauline.getCMgr()}) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline})
			        .wait([client, &berthe, &laure, &pauline, &add_participant, &msg] {
				        bool ret = false;
				        LinphoneChatMessage *lastMsg =
				            (client->lc == pauline.getLc()) ? msg : client->stat.last_received_chat_message;
				        if ((client->lc == laure.getLc()) || (!add_participant && (client->lc == berthe.getLc()))) {
					        ret = (lastMsg == nullptr);
				        } else {
					        ret = (lastMsg != nullptr);
					        if (lastMsg) {
						        bctbx_list_t *displayed_list = linphone_chat_message_get_participants_by_imdn_state(
						            lastMsg, LinphoneChatMessageStateDisplayed);
						        size_t expected_displayed_number = 2 + (add_participant ? 1 : 0);
						        ret &= (bctbx_list_size(displayed_list) == expected_displayed_number);
						        bctbx_list_free_with_data(displayed_list,
						                                  (bctbx_list_free_func)linphone_participant_imdn_state_unref);
					        }
				        }
				        return ret;
			        }));
		}

		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();
		ms_message("%s comes back online", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
		                             laure_stat.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
		LinphoneAddress *laureDeviceAddr = linphone_address_clone(
		    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(laure.getLc())));
		laureCr = linphone_core_search_chat_room(laure.getLc(), NULL, laureDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		char *uuid = NULL;
		if (linphone_config_get_string(linphone_core_get_config(laure.getLc()), "misc", "uuid", NULL)) {
			uuid =
			    bctbx_strdup(linphone_config_get_string(linphone_core_get_config(laure.getLc()), "misc", "uuid", NULL));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_NotifyReceived,
		                             laure_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));
		if (laureCr) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([laureCr] {
				    return linphone_chat_room_get_unread_messages_count(laureCr) == 1;
			    }));
			linphone_chat_room_mark_as_read(laureCr);
			// wait a bit longer to make sure that the IMDN of the display state has been sent out
			CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure}).waitUntil(chrono::seconds(1), [] {
				return false;
			});
			if (stop_core) {
				ms_message("%s stops its core", linphone_core_get_identity(laure.getLc()));
				coresList = bctbx_list_remove(coresList, laure.getLc());
				linphone_core_manager_stop(laure.getCMgr());
			}
		}
		linphone_address_unref(laureDeviceAddr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageDisplayed,
		                             pauline_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		if (stop_core) {
			linphone_core_manager_configure(laure.getCMgr());
			// Make sure gruu is preserved
			linphone_config_set_string(linphone_core_get_config(laure.getLc()), "misc", "uuid", uuid);
			ms_message("%s starts its core", linphone_core_get_identity(laure.getLc()));
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));
			linphone_core_manager_start(laure.getCMgr(), TRUE);
			coresList = bctbx_list_append(coresList, laure.getLc());
		}

		if (uuid) {
			bctbx_free(uuid);
		}

		for (const auto client : {marie.getCMgr(), michelle.getCMgr(), michelle2.getCMgr(), berthe.getCMgr(),
		                          laure.getCMgr(), pauline.getCMgr()}) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline})
			        .wait([client, &berthe, &add_participant, &confAddr] {
				        const LinphoneAddress *deviceAddr =
				            linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
				        LinphoneChatRoom *cr =
				            linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
				        LinphoneChatMessage *lastMsg = cr ? linphone_chat_room_get_last_message_in_history(cr) : NULL;
				        bool ret = false;
				        if (!add_participant && (client->lc == berthe.getLc())) {
					        ret = (lastMsg == nullptr);
				        } else {
					        ret = (lastMsg != nullptr);
					        if (lastMsg) {
						        bctbx_list_t *displayed_list = linphone_chat_message_get_participants_by_imdn_state(
						            lastMsg, LinphoneChatMessageStateDisplayed);
						        size_t expected_displayed_number = 3 + (add_participant ? 1 : 0);
						        ret &= (bctbx_list_size(displayed_list) == expected_displayed_number);
						        bctbx_list_free_with_data(displayed_list,
						                                  (bctbx_list_free_func)linphone_participant_imdn_state_unref);
					        }
				        }
				        return ret;
			        }));
		}

		linphone_chat_message_unref(msg);
		msg = nullptr;

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, berthe, laure}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle, michelle2, berthe, laure})
		    .waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		ms_free(conference_address);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_with_client_idmn_after_restart(void) {
	group_chat_room_with_client_idmn_after_restart_base(FALSE, TRUE, FALSE);
}

static void secure_group_chat_room_with_client_idmn_sent_after_restart(void) {
	group_chat_room_with_client_idmn_after_restart_base(TRUE, FALSE, FALSE);
}

static void secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added(void) {
	group_chat_room_with_client_idmn_after_restart_base(TRUE, TRUE, FALSE);
}

static void secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added_and_core_stopped(void) {
	group_chat_room_with_client_idmn_after_restart_base(TRUE, TRUE, TRUE);
}

static void group_chat_room_lime_session_corrupted(void) {
	Focus focus("chloe_rc");
	LinphoneChatMessage *msg;
	{ // to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), true);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));

		Address paulineAddr = pauline.getIdentity();
		Address laureAddr = laure.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr =
		    create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject,
		                                 TRUE, LinphoneChatRoomEphemeralModeDeviceManaged);
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
			msg = _send_message(marieCr, marieMessage);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                             pauline_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *laureLastMsg = laure.getStats().last_received_chat_message;
			linphone_chat_message_unref(msg);
			if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg)) goto end;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) goto end;

			// Check that the message was correctly decrypted if encrypted
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
			LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);

			Linphone::Tester::CoreManagerAssert({marie, laure, pauline}).waitUntil(std::chrono::seconds(3), [] {
				return false;
			});

			// Corrupt Pauline sessions in lime database: WARNING: if SOCI is not found, this call does nothing and the
			// test fails
			lime_delete_DRSessions(pauline.getCMgr()->lime_database_path,
			                       " WHERE Did = (SELECT Did FROM lime_PeerDevices WHERE DeviceId LIKE 'sip:marie%')");
			// restart core to force lime manager cache reload
			coresList = bctbx_list_remove(coresList, pauline.getLc());
			pauline.reStart();
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
			pauline_stat = pauline.getStats();
			coresList = bctbx_list_append(coresList, pauline.getLc());

			// Marie send a new message, it shall fail and get a 488 response
			const char *marieTextMessage2 = "Do you copy?";
			msg = _send_message(marieCr, marieTextMessage2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
			                             marie_stat.number_of_LinphoneMessageDelivered + 2,
			                             liblinphone_tester_sip_timeout)); // Delivered to the server
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 2,
			                             liblinphone_tester_sip_timeout)); // the message is correctly received by Laure
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageNotDelivered,
			                             marie_stat.number_of_LinphoneMessageNotDelivered + 1,
			                             liblinphone_tester_sip_timeout)); // Not delivered to pauline
			BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneMessageReceived, 0, int, "%d");
			linphone_chat_message_unref(msg);
			laureLastMsg = laure.getStats().last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) goto end;
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage2);
			marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);

			// Try again, it shall work this time
			const char *marieTextMessage3 = "Hello again";
			marie_stat = marie.getStats();
			msg = _send_message(marieCr, marieTextMessage3);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageSent, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
			                             marie_stat.number_of_LinphoneMessageDelivered + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                             pauline_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
			                             marie_stat.number_of_LinphoneMessageDeliveredToUser + 1,
			                             liblinphone_tester_sip_timeout));
			paulineLastMsg = pauline.getStats().last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg)) goto end;
			laureLastMsg = laure.getStats().last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) goto end;
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieTextMessage3);
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage3);
			marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);
			linphone_chat_message_unref(msg);
		}

	end:
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

static void group_chat_room_lime_server_clear_message(void) {
	group_chat_room_lime_server_message(FALSE);
}

static void secure_group_chat_message_state_transition_to_displayed(bool corrupt_lime_dr_session) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference pauline2("pauline_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), true);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(pauline2);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats pauline2_stat = pauline2.getStats();
		stats laure_stat = laure.getStats();
		stats michelle_stat = michelle.getStats();

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, pauline2.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		// Wait for lime user creation
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));

		Address paulineAddr = pauline.getIdentity();
		Address michelleAddr = michelle.getIdentity();
		Address laureAddr = laure.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr =
		    create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject,
		                                 TRUE, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *pauline2Cr = check_creation_chat_room_client_side(
		    coresList, pauline2.getCMgr(), &pauline2_stat, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(pauline2Cr);
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat,
		                                                                 confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &michelle_stat, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);

		std::string msg_text0("Welcome everybody");
		LinphoneChatMessage *msg0 = ClientConference::sendTextMsg(marieCr, msg_text0);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
		                             marie_stat.number_of_LinphoneMessageDelivered + 1,
		                             liblinphone_tester_sip_timeout)); // Delivered to the server
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
		                             laure_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Laure
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
		                             pauline_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Pauline
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageReceived,
		                             pauline2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Pauline2
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageReceived,
		                             michelle_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Michelle
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
		                             marie_stat.number_of_LinphoneMessageDelivered + 1,
		                             liblinphone_tester_sip_timeout)); // Message is received by everybody
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
		                             marie_stat.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout)); // Message is received by everybody

		Address paulineContactAddress =
		    *Address::toCpp(linphone_account_get_contact_address(linphone_core_get_default_account(pauline.getLc())));
		Address pauline2ContactAddress =
		    *Address::toCpp(linphone_account_get_contact_address(linphone_core_get_default_account(pauline2.getLc())));
		Address michelleContactAddress =
		    *Address::toCpp(linphone_account_get_contact_address(linphone_core_get_default_account(michelle.getLc())));

		// Pauline2 goes offline so that she cannot receives the next message
		ms_message("%s turns the network off", pauline2ContactAddress.toString().c_str());
		linphone_core_set_network_reachable(pauline2.getLc(), FALSE);

		// Pauline goes offline so that she cannot receives the next message
		ms_message("%s turns the network off", paulineContactAddress.toString().c_str());
		linphone_core_set_network_reachable(pauline.getLc(), FALSE);
		if (corrupt_lime_dr_session) {
			LinphoneAddress *paulineCrAddr = linphone_address_clone(linphone_chat_room_get_peer_address(paulineCr));
			coresList = bctbx_list_remove(coresList, pauline.getLc());

			ms_message("%s deletes %s's DR session", paulineContactAddress.toString().c_str(),
			           linphone_core_get_identity(marie.getLc()));
			// Corrupt Pauline sessions in lime database: WARNING: if SOCI is not found, this call does nothing and the
			// test fails Delete only the session linked to Marie
			lime_delete_DRSessions(pauline.getCMgr()->lime_database_path,
			                       " WHERE Did = (SELECT Did FROM lime_PeerDevices WHERE DeviceId LIKE 'sip:marie%')");
			pauline_stat = pauline.getStats();
			ms_message("%s restarts its core", paulineContactAddress.toString().c_str());
			pauline.reStart();
			coresList = bctbx_list_append(coresList, pauline.getLc());
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
			LinphoneImNotifPolicy *paulinePolicy = linphone_core_get_im_notif_policy(pauline.getLc());
			linphone_im_notif_policy_set_send_imdn_delivered(paulinePolicy, FALSE);

			paulineCr = linphone_core_search_chat_room(pauline.getLc(), NULL, NULL, paulineCrAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(paulineCr);
			linphone_address_unref(paulineCrAddr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_X3dhUserCreationSuccess, 1,
			                             x3dhServer_creationTimeout));
		}

		// Michelle goes offline so that she cannot receives the next message
		ms_message("%s turns the network off", michelleContactAddress.toString().c_str());
		linphone_core_set_network_reachable(michelle.getLc(), FALSE);

		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		pauline2_stat = pauline2.getStats();
		laure_stat = laure.getStats();
		michelle_stat = michelle.getStats();

		// Marie send a new message, Pauline and Michelle do not receive it as they're offline
		std::string msg_text1("Do you copy?");
		LinphoneChatMessage *msg1 = ClientConference::sendTextMsg(marieCr, msg_text1);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
		                             marie_stat.number_of_LinphoneMessageDelivered + 1,
		                             liblinphone_tester_sip_timeout)); // Delivered to the server
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
		                             laure_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Laure
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageReceived,
		                              pauline2_stat.number_of_LinphoneMessageReceived + 1,
		                              3000)); // the message is correctly not received by Pauline2 because she's offline
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageReceived,
		                              michelle_stat.number_of_LinphoneMessageReceived + 1,
		                              3000)); // the message is correctly not received by Pauline2 because she's offline

		if (corrupt_lime_dr_session) {
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &pauline.getStats().number_of_LinphoneMessageReceivedFailedToDecrypt,
			                             pauline_stat.number_of_LinphoneMessageReceivedFailedToDecrypt + 1,
			                             liblinphone_tester_sip_timeout)); // Pauline fails to decrypt
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageNotDelivered,
			                             marie_stat.number_of_LinphoneMessageNotDelivered + 1,
			                             liblinphone_tester_sip_timeout)); // Not delivered to pauline
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
			                             marie_stat.number_of_LinphoneMessageDelivered + 1,
			                             liblinphone_tester_sip_timeout)); // Not delivered to pauline
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                  pauline_stat.number_of_LinphoneMessageReceived + 1,
			                  3000)); // the message is correctly not received by Pauline2 because she's offline
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle})
		                   .wait([&msg1, corrupt_lime_dr_session] {
			                   return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
			                               msg1, LinphoneChatMessageStateNotDelivered)) ==
			                           ((corrupt_lime_dr_session) ? 1 : 0));
		                   }));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle})
		                   .wait([&msg1, corrupt_lime_dr_session] {
			                   return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
			                               msg1, LinphoneChatMessageStateDelivered)) ==
			                           ((corrupt_lime_dr_session) ? 1 : 2));
		                   }));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&msg1] {
			return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
			            msg1, LinphoneChatMessageStateDeliveredToUser)) == 1);
		}));

		bctbx_list_t *not_delivered_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateNotDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(not_delivered_list0), (corrupt_lime_dr_session) ? 1 : 0, size_t, "%zu");
		bctbx_list_free_with_data(not_delivered_list0, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		// Checks on Marie's side
		bctbx_list_t *delivered_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_list0), (corrupt_lime_dr_session) ? 1 : 2, size_t, "%zu");
		bctbx_list_free_with_data(delivered_list0, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		bctbx_list_t *delivered_to_user_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDeliveredToUser);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_to_user_list0), 1, size_t, "%zu");
		bctbx_list_free_with_data(delivered_to_user_list0, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		BC_ASSERT_EQUAL(
		    (int)linphone_chat_message_get_state(msg1),
		    (int)(corrupt_lime_dr_session ? LinphoneChatMessageStateNotDelivered : LinphoneChatMessageStateDelivered),
		    int, "%0d");

		// Checks on Laure's side
		LinphoneChatMessage *laureLastMsg = laureCr ? linphone_chat_room_get_last_message_in_history(laureCr) : NULL;

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&laureLastMsg] {
			return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
			            laureLastMsg, LinphoneChatMessageStateDeliveredToUser)) == 1);
		}));

		bctbx_list_t *laure_not_delivered_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(laureLastMsg, LinphoneChatMessageStateNotDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(laure_not_delivered_list0), (corrupt_lime_dr_session) ? 1 : 0, size_t, "%zu");
		bctbx_list_free_with_data(laure_not_delivered_list0,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		bctbx_list_t *laure_delivered_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(laureLastMsg, LinphoneChatMessageStateDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(laure_delivered_list0), (corrupt_lime_dr_session) ? 1 : 2, size_t, "%zu");
		bctbx_list_free_with_data(laure_delivered_list0, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		bctbx_list_t *laure_delivered_to_user_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(laureLastMsg, LinphoneChatMessageStateDeliveredToUser);
		BC_ASSERT_EQUAL(bctbx_list_size(laure_delivered_to_user_list0), 1, size_t, "%zu");
		bctbx_list_free_with_data(laure_delivered_to_user_list0,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		bctbx_list_t *laure_displayed_list0 =
		    linphone_chat_message_get_participants_by_imdn_state(laureLastMsg, LinphoneChatMessageStateDisplayed);
		BC_ASSERT_EQUAL(bctbx_list_size(laure_displayed_list0), 0, size_t, "%zu");
		bctbx_list_free_with_data(laure_displayed_list0, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		BC_ASSERT_EQUAL(
		    (int)linphone_chat_message_get_state(laureLastMsg),
		    (int)(corrupt_lime_dr_session ? LinphoneChatMessageStateNotDelivered : LinphoneChatMessageStateDelivered),
		    int, "%0d");

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneMessageDisplayed, 0, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneMessageDisplayed, 0, int, "%d");
		BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneMessageReceived, 1, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneMessageReceived, (corrupt_lime_dr_session ? 0 : 1), int,
		                "%d");
		BC_ASSERT_EQUAL(pauline2.getStats().number_of_LinphoneMessageReceived, 1, int, "%d");
		BC_ASSERT_EQUAL(laure.getStats().number_of_LinphoneMessageReceived, 2, int, "%d");

		marie_stat = marie.getStats();
		pauline2_stat = pauline2.getStats();

		// Pauline2 comes back online again and she should now receive Marie's message
		ms_message("%s comes back online", pauline2ContactAddress.toString().c_str());
		linphone_core_set_network_reachable(pauline2.getLc(), TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageReceived,
		                             pauline2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Pauline2
		BC_ASSERT_EQUAL(pauline2.getStats().number_of_LinphoneMessageReceived, 2, int, "%d");

		const LinphoneAddress *pauline2DeviceAddr =
		    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline2.getLc()));
		pauline2Cr = linphone_core_search_chat_room(pauline2.getLc(), NULL, pauline2DeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(pauline2Cr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&msg1] {
			return (linphone_chat_message_get_state(msg1) == LinphoneChatMessageStateDelivered);
		}));

		// wait until chat message reaches the DeliveredToUser state
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&msg1] {
			return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
			            msg1, LinphoneChatMessageStateDeliveredToUser)) == 2);
		}));

		// Checks on clients'side
		for (const auto client : {laure.getCMgr(), pauline2.getCMgr()}) {
			const LinphoneAddress *deviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
			LinphoneChatRoom *clientCr = linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(clientCr);
			LinphoneChatMessage *lastMsg = clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);

			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&lastMsg] {
				return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
				            lastMsg, LinphoneChatMessageStateDeliveredToUser)) == 2);
			}));
		}

		// Not delivered
		bctbx_list_t *not_delivered_list1 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateNotDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(not_delivered_list1), 0, size_t, "%zu");
		bctbx_list_free_with_data(not_delivered_list1, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		// Delivered
		bctbx_list_t *delivered_list1 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_list1), 1, size_t, "%zu");
		bctbx_list_free_with_data(delivered_list1, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		// Delivered to user
		bctbx_list_t *delivered_to_user_list1 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDeliveredToUser);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_to_user_list1), 2, size_t, "%zu");
		bctbx_list_free_with_data(delivered_to_user_list1, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		// Checks on clients'side
		for (const auto client : {laure.getCMgr(), michelle.getCMgr()}) {
			const LinphoneAddress *deviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
			LinphoneChatRoom *clientCr = linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(clientCr);
			LinphoneChatMessage *lastMsg = clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);

			bctbx_list_t *client_not_delivered =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateNotDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(client_not_delivered), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_not_delivered,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_delivered =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(client_delivered), 1, size_t, "%zu");
			bctbx_list_free_with_data(client_delivered, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_delivered_to_user =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDeliveredToUser);
			BC_ASSERT_EQUAL(bctbx_list_size(client_delivered_to_user), 2, size_t, "%zu");
			bctbx_list_free_with_data(client_delivered_to_user,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_displayed =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDisplayed);
			BC_ASSERT_EQUAL(bctbx_list_size(client_displayed), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_displayed, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(lastMsg), (int)LinphoneChatMessageStateDelivered, int,
			                "%0d");
		}

		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();

		// Michelle comes back online again and she should now receive Marie's message
		ms_message("%s comes back online", michelleContactAddress.toString().c_str());
		linphone_core_set_network_reachable(michelle.getLc(), TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageReceived,
		                             michelle_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout)); // the message is correctly received by Michelle
		BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneMessageReceived, 2, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
		                             marie_stat.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));

		const LinphoneAddress *michelleDeviceAddr =
		    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(michelle.getLc()));
		michelleCr = linphone_core_search_chat_room(michelle.getLc(), NULL, michelleDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);

		// wait until chat message reaches the DeliveredToUser state
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&msg1] {
			return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
			            msg1, LinphoneChatMessageStateDeliveredToUser)) == 3);
		}));

		// Checks on clients'side
		for (const auto client : {laure.getCMgr(), pauline2.getCMgr(), michelle.getCMgr()}) {
			const LinphoneAddress *deviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
			LinphoneChatRoom *clientCr = linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(clientCr);
			LinphoneChatMessage *lastMsg = clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);

			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&lastMsg] {
				return (linphone_chat_message_get_state(lastMsg) == LinphoneChatMessageStateDeliveredToUser);
			}));

			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&lastMsg] {
				return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
				            lastMsg, LinphoneChatMessageStateDeliveredToUser)) == 3);
			}));
		}

		// Not delivered
		bctbx_list_t *not_delivered_list2 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateNotDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(not_delivered_list2), 0, size_t, "%zu");
		bctbx_list_free_with_data(not_delivered_list2, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
		BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(msg1), (int)LinphoneChatMessageStateDeliveredToUser, int,
		                "%0d");

		// Delivered
		bctbx_list_t *delivered_list2 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_list2), 0, size_t, "%zu");
		bctbx_list_free_with_data(delivered_list2, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		// Delivered to user
		bctbx_list_t *delivered_to_user_list2 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDeliveredToUser);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_to_user_list2), 3, size_t, "%zu");
		bctbx_list_free_with_data(delivered_to_user_list2, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(msg1), (int)LinphoneChatMessageStateDeliveredToUser, int,
		                "%0d");

		// Checks on clients'side
		for (const auto client : {laure.getCMgr(), pauline2.getCMgr(), michelle.getCMgr()}) {
			const LinphoneAddress *deviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
			LinphoneChatRoom *clientCr = linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(clientCr);
			LinphoneChatMessage *lastMsg = clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);

			bctbx_list_t *client_not_delivered =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateNotDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(client_not_delivered), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_not_delivered,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_delivered =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(client_delivered), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_delivered, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_delivered_to_user =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDeliveredToUser);
			BC_ASSERT_EQUAL(bctbx_list_size(client_delivered_to_user), 3, size_t, "%zu");
			bctbx_list_free_with_data(client_delivered_to_user,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_displayed =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDisplayed);
			BC_ASSERT_EQUAL(bctbx_list_size(client_displayed), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_displayed, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(lastMsg), (int)LinphoneChatMessageStateDeliveredToUser,
			                int, "%0d");
		}

		BC_ASSERT_EQUAL(marie.getCore().getChatRooms().size(), 1, size_t, "%zu");
		for (auto &chatRoom : marie.getCore().getChatRooms()) {
			chatRoom->markAsRead();
		}

		BC_ASSERT_EQUAL(pauline2.getCore().getChatRooms().size(), 1, size_t, "%zu");
		for (auto &chatRoom : pauline2.getCore().getChatRooms()) {
			chatRoom->markAsRead();
		}

		BC_ASSERT_EQUAL(laure.getCore().getChatRooms().size(), 1, size_t, "%zu");
		for (auto &chatRoom : laure.getCore().getChatRooms()) {
			chatRoom->markAsRead();
		}

		BC_ASSERT_EQUAL(michelle.getCore().getChatRooms().size(), 1, size_t, "%zu");
		for (auto &chatRoom : michelle.getCore().getChatRooms()) {
			chatRoom->markAsRead();
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             marie_stat.number_of_LinphoneMessageDisplayed + 2,
		                             liblinphone_tester_sip_timeout));

		// wait until chat message reaches the Displayed state
		// Checks on clients'side
		for (const auto client : {laure.getCMgr(), pauline2.getCMgr(), michelle.getCMgr()}) {
			const LinphoneAddress *deviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
			LinphoneChatRoom *clientCr = linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(clientCr);
			LinphoneChatMessage *lastMsg = clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);

			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&lastMsg] {
				return (linphone_chat_message_get_state(lastMsg) == LinphoneChatMessageStateDisplayed);
			}));

			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&lastMsg] {
				return (bctbx_list_size(linphone_chat_message_get_participants_by_imdn_state(
				            lastMsg, LinphoneChatMessageStateDisplayed)) == 3);
			}));
		}

		// Checks on Marie's side
		bctbx_list_t *not_delivered_list3 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateNotDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(not_delivered_list3), 0, size_t, "%zu");
		bctbx_list_free_with_data(not_delivered_list3, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		bctbx_list_t *delivered_list3 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDelivered);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_list3), 0, size_t, "%zu");
		bctbx_list_free_with_data(delivered_list3, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		bctbx_list_t *delivered_to_user_list3 =
		    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDeliveredToUser);
		BC_ASSERT_EQUAL(bctbx_list_size(delivered_to_user_list3), 0, size_t, "%zu");
		bctbx_list_free_with_data(delivered_to_user_list3, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

		for (const auto &msg : {msg0, msg1}) {
			bctbx_list_t *displayed_list3 =
			    linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateDisplayed);
			BC_ASSERT_EQUAL(bctbx_list_size(displayed_list3), 3, size_t, "%zu");
			bctbx_list_free_with_data(displayed_list3, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(msg), (int)LinphoneChatMessageStateDisplayed, int,
			                "%0d");
		}

		// Checks on clients'side
		for (const auto client : {laure.getCMgr(), pauline2.getCMgr(), michelle.getCMgr()}) {
			const LinphoneAddress *deviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
			LinphoneChatRoom *clientCr = linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(clientCr);
			LinphoneChatMessage *lastMsg = clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);

			bctbx_list_t *client_not_delivered =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateNotDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(client_not_delivered), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_not_delivered,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_delivered =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(client_delivered), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_delivered, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_delivered_to_user =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDeliveredToUser);
			BC_ASSERT_EQUAL(bctbx_list_size(client_delivered_to_user), 0, size_t, "%zu");
			bctbx_list_free_with_data(client_delivered_to_user,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *client_displayed =
			    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDisplayed);
			BC_ASSERT_EQUAL(bctbx_list_size(client_displayed), 3, size_t, "%zu");
			bctbx_list_free_with_data(client_displayed, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(lastMsg), (int)LinphoneChatMessageStateDisplayed, int,
			                "%0d");
		}

		if (!corrupt_lime_dr_session) {
			// Pauline comes back online again and she should now receive Marie's message
			ms_message("%s comes back online", paulineContactAddress.toString().c_str());
			linphone_core_set_network_reachable(pauline.getLc(), TRUE);

			const LinphoneAddress *paulineDeviceAddr =
			    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline.getLc()));
			paulineCr = linphone_core_search_chat_room(pauline.getLc(), NULL, paulineDeviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(paulineCr);
			LinphoneChatMessage *paulineLastMsg =
			    paulineCr ? linphone_chat_room_get_last_message_in_history(paulineCr) : NULL;

			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&paulineCr] {
				return linphone_chat_room_get_history_size(paulineCr) == 2;
			}));

			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).wait([&paulineLastMsg] {
				    return (linphone_chat_message_get_state(paulineLastMsg) == LinphoneChatMessageStateDisplayed);
			    }));

			// Checks on Marie's side
			bctbx_list_t *not_delivered_list4 =
			    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateNotDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(not_delivered_list4), 0, size_t, "%zu");
			bctbx_list_free_with_data(not_delivered_list4, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *delivered_list4 =
			    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(delivered_list4), 0, size_t, "%zu");
			bctbx_list_free_with_data(delivered_list4, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *delivered_to_user_list4 =
			    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDeliveredToUser);
			BC_ASSERT_EQUAL(bctbx_list_size(delivered_to_user_list4), 0, size_t, "%zu");
			bctbx_list_free_with_data(delivered_to_user_list4,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			for (const auto &msg : {msg0, msg1}) {
				bctbx_list_t *displayed_list4 =
				    linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateDisplayed);
				BC_ASSERT_EQUAL(bctbx_list_size(displayed_list4), 3, size_t, "%zu");
				bctbx_list_free_with_data(displayed_list4, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(msg), (int)LinphoneChatMessageStateDisplayed, int,
				                "%0d");
			}

			// Checks on clients'side
			for (const auto client : {laure.getCMgr(), pauline.getCMgr(), pauline2.getCMgr(), michelle.getCMgr()}) {
				const LinphoneAddress *deviceAddr =
				    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
				LinphoneChatRoom *clientCr =
				    linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(clientCr);
				LinphoneChatMessage *lastMsg =
				    clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
				BC_ASSERT_PTR_NOT_NULL(lastMsg);

				bctbx_list_t *client_not_delivered =
				    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateNotDelivered);
				BC_ASSERT_EQUAL(bctbx_list_size(client_not_delivered), 0, size_t, "%zu");
				bctbx_list_free_with_data(client_not_delivered,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				bctbx_list_t *client_delivered =
				    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDelivered);
				BC_ASSERT_EQUAL(bctbx_list_size(client_delivered), 0, size_t, "%zu");
				bctbx_list_free_with_data(client_delivered,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				bctbx_list_t *client_delivered_to_user = linphone_chat_message_get_participants_by_imdn_state(
				    lastMsg, LinphoneChatMessageStateDeliveredToUser);
				BC_ASSERT_EQUAL(bctbx_list_size(client_delivered_to_user), 0, size_t, "%zu");
				bctbx_list_free_with_data(client_delivered_to_user,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				bctbx_list_t *client_displayed =
				    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDisplayed);
				BC_ASSERT_EQUAL(bctbx_list_size(client_displayed), 3, size_t, "%zu");
				bctbx_list_free_with_data(client_displayed,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(lastMsg), (int)LinphoneChatMessageStateDisplayed,
				                int, "%0d");
			}

			BC_ASSERT_EQUAL(pauline.getCore().getChatRooms().size(), 1, size_t, "%zu");
			for (auto &chatRoom : pauline.getCore().getChatRooms()) {
				chatRoom->markAsRead();
			}

			// wait a bit longer to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).waitUntil(chrono::seconds(5), [] {
				return false;
			});

			// Checks on Marie's side
			bctbx_list_t *not_delivered_list5 =
			    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateNotDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(not_delivered_list5), 0, size_t, "%zu");
			bctbx_list_free_with_data(not_delivered_list5, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *delivered_list5 =
			    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDelivered);
			BC_ASSERT_EQUAL(bctbx_list_size(delivered_list5), 0, size_t, "%zu");
			bctbx_list_free_with_data(delivered_list5, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			bctbx_list_t *delivered_to_user_list5 =
			    linphone_chat_message_get_participants_by_imdn_state(msg1, LinphoneChatMessageStateDeliveredToUser);
			BC_ASSERT_EQUAL(bctbx_list_size(delivered_to_user_list5), 0, size_t, "%zu");
			bctbx_list_free_with_data(delivered_to_user_list5,
			                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

			for (const auto &msg : {msg0, msg1}) {
				bctbx_list_t *displayed_list5 =
				    linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateDisplayed);
				BC_ASSERT_EQUAL(bctbx_list_size(displayed_list5), 3, size_t, "%zu");
				bctbx_list_free_with_data(displayed_list5, (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(msg), (int)LinphoneChatMessageStateDisplayed, int,
				                "%0d");
			}

			// Checks on clients'side
			for (const auto client : {laure.getCMgr(), pauline.getCMgr(), pauline2.getCMgr(), michelle.getCMgr()}) {
				const LinphoneAddress *deviceAddr =
				    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
				LinphoneChatRoom *clientCr =
				    linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(clientCr);
				LinphoneChatMessage *lastMsg =
				    clientCr ? linphone_chat_room_get_last_message_in_history(clientCr) : NULL;
				BC_ASSERT_PTR_NOT_NULL(lastMsg);

				bctbx_list_t *client_not_delivered =
				    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateNotDelivered);
				BC_ASSERT_EQUAL(bctbx_list_size(client_not_delivered), 0, size_t, "%zu");
				bctbx_list_free_with_data(client_not_delivered,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				bctbx_list_t *client_delivered =
				    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDelivered);
				BC_ASSERT_EQUAL(bctbx_list_size(client_delivered), 0, size_t, "%zu");
				bctbx_list_free_with_data(client_delivered,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				bctbx_list_t *client_delivered_to_user = linphone_chat_message_get_participants_by_imdn_state(
				    lastMsg, LinphoneChatMessageStateDeliveredToUser);
				BC_ASSERT_EQUAL(bctbx_list_size(client_delivered_to_user), 0, size_t, "%zu");
				bctbx_list_free_with_data(client_delivered_to_user,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				bctbx_list_t *client_displayed =
				    linphone_chat_message_get_participants_by_imdn_state(lastMsg, LinphoneChatMessageStateDisplayed);
				BC_ASSERT_EQUAL(bctbx_list_size(client_displayed), 3, size_t, "%zu");
				bctbx_list_free_with_data(client_displayed,
				                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);

				BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(lastMsg), (int)LinphoneChatMessageStateDisplayed,
				                int, "%0d");
			}
		}

		linphone_chat_message_unref(msg0);
		linphone_chat_message_unref(msg1);

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, pauline2, laure, michelle}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void secure_group_chat_message_state_transition_from_delivered_to_displayed(void) {
	secure_group_chat_message_state_transition_to_displayed(FALSE);
}

static void secure_group_chat_message_state_transition_from_not_delivered_to_displayed(void) {
	secure_group_chat_message_state_transition_to_displayed(TRUE);
}

} // namespace LinphoneTest

static test_t local_conference_chat_imdn_tests[] = {
    TEST_ONE_TAG("Group chat with client IMDN after restart",
                 LinphoneTest::group_chat_room_with_client_idmn_after_restart,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Group chat Lime Server chat room send imdn error",
                 LinphoneTest::group_chat_room_lime_session_corrupted,
                 "LeaksMemory"), /* because of core restart */
    TEST_ONE_TAG("Secure group chat with client IMDN sent after restart",
                 LinphoneTest::secure_group_chat_room_with_client_idmn_sent_after_restart,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure group chat with client IMDN sent after restart and participant added",
                 LinphoneTest::secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure group chat with message state going from delivered to displayed",
                 LinphoneTest::secure_group_chat_message_state_transition_from_delivered_to_displayed,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure group chat with message state going from not delivered to displayed",
                 LinphoneTest::secure_group_chat_message_state_transition_from_not_delivered_to_displayed,
                 "LeaksMemory"), /* because of network up and down */
    TEST_NO_TAG("Group chat with IMDN", LinphoneTest::group_chat_room_with_imdn),
    TEST_NO_TAG("Group chat with IMDN and core restarts", LinphoneTest::group_chat_room_with_imdn_and_core_restarts),
    TEST_ONE_TAG(
        "Secure group chat with client IMDN sent after restart and participant added and core stopped before sending "
        "IMDN",
        LinphoneTest::secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added_and_core_stopped,
        "LeaksMemory"), /* because of network up and down */
    TEST_NO_TAG("Group chat Lime Server chat room clear message",
                LinphoneTest::group_chat_room_lime_server_clear_message)};

test_suite_t local_conference_test_suite_chat_imdn = {"Local conference tester (Chat IMDN)",
                                                      NULL,
                                                      NULL,
                                                      liblinphone_tester_before_each,
                                                      liblinphone_tester_after_each,
                                                      sizeof(local_conference_chat_imdn_tests) /
                                                          sizeof(local_conference_chat_imdn_tests[0]),
                                                      local_conference_chat_imdn_tests,
                                                      0};
