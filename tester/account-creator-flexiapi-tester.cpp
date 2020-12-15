/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include "object/clonable-object-p.h"
#include "object/clonable-object.h"

#include "account_creator_flexiapi.hh"

#include "liblinphone_tester.h"
#include "tester_utils.h"

#include <jsoncpp/json/json.h>

using namespace Json;

static void flexiapi_ping() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	auto flexiAPIClient = new AccountCreatorFlexiAPI(marie->lc);

	const char *resolvedContent;
	int code = 0;
	int fetched = 0;

	flexiAPIClient
		->ping()
		->then([&resolvedContent, &code, &fetched](AccountCreatorFlexiAPI::Response response) -> void {
			resolvedContent = response.body;
			code = response.code;
			fetched = 1;
		});

	wait_for_until(marie->lc, NULL, &fetched, 1, 2000);

	BC_ASSERT_STRING_EQUAL(resolvedContent, "pong");
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	linphone_core_manager_destroy(marie);
	ms_free(flexiAPIClient);
}

static void flexiapi_accounts() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	auto flexiAPIClient = new AccountCreatorFlexiAPI(marie->lc);

	int code = 0;
	int fetched = 0;
	string resolvedDomain;

	// Unauthenticated
	flexiAPIClient
		->me()
		->error([&code, &fetched](AccountCreatorFlexiAPI::Response response) -> void {
			code = response.code;
			fetched = 1;
		});

	wait_for_until(marie->lc, NULL, &fetched, 1, 2000);
	BC_ASSERT_EQUAL(code, 401, int, "%d");

	code = 0;
	fetched = 0;

	// Authenticated
	flexiAPIClient->setApiKey("E74yK42XzarVILTTZUqHPrbHtNRESizfrrwe4RxU")
		->me()
		->then([&code, &fetched, &resolvedDomain](AccountCreatorFlexiAPI::Response response) -> void {
			code = response.code;
			resolvedDomain = response.json()["domain"].asString();
			fetched = 1;
		});

	wait_for_until(marie->lc, NULL, &fetched, 1, 2000);
	BC_ASSERT_EQUAL(code, 200, int, "%d");
	BC_ASSERT_STRING_EQUAL(resolvedDomain.c_str(), "sip.linphone.org");

	linphone_core_manager_destroy(marie);
	ms_free(flexiAPIClient);
}

test_t account_creator_flexiapi_tests[] = {
	TEST_NO_TAG("Ping", flexiapi_ping),
	TEST_NO_TAG("Accounts", flexiapi_accounts),
};

test_suite_t account_creator_flexiapi_suite = {
	"FlexiAPI Client", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(account_creator_flexiapi_tests) / sizeof(account_creator_flexiapi_tests[0]), account_creator_flexiapi_tests
};