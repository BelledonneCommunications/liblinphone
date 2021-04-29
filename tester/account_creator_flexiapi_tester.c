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
#include <ctype.h>

static const int TIMEOUT_REQUEST = 3000;

static LinphoneAccountCreator* init(LinphoneCore *lc) {
	LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	linphone_account_creator_service_set_constructor_cb(service, NULL);
	linphone_account_creator_service_set_destructor_cb(service, NULL);
	linphone_account_creator_service_set_is_account_exist_cb(service, linphone_account_creator_is_account_exist_flexiapi);
	linphone_account_creator_service_set_activate_account_cb(service, linphone_account_creator_activate_phone_account_flexiapi);
	linphone_account_creator_service_set_is_account_activated_cb(service, linphone_account_creator_is_account_activated_flexiapi);
	linphone_account_creator_service_set_link_account_cb(service, linphone_account_creator_link_phone_number_with_account_flexiapi);
	linphone_account_creator_service_set_activate_alias_cb(service, linphone_account_creator_activate_phone_number_link_flexiapi);
	linphone_account_creator_service_set_is_account_linked_cb(service, linphone_account_creator_is_account_linked_flexiapi);
	linphone_account_creator_service_set_update_account_cb(service, linphone_account_creator_update_password_flexiapi);
	linphone_core_set_account_creator_service(lc, service);

	return linphone_account_creator_create(lc);
}

static void account_creator_cb(LinphoneAccountCreator *creator, LinphoneAccountCreatorStatus status, const char* resp) {
	LinphoneAccountCreatorCbs *cbs = linphone_account_creator_get_current_callbacks(creator);

	LinphoneAccountCreatorStatus expected_status = (LinphoneAccountCreatorStatus)(
		(intptr_t)linphone_account_creator_service_get_user_data(linphone_account_creator_get_service(creator))
	);
	BC_ASSERT_EQUAL(
		status,
		expected_status,
		LinphoneAccountCreatorStatus,
		"%i");
	account_creator_set_cb_done(cbs);
}

static void server_account_exist(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_flexiapi_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusAccountExist);

	linphone_account_creator_set_username(creator, linphone_address_get_username(marie->identity));
	linphone_account_creator_set_domain(creator, linphone_address_get_domain(marie->identity));
	linphone_account_creator_cbs_set_is_account_exist(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_is_account_exist(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

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
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusAccountLinked);
	linphone_account_creator_set_username(creator, "pauline");
	linphone_account_creator_cbs_set_is_account_linked(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_is_account_linked(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

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
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusAccountActivated);
	linphone_account_creator_set_username(creator, "pauline");
	linphone_account_creator_cbs_set_is_account_activated(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_is_account_activated_flexiapi(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

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
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusRequestOk);

	// The following parameters are useless for the FlexiAPI endpoint
	linphone_account_creator_set_username(creator, "pauline");
	linphone_account_creator_set_email(creator, "user_2@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555455","1");
	linphone_account_creator_cbs_set_delete_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_delete_account_flexiapi(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

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
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusMissingArguments);

	linphone_account_creator_set_username(creator, "pauline");
    // Too short code
	linphone_account_creator_set_activation_code(creator, "123456789");
	linphone_account_creator_cbs_set_activate_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_activate_email_account_flexiapi(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

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
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusUnexpectedError);

	linphone_account_creator_set_pn_param(creator, "123456789");
	linphone_account_creator_set_pn_provider(creator, "123456789");
	linphone_account_creator_set_pn_prid(creator, "123456789");
	linphone_account_creator_cbs_set_send_token(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_send_token_flexiapi(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

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
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusMissingArguments);

	linphone_account_creator_set_username(creator, "hop");
	linphone_account_creator_set_password(creator, "1234");
	linphone_account_creator_set_algorithm(creator, "MD5");
	linphone_account_creator_set_token(creator, "123456789");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(
		linphone_account_creator_create_account_with_token_flexiapi(creator),
		LinphoneAccountCreatorStatusRequestOk,
		LinphoneAccountCreatorStatus,
		"%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

test_t account_creator_flexiapi_tests[] = {
	TEST_ONE_TAG(
		"Server - Account exists",
		server_account_exist,
		"Server"),
	TEST_ONE_TAG(
		"Server - Account activated",
		server_account_activated,
		"Server"),
	TEST_ONE_TAG(
		"Server - Account linked",
		server_account_linked,
		"Server"),
	TEST_ONE_TAG(
		"Server - Account delete",
		server_account_delete,
		"Server"),
	TEST_ONE_TAG(
		"Server - Account activate email",
		server_account_activate_email,
		"Server"),
	TEST_ONE_TAG(
		"Server - Account send token",
		server_account_send_token,
		"Server"),
	TEST_ONE_TAG(
		"Server - Account create with token",
		server_account_create_account_with_token,
		"Server"),
};

test_suite_t account_creator_flexiapi_test_suite = {
	"Account creator FlexiAPI",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(account_creator_flexiapi_tests) / sizeof(account_creator_flexiapi_tests[0]),
	account_creator_flexiapi_tests};
