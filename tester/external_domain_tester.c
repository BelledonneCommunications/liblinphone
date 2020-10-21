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
	simple_call_base_with_rcs("claire_rc", "pauline_rc", FALSE, FALSE, FALSE);
};

/**
 * @param[in] encryption	true to activate message encryption
 * @param[in] external_sender	if true claire (from the external domain) will send the message, otherwise marie will do it
 */
static void group_chat (bool_t encryption, bool_t external_sender) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *claire = linphone_core_manager_create("claire_rc"); // External

	bctbx_list_t *externalCoresManagerList = NULL;
	externalCoresManagerList = bctbx_list_append(externalCoresManagerList, claire);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	if (encryption == TRUE) {
		// marie and pauline are on the regular lime server
		linphone_config_set_string(linphone_core_get_config(marie->lc),"lime","curve","c25519");
		linphone_core_set_lime_x3dh_server_url(marie->lc, lime_server_c25519_tlsauth_opt_url);
		linphone_config_set_string(linphone_core_get_config(pauline->lc),"lime","curve","c25519");
		linphone_core_set_lime_x3dh_server_url(pauline->lc, lime_server_c25519_tlsauth_opt_url);
		// claire uses the lime-external one
		linphone_config_set_string(linphone_core_get_config(claire->lc),"lime","curve","c25519");
		linphone_core_set_lime_x3dh_server_url(claire->lc, lime_server_c25519_external_url);
	}

	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialClaireStats = claire->stat;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	const char * sFactoryUri = "sip:conference-factory@conf.external-domain.org";
	bctbx_list_t *externalCoresList = init_core_for_conference_with_factory_uri(externalCoresManagerList, sFactoryUri);

	start_core_for_conference(coresManagerList);
	start_core_for_conference(externalCoresManagerList);


	if (encryption == TRUE) {
		// Wait for lime users to be created on X3DH server
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(externalCoresList, &claire->stat.number_of_X3dhUserCreationSuccess, initialClaireStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

		// Check encryption status for both participants
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(claire->lc));
	}

	bctbx_list_t *participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(claire->lc)));

	coresList = bctbx_list_concat(coresList, externalCoresList);

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, encryption);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &claire->stat.number_of_LinphoneConferenceStateCreationPending, initialClaireStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
	// Check that the chat room is correctly created on Claire's side and that the participants are added
	LinphoneChatRoom *claireCr = check_creation_chat_room_client_side(coresList, claire, &initialClaireStats, confAddr, initialSubject, 2, FALSE);

	// Sender selection
	LinphoneChatRoom *senderCr = NULL;
	LinphoneCoreManager *recipient1CoreManager = NULL;
	LinphoneCoreManager *recipient2CoreManager = NULL;
	stats *recipient1InitialStats = NULL;
	stats *recipient2InitialStats = NULL;

	if (external_sender == TRUE) { // Claire is the sender
		senderCr = claireCr;
		recipient1CoreManager = marie;
		recipient2CoreManager = pauline;
		recipient1InitialStats = &initialMarieStats;
		recipient2InitialStats = &initialPaulineStats;
	} else { // Pauline - from external domaine - is the sender
		senderCr = paulineCr;
		recipient1CoreManager = marie;
		recipient2CoreManager = claire;
		recipient1InitialStats = &initialMarieStats;
		recipient2InitialStats = &initialClaireStats;
	}

	// Sender (Pauline or Claire - external domain -) begins composing a message
	const char *senderTextMessage = "Hello";
	LinphoneChatMessage *senderMessage = _send_message(senderCr, senderTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient1CoreManager->stat.number_of_LinphoneMessageReceived, recipient1InitialStats->number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient2CoreManager->stat.number_of_LinphoneMessageReceived, recipient2InitialStats->number_of_LinphoneMessageReceived + 1, 5000));
	LinphoneChatMessage *recipient1LastMsg = recipient1CoreManager->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(recipient1LastMsg))
		goto end;
	LinphoneChatMessage *recipient2LastMsg = recipient2CoreManager->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(recipient2LastMsg))
		goto end;

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(recipient1LastMsg), senderTextMessage);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(recipient2LastMsg), senderTextMessage);

end:
	linphone_chat_message_unref(senderMessage);

	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(claire, claireCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(claire);
}

static void group_chat_external_domain_participant (void) {
	group_chat(FALSE, FALSE);
}
static void group_chat_external_domain_participant_ext_sender (void) {
	group_chat(FALSE, TRUE);
}
static void encrypted_message(void) {
	group_chat(TRUE, FALSE);
}
static void encrypted_message_ext_sender(void) {
	group_chat(TRUE, TRUE);
}

test_t external_domain_tests[] = {
	TEST_NO_TAG("Simple call", simple_call),
	TEST_NO_TAG("Message sent from domainA", group_chat_external_domain_participant),
	TEST_NO_TAG("Message sent from domainB", group_chat_external_domain_participant_ext_sender),
	TEST_NO_TAG("Encrypted message sent from domainA", encrypted_message),
	TEST_NO_TAG("Encrypted message sent from domainB", encrypted_message_ext_sender)
};

test_suite_t external_domain_test_suite = {"External domain", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(external_domain_tests) / sizeof(external_domain_tests[0]), external_domain_tests};
