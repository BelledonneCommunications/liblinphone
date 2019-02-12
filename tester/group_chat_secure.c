/*
	liblinphone_tester - liblinphone test suite
	Copyright (C) 2013  Belledonne Communications SARL

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

bool_t simple_zrtp_call_with_sas_validation(LinphoneCoreManager *caller, LinphoneCoreManager *callee, bool_t callerValidation, bool_t calleeValidation) {
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok=call(caller, callee)));
	if (!call_ok) return FALSE;

	// If caller set ZRTP or (callee set ZRTP and caller has no encryption requested), ZRTP shall take place, wait for the SAS
	if ((linphone_core_get_media_encryption(caller->lc) == LinphoneMediaEncryptionZRTP)
		|| ((linphone_core_get_media_encryption(callee->lc) == LinphoneMediaEncryptionZRTP) && (linphone_core_get_media_encryption(caller->lc) == LinphoneMediaEncryptionNone))) {

		// Simulate SAS validation or invalidation
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(caller->lc), callerValidation);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(callee->lc), calleeValidation);
		BC_ASSERT_EQUAL(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(caller->lc)), callerValidation, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(callee->lc)), calleeValidation, int, "%d");
	}
	end_call(caller, callee);
	return TRUE;
}

static const int x3dhServerDelay = 3000; // TODO replace me with X3DH server callbacks

static void group_chat_lime_x3dh_create_lime_user (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
}

static void group_chat_lime_x3dh_change_server_url(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_no_server_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");

	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieEncryptedCr = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	//Check encryption status for both participants
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(marie->lc)); //should be false if no x3dh server defined
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	linphone_core_set_lime_x3dh_server_url(marie->lc, "http://x3dh.example.org:8082/flexisip-account-manager/x3dh-25519.php");

	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc)); //should be true now

	//Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	//Now create an encrypted chatroom to check that marie can
	//Marie creates an encrypted chatroom
	const char *initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieEncryptedCr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	if (marieEncryptedCr) linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	if (paulineEncryptedCr) linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_encrypted_chatrooms (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieEncryptedCr = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new regular chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *mariePlainCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	LinphoneAddress *plainConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(mariePlainCr));
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulinePlainCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, plainConfAddr, initialSubject, 1, 0);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends a plain message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(mariePlainCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Reset stats for new chatroom creation
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);

	// Marie creates an encrypted chatroom
	initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieEncryptedCr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends an encrypted message
	marieMessage = "We can say whatever we want in this chatrooom!";
	_send_message(marieEncryptedCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Marie deletes the regular chat room
	linphone_core_manager_delete_chat_room(marie, mariePlainCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the regular chat room again
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialSubject = "Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	mariePlainCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	// Marie sends a new plain message
	marieMessage = "Hey again";
	_send_message(mariePlainCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), marieMessage);

	// Check that the recreated regular chat room address is the same as before and the capabilities are correct
	const LinphoneAddress *newPlainConfAddr = linphone_chat_room_get_conference_address(mariePlainCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(plainConfAddr, newPlainConfAddr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie deletes the encrypted chat room
	linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the encrypted chat room again
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);

	// Marie sends a new encrypted message
	marieMessage = "Hey again from the encrypted chatroom";
	_send_message(marieEncryptedCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), marieMessage);

	// Check that the recreated encrypted chat room address is the same as before and the capabilities are correct
	const LinphoneAddress *newEncryptedConfAddr = linphone_chat_room_get_conference_address(marieEncryptedCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(encryptedConfAddr, newEncryptedConfAddr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	linphone_address_unref(plainConfAddr);
	linphone_address_unref(encryptedConfAddr);

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, mariePlainCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulinePlainCr, coresList);
	if (marieEncryptedCr) linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	if (paulineEncryptedCr) linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_basic_chat_rooms (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Send message in basic chat room
	LinphoneChatRoom *marieBasicCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *basicMessage1 = linphone_chat_room_create_message(marieBasicCr, "Hello from our basic chat room");
	LinphoneChatMessageCbs *cbs1 = linphone_chat_message_get_callbacks(basicMessage1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs1, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(basicMessage1);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneMessageDelivered,initialMarieStats.number_of_LinphoneMessageReceived + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneMessageReceived,initialPaulineStats.number_of_LinphoneMessageReceived + 1));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), linphone_chat_message_get_text(basicMessage1));
	}
	LinphoneChatRoom *paulineBasicCr = linphone_core_get_chat_room(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieBasicCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineBasicCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_PTR_NOT_NULL(paulineBasicCr);
	linphone_chat_message_unref(basicMessage1);

	// Marie creates an encrypted chatroom
	const char *initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneChatRoom *marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieEncryptedCr));
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends an encrypted message
	const char *marieEncryptedMessage1 = "Hello from our secured chat room";
	_send_message(marieEncryptedCr, marieEncryptedMessage1);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieEncryptedMessage1);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Marie deletes the basic chat room
	linphone_core_manager_delete_chat_room(marie, marieBasicCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the basic chat room again
	LinphoneChatRoom *marieNewBasicCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *basicMessage2 = linphone_chat_room_create_message(marieNewBasicCr, "Hello again from our basic chat room");
	LinphoneChatMessageCbs *cbs2 = linphone_chat_message_get_callbacks(basicMessage2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs2, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(basicMessage2);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneMessageDelivered,initialMarieStats.number_of_LinphoneMessageReceived + 3));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneMessageReceived,initialPaulineStats.number_of_LinphoneMessageReceived + 3));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), linphone_chat_message_get_text(basicMessage2));
	}
	linphone_chat_message_unref(basicMessage2);
	linphone_address_unref(encryptedConfAddr);
	linphone_address_unref(marieAddr);

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(pauline, paulineBasicCr, coresList);
	linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void lime_x3dh_message_test (bool_t with_composing, bool_t with_response) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (with_composing) {
		// Marie starts composing a message
		linphone_chat_room_compose(marieCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));
	}

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	if (with_response) {
		if (with_composing) {
			// Pauline starts composing a response
			linphone_chat_room_compose(paulineCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));
		}

		// Pauline sends the response
		const char *paulineMessage = "I'm fine thank you ! And you ?";
		_send_message(paulineCr, paulineMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
		LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
			goto end;

		// Check that the response was correctly decrypted
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineMessage);
		LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
		linphone_address_unref(paulineAddr);
	}

	// Check chat room security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message (void) {
	lime_x3dh_message_test(FALSE, FALSE);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_composing (void) {
	lime_x3dh_message_test(TRUE, FALSE);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_response (void) {
	lime_x3dh_message_test(FALSE, TRUE);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_response_and_composing (void) {
	lime_x3dh_message_test(TRUE, TRUE);
}

static void group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	linphone_core_enable_lime_x3dh(laure->lc, FALSE);
	linphone_core_add_linphone_spec(laure->lc, "lime"); //Forcing lime_x3dh spec even if encryption engine is disabled

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(laure->lc));


	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that Pauline received and decrypted the message
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check that Laure did not receive the message because she did not post keys on the X3DH server
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 1000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

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


static void group_chat_lime_x3dh_send_encrypted_file_with_or_without_text (bool_t with_text) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *receivePaulineFilepath = bc_tester_file("receive_file_pauline.dump");
	char *receiveChloeFilepath = bc_tester_file("receive_file_chloe.dump");
	const char *text = "Hello Group !";

	// Globally configure an http file transfer server
	linphone_core_set_file_transfer_server(marie->lc, "https://www.linphone.org:444/lft.php");
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;

	// Remove any previously downloaded file
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Send encrypted file
	if (with_text) {
		_send_file_plus_text(marieCr, sendFilepath, text);
	} else {
		_send_file(marieCr, sendFilepath);
	}

	// Check that chat rooms have received the file
	if (with_text) {
		_receive_file_plus_text(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, text);
		_receive_file_plus_text(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, text);
	} else {
		_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath);
		_receive_file(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath);
	}

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(chloe->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);
	bc_free(sendFilepath);
	bc_free(receivePaulineFilepath);
	bc_free(receiveChloeFilepath);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_send_encrypted_file (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(FALSE);
}

static void group_chat_lime_x3dh_send_encrypted_file_plus_text (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(TRUE);
}

static void group_chat_lime_x3dh_verify_sas_before_message (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	_send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// ZRTP verification call Marie rejects the SAS
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Delete chatrooms
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_reject_sas_before_message (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP verification call Marie rejects the SAS
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;


	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	_send_message(marieCr, marieMessage);

	if (lp_config_get_int(linphone_core_get_config(marie->lc), "lime", "allow_message_in_unsafe_chatroom", 0) == 1) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
		LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
			goto end;

		// Check that the message was correctly decrypted by Pauline
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	}

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	_send_message(paulineCr, paulineMessage);

	// Marie does not receive Pauline's message because Pauline is unsafe for Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));

	// ZRTP verification call between Marie and Pauline
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Delete chatrooms
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_message_before_verify_sas (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	_send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_message_before_reject_sas (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneChatRoom *paulineCr = NULL;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;

	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");

	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hi Pauline, how are you ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Check LIME and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP call both validate the SAS
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// ZRTP call Marie rejects the SAS
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Check security event
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ManInTheMiddleDetected, initialMarieStats.number_of_ManInTheMiddleDetected + 1, 3000));

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static void group_chat_lime_x3dh_chatroom_security_level_upgrade (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 3, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 3, 0);
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	// Check that the message was correctly received and decrypted by Chloe
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived, initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *chloeLastMsg = chloe->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(chloeLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(chloeLastMsg), marieMessage);
	LinphoneAddress *marieAddr3 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr3, linphone_chat_message_get_from_address(chloeLastMsg)));
	linphone_address_unref(marieAddr3);

	// Check chat room security level is encrypted
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(chloe->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t pauline_call_ok = FALSE;
	BC_ASSERT_TRUE((pauline_call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!pauline_call_ok) goto end;

	// Check chat room security level has not changed
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// ZRTP verification call between Marie and Laure
	bool_t laure_call_ok = FALSE;
	BC_ASSERT_TRUE((laure_call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!laure_call_ok) goto end;

	// Check chat room security level has not changed
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// ZRTP verification call between Marie and Chloe
	bool_t chloe_call_ok = FALSE;
	BC_ASSERT_TRUE((chloe_call_ok = simple_zrtp_call_with_sas_validation(marie, chloe, TRUE, TRUE)));
	if (!chloe_call_ok) goto end;

	// Check that Marie is now in a safe chatroom
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);
	linphone_core_delete_local_encryption_db(chloe->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	LinphoneChatRoom *chloeCr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Marie and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Pauline and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the maximum security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Marie adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	// Check the chat room security level got downgraded
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Check that participants have received a SecurityLevelDowngraded event
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_SecurityLevelDowngraded, initialLaureStats.number_of_SecurityLevelDowngraded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_SecurityLevelDowngraded, initialLaureStats.number_of_SecurityLevelDowngraded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_SecurityLevelDowngraded, initialLaureStats.number_of_SecurityLevelDowngraded + 1, 3000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);
	linphone_core_delete_local_encryption_db(chloe->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Marie and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Pauline and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the maximum security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// New call with ZRTP verification but pauline refuses the SAS
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline, marie, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check the chat room security level got downgraded for Pauline
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	if (lp_config_get_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1) == 1) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	} else {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	}

	// Check that pauline's chatroom received a security event
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_ManInTheMiddleDetected, initialPaulineStats.number_of_ManInTheMiddleDetected + 1, 3000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

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

static void group_chat_lime_x3dh_chatroom_security_alert (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *pauline2 = NULL;
	LinphoneChatRoom *pauline2Cr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline1 and Laure sides and that the participants are added
	LinphoneChatRoom *pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline1LastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(pauline1LastMsg)));
	linphone_address_unref(marieAddr1);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);
	laureLastMsg = NULL;

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline1->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline1
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline1, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Marie and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Pauline1 and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline1, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the maximum security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline1Cr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Marie sends a message to the chatroom
	marieMessage = "What are you doing tonight ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 2, 10000));
	pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline1LastMsg), marieMessage);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);

	// Create second device for Pauline
	pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	stats initialPauline2Stats = pauline2->stat;
	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, pauline2);
	bctbx_list_t *newCoresList = init_core_for_conference(newCoresManagerList);
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Wait for Pauline2 lime user to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2->lc));
	linphone_core_set_media_encryption(pauline2->lc, LinphoneMediaEncryptionZRTP);

	// Pauline2 is automatically added to the chatroom

	// Check that the chat room is correctly created on Pauline2's side and that she was added everywhere
	pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participant_devices_added, initialPauline1Stats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 1, 3000));

	// Check that the participants have received a security alert because Pauline2 is forbidden
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ParticipantMaxDeviceCountExceeded, initialMarieStats.number_of_ParticipantMaxDeviceCountExceeded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_ParticipantMaxDeviceCountExceeded, initialPauline1Stats.number_of_ParticipantMaxDeviceCountExceeded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_ParticipantMaxDeviceCountExceeded, initialLaureStats.number_of_ParticipantMaxDeviceCountExceeded + 1, 3000));

	// Check the security level was downgraded for Marie and Laure
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");

	// Laure sends a messages to trigger a LIME X3DH security alerts because maxNumberOfDevicePerParticipant has been exceeded
	if (lp_config_get_int(linphone_core_get_config(laure->lc), "lime", "allow_message_in_unsafe_chatroom", 0) == 0) {
		const char *laureMessage = "I'm going to the cinema";
		_send_message(laureCr, laureMessage);
		wait_for_list(coresList, &dummy, 1, 500);
		BC_ASSERT_FALSE((marie->stat.number_of_LinphoneMessageReceived == initialPauline1Stats.number_of_LinphoneMessageReceived + 3));
		BC_ASSERT_FALSE((pauline1->stat.number_of_LinphoneMessageReceived == initialPauline1Stats.number_of_LinphoneMessageReceived + 3));
		BC_ASSERT_FALSE((pauline2->stat.number_of_LinphoneMessageReceived == initialPauline2Stats.number_of_LinphoneMessageReceived + 1));
	}

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline1->lc);
	if (pauline2) linphone_core_delete_local_encryption_db(pauline2->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline1, pauline1Cr, coresList);
	if (pauline2Cr) linphone_core_manager_delete_chat_room(pauline2, pauline2Cr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline1);
	if (pauline2) linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_call_security_alert (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, FALSE)));
	if (!call_ok) goto end;

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	if (lp_config_get_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1) == 1) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	} else {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	}

	// Check chatroom security event
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_ManInTheMiddleDetected, initialPaulineStats.number_of_ManInTheMiddleDetected + 1, 3000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_multiple_successive_encrypted_messages (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for all participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, pauline, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends the message
	const char *marieMessage1 = "Hey !";
	_send_message(marieCr, marieMessage1);

	// Check that message 1 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage1);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);
	paulineLastMsg = NULL;

	// Check that message 1 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage1);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);
	laureLastMsg = NULL;

	// Marie sends another message
	const char *marieMessage2 = "What's up ?";
	_send_message(marieCr, marieMessage2);

	// Check that message 2 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage2);
	paulineLastMsg = NULL;

	// Check that message 2 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage2);
	laureLastMsg = NULL;

	// Marie sends yet another message
	const char *marieMessage3 = "I need to talk to you.";
	_send_message(marieCr, marieMessage3);

	// Check that message 3 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 3, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage3);

	// Check that message 3 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 3, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage3);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
// 	linphone_core_manager_delete_chat_room(laure, laureCr, coresList); // TODO crash in c-wrapper because Cpp Object is null

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Pauline disables LIME X3DH
	linphone_core_enable_lime_x3dh(pauline->lc, FALSE);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Check that the IsComposing is undecipherable and that an undecipherable message error IMDN is returned to Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 1000));

	// Marie sends the message
	const char *marieMessage = "What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message is discarded and that an undecipherable message error IMDN is returned to Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));

	// Check the chatrooms security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelClearText, int, "%d");

	// Clean local LIME X3DH databases
	linphone_core_enable_lime_x3dh(pauline->lc, TRUE);
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie disables LIME X3DH
	linphone_core_enable_lime_x3dh(marie->lc, FALSE);

	// Check encryption status
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Check that the IsComposing is correctly discarded
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));

	// Marie sends a message
	const char *marieMessage = "What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message is correctly discarded
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));

	// Clean local LIME X3DH databases
	linphone_core_enable_lime_x3dh(marie->lc, TRUE);
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants (void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on x3dh server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for all participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Change the value of max_nb_device_per_participant to allow multidevice
	linphone_config_set_int(linphone_core_get_config(marie1->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(marie2->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(pauline1->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(pauline2->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(laure->lc), "lime", "max_nb_device_per_participant", 2);

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr1 = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, TRUE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr1);

	// Check that the chat room is correctly created on Marie's second device
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, 1);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr1 = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, 0);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr1), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Marie sends a message
	const char *marie1Message = "Hey ! What's up guys ?";
	_send_message(marieCr1, marie1Message);

	// Check that the message was received by everybody
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived, initialMarie2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, initialPauline2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));

	LinphoneChatMessage *marie2LastMsg = marie2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marie2LastMsg))
		goto end;
	LinphoneChatMessage *pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	LinphoneChatMessage *pauline2LastMsg = pauline2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline2LastMsg))
		goto end;
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;

	// Check that the messages were correctly decrypted by everybody
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie2LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline1LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline2LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marie1Message);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr1), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie1->lc);
	linphone_core_delete_local_encryption_db(marie2->lc);
	linphone_core_delete_local_encryption_db(pauline1->lc);
	linphone_core_delete_local_encryption_db(pauline2->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marieCr1, coresList);
	linphone_core_manager_delete_chat_room(marie2, marieCr2, coresList);
	linphone_core_manager_delete_chat_room(pauline1, paulineCr1, coresList);
	linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_message_while_network_unreachable (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Simulate pauline has disconnected
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Reconnect pauline
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	// Check if the message is received
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_update_keys (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime user creation
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Set the last time a lime update was performed to an recent date
	LinphoneConfig *config = linphone_core_get_config(marie->lc);
	time_t recentUpdateTime = ms_time(NULL)-22000; // 6 hours = 21600 ms
	linphone_config_set_int(config, "misc", "last_update_time", (int)recentUpdateTime);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Check that we have not performed an update
	int newUpdateTime = linphone_config_get_int(config, "misc", "last_update_time", -1);
	BC_ASSERT_EQUAL(newUpdateTime, (int)recentUpdateTime, int, "%d");

	// Set the last time a lime update was performed to an old date
	time_t oldUpdateTime = ms_time(NULL)-88000; // 24 hours = 86400 ms
	linphone_config_set_int(config, "misc", "last_update_time", (int)oldUpdateTime);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Wait for update callback
	wait_for_list(coresList, &dummy, 1, 2000);

	// Check that we correctly performed an update
	newUpdateTime = linphone_config_get_int(config, "misc", "last_update_time", -1);
	BC_ASSERT_GREATER(newUpdateTime, (int)oldUpdateTime, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void imdn_for_group_chat_room (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	time_t initialTime = ms_time(NULL);
	
	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));
	
	
	// Wait for lime user creation
	wait_for_list(coresList, NULL, 1, 2*x3dhServerDelay);

	
	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);
	
	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	
	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	
	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);
	
	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed));
	bctbx_list_t *participantsThatReceivedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 2, int, "%d");
		for (bctbx_list_t *item = participantsThatReceivedChloeMessage; item; item = bctbx_list_next(item)) {
			LinphoneParticipantImdnState *state = (LinphoneParticipantImdnState *)bctbx_list_get_data(item);
			BC_ASSERT_GREATER((int)linphone_participant_imdn_state_get_state_change_time(state), (int)initialTime, int, "%d");
			BC_ASSERT_EQUAL(linphone_participant_imdn_state_get_state(state), LinphoneChatMessageStateDeliveredToUser, int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_participant_imdn_state_get_participant(state));
		}
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));
	
	// Marie marks the message as read, check that the state is not yet displayed on Chloe's side
	linphone_chat_room_mark_as_read(marieCr);
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
	bctbx_list_t *participantsThatDisplayedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	participantsThatReceivedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));
	
	// Pauline also marks the message as read, check that the state is now displayed on Chloe's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
	participantsThatDisplayedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 2, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));
	
	linphone_chat_message_unref(chloeMessage);
	
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

test_t secure_group_chat_tests[] = {
	TEST_ONE_TAG("LIME X3DH create lime user", group_chat_lime_x3dh_create_lime_user, "LimeX3DH"),
	TEST_TWO_TAGS("LIME X3DH change server url", group_chat_lime_x3dh_change_server_url, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH encrypted chatrooms", group_chat_lime_x3dh_encrypted_chatrooms, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH basic chatrooms", group_chat_lime_x3dh_basic_chat_rooms, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message", group_chat_lime_x3dh_send_encrypted_message, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message with composing", group_chat_lime_x3dh_send_encrypted_message_with_composing, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message with response", group_chat_lime_x3dh_send_encrypted_message_with_response, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message with response and composing", group_chat_lime_x3dh_send_encrypted_message_with_response_and_composing, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message to devices with and without keys on server", group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys, "LimeX3DH", "LeaksMemory"),
	TEST_ONE_TAG("LIME X3DH send encrypted file", group_chat_lime_x3dh_send_encrypted_file, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send encrypted file + text", group_chat_lime_x3dh_send_encrypted_file_plus_text, "LimeX3DH"),
	TEST_TWO_TAGS("LIME X3DH verify SAS before message", group_chat_lime_x3dh_verify_sas_before_message, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH reject SAS before message", group_chat_lime_x3dh_reject_sas_before_message, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message before verify SAS", group_chat_lime_x3dh_message_before_verify_sas, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message before reject SAS", group_chat_lime_x3dh_message_before_reject_sas, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security level upgrade", group_chat_lime_x3dh_chatroom_security_level_upgrade, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security level downgrade adding participant", group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security level downgrade resetting zrtp", group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security alert", group_chat_lime_x3dh_chatroom_security_alert, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH call security alert", group_chat_lime_x3dh_call_security_alert, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH multiple successive messages", group_chat_lime_x3dh_send_multiple_successive_encrypted_messages, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH encrypted message to disabled LIME X3DH", group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH plain message to enabled LIME X3DH", group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message to multidevice participants", group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH messages while network unreachable", group_chat_lime_x3dh_message_while_network_unreachable, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH update keys", group_chat_lime_x3dh_update_keys, "LimeX3DH", "LeaksMemory"),
	TEST_ONE_TAG("Imdn", imdn_for_group_chat_room, "LimeX3DH")
};

test_suite_t secure_group_chat_test_suite = {
	"Secure group chat",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(secure_group_chat_tests) / sizeof(secure_group_chat_tests[0]), secure_group_chat_tests
};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
