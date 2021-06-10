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

static const int x3dhServer_creationTimeout = 15000;

static bool_t simple_zrtp_call_with_sas_validation(LinphoneCoreManager *caller, LinphoneCoreManager *callee, bool_t callerValidation, bool_t calleeValidation) {
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



static void group_chat_lime_x3dh_create_lime_user_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
}
static void group_chat_lime_x3dh_create_lime_user(void) {
	group_chat_lime_x3dh_create_lime_user_curve(25519);
	group_chat_lime_x3dh_create_lime_user_curve(448);
}


#if 0
THIS TEST IS ACTUALLY BROKEN:
- an update on the url will not trigger the publishing of a lime user on server, we need a onRegisterStateChanged notification for that
- a user without keys on the X3DH server which think lime is enabled (following a failure in keys publication can create an encrypted chat room)

static void group_chat_lime_x3dh_change_server_url_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_no_server_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");

	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_curve(curveId,pauline); // do not set the curve (and server) for Marie
	LinphoneChatRoom *marieEncryptedCr = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;

	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server (pauline only as mary does not have any server to publish user)
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	//Check encryption status for both participants
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(marie->lc)); //should be false if no x3dh server defined
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	set_lime_curve(curveId,marie); // This will set the curve and server url

	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc)); //should be true now

// BROKEN PART -> this shall pass but it fails, changing the server url will not trigger the publishing of lime user on the server
	//Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

// BROKEN PART -> this shall fail but it pass, nothing prevent the marie to create an encrypted chatroom without keys on the server
	//Now create an encrypted chatroom to check that marie can create an encrypted chatroom
	const char *initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieEncryptedCr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Clean db from chat room
	if (marieEncryptedCr) linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	if (paulineEncryptedCr) linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	linphone_address_unref(encryptedConfAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_change_server_url(void) {
	group_chat_lime_x3dh_change_server_url_curve(25519);
	group_chat_lime_x3dh_change_server_url_curve(448);
}
#endif /* 0 */

static void group_chat_lime_x3dh_encrypted_chatrooms_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieEncryptedCr = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;

	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	set_lime_curve_list(curveId,coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(paulinePlainCr))
		goto end;
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends a plain message
	const char *marieMessage = "Hey ! What's up ?";
	LinphoneChatMessage *msg = _send_message(mariePlainCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
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
	const LinphoneAddress *marieConferenceAddress = linphone_chat_room_get_conference_address(marieEncryptedCr);
	if(!BC_ASSERT_PTR_NOT_NULL(marieConferenceAddress))
		goto end;
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(marieConferenceAddress);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineEncryptedCr))
		goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends an encrypted message
	marieMessage = "We can say whatever we want in this chatrooom!";
	msg = _send_message(marieEncryptedCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
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
	msg = _send_message(mariePlainCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline->stat.last_received_chat_message), marieMessage);
	linphone_chat_message_unref(msg);

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
	msg = _send_message(marieEncryptedCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline->stat.last_received_chat_message), marieMessage);
	linphone_chat_message_unref(msg);

	// Check that the recreated encrypted chat room address is the same as before and the capabilities are correct
	const LinphoneAddress *newEncryptedConfAddr = linphone_chat_room_get_conference_address(marieEncryptedCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(encryptedConfAddr, newEncryptedConfAddr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	linphone_address_unref(plainConfAddr);
	linphone_address_unref(encryptedConfAddr);

end:

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
static void group_chat_lime_x3dh_encrypted_chatrooms(void) {
	group_chat_lime_x3dh_encrypted_chatrooms_curve(25519);
	group_chat_lime_x3dh_encrypted_chatrooms_curve(448);
}


/**
 * - create users
 * - stop and start their linphone core
 * - perform an update
 */
static void group_chat_lime_x3dh_stop_start_core_curve(const int curveId) {
	int dummy=0;
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);

	stats initialMarieStats = marie->stat;

	set_lime_curve_list(curveId,coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Set the last time a lime update was performed to an old date
	LinphoneConfig *config = linphone_core_get_config(marie->lc);
	time_t oldUpdateTime = ms_time(NULL)-88000; // 24 hours = 86400 ms
	linphone_config_set_int(config, "lime", "last_update_time", (int)oldUpdateTime);

	// take refs on current proxy and auth
	LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_ref(cfg);
	LinphoneAuthInfo *auth_info = (LinphoneAuthInfo *)bctbx_list_get_data(linphone_core_get_auth_info_list(marie->lc));
	linphone_auth_info_ref(auth_info);
	// Stop/start Core
	linphone_core_stop(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneGlobalShutdown, initialMarieStats.number_of_LinphoneGlobalShutdown + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneGlobalOff, initialMarieStats.number_of_LinphoneGlobalOff + 1, 5000));
	linphone_core_start(marie->lc);

	// restore dns setup, proxy and auth info
	linphone_core_manager_setup_dns(marie);
	linphone_core_remove_proxy_config(marie->lc, linphone_core_get_default_proxy_config(marie->lc));
	linphone_core_add_proxy_config(marie->lc, cfg);
	linphone_core_set_default_proxy_config(marie->lc, cfg);
	linphone_proxy_config_unref(cfg);
	linphone_core_add_auth_info(marie->lc, auth_info);
	linphone_auth_info_unref(auth_info);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Wait for update callback
	wait_for_list(coresList, &dummy, 1, 2000);

	// Check that we correctly performed an update
	int newUpdateTime = linphone_config_get_int(config, "lime", "last_update_time", -1);
	BC_ASSERT_GREATER_STRICT(newUpdateTime, (int)oldUpdateTime, int, "%d");

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
}

static void group_chat_lime_x3dh_stop_start_core(void) {
	group_chat_lime_x3dh_stop_start_core_curve(25519);
	group_chat_lime_x3dh_stop_start_core_curve(448);
}

static void group_chat_lime_x3dh_basic_chat_rooms_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	set_lime_curve_list(curveId,coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Send message in basic chat room
	LinphoneChatRoom *marieBasicCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *basicMessage1 = linphone_chat_room_create_message_from_utf8(marieBasicCr, "Hello from our basic chat room");
	LinphoneChatMessageCbs *cbs1 = linphone_chat_message_get_callbacks(basicMessage1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs1, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(basicMessage1);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneMessageDelivered,initialMarieStats.number_of_LinphoneMessageReceived + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneMessageReceived,initialPaulineStats.number_of_LinphoneMessageReceived + 1));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline->stat.last_received_chat_message), linphone_chat_message_get_utf8_text(basicMessage1));
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
	const LinphoneAddress *marieConferenceAddress = linphone_chat_room_get_conference_address(marieEncryptedCr);
	if (!BC_ASSERT_PTR_NOT_NULL(marieConferenceAddress))
		goto end;
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(marieConferenceAddress);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineEncryptedCr))
		goto end;
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends an encrypted message
	const char *marieEncryptedMessage1 = "Hello from our secured chat room";
	LinphoneChatMessage *msg = _send_message(marieEncryptedCr, marieEncryptedMessage1);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	linphone_chat_message_unref(msg);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieEncryptedMessage1);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Marie deletes the basic chat room
	linphone_core_manager_delete_chat_room(marie, marieBasicCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the basic chat room again
	LinphoneChatRoom *marieNewBasicCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *basicMessage2 = linphone_chat_room_create_message_from_utf8(marieNewBasicCr, "Hello again from our basic chat room");
	LinphoneChatMessageCbs *cbs2 = linphone_chat_message_get_callbacks(basicMessage2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs2, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(basicMessage2);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneMessageDelivered,initialMarieStats.number_of_LinphoneMessageReceived + 3));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneMessageReceived,initialPaulineStats.number_of_LinphoneMessageReceived + 3));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline->stat.last_received_chat_message), linphone_chat_message_get_utf8_text(basicMessage2));
	}
	linphone_chat_message_unref(basicMessage2);
	linphone_address_unref(encryptedConfAddr);
	linphone_address_unref(marieAddr);

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(pauline, paulineBasicCr, coresList);
	linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_basic_chat_rooms(void) {
	group_chat_lime_x3dh_basic_chat_rooms_curve(25519);
	group_chat_lime_x3dh_basic_chat_rooms_curve(448);
}

static void lime_x3dh_message_test (bool_t with_composing, bool_t with_response, bool_t sal_error, const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatMessage* msg;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(paulineCr));

	if (with_composing) {
		// Marie starts composing a message
		linphone_chat_room_compose(marieCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));
	}
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(marieCr));
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(paulineCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(paulineCr), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count(pauline->lc), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_active_locals(pauline->lc), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_local(pauline->lc, linphone_chat_room_get_local_address(paulineCr)), 0, int, "%d");

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	msg = _send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	linphone_chat_message_unref(msg);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	BC_ASSERT_FALSE(linphone_chat_room_is_empty(marieCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_empty(paulineCr));
	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(paulineCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count(pauline->lc), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_active_locals(pauline->lc), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_local(pauline->lc, linphone_chat_room_get_local_address(paulineCr)), 1, int, "%d");

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
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
		msg = _send_message(paulineCr, paulineMessage);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
		LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
			goto end;

		// Check that the response was correctly decrypted
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), paulineMessage);
		LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
		linphone_address_unref(paulineAddr);
	}

	// Check chat room security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	if (sal_error) {
		sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
		msg = _send_message(marieCr, "Bli bli bli");
		char *message_id = ms_strdup(linphone_chat_message_get_message_id(msg));
		BC_ASSERT_STRING_NOT_EQUAL(message_id, "");

		wait_for_list(coresList, NULL, 0, 1000);

		sal_set_send_error(linphone_core_get_sal(marie->lc), 0);
		linphone_chat_message_send(msg);
		const char *message_id_2 = linphone_chat_message_get_message_id(msg);
		BC_ASSERT_STRING_NOT_EQUAL(message_id_2, "");
		BC_ASSERT_STRING_EQUAL(message_id, message_id_2);
		ms_free(message_id);

		wait_for_list(coresList, NULL, 0, 1000);

		linphone_core_refresh_registers(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneRegistrationOk, initialMarieStats.number_of_LinphoneRegistrationOk + 1, 10000));

		linphone_chat_message_unref(msg);
	}

end:

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message (void) {
	lime_x3dh_message_test(FALSE, FALSE, FALSE, 25519);
	lime_x3dh_message_test(FALSE, FALSE, FALSE, 448);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_error(void) {
	lime_x3dh_message_test(FALSE, FALSE, TRUE, 25519);
	lime_x3dh_message_test(FALSE, FALSE, TRUE, 448);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_composing (void) {
	lime_x3dh_message_test(TRUE, FALSE, FALSE, 25519);
	lime_x3dh_message_test(TRUE, FALSE, FALSE, 448);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_response (void) {
	lime_x3dh_message_test(FALSE, TRUE, FALSE, 25519);
	lime_x3dh_message_test(FALSE, TRUE, FALSE, 448);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_response_and_composing (void) {
	lime_x3dh_message_test(TRUE, TRUE, FALSE, 25519);
	lime_x3dh_message_test(TRUE, TRUE, FALSE, 448);
}

static void group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	linphone_core_enable_lime_x3dh(laure->lc, FALSE);
	linphone_core_add_linphone_spec(laure->lc, "lime"); //Forcing lime_x3dh spec even if encryption engine is disabled

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on X3DH server (not for Laure which is not actually created on server)
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that Pauline received and decrypted the message
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	linphone_chat_message_unref(msg);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check that Laure did not receive the message because she did not post keys on the X3DH server
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 1000));

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
static void group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys (void) {
	group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys_curve(25519);
	group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys_curve(448);
}


static void group_chat_lime_x3dh_send_encrypted_file_with_or_without_text (bool_t with_text, bool_t two_files, bool_t use_buffer, const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *sendFilepath2 = NULL;
	char *receivePaulineFilepath = bc_tester_file("receive_file_secure_pauline.dump");
	char *receiveChloeFilepath = bc_tester_file("receive_file_secure_chloe.dump");
	const char *text = "Hello Group !";

	if (two_files) {
		sendFilepath2 = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	}

	// Globally configure an http file transfer server
	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));

	// Remove any previously downloaded file
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_X3dhUserCreationSuccess, initialChloeStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(chloeCr))
	    goto end;

	// Send encrypted file
	if (with_text) {
		_send_file_plus_text(marieCr, sendFilepath, sendFilepath2, text, use_buffer);
	} else {
		_send_file(marieCr, sendFilepath, sendFilepath2, use_buffer);
	}

	// Check that chat rooms have received the file
	if (with_text) {
		_receive_file_plus_text(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, sendFilepath2, text, use_buffer);
		_receive_file_plus_text(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, sendFilepath2, text, use_buffer);
	} else {
		_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, sendFilepath2, use_buffer);
		_receive_file(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, sendFilepath2, use_buffer);
	}
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);
	bc_free(sendFilepath);
	if (sendFilepath2) bc_free(sendFilepath2);
	bc_free(receivePaulineFilepath);
	bc_free(receiveChloeFilepath);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_send_encrypted_file (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(FALSE, FALSE, FALSE, 25519);
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(FALSE, FALSE, FALSE, 448);
}

static void group_chat_lime_x3dh_send_encrypted_file_2 (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(FALSE, FALSE, TRUE, 25519);
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(FALSE, FALSE, TRUE, 448);
}

static void group_chat_lime_x3dh_send_encrypted_file_plus_text (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(TRUE, FALSE, FALSE, 25519);
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(TRUE, FALSE, FALSE, 448);
}

static void group_chat_lime_x3dh_send_two_encrypted_files_plus_text (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(TRUE, TRUE, FALSE, 25519);
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(TRUE, TRUE, FALSE, 448);
}

static void group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base (bool_t with_app_restart, bool_t forward_message, bool_t reply_message, const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	char *messageId, *secondMessageId = NULL;
	set_lime_curve_list(curveId,coresManagerList);

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	const bctbx_list_t *contents = linphone_chat_message_get_contents(message);
	BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int , "%d");
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	messageId = bctbx_strdup(linphone_chat_message_get_message_id(message));
	linphone_chat_message_unref(message);

	if (forward_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int," %i");
		if (linphone_chat_room_get_history_size(paulineCr) > 0) {
			bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 1);
			LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)(history->data);

			LinphoneChatMessage *msg = linphone_chat_room_create_forward_message(paulineCr, recv_msg);
			const LinphoneAddress *forwarded_from_address = linphone_chat_message_get_from_address(recv_msg);
			char *forwarded_from = linphone_address_as_string_uri_only(forwarded_from_address);

			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_send(msg);

			BC_ASSERT_TRUE(linphone_chat_message_is_forward(msg));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(msg), forwarded_from);

			BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDelivered,1));
			BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);

			if (marie->stat.last_received_chat_message != NULL) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int," %i");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), textMessage);
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(marie->stat.last_received_chat_message),textMessage);
				BC_ASSERT_TRUE(linphone_chat_message_is_forward(marie->stat.last_received_chat_message));
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(marie->stat.last_received_chat_message), forwarded_from);
			}

			linphone_chat_message_unref(msg);
			ms_free(forwarded_from);
		}
	} else if (reply_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int," %i");
		if (linphone_chat_room_get_history_size(paulineCr) > 0) {
			bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 1);
			LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)(history->data);

			LinphoneChatMessage *msg = linphone_chat_room_create_reply_message(paulineCr, recv_msg);
			BC_ASSERT_TRUE(linphone_chat_message_is_reply(msg));
			linphone_chat_message_add_utf8_text_content(msg, "Quite funny!");
			BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(msg), 
														linphone_chat_message_get_from_address(recv_msg)));

			LinphoneChatMessage *orig = linphone_chat_message_get_reply_message(msg);
			BC_ASSERT_PTR_NOT_NULL(orig);
			BC_ASSERT_PTR_EQUAL(orig, recv_msg);
			if (orig) {
				const char *reply = linphone_chat_message_get_utf8_text(orig);
				BC_ASSERT_STRING_EQUAL(reply, "Hello");
 				linphone_chat_message_unref(orig);
			}

			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_send(msg);

			BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDelivered,1));
			BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);

			if (marie->stat.last_received_chat_message != NULL) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int," %i");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Quite funny!");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(marie->stat.last_received_chat_message),"Quite funny!");
				BC_ASSERT_TRUE(linphone_chat_message_is_reply(marie->stat.last_received_chat_message));
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_reply_message_id(marie->stat.last_received_chat_message), messageId);
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(msg), 
															linphone_chat_room_get_local_address(marieCr)));
			}

			secondMessageId = bctbx_strdup(linphone_chat_message_get_message_id(msg));
			linphone_chat_message_unref(msg);
		}
	}

	if (with_app_restart) {
		// To simulate dialog removal
		LinphoneAddress *marieAddr = linphone_address_clone(linphone_chat_room_get_peer_address(marieCr));
		linphone_core_set_network_reachable(marie->lc, FALSE);
		coresList = bctbx_list_remove(coresList, marie->lc);
		linphone_core_manager_reinit(marie);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(marie, TRUE);
		marieCr = linphone_core_get_chat_room(marie->lc, marieAddr);
		linphone_address_unref(marieAddr);
	}

	if (forward_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int," %i");
		if (linphone_chat_room_get_history_size(marieCr) > 1) {
			LinphoneChatMessage *recv_msg = linphone_chat_room_get_last_message_in_history(marieCr);
			BC_ASSERT_TRUE(linphone_chat_message_is_forward(recv_msg));

			// for marie, forward message by anonymous
			LinphoneChatMessage *msgFromMarie = linphone_chat_room_create_forward_message(marieCr, recv_msg);
			linphone_chat_message_send(msgFromMarie);

			BC_ASSERT_TRUE(linphone_chat_message_is_forward(msgFromMarie));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(msgFromMarie), "Anonymous");
			linphone_chat_message_unref(msgFromMarie);

			linphone_chat_message_unref(recv_msg);
		}
	} else if (reply_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int," %i");
		if (linphone_chat_room_get_history_size(marieCr) > 1) {
			LinphoneChatMessage *recv_msg = linphone_chat_room_get_last_message_in_history(marieCr);
			const char *body = linphone_chat_message_get_utf8_text(recv_msg);
			BC_ASSERT_STRING_EQUAL(body, "Quite funny!");

			BC_ASSERT_TRUE(linphone_chat_message_is_reply(recv_msg));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_reply_message_id(recv_msg), messageId);
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_message_id(recv_msg), secondMessageId);

			LinphoneChatMessage *orig = linphone_chat_message_get_reply_message(recv_msg);
			BC_ASSERT_PTR_NOT_NULL(orig);
			if (orig) {
				const char *reply = linphone_chat_message_get_utf8_text(orig);
				BC_ASSERT_STRING_EQUAL(reply, "Hello");
 				linphone_chat_message_unref(orig);
			}

			LinphoneChatMessage *msgFromMarie = linphone_chat_room_create_reply_message(marieCr, recv_msg);
			BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(recv_msg), 
														linphone_chat_message_get_from_address(msgFromMarie)));

			BC_ASSERT_TRUE(linphone_chat_message_is_reply(msgFromMarie));

			linphone_chat_message_add_utf8_text_content(msgFromMarie, "Still laughing!");

			linphone_chat_message_send(msgFromMarie);

			linphone_chat_message_unref(msgFromMarie);
			linphone_chat_message_unref(recv_msg);
		}
	} else {
		// Marie deletes the chat room
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		wait_for_list(coresList, 0, 1, 2000);
		BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

		// Pauline sends a new message
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;

		// Pauline sends a new message
		textMessage = "Hey you";
		message = _send_message(paulineCr, textMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), textMessage);
		linphone_chat_message_unref(message);

		// Check that the chat room has been correctly recreated on Marie's side
		marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 1, FALSE);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);
	}

	if (messageId) bc_free(messageId);
	if (secondMessageId) bc_free(secondMessageId);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int,"%i");
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(pauline->lc));

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void group_chat_lime_x3dh_unique_one_to_one_chat_room_send_forward_message (void) {
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(FALSE, TRUE, FALSE, 25519);
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(FALSE, TRUE, FALSE, 448);
}

static void group_chat_lime_x3dh_unique_one_to_one_chat_room_send_forward_message_with_restart (void) {
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE, TRUE, FALSE, 25519);
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE, TRUE, FALSE, 448);
}

static void group_chat_lime_x3dh_unique_one_to_one_chat_room_reply_forward_message (void) {
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(FALSE, FALSE, TRUE, 25519);
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(FALSE, FALSE, TRUE, 448);
}

static void group_chat_lime_x3dh_unique_one_to_one_chat_room_reply_forward_message_with_restart(void) {
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE, FALSE, TRUE, 25519);
	group_chat_lime_x3dh_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE, FALSE, TRUE, 448);
}

static void group_chat_lime_x3dh_verify_sas_before_message_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	BC_ASSERT_PTR_NOT_NULL(confAddr);
	if (confAddr) {
		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	}
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (!paulineCr) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	msg = _send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), paulineMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// ZRTP verification call Marie rejects the SAS
	initialMarieStats = marie->stat;

	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME status degraded to encrypted and ZRTP status
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_SecurityLevelDowngraded, initialMarieStats.number_of_SecurityLevelDowngraded + 1, 10000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

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


	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_verify_sas_before_message(void) {
	group_chat_lime_x3dh_verify_sas_before_message_curve(25519);
	group_chat_lime_x3dh_verify_sas_before_message_curve(448);
}

static void group_chat_lime_x3dh_reject_sas_before_message_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "lime", "unsafe_if_sas_refused", 1);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	if (linphone_config_get_int(linphone_core_get_config(marie->lc), "lime", "allow_message_in_unsafe_chatroom", 0) == 1) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
		LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
			goto end;

		// Check that the message was correctly decrypted by Pauline
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	linphone_chat_message_unref(msg);
	}

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	msg = _send_message(paulineCr, paulineMessage);

	// Marie does not receive Pauline's message because Pauline is unsafe for Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	linphone_chat_message_unref(msg);

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

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_reject_sas_before_message(void) {
	group_chat_lime_x3dh_reject_sas_before_message_curve(25519);
	group_chat_lime_x3dh_reject_sas_before_message_curve(448);
}

static void group_chat_lime_x3dh_message_before_verify_sas_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	msg = _send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), paulineMessage);
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


	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_message_before_verify_sas(void) {
	group_chat_lime_x3dh_message_before_verify_sas_curve(25519);
	group_chat_lime_x3dh_message_before_verify_sas_curve(448);
}

static void group_chat_lime_x3dh_message_before_reject_sas_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "lime", "unsafe_if_sas_refused", 1);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneChatRoom *paulineCr = NULL;

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
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


	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_message_before_reject_sas(void) {
	group_chat_lime_x3dh_message_before_reject_sas_curve(25519);
	group_chat_lime_x3dh_message_before_reject_sas_curve(448);
}

static void group_chat_lime_x3dh_message_before_verify_sas_with_call_from_device_with_zrtp_de_activated_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	//explicitly disable ZRTP on calling side to start ZRTP in automatic mode
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	msg = _send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), paulineMessage);
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


	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_message_before_verify_sas_with_call_from_device_with_zrtp_de_activated(void) {
	group_chat_lime_x3dh_message_before_verify_sas_with_call_from_device_with_zrtp_de_activated_curve(25519);
	group_chat_lime_x3dh_message_before_verify_sas_with_call_from_device_with_zrtp_de_activated_curve(448);
}


static void group_chat_lime_x3dh_chatroom_security_level_upgrade_curve(const int curveId) {
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

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_X3dhUserCreationSuccess, initialChloeStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr) || !BC_ASSERT_PTR_NOT_NULL(chloeCr))
		goto end;
	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	LinphoneChatMessage *marie_msg = _send_message(marieCr, marieMessage);
	linphone_chat_message_unref(marie_msg);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	// Check that the message was correctly received and decrypted by Chloe
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived, initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *chloeLastMsg = chloe->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(chloeLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(chloeLastMsg), marieMessage);
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
static void group_chat_lime_x3dh_chatroom_security_level_upgrade(void) {
	group_chat_lime_x3dh_chatroom_security_level_upgrade_curve(25519);
	group_chat_lime_x3dh_chatroom_security_level_upgrade_curve(448);
}

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant_curve(const int curveId) {
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

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_X3dhUserCreationSuccess, initialChloeStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	LinphoneChatMessage *marie_msg = _send_message(marieCr, marieMessage);
	linphone_chat_message_unref(marie_msg);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
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
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr))
		goto end;
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
static void group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant(void) {
	group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant_curve(25519);
	group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant_curve(448);
}

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp_arg (const bool_t unsafe_if_sas_refused, const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	LinphoneChatMessage *marie_msg = _send_message(marieCr, marieMessage);
	linphone_chat_message_unref(marie_msg);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// turn unsafe_if_sas_refused on according to parameter
	if (unsafe_if_sas_refused) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1);
	}

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
	if (linphone_config_get_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 0) == 1) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
		// Check that pauline's chatroom received a security event
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_ManInTheMiddleDetected, initialPaulineStats.number_of_ManInTheMiddleDetected + 1, 3000));
	} else {
		// Check that pauline's chatroom received a security event
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_SecurityLevelDowngraded, initialPaulineStats.number_of_SecurityLevelDowngraded + 1, 3000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	}


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

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp (void) {
	// First try without the unsafe_if_sas_refused flag on in pauline rc file
	group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp_arg(FALSE, 25519);
	group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp_arg(FALSE, 448);
	// Second try with the unsafe_if_sas_refused flag on in pauline rc file
	group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp_arg(TRUE, 25519);
	group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp_arg(TRUE, 448);
}

/**
 * Scenario:
 *  - marie, pauline and laure create a lime user and a chatroom
 *  - check their chatroom security level is encrypted
 *  - perform ZRTP call with SAS validation between all participants
 *  - check their chatroom security level is safe
 *  - pauline add a device
 *  - check all participants have a chatroom security level back to encrypted
 *  - pauline1 call pauline2 and reject the SAS (with unsafe_if_sas_rejected on)
 *  - check pauline1 chatroom security level is unsafe while others are encrypted
 */
static void group_chat_lime_x3dh_chatroom_security_level_self_multidevices_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *pauline2 = NULL;
	LinphoneChatRoom *pauline2Cr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	// pauline1 has unsafe_if_sas_refused turned on
	linphone_config_set_int(linphone_core_get_config(pauline1->lc), "lime", "unsafe_if_sas_refused", 1);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_X3dhUserCreationSuccess, initialPauline1Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1Cr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	LinphoneChatMessage *marie_msg = _send_message(marieCr, marieMessage);
	linphone_chat_message_unref(marie_msg);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline1LastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(pauline1LastMsg)));
	linphone_address_unref(marieAddr1);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);
	laureLastMsg = NULL;

	// Check that the encrypted security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline1Cr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// enable ZRTP on all devices
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline1->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification calls
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline1, TRUE, TRUE)));
	if (!call_ok) goto end;

	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(laure, pauline1, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the safe security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline1Cr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Create second device for Pauline
	pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	set_lime_curve(curveId,pauline2);
	stats initialPauline2Stats = pauline2->stat;
	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, pauline2);
	bctbx_list_t *newCoresList = init_core_for_conference(newCoresManagerList);
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Wait for Pauline2 lime user to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_X3dhUserCreationSuccess, initialPauline2Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2->lc));
	linphone_core_set_media_encryption(pauline2->lc, LinphoneMediaEncryptionZRTP);

	// Pauline2 is automatically added to the chatroom

	// Check that the chat room is correctly created on Pauline2's side and that she was added everywhere
	pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline2Cr))
		goto end;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participant_devices_added, initialPauline1Stats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 1, 3000));

	// Check that the security level is back to encrypted for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	// this one will fail if self other device is not used to compute the chatroom status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline1Cr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline2Cr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// ZRTP call with SAS rejected by pauline1 with pauline2 -> its status is set to unsafe
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline1, pauline2, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check that the security level is back to encrypted for everyone and unsafe for pauline1
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	// this one will fail if self other device is not used to compute the chatroom status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline1Cr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline2Cr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Marie sends a message to the chatroom
	marieMessage = "Hey guys ! What's up Pauline2!";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 2, 10000));
	linphone_chat_message_unref(msg);
	pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline1LastMsg), marieMessage);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Pauline2
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, initialPauline2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *pauline2LastMsg = pauline2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline2LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline2LastMsg), marieMessage);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
	laureLastMsg = NULL;

end:

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
static void group_chat_lime_x3dh_chatroom_security_level_self_multidevices(void) {
	group_chat_lime_x3dh_chatroom_security_level_self_multidevices_curve(25519);
	group_chat_lime_x3dh_chatroom_security_level_self_multidevices_curve(448);
}

static void group_chat_lime_x3dh_chatroom_security_alert_curve(const int curveId) {
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

	// Change the value of max_nb_device_per_participant to disallow multidevice
	linphone_config_set_int(linphone_core_get_config(marie->lc), "lime", "max_nb_device_per_participant", 1);
	linphone_config_set_int(linphone_core_get_config(pauline1->lc), "lime", "max_nb_device_per_participant", 1);
	linphone_config_set_int(linphone_core_get_config(laure->lc), "lime", "max_nb_device_per_participant", 1);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_X3dhUserCreationSuccess, initialPauline1Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1Cr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	LinphoneChatMessage *marie_msg = _send_message(marieCr, marieMessage);
	linphone_chat_message_unref(marie_msg);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline1LastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(pauline1LastMsg)));
	linphone_address_unref(marieAddr1);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 2, 10000));
	linphone_chat_message_unref(msg);
	pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline1LastMsg), marieMessage);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);

	// Create second device for Pauline
	pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	set_lime_curve(curveId,pauline2);
	linphone_config_set_int(linphone_core_get_config(pauline2->lc), "lime", "max_nb_device_per_participant", 1);
	stats initialPauline2Stats = pauline2->stat;
	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, pauline2);
	bctbx_list_t *newCoresList = init_core_for_conference(newCoresManagerList);
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Wait for Pauline2 lime user to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_X3dhUserCreationSuccess, initialPauline2Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
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

	const char *laureMessage = "I'm going to the cinema";
	// Laure sends a messages to trigger a LIME X3DH security alerts because maxNumberOfDevicePerParticipant has been exceeded
	if (linphone_config_get_int(linphone_core_get_config(laure->lc), "lime", "allow_message_in_unsafe_chatroom", 0) == 0) {
		LinphoneChatMessage *msg = _send_message(laureCr, laureMessage);
		int dummy=0;
		wait_for_list(coresList, &dummy, 1, 500); // sleep for 500 ms
		BC_ASSERT_FALSE((marie->stat.number_of_LinphoneMessageReceived == initialMarieStats.number_of_LinphoneMessageReceived + 1));
		BC_ASSERT_FALSE((pauline1->stat.number_of_LinphoneMessageReceived == initialPauline1Stats.number_of_LinphoneMessageReceived + 3));
		BC_ASSERT_FALSE((pauline2->stat.number_of_LinphoneMessageReceived == initialPauline2Stats.number_of_LinphoneMessageReceived + 1));
		linphone_chat_message_unref(msg);

		linphone_config_set_int(linphone_core_get_config(laure->lc), "lime", "allow_message_in_unsafe_chatroom", 1);
	}
	// to allow message to be sent
	linphone_config_set_int(linphone_core_get_config(laure->lc), "lime", "max_nb_device_per_participant", 2);
	msg = _send_message(laureCr, laureMessage);

	// Check that the message was correctly received and decrypted by Marie
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), laureMessage);
	marieLastMsg = NULL;

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 3, 10000));
	pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline1LastMsg), laureMessage);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Pauline2
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, initialPauline2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *pauline2LastMsg = pauline2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline2LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline2LastMsg), laureMessage);
	pauline2LastMsg = NULL;

end:

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
static void group_chat_lime_x3dh_chatroom_security_alert(void) {
	group_chat_lime_x3dh_chatroom_security_alert_curve(25519);
	group_chat_lime_x3dh_chatroom_security_alert_curve(448);
}

static void group_chat_lime_x3dh_call_security_alert_curve(const int curveId) {
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

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
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
	if (linphone_config_get_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 0) == 1) {
		// Check chatroom security event
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_ManInTheMiddleDetected, initialPaulineStats.number_of_ManInTheMiddleDetected + 1, 3000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	} else {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	}


end:

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_1(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_curve_list(curveId, coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", TRUE);
	
	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, "one to one", 1, TRUE);

	LinphoneChatMessage *message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "Do. Or do not. There is no try.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, 5000));
	linphone_chat_message_unref(message);

	if (marieOneToOneCr) {
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));
		/* The chatroom from Pauline is expected to terminate as well */
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));

		bctbx_list_t *participants = linphone_chat_room_get_participants(paulineOneToOneCr);
		BC_ASSERT_EQUAL((int)bctbx_list_size(participants), 1, int , "%d");
		bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);

		LinphoneChatMessage *exhume_message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "No. I am your father.");
		linphone_chat_message_send(exhume_message);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2, 5000));
		linphone_chat_message_unref(exhume_message);

		exhumedConfAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(paulineOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
		BC_ASSERT_EQUAL(pauline_messages, 2, int , "%d");
		
		if (exhumedConfAddr) {
			BC_ASSERT_FALSE(linphone_address_weak_equal(confAddr, exhumedConfAddr));
			marieOneToOneCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, exhumedConfAddr, "one to one", 1, TRUE);
			BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr);
			if (marieOneToOneCr) {
				int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
				BC_ASSERT_EQUAL(marie_messages, 1, int , "%d");

				LinphoneChatMessage *exhume_answer_message = linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "Nooooooooooooo !");
				linphone_chat_message_send(exhume_answer_message);
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
				linphone_chat_message_unref(exhume_answer_message);
			}
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int , "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int , "%d");
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_2(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_curve_list(curveId, coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", TRUE);
	
	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, "one to one", 1, TRUE);

	LinphoneChatMessage *message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "Help me, Obi-Wan Kenobi. You’re my only hope.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, 5000));
	linphone_chat_message_unref(message);

	if (marieOneToOneCr) {
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));
		/* The chatroom from Pauline is expected to terminate as well */
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));

		participantsAddresses = NULL;
		initialMarieStats = marie->stat;
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
		marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", TRUE);
		wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000);
		if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, 2, 5000));

		exhumedConfAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		if (exhumedConfAddr) {
			BC_ASSERT_FALSE(linphone_address_weak_equal(confAddr, exhumedConfAddr));
			LinphoneAddress *paulineNewConfAddr = linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(paulineOneToOneCr));
			BC_ASSERT_FALSE(linphone_address_weak_equal(confAddr, paulineNewConfAddr));
			BC_ASSERT_TRUE(linphone_address_weak_equal(exhumedConfAddr, paulineNewConfAddr));
			if (paulineNewConfAddr) linphone_address_unref(paulineNewConfAddr);

			LinphoneChatMessage *exhume_message = linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "I find your lack of faith disturbing.");
			linphone_chat_message_send(exhume_message);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
			linphone_chat_message_unref(exhume_message);

			int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 2, int , "%d");

			int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 1, int , "%d");

			LinphoneChatRoom *paulineOneToOneCr2 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, exhumedConfAddr, "one to one", 1, TRUE);
			BC_ASSERT_PTR_NOT_NULL(paulineOneToOneCr2);
			if (paulineOneToOneCr2) {
				int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr2);
				BC_ASSERT_EQUAL(pauline_messages, 2, int , "%d");

				LinphoneChatMessage *exhume_answer_message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr2, "Your focus determines your reality.");
				linphone_chat_message_send(exhume_answer_message);
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 2, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2, 5000));
				linphone_chat_message_unref(exhume_answer_message);
			}
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int , "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int , "%d");
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_1(void) {
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_1(25519);
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_1(448);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_2(void) {
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_2(25519);
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_2(448);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_3(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_curve_list(curveId, coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", FALSE);
	
	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, "one to one", 1, FALSE);

	LinphoneChatMessage *message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "Hasta la vista, baby.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, 5000));
	linphone_chat_message_unref(message);

	// Pauline goes offline
	int dummy = 0;
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	
	if (marieOneToOneCr) {
		LinphoneChatMessage *offline_message = linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "I'll be back.");
		linphone_chat_message_send(offline_message);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
		linphone_chat_message_unref(offline_message);

		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));
		/* The chatroom from Pauline won't be terminated as it is offline */
		BC_ASSERT_FALSE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));

		participantsAddresses = NULL;
		initialMarieStats = marie->stat;
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
		marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", FALSE);
		wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000);
		if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
		
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, 2, 5000));

		exhumedConfAddr = linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		if (exhumedConfAddr) {
			LinphoneChatMessage *exhume_message = linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "I'm back.");
			linphone_chat_message_send(exhume_message);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 2, 5000));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 2, 5000));
			linphone_chat_message_unref(exhume_message);

			// Pauline goes back online
			linphone_core_set_network_reachable(pauline->lc, TRUE);
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, 2, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 2, 5000));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));

			int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 3, int , "%d");

			int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 1, int , "%d");

			LinphoneChatMessage *post_exhume_message = linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "Sarah Connor ?");
			linphone_chat_message_send(post_exhume_message);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 3, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 3, 5000));
			linphone_chat_message_unref(post_exhume_message);

			pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 4, int , "%d");

			marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 2, int , "%d");
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int , "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int , "%d");
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_3(void) {
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_3(25519);
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_3(448);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_4(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_curve_list(curveId, coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", FALSE);
	
	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, "one to one", 1, FALSE);

	LinphoneChatMessage *message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "There is only one Lord of the Ring, only one who can bend it to his will. And he does not share power.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, 5000));
	linphone_chat_message_unref(message);

	if (marieOneToOneCr) {
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));
		/* The chatroom from Pauline is expected to terminate as well */
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));

		bctbx_list_t *participants = linphone_chat_room_get_participants(paulineOneToOneCr);
		BC_ASSERT_EQUAL((int)bctbx_list_size(participants), 1, int , "%d");
		bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);

		// Marie goes offline
		int dummy = 0;
		linphone_core_set_network_reachable(marie->lc, FALSE);
		wait_for_list(coresList, &dummy, 1, 2000);

		LinphoneChatMessage *exhume_message = linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "I am Gandalf the White. And I come back to you now... at the turn of the tide.");
		linphone_chat_message_send(exhume_message);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 2, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2, 5000));
		linphone_chat_message_unref(exhume_message);

		exhumedConfAddr = linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(paulineOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
		BC_ASSERT_EQUAL(pauline_messages, 2, int , "%d");

		// Marie goes back online
		linphone_core_set_network_reachable(marie->lc, TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneConferenceStateCreated, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomConferenceJoined, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2, 5000));
		
		if (exhumedConfAddr) {
			BC_ASSERT_FALSE(linphone_address_equal(confAddr, exhumedConfAddr));
			marieOneToOneCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, exhumedConfAddr, "one to one", 1, FALSE);
			BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr);
			if (marieOneToOneCr) {
				int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
				BC_ASSERT_EQUAL(marie_messages, 1, int , "%d");

				LinphoneChatMessage *exhume_answer_message = linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "In my experience there is no such thing as luck.");
				linphone_chat_message_send(exhume_answer_message);
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
				linphone_chat_message_unref(exhume_answer_message);
			}
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int , "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int , "%d");
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void exhume_group_chat_lime_x3dh_one_to_one_chat_room_4(void) {
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_4(25519);
	exhume_group_chat_lime_x3dh_one_to_one_chat_room_base_4(448);
}

static void group_chat_lime_x3dh_call_security_alert(void) {
	group_chat_lime_x3dh_call_security_alert_curve(25519);
	group_chat_lime_x3dh_call_security_alert_curve(448);
}

static void group_chat_lime_x3dh_send_multiple_successive_encrypted_messages_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

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
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie sends the message
	const char *marieMessage1 = "Hey !";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage1);

	// Check that message 1 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage1);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);
	paulineLastMsg = NULL;

	// Check that message 1 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage1);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);
	laureLastMsg = NULL;

	// Marie sends another message
	const char *marieMessage2 = "What's up ?";
	msg = _send_message(marieCr, marieMessage2);

	// Check that message 2 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	linphone_chat_message_unref(msg);
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage2);
	paulineLastMsg = NULL;

	// Check that message 2 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage2);
	laureLastMsg = NULL;

	// Marie sends yet another message
	const char *marieMessage3 = "I need to talk to you.";
	msg = _send_message(marieCr, marieMessage3);

	// Check that message 3 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 3, 10000));
	linphone_chat_message_unref(msg);
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage3);

	// Check that message 3 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 3, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage3);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

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
static void group_chat_lime_x3dh_send_multiple_successive_encrypted_messages(void) {
	group_chat_lime_x3dh_send_multiple_successive_encrypted_messages_curve(25519);
	group_chat_lime_x3dh_send_multiple_successive_encrypted_messages_curve(448);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh_curve(const int curveId) {
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

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
	// Pauline disables LIME X3DH
	linphone_core_enable_lime_x3dh(pauline->lc, FALSE);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Check that the IsComposing is ignored on pauline's side
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 1000));

	// Marie sends the message
	const char *marieMessage = "What's up ?";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that the message is discarded on pauline's side
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	linphone_chat_message_unref(msg);

	// Check the chatrooms security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelClearText, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_enable_lime_x3dh(pauline->lc, TRUE);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh(void) {
	group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh_curve(25519);
	group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh_curve(448);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_unable_to_decrypt_lime_x3dh_curve(const int curveId) {
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

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
	// Pauline discards marie LIME X3DH key
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "test", "force_lime_decryption_failure", 1);

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Check that the IsComposing is undecipherable and that an undecipherable message error IMDN is returned to Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 1000));

	// Marie sends the message
	const char *marieMessage = "What's up ?";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that the message is discarded and that an undecipherable message error IMDN is returned to Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	bctbx_list_t *participants = linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateNotDelivered);
	BC_ASSERT_PTR_NOT_NULL(participants);
	if (participants) {
		bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);
	}
	linphone_chat_message_unref(msg);

	// Check the chatrooms security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_unable_to_decrypt_lime_x3dh(void) {
	group_chat_lime_x3dh_send_encrypted_message_to_unable_to_decrypt_lime_x3dh_curve(25519);
	group_chat_lime_x3dh_send_encrypted_message_to_unable_to_decrypt_lime_x3dh_curve(448);
}

static void group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh_curve(const int curveId) {
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

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
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
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Check that the message is correctly discarded
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	linphone_chat_message_unref(msg);
end:
	// Clean local LIME X3DH databases
	linphone_core_enable_lime_x3dh(marie->lc, TRUE);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh(void) {
	group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh_curve(25519);
	group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh_curve(448);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants_curve(const int curveId) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *pauline3 = NULL;
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	LinphoneChatRoom *paulineCr3 = NULL;
	int dummy = 0;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Wait for lime users to be created on x3dh server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_X3dhUserCreationSuccess, initialMarie1Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_X3dhUserCreationSuccess, initialMarie2Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_X3dhUserCreationSuccess, initialPauline1Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_X3dhUserCreationSuccess, initialPauline2Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for all participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

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
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr1) || !BC_ASSERT_PTR_NOT_NULL(marieCr2) || !BC_ASSERT_PTR_NOT_NULL(paulineCr1) || !BC_ASSERT_PTR_NOT_NULL(paulineCr2) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr1), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Marie sends a message
	const char *marie1Message = "Hey ! What's up guys ?";
	LinphoneChatMessage *msg = _send_message(marieCr1, marie1Message);

	// Check that the message was received by everybody
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived, initialMarie2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, initialPauline2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);

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
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie2LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline1LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline2LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marie1Message);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr1), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	//pauline 3 arives late
	pauline3 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	set_lime_curve(curveId,pauline3);
	stats initialPauline3Stats = pauline3->stat;
	coresManagerList = bctbx_list_append(coresManagerList, pauline3);
	LinphoneAddress *factoryAddr = linphone_address_new(sFactoryUri);
	_configure_core_for_conference(pauline3, factoryAddr);
	linphone_address_unref(factoryAddr);
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
	configure_core_for_callbacks(pauline3, cbs);
	linphone_core_cbs_unref(cbs);
	coresList = bctbx_list_append(coresList, pauline3->lc);
	_start_core(pauline3);

	// Wait for lime users to be created on x3dh server
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline3->stat.number_of_X3dhUserCreationSuccess, initialPauline3Stats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	paulineCr3 = check_creation_chat_room_client_side(coresList, pauline3, &initialPauline3Stats, confAddr, initialSubject, 2, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr3))
		goto end;

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr3), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	// Marie sends a message
	const char *marie1Message2 = "Un nouveau ?";
	msg = _send_message(marieCr1, marie1Message2);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline3->stat.number_of_LinphoneMessageReceived, initialPauline3Stats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *pauline3LastMsg = pauline3->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline3LastMsg))
		goto end;

	// Check that the messages were correctly decrypted at least for pauline3
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline3LastMsg), marie1Message2);

end:

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marieCr1, coresList);
	wait_for_list(coresList, &dummy, 1, 1000); /* When marie 1 leaves, marie 2 will be left by the server too. This wait is to avoid a race between the two BYEs.*/
	linphone_core_manager_delete_chat_room(marie2, marieCr2, coresList);
	linphone_core_manager_delete_chat_room(pauline1, paulineCr1, coresList);
	wait_for_list(coresList, &dummy, 1, 1000); /* When marie 1 leaves, marie 2 will be left by the server too. This wait is to avoid a race between the two BYEs.*/
	linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	if (paulineCr3)
		linphone_core_manager_delete_chat_room(pauline3, paulineCr3, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	if (pauline3)
		linphone_core_manager_destroy(pauline3);
	linphone_core_manager_destroy(laure);
}
static void group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants(void) {
	group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants_curve(25519);
	group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants_curve(448);
}

static void group_chat_lime_x3dh_message_while_network_unreachable_curve(const int curveId, bool_t unreachable_during_setup) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	
	if (unreachable_during_setup){
		// Simulate pauline has disconnected
		linphone_core_set_network_reachable(pauline->lc, FALSE);
	}

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	
	if (!unreachable_during_setup){
		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
		if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr))
			goto end;

		// Simulate pauline has disconnected
		linphone_core_set_network_reachable(pauline->lc, FALSE);
	}

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);

	// Reconnect pauline
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	
	if (unreachable_during_setup){
		paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
		if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr))
			goto end;
	}

	// Check if the message is received
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_message_while_network_unreachable(void) {
	group_chat_lime_x3dh_message_while_network_unreachable_curve(25519, FALSE);
	group_chat_lime_x3dh_message_while_network_unreachable_curve(448, FALSE);
}

static void group_chat_lime_x3dh_message_while_network_unreachable_2(void){
	group_chat_lime_x3dh_message_while_network_unreachable_curve(448, TRUE);
}

static void group_chat_lime_x3dh_update_keys_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	int dummy=0;

	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime user creation
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	LinphoneChatMessage *msg = _send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Set the last time a lime update was performed to an recent date
	LinphoneConfig *config = linphone_core_get_config(marie->lc);
	time_t recentUpdateTime = ms_time(NULL)-22000; // 6 hours = 21600 ms
	linphone_config_set_int(config, "lime", "last_update_time", (int)recentUpdateTime);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Check that we have not performed an update
	int newUpdateTime = linphone_config_get_int(config, "lime", "last_update_time", -1);
	BC_ASSERT_EQUAL(newUpdateTime, (int)recentUpdateTime, int, "%d");

	// Set the last time a lime update was performed to an old date
	time_t oldUpdateTime = ms_time(NULL)-88000; // 24 hours = 86400 ms
	linphone_config_set_int(config, "lime", "last_update_time", (int)oldUpdateTime);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Wait for update callback
	wait_for_list(coresList, &dummy, 1, 2000);

	// Check that we correctly performed an update
	newUpdateTime = linphone_config_get_int(config, "lime", "last_update_time", -1);
	BC_ASSERT_GREATER_STRICT(newUpdateTime, (int)oldUpdateTime, int, "%d");

end:

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_lime_x3dh_update_keys(void) {
	group_chat_lime_x3dh_update_keys_curve(25519);
	group_chat_lime_x3dh_update_keys_curve(448);
}

static void chat_room_message_participant_state_changed(LinphoneChatRoom *cr, LinphoneChatMessage *msg, const LinphoneParticipantImdnState *state) {
	LinphoneChatRoomCbs *cbs = linphone_chat_room_get_current_callbacks(cr);
	LinphoneCoreManager *chloe = (LinphoneCoreManager *)linphone_chat_room_cbs_get_user_data(cbs);
	chloe->stat.number_of_participant_state_changed += 1;
}

static void imdn_for_group_chat_room_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	time_t initialTime = ms_time(NULL);

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));


	// Wait for lime user creation
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_X3dhUserCreationSuccess, initialChloeStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));


	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(chloeCr))
		goto end;
	LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_chat_message_participant_imdn_state_changed(cbs, chat_room_message_participant_state_changed);
	linphone_chat_room_add_callbacks(chloeCr, cbs);
	linphone_chat_room_cbs_set_user_data(cbs, chloe);
	linphone_chat_room_cbs_unref(cbs);

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageSent, 1, 1000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 5000));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participant_state_changed, 2, 1000));
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
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 0, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participant_state_changed, 3, 1000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 0, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participant_state_changed, 4, 1000));
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
static void imdn_for_group_chat_room(void) {
	imdn_for_group_chat_room_curve(25519);
	imdn_for_group_chat_room_curve(448);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Wait for lime user creation
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Marie deletes the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Pauline sends a new message
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;

	//kill flexisip just before this line (not LinphoneConferenceStateDeleted will not work a the end
	linphone_core_refresh_registers(marie->lc);
	linphone_core_refresh_registers(pauline->lc);
	wait_for_list(coresList, 0, 1, 2000);
	// Pauline sends a new message
	textMessage = "Hey you";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Check that the chat room has been correctly recreated on Marie's side
	marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr))
		goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesEncrypted);
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int,"%i");
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(pauline->lc));

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message(void) {
	group_chat_room_unique_one_to_one_chat_room_recreated_from_message_curve(25519);
	group_chat_room_unique_one_to_one_chat_room_recreated_from_message_curve(448);
}

/**
 * Scenario:
 * - Marie and Pauline exchange messages
 * - Pauline db is corrupted, its session with Marie is cancelled
 * - Marie sends a message to Pauline, she cannot decrypt, Marie is notified with a 488
 * - Marie resend the message, it is encrypted again with a new session as the old one is staled.
 * - Pauline decrypts with succes
 */
static void group_chat_lime_x3dh_session_corrupted_curve(const int curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	set_lime_curve_list(curveId,coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure->lc));

	// Wait for lime user creation
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's and Laure's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;

	// Marie send a message to Pauline and Laure
	const char *marieTextMessage = "Hello";
	LinphoneChatMessage *marieMessage = _send_message(marieCr, marieTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieTextMessage);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 10000)); // make sure the IMDN is back to marie
	wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients (Laure and Pauline receive each others IMDN)
	linphone_chat_message_unref(marieMessage);

	// Corrupt Pauline sessions in lime database: WARNING: if SOCI is not found, this call does nothing and the test fails
	lime_delete_DRSessions(pauline->lime_database_path);
	// Trick to force the reloading of the lime engine so the session in cache is cleared
	if (curveId==448) {
		set_lime_curve_list(25519,coresManagerList);
	} else {
		set_lime_curve_list(448,coresManagerList);
	}
	set_lime_curve_list(curveId,coresManagerList);

	// Marie send a new message, it shall fail and get a 488 response
	const char *marieTextMessage2 = "Do you copy?";
	marieMessage = _send_message(marieCr, marieTextMessage2);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 2, 10000)); // Delivered to the server
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000)); // the message is correctly received by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageNotDelivered, initialMarieStats.number_of_LinphoneMessageNotDelivered + 1, 10000)); // Not delivered to pauline
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived, 1, int, "%d");
	linphone_chat_message_unref(marieMessage);
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage2);
	marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr);

	// Try again, it shall work this time
	const char *marieTextMessage3 = "Hello again";
	initialMarieStats = marie->stat;
	marieMessage = _send_message(marieCr, marieTextMessage3);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 3, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieTextMessage3);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage3);
	marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr);
	linphone_chat_message_unref(marieMessage);

	end:
	wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients (Laure and Pauline receive each others IMDN)
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

static void group_chat_lime_x3dh_session_corrupted(void) {
	group_chat_lime_x3dh_session_corrupted_curve(25519);
	group_chat_lime_x3dh_session_corrupted_curve(448);
}

test_t secure_group_chat_tests[] = {
	TEST_ONE_TAG("LIME X3DH create lime user", group_chat_lime_x3dh_create_lime_user, "LimeX3DH"),
#if 0
	BROKEN TEST - see comment at the beginning of the test
	TEST_TWO_TAGS("LIME X3DH change server url", group_chat_lime_x3dh_change_server_url, "LimeX3DH", "LeaksMemory"),
#endif
	TEST_ONE_TAG("LIME X3DH encrypted chatrooms", group_chat_lime_x3dh_encrypted_chatrooms, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH basic chatrooms", group_chat_lime_x3dh_basic_chat_rooms, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message", group_chat_lime_x3dh_send_encrypted_message, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message with error", group_chat_lime_x3dh_send_encrypted_message_with_error, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message with composing", group_chat_lime_x3dh_send_encrypted_message_with_composing, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message with response", group_chat_lime_x3dh_send_encrypted_message_with_response, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message with response and composing", group_chat_lime_x3dh_send_encrypted_message_with_response_and_composing, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message to devices with and without keys on server", group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send encrypted file", group_chat_lime_x3dh_send_encrypted_file, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send encrypted file using buffer", group_chat_lime_x3dh_send_encrypted_file_2, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send encrypted file + text", group_chat_lime_x3dh_send_encrypted_file_plus_text, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send 2 encrypted files + text", group_chat_lime_x3dh_send_two_encrypted_files_plus_text, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send forward message", group_chat_lime_x3dh_unique_one_to_one_chat_room_send_forward_message, "LimeX3DH"),
	TEST_TWO_TAGS("LIME X3DH send forward message with restart", group_chat_lime_x3dh_unique_one_to_one_chat_room_send_forward_message_with_restart, "LimeX3DH", "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("LIME X3DH send reply message", group_chat_lime_x3dh_unique_one_to_one_chat_room_reply_forward_message, "LimeX3DH"),
	TEST_TWO_TAGS("LIME X3DH send reply message with core restart", group_chat_lime_x3dh_unique_one_to_one_chat_room_reply_forward_message_with_restart, "LimeX3DH", "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("LIME X3DH verify SAS before message", group_chat_lime_x3dh_verify_sas_before_message, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH reject SAS before message", group_chat_lime_x3dh_reject_sas_before_message, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message before verify SAS", group_chat_lime_x3dh_message_before_verify_sas, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message before reject SAS", group_chat_lime_x3dh_message_before_reject_sas, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message before verify SAS from a device with ZRTP configured called side only",group_chat_lime_x3dh_message_before_verify_sas_with_call_from_device_with_zrtp_de_activated,"LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH chatroom security level upgrade", group_chat_lime_x3dh_chatroom_security_level_upgrade, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH chatroom security level downgrade adding participant", group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH chatroom security level downgrade resetting zrtp", group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH chatroom security level self multidevices", group_chat_lime_x3dh_chatroom_security_level_self_multidevices, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH chatroom security alert", group_chat_lime_x3dh_chatroom_security_alert, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH exhumed one-to-one chat room 1", exhume_group_chat_lime_x3dh_one_to_one_chat_room_1, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH exhumed one-to-one chat room 2", exhume_group_chat_lime_x3dh_one_to_one_chat_room_2, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH exhumed one-to-one chat room 3", exhume_group_chat_lime_x3dh_one_to_one_chat_room_3, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH exhumed one-to-one chat room 4", exhume_group_chat_lime_x3dh_one_to_one_chat_room_4, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH call security alert", group_chat_lime_x3dh_call_security_alert, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH multiple successive messages", group_chat_lime_x3dh_send_multiple_successive_encrypted_messages, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH encrypted message to disabled LIME X3DH", group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH encrypted message to unable to decrypt LIME X3DH", group_chat_lime_x3dh_send_encrypted_message_to_unable_to_decrypt_lime_x3dh, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH plain message to enabled LIME X3DH", group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH message to multidevice participants", group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH messages while network unreachable", group_chat_lime_x3dh_message_while_network_unreachable, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH messages while network unreachable 2", group_chat_lime_x3dh_message_while_network_unreachable_2, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH update keys", group_chat_lime_x3dh_update_keys, "LimeX3DH"),
	TEST_ONE_TAG("Imdn", imdn_for_group_chat_room, "LimeX3DH"),
	TEST_ONE_TAG("Lime Unique one-to-one chatroom recreated from message", group_chat_room_unique_one_to_one_chat_room_recreated_from_message, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH stop/start core", group_chat_lime_x3dh_stop_start_core, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH session corrupted", group_chat_lime_x3dh_session_corrupted, "LimeX3DH")
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
