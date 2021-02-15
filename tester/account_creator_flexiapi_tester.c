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

static const char XMLRPC_URL[] = "http://subscribe.example.org:8082/flexisip-account-manager/xmlrpc.php";
static const int TIMEOUT_REQUEST = 10000;

static void init_linphone_account_creator_service(LinphoneCore *lc) {
	LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	linphone_account_creator_service_set_constructor_cb(service, NULL);
	linphone_account_creator_service_set_destructor_cb(service, NULL);
	//linphone_account_creator_service_set_create_account_cb(service, linphone_account_creator_create_account_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_exist_cb(service, linphone_account_creator_is_account_exist_linphone_flexiapi);
	linphone_account_creator_service_set_activate_account_cb(service, linphone_account_creator_activate_phone_account_linphone_flexiapi);
	linphone_account_creator_service_set_is_account_activated_cb(service, linphone_account_creator_is_account_activated_linphone_flexiapi);
	linphone_account_creator_service_set_link_account_cb(service, linphone_account_creator_link_phone_number_with_account_linphone_xmlrpc);
	linphone_account_creator_service_set_activate_alias_cb(service, linphone_account_creator_activate_phone_number_link_linphone_xmlrpc);
	linphone_account_creator_service_set_is_alias_used_cb(service, linphone_account_creator_is_phone_number_used_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_linked_cb(service, linphone_account_creator_is_account_linked_linphone_xmlrpc);
	linphone_account_creator_service_set_recover_account_cb(service, linphone_account_creator_recover_phone_account_linphone_xmlrpc);
	linphone_account_creator_service_set_update_account_cb(service, linphone_account_creator_update_password_linphone_xmlrpc);
	linphone_account_creator_service_set_login_linphone_account_cb(service, linphone_account_creator_login_linphone_account_linphone_xmlrpc);
	linphone_core_set_account_creator_service(lc, service);
}

static LinphoneAccountCreator * _linphone_account_creator_new(LinphoneCore *lc, const char * url) {
	init_linphone_account_creator_service(lc);
	LinphoneAccountCreator *creator = linphone_account_creator_new(lc, url);
	return creator;
}

static void account_creator_cb(LinphoneAccountCreator *creator, LinphoneAccountCreatorStatus status, const char* resp) {
	LinphoneAccountCreatorCbs *cbs = linphone_account_creator_get_callbacks(creator);
	LinphoneAccountCreatorStatus expected_status = (LinphoneAccountCreatorStatus)linphone_account_creator_service_get_user_data(
		linphone_account_creator_get_service(creator));
	BC_ASSERT_EQUAL(
		status,
		expected_status,
		LinphoneAccountCreatorStatus,
		"%i");
	account_creator_set_cb_done(cbs);
}

static void server_account_exist(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new2("account_creator_rc", 0);
	LinphoneAccountCreator *creator = _linphone_account_creator_new(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorCbs *cbs = linphone_account_creator_get_callbacks(creator);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(
		linphone_account_creator_get_service(creator),
		(void*)LinphoneAccountCreatorStatusAccountExist);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
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
}

test_t account_creator_flexiapi_tests[] = {
    TEST_ONE_TAG(
		"Server - Account exists",
		server_account_exist,
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
