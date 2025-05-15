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

#include "linphone/types.h"
#include "mediastreamer2/msanalysedisplay.h"
#include "mediastreamer2/msmediaplayer.h"
#include "mediastreamer2/msmire.h"

#include "call/call.h"
#include "liblinphone_tester.h"
#include "linphone/api//c-address.h"
#include "linphone/api//c-call-log.h"
#include "linphone/api//c-video-source-descriptor.h"
#include "linphone/core.h"
#include "sal/call-op.h"
#include "sal/sal_media_description.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

#ifdef VIDEO_ENABLED
std::string g_display_filter =
    ""; // Global variable to test unit in order to select the display filter to use : "" use the default

static std::string generateRandomFilename(const std::string &name) {
	char token[6];
	belle_sip_random_token(token, sizeof(token));
	return name + token;
}

static void call_paused_resumed_with_video_base_call_cb(LinphoneCore *lc,
                                                        LinphoneCall *call,
                                                        LinphoneCallState cstate,
                                                        BCTBX_UNUSED(const char *message)) {
	if (cstate == LinphoneCallUpdatedByRemote) {
		LinphoneCallParams *params = linphone_core_create_call_params(lc, call);
		linphone_call_params_enable_video(params, TRUE);
		ms_message(" New state LinphoneCallUpdatedByRemote on call [%p], accepting with video on", call);
		BC_ASSERT_NOT_EQUAL(linphone_call_accept_update(call, params), 0, int, "%i");
		linphone_call_params_unref(params);
	}
}
/*this test makes sure that pause/resume will not bring up video by accident*/
static void call_paused_resumed_with_video_base(bool_t sdp_200_ack,
                                                bool_t use_video_policy_for_re_invite_sdp_200,
                                                bool_t resume_in_audio_send_only_video_inactive_first,
                                                bool_t with_call_accept) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline, *call_marie;
	bctbx_list_t *lcs = NULL;
	LinphoneVideoActivationPolicy *vpol;

	bool_t call_ok;
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_paused_resumed_with_video_base_call_cb);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, marie->lc);

	vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());

	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE); /* needed to present a video mline*/
	linphone_video_activation_policy_set_automatically_accept(vpol, FALSE);
	/* needed to present a video mline*/

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	linphone_video_activation_policy_unref(vpol);

	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));

	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(call_marie)) goto end;
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	if (resume_in_audio_send_only_video_inactive_first) {
		LinphoneCallParams *params = linphone_core_create_call_params(pauline->lc, call_pauline);
		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendOnly);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionInactive);
		linphone_call_update(call_pauline, params);
		linphone_call_params_unref(params);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
	} else {
		linphone_call_pause(call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
	}
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_remote_params(call_marie)));
	if (resume_in_audio_send_only_video_inactive_first) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	} else {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));
	}

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	/*check if video stream is still offered even if disabled*/
	BC_ASSERT_EQUAL((int)_linphone_call_get_local_desc(call_pauline)->getNbStreams(), 2, int, "%i");
	BC_ASSERT_EQUAL((int)_linphone_call_get_local_desc(call_marie)->getNbStreams(), 2, int, "%i");

	linphone_core_enable_sdp_200_ack(pauline->lc, sdp_200_ack);

	if (use_video_policy_for_re_invite_sdp_200) {
		LpConfig *marie_lp;
		marie_lp = linphone_core_get_config(marie->lc);
		linphone_config_set_int(marie_lp, "sip", "sdp_200_ack_follow_video_policy", 1);
	}
	/*now pauline wants to resume*/
	if (resume_in_audio_send_only_video_inactive_first) {
		LinphoneCallParams *params = linphone_core_create_call_params(pauline->lc, call_pauline);
		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendOnly);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionInactive);
		linphone_call_update(call_pauline, params);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));
		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendRecv);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendRecv);
		if (with_call_accept) {
			linphone_core_add_callbacks(marie->lc, cbs);
		}
		linphone_call_update(call_pauline, params);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 4));
		linphone_call_params_unref(params);
	} else {
		linphone_call_resume(call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallResuming, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	if (use_video_policy_for_re_invite_sdp_200) {
		/*make sure video was offered*/
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_remote_params(call_pauline)));
	} else {
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_pauline)));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_marie)));
	}
	end_call(marie, pauline);

end:
	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void call_paused_resumed_with_video(void) {
	call_paused_resumed_with_video_base(FALSE, FALSE, FALSE, FALSE);
}

static void call_paused_resumed_with_no_sdp_ack(void) {
	call_paused_resumed_with_video_base(TRUE, FALSE, FALSE, FALSE);
}
static void call_paused_resumed_with_no_sdp_ack_using_video_policy(void) {
	call_paused_resumed_with_video_base(TRUE, TRUE, FALSE, FALSE);
}
static void call_paused_updated_resumed_with_no_sdp_ack_using_video_policy(void) {
	call_paused_resumed_with_video_base(TRUE, TRUE, TRUE, FALSE);
}
static void call_paused_updated_resumed_with_no_sdp_ack_using_video_policy_and_accept_call_update(void) {
	call_paused_resumed_with_video_base(TRUE, TRUE, TRUE, TRUE);
}

static void _call_paused_resumed_with_video_enabled(LinphoneMediaDirection video_direction) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline, *call_marie;
	bctbx_list_t *lcs = NULL;
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	bool_t call_ok;
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	LinphoneCallParams *params;
	VideoStream *vs;

	linphone_core_cbs_set_call_state_changed(cbs, call_paused_resumed_with_video_base_call_cb);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, marie->lc);

	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	linphone_video_activation_policy_unref(vpol);

	params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_set_video_direction(params, video_direction);

	BC_ASSERT_TRUE((call_ok = call_with_caller_params(pauline, marie, params)));
	linphone_call_params_unref(params);

	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(call_marie)) goto end;
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_call_pause(call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_remote_params(call_marie)));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	/*check if video stream is still offered even if disabled*/
	BC_ASSERT_EQUAL((int)_linphone_call_get_local_desc(call_pauline)->getNbStreams(), 2, int, "%i");
	BC_ASSERT_EQUAL((int)_linphone_call_get_local_desc(call_marie)->getNbStreams(), 2, int, "%i");

	linphone_call_resume(call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallResuming, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_pauline)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_marie)));

	vs = (VideoStream *)linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo);

	/* Make sure that the video stream restarted to stream the camera (rather than the static image place holder) */
	BC_ASSERT_TRUE(vs->source->desc->id == MS_MIRE_ID);
	BC_ASSERT_TRUE(vs->source->desc->id != MS_STATIC_IMAGE_ID);

	end_call(marie, pauline);

end:
	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void call_paused_resumed_with_video_enabled(void) {
	_call_paused_resumed_with_video_enabled(LinphoneMediaDirectionSendRecv);
}

static void call_paused_resumed_with_video_send_only(void) {
	_call_paused_resumed_with_video_enabled(LinphoneMediaDirectionSendOnly);
}

static void zrtp_video_call(void) {
	call_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE, LinphonePolicyNoFirewall, FALSE);
}

static void call_state_changed_callback_to_accept_video(LinphoneCore *lc,
                                                        LinphoneCall *call,
                                                        LinphoneCallState state,
                                                        BCTBX_UNUSED(const char *message)) {
	LinphoneCoreCbs *cbs;
	if (state == LinphoneCallUpdatedByRemote) {
		LinphoneCallParams *params = linphone_core_create_call_params(lc, call);
		linphone_call_params_enable_video(params, TRUE);
		linphone_call_accept_update(call, params);
		linphone_call_params_unref(params);
	}
	ms_message("video acceptance listener about to be dropped");
	cbs = (LinphoneCoreCbs *)belle_sip_object_data_get(BELLE_SIP_OBJECT(call),
	                                                   "call_state_changed_callback_to_accept_video");
	linphone_core_remove_callbacks(lc, cbs);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(call), "call_state_changed_callback_to_accept_video", NULL, NULL);
}

static LinphoneCall *
_request_video(LinphoneCoreManager *caller, LinphoneCoreManager *callee, bool_t accept_with_params) {
	LinphoneCallParams *callee_params;
	LinphoneCall *call_obj;

	if (!linphone_core_get_current_call(callee->lc) ||
	    linphone_call_get_state(linphone_core_get_current_call(callee->lc)) != LinphoneCallStreamsRunning ||
	    !linphone_core_get_current_call(caller->lc) ||
	    linphone_call_get_state(linphone_core_get_current_call(caller->lc)) != LinphoneCallStreamsRunning) {
		ms_warning("bad state for adding video");
		return NULL;
	}
	/*Assert the sanity of the developer, that is not expected to request video if video is already active.*/
	if (!BC_ASSERT_FALSE(linphone_call_params_video_enabled(
	        linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))))) {
		BC_FAIL("Video was requested while it was already active. This test doesn't look very sane.");
	}

	if (accept_with_params) {
		LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		linphone_core_cbs_set_call_state_changed(cbs, call_state_changed_callback_to_accept_video);
		linphone_core_add_callbacks(caller->lc, cbs);
		belle_sip_object_data_set(BELLE_SIP_OBJECT(linphone_core_get_current_call(caller->lc)),
		                          "call_state_changed_callback_to_accept_video", cbs,
		                          (void (*)(void *))linphone_core_cbs_unref);
	}
	linphone_core_enable_video_capture(callee->lc, TRUE);
	linphone_core_enable_video_display(callee->lc, TRUE);
	linphone_core_enable_video_capture(caller->lc, TRUE);
	linphone_core_enable_video_display(caller->lc, FALSE);

	if ((call_obj = linphone_core_get_current_call(callee->lc))) {
		callee_params = linphone_core_create_call_params(callee->lc, call_obj);
		/*add video*/
		linphone_call_params_enable_video(callee_params, TRUE);
		/* try to add a new custom header */
		linphone_call_params_add_custom_header(callee_params, "VIDEO-REINVITE", "1");
		linphone_call_update(call_obj, callee_params);
		linphone_call_params_unref(callee_params);
	}
	return call_obj;
}

/*
 * This function requests the addon of a video stream, initiated by "callee" and potentially accepted by "caller",
 * and asserts a number of things after this is done.
 * However the video addon may fail due to video policy, so that there is no insurance that video is actually added.
 * This function returns TRUE if video was successfully added, FALSE otherwise or if video is already there.
 **/
bool_t request_video(LinphoneCoreManager *caller, LinphoneCoreManager *callee, bool_t accept_with_params) {
	stats initial_caller_stat = caller->stat;
	stats initial_callee_stat = callee->stat;
	LinphoneCall *call_obj;
	bool_t video_added = FALSE;

	if ((call_obj = _request_video(caller, callee, accept_with_params))) {
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallUpdatedByRemote,
		                        initial_caller_stat.number_of_LinphoneCallUpdatedByRemote + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallUpdating,
		                        initial_callee_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_caller_stat.number_of_LinphoneCallStreamsRunning + 1));

		const LinphoneVideoActivationPolicy *video_policy = linphone_core_get_video_activation_policy(caller->lc);
		if (linphone_video_activation_policy_get_automatically_accept(video_policy) || accept_with_params) {
			video_added = BC_ASSERT_TRUE(linphone_call_params_video_enabled(
			    linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
			video_added = BC_ASSERT_TRUE(linphone_call_params_video_enabled(
			                  linphone_call_get_current_params(linphone_core_get_current_call(caller->lc)))) &&
			              video_added;
		} else {
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(
			    linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(
			    linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));
		}

		/* Check custom header added in re-INVITE is available on both sides */
		BC_ASSERT_STRING_EQUAL(
		    linphone_call_params_get_custom_header(
		        linphone_call_get_remote_params(linphone_core_get_current_call(caller->lc)), "VIDEO-REINVITE"),
		    "1");
		BC_ASSERT_STRING_EQUAL(
		    linphone_call_params_get_custom_header(linphone_call_get_params(linphone_core_get_current_call(callee->lc)),
		                                           "VIDEO-REINVITE"),
		    "1");

		if (linphone_core_get_media_encryption(caller->lc) != LinphoneMediaEncryptionNone &&
		    linphone_core_get_media_encryption(callee->lc) != LinphoneMediaEncryptionNone) {
			const LinphoneCallParams *call_param;

			switch (linphone_core_get_media_encryption(caller->lc)) {
				case LinphoneMediaEncryptionZRTP:
				case LinphoneMediaEncryptionDTLS:
					/*wait for encryption to be on, in case of zrtp/dtls, it can take a few seconds*/
					wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallEncryptedOn,
					         initial_caller_stat.number_of_LinphoneCallEncryptedOn + 1);
					break;
				case LinphoneMediaEncryptionNone:
				case LinphoneMediaEncryptionSRTP:
					break;
			}
			switch (linphone_core_get_media_encryption(callee->lc)) {
				case LinphoneMediaEncryptionZRTP:
				case LinphoneMediaEncryptionDTLS:
					wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallEncryptedOn,
					         initial_callee_stat.number_of_LinphoneCallEncryptedOn + 1);
					break;
				case LinphoneMediaEncryptionNone:
				case LinphoneMediaEncryptionSRTP:
					break;
			}

			call_param = linphone_call_get_current_params(linphone_core_get_current_call(callee->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),
			                linphone_core_get_media_encryption(caller->lc), int, "%d");
			call_param = linphone_call_get_current_params(linphone_core_get_current_call(caller->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),
			                linphone_core_get_media_encryption(caller->lc), int, "%d");
		}

		if (video_added) {
			liblinphone_tester_set_next_video_frame_decoded_cb(call_obj);
			/*send vfu*/
			linphone_call_send_vfu_request(call_obj);
			BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_IframeDecoded,
			                        initial_callee_stat.number_of_IframeDecoded + 1));
			return TRUE;
		}
	}
	return FALSE;
}

bool_t remove_video(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	LinphoneCallParams *callee_params;
	LinphoneCall *call_obj;
	stats initial_caller_stat = caller->stat;
	stats initial_callee_stat = callee->stat;

	if (!linphone_core_get_current_call(callee->lc) ||
	    (linphone_call_get_state(linphone_core_get_current_call(callee->lc)) != LinphoneCallStreamsRunning) ||
	    !linphone_core_get_current_call(caller->lc) ||
	    (linphone_call_get_state(linphone_core_get_current_call(caller->lc)) != LinphoneCallStreamsRunning)) {
		ms_warning("bad state for removing video");
		return FALSE;
	}

	if ((call_obj = linphone_core_get_current_call(callee->lc))) {

		if (!BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_obj)))) {
			BC_FAIL("Video was asked to be dropped while it was not active. This test doesn't look very sane.");
			return FALSE;
		}

		callee_params = linphone_core_create_call_params(callee->lc, call_obj);
		/* Remove video. */
		linphone_call_params_enable_video(callee_params, FALSE);
		linphone_call_update(call_obj, callee_params);
		linphone_call_params_unref(callee_params);

		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallUpdatedByRemote,
		                        initial_caller_stat.number_of_LinphoneCallUpdatedByRemote + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallUpdating,
		                        initial_callee_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_caller_stat.number_of_LinphoneCallStreamsRunning + 1));

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(
		    linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(
		    linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));

		return TRUE;
	}
	return FALSE;
}

static void call_with_video_added(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_video_added_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	/*in this variant marie is already in automatically accept*/
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	vpol->automatically_accept = TRUE;
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, FALSE);

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(request_video(marie, pauline, TRUE));

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_video_added_random_ports(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_set_audio_port(marie->lc, -1);
	linphone_core_set_video_port(marie->lc, -1);
	linphone_core_set_audio_port(pauline->lc, -1);
	linphone_core_set_video_port(pauline->lc, -1);

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionNone, LinphoneMediaEncryptionNone);
}

static void call_with_declined_video_base(bool_t using_policy) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *marie_call;
	LinphoneCall *pauline_call;
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	bool_t call_ok;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);

	if (using_policy) {
		LinphoneVideoActivationPolicy *marie_vpol =
		    linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(marie_vpol, FALSE);
		linphone_video_activation_policy_set_automatically_initiate(marie_vpol, FALSE);
		linphone_core_set_video_activation_policy(marie->lc, marie_vpol);
		linphone_video_activation_policy_unref(marie_vpol);

		LinphoneVideoActivationPolicy *pauline_vpol =
		    linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pauline_vpol, FALSE);
		linphone_video_activation_policy_set_automatically_initiate(pauline_vpol, TRUE);
		linphone_core_set_video_activation_policy(pauline->lc, pauline_vpol);
		linphone_video_activation_policy_unref(pauline_vpol);
	}

	caller_test_params.base = linphone_core_create_call_params(pauline->lc, NULL);
	if (!using_policy) linphone_call_params_enable_video(caller_test_params.base, TRUE);

	if (!using_policy) {
		callee_test_params.base = linphone_core_create_call_params(marie->lc, NULL);
		linphone_call_params_enable_video(callee_test_params.base, FALSE);
	}

	BC_ASSERT_TRUE(
	    (call_ok = call_with_params2(pauline, marie, &caller_test_params, &callee_test_params, using_policy)));
	if (!call_ok) goto end;

	linphone_call_params_unref(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_unref(callee_test_params.base);
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_call = linphone_core_get_current_call(pauline->lc);

	BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
	BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_with_declined_video(void) {
	call_with_declined_video_base(FALSE);
}

static void call_with_declined_video_despite_policy(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *marie_call;
	LinphoneCall *pauline_call;
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	bool_t call_ok;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	caller_test_params.base = linphone_core_create_call_params(pauline->lc, NULL);

	callee_test_params.base = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(callee_test_params.base, FALSE);

	BC_ASSERT_TRUE((call_ok = call_with_params2(pauline, marie, &caller_test_params, &callee_test_params, FALSE)));
	if (!call_ok) goto end;

	linphone_call_params_unref(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_unref(callee_test_params.base);
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_call = linphone_core_get_current_call(pauline->lc);

	BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
	BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_declined_video_using_policy(void) {
	call_with_declined_video_base(TRUE);
}

void video_call_base_2(LinphoneCoreManager *caller,
                       LinphoneCoreManager *callee,
                       bool_t using_policy,
                       LinphoneMediaEncryption mode,
                       bool_t callee_video_enabled,
                       bool_t caller_video_enabled) {
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	LinphoneCall *callee_call;
	LinphoneCall *caller_call;

	if (using_policy) {
		LinphoneVideoActivationPolicy *callee_vpol =
		    linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
		linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);
		linphone_core_set_video_activation_policy(callee->lc, callee_vpol);
		linphone_video_activation_policy_unref(callee_vpol);

		LinphoneVideoActivationPolicy *caller_vpol =
		    linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
		linphone_video_activation_policy_set_automatically_accept(caller_vpol, FALSE);
		linphone_core_set_video_activation_policy(caller->lc, caller_vpol);
		linphone_video_activation_policy_unref(caller_vpol);
	}

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_display(callee->lc, callee_video_enabled);
	linphone_core_enable_video_capture(callee->lc, callee_video_enabled);

	linphone_core_enable_video_display(caller->lc, caller_video_enabled);
	linphone_core_enable_video_capture(caller->lc, caller_video_enabled);

	if (mode == LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a directory to
		                                          store them */
		char *path = bc_tester_file("certificates-marie");
		linphone_core_set_user_certificates_path(callee->lc, path);
		bc_free(path);
		path = bc_tester_file("certificates-pauline");
		linphone_core_set_user_certificates_path(caller->lc, path);
		bc_free(path);
		bctbx_mkdir(linphone_core_get_user_certificates_path(callee->lc));
		bctbx_mkdir(linphone_core_get_user_certificates_path(caller->lc));
	}

	linphone_core_set_media_encryption(callee->lc, mode);
	linphone_core_set_media_encryption(caller->lc, mode);

	caller_test_params.base = linphone_core_create_call_params(caller->lc, NULL);
	if (!using_policy) linphone_call_params_enable_video(caller_test_params.base, TRUE);

	if (!using_policy) {
		callee_test_params.base = linphone_core_create_call_params(callee->lc, NULL);
		linphone_call_params_enable_video(callee_test_params.base, TRUE);
	}

	BC_ASSERT_TRUE(call_with_params2(caller, callee, &caller_test_params, &callee_test_params, using_policy));
	callee_call = linphone_core_get_current_call(callee->lc);
	caller_call = linphone_core_get_current_call(caller->lc);

	linphone_call_params_unref(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_unref(callee_test_params.base);

	if (callee_call && caller_call) {
		if (callee_video_enabled && caller_video_enabled) {
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));

			/*check video path*/
			liblinphone_tester_set_next_video_frame_decoded_cb(callee_call);
			linphone_call_send_vfu_request(callee_call);
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_IframeDecoded, 1));
		} else {
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));
		}
		liblinphone_tester_check_rtcp(callee, caller);
	}
}

static void check_fir(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	LinphoneCall *callee_call;
	LinphoneCall *caller_call;
	VideoStream *callee_vstream;
	VideoStream *caller_vstream;

	callee_call = linphone_core_get_current_call(callee->lc);
	caller_call = linphone_core_get_current_call(caller->lc);

	/*check video path is established in both directions.
	 Indeed, FIR are ignored until the first RTP packet is received, because SSRC is not known.*/
	liblinphone_tester_set_next_video_frame_decoded_cb(callee_call);
	liblinphone_tester_set_next_video_frame_decoded_cb(caller_call);

	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_IframeDecoded, 1));
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_IframeDecoded, 1));

	linphone_call_send_vfu_request(callee_call);

	callee_vstream = (VideoStream *)linphone_call_get_stream(callee_call, LinphoneStreamTypeVideo);
	caller_vstream = (VideoStream *)linphone_call_get_stream(caller_call, LinphoneStreamTypeVideo);
	if (media_stream_avpf_enabled(&callee_vstream->ms))
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller_vstream->ms_video_stat.counter_rcvd_fir, 1));
	else BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller_vstream->ms_video_stat.counter_rcvd_fir, 0));
	ms_message("check_fir: [%p] received  %d FIR  ", &caller_call, caller_vstream->ms_video_stat.counter_rcvd_fir);
	ms_message("check_fir: [%p] stat number of iframe decoded  %d ", &callee_call,
	           callee->stat.number_of_IframeDecoded);

	liblinphone_tester_set_next_video_frame_decoded_cb(caller_call);
	linphone_call_send_vfu_request(caller_call);
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_IframeDecoded, 1));

	if (media_stream_avpf_enabled(&caller_vstream->ms)) {
		if (media_stream_avpf_enabled(&callee_vstream->ms))
			BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee_vstream->ms_video_stat.counter_rcvd_fir, 1));
	} else BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee_vstream->ms_video_stat.counter_rcvd_fir, 0));
	ms_message("check_fir: [%p] received  %d FIR  ", &callee_call, callee_vstream->ms_video_stat.counter_rcvd_fir);
	ms_message("check_fir: [%p] stat number of iframe decoded  %d ", &caller_call,
	           caller->stat.number_of_IframeDecoded);
}

void video_call_base_3(LinphoneCoreManager *caller,
                       LinphoneCoreManager *callee,
                       bool_t using_policy,
                       LinphoneMediaEncryption mode,
                       bool_t callee_video_enabled,
                       bool_t caller_video_enabled) {
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};

	LinphoneCall *callee_call;
	LinphoneCall *caller_call;

	if (using_policy) {
		LinphoneVideoActivationPolicy *callee_vpol =
		    linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
		linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);
		linphone_core_set_video_activation_policy(callee->lc, callee_vpol);
		linphone_video_activation_policy_unref(callee_vpol);

		LinphoneVideoActivationPolicy *caller_vpol =
		    linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
		linphone_video_activation_policy_set_automatically_accept(caller_vpol, FALSE);
		linphone_core_set_video_activation_policy(caller->lc, caller_vpol);
		linphone_video_activation_policy_unref(caller_vpol);
	}

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
	}

	linphone_core_set_preferred_video_definition_by_name(caller->lc, "QVGA");
	linphone_core_set_preferred_video_definition_by_name(callee->lc, "QVGA");

	linphone_core_set_video_device(caller->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(callee->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_display(callee->lc, callee_video_enabled);
	linphone_core_enable_video_capture(callee->lc, callee_video_enabled);

	linphone_core_enable_video_display(caller->lc, caller_video_enabled);
	linphone_core_enable_video_capture(caller->lc, caller_video_enabled);

	if (mode == LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a directory to
		                                          store them */
		char *path = bc_tester_file("certificates-marie");
		linphone_core_set_user_certificates_path(callee->lc, path);
		bc_free(path);
		path = bc_tester_file("certificates-pauline");
		linphone_core_set_user_certificates_path(caller->lc, path);
		bc_free(path);
		bctbx_mkdir(linphone_core_get_user_certificates_path(callee->lc));
		bctbx_mkdir(linphone_core_get_user_certificates_path(caller->lc));
	}

	linphone_core_set_media_encryption(callee->lc, mode);
	linphone_core_set_media_encryption(caller->lc, mode);
	/* Create call params */
	caller_test_params.base = linphone_core_create_call_params(caller->lc, NULL);

	if (!using_policy) linphone_call_params_enable_video(caller_test_params.base, TRUE);

	if (!using_policy) {
		callee_test_params.base = linphone_core_create_call_params(callee->lc, NULL);
		linphone_call_params_enable_video(callee_test_params.base, TRUE);
	}

	BC_ASSERT_TRUE(call_with_params2(caller, callee, &caller_test_params, &callee_test_params, using_policy));
	callee_call = linphone_core_get_current_call(callee->lc);
	caller_call = linphone_core_get_current_call(caller->lc);

	linphone_call_params_unref(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_unref(callee_test_params.base);

	if (callee_call && caller_call) {
		if (callee_video_enabled && caller_video_enabled) {
			check_fir(caller, callee);
		} else {
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));
		}
		liblinphone_tester_check_rtcp(callee, caller);
	}
}

static void video_call_base(LinphoneCoreManager *pauline,
                            LinphoneCoreManager *marie,
                            bool_t using_policy,
                            LinphoneMediaEncryption mode,
                            bool_t callee_video_enabled,
                            bool_t caller_video_enabled) {
	video_call_base_2(pauline, marie, using_policy, mode, callee_video_enabled, caller_video_enabled);
	end_call(pauline, marie);
}

static void video_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie, pauline, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_dummy_codec(void) {
	LpConfig *lp;
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_get_payload_type(pauline->lc, "h264", -1, -1) != NULL) {
		bctbx_warning("Test skipped: this test is enabled only when h264 is not available");
		linphone_core_manager_destroy(pauline);
		return;
	}
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	lp = linphone_core_get_config(marie->lc);
	linphone_config_set_bool(lp, "video", "fallback_to_dummy_codec", FALSE);
	linphone_config_set_bool(lp, "video", "dont_check_codecs", TRUE);
	linphone_core_reload_ms_plugins(marie->lc, NULL); // force codec config reload
	lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_bool(lp, "video", "fallback_to_dummy_codec", FALSE);
	linphone_config_set_bool(lp, "video", "dont_check_codecs", TRUE);
	linphone_core_reload_ms_plugins(pauline->lc, NULL); // force codec config reload
	disable_all_video_codecs_except_one(marie->lc, "h264");
	disable_all_video_codecs_except_one(pauline->lc, "h264");
	LinphoneCallTestParams marie_test_params = {0}, pauline_test_params = {0};

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	/* Perform a call, with fallback to dummy deactivated
	 * Call is established but the video stream is not uploading data */
	marie_test_params.base = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(marie_test_params.base, TRUE);
	pauline_test_params.base = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_video(pauline_test_params.base, TRUE);

	BC_ASSERT_TRUE(call_with_params2(marie, pauline, &marie_test_params, &pauline_test_params, FALSE));
	linphone_call_params_unref(marie_test_params.base);
	linphone_call_params_unref(pauline_test_params.base);
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	/* 2 seconds wait and check the videostream is only initialized (and not started) */
	int dummy = 0;
	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 2000);

	if (pauline_call && marie_call) {
		BC_ASSERT_TRUE(media_stream_get_state(linphone_call_get_stream(pauline_call, LinphoneStreamTypeVideo)) ==
		               MSStreamInitialized);
		BC_ASSERT_TRUE(media_stream_get_state(linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo)) ==
		               MSStreamInitialized);
	} else {
		BC_FAIL("Fail to get current calls");
	}
	end_call(pauline, marie);

	/* Perform a call, with fallback to dummy activated
	 * Call is established and the video stream is uploading(meaningless) data */
	lp = linphone_core_get_config(marie->lc);
	linphone_config_set_bool(lp, "video", "fallback_to_dummy_codec", TRUE);
	lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_bool(lp, "video", "fallback_to_dummy_codec", TRUE);
	marie_test_params.base = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(marie_test_params.base, TRUE);
	pauline_test_params.base = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_video(pauline_test_params.base, TRUE);

	BC_ASSERT_TRUE(call_with_params2(marie, pauline, &marie_test_params, &pauline_test_params, FALSE));
	linphone_call_params_unref(marie_test_params.base);
	linphone_call_params_unref(pauline_test_params.base);
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);

	/* 2 seconds wait and check the videostream is started */
	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 2000);
	if (pauline_call && marie_call) {
		BC_ASSERT_TRUE(media_stream_get_state(linphone_call_get_stream(pauline_call, LinphoneStreamTypeVideo)) ==
		               MSStreamStarted);
		BC_ASSERT_TRUE(media_stream_get_state(linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo)) ==
		               MSStreamStarted);
	} else {
		BC_FAIL("Fail to get current calls");
	}
	end_call(pauline, marie);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_without_rtcp(void) {
	LpConfig *lp;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(lp, "rtp", "rtcp_enabled", 0);

	lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_int(lp, "rtp", "rtcp_enabled", 0);

	video_call_base(marie, pauline, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_disable_implicit_AVPF_on_callee(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *callee_lp;
	const LinphoneCallParams *params, *params2;

	callee_lp = linphone_core_get_config(callee->lc);
	linphone_config_set_int(callee_lp, "rtp", "rtcp_fb_implicit_rtcp_fb", 0);

	video_call_base_3(caller, callee, TRUE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	if (BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(callee->lc))) {
		params = linphone_call_get_current_params(linphone_core_get_current_call(callee->lc));
		BC_ASSERT_STRING_EQUAL(linphone_call_params_get_rtp_profile(params), "RTP/AVP");
	}
	if (BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(caller->lc))) {
		params2 = linphone_call_get_current_params(linphone_core_get_current_call(caller->lc));
		BC_ASSERT_STRING_EQUAL(linphone_call_params_get_rtp_profile(params2), "RTP/AVP");
	}
	end_call(caller, callee);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_disable_implicit_AVPF_on_caller(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *caller_lp;
	const LinphoneCallParams *params, *params2;

	caller_lp = linphone_core_get_config(caller->lc);
	linphone_config_set_int(caller_lp, "rtp", "rtcp_fb_implicit_rtcp_fb", 0);

	video_call_base_3(caller, callee, TRUE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	params = linphone_call_get_current_params(linphone_core_get_current_call(callee->lc));
	BC_ASSERT_STRING_EQUAL(linphone_call_params_get_rtp_profile(params), "RTP/AVP");
	params2 = linphone_call_get_current_params(linphone_core_get_current_call(caller->lc));
	BC_ASSERT_STRING_EQUAL(linphone_call_params_get_rtp_profile(params2), "RTP/AVP");
	end_call(caller, callee);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_AVPF_to_implicit_AVPF(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_avpf_mode(caller->lc, LinphoneAVPFEnabled);
	video_call_base_3(caller, callee, TRUE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	end_call(caller, callee);

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_implicit_AVPF_to_AVPF(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_avpf_mode(callee->lc, LinphoneAVPFEnabled);
	video_call_base_3(caller, callee, TRUE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	end_call(caller, callee);

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_using_policy_AVPF_implicit_caller_and_callee(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base_3(caller, callee, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	end_call(caller, callee);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_established_by_reinvite_with_implicit_avpf(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *caller_call, *callee_call;
	LinphoneCallParams *params;
	VideoStream *vstream;
	char *record_file = bc_tester_file((generateRandomFilename("avrecord") + ".mkv").c_str());

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, FALSE);
	linphone_core_set_video_activation_policy(callee->lc, callee_vpol);
	linphone_video_activation_policy_unref(callee_vpol);

	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, TRUE);
	linphone_core_set_video_activation_policy(caller->lc, caller_vpol);
	linphone_video_activation_policy_unref(caller_vpol);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
	}

	disable_all_video_codecs_except_one(callee->lc, "VP8");
	disable_all_video_codecs_except_one(caller->lc, "VP8");

	linphone_core_enable_video_display(callee->lc, TRUE);
	linphone_core_enable_video_capture(callee->lc, TRUE);
	LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(callee->lc);
	linphone_proxy_config_edit(config);
	linphone_proxy_config_set_avpf_mode(config, LinphoneAVPFEnabled);
	linphone_proxy_config_done(config);

	linphone_core_enable_video_display(caller->lc, TRUE);
	linphone_core_enable_video_capture(caller->lc, TRUE);

	linphone_core_set_video_device(caller->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(callee->lc, liblinphone_tester_mire_id);

	caller_call = linphone_core_invite_address(caller->lc, callee->identity);
	if (BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallIncomingReceived, 1))) {
		callee_call = linphone_core_get_current_call(callee->lc);
		params = linphone_core_create_call_params(callee->lc, callee_call);
		linphone_call_params_set_record_file(params, record_file);

		linphone_core_accept_call_with_params(callee->lc, callee_call, params);
		linphone_call_params_unref(params);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning, 1));
		linphone_call_start_recording(callee_call);
		wait_for_until(caller->lc, callee->lc, NULL, 0, 3000);

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(callee_call)));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(caller_call)));

		/*then callee adds video*/
		params = linphone_core_create_call_params(callee->lc, callee_call);
		linphone_call_params_enable_video(params, TRUE);
		linphone_call_update(callee_call, params);
		linphone_call_params_unref(params);
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(callee_call)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(caller_call)));

		liblinphone_tester_set_next_video_frame_decoded_cb(caller_call);
		liblinphone_tester_set_next_video_frame_decoded_cb(callee_call);

		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_IframeDecoded, 1));
		BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &caller->stat.number_of_IframeDecoded, 1));
		wait_for_until(caller->lc, callee->lc, NULL, 0, 3000);

		vstream = (VideoStream *)linphone_call_get_stream(caller_call, LinphoneStreamTypeVideo);
		BC_ASSERT_TRUE(media_stream_avpf_enabled((MediaStream *)vstream));
		vstream = (VideoStream *)linphone_call_get_stream(callee_call, LinphoneStreamTypeVideo);
		BC_ASSERT_TRUE(media_stream_avpf_enabled((MediaStream *)vstream));
	}

	end_call(caller, callee);
	BC_ASSERT_EQUAL(bctbx_file_exist(record_file), 0, int, "%d");
	/* make sure the recorded file has a video track */
	MSMediaPlayer *mp = ms_media_player_new(linphone_core_get_ms_factory(caller->lc), NULL, NULL, NULL);
	BC_ASSERT_TRUE(ms_media_player_open(mp, record_file));
	BC_ASSERT_TRUE(ms_media_player_has_video_track(mp));
	ms_media_player_close(mp);
	ms_media_player_free(mp);

	bc_free(record_file);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_base_avpf(LinphoneCoreManager *caller,
                                 LinphoneCoreManager *callee,
                                 bool_t using_policy,
                                 LinphoneMediaEncryption mode,
                                 bool_t callee_video_enabled,
                                 bool_t caller_video_enabled) {
	linphone_core_set_avpf_mode(caller->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(callee->lc, LinphoneAVPFEnabled);
	video_call_base_3(caller, callee, using_policy, mode, callee_video_enabled, caller_video_enabled);
	end_call(caller, callee);
}

static void video_call_avpf(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	video_call_base_avpf(caller, callee, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_zrtp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP)) {
		video_call_base(marie, pauline, FALSE, LinphoneMediaEncryptionZRTP, TRUE, TRUE);
	} else ms_message("Skipping video_call_zrtp");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_dtls(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(pauline->lc, LinphoneMediaEncryptionDTLS)) {
		video_call_base(marie, pauline, FALSE, LinphoneMediaEncryptionDTLS, TRUE, TRUE);
	} else ms_message("Skipping video_call_dtls");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_using_policy(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(pauline, marie, TRUE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_using_policy_with_callee_video_disabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie, pauline, TRUE, LinphoneMediaEncryptionNone, FALSE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_using_policy_with_caller_video_disabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie, pauline, TRUE, LinphoneMediaEncryptionNone, TRUE, FALSE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_no_sdp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_sdp_200_ack(pauline->lc, TRUE);
	video_call_base(pauline, marie, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_video_to_novideo(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneVideoActivationPolicy *pauline_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(pauline_vpol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pauline_vpol);
	linphone_video_activation_policy_unref(pauline_vpol);

	LinphoneVideoActivationPolicy *marie_vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(marie_vpol, FALSE);
	linphone_core_set_video_activation_policy(marie->lc, marie_vpol);
	linphone_video_activation_policy_unref(marie_vpol);

	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, TRUE); // Cancel while gathering candidates.
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This function aims at testing ICE together with video enablement policies, and video enablements/disablements by
 * either caller or callee. It doesn't use linphone_core_accept_call_with_params() to accept video despite of default
 * policies.
 */
static void _call_with_ice_video(LinphoneVideoActivationPolicy *caller_policy,
                                 LinphoneVideoActivationPolicy *callee_policy,
                                 bool_t video_added_by_caller,
                                 bool_t video_added_by_callee,
                                 bool_t video_removed_by_caller,
                                 bool_t video_removed_by_callee,
                                 bool_t video_only) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	unsigned int nb_audio_starts = 1, nb_video_starts = 0;
	const LinphoneCallParams *marie_remote_params;
	const LinphoneCallParams *pauline_current_params;

	/*
	 * Pauline is the caller
	 * Marie is the callee
	 */

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, caller_policy);
	linphone_core_set_video_activation_policy(marie->lc, callee_policy);
	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	if (video_only) {
		LinphonePayloadType *pt_pcmu = linphone_core_get_payload_type(marie->lc, "PCMU", 8000, 1);
		linphone_payload_type_enable(pt_pcmu, FALSE); /* Disable PCMU */
		LinphonePayloadType *pt_pcma = linphone_core_get_payload_type(marie->lc, "PCMA", 8000, 1);
		linphone_payload_type_enable(pt_pcma, TRUE); /* Enable PCMA */
		nb_audio_starts = 0;

		linphone_payload_type_unref(pt_pcmu);
		linphone_payload_type_unref(pt_pcma);
	}

	linphone_core_manager_wait_for_stun_resolution(marie);
	linphone_core_manager_wait_for_stun_resolution(pauline);

	/* This is to activate media relay on Flexisip server.
	 * Indeed, we want to test ICE with relay candidates as well, even though
	 * they will not be used at the end.*/
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);

	linphone_core_set_audio_port(marie->lc, -1);
	linphone_core_set_video_port(marie->lc, -1);
	linphone_core_set_audio_port(pauline->lc, -1);
	linphone_core_set_video_port(pauline->lc, -1);

	linphone_core_invite_address(pauline->lc, marie->identity);
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1)))
		goto end;
	marie_remote_params = linphone_call_get_remote_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(marie_remote_params);
	if (marie_remote_params) {
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_remote_params) ==
		               caller_policy->automatically_initiate);
	}

	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1) &&
	               wait_for(pauline->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	pauline_current_params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(pauline_current_params);
	if (pauline_current_params) {
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(pauline_current_params) ==
		               (caller_policy->automatically_initiate && callee_policy->automatically_accept));
		if (linphone_call_params_video_enabled(pauline_current_params)) nb_video_starts++;
	}

	/* Wait for ICE reINVITEs to complete. */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2) &&
	               wait_for(pauline->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	if (callee_policy->automatically_accept == FALSE) {
		marie_remote_params = linphone_call_get_remote_params(linphone_core_get_current_call(marie->lc));
		/*The ICE reINVITE must not propose again video if was refused by callee*/
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(marie_remote_params));
	}

	BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	BC_ASSERT_TRUE(check_nb_media_starts(AUDIO_START, pauline, marie, nb_audio_starts, nb_audio_starts));
	BC_ASSERT_TRUE(check_nb_media_starts(VIDEO_START, pauline, marie, nb_video_starts, nb_video_starts));

	BC_ASSERT_FALSE(linphone_call_media_in_progress(linphone_core_get_current_call(marie->lc)));
	BC_ASSERT_FALSE(linphone_call_media_in_progress(linphone_core_get_current_call(pauline->lc)));

	if (caller_policy->automatically_initiate && callee_policy->automatically_accept &&
	    (video_added_by_caller || video_added_by_callee)) {
		BC_FAIL("Tired developer detected. You have requested the test to add video while it is already established "
		        "from the beginning of the call.");
	} else {
		if (video_added_by_caller) {
			BC_ASSERT_TRUE(request_video(marie, pauline, FALSE) == callee_policy->automatically_accept);
		} else if (video_added_by_callee) {
			BC_ASSERT_TRUE(request_video(pauline, marie, FALSE) == caller_policy->automatically_accept);
		}
		if (video_added_by_caller || video_added_by_callee) {
			BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
			if (linphone_call_params_video_enabled(
			        linphone_call_get_current_params(linphone_core_get_current_call(marie->lc)))) {
				/* Wait for ICE reINVITEs to complete if video was really added */
				BC_ASSERT_TRUE(
				    wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 4) &&
				    wait_for(pauline->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 4));
				/*the video addon should have triggered a media start, but the ICE reINVITE shall not*/
				nb_video_starts++;
				BC_ASSERT_TRUE(check_nb_media_starts(VIDEO_START, pauline, marie, nb_video_starts, nb_video_starts));
				BC_ASSERT_FALSE(linphone_call_media_in_progress(linphone_core_get_current_call(marie->lc)));
				BC_ASSERT_FALSE(linphone_call_media_in_progress(linphone_core_get_current_call(pauline->lc)));
			}
		}
	}

	if (video_removed_by_caller) {
		BC_ASSERT_TRUE(remove_video(marie, pauline));
	} else if (video_removed_by_callee) {
		BC_ASSERT_TRUE(remove_video(pauline, marie));
	}
	if (video_removed_by_caller || video_removed_by_callee) {
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_nb_media_starts(VIDEO_START, pauline, marie, nb_video_starts, nb_video_starts));
		BC_ASSERT_FALSE(linphone_call_media_in_progress(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_FALSE(linphone_call_media_in_progress(linphone_core_get_current_call(pauline->lc)));
	}

	end_call(pauline, marie);

end:
	linphone_video_activation_policy_unref(caller_policy);
	linphone_video_activation_policy_unref(callee_policy);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_video_added(void) {
	/*
	 * Scenario: video is not active at the beginning of the call,
	  caller requests it but callee declines it
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, FALSE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, FALSE);

	_call_with_ice_video(caller_vpol, callee_vpol, TRUE, FALSE, FALSE, FALSE, FALSE);
}

static void call_with_ice_video_added_2(void) {
	/*
	 * Scenario: video is not active at the beginning of the call, callee requests it but caller declines it
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, FALSE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, FALSE);

	_call_with_ice_video(caller_vpol, callee_vpol, FALSE, TRUE, FALSE, FALSE, FALSE);
}

static void call_with_ice_video_added_3(void) { /*
	                                             * Scenario: video is not active at the beginning of the call, caller
	                                             * requests it and callee accepts. Finally caller removes it.
	                                             */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, FALSE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);

	_call_with_ice_video(caller_vpol, callee_vpol, TRUE, FALSE, TRUE, FALSE, FALSE);
}

static void call_with_ice_video_added_4(void) {
	/*
	 * Scenario: video is not active at the beginning of the call, callee requests it and caller accepts.
	 * Finally caller removes it.
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, TRUE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, FALSE);

	_call_with_ice_video(caller_vpol, callee_vpol, FALSE, TRUE, TRUE, FALSE, FALSE);
}

static void call_with_ice_video_added_5(void) {
	/*
	 * Scenario: video is not active at the beginning of the call, callee requests it and caller accepts.
	 * Finally callee removes it.
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, TRUE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, FALSE);

	_call_with_ice_video(caller_vpol, callee_vpol, FALSE, TRUE, FALSE, TRUE, FALSE);
}

static void call_with_ice_video_added_6(void) {
	/*
	 * Scenario: video is active at the beginning of the call, caller removes it.
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, TRUE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);

	_call_with_ice_video(caller_vpol, callee_vpol, FALSE, FALSE, TRUE, FALSE, FALSE);
}

static void call_with_ice_video_added_7(void) {
	/*
	 * Scenario: video is active at the beginning of the call, callee removes it.
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, TRUE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);

	_call_with_ice_video(caller_vpol, callee_vpol, FALSE, FALSE, FALSE, TRUE, FALSE);
}

static void call_with_ice_video_and_rtt(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	LinphoneCallParams *params = NULL;
	LinphoneCall *marie_call = NULL;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, FALSE);
	linphone_core_enable_video_capture(pauline->lc, FALSE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);

	linphone_core_manager_wait_for_stun_resolution(marie);
	linphone_core_manager_wait_for_stun_resolution(pauline);

	linphone_core_set_audio_port(marie->lc, -1);
	linphone_core_set_video_port(marie->lc, -1);
	linphone_core_set_text_port(marie->lc, -1);
	linphone_core_set_audio_port(pauline->lc, -1);
	linphone_core_set_video_port(pauline->lc, -1);
	linphone_core_set_text_port(pauline->lc, -1);

	params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_realtime_text(params, TRUE);
	BC_ASSERT_TRUE(call_ok = call_with_caller_params(pauline, marie, params));
	if (!call_ok) goto end;
	BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));

	marie_call = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call)) goto end;
	BC_ASSERT_TRUE(linphone_call_params_audio_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(marie_call)));

	end_call(pauline, marie);
end:
	linphone_call_params_unref(params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_video_only(void) {
	/*
	 * Scenario: video is active at the beginning of the call, but no audio codecs match.
	 */
	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, TRUE);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);

	_call_with_ice_video(caller_vpol, callee_vpol, FALSE, FALSE, FALSE, FALSE, TRUE);
}

static void video_call_with_early_media_no_matching_audio_codecs(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call, *pauline_call;
	AudioStream *astream;
	VideoStream *caller_vstream;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	LinphonePayloadType *pt_pcmu = linphone_core_get_payload_type(marie->lc, "PCMU", 8000, 1);
	linphone_payload_type_enable(pt_pcmu, FALSE); /* Disable PCMU */
	LinphonePayloadType *pt_pcma = linphone_core_get_payload_type(marie->lc, "PCMA", 8000, 1);
	linphone_payload_type_enable(pt_pcma, TRUE); /* Enable PCMA */

	linphone_payload_type_unref(pt_pcmu);
	linphone_payload_type_unref(pt_pcma);

	out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!pauline_call) goto end;

	linphone_call_accept_early_media(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));
	/*audio stream shall not have been requested to start*/
	astream = (AudioStream *)linphone_call_get_stream(pauline_call, LinphoneStreamTypeAudio);
	BC_ASSERT_PTR_NULL(astream->soundread);

	/* assert that the caller does not sends camera, since it did not requested with
	 * linphone_call_params_enable_early_media_sending(): */
	caller_vstream = (VideoStream *)linphone_call_get_stream(out_call, LinphoneStreamTypeVideo);
	if (BC_ASSERT_PTR_NOT_NULL(caller_vstream->source))
		BC_ASSERT_NOT_EQUAL(caller_vstream->source->desc->id, MS_MIRE_ID, int, "%d");

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(out_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));

	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	end_call(marie, pauline);

end:
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_limited_bandwidth(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_download_bandwidth(pauline->lc, 100);
	video_call_base(marie, pauline, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void dtls_srtp_video_call(void) {
	call_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE, LinphonePolicyNoFirewall, FALSE);
}

static void dtls_srtp_ice_video_call(void) {
	call_base(LinphoneMediaEncryptionDTLS, TRUE, FALSE, LinphonePolicyUseIce, FALSE);
}
static void dtls_srtp_ice_video_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionDTLS, TRUE, TRUE, LinphonePolicyUseIce, FALSE);
}
static void srtp_video_ice_call(void) {
	call_base(LinphoneMediaEncryptionSRTP, TRUE, FALSE, LinphonePolicyUseIce, FALSE);
}
static void zrtp_video_ice_call(void) {
	call_base(LinphoneMediaEncryptionZRTP, TRUE, FALSE, LinphonePolicyUseIce, FALSE);
}

static void accept_call_in_send_only_base(LinphoneCoreManager *pauline, LinphoneCoreManager *marie, bctbx_list_t *lcs) {
#define DEFAULT_WAIT_FOR 10000
	LinphoneCallParams *params;
	LinphoneCall *call;
	int dummy = 0;

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	LinphonePayloadType *type = linphone_core_get_payload_type(pauline->lc, "h264", -1, -1);
	if (type != NULL) {
		linphone_payload_type_unref(type);
		disable_all_video_codecs_except_one(pauline->lc, "h264");
		disable_all_video_codecs_except_one(marie->lc, "h264");
	}
	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	/*The send-only client shall set rtp symmetric in absence of media relay for this test.*/
	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "symmetric", 1);

	liblinphone_tester_set_next_video_frame_decoded_cb(linphone_core_invite_address(pauline->lc, marie->identity));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, DEFAULT_WAIT_FOR));

	char *remote_uri = linphone_address_as_string_uri_only(pauline->identity);
	call = linphone_core_find_call_from_uri(marie->lc, remote_uri);

	stats initial_marie_stats = marie->stat;
	stats initial_pauline_stats = pauline->stat;

	if (call) {
		call = linphone_call_ref(call);
		params = linphone_core_create_call_params(marie->lc, NULL);
		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendOnly);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendOnly);
		linphone_call_accept_with_params(call, params);
		linphone_call_params_unref(params);

		BC_ASSERT_PTR_NOT_NULL(linphone_core_find_call_from_uri(marie->lc, remote_uri));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, DEFAULT_WAIT_FOR));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1, DEFAULT_WAIT_FOR));
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallError, initial_marie_stats.number_of_LinphoneCallError, int,
		                "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd, initial_marie_stats.number_of_LinphoneCallEnd, int,
		                "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallReleased, initial_marie_stats.number_of_LinphoneCallReleased,
		                int, "%d");

		// if the call is ended, released or in error state, the CPP pointer may have freed and the C pointer became
		// dangling
		if ((marie->stat.number_of_LinphoneCallEnd == initial_marie_stats.number_of_LinphoneCallEnd) &&
		    (marie->stat.number_of_LinphoneCallReleased == initial_marie_stats.number_of_LinphoneCallReleased) &&
		    (marie->stat.number_of_LinphoneCallError == initial_marie_stats.number_of_LinphoneCallError)) {
			check_media_direction(marie, call, lcs, LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionSendOnly);
			float quality = linphone_call_get_current_quality(call);
			BC_ASSERT_GREATER(quality, 1.0, float, "%f");
			wait_for_until(marie->lc, pauline->lc, &dummy, 1, 3000);
			quality = linphone_call_get_current_quality(call);
			BC_ASSERT_GREATER(quality, 1.0, float, "%f");
		}
		linphone_call_unref(call);
	}

	call = linphone_core_get_current_call(pauline->lc);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallError, initial_pauline_stats.number_of_LinphoneCallError, int,
	                "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd, initial_pauline_stats.number_of_LinphoneCallEnd, int,
	                "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallReleased, initial_pauline_stats.number_of_LinphoneCallReleased,
	                int, "%d");

	if (call) {
		call = linphone_call_ref(call);
		check_media_direction(pauline, call, lcs, LinphoneMediaDirectionRecvOnly, LinphoneMediaDirectionRecvOnly);

		float quality = linphone_call_get_current_quality(call);
		BC_ASSERT_GREATER(quality, 1.0, float, "%f");
		wait_for_until(marie->lc, pauline->lc, &dummy, 1, 3000);
		quality = linphone_call_get_current_quality(call);
		BC_ASSERT_GREATER(quality, 1.0, float, "%f");
		linphone_call_unref(call);
	}

	ms_free(remote_uri);
}
static void accept_call_in_send_base(bool_t caller_has_ice) {
	LinphoneCoreManager *pauline, *marie;
	bctbx_list_t *lcs = NULL;
	;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (caller_has_ice) {
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	}

	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, marie->lc);

	accept_call_in_send_only_base(pauline, marie, lcs);

	end_call(marie, pauline);
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void accept_call_in_send_only(void) {
	accept_call_in_send_base(FALSE);
}

static void accept_call_in_send_only_with_ice(void) {
	accept_call_in_send_base(TRUE);
}

// The goal of this test is to verify that 2 calls with streams in send only can be taken at the same time
static void two_accepted_call_in_send_only(void) {
	LinphoneCoreManager *pauline, *marie, *laure;
	bctbx_list_t *lcs = NULL;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_media_resource_mode(marie->lc, LinphoneSharedMediaResources);
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	laure = linphone_core_manager_new("laure_tcp_rc");

	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	accept_call_in_send_only_base(pauline, marie, lcs);

	reset_counters(&marie->stat);
	accept_call_in_send_only_base(laure, marie, lcs);

	end_call(pauline, marie);
	end_call(laure, marie);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}

/*this is call forking with early media managed at client side (not by flexisip server)*/
static void multiple_early_media(void) {
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *marie1 = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_new("marie_early_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCallParams *params = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCall *marie1_call;
	LinphoneCall *marie2_call;
	LinphoneCall *pauline_call;
	LinphoneInfoMessage *info;
	int dummy = 0;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(marie1->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(marie2->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie1->lc, vpol);
	linphone_core_set_video_activation_policy(marie2->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(marie1->lc, TRUE);
	linphone_core_enable_video_display(marie1->lc, TRUE);

	linphone_core_enable_video_capture(marie2->lc, TRUE);
	linphone_core_enable_video_display(marie2->lc, TRUE);

	lcs = bctbx_list_append(lcs, marie1->lc);
	lcs = bctbx_list_append(lcs, marie2->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	linphone_call_params_enable_early_media_sending(params, TRUE);
	linphone_call_params_enable_video(params, TRUE);

	linphone_core_invite_address_with_params(pauline->lc, marie1->identity, params);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 3000));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie1_call = linphone_core_get_current_call(marie1->lc);
	marie2_call = linphone_core_get_current_call(marie2->lc);

	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(marie1_call);
	BC_ASSERT_PTR_NOT_NULL(marie2_call);

	if (pauline_call && marie1_call && marie2_call) {

		/*wait a bit that streams are established*/
		wait_for_list(lcs, &dummy, 1, 6000);
		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(pauline), 70, int, "%i");
		BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie1), 70, int, "%i");
		BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie2), 70, int, "%i");

		linphone_call_accept(linphone_core_get_current_call(marie1->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));

		/*marie2 should get her call terminated*/
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallEnd, 1, 1000));

		/*wait a bit that streams are established*/
		wait_for_list(lcs, &dummy, 1, 3000);
		BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(pauline), 71, int, "%i");
		BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie1), 71, int, "%i");

		/*send an INFO in reverse side to check that dialogs are properly established*/
		info = linphone_core_create_info_message(marie1->lc);
		linphone_call_send_info_message(marie1_call, info);
		linphone_info_message_unref(info);
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_InfoReceived, 1, 3000));
	}

	end_call(pauline, marie1);

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
}
static void video_call_ice_params(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	video_call_base(marie, pauline, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void audio_call_with_ice_with_video_policy_enabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
	}
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *marie_vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(marie_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(marie_vpol, FALSE);
	linphone_core_set_video_activation_policy(marie->lc, marie_vpol);
	linphone_video_activation_policy_unref(marie_vpol);

	LinphoneVideoActivationPolicy *pauline_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(pauline_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(pauline_vpol, FALSE);
	linphone_core_set_video_activation_policy(pauline->lc, pauline_vpol);
	linphone_video_activation_policy_unref(pauline_vpol);

	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);

	linphone_core_invite_address(pauline->lc, marie->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1)))
		goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc))) goto end;
	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	/*
	LinphoneCallParams *params;
	params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));
	linphone_call_params_enable_video(params, TRUE);
	linphone_core_accept_call_with_params(marie->lc, linphone_core_get_current_call(marie->lc), params);
	linphone_call_params_unref(params);*/

	/*wait for call to be established and ICE reINVITEs to be done */
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	linphone_call_pause(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

	end_call(marie, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void classic_video_entry_phone_setup(LinphoneMediaDirection callee_video_direction) {
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *early_media_params = NULL;
	LinphoneCallParams *in_call_params = NULL;
	LinphoneCallParams *caller_params;
	LinphoneCall *callee_call = NULL, *caller_call = NULL;
	LinphoneVideoActivationPolicy *vpol;
	bctbx_list_t *lcs = NULL;
	char *video_recording_file = bc_tester_file((generateRandomFilename("video_entry_phone_record_") + ".mkv").c_str());
	bool_t ok;
	VideoStream *caller_vstream;
	VideoStream *callee_vstream;

	vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);

	lcs = bctbx_list_append(lcs, caller_mgr->lc);
	lcs = bctbx_list_append(lcs, callee_mgr->lc);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(callee_mgr->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(caller_mgr->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(caller_mgr->lc, TRUE);
	linphone_core_enable_video_display(caller_mgr->lc, TRUE);
	linphone_core_enable_video_capture(callee_mgr->lc, TRUE);
	linphone_core_enable_video_display(callee_mgr->lc, TRUE);
	linphone_core_set_avpf_mode(caller_mgr->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(callee_mgr->lc, LinphoneAVPFEnabled);
	linphone_core_set_video_activation_policy(caller_mgr->lc, vpol);
	linphone_core_set_video_activation_policy(callee_mgr->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	remove(video_recording_file);

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	LinphonePayloadType *type = linphone_core_get_payload_type(caller_mgr->lc, "h264", -1, -1);
	if (type != NULL) {
		linphone_payload_type_unref(type);
		disable_all_video_codecs_except_one(caller_mgr->lc, "h264");
		disable_all_video_codecs_except_one(callee_mgr->lc, "h264");

		/*On Mac OS, set VGA as the prefered size, otherwise we don't benefit from the hardware
		 * accelerated H264 videotoolbox codec*/
		if (ms_factory_get_encoder(linphone_core_get_ms_factory(callee_mgr->lc), "H264")->id == MS_VT_H264_ENC_ID) {
			MSVideoSize vsize = MS_VIDEO_SIZE_VGA;
			linphone_core_set_preferred_video_size(callee_mgr->lc, vsize);
			linphone_core_set_preferred_video_size(caller_mgr->lc, vsize);
			linphone_core_set_download_bandwidth(callee_mgr->lc, 512);
			linphone_core_set_download_bandwidth(caller_mgr->lc, 512);
			linphone_core_set_upload_bandwidth(callee_mgr->lc, 512);
			linphone_core_set_upload_bandwidth(caller_mgr->lc, 512);
		}
	}

	linphone_core_set_video_device(caller_mgr->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(callee_mgr->lc, liblinphone_tester_mire_id);

	caller_params = linphone_core_create_call_params(caller_mgr->lc, NULL);
	linphone_call_params_enable_early_media_sending(caller_params, TRUE);

	caller_call = linphone_core_invite_address_with_params(caller_mgr->lc, callee_mgr->identity, caller_params);
	linphone_call_params_unref(caller_params);
	BC_ASSERT_PTR_NOT_NULL(caller_call);

	ok = wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1);
	BC_ASSERT_TRUE(ok);
	if (!ok) goto end;
	BC_ASSERT_TRUE(caller_mgr->stat.number_of_LinphoneCallOutgoingProgress == 1);

	callee_call = linphone_core_get_call_by_remote_address2(callee_mgr->lc, caller_mgr->identity);
	early_media_params = linphone_core_create_call_params(callee_mgr->lc, callee_call);
	linphone_call_params_set_audio_direction(early_media_params, LinphoneMediaDirectionInactive);
	linphone_call_params_set_video_direction(early_media_params, callee_video_direction);
	linphone_call_params_set_record_file(early_media_params, video_recording_file);
	linphone_call_accept_early_media_with_params(callee_call, early_media_params);
	linphone_call_start_recording(callee_call);
	linphone_call_params_unref(early_media_params);

	BC_ASSERT_TRUE(
	    wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));
	BC_ASSERT_TRUE(
	    wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));

	/* assert that the caller really sends camera, and that callee does not send anything */
	caller_vstream = (VideoStream *)linphone_call_get_stream(caller_call, LinphoneStreamTypeVideo);
	callee_vstream = (VideoStream *)linphone_call_get_stream(callee_call, LinphoneStreamTypeVideo);
	if (BC_ASSERT_PTR_NOT_NULL(caller_vstream->source))
		BC_ASSERT_EQUAL(caller_vstream->source->desc->id, MS_MIRE_ID, int, "%d");

	if (callee_video_direction == LinphoneMediaDirectionSendRecv) {
		if (BC_ASSERT_PTR_NOT_NULL(callee_vstream->source))
			BC_ASSERT_NOT_EQUAL(callee_vstream->source->desc->id, MS_MIRE_ID, int, "%d");
	} else if (callee_video_direction == LinphoneMediaDirectionRecvOnly) {
		BC_ASSERT_PTR_NULL(callee_vstream->source);
	}

	check_media_direction(callee_mgr, callee_call, lcs, LinphoneMediaDirectionInactive, callee_video_direction);
	callee_call = linphone_core_get_call_by_remote_address2(callee_mgr->lc, caller_mgr->identity);
	in_call_params = linphone_core_create_call_params(callee_mgr->lc, callee_call);
	linphone_call_params_set_audio_direction(in_call_params, LinphoneMediaDirectionSendRecv);
	linphone_call_params_set_video_direction(in_call_params, callee_video_direction);
	linphone_call_accept_with_params(callee_call, in_call_params);
	linphone_call_params_unref(in_call_params);

	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallConnected, 1));

	ok =
	    wait_for_until(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
	                   5000) &&
	    wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallStreamsRunning, 1, 5000);
	BC_ASSERT_TRUE(ok);
	if (!ok) goto end;
	check_media_direction(callee_mgr, callee_call, lcs, LinphoneMediaDirectionSendRecv, callee_video_direction);

	caller_vstream = (VideoStream *)linphone_call_get_stream(caller_call, LinphoneStreamTypeVideo);
	callee_vstream = (VideoStream *)linphone_call_get_stream(callee_call, LinphoneStreamTypeVideo);
	if (BC_ASSERT_PTR_NOT_NULL(caller_vstream->source))
		BC_ASSERT_EQUAL(caller_vstream->source->desc->id, MS_MIRE_ID, int, "%d");

	if (callee_video_direction == LinphoneMediaDirectionSendRecv) {
		if (BC_ASSERT_PTR_NOT_NULL(callee_vstream->source))
			BC_ASSERT_EQUAL(callee_vstream->source->desc->id, MS_MIRE_ID, int, "%d");
	} else if (callee_video_direction == LinphoneMediaDirectionRecvOnly) {
		BC_ASSERT_PTR_NULL(callee_vstream->source);
	}

	callee_call = linphone_core_get_current_call(callee_mgr->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(callee_call)) goto end;
	in_call_params = linphone_core_create_call_params(callee_mgr->lc, callee_call);
	linphone_call_params_set_audio_direction(in_call_params, LinphoneMediaDirectionRecvOnly);
	linphone_call_params_set_video_direction(in_call_params, LinphoneMediaDirectionSendOnly);
	linphone_call_update(callee_call, in_call_params);
	linphone_call_params_unref(in_call_params);

	ok =
	    wait_for_until(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
	                   2000) &&
	    wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallStreamsRunning, 2, 2000);
	BC_ASSERT_TRUE(ok);
	if (!ok) goto end;
	callee_call = linphone_core_get_current_call(callee_mgr->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(callee_call)) goto end;
	check_media_direction(callee_mgr, callee_call, lcs, LinphoneMediaDirectionRecvOnly, LinphoneMediaDirectionSendOnly);

	end_call(caller_mgr, callee_mgr);

end:
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
	bctbx_list_free(lcs);
	bc_free(video_recording_file);
}

static void classic_video_entry_phone_setup_sendrecv(void) {
	classic_video_entry_phone_setup(LinphoneMediaDirectionSendRecv);
}

static void classic_video_entry_phone_setup_recvonly(void) {
	classic_video_entry_phone_setup(LinphoneMediaDirectionRecvOnly);
}

static void video_call_recording_h264_test(void) {
	record_call(generateRandomFilename("recording_").c_str(), TRUE, "H264");
}

static void video_call_recording_vp8_test(void) {
	record_call(generateRandomFilename("recording_").c_str(), TRUE, "VP8");
}

static void snapshot_taken(LinphoneCall *call, const char *filepath) {
	// This is a check on file name. It must not by dynamic : based from filter should be enough
	char *filename = bc_tester_file((g_display_filter + "snapshot.jpeg").c_str());
	LinphoneCore *lc = linphone_call_get_core(call);
	stats *callstats = get_stats(lc);
	BC_ASSERT_STRING_EQUAL(filepath, filename);
	callstats->number_of_snapshot_taken++;
	ms_free(filename);
}

static void video_call_snapshot(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *marieParams = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *paulineParams = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCall *callInst = NULL;
	// This is a check on file name. It must not by dynamic : based from filter should be enough
	char *filename = bc_tester_file((g_display_filter + "snapshot.jpeg").c_str());
	bool_t call_succeeded = FALSE;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);
	linphone_call_params_enable_video(marieParams, TRUE);
	linphone_call_params_enable_video(paulineParams, TRUE);

	BC_ASSERT_TRUE(call_succeeded = call_with_params(marie, pauline, marieParams, paulineParams));
	BC_ASSERT_PTR_NOT_NULL(callInst = linphone_core_get_current_call(marie->lc));
	if (call_succeeded == TRUE && callInst != NULL) {
		LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
		LinphoneCallCbs *marie_call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
		BC_ASSERT_PTR_NOT_NULL(marie_call);
		linphone_call_cbs_set_snapshot_taken(marie_call_cbs, snapshot_taken);
		linphone_call_add_callbacks(marie_call, marie_call_cbs);
		linphone_call_cbs_unref(marie_call_cbs);
		int jpeg_support = linphone_call_take_video_snapshot(callInst, filename);
		if (jpeg_support < 0) {
			ms_warning("No jpegwriter support!");
		} else {
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_snapshot_taken, 1));
			BC_ASSERT_EQUAL(bctbx_file_exist(filename), 0, int, "%d");
			remove(filename);
		}
		end_call(marie, pauline);
	}
	ms_free(filename);
	linphone_call_params_unref(marieParams);
	linphone_call_params_unref(paulineParams);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_snapshots(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *marieParams = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *paulineParams = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCall *callInst = NULL;
	// This is a check on file name. It must not by dynamic : based from filter should be enough
	char *filename = bc_tester_file((g_display_filter + "snapshot.jpeg").c_str());
	bool_t call_succeeded = FALSE;
	int dummy = 0;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);
	linphone_call_params_enable_video(marieParams, TRUE);
	linphone_call_params_enable_video(paulineParams, TRUE);

	BC_ASSERT_TRUE(call_succeeded = call_with_params(marie, pauline, marieParams, paulineParams));
	BC_ASSERT_PTR_NOT_NULL(callInst = linphone_core_get_current_call(marie->lc));
	if (call_succeeded == TRUE && callInst != NULL) {
		LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
		LinphoneCallCbs *marie_call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
		BC_ASSERT_PTR_NOT_NULL(marie_call);
		linphone_call_cbs_set_snapshot_taken(marie_call_cbs, snapshot_taken);
		linphone_call_add_callbacks(marie_call, marie_call_cbs);
		linphone_call_cbs_unref(marie_call_cbs);
		int jpeg_support = linphone_call_take_video_snapshot(callInst, filename);
		if (jpeg_support < 0) {
			ms_warning("No jpegwriter support!");
		} else {
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_snapshot_taken, 1));
			BC_ASSERT_EQUAL(bctbx_file_exist(filename), 0, int, "%d");
			remove(filename);

			wait_for_until(marie->lc, pauline->lc, &dummy, 1, 1000);
			linphone_call_take_video_snapshot(callInst, filename);
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_snapshot_taken, 2));
			BC_ASSERT_EQUAL(bctbx_file_exist(filename), 0, int, "%d");
			remove(filename);

			wait_for_until(marie->lc, pauline->lc, &dummy, 1, 1000);
			linphone_call_take_video_snapshot(callInst, filename);
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_snapshot_taken, 3));
			BC_ASSERT_EQUAL(bctbx_file_exist(filename), 0, int, "%d");
			remove(filename);
		}
		end_call(marie, pauline);
	}
	ms_free(filename);
	linphone_call_params_unref(marieParams);
	linphone_call_params_unref(paulineParams);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryption mode, bool_t no_sdp) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCallParams *params;
	const LinphoneCallParams *current_params;
	bctbx_list_t *lcs = NULL;
	bool_t calls_ok;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	LinphonePayloadType *type = linphone_core_get_payload_type(pauline->lc, "h264", -1, -1);
	if (type != NULL) {
		linphone_payload_type_unref(type);
		disable_all_video_codecs_except_one(pauline->lc, "h264");
		disable_all_video_codecs_except_one(marie->lc, "h264");
	}
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, marie->lc);

	video_call_base_2(marie, pauline, TRUE, mode, TRUE, TRUE);

	calls_ok = linphone_core_get_current_call(marie->lc) != NULL && linphone_core_get_current_call(pauline->lc) != NULL;
	BC_ASSERT_TRUE(calls_ok);

	if (calls_ok) {
		params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));
		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionInactive);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionInactive);

		linphone_call_update(linphone_core_get_current_call(marie->lc), params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));

		check_media_direction(marie, linphone_core_get_current_call(marie->lc), lcs, LinphoneMediaDirectionInactive,
		                      LinphoneMediaDirectionInactive);
		check_media_direction(pauline, linphone_core_get_current_call(pauline->lc), lcs, LinphoneMediaDirectionInactive,
		                      LinphoneMediaDirectionInactive);

		const LinphoneCallParams *remote_params =
		    linphone_call_get_remote_params(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_EQUAL(linphone_call_params_get_audio_direction(remote_params), LinphoneMediaDirectionInactive, int,
		                "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(remote_params), LinphoneMediaDirectionInactive, int,
		                "%d");

		if (no_sdp) {
			linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
		}

		params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));
		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendRecv);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendRecv);
		linphone_call_update(linphone_core_get_current_call(marie->lc), params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

		check_media_direction(marie, linphone_core_get_current_call(marie->lc), lcs, LinphoneMediaDirectionSendRecv,
		                      LinphoneMediaDirectionSendRecv);
		check_media_direction(pauline, linphone_core_get_current_call(pauline->lc), lcs, LinphoneMediaDirectionSendRecv,
		                      LinphoneMediaDirectionSendRecv);

		remote_params = linphone_call_get_remote_params(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_EQUAL(linphone_call_params_get_audio_direction(remote_params), LinphoneMediaDirectionSendRecv, int,
		                "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(remote_params), LinphoneMediaDirectionSendRecv, int,
		                "%d");

		/*assert that after pause and resume, SRTP is still being used*/
		current_params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(current_params), mode, int, "%d");
		current_params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(current_params), mode, int, "%d");
	}
	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void video_call_with_re_invite_inactive_followed_by_re_invite(void) {
	video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionNone, FALSE);
}

static void video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp(void) {
	video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionNone, TRUE);
}

static void srtp_video_call_with_re_invite_inactive_followed_by_re_invite(void) {
	if (ms_srtp_supported())
		video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionSRTP, FALSE);
	else ms_message("srtp_video_call_with_re_invite_inactive_followed_by_re_invite skipped, missing srtp support");
}

static void srtp_video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp(void) {
	if (ms_srtp_supported())
		video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionSRTP, TRUE);
	else
		ms_message(
		    "srtp_video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp skipped, missing srtp support");
}

static void incoming_reinvite_with_invalid_ack_sdp(void) {
	LinphoneCoreManager *caller = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCall *inc_call;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
	}
	BC_ASSERT_TRUE(call(caller, callee));
	inc_call = linphone_core_get_current_call(callee->lc);

	BC_ASSERT_PTR_NOT_NULL(inc_call);
	if (inc_call) {
		const LinphoneCallParams *caller_params;
		stats initial_caller_stat = caller->stat;
		stats initial_callee_stat = callee->stat;
		/* will force a parse error for the ACK SDP*/
		LinphonePrivate::SalCallOp *op = LinphonePrivate::Call::toCpp(inc_call)->getOp();
		op->setSdpHandling(SalOpSDPSimulateError);
		BC_ASSERT_PTR_NOT_NULL(_request_video(caller, callee, TRUE));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallUpdating,
		                        initial_callee_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_caller_stat.number_of_LinphoneCallStreamsRunning));
		/*Basically the negotiation failed but since the call was already running, we expect it to restore to
		the previous state so error stats should not be changed*/
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallError, initial_callee_stat.number_of_LinphoneCallError, int,
		                "%d");
		/*and remote should have received an update notification*/
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallUpdatedByRemote,
		                initial_caller_stat.number_of_LinphoneCallUpdatedByRemote + 1, int, "%d");

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(
		    linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
		caller_params = linphone_call_get_current_params(linphone_core_get_current_call(caller->lc));
		// TODO [refactoring]: BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,(int*)&caller_params->has_video,FALSE));
		(void)caller_params;
		op->setSdpHandling(SalOpSDPNormal);
	}
	end_call(caller, callee);

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void outgoing_reinvite_with_invalid_ack_sdp(void) {
	LinphoneCoreManager *caller = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCall *out_call;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
	}

	BC_ASSERT_TRUE(call(caller, callee));
	out_call = linphone_core_get_current_call(caller->lc);

	BC_ASSERT_PTR_NOT_NULL(out_call);
	if (out_call) {
		stats initial_caller_stat = caller->stat;
		stats initial_callee_stat = callee->stat;
		/* will force a parse error for the ACK SDP*/
		LinphonePrivate::SalCallOp *op = LinphonePrivate::Call::toCpp(out_call)->getOp();
		op->setSdpHandling(SalOpSDPSimulateError);
		BC_ASSERT_PTR_NOT_NULL(_request_video(caller, callee, TRUE));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallUpdating,
		                        initial_callee_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_caller_stat.number_of_LinphoneCallStreamsRunning));
		/*Basically the negotiation failed but since the call was already running, we expect it to restore to
		the previous state so error stats should not be changed*/
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallError, initial_callee_stat.number_of_LinphoneCallError, int,
		                "%d");
		/*and remote should not have received any update notification*/
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallUpdatedByRemote,
		                initial_caller_stat.number_of_LinphoneCallUpdatedByRemote, int, "%d");

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(
		    linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(
		    linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));

		op->setSdpHandling(SalOpSDPNormal);
	}
	end_call(caller, callee);

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_with_no_audio_and_no_video_codec(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call;
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
	}

	const bctbx_list_t *elem_video = linphone_core_get_video_codecs(caller->lc);

	const bctbx_list_t *elem_audio = linphone_core_get_audio_codecs(caller->lc);

	disable_all_codecs(elem_audio, caller);
	disable_all_codecs(elem_video, caller);

	LinphoneVideoActivationPolicy *callee_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(callee_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(callee_vpol, TRUE);
	linphone_core_set_video_activation_policy(callee->lc, callee_vpol);
	linphone_video_activation_policy_unref(callee_vpol);

	LinphoneVideoActivationPolicy *caller_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(caller_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(caller_vpol, FALSE);
	linphone_core_set_video_activation_policy(caller->lc, caller_vpol);
	linphone_video_activation_policy_unref(caller_vpol);

	linphone_core_enable_video_display(callee->lc, TRUE);
	linphone_core_enable_video_capture(callee->lc, TRUE);

	linphone_core_enable_video_display(caller->lc, TRUE);
	linphone_core_enable_video_capture(caller->lc, TRUE);

	/* Create call params */
	caller_test_params.base = linphone_core_create_call_params(caller->lc, NULL);

	out_call = linphone_core_invite_address_with_params(caller->lc, callee->identity, caller_test_params.base);
	linphone_call_ref(out_call);

	linphone_call_params_unref(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_unref(callee_test_params.base);

	BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallOutgoingInit, 1));

	BC_ASSERT_TRUE(wait_for_until(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void video_call_with_auto_video_accept_disabled_on_one_end(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	LinphoneVideoActivationPolicy *marie_vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(marie_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(marie_vpol, FALSE);
	linphone_core_set_video_activation_policy(marie->lc, marie_vpol);
	linphone_video_activation_policy_unref(marie_vpol);

	LinphoneVideoActivationPolicy *pauline_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(pauline_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(pauline_vpol, FALSE);
	linphone_core_set_video_activation_policy(pauline->lc, pauline_vpol);
	linphone_video_activation_policy_unref(pauline_vpol);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	// Marie calls Pauline
	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (pauline_call && marie_call) {
		LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
		linphone_call_params_enable_video(pauline_call_params, TRUE);
		linphone_call_update(pauline_call, pauline_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

		const LinphoneCallParams *updated_pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(updated_pauline_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(updated_pauline_call_params) ==
		               linphone_call_params_video_enabled(pauline_call_params));

		const LinphoneCallParams *marie_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_PTR_NOT_NULL(marie_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_params) ==
		               linphone_call_params_video_enabled(pauline_call_params));

		linphone_call_params_unref(pauline_call_params);
	}

	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

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

static void asymmetrical_video_call(bool_t with_call_params, bool_t with_flexfec) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	// enable flexfec
	if (with_flexfec) {
		OrtpNetworkSimulatorParams network_params = {0};
		network_params.enabled = TRUE;
		network_params.loss_rate = 8.f;
		network_params.mode = OrtpNetworkSimulatorOutbound;
		linphone_core_set_network_simulator_params(marie->lc, &network_params);
		linphone_core_set_network_simulator_params(pauline->lc, &network_params);
		enable_rtp_bundle(marie->lc, TRUE);
		enable_rtp_bundle(pauline->lc, TRUE);
		linphone_core_enable_fec(marie->lc, TRUE);
		linphone_core_enable_fec(pauline->lc, TRUE);
		disable_all_video_codecs_except_one(marie->lc, "VP8");
		disable_all_video_codecs_except_one(pauline->lc, "VP8");
		linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
		linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
	}

	// asymmetrical video policy
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, FALSE);
	vpol->accept_media_direction = LinphoneMediaDirectionRecvOnly;
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	// Marie calls Pauline
	if (with_call_params) {
		// Pauline enables her video
		linphone_call_params_enable_video(pauline_call_params, TRUE);
		BC_ASSERT_TRUE(call_with_params(pauline, marie, pauline_call_params, NULL));
	} else {
		BC_ASSERT_TRUE(call(pauline, marie));
	}

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (with_flexfec) {
		BC_ASSERT_TRUE(linphone_call_params_fec_enabled(linphone_call_get_current_params(pauline_call)));
		BC_ASSERT_TRUE(linphone_call_params_fec_enabled(linphone_call_get_current_params(marie_call)));
	}

	if (pauline_call && marie_call) {
		if (!with_call_params) {
			linphone_call_params_enable_video(pauline_call_params, TRUE);
			linphone_call_update(pauline_call, pauline_call_params);

			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		}

		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

		const LinphoneCallParams *updated_pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(updated_pauline_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(updated_pauline_call_params) ==
		               linphone_call_params_video_enabled(pauline_call_params));

		const LinphoneCallParams *marie_call_current_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_PTR_NOT_NULL(marie_call_current_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_current_params) ==
		               linphone_call_params_video_enabled(pauline_call_params));

		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(marie_call_current_params),
		                LinphoneMediaDirectionRecvOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_params(marie_call)),
		                LinphoneMediaDirectionRecvOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(updated_pauline_call_params),
		                LinphoneMediaDirectionSendOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_remote_params(pauline_call)),
		                LinphoneMediaDirectionRecvOnly, int, "%d");

		stats marie_stats = marie->stat;
		stats pauline_stats = pauline->stat;

		// Marie enables her video
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);

		LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, marie_call);
		linphone_call_params_set_video_direction(marie_call_params, LinphoneMediaDirectionSendRecv);
		linphone_call_update(marie_call, marie_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
		                        pauline_stats.number_of_LinphoneCallUpdatedByRemote + 1));

		linphone_call_params_unref(pauline_call_params);
		pauline_call_params = linphone_core_create_call_params(pauline->lc, pauline_call);
		linphone_call_accept_update(pauline_call, pauline_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating,
		                        marie_stats.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        marie_stats.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        pauline_stats.number_of_LinphoneCallStreamsRunning + 1));

		marie_call_current_params = linphone_call_get_current_params(marie_call);
		updated_pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(marie_call_current_params),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_params(marie_call)),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(updated_pauline_call_params),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_remote_params(pauline_call)),
		                LinphoneMediaDirectionSendRecv, int, "%d");

		if (with_flexfec) {
			VideoStream *vstream_marie = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);
			fec_stats *fec_stats_marie = fec_stream_get_stats(vstream_marie->ms.fec_stream);
			VideoStream *vstream_pauline =
			    (VideoStream *)linphone_call_get_stream(pauline_call, LinphoneStreamTypeVideo);
			fec_stats *fec_stats_pauline = fec_stream_get_stats(vstream_pauline->ms.fec_stream);
			BC_ASSERT_TRUE(
			    wait_for_until_for_uint64(marie->lc, pauline->lc, &fec_stats_marie->packets_recovered, 3, 45000));
			BC_ASSERT_TRUE(
			    wait_for_until_for_uint64(marie->lc, pauline->lc, &fec_stats_pauline->packets_recovered, 3, 45000));
			LinphoneCallStats *marie_call_stats = linphone_call_get_video_stats(marie_call);
			LinphoneCallStats *pauline_call_stats = linphone_call_get_video_stats(pauline_call);
			int marie_repaired_packets =
			    static_cast<int>(linphone_call_stats_get_fec_repaired_packets_number(marie_call_stats));
			int pauline_repaired_packets =
			    static_cast<int>(linphone_call_stats_get_fec_repaired_packets_number(pauline_call_stats));
			BC_ASSERT_GREATER_STRICT(marie_repaired_packets, 0, int, "%d");
			BC_ASSERT_GREATER_STRICT(pauline_repaired_packets, 0, int, "%d");
			if (marie_call_stats) linphone_call_stats_unref(marie_call_stats);
			if (pauline_call_stats) linphone_call_stats_unref(pauline_call_stats);
		}

		linphone_call_params_unref(marie_call_params);
		linphone_call_params_unref(pauline_call_params);
	}

	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void asymmetrical_video_call_with_callee_enabled_video_first() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	// asymmetrical video policy
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, FALSE);
	vpol->accept_media_direction = LinphoneMediaDirectionRecvOnly;
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	// Pauline calls Marie
	BC_ASSERT_TRUE(call(pauline, marie));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (pauline_call && marie_call) {
		linphone_call_params_enable_video(marie_call_params, TRUE);
		linphone_call_update(marie_call, marie_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

		const LinphoneCallParams *updated_marie_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_PTR_NOT_NULL(updated_marie_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(updated_marie_call_params) ==
		               linphone_call_params_video_enabled(marie_call_params));

		const LinphoneCallParams *pauline_call_current_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_call_current_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(pauline_call_current_params) ==
		               linphone_call_params_video_enabled(marie_call_params));

		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(pauline_call_current_params),
		                LinphoneMediaDirectionRecvOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_params(pauline_call)),
		                LinphoneMediaDirectionRecvOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(updated_marie_call_params),
		                LinphoneMediaDirectionSendOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_remote_params(marie_call)),
		                LinphoneMediaDirectionRecvOnly, int, "%d");

		stats marie_stats = marie->stat;
		stats pauline_stats = pauline->stat;

		// Pauline enables her video
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "defer_update_default", TRUE);

		LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, pauline_call);
		linphone_call_params_set_video_direction(pauline_call_params, LinphoneMediaDirectionSendRecv);
		linphone_call_update(pauline_call, pauline_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
		                        marie_stats.number_of_LinphoneCallUpdatedByRemote + 1));

		linphone_call_params_unref(marie_call_params);
		marie_call_params = linphone_core_create_call_params(marie->lc, marie_call);
		linphone_call_accept_update(marie_call, marie_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating,
		                        pauline_stats.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        marie_stats.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        pauline_stats.number_of_LinphoneCallStreamsRunning + 1));

		pauline_call_current_params = linphone_call_get_current_params(pauline_call);
		updated_marie_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(pauline_call_current_params),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_params(pauline_call)),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(updated_marie_call_params),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_remote_params(marie_call)),
		                LinphoneMediaDirectionSendRecv, int, "%d");

		linphone_call_params_unref(marie_call_params);
		linphone_call_params_unref(pauline_call_params);
	}

	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void asymmetrical_video_call_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	// asymmetrical video policy
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, FALSE);
	vpol->accept_media_direction = LinphoneMediaDirectionRecvOnly;
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	// Marie calls Pauline
	BC_ASSERT_TRUE(call_with_params(pauline, marie, pauline_call_params, NULL));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (pauline_call && marie_call) {
		linphone_call_params_enable_video(pauline_call_params, TRUE);
		linphone_call_update(pauline_call, pauline_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

		const LinphoneCallParams *updated_pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(updated_pauline_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(updated_pauline_call_params) ==
		               linphone_call_params_video_enabled(pauline_call_params));

		const LinphoneCallParams *marie_call_current_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_PTR_NOT_NULL(marie_call_current_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_current_params) ==
		               linphone_call_params_video_enabled(pauline_call_params));

		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(marie_call_current_params),
		                LinphoneMediaDirectionRecvOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_params(marie_call)),
		                LinphoneMediaDirectionRecvOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(updated_pauline_call_params),
		                LinphoneMediaDirectionSendOnly, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_remote_params(pauline_call)),
		                LinphoneMediaDirectionRecvOnly, int, "%d");

		stats marie_stats = marie->stat;
		stats pauline_stats = pauline->stat;

		// Marie enables her video
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);

		LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, marie_call);
		linphone_call_params_set_video_direction(marie_call_params, LinphoneMediaDirectionSendRecv);
		linphone_call_update(marie_call, marie_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
		                        pauline_stats.number_of_LinphoneCallUpdatedByRemote + 1));

		linphone_call_accept_update(pauline_call, pauline_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating,
		                        marie_stats.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        marie_stats.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        pauline_stats.number_of_LinphoneCallStreamsRunning + 1));

		marie_call_current_params = linphone_call_get_current_params(marie_call);
		updated_pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(marie_call_current_params),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_params(marie_call)),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(updated_pauline_call_params),
		                LinphoneMediaDirectionSendRecv, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(linphone_call_get_remote_params(pauline_call)),
		                LinphoneMediaDirectionSendRecv, int, "%d");

		linphone_call_params_unref(marie_call_params);
		linphone_call_params_unref(pauline_call_params);
	}

	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void asymmetrical_video_call_starts_with_video() {
	asymmetrical_video_call(true, false);
}

static void asymmetrical_video_call_starts_without_video() {
	asymmetrical_video_call(false, false);
}

static void asymmetrical_video_call_with_flexfec_starts_with_video() {
	asymmetrical_video_call(true, true);
}

static void call_with_early_media_and_no_sdp_in_200_with_video(void) {
	early_media_without_sdp_in_200_base(TRUE, FALSE);
}

static void camera_not_working(LinphoneCall *call, const char *camera_name) {
	LinphoneCore *lc = linphone_call_get_core(call);
	stats *callstats = get_stats(lc);
	BC_ASSERT_STRING_EQUAL(camera_name, "Mire (synthetic moving picture)");
	callstats->number_of_LinphoneCallCameraNotWorking++;
}

static void video_call_with_fallback_to_static_picture_when_no_fps(void) {
	LinphoneCoreManager *caller = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *callee =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	LinphoneCall *callee_call;
	LinphoneCall *caller_call;

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(caller->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(callee->lc, g_display_filter.c_str());
	}

	linphone_core_set_preferred_video_definition_by_name(caller->lc, "QVGA");
	linphone_core_set_preferred_video_definition_by_name(callee->lc, "QVGA");

	linphone_core_set_video_device(caller->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(callee->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_display(callee->lc, TRUE);
	linphone_core_enable_video_capture(callee->lc, TRUE);

	linphone_core_enable_video_display(caller->lc, TRUE);
	linphone_core_enable_video_capture(caller->lc, TRUE);

	linphone_core_set_media_encryption(callee->lc, LinphoneMediaEncryptionNone);
	linphone_core_set_media_encryption(caller->lc, LinphoneMediaEncryptionNone);

	caller_test_params.base = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_video(caller_test_params.base, TRUE);

	callee_test_params.base = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_video(callee_test_params.base, TRUE);

	BC_ASSERT_TRUE(call_with_params2(caller, callee, &caller_test_params, &callee_test_params, FALSE));
	callee_call = linphone_core_get_current_call(callee->lc);
	caller_call = linphone_core_get_current_call(caller->lc);

	linphone_call_params_unref(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_unref(callee_test_params.base);

	if (callee_call && caller_call) {
		LinphoneCallCbs *caller_cbs;
		VideoStream *caller_stream;
		const MSWebCam *camera;

		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(callee_call)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(caller_call)));

		caller_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
		linphone_call_cbs_set_camera_not_working(caller_cbs, camera_not_working);
		linphone_call_add_callbacks(caller_call, caller_cbs);
		linphone_call_cbs_unref(caller_cbs);

		caller_stream = (VideoStream *)linphone_call_get_stream(caller_call, LinphoneStreamTypeVideo);
		// Set the FPS to 0 and keep away further set in order to simulate a defunct camera
		liblinphone_tester_simulate_mire_defunct(caller_stream->source, TRUE, 0);
		BC_ASSERT_TRUE(
		    wait_for_until(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallCameraNotWorking, 1, 10000));
		camera = video_stream_get_camera(caller_stream);
		if (BC_ASSERT_PTR_NOT_NULL(camera)) BC_ASSERT_STRING_EQUAL(ms_web_cam_get_name(camera), "Static picture");
	}
	end_call(caller, callee);

	linphone_core_manager_destroy(caller);
	linphone_core_manager_destroy(callee);
}

static void call_paused_resumed_with_automatic_video_accept(void) {
	call_paused_resumed_base(FALSE, FALSE, TRUE);
}

static void video_call_with_mire_and_analyse(void) {
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *callee_call;
	LinphoneCall *caller_call;

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	vpol->automatically_accept = TRUE;
	linphone_core_set_video_activation_policy(callee->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_set_video_device(callee->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(caller->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_display(callee->lc, TRUE);
	linphone_core_enable_video_capture(callee->lc, TRUE);
	linphone_core_enable_video_display(caller->lc, TRUE);
	linphone_core_enable_video_capture(caller->lc, TRUE);
	linphone_core_set_video_display_filter(callee->lc, "MSAnalyseDisplay");
	linphone_core_set_video_display_filter(caller->lc, "MSAnalyseDisplay");

	LinphoneCallParams *callee_params = linphone_core_create_call_params(callee->lc, NULL);
	linphone_call_params_enable_video(callee_params, TRUE);
	LinphoneCallParams *caller_params = linphone_core_create_call_params(caller->lc, NULL);
	linphone_call_params_enable_video(caller_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(caller, callee, caller_params, callee_params));
	callee_call = linphone_core_get_current_call(callee->lc);
	caller_call = linphone_core_get_current_call(caller->lc);

	linphone_call_params_unref(caller_params);
	linphone_call_params_unref(callee_params);

	BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(callee->lc, caller->lc, &callee->stat.number_of_LinphoneCallStreamsRunning, 1));

	if (callee_call && caller_call) {
		VideoStream *vstream_callee = (VideoStream *)linphone_call_get_stream(callee_call, LinphoneStreamTypeVideo);
		BC_ASSERT_PTR_NOT_NULL(vstream_callee);
		VideoStream *vstream_caller = (VideoStream *)linphone_call_get_stream(caller_call, LinphoneStreamTypeVideo);
		BC_ASSERT_PTR_NOT_NULL(vstream_caller);
		BC_ASSERT_TRUE(vstream_callee && vstream_callee->source &&
		               ms_filter_get_id(vstream_callee->source) == MS_MIRE_ID);
		BC_ASSERT_TRUE(vstream_caller && vstream_caller->source &&
		               ms_filter_get_id(vstream_caller->source) == MS_MIRE_ID);
		MSMireControl c1 = {{0, 5, 10, 15, 20, 25}};
		MSMireControl c2 = {{100, 105, 110, 115, 120, 125}};

		if (vstream_callee && vstream_callee->source && ms_filter_get_id(vstream_callee->source) == MS_MIRE_ID) {
			ms_filter_call_method(vstream_callee->source, MS_MIRE_SET_COLOR, &c1);
		}
		if (vstream_caller && vstream_caller->source && ms_filter_get_id(vstream_caller->source) == MS_MIRE_ID) {
			ms_filter_call_method(vstream_caller->source, MS_MIRE_SET_COLOR, &c2);
		}

		wait_for_until(callee->lc, caller->lc, NULL, 5, 2000);

		BC_ASSERT_TRUE(vstream_callee && vstream_callee->output &&
		               ms_filter_get_id(vstream_callee->output) == MS_ANALYSE_DISPLAY_ID);
		if (vstream_callee && vstream_callee->output &&
		    ms_filter_get_id(vstream_callee->output) == MS_ANALYSE_DISPLAY_ID) {
			BC_ASSERT_TRUE(ms_filter_call_method(vstream_callee->output, MS_ANALYSE_DISPLAY_COMPARE_COLOR, &c2) == 0);
		}

		BC_ASSERT_TRUE(vstream_caller && vstream_caller->output &&
		               ms_filter_get_id(vstream_caller->output) == MS_ANALYSE_DISPLAY_ID);
		if (vstream_caller && vstream_caller->output &&
		    ms_filter_get_id(vstream_caller->output) == MS_ANALYSE_DISPLAY_ID) {
			BC_ASSERT_TRUE(ms_filter_call_method(vstream_caller->output, MS_ANALYSE_DISPLAY_COMPARE_COLOR, &c1) == 0);
		}
	}

	end_call(caller, callee);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void on_eof(LinphonePlayer *player) {
	LinphonePlayerCbs *cbs = linphone_player_get_current_callbacks(player);
	LinphoneCoreManager *marie = (LinphoneCoreManager *)linphone_player_cbs_get_user_data(cbs);
	marie->stat.number_of_player_eof++;
}

/*
 * TODO: make the same test with opus + H264.
 */
static void call_with_video_mkv_file_player(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	char *src_mkv, *hellowav;
	bool_t call_ok;
	LinphoneVideoActivationPolicy *pol;

	src_mkv = bc_tester_res("sounds/sintel_trailer_opus_vp8.mkv");
	hellowav = bc_tester_res("sounds/hello8000_mkv_ref.wav");

	if (!linphone_core_file_format_supported(marie->lc, "mkv")) {
		ms_warning("Test skipped, no mkv support.");
		goto end;
	}
	if (!ms_factory_codec_supported(linphone_core_get_ms_factory(marie->lc), "opus") ||
	    !ms_factory_codec_supported(linphone_core_get_ms_factory(pauline->lc), "vp8")) {
		ms_warning("Test skipped, no opus or VP8 support.");
		goto end;
	}

	disable_all_video_codecs_except_one(marie->lc, "VP8");
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

	linphone_core_set_video_device(
	    marie->lc,
	    liblinphone_tester_static_image_id); /* so that if player doesn't work there will be very little RTP packets. */
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_video_activation_policy_unref(pol);

	/*caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using call's
	 * player*/
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, NULL);
	/*callee is recording and plays file*/
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_play_file(pauline->lc,
	                            hellowav); /*just to send something but we are not testing what is sent by pauline*/

	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (!call_ok) goto end;
	player = linphone_call_get_player(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(player);
	if (player) {
		LinphoneCallStats *video_stats;
		cbs = linphone_factory_create_player_cbs(linphone_factory_get());
		linphone_player_cbs_set_eof_reached(cbs, on_eof);
		linphone_player_cbs_set_user_data(cbs, marie);
		linphone_player_add_callbacks(player, cbs);
		int res = linphone_player_open(player, src_mkv);

		BC_ASSERT_EQUAL(res, 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");

		liblinphone_tester_set_next_video_frame_decoded_cb(linphone_core_get_current_call(pauline->lc));

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded, 1));
		/*wait for some seconds so that we can have an fps measurement */

		wait_for_until(pauline->lc, marie->lc, NULL, 0, 6000);
		BC_ASSERT_GREATER(linphone_call_params_get_received_framerate(
		                      linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc))),
		                  15.0f, float, "%f");

		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_player_eof, 1, 25000));
		linphone_player_close(player);

		video_stats = linphone_call_get_video_stats(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_PTR_NOT_NULL(video_stats);
		if (video_stats) {
			/* Make a few verification on the amount of RTP packets received to check that streaming from file appeared
			 * to work. */
			BC_ASSERT_GREATER((unsigned long)linphone_call_stats_get_rtp_stats(video_stats)->packet_recv, 2200UL,
			                  unsigned long, "%lu");
			BC_ASSERT_LOWER((unsigned long)linphone_call_stats_get_rtp_stats(video_stats)->outoftime, 20UL,
			                unsigned long, "%lu");
			BC_ASSERT_LOWER((unsigned long)linphone_call_stats_get_rtp_stats(video_stats)->discarded, 20UL,
			                unsigned long, "%lu");
		}
		linphone_call_stats_unref(video_stats);
	}
	end_call(marie, pauline);
	goto end;

end:
	if (cbs) linphone_player_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(src_mkv);
	ms_free(hellowav);
}

static void video_call_with_video_forwarding_base(bool_t forwardee_end_call) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_tcp_rc");
	int dummy = 0;

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_core_set_video_activation_policy(laure->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	disable_all_video_codecs_except_one(marie->lc, "VP8");

	// Set Marie to shared media resources so that she can have two calls without pause
	linphone_core_set_media_resource_mode(marie->lc, LinphoneSharedMediaResources);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(laure->lc, TRUE);
	linphone_core_enable_video_display(laure->lc, TRUE);
	disable_all_video_codecs_except_one(laure->lc, "VP8");

	// Make first a call from marie to pauline
	if (BC_ASSERT_TRUE(call(marie, pauline))) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));

		// Put pauline on "pause" by changing the audio direction to inactive and video to recvonly
		LinphoneCall *marie_pauline_call = linphone_core_get_current_call(marie->lc);
		LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, marie_pauline_call);

		linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionInactive);
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionRecvOnly);

		linphone_call_update(marie_pauline_call, params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));

		wait_for_list(lcs, &dummy, 1, 2000);

		// Make a call from marie to laure, since Marie has media resource mode to shared it shouldn't pause it
		if (BC_ASSERT_TRUE(call(marie, laure))) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));

			LinphoneCall *marie_laure_call = linphone_core_get_call_by_remote_address2(marie->lc, laure->identity);
			LinphoneVideoSourceDescriptor *descriptor = linphone_video_source_descriptor_new();
			linphone_video_source_descriptor_set_call(descriptor, marie_pauline_call);
			linphone_call_set_video_source(marie_laure_call, descriptor);
			linphone_video_source_descriptor_unref(descriptor);

			VideoStream *vstream = (VideoStream *)linphone_call_get_stream(marie_laure_call, LinphoneStreamTypeVideo);
			BC_ASSERT_TRUE(vstream->is_forwarding);
			BC_ASSERT_STRING_EQUAL(ms_filter_get_name(vstream->source), "MSItcSource");

			if (forwardee_end_call) {
				end_call(pauline, marie);
			}

			descriptor = linphone_video_source_descriptor_new();
			linphone_video_source_descriptor_set_camera_id(descriptor, liblinphone_tester_static_image_id);
			linphone_call_set_video_source(marie_laure_call, descriptor);
			linphone_video_source_descriptor_unref(descriptor);

			BC_ASSERT_FALSE(vstream->is_forwarding);
			BC_ASSERT_STRING_EQUAL(ms_filter_get_name(vstream->source), "MSStaticImage");

			const MSWebCam *current_cam = video_stream_get_camera(vstream);
			if (BC_ASSERT_PTR_NOT_NULL(current_cam))
				BC_ASSERT_STRING_EQUAL(ms_web_cam_get_name(current_cam), "Static picture");

			end_call(laure, marie);
		}

		if (!forwardee_end_call) {
			// Stop the "pause"
			params = linphone_core_create_call_params(marie->lc, marie_pauline_call);

			linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendRecv);
			linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendRecv);

			linphone_call_update(marie_pauline_call, params);
			linphone_call_params_unref(params);

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 3, 10000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3, 10000));

			end_call(marie, pauline);
		}
	}

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void video_call_with_video_forwarding(void) {
	video_call_with_video_forwarding_base(FALSE);
}

static void video_call_with_video_forwarding_forwardee_ends_first(void) {
	video_call_with_video_forwarding_base(TRUE);
}

static void video_call_set_image_as_video_source(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	char *qrcode_image = bc_tester_res("images/linphonesiteqr.jpg");

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	// Make first a call from marie to pauline
	if (BC_ASSERT_TRUE(call(marie, pauline))) {
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));

		LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
		LinphoneVideoSourceDescriptor *descriptor = linphone_video_source_descriptor_new();
		linphone_video_source_descriptor_set_image(descriptor, qrcode_image);
		linphone_call_set_video_source(marie_call, descriptor);
		linphone_video_source_descriptor_unref(descriptor);

		VideoStream *vstream = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);
		BC_ASSERT_STRING_EQUAL(ms_filter_get_name(vstream->source), "MSStaticImage");

		descriptor = linphone_video_source_descriptor_new();
		linphone_video_source_descriptor_set_camera_id(descriptor, liblinphone_tester_mire_id);
		linphone_call_set_video_source(marie_call, descriptor);
		linphone_video_source_descriptor_unref(descriptor);

		BC_ASSERT_STRING_EQUAL(ms_filter_get_name(vstream->source), "MSMire");

		end_call(marie, pauline);
	}

	if (qrcode_image) bctbx_free(qrcode_image);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static const char *_linphone_media_direction_to_string(LinphoneMediaDirection dir) {
	switch (dir) {
		case LinphoneMediaDirectionInactive:
			return "inactive";
			break;
		case LinphoneMediaDirectionRecvOnly:
			return "recv-only";
			break;
		case LinphoneMediaDirectionSendOnly:
			return "send-only";
			break;
		case LinphoneMediaDirectionSendRecv:
			return "send-recv";
			break;
		case LinphoneMediaDirectionInvalid:
			return "invalid";
			break;
	}
	return "bug";
}

static void on_call_state_change(BCTBX_UNUSED(LinphoneCore *core),
                                 LinphoneCall *call,
                                 LinphoneCallState state,
                                 BCTBX_UNUSED(const char *msg)) {
	switch (state) {
		case LinphoneCallIncomingReceived:
			BC_ASSERT_TRUE(linphone_call_params_get_video_direction(linphone_call_get_remote_params(call)) ==
			               LinphoneMediaDirectionRecvOnly);
			ms_message("Call received with video direction: %s",
			           _linphone_media_direction_to_string(
			               linphone_call_params_get_video_direction(linphone_call_get_remote_params(call))));
			break;
		default:
			break;
	}
}

static void call_with_video_recvonly(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const LinphoneCallParams *remote_params;
	LinphoneCallParams *callee_params;
	LinphoneCoreCbs *pauline_cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	linphone_core_cbs_set_call_state_changed(pauline_cbs, on_call_state_change);
	linphone_core_add_callbacks(pauline->lc, pauline_cbs);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	LinphoneCallParams *caller_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(caller_params, TRUE);
	linphone_call_params_set_video_direction(caller_params, LinphoneMediaDirectionRecvOnly);
	LinphoneCall *marie_call = linphone_core_invite_address_with_params(marie->lc, pauline->identity, caller_params);
	linphone_call_params_unref(caller_params);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_call)) goto end;

	remote_params = linphone_call_get_remote_params(pauline_call);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(remote_params));
	BC_ASSERT_TRUE(linphone_call_params_get_video_direction(remote_params) == LinphoneMediaDirectionRecvOnly);
	callee_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_video(callee_params, TRUE);
	linphone_call_accept_with_params(pauline_call, callee_params);
	linphone_call_params_unref(callee_params);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_TRUE(linphone_call_params_get_video_direction(linphone_call_get_current_params(marie_call)) ==
	               LinphoneMediaDirectionRecvOnly);

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_params_get_video_direction(linphone_call_get_current_params(pauline_call)) ==
	               LinphoneMediaDirectionSendOnly);

	linphone_call_terminate(pauline_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

end:
	linphone_core_cbs_unref(pauline_cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void disable_all_audio_codecs(LinphoneCore *lc) {
	const bctbx_list_t *elem = linphone_core_get_audio_codecs(lc);
	PayloadType *pt;

	for (; elem != NULL; elem = elem->next) {
		pt = (PayloadType *)elem->data;
		linphone_core_enable_payload_type(lc, pt, FALSE);
	}
}

static void video_call_without_audio_disable_video(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	linphone_core_set_preferred_video_definition_by_name(marie->lc, "QVGA");
	linphone_core_set_preferred_video_definition_by_name(pauline->lc, "QVGA");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	// no audio
	disable_all_audio_codecs(marie->lc);
	disable_all_audio_codecs(pauline->lc);

	// Marie calls Pauline
	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (pauline_call && marie_call) {
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
		BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

		const LinphoneCallParams *pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(pauline_call_params));
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(pauline_call_params));

		const LinphoneCallParams *marie_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_PTR_NOT_NULL(marie_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_params));
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(marie_call_params));

		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		// disable video
		LinphoneCallParams *new_params = linphone_core_create_call_params(pauline->lc, pauline_call);
		linphone_call_params_enable_video(new_params, FALSE);
		linphone_call_update(pauline_call, new_params);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote,
		                        initial_marie_stat.number_of_LinphoneCallPausedByRemote + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
		                        initial_pauline_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		// Check video parameters
		pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(pauline_call_params));
		marie_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(marie_call_params));

		initial_marie_stat = marie->stat;
		initial_pauline_stat = pauline->stat;

		// enable video
		new_params = linphone_core_create_call_params(marie->lc, marie_call);
		linphone_call_params_enable_video(new_params, TRUE);
		linphone_call_update(marie_call, new_params);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
		                        initial_pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdating,
		                        initial_marie_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		// Check video parameters
		pauline_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(pauline_call_params));
		marie_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_params));

		liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded,
		                        initial_pauline_stat.number_of_IframeDecoded + 1));
		liblinphone_tester_set_next_video_frame_decoded_cb(marie_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded,
		                        initial_marie_stat.number_of_IframeDecoded + 1));
	}

	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_video_requested_and_terminate(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");

	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	LinphoneVideoActivationPolicy *marie_vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(marie_vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(marie_vpol, FALSE);
	linphone_core_set_video_activation_policy(marie->lc, marie_vpol);
	linphone_video_activation_policy_unref(marie_vpol);

	LinphoneVideoActivationPolicy *pauline_vpol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(pauline_vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(pauline_vpol, FALSE);
	linphone_core_set_video_activation_policy(pauline->lc, pauline_vpol);
	linphone_video_activation_policy_unref(pauline_vpol);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);

	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);

	// Marie calls Pauline
	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (pauline_call && marie_call) {
		LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
		linphone_call_params_enable_video(marie_call_params, TRUE);
		linphone_call_update(marie_call, marie_call_params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));

		const LinphoneCallParams *updated_pauline_call_params = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(updated_pauline_call_params);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(updated_pauline_call_params));

		linphone_call_params_unref(marie_call_params);
	}
	// marie ends the call immediately
	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_memory() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");

	bool_t call_ok = TRUE;
	if (g_display_filter != "") {
		linphone_core_set_video_display_filter(marie->lc, g_display_filter.c_str());
		linphone_core_set_video_display_filter(pauline->lc, g_display_filter.c_str());
	}

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_use_preview_window(marie->lc, TRUE);

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	uint64_t memory_ref = 0;
	int excessCount = 0;
	for (int count = 0; call_ok && count < 50; ++count) {
		if (count == 3) { // Ignore first allocations to ignore preallocations/caches
			memory_ref = bc_tester_get_memory_consumption();
			ms_message("Memory consumption first reference: %lld KB", (long long)memory_ref / 1024);
		} else if (count > 3) { // Bad if more than 10% of memory
			auto current_memory = bc_tester_get_memory_consumption();
			call_ok = current_memory <= 1.1 * memory_ref;
			if (!call_ok) {
				ms_warning("Excess memory consumption : %lld KB / %lld KB [count=%d]", (long long)current_memory / 1024,
				           (long long)memory_ref / 1024, count);
				if (++excessCount <= 3) { // Let 3 attempts on growing memory.
					memory_ref = current_memory;
					call_ok = TRUE;
				} else {
					BC_ASSERT_TRUE(current_memory <= 1.1 * memory_ref); // Make assert
				}
			}
		}
		// Switch on
		BC_ASSERT_TRUE(request_video(marie, pauline, TRUE));
		linphone_core_enable_video_preview(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_enable_video_capture(pauline->lc, TRUE);
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 50);

		linphone_core_set_native_preview_window_id(marie->lc, LINPHONE_VIDEO_DISPLAY_AUTO);
		linphone_core_set_native_video_window_id(marie->lc, LINPHONE_VIDEO_DISPLAY_AUTO);
		linphone_core_set_native_preview_window_id(pauline->lc, LINPHONE_VIDEO_DISPLAY_AUTO);
		linphone_core_set_native_video_window_id(pauline->lc, LINPHONE_VIDEO_DISPLAY_AUTO);

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 5000); // Let some time to open and to print something.

		// Switch off
		linphone_core_enable_video_preview(marie->lc, FALSE);
		BC_ASSERT_TRUE(remove_video(marie, pauline));
		linphone_core_set_native_preview_window_id(marie->lc, LINPHONE_VIDEO_DISPLAY_NONE);
		linphone_core_set_native_video_window_id(marie->lc, LINPHONE_VIDEO_DISPLAY_NONE);
		linphone_core_set_native_preview_window_id(pauline->lc, LINPHONE_VIDEO_DISPLAY_NONE);
		linphone_core_set_native_video_window_id(pauline->lc, LINPHONE_VIDEO_DISPLAY_NONE);
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 50); // Closing windows
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static test_t call_video_tests[] = {
    TEST_NO_TAG("Call paused resumed with video", call_paused_resumed_with_video),
    TEST_NO_TAG("Call paused resumed with video enabled", call_paused_resumed_with_video_enabled),
    TEST_NO_TAG("Call paused resumed with video send-only", call_paused_resumed_with_video_send_only),
    TEST_NO_TAG("Call paused resumed with automatic video accept", call_paused_resumed_with_automatic_video_accept),
    TEST_NO_TAG("ZRTP video call", zrtp_video_call),
    TEST_NO_TAG("Simple video call AVPF", video_call_avpf),
    TEST_NO_TAG("Simple video call implicit AVPF both", video_call_using_policy_AVPF_implicit_caller_and_callee),
    TEST_NO_TAG("Simple video call disable implicit AVPF on callee", video_call_disable_implicit_AVPF_on_callee),
    TEST_NO_TAG("Simple video call disable implicit AVPF on caller", video_call_disable_implicit_AVPF_on_caller),
    TEST_NO_TAG("Simple video call AVPF to implicit AVPF", video_call_AVPF_to_implicit_AVPF),
    TEST_NO_TAG("Simple video call implicit AVPF to AVPF", video_call_implicit_AVPF_to_AVPF),
    TEST_NO_TAG("Video added by reINVITE, with implicit AVPF", video_call_established_by_reinvite_with_implicit_avpf),
    TEST_NO_TAG("Simple video call", video_call),
    TEST_NO_TAG("Simple video call fallback to dummy codec", video_call_dummy_codec),
    TEST_NO_TAG("Simple video call without rtcp", video_call_without_rtcp),
    TEST_NO_TAG("Simple ZRTP video call", video_call_zrtp),
    TEST_ONE_TAG("Simple DTLS video call", video_call_dtls, "DTLS"),
    TEST_NO_TAG("Simple video call using policy", video_call_using_policy),
    TEST_NO_TAG("Video call using policy with callee video disabled",
                video_call_using_policy_with_callee_video_disabled),
    TEST_NO_TAG("Video call using policy with caller video disabled",
                video_call_using_policy_with_caller_video_disabled),
    TEST_NO_TAG("Video call without SDP", video_call_no_sdp),
    TEST_ONE_TAG("SRTP ice video call", srtp_video_ice_call, "ICE"),
    TEST_ONE_TAG("ZRTP ice video call", zrtp_video_ice_call, "ICE"),
    TEST_NO_TAG("Call with video added", call_with_video_added),
    TEST_NO_TAG("Call with video added 2", call_with_video_added_2),
    TEST_NO_TAG("Call with video added (random ports)", call_with_video_added_random_ports),
    TEST_NO_TAG("Call with several video switches", call_with_several_video_switches),
    TEST_NO_TAG("Call with video declined", call_with_declined_video),
    TEST_NO_TAG("Call with video declined despite policy", call_with_declined_video_despite_policy),
    TEST_NO_TAG("Call with video declined using policy", call_with_declined_video_using_policy),
    TEST_NO_TAG("Call with multiple early media", multiple_early_media),
    TEST_ONE_TAG("Call with ICE from video to non-video", call_with_ice_video_to_novideo, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added", call_with_ice_video_added, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added 2", call_with_ice_video_added_2, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added 3", call_with_ice_video_added_3, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added 4", call_with_ice_video_added_4, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added 5", call_with_ice_video_added_5, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added 6", call_with_ice_video_added_6, "ICE"),
    TEST_ONE_TAG("Call with ICE and video added 7", call_with_ice_video_added_7, "ICE"),
    TEST_NO_TAG("Call with video requested but call ends right after", call_with_video_requested_and_terminate),
    TEST_ONE_TAG("Call with ICE, video and realtime text", call_with_ice_video_and_rtt, "ICE"),
    TEST_ONE_TAG("Call with ICE, video only", call_with_ice_video_only, "ICE"),
    TEST_ONE_TAG("Video call with ICE accepted using call params", video_call_ice_params, "ICE"),
    TEST_ONE_TAG("Audio call with ICE paused with caller video policy enabled",
                 audio_call_with_ice_with_video_policy_enabled,
                 "ICE"),
    TEST_ONE_TAG("Video call recording (H264)", video_call_recording_h264_test, "H264"),
    TEST_NO_TAG("Video call recording (VP8)", video_call_recording_vp8_test),
    TEST_NO_TAG("Snapshot", video_call_snapshot),
    TEST_NO_TAG("Snapshots", video_call_snapshots),
    TEST_NO_TAG("Video call with early media and no matching audio codecs",
                video_call_with_early_media_no_matching_audio_codecs),
    TEST_ONE_TAG("DTLS SRTP video call", dtls_srtp_video_call, "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ice video call", dtls_srtp_ice_video_call, "ICE", "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP ice video call with relay", dtls_srtp_ice_video_call_with_relay, "ICE", "DTLS"),
    TEST_NO_TAG("Video call with limited bandwidth", video_call_limited_bandwidth),
    TEST_ONE_TAG("Video call accepted in send only", accept_call_in_send_only, "H264"),
    TEST_TWO_TAGS("Video call accepted in send only with ice", accept_call_in_send_only_with_ice, "ICE", "H264"),
    TEST_ONE_TAG("2 Video call accepted in send only", two_accepted_call_in_send_only, "H264"),
    TEST_ONE_TAG("Classic video entry phone setup (sendrecv)", classic_video_entry_phone_setup_sendrecv, "H264"),
    TEST_ONE_TAG("Classic video entry phone setup (recvonly)", classic_video_entry_phone_setup_recvonly, "H264"),
    TEST_NO_TAG("Video call with no audio and no video codec", video_call_with_no_audio_and_no_video_codec),
    TEST_NO_TAG("Video call with automatic video acceptance disabled on one end only",
                video_call_with_auto_video_accept_disabled_on_one_end),
    TEST_NO_TAG("Asymmetrical video call starts with video", asymmetrical_video_call_starts_with_video),
    TEST_NO_TAG("Asymmetrical video call starts without video", asymmetrical_video_call_starts_without_video),
    TEST_NO_TAG("Asymmetrical video call with flexfec starts with video",
                asymmetrical_video_call_with_flexfec_starts_with_video),
    TEST_NO_TAG("Asymmetrical video call with callee enabled video first",
                asymmetrical_video_call_with_callee_enabled_video_first),
    TEST_NO_TAG("Asymmetrical video call 2", asymmetrical_video_call_2),
    TEST_NO_TAG("Call with early media and no SDP in 200 Ok with video",
                call_with_early_media_and_no_sdp_in_200_with_video),
    TEST_NO_TAG("Video call with fallback to Static Picture when no fps",
                video_call_with_fallback_to_static_picture_when_no_fps),
    TEST_NO_TAG("Video call with mire and analyse", video_call_with_mire_and_analyse),

    TEST_NO_TAG("Video call recv-only", call_with_video_recvonly),
    TEST_NO_TAG("Video call without audio, disable video", video_call_without_audio_disable_video),
    TEST_ONE_TAG("Video call memory", video_call_memory, "skip")};

static test_t call_video_advanced_scenarios_tests[] = {
    TEST_NO_TAG("Call paused resumed with video no sdp ack", call_paused_resumed_with_no_sdp_ack),
    TEST_NO_TAG("Call paused resumed with video no sdk ack using video policy for resume offers",
                call_paused_resumed_with_no_sdp_ack_using_video_policy),
    TEST_NO_TAG("Call paused, updated and resumed with video no sdk ack using video policy for resume offers",
                call_paused_updated_resumed_with_no_sdp_ack_using_video_policy),
    TEST_NO_TAG("Call paused, updated and resumed with video no sdk ack using video policy for resume offers with "
                "accept call update",
                call_paused_updated_resumed_with_no_sdp_ack_using_video_policy_and_accept_call_update),
    TEST_ONE_TAG("Video call with re-invite(inactive) followed by re-invite",
                 video_call_with_re_invite_inactive_followed_by_re_invite,
                 "H264"),
    TEST_ONE_TAG("Video call with re-invite(inactive) followed by re-invite(no sdp)",
                 video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp,
                 "H264"),
    TEST_ONE_TAG("SRTP Video call with re-invite(inactive) followed by re-invite",
                 srtp_video_call_with_re_invite_inactive_followed_by_re_invite,
                 "H264"),
    TEST_ONE_TAG("SRTP Video call with re-invite(inactive) followed by re-invite(no sdp)",
                 srtp_video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp,
                 "H264"),
    TEST_NO_TAG("Incoming REINVITE with invalid SDP in ACK", incoming_reinvite_with_invalid_ack_sdp),
    TEST_NO_TAG("Outgoing REINVITE with invalid SDP in ACK", outgoing_reinvite_with_invalid_ack_sdp),
    TEST_NO_TAG("Video call with file streaming", call_with_video_mkv_file_player),
    TEST_NO_TAG("Video call with video forwarding", video_call_with_video_forwarding),
    TEST_NO_TAG("Video call with video forwarding forwardee ends first",
                video_call_with_video_forwarding_forwardee_ends_first),
    TEST_NO_TAG("Video call set image as video source", video_call_set_image_as_video_source)};

static int init_msogl_call_suite() {
#if defined(__ANDROID__) || defined(__APPLE__)
	return -1;
#else
	if (std::string(ms_factory_get_default_video_renderer(NULL)) == "MSOGL")
		return -1; // Do not test MSOGL as it is already used by default tests
	else {
		g_display_filter = "MSOGL";
		return 0;
	}
#endif
}
test_suite_t call_video_test_suite = {"Video Call",
                                      NULL,
                                      NULL,
                                      liblinphone_tester_before_each,
                                      liblinphone_tester_after_each,
                                      sizeof(call_video_tests) / sizeof(call_video_tests[0]),
                                      call_video_tests,
                                      0,
                                      2};
test_suite_t call_video_msogl_test_suite = {"Video Call MSOGL",
                                            init_msogl_call_suite,
                                            NULL,
                                            liblinphone_tester_before_each,
                                            liblinphone_tester_after_each,
                                            sizeof(call_video_tests) / sizeof(call_video_tests[0]),
                                            call_video_tests,
                                            0,
                                            2};
test_suite_t call_video_advanced_scenarios_test_suite = {"Video Call advanced scenarios",
                                                         NULL,
                                                         NULL,
                                                         liblinphone_tester_before_each,
                                                         liblinphone_tester_after_each,
                                                         sizeof(call_video_advanced_scenarios_tests) /
                                                             sizeof(call_video_advanced_scenarios_tests[0]),
                                                         call_video_advanced_scenarios_tests,
                                                         0,
                                                         2};

#endif // ifdef VIDEO_ENABLED
