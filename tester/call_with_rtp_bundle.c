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


#include <sys/types.h>
#include <sys/stat.h>
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "mediastreamer2/msutils.h"
#include "belle-sip/sipstack.h"
#include <bctoolbox/defs.h>

static void check_rtp_bundle(LinphoneCall *call, bool_t should_be_active){
	const LinphoneCallParams *remote_params = linphone_call_get_remote_params(call);
	const LinphoneCallParams *current_params = linphone_call_get_current_params(call);
	if (should_be_active){
		BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(remote_params));
		BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(current_params));
	}else{
		BC_ASSERT_FALSE(linphone_call_params_rtp_bundle_enabled(remote_params));
		BC_ASSERT_FALSE(linphone_call_params_rtp_bundle_enabled(current_params));
	}
}

static bool_t setup_dtls_srtp(LinphoneCoreManager *marie, LinphoneCoreManager *pauline){
	if (!linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionDTLS)){
		BC_FAIL("SRTP-DTLS not supported.");
		return FALSE;
	}
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionDTLS);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionDTLS);
	char *path = bc_tester_file("certificates-marie");
	linphone_core_set_user_certificates_path(marie->lc, path);
	bc_free(path);
	path = bc_tester_file("certificates-pauline");
	linphone_core_set_user_certificates_path(pauline->lc, path);
	bc_free(path);
	belle_sip_mkdir(linphone_core_get_user_certificates_path(marie->lc));
	belle_sip_mkdir(linphone_core_get_user_certificates_path(pauline->lc));
	return TRUE;
}

static void _simple_audio_call(bool_t with_dtls_srtp) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall *pauline_call, *marie_call;
	
	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_rtp_bundle(marie->lc, TRUE);
	
	if (with_dtls_srtp){
		setup_dtls_srtp(marie, pauline);
	}
	
	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_call=linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	
	if (BC_ASSERT_PTR_NOT_NULL(pauline_call))
		check_rtp_bundle(pauline_call, TRUE);
		
	if (BC_ASSERT_PTR_NOT_NULL(marie_call))
		check_rtp_bundle(marie_call, TRUE);

	liblinphone_tester_check_rtcp(marie,pauline);
	end_call(marie,pauline);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_audio_call(void){
	_simple_audio_call(FALSE);
}

static void simple_audio_call_with_srtp_dtls(void){
	_simple_audio_call(TRUE);
}

typedef struct params{
	bool_t with_ice;
	bool_t with_dtls_srtp;
} params_t;

static void audio_video_call(const params_t *params) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall *pauline_call, *marie_call;
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	
	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_rtp_bundle(marie->lc, TRUE);
	
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	
	linphone_core_set_preferred_video_size_by_name(marie->lc, "QVGA");
	linphone_core_set_preferred_video_size_by_name(pauline->lc, "QVGA");

	linphone_core_set_video_device(marie->lc, "Mire: Mire (synthetic moving picture)");
	linphone_core_set_video_device(pauline->lc, "Mire: Mire (synthetic moving picture)");
	
	if (params->with_ice){
		/*enable ICE on both ends*/
		LinphoneNatPolicy *pol;
		pol = linphone_core_get_nat_policy(marie->lc);
		linphone_nat_policy_enable_ice(pol, TRUE);
		linphone_nat_policy_enable_stun(pol, TRUE);
		linphone_core_set_nat_policy(marie->lc, pol);
		pol = linphone_core_get_nat_policy(pauline->lc);
		linphone_nat_policy_enable_ice(pol, TRUE);
		linphone_nat_policy_enable_stun(pol, TRUE);
		linphone_core_set_nat_policy(pauline->lc, pol);
	}
	
	if (params->with_dtls_srtp){
		setup_dtls_srtp(marie, pauline);
	}
	
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);
	
	if (!BC_ASSERT_TRUE(call(marie,pauline))) goto end;
	pauline_call=linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	
	check_rtp_bundle(pauline_call, TRUE);
	check_rtp_bundle(marie_call, TRUE);
	
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	
	if (params->with_ice){
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
	}
	
	liblinphone_tester_check_rtcp(marie,pauline);
	liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_IframeDecoded,1));
	liblinphone_tester_set_next_video_frame_decoded_cb(marie_call);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_IframeDecoded,1));
	
	end_call(marie,pauline);
	
end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_audio_video_call(void) {
	params_t params = {0};
	audio_video_call(&params);
}

static void audio_video_call_with_ice(void) {
	params_t params = {0};
	params.with_ice = TRUE;
	audio_video_call(&params);
}

static void audio_video_call_with_ice_and_dtls_srtp(void) {
	params_t params = {0};
	params.with_ice = TRUE;
	params.with_dtls_srtp = TRUE;
	audio_video_call(&params);
}

static test_t call_with_rtp_bundle_tests[] = {
	TEST_NO_TAG("Simple audio call", simple_audio_call),
	TEST_NO_TAG("Simple audio call with DTLS-SRTP", simple_audio_call_with_srtp_dtls),
	TEST_NO_TAG("Simple audio-video call", simple_audio_video_call),
	TEST_NO_TAG("Audio-video call with ICE", audio_video_call_with_ice),
	TEST_NO_TAG("Audio-video call with ICE and DTLS-SRTP", audio_video_call_with_ice_and_dtls_srtp)
};

test_suite_t call_with_rtp_bundle_test_suite = {"Call with RTP bundle", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_with_rtp_bundle_tests) / sizeof(call_with_rtp_bundle_tests[0]), call_with_rtp_bundle_tests};
								

