/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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
#include "tester_utils.h"

static void simple_account_creation(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	int default_index = linphone_config_get_int(linphone_core_get_config(marie->lc), "sip", "default_proxy", 0);

	BC_ASSERT_PTR_NOT_NULL(marie_account);
	BC_ASSERT_PTR_NOT_NULL(linphone_account_get_params(marie_account));

	// Use default_index from config file so that the account params is already configured
	LinphoneAccountParams *params = linphone_account_params_new_with_config(marie->lc, default_index);
	LinphoneAccount *new_account = linphone_account_new(marie->lc, params);

	linphone_core_remove_account(marie->lc, marie_account);
	BC_ASSERT_PTR_NULL(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_default_proxy_config(marie->lc));

	linphone_core_add_account(marie->lc, new_account);
	linphone_core_set_default_account(marie->lc, new_account);

	BC_ASSERT_TRUE(linphone_core_get_default_account(marie->lc) == new_account);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_default_proxy_config(marie->lc));

	linphone_account_params_unref(params);
	linphone_account_unref(new_account);
	linphone_core_manager_destroy(marie);
}

void registration_state_changed_on_account(LinphoneAccount *account, LinphoneRegistrationState state, const char *message) {
	LinphoneCore *lc = linphone_account_get_core(account);
	stats* counters;
	ms_message("New registration state %s for user id [%s] at account [%s]\n"
		   ,linphone_registration_state_to_string(state)
		   ,linphone_account_params_get_identity(linphone_account_get_params(account))
		   ,linphone_account_params_get_server_addr(linphone_account_get_params(account)));
	counters = get_stats(lc);
	switch (state) {
	case LinphoneRegistrationNone:counters->number_of_LinphoneRegistrationNone++;break;
	case LinphoneRegistrationProgress:counters->number_of_LinphoneRegistrationProgress++;break;
	case LinphoneRegistrationOk:counters->number_of_LinphoneRegistrationOk++;break;
	case LinphoneRegistrationCleared:counters->number_of_LinphoneRegistrationCleared++;break;
	case LinphoneRegistrationFailed:counters->number_of_LinphoneRegistrationFailed++;break;
	default:
		BC_FAIL("unexpected event");break;
	}
}

static void registration_state_changed_callback_on_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 5000));

	LinphoneAccountCbs *cbs = linphone_factory_create_account_cbs(linphone_factory_get());
	linphone_account_cbs_set_registration_state_changed(cbs, registration_state_changed_on_account);
	linphone_account_add_callbacks(marie_account, cbs);
	linphone_account_cbs_unref(cbs);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Number on registrations should be 3 as it should add +1 on the already present callback in core
	// and +1 with the newly added callback from the account
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 3, 5000));

	linphone_core_manager_destroy(marie);
}

static void no_unregister_when_changing_transport(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

	initialPaulineStats = pauline->stat;

	// Change pauline tranport
	LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);
	LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(pauline_account));
	linphone_account_params_set_transport(params, LinphoneTransportUdp);
	linphone_account_set_params(pauline_account, params);
	linphone_account_params_unref(params);

	// Check that pauline does not receive another invite
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 10000));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t account_tests[] = {
	TEST_NO_TAG("Simple account creation", simple_account_creation),
	TEST_NO_TAG("Registration state changed callback on account", registration_state_changed_callback_on_account),
	TEST_NO_TAG("No unregister when changing transport", no_unregister_when_changing_transport)
};

test_suite_t account_test_suite = {
	"Account", NULL, NULL,
	liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(account_tests) / sizeof(account_tests[0]), account_tests
};