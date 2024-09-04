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

#include <ctype.h>

#include <json/json.h>

#include "bctoolbox/defs.h"

#include "account/account-params.h"
#include "core/core.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/flexi-api-client.h"
#include "tester_utils.h"

static const int TIMEOUT_REQUEST = 3000;

static LinphoneAccountCreator *init(LinphoneCore *lc) {
	LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	linphone_account_creator_service_set_constructor_cb(service, NULL);
	linphone_account_creator_service_set_destructor_cb(service, NULL);
	linphone_account_creator_service_set_is_account_exist_cb(service,
	                                                         linphone_account_creator_is_account_exist_flexiapi);
	linphone_account_creator_service_set_activate_account_cb(service,
	                                                         linphone_account_creator_activate_phone_account_flexiapi);
	linphone_account_creator_service_set_is_account_activated_cb(
	    service, linphone_account_creator_is_account_activated_flexiapi);
	linphone_account_creator_service_set_link_account_cb(
	    service, linphone_account_creator_link_phone_number_with_account_flexiapi);
	linphone_account_creator_service_set_activate_alias_cb(
	    service, linphone_account_creator_activate_phone_number_link_flexiapi);
	linphone_account_creator_service_set_is_account_linked_cb(service,
	                                                          linphone_account_creator_is_account_linked_flexiapi);
	linphone_account_creator_service_set_update_account_cb(service, linphone_account_creator_update_password_flexiapi);
	linphone_core_set_account_creator_service(lc, service);

	return linphone_account_creator_create(lc);
}

static void account_delete_on_api(LinphoneCore *core, string username, string password) {
	auto *params = linphone_core_create_account_params(core);
	LinphoneAddress *addr = linphone_factory_create_address(
	    linphone_factory_get(), string("sip:").append(username).append("@sip.example.org").c_str());
	linphone_account_params_set_identity_address(params, addr);

	auto *account = linphone_core_create_account(core, params);
	linphone_core_add_account(core, account);
	linphone_core_set_default_account(core, account);

	auto *authInfo =
	    linphone_factory_create_auth_info(linphone_factory_get(), username.c_str(), "", password.c_str(), "", "", "");
	linphone_core_add_auth_info(core, authInfo);

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(core);
	flexiAPIClient->accountDelete();

	linphone_address_unref(addr);
	linphone_account_unref(account);
	linphone_account_params_unref(params);
	linphone_auth_info_unref(authInfo);
}

static void account_creator_cb(LinphoneAccountCreator *creator,
                               LinphoneAccountCreatorStatus status,
                               BCTBX_UNUSED(const char *resp)) {
	LinphoneAccountCreatorCbs *cbs = linphone_account_creator_get_current_callbacks(creator);

	LinphoneAccountCreatorStatus expected_status = (LinphoneAccountCreatorStatus)((
	    intptr_t)linphone_account_creator_service_get_user_data(linphone_account_creator_get_service(creator)));
	BC_ASSERT_EQUAL(status, expected_status, LinphoneAccountCreatorStatus, "%i");
	account_creator_set_cb_done(cbs);
}

static void server_account_exist(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountExist);

	linphone_account_creator_set_username(creator, linphone_address_get_username(marie->identity));
	linphone_account_creator_set_domain(creator, linphone_address_get_domain(marie->identity));
	linphone_account_creator_cbs_set_is_account_exist(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_exist(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_linked(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotLinked);
	linphone_account_creator_set_username(creator, linphone_address_get_username(marie->identity));
	linphone_account_creator_cbs_set_is_account_linked(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_linked(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_activated(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotExist);
	linphone_account_creator_set_username(creator, linphone_address_get_username(marie->identity));
	linphone_account_creator_cbs_set_is_account_activated(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_activated_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_delete(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	// The following parameters are useless for the LinphonePrivate::FlexiAPI endpoint
	linphone_account_creator_set_username(creator, linphone_address_get_username(marie->identity));
	linphone_account_creator_set_email(creator, "user_2@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555455", "1");
	linphone_account_creator_cbs_set_delete_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_flexiapi(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_activate_email(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusMissingArguments);

	linphone_account_creator_set_username(creator, linphone_address_get_username(marie->identity));
	// Too short code
	linphone_account_creator_set_activation_code(creator, "123456789");
	linphone_account_creator_cbs_set_activate_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_email_account_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_send_token(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusUnexpectedError);

	linphone_account_creator_set_pn_param(creator, "123456789");
	linphone_account_creator_set_pn_provider(creator, "123456789");
	linphone_account_creator_set_pn_prid(creator, "123456789");
	linphone_account_creator_cbs_set_send_token(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_send_token_flexiapi(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");
	BC_ASSERT_EQUAL(linphone_account_creator_account_creation_request_token_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");
	wait_for_until(marie->lc, NULL, &stats->cb_done, 2, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_create_account_with_token(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusMissingArguments);

	linphone_account_creator_set_username(creator, "hop");
	linphone_account_creator_set_password(creator, "1234");
	linphone_account_creator_set_algorithm(creator, "MD5");
	linphone_account_creator_set_token(creator, "123456789");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account_with_token_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

/**
 * Dangerous endpoints tests
 */
static void server_account_created_with_email(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotCreated);

	string username = linphone_address_get_username(marie->identity);
	string password = "password";

	linphone_account_creator_set_username(creator, username.c_str());
	linphone_account_creator_set_email(creator, "username@linphone.org");
	linphone_account_creator_set_password(creator, password.c_str());
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account_flexiapi(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	account_delete_on_api(creator->core, username, password);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static string obtain_auth_token(LinphoneCoreManager *mgr) {
	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(mgr->lc);
	flexiAPIClient->useTestAdminAccount(true);

	int code = 0;
	int fetched = 0;
	string token;

	// Create the account
	flexiAPIClient->sendAccountCreationToken()->then(
	    [&code, &fetched, &token](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    fetched = 1;
		    token = response.json()["token"].asString();
	    });

	wait_for_until(mgr->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 201, int, "%d");
	return token;
}

static void server_account_created_with_phone(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountCreated);

	// Obtain auth token (normally it is sent via push notification.

	string authToken = obtain_auth_token(marie);

	// Create
	string phone = string("000").append(to_string(bctbx_random()).substr(0, 7));
	string password = "password";

	linphone_account_creator_set_phone_number(creator, phone.c_str(), "1");
	linphone_account_creator_set_email(creator, "username@linphone.org");
	linphone_account_creator_set_password(creator, password.c_str());
	linphone_account_creator_set_token(creator, authToken.c_str());
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account_flexiapi(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	// Check exists
	BC_ASSERT_EQUAL(linphone_account_creator_is_phone_number_used_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 2, TIMEOUT_REQUEST);

	// Start a recovery
	BC_ASSERT_EQUAL(linphone_account_creator_recover_phone_account_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 3, TIMEOUT_REQUEST);

	// Delete
	account_delete_on_api(creator->core, string("+1").append(phone), password);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

// Automatic account creation (Used by Linhome for automatic creation of push account set as depdent proxy config of non
// push enabled SIP accounts)
static void server_account_created_with_generated_username(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountCreated);

	// Obtain auth token (normally it is sent via push notification.

	string authToken = obtain_auth_token(marie);

	// Create
	linphone_account_creator_set_token(creator, authToken.c_str());
	linphone_account_creator_set_domain(creator, linphone_address_get_domain(marie->identity));
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_push_account_with_token_flexiapi(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	// Delete
	account_delete_on_api(creator->core, creator->username, creator->password);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

test_t account_creator_flexiapi_tests[] = {
    TEST_ONE_TAG("Server - Account exists", server_account_exist, "Server"),
    TEST_ONE_TAG("Server - Account activated", server_account_activated, "Server"),
    TEST_ONE_TAG("Server - Account linked", server_account_linked, "Server"),
    TEST_ONE_TAG("Server - Account delete", server_account_delete, "Server"),
    TEST_ONE_TAG("Server - Account activate email", server_account_activate_email, "Server"),
    TEST_ONE_TAG("Server - Account send token", server_account_send_token, "Server"),
    TEST_ONE_TAG("Server - Account create with token", server_account_create_account_with_token, "Server"),
    // Dangerous endpoints
    TEST_ONE_TAG("Server - Account created with email", server_account_created_with_email, "Server"),
    TEST_ONE_TAG("Server - Account created with phone", server_account_created_with_phone, "Server"),
    TEST_ONE_TAG("Server - Account created with generated random user/pass",
                 server_account_created_with_generated_username,
                 "Server"), // Push Account for Linhome
};

test_suite_t account_creator_flexiapi_test_suite = {"Account creator FlexiAPI",
                                                    NULL,
                                                    NULL,
                                                    liblinphone_tester_before_each,
                                                    liblinphone_tester_after_each,
                                                    sizeof(account_creator_flexiapi_tests) /
                                                        sizeof(account_creator_flexiapi_tests[0]),
                                                    account_creator_flexiapi_tests,
                                                    0};
