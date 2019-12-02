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

static void simple_audio_call(void) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall *pauline_call, *marie_call;
	
	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_rtp_bundle(marie->lc, TRUE);
	
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

static void simple_audio_video_call(void) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall *pauline_call, *marie_call;
	LinphoneVideoPolicy vpol;
	
	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_rtp_bundle(marie->lc, TRUE);
	
	vpol.automatically_accept = TRUE;
	vpol.automatically_initiate = TRUE;

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	
	linphone_core_set_video_policy(marie->lc, &vpol);
	linphone_core_set_video_policy(pauline->lc, &vpol);
	
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


static test_t call_with_rtp_bundle_tests[] = {
	TEST_NO_TAG("Simple audio call", simple_audio_call),
	TEST_NO_TAG("Simple audio-video call", simple_audio_video_call)
};

test_suite_t call_with_rtp_bundle_test_suite = {"Call with RTP bundle", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_with_rtp_bundle_tests) / sizeof(call_with_rtp_bundle_tests[0]), call_with_rtp_bundle_tests};
								

