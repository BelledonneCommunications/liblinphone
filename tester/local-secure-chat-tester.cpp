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

#include "conference/participant.h"
#include "liblinphone_tester.h"
#include "local-conference-tester-functions.h"

namespace LinphoneTest {

static void secure_group_chat_room_with_client_restart(void) {
	group_chat_room_with_client_restart_base(true);
}

static void secure_group_chat_room_with_invite_error(void) {
	group_chat_room_with_sip_errors_base(true, false, true);
}

static void secure_group_chat_room_with_subscribe_error(void) {
	group_chat_room_with_sip_errors_base(false, true, true);
}

static void secure_group_chat_room_with_chat_room_deleted_before_server_restart(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference marie2("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), true);

		stats initialFocusStats = focus.getStats();
		stats initialMarieStats = marie.getStats();
		stats initialMarie2Stats = marie2.getStats();
		stats initialMichelleStats = michelle.getStats();
		stats initialMichelle2Stats = michelle2.getStats();

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, marie2.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(marie2);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));

		const char *initialSubject = "Colleagues (characters: $ £ çà)";

		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_enable_encryption(params, TRUE);
		linphone_chat_room_params_set_ephemeral_mode(params, LinphoneChatRoomEphemeralModeDeviceManaged);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
		linphone_chat_room_params_enable_group(params, FALSE);
		LinphoneChatRoom *marieCr =
		    linphone_core_create_chat_room_2(marie.getLc(), params, initialSubject, participantsAddresses);
		linphone_chat_room_params_unref(params);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 1;
		}));

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			linphone_chat_room_set_user_data(L_GET_C_BACK_PTR(chatRoom), marie.getCMgr());
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             initialFocusStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             initialFocusStats.number_of_participant_devices_added + 2, 5000));

		check_create_chat_room_client_side(coresList, marie.getCMgr(), marieCr, &initialMarieStats,
		                                   participantsAddresses, initialSubject, 1);

		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);
		// Check that the chat room is correctly created on Pauline's and Michelle's side and that the participants are
		// added
		LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(
		    coresList, marie2.getCMgr(), &initialMarie2Stats, confAddr, initialSubject, 1, FALSE);
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 1, FALSE);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &initialMichelle2Stats, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMarieStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMarie2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		// Send a few messages
		std::string msg_text = "message marie2 blabla";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(marie2Cr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(michelleCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelle2Cr] {
			return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		}));
		LinphoneChatMessage *michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(michelle2Cr);
		linphone_chat_room_mark_as_read(marieCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMarie2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
		msg = nullptr;

		msg_text = "message marie blabla";
		msg = ClientConference::sendTextMsg(marieCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		michelleLastMsg = michelle.getStats().last_received_chat_message;
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelle2Cr] {
			return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		}));
		michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(marie2Cr);
		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(michelle2Cr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		msg_text = "message michelle2 blabla";
		msg = ClientConference::sendTextMsg(michelle2Cr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		LinphoneChatMessage *marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marie2Cr] {
			return linphone_chat_room_get_unread_messages_count(marie2Cr) == 1;
		}));
		LinphoneChatMessage *marie2LastMsg = marie2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marie2LastMsg);
		if (marie2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie2LastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(marie2Cr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMichelle2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		// Marie deletes the chat room
		char *confAddrStr = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");
		ms_message("%s deletes chat room %s", linphone_core_get_identity(marie.getLc()), confAddrStr);
		ms_free(confAddrStr);

		linphone_core_manager_delete_chat_room(marie.getCMgr(), marieCr, coresList);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                             initialMarieStats.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateTerminated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateTerminated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
		coresList = bctbx_list_remove(coresList, focus.getLc());
		// Restart flexisip
		focus.reStart();
		coresList = bctbx_list_append(coresList, focus.getLc());

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		msg_text = "Cou cou Marieeee.....";
		msg = ClientConference::sendTextMsg(michelle2Cr, msg_text);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMarieStats.number_of_LinphoneConferenceStateCreated + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateCreated + 2,
		                             liblinphone_tester_sip_timeout));

		confAddr = linphone_chat_room_get_conference_address(michelle2Cr);
		marieCr = check_creation_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, confAddr,
		                                               initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marie2Cr] {
			return linphone_chat_room_get_unread_messages_count(marie2Cr) == 1;
		}));
		marie2LastMsg = marie2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marie2LastMsg);
		if (marie2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie2LastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(marie2Cr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMichelle2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(L_GET_C_BACK_PTR(chatRoom), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		// to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_lime_server_encrypted_message(void) {
	group_chat_room_lime_server_message(TRUE);
}

static void secure_one_to_one_group_chat_room_deletion_by_server_client(void) {
	one_to_one_group_chat_room_deletion_by_server_client_base(TRUE);
}

} // namespace LinphoneTest

static test_t local_conference_secure_chat_tests[] = {
    TEST_ONE_TAG("Secure Group chat with client restart",
                 LinphoneTest::secure_group_chat_room_with_client_restart,
                 "LeaksMemory"), /* beacause of coreMgr restart*/
    TEST_NO_TAG("Secure group chat with INVITE session error", LinphoneTest::secure_group_chat_room_with_invite_error),
    TEST_NO_TAG("Secure group chat with SUBSCRIBE session error",
                LinphoneTest::secure_group_chat_room_with_subscribe_error),
    TEST_ONE_TAG("Secure group chat with chat room deleted before server restart",
                 LinphoneTest::secure_group_chat_room_with_chat_room_deleted_before_server_restart,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure one to one group chat deletion initiated by server and client",
                 LinphoneTest::secure_one_to_one_group_chat_room_deletion_by_server_client,
                 "LeaksMemory"), /* because of network up and down */
    TEST_NO_TAG("Group chat Lime Server chat room encrypted message",
                LinphoneTest::group_chat_room_lime_server_encrypted_message)};

test_suite_t local_conference_test_suite_secure_chat = {"Local conference tester (Secure Chat)",
                                                        NULL,
                                                        NULL,
                                                        liblinphone_tester_before_each,
                                                        liblinphone_tester_after_each,
                                                        sizeof(local_conference_secure_chat_tests) /
                                                            sizeof(local_conference_secure_chat_tests[0]),
                                                        local_conference_secure_chat_tests,
                                                        0};
