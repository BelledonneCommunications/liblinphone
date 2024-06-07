/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "liblinphone_tester.h"
#include "linphone/api/c-account-manager-services-request-cbs.h"
#include "linphone/api/c-account-manager-services-request.h"
#include "linphone/api/c-account-manager-services.h"
#include "linphone/api/c-auth-info.h"
#include "tester_utils.h"
#include <ctype.h>

typedef struct _LinphoneAccountManagerServicesStats {
	int request_result_received;
	int success_request_count;
	int error_request_count;
	int parameters_error_count;
	char *data;
	size_t devices_count;
	LinphoneAccountDevice *device;
} LinphoneAccountManagerServicesStats;

static const int TIMEOUT_REQUEST = 3000;

// =============================================================================

static void account_manager_services_request_on_success(const LinphoneAccountManagerServicesRequest *request,
                                                        const char *data) {
	LinphoneAccountManagerServicesRequestType type = linphone_account_manager_services_request_get_type(request);
	ms_message("Request %i is successful, data is %s", type, data);
	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_account_manager_services_request_get_current_callbacks(request);
	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)linphone_account_manager_services_request_cbs_get_user_data(cbs);

	if (stats->data) {
		ms_free(stats->data);
		stats->data = NULL;
	}
	if (data) {
		stats->data = ms_strdup(data);
	}

	stats->success_request_count += 1;
	stats->request_result_received += 1;
}

static void account_manager_services_request_on_error(const LinphoneAccountManagerServicesRequest *request,
                                                      int status_code,
                                                      const char *error_message,
                                                      const LinphoneDictionary *parameter_errors) {
	LinphoneAccountManagerServicesRequestType type = linphone_account_manager_services_request_get_type(request);
	ms_message("Request %i is in error, status code is %i and error message %s", type, status_code, error_message);

	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_account_manager_services_request_get_current_callbacks(request);
	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)linphone_account_manager_services_request_cbs_get_user_data(cbs);

	if (parameter_errors != NULL) {
		bctbx_list_t *keys = linphone_dictionary_get_keys(parameter_errors);
		bctbx_list_t *it = keys;
		for (; it != NULL; it = bctbx_list_next(it)) {
			const char *key = (const char *)bctbx_list_get_data(it);
			const char *parameter = linphone_dictionary_get_string(parameter_errors, key);
			ms_message("Request %i parameter %s is in error %s", type, parameter, error_message);

			if (type == LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken) {
				BC_ASSERT_STRING_EQUAL(key, "account_creation_request_token");
			}
			stats->parameters_error_count += 1;
		}
		bctbx_list_free_with_data(keys, ms_free);
	}

	stats->error_request_count += 1;
	stats->request_result_received += 1;
}

// =============================================================================

static void desktop_request_account_creation_request_token_fail_use(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("account_manager_services_rc", FALSE);
	LinphoneAccountManagerServices *ams = linphone_core_create_account_manager_services(manager->lc);
	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_factory_create_account_manager_services_request_cbs(linphone_factory_get());

	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)ms_new0(LinphoneAccountManagerServicesStats, 1);
	linphone_account_manager_services_request_cbs_set_user_data(cbs, stats);

	linphone_account_manager_services_request_cbs_set_request_successful(cbs,
	                                                                     account_manager_services_request_on_success);
	linphone_account_manager_services_request_cbs_set_request_error(cbs, account_manager_services_request_on_error);

	// Request an account creation request token
	LinphoneAccountManagerServicesRequest *request =
	    linphone_account_manager_services_create_get_account_creation_request_token_request(ams);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, 1, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, 1, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// For the token to be valid, a captcha must be validated at the URL given in parameter of success callback, which
	// won't be the case here.
	request =
	    linphone_account_manager_services_create_get_account_creation_token_from_request_token_request(ams, "1234");
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, 2, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, 1, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 1, int, "%d");
	BC_ASSERT_EQUAL(stats->parameters_error_count, 1, int, "%d");
	linphone_account_manager_services_request_unref(request);

	if (stats->data) {
		ms_free(stats->data);
	}
	ms_free(stats);
	linphone_account_manager_services_request_cbs_unref(cbs);
	linphone_account_manager_services_unref(ams);
	linphone_core_manager_destroy(manager);
}

static void desktop_create_account_active_using_email_code(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("account_manager_services_rc", FALSE);
	LinphoneAccountManagerServices *ams = linphone_core_create_account_manager_services(manager->lc);
	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_factory_create_account_manager_services_request_cbs(linphone_factory_get());

	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)ms_new0(LinphoneAccountManagerServicesStats, 1);
	linphone_account_manager_services_request_cbs_set_user_data(cbs, stats);

	linphone_account_manager_services_request_cbs_set_request_successful(cbs,
	                                                                     account_manager_services_request_on_success);
	linphone_account_manager_services_request_cbs_set_request_error(cbs, account_manager_services_request_on_error);

	int request_count = 1;

	// Step 1: get an account creation token
	LinphoneAccountManagerServicesRequest *request =
	    linphone_account_manager_services_create_get_account_creation_token_as_admin_request(ams);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Step 2: create the account
	char random_id[9];
	const char *symbols = "0123456789";
	belle_sip_random_token_with_charset(random_id, sizeof random_id, symbols, strlen(symbols));
	char *username = bctbx_strdup_printf("desktop_test_%s", random_id);
	const char *password = "not_really_secret";
	const char *algorithm = "SHA-256";

	BC_ASSERT_PTR_NOT_NULL(stats->data);
	const char *token = stats->data;
	ms_message("Token is %s", token);

	request_count += 1;
	request = linphone_account_manager_services_create_new_account_using_token_request(ams, username, password,
	                                                                                   algorithm, token);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	int account_id = -1;
	BC_ASSERT_PTR_NOT_NULL(stats->data);
	if (stats->data) {
		account_id = atoi(stats->data);
		ms_message("Account id is %i", account_id);
	}

	// Create auth info with credentials
	LinphoneAuthInfo *ai = linphone_core_create_auth_info(manager->lc, username, NULL, password, NULL, NULL, NULL);
	linphone_core_add_auth_info(manager->lc, ai);

	// Step 3.1: link the account to an email address to activate it
	char *email_address = bctbx_strdup_printf("%s@example.org", username);
	char *sip_address = bctbx_strdup_printf("%s@sip.example.org", username);
	LinphoneAddress *sip_identity = linphone_core_interpret_url(manager->lc, sip_address);

	request_count += 1;
	request = linphone_account_manager_services_create_send_email_linking_code_by_email_request(ams, sip_identity,
	                                                                                            email_address);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Step 3.2: get code sent by email
	request_count += 1;
	request = linphone_account_manager_services_create_get_account_info_as_admin_request(ams, account_id);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Step 3.3: validate email link using code
	BC_ASSERT_PTR_NOT_NULL(stats->data);
	const char *code = stats->data;
	ms_message("Code sent by email is %s", code);

	request_count += 1;
	request =
	    linphone_account_manager_services_create_link_email_to_account_using_code_request(ams, sip_identity, code);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Final step: delete created account
	request_count += 1;
	request = linphone_account_manager_services_create_delete_account_as_admin_request(ams, account_id);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	linphone_auth_info_unref(ai);
	if (stats->data) {
		ms_free(stats->data);
	}
	ms_free(stats);
	ms_free(username);
	ms_free(email_address);
	ms_free(sip_address);
	linphone_address_unref(sip_identity);
	linphone_account_manager_services_request_cbs_unref(cbs);
	linphone_account_manager_services_unref(ams);
	linphone_core_manager_destroy(manager);
}

// =============================================================================

static void mobile_request_account_creation_request_token_by_push(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("account_manager_services_rc", FALSE);
	LinphoneAccountManagerServices *ams = linphone_core_create_account_manager_services(manager->lc);
	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_factory_create_account_manager_services_request_cbs(linphone_factory_get());

	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)ms_new0(LinphoneAccountManagerServicesStats, 1);
	linphone_account_manager_services_request_cbs_set_user_data(cbs, stats);

	linphone_account_manager_services_request_cbs_set_request_successful(cbs,
	                                                                     account_manager_services_request_on_success);
	linphone_account_manager_services_request_cbs_set_request_error(cbs, account_manager_services_request_on_error);

	// Request an account creation token to be sent by push
	LinphoneAccountManagerServicesRequest *request =
	    linphone_account_manager_services_create_send_account_creation_token_by_push_request(ams, "liblinphone_tester",
	                                                                                         "param", "prid");
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	// It will fail because of tester env without real push support
	wait_for_until(manager->lc, NULL, &stats->request_result_received, 1, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, 0, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 1, int, "%d");
	BC_ASSERT_EQUAL(stats->parameters_error_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	if (stats->data) {
		ms_free(stats->data);
	}
	ms_free(stats);
	linphone_account_manager_services_request_cbs_unref(cbs);
	linphone_account_manager_services_unref(ams);
	linphone_core_manager_destroy(manager);
}

static void mobile_create_account_active_using_sms_code(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("account_manager_services_rc", FALSE);
	LinphoneAccountManagerServices *ams = linphone_core_create_account_manager_services(manager->lc);
	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_factory_create_account_manager_services_request_cbs(linphone_factory_get());

	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)ms_new0(LinphoneAccountManagerServicesStats, 1);
	linphone_account_manager_services_request_cbs_set_user_data(cbs, stats);

	linphone_account_manager_services_request_cbs_set_request_successful(cbs,
	                                                                     account_manager_services_request_on_success);
	linphone_account_manager_services_request_cbs_set_request_error(cbs, account_manager_services_request_on_error);

	int request_count = 1;

	// Step 1: get an account creation token
	LinphoneAccountManagerServicesRequest *request =
	    linphone_account_manager_services_create_get_account_creation_token_as_admin_request(ams);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Step 2: create the account
	char random_id[9];
	const char *symbols = "0123456789";
	belle_sip_random_token_with_charset(random_id, sizeof random_id, symbols, strlen(symbols));
	char *username = bctbx_strdup_printf("mobile_test_%s", random_id);
	const char *password = "not_really_secret";
	const char *algorithm = "SHA-256";

	BC_ASSERT_PTR_NOT_NULL(stats->data);
	const char *token = stats->data;
	ms_message("Token is %s", token);

	request_count += 1;
	request = linphone_account_manager_services_create_new_account_using_token_request(ams, username, password,
	                                                                                   algorithm, token);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	int account_id = -1;
	BC_ASSERT_PTR_NOT_NULL(stats->data);
	if (stats->data) {
		account_id = atoi(stats->data);
		ms_message("Account id is %i", account_id);
	}

	// Create auth info with credentials
	LinphoneAuthInfo *ai = linphone_core_create_auth_info(manager->lc, username, NULL, password, NULL, NULL, NULL);
	linphone_core_add_auth_info(manager->lc, ai);

	// Step 3.1: link the account to a phone number to activate it
	char *phone_number = bctbx_strdup_printf("+336%s", random_id);
	char *sip_address = bctbx_strdup_printf("%s@sip.example.org", username);
	LinphoneAddress *sip_identity = linphone_core_interpret_url(manager->lc, sip_address);

	request_count += 1;
	request = linphone_account_manager_services_create_send_phone_number_linking_code_by_sms_request(ams, sip_identity,
	                                                                                                 phone_number);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Step 3.2: get code sent by SMS
	request_count += 1;
	request = linphone_account_manager_services_create_get_account_info_as_admin_request(ams, account_id);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Step 3.3: validate phone number link using code
	BC_ASSERT_PTR_NOT_NULL(stats->data);
	const char *code = stats->data;
	ms_message("Code sent by SMS is %s", code);

	request_count += 1;
	request = linphone_account_manager_services_create_link_phone_number_to_account_using_code_request(
	    ams, sip_identity, code);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	// Final step: delete created account
	request_count += 1;
	request = linphone_account_manager_services_create_delete_account_as_admin_request(ams, account_id);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	linphone_account_manager_services_request_unref(request);

	linphone_auth_info_unref(ai);
	if (stats->data) {
		ms_free(stats->data);
	}
	ms_free(stats);
	ms_free(username);
	ms_free(phone_number);
	ms_free(sip_address);
	linphone_address_unref(sip_identity);
	linphone_account_manager_services_request_cbs_unref(cbs);
	linphone_account_manager_services_unref(ams);
	linphone_core_manager_destroy(manager);
}

// =============================================================================

static void
account_manager_services_request_on_devices_list_fetched(const LinphoneAccountManagerServicesRequest *request,
                                                         const bctbx_list_t *devices_list) {
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeGetDevicesList;
	size_t count = bctbx_list_size(devices_list);
	ms_message("Request %i devices fetched, contains %zu devices", type, count);

	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_account_manager_services_request_get_current_callbacks(request);
	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)linphone_account_manager_services_request_cbs_get_user_data(cbs);
	stats->devices_count = count;

	if (stats->device) {
		linphone_account_device_unref(stats->device);
		stats->device = NULL;
	}

	if (count > 0) {
		stats->device = linphone_account_device_ref((LinphoneAccountDevice *)bctbx_list_get_data(devices_list));
	}
}

static void account_devices_list(bool_t with_user_agent) {
	time_t before = ms_time(NULL);
	LinphoneCoreManager *manager = linphone_core_manager_create("account_creator_flexiapi_rc");
	LinphoneAccountManagerServices *ams = linphone_core_create_account_manager_services(manager->lc);
	LinphoneAccountManagerServicesRequestCbs *cbs =
	    linphone_factory_create_account_manager_services_request_cbs(linphone_factory_get());

	LinphoneAccountManagerServicesStats *stats =
	    (LinphoneAccountManagerServicesStats *)ms_new0(LinphoneAccountManagerServicesStats, 1);
	linphone_account_manager_services_request_cbs_set_user_data(cbs, stats);

	linphone_account_manager_services_request_cbs_set_request_successful(cbs,
	                                                                     account_manager_services_request_on_success);
	linphone_account_manager_services_request_cbs_set_request_error(cbs, account_manager_services_request_on_error);
	linphone_account_manager_services_request_cbs_set_devices_list_fetched(
	    cbs, account_manager_services_request_on_devices_list_fetched);

	if (with_user_agent) {
		linphone_core_set_user_agent(manager->lc, "blabla (Pauline device) blibli/blublu (bloblo)", NULL);
	}

	linphone_core_manager_start(manager, TRUE);

	int request_count = 1;

	LinphoneAddress *identity = manager->identity;
	LinphoneAccountManagerServicesRequest *request =
	    linphone_account_manager_services_create_get_devices_list_request(ams, identity);
	BC_ASSERT_PTR_NOT_NULL(request);
	linphone_account_manager_services_request_add_callbacks(request, cbs);
	linphone_account_manager_services_request_submit(request);

	wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
	BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
	BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
	BC_ASSERT_EQUAL(stats->devices_count, 1, size_t, "%zu");
	linphone_account_manager_services_request_unref(request);

	if (stats->devices_count > 0) {
		BC_ASSERT_PTR_NOT_NULL(stats->device);
		if (stats->device) {
			if (with_user_agent) {
				BC_ASSERT_STRING_EQUAL(linphone_account_device_get_name(stats->device), "Pauline device");
			} else {
				BC_ASSERT_PTR_NULL(linphone_account_device_get_name(stats->device));
			}

			const char *uuid = linphone_config_get_string(linphone_core_get_config(manager->lc), "misc", "uuid", "");
			char *full_uuid = bctbx_strdup_printf("urn:uuid:%s", uuid);
			BC_ASSERT_STRING_EQUAL(linphone_account_device_get_uuid(stats->device), full_uuid);
			ms_free(full_uuid);

			time_t last_update = linphone_account_device_get_last_update_timestamp(stats->device);
			time_t after = ms_time(NULL);
			BC_ASSERT_GREATER((unsigned)last_update, (unsigned)before, unsigned, "%u");
			BC_ASSERT_LOWER((unsigned)last_update, (unsigned)after, unsigned, "%u");

			// Now let's delete the device
			request_count += 1;
			request = linphone_account_manager_services_create_delete_device_request(ams, identity, stats->device);
			BC_ASSERT_PTR_NOT_NULL(request);
			linphone_account_manager_services_request_add_callbacks(request, cbs);
			linphone_account_manager_services_request_submit(request);

			wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
			BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
			BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
			linphone_account_manager_services_request_unref(request);

			// Then query list again
			request_count += 1;
			request = linphone_account_manager_services_create_get_devices_list_request(ams, identity);
			BC_ASSERT_PTR_NOT_NULL(request);
			linphone_account_manager_services_request_add_callbacks(request, cbs);
			linphone_account_manager_services_request_submit(request);

			wait_for_until(manager->lc, NULL, &stats->request_result_received, request_count, TIMEOUT_REQUEST);
			BC_ASSERT_EQUAL(stats->success_request_count, request_count, int, "%d");
			BC_ASSERT_EQUAL(stats->error_request_count, 0, int, "%d");
			BC_ASSERT_EQUAL(stats->devices_count, 0, size_t, "%zu");
			linphone_account_manager_services_request_unref(request);
		}
	}

	if (stats->device) {
		linphone_account_device_unref(stats->device);
	}
	if (stats->data) {
		ms_free(stats->data);
	}
	ms_free(stats);
	linphone_account_manager_services_request_cbs_unref(cbs);
	linphone_account_manager_services_unref(ams);
	linphone_core_manager_destroy(manager);
}

static void account_devices_list_with_user_agent(void) {
	account_devices_list(TRUE);
}

static void account_devices_list_without_user_agent(void) {
	account_devices_list(FALSE);
}

// =============================================================================

test_t account_manager_services_tests[] = {
    TEST_NO_TAG("Desktop - Request account creation request token fail use",
                desktop_request_account_creation_request_token_fail_use),
    TEST_NO_TAG("Desktop - Create account & activate using email address",
                desktop_create_account_active_using_email_code),
    TEST_NO_TAG("Mobile - Request account creation token by push",
                mobile_request_account_creation_request_token_by_push),
    TEST_NO_TAG("Mobile - Create account & activate using phone number", mobile_create_account_active_using_sms_code),
    TEST_NO_TAG("Devices list with user-agent", account_devices_list_with_user_agent),
    TEST_NO_TAG("Devices list without user-agent", account_devices_list_without_user_agent),
};

test_suite_t account_manager_services_test_suite = {"Account Manager Services",
                                                    NULL,
                                                    NULL,
                                                    liblinphone_tester_before_each,
                                                    liblinphone_tester_after_each,
                                                    sizeof(account_manager_services_tests) /
                                                        sizeof(account_manager_services_tests[0]),
                                                    account_manager_services_tests,
                                                    0};
