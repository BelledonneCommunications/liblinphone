/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#include "linphone/core.h"
#include "tester_utils.h"
#include "linphone/wrapper_utils.h"
#include "liblinphone_tester.h"
#include "bctoolbox/crypto.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

static const int x3dhServer_creationTimeout = 5000;

static void set_ephemeral_cbs (bctbx_list_t *history) {
	for (bctbx_list_t *item = history; item; item = bctbx_list_next(item)) {
		const LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
		if (linphone_chat_message_is_ephemeral(msg)) {
			LinphoneChatMessageCbs *msgCbs = linphone_chat_message_get_callbacks(msg);
			linphone_chat_message_cbs_set_ephemeral_message_timer_started(msgCbs, liblinphone_tester_chat_message_ephemeral_timer_started);
			linphone_chat_message_cbs_set_ephemeral_message_deleted(msgCbs, liblinphone_tester_chat_message_ephemeral_deleted);
		}
	}
}

static void ephemeral_message_test (bool_t encrypted, bool_t remained, bool_t expired, const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int size;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, encrypted);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));

	LinphoneChatMessage *message[10];
	if (remained) {
		linphone_chat_room_enable_ephemeral(marieCr, TRUE);
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 60);

		// Marie sends messages
		for (int i=0; i<10; i++) {
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
	for (int i=0; i<10; i++) {
		messagef[i] = _send_message_ephemeral(marieCr, "This is Marie", TRUE);
	}

	size = remained ? 20 : 10;

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + size+1,60000));

	bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history);

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, initialMarieStats.number_of_LinphoneMessageDeliveredToUser + size+1, 6000));

	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, initialMarieStats.number_of_LinphoneMessageDisplayed + size+1, 6000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted, initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + size, 6000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted, initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + size, 6000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted, initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + size, 6000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted, initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + size, 6000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted, initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 6000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted, initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 6000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted, initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 10, 6000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted, initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 10, 6000));

	wait_for_list(coresList, NULL, 1, 10000);
	size = size-9;
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
		LinphoneAddress *paulineAddr = linphone_address_clone(linphone_chat_room_get_peer_address(paulineCr));
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		coresList = bctbx_list_remove(coresList, pauline->lc);
		linphone_core_manager_reinit(pauline);
		set_lime_curve(curveId,pauline);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		if (expired)
			wait_for_list(coresList, NULL, 0, 60000);

		linphone_core_manager_start(pauline, TRUE);
		paulineCr = linphone_core_get_chat_room(pauline->lc, paulineAddr);
		bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
		set_ephemeral_cbs(history);
		linphone_address_unref(paulineAddr);

		wait_for_list(coresList, NULL, 1, 60000);

		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted, initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 6000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted, initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 10, 6000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted, initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 10, 6000));
		if (!expired)
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted, initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 10, 6000));

		bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	// Check chat room security level
	LinphoneChatRoomSecurityLevel level = encrypted ? LinphoneChatRoomSecurityLevelEncrypted : LinphoneChatRoomSecurityLevelClearText;
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), level, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), level, int, "%d");

	bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

	if (remained) {
		for (int i=0; i<10; i++) {
			linphone_chat_message_unref(message[i]);
		}
	}
	for (int i=0; i<10; i++) {
		linphone_chat_message_unref(messagef[i]);
	}
	linphone_chat_message_unref(messageNormal);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void encrypted_chat_room_ephemeral_message_test (void) {
	ephemeral_message_test(TRUE, FALSE, FALSE, 25519);
	ephemeral_message_test(TRUE, FALSE, FALSE, 448);
}

static void unencrypted_chat_room_ephemeral_message_test (void) {
	ephemeral_message_test(FALSE, FALSE, FALSE, 25519);
	ephemeral_message_test(FALSE, FALSE, FALSE, 448);
}

static void chat_room_remaining_ephemeral_message_test (void) {
	ephemeral_message_test(TRUE, TRUE, FALSE, 25519);
	ephemeral_message_test(TRUE, TRUE, FALSE, 448);
}

static void chat_room_expired_ephemeral_message_test (void) {
	ephemeral_message_test(TRUE, TRUE, TRUE, 25519);
	ephemeral_message_test(TRUE, TRUE, TRUE, 448);
}

static void send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	linphone_chat_room_enable_ephemeral(paulineCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(paulineCr, 1);

	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));

	LinphoneChatMessage *message = _send_message(marieCr, "Hello");
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived +1,3000));
	bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history);

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, initialMarieStats.number_of_LinphoneMessageDeliveredToUser +1, 3000));
	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, initialMarieStats.number_of_LinphoneMessageDisplayed +1, 3000));

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

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room(void) {
	send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room_curve(25519);
	send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room_curve(448);
}

static void mixed_ephemeral_message_test_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	linphone_chat_room_enable_ephemeral(marieCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(marieCr, 2000);
	// Marie sends messages
	LinphoneChatMessage *message = _send_message_ephemeral(marieCr, "This is Marie", TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1,3000));

	bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history);

	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));

	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, initialMarieStats.number_of_LinphoneMessageDisplayed + 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted, initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted, initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted, initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted, initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 3000));

	linphone_chat_room_set_ephemeral_lifetime(marieCr, 1);
	// Marie sends messages
	LinphoneChatMessage *message2 = _send_message_ephemeral(marieCr, "Hello", TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1,3000));


	// Check that the message has been delivered to Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));

	// wait messages inserted in db
	wait_for_list(coresList, NULL, 1, 10000);
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");
	bctbx_list_t *history2 = linphone_chat_room_get_history(paulineCr, 0);
	set_ephemeral_cbs(history2);
	// Pauline  marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, initialMarieStats.number_of_LinphoneMessageDisplayed + 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralTimerStarted, initialMarieStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralTimerStarted, initialPaulineStats.number_of_LinphoneChatRoomEphemeralTimerStarted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralTimerStarted, initialMarieStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralTimerStarted, initialPaulineStats.number_of_LinphoneMessageEphemeralTimerStarted + 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomEphemeralDeleted, initialMarieStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomEphemeralDeleted, initialPaulineStats.number_of_LinphoneChatRoomEphemeralDeleted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageEphemeralDeleted, initialMarieStats.number_of_LinphoneMessageEphemeralDeleted + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageEphemeralDeleted, initialPaulineStats.number_of_LinphoneMessageEphemeralDeleted + 1, 3000));

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

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void mixed_ephemeral_message_test(void) {
	mixed_ephemeral_message_test_curve(25519);
	mixed_ephemeral_message_test_curve(448);
}

static void chat_room_ephemeral_settings_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 86400, long, "%ld");
	
	//TODO: uncomment this assert when linphone_chat_room_ephemeral_supported_by_all_participants() is implemented.
	// Today (2020, March), the conference server does not notify the device capabilities to the participants.
	//BC_ASSERT_TRUE(linphone_chat_room_ephemeral_supported_by_all_participants(marieCr));

	linphone_chat_room_enable_ephemeral(marieCr, TRUE);
	linphone_chat_room_set_ephemeral_lifetime(marieCr, 1);

	BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 1, long, "%ld");

	{
		// To simulate dialog removal
		LinphoneAddress *marieAddr = linphone_address_clone(linphone_chat_room_get_peer_address(marieCr));
		linphone_core_set_network_reachable(marie->lc, FALSE);
		coresList = bctbx_list_remove(coresList, marie->lc);
		linphone_core_manager_reinit(marie);
		set_lime_curve(curveId,marie);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);

		linphone_core_manager_start(marie, TRUE);
		marieCr = linphone_core_get_chat_room(marie->lc, marieAddr);
		linphone_address_unref(marieAddr);
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

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void chat_room_ephemeral_settings(void) {
	chat_room_ephemeral_settings_curve(25519);
	chat_room_ephemeral_settings_curve(448);
}

test_t ephemeral_group_chat_tests[] = {
	TEST_ONE_TAG("Unencrypted chat room ephemeral messages", unencrypted_chat_room_ephemeral_message_test, "Ephemeral"),
	TEST_ONE_TAG("Encrypted chat room ephemeral messages", encrypted_chat_room_ephemeral_message_test, "Ephemeral"),
	TEST_TWO_TAGS("Chat room remaining ephemeral messages", chat_room_remaining_ephemeral_message_test, "Ephemeral", "LeaksMemory"), /*due to core restart*/
	TEST_TWO_TAGS("Chat room expired ephemeral messages", chat_room_expired_ephemeral_message_test, "Ephemeral", "LeaksMemory"), /*due to core restart*/
	TEST_ONE_TAG("Mixed ephemeral messages", mixed_ephemeral_message_test, "Ephemeral"),
	TEST_TWO_TAGS("Chat room ephemeral settings", chat_room_ephemeral_settings, "Ephemeral", "LeaksMemory") /*due to core restart*/,
	TEST_ONE_TAG("Send non ephemeral message", send_msg_from_no_ephemeral_chat_room_to_ephmeral_chat_room, "Ephemeral")
};

test_suite_t ephemeral_group_chat_test_suite = {
	"Ephemeral group chat",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(ephemeral_group_chat_tests) / sizeof(ephemeral_group_chat_tests[0]), ephemeral_group_chat_tests
};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
