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

#include "c-wrapper/c-wrapper.h"
#include "capability_negotiation_tester.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-call-log.h"
#include "linphone/core.h"
#include "tester_utils.h"

static void call_with_srtp_default_encryption(void) {
	call_with_default_encryption(LinphoneMediaEncryptionSRTP);
}

static void simple_srtp_call_with_capability_negotiations_with_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations_without_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negotiation_reinvite(marie->lc, FALSE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negotiation_reinvite(pauline->lc, FALSE);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations_removed_after_update(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_removed_after_update(marie, pauline, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations_with_no_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionNone);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations_with_zrtp_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations_with_dtls_srtp_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionDTLS);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_srtp_call_with_capability_negotiations_with_resume_and_media_change(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_resume_and_media_change_base(
	    marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_srtp_call_with_capability_negotiations_base(bool_t cfg_lines_merge) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	// AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP is not supported hence it should not be put in the offer or accepted as
	// answer
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 "
	                                                "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");
	linphone_core_enable_cfg_lines_merging(marie->lc, cfg_lines_merge);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	// AES_CM_256_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP and AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP are not
	// supported hence they should not be put in the offer or accepted as answer
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc,
	    "AES_CM_256_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 "
	    "UNAUTHENTICATED_SRTP,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 "
	    "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 ,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");
	linphone_core_enable_cfg_lines_merging(pauline->lc, cfg_lines_merge);

	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);

	// Spaces between words are added on purpose to ensure that parsing of crypto suites is correctly done
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP , AES_CM_128_HMAC_SHA1_32   UNAUTHENTICATED_SRTP");
	linphone_core_set_srtp_crypto_suites(
	    marie->lc, "AES_CM_256_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 "
	               "UNAUTHENTICATED_SRTP,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 "
	               "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");
	simple_call_with_capability_negotiations(pauline, marie, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_srtp_call_with_capability_negotiations_without_cfg_lines_merge(void) {
	unencrypted_srtp_call_with_capability_negotiations_base(FALSE);
}

static void unencrypted_srtp_call_with_capability_negotiations_with_cfg_lines_merge(void) {
	unencrypted_srtp_call_with_capability_negotiations_base(TRUE);
}

static void srtp_call_with_capability_negotiations_caller_unencrypted(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	// AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP is not supported hence it should not be put in the offer or accepted as
	// answer
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 "
	                                                "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionNone);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_capability_negotiations_callee_unencrypted(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	// AES_CM_256_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP and AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP are not
	// supported hence they should not be put in the offer or accepted as answer
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_256_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 "
	                 "UNAUTHENTICATED_SRTP,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP "
	                 "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP");

	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionNone);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_encryption_supported_in_call_params_only(void) {
	call_with_encryption_supported_in_call_params_only_base(LinphoneMediaEncryptionSRTP);
}

static void srtp_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionSRTP);
}

static void encrypted_call_with_suite_mismatch_and_capability_negotiations_base(
    LinphoneCoreManager *caller, LinphoneCoreManager *callee, const LinphoneMediaEncryption optionalEncryption) {

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(callee->lc, optionalEncryption));
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(caller->lc, optionalEncryption));

	bctbx_list_t *encryption_list = NULL;
	encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(optionalEncryption));

	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_capability_negotiations(caller_params, TRUE);
	linphone_call_params_set_supported_encryptions(caller_params, encryption_list);
	linphone_call_params_enable_mandatory_media_encryption(caller_params, 0);
	linphone_call_params_set_media_encryption(caller_params, LinphoneMediaEncryptionNone);
	BC_ASSERT_TRUE(linphone_call_params_is_media_encryption_supported(caller_params, optionalEncryption));

	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_capability_negotiations(callee_params, TRUE);
	linphone_call_params_set_supported_encryptions(callee_params, encryption_list);
	linphone_call_params_enable_mandatory_media_encryption(callee_params, 0);
	linphone_call_params_set_media_encryption(callee_params, LinphoneMediaEncryptionNone);
	BC_ASSERT_TRUE(linphone_call_params_is_media_encryption_supported(callee_params, optionalEncryption));

	if (encryption_list) {
		bctbx_list_free(encryption_list);
	}

	encrypted_call_with_params_base(caller, callee, LinphoneMediaEncryptionNone, caller_params, callee_params, TRUE);

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);

	pause_resume_calls(caller, callee);

	LinphoneCall *callerCall = linphone_core_get_current_call(caller->lc);
	BC_ASSERT_PTR_NOT_NULL(callerCall);
	LinphoneCall *calleeCall = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(calleeCall);

	if (calleeCall && callerCall) {
		end_call(callee, caller);
	}
}

static void srtp_call_with_suite_mismatch_and_capability_negotiations_caller_unencrypted(void) {

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_DISABLED;
	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, FALSE, FALSE, TRUE);

	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_80 "
	                                                "UNENCRYPTED_SRTP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_DISABLED;
	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, FALSE,
	    FALSE, TRUE);

	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_CM_256_HMAC_SHA1_80 , AES_CM_256_HMAC_SHA1_32");

	encrypted_call_with_suite_mismatch_and_capability_negotiations_base(marie, pauline, LinphoneMediaEncryptionSRTP);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_suite_mismatch_and_capability_negotiations_callee_unencrypted(void) {

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_DISABLED;
	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, FALSE, FALSE, TRUE);
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80");

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_DISABLED;
	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, FALSE,
	    FALSE, TRUE);
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP");

	encrypted_call_with_suite_mismatch_and_capability_negotiations_base(marie, pauline, LinphoneMediaEncryptionSRTP);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE);
}

static void srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionSRTP, FALSE, TRUE);
}

static void srtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionSRTP, TRUE);
}

static void srtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionSRTP, FALSE);
}

static void srtp_call_from_enc_to_no_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE, TRUE);
}

static void srtp_call_from_no_enc_to_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE, FALSE);
}

static void srtp_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE);
}

static void srtp_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE);
}

static void srtp_video_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, TRUE, TRUE);
}

static void srtp_video_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionSRTP, FALSE, TRUE);
}

static void srtp_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionSRTP, FALSE);
}

static void srtp_video_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionSRTP, TRUE);
}

static void srtp_call_with_video_and_capability_negotiation(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionSRTP; // Desired encryption
	call_with_video_and_capability_negotiation_base(encryption);
}

static void
call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update_base(bool_t pauline_supports_unencrypted) {
	const LinphoneMediaEncryption optionalEncryption = LinphoneMediaEncryptionSRTP;

	std::list<LinphoneMediaEncryption> enc_list{optionalEncryption};

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = enc_list;

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, TRUE);
	if (pauline_supports_unencrypted) {
		// AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP is not supported hence it should not be put in the offer or
		// accepted as answer
		linphone_core_set_srtp_crypto_suites(pauline->lc,
		                                     "AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP, AES_CM_256_HMAC_SHA1_32 "
		                                     "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP");
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = enc_list;

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);
	// AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP is not supported hence it should not be put in the offer or accepted as
	// answer
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 "
	                                                "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc, optionalEncryption));
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, optionalEncryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, TRUE);
	linphone_call_params_add_custom_sdp_attribute(
	    marie_params, "acap",
	    "870 crypto:99 AES_CM_256_HMAC_SHA1_32 inline:fWgTsTLqHc/xC7VQl7air+at/Ko1DpXudbS0KG3s UNENCRYPTED_SRTCP");
	// This extra attribute will be added to the 200Ok to Pauline's initial INVITE. It should be ignored
	linphone_call_params_add_custom_sdp_media_attribute(marie_params, LinphoneStreamTypeAudio, "pcfg", "36 a=870 t=1");
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, TRUE);
	linphone_call_params_add_custom_sdp_attribute(
	    pauline_params, "acap",
	    "999 crypto:25 AES_CM_256_HMAC_SHA1_80 inline:fWgTsTLqHc/xC7VQl7air+at/Ko1DpXudbS0KG3s UNENCRYPTED_SRTP "
	    "UNENCRYPTED_SRTCP");
	linphone_call_params_add_custom_sdp_media_attribute(pauline_params, LinphoneStreamTypeAudio, "pcfg",
	                                                    "49 a=999 t=1");

	// Start second call with same call params as the first one
	BC_ASSERT_TRUE(call_with_params(pauline, marie, pauline_params, marie_params));

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "defer_update_default", TRUE);

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);

	stats marie_stat = marie->stat;
	stats pauline_stat = pauline->stat;
	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
	bool potentialConfigurationChosen = false;

	// Update call with unsupported cfg
	LinphoneCallParams *params1 = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_add_custom_sdp_media_attribute(
	    params1, LinphoneStreamTypeAudio, "acap",
	    "52 crypto:654 AES_256_CM_HMAC_SHA1_80 inline:fWgTsTLqHc/xC7VQl7air+at/Ko1DpXudbS0KG3s UNAUTHENTICATED_SRTP");
	if (pauline_supports_unencrypted) {
		linphone_call_params_add_custom_sdp_media_attribute(params1, LinphoneStreamTypeAudio, "pcfg",
		                                                    "238 a=52|1|2|999 t=1");
		linphone_call_params_add_custom_sdp_media_attribute(params1, LinphoneStreamTypeVideo, "pcfg",
		                                                    "82 a=3|4|999 t=1");
	} else {
		linphone_call_params_add_custom_sdp_media_attribute(params1, LinphoneStreamTypeAudio, "pcfg",
		                                                    "179 a=52|1|2|3|4|999 t=1");
		linphone_call_params_add_custom_sdp_media_attribute(params1, LinphoneStreamTypeVideo, "pcfg",
		                                                    "294 a=5|6|7|8|999 t=1");
	}
	linphone_call_params_enable_video(params1, TRUE);
	linphone_call_update(pauline_call, params1);
	linphone_call_params_unref(params1);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (marie_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	LinphoneCallParams *params1a = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_enable_video(params1a, TRUE);
	linphone_call_params_clear_custom_sdp_attributes(params1a);
	linphone_call_accept_update(marie_call, params1a);
	linphone_call_params_unref(params1a);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", FALSE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "defer_update_default", FALSE);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        (pauline_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

	get_expected_encryption_from_call_params(pauline_call, marie_call, &encryption, &potentialConfigurationChosen);

	BC_ASSERT_TRUE(
	    wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	             (pauline_stat.number_of_LinphoneCallStreamsRunning + 1 + ((potentialConfigurationChosen) ? 1 : 0))));
	BC_ASSERT_TRUE(
	    wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	             (marie_stat.number_of_LinphoneCallStreamsRunning + 1 + ((potentialConfigurationChosen) ? 1 : 0))));

	liblinphone_tester_set_next_video_frame_decoded_cb(marie_call);
	liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded,
	                        (pauline_stat.number_of_IframeDecoded + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
	                        (marie_stat.number_of_IframeDecoded + 1)));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marie_call) {
		check_stream_encryption(marie_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marie_call)),
		                encryption, int, "%i");
	}
	if (pauline_call) {
		check_stream_encryption(pauline_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(pauline_call)),
		                encryption, int, "%i");
	}

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "defer_update_default", TRUE);

	// Update call with unsupported cfg
	marie_stat = marie->stat;
	pauline_stat = pauline->stat;
	LinphoneCallParams *params2 = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_add_custom_sdp_media_attribute(
	    params2, LinphoneStreamTypeAudio, "acap",
	    "86 crypto:654 AES_256_CM_HMAC_SHA1_80 inline:fWgTsTLqHc/xC7VQl7air+at/Ko1DpXudbS0KG3s UNAUTHENTICATED_SRTP");
	linphone_call_params_add_custom_sdp_media_attribute(params2, LinphoneStreamTypeAudio, "pcfg",
	                                                    "875 a=870|1|2|3|4|86 t=1");
	linphone_call_params_enable_video(params2, FALSE);
	linphone_call_update(marie_call, params2);
	linphone_call_params_unref(params2);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        (pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1)));

	LinphoneCallParams *params2a = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_video(params2a, FALSE);
	linphone_call_params_clear_custom_sdp_attributes(params2a);
	linphone_call_accept_update(pauline_call, params2a);
	linphone_call_params_unref(params2a);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", FALSE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "defer_update_default", FALSE);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdating,
	                        (marie_stat.number_of_LinphoneCallUpdating + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (pauline_stat.number_of_LinphoneCallStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        (marie_stat.number_of_LinphoneCallStreamsRunning + 1)));

	encryption = LinphoneMediaEncryptionNone;
	potentialConfigurationChosen = false;
	get_expected_encryption_from_call_params(pauline_call, marie_call, &encryption, &potentialConfigurationChosen);

	BC_ASSERT_TRUE(
	    wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	             (pauline_stat.number_of_LinphoneCallStreamsRunning + 1 + ((potentialConfigurationChosen) ? 1 : 0))));
	BC_ASSERT_TRUE(
	    wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	             (marie_stat.number_of_LinphoneCallStreamsRunning + 1 + ((potentialConfigurationChosen) ? 1 : 0))));

	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));

	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));

	liblinphone_tester_check_rtcp(marie, pauline);

	// Check that encryption has not changed after sending update
	if (marie_call) {
		check_stream_encryption(marie_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marie_call)),
		                encryption, int, "%i");
	}
	if (pauline_call) {
		check_stream_encryption(pauline_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(pauline_call)),
		                encryption, int, "%i");
	}

	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update(void) {
	call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update_base(FALSE);
}

static void srtp_call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update(void) {
	call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update_base(TRUE);
}

static void call_with_capability_negotiations_and_unsupported_crypto_in_sdp_base(bool_t pauline_supports_unencrypted) {
	const LinphoneMediaEncryption optionalEncryption = LinphoneMediaEncryptionSRTP;

	std::list<LinphoneMediaEncryption> enc_list{optionalEncryption};

	encryption_params pauline_enc_mgr_params;
	pauline_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	pauline_enc_mgr_params.level = E_OPTIONAL;
	pauline_enc_mgr_params.preferences = enc_list;

	LinphoneCoreManager *pauline = create_core_mgr_with_capability_negotiation_setup(
	    (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"), pauline_enc_mgr_params, TRUE,
	    FALSE, TRUE);
	if (pauline_supports_unencrypted) {
		// AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP is not supported hence it should not be put in the offer or
		// accepted as answer
		linphone_core_set_srtp_crypto_suites(pauline->lc,
		                                     "AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP, AES_CM_256_HMAC_SHA1_32 "
		                                     "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP");
	}

	encryption_params marie_enc_mgr_params;
	marie_enc_mgr_params.encryption = LinphoneMediaEncryptionNone;
	marie_enc_mgr_params.level = E_OPTIONAL;
	marie_enc_mgr_params.preferences = enc_list;

	LinphoneCoreManager *marie =
	    create_core_mgr_with_capability_negotiation_setup("marie_rc", marie_enc_mgr_params, TRUE, FALSE, TRUE);
	// AES_CM_128_HMAC_SHA1_32 UNENCRYPTED_SRTCP is not supported hence it should not be put in the offer or accepted as
	// answer
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 "
	                                                "UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP");

	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(pauline->lc, optionalEncryption));
	BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, optionalEncryption));

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_capability_negotiations(marie_params, TRUE);
	linphone_call_params_add_custom_sdp_attribute(
	    marie_params, "acap",
	    "870 crypto:99 AES_CM_256_HMAC_SHA1_32 inline:fWgTsTLqHc/xC7VQl7air+at/Ko1DpXudbS0KG3s UNENCRYPTED_SRTCP");
	linphone_call_params_add_custom_sdp_media_attribute(marie_params, LinphoneStreamTypeAudio, "pcfg", "36 a=870 t=1");
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_capability_negotiations(pauline_params, TRUE);
	linphone_call_params_add_custom_sdp_attribute(
	    pauline_params, "acap",
	    "999 crypto:25 AES_CM_256_HMAC_SHA1_80 inline:fWgTsTLqHc/xC7VQl7air+at/Ko1DpXudbS0KG3s UNENCRYPTED_SRTP "
	    "UNENCRYPTED_SRTCP");

	const LinphoneMediaEncryption expectedEncryption =
	    (pauline_supports_unencrypted) ? optionalEncryption : LinphoneMediaEncryptionNone;
	encrypted_call_with_params_base(marie, pauline, expectedEncryption, marie_params, pauline_params, TRUE);
	if (linphone_core_get_current_call(marie->lc)) {
		end_call(marie, pauline);
	}

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void unencrypted_call_with_capability_negotiations_and_unsupported_crypto_in_sdp(void) {
	call_with_capability_negotiations_and_unsupported_crypto_in_sdp_base(FALSE);
}

static void srtp_call_with_capability_negotiations_and_unsupported_crypto_in_sdp(void) {
	call_with_capability_negotiations_and_unsupported_crypto_in_sdp_base(TRUE);
}

static void srtp_call_with_toggling_encryption(void) {
	call_with_toggling_encryption_base(LinphoneMediaEncryptionSRTP);
}

test_t srtp_capability_negotiation_tests[] = {
    TEST_NO_TAG("Simple SRTP call with capability negotiations with no encryption after resume",
                simple_srtp_call_with_capability_negotiations_with_no_encryption_after_resume),
    TEST_NO_TAG("Simple SRTP call with capability negotiations with ZRTP encryption after resume",
                simple_srtp_call_with_capability_negotiations_with_zrtp_encryption_after_resume),
    TEST_NO_TAG("Simple SRTP call with capability negotiations with DTLS SRTP encryption after resume",
                simple_srtp_call_with_capability_negotiations_with_dtls_srtp_encryption_after_resume),
    TEST_NO_TAG("Simple SRTP call with capability negotiations removed after update",
                simple_srtp_call_with_capability_negotiations_removed_after_update),
    TEST_NO_TAG("Simple SRTP call with capability negotiations with resume and media change",
                simple_srtp_call_with_capability_negotiations_with_resume_and_media_change),
    TEST_NO_TAG("SRTP call with suite mismatch and capability negotiations (caller unencrypted)",
                srtp_call_with_suite_mismatch_and_capability_negotiations_caller_unencrypted),
    TEST_NO_TAG("SRTP call with suite mismatch and capability negotiations (callee unencrypted)",
                srtp_call_with_suite_mismatch_and_capability_negotiations_callee_unencrypted),
    TEST_NO_TAG("SRTP call with different encryptions in call params",
                srtp_call_with_encryption_supported_in_call_params_only),
    TEST_NO_TAG("SRTP call with mandatory encryption", srtp_call_with_mandatory_encryption),
    TEST_NO_TAG("SRTP call with mandatory encryption and capability negotiation on both sides",
                srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides),
    TEST_NO_TAG("SRTP call with mandatory encryption and capability negotiation on callee side",
                srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side),
    TEST_NO_TAG("SRTP call with mandatory encryption and capability negotiation on caller side",
                srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side),
    TEST_NO_TAG("SRTP call from endpoint with mandatory encryption to endpoint with none",
                srtp_call_from_enc_to_no_enc),
    TEST_NO_TAG("SRTP call from endpoint with no encryption to endpoint with mandatory", srtp_call_from_no_enc_to_enc),
    TEST_NO_TAG("SRTP call from endpoint with optional encryption to endpoint with mandatory",
                srtp_call_from_opt_enc_to_enc),
    TEST_NO_TAG("SRTP call from endpoint with mandatory encryption to endpoint with optional",
                srtp_call_from_enc_to_opt_enc),
    TEST_NO_TAG("SRTP call from endpoint with optional encryption to endpoint with none",
                srtp_call_with_optional_encryption_on_caller),
    TEST_NO_TAG("SRTP call from endpoint with no encryption to endpoint with optional",
                srtp_call_with_optional_encryption_on_callee),
    TEST_NO_TAG("SRTP call with optional encryption on both sides", srtp_call_with_optional_encryption_on_both_sides),
    TEST_NO_TAG("SRTP video call with optional encryption on caller",
                srtp_video_call_with_optional_encryption_on_caller),
    TEST_NO_TAG("SRTP video call with optional encryption on callee",
                srtp_video_call_with_optional_encryption_on_callee),
    TEST_NO_TAG("SRTP video call with optional encryption on both sides",
                srtp_video_call_with_optional_encryption_on_both_sides),
    TEST_NO_TAG("SRTP call with toggling encryption", srtp_call_with_toggling_encryption)};

test_t srtp_capability_negotiation_basic_tests[] = {
    TEST_NO_TAG("Call with default SRTP encryption", call_with_srtp_default_encryption),
    TEST_NO_TAG("Simple SRTP call with capability negotiations with reINVITE",
                simple_srtp_call_with_capability_negotiations_with_reinvite),
    TEST_NO_TAG("Simple SRTP call with capability negotiations without reINVITE",
                simple_srtp_call_with_capability_negotiations_without_reinvite),
    TEST_NO_TAG("SRTP unencrypted call and capability negotiations without cfg line merge",
                unencrypted_srtp_call_with_capability_negotiations_without_cfg_lines_merge),
    TEST_NO_TAG("SRTP unencrypted call and capability negotiations with cfg line merge",
                unencrypted_srtp_call_with_capability_negotiations_with_cfg_lines_merge),
    TEST_NO_TAG("SRTP call and capability negotiations (caller unencrypted)",
                srtp_call_with_capability_negotiations_caller_unencrypted),
    TEST_NO_TAG("SRTP call and capability negotiations (callee unencrypted)",
                srtp_call_with_capability_negotiations_callee_unencrypted),
    TEST_NO_TAG("Unencrypted call with capability negotiations and unsupported crypto in SDP",
                unencrypted_call_with_capability_negotiations_and_unsupported_crypto_in_sdp),
    TEST_NO_TAG("SRTP call with capability negotiations and unsupported crypto in SDP",
                srtp_call_with_capability_negotiations_and_unsupported_crypto_in_sdp),
    TEST_NO_TAG("Unencrypted call with capability negotiations and unsupported crypto in SDP update",
                unencrypted_call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update),
    TEST_NO_TAG("SRTP call with capability negotiations and unsupported crypto in SDP update",
                srtp_call_with_capability_negotiations_and_unsupported_crypto_in_sdp_update),
    TEST_NO_TAG("SRTP call with potential configuration same as actual one",
                srtp_call_with_potential_configuration_same_as_actual_configuration),
    TEST_NO_TAG("SRTP call started with video and capability negotiation",
                srtp_call_with_video_and_capability_negotiation)};

test_suite_t srtp_capability_negotiation_test_suite = {"SRTP Capability Negotiation (Encryption Change)",
                                                       NULL,
                                                       NULL,
                                                       liblinphone_tester_before_each,
                                                       liblinphone_tester_after_each,
                                                       sizeof(srtp_capability_negotiation_tests) /
                                                           sizeof(srtp_capability_negotiation_tests[0]),
                                                       srtp_capability_negotiation_tests,
                                                       0,
                                                       2};

test_suite_t srtp_capability_negotiation_basic_test_suite = {"SRTP Capability Negotiation (Basic)",
                                                             NULL,
                                                             NULL,
                                                             liblinphone_tester_before_each,
                                                             liblinphone_tester_after_each,
                                                             sizeof(srtp_capability_negotiation_basic_tests) /
                                                                 sizeof(srtp_capability_negotiation_basic_tests[0]),
                                                             srtp_capability_negotiation_basic_tests,
                                                             0,
                                                             2};
