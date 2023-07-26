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

#include <json/json.h>

#include "liblinphone_tester.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/flexi-api-client.h"
#include "object/clonable-object-p.h"
#include "object/clonable-object.h"
#include "tester_utils.h"

using namespace Json;

static void flexiapiPing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);

	string resolvedContent;
	int code = 0;
	int fetched = 0;

	flexiAPIClient->ping()->then(
	    [&resolvedContent, &code, &fetched](LinphonePrivate::FlexiAPIClient::Response response) {
		    resolvedContent = response.body;
		    code = response.code;
		    fetched = 1;
	    });

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);

	BC_ASSERT_STRING_EQUAL(resolvedContent.c_str(), "pong");
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	linphone_core_manager_destroy(marie);
}

static void flexiapiAccounts() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);

	int code = 0;
	int fetched = 0;
	string resolvedDomain;

	// Unauthenticated
	flexiAPIClient->me()->then([&code, &fetched](LinphonePrivate::FlexiAPIClient::Response response) {
		code = response.code;
		fetched = 1;
	});

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);

	// The internal resolver will handle a 401 and then try to re-send
	// the request with a proper DIGEST authentication
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	code = 0;
	fetched = 0;

	// Authenticated
	flexiAPIClient->me()->then([&code, &fetched, &resolvedDomain](LinphonePrivate::FlexiAPIClient::Response response) {
		code = response.code;
		resolvedDomain = response.json()["domain"].asString();
		fetched = 1;
	});

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");
	BC_ASSERT_STRING_EQUAL(resolvedDomain.c_str(), "sip.example.org");

	linphone_core_manager_destroy(marie);
}
#if 0 // echoue en permanence sur master du 1/10/2021
static void flexiapiChangeEmail() {
	LinphoneCoreManager *marie = linphone_core_manager_new("pauline_rc");

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);

	int code = 0;
	int fetched = 0;
	string resolvedDomain;

	flexiAPIClient
		->accountEmailChangeRequest("changed@test.com")
		->then([&code, &fetched](LinphonePrivate::FlexiAPIClient::Response response) {
			code = response.code;
			fetched = 1;
		});

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	linphone_core_manager_destroy(marie);
}
#endif
/**
 * This test is only passing if the setting "everyone_is_admin" is set to true
 * on the API
 */
static void flexiapiCreateAccount() {
	LinphoneCoreManager *marie = linphone_core_manager_new("pauline_rc");

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);
	flexiAPIClient->useTestAdminAccount(true);

	int code = 0;
	int fetched = 0;
	int id = 0;
	char *token = sal_get_random_token_lowercase(12);
	string username = string("test_").append(token);
	bctbx_free(token);
	string resolvedDomain;
	bool activated = true;

	// Create the account
	flexiAPIClient->adminAccountCreate(username, "test", "MD5", activated)
	    ->then([&code, &fetched, &id](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    fetched = 1;
		    id = response.json()["id"].asInt();
	    });

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	code = 0;
	fetched = 0;
	string resolvedUsername;
	bool resolvedActivated;

	// Request it
	flexiAPIClient->adminAccount(id)->then(
	    [&code, &fetched, &resolvedUsername, &resolvedActivated](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    fetched = 1;
		    resolvedUsername = response.json()["username"].asString();
		    resolvedActivated = response.json()["activated"].asBool();
	    });

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");
	BC_ASSERT_TRUE(resolvedActivated);
	BC_ASSERT_STRING_EQUAL(resolvedUsername.c_str(), username.c_str());

	code = 0;
	fetched = 0;

	// Destroy it
	flexiAPIClient->adminAccountDelete(id)->then([&code, &fetched](LinphonePrivate::FlexiAPIClient::Response response) {
		code = response.code;
		fetched = 1;
	});

	wait_for_until(marie->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	linphone_core_manager_destroy(marie);
}

static void flexiapiChangePassword() {
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");

	// Resolve the password
	LinphoneAddress *identityAddress = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneAuthInfo *authInfo =
	    (LinphoneAuthInfo *)linphone_core_find_auth_info(pauline->lc, linphone_address_get_domain(identityAddress),
	                                                     linphone_address_get_username(identityAddress), NULL);

	const char *password = linphone_auth_info_get_password(authInfo);
	BC_ASSERT_PTR_NOT_NULL(password);

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(pauline->lc);

	int code = 0;
	int fetched = 0;
	string resolvedDomain;

	flexiAPIClient->accountPasswordChange("MD5", "new_password", password)
	    ->then([&code, &fetched](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    fetched = 1;
	    });

	wait_for_until(pauline->lc, NULL, &fetched, 1, 10000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	linphone_address_unref(identityAddress);
	linphone_core_manager_destroy(pauline);
}

test_t flexiapiclient_tests[] = {TEST_NO_TAG("Ping", flexiapiPing),
                                 TEST_NO_TAG("Create Account", flexiapiCreateAccount),
                                 TEST_NO_TAG("Accounts", flexiapiAccounts),
#if 0 // echoue en permanence sur master du 1/10/2021
	TEST_NO_TAG("Change Email", flexiapiChangeEmail),
#endif
                                 TEST_NO_TAG("Change Password", flexiapiChangePassword)};

test_suite_t flexiapiclient_suite = {"FlexiAPI Client",
                                     NULL,
                                     NULL,
                                     liblinphone_tester_before_each,
                                     liblinphone_tester_after_each,
                                     sizeof(flexiapiclient_tests) / sizeof(flexiapiclient_tests[0]),
                                     flexiapiclient_tests,
                                     0};
