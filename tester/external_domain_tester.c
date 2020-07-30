/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

static void simple_call(void) {
	simple_call_base_with_rcs("claire_sips_rc", "pauline_sips_rc", FALSE, FALSE, FALSE);
};

test_t external_domain_tests[] = {
	TEST_NO_TAG("Simple call", simple_call)
};

test_suite_t external_domain_test_suite = {"External domain", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(external_domain_tests) / sizeof(external_domain_tests[0]), external_domain_tests};
