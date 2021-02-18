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
#include <string>

#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

enum encryption_level {
	E_DISABLED,
	E_OPTIONAL,
	E_MANDATORY
};

struct encryption_params {
	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone; // Desired encryption
	encryption_level level = E_DISABLED;
	std::list<LinphoneMediaEncryption> preferences;
};

static std::list<LinphoneMediaEncryption> set_encryption_preference(const bool_t encryption_preferred) {
	std::list<LinphoneMediaEncryption> preferences;
	for (int idx = 0; idx <= LinphoneMediaEncryptionDTLS; idx++) {
		LinphoneMediaEncryption candidateEncryption = static_cast<LinphoneMediaEncryption>(idx);
		if (candidateEncryption != LinphoneMediaEncryptionNone) {
			preferences.push_back(candidateEncryption);
		}
	}

	if (encryption_preferred) {
		preferences.push_back(LinphoneMediaEncryptionNone); /**< No media encryption is used */
	} else {
		preferences.push_front(LinphoneMediaEncryptionNone); /**< No media encryption is used */
	}
	return preferences;
}

static std::list<LinphoneMediaEncryption> set_encryption_preference_with_priority(const LinphoneMediaEncryption encryption, const bool append) {
	std::list<LinphoneMediaEncryption> preferences;
	for (int idx = 0; idx <= LinphoneMediaEncryptionDTLS; idx++) {
		LinphoneMediaEncryption candidateEncryption = static_cast<LinphoneMediaEncryption>(idx);
		if (candidateEncryption != encryption) {
			if (candidateEncryption == LinphoneMediaEncryptionNone) {
				// No encryption should be added last
				preferences.push_back(candidateEncryption);
			} else {
				preferences.push_front(candidateEncryption);
			}
		}
	}
	if (append) {
		preferences.push_back(encryption);
	} else {
		preferences.push_front(encryption);
	}
	return preferences;
}

static bctbx_list_t * create_confg_encryption_preference_list_except(const LinphoneMediaEncryption encryption) {
	bctbx_list_t * encryption_list = NULL;
	for (int idx = 0; idx <= LinphoneMediaEncryptionDTLS; idx++) {
		if (static_cast<LinphoneMediaEncryption>(idx) != encryption) {
			encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(idx))));
		}
	}
	return encryption_list;
}

static bctbx_list_t * create_confg_encryption_preference_list_from_param_preferences(const std::list<LinphoneMediaEncryption> & preferences) {
	bctbx_list_t * encryption_list = NULL;
	for (const auto & enc : preferences) {
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(enc)));
	}
	return encryption_list;
}

static void call_with_encryption_negotiation_failure_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee) {

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
}

static void call_from_enc_to_different_enc_base(const LinphoneMediaEncryption mandatory_encryption, const LinphoneMediaEncryption non_mandatory_encryption, const bool_t enable_mandatory_enc_mgr_capability_negotiations, const bool_t enable_non_mandatory_enc_mgr_capability_negotiations, bool_t mandatory_to_non_mandatory) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, enable_non_mandatory_enc_mgr_capability_negotiations);
	linphone_core_set_media_encryption_mandatory(marie->lc,FALSE);
	linphone_core_set_media_encryption(marie->lc,non_mandatory_encryption);
	bctbx_list_t * cfg_enc = create_confg_encryption_preference_list_except(mandatory_encryption);
	linphone_core_set_supported_media_encryptions(marie->lc,cfg_enc);
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(marie->lc, mandatory_encryption));
	bctbx_list_free_with_data(cfg_enc, (bctbx_list_free_func)bctbx_free);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, enable_mandatory_enc_mgr_capability_negotiations);
	if (linphone_core_media_encryption_supported(pauline->lc,mandatory_encryption)) {
		linphone_core_set_media_encryption_mandatory(pauline->lc,TRUE);
		linphone_core_set_media_encryption(pauline->lc,mandatory_encryption);
	}
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, mandatory_encryption));

	if (mandatory_to_non_mandatory) {
		ms_message("Core with mandatory encryption calls core with non mandatory encryption");
		call_with_encryption_negotiation_failure_base(pauline, marie);
	} else {
		ms_message("Core with non mandatory encryption calls core with mandatory encryption");
		call_with_encryption_negotiation_failure_base(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_from_enc_to_no_enc_base(const LinphoneMediaEncryption encryption, const bool_t enable_mandatory_enc_mgr_capability_negotiations, const bool_t enable_non_mandatory_enc_mgr_capability_negotiations, bool_t mandatory_to_non_mandatory) {
	call_from_enc_to_different_enc_base(encryption, LinphoneMediaEncryptionNone, enable_mandatory_enc_mgr_capability_negotiations, enable_non_mandatory_enc_mgr_capability_negotiations, mandatory_to_non_mandatory);
}

static void call_from_enc_to_dtls_enc_base(const LinphoneMediaEncryption encryption, const bool_t enable_mandatory_enc_mgr_capability_negotiations, const bool_t enable_non_mandatory_enc_mgr_capability_negotiations, bool_t mandatory_to_non_mandatory) {
	call_from_enc_to_different_enc_base(encryption, LinphoneMediaEncryptionDTLS, enable_mandatory_enc_mgr_capability_negotiations, enable_non_mandatory_enc_mgr_capability_negotiations, mandatory_to_non_mandatory);
}
static void srtp_call_from_enc_to_no_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE, TRUE);
}

static void dtls_call_from_enc_to_no_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE);
}

static void zrtp_call_from_enc_to_dtls_enc(void) {
	call_from_enc_to_dtls_enc_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE, TRUE);
}

static void srtp_call_from_no_enc_to_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE, FALSE);
}

static void dtls_call_from_no_enc_to_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, FALSE);
}

static void zrtp_call_from_dtls_enc_to_enc(void) {
	call_from_enc_to_dtls_enc_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE, FALSE);
}

static void call_with_encryption_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption, const bool_t enable_caller_capability_negotiations, const bool_t enable_callee_capability_negotiations) {

	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations (caller_params, enable_caller_capability_negotiations);
	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations (callee_params, enable_callee_capability_negotiations);

	bool_t caller_enc_mandatory = linphone_core_is_media_encryption_mandatory(caller->lc);
	bool_t callee_enc_mandatory = linphone_core_is_media_encryption_mandatory(callee->lc);
	const LinphoneMediaEncryption caller_encryption = linphone_core_get_media_encryption(caller->lc);
	const LinphoneMediaEncryption callee_encryption = linphone_core_get_media_encryption(callee->lc);
	if (caller_enc_mandatory && callee_enc_mandatory && (caller_encryption != callee_encryption)) {
		BC_ASSERT_FALSE(call_with_params(caller, callee, caller_params, callee_params));
	} else {
		BC_ASSERT_TRUE(call_with_params(caller, callee, caller_params, callee_params));

		LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
		BC_ASSERT_PTR_NOT_NULL(callerCall);
		LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
		BC_ASSERT_PTR_NOT_NULL(calleeCall);

		// Find expected call encryption as well as if a reinvite following capability negotiation is required
		LinphoneMediaEncryption expectedEncryption =  LinphoneMediaEncryptionNone;
		bool potentialConfigurationChosen = false;
		if (caller_enc_mandatory) {
			expectedEncryption = caller_encryption;
			// reINVITE is not sent because the call should not offer potential configurations as it must enforce an encryption that will be stored in the actual configuration
			potentialConfigurationChosen = false;
		} else if (callee_enc_mandatory) {
			expectedEncryption = callee_encryption;

			
			// reINVITE is only sent if caller and callee support capability negotiations enabled and the expected encryption is listed in one potential configuration offered by the caller
			potentialConfigurationChosen = (enable_callee_capability_negotiations && enable_caller_capability_negotiations && linphone_core_is_media_encryption_supported(caller->lc,expectedEncryption));
		} else if (enable_callee_capability_negotiations && enable_caller_capability_negotiations && (linphone_call_params_is_media_encryption_supported (linphone_call_get_params(callerCall), encryption)) && (linphone_call_params_is_media_encryption_supported (linphone_call_get_params(calleeCall), encryption))) {
			expectedEncryption = encryption;
			// reINVITE is always sent
			potentialConfigurationChosen = linphone_call_params_is_media_encryption_supported (linphone_call_get_params(callerCall), encryption) && (linphone_core_get_media_encryption(caller->lc) != encryption);
		} else {
			expectedEncryption = linphone_core_get_media_encryption(caller->lc);
			// reINVITE is not sent because either parts of the call doesn't support capability negotiations
			potentialConfigurationChosen = false;
		}

		liblinphone_tester_check_rtcp(caller, callee);

		const int expectedStreamsRunning = 1 + ((potentialConfigurationChosen) ? 1 : 0);
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");

		if (callerCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(callerCall)), expectedEncryption, int, "%i");
		}
		if (calleeCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(calleeCall)), expectedEncryption, int, "%i");
		}

		end_call(callee, caller);

	}

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);

}

static void call_with_capability_negotiation_failure(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 1);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 1);

	const LinphoneMediaEncryption paulineOptionalEncryption = LinphoneMediaEncryptionSRTP;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc,paulineOptionalEncryption));
	if (linphone_core_media_encryption_supported(pauline->lc,paulineOptionalEncryption)) {
		bctbx_list_t * encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(paulineOptionalEncryption))));
		linphone_core_set_media_encryption_mandatory(pauline->lc,0);
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(pauline->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
		if (encryption_list) {
			bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
		}
	}

	const LinphoneMediaEncryption marieOptionalEncryption = LinphoneMediaEncryptionDTLS;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc,marieOptionalEncryption));
	if (linphone_core_media_encryption_supported(marie->lc,marieOptionalEncryption)) {
		bctbx_list_t * encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(marieOptionalEncryption))));
		linphone_core_set_media_encryption_mandatory(marie->lc,0);
		linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(marie->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption));
		if (encryption_list) {
			bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
		}
	}

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption));

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption));

	call_with_encryption_base(marie, pauline, marieOptionalEncryption, TRUE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_capability_negotiation_failure_multiple_potential_configurations (void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 1);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 1);

	const LinphoneMediaEncryption paulineOptionalEncryption = LinphoneMediaEncryptionSRTP;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc,paulineOptionalEncryption));
	if (linphone_core_media_encryption_supported(pauline->lc,paulineOptionalEncryption)) {
		bctbx_list_t * encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(paulineOptionalEncryption))));
		linphone_core_set_media_encryption_mandatory(pauline->lc,0);
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(pauline->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
		if (encryption_list) {
			bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
		}
	}

	bctbx_list_t * encryption_list = NULL;
	const LinphoneMediaEncryption marieOptionalEncryption0 = LinphoneMediaEncryptionDTLS;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc,marieOptionalEncryption0));
	if (linphone_core_media_encryption_supported(marie->lc,marieOptionalEncryption0)) {
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(marieOptionalEncryption0)));
	}

	const LinphoneMediaEncryption marieOptionalEncryption1 = LinphoneMediaEncryptionZRTP;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc,marieOptionalEncryption1));
	if (linphone_core_media_encryption_supported(marie->lc,marieOptionalEncryption1)) {
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(marieOptionalEncryption1)));
	}

	linphone_core_set_media_encryption_mandatory(marie->lc,0);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(marie->lc,encryption_list);
	if (encryption_list) {
		bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
	}

	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(marie->lc, paulineOptionalEncryption));
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption0));
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption1));

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption0));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption1));

	call_with_encryption_base(marie, pauline, marieOptionalEncryption0, TRUE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_capability_negotiation_disable_call_level(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 1);
	linphone_core_set_media_encryption_mandatory(marie->lc,0);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
	bctbx_list_t * cfg_enc = create_confg_encryption_preference_list_except(LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(marie->lc,cfg_enc);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 1);
	linphone_core_set_media_encryption_mandatory(pauline->lc,0);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(pauline->lc,cfg_enc);
	bctbx_list_free_with_data(cfg_enc, (bctbx_list_free_func)bctbx_free);

	call_with_encryption_base(marie, pauline, LinphoneMediaEncryptionNone, FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_capability_negotiation_disable_core_level(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 0);
	linphone_core_set_media_encryption_mandatory(marie->lc,0);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
	bctbx_list_t * cfg_enc = create_confg_encryption_preference_list_except(LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(marie->lc,cfg_enc);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 0);
	linphone_core_set_media_encryption_mandatory(pauline->lc,0);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(pauline->lc,cfg_enc);
	bctbx_list_free_with_data(cfg_enc, (bctbx_list_free_func)bctbx_free);

	call_with_encryption_base(marie, pauline, LinphoneMediaEncryptionNone, TRUE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void call_with_different_encryptions_in_call_params(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Desired encryption
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 1);
	linphone_core_set_media_encryption_mandatory(marie->lc,0);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
	bctbx_list_t * marie_cfg_enc = NULL;
	marie_cfg_enc = bctbx_list_append(marie_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionSRTP)));
	marie_cfg_enc = bctbx_list_append(marie_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	marie_cfg_enc = bctbx_list_append(marie_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionNone)));
	marie_cfg_enc = bctbx_list_append(marie_cfg_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_core_set_supported_media_encryptions(marie->lc,marie_cfg_enc);
	bctbx_list_free_with_data(marie_cfg_enc, (bctbx_list_free_func)bctbx_free);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 1);
	linphone_core_set_media_encryption_mandatory(pauline->lc,0);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionNone);
	bctbx_list_t * pauline_cfg_enc = NULL;
	pauline_cfg_enc = bctbx_list_append(pauline_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionSRTP)));
	pauline_cfg_enc = bctbx_list_append(pauline_cfg_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	pauline_cfg_enc = bctbx_list_append(pauline_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	pauline_cfg_enc = bctbx_list_append(pauline_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionNone)));
	linphone_core_set_supported_media_encryptions(pauline->lc,pauline_cfg_enc);
	bctbx_list_free_with_data(pauline_cfg_enc, (bctbx_list_free_func)bctbx_free);

	bctbx_list_t * call_enc = NULL;
	call_enc = bctbx_list_append(call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_supported_encryptions (marie_params, call_enc);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, 1);
	linphone_call_params_set_supported_encryptions (pauline_params, call_enc);

	bctbx_list_free_with_data(call_enc, (bctbx_list_free_func)bctbx_free);

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	const int expectedStreamsRunning = 2;
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");

	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(paulineCall)), encryption, int, "%i");
	}

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);


}

static void call_with_potential_configuration_same_as_actual_configuration_base (const LinphoneMediaEncryption encryption) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 1);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 1);

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc,encryption));
	if (linphone_core_media_encryption_supported(pauline->lc,encryption)) {
		bctbx_list_t * encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
		linphone_core_set_media_encryption_mandatory(pauline->lc,0);
		linphone_core_set_media_encryption(pauline->lc,encryption);
		linphone_core_set_supported_media_encryptions(pauline->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, encryption));
		if (encryption_list) {
			bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
		}
	}

	linphone_core_set_media_encryption_mandatory(marie->lc,0);
	linphone_core_set_media_encryption(marie->lc,encryption);
	// Desired encryption is put last
	bctbx_list_t * encryption_list = create_confg_encryption_preference_list_from_param_preferences(set_encryption_preference_with_priority(encryption, true));
	linphone_core_set_supported_media_encryptions(marie->lc,encryption_list);
	if (encryption_list) {
		bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
	}

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, encryption));
	BC_ASSERT_TRUE(linphone_core_get_media_encryption(marie->lc) == encryption);
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, encryption));
	BC_ASSERT_TRUE(linphone_core_get_media_encryption(pauline->lc) == encryption);

	call_with_encryption_base(marie, pauline, encryption, TRUE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionNone);
}

static void srtp_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionZRTP);
}

static void simple_call_with_capability_negotiations(const LinphoneMediaEncryption optionalEncryption) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_support_capability_negotiation(marie->lc, 1);

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_support_capability_negotiation(pauline->lc, 1);

	bctbx_list_t * encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(optionalEncryption))));

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc,optionalEncryption));
	if (linphone_core_media_encryption_supported(pauline->lc,optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(pauline->lc,0);
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(pauline->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, optionalEncryption));
	}

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc,optionalEncryption));
	if (linphone_core_media_encryption_supported(marie->lc,optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(marie->lc,0);
		linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(marie->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, optionalEncryption));
	}

	if (encryption_list) {
		bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
	}

	call_with_encryption_base(marie, pauline, optionalEncryption, TRUE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations(void) {
	simple_call_with_capability_negotiations(LinphoneMediaEncryptionSRTP);
}

static void simple_zrtp_call_with_capability_negotiations(void) {
	simple_call_with_capability_negotiations(LinphoneMediaEncryptionZRTP);
}

static void simple_dtls_call_with_capability_negotiations(void) {
	simple_call_with_capability_negotiations(LinphoneMediaEncryptionDTLS);
}

static LinphoneCoreManager * create_core_mgr_with_capabiliy_negotiation_setup(const char * rc_file, const encryption_params enc_params, const bool_t enable_capability_negotiations) {
	LinphoneCoreManager* mgr = linphone_core_manager_new(rc_file);
	linphone_core_set_support_capability_negotiation(mgr->lc, enable_capability_negotiations);

	const LinphoneMediaEncryption encryption = enc_params.encryption;
	if (linphone_core_media_encryption_supported(mgr->lc,encryption)) {
		linphone_core_set_media_encryption_mandatory(mgr->lc,(enc_params.level == E_MANDATORY));

		if (enc_params.level == E_MANDATORY) {
			linphone_core_set_media_encryption(mgr->lc,encryption);
			BC_ASSERT_EQUAL(linphone_core_get_media_encryption(mgr->lc), encryption, int, "%i");
		} if (enc_params.level == E_OPTIONAL) {
			bctbx_list_t * cfg_enc = create_confg_encryption_preference_list_from_param_preferences(enc_params.preferences);
			linphone_core_set_supported_media_encryptions(mgr->lc,cfg_enc);
			BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(mgr->lc, encryption));
			bctbx_list_free_with_data(cfg_enc, (bctbx_list_free_func)bctbx_free);

			linphone_core_set_media_encryption(mgr->lc,LinphoneMediaEncryptionNone);
		}
	}

	return mgr;
}

static void call_with_encryption_test_base(const encryption_params marie_enc_params, const bool_t enable_marie_capability_negotiations, const encryption_params pauline_enc_params, const bool_t enable_pauline_capability_negotiations) {

	LinphoneCoreManager * marie = create_core_mgr_with_capabiliy_negotiation_setup("marie_rc", marie_enc_params, enable_marie_capability_negotiations);
	LinphoneCoreManager * pauline = create_core_mgr_with_capabiliy_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_params, enable_pauline_capability_negotiations);

	LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
	if ((enable_marie_capability_negotiations == TRUE) && (enable_pauline_capability_negotiations == TRUE)) {
		expectedEncryption = marie_enc_params.encryption;
	}

	call_with_encryption_base(marie, pauline, expectedEncryption, enable_marie_capability_negotiations, enable_pauline_capability_negotiations);

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
	call_with_encryption_test_base(marie_enc_params, FALSE, pauline_enc_params, FALSE);
}

static void call_with_mandatory_encryption_base(const LinphoneMediaEncryption encryption, const bool_t caller_capability_negotiation, const bool_t callee_capability_negotiation) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_MANDATORY;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_MANDATORY;
	call_with_encryption_test_base(marie_enc_params, caller_capability_negotiation, pauline_enc_params, callee_capability_negotiation);
}

static void srtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE);
}

static void dtls_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE);
}

static void zrtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE);
}

static void dtls_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE);
}

static void dtls_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, FALSE, TRUE);
}

static void dtls_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, FALSE, TRUE);
}

static void call_from_opt_enc_to_enc_base(const LinphoneMediaEncryption encryption, bool_t opt_enc_to_enc) {
	encryption_params optional_enc_mgr_params;
	optional_enc_mgr_params.encryption = encryption;
	optional_enc_mgr_params.level = E_OPTIONAL;
	optional_enc_mgr_params.preferences = set_encryption_preference(TRUE);

	encryption_params mandatory_enc_mgr_params;
	mandatory_enc_mgr_params.encryption = encryption;
	mandatory_enc_mgr_params.level = E_MANDATORY;
	if (opt_enc_to_enc) {
		call_with_encryption_test_base(optional_enc_mgr_params, TRUE, mandatory_enc_mgr_params, TRUE);
	} else {
		call_with_encryption_test_base(mandatory_enc_mgr_params, TRUE, optional_enc_mgr_params, TRUE);
	}
}

static void srtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionSRTP, TRUE);
}

static void dtls_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionDTLS, TRUE);
}

static void zrtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionZRTP, TRUE);
}

static void srtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionSRTP, FALSE);
}

static void dtls_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionDTLS, FALSE);
}

static void zrtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionZRTP, FALSE);
}

static void call_from_opt_enc_to_none_base(const LinphoneMediaEncryption encryption, bool_t opt_enc_to_none) {
	encryption_params no_enc_mgr_params;
	no_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	no_enc_mgr_params.level = E_DISABLED;

	encryption_params enc_mgr_params;
	enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	enc_mgr_params.level = E_OPTIONAL;
	enc_mgr_params.preferences = set_encryption_preference_with_priority(encryption, false);
	if (opt_enc_to_none) {
		call_with_encryption_test_base(enc_mgr_params, TRUE, no_enc_mgr_params, FALSE);
	} else {
		call_with_encryption_test_base(no_enc_mgr_params, FALSE, enc_mgr_params, TRUE);
	}
}

static void srtp_call_from_opt_enc_to_none(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE);
}

static void dtls_call_from_opt_enc_to_none(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE);
}

static void zrtp_call_from_opt_enc_to_none(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE);
}

static void srtp_call_from_no_enc_to_opt(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE);
}

static void dtls_call_from_no_enc_to_opt(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE);
}

static void zrtp_call_from_no_enc_to_opt(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE);
}

static void call_with_optional_encryption_on_both_sides_base(const LinphoneMediaEncryption encryption) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_OPTIONAL;
	marie_enc_params.preferences = set_encryption_preference_with_priority(encryption, false);

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_OPTIONAL;
	pauline_enc_params.preferences = set_encryption_preference_with_priority(encryption, true);

	call_with_encryption_test_base(marie_enc_params, TRUE, pauline_enc_params, TRUE);
}

static void srtp_call_with_optional_encryption_on_both_sides_side(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionSRTP);
}

static void dtls_call_with_optional_encryption_on_both_sides_side(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionDTLS);
}

static void zrtp_call_with_optional_encryption_on_both_sides_side(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionZRTP);
}

test_t capability_negotiation_tests[] = {
	TEST_NO_TAG("Call with no encryption", call_with_no_encryption),
	TEST_NO_TAG("Call with capability negotiation failure", call_with_capability_negotiation_failure),
	TEST_NO_TAG("Call with capability negotiation failure and multiple potential configurations", call_with_capability_negotiation_failure_multiple_potential_configurations),
	TEST_NO_TAG("Call with capability negotiation disabled at call level", call_with_capability_negotiation_disable_call_level),
	TEST_NO_TAG("Call with capability negotiation disabled at core level", call_with_capability_negotiation_disable_core_level),
	TEST_NO_TAG("Call with different encryptions in call params", call_with_different_encryptions_in_call_params),
	TEST_NO_TAG("Unencrypted call with potential configuration same as actual one", unencrypted_call_with_potential_configuration_same_as_actual_configuration),
	TEST_NO_TAG("Simple SRTP call with capability negotiations", simple_srtp_call_with_capability_negotiations),
	TEST_NO_TAG("SRTP call with potential configuration same as actual one", srtp_call_with_potential_configuration_same_as_actual_configuration),
	TEST_NO_TAG("SRTP call with mandatory encryption", srtp_call_with_mandatory_encryption),
	TEST_NO_TAG("SRTP call with mandatory encryption and capability negotiation on both sides", srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides),
	TEST_NO_TAG("SRTP call with mandatory encryption and capability negotiation on callee side", srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side),
	TEST_NO_TAG("SRTP call with mandatory encryption and capability negotiation on caller side", srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side),
	TEST_NO_TAG("SRTP call from endpoint with mandatory encryption to endpoint with none", srtp_call_from_enc_to_no_enc),
	TEST_NO_TAG("SRTP call from endpoint with no encryption to endpoint with mandatory", srtp_call_from_no_enc_to_enc),
	TEST_NO_TAG("SRTP call from endpoint with optional encryption to endpoint with mandatory", srtp_call_from_opt_enc_to_enc),
	TEST_NO_TAG("SRTP call from endpoint with mandatory encryption to endpoint with optional", srtp_call_from_enc_to_opt_enc),
	TEST_NO_TAG("SRTP call from endpoint with optional encryption to endpoint with none", srtp_call_from_opt_enc_to_none),
	TEST_NO_TAG("SRTP call from endpoint with no encryption to endpoint with optional", srtp_call_from_no_enc_to_opt),
	TEST_NO_TAG("SRTP call with optional encryption on both sides", srtp_call_with_optional_encryption_on_both_sides_side),
	TEST_NO_TAG("Simple DTLS call with capability negotiations", simple_dtls_call_with_capability_negotiations),
	TEST_NO_TAG("DTLS call with potential configuration same as actual one", dtls_call_with_potential_configuration_same_as_actual_configuration),
	TEST_NO_TAG("DTLS call with mandatory encryption", dtls_call_with_mandatory_encryption),
	TEST_NO_TAG("DTLS call with mandatory encryption and capability negotiation on both sides", dtls_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides),
	TEST_NO_TAG("DTLS call with mandatory encryption and capability negotiation on callee side", dtls_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side),
	TEST_NO_TAG("DTLS call with mandatory encryption and capability negotiation on caller side", dtls_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side),
	TEST_NO_TAG("DTLS call from endpoint with mandatory encryption to endpoint with none", dtls_call_from_enc_to_no_enc),
	TEST_NO_TAG("DTLS call from endpoint with no encryption to endpoint with mandatory", dtls_call_from_no_enc_to_enc),
	TEST_NO_TAG("DTLS call from endpoint with optional encryption to endpoint with mandatory", dtls_call_from_opt_enc_to_enc),
	TEST_NO_TAG("DTLS call from endpoint with mandatory encryption to endpoint with optional", dtls_call_from_enc_to_opt_enc),
	TEST_NO_TAG("DTLS call from endpoint with optional encryption to endpoint with none", dtls_call_from_opt_enc_to_none),
	TEST_NO_TAG("DTLS call from endpoint with no encryption to endpoint with optional", dtls_call_from_no_enc_to_opt),
	TEST_NO_TAG("DTLS call with optional encryption on both sides", dtls_call_with_optional_encryption_on_both_sides_side),
	TEST_NO_TAG("Simple ZRTP call with capability negotiations", simple_zrtp_call_with_capability_negotiations),
	TEST_NO_TAG("ZRTP call with potential configuration same as actual one", zrtp_call_with_potential_configuration_same_as_actual_configuration),
	TEST_NO_TAG("ZRTP call with mandatory encryption", zrtp_call_with_mandatory_encryption),
	TEST_NO_TAG("ZRTP call with mandatory encryption and capability negotiation on both sides", zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides),
	TEST_NO_TAG("ZRTP call with mandatory encryption and capability negotiation on callee side", zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side),
	TEST_NO_TAG("ZRTP call with mandatory encryption and capability negotiation on caller side", zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side),
	TEST_NO_TAG("ZRTP call from endpoint with mandatory encryption to endpoint with DTLS", zrtp_call_from_enc_to_dtls_enc),
	TEST_NO_TAG("ZRTP call from endpoint with DTLS encryption to endpoint with mandatory", zrtp_call_from_dtls_enc_to_enc),
	TEST_NO_TAG("ZRTP call from endpoint with optional encryption to endpoint with mandatory", zrtp_call_from_opt_enc_to_enc),
	TEST_NO_TAG("ZRTP call from endpoint with mandatory encryption to endpoint with optional", zrtp_call_from_enc_to_opt_enc),
	TEST_NO_TAG("ZRTP call from endpoint with optional encryption to endpoint with none", zrtp_call_from_opt_enc_to_none),
	TEST_NO_TAG("ZRTP call from endpoint with no encryption to endpoint with optional", zrtp_call_from_no_enc_to_opt),
	TEST_NO_TAG("ZRTP call with optional encryption on both sides", zrtp_call_with_optional_encryption_on_both_sides_side)
};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(capability_negotiation_tests) / sizeof(capability_negotiation_tests[0]), capability_negotiation_tests};
