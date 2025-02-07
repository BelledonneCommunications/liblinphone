/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "bctoolbox/defs.h"
#include <bctoolbox/vfs.h>

#include "liblinphone_tester.h"
#include "linphone/api/c-account-cbs.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-chat-room.h"
#include "tester_utils.h"

static void
simple_account_creation_base(bool_t remove_accounts, bool_t bring_offline_while_removal, bool_t quick_shutdown) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	// Set the deletion timeout to 100s when the deletion timer is not expected to expire to ensure that account is
	// freed well before the timeout
	unsigned int account_deletion_timeout = (!!quick_shutdown || !bring_offline_while_removal) ? 100 : 5;
	linphone_core_set_account_deletion_timeout(marie->lc, account_deletion_timeout);
	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	int default_index = linphone_config_get_int(linphone_core_get_config(marie->lc), "sip", "default_proxy", 0);

	BC_ASSERT_PTR_NOT_NULL(marie_account);
	const LinphoneAccountParams *initial_params = linphone_account_get_params(marie_account);
	BC_ASSERT_PTR_NOT_NULL(initial_params);
	if (initial_params) {
		BC_ASSERT_STRING_EQUAL(linphone_account_params_get_custom_param(initial_params, "hidden"), "1");
	}

	// Use default_index from config file so that the account params is already configured
	LinphoneAccountParams *params = linphone_account_params_new_with_config(marie->lc, default_index);
	linphone_account_params_add_custom_param(params, "default", "yes");
	BC_ASSERT_STRING_EQUAL(linphone_account_params_get_custom_param(params, "default"), "yes");
	LinphoneAccount *new_account = linphone_account_new(marie->lc, params);
	linphone_account_add_custom_param(new_account, "main-account", "1");

	stats marieStats = marie->stat;
	const bctbx_list_t *accounts = linphone_core_get_account_list(marie->lc);
	int number_accounts = (int)bctbx_list_size(accounts);

	linphone_core_remove_account(marie->lc, marie_account);

	for (; accounts != NULL; accounts = accounts->next) {
		LinphoneAccount *account = (LinphoneAccount *)accounts->data;
		linphone_core_remove_account(marie->lc, account);
	}
	BC_ASSERT_PTR_NULL(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_default_proxy_config(marie->lc));
	if (!quick_shutdown) {
		if (!!bring_offline_while_removal) {
			linphone_core_set_network_reachable(marie->lc, FALSE);
			BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, 0, 1, (account_deletion_timeout + 1) * 1000));
			linphone_core_set_network_reachable(marie->lc, TRUE);
		} else {
			BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared,
			                        marieStats.number_of_LinphoneRegistrationCleared + number_accounts));
		}
		BC_ASSERT_PTR_NULL(linphone_core_get_deleted_account_list(marie->lc));
	}

	linphone_core_add_account(marie->lc, new_account);
	linphone_core_set_default_account(marie->lc, new_account);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationFailed,
	                        marieStats.number_of_LinphoneRegistrationFailed + 1));

	BC_ASSERT_TRUE(linphone_core_get_default_account(marie->lc) == new_account);
	BC_ASSERT_STRING_EQUAL(linphone_account_get_custom_param(new_account, "main-account"), "1");
	BC_ASSERT_STRING_EQUAL(linphone_account_get_custom_param(new_account, "default"), "yes");

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_default_proxy_config(marie->lc));

	linphone_account_params_unref(params);
	linphone_account_unref(new_account);

	// Verify that the config has the custom parameters
	BC_ASSERT_EQUAL(
	    linphone_config_get_int(linphone_core_get_config(marie->lc), "proxy_0", "x-custom-property:main-account", 0), 1,
	    int, "%0d");
	BC_ASSERT_STRING_EQUAL(
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "x-custom-property:default", 0),
	    "yes");

	char *local_rc = ms_strdup(marie->rc_local);
	bctbx_list_t *saved_accounts = NULL;

	accounts = linphone_core_get_account_list(marie->lc);
	if (!!remove_accounts) {
		unsigned int account_deletion_timeout = 5;
		linphone_core_set_account_deletion_timeout(marie->lc, account_deletion_timeout);
		for (; accounts != NULL; accounts = accounts->next) {
			LinphoneAccount *account = (LinphoneAccount *)accounts->data;
			linphone_core_remove_account(marie->lc, account);
		}
		BC_ASSERT_PTR_NULL(linphone_core_get_default_account(marie->lc));
		BC_ASSERT_PTR_NULL(linphone_core_get_default_proxy_config(marie->lc));
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, 0, 1, (account_deletion_timeout + 1) * 1000));
		BC_ASSERT_PTR_NULL(linphone_core_get_deleted_account_list(marie->lc));
	} else {
		for (; accounts != NULL; accounts = accounts->next) {
			LinphoneAccount *account = (LinphoneAccount *)accounts->data;
			saved_accounts = bctbx_list_append(saved_accounts, linphone_account_ref(account));
			LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(account));
			linphone_account_params_enable_register(account_params, FALSE);
			linphone_account_set_params(account, account_params);
			linphone_account_params_unref(account_params);
		}
	}

	if (!!remove_accounts) {
		unsigned int account_deletion_timeout = 5;
		linphone_core_set_account_deletion_timeout(marie->lc, account_deletion_timeout);
		marieStats = marie->stat;
		accounts = linphone_core_get_account_list(marie->lc);
		number_accounts = (int)bctbx_list_size(accounts);
		for (; accounts != NULL; accounts = accounts->next) {
			LinphoneAccount *account = (LinphoneAccount *)accounts->data;
			linphone_core_remove_account(marie->lc, account);
		}
		BC_ASSERT_PTR_NULL(linphone_core_get_default_account(marie->lc));
		BC_ASSERT_PTR_NULL(linphone_core_get_default_proxy_config(marie->lc));
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, 0, 1, (account_deletion_timeout + 1) * 1000));
		BC_ASSERT_PTR_NULL(linphone_core_get_deleted_account_list(marie->lc));
	}

	linphone_core_manager_stop(marie);
	linphone_core_manager_uninit2(marie, TRUE, FALSE); // uninit but do not unlink the rc file
	ms_free(marie);

	if (!remove_accounts) {
		// Verify that the custom parameters are written to the rc file
		bctbx_vfs_file_t *cfg_file = bctbx_file_open(bctbx_vfs_get_default(), local_rc, "r");
		size_t cfg_file_size = (size_t)bctbx_file_size(cfg_file);
		char *buf = bctbx_malloc(cfg_file_size);
		bctbx_file_read(cfg_file, buf, cfg_file_size, 0);
		BC_ASSERT_PTR_NOT_NULL(strstr(buf, "x-custom-property:main-account"));
		BC_ASSERT_PTR_NOT_NULL(strstr(buf, "x-custom-property:default"));
		bctbx_file_close(cfg_file);
		bctbx_free(buf);
	}

	if (saved_accounts) {
		bctbx_list_free_with_data(saved_accounts, (bctbx_list_free_func)linphone_address_unref);
	}

	unlink(local_rc);

	ms_free(local_rc);
}

static void simple_account_creation(void) {
	simple_account_creation_base(FALSE, FALSE, FALSE);
}

static void simple_account_creation_with_removal(void) {
	// Account freed when the core receives the 200Ok response to the unREGISTER message
	simple_account_creation_base(TRUE, FALSE, FALSE);
}

static void simple_account_creation_with_removal_offline(void) {
	// Account freed when the deletion timer expires
	simple_account_creation_base(TRUE, TRUE, FALSE);
}

static void simple_account_creation_with_removal_and_shutdown(void) {
	// Account freed during the core shutdown procedure
	simple_account_creation_base(TRUE, FALSE, TRUE);
}

static void simple_account_params_creation(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	LinphoneAccountParams *params = linphone_core_create_account_params(marie->lc);
	linphone_account_params_set_custom_contact(params, NULL);

	BC_ASSERT_PTR_NULL(linphone_account_params_get_custom_contact(params));

	linphone_account_params_unref(params);

	linphone_core_manager_destroy(marie);
}

static void default_account_removal(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *account = linphone_core_get_default_account(marie->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		BC_ASSERT_TRUE(linphone_account_get_state(account) == LinphoneRegistrationOk);
		linphone_core_remove_account(marie->lc, account);
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared, 1));
	}
	linphone_core_manager_destroy(marie);
}

static void default_account_removal_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *account = linphone_core_get_default_account(marie->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		BC_ASSERT_TRUE(linphone_account_get_state(account) == LinphoneRegistrationOk);
		linphone_account_refresh_register(account);
		BC_ASSERT_TRUE(linphone_account_get_state(account) == LinphoneRegistrationRefreshing);
		/* then immediately drop the account */
		linphone_core_remove_account(marie->lc, account);
		/* It should unregister anyway */
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared, 1));
	}
	linphone_core_manager_destroy(marie);
}

static void added_account_removal_base(bool_t enable_register) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	// Set the deletion timeout to 100s to ensure that account is freed well before the timeout
	linphone_core_set_account_deletion_timeout(marie->lc, 100);

	LinphoneAccount *marie_default_account = linphone_core_get_default_account(marie->lc);
	LinphoneAccountParams *marie_account_params =
	    linphone_account_params_clone(linphone_account_get_params(marie_default_account));
	linphone_account_params_enable_register(marie_account_params, enable_register);
	linphone_account_set_params(marie_default_account, marie_account_params);
	linphone_account_params_unref(marie_account_params);

	int default_index = linphone_config_get_int(linphone_core_get_config(marie->lc), "sip", "default_proxy", 0);
	LinphoneAccountParams *new_account_params = linphone_account_params_new_with_config(marie->lc, default_index);
	linphone_account_params_enable_register(new_account_params, enable_register);

	stats marieStats = marie->stat;
	linphone_core_remove_account(marie->lc, marie_default_account);
	if (enable_register) {
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared,
		                        marieStats.number_of_LinphoneRegistrationCleared + 1));
	}
	BC_ASSERT_PTR_NULL(linphone_core_get_deleted_account_list(marie->lc));

	LinphoneAccount *account = linphone_core_create_account(marie->lc, new_account_params);
	linphone_core_add_account(marie->lc, account);
	linphone_account_params_unref(new_account_params);
	linphone_account_unref(account);

	marieStats = marie->stat;
	if (enable_register) {
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationProgress,
		                        marieStats.number_of_LinphoneRegistrationProgress + 1));
	} else {
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, 0, 1, 1000));
	}
	const bctbx_list_t *marie_account_list = linphone_core_get_account_list(marie->lc);
	for (const bctbx_list_t *account_it = marie_account_list; account_it != NULL; account_it = account_it->next) {
		LinphoneAccount *account = (LinphoneAccount *)account_it->data;
		linphone_core_remove_account(marie->lc, account);
	}
	if (enable_register) {
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared,
		                        marieStats.number_of_LinphoneRegistrationCleared + 1));
	} else {
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, 0, 1, 1000));
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_deleted_account_list(marie->lc));

	linphone_core_manager_destroy(marie);
}

static void added_account_removal_no_register(void) {
	// The account is added but it didn't REGISTER therefore delete it immediately
	added_account_removal_base(FALSE);
}

static void added_account_removal_while_registering(void) {
	// This test verifies the behaviour of an account being added and immediately removed just after attempting to
	// REGISTER. The core will wait for the 200Ok response and send a second REGISTER with the Expires header set to 0
	// to unREGISTER. Upon reception of this 200Ok, the account is definitely freed
	added_account_removal_base(TRUE);
}

static void registration_state_changed_on_account(LinphoneAccount *account,
                                                  LinphoneRegistrationState state,
                                                  BCTBX_UNUSED(const char *message)) {
	LinphoneCore *lc = linphone_account_get_core(account);
	stats *counters;
	ms_message("New registration state %s for user id [%s] at account [%s]\n",
	           linphone_registration_state_to_string(state),
	           linphone_account_params_get_identity(linphone_account_get_params(account)),
	           linphone_account_params_get_server_addr(linphone_account_get_params(account)));
	counters = get_stats(lc);
	switch (state) {
		case LinphoneRegistrationNone:
			counters->number_of_LinphoneRegistrationNone++;
			break;
		case LinphoneRegistrationProgress:
			counters->number_of_LinphoneRegistrationProgress++;
			break;
		case LinphoneRegistrationOk:
			counters->number_of_LinphoneRegistrationOk++;
			break;
		case LinphoneRegistrationCleared:
			counters->number_of_LinphoneRegistrationCleared++;
			break;
		case LinphoneRegistrationFailed:
			counters->number_of_LinphoneRegistrationFailed++;
			break;
		default:
			BC_FAIL("unexpected event");
			break;
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

static void reconnection_after_network_change(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 10000));

	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2, 10000));
	/* simulate a transport error*/
	sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
	linphone_core_refresh_registers(marie->lc);
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationFailed, 1, 10000));
	sal_set_send_error(linphone_core_get_sal(marie->lc), 0);
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(marie->lc, TRUE);

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
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);

	initialPaulineStats = pauline->stat;

	// Change pauline transport
	LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);
	LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(pauline_account));
	linphone_account_params_set_transport(params, LinphoneTransportUdp);
	linphone_account_set_params(pauline_account, params);
	linphone_account_params_unref(params);

	// Check that pauline does not receive another invite
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined,
	                              initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 10000));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unregister_at_stop(void) {

	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	stats marieStats = marie->stat;
	stats paulineStats = pauline->stat;

	// Set value for unregister at stop parameter
	LinphoneAccount *marieAccount = linphone_core_get_default_account(marie->lc);
	LinphoneAccount *paulineAccount = linphone_core_get_default_account(pauline->lc);
	LinphoneAccountParams *marieAccountParams =
	    linphone_account_params_clone(linphone_account_get_params(marieAccount));
	LinphoneAccountParams *paulineAccountParams =
	    linphone_account_params_clone(linphone_account_get_params(paulineAccount));
	linphone_account_params_enable_unregister_at_stop(marieAccountParams, TRUE);
	linphone_account_params_enable_unregister_at_stop(paulineAccountParams, FALSE);
	linphone_account_set_params(marieAccount, marieAccountParams);
	linphone_account_set_params(paulineAccount, paulineAccountParams);
	linphone_account_params_unref(marieAccountParams);
	linphone_account_params_unref(paulineAccountParams);

	// Stop cores
	if (marie->lc) {
		linphone_core_stop(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneGlobalShutdown,
		                             marieStats.number_of_LinphoneGlobalShutdown + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneGlobalOff,
		                             marieStats.number_of_LinphoneGlobalOff + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneRegistrationCleared,
		                             marieStats.number_of_LinphoneRegistrationCleared + 1, 1000));
	} else {
		BC_FAIL("no core for Marie");
	}
	if (pauline->lc) {
		linphone_core_stop(pauline->lc);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneGlobalShutdown,
		                             paulineStats.number_of_LinphoneGlobalShutdown + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneGlobalOff,
		                             paulineStats.number_of_LinphoneGlobalOff + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneRegistrationCleared,
		                              paulineStats.number_of_LinphoneRegistrationCleared + 1, 1000));
	} else {
		BC_FAIL("no core for Pauline");
	}
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void account_dependency_to_self(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *marie_dependent_cfg =
	    (LinphoneProxyConfig *)linphone_core_get_proxy_config_list(marie->lc)->next->data;
	LinphoneAddress *marie_secondary_address = NULL;

	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));
	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));

	marie_secondary_address = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_dependent_cfg));

	/* Clear all proxy config, wait for unregistration*/
	linphone_core_clear_proxy_config(marie->lc);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared, 2));

	LinphoneAccountParams *marie_dependent_params = linphone_account_params_new(marie->lc, TRUE);
	linphone_account_params_set_identity_address(marie_dependent_params, marie_secondary_address);
	linphone_account_params_set_server_addr(marie_dependent_params, "sip:external.example.org:5068;transport=tcp");

	bctbx_list_t *list = NULL;
	const char *route = "sip:external.example.org:5068;transport=tcp";
	list = bctbx_list_append(list, linphone_address_new(route));
	linphone_account_params_set_routes_addresses(marie_dependent_params, list);
	bctbx_list_free_with_data(list, (bctbx_list_free_func)linphone_address_unref);

	linphone_account_params_set_register_enabled(marie_dependent_params, TRUE);
	linphone_address_unref(marie_secondary_address);
	LinphoneAccount *new_account = linphone_account_new(marie->lc, marie_dependent_params);
	linphone_account_set_dependency(new_account, new_account);
	linphone_core_add_account(marie->lc, new_account);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 3));

	linphone_account_params_unref(marie_dependent_params);
	linphone_account_unref(new_account);

	linphone_core_manager_destroy(marie);
}

static void supported_tags_handled_by_account_test(bool_t empty_list) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 5000));

	LinphoneAccountCbs *cbs = linphone_factory_create_account_cbs(linphone_factory_get());
	linphone_account_cbs_set_registration_state_changed(cbs, registration_state_changed_on_account);
	linphone_account_add_callbacks(marie_account, cbs);
	linphone_account_cbs_unref(cbs);

	// Clone Marie's account parameters to modify them
	LinphoneAccountParams *marie_account_params =
	    linphone_account_params_clone(linphone_account_get_params(marie_account));
	// Define a new list of supported tags
	bctbx_list_t *list = NULL;
	if (!empty_list) {
		list = bctbx_list_append(list, "replaces");
		list = bctbx_list_append(list, "outbound");
		list = bctbx_list_append(list, "path");
	}
	// Apply the new list of supported tags to Marie's account
	linphone_account_params_set_supported_tags_list(marie_account_params, list);
	linphone_account_set_params(marie_account, marie_account_params);
	linphone_account_params_unref(marie_account_params);

	// Simulate a network reconnection to trigger re-registration
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 3, 5000));

	// Retrieve Marie's contact address and verify the "gr" parameter is absent
	LinphoneAddress *marie_contact_address = linphone_account_get_contact_address(marie_account);
	const char *gruu = linphone_address_get_uri_param(marie_contact_address, "gr");
	BC_ASSERT_PTR_NULL(gruu);

	bctbx_list_free(list);
	linphone_core_manager_destroy(marie);
}

static void supported_tags_handled_by_account_without_gruu_test(void) {
	supported_tags_handled_by_account_test(false);
}

static void supported_tags_handled_by_account_empty_list_test(void) {
	supported_tags_handled_by_account_test(true);
}

static test_t account_tests[] = {
    TEST_NO_TAG("Simple account creation", simple_account_creation),
    TEST_NO_TAG("Simple account creation with removal", simple_account_creation_with_removal),
    TEST_NO_TAG("Simple account creation with removal offline", simple_account_creation_with_removal_offline),
    TEST_NO_TAG("Simple account creation with removal and shutdown", simple_account_creation_with_removal_and_shutdown),
    TEST_NO_TAG("Default account removal", default_account_removal),
    TEST_NO_TAG("Default account removal while refreshing", default_account_removal_2),
    TEST_NO_TAG("Added account removal (while REGISTERing)", added_account_removal_while_registering),
    TEST_NO_TAG("Added account removal (no REGISTER)", added_account_removal_no_register),
    TEST_NO_TAG("Simple account params creation", simple_account_params_creation),
    TEST_NO_TAG("Account dependency to self", account_dependency_to_self),
    TEST_NO_TAG("Registration state changed callback on account", registration_state_changed_callback_on_account),
    TEST_NO_TAG("No unregister when changing transport", no_unregister_when_changing_transport),
    TEST_NO_TAG("Unregister at stop", unregister_at_stop),
    TEST_NO_TAG("Account reconnection after multiple network changes", reconnection_after_network_change),
    TEST_NO_TAG("Supported tags handled by account (without gruu)",
                supported_tags_handled_by_account_without_gruu_test),
    TEST_NO_TAG("Supported tags handled by account (empty list)", supported_tags_handled_by_account_empty_list_test),
};

test_suite_t account_test_suite = {"Account",
                                   NULL,
                                   NULL,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(account_tests) / sizeof(account_tests[0]),
                                   account_tests,
                                   0};
