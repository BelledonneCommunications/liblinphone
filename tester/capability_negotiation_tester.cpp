/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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
#include <algorithm>

#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "shared_tester_functions.h"

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

void get_expected_encryption_from_call_params(LinphoneCall *offererCall, LinphoneCall *answererCall, LinphoneMediaEncryption* expectedEncryption, bool* potentialConfigurationChosen) {
	const LinphoneCallParams *offerer_params = linphone_call_get_params(offererCall);
	const LinphoneCallParams *answerer_params = linphone_call_get_params(answererCall);
	bool_t offerer_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(offerer_params);
	bool_t answerer_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(answerer_params);
	const LinphoneMediaEncryption offerer_encryption = linphone_call_params_get_media_encryption(offerer_params);
	const LinphoneMediaEncryption answerer_encryption = linphone_call_params_get_media_encryption(answerer_params);
	const bool_t offerer_capability_negotiations = linphone_call_params_capability_negotiations_enabled(offerer_params);
	const bool_t answerer_capability_negotiations = linphone_call_params_capability_negotiations_enabled(answerer_params);

	if (offerer_enc_mandatory) {
		*expectedEncryption = offerer_encryption;
		// reINVITE is not sent because the call should not offer potential configurations as it must enforce an encryption that will be stored in the actual configuration
		*potentialConfigurationChosen = false;
	} else if (answerer_enc_mandatory) {
		*expectedEncryption = answerer_encryption;

		// reINVITE is only sent if offerer and answerer support capability negotiations enabled and the expected encryption is listed in one potential configuration offered by the offerer
		*potentialConfigurationChosen = (offerer_capability_negotiations && answerer_capability_negotiations && linphone_call_params_is_media_encryption_supported(answerer_params, *expectedEncryption));
	} else if (answerer_capability_negotiations && offerer_capability_negotiations) {

		bctbx_list_t* offerer_supported_encs = linphone_call_params_get_supported_encryptions (offerer_params);
		bool_t enc_check_result = FALSE;
		// Find first encryption listed in the list of supported encryptions of the offerer that is supported by the answerer
		for(bctbx_list_t * enc = offerer_supported_encs;enc!=NULL;enc=enc->next){
			const char *enc_string = (const char *)bctbx_list_get_data(enc);
			*expectedEncryption = static_cast<LinphoneMediaEncryption>(string_to_linphone_media_encryption(enc_string));
			enc_check_result |= (linphone_call_params_is_media_encryption_supported (answerer_params, *expectedEncryption) || (linphone_call_params_get_media_encryption(answerer_params) == *expectedEncryption));
			if (enc_check_result) {
				break;
			}
		}

		if (!enc_check_result) {
			*expectedEncryption = linphone_call_params_get_media_encryption(offerer_params);
		}
		// reINVITE is always sent
		*potentialConfigurationChosen = linphone_call_params_is_media_encryption_supported (offerer_params, *expectedEncryption) && (linphone_call_params_get_media_encryption(offerer_params) != *expectedEncryption);

		if (offerer_supported_encs) {
			bctbx_list_free_with_data(offerer_supported_encs, (bctbx_list_free_func)bctbx_free);
		}
	} else {
		*expectedEncryption = linphone_call_params_get_media_encryption(offerer_params);
		// reINVITE is not sent because either parts of the call doesn't support capability negotiations
		*potentialConfigurationChosen = false;
	}
}

std::list<LinphoneMediaEncryption> set_encryption_preference(const bool_t encryption_preferred) {
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

std::list<LinphoneMediaEncryption> set_encryption_preference_with_priority(const LinphoneMediaEncryption encryption, const bool append) {
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

bctbx_list_t * create_confg_encryption_preference_list_except(const LinphoneMediaEncryption encryption) {
	bctbx_list_t * encryption_list = NULL;
	for (int idx = 0; idx <= LinphoneMediaEncryptionDTLS; idx++) {
		if (static_cast<LinphoneMediaEncryption>(idx) != encryption) {
			encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(idx))));
		}
	}
	return encryption_list;
}

bctbx_list_t * create_confg_encryption_preference_list_from_param_preferences(const std::list<LinphoneMediaEncryption> & preferences) {
	bctbx_list_t * encryption_list = NULL;
	for (const auto & enc : preferences) {
		encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(enc)));
	}
	return encryption_list;
}

LinphoneCoreManager * create_core_mgr_with_capability_negotiation_setup(const char * rc_file, const encryption_params enc_params, const bool_t enable_capability_negotiations, const bool_t enable_ice, const bool_t enable_video) {
	LinphoneCoreManager* mgr = linphone_core_manager_new(rc_file);
	linphone_core_set_support_capability_negotiation(mgr->lc, enable_capability_negotiations);

	const LinphoneMediaEncryption encryption = enc_params.encryption;
	if (linphone_core_media_encryption_supported(mgr->lc,encryption)) {
		linphone_core_set_media_encryption_mandatory(mgr->lc,(enc_params.level == E_MANDATORY));

		if ((enc_params.level == E_MANDATORY) || (enc_params.level == E_OPTIONAL)) {
			linphone_core_set_media_encryption(mgr->lc,encryption);
			BC_ASSERT_EQUAL(linphone_core_get_media_encryption(mgr->lc), encryption, int, "%i");
		} else {
			linphone_core_set_media_encryption(mgr->lc,LinphoneMediaEncryptionNone);
			BC_ASSERT_EQUAL(linphone_core_get_media_encryption(mgr->lc), LinphoneMediaEncryptionNone, int, "%i");
		}

		if (!enc_params.preferences.empty()) {
			bctbx_list_t * cfg_enc = create_confg_encryption_preference_list_from_param_preferences(enc_params.preferences);
			linphone_core_set_supported_media_encryptions(mgr->lc,cfg_enc);
			bctbx_list_free_with_data(cfg_enc, (bctbx_list_free_func)bctbx_free);
		}
	}

#ifdef VIDEO_ENABLED
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
#endif // VIDEO_ENABLED

	if (enable_ice){
		enable_stun_in_core(mgr, enable_ice);
		linphone_core_manager_wait_for_stun_resolution(mgr);
	}

	return mgr;
}

void encrypted_call_with_params_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption, const LinphoneCallParams *caller_params, LinphoneCallParams *callee_params, const bool_t enable_video) {

	bool_t caller_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(caller_params);
	bool_t callee_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(callee_params);
	const LinphoneMediaEncryption caller_encryption = linphone_call_params_get_media_encryption(caller_params);
	const LinphoneMediaEncryption callee_encryption = linphone_call_params_get_media_encryption(callee_params);
	if (caller_enc_mandatory && callee_enc_mandatory && (caller_encryption != callee_encryption)) {
		BC_ASSERT_FALSE(call_with_params(caller, callee, caller_params, callee_params));
	} else {

		const bool_t caller_capability_negotiations = linphone_call_params_capability_negotiations_enabled(caller_params);
		const bool_t callee_capability_negotiations = linphone_call_params_capability_negotiations_enabled(callee_params);

		char *path = bc_tester_file("certificates-marie");
		linphone_core_set_user_certificates_path(callee->lc, path);
		bc_free(path);
		path = bc_tester_file("certificates-pauline");
		linphone_core_set_user_certificates_path(caller->lc, path);
		bc_free(path);
		belle_sip_mkdir(linphone_core_get_user_certificates_path(callee->lc));
		belle_sip_mkdir(linphone_core_get_user_certificates_path(caller->lc));

		stats caller_stat = caller->stat;
		stats callee_stat = callee->stat;

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
			potentialConfigurationChosen = (callee_capability_negotiations && caller_capability_negotiations && linphone_call_params_is_media_encryption_supported(caller_params,expectedEncryption) && (linphone_call_params_get_media_encryption(caller_params) != expectedEncryption));
		} else if (callee_capability_negotiations && caller_capability_negotiations && (linphone_call_params_is_media_encryption_supported (caller_params, encryption)) && (linphone_call_params_is_media_encryption_supported (callee_params, encryption))) {
			expectedEncryption = encryption;
			// reINVITE is always sent
			potentialConfigurationChosen = linphone_call_params_is_media_encryption_supported (caller_params, encryption) && (linphone_call_params_get_media_encryption(caller_params) != encryption);
		} else {
			expectedEncryption = linphone_call_params_get_media_encryption(caller_params);
			// reINVITE is not sent because either parts of the call doesn't support capability negotiations
			potentialConfigurationChosen = false;
		}

		LinphoneNatPolicy *caller_nat_policy = linphone_core_get_nat_policy(caller->lc);
		const bool_t caller_ice_enabled = linphone_nat_policy_ice_enabled(caller_nat_policy);
		LinphoneNatPolicy *callee_nat_policy = linphone_core_get_nat_policy(callee->lc);
		const bool_t callee_ice_enabled = linphone_nat_policy_ice_enabled(callee_nat_policy);
		const int expectedStreamsRunning = 1 + ((potentialConfigurationChosen || (callee_ice_enabled && caller_ice_enabled)) ? 1 : 0);

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,(callee_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
		BC_ASSERT_TRUE(wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,(caller_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

		if (callee_ice_enabled && caller_ice_enabled) {
			BC_ASSERT_TRUE(check_ice(caller, callee, LinphoneIceStateHostConnection));
			BC_ASSERT_TRUE(check_ice(callee, caller, LinphoneIceStateHostConnection));
		}

		liblinphone_tester_check_rtcp(caller, callee);

		// Check that no reINVITE is sent while checking streams
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallStreamsRunning, (callee_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%i");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallStreamsRunning, (caller_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%i");

		if (callerCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)), expectedEncryption, int, "%i");
		}
		if (calleeCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)), expectedEncryption, int, "%i");
		}

		LinphoneCall * callee_call = linphone_core_get_current_call(callee->lc);
		BC_ASSERT_PTR_NOT_NULL(callee_call);

		LinphoneCall * caller_call = linphone_core_get_current_call(caller->lc);
		BC_ASSERT_PTR_NOT_NULL(caller_call);

		if (!enable_video) {
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(callee_call)));
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(caller_call)));

			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));
		}
#ifdef VIDEO_ENABLED
		else {
			stats caller_stat = caller->stat;
			stats callee_stat = callee->stat;
			LinphoneCallParams * params = linphone_core_create_call_params(callee->lc, callee_call);
			linphone_call_params_enable_video(params, TRUE);
			linphone_call_update(callee_call, params);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallUpdating,(callee_stat.number_of_LinphoneCallUpdating+1))); BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallUpdatedByRemote,(caller_stat.number_of_LinphoneCallUpdatedByRemote+1)));

			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,(callee_stat.number_of_LinphoneCallStreamsRunning+1)));
			BC_ASSERT_TRUE( wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,(caller_stat.number_of_LinphoneCallStreamsRunning+1)));

			get_expected_encryption_from_call_params(calleeCall, callerCall, &expectedEncryption, &potentialConfigurationChosen);

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
		}
#endif // VIDEO_ENABLED

		// Check that encryption has not changed after sending update
		if (callerCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)), expectedEncryption, int, "%i");
		}
		if (calleeCall) {
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)), expectedEncryption, int, "%i");
		}

		end_call(callee, caller);

	}

}

void encrypted_call_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption, const bool_t enable_caller_capability_negotiations, const bool_t enable_callee_capability_negotiations, const bool_t enable_video) {

	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations (caller_params, enable_caller_capability_negotiations);
	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations (callee_params, enable_callee_capability_negotiations);

	encrypted_call_with_params_base(caller, callee, encryption, caller_params, callee_params, enable_video);

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);

}

void call_with_update_and_incompatible_encs_in_call_params_base (const bool_t enable_ice) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP; // Desired encryption
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

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, enable_ice, TRUE);

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

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE, enable_ice, TRUE);

	bctbx_list_t * marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);
	linphone_call_params_set_media_encryption (marie_params, LinphoneMediaEncryptionDTLS);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);

	bctbx_list_t * pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, 1);
	linphone_call_params_set_media_encryption (pauline_params, encryption);
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);
	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);

	// Offerer media description:
	// - actual configuration: DTLS
	// - potential configuration: SRTP
	// Answerer media description:
	// - actual configuration: SRTP
	// - potential configuration: ZRTP
	// Result: potential configuration is chosen (encryption SRTP)
	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, 2, int, "%d");

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	stats marie_stat = marie->stat; 
	stats pauline_stat = pauline->stat; 
	LinphoneCallParams * params = linphone_core_create_call_params(pauline->lc, paulineCall);
	linphone_call_params_enable_video(params, TRUE);
	// Offerer media description:
	// - actual configuration: SRTP
	// - potential configuration: ZRTP
	// Answerer media description:
	// - actual configuration: DTLS
	// - potential configuration: SRTP
	// Result: actual configuration is chosen (encryption SRTP)
	linphone_call_update(paulineCall, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdating,(pauline_stat.number_of_LinphoneCallUpdating+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallUpdatedByRemote,(marie_stat.number_of_LinphoneCallUpdatedByRemote+1)));

	LinphoneNatPolicy *marie_nat_policy = linphone_core_get_nat_policy(marie->lc);
	const bool_t marie_ice_enabled = linphone_nat_policy_ice_enabled(marie_nat_policy);
	LinphoneNatPolicy *pauline_nat_policy = linphone_core_get_nat_policy(pauline->lc);
	const bool_t pauline_ice_enabled = linphone_nat_policy_ice_enabled(pauline_nat_policy);
	const int expectedStreamsRunning = 1 + ((pauline_ice_enabled && marie_ice_enabled) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,(pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,(marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

	if (pauline_ice_enabled && marie_ice_enabled) {
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_IframeDecoded,(pauline_stat.number_of_IframeDecoded+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_IframeDecoded,(marie_stat.number_of_IframeDecoded+1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, (pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, (marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_with_encryption_test_base(const encryption_params marie_enc_params, const bool_t enable_marie_capability_negotiations, const bool_t enable_marie_ice, const encryption_params pauline_enc_params, const bool_t enable_pauline_capability_negotiations, const bool_t enable_pauline_ice, const bool_t enable_video) {

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_params, enable_marie_capability_negotiations, enable_marie_ice, enable_video);
	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_params, enable_pauline_capability_negotiations, enable_pauline_ice, enable_video);

	LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
	if (marie_enc_params.level == E_MANDATORY) {
		expectedEncryption = marie_enc_params.encryption;
	} else if (pauline_enc_params.level == E_MANDATORY) {
		expectedEncryption = pauline_enc_params.encryption;
	} else if ((enable_marie_capability_negotiations == TRUE) && (enable_pauline_capability_negotiations == TRUE)) {
		for (const auto & enc : marie_enc_params.preferences) {
			if ((std::find(pauline_enc_params.preferences.cbegin(), pauline_enc_params.preferences.cend(), enc) != pauline_enc_params.preferences.cend()) || (pauline_enc_params.encryption == enc)) {
				expectedEncryption = enc;
				break;
			}
		}
	} else if (enable_marie_capability_negotiations == TRUE) {
		expectedEncryption = pauline_enc_params.encryption;
	} else {
		expectedEncryption = marie_enc_params.encryption;
	}

	encrypted_call_base(marie, pauline, expectedEncryption, enable_marie_capability_negotiations, enable_pauline_capability_negotiations, enable_video);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
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

	encrypted_call_base(marie, pauline, marieOptionalEncryption, TRUE, TRUE, FALSE);

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

	encrypted_call_base(marie, pauline, marieOptionalEncryption0, TRUE, TRUE, FALSE);

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

	encrypted_call_base(marie, pauline, LinphoneMediaEncryptionNone, FALSE, FALSE, FALSE);

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

	encrypted_call_base(marie, pauline, static_cast<LinphoneMediaEncryption>(string_to_linphone_media_encryption(chosenMediaEnc)), TRUE, TRUE, FALSE);
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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(marie->lc, marieEncryption));
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(marie->lc, paulineEncryption));

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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, marieEncryption));
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, paulineEncryption));

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

	bctbx_list_t* pauline_supported_encs = linphone_call_params_get_supported_encryptions (pauline_params);
	BC_ASSERT_EQUAL(bctbx_list_size(pauline_supported_encs), 1, int, "%d");
	if (pauline_supported_encs) {
		bctbx_list_free_with_data(pauline_supported_encs, (bctbx_list_free_func)bctbx_free);
	}

	bctbx_list_t* marie_supported_encs = linphone_call_params_get_supported_encryptions (marie_params);
	BC_ASSERT_EQUAL(bctbx_list_size(marie_supported_encs), 1, int, "%d");
	if (marie_supported_encs) {
		bctbx_list_free_with_data(marie_supported_encs, (bctbx_list_free_func)bctbx_free);
	}

	BC_ASSERT_TRUE(call_with_params_and_encryption_negotiation_failure_base(marie, pauline, marie_params, pauline_params, TRUE));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), defaultEncryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), defaultEncryption, int, "%i");
	}

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void call_with_video_and_capability_negotiation_base(const LinphoneMediaEncryption encryption) {

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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(marie->lc, encryption));

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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, encryption));

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
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
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

static void srtp_call_with_video_and_capability_negotiation(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP; // Desired encryption
	call_with_video_and_capability_negotiation_base(encryption);
}

static void zrtp_call_with_video_and_capability_negotiation(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionZRTP; // Desired encryption
	call_with_video_and_capability_negotiation_base(encryption);
}

static void dtls_srtp_call_with_video_and_capability_negotiation(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Desired encryption
	call_with_video_and_capability_negotiation_base(encryption);
}

static void call_with_tcap_line_merge_base(const bool_t caller_tcap_merge, const bool_t callee_tcap_merge) {
	encryption_params caller_enc_mgr_params;
	caller_enc_mgr_params.encryption = LinphoneMediaEncryptionZRTP;
	caller_enc_mgr_params.level = E_OPTIONAL;
	caller_enc_mgr_params.preferences = set_encryption_preference(TRUE);

	encryption_params callee_enc_mgr_params;
	callee_enc_mgr_params.encryption = LinphoneMediaEncryptionDTLS;
	callee_enc_mgr_params.level = E_OPTIONAL;
	callee_enc_mgr_params.preferences = set_encryption_preference(FALSE);

	LinphoneCoreManager * caller = create_core_mgr_with_capability_negotiation_setup("marie_rc", caller_enc_mgr_params, TRUE, FALSE, TRUE);
	LinphoneCoreManager * callee = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), callee_enc_mgr_params, TRUE, FALSE, TRUE);

	// Caller call params:
	// - RFC5939 is supported
	// - Merge tcap lines
	// - Default encryption SRTP
	// - No mandatory encryption
	// - Supported optional encryptions: DTLS, ZRTP
	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations (caller_params, TRUE);
	bctbx_list_t * caller_cfg_enc = NULL;
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	linphone_call_params_set_supported_encryptions(caller_params,caller_cfg_enc);
	bctbx_list_free_with_data(caller_cfg_enc, (bctbx_list_free_func)bctbx_free);
	linphone_call_params_set_media_encryption (caller_params, LinphoneMediaEncryptionSRTP);
	linphone_call_params_enable_mandatory_media_encryption(caller_params,0);
	linphone_call_params_enable_tcap_line_merging(caller_params, TRUE);

	// Callee call params:
	// - RFC5939 is supported
	// - Merge tcap lines
	// - Default encryption SRTP
	// - No mandatory encryption
	// - Supported optional encryptions: SRTP, ZRTP, DTLS
	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations (callee_params, TRUE);
	bctbx_list_t * callee_cfg_enc = NULL;
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionSRTP)));
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));
	linphone_call_params_set_supported_encryptions(callee_params,callee_cfg_enc);
	bctbx_list_free_with_data(callee_cfg_enc, (bctbx_list_free_func)bctbx_free);
	linphone_call_params_set_media_encryption (callee_params, LinphoneMediaEncryptionNone);
	linphone_call_params_enable_mandatory_media_encryption(callee_params,0);
	linphone_call_params_enable_tcap_line_merging(callee_params, TRUE);

	const LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionDTLS;
	encrypted_call_with_params_base(caller, callee, expectedEncryption, caller_params, callee_params, TRUE);

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);

	linphone_core_manager_destroy(caller);
	linphone_core_manager_destroy(callee);
}

static void call_with_tcap_line_merge_on_caller(void) {
	call_with_tcap_line_merge_base(TRUE, FALSE);
}

static void call_with_tcap_line_merge_on_callee(void) {
	call_with_tcap_line_merge_base(FALSE, TRUE);
}

static void call_with_tcap_line_merge_on_both_sides(void) {
	call_with_tcap_line_merge_base(TRUE, TRUE);
}

static void call_with_no_sdp_on_update_base (const bool_t caller_cap_neg, const bool_t callee_cap_neg) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP; // Desired encryption

	LinphoneMediaEncryption marieEncryption = LinphoneMediaEncryptionNone;
	if (caller_cap_neg && callee_cap_neg) {
		marieEncryption = LinphoneMediaEncryptionDTLS;
	} else {
		marieEncryption = encryption;
	}

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

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, caller_cap_neg, FALSE, TRUE);

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

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, callee_cap_neg, FALSE, TRUE);

	bctbx_list_t * marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, caller_cap_neg);
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);

	linphone_call_params_set_media_encryption (marie_params, marieEncryption);

	bctbx_list_t * pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, callee_cap_neg);
	linphone_call_params_set_media_encryption (pauline_params, encryption);
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);

	// Different scenarios:
	// ==================== OFFERER =====================
	// If offerer and answerer supports capability negotiations:
	// - actual configuration: DTLS
	// - potential configuration: SRTP
	// If offerer only supports capability negotiations:
	// - actual configuration: SRTP
	// - potential configuration: SRTP
	// If offerer doesn't support capability negotiations:
	// - actual configuration: SRTP
	// ==================== ANSWERER =====================
	// If answerer supports capability negotiations:
	// - actual configuration: SRTP
	// - potential configuration: ZRTP
	// If answerer doesn't support capability negotiations:
	// - actual configuration: SRTP
	// ==================== RESULT =====================
	// Result: encryption SRTP is chosen
	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	LinphoneNatPolicy *marie_nat_policy = linphone_core_get_nat_policy(marie->lc);
	const bool_t marie_ice_enabled = linphone_nat_policy_ice_enabled(marie_nat_policy);
	LinphoneNatPolicy *pauline_nat_policy = linphone_core_get_nat_policy(pauline->lc);
	const bool_t pauline_ice_enabled = linphone_nat_policy_ice_enabled(pauline_nat_policy);

	bool potentialConfigurationChosen = (caller_cap_neg && callee_cap_neg);
	bool sendReInvite = (potentialConfigurationChosen || (marie_ice_enabled && pauline_ice_enabled));
	int expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	ms_message("Pauline sends a reINVITE without SDP");
	// Pauline send an empty SDP for the update
	linphone_core_enable_sdp_200_ack(pauline->lc,TRUE);

	stats marie_stat = marie->stat; 
	stats pauline_stat = pauline->stat; 
	LinphoneCallParams * params0 = linphone_core_create_call_params(pauline->lc, paulineCall);
	BC_ASSERT_TRUE(linphone_core_sdp_200_ack_enabled(pauline->lc));
	linphone_call_update(paulineCall, params0);
	linphone_call_params_unref(params0);
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdating,(pauline_stat.number_of_LinphoneCallUpdating+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallUpdatedByRemote,(marie_stat.number_of_LinphoneCallUpdatedByRemote+1)));

	LinphoneMediaEncryption expectedEncryption =  LinphoneMediaEncryptionNone;
	bool dummyPotentialConfigurationChosen = false;
	get_expected_encryption_from_call_params(paulineCall, marieCall, &expectedEncryption, &dummyPotentialConfigurationChosen);
	// As Pauline sends a reINVITE without SDP, Marie replies with the same SDP as she previously sent hence it has the same capability negotiation flags as the previous answer to the INVITE.
	// A reINVITE is sent again if capability negotiations are sent
	sendReInvite = potentialConfigurationChosen || (marie_ice_enabled && pauline_ice_enabled);
	expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,(pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,(marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

	linphone_core_enable_sdp_200_ack(pauline->lc,FALSE);

	liblinphone_tester_check_rtcp(marie, pauline);

	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, (pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, (marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");

	marie_stat = marie->stat; 
	pauline_stat = pauline->stat; 
	LinphoneCallParams * params1 = linphone_core_create_call_params(pauline->lc, paulineCall);
	linphone_call_params_enable_video(params1, TRUE);
	BC_ASSERT_FALSE(linphone_core_sdp_200_ack_enabled(pauline->lc));
	ms_message("Pauline enables video");
	linphone_call_update(paulineCall, params1);
	linphone_call_params_unref(params1);
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdating,(pauline_stat.number_of_LinphoneCallUpdating+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallUpdatedByRemote,(marie_stat.number_of_LinphoneCallUpdatedByRemote+1)));

	get_expected_encryption_from_call_params(paulineCall, marieCall, &expectedEncryption, &potentialConfigurationChosen);
	expectedStreamsRunning = 1 + ((potentialConfigurationChosen) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,(pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,(marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

	if (pauline_ice_enabled && marie_ice_enabled) {
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_IframeDecoded,(pauline_stat.number_of_IframeDecoded+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_IframeDecoded,(marie_stat.number_of_IframeDecoded+1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	BC_ASSERT_EQUAL(expectedEncryption, encryption, int, "%i");
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, (pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, (marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");

	// Marie send an empty SDP for the update
	linphone_core_enable_sdp_200_ack(marie->lc,TRUE);

	marie_stat = marie->stat; 
	pauline_stat = pauline->stat; 
	LinphoneCallParams * params2 = linphone_core_create_call_params(marie->lc, marieCall);
	linphone_call_params_set_media_encryption (params2, marieEncryption);
	BC_ASSERT_TRUE(linphone_core_sdp_200_ack_enabled(marie->lc));
	ms_message("Marie sends a reINVITE without SDP");
	linphone_call_update(marieCall, params2);
	linphone_call_params_unref(params2);
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallUpdating,(marie_stat.number_of_LinphoneCallUpdating+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdatedByRemote,(pauline_stat.number_of_LinphoneCallUpdatedByRemote+1)));

	get_expected_encryption_from_call_params(marieCall, paulineCall, &expectedEncryption, &potentialConfigurationChosen);
	expectedStreamsRunning = 1 + ((potentialConfigurationChosen) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,(marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,(pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

	linphone_core_enable_sdp_200_ack(marie->lc,FALSE);

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_IframeDecoded,(pauline_stat.number_of_IframeDecoded+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_IframeDecoded,(marie_stat.number_of_IframeDecoded+1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, (pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, (marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");

	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_on_update_cap_neg_caller (void) {
	call_with_no_sdp_on_update_base (TRUE, FALSE);
}

static void call_with_no_sdp_on_update_cap_neg_callee (void) {
	call_with_no_sdp_on_update_base (FALSE, TRUE);
}

static void call_with_no_sdp_on_update_cap_neg_both_sides (void) {
	call_with_no_sdp_on_update_base (TRUE, TRUE);
}

static void call_changes_enc_on_update_base (const bool_t caller_cap_neg, const bool_t callee_cap_neg) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP; // Desired encryption
	LinphoneMediaEncryption expectedEncryption = encryption; // Expected encryption

	LinphoneMediaEncryption marieEncryption = LinphoneMediaEncryptionNone;
	if (caller_cap_neg && callee_cap_neg) {
		marieEncryption = LinphoneMediaEncryptionDTLS;
	} else {
		marieEncryption = encryption;
	}

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

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, caller_cap_neg, FALSE, TRUE);

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

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, callee_cap_neg, FALSE, TRUE);

	bctbx_list_t * marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, caller_cap_neg);
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);

	linphone_call_params_set_media_encryption (marie_params, marieEncryption);

	bctbx_list_t * pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, callee_cap_neg);
	linphone_call_params_set_media_encryption (pauline_params, encryption);
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);
	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);

	// Different scenarios:
	// ==================== OFFERER =====================
	// If offerer and answerer supports capability negotiations:
	// - actual configuration: DTLS
	// - potential configuration: SRTP and DTLS
	// If offerer only supports capability negotiations:
	// - actual configuration: SRTP
	// - potential configuration: SRTP and DTLS
	// If offerer doesn't support capability negotiations:
	// - actual configuration: SRTP
	// ==================== ANSWERER =====================
	// If answerer supports capability negotiations:
	// - actual configuration: SRTP
	// - potential configuration: ZRTP
	// If answerer doesn't support capability negotiations:
	// - actual configuration: SRTP
	// ==================== RESULT =====================
	// Result: encryption SRTP is chosen
	ms_message("SRTP Call from Marie to Pauline");
	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	LinphoneNatPolicy *marie_nat_policy = linphone_core_get_nat_policy(marie->lc);
	const bool_t marie_ice_enabled = linphone_nat_policy_ice_enabled(marie_nat_policy);
	LinphoneNatPolicy *pauline_nat_policy = linphone_core_get_nat_policy(pauline->lc);
	const bool_t pauline_ice_enabled = linphone_nat_policy_ice_enabled(pauline_nat_policy);

	bool sendReInvite = ((caller_cap_neg && callee_cap_neg) || (marie_ice_enabled && pauline_ice_enabled));

	const int expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), expectedEncryption, int, "%i");
	}

	// If capability negotiation is enabled on both sides, DTLS is chosen
	// Callee:
	// Optional encryptions: DTLS and ZRTP
	// Default: SRTP
	// Caller:
	// Optional encryptions: SRTP and DTLS
	// Default: SRTP if capability negotiation is disabled, DTLS otherwise
	stats marie_stat = marie->stat; 
	stats pauline_stat = pauline->stat; 
	LinphoneCallParams * params0 = linphone_core_create_call_params(pauline->lc, paulineCall);
	linphone_call_params_set_media_encryption (params0, encryption);
	linphone_call_params_enable_video(params0, TRUE);
	bctbx_list_t * pauline_call_enc0 = NULL;
	if (caller_cap_neg && callee_cap_neg) {
		expectedEncryption = LinphoneMediaEncryptionDTLS;
	}
	ms_message("Pauline changes encryption to %s", linphone_media_encryption_to_string(expectedEncryption));
	pauline_call_enc0 = bctbx_list_append(pauline_call_enc0, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));
	pauline_call_enc0 = bctbx_list_append(pauline_call_enc0, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	linphone_call_params_set_supported_encryptions (params0, pauline_call_enc0);
	bctbx_list_free_with_data(pauline_call_enc0, (bctbx_list_free_func)bctbx_free);
	linphone_call_update(paulineCall, params0);
	linphone_call_params_unref(params0);
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdating,(pauline_stat.number_of_LinphoneCallUpdating+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallUpdatedByRemote,(marie_stat.number_of_LinphoneCallUpdatedByRemote+1)));

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,(pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,(marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

	if (pauline_ice_enabled && marie_ice_enabled) {
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_IframeDecoded,(pauline_stat.number_of_IframeDecoded+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_IframeDecoded,(marie_stat.number_of_IframeDecoded+1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, (pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, (marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");

	// If capability negotiation is enabled on both sides, DTLS is chosen
	// Callee:
	// Optional encryptions: DTLS and ZRTP
	// Default: SRTP
	// Caller:
	// Optional encryptions: ZRTP and DTLS
	// Default: SRTP if capability negotiation is disabled, DTLS otherwise
	marie_stat = marie->stat; 
	pauline_stat = pauline->stat; 
	LinphoneCallParams * params1 = linphone_core_create_call_params(marie->lc, marieCall);
	linphone_call_params_set_media_encryption (params1, marieEncryption);
	bctbx_list_t * marie_call_enc1 = NULL;
	if (caller_cap_neg && callee_cap_neg) {
		expectedEncryption = LinphoneMediaEncryptionZRTP;
	}
	ms_message("Marie changes encryption to %s", linphone_media_encryption_to_string(expectedEncryption));
	marie_call_enc1 = bctbx_list_append(marie_call_enc1, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	marie_call_enc1 = bctbx_list_append(marie_call_enc1, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));
	linphone_call_params_set_supported_encryptions (params1, marie_call_enc1);
	bctbx_list_free_with_data(marie_call_enc1, (bctbx_list_free_func)bctbx_free);
	linphone_call_update(marieCall, params1);
	linphone_call_params_unref(params1);
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallUpdating,(marie_stat.number_of_LinphoneCallUpdating+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdatedByRemote,(pauline_stat.number_of_LinphoneCallUpdatedByRemote+1)));

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,(marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,(pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning)));

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_IframeDecoded,(pauline_stat.number_of_IframeDecoded+1)));
	BC_ASSERT_TRUE( wait_for(pauline->lc,marie->lc,&marie->stat.number_of_IframeDecoded,(marie_stat.number_of_IframeDecoded+1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, (pauline_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, (marie_stat.number_of_LinphoneCallStreamsRunning+expectedStreamsRunning), int, "%d");

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_changes_enc_on_update_cap_neg_caller (void) {
	call_changes_enc_on_update_base (TRUE, FALSE);
}

static void call_changes_enc_on_update_cap_neg_callee (void) {
	call_changes_enc_on_update_base (FALSE, TRUE);
}

static void call_changes_enc_on_update_cap_neg_both_sides (void) {
	call_changes_enc_on_update_base (TRUE, TRUE);
}

static void back_to_back_calls_cap_neg_base(bool_t enable_cap_neg_both_sides) {
	std::list<LinphoneMediaEncryption> marie_enc_list;
	marie_enc_list.push_back(LinphoneMediaEncryptionNone);

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionZRTP;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager * marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	pauline_enc_list.push_back(LinphoneMediaEncryptionNone);

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionSRTP;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager * pauline = create_core_mgr_with_capability_negotiation_setup((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE, FALSE, TRUE);

	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Expected encryption
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	bctbx_list_t * marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionSRTP)));
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);
	marie_call_enc = NULL;

	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, enable_cap_neg_both_sides);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	bctbx_list_t * pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionNone)));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);
	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
	pauline_call_enc = NULL;

	// TODO: Delete the following 2 lines when relying only on call params to check validity of SDP
	linphone_core_set_media_encryption(pauline->lc,encryption);
	linphone_core_set_media_encryption(marie->lc,encryption);

	// Caller (Marie) supports SRTP and DTLS as potential configurations and DTLS as default one
	// Callee (Pauline) supports ZRTP, no encryption and DTLS as potential configurations and DTLS as default one
	// DTLS is expected to be chosen
	encrypted_call_with_params_base(marie, pauline, encryption, marie_params, pauline_params, TRUE);

	encryption = LinphoneMediaEncryptionSRTP;
	if (marie_params) {
		linphone_call_params_unref(marie_params);
		marie_params = NULL;
	}
	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	if (marie_call_enc) {
		bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);
		marie_call_enc = NULL;
	}
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);
	marie_call_enc = NULL;

	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
		pauline_params = NULL;
	}
	pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, enable_cap_neg_both_sides);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	if (pauline_call_enc) {
		bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
		pauline_call_enc = NULL;
	}
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionNone)));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);
	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
	pauline_call_enc = NULL;

	// TODO: Delete the following 2 lines when relying only on call params to check validity of SDP
	linphone_core_set_media_encryption(pauline->lc,encryption);
	linphone_core_set_media_encryption(marie->lc,encryption);

	// Caller (Pauline) supports DTLS, no encryption and SRTP as potential configurations and SRTP as default one
	// Callee (Marie) supports ZRTP and SRTP as potential configurations and SRTP as default one
	// DTLS is expected to be chosen
	encrypted_call_with_params_base(pauline, marie, encryption, marie_params, pauline_params, TRUE);

	encryption = LinphoneMediaEncryptionZRTP;
	if (marie_params) {
		linphone_call_params_unref(marie_params);
		marie_params = NULL;
	}
	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	if (marie_call_enc) {
		bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);
		marie_call_enc = NULL;
	}
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionNone)));
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionZRTP)));
	marie_call_enc = bctbx_list_append(marie_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_call_params_set_supported_encryptions (marie_params, marie_call_enc);
	bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);
	marie_call_enc = NULL;

	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
		pauline_params = NULL;
	}
	pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, enable_cap_neg_both_sides);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	if (pauline_call_enc) {
		bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
		pauline_call_enc = NULL;
	}
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(LinphoneMediaEncryptionDTLS)));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(encryption))));
	linphone_call_params_set_supported_encryptions (pauline_params, pauline_call_enc);
	bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
	pauline_call_enc = NULL;

	// TODO: Delete the following 2 lines when relying only on call params to check validity of SDP
	linphone_core_set_media_encryption(pauline->lc,encryption);
	linphone_core_set_media_encryption(marie->lc,encryption);

	// Caller (Marie) supports SRTP, DTLS and ZRTP as potential configurations and ZRTP as default one
	// Callee (Pauline) supports no encryption and ZRTP as potential configurations and ZRTP as default one
	// DTLS is expected to be chosen
	encrypted_call_with_params_base(pauline, marie, encryption, marie_params, pauline_params, TRUE);

	if (marie_call_enc) {
		bctbx_list_free_with_data(marie_call_enc, (bctbx_list_free_func)bctbx_free);
		marie_call_enc = NULL;
	}
	if (pauline_call_enc) {
		bctbx_list_free_with_data(pauline_call_enc, (bctbx_list_free_func)bctbx_free);
		pauline_call_enc = NULL;
	}
	if (marie_params) {
		linphone_call_params_unref(marie_params);
		marie_params = NULL;
	}
	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
		pauline_params = NULL;
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void back_to_back_calls_cap_neg_one_side(void) {
	back_to_back_calls_cap_neg_base(FALSE);
}

static void back_to_back_calls_cap_neg_both_sides(void) {
	back_to_back_calls_cap_neg_base(TRUE);
}

static void call_with_update_and_incompatible_encs_in_call_params (void) {
	call_with_update_and_incompatible_encs_in_call_params_base (FALSE);
}

static void call_with_encryption_supported_in_call_params_only_base(const LinphoneMediaEncryption encryption) {
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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(marie->lc, encryption));

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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, encryption));

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
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)), encryption, int, "%i");
	}
	if (paulineCall) {
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)), encryption, int, "%i");
	}

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void srtp_call_with_encryption_supported_in_call_params_only(void) {
	call_with_encryption_supported_in_call_params_only_base(LinphoneMediaEncryptionSRTP);
}

static void zrtp_call_with_encryption_supported_in_call_params_only(void) {
	call_with_encryption_supported_in_call_params_only_base(LinphoneMediaEncryptionZRTP);
}

static void dtls_srtp_call_with_encryption_supported_in_call_params_only(void) {
	call_with_encryption_supported_in_call_params_only_base(LinphoneMediaEncryptionDTLS);
}

static void call_with_default_encryption(const LinphoneMediaEncryption encryption) {

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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(marie->lc, encryption));

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
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, encryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations (marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations (pauline_params, 1);
	linphone_call_params_set_media_encryption(pauline_params, encryption);

	encrypted_call_with_params_base(marie, pauline, marie_enc_list.front(), marie_params, pauline_params, TRUE);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void call_with_srtp_default_encryption(void) {
	call_with_default_encryption(LinphoneMediaEncryptionSRTP);
}

static void call_with_zrtp_default_encryption(void) {
	call_with_default_encryption(LinphoneMediaEncryptionZRTP);
}

static void call_with_dtls_srtp_default_encryption(void) {
	call_with_default_encryption(LinphoneMediaEncryptionDTLS);
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

	encrypted_call_base(marie, pauline, encryption, TRUE, TRUE, FALSE);

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

static void simple_call_with_capability_negotiations(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption optionalEncryption) {
	linphone_core_set_support_capability_negotiation(caller->lc, 1);
	linphone_core_set_support_capability_negotiation(callee->lc, 1);
	bctbx_list_t * encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(optionalEncryption))));

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc,optionalEncryption));
	if (linphone_core_media_encryption_supported(callee->lc,optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(callee->lc,0);
		linphone_core_set_media_encryption(callee->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(callee->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(callee->lc, optionalEncryption));
	}

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc,optionalEncryption));
	if (linphone_core_media_encryption_supported(caller->lc,optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(caller->lc,0);
		linphone_core_set_media_encryption(caller->lc,LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(caller->lc,encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(caller->lc, optionalEncryption));
	}

	if (encryption_list) {
		bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
	}

	encrypted_call_base(caller, callee, optionalEncryption, TRUE, TRUE, FALSE);
}

static void simple_srtp_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_srtp_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_srtpsuite_unencrypted_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_srtpsuite_unencrypted_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_capability_negotiations_caller_unencrypted(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_srtpsuite_unencrypted_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_capability_negotiations_callee_unencrypted(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_srtpsuite_unencrypted_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_sas_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_b256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_b256_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_sas_call_with_capability_negotiations_default_keys_on_callee(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_b256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_cipher_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_srtpsuite_aes256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_srtpsuite_aes256_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_cipher_call_with_capability_negotiations_aes256(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_aes256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_aes256_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_call_with_different_cipher_suites_and_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_aes256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_ecdh255_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_cipher_call_with_capability_negotiations_default_keys_on_callee(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_aes256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_key_agreement_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_ecdh255_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_ecdh255_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionDTLS);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void encrypted_call_with_suite_mismatch_and_capability_negotiations_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption optionalEncryption) {

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc,optionalEncryption));
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc,optionalEncryption));

	bctbx_list_t * encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, ms_strdup(linphone_media_encryption_to_string(static_cast<LinphoneMediaEncryption>(optionalEncryption))));

	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations (caller_params, TRUE);
	linphone_call_params_set_supported_encryptions(caller_params,encryption_list);
	linphone_call_params_enable_mandatory_media_encryption(caller_params,0);
	linphone_call_params_set_media_encryption(caller_params,LinphoneMediaEncryptionNone);
	BC_ASSERT_TRUE(linphone_call_params_is_media_encryption_supported(caller_params, optionalEncryption));

	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations (callee_params, TRUE);
	linphone_call_params_set_supported_encryptions(callee_params,encryption_list);
	linphone_call_params_enable_mandatory_media_encryption(callee_params,0);
	linphone_call_params_set_media_encryption(callee_params,LinphoneMediaEncryptionNone);
	BC_ASSERT_TRUE(linphone_call_params_is_media_encryption_supported(callee_params, optionalEncryption));

	if (encryption_list) {
		bctbx_list_free_with_data(encryption_list, (bctbx_list_free_func)bctbx_free);
	}


	BC_ASSERT_FALSE(call_with_params(caller, callee, caller_params, callee_params));

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);
}

static void srtp_call_with_suite_mismatch_and_capability_negotiations_caller_unencrypted(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_srtpsuite_unencrypted_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_srtpsuite_aes256_rc");
	encrypted_call_with_suite_mismatch_and_capability_negotiations_base(marie, pauline, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_suite_mismatch_and_capability_negotiations_callee_unencrypted(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_srtpsuite_aes256_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_srtpsuite_unencrypted_rc");
	encrypted_call_with_suite_mismatch_and_capability_negotiations_base(marie, pauline, LinphoneMediaEncryptionSRTP);
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
	// Avoid setting the actual configuration with the same encryption as the desired one
	if (encryption == LinphoneMediaEncryptionSRTP) {
		optional_enc_mgr_params.encryption = LinphoneMediaEncryptionDTLS;
	} else {
		optional_enc_mgr_params.encryption = LinphoneMediaEncryptionSRTP;
	}
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
	// Avoid setting the actual configuration with the same encryption as the desired one
	if (encryption == LinphoneMediaEncryptionSRTP) {
		marie_enc_params.encryption = LinphoneMediaEncryptionDTLS;
	} else {
		marie_enc_params.encryption = LinphoneMediaEncryptionSRTP;
	}
	marie_enc_params.level = E_OPTIONAL;
	marie_enc_params.preferences = set_encryption_preference_with_priority(encryption, false);

	encryption_params pauline_enc_params;
	// Avoid setting the actual configuration with the same encryption as the desired one
	if (encryption == LinphoneMediaEncryptionZRTP) {
		pauline_enc_params.encryption = LinphoneMediaEncryptionZRTP;
	} else {
		pauline_enc_params.encryption = LinphoneMediaEncryptionDTLS;
	}
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

test_t capability_negotiation_tests[] = {
	TEST_NO_TAG("Call with no encryption", call_with_no_encryption),
	TEST_NO_TAG("Call with capability negotiation failure", call_with_capability_negotiation_failure),
	TEST_NO_TAG("Call with capability negotiation failure and multiple potential configurations", call_with_capability_negotiation_failure_multiple_potential_configurations),
	TEST_NO_TAG("Call with capability negotiation disabled at call level", call_with_capability_negotiation_disable_call_level),
	TEST_NO_TAG("Call with capability negotiation disabled at core level", call_with_capability_negotiation_disable_core_level),
	TEST_NO_TAG("Call with incompatible encryptions in call params", call_with_incompatible_encs_in_call_params),
	TEST_NO_TAG("Call with update and incompatible encryptions in call params", call_with_update_and_incompatible_encs_in_call_params),
	TEST_NO_TAG("Call with default SRTP default encryption", call_with_srtp_default_encryption),
	TEST_NO_TAG("Call with default ZRTP default encryption", call_with_zrtp_default_encryption),
	TEST_NO_TAG("Call with default DTLS SRTP default encryption", call_with_dtls_srtp_default_encryption),
	TEST_NO_TAG("Call with tcap line merge on caller", call_with_tcap_line_merge_on_caller),
	TEST_NO_TAG("Call with tcap line merge on callee", call_with_tcap_line_merge_on_callee),
	TEST_NO_TAG("Call with tcap line merge on both sides", call_with_tcap_line_merge_on_both_sides),
	TEST_NO_TAG("Call with no SDP on update and capability negotiations on caller", call_with_no_sdp_on_update_cap_neg_caller),
	TEST_NO_TAG("Call with no SDP on update and capability negotiations on callee", call_with_no_sdp_on_update_cap_neg_callee),
	TEST_NO_TAG("Call with no SDP on update and capability negotiations on both sides", call_with_no_sdp_on_update_cap_neg_both_sides),
	TEST_NO_TAG("Call changes encryption with update and capability negotiations on caller", call_changes_enc_on_update_cap_neg_caller),
	TEST_NO_TAG("Call changes encryption with update and capability negotiations on callee", call_changes_enc_on_update_cap_neg_callee),
	TEST_NO_TAG("Call changes encryption with update and capability negotiations on both sides", call_changes_enc_on_update_cap_neg_both_sides),
	TEST_NO_TAG("Unencrypted call with potential configuration same as actual one", unencrypted_call_with_potential_configuration_same_as_actual_configuration),
	TEST_NO_TAG("Back to back call with capability negotiations on one side", back_to_back_calls_cap_neg_one_side),
	TEST_NO_TAG("Back to back call with capability negotiations on both sides", back_to_back_calls_cap_neg_both_sides),
	TEST_NO_TAG("Simple SRTP call with capability negotiations", simple_srtp_call_with_capability_negotiations),
	TEST_NO_TAG("SRTP unencrypted call and capability negotiations", unencrypted_srtp_call_with_capability_negotiations),
	TEST_NO_TAG("SRTP call and capability negotiations (caller unencrypted)", srtp_call_with_capability_negotiations_caller_unencrypted),
	TEST_NO_TAG("SRTP call and capability negotiations (callee unencrypted)", srtp_call_with_capability_negotiations_callee_unencrypted),
	TEST_NO_TAG("SRTP call with suite mismatch and capability negotiations (caller unencrypted)", srtp_call_with_suite_mismatch_and_capability_negotiations_caller_unencrypted),
	TEST_NO_TAG("SRTP call with suite mismatch and capability negotiations (callee unencrypted)", srtp_call_with_suite_mismatch_and_capability_negotiations_callee_unencrypted),
	TEST_NO_TAG("SRTP call with different encryptions in call params", srtp_call_with_encryption_supported_in_call_params_only),
	TEST_NO_TAG("SRTP call started with video and capability negotiation", srtp_call_with_video_and_capability_negotiation),
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
	TEST_NO_TAG("SRTP video call with optional encryption on caller", srtp_video_call_with_optional_encryption_on_caller),
	TEST_NO_TAG("SRTP video call with optional encryption on callee", srtp_video_call_with_optional_encryption_on_callee),
	TEST_NO_TAG("SRTP video call with optional encryption on both sides", srtp_video_call_with_optional_encryption_on_both_sides),
	TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations", simple_dtls_srtp_call_with_capability_negotiations, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with different encryptions in call params", dtls_srtp_call_with_encryption_supported_in_call_params_only, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call started with video and capability negotiation", dtls_srtp_call_with_video_and_capability_negotiation, "DTLS"),
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
	TEST_ONE_TAG("DTLS SRTP call with optional encryption on both sides", dtls_srtp_call_with_optional_encryption_on_both_sides, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP video call with optional encryption on caller", dtls_srtp_video_call_with_optional_encryption_on_caller, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP video call with optional encryption on callee", dtls_srtp_video_call_with_optional_encryption_on_callee, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP video call with optional encryption on both sides", dtls_srtp_video_call_with_optional_encryption_on_both_sides, "DTLS"),
	TEST_NO_TAG("Simple ZRTP call with capability negotiations", simple_zrtp_call_with_capability_negotiations),
	TEST_NO_TAG("ZRTP call with differet cipher suites and capability negotiations", zrtp_call_with_different_cipher_suites_and_capability_negotiations),
	TEST_NO_TAG("ZRTP cipher call with capability negotiations default keys on callee", zrtp_cipher_call_with_capability_negotiations_default_keys_on_callee),
	TEST_NO_TAG("ZRTP cipher call with capability negotiations aes256", zrtp_cipher_call_with_capability_negotiations_aes256),
	TEST_NO_TAG("ZRTP cipher call with capability negotiations", zrtp_cipher_call_with_capability_negotiations),
	TEST_NO_TAG("ZRTP sas call with capability negotiations default keys on callee", zrtp_sas_call_with_capability_negotiations_default_keys_on_callee),
	TEST_NO_TAG("ZRTP sas call with capability negotiations", zrtp_sas_call_with_capability_negotiations),
	TEST_NO_TAG("ZRTP key agreement call with capability negotiations", zrtp_key_agreement_call_with_capability_negotiations),
	TEST_NO_TAG("ZRTP call with different encryptions in call params", zrtp_call_with_encryption_supported_in_call_params_only),
	TEST_NO_TAG("ZRTP call started with video and capability negotiation", zrtp_call_with_video_and_capability_negotiation),
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
	TEST_NO_TAG("ZRTP video call with optional encryption on caller", zrtp_video_call_with_optional_encryption_on_caller),
	TEST_NO_TAG("ZRTP video call with optional encryption on callee", zrtp_video_call_with_optional_encryption_on_callee),
	TEST_NO_TAG("ZRTP video call with optional encryption on both sides", zrtp_video_call_with_optional_encryption_on_both_sides)
};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(capability_negotiation_tests) / sizeof(capability_negotiation_tests[0]), capability_negotiation_tests};
