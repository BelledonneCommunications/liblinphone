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

static LinphoneCoreManager * create_core_mgr_with_capability_negotiation_setup(const char * rc_file, const encryption_params enc_params, const bool_t enable_capability_negotiations, const bool_t enable_ice, const bool_t enable_video) {
	LinphoneCoreManager* mgr = linphone_core_manager_new(rc_file);
	linphone_core_set_support_capability_negotiation(mgr->lc, enable_capability_negotiations);
	if (enable_ice){
		LinphoneNatPolicy *pol = linphone_core_get_nat_policy(mgr->lc);
		linphone_nat_policy_enable_ice(pol, TRUE);
		linphone_core_set_nat_policy(mgr->lc, pol);
	}

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

	if (enable_video) {
		// important: VP8 has really poor performances with the mire camera, at least
		// on iOS - so when ever h264 is available, let's use it instead
		if (linphone_core_get_payload_type(mgr->lc,"h264", -1, -1)!=NULL) {
			disable_all_video_codecs_except_one(mgr->lc,"h264");
		}

		linphone_core_set_video_device(mgr->lc,liblinphone_tester_mire_id);

		LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
		linphone_core_set_video_activation_policy(mgr->lc, pol);
		linphone_video_activation_policy_unref(pol);

		linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);

		linphone_core_enable_video_capture(mgr->lc, TRUE);
		linphone_core_enable_video_display(mgr->lc, TRUE);
	}

	return mgr;
}

static bool_t call_with_params_and_encryption_negotiation_failure_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneCallParams * caller_params, LinphoneCallParams * callee_params, bool_t expSdpSuccess) {

	const bctbx_list_t *initLogs = linphone_core_get_call_logs(callee->lc);
	int initLogsSize = (int)bctbx_list_size(initLogs);
	stats initial_callee=callee->stat;
	stats initial_caller=callee->stat;

	LinphoneCallTestParams caller_test_params = {0}, callee_test_params =  {0};
	caller_test_params.base = (LinphoneCallParams*)caller_params;
	callee_test_params.base = (LinphoneCallParams*)callee_params;
	callee_test_params.sdp_simulate_error = !expSdpSuccess;

	bool_t ret = call_with_params2(caller, callee, &caller_test_params, &callee_test_params, FALSE);

	LinphoneCall * callee_call = linphone_core_get_current_call(callee->lc);
	if (!expSdpSuccess) {
		BC_ASSERT_PTR_NULL(callee_call);
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError,(initial_caller.number_of_LinphoneCallError + 1), int, "%d");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallReleased,(initial_caller.number_of_LinphoneCallReleased + 1), int, "%d");
		// actually callee does not receive error because it replies to the INVITE with a 488 Not Acceptable Here
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived, (initial_callee.number_of_LinphoneCallIncomingReceived), int, "%d");

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

		return FALSE;

	}

	return ret;
}

static bool_t call_with_encryption_negotiation_failure_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, bool_t expSdpSuccess) {
	return call_with_params_and_encryption_negotiation_failure_base(caller, callee, NULL, NULL, expSdpSuccess);
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
		BC_ASSERT_FALSE(call_with_encryption_negotiation_failure_base(pauline, marie, FALSE));
	} else {
		ms_message("Core with non mandatory encryption calls core with mandatory encryption");
		BC_ASSERT_FALSE(call_with_encryption_negotiation_failure_base(marie, pauline, FALSE));
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

static void dtls_srtp_call_from_enc_to_no_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE);
}

static void zrtp_call_from_enc_to_dtls_enc(void) {
	call_from_enc_to_dtls_enc_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE, TRUE);
}

static void srtp_call_from_no_enc_to_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE, FALSE);
}

static void dtls_srtp_call_from_no_enc_to_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, FALSE);
}

static void zrtp_call_from_dtls_enc_to_enc(void) {
	call_from_enc_to_dtls_enc_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE, FALSE);
}

static void call_with_encryption_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption, const bool_t enable_caller_capability_negotiations, const bool_t enable_callee_capability_negotiations, const bool_t enable_video) {

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

		char *path = bc_tester_file("certificates-marie");
		linphone_core_set_user_certificates_path(callee->lc, path);
		bc_free(path);
		path = bc_tester_file("certificates-pauline");
		linphone_core_set_user_certificates_path(caller->lc, path);
		bc_free(path);
		belle_sip_mkdir(linphone_core_get_user_certificates_path(callee->lc));
		belle_sip_mkdir(linphone_core_get_user_certificates_path(caller->lc));

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

		LinphoneNatPolicy *caller_nat_policy = linphone_core_get_nat_policy(caller->lc);
		const bool_t caller_ice_enabled = linphone_nat_policy_ice_enabled(caller_nat_policy);
		LinphoneNatPolicy *callee_nat_policy = linphone_core_get_nat_policy(callee->lc);
		const bool_t callee_ice_enabled = linphone_nat_policy_ice_enabled(callee_nat_policy);
		const int expectedStreamsRunning = 1 + ((potentialConfigurationChosen || (callee_ice_enabled && caller_ice_enabled)) ? 1 : 0);

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,expectedStreamsRunning));
		BC_ASSERT_TRUE(wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,expectedStreamsRunning));

		liblinphone_tester_check_rtcp(caller, callee);

		// Check that no reINVITE is sent while checking streams
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");

		if (callerCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(callerCall)), expectedEncryption, int, "%i");
		}
		if (calleeCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(calleeCall)), expectedEncryption, int, "%i");
		}

		LinphoneCall * callee_call = linphone_core_get_current_call(callee->lc);
		BC_ASSERT_PTR_NOT_NULL(callee_call);

		LinphoneCall * caller_call = linphone_core_get_current_call(caller->lc);
		BC_ASSERT_PTR_NOT_NULL(caller_call);

		if (enable_video) {
			stats caller_stat = caller->stat; 
			stats callee_stat = callee->stat; 
			LinphoneCallParams * params = linphone_core_create_call_params(callee->lc, callee_call);
			linphone_call_params_enable_video(params, TRUE);
			linphone_call_update(callee_call, params);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallUpdating,(callee_stat.number_of_LinphoneCallUpdating+1)));
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallUpdatedByRemote,(caller_stat.number_of_LinphoneCallUpdatedByRemote+1)));
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,(callee_stat.number_of_LinphoneCallStreamsRunning+1+((potentialConfigurationChosen) ? 1 : 0))));
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,(caller_stat.number_of_LinphoneCallStreamsRunning+1+((potentialConfigurationChosen) ? 1 : 0))));
			liblinphone_tester_set_next_video_frame_decoded_cb(caller_call);
			liblinphone_tester_set_next_video_frame_decoded_cb(callee_call);

			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&callee->stat.number_of_IframeDecoded,(callee_stat.number_of_IframeDecoded+1)));
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&caller->stat.number_of_IframeDecoded,(caller_stat.number_of_IframeDecoded+1)));

			BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(callee_call)));
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(caller_call)));

			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));

			liblinphone_tester_check_rtcp(caller, callee);
		} else {
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(callee_call)));
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(caller_call)));

			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));
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

	call_with_encryption_base(marie, pauline, marieOptionalEncryption, TRUE, TRUE, FALSE);

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

	call_with_encryption_base(marie, pauline, marieOptionalEncryption0, TRUE, TRUE, FALSE);

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

	call_with_encryption_base(marie, pauline, LinphoneMediaEncryptionNone, FALSE, FALSE, FALSE);

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

	const char * chosenMediaEnc = static_cast<const char *>(bctbx_list_get_data(cfg_enc));

	call_with_encryption_base(marie, pauline, static_cast<LinphoneMediaEncryption>(string_to_linphone_media_encryption(chosenMediaEnc)), TRUE, TRUE, FALSE);
	bctbx_list_free_with_data(cfg_enc, (bctbx_list_free_func)bctbx_free);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void call_with_incompatible_encs_in_call_params(void) {
	const LinphoneMediaEncryption defaultEncryption = LinphoneMediaEncryptionNone;
	const LinphoneMediaEncryption marieEncryption = LinphoneMediaEncryptionDTLS; // Desired encryption
	const LinphoneMediaEncryption paulineEncryption = LinphoneMediaEncryptionSRTP; // Desired encryption
	std::list<LinphoneMediaEncryption> marie_enc_list;
	if ((paulineEncryption != LinphoneMediaEncryptionZRTP) && (marieEncryption != LinphoneMediaEncryptionZRTP)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if ((paulineEncryption != LinphoneMediaEncryptionDTLS) && (marieEncryption != LinphoneMediaEncryptionDTLS)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}
	if ((paulineEncryption != LinphoneMediaEncryptionSRTP) && (marieEncryption != LinphoneMediaEncryptionSRTP)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if ((paulineEncryption != LinphoneMediaEncryptionNone) && (marieEncryption != LinphoneMediaEncryptionNone)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionNone);
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, FALSE);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	if ((paulineEncryption != LinphoneMediaEncryptionSRTP) && (marieEncryption != LinphoneMediaEncryptionSRTP)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if ((paulineEncryption != LinphoneMediaEncryptionZRTP) && (marieEncryption != LinphoneMediaEncryptionZRTP)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if ((paulineEncryption != LinphoneMediaEncryptionNone) && (marieEncryption != LinphoneMediaEncryptionNone)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionNone);
	}
	if ((paulineEncryption != LinphoneMediaEncryptionDTLS) && (marieEncryption != LinphoneMediaEncryptionDTLS)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE, FALSE, FALSE);



	bctbx_list_t * marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(marieEncryption))));
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);

	bctbx_list_t * pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(paulineEncryption))));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, 1);
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);
	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);

	// Check requirements for this test:
	// - encryption of Marie and Pauline should be different
	// - Marie and Pauline encryption list in the call params must be equal to 1
	BC_ASSERT_NOT_EQUAL(paulineEncryption, marieEncryption, int, "%i");
	BC_ASSERT_EQUAL(bctbx_list_size(linphone_call_params_get_supported_encryptions(pauline_params)), 1, int, "%d");
	BC_ASSERT_EQUAL(bctbx_list_size(linphone_call_params_get_supported_encryptions(marie_params)), 1, int, "%d");

	BC_ASSERT_TRUE(call_with_params_and_encryption_negotiation_failure_base(marie, pauline, marie_params, pauline_params, TRUE));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(marieCall)), defaultEncryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(paulineCall)), defaultEncryption, int, "%i");
	}

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void call_with_video_and_capability_negotiation(void) {

	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Desired encryption
	std::list<LinphoneMediaEncryption> marie_enc_list;
	if (encryption != LinphoneMediaEncryptionZRTP) {
		marie_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (encryption != LinphoneMediaEncryptionDTLS) {
		marie_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}
	if (encryption != LinphoneMediaEncryptionSRTP) {
		marie_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (encryption != LinphoneMediaEncryptionNone) {
		marie_enc_list.push_back(LinphoneMediaEncryptionNone);
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	if (encryption != LinphoneMediaEncryptionSRTP) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (encryption != LinphoneMediaEncryptionZRTP) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (encryption != LinphoneMediaEncryptionNone) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionNone);
	}
	if (encryption != LinphoneMediaEncryptionDTLS) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE, FALSE, TRUE);

	bctbx_list_t * call_enc = NULL;
	call_enc = bctbx_list_append(call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(marie_params, 1);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_supported_encryptions (marie_params, call_enc);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_video(pauline_params, 1);
	linphone_call_params_enable_capability_negotiations (pauline_params, 1);
	linphone_call_params_set_supported_encryptions (pauline_params, call_enc);

	bctbx_list_free_with_data(call_enc, (bctbx_list_free_func)bctbx_free);

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	const int expectedStreamsRunning = 2;
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");

	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_params(paulineCall)), encryption, int, "%i");
	}

	if (marieCall && paulineCall ) {
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));

		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));

		/*check video path*/
		liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);
		liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
		linphone_call_send_vfu_request(paulineCall);
		BC_ASSERT_TRUE( wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_IframeDecoded,1));
		BC_ASSERT_TRUE( wait_for(marie->lc,pauline->lc,&marie->stat.number_of_IframeDecoded,1));
	}

	liblinphone_tester_check_rtcp(pauline,marie);

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);


}

static void call_with_different_encryptions_in_call_params(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Desired encryption
	std::list<LinphoneMediaEncryption> marie_enc_list;
	if (encryption != LinphoneMediaEncryptionZRTP) {
		marie_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (encryption != LinphoneMediaEncryptionDTLS) {
		marie_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}
	if (encryption != LinphoneMediaEncryptionSRTP) {
		marie_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (encryption != LinphoneMediaEncryptionNone) {
		marie_enc_list.push_back(LinphoneMediaEncryptionNone);
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, FALSE);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	if (encryption != LinphoneMediaEncryptionSRTP) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (encryption != LinphoneMediaEncryptionZRTP) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (encryption != LinphoneMediaEncryptionNone) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionNone);
	}
	if (encryption != LinphoneMediaEncryptionDTLS) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE, FALSE, FALSE);

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

	call_with_encryption_base(marie, pauline, encryption, TRUE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionNone);
}

static void srtp_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionSRTP);
}

static void dtls_srtp_call_with_potential_configuration_same_as_actual_configuration(void) {
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

	call_with_encryption_base(marie, pauline, optionalEncryption, TRUE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations(void) {
	simple_call_with_capability_negotiations(LinphoneMediaEncryptionSRTP);
}

static void simple_zrtp_call_with_capability_negotiations(void) {
	simple_call_with_capability_negotiations(LinphoneMediaEncryptionZRTP);
}

static void simple_dtls_srtp_call_with_capability_negotiations(void) {
	simple_call_with_capability_negotiations(LinphoneMediaEncryptionDTLS);
}

static void call_with_encryption_test_base(const encryption_params marie_enc_params, const bool_t enable_marie_capability_negotiations, const bool_t enable_marie_ice, const encryption_params pauline_enc_params, const bool_t enable_pauline_capability_negotiations, const bool_t enable_pauline_ice, const bool_t enable_video) {

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_params, enable_marie_capability_negotiations, enable_marie_ice, enable_video);
	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_params, enable_pauline_capability_negotiations, enable_pauline_ice, enable_video);

	LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
	if ((enable_marie_capability_negotiations == TRUE) && (enable_pauline_capability_negotiations == TRUE)) {
		expectedEncryption = marie_enc_params.encryption;
	}

	call_with_encryption_base(marie, pauline, expectedEncryption, enable_marie_capability_negotiations, enable_pauline_capability_negotiations, enable_video);

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
	call_with_encryption_test_base(marie_enc_params, FALSE, FALSE, pauline_enc_params, FALSE, FALSE, FALSE);
}

static void call_with_mandatory_encryption_base(const LinphoneMediaEncryption encryption, const bool_t caller_capability_negotiation, const bool_t callee_capability_negotiation) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_MANDATORY;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_MANDATORY;
	call_with_encryption_test_base(marie_enc_params, caller_capability_negotiation, FALSE, pauline_enc_params, callee_capability_negotiation, FALSE, FALSE);
}

static void srtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE);
}

static void dtls_srtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE);
}

static void zrtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE);
}

static void dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE);
}

static void dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, FALSE, TRUE);
}

static void dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
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
		call_with_encryption_test_base(optional_enc_mgr_params, TRUE, FALSE, mandatory_enc_mgr_params, TRUE, FALSE, FALSE);
	} else {
		call_with_encryption_test_base(mandatory_enc_mgr_params, TRUE, FALSE, optional_enc_mgr_params, TRUE, FALSE, FALSE);
	}
}

static void srtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionSRTP, TRUE);
}

static void dtls_srtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionDTLS, TRUE);
}

static void zrtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionZRTP, TRUE);
}

static void srtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionSRTP, FALSE);
}

static void dtls_srtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionDTLS, FALSE);
}

static void zrtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionZRTP, FALSE);
}

static void call_from_opt_enc_to_none_base(const LinphoneMediaEncryption encryption, bool_t opt_enc_to_none, const bool_t enable_video) {
	encryption_params no_enc_mgr_params;
	no_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	no_enc_mgr_params.level = E_DISABLED;

	encryption_params enc_mgr_params;
	enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	enc_mgr_params.level = E_OPTIONAL;
	enc_mgr_params.preferences = set_encryption_preference_with_priority(encryption, false);
	if (opt_enc_to_none) {
		call_with_encryption_test_base(enc_mgr_params, TRUE, FALSE, no_enc_mgr_params, FALSE, FALSE, enable_video);
	} else {
		call_with_encryption_test_base(no_enc_mgr_params, FALSE, FALSE, enc_mgr_params, TRUE, FALSE, enable_video);
	}
}

static void srtp_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE);
}

static void zrtp_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE);
}

static void srtp_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE);
}

static void zrtp_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE);
}

static void srtp_video_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE);
}

static void dtls_srtp_video_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE);
}

static void zrtp_video_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE);
}

static void srtp_video_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE, TRUE);
}

static void dtls_srtp_video_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE);
}

static void zrtp_video_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE, TRUE);
}

static void call_with_optional_encryption_on_both_sides_base(const LinphoneMediaEncryption encryption, const bool_t enable_video) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_OPTIONAL;
	marie_enc_params.preferences = set_encryption_preference_with_priority(encryption, false);

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_OPTIONAL;
	pauline_enc_params.preferences = set_encryption_preference_with_priority(encryption, true);

	call_with_encryption_test_base(marie_enc_params, TRUE, FALSE, pauline_enc_params, TRUE, FALSE, enable_video);
}

static void srtp_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionSRTP, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionDTLS, FALSE);
}

static void zrtp_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionZRTP, FALSE);
}

static void srtp_video_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionSRTP, TRUE);
}

static void dtls_srtp_video_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionDTLS, TRUE);
}

static void zrtp_video_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionZRTP, TRUE);
}

static void ice_call_with_optional_encryption(const LinphoneMediaEncryption encryption, const bool_t caller_with_ice, const bool_t callee_with_ice, const bool_t enable_video) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_OPTIONAL;
	marie_enc_params.preferences = set_encryption_preference_with_priority(encryption, false);

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_OPTIONAL;
	pauline_enc_params.preferences = set_encryption_preference_with_priority(encryption, true);

	call_with_encryption_test_base(marie_enc_params, TRUE, caller_with_ice, pauline_enc_params, TRUE, callee_with_ice, enable_video);
}

static void srtp_ice_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, TRUE, FALSE);
}

static void dtls_srtp_ice_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, TRUE, FALSE);
}

static void zrtp_ice_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, TRUE, FALSE);
}

static void srtp_call_with_optional_encryption_caller_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, FALSE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_caller_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, FALSE, FALSE);
}

static void zrtp_call_with_optional_encryption_caller_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, FALSE, FALSE);
}

static void srtp_call_with_optional_encryption_callee_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, FALSE, TRUE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_callee_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, FALSE, TRUE, FALSE);
}

static void zrtp_call_with_optional_encryption_callee_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, FALSE, TRUE, FALSE);
}

static void srtp_ice_video_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, TRUE, TRUE);
}

static void dtls_srtp_ice_video_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE);
}

static void zrtp_ice_video_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, TRUE, TRUE);
}

static void ice_call_from_opt_enc_to_none_base(const LinphoneMediaEncryption encryption, bool_t opt_enc_to_none, const bool_t enable_video) {
	encryption_params no_enc_mgr_params;
	no_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	no_enc_mgr_params.level = E_DISABLED;

	encryption_params enc_mgr_params;
	enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	enc_mgr_params.level = E_OPTIONAL;
	enc_mgr_params.preferences = set_encryption_preference_with_priority(encryption, false);
	if (opt_enc_to_none) {
		call_with_encryption_test_base(enc_mgr_params, TRUE, TRUE, no_enc_mgr_params, FALSE, TRUE, enable_video);
	} else {
		call_with_encryption_test_base(no_enc_mgr_params, FALSE, TRUE, enc_mgr_params, TRUE, TRUE, enable_video);
}
}

static void srtp_ice_call_with_optional_encryption_on_caller(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE);
}

static void dtls_srtp_ice_call_with_optional_encryption_on_caller(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE);
}

static void zrtp_ice_call_with_optional_encryption_on_caller(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE);
}

static void srtp_ice_call_with_optional_encryption_on_callee(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE);
}

static void dtls_srtp_ice_call_with_optional_encryption_on_callee(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE);
}

static void zrtp_ice_call_with_optional_encryption_on_callee(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE);
}

static void srtp_ice_video_call_with_optional_encryption_on_caller(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE);
}

static void dtls_srtp_ice_video_call_with_optional_encryption_on_caller(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE);
}

static void zrtp_ice_video_call_with_optional_encryption_on_caller(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE);
}

static void srtp_ice_video_call_with_optional_encryption_on_callee(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE, TRUE);
}

static void dtls_srtp_ice_video_call_with_optional_encryption_on_callee(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE);
}

static void zrtp_ice_video_call_with_optional_encryption_on_callee(void) {
	ice_call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE, TRUE);
}

test_t capability_negotiation_tests[] = {
	TEST_NO_TAG("Call with no encryption", call_with_no_encryption),
	TEST_NO_TAG("Call with capability negotiation failure", call_with_capability_negotiation_failure),
	TEST_NO_TAG("Call with capability negotiation failure and multiple potential configurations", call_with_capability_negotiation_failure_multiple_potential_configurations),
	TEST_NO_TAG("Call with capability negotiation disabled at call level", call_with_capability_negotiation_disable_call_level),
	TEST_NO_TAG("Call with capability negotiation disabled at core level", call_with_capability_negotiation_disable_core_level),
	TEST_NO_TAG("Call with different encryptions in call params", call_with_different_encryptions_in_call_params),
	TEST_NO_TAG("Call with incompatible encryptions in call params", call_with_incompatible_encs_in_call_params),
	TEST_NO_TAG("Call started with video and capability negotiation", call_with_video_and_capability_negotiation),
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
	TEST_NO_TAG("SRTP call from endpoint with optional encryption to endpoint with none", srtp_call_with_optional_encryption_on_caller),
	TEST_NO_TAG("SRTP call from endpoint with no encryption to endpoint with optional", srtp_call_with_optional_encryption_on_callee),
	TEST_NO_TAG("SRTP call with optional encryption on both sides", srtp_call_with_optional_encryption_on_both_sides),
	TEST_ONE_TAG("SRTP ICE call with optional encryption on caller", srtp_ice_call_with_optional_encryption_on_caller, "ICE"),
	TEST_ONE_TAG("SRTP ICE call with optional encryption on callee", srtp_ice_call_with_optional_encryption_on_callee, "ICE"),
	TEST_ONE_TAG("SRTP ICE call with optional encryption on both sides", srtp_ice_call_with_optional_encryption_on_both_sides, "ICE"),
	TEST_ONE_TAG("SRTP call with optional encryption (caller with ICE)", srtp_call_with_optional_encryption_caller_with_ice, "ICE"),
	TEST_ONE_TAG("SRTP call with optional encryption (callee with ICE)", srtp_call_with_optional_encryption_callee_with_ice, "ICE"),
	TEST_ONE_TAG("SRTP ICE call with optional encryption on both sides", srtp_ice_call_with_optional_encryption_on_both_sides, "ICE"),
	TEST_NO_TAG("SRTP video call with optional encryption on caller", srtp_video_call_with_optional_encryption_on_caller),
	TEST_NO_TAG("SRTP video call with optional encryption on callee", srtp_video_call_with_optional_encryption_on_callee),
	TEST_NO_TAG("SRTP video call with optional encryption on both sides", srtp_video_call_with_optional_encryption_on_both_sides),
	TEST_ONE_TAG("SRTP ICE video call with optional encryption on caller", srtp_ice_video_call_with_optional_encryption_on_caller, "ICE"),
	TEST_ONE_TAG("SRTP ICE video call with optional encryption on callee", srtp_ice_video_call_with_optional_encryption_on_callee, "ICE"),
	TEST_ONE_TAG("SRTP ICE video call with optional encryption on both sides", srtp_ice_video_call_with_optional_encryption_on_both_sides, "ICE"),
	TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations", simple_dtls_srtp_call_with_capability_negotiations, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with potential configuration same as actual one", dtls_srtp_call_with_potential_configuration_same_as_actual_configuration, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with mandatory encryption", dtls_srtp_call_with_mandatory_encryption, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with mandatory encryption and capability negotiation on both sides", dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with mandatory encryption and capability negotiation on callee side", dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with mandatory encryption and capability negotiation on caller side", dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call from endpoint with mandatory encryption to endpoint with none", dtls_srtp_call_from_enc_to_no_enc, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call from endpoint with no encryption to endpoint with mandatory", dtls_srtp_call_from_no_enc_to_enc, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call from endpoint with optional encryption to endpoint with mandatory", dtls_srtp_call_from_opt_enc_to_enc, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call from endpoint with mandatory encryption to endpoint with optional", dtls_srtp_call_from_enc_to_opt_enc, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call from endpoint with optional encryption to endpoint with none", dtls_srtp_call_with_optional_encryption_on_caller, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call from endpoint with no encryption to endpoint with optional", dtls_srtp_call_with_optional_encryption_on_callee, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP ICE call with optional encryption on caller", dtls_srtp_ice_call_with_optional_encryption_on_caller, "ICE"),
	TEST_ONE_TAG("DTLS SRTP ICE call with optional encryption on callee", dtls_srtp_ice_call_with_optional_encryption_on_callee, "ICE"),
	TEST_ONE_TAG("DTLS SRTP call with optional encryption on both sides", dtls_srtp_call_with_optional_encryption_on_both_sides, "DTLS"),
	TEST_ONE_TAG("DTLS ICE call with optional encryption on both sides", dtls_srtp_ice_call_with_optional_encryption_on_both_sides, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with optional encryption (caller with ICE)", dtls_srtp_call_with_optional_encryption_caller_with_ice, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with optional encryption (callee with ICE)", dtls_srtp_call_with_optional_encryption_callee_with_ice, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP video call with optional encryption on caller", dtls_srtp_video_call_with_optional_encryption_on_caller, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP video call with optional encryption on callee", dtls_srtp_video_call_with_optional_encryption_on_callee, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP video call with optional encryption on both sides", dtls_srtp_video_call_with_optional_encryption_on_both_sides, "DTLS"),
	TEST_TWO_TAGS("DTLS SRTP ICE video call with optional encryption on caller", dtls_srtp_ice_video_call_with_optional_encryption_on_caller, "ICE", "DTLS"),
	TEST_TWO_TAGS("DTLS SRTP ICE video call with optional encryption on callee", dtls_srtp_ice_video_call_with_optional_encryption_on_callee, "ICE", "DTLS"),
	TEST_TWO_TAGS("DTLS SRTP ICE video call with optional encryption on both sides", dtls_srtp_ice_video_call_with_optional_encryption_on_both_sides, "ICE", "DTLS"),
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
	TEST_NO_TAG("ZRTP call from endpoint with optional encryption to endpoint with none", zrtp_call_with_optional_encryption_on_caller),
	TEST_NO_TAG("ZRTP call from endpoint with no encryption to endpoint with optional", zrtp_call_with_optional_encryption_on_callee),
	TEST_NO_TAG("ZRTP call with optional encryption on both sides", zrtp_call_with_optional_encryption_on_both_sides),
	TEST_ONE_TAG("ZRTP ICE call with optional encryption on caller", zrtp_ice_call_with_optional_encryption_on_caller, "ICE"),
	TEST_ONE_TAG("ZRTP ICE call with optional encryption on callee", zrtp_ice_call_with_optional_encryption_on_callee, "ICE"),
	TEST_ONE_TAG("ZRTP ICE call with optional encryption on both sides", zrtp_ice_call_with_optional_encryption_on_both_sides, "ICE"),
	TEST_ONE_TAG("ZRTP call with optional encryption (caller with ICE)", zrtp_call_with_optional_encryption_caller_with_ice, "ICE"),
	TEST_ONE_TAG("ZRTP call with optional encryption (callee with ICE)", zrtp_call_with_optional_encryption_callee_with_ice, "ICE"),
	TEST_NO_TAG("ZRTP video call with optional encryption on caller", zrtp_video_call_with_optional_encryption_on_caller),
	TEST_NO_TAG("ZRTP video call with optional encryption on callee", zrtp_video_call_with_optional_encryption_on_callee),
	TEST_NO_TAG("ZRTP video call with optional encryption on both sides", zrtp_video_call_with_optional_encryption_on_both_sides),
	TEST_ONE_TAG("ZRTP ICE video call with optional encryption on caller", zrtp_ice_video_call_with_optional_encryption_on_caller, "ICE"),
	TEST_ONE_TAG("ZRTP ICE video call with optional encryption on callee", zrtp_ice_video_call_with_optional_encryption_on_callee, "ICE"),
	TEST_ONE_TAG("ZRTP ICE video call with optional encryption on both sides", zrtp_ice_video_call_with_optional_encryption_on_both_sides, "ICE")
};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(capability_negotiation_tests) / sizeof(capability_negotiation_tests[0]), capability_negotiation_tests};
