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

#include "linphone/utils/utils.h"

#include "liblinphone_tester.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void split () {
	string emptyString;
	vector<string> result = Utils::split(emptyString, ",");
	BC_ASSERT_EQUAL(result.size(), 1, size_t, "%zu");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "");
	string contentDisposition("positive-delivery, negative-delivery, display");
	result = Utils::split(contentDisposition, ", ");
	BC_ASSERT_EQUAL(result.size(), 3, size_t, "%zu");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "positive-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(1).c_str(), "negative-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(2).c_str(), "display");
	result = Utils::split(contentDisposition, ",");
	BC_ASSERT_EQUAL(result.size(), 3, size_t, "%zu");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "positive-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(1).c_str(), " negative-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(2).c_str(), " display");
	result = Utils::split(contentDisposition, ',');
	BC_ASSERT_EQUAL(result.size(), 3, size_t, "%zu");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "positive-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(1).c_str(), " negative-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(2).c_str(), " display");
	result = Utils::split(contentDisposition, "|");
	BC_ASSERT_EQUAL(result.size(), 1, size_t, "%zu");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), contentDisposition.c_str());
}

static void trim () {
	string emptyString;
	string result = Utils::trim(emptyString);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "");
	string stringWithLeadingSpace(" hello");
	result = Utils::trim(stringWithLeadingSpace);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello");
	string stringWithTailingSpace("hello ");
	result = Utils::trim(stringWithTailingSpace);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello");
	string stringWithSpaces("   hello  ");
	result = Utils::trim(stringWithSpaces);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello");
	string stringContainingSpaces(" hello world!    ");
	result = Utils::trim(stringContainingSpaces);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello world!");
}

test_t utils_tests[] = {
	TEST_NO_TAG("split", split),
	TEST_NO_TAG("trim", trim)
};

test_suite_t utils_test_suite = {
	"Utils", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(utils_tests) / sizeof(utils_tests[0]), utils_tests
};
