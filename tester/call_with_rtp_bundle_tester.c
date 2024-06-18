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

#include <sys/stat.h>
#include <sys/types.h>

#include <bctoolbox/defs.h>

#include "belle-sip/sipstack.h"

#include "mediastreamer2/msutils.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

static void check_rtp_bundle(LinphoneCall *call, bool_t bundle_in_remote_expected, bool_t bundle_in_current_expected) {
	const LinphoneCallParams *remote_params = linphone_call_get_remote_params(call);
	const LinphoneCallParams *current_params = linphone_call_get_current_params(call);

	if (bundle_in_remote_expected) BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(remote_params));
	else BC_ASSERT_FALSE(linphone_call_params_rtp_bundle_enabled(remote_params));

	if (bundle_in_current_expected) {
		BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(current_params));

		if (linphone_call_params_video_enabled(current_params)) {
			if (linphone_call_params_has_custom_sdp_media_attribute(remote_params, LinphoneStreamTypeVideo,
			                                                        "bundle-only")) {
				// No rtcp port should be given
				BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_sdp_media_attribute(
				    remote_params, LinphoneStreamTypeVideo, "rtcp"));
			}
		}
	} else {
		BC_ASSERT_FALSE(linphone_call_params_rtp_bundle_enabled(current_params));
	}
}

static void check_rtp_bundle_mandatory(const LinphoneCallParams *remote_params) {
	if (linphone_call_params_video_enabled(remote_params)) {
		// No rtcp port should be given
		BC_ASSERT_PTR_NULL(
		    linphone_call_params_get_custom_sdp_media_attribute(remote_params, LinphoneStreamTypeVideo, "rtcp"));
		BC_ASSERT_TRUE(
		    linphone_call_params_has_custom_sdp_media_attribute(remote_params, LinphoneStreamTypeVideo, "bundle-only"));
	}
}

static bool_t setup_dtls_srtp(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	if (!linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionDTLS)) {
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
	bctbx_mkdir(linphone_core_get_user_certificates_path(marie->lc));
	bctbx_mkdir(linphone_core_get_user_certificates_path(pauline->lc));

	linphone_config_set_bool(linphone_core_get_config(marie->lc), "sip", "update_call_when_ice_completed_with_dtls",
	                         TRUE);
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "sip", "update_call_when_ice_completed_with_dtls",
	                         TRUE);

	return TRUE;
}

static void _simple_audio_call(bool_t with_dtls_srtp) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call, *marie_call;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	{
		LinphoneAccount *account = linphone_core_get_default_account(marie->lc);
		LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(account));
		linphone_account_params_enable_rtp_bundle(account_params, TRUE);
		linphone_account_set_params(account, account_params);
		linphone_account_params_unref(account_params);
	}

	if (with_dtls_srtp) {
		setup_dtls_srtp(marie, pauline);
	}

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);

	if (BC_ASSERT_PTR_NOT_NULL(pauline_call)) check_rtp_bundle(pauline_call, TRUE, TRUE);

	if (BC_ASSERT_PTR_NOT_NULL(marie_call)) check_rtp_bundle(marie_call, TRUE, TRUE);

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_audio_call(void) {
	_simple_audio_call(FALSE);
}

static void simple_audio_call_with_srtp_dtls(void) {
	_simple_audio_call(TRUE);
}

typedef struct params {
	bool_t with_ice;
	bool_t with_dtls_srtp;
	bool_t with_media_relay;
	bool_t disable_bundle;
	bool_t with_mandatory_bundle;
	bool_t bundle_not_supported;
} params_t;

static void audio_video_call(const params_t *params) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call, *marie_call;
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	bool_t bundle_accepted = TRUE;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	if (params->with_media_relay) {
		linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
		linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);

		linphone_core_enable_forced_ice_relay(marie->lc, TRUE);
		linphone_core_enable_forced_ice_relay(pauline->lc, TRUE);
	}

	{
		LinphoneAccount *account = linphone_core_get_default_account(marie->lc);
		LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(account));
		linphone_account_params_enable_rtp_bundle(account_params, TRUE);

		if (params->with_mandatory_bundle) {
			linphone_account_params_enable_rtp_bundle_assumption(account_params, TRUE);
		}
		linphone_account_set_params(account, account_params);
		linphone_account_params_unref(account_params);
	}

	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	linphone_core_set_preferred_video_definition_by_name(marie->lc, "QVGA");
	linphone_core_set_preferred_video_definition_by_name(pauline->lc, "QVGA");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	if (params->with_ice) {
		/*enable ICE on both ends*/
		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
		enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);
	}

	if (params->with_dtls_srtp) {
		setup_dtls_srtp(marie, pauline);
	}

	if (params->bundle_not_supported) {
		linphone_config_set_bool(linphone_core_get_config(pauline->lc), "rtp", "accept_bundle", FALSE);
		bundle_accepted = FALSE;
	}

	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);

	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) {
		goto end;
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);

	linphone_call_accept(pauline_call);

	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1)))
		goto end;

	if (params->with_mandatory_bundle) {
		check_rtp_bundle_mandatory(linphone_call_get_remote_params(pauline_call));
	}

	check_rtp_bundle(pauline_call, TRUE, bundle_accepted);
	check_rtp_bundle(marie_call, bundle_accepted, bundle_accepted);

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));

	if (params->with_ice) {
		/* Ice reinvite */
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));
		/* Check that ICE doesn't remove bundle accidentally */
		check_rtp_bundle(pauline_call, bundle_accepted, bundle_accepted);
		check_rtp_bundle(marie_call, bundle_accepted, bundle_accepted);
	}

	liblinphone_tester_check_rtcp(marie, pauline);
	liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded, 1));
	liblinphone_tester_set_next_video_frame_decoded_cb(marie_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded, 1));

	if (params->with_dtls_srtp) {
		BC_ASSERT_TRUE(linphone_call_params_get_media_encryption(linphone_call_get_current_params(pauline_call)) ==
		               LinphoneMediaEncryptionDTLS);
		BC_ASSERT_TRUE(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marie_call)) ==
		               LinphoneMediaEncryptionDTLS);
	}

	// make sure receive frame rate computation is done with a significant number of frame
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	BC_ASSERT_GREATER(linphone_call_params_get_received_framerate(linphone_call_get_current_params(pauline_call)), 8.0,
	                  float, "%f");
	BC_ASSERT_GREATER(linphone_call_params_get_received_framerate(linphone_call_get_current_params(marie_call)), 8.0,
	                  float, "%f");

	stats initial_marie_stat = marie->stat;
	stats initial_pauline_stat = pauline->stat;

	LinphoneCallParams *new_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_video(new_params, FALSE);
	if (params->disable_bundle) {
		/* This method is deprecated, but we still use to test that disablement of bundle
		 * is working. */
		linphone_call_params_enable_rtp_bundle(new_params, FALSE);
	} else if (bundle_accepted) {
		BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(new_params));
	}
	linphone_call_update(pauline_call, new_params);
	linphone_call_params_unref(new_params);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        initial_pauline_stat.number_of_LinphoneCallUpdating + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

	// Check video parameters
	const LinphoneCallParams *pauline_call_params = linphone_call_get_current_params(pauline_call);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(pauline_call_params));
	const LinphoneCallParams *marie_call_params = linphone_call_get_current_params(marie_call);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(marie_call_params));

	// Wait to see any undesirable side effect
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;

	new_params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_enable_video(new_params, TRUE);
	linphone_call_update(marie_call, new_params);
	linphone_call_params_unref(new_params);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdating,
	                        initial_marie_stat.number_of_LinphoneCallUpdating + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

	// Check video parameters
	pauline_call_params = linphone_call_get_current_params(pauline_call);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(pauline_call_params));
	marie_call_params = linphone_call_get_current_params(marie_call);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_params));

	/* Check that bundle is still activated*/
	if (!params->disable_bundle) {
		check_rtp_bundle(pauline_call, bundle_accepted, bundle_accepted);
		check_rtp_bundle(marie_call, bundle_accepted, bundle_accepted);
	}

	liblinphone_tester_check_rtcp(marie, pauline);
	liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded,
	                        initial_pauline_stat.number_of_IframeDecoded + 1));
	liblinphone_tester_set_next_video_frame_decoded_cb(marie_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded,
	                        initial_marie_stat.number_of_IframeDecoded + 1));

	// Wait to see any undesirable side effect
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_audio_video_call(void) {
	params_t params = {0};
	audio_video_call(&params);
}

static void simple_audio_video_call_with_bundle_disable(void) {
	params_t params = {0};
	params.disable_bundle = TRUE;
	audio_video_call(&params);
}

static void audio_video_call_with_ice(void) {
	params_t params = {0};
	params.with_ice = TRUE;
	audio_video_call(&params);
}

static void simple_audio_video_call_bundle_refused(void) {
	params_t params = {0};
	params.with_ice = TRUE;
	params.bundle_not_supported = TRUE;
	audio_video_call(&params);
}

static void audio_video_call_with_ice_and_dtls_srtp(void) {
	params_t params = {0};
	params.with_ice = TRUE;
	params.with_dtls_srtp = TRUE;
	audio_video_call(&params);
}

static void audio_video_call_with_forced_media_relay(void) {
	params_t params = {0};
	params.with_media_relay = TRUE;
	audio_video_call(&params);
}

static void call_with_mandatory_bundle(void) {
	params_t params = {0};
	params.with_ice = TRUE;
	params.with_dtls_srtp = TRUE;
	params.with_mandatory_bundle = TRUE;
	audio_video_call(&params);
}

static void simple_audio_video_call_with_bundle_enabled_by_reinvite(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call, *marie_call;
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	{
		LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
		LinphoneAccountParams *marie_account_params =
		    linphone_account_params_clone(linphone_account_get_params(marie_account));
		linphone_account_params_enable_rtp_bundle(marie_account_params, TRUE);
		linphone_account_set_params(marie_account, marie_account_params);
		linphone_account_params_unref(marie_account_params);

		LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);
		LinphoneAccountParams *pauline_account_params =
		    linphone_account_params_clone(linphone_account_get_params(pauline_account));
		linphone_account_params_enable_rtp_bundle(pauline_account_params, TRUE);
		linphone_account_set_params(pauline_account, pauline_account_params);
		linphone_account_params_unref(pauline_account_params);
	}

	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	linphone_core_set_preferred_video_definition_by_name(marie->lc, "QVGA");
	linphone_core_set_preferred_video_definition_by_name(pauline->lc, "QVGA");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(marie_params, FALSE);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_rtp_bundle(pauline_params, FALSE);
	linphone_call_params_enable_video(pauline_params, FALSE);

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params));

	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (!pauline_call) goto end;
	// As RTP bundle is enabled in the account parameters, there is no way to avoid from having it in the offer
	check_rtp_bundle(pauline_call, TRUE, FALSE);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_FALSE(linphone_call_params_rtp_bundle_enabled(linphone_call_get_current_params(pauline_call)));

	marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (!marie_call) goto end;
	check_rtp_bundle(marie_call, FALSE, FALSE);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_FALSE(linphone_call_params_rtp_bundle_enabled(linphone_call_get_current_params(marie_call)));

	stats initial_marie_stat = marie->stat;
	stats initial_pauline_stat = pauline->stat;

	LinphoneCallParams *new_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_video(new_params, TRUE);
	/* This method is deprecated, but we still use to test that disablement of bundle is working. */
	linphone_call_params_enable_rtp_bundle(new_params, TRUE);
	linphone_call_update(pauline_call, new_params);
	linphone_call_params_unref(new_params);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        initial_pauline_stat.number_of_LinphoneCallUpdating + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

	check_rtp_bundle(pauline_call, TRUE, TRUE);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(linphone_call_get_current_params(pauline_call)));

	check_rtp_bundle(marie_call, TRUE, TRUE);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_TRUE(linphone_call_params_rtp_bundle_enabled(linphone_call_get_current_params(marie_call)));

	liblinphone_tester_check_rtcp(marie, pauline);
	liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded, 1));
	liblinphone_tester_set_next_video_frame_decoded_cb(marie_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded, 1));

	// make sure receive frame rate computation is done with a significant number of frame
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	BC_ASSERT_GREATER(linphone_call_params_get_received_framerate(linphone_call_get_current_params(pauline_call)), 8.0,
	                  float, "%f");
	BC_ASSERT_GREATER(linphone_call_params_get_received_framerate(linphone_call_get_current_params(marie_call)), 8.0,
	                  float, "%f");

	// Wait to see any undesirable side effect
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static test_t call_with_rtp_bundle_tests[] = {
    TEST_NO_TAG("Simple audio call", simple_audio_call),
    TEST_NO_TAG("Simple audio call with DTLS-SRTP", simple_audio_call_with_srtp_dtls),
    TEST_NO_TAG("Simple audio-video call", simple_audio_video_call),
    TEST_NO_TAG("Simple audio-video call with bundle refused", simple_audio_video_call_bundle_refused),
    TEST_NO_TAG("Simple audio-video call with bundle disable", simple_audio_video_call_with_bundle_disable),
    TEST_NO_TAG("Simple audio-video call with bundle enabled by reINVITE",
                simple_audio_video_call_with_bundle_enabled_by_reinvite),
    TEST_NO_TAG("Audio-video call with ICE", audio_video_call_with_ice),
    TEST_NO_TAG("Audio-video call with ICE and DTLS-SRTP", audio_video_call_with_ice_and_dtls_srtp),
    TEST_NO_TAG("Mandatory bundle", call_with_mandatory_bundle),
    TEST_NO_TAG("Audio-video call with forced media relay", audio_video_call_with_forced_media_relay),
};

test_suite_t call_with_rtp_bundle_test_suite = {"Call with RTP bundle",
                                                NULL,
                                                NULL,
                                                liblinphone_tester_before_each,
                                                liblinphone_tester_after_each,
                                                sizeof(call_with_rtp_bundle_tests) /
                                                    sizeof(call_with_rtp_bundle_tests[0]),
                                                call_with_rtp_bundle_tests,
                                                0};
