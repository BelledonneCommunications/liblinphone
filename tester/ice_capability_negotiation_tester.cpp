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

#include "capability_negotiation_tester.h"
#include "liblinphone_tester.h"
#include "linphone/core.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

static void ice_call_with_update_and_incompatible_encs_in_call_params(void) {
	call_with_update_and_incompatible_encs_in_call_params_base(TRUE);
}

static void ice_call_with_optional_encryption(const LinphoneMediaEncryption encryption,
                                              const bool_t caller_with_ice,
                                              const bool_t callee_with_ice,
                                              const bool_t enable_video,
                                              bool_t change_encryption) {
	encryption_params marie_enc_params;
	marie_enc_params.encryption = encryption;
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
	// Avoid setting the actual configuration with the same encryption as the desired one
	if (encryption == LinphoneMediaEncryptionZRTP) {
		pauline_enc_params.encryption = LinphoneMediaEncryptionDTLS;
	} else {
		pauline_enc_params.encryption = LinphoneMediaEncryptionZRTP;
	}
	pauline_enc_params.level = E_OPTIONAL;
	pauline_enc_params.preferences = set_encryption_preference_with_priority(encryption, change_encryption);

	call_with_encryption_test_base(marie_enc_params, TRUE, caller_with_ice, pauline_enc_params, TRUE, callee_with_ice,
	                               enable_video);
}

static void srtp_ice_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, TRUE, FALSE, FALSE);
}

static void dtls_srtp_ice_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, TRUE, FALSE, FALSE);
}

static void zrtp_ice_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, TRUE, FALSE, FALSE);
}

static void srtp_call_with_optional_encryption_caller_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, FALSE, FALSE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_caller_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, FALSE, FALSE, FALSE);
}

static void zrtp_call_with_optional_encryption_caller_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, FALSE, FALSE, FALSE);
}

static void srtp_call_with_optional_encryption_callee_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, FALSE, TRUE, FALSE, FALSE);
}

static void dtls_srtp_call_with_optional_encryption_callee_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, FALSE, TRUE, FALSE, FALSE);
}

static void zrtp_call_with_optional_encryption_callee_with_ice(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, FALSE, TRUE, FALSE, FALSE);
}

static void srtp_ice_video_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, TRUE, TRUE, FALSE);
}

static void dtls_srtp_ice_video_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE, FALSE);
}

static void zrtp_ice_video_call_with_optional_encryption_on_both_sides(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, TRUE, TRUE, FALSE);
}

static void srtp_ice_video_call_with_encryption_change_when_adding_video(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionSRTP, TRUE, TRUE, TRUE, TRUE);
}

static void dtls_srtp_ice_video_call_with_encryption_change_when_adding_video(void) {
	ice_call_with_optional_encryption(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE, TRUE);
}

static void zrtp_ice_video_call_with_encryption_change_when_adding_video(void) {
#if 0
	ice_call_with_optional_encryption(LinphoneMediaEncryptionZRTP, TRUE, TRUE, TRUE, TRUE);
#endif
	BC_PASS("Test temporarely disabled");
}

static void ice_call_from_opt_enc_to_none_base(const LinphoneMediaEncryption encryption,
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
test_t ice_capability_negotiation_tests[] = {
    TEST_ONE_TAG("ICE call with update and incompatible encryptions in call params",
                 ice_call_with_update_and_incompatible_encs_in_call_params,
                 "ICE")};

test_t srtp_ice_capability_negotiation_tests[] = {
    TEST_ONE_TAG(
        "SRTP ICE call with optional encryption on caller", srtp_ice_call_with_optional_encryption_on_caller, "ICE"),
    TEST_ONE_TAG(
        "SRTP ICE call with optional encryption on callee", srtp_ice_call_with_optional_encryption_on_callee, "ICE"),
    TEST_ONE_TAG("SRTP ICE call with optional encryption on both sides",
                 srtp_ice_call_with_optional_encryption_on_both_sides,
                 "ICE"),
    TEST_ONE_TAG("SRTP call with optional encryption (caller with ICE)",
                 srtp_call_with_optional_encryption_caller_with_ice,
                 "ICE"),
    TEST_ONE_TAG("SRTP call with optional encryption (callee with ICE)",
                 srtp_call_with_optional_encryption_callee_with_ice,
                 "ICE"),
    TEST_ONE_TAG("SRTP ICE video call with optional encryption on caller",
                 srtp_ice_video_call_with_optional_encryption_on_caller,
                 "ICE"),
    TEST_ONE_TAG("SRTP ICE video call with optional encryption on callee",
                 srtp_ice_video_call_with_optional_encryption_on_callee,
                 "ICE"),
    TEST_ONE_TAG("SRTP ICE video call with optional encryption on both sides",
                 srtp_ice_video_call_with_optional_encryption_on_both_sides,
                 "ICE"),
    TEST_ONE_TAG("SRTP ICE video call with encryption change when adding video",
                 srtp_ice_video_call_with_encryption_change_when_adding_video,
                 "ICE")};

test_t zrtp_ice_capability_negotiation_tests[] = {
    TEST_ONE_TAG(
        "ZRTP ICE call with optional encryption on caller", zrtp_ice_call_with_optional_encryption_on_caller, "ICE"),
    TEST_ONE_TAG(
        "ZRTP ICE call with optional encryption on callee", zrtp_ice_call_with_optional_encryption_on_callee, "ICE"),
    TEST_ONE_TAG("ZRTP ICE call with optional encryption on both sides",
                 zrtp_ice_call_with_optional_encryption_on_both_sides,
                 "ICE"),
    TEST_ONE_TAG("ZRTP call with optional encryption (caller with ICE)",
                 zrtp_call_with_optional_encryption_caller_with_ice,
                 "ICE"),
    TEST_ONE_TAG("ZRTP call with optional encryption (callee with ICE)",
                 zrtp_call_with_optional_encryption_callee_with_ice,
                 "ICE"),
    TEST_ONE_TAG("ZRTP ICE video call with optional encryption on caller",
                 zrtp_ice_video_call_with_optional_encryption_on_caller,
                 "ICE"),
    TEST_ONE_TAG("ZRTP ICE video call with optional encryption on callee",
                 zrtp_ice_video_call_with_optional_encryption_on_callee,
                 "ICE"),
    TEST_ONE_TAG("ZRTP ICE video call with optional encryption on both sides",
                 zrtp_ice_video_call_with_optional_encryption_on_both_sides,
                 "ICE"),
    TEST_ONE_TAG("ZRTP ICE video call with encryption change when adding video",
                 zrtp_ice_video_call_with_encryption_change_when_adding_video,
                 "ICE")};

test_t dtls_srtp_ice_capability_negotiation_tests[] = {
    TEST_TWO_TAGS("DTLS SRTP ICE call with optional encryption on caller",
                  dtls_srtp_ice_call_with_optional_encryption_on_caller,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ICE call with optional encryption on callee",
                  dtls_srtp_ice_call_with_optional_encryption_on_callee,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ICE call with optional encryption on both sides",
                  dtls_srtp_ice_call_with_optional_encryption_on_both_sides,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP call with optional encryption (caller with ICE)",
                  dtls_srtp_call_with_optional_encryption_caller_with_ice,
                  "DTLS",
                  "ICE"),
    TEST_TWO_TAGS("DTLS SRTP call with optional encryption (callee with ICE)",
                  dtls_srtp_call_with_optional_encryption_callee_with_ice,
                  "DTLS",
                  "ICE"),
    TEST_TWO_TAGS("DTLS SRTP ICE video call with optional encryption on caller",
                  dtls_srtp_ice_video_call_with_optional_encryption_on_caller,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ICE video call with optional encryption on callee",
                  dtls_srtp_ice_video_call_with_optional_encryption_on_callee,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ICE video call with optional encryption on both sides",
                  dtls_srtp_ice_video_call_with_optional_encryption_on_both_sides,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ICE video call with encryption change when adding video",
                  dtls_srtp_ice_video_call_with_encryption_change_when_adding_video,
                  "ICE",
                  "DTLS")};

test_suite_t ice_capability_negotiation_test_suite = {"ICE Capability Negotiation",
                                                      NULL,
                                                      NULL,
                                                      liblinphone_tester_before_each,
                                                      liblinphone_tester_after_each,
                                                      sizeof(ice_capability_negotiation_tests) /
                                                          sizeof(ice_capability_negotiation_tests[0]),
                                                      ice_capability_negotiation_tests,
                                                      0,
                                                      2};

test_suite_t srtp_ice_capability_negotiation_test_suite = {"SRTP ICE Capability Negotiation",
                                                           NULL,
                                                           NULL,
                                                           liblinphone_tester_before_each,
                                                           liblinphone_tester_after_each,
                                                           sizeof(srtp_ice_capability_negotiation_tests) /
                                                               sizeof(srtp_ice_capability_negotiation_tests[0]),
                                                           srtp_ice_capability_negotiation_tests,
                                                           0,
                                                           2};

test_suite_t zrtp_ice_capability_negotiation_test_suite = {"ZRTP ICE Capability Negotiation",
                                                           NULL,
                                                           NULL,
                                                           liblinphone_tester_before_each,
                                                           liblinphone_tester_after_each,
                                                           sizeof(zrtp_ice_capability_negotiation_tests) /
                                                               sizeof(zrtp_ice_capability_negotiation_tests[0]),
                                                           zrtp_ice_capability_negotiation_tests,
                                                           0,
                                                           2};

test_suite_t dtls_srtp_ice_capability_negotiation_test_suite = {
    "DTLS SRTP ICE Capability Negotiation",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(dtls_srtp_ice_capability_negotiation_tests) / sizeof(dtls_srtp_ice_capability_negotiation_tests[0]),
    dtls_srtp_ice_capability_negotiation_tests,
    0};
