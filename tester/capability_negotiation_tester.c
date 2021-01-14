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
#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

static void call_with_encryption_negotiation_failure(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(pauline->lc,LinphoneMediaEncryptionSRTP)) {
		LpConfig *marie_lp = linphone_core_get_config(marie->lc);
		linphone_config_set_int(marie_lp,"sip","enable_capability_negotiations",1);
		LpConfig *pauline_lp = linphone_core_get_config(marie->lc);
		linphone_config_set_int(pauline_lp,"sip","enable_capability_negotiations",1);
		linphone_core_set_media_encryption_mandatory(pauline->lc,TRUE);
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption_mandatory(marie->lc,FALSE);
		linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);

		LinphoneCallTestParams marie_test_params = {0}, pauline_test_params = {0};

		pauline_test_params.sdp_simulate_error = TRUE;
		BC_ASSERT_FALSE(call_with_params2(marie,pauline,&marie_test_params, &pauline_test_params, FALSE));

		BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallError,1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallReleased,1, int, "%d");
		// actually pauline does not receive error because it replies to the INVITE with a 488 Not Acceptable Here
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallError,0, int, "%d");

		BC_ASSERT_EQUAL(linphone_core_get_calls_nb(marie->lc), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_calls_nb(pauline->lc), 0, int, "%d");
	} else {
		ms_warning ("not tested because srtp not available");
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


test_t capability_negotiation_tests[] = {
	TEST_NO_TAG("SRTP call with encryption negotiation failure", call_with_encryption_negotiation_failure)
};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(capability_negotiation_tests) / sizeof(capability_negotiation_tests[0]), capability_negotiation_tests};
