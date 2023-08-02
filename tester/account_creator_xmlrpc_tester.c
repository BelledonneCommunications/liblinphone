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

#include <bctoolbox/defs.h>

#include "liblinphone_tester.h"
#include "tester_utils.h"

static const char XMLRPC_URL[] = "http://subscribe.example.org:8082/flexisip-account-manager/xmlrpc.php";
static const int TIMEOUT_REQUEST = 10000;

static LinphoneAccountCreator *init(LinphoneCore *lc, const char *url) {
	LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	linphone_account_creator_service_set_constructor_cb(service, NULL);
	linphone_account_creator_service_set_destructor_cb(service, NULL);
	linphone_account_creator_service_set_create_account_cb(service,
	                                                       linphone_account_creator_create_account_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_exist_cb(service,
	                                                         linphone_account_creator_is_account_exist_linphone_xmlrpc);
	linphone_account_creator_service_set_activate_account_cb(
	    service, linphone_account_creator_activate_phone_account_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_activated_cb(
	    service, linphone_account_creator_is_account_activated_linphone_xmlrpc);
	linphone_account_creator_service_set_link_account_cb(
	    service, linphone_account_creator_link_phone_number_with_account_linphone_xmlrpc);
	linphone_account_creator_service_set_activate_alias_cb(
	    service, linphone_account_creator_activate_phone_number_link_linphone_xmlrpc);
	linphone_account_creator_service_set_is_alias_used_cb(
	    service, linphone_account_creator_is_phone_number_used_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_linked_cb(
	    service, linphone_account_creator_is_account_linked_linphone_xmlrpc);
	linphone_account_creator_service_set_recover_account_cb(
	    service, linphone_account_creator_recover_phone_account_linphone_xmlrpc);
	linphone_account_creator_service_set_update_account_cb(service,
	                                                       linphone_account_creator_update_password_linphone_xmlrpc);
	linphone_account_creator_service_set_login_linphone_account_cb(
	    service, linphone_account_creator_login_linphone_account_linphone_xmlrpc);
	linphone_core_set_account_creator_service(lc, service);

	return linphone_account_creator_new(lc, url);
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

static void get_activation_code(LinphoneAccountCreator *creator, int *cb_done) {
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_get_confirmation_key_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(creator->core, NULL, cb_done, 1, TIMEOUT_REQUEST);
}

static void server_delete_account_test(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_email(creator, "user_2@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555455", "1");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	// First attempt with the first password
	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "password");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	// Second attempt with the second password
	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "newpassword");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_username(creator, "xxxtestuser_3");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "password");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	// Fourth attempt with the password and sha256
	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_username(creator, "xxxtestuser_0");
	linphone_account_creator_set_email(creator, "user_0@linphone.org");
	linphone_account_creator_set_password(creator, "newpassword");
	linphone_account_creator_set_algorithm(creator, "SHA-256");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	// Another attempt with previous password if previous suite crashed before the update
	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_username(creator, "xxxtestuser_0");
	linphone_account_creator_set_email(creator, "user_0@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_algorithm(creator, "SHA-256");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	// fifth attempt with the second password
	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_set_email(creator, "user_5@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555450", "1");
	linphone_account_creator_set_algorithm(creator, "SHA-256");

	BC_ASSERT_EQUAL(linphone_account_creator_delete_account_linphone_xmlrpc(creator),
	                LinphoneAccountCreatorStatusRequestOk, LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

/****************** Start Is Account Exist ************************/
static void server_account_doesnt_exist(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_cbs_set_is_account_exist(cbs, account_creator_cb);
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotExist);
	linphone_account_creator_set_username(creator, "user_not_exist");

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_exist(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_exist(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_cbs_set_is_account_exist(cbs, account_creator_cb);
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountExist);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_exist(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

/****************** End Is Account Exist ************************/

/****************** Start Create Account ************************/
static void server_account_created_with_email(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountCreated);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "password");

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_add_callbacks(creator, cbs);

	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountCreated);
	linphone_account_creator_set_username(creator, "xxxtestuser_0");
	linphone_account_creator_set_email(creator, "user_0@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_algorithm(creator, "SHA-256");

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_create_account_already_create_with_email(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountExist);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "password");

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_created_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	linphone_account_creator_cbs_set_user_data(cbs, stats);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountCreated);
	linphone_account_creator_set_email(creator, "user_2@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555455", "1");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountCreated);
	linphone_account_creator_set_email(creator, "user_5@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555450", "1");
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);
	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_create_account_already_create_as_account_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	linphone_account_creator_cbs_set_user_data(cbs, stats);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountExist);
	linphone_account_creator_set_email(creator, "user_2@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555455", "1");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_create_account_already_create_as_alias_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	linphone_account_creator_cbs_set_user_data(cbs, stats);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountExistWithAlias);
	linphone_account_creator_set_username(creator, "xxxtestuser_3");
	linphone_account_creator_set_email(creator, "user_2@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_phone_number(creator, "000555456", "1");
	linphone_account_creator_cbs_set_create_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_create_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Create Account ************************/

/****************** Start Is Account Activated ************************/
static void server_account_not_activated(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotActivated);
	linphone_account_creator_cbs_set_is_account_activated(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_activated(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_already_activated(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountActivated);
	linphone_account_creator_cbs_set_is_account_activated(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_activated(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Is Account Activated ************************/

/****************** Start Activate Account ************************/
static void server_activate_account_not_activated(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	get_activation_code(creator, &stats->cb_done);
	account_creator_reset_cb_done(bctbx_list_get_data(creator->callbacks));

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountActivated);
	linphone_account_creator_cbs_set_activate_account(cbs, account_creator_cb);

	linphone_account_creator_service_set_activate_account_cb(
	    linphone_account_creator_get_service(creator), linphone_account_creator_activate_email_account_linphone_xmlrpc);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);
	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_set_username(creator, "xxxtestuser_0");
	linphone_account_creator_set_email(creator, "user_0@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	get_activation_code(creator, &stats->cb_done);
	account_creator_reset_cb_done(cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountActivated);
	linphone_account_creator_cbs_set_activate_account(cbs, account_creator_cb);

	linphone_account_creator_service_set_activate_account_cb(
	    linphone_account_creator_get_service(creator), linphone_account_creator_activate_email_account_linphone_xmlrpc);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_activate_account_already_activated(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	linphone_account_creator_set_password(creator, "password");
	get_activation_code(creator, &stats->cb_done);
	account_creator_reset_cb_done(cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountAlreadyActivated);
	linphone_account_creator_cbs_set_activate_account(cbs, account_creator_cb);
	linphone_account_creator_service_set_activate_account_cb(
	    linphone_account_creator_get_service(creator), linphone_account_creator_activate_email_account_linphone_xmlrpc);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_activate_non_existent_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "unknown_user");
	linphone_account_creator_set_activation_code(creator, "58c9");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotExist);
	linphone_account_creator_cbs_set_activate_account(cbs, account_creator_cb);
	linphone_account_creator_service_set_activate_account_cb(
	    linphone_account_creator_get_service(creator), linphone_account_creator_activate_email_account_linphone_xmlrpc);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Activate Account ************************/

/****************** Start Link Account ************************/
static void server_link_account_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_phone_number(creator, "000555456", "1");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_cbs_set_link_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_link_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_link_non_existent_account_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "unknown_user");
	linphone_account_creator_set_phone_number(creator, "012345678", "33");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotLinked);
	linphone_account_creator_cbs_set_link_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_link_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Link Account ************************/

/****************** Start Activate Alias ************************/
static void server_activate_phone_number_for_non_existent_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "unknown_user");
	linphone_account_creator_set_phone_number(creator, "012345678", "33");
	linphone_account_creator_set_activation_code(creator, "12345679");
	linphone_account_creator_set_password(creator, "password");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotActivated);
	linphone_account_creator_cbs_set_activate_alias(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_alias(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_activate_phone_number_for_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_phone_number(creator, "000555456", "1");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_email(creator, "user_1@linphone.org");
	get_activation_code(creator, &stats->cb_done);
	account_creator_reset_cb_done(cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountActivated);
	linphone_account_creator_cbs_set_activate_alias(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_alias(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_set_username(creator, "xxxtestuser_0");
	linphone_account_creator_set_phone_number(creator, "000555458", "1");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_email(creator, "user_0@linphone.org");
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	get_activation_code(creator, &stats->cb_done);
	account_creator_reset_cb_done(cbs);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountActivated);
	linphone_account_creator_cbs_set_activate_alias(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_activate_alias(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Activate Alias ************************/

/****************** Start Is Alias Used ************************/
static void server_phone_number_is_used_as_alias(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_phone_number(creator, "000555456", "1");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAliasExist);
	linphone_account_creator_cbs_set_is_alias_used(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_alias_used(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_phone_number_is_used_as_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_phone_number(creator, "000555455", "1");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAliasIsAccount);
	linphone_account_creator_cbs_set_is_alias_used(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_alias_used(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_phone_number_not_used(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_phone_number(creator, "012345678", "33");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAliasNotExist);
	linphone_account_creator_cbs_set_is_alias_used(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_alias_used(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Is Alias Used ************************/

/****************** Start Is Account Linked ************************/
static void server_account_link_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountLinked);
	linphone_account_creator_cbs_set_is_account_linked(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_linked(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_account_not_link_with_phone_number(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotLinked);
	linphone_account_creator_cbs_set_is_account_linked(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_is_account_linked(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Is Account Linked ************************/

/****************** Start Recover Account ************************/
static void server_recover_account_with_phone_number_used(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_phone_number(creator, "000555456", "1");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_cbs_set_recover_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_recover_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_recover_account_with_phone_number_not_used(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_phone_number(creator, "012345678", "33");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotExist);
	linphone_account_creator_cbs_set_recover_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_recover_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}
/****************** End Recover Account ************************/

/****************** Start Update Account ************************/
static void server_update_account_password_with_wrong_password(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_password(creator, "pssword");
	linphone_account_creator_set_user_data(creator, "newpassword");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotExist);
	linphone_account_creator_cbs_set_update_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_update_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_update_account_password_with_correct_password(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_1");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_user_data(creator, "newpassword");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_cbs_set_update_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_update_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "xxxtestuser_0");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_user_data(creator, "newpassword");
	linphone_account_creator_set_algorithm(creator, "SHA-256");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	account_creator_reset_cb_done(cbs);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);
	linphone_account_creator_cbs_set_update_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_update_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_update_account_password_for_non_existent_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "unknown_user");
	linphone_account_creator_set_password(creator, "password");
	linphone_account_creator_set_user_data(creator, "newpassword");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusServerError);
	linphone_account_creator_cbs_set_update_account(cbs, account_creator_cb);

	BC_ASSERT_EQUAL(linphone_account_creator_update_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void login_linphone_account_creator_cb(LinphoneAccountCreator *creator,
                                              LinphoneAccountCreatorStatus status,
                                              BCTBX_UNUSED(const char *resp)) {
	LinphoneAccountCreatorCbs *cbs = linphone_account_creator_get_current_callbacks(creator);
	LinphoneAccountCreatorStatus expected_status = (LinphoneAccountCreatorStatus)((
	    intptr_t)linphone_account_creator_service_get_user_data(linphone_account_creator_get_service(creator)));
	BC_ASSERT_EQUAL(status, expected_status, LinphoneAccountCreatorStatus, "%i");

	if (expected_status == LinphoneAccountCreatorStatusRequestOk) {
		const char *expected_password =
		    (const char *)belle_sip_object_data_get((belle_sip_object_t *)creator, "expected_ha1");
		const char *expected_algorithm =
		    (const char *)belle_sip_object_data_get((belle_sip_object_t *)creator, "expected_algorithm");
		const char *response_password = linphone_account_creator_get_ha1(creator);
		const char *response_algorithm = linphone_account_creator_get_algorithm(creator);
		BC_ASSERT_STRING_EQUAL(response_password, expected_password);
		BC_ASSERT_STRING_EQUAL(response_algorithm, expected_algorithm);
	}
	account_creator_set_cb_done(cbs);
}

static void server_recover_phone_account_doesnt_exists(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "user_inexistant");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "SHA-256");

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusAccountNotExist);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

static void server_recover_phone_account_exists(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc, XMLRPC_URL);
	LinphoneAccountCreatorStats *stats = new_linphone_account_creator_stats();

	LinphoneAccountCreatorCbs *cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);

	linphone_account_creator_set_username(creator, "user_md5_only");
	linphone_account_creator_set_domain(creator, "sip.example.org");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "MD5");
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_ha1", ms_strdup("secret_md5"), ms_free);
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_algorithm", ms_strdup("MD5"), ms_free);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	stats = new_linphone_account_creator_stats();

	linphone_account_creator_set_username(creator, "user_sha256_only");
	linphone_account_creator_set_domain(creator, "sip.example.org");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_ha1", ms_strdup("secret_sha256"), ms_free);
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_algorithm", ms_strdup("SHA-256"), ms_free);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	stats = new_linphone_account_creator_stats();

	linphone_account_creator_set_username(creator, "user_sha256_only");
	linphone_account_creator_set_domain(creator, "sip.example.org");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "MD5");
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_ha1", ms_strdup("secret_sha256"), ms_free);
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_algorithm", ms_strdup("SHA-256"), ms_free);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	stats = new_linphone_account_creator_stats();

	linphone_account_creator_set_username(creator, "user_md5_only");
	linphone_account_creator_set_domain(creator, "sip.example.org");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_ha1", ms_strdup("secret_md5"), ms_free);
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_algorithm", ms_strdup("MD5"), ms_free);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	stats = new_linphone_account_creator_stats();

	linphone_account_creator_set_username(creator, "user_both_md5_sha256");
	linphone_account_creator_set_domain(creator, "sip.example.org");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "MD5");
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_ha1", ms_strdup("secret_md5"), ms_free);
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_algorithm", ms_strdup("MD5"), ms_free);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_account_creator_cbs_unref(cbs);

	creator = init(marie->lc, XMLRPC_URL);
	cbs = linphone_factory_create_account_creator_cbs(linphone_factory_get());
	linphone_account_creator_add_callbacks(creator, cbs);
	stats = new_linphone_account_creator_stats();

	linphone_account_creator_set_username(creator, "user_both_md5_sha256");
	linphone_account_creator_set_domain(creator, "sip.example.org");
	linphone_account_creator_set_activation_code(creator, "1666");
	linphone_account_creator_set_algorithm(creator, "SHA-256");
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_ha1", ms_strdup("secret_sha256"), ms_free);
	belle_sip_object_data_set((belle_sip_object_t *)creator, "expected_algorithm", ms_strdup("SHA-256"), ms_free);

	linphone_account_creator_cbs_set_user_data(cbs, stats);
	linphone_account_creator_cbs_set_login_linphone_account(cbs, login_linphone_account_creator_cb);
	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator),
	                                               (void *)LinphoneAccountCreatorStatusRequestOk);

	BC_ASSERT_EQUAL(linphone_account_creator_login_linphone_account(creator), LinphoneAccountCreatorStatusRequestOk,
	                LinphoneAccountCreatorStatus, "%i");

	wait_for_until(marie->lc, NULL, &stats->cb_done, 1, TIMEOUT_REQUEST);

	ms_free(stats);
	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
	linphone_account_creator_cbs_unref(cbs);
}

/****************** End Update Account ************************/

test_t account_creator_tests[] = {
    TEST_ONE_TAG("Server - Delete accounts test", server_delete_account_test, "Server"),
    TEST_ONE_TAG("Server - Account doesn\'t exist", server_account_doesnt_exist, "Server"),
    TEST_ONE_TAG("Server - Activate a non existent account", server_activate_non_existent_account, "Server"),
    TEST_ONE_TAG("Server - Activate phone number for a non existent account",
                 server_activate_phone_number_for_non_existent_account,
                 "Server"),
    TEST_ONE_TAG("Server - Phone number not used", server_phone_number_not_used, "Server"),
    TEST_ONE_TAG("Server - Update account password for a non existent account",
                 server_update_account_password_for_non_existent_account,
                 "Server"),
    TEST_ONE_TAG("Server - Recover account with phone number not used",
                 server_recover_account_with_phone_number_not_used,
                 "Server"),
    TEST_ONE_TAG("Server - Link a non existent account with phone number",
                 server_link_non_existent_account_with_phone_number,
                 "Server"),
    TEST_ONE_TAG("Server - Account created with email", server_account_created_with_email, "Server"),
    TEST_ONE_TAG("Server - Account exist", server_account_exist, "Server"),
    TEST_ONE_TAG(
        "Server - Account already create with email", server_create_account_already_create_with_email, "Server"),
    TEST_ONE_TAG("Server - Account created with phone number", server_account_created_with_phone_number, "Server"),
    TEST_ONE_TAG("Server - Account not activated", server_account_not_activated, "Server"),
    TEST_ONE_TAG("Server - Account already created with phone number as account",
                 server_create_account_already_create_as_account_with_phone_number,
                 "Server"),
    TEST_ONE_TAG("Server - Phone number is used as account", server_phone_number_is_used_as_account, "Server"),
    TEST_ONE_TAG("Server - Account not link with phone number", server_account_not_link_with_phone_number, "Server"),
    TEST_ONE_TAG("Server - Activate account", server_activate_account_not_activated, "Server"),
    TEST_ONE_TAG("Server - Account already activated", server_account_already_activated, "Server"),
    TEST_ONE_TAG("Server - Activate account already activated", server_activate_account_already_activated, "Server"),
    TEST_ONE_TAG("Server - Link account with phone number", server_link_account_with_phone_number, "Server"),
    TEST_ONE_TAG("Server - Activate phone number for an account", server_activate_phone_number_for_account, "Server"),
    TEST_ONE_TAG("Server - Account already created with phone number as alias",
                 server_create_account_already_create_as_alias_with_phone_number,
                 "Server"),
    TEST_ONE_TAG("Server - Phone number is used as alias", server_phone_number_is_used_as_alias, "Server"),
    TEST_ONE_TAG("Server - Account link with phone number", server_account_link_with_phone_number, "Server"),
    TEST_ONE_TAG("Server - Update account password with wrong password",
                 server_update_account_password_with_wrong_password,
                 "Server"),
    TEST_ONE_TAG("Server - Update account password with correct password",
                 server_update_account_password_with_correct_password,
                 "Server"),
    TEST_ONE_TAG(
        "Server - Recover account with phone number used", server_recover_account_with_phone_number_used, "Server"),
    TEST_ONE_TAG("Server - Recover account password and algorithm from confirmation key when doesnt exists",
                 server_recover_phone_account_doesnt_exists,
                 "Server"),
    TEST_ONE_TAG("Server - Recover account password and algorithm from confirmation key",
                 server_recover_phone_account_exists,
                 "Server"),
};


test_suite_t account_creator_xmlrpc_test_suite = {"Account creator XMLRPC",
                                                  NULL,
                                                  NULL,
                                                  liblinphone_tester_before_each,
                                                  liblinphone_tester_after_each,
                                                  sizeof(account_creator_tests) / sizeof(account_creator_tests[0]),
                                                  account_creator_tests,
                                                  0};
