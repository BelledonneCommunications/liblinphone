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

#include "capability_negotiation_tester.h"
#include "liblinphone_tester.h"
#include "linphone/core.h"
#include "tester_utils.h"

static void call_with_zrtp_default_encryption(void) {
	call_with_default_encryption(LinphoneMediaEncryptionZRTP);
}

static void simple_zrtp_call_with_capability_negotiations_removed_after_update(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_removed_after_update(marie, pauline, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations_with_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations_without_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negotiation_reinvite(marie->lc, FALSE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negotiation_reinvite(pauline->lc, FALSE);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations_with_no_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionNone);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations_with_srtp_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations_with_dtls_srtp_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionDTLS);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_zrtp_call_with_capability_negotiations_with_resume_and_media_change(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_zrtp_go_clear(marie->lc, TRUE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_zrtp_go_clear(pauline->lc, TRUE);
	simple_call_with_capability_negotiations_with_resume_and_media_change_base(
	    marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionNone);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_call_from_dtls_enc_to_enc(void) {
	call_from_enc_to_dtls_enc_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE, FALSE);
}

static void zrtp_call_from_enc_to_dtls_enc(void) {
	call_from_enc_to_dtls_enc_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE, TRUE);
}

static void zrtp_call_with_video_and_capability_negotiation(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionZRTP; // Desired encryption
	call_with_video_and_capability_negotiation_base(encryption);
}

static void zrtp_call_with_encryption_supported_in_call_params_only(void) {
	call_with_encryption_supported_in_call_params_only_base(LinphoneMediaEncryptionZRTP);
}

static void zrtp_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionZRTP);
}

static void zrtp_sas_call_with_capability_negotiations(void) {
	// LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_b256_rc");
	// LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_b256_rc");

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	LpConfig *lpp = linphone_core_get_config(pauline->lc);
	linphone_config_set_string(lpm, "sip", "zrtp_sas_suites", "MS_ZRTP_SAS_B256");
	linphone_config_set_string(lpp, "sip", "zrtp_sas_suites", "MS_ZRTP_SAS_B256");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_sas_call_with_capability_negotiations_default_keys_on_callee(void) {
	//	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_b256_rc");

	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	linphone_config_set_string(lpm, "sip", "zrtp_sas_suites", "MS_ZRTP_SAS_B256");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_cipher_call_with_capability_negotiations(void) {
	//	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_srtpsuite_aes256_rc");
	//	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_srtpsuite_aes256_rc");

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	LpConfig *lpp = linphone_core_get_config(pauline->lc);
	linphone_config_set_string(lpm, "sip", "srtp_crypto_suites", "AES_CM_256_HMAC_SHA1_80,AES_CM_256_HMAC_SHA1_32");
	linphone_config_set_string(lpp, "sip", "srtp_crypto_suites", "AES_CM_256_HMAC_SHA1_80,AES_CM_256_HMAC_SHA1_32");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_cipher_call_with_capability_negotiations_aes256(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	LpConfig *lpp = linphone_core_get_config(pauline->lc);
	linphone_config_set_string(lpm, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	linphone_config_set_string(lpp, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_call_with_different_cipher_suites_and_capability_negotiations(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	LpConfig *lpp = linphone_core_get_config(pauline->lc);
	linphone_config_set_string(lpm, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	linphone_config_set_string(lpp, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	linphone_config_set_string(lpp, "sip", "zrtp_key_agreements_suites", "MS_ZRTP_KEY_AGREEMENT_X255");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_cipher_call_with_capability_negotiations_default_keys_on_callee(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	linphone_config_set_string(lpm, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_key_agreement_call_with_capability_negotiations(void) {
	//	LinphoneCoreManager* marie = linphone_core_manager_new("marie_zrtp_ecdh255_rc");
	//	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_zrtp_ecdh255_rc");

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	LpConfig *lpm = linphone_core_get_config(marie->lc);
	LpConfig *lpp = linphone_core_get_config(pauline->lc);
	linphone_config_set_string(lpm, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	linphone_config_set_string(lpm, "sip", "zrtp_key_agreements_suites", "MS_ZRTP_KEY_AGREEMENT_X255");
	linphone_config_set_string(lpp, "sip", "zrtp_cipher_suites", "MS_ZRTP_CIPHER_AES3,MS_ZRTP_CIPHER_AES1");
	linphone_config_set_string(lpp, "sip", "zrtp_key_agreements_suites", "MS_ZRTP_KEY_AGREEMENT_X255");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void zrtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE);
}

static void zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionZRTP, FALSE, TRUE);
}

static void zrtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionZRTP, FALSE);
}

static void zrtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionZRTP, TRUE);
}

static void zrtp_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE);
}

static void zrtp_video_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE, TRUE);
}

static void zrtp_video_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, FALSE, TRUE);
}

static void zrtp_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE);
}

static void zrtp_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionZRTP, FALSE);
}

static void zrtp_video_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionZRTP, TRUE);
}

static void zrtp_call_with_toggling_encryption(void) {
	call_with_toggling_encryption_base(LinphoneMediaEncryptionZRTP);
}

test_t zrtp_capability_negotiation_tests[] = {
    TEST_NO_TAG("Simple ZRTP call with capability negotiations removed after update",
                simple_zrtp_call_with_capability_negotiations_removed_after_update),
    TEST_NO_TAG("Simple ZRTP call with capability negotiations with resume and media change",
                simple_zrtp_call_with_capability_negotiations_with_resume_and_media_change),
    TEST_NO_TAG("Simple ZRTP call with capability negotiations with no encryption after resume",
                simple_zrtp_call_with_capability_negotiations_with_no_encryption_after_resume),
    TEST_NO_TAG("Simple ZRTP call with capability negotiations with SRTP encryption after resume",
                simple_zrtp_call_with_capability_negotiations_with_srtp_encryption_after_resume),
    TEST_NO_TAG("Simple ZRTP call with capability negotiations with DTLS SRTP encryption after resume",
                simple_zrtp_call_with_capability_negotiations_with_dtls_srtp_encryption_after_resume),
    TEST_NO_TAG("ZRTP call with toggling encryption", zrtp_call_with_toggling_encryption),
    TEST_NO_TAG("ZRTP call with mandatory encryption", zrtp_call_with_mandatory_encryption),
    TEST_NO_TAG("ZRTP call with mandatory encryption and capability negotiation on both sides",
                zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides),
    TEST_NO_TAG("ZRTP call with mandatory encryption and capability negotiation on callee side",
                zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side),
    TEST_NO_TAG("ZRTP call with mandatory encryption and capability negotiation on caller side",
                zrtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side),
    TEST_NO_TAG("ZRTP call from endpoint with mandatory encryption to endpoint with DTLS",
                zrtp_call_from_enc_to_dtls_enc),
    TEST_NO_TAG("ZRTP call from endpoint with DTLS encryption to endpoint with mandatory",
                zrtp_call_from_dtls_enc_to_enc),
    TEST_NO_TAG("ZRTP call from endpoint with optional encryption to endpoint with mandatory",
                zrtp_call_from_opt_enc_to_enc),
    TEST_NO_TAG("ZRTP call from endpoint with mandatory encryption to endpoint with optional",
                zrtp_call_from_enc_to_opt_enc),
    TEST_NO_TAG("ZRTP call from endpoint with optional encryption to endpoint with none",
                zrtp_call_with_optional_encryption_on_caller),
    TEST_NO_TAG("ZRTP call from endpoint with no encryption to endpoint with optional",
                zrtp_call_with_optional_encryption_on_callee),
    TEST_NO_TAG("ZRTP call with optional encryption on both sides", zrtp_call_with_optional_encryption_on_both_sides),
    TEST_NO_TAG("ZRTP video call with optional encryption on caller",
                zrtp_video_call_with_optional_encryption_on_caller),
    TEST_NO_TAG("ZRTP video call with optional encryption on callee",
                zrtp_video_call_with_optional_encryption_on_callee),
    TEST_NO_TAG("ZRTP video call with optional encryption on both sides",
                zrtp_video_call_with_optional_encryption_on_both_sides)};

test_t zrtp_capability_negotiation_basic_tests[] = {
    TEST_NO_TAG("Call with default ZRTP encryption", call_with_zrtp_default_encryption),
    TEST_NO_TAG("Simple ZRTP call with capability negotiations with reINVITE",
                simple_zrtp_call_with_capability_negotiations_with_reinvite),
    TEST_NO_TAG("Simple ZRTP call with capability negotiations without reINVITE",
                simple_zrtp_call_with_capability_negotiations_without_reinvite),
    TEST_NO_TAG("ZRTP call with differet cipher suites and capability negotiations",
                zrtp_call_with_different_cipher_suites_and_capability_negotiations),
    TEST_NO_TAG("ZRTP cipher call with capability negotiations default keys on callee",
                zrtp_cipher_call_with_capability_negotiations_default_keys_on_callee),
    TEST_NO_TAG("ZRTP cipher call with capability negotiations aes256",
                zrtp_cipher_call_with_capability_negotiations_aes256),
    TEST_NO_TAG("ZRTP cipher call with capability negotiations", zrtp_cipher_call_with_capability_negotiations),
    TEST_NO_TAG("ZRTP sas call with capability negotiations default keys on callee",
                zrtp_sas_call_with_capability_negotiations_default_keys_on_callee),
    TEST_NO_TAG("ZRTP sas call with capability negotiations", zrtp_sas_call_with_capability_negotiations),
    TEST_NO_TAG("ZRTP key agreement call with capability negotiations",
                zrtp_key_agreement_call_with_capability_negotiations),
    TEST_NO_TAG("ZRTP call with different encryptions in call params",
                zrtp_call_with_encryption_supported_in_call_params_only),
    TEST_NO_TAG("ZRTP call started with video and capability negotiation",
                zrtp_call_with_video_and_capability_negotiation),
    TEST_NO_TAG("ZRTP call with potential configuration same as actual one",
                zrtp_call_with_potential_configuration_same_as_actual_configuration)};

test_suite_t zrtp_capability_negotiation_test_suite = {"ZRTP Capability Negotiation",
                                                       NULL,
                                                       NULL,
                                                       liblinphone_tester_before_each,
                                                       liblinphone_tester_after_each,
                                                       sizeof(zrtp_capability_negotiation_tests) /
                                                           sizeof(zrtp_capability_negotiation_tests[0]),
                                                       zrtp_capability_negotiation_tests,
                                                       0};

test_suite_t zrtp_capability_negotiation_basic_test_suite = {"ZRTP Capability Negotiation (Basic)",
                                                             NULL,
                                                             NULL,
                                                             liblinphone_tester_before_each,
                                                             liblinphone_tester_after_each,
                                                             sizeof(zrtp_capability_negotiation_basic_tests) /
                                                                 sizeof(zrtp_capability_negotiation_basic_tests[0]),
                                                             zrtp_capability_negotiation_basic_tests,
                                                             0};
