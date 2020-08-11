/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include "liblinphone_tester.h"
static const int x3dhServer_creationTimeout = 5000;

static void simple_call(void) {
	simple_call_base_with_rcs("claire_sips_rc", "pauline_sips_rc", FALSE, FALSE, FALSE);
};

static void group_chat_external_domain_participant (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *claire = linphone_core_manager_create("claire_sips_rc"); // External

	bctbx_list_t *externalCoresManagerList = NULL;
	const char * sFactoryUri = "sip:conference-factory@conf.external-domain.org";
	externalCoresManagerList = bctbx_list_append(externalCoresManagerList, claire);
	bctbx_list_t *externalCoresList = init_core_for_conference_with_factori_uri(externalCoresManagerList, sFactoryUri);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);
	start_core_for_conference(externalCoresManagerList);

	bctbx_list_t *participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(claire->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialClaireStats = claire->stat;

	coresList = bctbx_list_concat(coresList, externalCoresList);

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &claire->stat.number_of_LinphoneChatRoomStateCreationPending, initialClaireStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	// Check that the chat room is correctly created on Claire's side and that the participants are added
	LinphoneChatRoom *claireCr = check_creation_chat_room_client_side(coresList, claire, &initialClaireStats, confAddr, initialSubject, 2, FALSE);

	// Pauline begins composing a message
	const char *paulineTextMessage = "Hello";
	LinphoneChatMessage *paulineMessage = _send_message(paulineCr, paulineTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &claire->stat.number_of_LinphoneMessageReceived, initialClaireStats.number_of_LinphoneMessageReceived + 1, 5000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineTextMessage);

end:
	linphone_chat_message_unref(paulineMessage);

	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(claire, claireCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(claire);
}

static void encrypted_message(void) {
	LinphoneCoreManager *claire = linphone_core_manager_create("claire_sips_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_sips_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, claire);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *claireEncryptedCr = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;

	stats initialClaireStats = claire->stat;
	stats initialPaulineStats = pauline->stat;

	linphone_config_set_string(linphone_core_get_config(claire->lc),"lime","curve","c25519");
	linphone_core_set_lime_x3dh_server_url(claire->lc, lime_server_external_url);
	linphone_config_set_string(linphone_core_get_config(pauline->lc),"lime","curve","c25519");
	linphone_core_set_lime_x3dh_server_url(pauline->lc, lime_server_c25519_tlsauth_opt_url);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &claire->stat.number_of_X3dhUserCreationSuccess, initialClaireStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(claire->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Pauline creates an encrypted chatroom
	const char *initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(claire->lc)));
	paulineEncryptedCr = create_chat_room_client_side(coresList, pauline, &initialPaulineStats, participantsAddresses, initialSubject, TRUE);
	//LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(paulineEncryptedCr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	/* TODO: multidomain conference is not operational yet...

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	claireEncryptedCr = check_creation_chat_room_client_side(coresList, claire, &initialClaireStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(claireEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(claireEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Pauline sends an encrypted message
	const char *paulineMessage = "We can say whatever we want in this chatrooom!";
	LinphoneChatMessage *msg = _send_message(paulineEncryptedCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &claire->stat.number_of_LinphoneMessageReceived, initialClaireStats.number_of_LinphoneMessageReceived + 1, 10000));
	linphone_chat_message_unref(msg);
	LinphoneChatMessage *claireLastMsg = claire->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(claireLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(claireLastMsg), paulineMessage);
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(claireLastMsg)));
	linphone_address_unref(paulineAddr);
	*/

//end:
	// Clean db from chat room
	if (claireEncryptedCr) linphone_core_manager_delete_chat_room(claire, claireEncryptedCr, coresList);
	if (paulineEncryptedCr) linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(claire);
	linphone_core_manager_destroy(pauline);
}

test_t external_domain_tests[] = {
	TEST_NO_TAG("Simple call", simple_call),
	TEST_NO_TAG("Chatroom with external domain participant", group_chat_external_domain_participant),
	TEST_NO_TAG("Encrypted Message", encrypted_message)
};

test_suite_t external_domain_test_suite = {"External domain", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(external_domain_tests) / sizeof(external_domain_tests[0]), external_domain_tests};
