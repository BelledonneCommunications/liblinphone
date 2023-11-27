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

static void call_with_dtls_srtp_default_encryption(void) {
	call_with_default_encryption(LinphoneMediaEncryptionDTLS);
}

static void simple_dtls_srtp_call_with_capability_negotiations_removed_after_update(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_removed_after_update(marie, pauline, LinphoneMediaEncryptionDTLS);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations_with_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionDTLS);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations_without_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_enable_capability_negotiation_reinvite(marie->lc, FALSE);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_capability_negotiation_reinvite(pauline->lc, FALSE);
	simple_call_with_capability_negotiations(marie, pauline, LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionDTLS);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations_with_no_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionNone);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations_with_srtp_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations_with_zrtp_encryption_after_resume(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_different_encryption_after_resume(
	    marie, pauline, LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionZRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_dtls_srtp_call_with_capability_negotiations_with_resume_and_media_change(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	simple_call_with_capability_negotiations_with_resume_and_media_change_base(
	    marie, pauline, LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionSRTP);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void dtls_srtp_call_from_opt_enc_to_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionDTLS, TRUE);
}

static void dtls_srtp_call_from_enc_to_opt_enc(void) {
	call_from_opt_enc_to_enc_base(LinphoneMediaEncryptionDTLS, FALSE);
}

static void dtls_srtp_call_with_mandatory_encryption(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE);
}

static void dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE);
}

static void dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE);
}

static void dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side(void) {
	call_with_mandatory_encryption_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE);
}

static void dtls_srtp_call_with_potential_configuration_same_as_actual_configuration(void) {
	call_with_potential_configuration_same_as_actual_configuration_base(LinphoneMediaEncryptionDTLS);
}

static void dtls_srtp_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE);
}

static void dtls_srtp_video_call_with_optional_encryption_on_caller(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE);
}

static void dtls_srtp_video_call_with_optional_encryption_on_callee(void) {
	call_from_opt_enc_to_none_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE);
}

static void dtls_srtp_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionDTLS, FALSE);
}

static void dtls_srtp_video_call_with_optional_encryption_on_both_sides(void) {
	call_with_optional_encryption_on_both_sides_base(LinphoneMediaEncryptionDTLS, TRUE);
}

static void dtls_srtp_call_with_encryption_supported_in_call_params_only(void) {
	call_with_encryption_supported_in_call_params_only_base(LinphoneMediaEncryptionDTLS);
}

static void dtls_srtp_call_with_video_and_capability_negotiation(void) {
	const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionDTLS; // Desired encryption
	call_with_video_and_capability_negotiation_base(encryption);
}

static void dtls_srtp_call_from_enc_to_no_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE);
}

static void dtls_srtp_call_from_no_enc_to_enc(void) {
	call_from_enc_to_no_enc_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, FALSE);
}

static void dtls_srtp_call_with_toggling_encryption(void) {
	call_with_toggling_encryption_base(LinphoneMediaEncryptionDTLS);
}

test_t dtls_srtp_capability_negotiation_tests[] = {
    TEST_ONE_TAG("DTLS SRTP call with toggling encryption", dtls_srtp_call_with_toggling_encryption, "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with mandatory encryption", dtls_srtp_call_with_mandatory_encryption, "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with mandatory encryption and capability negotiation on both sides",
                 dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_both_sides,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with mandatory encryption and capability negotiation on callee side",
                 dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_callee_side,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with mandatory encryption and capability negotiation on caller side",
                 dtls_srtp_call_with_mandatory_encryption_and_capability_negotiation_on_caller_side,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call from endpoint with mandatory encryption to endpoint with none",
                 dtls_srtp_call_from_enc_to_no_enc,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call from endpoint with no encryption to endpoint with mandatory",
                 dtls_srtp_call_from_no_enc_to_enc,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call from endpoint with optional encryption to endpoint with mandatory",
                 dtls_srtp_call_from_opt_enc_to_enc,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call from endpoint with mandatory encryption to endpoint with optional",
                 dtls_srtp_call_from_enc_to_opt_enc,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call from endpoint with optional encryption to endpoint with none",
                 dtls_srtp_call_with_optional_encryption_on_caller,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call from endpoint with no encryption to endpoint with optional",
                 dtls_srtp_call_with_optional_encryption_on_callee,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with optional encryption on both sides",
                 dtls_srtp_call_with_optional_encryption_on_both_sides,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP video call with optional encryption on caller",
                 dtls_srtp_video_call_with_optional_encryption_on_caller,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP video call with optional encryption on callee",
                 dtls_srtp_video_call_with_optional_encryption_on_callee,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP video call with optional encryption on both sides",
                 dtls_srtp_video_call_with_optional_encryption_on_both_sides,
                 "DTLS")};

test_t dtls_srtp_capability_negotiation_basic_tests[] = {
    TEST_ONE_TAG("Call with default DTLS SRTP encryption", call_with_dtls_srtp_default_encryption, "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations with reINVITE",
                 simple_dtls_srtp_call_with_capability_negotiations_with_reinvite,
                 "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations without reINVITE",
                 simple_dtls_srtp_call_with_capability_negotiations_without_reinvite,
                 "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations removed after update",
                 simple_dtls_srtp_call_with_capability_negotiations_removed_after_update,
                 "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations with resume and media change",
                 simple_dtls_srtp_call_with_capability_negotiations_with_resume_and_media_change,
                 "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations with no encryption after resume",
                 simple_dtls_srtp_call_with_capability_negotiations_with_no_encryption_after_resume,
                 "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations with ZRTP encryption after resume",
                 simple_dtls_srtp_call_with_capability_negotiations_with_zrtp_encryption_after_resume,
                 "DTLS"),
    TEST_ONE_TAG("Simple DTLS SRTP call with capability negotiations with SRTP encryption after resume",
                 simple_dtls_srtp_call_with_capability_negotiations_with_srtp_encryption_after_resume,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with different encryptions in call params",
                 dtls_srtp_call_with_encryption_supported_in_call_params_only,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call started with video and capability negotiation",
                 dtls_srtp_call_with_video_and_capability_negotiation,
                 "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with potential configuration same as actual one",
                 dtls_srtp_call_with_potential_configuration_same_as_actual_configuration,
                 "DTLS")};

test_suite_t dtls_srtp_capability_negotiation_test_suite = {"DTLS SRTP Capability Negotiation (Encryption change)",
                                                            NULL,
                                                            NULL,
                                                            liblinphone_tester_before_each,
                                                            liblinphone_tester_after_each,
                                                            sizeof(dtls_srtp_capability_negotiation_tests) /
                                                                sizeof(dtls_srtp_capability_negotiation_tests[0]),
                                                            dtls_srtp_capability_negotiation_tests,
                                                            0,
                                                            2};

test_suite_t dtls_srtp_capability_negotiation_basic_test_suite = {
    "DTLS SRTP Capability Negotiation (Basic)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(dtls_srtp_capability_negotiation_basic_tests) / sizeof(dtls_srtp_capability_negotiation_basic_tests[0]),
    dtls_srtp_capability_negotiation_basic_tests,
    0,
    2};
