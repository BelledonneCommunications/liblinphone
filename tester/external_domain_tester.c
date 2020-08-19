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

test_t external_domain_tests[] = {
	TEST_NO_TAG("Simple call", simple_call),
	TEST_NO_TAG("Chatroom with external domain participant", group_chat_external_domain_participant)
};

test_suite_t external_domain_test_suite = {"External domain", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(external_domain_tests) / sizeof(external_domain_tests[0]), external_domain_tests};
