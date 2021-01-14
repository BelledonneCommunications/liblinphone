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

static void call_with_encryption_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption) {
	if (linphone_core_media_encryption_supported(caller->lc,encryption)) {
		BC_ASSERT_TRUE(call(caller, callee));
		end_call(callee, caller);

	} else {
		ms_warning ("not tested because srtp not available");
	}
}

static void call_with_mandatory_encryption_wrapper(const LinphoneMediaEncryption encryption) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *marie_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(marie_lp,"sip","enable_capability_negotiations",0);
	LpConfig *pauline_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(pauline_lp,"sip","enable_capability_negotiations",0);

	linphone_core_set_media_encryption_mandatory(marie->lc,TRUE);
	linphone_core_set_media_encryption(marie->lc,encryption);
	linphone_core_set_media_encryption_mandatory(pauline->lc,TRUE);
	linphone_core_set_media_encryption(pauline->lc,encryption);

	call_with_encryption_base(marie, pauline, encryption);
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 1000);
	call_with_encryption_base(pauline, marie, encryption);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_wrapper(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_wrapper(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_wrapper(LinphoneMediaEncryptionZRTP);
}

static void call_with_encryption_negotiation_failure_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption) {
	if (linphone_core_media_encryption_supported(callee->lc,encryption)) {

		const bctbx_list_t *initLogs = linphone_core_get_call_logs(callee->lc);
		int initLogsSize = (int)bctbx_list_size(initLogs);

		LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};

		callee_test_params.sdp_simulate_error = TRUE;
		BC_ASSERT_FALSE(call_with_params2(caller,callee,&caller_test_params, &callee_test_params, FALSE));

		BC_ASSERT_PTR_NULL(linphone_core_get_current_call(callee->lc));
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError,1, int, "%d");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallReleased,1, int, "%d");
		// actually callee does not receive error because it replies to the INVITE with a 488 Not Acceptable Here
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived,0, int, "%d");

		const bctbx_list_t *logs = linphone_core_get_call_logs(callee->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(logs), (initLogsSize+1), int, "%i");
		// Forward logs pointer to the element desired
		for (int i = 0; i < initLogsSize; i++) logs=logs->next;
		if (logs){
			const LinphoneErrorInfo *ei;
			LinphoneCallLog *cl = (LinphoneCallLog*)logs->data;
			BC_ASSERT_TRUE(linphone_call_log_get_start_date(cl) != 0);
			ei = linphone_call_log_get_error_info(cl);
			BC_ASSERT_PTR_NOT_NULL(ei);
			if (ei){
				BC_ASSERT_EQUAL(linphone_error_info_get_reason(ei), LinphoneReasonNotAcceptable, int, "%d");
			}
		}



		BC_ASSERT_EQUAL(linphone_core_get_calls_nb(caller->lc), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_calls_nb(callee->lc), 0, int, "%d");
	} else {
		ms_warning ("not tested because srtp not available");
	}
}

static void call_with_encryption_negotiation_failure_wrapper(const LinphoneMediaEncryption encryption) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *marie_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(marie_lp,"sip","enable_capability_negotiations",0);
	LpConfig *pauline_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(pauline_lp,"sip","enable_capability_negotiations",0);

	if (linphone_core_media_encryption_supported(pauline->lc,encryption)) {
		linphone_core_set_media_encryption_mandatory(pauline->lc,TRUE);
		linphone_core_set_media_encryption(pauline->lc,encryption);
		linphone_core_set_media_encryption_mandatory(marie->lc,FALSE);
		linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
	}

	call_with_encryption_negotiation_failure_base(marie, pauline, encryption);
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 1000);
	call_with_encryption_negotiation_failure_base(pauline, marie, encryption);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_encryption_negotiation_failure(void) {
	call_with_encryption_negotiation_failure_wrapper(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_encryption_negotiation_failure(void) {
	call_with_encryption_negotiation_failure_wrapper(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_encryption_negotiation_failure(void) {
	call_with_encryption_negotiation_failure_wrapper(LinphoneMediaEncryptionZRTP);
}

static void call_with_no_encryption(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *marie_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(marie_lp,"sip","enable_capability_negotiations",0);
	LpConfig *pauline_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(pauline_lp,"sip","enable_capability_negotiations",0);

	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
	linphone_core_set_media_encryption_mandatory(pauline->lc,FALSE);
	linphone_core_set_media_encryption(pauline->lc,encryption);
	linphone_core_set_media_encryption_mandatory(marie->lc,FALSE);
	linphone_core_set_media_encryption(marie->lc,encryption);

	call_with_encryption_base(marie, pauline, encryption);
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 1000);
	call_with_encryption_base(pauline, marie, encryption);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t capability_negotiation_tests[] = {
	TEST_NO_TAG("SRTP call with no encryption", call_with_no_encryption),
	TEST_NO_TAG("SRTP call with mandatory encryption", srtp_call_with_mandatory_encryption),
	TEST_NO_TAG("SRTP call with encryption negotiation failure", srtp_call_with_encryption_negotiation_failure),
	TEST_NO_TAG("DTLS call with mandatory encryption", dtls_call_with_mandatory_encryption),
	TEST_NO_TAG("DTLS call with encryption negotiation failure", dtls_call_with_encryption_negotiation_failure),
	TEST_NO_TAG("ZRTP call with mandatory encryption", zrtp_call_with_mandatory_encryption),
	TEST_NO_TAG("ZRTP call with encryption negotiation failure", zrtp_call_with_encryption_negotiation_failure)
};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(capability_negotiation_tests) / sizeof(capability_negotiation_tests[0]), capability_negotiation_tests};
