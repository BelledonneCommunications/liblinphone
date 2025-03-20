/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <algorithm>
#include <list>
#include <string>

#include "bctoolbox/defs.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call.h"
#include "capability_negotiation_tester.h"
#include "conference/session/media-session-p.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-nat-policy.h"
#include "linphone/core.h"
#include "sal/call-op.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

void get_expected_encryption_from_call_params(LinphoneCall *offererCall,
                                              LinphoneCall *answererCall,
                                              LinphoneMediaEncryption *expectedEncryption,
                                              bool *potentialConfigurationChosen) {
	const LinphoneCallParams *offerer_params = linphone_call_get_params(offererCall);
	const LinphoneCallParams *answerer_params = linphone_call_get_params(answererCall);
	bool_t offerer_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(offerer_params);
	bool_t answerer_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(answerer_params);
	const LinphoneMediaEncryption offerer_encryption = linphone_call_params_get_media_encryption(offerer_params);
	const LinphoneMediaEncryption answerer_encryption = linphone_call_params_get_media_encryption(answerer_params);
	const bool_t offerer_capability_negotiations = linphone_call_params_capability_negotiations_enabled(offerer_params);
	const bool_t answerer_capability_negotiations =
	    linphone_call_params_capability_negotiations_enabled(answerer_params);

	if (offerer_enc_mandatory) {
		*expectedEncryption = offerer_encryption;
		// reINVITE is not sent because the call should not offer potential configurations as it must enforce an
		// encryption that will be stored in the actual configuration
		*potentialConfigurationChosen = false;
	} else if (answerer_enc_mandatory) {
		*expectedEncryption = answerer_encryption;

		// reINVITE is only sent if offerer and answerer support capability negotiations enabled and the expected
		// encryption is listed in one potential configuration offered by the offerer
		*potentialConfigurationChosen =
		    (offerer_capability_negotiations && answerer_capability_negotiations &&
		     linphone_call_params_is_media_encryption_supported(answerer_params, *expectedEncryption));
	} else if (answerer_capability_negotiations && offerer_capability_negotiations) {

		bctbx_list_t *offerer_supported_encs = linphone_call_params_get_supported_encryptions(offerer_params);
		bool_t enc_check_result = FALSE;
		// Find first encryption listed in the list of supported encryptions of the offerer that is supported by the
		// answerer
		for (bctbx_list_t *enc = offerer_supported_encs; enc != NULL; enc = enc->next) {
			*expectedEncryption = static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(enc)));
			enc_check_result |=
			    (linphone_call_params_is_media_encryption_supported(answerer_params, *expectedEncryption) ||
			     (linphone_call_params_get_media_encryption(answerer_params) == *expectedEncryption));
			if (enc_check_result) {
				break;
			}
		}

		if (!enc_check_result) {
			*expectedEncryption = linphone_call_params_get_media_encryption(offerer_params);
		}
		// reINVITE is always sent
		*potentialConfigurationChosen =
		    linphone_call_params_is_media_encryption_supported(offerer_params, *expectedEncryption) &&
		    (linphone_call_params_get_media_encryption(offerer_params) != *expectedEncryption);

		if (offerer_supported_encs) {
			bctbx_list_free(offerer_supported_encs);
		}

		if (*potentialConfigurationChosen && (*expectedEncryption == LinphoneMediaEncryptionSRTP)) {
			const bool srtpSuiteMatch = search_matching_srtp_suite(get_manager(linphone_call_get_core(offererCall)),
			                                                       get_manager(linphone_call_get_core(answererCall)));
			if (!srtpSuiteMatch) {
				*potentialConfigurationChosen = false;
				*expectedEncryption = linphone_call_params_get_media_encryption(offerer_params);
			}
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

std::list<LinphoneMediaEncryption> set_encryption_preference_with_priority(const LinphoneMediaEncryption encryption,
                                                                           const bool append) {
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

bctbx_list_t *create_confg_encryption_preference_list_except(const LinphoneMediaEncryption encryption) {
	bctbx_list_t *encryption_list = NULL;
	for (int idx = 0; idx <= LinphoneMediaEncryptionDTLS; idx++) {
		if (static_cast<LinphoneMediaEncryption>(idx) != encryption) {
			encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(idx));
		}
	}
	return encryption_list;
}

bctbx_list_t *
create_confg_encryption_preference_list_from_param_preferences(const std::list<LinphoneMediaEncryption> &preferences) {
	bctbx_list_t *encryption_list = NULL;
	for (const auto &enc : preferences) {
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(enc));
	}
	return encryption_list;
}

LinphoneCoreManager *create_core_mgr_with_capability_negotiation_setup(const char *rc_file,
                                                                       const encryption_params enc_params,
                                                                       const bool_t enable_capability_negotiations,
                                                                       const bool_t enable_ice,
                                                                       const bool_t enable_video) {
	LinphoneCoreManager *mgr = linphone_core_manager_new(rc_file);
	linphone_core_enable_capability_negociation(mgr->lc, enable_capability_negotiations);

	const LinphoneMediaEncryption encryption = enc_params.encryption;
	if (linphone_core_media_encryption_supported(mgr->lc, encryption)) {
		linphone_core_set_media_encryption_mandatory(mgr->lc, (enc_params.level == E_MANDATORY));

		if ((enc_params.level == E_MANDATORY) || (enc_params.level == E_OPTIONAL)) {
			linphone_core_set_media_encryption(mgr->lc, encryption);
			BC_ASSERT_EQUAL(linphone_core_get_media_encryption(mgr->lc), encryption, int, "%i");
		} else {
			linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionNone);
			BC_ASSERT_EQUAL(linphone_core_get_media_encryption(mgr->lc), LinphoneMediaEncryptionNone, int, "%i");
		}

		if (!enc_params.preferences.empty()) {
			bctbx_list_t *cfg_enc =
			    create_confg_encryption_preference_list_from_param_preferences(enc_params.preferences);
			linphone_core_set_supported_media_encryptions(mgr->lc, cfg_enc);
			bctbx_list_free(cfg_enc);
		}
	}

	if (enable_video) {
#ifdef VIDEO_ENABLED
		// important: VP8 has really poor performances with the mire camera, at least
		// on iOS - so when ever h264 is available, let's use it instead
		LinphonePayloadType *h264_payload = linphone_core_get_payload_type(mgr->lc, "h264", -1, -1);
		if (h264_payload != NULL) {
			disable_all_video_codecs_except_one(mgr->lc, "h264");
			linphone_payload_type_unref(h264_payload);
		}

		linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);

		LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
		linphone_core_set_video_activation_policy(mgr->lc, pol);
		linphone_video_activation_policy_unref(pol);

		linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);

		linphone_core_enable_video_capture(mgr->lc, TRUE);
		linphone_core_enable_video_display(mgr->lc, TRUE);
#endif // VIDEO_ENABLED
	}

	if (enable_ice) {
		// Enable ICE at the account level but not at the core level
		enable_stun_in_mgr(mgr, TRUE, enable_ice, FALSE, FALSE);
	}

	return mgr;
}

void encrypted_call_with_params_base(LinphoneCoreManager *caller,
                                     LinphoneCoreManager *callee,
                                     const LinphoneMediaEncryption encryption,
                                     const LinphoneCallParams *caller_params,
                                     LinphoneCallParams *callee_params,
                                     const bool_t enable_video) {

	reset_counters(&caller->stat);
	reset_counters(&callee->stat);
	linphone_core_reset_tone_manager_stats(caller->lc);
	linphone_core_reset_tone_manager_stats(callee->lc);

	bool_t caller_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(caller_params);
	bool_t callee_enc_mandatory = linphone_call_params_mandatory_media_encryption_enabled(callee_params);
	const LinphoneMediaEncryption caller_encryption = linphone_call_params_get_media_encryption(caller_params);
	const LinphoneMediaEncryption callee_encryption = linphone_call_params_get_media_encryption(callee_params);
	if (caller_enc_mandatory && callee_enc_mandatory && (caller_encryption != callee_encryption)) {
		BC_ASSERT_FALSE(call_with_params(caller, callee, caller_params, callee_params));
	} else {

		const bool_t caller_capability_negotiations =
		    linphone_call_params_capability_negotiations_enabled(caller_params);
		const bool_t callee_capability_negotiations =
		    linphone_call_params_capability_negotiations_enabled(callee_params);

		char *path = bc_tester_file("certificates-marie");
		linphone_core_set_user_certificates_path(callee->lc, path);
		bc_free(path);
		path = bc_tester_file("certificates-pauline");
		linphone_core_set_user_certificates_path(caller->lc, path);
		bc_free(path);
		bctbx_mkdir(linphone_core_get_user_certificates_path(callee->lc));
		bctbx_mkdir(linphone_core_get_user_certificates_path(caller->lc));

		stats caller_stat = caller->stat;
		stats callee_stat = callee->stat;

		BC_ASSERT_TRUE(call_with_params(caller, callee, caller_params, callee_params));

		LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
		BC_ASSERT_PTR_NOT_NULL(callerCall);
		LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
		BC_ASSERT_PTR_NOT_NULL(calleeCall);

		// Find expected call encryption as well as if a reinvite following capability negotiation is required
		LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
		bool potentialConfigurationChosen = false;
		if (caller_enc_mandatory) {
			expectedEncryption = caller_encryption;
			// reINVITE is not sent because the call should not offer potential configurations as it must enforce an
			// encryption that will be stored in the actual configuration
			potentialConfigurationChosen = false;
		} else if (callee_enc_mandatory) {
			expectedEncryption = callee_encryption;

			// reINVITE is only sent if caller and callee support capability negotiations enabled and the expected
			// encryption is listed in one potential configuration offered by the caller
			potentialConfigurationChosen =
			    (callee_capability_negotiations && caller_capability_negotiations &&
			     linphone_call_params_is_media_encryption_supported(caller_params, expectedEncryption) &&
			     (linphone_call_params_get_media_encryption(caller_params) != expectedEncryption));
		} else if (callee_capability_negotiations && caller_capability_negotiations &&
		           (linphone_call_params_is_media_encryption_supported(caller_params, encryption)) &&
		           (linphone_call_params_is_media_encryption_supported(callee_params, encryption))) {
			expectedEncryption = encryption;
			// reINVITE is always sent
			potentialConfigurationChosen =
			    linphone_call_params_is_media_encryption_supported(caller_params, encryption) &&
			    (linphone_call_params_get_media_encryption(caller_params) != encryption);
			if (potentialConfigurationChosen && (expectedEncryption == LinphoneMediaEncryptionSRTP)) {
				const bool srtpSuiteMatch = search_matching_srtp_suite(caller, callee);
				if (!srtpSuiteMatch) {
					potentialConfigurationChosen = false;
					BC_ASSERT_EQUAL(expectedEncryption, linphone_call_params_get_media_encryption(caller_params), int,
					                "%i");
				}
			}
		} else {
			expectedEncryption = linphone_call_params_get_media_encryption(caller_params);
			// reINVITE is not sent because either parts of the call doesn't support capability negotiations
			potentialConfigurationChosen = false;
		}

		LinphoneNatPolicy *caller_nat_policy = get_nat_policy_for_call(caller, callerCall);
		const bool_t caller_ice_enabled = linphone_nat_policy_ice_enabled(caller_nat_policy);
		LinphoneNatPolicy *callee_nat_policy = get_nat_policy_for_call(callee, calleeCall);
		const bool_t callee_ice_enabled = linphone_nat_policy_ice_enabled(callee_nat_policy);

		const bool_t capabilityNegotiationReinviteEnabled =
		    linphone_core_sdp_200_ack_enabled(caller->lc)
		        ? linphone_call_params_capability_negotiation_reinvite_enabled(callee_params)
		        : linphone_call_params_capability_negotiation_reinvite_enabled(caller_params);
		bool sendReInvite = (potentialConfigurationChosen && capabilityNegotiationReinviteEnabled) ||
		                    (caller_ice_enabled && callee_ice_enabled);
		const int expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

		if (callee_ice_enabled && caller_ice_enabled) {
			BC_ASSERT_TRUE(check_ice(caller, callee, LinphoneIceStateHostConnection));
			BC_ASSERT_TRUE(check_ice(callee, caller, LinphoneIceStateHostConnection));
		}

		int dummy = 0;
		wait_for_until(caller->lc, callee->lc, &dummy, 1, 3000); /*just to sleep while iterating 1s*/

		liblinphone_tester_check_rtcp(caller, callee);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 70, int, "%i");
		LinphoneCallStats *calleeStats = linphone_call_get_audio_stats(linphone_core_get_current_call(callee->lc));
		BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(calleeStats), 70, int, "%i");
		linphone_call_stats_unref(calleeStats);
		calleeStats = NULL;

		// Check that no reINVITE is sent while checking streams
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallStreamsRunning,
		                (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%i");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallStreamsRunning,
		                (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%i");

		if (callerCall) {
			check_stream_encryption(callerCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
			                expectedEncryption, int, "%i");
		}
		if (calleeCall) {
			check_stream_encryption(calleeCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
			                expectedEncryption, int, "%i");
		}

		if (!enable_video) {
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(calleeCall)));
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(callerCall)));

			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(calleeCall)));
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(callerCall)));
		}
#ifdef VIDEO_ENABLED
		else {
			reset_counters(&caller->stat);
			reset_counters(&callee->stat);
			stats caller_stat = caller->stat;
			stats callee_stat = callee->stat;
			LinphoneCallParams *params = linphone_core_create_call_params(callee->lc, calleeCall);
			linphone_call_params_enable_video(params, TRUE);
			linphone_call_update(calleeCall, params);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallUpdating,
			                        (callee_stat.number_of_LinphoneCallUpdating + 1)));
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallUpdatedByRemote,
			                        (caller_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
			                        (callee_stat.number_of_LinphoneCallStreamsRunning + 1)));
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
			                        (caller_stat.number_of_LinphoneCallStreamsRunning + 1)));

			// Update expected encryption
			get_expected_encryption_from_call_params(calleeCall, callerCall, &expectedEncryption,
			                                         &potentialConfigurationChosen);

			const bool_t capabilityNegotiationReinviteEnabledAfterUpdate =
			    linphone_core_sdp_200_ack_enabled(callee->lc)
			        ? linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(callerCall))
			        : linphone_call_params_capability_negotiation_reinvite_enabled(
			              linphone_call_get_params(calleeCall));
			bool sendReInviteAfterUpdate =
			    (potentialConfigurationChosen && capabilityNegotiationReinviteEnabledAfterUpdate) ||
			    (caller_ice_enabled && callee_ice_enabled);

			const int expectedStreamsRunningAfterUpdate = 1 + ((sendReInviteAfterUpdate) ? 1 : 0);

			BC_ASSERT_TRUE(
			    wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
			             (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunningAfterUpdate)));
			BC_ASSERT_TRUE(
			    wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
			             (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunningAfterUpdate)));
			liblinphone_tester_set_next_video_frame_decoded_cb(callerCall);
			liblinphone_tester_set_next_video_frame_decoded_cb(calleeCall);

			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_IframeDecoded,
			                        (callee_stat.number_of_IframeDecoded + 1)));
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_IframeDecoded,
			                        (caller_stat.number_of_IframeDecoded + 1)));

			BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(calleeCall)));
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(callerCall)));

			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(calleeCall)));
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(callerCall)));

			if ((expectedEncryption == LinphoneMediaEncryptionDTLS) ||
			    (expectedEncryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_until(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallEncryptedOn,
				                              caller_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
				BC_ASSERT_TRUE(wait_for_until(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallEncryptedOn,
				                              callee_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
			}

			int dummy = 0;
			wait_for_until(caller->lc, callee->lc, &dummy, 1, 3000); /*just to sleep while iterating 1s*/

			liblinphone_tester_check_rtcp(caller, callee);

			BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 70, int, "%i");
			calleeStats = linphone_call_get_audio_stats(linphone_core_get_current_call(callee->lc));
			BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(calleeStats), 70, int, "%i");
			linphone_call_stats_unref(calleeStats);
			calleeStats = NULL;
		}
#endif // VIDEO_ENABLED

		// Check that encryption has not changed after sending update
		if (callerCall) {
			check_stream_encryption(callerCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
			                expectedEncryption, int, "%i");
		}
		if (calleeCall) {
			check_stream_encryption(calleeCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
			                expectedEncryption, int, "%i");
		}
	}
}

void encrypted_call_base(LinphoneCoreManager *caller,
                         LinphoneCoreManager *callee,
                         const LinphoneMediaEncryption encryption,
                         const bool_t enable_caller_capability_negotiations,
                         const bool_t enable_callee_capability_negotiations,
                         const bool_t enable_video) {

	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations(caller_params, enable_caller_capability_negotiations);
	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations(callee_params, enable_callee_capability_negotiations);

	encrypted_call_with_params_base(caller, callee, encryption, caller_params, callee_params, enable_video);

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);
}

void pause_resume_calls(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(callerCall);
	LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(calleeCall);

	if (calleeCall && callerCall) {
		LinphoneMediaEncryption calleeEncryption =
		    linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall));
		LinphoneMediaEncryption callerEncryption =
		    linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall));

		ms_message("%s pauses call with %s", linphone_core_get_identity(callee->lc),
		           linphone_core_get_identity(caller->lc));
		// Pause callee call
		BC_ASSERT_TRUE(pause_call_1(callee, calleeCall, caller, callerCall));
		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		// Resume callee call
		reset_counters(&caller->stat);
		reset_counters(&callee->stat);
		stats caller_stat = caller->stat;
		stats callee_stat = callee->stat;

		ms_message("%s resumes call with %s", linphone_core_get_identity(callee->lc),
		           linphone_core_get_identity(caller->lc));
		linphone_call_resume(calleeCall);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        caller_stat.number_of_LinphoneCallStreamsRunning + 1));

		// Verify that encryption didn't change
		BC_ASSERT_EQUAL(calleeEncryption, callerEncryption, int, "%i");
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
		                callerEncryption, int, "%i");
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
		                calleeEncryption, int, "%i");

		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		liblinphone_tester_check_rtcp(caller, callee);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 70, int, "%i");
		LinphoneCallStats *calleeStats = linphone_call_get_audio_stats(linphone_core_get_current_call(callee->lc));
		BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(calleeStats), 70, int, "%i");
		linphone_call_stats_unref(calleeStats);
		calleeStats = NULL;

		check_stream_encryption(callerCall);
		check_stream_encryption(calleeCall);

		// Pause caller call
		ms_message("%s pauses call with %s", linphone_core_get_identity(caller->lc),
		           linphone_core_get_identity(callee->lc));
		BC_ASSERT_TRUE(pause_call_1(caller, callerCall, callee, calleeCall));
		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		// Resume caller call
		reset_counters(&caller->stat);
		reset_counters(&callee->stat);
		caller_stat = caller->stat;
		callee_stat = callee->stat;

		ms_message("%s resumes call with %s", linphone_core_get_identity(caller->lc),
		           linphone_core_get_identity(callee->lc));
		linphone_call_resume(callerCall);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        caller_stat.number_of_LinphoneCallStreamsRunning + 1));

		// Verify that encryption didn't change
		BC_ASSERT_EQUAL(calleeEncryption, callerEncryption, int, "%i");
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
		                callerEncryption, int, "%i");
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
		                calleeEncryption, int, "%i");

		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		liblinphone_tester_check_rtcp(caller, callee);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(callee), 70, int, "%i");
		LinphoneCallStats *callerStats = linphone_call_get_audio_stats(linphone_core_get_current_call(caller->lc));
		BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(callerStats), 70, int, "%i");
		linphone_call_stats_unref(callerStats);
		callerStats = NULL;

		check_stream_encryption(callerCall);
		check_stream_encryption(calleeCall);
	}
}

void call_with_update_and_incompatible_encs_in_call_params_base(const bool_t enable_ice) {
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

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, enable_ice, TRUE);

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

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    enable_ice, TRUE);

	if (enable_ice) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_mux", 1);
	}

	bctbx_list_t *marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	linphone_call_params_set_media_encryption(marie_params, LinphoneMediaEncryptionDTLS);
	bctbx_list_free(marie_call_enc);

	bctbx_list_t *pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, 1);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);
	bctbx_list_free(pauline_call_enc);

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
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                encryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                encryption, int, "%i");
	}

	stats marie_stat = marie->stat;
	stats pauline_stat = pauline->stat;
	LinphoneCallParams *params = linphone_core_create_call_params(pauline->lc, paulineCall);
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
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	LinphoneNatPolicy *marie_nat_policy = get_nat_policy_for_call(marie, marieCall);
	const bool_t marie_ice_enabled = linphone_nat_policy_ice_enabled(marie_nat_policy);
	LinphoneNatPolicy *pauline_nat_policy = get_nat_policy_for_call(pauline, paulineCall);
	const bool_t pauline_ice_enabled = linphone_nat_policy_ice_enabled(pauline_nat_policy);
	const int expectedStreamsRunning = 1 + ((pauline_ice_enabled && marie_ice_enabled) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

	if (pauline_ice_enabled && marie_ice_enabled) {
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded,
	                        (pauline_stat.number_of_IframeDecoded + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
	                        (marie_stat.number_of_IframeDecoded + 1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                encryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                encryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
	                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_with_encryption_test_base(const encryption_params marie_enc_params,
                                    const bool_t enable_marie_capability_negotiations,
                                    const bool_t enable_marie_ice,
                                    const encryption_params pauline_enc_params,
                                    const bool_t enable_pauline_capability_negotiations,
                                    const bool_t enable_pauline_ice,
                                    const bool_t enable_video) {

	LinphoneCoreManager *marie = create_core_mgr_with_capability_negotiation_setup(
	    "marie_rc", marie_enc_params, enable_marie_capability_negotiations, enable_marie_ice, enable_video);
	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_params,
	    enable_pauline_capability_negotiations, enable_pauline_ice, enable_video);

	LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
	if (marie_enc_params.level == E_MANDATORY) {
		expectedEncryption = marie_enc_params.encryption;
	} else if (pauline_enc_params.level == E_MANDATORY) {
		expectedEncryption = pauline_enc_params.encryption;
	} else if ((enable_marie_capability_negotiations == TRUE) && (enable_pauline_capability_negotiations == TRUE)) {
		for (const auto &enc : marie_enc_params.preferences) {
			if ((std::find(pauline_enc_params.preferences.cbegin(), pauline_enc_params.preferences.cend(), enc) !=
			     pauline_enc_params.preferences.cend()) ||
			    (pauline_enc_params.encryption == enc)) {
				expectedEncryption = enc;
				break;
			}
		}
	} else if (enable_marie_capability_negotiations == TRUE) {
		expectedEncryption = pauline_enc_params.encryption;
	} else {
		expectedEncryption = marie_enc_params.encryption;
	}

	if (enable_marie_ice && enable_pauline_ice) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_mux", 1);
	}

	encrypted_call_base(marie, pauline, expectedEncryption, enable_marie_capability_negotiations,
	                    enable_pauline_capability_negotiations, enable_video);
	if (linphone_core_get_current_call(marie->lc) && linphone_core_get_current_call(pauline->lc)) {
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static bool_t call_with_params_and_encryption_negotiation_failure_base(LinphoneCoreManager *caller,
                                                                       LinphoneCoreManager *callee,
                                                                       LinphoneCallParams *caller_params,
                                                                       LinphoneCallParams *callee_params,
                                                                       bool_t expSdpSuccess) {

	const bctbx_list_t *initLogs = linphone_core_get_call_logs(callee->lc);
	int initLogsSize = (int)bctbx_list_size(initLogs);
	stats initial_callee = callee->stat;
	stats initial_caller = callee->stat;

	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	caller_test_params.base = (LinphoneCallParams *)caller_params;
	callee_test_params.base = (LinphoneCallParams *)callee_params;
	callee_test_params.sdp_simulate_error = !expSdpSuccess;

	bool_t ret = call_with_params2(caller, callee, &caller_test_params, &callee_test_params, FALSE);

	LinphoneCall *callee_call = linphone_core_get_current_call(callee->lc);
	if (!expSdpSuccess) {
		BC_ASSERT_PTR_NULL(callee_call);
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError, (initial_caller.number_of_LinphoneCallError + 1), int,
		                "%d");
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallReleased,
		                (initial_caller.number_of_LinphoneCallReleased + 1), int, "%d");
		// actually callee does not receive error because it replies to the INVITE with a 488 Not Acceptable Here
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived,
		                (initial_callee.number_of_LinphoneCallIncomingReceived), int, "%d");

		const bctbx_list_t *logs = linphone_core_get_call_logs(callee->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(logs), (initLogsSize + 1), int, "%i");
		// Forward logs pointer to the element desired
		for (int i = 0; i < initLogsSize; i++)
			logs = logs->next;
		if (logs) {
			const LinphoneErrorInfo *ei;
			LinphoneCallLog *cl = (LinphoneCallLog *)logs->data;
			BC_ASSERT_TRUE(linphone_call_log_get_start_date(cl) != 0);
			ei = linphone_call_log_get_error_info(cl);
			BC_ASSERT_PTR_NOT_NULL(ei);
			if (ei) {
				BC_ASSERT_EQUAL(linphone_error_info_get_reason(ei), LinphoneReasonNotAcceptable, int, "%d");
			}
		}

		BC_ASSERT_EQUAL(linphone_core_get_calls_nb(caller->lc), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_calls_nb(callee->lc), 0, int, "%d");

		return FALSE;
	}

	return ret;
}

static bool_t call_with_encryption_negotiation_failure_base(LinphoneCoreManager *caller,
                                                            LinphoneCoreManager *callee,
                                                            bool_t expSdpSuccess) {
	return call_with_params_and_encryption_negotiation_failure_base(caller, callee, NULL, NULL, expSdpSuccess);
}

static void call_from_enc_to_different_enc_base(const LinphoneMediaEncryption mandatory_encryption,
                                                const LinphoneMediaEncryption non_mandatory_encryption,
                                                const bool_t enable_mandatory_enc_mgr_capability_negotiations,
                                                const bool_t enable_non_mandatory_enc_mgr_capability_negotiations,
                                                bool_t mandatory_to_non_mandatory) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, enable_non_mandatory_enc_mgr_capability_negotiations);
	linphone_core_set_media_encryption_mandatory(marie->lc, FALSE);
	linphone_core_set_media_encryption(marie->lc, non_mandatory_encryption);
	bctbx_list_t *cfg_enc = create_confg_encryption_preference_list_except(mandatory_encryption);
	linphone_core_set_supported_media_encryptions(marie->lc, cfg_enc);
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(marie->lc, mandatory_encryption));
	bctbx_list_free(cfg_enc);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negociation(pauline->lc, enable_mandatory_enc_mgr_capability_negotiations);
	if (linphone_core_media_encryption_supported(pauline->lc, mandatory_encryption)) {
		linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
		linphone_core_set_media_encryption(pauline->lc, mandatory_encryption);
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

void call_from_enc_to_no_enc_base(const LinphoneMediaEncryption encryption,
                                  const bool_t enable_mandatory_enc_mgr_capability_negotiations,
                                  const bool_t enable_non_mandatory_enc_mgr_capability_negotiations,
                                  bool_t mandatory_to_non_mandatory) {
	call_from_enc_to_different_enc_base(
	    encryption, LinphoneMediaEncryptionNone, enable_mandatory_enc_mgr_capability_negotiations,
	    enable_non_mandatory_enc_mgr_capability_negotiations, mandatory_to_non_mandatory);
}

void call_from_enc_to_dtls_enc_base(const LinphoneMediaEncryption encryption,
                                    const bool_t enable_mandatory_enc_mgr_capability_negotiations,
                                    const bool_t enable_non_mandatory_enc_mgr_capability_negotiations,
                                    bool_t mandatory_to_non_mandatory) {
	call_from_enc_to_different_enc_base(
	    encryption, LinphoneMediaEncryptionDTLS, enable_mandatory_enc_mgr_capability_negotiations,
	    enable_non_mandatory_enc_mgr_capability_negotiations, mandatory_to_non_mandatory);
}

static void call_with_capability_negotiation_failure(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, 1);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negociation(pauline->lc, 1);

	const LinphoneMediaEncryption paulineOptionalEncryption = LinphoneMediaEncryptionSRTP;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
	if (linphone_core_media_encryption_supported(pauline->lc, paulineOptionalEncryption)) {
		bctbx_list_t *encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(paulineOptionalEncryption));
		linphone_core_set_media_encryption_mandatory(pauline->lc, 0);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(pauline->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
		if (encryption_list) {
			bctbx_list_free(encryption_list);
		}
	}

	const LinphoneMediaEncryption marieOptionalEncryption = LinphoneMediaEncryptionDTLS;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, marieOptionalEncryption));
	if (linphone_core_media_encryption_supported(marie->lc, marieOptionalEncryption)) {
		bctbx_list_t *encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(marieOptionalEncryption));
		linphone_core_set_media_encryption_mandatory(marie->lc, 0);
		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(marie->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption));
		if (encryption_list) {
			bctbx_list_free(encryption_list);
		}
	}

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption));

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption));

	encrypted_call_base(marie, pauline, marieOptionalEncryption, TRUE, TRUE, FALSE);
	if (linphone_core_get_current_call(marie->lc) && linphone_core_get_current_call(pauline->lc)) {
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_capability_negotiation_failure_multiple_potential_configurations(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, 1);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negociation(pauline->lc, 1);

	const LinphoneMediaEncryption paulineOptionalEncryption = LinphoneMediaEncryptionSRTP;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
	if (linphone_core_media_encryption_supported(pauline->lc, paulineOptionalEncryption)) {
		bctbx_list_t *encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(paulineOptionalEncryption));
		linphone_core_set_media_encryption_mandatory(pauline->lc, 0);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(pauline->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
		if (encryption_list) {
			bctbx_list_free(encryption_list);
		}
	}

	bctbx_list_t *encryption_list = NULL;
	const LinphoneMediaEncryption marieOptionalEncryption0 = LinphoneMediaEncryptionDTLS;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, marieOptionalEncryption0));
	if (linphone_core_media_encryption_supported(marie->lc, marieOptionalEncryption0)) {
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(marieOptionalEncryption0));
	}

	const LinphoneMediaEncryption marieOptionalEncryption1 = LinphoneMediaEncryptionZRTP;
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, marieOptionalEncryption1));
	if (linphone_core_media_encryption_supported(marie->lc, marieOptionalEncryption1)) {
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(marieOptionalEncryption1));
	}

	linphone_core_set_media_encryption_mandatory(marie->lc, 0);
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(marie->lc, encryption_list);
	if (encryption_list) {
		bctbx_list_free(encryption_list);
	}

	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(marie->lc, paulineOptionalEncryption));
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption0));
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, marieOptionalEncryption1));

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, paulineOptionalEncryption));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption0));
	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, marieOptionalEncryption1));

	encrypted_call_base(marie, pauline, marieOptionalEncryption0, TRUE, TRUE, FALSE);
	if (linphone_core_get_current_call(marie->lc) && linphone_core_get_current_call(pauline->lc)) {
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_capability_negotiation_disable_call_level(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, 1);
	linphone_core_set_media_encryption_mandatory(marie->lc, 0);
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionNone);
	bctbx_list_t *cfg_enc = create_confg_encryption_preference_list_except(LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(marie->lc, cfg_enc);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negociation(pauline->lc, 1);
	linphone_core_set_media_encryption_mandatory(pauline->lc, 0);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(pauline->lc, cfg_enc);
	bctbx_list_free(cfg_enc);

	encrypted_call_base(marie, pauline, LinphoneMediaEncryptionNone, FALSE, FALSE, FALSE);
	if (linphone_core_get_current_call(marie->lc) && linphone_core_get_current_call(pauline->lc)) {
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_capability_negotiation_disable_core_level(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, 0);
	linphone_core_set_media_encryption_mandatory(marie->lc, 0);
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionNone);
	bctbx_list_t *cfg_enc = create_confg_encryption_preference_list_except(LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(marie->lc, cfg_enc);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negociation(pauline->lc, 0);
	linphone_core_set_media_encryption_mandatory(pauline->lc, 0);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionNone);
	linphone_core_set_supported_media_encryptions(pauline->lc, cfg_enc);

	LinphoneMediaEncryption chosenMediaEnc =
	    static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(cfg_enc)));

	encrypted_call_base(marie, pauline, chosenMediaEnc, TRUE, TRUE, FALSE);
	bctbx_list_free(cfg_enc);
	if (linphone_core_get_current_call(marie->lc) && linphone_core_get_current_call(pauline->lc)) {
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_incompatible_encs_in_call_params(void) {
	const LinphoneMediaEncryption defaultEncryption = LinphoneMediaEncryptionNone;
	const LinphoneMediaEncryption marieEncryption = LinphoneMediaEncryptionDTLS;   // Desired encryption
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

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, FALSE);
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

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, FALSE);
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, marieEncryption));
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, paulineEncryption));

	bctbx_list_t *marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(marieEncryption));
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	bctbx_list_free(marie_call_enc);

	bctbx_list_t *pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(paulineEncryption));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, 1);
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);
	bctbx_list_free(pauline_call_enc);

	// Check requirements for this test:
	// - encryption of Marie and Pauline should be different
	// - Marie and Pauline encryption list in the call params must be equal to 1
	BC_ASSERT_NOT_EQUAL(paulineEncryption, marieEncryption, int, "%i");

	bctbx_list_t *pauline_supported_encs = linphone_call_params_get_supported_encryptions(pauline_params);
	BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_supported_encs), 1, int, "%d");
	if (pauline_supported_encs) {
		bctbx_list_free(pauline_supported_encs);
	}

	bctbx_list_t *marie_supported_encs = linphone_call_params_get_supported_encryptions(marie_params);
	BC_ASSERT_EQUAL((int)bctbx_list_size(marie_supported_encs), 1, int, "%d");
	if (marie_supported_encs) {
		bctbx_list_free(marie_supported_encs);
	}

	BC_ASSERT_TRUE(
	    call_with_params_and_encryption_negotiation_failure_base(marie, pauline, marie_params, pauline_params, TRUE));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	liblinphone_tester_check_rtcp(marie, pauline);

	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                defaultEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                defaultEncryption, int, "%i");
	}

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_with_video_and_capability_negotiation_base(const LinphoneMediaEncryption encryption) {

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

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);
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

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, TRUE);
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, encryption));

	bctbx_list_t *call_enc = NULL;
	call_enc = bctbx_list_append(call_enc, LINPHONE_INT_TO_PTR(encryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(marie_params, 1);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_supported_encryptions(marie_params, call_enc);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_video(pauline_params, 1);
	linphone_call_params_enable_capability_negotiations(pauline_params, 1);
	linphone_call_params_set_supported_encryptions(pauline_params, call_enc);

	bctbx_list_free(call_enc);

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	const int expectedStreamsRunning = 2;
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%i");

	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                encryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                encryption, int, "%i");
	}

	if (marieCall && paulineCall) {
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));

		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));

		/*check video path*/
		liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);
		liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
		linphone_call_send_vfu_request(paulineCall);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded, 1));
	}

	liblinphone_tester_check_rtcp(pauline, marie);

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_cfg_lines_merge_base(const bool_t caller_cfg_merge, const bool_t callee_cfg_merge) {
	encryption_params caller_enc_mgr_params;
	caller_enc_mgr_params.encryption = LinphoneMediaEncryptionZRTP;
	caller_enc_mgr_params.level = E_OPTIONAL;
	caller_enc_mgr_params.preferences = set_encryption_preference(TRUE);

	encryption_params callee_enc_mgr_params;
	callee_enc_mgr_params.encryption = LinphoneMediaEncryptionDTLS;
	callee_enc_mgr_params.level = E_OPTIONAL;
	callee_enc_mgr_params.preferences = set_encryption_preference(FALSE);

	LinphoneCoreManager *caller =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", caller_enc_mgr_params, TRUE, FALSE, TRUE);
	LinphoneCoreManager *callee = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), callee_enc_mgr_params, TRUE,
	    FALSE, TRUE);

	// Caller call params:
	// - RFC5939 is supported
	// - Merge pcfg lines
	// - Default encryption SRTP
	// - No mandatory encryption
	// - Supported optional encryptions: DTLS, ZRTP
	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations(caller_params, TRUE);
	bctbx_list_t *caller_cfg_enc = NULL;
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionSRTP));
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	linphone_call_params_set_supported_encryptions(caller_params, caller_cfg_enc);
	bctbx_list_free(caller_cfg_enc);
	linphone_call_params_set_media_encryption(caller_params, LinphoneMediaEncryptionNone);
	linphone_call_params_enable_mandatory_media_encryption(caller_params, 0);
	linphone_call_params_enable_cfg_lines_merging(caller_params, caller_cfg_merge);

	// Callee call params:
	// - RFC5939 is supported
	// - Merge pcfg lines
	// - Default encryption SRTP
	// - No mandatory encryption
	// - Supported optional encryptions: SRTP, ZRTP, DTLS
	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations(callee_params, TRUE);
	bctbx_list_t *callee_cfg_enc = NULL;
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionSRTP));
	linphone_call_params_set_supported_encryptions(callee_params, callee_cfg_enc);
	bctbx_list_free(callee_cfg_enc);
	linphone_call_params_set_media_encryption(callee_params, LinphoneMediaEncryptionDTLS);
	linphone_call_params_enable_mandatory_media_encryption(callee_params, 0);
	linphone_call_params_enable_cfg_lines_merging(callee_params, callee_cfg_merge);

	const LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionSRTP;
	encrypted_call_with_params_base(caller, callee, expectedEncryption, caller_params, callee_params, TRUE);

	LinphoneCall *callee_call = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(callee_call);
	if (callee_call) {
		BC_ASSERT_TRUE(linphone_call_params_cfg_lines_merged(linphone_call_get_params(callee_call)) ==
		               callee_cfg_merge);
	}

	LinphoneCall *caller_call = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(caller_call);
	if (caller_call) {
		BC_ASSERT_TRUE(linphone_call_params_cfg_lines_merged(linphone_call_get_params(caller_call)) ==
		               caller_cfg_merge);
		end_call(caller, callee);
	}

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);

	linphone_core_manager_destroy(caller);
	linphone_core_manager_destroy(callee);
}

static void call_with_no_cfg_lines_merge(void) {
	call_with_cfg_lines_merge_base(FALSE, FALSE);
}

static void call_with_cfg_lines_merge_on_caller(void) {
	call_with_cfg_lines_merge_base(TRUE, FALSE);
}

static void call_with_cfg_lines_merge_on_callee(void) {
	call_with_cfg_lines_merge_base(FALSE, TRUE);
}

static void call_with_cfg_lines_merge_on_both_sides(void) {
	call_with_cfg_lines_merge_base(TRUE, TRUE);
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

	LinphoneCoreManager *caller =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", caller_enc_mgr_params, TRUE, FALSE, TRUE);
	LinphoneCoreManager *callee = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), callee_enc_mgr_params, TRUE,
	    FALSE, TRUE);

	// Caller call params:
	// - RFC5939 is supported
	// - Merge tcap lines
	// - Default encryption SRTP
	// - No mandatory encryption
	// - Supported optional encryptions: DTLS, ZRTP
	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations(caller_params, TRUE);
	bctbx_list_t *caller_cfg_enc = NULL;
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	caller_cfg_enc = bctbx_list_append(caller_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	linphone_call_params_set_supported_encryptions(caller_params, caller_cfg_enc);
	bctbx_list_free(caller_cfg_enc);
	linphone_call_params_set_media_encryption(caller_params, LinphoneMediaEncryptionSRTP);
	linphone_call_params_enable_mandatory_media_encryption(caller_params, 0);
	linphone_call_params_enable_tcap_line_merging(caller_params, caller_tcap_merge);

	// Callee call params:
	// - RFC5939 is supported
	// - Merge tcap lines
	// - Default encryption SRTP
	// - No mandatory encryption
	// - Supported optional encryptions: SRTP, ZRTP, DTLS
	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations(callee_params, TRUE);
	bctbx_list_t *callee_cfg_enc = NULL;
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionSRTP));
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	callee_cfg_enc = bctbx_list_append(callee_cfg_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	linphone_call_params_set_supported_encryptions(callee_params, callee_cfg_enc);
	bctbx_list_free(callee_cfg_enc);
	linphone_call_params_set_media_encryption(callee_params, LinphoneMediaEncryptionNone);
	linphone_call_params_enable_mandatory_media_encryption(callee_params, 0);
	linphone_call_params_enable_tcap_line_merging(callee_params, callee_tcap_merge);

	const LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionDTLS;
	encrypted_call_with_params_base(caller, callee, expectedEncryption, caller_params, callee_params, TRUE);

	LinphoneCall *callee_call = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(callee_call);
	if (callee_call) {
		BC_ASSERT_TRUE(linphone_call_params_tcap_lines_merged(linphone_call_get_params(callee_call)) ==
		               callee_tcap_merge);
	}

	LinphoneCall *caller_call = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(caller_call);
	if (caller_call) {
		BC_ASSERT_TRUE(linphone_call_params_tcap_lines_merged(linphone_call_get_params(caller_call)) ==
		               caller_tcap_merge);
		end_call(caller, callee);
	}

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

static void call_with_no_sdp_on_update_base(const bool_t caller_cap_neg,
                                            const bool_t callee_cap_neg,
                                            const bool_t caller_cap_neg_reinvite,
                                            const bool_t callee_cap_neg_reinvite) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP;  // Desired encryption
	const LinphoneMediaEncryption encryption1 = LinphoneMediaEncryptionNone; // Desired encryption after first update
	const LinphoneMediaEncryption encryption2 = LinphoneMediaEncryptionZRTP; // Desired encryption after Marie's update

	LinphoneMediaEncryption marieEncryption = LinphoneMediaEncryptionNone;
	if (caller_cap_neg && callee_cap_neg) {
		marieEncryption = LinphoneMediaEncryptionDTLS;
	} else {
		marieEncryption = encryption;
	}

	std::list<LinphoneMediaEncryption> marie_enc_list;
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionZRTP)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionDTLS)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionSRTP)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionNone)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionNone);
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager *marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params,
	                                                                               caller_cap_neg, FALSE, TRUE);
	linphone_core_enable_capability_negotiation_reinvite(marie->lc, caller_cap_neg_reinvite);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionSRTP)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionZRTP)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionNone)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionNone);
	}
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionDTLS)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params,
	    callee_cap_neg, FALSE, TRUE);
	linphone_core_enable_capability_negotiation_reinvite(pauline->lc, callee_cap_neg_reinvite);

	bctbx_list_t *marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption));
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption1));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, caller_cap_neg);
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	linphone_call_params_set_media_encryption(marie_params, marieEncryption);

	bctbx_list_free(marie_call_enc);

	bctbx_list_t *pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, callee_cap_neg);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);

	bctbx_list_free(pauline_call_enc);

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

	LinphoneNatPolicy *marie_nat_policy = get_nat_policy_for_call(marie, marieCall);
	const bool_t marie_ice_enabled = linphone_nat_policy_ice_enabled(marie_nat_policy);
	LinphoneNatPolicy *pauline_nat_policy = get_nat_policy_for_call(pauline, paulineCall);
	const bool_t pauline_ice_enabled = linphone_nat_policy_ice_enabled(pauline_nat_policy);

	bool potentialConfigurationChosen = (caller_cap_neg && callee_cap_neg);
	bool_t capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(marieCall));
	bool sendReInvite = (potentialConfigurationChosen && capabilityNegotiationReinviteEnabled) ||
	                    (marie_ice_enabled && pauline_ice_enabled);
	int expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");

	// Check that encryption has not changed after sending update
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                encryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                encryption, int, "%i");
	}

	ms_message("Pauline sends a reINVITE without SDP");
	// Pauline send an empty SDP for the update
	linphone_core_enable_sdp_200_ack(pauline->lc, TRUE);

	stats marie_stat = marie->stat;
	stats pauline_stat = pauline->stat;
	LinphoneCallParams *params0 = linphone_core_create_call_params(pauline->lc, paulineCall);
	if (caller_cap_neg && callee_cap_neg) {
		linphone_call_params_set_media_encryption(params0, encryption1);
	}
	bctbx_list_t *encs0 = NULL;
	encs0 = bctbx_list_append(encs0, LINPHONE_INT_TO_PTR(encryption1));
	encs0 = bctbx_list_append(encs0, LINPHONE_INT_TO_PTR(encryption2));
	linphone_call_params_set_supported_encryptions(params0, encs0);
	bctbx_list_free(encs0);

	BC_ASSERT_TRUE(linphone_core_sdp_200_ack_enabled(pauline->lc));
	linphone_call_update(paulineCall, params0);
	linphone_call_params_unref(params0);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

	LinphoneMediaEncryption encryptionAfterUpdate = (caller_cap_neg && callee_cap_neg) ? encryption1 : encryption;
	LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
	bool dummyPotentialConfigurationChosen = false;
	get_expected_encryption_from_call_params(paulineCall, marieCall, &expectedEncryption,
	                                         &dummyPotentialConfigurationChosen);
	// As Pauline sends a reINVITE without SDP, Marie replies with the same SDP as she previously sent hence it has the
	// same capability negotiation flags as the previous answer to the INVITE. A reINVITE is sent again if capability
	// negotiations are sent
	capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(marieCall));
	sendReInvite = (potentialConfigurationChosen && capabilityNegotiationReinviteEnabled) ||
	               (marie_ice_enabled && pauline_ice_enabled);
	expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

	linphone_core_enable_sdp_200_ack(pauline->lc, FALSE);

	liblinphone_tester_check_rtcp(marie, pauline);

	wait_for_until(marie->lc, pauline->lc, NULL, 5, 2000);

	// Check that encryption has not changed after sending update
	BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
	                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	marie_stat = marie->stat;
	pauline_stat = pauline->stat;
	LinphoneCallParams *params1 = linphone_core_create_call_params(pauline->lc, paulineCall);
	linphone_call_params_enable_video(params1, TRUE);
	BC_ASSERT_FALSE(linphone_core_sdp_200_ack_enabled(pauline->lc));
	ms_message("Pauline enables video");
	linphone_call_update(paulineCall, params1);
	linphone_call_params_unref(params1);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

	get_expected_encryption_from_call_params(paulineCall, marieCall, &expectedEncryption,
	                                         &potentialConfigurationChosen);
	capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(paulineCall));
	sendReInvite = (potentialConfigurationChosen && capabilityNegotiationReinviteEnabled);
	expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	// wait for reINVITEs to complete
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

	if (pauline_ice_enabled && marie_ice_enabled) {
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded,
	                        (pauline_stat.number_of_IframeDecoded + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
	                        (marie_stat.number_of_IframeDecoded + 1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	wait_for_until(marie->lc, pauline->lc, NULL, 5, 2000);

	// Check that encryption has not changed after sending update
	BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
	                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	// Marie send an empty SDP for the update
	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);

	marie_stat = marie->stat;
	pauline_stat = pauline->stat;
	LinphoneCallParams *params2 = linphone_core_create_call_params(marie->lc, marieCall);
	linphone_call_params_set_media_encryption(params2, marieEncryption);
	bctbx_list_t *encs2 = NULL;
	encs2 = bctbx_list_append(encs2, LINPHONE_INT_TO_PTR(encryption2));
	encs2 = bctbx_list_append(encs2, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(params2, encs2);
	bctbx_list_free(encs2);
	BC_ASSERT_TRUE(linphone_core_sdp_200_ack_enabled(marie->lc));
	ms_message("Marie sends a reINVITE without SDP");
	linphone_call_update(marieCall, params2);
	linphone_call_params_unref(params2);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdating,
	                        (marie_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

	encryptionAfterUpdate = (caller_cap_neg && callee_cap_neg) ? encryption2 : encryption;
	get_expected_encryption_from_call_params(marieCall, paulineCall, &expectedEncryption,
	                                         &potentialConfigurationChosen);
	capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(paulineCall));
	sendReInvite = (potentialConfigurationChosen && capabilityNegotiationReinviteEnabled);
	expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	// wait for reINVITEs to complete
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

	linphone_core_enable_sdp_200_ack(marie->lc, FALSE);

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded,
	                        (pauline_stat.number_of_IframeDecoded + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
	                        (marie_stat.number_of_IframeDecoded + 1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	if ((expectedEncryption == LinphoneMediaEncryptionZRTP) || (expectedEncryption == LinphoneMediaEncryptionDTLS)) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEncryptedOn,
		                        (marie_stat.number_of_LinphoneCallEncryptedOn + 1)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEncryptedOn,
		                        (pauline_stat.number_of_LinphoneCallEncryptedOn + 1)));
	}

	// Check that encryption has not changed after sending update
	BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
	                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_on_update_cap_neg_caller(void) {
	call_with_no_sdp_on_update_base(TRUE, FALSE, TRUE, TRUE);
}

static void call_with_no_sdp_on_update_cap_neg_callee(void) {
	call_with_no_sdp_on_update_base(FALSE, TRUE, TRUE, TRUE);
}

static void call_with_no_sdp_on_update_cap_neg_both_sides_with_reinvite(void) {
	call_with_no_sdp_on_update_base(TRUE, TRUE, TRUE, TRUE);
}

static void call_with_no_sdp_on_update_cap_neg_both_sides_without_reinvite_on_caller(void) {
	call_with_no_sdp_on_update_base(TRUE, TRUE, FALSE, TRUE);
}

static void call_with_no_sdp_on_update_cap_neg_both_sides_without_reinvite_on_callee(void) {
	call_with_no_sdp_on_update_base(TRUE, TRUE, TRUE, FALSE);
}

static void call_with_no_sdp_on_update_cap_neg_both_sides_without_reinvite(void) {
	call_with_no_sdp_on_update_base(TRUE, TRUE, FALSE, FALSE);
}

static void call_changes_enc_on_update_base(const bool_t caller_cap_neg,
                                            const bool_t callee_cap_neg,
                                            const bool_t caller_cap_neg_reinvite,
                                            const bool_t callee_cap_neg_reinvite) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP; // Desired encryption
	LinphoneMediaEncryption expectedEncryption = encryption;                // Expected encryption

	LinphoneMediaEncryption marieEncryption = LinphoneMediaEncryptionNone;
	if (caller_cap_neg && callee_cap_neg) {
		marieEncryption = LinphoneMediaEncryptionDTLS;
	} else {
		marieEncryption = encryption;
	}

	std::list<LinphoneMediaEncryption> marie_enc_list;
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionZRTP)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionDTLS)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionSRTP)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (!caller_cap_neg || (encryption != LinphoneMediaEncryptionNone)) {
		marie_enc_list.push_back(LinphoneMediaEncryptionNone);
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager *marie = create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params,
	                                                                               caller_cap_neg, FALSE, TRUE);
	linphone_core_enable_capability_negotiation_reinvite(marie->lc, caller_cap_neg_reinvite);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionSRTP)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionSRTP);
	}
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionZRTP)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionZRTP);
	}
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionNone)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionNone);
	}
	if (!callee_cap_neg || (encryption != LinphoneMediaEncryptionDTLS)) {
		pauline_enc_list.push_back(LinphoneMediaEncryptionDTLS);
	}

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params,
	    callee_cap_neg, FALSE, TRUE);
	linphone_core_enable_capability_negotiation_reinvite(pauline->lc, callee_cap_neg_reinvite);

	bctbx_list_t *marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption));
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, caller_cap_neg);
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	bctbx_list_free(marie_call_enc);

	linphone_call_params_set_media_encryption(marie_params, marieEncryption);

	bctbx_list_t *pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, callee_cap_neg);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);
	bctbx_list_free(pauline_call_enc);

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

	LinphoneNatPolicy *marie_nat_policy = get_nat_policy_for_call(marie, marieCall);
	const bool_t marie_ice_enabled = linphone_nat_policy_ice_enabled(marie_nat_policy);
	LinphoneNatPolicy *pauline_nat_policy = get_nat_policy_for_call(pauline, paulineCall);
	const bool_t pauline_ice_enabled = linphone_nat_policy_ice_enabled(pauline_nat_policy);

	bool capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(marieCall));
	bool sendReInvite = ((caller_cap_neg && callee_cap_neg && capabilityNegotiationReinviteEnabled) ||
	                     (marie_ice_enabled && pauline_ice_enabled));
	int expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, expectedStreamsRunning, int, "%d");

	// Check that encryption has not changed after sending update
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                expectedEncryption, int, "%i");
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
	LinphoneCallParams *params0 = linphone_core_create_call_params(pauline->lc, paulineCall);
	linphone_call_params_set_media_encryption(params0, encryption);
	linphone_call_params_enable_video(params0, TRUE);
	bctbx_list_t *pauline_call_enc0 = NULL;
	if (caller_cap_neg && callee_cap_neg) {
		expectedEncryption = LinphoneMediaEncryptionDTLS;
	}
	ms_message("%s changes encryption to %s", linphone_core_get_identity(pauline->lc),
	           linphone_media_encryption_to_string(expectedEncryption));
	pauline_call_enc0 = bctbx_list_append(pauline_call_enc0, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	pauline_call_enc0 = bctbx_list_append(pauline_call_enc0, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	linphone_call_params_set_supported_encryptions(params0, pauline_call_enc0);
	bctbx_list_free(pauline_call_enc0);
	linphone_call_update(paulineCall, params0);
	linphone_call_params_unref(params0);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(paulineCall));
	sendReInvite = (caller_cap_neg && callee_cap_neg && capabilityNegotiationReinviteEnabled);
	expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

	if (pauline_ice_enabled && marie_ice_enabled) {
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded,
	                        (pauline_stat.number_of_IframeDecoded + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
	                        (marie_stat.number_of_IframeDecoded + 1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
	                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	// If capability negotiation is enabled on both sides, DTLS is chosen
	// Callee:
	// Optional encryptions: DTLS and ZRTP
	// Default: SRTP
	// Caller:
	// Optional encryptions: ZRTP and DTLS
	// Default: SRTP if capability negotiation is disabled, DTLS otherwise
	marie_stat = marie->stat;
	pauline_stat = pauline->stat;
	LinphoneCallParams *params1 = linphone_core_create_call_params(marie->lc, marieCall);
	linphone_call_params_set_media_encryption(params1, marieEncryption);
	bctbx_list_t *marie_call_enc1 = NULL;
	if (caller_cap_neg && callee_cap_neg) {
		expectedEncryption = LinphoneMediaEncryptionZRTP;
	}
	ms_message("%s changes encryption to %s", linphone_core_get_identity(marie->lc),
	           linphone_media_encryption_to_string(expectedEncryption));
	marie_call_enc1 = bctbx_list_append(marie_call_enc1, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	marie_call_enc1 = bctbx_list_append(marie_call_enc1, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	linphone_call_params_set_supported_encryptions(params1, marie_call_enc1);
	bctbx_list_free(marie_call_enc1);
	linphone_call_update(marieCall, params1);
	linphone_call_params_unref(params1);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdating,
	                        (marie_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	capabilityNegotiationReinviteEnabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(marieCall));
	sendReInvite = (caller_cap_neg && callee_cap_neg && capabilityNegotiationReinviteEnabled);
	expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

	/*wait for reINVITEs to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

	liblinphone_tester_set_next_video_frame_decoded_cb(marieCall);
	liblinphone_tester_set_next_video_frame_decoded_cb(paulineCall);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded,
	                        (pauline_stat.number_of_IframeDecoded + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
	                        (marie_stat.number_of_IframeDecoded + 1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marieCall)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(paulineCall)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marieCall)));

	if ((expectedEncryption == LinphoneMediaEncryptionDTLS) || (expectedEncryption == LinphoneMediaEncryptionZRTP)) {
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEncryptedOn,
		                              marie_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEncryptedOn,
		                              pauline_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
	}

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marieCall) {
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                expectedEncryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                expectedEncryption, int, "%i");
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
	                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_changes_enc_on_update_cap_neg_caller(void) {
	call_changes_enc_on_update_base(TRUE, FALSE, TRUE, TRUE);
}

static void call_changes_enc_on_update_cap_neg_callee(void) {
	call_changes_enc_on_update_base(FALSE, TRUE, TRUE, TRUE);
}

static void call_changes_enc_on_update_cap_neg_both_sides_with_reinvite(void) {
	call_changes_enc_on_update_base(TRUE, TRUE, TRUE, TRUE);
}

static void call_changes_enc_on_update_cap_neg_both_sides_without_reinvite(void) {
	call_changes_enc_on_update_base(TRUE, TRUE, FALSE, FALSE);
}

static void back_to_back_calls_cap_neg_base(bool_t enable_cap_neg_both_sides) {
	std::list<LinphoneMediaEncryption> marie_enc_list;
	marie_enc_list.push_back(LinphoneMediaEncryptionNone);

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionZRTP;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = marie_enc_list;

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);

	std::list<LinphoneMediaEncryption> pauline_enc_list;
	pauline_enc_list.push_back(LinphoneMediaEncryptionNone);

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionSRTP;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = pauline_enc_list;

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, TRUE);

	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Expected encryption
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	bctbx_list_t *marie_call_enc = NULL;
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionSRTP));
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	bctbx_list_free(marie_call_enc);
	marie_call_enc = NULL;

	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, enable_cap_neg_both_sides);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	bctbx_list_t *pauline_call_enc = NULL;
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionNone));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);
	bctbx_list_free(pauline_call_enc);
	pauline_call_enc = NULL;

	// TODO: Delete the following 2 lines when relying only on call params to check validity of SDP
	linphone_core_set_media_encryption(pauline->lc, encryption);
	linphone_core_set_media_encryption(marie->lc, encryption);

	// Caller (Marie) supports SRTP and DTLS as potential configurations and DTLS as default one
	// Callee (Pauline) supports ZRTP, no encryption and DTLS as potential configurations and DTLS as default one
	// DTLS is expected to be chosen
	encrypted_call_with_params_base(marie, pauline, encryption, marie_params, pauline_params, TRUE);
	if (linphone_core_get_current_call(marie->lc)) {
		end_call(marie, pauline);
	}

	encryption = LinphoneMediaEncryptionSRTP;
	if (marie_params) {
		linphone_call_params_unref(marie_params);
		marie_params = NULL;
	}
	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	if (marie_call_enc) {
		bctbx_list_free(marie_call_enc);
		marie_call_enc = NULL;
	}
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	bctbx_list_free(marie_call_enc);
	marie_call_enc = NULL;

	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
		pauline_params = NULL;
	}
	pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, enable_cap_neg_both_sides);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	if (pauline_call_enc) {
		bctbx_list_free(pauline_call_enc);
		pauline_call_enc = NULL;
	}
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionNone));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);
	bctbx_list_free(pauline_call_enc);
	pauline_call_enc = NULL;

	// TODO: Delete the following 2 lines when relying only on call params to check validity of SDP
	linphone_core_set_media_encryption(pauline->lc, encryption);
	linphone_core_set_media_encryption(marie->lc, encryption);

	// Caller (Pauline) supports DTLS, no encryption and SRTP as potential configurations and SRTP as default one
	// Callee (Marie) supports ZRTP and SRTP as potential configurations and SRTP as default one
	// DTLS is expected to be chosen
	encrypted_call_with_params_base(pauline, marie, encryption, marie_params, pauline_params, TRUE);
	if (linphone_core_get_current_call(pauline->lc)) {
		end_call(pauline, marie);
	}

	encryption = LinphoneMediaEncryptionZRTP;
	if (marie_params) {
		linphone_call_params_unref(marie_params);
		marie_params = NULL;
	}
	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	if (marie_call_enc) {
		bctbx_list_free(marie_call_enc);
		marie_call_enc = NULL;
	}
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionNone));
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionZRTP));
	marie_call_enc = bctbx_list_append(marie_call_enc, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(marie_params, marie_call_enc);
	bctbx_list_free(marie_call_enc);
	marie_call_enc = NULL;

	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
		pauline_params = NULL;
	}
	pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, enable_cap_neg_both_sides);
	linphone_call_params_set_media_encryption(pauline_params, encryption);
	if (pauline_call_enc) {
		bctbx_list_free(pauline_call_enc);
		pauline_call_enc = NULL;
	}
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionDTLS));
	pauline_call_enc = bctbx_list_append(pauline_call_enc, LINPHONE_INT_TO_PTR(encryption));
	linphone_call_params_set_supported_encryptions(pauline_params, pauline_call_enc);
	bctbx_list_free(pauline_call_enc);
	pauline_call_enc = NULL;

	// TODO: Delete the following 2 lines when relying only on call params to check validity of SDP
	linphone_core_set_media_encryption(pauline->lc, encryption);
	linphone_core_set_media_encryption(marie->lc, encryption);

	// Caller (Marie) supports SRTP, DTLS and ZRTP as potential configurations and ZRTP as default one
	// Callee (Pauline) supports no encryption and ZRTP as potential configurations and ZRTP as default one
	// DTLS is expected to be chosen
	encrypted_call_with_params_base(pauline, marie, encryption, marie_params, pauline_params, TRUE);
	if (linphone_core_get_current_call(pauline->lc)) {
		end_call(pauline, marie);
	}

	if (marie_call_enc) {
		bctbx_list_free(marie_call_enc);
		marie_call_enc = NULL;
	}
	if (pauline_call_enc) {
		bctbx_list_free(pauline_call_enc);
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

static void call_with_update_and_incompatible_encs_in_call_params(void) {
	call_with_update_and_incompatible_encs_in_call_params_base(FALSE);
}

void call_with_encryption_supported_in_call_params_only_base(const LinphoneMediaEncryption encryption) {
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

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, FALSE);
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

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, FALSE);
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, encryption));

	bctbx_list_t *call_enc = NULL;
	call_enc = bctbx_list_append(call_enc, LINPHONE_INT_TO_PTR(encryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_supported_encryptions(marie_params, call_enc);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, 1);
	linphone_call_params_set_supported_encryptions(pauline_params, call_enc);

	bctbx_list_free(call_enc);

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
		check_stream_encryption(marieCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
		                encryption, int, "%i");
	}
	if (paulineCall) {
		check_stream_encryption(paulineCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
		                encryption, int, "%i");
	}

	end_call(pauline, marie);

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_with_default_encryption(const LinphoneMediaEncryption encryption) {

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

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);
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

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, TRUE);
	BC_ASSERT_FALSE(linphone_core_media_encryption_supported(pauline->lc, encryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, 1);
	linphone_call_params_set_media_encryption(marie_params, encryption);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, 1);
	linphone_call_params_set_media_encryption(pauline_params, encryption);

	encrypted_call_with_params_base(marie, pauline, marie_enc_list.front(), marie_params, pauline_params, TRUE);
	if (linphone_core_get_current_call(marie->lc)) {
		end_call(marie, pauline);
	}

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_with_potential_configuration_same_as_actual_configuration_base(const LinphoneMediaEncryption encryption) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, 1);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negociation(pauline->lc, 1);

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc, encryption));
	if (linphone_core_media_encryption_supported(pauline->lc, encryption)) {
		bctbx_list_t *encryption_list = NULL;
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(encryption));
		linphone_core_set_media_encryption_mandatory(pauline->lc, 0);
		linphone_core_set_media_encryption(pauline->lc, encryption);
		linphone_core_set_supported_media_encryptions(pauline->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, encryption));
		if (encryption_list) {
			bctbx_list_free(encryption_list);
		}
	}

	linphone_core_set_media_encryption_mandatory(marie->lc, 0);
	linphone_core_set_media_encryption(marie->lc, encryption);
	// Desired encryption is put last
	bctbx_list_t *encryption_list = create_confg_encryption_preference_list_from_param_preferences(
	    set_encryption_preference_with_priority(encryption, true));
	linphone_core_set_supported_media_encryptions(marie->lc, encryption_list);
	if (encryption_list) {
		bctbx_list_free(encryption_list);
	}

	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(marie->lc, encryption));
	BC_ASSERT_TRUE(linphone_core_get_media_encryption(marie->lc) == encryption);
	BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(pauline->lc, encryption));
	BC_ASSERT_TRUE(linphone_core_get_media_encryption(pauline->lc) == encryption);

	encrypted_call_base(marie, pauline, encryption, TRUE, TRUE, FALSE);
	if (linphone_core_get_current_call(marie->lc) && linphone_core_get_current_call(pauline->lc)) {
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionNone);
}

void simple_call_with_capability_negotiations_removed_after_update(LinphoneCoreManager *caller,
                                                                   LinphoneCoreManager *callee,
                                                                   const LinphoneMediaEncryption optionalEncryption) {
	linphone_core_enable_capability_negociation(caller->lc, 1);
	linphone_core_enable_capability_negociation(callee->lc, 1);
	bctbx_list_t *encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(optionalEncryption));

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(callee->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(callee->lc, 0);
		linphone_core_set_media_encryption(callee->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(callee->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(callee->lc, optionalEncryption));
	}

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(caller->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(caller->lc, 0);
		linphone_core_set_media_encryption(caller->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(caller->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(caller->lc, optionalEncryption));
	}

	if (encryption_list) {
		bctbx_list_free(encryption_list);
	}

	encrypted_call_base(caller, callee, optionalEncryption, TRUE, TRUE, FALSE);

	LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(callerCall);
	LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(calleeCall);

	stats caller_stat = caller->stat;
	stats callee_stat = callee->stat;
	LinphoneCallParams *params0 = linphone_core_create_call_params(callee->lc, calleeCall);
	linphone_call_params_enable_capability_negotiations(params0, FALSE);
	linphone_call_update(calleeCall, params0);
	linphone_call_params_unref(params0);
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallUpdating,
	                        (callee_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (caller_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
	                        (callee_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
	                        (caller_stat.number_of_LinphoneCallStreamsRunning + 1)));

	LinphoneMediaEncryption encryptionAfterUpdate = LinphoneMediaEncryptionNone;
	LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
	bool dummyPotentialConfigurationChosen = false;
	get_expected_encryption_from_call_params(calleeCall, callerCall, &expectedEncryption,
	                                         &dummyPotentialConfigurationChosen);
	int expectedStreamsRunning = 1;

	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallStreamsRunning,
	                (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallStreamsRunning,
	                (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	liblinphone_tester_check_rtcp(caller, callee);

	// Check that encryption has not changed after sending update
	BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");
	if (callerCall) {
		check_stream_encryption(callerCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
		                expectedEncryption, int, "%i");
	}
	if (calleeCall) {
		check_stream_encryption(calleeCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
		                expectedEncryption, int, "%i");
	}

	wait_for_until(callee->lc, caller->lc, NULL, 5, 2000);

	caller_stat = caller->stat;
	callee_stat = callee->stat;
	LinphoneCallParams *params1 = linphone_core_create_call_params(caller->lc, callerCall);
	linphone_call_update(callerCall, params1);
	linphone_call_params_unref(params1);
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallUpdating,
	                        (caller_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (callee_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
	                        (callee_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
	                        (caller_stat.number_of_LinphoneCallStreamsRunning + 1)));

	get_expected_encryption_from_call_params(calleeCall, callerCall, &expectedEncryption,
	                                         &dummyPotentialConfigurationChosen);

	liblinphone_tester_check_rtcp(caller, callee);

	// Check that encryption has not changed after sending update
	BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");
	if (callerCall) {
		check_stream_encryption(callerCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
		                expectedEncryption, int, "%i");
	}
	if (calleeCall) {
		check_stream_encryption(calleeCall);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
		                expectedEncryption, int, "%i");
	}

	wait_for_until(callee->lc, caller->lc, NULL, 5, 2000);

	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallStreamsRunning,
	                (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
	BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallStreamsRunning,
	                (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

	if (callerCall && calleeCall) {
		end_call(caller, callee);
	}
}

void simple_call_with_capability_negotiations(LinphoneCoreManager *caller,
                                              LinphoneCoreManager *callee,
                                              const LinphoneMediaEncryption optionalEncryption,
                                              const LinphoneMediaEncryption expectedEncryption) {
	linphone_core_enable_capability_negociation(caller->lc, 1);
	linphone_core_enable_capability_negociation(callee->lc, 1);
	bctbx_list_t *encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(optionalEncryption));

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(callee->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(callee->lc, 0);
		linphone_core_set_media_encryption(callee->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(callee->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(callee->lc, optionalEncryption));
	}

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(caller->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(caller->lc, 0);
		linphone_core_set_media_encryption(caller->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(caller->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(caller->lc, optionalEncryption));
	}

	if (encryption_list) {
		bctbx_list_free(encryption_list);
	}

	encrypted_call_base(caller, callee, expectedEncryption, TRUE, TRUE, FALSE);
	pause_resume_calls(caller, callee);

	if (linphone_core_get_current_call(caller->lc) && linphone_core_get_current_call(callee->lc)) {
		end_call(caller, callee);
	}
}

static void call_with_no_sdp_cap_neg_on_caller(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_cap_neg_on_callee(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_sdp_200_ack(pauline->lc, TRUE);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_cap_neg_on_both_sides(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_sdp_200_ack(pauline->lc, TRUE);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_avpf_and_cap_neg_on_caller(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_avpf_and_cap_neg_on_callee(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_avpf_and_cap_neg_on_both_sides(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_sdp_200_ack(pauline->lc, TRUE);
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void simple_call_with_capability_negotiations_with_different_encryption_after_resume(
    LinphoneCoreManager *caller,
    LinphoneCoreManager *callee,
    const LinphoneMediaEncryption optionalEncryption,
    const LinphoneMediaEncryption encryptionAfterResume) {
	linphone_core_enable_capability_negociation(caller->lc, 1);
	linphone_core_enable_capability_negociation(callee->lc, 1);
	bctbx_list_t *encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(optionalEncryption));

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(callee->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(callee->lc, 0);
		linphone_core_set_media_encryption(callee->lc, encryptionAfterResume);
		linphone_core_set_supported_media_encryptions(callee->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(callee->lc, optionalEncryption));
	}

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(caller->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(caller->lc, 0);
		if (optionalEncryption == LinphoneMediaEncryptionZRTP) {
			linphone_core_set_media_encryption(caller->lc, (encryptionAfterResume == LinphoneMediaEncryptionDTLS)
			                                                   ? LinphoneMediaEncryptionSRTP
			                                                   : LinphoneMediaEncryptionDTLS);
		} else {
			linphone_core_set_media_encryption(caller->lc, LinphoneMediaEncryptionNone);
		}
		linphone_core_set_supported_media_encryptions(caller->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(caller->lc, optionalEncryption));
	}

	if (encryption_list) {
		bctbx_list_free(encryption_list);
		encryption_list = NULL;
	}

	encrypted_call_base(caller, callee, optionalEncryption, TRUE, TRUE, FALSE);

	LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(callerCall);
	LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(calleeCall);

	if (calleeCall && callerCall) {

		// Pause callee call
		BC_ASSERT_TRUE(pause_call_1(callee, calleeCall, caller, callerCall));
		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		// Resume callee call
		reset_counters(&caller->stat);
		reset_counters(&callee->stat);
		stats caller_stat = caller->stat;
		stats callee_stat = callee->stat;

		// Update call params before resuming call
		LinphoneCallParams *callerParams = linphone_core_create_call_params(caller->lc, callerCall);
		encryption_list = bctbx_list_append(NULL, LINPHONE_INT_TO_PTR(encryptionAfterResume));
		linphone_call_params_set_supported_encryptions(callerParams, encryption_list);
		if (encryption_list) {
			bctbx_list_free(encryption_list);
			encryption_list = NULL;
		}
		L_GET_PRIVATE(std::static_pointer_cast<LinphonePrivate::MediaSession>(
		                  LinphonePrivate::Call::toCpp(callerCall)->getActiveSession()))
		    ->setParams(new LinphonePrivate::MediaSessionParams(*L_GET_CPP_PTR_FROM_C_OBJECT(callerParams)));
		linphone_call_params_unref(callerParams);
		linphone_call_resume(calleeCall);
		LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallResuming,
		                        callee_stat.number_of_LinphoneCallResuming + 1));
		if (encryptionAfterResume == optionalEncryption) {
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
			                        callee_stat.number_of_LinphoneCallStreamsRunning + 1));
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
			                        caller_stat.number_of_LinphoneCallStreamsRunning + 1));

			bool potentialConfigurationChosen = false;
			get_expected_encryption_from_call_params(calleeCall, callerCall, &encryption,
			                                         &potentialConfigurationChosen);

			int expectedStreamsRunning = 1 + ((potentialConfigurationChosen) ? 1 : 0);

			/*wait for reINVITEs to complete*/
			BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
			                        (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
			BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
			                        (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

			BC_ASSERT_EQUAL(encryptionAfterResume, encryption, int, "%i");

			wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

			liblinphone_tester_check_rtcp(caller, callee);

			BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 70, int, "%i");
			LinphoneCallStats *calleeStats = linphone_call_get_audio_stats(linphone_core_get_current_call(callee->lc));
			BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(calleeStats), 70, int, "%i");
			linphone_call_stats_unref(calleeStats);
			calleeStats = NULL;

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_until(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallEncryptedOn,
				                              caller_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
				BC_ASSERT_TRUE(wait_for_until(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallEncryptedOn,
				                              callee_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
			}

		} else {
			encryption = optionalEncryption;
			// Resume fails because requested encryption is not supported
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallPaused,
			                        callee_stat.number_of_LinphoneCallPaused + 1));

			wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

			BC_ASSERT_LOWER(caller->stat.number_of_rtcp_received, 5, int, "%d");
			BC_ASSERT_LOWER(callee->stat.number_of_rtcp_received, 5, int, "%d");

			BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 0, int, "%i");
			LinphoneCall *calleeCall2 = linphone_core_get_current_call(callee->lc);
			BC_ASSERT_PTR_NOT_NULL(calleeCall2);
			if (calleeCall2) {
				LinphoneCallStats *calleeStats = linphone_call_get_audio_stats(calleeCall2);
				BC_ASSERT_EQUAL((int)linphone_call_stats_get_download_bandwidth(calleeStats), 0, int, "%d");
				linphone_call_stats_unref(calleeStats);
				calleeStats = NULL;
			}
		}

		if (calleeCall) {
			check_stream_encryption(calleeCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
			                encryption, int, "%i");
		}
		if (callerCall) {
			check_stream_encryption(callerCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
			                encryption, int, "%i");
		}

		/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
		const rtp_stats_t *stats =
		    rtp_session_get_stats(linphone_call_get_stream(calleeCall, LinphoneStreamTypeAudio)->sessions.rtp_session);
		BC_ASSERT_LOWER((int)stats->cum_packet_loss, 10, int, "%d");

		end_call(caller, callee);
	}
}

void simple_call_with_capability_negotiations_with_resume_and_media_change_base(
    LinphoneCoreManager *caller,
    LinphoneCoreManager *callee,
    const LinphoneMediaEncryption optionalEncryption,
    const LinphoneMediaEncryption encryptionAfterResume) {
	linphone_core_enable_capability_negociation(caller->lc, 1);
	linphone_core_enable_capability_negociation(callee->lc, 1);
	bctbx_list_t *encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(optionalEncryption));

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(callee->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(callee->lc, 0);
		linphone_core_set_media_encryption(callee->lc, encryptionAfterResume);
		linphone_core_set_supported_media_encryptions(callee->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(callee->lc, optionalEncryption));
	}

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc, optionalEncryption));
	if (linphone_core_media_encryption_supported(caller->lc, optionalEncryption)) {
		linphone_core_set_media_encryption_mandatory(caller->lc, 0);
		linphone_core_set_media_encryption(caller->lc, LinphoneMediaEncryptionNone);
		linphone_core_set_supported_media_encryptions(caller->lc, encryption_list);
		BC_ASSERT_TRUE(linphone_core_is_media_encryption_supported(caller->lc, optionalEncryption));
	}

	if (encryption_list) {
		bctbx_list_free(encryption_list);
		encryption_list = NULL;
	}

	encrypted_call_base(caller, callee, optionalEncryption, TRUE, TRUE, FALSE);

	LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(callerCall);
	LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(calleeCall);

	if (calleeCall && callerCall) {

		// Pause callee call
		BC_ASSERT_TRUE(pause_call_1(callee, calleeCall, caller, callerCall));
		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		// Resume callee call
		reset_counters(&caller->stat);
		reset_counters(&callee->stat);
		stats caller_stat = caller->stat;
		stats callee_stat = callee->stat;

		linphone_call_resume(calleeCall);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallResuming,
		                        callee_stat.number_of_LinphoneCallResuming + 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        caller_stat.number_of_LinphoneCallStreamsRunning + 1));

		caller_stat = caller->stat;
		callee_stat = callee->stat;

		LinphoneCallParams *callerParams = linphone_core_create_call_params(caller->lc, callerCall);
		encryption_list = bctbx_list_append(NULL, LINPHONE_INT_TO_PTR(encryptionAfterResume));
		linphone_call_params_set_supported_encryptions(callerParams, encryption_list);
		if (encryption_list) {
			bctbx_list_free(encryption_list);
			encryption_list = NULL;
		}
		linphone_call_update(callerCall, callerParams);
		linphone_call_params_unref(callerParams);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallUpdating,
		                        (caller_stat.number_of_LinphoneCallUpdating + 1)));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallUpdatedByRemote,
		                        (callee_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        (callee_stat.number_of_LinphoneCallStreamsRunning + 1)));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        (caller_stat.number_of_LinphoneCallStreamsRunning + 1)));

		if (optionalEncryption == LinphoneMediaEncryptionZRTP) {
			BC_ASSERT_TRUE(wait_for_until(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallGoClearAckSent,
			                              callee_stat.number_of_LinphoneCallGoClearAckSent + 1, 10000));
		}

		bool potentialConfigurationChosen = false;
		LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		get_expected_encryption_from_call_params(calleeCall, callerCall, &encryption, &potentialConfigurationChosen);

		int expectedStreamsRunning = 1 + ((potentialConfigurationChosen) ? 1 : 0);

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

		BC_ASSERT_EQUAL(encryptionAfterResume, encryption, int, "%i");

		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		liblinphone_tester_check_rtcp(caller, callee);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 70, int, "%i");
		LinphoneCallStats *calleeStats = linphone_call_get_audio_stats(linphone_core_get_current_call(callee->lc));
		BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(calleeStats), 70, int, "%i");
		linphone_call_stats_unref(calleeStats);
		calleeStats = NULL;

		if (calleeCall) {
			check_stream_encryption(calleeCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
			                encryption, int, "%i");
		}
		if (callerCall) {
			check_stream_encryption(callerCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
			                encryption, int, "%i");
		}

		/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
		const rtp_stats_t *stats =
		    rtp_session_get_stats(linphone_call_get_stream(calleeCall, LinphoneStreamTypeAudio)->sessions.rtp_session);
		BC_ASSERT_LOWER((int)stats->cum_packet_loss, 10, int, "%d");

		callerParams = linphone_core_create_call_params(caller->lc, callerCall);
		encryption_list = bctbx_list_append(NULL, LINPHONE_INT_TO_PTR(optionalEncryption));
		linphone_call_params_set_supported_encryptions(callerParams, encryption_list);
		if (encryption_list) {
			bctbx_list_free(encryption_list);
			encryption_list = NULL;
		}
		linphone_call_update(callerCall, callerParams);
		linphone_call_params_unref(callerParams);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallUpdating,
		                        (caller_stat.number_of_LinphoneCallUpdating + 1)));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallUpdatedByRemote,
		                        (callee_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        (callee_stat.number_of_LinphoneCallStreamsRunning + 1)));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        (caller_stat.number_of_LinphoneCallStreamsRunning + 1)));

		potentialConfigurationChosen = false;
		encryption = LinphoneMediaEncryptionNone;
		get_expected_encryption_from_call_params(callerCall, calleeCall, &encryption, &potentialConfigurationChosen);

		expectedStreamsRunning = 1 + ((potentialConfigurationChosen) ? 1 : 0);

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        (caller_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        (callee_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
		if ((encryptionAfterResume == LinphoneMediaEncryptionNone) &&
		    ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP))) {
			BC_ASSERT_TRUE(wait_for_until(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallEncryptedOn,
			                              caller_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
			BC_ASSERT_TRUE(wait_for_until(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallEncryptedOn,
			                              callee_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
		}

		BC_ASSERT_EQUAL(optionalEncryption, encryption, int, "%i");

		wait_for_until(callee->lc, caller->lc, NULL, 5, 10000);

		liblinphone_tester_check_rtcp(caller, callee);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(caller), 70, int, "%i");
		calleeStats = linphone_call_get_audio_stats(linphone_core_get_current_call(callee->lc));
		BC_ASSERT_GREATER((int)linphone_call_stats_get_download_bandwidth(calleeStats), 70, int, "%i");
		linphone_call_stats_unref(calleeStats);
		calleeStats = NULL;

		if (calleeCall) {
			check_stream_encryption(calleeCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(calleeCall)),
			                encryption, int, "%i");
		}
		if (callerCall) {
			check_stream_encryption(callerCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(callerCall)),
			                encryption, int, "%i");
		}

		/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
		stats =
		    rtp_session_get_stats(linphone_call_get_stream(calleeCall, LinphoneStreamTypeAudio)->sessions.rtp_session);
		BC_ASSERT_LOWER((int)stats->cum_packet_loss, 10, int, "%d");

		end_call(caller, callee);
	}
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

void call_with_mandatory_encryption_base(const LinphoneMediaEncryption encryption,
                                         const bool_t caller_capability_negotiation,
                                         const bool_t callee_capability_negotiation) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
	marie_enc_params.level = E_MANDATORY;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_MANDATORY;
	call_with_encryption_test_base(marie_enc_params, caller_capability_negotiation, FALSE, pauline_enc_params,
	                               callee_capability_negotiation, FALSE, FALSE);
}

void call_from_opt_enc_to_enc_base(const LinphoneMediaEncryption encryption, bool_t opt_enc_to_enc) {
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
		call_with_encryption_test_base(optional_enc_mgr_params, TRUE, FALSE, mandatory_enc_mgr_params, TRUE, FALSE,
		                               FALSE);
	} else {
		call_with_encryption_test_base(mandatory_enc_mgr_params, TRUE, FALSE, optional_enc_mgr_params, TRUE, FALSE,
		                               FALSE);
	}
}

void call_from_opt_enc_to_none_base(const LinphoneMediaEncryption encryption,
                                    bool_t opt_enc_to_none,
                                    const bool_t enable_video) {
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

void call_with_optional_encryption_on_both_sides_base(const LinphoneMediaEncryption encryption,
                                                      const bool_t enable_video) {
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
	pauline_enc_params.encryption = encryption;
	pauline_enc_params.level = E_OPTIONAL;
	pauline_enc_params.preferences = set_encryption_preference_with_priority(encryption, true);

	call_with_encryption_test_base(marie_enc_params, TRUE, FALSE, pauline_enc_params, TRUE, FALSE, enable_video);
}

void call_with_toggling_encryption_base(const LinphoneMediaEncryption encryption) {
	std::list<LinphoneMediaEncryption> enc_list{encryption};

	encryption_params marie_enc_params;
	marie_enc_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_params.level = E_OPTIONAL;
	marie_enc_params.preferences = enc_list;

	encryption_params pauline_enc_params;
	pauline_enc_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_params.level = E_OPTIONAL;
	pauline_enc_params.preferences = enc_list;

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_params, TRUE, FALSE, TRUE);
	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_params, TRUE, FALSE,
	    TRUE);

	if (encryption == LinphoneMediaEncryptionZRTP) {
		linphone_core_enable_zrtp_go_clear(marie->lc, TRUE);
		linphone_core_enable_zrtp_go_clear(pauline->lc, TRUE);
	}

	encrypted_call_base(marie, pauline, encryption, TRUE, TRUE, TRUE);

	LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marieCall);
	LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(paulineCall);

	if (marieCall && paulineCall) {
		linphone_call_create_cbs_security_level_downgraded(marieCall);
		linphone_call_create_cbs_security_level_downgraded(paulineCall);
		stats marie_stat = marie->stat;
		stats pauline_stat = pauline->stat;
		LinphoneMediaEncryption downgradedEncryption = LinphoneMediaEncryptionNone;
		ms_message("%s downgrades encryption to %s", linphone_core_get_identity(pauline->lc),
		           linphone_media_encryption_to_string(downgradedEncryption));
		LinphoneCallParams *params0 = linphone_core_create_call_params(pauline->lc, paulineCall);
		bctbx_list_t *encs0 = NULL;
		encs0 = bctbx_list_append(encs0, LINPHONE_INT_TO_PTR(downgradedEncryption));
		linphone_call_params_set_supported_encryptions(params0, encs0);
		bctbx_list_free(encs0);

		linphone_call_update(paulineCall, params0);
		linphone_call_params_unref(params0);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
		                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
		                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

		LinphoneMediaEncryption encryptionAfterUpdate = LinphoneMediaEncryptionNone;
		LinphoneMediaEncryption expectedEncryption = LinphoneMediaEncryptionNone;
		bool potentialConfigurationChosen = false;
		get_expected_encryption_from_call_params(paulineCall, marieCall, &expectedEncryption,
		                                         &potentialConfigurationChosen);
		BC_ASSERT_FALSE(potentialConfigurationChosen);
		int expectedStreamsRunning = 1;

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

		liblinphone_tester_check_rtcp(marie, pauline);

		wait_for_until(marie->lc, pauline->lc, NULL, 5, 2000);

		// Check that encryption has not changed after sending update
		BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");
		if (marieCall) {
			check_stream_encryption(marieCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
			                expectedEncryption, int, "%i");
		}
		if (paulineCall) {
			check_stream_encryption(paulineCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
			                expectedEncryption, int, "%i");
		}

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
		                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
		                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEncryptedOff,
		                        marie_stat.number_of_LinphoneCallEncryptedOff + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEncryptedOff,
		                        pauline_stat.number_of_LinphoneCallEncryptedOff + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallSecurityLevelDowngraded,
		                        marie_stat.number_of_LinphoneCallSecurityLevelDowngraded + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallSecurityLevelDowngraded,
		                        pauline_stat.number_of_LinphoneCallSecurityLevelDowngraded + 1));

		pause_resume_calls(marie, pauline);

		marie_stat = marie->stat;
		pauline_stat = pauline->stat;

		ms_message("%s restores encryption to %s", linphone_core_get_identity(pauline->lc),
		           linphone_media_encryption_to_string(encryption));
		LinphoneCallParams *params1 = linphone_core_create_call_params(pauline->lc, paulineCall);
		bctbx_list_t *encs1 = NULL;
		encs1 = bctbx_list_append(encs1, LINPHONE_INT_TO_PTR(encryption));
		linphone_call_params_set_supported_encryptions(params1, encs1);
		bctbx_list_free(encs1);

		linphone_call_update(paulineCall, params1);
		linphone_call_params_unref(params1);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
		                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
		                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

		encryptionAfterUpdate = encryption;
		get_expected_encryption_from_call_params(paulineCall, marieCall, &expectedEncryption,
		                                         &potentialConfigurationChosen);
		BC_ASSERT_TRUE(potentialConfigurationChosen);
		bool_t capabilityNegotiationReinviteEnabled =
		    linphone_call_params_capability_negotiation_reinvite_enabled(linphone_call_get_params(paulineCall));
		bool sendReInvite = (potentialConfigurationChosen && capabilityNegotiationReinviteEnabled);
		expectedStreamsRunning = 1 + ((sendReInvite) ? 1 : 0);

		/*wait for reINVITEs to complete*/
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning)));

		liblinphone_tester_check_rtcp(marie, pauline);

		wait_for_until(marie->lc, pauline->lc, NULL, 5, 2000);

		// Check that encryption has not changed after sending update
		BC_ASSERT_EQUAL(expectedEncryption, encryptionAfterUpdate, int, "%i");

		if ((expectedEncryption == LinphoneMediaEncryptionDTLS) ||
		    (expectedEncryption == LinphoneMediaEncryptionZRTP)) {
			BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEncryptedOn,
			                              marie_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
			BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEncryptedOn,
			                              pauline_stat.number_of_LinphoneCallEncryptedOn + 1, 10000));
		}

		if (marieCall) {
			check_stream_encryption(marieCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marieCall)),
			                expectedEncryption, int, "%i");
		}
		if (paulineCall) {
			check_stream_encryption(paulineCall);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(paulineCall)),
			                expectedEncryption, int, "%i");
		}

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,
		                (pauline_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
		                (marie_stat.number_of_LinphoneCallStreamsRunning + expectedStreamsRunning), int, "%d");

		pause_resume_calls(marie, pauline);

		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_200ok_lost(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, TRUE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	linphone_core_set_nortp_timeout(marie->lc, 100);

	LinphoneCall *in_call = NULL;
	LinphoneCall *out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(out_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(marie->lc));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(marie->lc));

	linphone_call_accept(in_call);

	// Pauline goes offline so that it cannot send the ACK
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1, 90000));

	check_media_stream(in_call, TRUE);
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(in_call)),
	                LinphoneMediaEncryptionSRTP, int, "%i");

	// Pauline comes back online to answer the BYE
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	const LinphoneErrorInfo *ei = linphone_call_get_error_info(in_call);
	if (BC_ASSERT_PTR_NOT_NULL(ei)) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(ei), 408, int, "%d");
	}

	// Check that Pauline never went to the streams running state
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, 0, int, "%d");

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ack_not_sent(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negociation(marie->lc, TRUE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	linphone_core_set_nortp_timeout(marie->lc, 100);

	LinphoneCall *in_call = NULL;
	LinphoneCall *out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(out_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(marie->lc));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(marie->lc));

	// Tell the dialog to loose all ACK
	LinphonePrivate::SalCallOp *op = LinphonePrivate::Call::toCpp(in_call)->getOp();
	op->simulateLostAckOnDialog(true);

	linphone_call_accept(in_call);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1, 90000));

	check_media_stream(in_call, TRUE);
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(in_call)),
	                LinphoneMediaEncryptionSRTP, int, "%i");

	const LinphoneErrorInfo *ei = linphone_call_get_error_info(in_call);
	if (BC_ASSERT_PTR_NOT_NULL(ei)) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(ei), 408, int, "%d");
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t capability_negotiation_tests[] = {
    TEST_NO_TAG("Call with no encryption", call_with_no_encryption),
    TEST_NO_TAG("Call with 200Ok lost", call_with_200ok_lost),
    TEST_NO_TAG("Call with ACK not sent", call_with_ack_not_sent),
    TEST_NO_TAG("Call with capability negotiation failure", call_with_capability_negotiation_failure),
    TEST_NO_TAG("Call with capability negotiation failure and multiple potential configurations",
                call_with_capability_negotiation_failure_multiple_potential_configurations),
    TEST_NO_TAG("Call with capability negotiation disabled at call level",
                call_with_capability_negotiation_disable_call_level),
    TEST_NO_TAG("Call with capability negotiation disabled at core level",
                call_with_capability_negotiation_disable_core_level),
    TEST_NO_TAG("Call with incompatible encryptions in call params", call_with_incompatible_encs_in_call_params),
    TEST_NO_TAG("Call with update and incompatible encryptions in call params",
                call_with_update_and_incompatible_encs_in_call_params),
    TEST_NO_TAG("Call changes encryption with update and capability negotiations on caller",
                call_changes_enc_on_update_cap_neg_caller),
    TEST_NO_TAG("Call changes encryption with update and capability negotiations on callee",
                call_changes_enc_on_update_cap_neg_callee),
    TEST_NO_TAG("Call changes encryption with update and capability negotiations on both sides with reINVITE",
                call_changes_enc_on_update_cap_neg_both_sides_with_reinvite),
    TEST_NO_TAG("Call changes encryption with update and capability negotiations on both sides without reINVITE",
                call_changes_enc_on_update_cap_neg_both_sides_without_reinvite),
    TEST_NO_TAG("Unencrypted call with potential configuration same as actual one",
                unencrypted_call_with_potential_configuration_same_as_actual_configuration),
    TEST_NO_TAG("Back to back call with capability negotiations on one side", back_to_back_calls_cap_neg_one_side),
    TEST_NO_TAG("Back to back call with capability negotiations on both sides", back_to_back_calls_cap_neg_both_sides)};

test_t capability_negotiation_parameters_tests[] = {
    TEST_NO_TAG("Call with tcap line merge on caller", call_with_tcap_line_merge_on_caller),
    TEST_NO_TAG("Call with tcap line merge on callee", call_with_tcap_line_merge_on_callee),
    TEST_NO_TAG("Call with tcap line merge on both sides", call_with_tcap_line_merge_on_both_sides),
    TEST_NO_TAG("Call with no cfg line merge", call_with_no_cfg_lines_merge),
    TEST_NO_TAG("Call with cfg line merge on caller", call_with_cfg_lines_merge_on_caller),
    TEST_NO_TAG("Call with cfg line merge on callee", call_with_cfg_lines_merge_on_callee),
    TEST_NO_TAG("Call with cfg line merge on both sides", call_with_cfg_lines_merge_on_both_sides),
    TEST_NO_TAG("Call with AVPF and capability negotiations on caller", call_with_avpf_and_cap_neg_on_caller),
    TEST_NO_TAG("Call with AVPF and capability negotiations on callee", call_with_avpf_and_cap_neg_on_callee),
    TEST_NO_TAG("Call with AVPF and capability negotiations on both sides", call_with_avpf_and_cap_neg_on_both_sides)};

test_t capability_negotiation_tests_no_sdp[] = {
    TEST_NO_TAG("Call with no SDP and capability negotiations on caller", call_with_no_sdp_cap_neg_on_caller),
    TEST_NO_TAG("Call with no SDP and capability negotiations on callee", call_with_no_sdp_cap_neg_on_callee),
    TEST_NO_TAG("Call with no SDP and capability negotiations on both sides", call_with_no_sdp_cap_neg_on_both_sides),
    TEST_NO_TAG("Call with no SDP on update and capability negotiations on caller",
                call_with_no_sdp_on_update_cap_neg_caller),
    TEST_NO_TAG("Call with no SDP on update and capability negotiations on callee",
                call_with_no_sdp_on_update_cap_neg_callee),
    TEST_NO_TAG("Call with no SDP on update and capability negotiations on both sides with reINVITE",
                call_with_no_sdp_on_update_cap_neg_both_sides_with_reinvite),
    TEST_NO_TAG("Call with no SDP on update and capability negotiations on both sides without reINVITE on caller",
                call_with_no_sdp_on_update_cap_neg_both_sides_without_reinvite_on_caller),
    TEST_NO_TAG("Call with no SDP on update and capability negotiations on both sides without reINVITE on callee",
                call_with_no_sdp_on_update_cap_neg_both_sides_without_reinvite_on_callee),
    TEST_NO_TAG("Call with no SDP on update and capability negotiations on both sides without reINVITE",
                call_with_no_sdp_on_update_cap_neg_both_sides_without_reinvite)};

test_suite_t capability_negotiation_test_suite = {"Capability Negotiation (SDP)",
                                                  NULL,
                                                  NULL,
                                                  liblinphone_tester_before_each,
                                                  liblinphone_tester_after_each,
                                                  sizeof(capability_negotiation_tests) /
                                                      sizeof(capability_negotiation_tests[0]),
                                                  capability_negotiation_tests,
                                                  0,
                                                  2};

test_suite_t capability_negotiation_parameters_test_suite = {"Capability Negotiation (Parameters)",
                                                             NULL,
                                                             NULL,
                                                             liblinphone_tester_before_each,
                                                             liblinphone_tester_after_each,
                                                             sizeof(capability_negotiation_parameters_tests) /
                                                                 sizeof(capability_negotiation_parameters_tests[0]),
                                                             capability_negotiation_parameters_tests,
                                                             0,
                                                             2};

test_suite_t capability_negotiation_no_sdp_test_suite = {"Capability Negotiation (No SDP)",
                                                         NULL,
                                                         NULL,
                                                         liblinphone_tester_before_each,
                                                         liblinphone_tester_after_each,
                                                         sizeof(capability_negotiation_tests_no_sdp) /
                                                             sizeof(capability_negotiation_tests_no_sdp[0]),
                                                         capability_negotiation_tests_no_sdp,
                                                         0,
                                                         2};
