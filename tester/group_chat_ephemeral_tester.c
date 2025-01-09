/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "bctoolbox/crypto.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-event-log.h"
#include "linphone/api/c-participant-imdn-state.h"
#include "linphone/core.h"
#include "linphone/wrapper_utils.h"
#include "tester_utils.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

void set_ephemeral_cbs(bctbx_list_t *history) {
	for (bctbx_list_t *item = history; item; item = bctbx_list_next(item)) {
		const LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
		if (linphone_chat_message_is_ephemeral(msg)) {
			LinphoneChatMessageCbs *msgCbs = linphone_chat_message_get_callbacks(msg);
			linphone_chat_message_cbs_set_ephemeral_message_timer_started(
			    msgCbs, liblinphone_tester_chat_message_ephemeral_timer_started);
			linphone_chat_message_cbs_set_ephemeral_message_deleted(msgCbs,
			                                                        liblinphone_tester_chat_message_ephemeral_deleted);
		}
	}
}

static void ephemeral_message_test(bool_t encrypted, bool_t remained, bool_t expired, const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int size;

	set_lime_server_and_curve_list(curveId, coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                 encrypted, LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr =
	    check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;
	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));

	LinphoneChatMessage *message[10];
	if (remained) {
		linphone_chat_room_enable_ephemeral(marieCr, TRUE);
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 60);

		// Marie sends messages
		for (int i = 0; i < 10; i++) {
			message[i] = _send_message_ephemeral(marieCr, "Hello", TRUE);
		}
	}

	// Marie disable ephemeral in the group chat room
	linphone_chat_room_enable_ephemeral(marieCr, FALSE);
	LinphoneChatMessage *messageNormal = _send_message(marieCr, "See you later");

	LinphoneChatMessage *messagef[10];
	linphone_chat_room_enable_ephemeral(marieCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(marieCr, 1);

	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));

	// Marie sends messages
	for (int i = 0; i < 10; i++) {
		messagef[i] = _send_message_ephemeral(marieCr, "This is Marie", TRUE);
	}

	size = remained ? 20 : 10;

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + size + 1, 60000));

	bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history);

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + size + 1, 10000));

	// Pauline marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + size + 1, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + size, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + size, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + size, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + size, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 10, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 10, 10000));

	wait_for_list(coresList, NULL, 1, 10000);
	size = size - 9;
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), size, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), size, int, "%d");
	if (remained) {
		LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
		if (BC_ASSERT_PTR_NOT_NULL(msg)) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "See you later");
			linphone_chat_message_unref(msg);
		}
	}

	if (remained) {
		// To simulate dialog removal
		LinphoneAddress *localAddr = linphone_address_clone(linphone_chat_room_get_local_address(paulineCr));
		LinphoneAddress *peerAddr = linphone_address_clone(linphone_chat_room_get_peer_address(paulineCr));
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		coresList = bctbx_list_remove(coresList, pauline->lc);
		linphone_core_manager_reinit(pauline);
		set_lime_server_and_curve(curveId, pauline);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		if (expired) wait_for_list(coresList, NULL, 0, 60000);

		linphone_core_manager_start(pauline, TRUE);
		paulineCr = linphone_core_search_chat_room(pauline->lc, NULL, localAddr, peerAddr, NULL);
		bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
		set_ephemeral_cbs(history);
		linphone_address_unref(localAddr);
		linphone_address_unref(peerAddr);

		wait_for_list(coresList, NULL, 1, 60000);

		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted,
		                             initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted,
		                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted,
		                             initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 10, 10000));
		if (!expired)
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted,
			                             initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 10, 10000));

		bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	// Check chat room security level
	LinphoneChatRoomSecurityLevel level =
	    encrypted ? LinphoneChatRoomSecurityLevelEncrypted : LinphoneChatRoomSecurityLevelClearText;
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), level, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), level, int, "%d");

	bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

	if (remained) {
		for (int i = 0; i < 10; i++) {
			linphone_chat_message_unref(message[i]);
		}
	}
	for (int i = 0; i < 10; i++) {
		linphone_chat_message_unref(messagef[i]);
	}
	linphone_chat_message_unref(messageNormal);
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void encrypted_chat_room_ephemeral_message_test(void) {
	ephemeral_message_test(TRUE, FALSE, FALSE, C25519);
	ephemeral_message_test(TRUE, FALSE, FALSE, C448);
}

static void unencrypted_chat_room_ephemeral_message_test(void) {
	ephemeral_message_test(FALSE, FALSE, FALSE, C25519);
	ephemeral_message_test(FALSE, FALSE, FALSE, C448);
}

static void chat_room_remaining_ephemeral_message_test(void) {
	ephemeral_message_test(TRUE, TRUE, FALSE, C25519);
	ephemeral_message_test(TRUE, TRUE, FALSE, C448);
}

static void chat_room_expired_ephemeral_message_test(void) {
	ephemeral_message_test(TRUE, TRUE, TRUE, C25519);
	ephemeral_message_test(TRUE, TRUE, TRUE, C448);
}

static void send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_server_and_curve_list(curveId, coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr =
	    check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	linphone_chat_room_enable_ephemeral(paulineCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(paulineCr, 1);

	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));

	LinphoneChatMessage *message = _send_message(marieCr, "Hello");
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history);

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 10000));
	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1, 10000));

	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, "%d");

	LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
	if (BC_ASSERT_PTR_NOT_NULL(msg)) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "Hello");
		BC_ASSERT_FALSE(linphone_chat_message_is_ephemeral(msg));
		linphone_chat_message_unref(msg);
	}

	// Check chat room security level
	LinphoneChatRoomSecurityLevel level = LinphoneChatRoomSecurityLevelEncrypted;
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), level, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), level, int, "%d");

	bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
	linphone_chat_message_unref(message);

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room(void) {
	send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room_curve(C25519);
	send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room_curve(C448);
}

static void mixed_ephemeral_message_test_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_server_and_curve_list(curveId, coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr =
	    check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;
	linphone_chat_room_enable_ephemeral(marieCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(marieCr, 2000);
	// Marie sends messages
	LinphoneChatMessage *message = _send_message_ephemeral(marieCr, "This is Marie", TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));

	bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history);

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 10000));

	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));

	linphone_chat_room_set_ephemeral_lifetime(marieCr, 1);
	// Marie sends messages
	LinphoneChatMessage *message2 = _send_message_ephemeral(marieCr, "Hello", TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 10000));

	// wait messages inserted in db
	wait_for_list(coresList, NULL, 1, 10000);
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");
	bctbx_list_t *history2 = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history2);
	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 1, 10000));

	LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
	if (BC_ASSERT_PTR_NOT_NULL(msg)) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "This is Marie");
		linphone_chat_message_unref(msg);
	}

	// Check chat room security level
	LinphoneChatRoomSecurityLevel level = LinphoneChatRoomSecurityLevelEncrypted;
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), level, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), level, int, "%d");

	bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(history2, (bctbx_list_free_func)linphone_chat_message_unref);

	linphone_chat_message_unref(message);
	linphone_chat_message_unref(message2);

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void mixed_ephemeral_message_test(void) {
	mixed_ephemeral_message_test_curve(C25519);
	mixed_ephemeral_message_test_curve(C448);
}

static void chat_room_ephemeral_settings_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_server_and_curve_list(curveId, coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr =
	    check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 0, long, "%ld");

	// TODO: uncomment this assert when linphone_chat_room_ephemeral_supported_by_all_participants() is implemented.
	//  Today (2020, March), the conference server does not notify the device capabilities to the participants.
	// BC_ASSERT_TRUE(linphone_chat_room_ephemeral_supported_by_all_participants(marieCr));

	linphone_chat_room_enable_ephemeral(marieCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(marieCr, 1);

	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 1, long, "%ld");

	{
		// To simulate dialog removal
		LinphoneAddress *localAddr = linphone_address_clone(linphone_chat_room_get_local_address(marieCr));
		LinphoneAddress *peerAddr = linphone_address_clone(linphone_chat_room_get_peer_address(marieCr));
		linphone_core_set_network_reachable(marie->lc, FALSE);
		coresList = bctbx_list_remove(coresList, marie->lc);
		linphone_core_manager_reinit(marie);
		set_lime_server_and_curve(curveId, marie);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);

		linphone_core_manager_start(marie, TRUE);
		marieCr = linphone_core_search_chat_room(marie->lc, NULL, localAddr, peerAddr, NULL);
		linphone_address_unref(localAddr);
		linphone_address_unref(peerAddr);
	}

	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 1, long, "%ld");

	unsigned int nbMarieConferenceEphemeralMessageLifetimeChanged = 0;
	unsigned int nbMarieConferenceEphemeralMessageEnabled = 0;
	unsigned int nbMarieConferenceEphemeralMessageDisabled = 0;
	bctbx_list_t *marieHistory = linphone_chat_room_get_history_events(marieCr, 0);
	for (bctbx_list_t *item = marieHistory; item; item = bctbx_list_next(item)) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
		if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceEphemeralMessageLifetimeChanged)
			nbMarieConferenceEphemeralMessageLifetimeChanged++;
		else if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceEphemeralMessageEnabled)
			nbMarieConferenceEphemeralMessageEnabled++;
		else if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceEphemeralMessageDisabled)
			nbMarieConferenceEphemeralMessageDisabled++;
	}
	bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_event_log_unref);
	BC_ASSERT_EQUAL(nbMarieConferenceEphemeralMessageLifetimeChanged, 1, unsigned int, "%u");
	BC_ASSERT_EQUAL(nbMarieConferenceEphemeralMessageEnabled, 1, unsigned int, "%u");
	BC_ASSERT_EQUAL(nbMarieConferenceEphemeralMessageDisabled, 0, unsigned int, "%u");

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void chat_room_ephemeral_settings(void) {
	chat_room_ephemeral_settings_curve(C25519);
	chat_room_ephemeral_settings_curve(C448);
}

static void ephemeral_group_message_test_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);

	set_lime_server_and_curve_list(curveId, coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess,
	                             initialLaureStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr =
	    check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));

	// Marie disable ephemeral in the group chat room
	linphone_chat_room_enable_ephemeral(marieCr, FALSE);
	LinphoneChatMessage *messageNormal = _send_message(marieCr, "See you later");

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1, 60000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
	                             initialLaureStats.number_of_LinphoneMessageReceived + 1, 60000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 10000));

	linphone_chat_room_enable_ephemeral(marieCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(marieCr, 1);

	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));

	// Marie sends a message
	LinphoneChatMessage *messageEphemeral = _send_message_ephemeral(marieCr, "This is Marie", TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 2, 60000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
	                             initialLaureStats.number_of_LinphoneMessageReceived + 2, 60000));

	bctbx_list_t *pauline_history = linphone_chat_room_get_history(paulineCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_history), 2, int, "%i");
	set_ephemeral_cbs(pauline_history);

	bctbx_list_t *laure_history = linphone_chat_room_get_history(laureCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(laure_history), 2, int, "%i");
	set_ephemeral_cbs(laure_history);

	// Check that the message has been delivered to Pauline & Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 2, 10000));

	// Pauline marks the message as read, check that the state is not displayed on Marie's side since Laure hasn't read
	// it yet
	linphone_chat_room_mark_as_read(paulineCr);
	wait_for_list(coresList, NULL, 1, 10000);
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneMessageDisplayed,
	                    initialMarieStats.number_of_LinphoneMessageDisplayed + 2, int, "%i");

	linphone_chat_room_mark_as_read(laureCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + 2, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomEphemeralTimerStarted,
	                             initialLaureStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageEphemeralTimerStarted,
	                             initialLaureStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomEphemeralDeleted,
	                             initialLaureStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageEphemeralDeleted,
	                             initialLaureStats.number_of_LinphoneMessageEphemeralDeleted + 1, 10000));

	wait_for_list(coresList, NULL, 1, 10000);
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(laureCr), 1, int, "%d");

	// Check chat room security level
	LinphoneChatRoomSecurityLevel level = LinphoneChatRoomSecurityLevelEncrypted;
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), level, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), level, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), level, int, "%d");

	bctbx_list_free_with_data(pauline_history, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(laure_history, (bctbx_list_free_func)linphone_chat_message_unref);

	linphone_chat_message_unref(messageNormal);
	linphone_chat_message_unref(messageEphemeral);
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void ephemeral_group_message_test(void) {
	ephemeral_group_message_test_curve(C25519);
	ephemeral_group_message_test_curve(C448);
}

test_t ephemeral_group_chat_tests[] = {
    TEST_TWO_TAGS("Chat room remaining ephemeral messages",
                  chat_room_remaining_ephemeral_message_test,
                  "Ephemeral",
                  "LeaksMemory"), /*due to core restart*/
    TEST_TWO_TAGS("Chat room expired ephemeral messages",
                  chat_room_expired_ephemeral_message_test,
                  "Ephemeral",
                  "LeaksMemory"), /*due to core restart*/
    TEST_ONE_TAG("Mixed ephemeral messages", mixed_ephemeral_message_test, "Ephemeral"),
    TEST_ONE_TAG(
        "Send non ephemeral message", send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room, "Ephemeral")};

test_t ephemeral_group_chat_basic_tests[] = {
    TEST_ONE_TAG("Unencrypted chat room ephemeral messages", unencrypted_chat_room_ephemeral_message_test, "Ephemeral"),
    TEST_ONE_TAG("Encrypted chat room ephemeral messages", encrypted_chat_room_ephemeral_message_test, "Ephemeral"),
    TEST_ONE_TAG("Encrypted group chat room ephemeral messages", ephemeral_group_message_test, "Ephemeral"),
    TEST_TWO_TAGS("Chat room ephemeral settings",
                  chat_room_ephemeral_settings,
                  "Ephemeral",
                  "LeaksMemory") /*due to core restart*/
};

test_suite_t ephemeral_group_chat_test_suite = {"Ephemeral group chat",
                                                NULL,
                                                NULL,
                                                liblinphone_tester_before_each,
                                                liblinphone_tester_after_each,
                                                sizeof(ephemeral_group_chat_tests) /
                                                    sizeof(ephemeral_group_chat_tests[0]),
                                                ephemeral_group_chat_tests,
                                                0};

test_suite_t ephemeral_group_chat_basic_test_suite = {"Ephemeral group chat (Basic)",
                                                      NULL,
                                                      NULL,
                                                      liblinphone_tester_before_each,
                                                      liblinphone_tester_after_each,
                                                      sizeof(ephemeral_group_chat_basic_tests) /
                                                          sizeof(ephemeral_group_chat_basic_tests[0]),
                                                      ephemeral_group_chat_basic_tests,
                                                      0};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
