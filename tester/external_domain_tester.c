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

static void send_chat_message_to_group_chat_room(bctbx_list_t *coresList, LinphoneChatRoom *senderCr, bctbx_list_t *recipients, const char *msgText) {

	stats * recipients_initial_stats = NULL;
	int counter = 1;
	for (bctbx_list_t *it = recipients; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		// Allocate memory
		recipients_initial_stats = (stats*)realloc(recipients_initial_stats, counter * sizeof(stats));

		// Append element
		recipients_initial_stats[counter - 1] = m->stat;
		// Increment counter
		counter++;
	}

	LinphoneChatMessage *senderMessage = _send_message(senderCr, msgText);

	int idx = 0;
	for (bctbx_list_t *it = recipients; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		BC_ASSERT_TRUE(wait_for_list(coresList, &m->stat.number_of_LinphoneMessageReceived, recipients_initial_stats[idx].number_of_LinphoneMessageReceived + 1, 5000));

		LinphoneChatMessage *recipientLastMsg = m->stat.last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(recipientLastMsg);
		if (recipientLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(recipientLastMsg), msgText);
		}

		idx++;
	}

	linphone_chat_message_unref(senderMessage);
	if (recipients_initial_stats) {
		ms_free(recipients_initial_stats);
	}
}

/**
 * @param[in] encryption	true to activate message encryption
 * @param[in] external_sender	if true claire (from the external domain) will send the message, otherwise marie will do it
 */
static void group_chat (bool_t encryption, bool_t external_sender, bool_t restart_external_participant) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *claire = linphone_core_manager_create("claire_rc"); // External

	bctbx_list_t *externalCoresManagerList = NULL;
	externalCoresManagerList = bctbx_list_append(externalCoresManagerList, claire);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	LinphoneChatRoom *claireCr = NULL;

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
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, encryption);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr))
		goto end;

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, initialPaulineStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr))
		goto end;

	BC_ASSERT_TRUE(wait_for_list(coresList, &claire->stat.number_of_LinphoneConferenceStateCreationPending, initialClaireStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
	// Check that the chat room is correctly created on Claire's side and that the participants are added
	claireCr = check_creation_chat_room_client_side(coresList, claire, &initialClaireStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(claireCr))
		goto end;

	// Sender selection
	LinphoneChatRoom *senderCr = NULL;
	bctbx_list_t *recipients = NULL;
	recipients = bctbx_list_append(recipients, marie);

	// Sender (Pauline or Claire - external domain -) begins composing a message
	if (external_sender == TRUE) { // Claire is the sender
		senderCr = claireCr;
		recipients = bctbx_list_append(recipients, pauline);
	} else { // Pauline - from external domaine - is the sender
		senderCr = paulineCr;
		recipients = bctbx_list_append(recipients, claire);
	}

	if (!BC_ASSERT_PTR_NOT_NULL(senderCr))
		goto end;
	const char *msgText = "Hello";
	send_chat_message_to_group_chat_room(coresList, senderCr, recipients, msgText);

	LinphoneCoreManager * manager_to_restart = NULL;
	if (restart_external_participant) {
		manager_to_restart = claire;
	} else {
		manager_to_restart = marie;
	}

	BC_ASSERT_PTR_NOT_NULL(manager_to_restart);

	// Restart core for Marie
	coresList = bctbx_list_remove(coresList, manager_to_restart->lc);
	linphone_core_manager_reinit(manager_to_restart);

	if (encryption == TRUE) {
		linphone_config_set_string(linphone_core_get_config(manager_to_restart->lc),"lime","curve","c25519");
		if (bctbx_list_find(coresManagerList, manager_to_restart)) {
			linphone_core_set_lime_x3dh_server_url(manager_to_restart->lc, lime_server_c25519_tlsauth_opt_url);
		} else {
			linphone_core_set_lime_x3dh_server_url(manager_to_restart->lc, lime_server_c25519_external_url);
		}

	}

	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, manager_to_restart);
	init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);

	linphone_core_manager_start(manager_to_restart, TRUE);

	if (encryption == TRUE) {
		// Check encryption status for both participants
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(manager_to_restart->lc));
	}

	coresList = bctbx_list_append(coresList, manager_to_restart->lc);

	// Retrieve chat room
	LinphoneAddress *restartedManagerDeviceAddr = linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(manager_to_restart->lc)));
	LinphoneChatRoom *restartedManagerCr = linphone_core_search_chat_room(manager_to_restart->lc, NULL, restartedManagerDeviceAddr, confAddr, NULL);
	linphone_address_unref(restartedManagerDeviceAddr);
	BC_ASSERT_PTR_NOT_NULL(restartedManagerCr);

	if (restart_external_participant) {
		claireCr = restartedManagerCr;
	} else {
		marieCr = restartedManagerCr;
	}

	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(restartedManagerCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(restartedManagerCr), 1, int, "%d");

	// Sender selection
	LinphoneChatRoom *senderCr2 = NULL;
	bctbx_list_t *recipients2 = NULL;
	recipients2 = bctbx_list_append(recipients2, marie);

	// Sender (Pauline or Claire - external domain -) begins composing a message
	if (external_sender == TRUE) { // Claire is the sender
		if (restart_external_participant) {
			senderCr2 = restartedManagerCr;
		} else {
			senderCr2 = claireCr;
		}
		recipients2 = bctbx_list_append(recipients2, pauline);
	} else { // Pauline - from external domaine - is the sender
		senderCr2 = paulineCr;
		recipients2 = bctbx_list_append(recipients2, claire);
	}

	if (!BC_ASSERT_PTR_NOT_NULL(senderCr2))
		goto end;
	const char *msgText2 = "Hello again";
	send_chat_message_to_group_chat_room(coresList, senderCr2, recipients2, msgText2);

	linphone_address_unref(confAddr);

end:
	if (marieCr) {
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	if (claireCr) {
		linphone_core_manager_delete_chat_room(claire, claireCr, coresList);
	}

	if (paulineCr) {
		linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	}

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	bctbx_list_free(externalCoresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(claire);
}

static void group_chat_external_domain_participant (void) {
	group_chat(FALSE, FALSE, FALSE);
}
static void group_chat_external_domain_participant_ext_sender (void) {
	group_chat(FALSE, TRUE, FALSE);
}
static void encrypted_message(void) {
	group_chat(TRUE, FALSE, FALSE);
}
static void encrypted_message_ext_sender(void) {
	group_chat(TRUE, TRUE, FALSE);
}
static void group_chat_external_domain_participant_external_restart (void) {
	group_chat(FALSE, FALSE, TRUE);
}
static void encrypted_message_ext_sender_external_restart(void) {
	group_chat(TRUE, TRUE, TRUE);
}

test_t external_domain_tests[] = {
	TEST_NO_TAG("Simple call", simple_call),
	TEST_ONE_TAG("Message sent from domainA", group_chat_external_domain_participant, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Message sent from domainB", group_chat_external_domain_participant_ext_sender, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Encrypted message sent from domainA", encrypted_message, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Encrypted message sent from domainB", encrypted_message_ext_sender, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Message sent from domainA with external core restart", group_chat_external_domain_participant_external_restart, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Encrypted message sent from domainB with external core restart", encrypted_message_ext_sender_external_restart, "LeaksMemory" /*due to core restart*/)
};

test_suite_t external_domain_test_suite = {"External domain", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(external_domain_tests) / sizeof(external_domain_tests[0]), external_domain_tests};
