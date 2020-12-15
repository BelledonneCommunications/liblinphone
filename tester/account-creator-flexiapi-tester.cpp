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

static void flexiapi_ping() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	auto flexiAPIClient = new AccountCreatorFlexiAPI(marie->lc);

	const char *resolvedContent;

	flexiAPIClient->ping([&resolvedContent](auto body) -> void {
		resolvedContent = body;
	});

	wait_for_until(marie->lc, NULL, NULL, 1, 5000);

	BC_ASSERT_STRING_EQUAL(resolvedContent, "pong");

	/*LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	linphone_account_creator_service_set_is_account_exist_cb(service, account_creator_flexiapi_ping);

	linphone_core_set_account_creator_service(marie->lc, service);

	LinphoneAccountCreator *creator = linphone_account_creator_new(marie->lc, "");
	linphone_account_creator_set_username(creator, "user_exist");

	if (linphone_account_creator_is_account_exist(creator)) {
		ms_message("Error with the request is_account_exist");
	}

	wait_for_until(marie->lc, NULL, NULL, 1, 5000);

	linphone_account_creator_unref(creator);*/
	linphone_core_manager_destroy(marie);
	ms_free(flexiAPIClient);
}

test_t account_creator_flexiapi_tests[] = {
	TEST_NO_TAG("Ping", flexiapi_ping),
};

test_suite_t account_creator_flexiapi_suite = {
	"FlexiAPI Client", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(account_creator_flexiapi_tests) / sizeof(account_creator_flexiapi_tests[0]), account_creator_flexiapi_tests
};