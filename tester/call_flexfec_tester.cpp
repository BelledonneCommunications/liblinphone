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

#include "bctoolbox/defs.h"

#include "call/call.h"
#include "liblinphone_tester.h"
#include "linphone/core.h"

#include "tester_utils.h"

#ifdef VIDEO_ENABLED

static void enable_rtp_bundle(LinphoneCore *lc, bool_t enable) {

	if (enable == false) {
		linphone_config_set_bool(linphone_core_get_config(lc), "rtp", "accept_bundle", FALSE);
	}
	LinphoneAccount *account = linphone_core_get_default_account(lc);
	LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(account));
	linphone_account_params_enable_rtp_bundle(account_params, enable);
	linphone_account_set_params(account, account_params);
	linphone_account_params_unref(account_params);
}
static void enable_video_stream(LinphoneCore *lc, LinphoneVideoActivationPolicy *policy) {

	linphone_core_set_video_device(lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(lc, TRUE);
	linphone_core_enable_video_display(lc, TRUE);
	linphone_core_set_video_activation_policy(lc, policy);
	linphone_core_set_preferred_video_size(lc, {800, 600});
}
static void disable_all_audio_codecs(LinphoneCore *lc) {
	const bctbx_list_t *elem = linphone_core_get_audio_codecs(lc);
	PayloadType *pt;

	for (; elem != NULL; elem = elem->next) {
		pt = (PayloadType *)elem->data;
		linphone_core_enable_payload_type(lc, pt, FALSE);
	}
}
typedef struct _flexfec_tests_params {

	LinphoneMediaEncryption encryption_mode;
	bool_t ice;
	bool_t audio_enabled;

} flexfec_tests_params;

static void video_call_with_flexfec_base(flexfec_tests_params params) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 5.;
	network_params.mode = OrtpNetworkSimulatorOutbound;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");
	fec_stats *stats = NULL;

	linphone_core_set_media_encryption(marie->lc, params.encryption_mode);
	linphone_core_set_media_encryption(pauline->lc, params.encryption_mode);
	if (params.encryption_mode == LinphoneMediaEncryptionDTLS) {
		char *path = bc_tester_file("certificates-marie");
		linphone_core_set_user_certificates_path(marie->lc, path);
		bc_free(path);
		path = bc_tester_file("certificates-pauline");
		linphone_core_set_user_certificates_path(pauline->lc, path);
		bc_free(path);
		bctbx_mkdir(linphone_core_get_user_certificates_path(marie->lc));
		bctbx_mkdir(linphone_core_get_user_certificates_path(pauline->lc));
	}
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);

	linphone_core_set_network_simulator_params(marie->lc, &network_params);
	linphone_core_set_network_simulator_params(pauline->lc, &network_params);

	if (params.ice) {
		enable_stun_in_core(marie, TRUE, TRUE);
		enable_stun_in_core(pauline, TRUE, TRUE);
	}
	enable_rtp_bundle(marie->lc, TRUE);
	enable_rtp_bundle(pauline->lc, TRUE);

	linphone_core_enable_fec(marie->lc, TRUE);
	linphone_core_enable_fec(pauline->lc, TRUE);

	if (!params.audio_enabled) {
		disable_all_audio_codecs(marie->lc);
		disable_all_audio_codecs(pauline->lc);
	}

	enable_video_stream(marie->lc, pol);
	enable_video_stream(pauline->lc, pol);

	linphone_video_activation_policy_unref(pol);

	BC_ASSERT_TRUE(call(marie, pauline));
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	VideoStream *vstream = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);
	if (vstream->ms.fec_stream) {
		stats = fec_stream_get_stats(vstream->ms.fec_stream);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &stats->packets_recovered, 50, 15000));
	}
	end_call(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_with_flexfec_and_ice(void) {
	flexfec_tests_params params{
	    LinphoneMediaEncryptionNone,
	    TRUE,
	    FALSE,
	};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec(void) {
	flexfec_tests_params params{
	    LinphoneMediaEncryptionNone,
	    FALSE,
	    FALSE,
	};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_srtp(void) {
	flexfec_tests_params params{
	    LinphoneMediaEncryptionSRTP,
	    FALSE,
	    FALSE,
	};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_dtls(void) {
	flexfec_tests_params params{
	    LinphoneMediaEncryptionDTLS,
	    FALSE,
	    FALSE,
	};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_zrtp(void) {
	flexfec_tests_params params{LinphoneMediaEncryptionZRTP, FALSE, TRUE};
	video_call_with_flexfec_base(params);
}

static test_t call_flexfec_tests[] = {
    TEST_NO_TAG("Video call with flexfec", video_call_with_flexfec),
    TEST_NO_TAG("Video call with flexfec", video_call_with_flexfec),
    TEST_NO_TAG("Video call with flexfec and ice", video_call_with_flexfec_and_ice),
    TEST_NO_TAG("Video call with flexfec and srtp", video_call_with_flexfec_and_srtp),
    TEST_NO_TAG("Video call with flexfec and dtls", video_call_with_flexfec_and_dtls),
    TEST_NO_TAG("Video call with flexfec and zrtp", video_call_with_flexfec_and_zrtp),
};

test_suite_t call_flexfec_suite = {"Call with FlexFec",
                                   NULL,
                                   NULL,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(call_flexfec_tests) / sizeof(call_flexfec_tests[0]),
                                   call_flexfec_tests};
#endif // VIDEO_ENABLED
