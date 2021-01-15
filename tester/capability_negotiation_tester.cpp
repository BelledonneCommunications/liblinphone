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
#include <list>

#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

enum encryption_level {
	E_DISABLED,
	E_OPTIONAL,
	E_MANDATORY
};

struct encryption_params {
	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
	encryption_level level = E_DISABLED;
	std::list<LinphoneMediaEncryption> preferences;
};

static void set_encryption_preference(std::list<LinphoneMediaEncryption> & preferences, const bool_t encryption_preferred) {
	preferences.push_back(LinphoneMediaEncryptionSRTP); /**< Use SRTP media encryption */
	preferences.push_back(LinphoneMediaEncryptionZRTP); /**< Use ZRTP media encryption */
	preferences.push_back(LinphoneMediaEncryptionDTLS); /**< Use DTLS media encryption */
	if (encryption_preferred) {
		preferences.push_front(LinphoneMediaEncryptionNone); /**< No media encryption is used */
	} else {
		preferences.push_back(LinphoneMediaEncryptionNone); /**< No media encryption is used */
	}
}

static void set_encryption_preference_except(std::list<LinphoneMediaEncryption> & preferences, const LinphoneMediaEncryption encryption) {
	if (encryption != LinphoneMediaEncryptionSRTP) {
		preferences.push_back(LinphoneMediaEncryptionSRTP); /**< Use SRTP media encryption */
	}
	if (encryption != LinphoneMediaEncryptionZRTP) {
		preferences.push_back(LinphoneMediaEncryptionZRTP); /**< Use ZRTP media encryption */
	}
	if (encryption != LinphoneMediaEncryptionDTLS) {
		preferences.push_back(LinphoneMediaEncryptionDTLS); /**< Use DTLS media encryption */
	}
	if (encryption != LinphoneMediaEncryptionNone) {
		preferences.push_back(LinphoneMediaEncryptionNone); /**< No media encryption is used */
	}
}

static void call_with_encryption_negotiation_failure_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption) {
	if (linphone_core_media_encryption_supported(callee->lc,encryption)) {

		const bctbx_list_t *initLogs = linphone_core_get_call_logs(callee->lc);
		int initLogsSize = (int)bctbx_list_size(initLogs);
		stats initial_callee=callee->stat;

		LinphoneCall *caller_call=linphone_core_invite_address(caller->lc,callee->identity);
		BC_ASSERT_PTR_NOT_NULL(caller_call);
		if (caller_call) {
			BC_ASSERT_PTR_NULL(linphone_call_get_remote_params(caller_call)); /*assert that remote params are NULL when no response is received yet*/
		}
		//test ios simulator needs more time, 3s plus for connectng the network
		BC_ASSERT_FALSE(wait_for_until(callee->lc
					,caller->lc
					,&callee->stat.number_of_LinphoneCallIncomingReceived
					,initial_callee.number_of_LinphoneCallIncomingReceived+1, 12000));

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
	LpConfig *pauline_lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_int(pauline_lp,"sip","enable_capability_negotiations",0);

	linphone_core_set_media_encryption_mandatory(marie->lc,FALSE);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
	if (linphone_core_media_encryption_supported(pauline->lc,encryption)) {
		linphone_core_set_media_encryption_mandatory(pauline->lc,TRUE);
		linphone_core_set_media_encryption(pauline->lc,encryption);
	}

	ms_message("Core with no encryption calls core with mandatory encryption");
	call_with_encryption_negotiation_failure_base(marie, pauline, encryption);
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 1000);
	ms_message("Core with mandatory encryption calls core with no encryption");
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

static void call_with_encryption_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption) {
	if (linphone_core_media_encryption_supported(caller->lc,encryption)) {
		BC_ASSERT_TRUE(call(caller, callee));
		end_call(callee, caller);

	} else {
		ms_warning ("not tested because srtp not available");
	}
}

static void call_with_encryption_wrapper(const encryption_params marie_enc_params, const bool_t enable_marie_capability_negotiations, const encryption_params pauline_enc_params, const bool_t enable_pauline_capability_negotiations) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *marie_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(marie_lp,"sip","enable_capability_negotiations",enable_marie_capability_negotiations);
	LpConfig *pauline_lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_int(pauline_lp,"sip","enable_capability_negotiations",enable_pauline_capability_negotiations);

	const LinphoneMediaEncryption pauline_encryption = pauline_enc_params.encryption;
	if (linphone_core_media_encryption_supported(pauline->lc,pauline_encryption)) {
		linphone_core_set_media_encryption_mandatory(pauline->lc,(pauline_enc_params.level == E_MANDATORY));
		linphone_core_set_media_encryption(pauline->lc,pauline_encryption);
	}
	const LinphoneMediaEncryption marie_encryption = marie_enc_params.encryption;
	if (linphone_core_media_encryption_supported(marie->lc,marie_encryption)) {
		linphone_core_set_media_encryption_mandatory(marie->lc,(marie_enc_params.level == E_MANDATORY));
		linphone_core_set_media_encryption(marie->lc,marie_encryption);
	}

	call_with_encryption_base(marie, pauline, marie_encryption);
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 1000);
	call_with_encryption_base(pauline, marie, pauline_encryption);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_encryption(void) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_params.level = E_DISABLED;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_params.level = E_DISABLED;
	call_with_encryption_wrapper(marie_enc_params, FALSE, pauline_enc_params, FALSE);
}

static void call_with_mandatory_encryption_wrapper(const LinphoneMediaEncryption encryption) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_MANDATORY;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_MANDATORY;
	call_with_encryption_wrapper(marie_enc_params, FALSE, pauline_enc_params, FALSE);
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

static void call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_wrapper(const LinphoneMediaEncryption encryption) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_params.level = E_OPTIONAL;
	set_encryption_preference(marie_enc_params.preferences, TRUE);

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_MANDATORY;
	call_with_encryption_wrapper(marie_enc_params, FALSE, pauline_enc_params, FALSE);
}

static void srtp_call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_side(void) {
	call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_wrapper(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_side(void) {
	call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_wrapper(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_side(void) {
	call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_wrapper(LinphoneMediaEncryptionZRTP);
}

static void call_with_optional_encryption_on_one_side_and_none_on_the_other_wrapper(const LinphoneMediaEncryption encryption) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_params.level = E_DISABLED;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_MANDATORY;
	call_with_encryption_wrapper(marie_enc_params, FALSE, pauline_enc_params, FALSE);
}

static void srtp_call_with_optional_encryption_on_one_side_and_none_on_the_other_side(void) {
	call_with_optional_encryption_on_one_side_and_none_on_the_other_wrapper(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_optional_encryption_on_one_side_and_none_on_the_other_side(void) {
	call_with_optional_encryption_on_one_side_and_none_on_the_other_wrapper(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_optional_encryption_on_one_side_and_none_on_the_other_side(void) {
	call_with_optional_encryption_on_one_side_and_none_on_the_other_wrapper(LinphoneMediaEncryptionZRTP);
}

static void call_with_optional_encryption_on_both_sides_wrapper(const LinphoneMediaEncryption encryption) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_OPTIONAL;
	set_encryption_preference_except(marie_enc_params.preferences, encryption);

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_OPTIONAL;
	set_encryption_preference_except(pauline_enc_params.preferences, encryption);

	call_with_encryption_wrapper(marie_enc_params, FALSE, pauline_enc_params, FALSE);
}

static void srtp_call_with_optional_encryption_on_both_sides_side(void) {
	call_with_optional_encryption_on_both_sides_wrapper(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_optional_encryption_on_both_sides_side(void) {
	call_with_optional_encryption_on_both_sides_wrapper(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_optional_encryption_on_both_sides_side(void) {
	call_with_optional_encryption_on_both_sides_wrapper(LinphoneMediaEncryptionZRTP);
}
test_t capability_negotiation_tests[] = {
	TEST_NO_TAG("SRTP call with no encryption", call_with_no_encryption),
	TEST_NO_TAG("SRTP call with mandatory encryption", srtp_call_with_mandatory_encryption),
	TEST_NO_TAG("SRTP call with encryption negotiation failure", srtp_call_with_encryption_negotiation_failure),
	TEST_NO_TAG("SRTP call with optional encryption on one side and mandatory on the other side", srtp_call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_side),
	TEST_NO_TAG("SRTP call with optional encryption on one side and none on the other side", srtp_call_with_optional_encryption_on_one_side_and_none_on_the_other_side),
	TEST_NO_TAG("SRTP call with optional encryption on both sides", srtp_call_with_optional_encryption_on_both_sides_side),
	TEST_NO_TAG("DTLS call with mandatory encryption", dtls_call_with_mandatory_encryption),
	TEST_NO_TAG("DTLS call with encryption negotiation failure", dtls_call_with_encryption_negotiation_failure),
	TEST_NO_TAG("DTLS call with optional encryption on one side and mandatory on the other side", dtls_call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_side),
	TEST_NO_TAG("DTLS call with optional encryption on one side and none on the other side", dtls_call_with_optional_encryption_on_one_side_and_none_on_the_other_side),
	TEST_NO_TAG("DTLS call with optional encryption on both sides", dtls_call_with_optional_encryption_on_both_sides_side),
	TEST_NO_TAG("ZRTP call with mandatory encryption", zrtp_call_with_mandatory_encryption),
	TEST_NO_TAG("ZRTP call with encryption negotiation failure", zrtp_call_with_encryption_negotiation_failure),
	TEST_NO_TAG("ZRTP call with optional encryption on one side and mandatory on the other side", zrtp_call_with_optional_encryption_on_one_side_and_mandatory_on_the_other_side),
	TEST_NO_TAG("ZRTP call with optional encryption on one side and none on the other side", zrtp_call_with_optional_encryption_on_one_side_and_none_on_the_other_side),
	TEST_NO_TAG("ZRTP call with optional encryption on both sides", zrtp_call_with_optional_encryption_on_both_sides_side)
};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(capability_negotiation_tests) / sizeof(capability_negotiation_tests[0]), capability_negotiation_tests};
