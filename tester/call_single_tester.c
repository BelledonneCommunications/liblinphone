/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "bctoolbox/defs.h"

#include "belle-sip/sipstack.h"

#include "linphone/api/c-types.h"
#include "mediastreamer2/msutils.h"
#include "mediastreamer2/msvolume.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

#ifdef _WIN32
#define unlink _unlink
#ifndef F_OK
#define F_OK 00 /*visual studio does not define F_OK*/
#endif
#endif

// prototype definition for call_recording()
#ifdef __ANDROID__
#ifdef HAVE_OPENH264
extern void libmsopenh264_init(MSFactory *factory);
#endif
#endif

static void linphone_call_next_video_frame_decoded_cb(LinphoneCall *call) {
	LinphoneCallLog *clog = linphone_call_get_call_log(call);
	char *to = linphone_address_as_string(linphone_call_log_get_to_address(clog));
	char *from = linphone_address_as_string(linphone_call_log_get_to_address(clog));
	stats *counters;
	LinphoneCore *lc = linphone_call_get_core(call);
	ms_message("call from [%s] to [%s] receive iFrame", from, to);
	ms_free(to);
	ms_free(from);
	counters = (stats *)get_stats(lc);
	counters->number_of_IframeDecoded++;
}

void liblinphone_tester_set_next_video_frame_decoded_cb(LinphoneCall *call) {
	if (belle_sip_object_data_get(BELLE_SIP_OBJECT(call), "next_video_frame_decoded_set") == NULL) {
		LinphoneCallCbs *call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
		linphone_call_cbs_set_next_video_frame_decoded(call_cbs, linphone_call_next_video_frame_decoded_cb);
		linphone_call_add_callbacks(call, call_cbs);
		linphone_call_cbs_unref(call_cbs);
		belle_sip_object_data_set(BELLE_SIP_OBJECT(call), "next_video_frame_decoded_set", (void *)1, NULL);
	}
	linphone_call_request_notify_next_video_frame_decoded(call);
}

#define reset_call_stats(var, value)                                                                                   \
	if (var) linphone_call_stats_unref(var);                                                                           \
	var = value

static void liblinphone_tester_check_rtcp_base(LinphoneCoreManager *caller,
                                               LinphoneCoreManager *callee,
                                               LinphoneCall *c1,
                                               LinphoneCall *c2) {
	MSTimeSpec ts;
	int max_time_to_wait;
	LinphoneCallStats *audio_stats1 = NULL, *video_stats1 = NULL, *audio_stats2 = NULL, *video_stats2 = NULL;
	const LinphoneCallParams *params;
	bool_t audio_active, video_active;

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	if (!c1 || !c2) return;

	linphone_call_ref(c1);
	linphone_call_ref(c2);
	liblinphone_tester_clock_start(&ts);
	max_time_to_wait = 15000;

	params = linphone_call_get_current_params(c1);
	audio_active = linphone_call_params_audio_enabled(params) &&
	               linphone_call_params_get_audio_direction(params) != LinphoneMediaDirectionInactive;
	video_active = linphone_call_params_video_enabled(params) &&
	               linphone_call_params_get_video_direction(params) != LinphoneMediaDirectionInactive;

	bool_t stats_ok;
	do {
		stats_ok = TRUE;
		reset_call_stats(audio_stats1, linphone_call_get_audio_stats(c1));
		reset_call_stats(video_stats1, linphone_call_get_video_stats(c1));
		reset_call_stats(audio_stats2, linphone_call_get_audio_stats(c2));
		reset_call_stats(video_stats2, linphone_call_get_video_stats(c2));
		if (linphone_core_rtcp_enabled(caller->lc) && linphone_core_rtcp_enabled(callee->lc)) {
			if (audio_active) {
				BC_ASSERT_PTR_NOT_NULL(audio_stats1);
				BC_ASSERT_PTR_NOT_NULL(audio_stats2);
				if (!audio_stats1 || !audio_stats2) break;
				if (linphone_call_stats_get_round_trip_delay(audio_stats1) == 0.0 ||
				    linphone_call_stats_get_round_trip_delay(audio_stats2) == 0.0) {
					stats_ok = FALSE;
				}
			}
			if (video_active) {
				BC_ASSERT_PTR_NOT_NULL(video_stats1);
				BC_ASSERT_PTR_NOT_NULL(video_stats2);
				if (!video_stats1 || !video_stats2) break;
				if (linphone_call_stats_get_round_trip_delay(video_stats1) == 0.0 ||
				    linphone_call_stats_get_round_trip_delay(video_stats2) == 0.0) {
					stats_ok = FALSE;
				}
			}
		}
		wait_for_until(caller->lc, callee->lc, NULL, 0, 20); /*just to sleep while iterating*/
	} while (!(stats_ok || liblinphone_tester_clock_elapsed(&ts, max_time_to_wait)));

	reset_call_stats(audio_stats1, linphone_call_get_audio_stats(c1));
	reset_call_stats(video_stats1, linphone_call_get_video_stats(c1));
	reset_call_stats(audio_stats2, linphone_call_get_audio_stats(c2));
	reset_call_stats(video_stats2, linphone_call_get_video_stats(c2));
	if (linphone_core_rtcp_enabled(caller->lc) && linphone_core_rtcp_enabled(callee->lc)) {
		BC_ASSERT_GREATER(caller->stat.number_of_rtcp_received, 1, int, "%i");
		BC_ASSERT_GREATER(callee->stat.number_of_rtcp_received, 1, int, "%i");
		if (linphone_call_stats_get_rtp_packet_sent(audio_stats1) > 0) {
			BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_round_trip_delay(audio_stats1), 0.0, float, "%f");
		}
		BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_round_trip_delay(audio_stats2), 0.0, float, "%f");
		if (linphone_call_log_video_enabled(linphone_call_get_call_log(c1))) {
			BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_round_trip_delay(video_stats1), 0.0, float, "%f");
		}
		if (linphone_call_log_video_enabled(linphone_call_get_call_log(c2))) {
			BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_round_trip_delay(video_stats2), 0.0, float, "%f");
		}
	} else {
		if (linphone_core_rtcp_enabled(caller->lc)) {
			BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(audio_stats1)->sent_rtcp_packets, 0, unsigned long long,
			                "%llu");
			BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(audio_stats2)->recv_rtcp_packets, 0, unsigned long long,
			                "%llu");
			if (linphone_call_log_video_enabled(linphone_call_get_call_log(c1))) {
				BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(video_stats1)->sent_rtcp_packets, 0,
				                unsigned long long, "%llu");
			}
			if (linphone_call_log_video_enabled(linphone_call_get_call_log(c2))) {
				BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(video_stats2)->recv_rtcp_packets, 0,
				                unsigned long long, "%llu");
			}
		}
		if (linphone_core_rtcp_enabled(callee->lc)) {
			BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(audio_stats2)->sent_rtcp_packets, 0, unsigned long long,
			                "%llu");
			BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(audio_stats1)->recv_rtcp_packets, 0, unsigned long long,
			                "%llu");
			if (linphone_call_log_video_enabled(linphone_call_get_call_log(c1))) {
				BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(video_stats1)->recv_rtcp_packets, 0,
				                unsigned long long, "%llu");
			}
			if (linphone_call_log_video_enabled(linphone_call_get_call_log(c2))) {
				BC_ASSERT_EQUAL(linphone_call_stats_get_rtp_stats(video_stats2)->sent_rtcp_packets, 0,
				                unsigned long long, "%llu");
			}
		}
	}

	if (audio_stats1) linphone_call_stats_unref(audio_stats1);
	if (audio_stats2) linphone_call_stats_unref(audio_stats2);
	if (video_stats1) linphone_call_stats_unref(video_stats1);
	if (video_stats2) linphone_call_stats_unref(video_stats2);

	linphone_call_unref(c1);
	linphone_call_unref(c2);
}

void liblinphone_tester_check_rtcp(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	LinphoneCall *c1, *c2;
	c1 = linphone_core_get_current_call(caller->lc);
	c2 = linphone_core_get_current_call(callee->lc);

	liblinphone_tester_check_rtcp_base(caller, callee, c1, c2);
}

void liblinphone_tester_check_rtcp_2(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	LinphoneCall *c1, *c2;
	c1 = linphone_core_get_call_by_remote_address2(caller->lc, callee->identity);
	c2 = linphone_core_get_call_by_remote_address2(callee->lc, caller->identity);

	liblinphone_tester_check_rtcp_base(caller, callee, c1, c2);
}

static const char *info_content = "<somexml>blabla</somexml>";

static void call_check_log_duration_cb(BCTBX_UNUSED(LinphoneCore *lc),
                                       LinphoneCall *call,
                                       LinphoneCallState cstate,
                                       BCTBX_UNUSED(const char *message)) {
	if (cstate == LinphoneCallStateEnd || cstate == LinphoneCallStateReleased) {
		LinphoneCallLog *call_log = linphone_call_get_call_log(call);
		BC_ASSERT_PTR_NOT_NULL(call_log);
		if (call_log) {
			BC_ASSERT_GREATER_STRICT(linphone_call_log_get_duration(call_log), 0, int, "%d");
		}
		BC_ASSERT_GREATER_STRICT(linphone_call_get_duration(call), 0, int, "%d");
	}
}

void simple_call_base_with_rcs(const char *caller_rc,
                               const char *callee_rc,
                               bool_t enable_multicast_recv_side,
                               bool_t disable_soundcard,
                               bool_t use_multipart_invite_body,
                               bool_t double_call) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	const LinphoneAddress *from;
	LinphoneCall *pauline_call;
	LinphoneProxyConfig *marie_cfg;

	if (disable_soundcard) {
		ms_snd_card_manager_bypass_soundcard_detection(TRUE);
	}

	marie = linphone_core_manager_new(caller_rc ? caller_rc : "marie_rc");
	pauline = linphone_core_manager_new(
	    callee_rc ? callee_rc : (transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"));

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_check_log_duration_cb);
	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_add_callbacks(pauline->lc, cbs);

	/* with the account manager, we might lose the identity */
	marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	{
		BC_ASSERT_PTR_NOT_NULL(marie_cfg);
		LinphoneAddress *marie_addr = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_cfg));
		char *marie_tmp_id = NULL;
		linphone_address_set_display_name(marie_addr, "Super Marie");
		marie_tmp_id = linphone_address_as_string(marie_addr);

		linphone_proxy_config_edit(marie_cfg);
		linphone_proxy_config_set_identity_address(marie_cfg, marie_addr);
		linphone_proxy_config_done(marie_cfg);

		ms_free(marie_tmp_id);
		linphone_address_unref(marie_addr);
	}

	linphone_core_enable_audio_multicast(pauline->lc, enable_multicast_recv_side);

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	if (use_multipart_invite_body) {
		LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, NULL);

		LinphoneContent *content = linphone_core_create_content(marie->lc);
		linphone_content_set_type(content, "application");
		linphone_content_set_subtype(content, "somexml");
		linphone_content_set_buffer(content, (const uint8_t *)info_content, strlen(info_content));

		linphone_call_params_add_custom_content(params, content);

		BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, params));
		linphone_call_params_unref(params);
		linphone_content_unref(content);
	} else {
		BC_ASSERT_TRUE(call(marie, pauline));
	}

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	/*check that display name is correctly propagated in From */
	if (pauline_call) {
		from = linphone_call_get_remote_address(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_PTR_NOT_NULL(from);
		if (from) {
			const char *dname = linphone_address_get_display_name(from);
			BC_ASSERT_PTR_NOT_NULL(dname);
			if (dname) {
				BC_ASSERT_STRING_EQUAL(dname, "Super Marie");
			}
		}

		const LinphoneCallParams *params = linphone_call_get_remote_params(pauline_call);
		bctbx_list_t *parts = linphone_call_params_get_custom_contents(params);
		if (use_multipart_invite_body) {
			BC_ASSERT_PTR_NOT_NULL(parts);
			if (parts) {
				BC_ASSERT_EQUAL((int)bctbx_list_size(parts), 1, int, "%i");
				LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(parts);
				BC_ASSERT_PTR_NOT_NULL(content);
				if (content) {
					BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
					BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "somexml");
					BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(content), info_content);
				}
				bctbx_list_free_with_data(parts, (void (*)(void *))linphone_content_unref);
			}
		} else {
			BC_ASSERT_PTR_NULL(parts);
		}
	}

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	if (double_call) {
		stats pStats = pauline->stat;
		stats mStats = marie->stat;
		int nStop = linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopTone;
		LinphoneCall *call = linphone_core_invite_address(marie->lc, pauline->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived,
		                        pStats.number_of_LinphoneCallIncomingReceived + 1));
		LinphoneCall *pCall = linphone_core_get_current_call(pauline->lc);
		BC_ASSERT_PTR_NOT_NULL(pCall);
		if (call && pCall) {
			BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopTone, nStop + 1, int,
			                "%d");
			linphone_call_accept(pCall);
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
			                        pStats.number_of_LinphoneCallStreamsRunning + 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
			                        mStats.number_of_LinphoneCallStreamsRunning + 1));
			liblinphone_tester_check_rtcp(marie, pauline);
			end_call(marie, pauline);
		}
	}

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	if (disable_soundcard) {
		ms_snd_card_manager_bypass_soundcard_detection(FALSE);
	}
}

void simple_call_base(bool_t enable_multicast_recv_side, bool_t disable_soundcard, bool_t use_multipart_invite_body) {
	simple_call_base_with_rcs(NULL, NULL, enable_multicast_recv_side, disable_soundcard, use_multipart_invite_body,
	                          FALSE);
}

static void simple_double_call(void) {
	simple_call_base_with_rcs(NULL, NULL, FALSE, FALSE, FALSE, TRUE);
}

static void simple_call_without_soundcard(void) {
	simple_call_base(FALSE, TRUE, FALSE);
}

static void simple_call_with_multipart_invite_body(void) {
	simple_call_base(FALSE, FALSE, TRUE);
}

static void simple_call_with_video_declined(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(vpol, FALSE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (marie_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
	}
	LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
	}

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);

	stats initial_pauline_stat = pauline->stat;
	stats initial_marie_stat = marie->stat;
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_enable_video(marie_params, TRUE);
	linphone_call_update(marie_call, marie_params);
	linphone_call_params_unref(marie_params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1));

	int pauline_defer_update =
	    !!linphone_config_get_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", FALSE);
	BC_ASSERT_TRUE(pauline_defer_update);
	if (pauline_defer_update == TRUE) {
		LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);
		linphone_call_params_enable_video(pauline_params, FALSE);
		linphone_call_accept_update(pauline_call, pauline_params);
		linphone_call_params_unref(pauline_params);
	}
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating,
	                        initial_marie_stat.number_of_LinphoneCallUpdating + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));

	if (marie_call) {
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
	}
	if (pauline_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
	}

	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*This test is added to reproduce a crash when a call is failed synchronously*/
static void simple_call_with_no_sip_transport(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneSipTransports tr = {0};
	LinphoneSipTransports oldTr;
	LinphoneCall *call;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/*disable all transports so that the call will fail synchronously*/
	linphone_core_get_sip_transports(marie->lc, &oldTr);
	linphone_core_set_sip_transports(marie->lc, &tr);

	call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NULL(call);

	/* restore initial transports for Marie's core in order it be able to unregister */
	linphone_core_set_sip_transports(marie->lc, &oldTr);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_call_with_udp(void) {
	LinphoneCoreManager *michelle;
	LinphoneCoreManager *laure;
	const LinphoneAddress *from;
	LinphoneCall *laure_call;
	LinphoneProxyConfig *michelle_cfg;

	michelle = linphone_core_manager_new("michelle_rc_udp");
	laure = linphone_core_manager_new("laure_rc_udp");

	/* with the account manager, we might lose the identity */
	michelle_cfg = linphone_core_get_default_proxy_config(michelle->lc);
	{
		LinphoneAddress *michelle_addr =
		    linphone_address_clone(linphone_proxy_config_get_identity_address(michelle_cfg));
		char *michelle_tmp_id = NULL;
		linphone_address_set_display_name(michelle_addr, "Super michelle");
		michelle_tmp_id = linphone_address_as_string(michelle_addr);

		linphone_proxy_config_edit(michelle_cfg);
		linphone_proxy_config_set_identity_address(michelle_cfg, michelle_addr);
		linphone_proxy_config_done(michelle_cfg);

		ms_free(michelle_tmp_id);
		linphone_address_unref(michelle_addr);
	}

	BC_ASSERT_TRUE(call(michelle, laure));
	laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	/*check that display name is correctly propagated in From */
	if (laure_call) {
		from = linphone_call_get_remote_address(linphone_core_get_current_call(laure->lc));
		BC_ASSERT_PTR_NOT_NULL(from);
		if (from) {
			const char *dname = linphone_address_get_display_name(from);
			BC_ASSERT_PTR_NOT_NULL(dname);
			if (dname) {
				BC_ASSERT_STRING_EQUAL(dname, "Super michelle");
			}
		}
	}

	liblinphone_tester_check_rtcp(michelle, laure);
	end_call(michelle, laure);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(michelle);
}

static void automatic_call_termination(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;

	liblinphone_tester_check_rtcp(marie, pauline);

	linphone_core_destroy(pauline->lc);
	pauline->lc = NULL;
	/*marie shall receive the BYE*/
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_timed_out_bye(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	belle_sip_timer_config_t timer_config;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	BC_ASSERT_TRUE(call(marie, pauline));

	sal_set_send_error(linphone_core_get_sal(pauline->lc), 1500); /*to trash the message without generating error*/
	timer_config.T1 = 50;                                         /*to have timer F = 3s*/
	timer_config.T2 = 4000;
	timer_config.T3 = 0;
	timer_config.T4 = 5000;

	belle_sip_stack_set_timer_config(sal_get_stack_impl(linphone_core_get_sal(pauline->lc)), &timer_config);
	linphone_core_terminate_all_calls(pauline->lc);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1, timer_config.T1 * 84));

	sal_set_send_error(linphone_core_get_sal(pauline->lc), 0);

	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1, 5000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1, 5000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void direct_call_over_ipv6(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	if (liblinphone_tester_ipv6_available()) {
		LinphoneSipTransports pauline_transports;
		LinphoneAddress *pauline_dest = linphone_address_new("sip:[::1];transport=tcp");
		LinphoneCall *pauline_call;
		LinphoneCall *marie_call;
		marie = linphone_core_manager_new("marie_rc");
		pauline =
		    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
		linphone_core_set_default_proxy_config(marie->lc, NULL);
		/* We do not unset the default account for pauline (the callee): it shall be able to handle the call
		 * even if it does not come from the server */

		linphone_core_get_sip_transports_used(pauline->lc, &pauline_transports);
		linphone_address_set_port(pauline_dest, pauline_transports.tcp_port);
		marie_call = linphone_core_invite_address(marie->lc, pauline_dest);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
		pauline_call = linphone_core_get_current_call(pauline->lc);
		if (BC_ASSERT_PTR_NOT_NULL(pauline_call)) {
			const LinphoneCallParams *remote_params = linphone_call_get_remote_params(pauline_call);
			const char *contact_value = linphone_call_params_get_custom_header(remote_params, "Contact");
			BC_ASSERT_PTR_NOT_NULL(contact_value);
			/* assert that the Contact header in the INVITE is not the one coming from the registratation from the
			 * server */
			BC_ASSERT_TRUE(strstr(contact_value, "gr=") == NULL);
			linphone_call_accept(pauline_call);
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

			remote_params = linphone_call_get_remote_params(marie_call);
			contact_value = linphone_call_params_get_custom_header(remote_params, "Contact");
			BC_ASSERT_PTR_NOT_NULL(contact_value);
			/* assert that the Contact header in the 200 OK is not the one coming from the registration with the server
			 */
			BC_ASSERT_TRUE(strstr(contact_value, "gr=") == NULL);

			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
			BC_ASSERT_PTR_NULL(linphone_call_params_get_account(linphone_call_get_current_params(marie_call)));
			BC_ASSERT_PTR_NULL(linphone_call_params_get_account(linphone_call_get_current_params(pauline_call)));

			liblinphone_tester_check_rtcp(marie, pauline);
		}
		end_call(marie, pauline);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		linphone_address_unref(pauline_dest);
	} else ms_warning("Test skipped, no ipv6 available");
}

// Testing the well known port config from linphonerc in call
static void _direct_call_well_known_port(int iptype) {
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *pauline = NULL;

	LinphoneSipTransports pauline_transports;
	LinphoneAddress *pauline_dest = NULL;

	// we use new2 because we do not need to check proxy registration
	// because if we do it, the client will try to register with the well_known port, and the test server doesn't handle
	// it.
	marie = linphone_core_manager_new_with_proxies_check("marie_well_known_port_rc", FALSE);
	pauline = linphone_core_manager_new_with_proxies_check("pauline_well_known_port_rc", FALSE);

	if (iptype == 6) { // if ipv6 wanted
		if (liblinphone_tester_ipv6_available()) {
			pauline_dest = linphone_address_new("sip:[::1];transport=tcp");
		} else {
			ms_warning("Test skipped, no ipv6 available");
		}

	} else { // assumes ipv4
		pauline_dest = linphone_address_new("sip:127.0.0.1;transport=tcp");
		linphone_core_enable_ipv6(marie->lc, FALSE);
		linphone_core_enable_ipv6(pauline->lc, FALSE);
	}
	BC_ASSERT_PTR_NOT_NULL(pauline_dest);
	if (pauline_dest == NULL) goto end;

	linphone_core_set_default_proxy_config(marie->lc, NULL);
	linphone_core_set_default_proxy_config(pauline->lc, NULL);

	linphone_core_get_sip_transports_used(pauline->lc, &pauline_transports);
	linphone_address_set_port(pauline_dest, pauline_transports.tcp_port);

	linphone_core_invite_address(marie->lc, pauline_dest);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_address_unref(pauline_dest);
	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void direct_call_well_known_port_ipv4(void) {
	_direct_call_well_known_port(4);
}

static void direct_call_well_known_port_ipv6(void) {
	_direct_call_well_known_port(6);
}

static void direct_call_with_ipv4_dest_and_ipv6_media_ip(void) {
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *pauline = NULL;

	LinphoneSipTransports pauline_transports;
	LinphoneAddress *pauline_dest = NULL;

	// we use new2 because we do not need to check proxy registration
	// because if we do it, the client will try to register with the well_known port, and the test server doesn't handle
	// it.
	marie = linphone_core_manager_new_with_proxies_check("marie_well_known_port_rc", FALSE);
	pauline = linphone_core_manager_new_with_proxies_check("pauline_well_known_port_rc", FALSE);
	pauline_dest = linphone_address_new("sip:127.0.0.1;transport=tcp");
	linphone_core_enable_ipv6(marie->lc, TRUE);
	linphone_config_set_bool(linphone_core_get_config(marie->lc), "rtp", "prefer_ipv6", TRUE);
	linphone_core_enable_ipv6(pauline->lc, TRUE);
	BC_ASSERT_PTR_NOT_NULL(pauline_dest);
	if (pauline_dest == NULL) goto end;

	linphone_core_set_default_proxy_config(marie->lc, NULL);
	linphone_core_set_default_proxy_config(pauline->lc, NULL);

	linphone_core_get_sip_transports_used(pauline->lc, &pauline_transports);
	linphone_address_set_port(pauline_dest, pauline_transports.tcp_port);

	linphone_core_invite_address(marie->lc, pauline_dest);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (marie_call) {
		_check_call_media_ip_consistency(marie_call);
	}
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		_check_call_media_ip_consistency(pauline_call);
	}

	wait_for_until(marie->lc, pauline->lc, NULL, 5, 500);
	LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_update(marie_call, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));

	if (marie_call) {
		_check_call_media_ip_consistency(marie_call);
	}
	if (pauline_call) {
		_check_call_media_ip_consistency(pauline_call);
	}

	linphone_address_unref(pauline_dest);
	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_outbound_with_multiple_proxy(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check("pauline_tcp_rc", FALSE);

	LinphoneProxyConfig *lpc = NULL;
	LinphoneProxyConfig *registered_lpc = linphone_core_create_proxy_config(marie->lc);

	lpc = linphone_core_get_default_proxy_config(marie->lc);
	linphone_core_set_default_proxy_config(marie->lc, NULL);

	if (!BC_ASSERT_PTR_NOT_NULL(lpc) || !BC_ASSERT_PTR_NOT_NULL(registered_lpc)) return;

	// create new LPC that will successfully register
	LinphoneAddress *identity_address = linphone_address_clone(linphone_proxy_config_get_identity_address(lpc));
	linphone_proxy_config_set_identity_address(registered_lpc, identity_address);
	linphone_address_unref(identity_address);
	linphone_proxy_config_set_server_addr(registered_lpc, linphone_proxy_config_get_addr(lpc));
	const char *route = linphone_proxy_config_get_route(lpc);
	linphone_proxy_config_set_route(registered_lpc, route);
	linphone_proxy_config_enable_register(registered_lpc, TRUE);

	linphone_core_add_proxy_config(marie->lc, registered_lpc);
	linphone_proxy_config_unref(registered_lpc);

	// set first LPC to unreacheable proxy addr
	linphone_proxy_config_edit(lpc);
	linphone_proxy_config_set_server_addr(lpc, "sip:linphone.org:9016;transport=udp");
	linphone_proxy_config_set_route(lpc, "sip:linphone.org:9016;transport=udp;lr");
	linphone_proxy_config_done(lpc);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 10000));

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationProgress, 2, 200));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 10000));

	// calling marie should go through the second proxy config
	BC_ASSERT_TRUE(call(marie, pauline));

	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_outbound_using_secondary_account(void) {
	// Caller
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dual_proxy_rc");
	set_lime_server_and_curve(C25519, marie);
	linphone_core_manager_start(marie, TRUE);

	// Callee
	LinphoneCoreManager *pauline =
	    linphone_core_manager_create(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	set_lime_server_and_curve(C25519, pauline);
	linphone_core_manager_start(pauline, TRUE);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_X3dhUserCreationSuccess, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_X3dhUserCreationSuccess, 2));

	LinphoneAccount *secondary_account = NULL;
	const LinphoneAccount *default_account = linphone_core_get_default_account(marie->lc);
	const bctbx_list_t *accounts = linphone_core_get_account_list(marie->lc);
	for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
		LinphoneAccount *account = (LinphoneAccount *)account_it->data;
		if (account != default_account) {
			secondary_account = account;
		}
	}

	BC_ASSERT_PTR_NOT_NULL(secondary_account);
	if (!secondary_account) {
		goto end;
	}

	const LinphoneAccountParams *secondary_account_params = linphone_account_get_params(secondary_account);
	const LinphoneAddress *secondary_account_identity =
	    linphone_account_params_get_identity_address(secondary_account_params);

	const LinphoneAccountParams *default_account_params = linphone_account_get_params(default_account);
	const LinphoneAddress *default_account_identity =
	    linphone_account_params_get_identity_address(default_account_params);

	BC_ASSERT_FALSE(linphone_address_weak_equal(default_account_identity, secondary_account_identity));

	LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, NULL);
	char *secondary_account_identity_str = linphone_address_as_string(secondary_account_identity);
	linphone_call_params_set_from_header(params, secondary_account_identity_str);
	ms_free(secondary_account_identity_str);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, params));
	linphone_call_params_unref(params);

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (marie_call) {
		const LinphoneCallParams *marie_call_parameters = linphone_call_get_params(marie_call);
		const LinphoneAccount *marie_call_account = linphone_call_params_get_account(marie_call_parameters);
		const LinphoneAccountParams *marie_call_account_params = linphone_account_get_params(marie_call_account);
		const LinphoneAddress *marie_call_account_identity =
		    linphone_account_params_get_identity_address(marie_call_account_params);
		BC_ASSERT_TRUE(linphone_address_weak_equal(marie_call_account_identity, secondary_account_identity));
	}

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		const LinphoneAddress *pauline_call_remote_contact_address =
		    linphone_call_get_remote_contact_address(pauline_call);
		BC_ASSERT_TRUE(linphone_address_weak_equal(pauline_call_remote_contact_address, secondary_account_identity));
	}
	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_outbound_using_different_proxies(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_dual_proxy_rc", FALSE); // Caller
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check("pauline_tcp_rc", FALSE);    // Callee
	int call_count = 0;

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 10000));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2, 10000));

	for (const bctbx_list_t *proxy = linphone_core_get_proxy_config_list(marie->lc); proxy != NULL;
	     proxy = proxy->next) {
		const LinphoneAddress *marieProxyAddress =
		    linphone_proxy_config_get_identity_address((LinphoneProxyConfig *)proxy->data);
		// Set the proxy to be used in call
		linphone_core_set_default_proxy_config(marie->lc, (LinphoneProxyConfig *)proxy->data);
		LinphoneCall *caller = linphone_core_invite(marie->lc, linphone_core_get_identity(pauline->lc));
		call_count++;
		if (BC_ASSERT_PTR_NOT_NULL(caller)) {
			wait_for_until(marie->lc, pauline->lc, NULL, 5, 500);
			const LinphoneCallParams *callerParameters = linphone_call_get_params(caller);
			if (BC_ASSERT_PTR_NOT_NULL(callerParameters)) {
				const LinphoneProxyConfig *callerProxyConfig = linphone_call_params_get_proxy_config(callerParameters);
				if (BC_ASSERT_PTR_NOT_NULL(callerProxyConfig)) {
					const LinphoneAddress *callerAddress =
					    linphone_proxy_config_get_identity_address(callerProxyConfig);
					if (BC_ASSERT_PTR_NOT_NULL(callerAddress)) {
						BC_ASSERT_TRUE(linphone_address_weak_equal(
						    callerAddress,
						    marieProxyAddress)); // Main test : the caller address must use the selected proxy
						BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
						                              &pauline->stat.number_of_LinphoneCallIncomingReceived, call_count,
						                              10000));
					}
					LinphoneCall *callee = linphone_core_get_current_call(pauline->lc);
					BC_ASSERT_PTR_NOT_NULL(callee);
					if (callee) {
						const LinphoneAddress *remoteAddress = linphone_call_get_remote_address(callee);
						BC_ASSERT_TRUE(linphone_address_weak_equal(
						    remoteAddress,
						    marieProxyAddress)); // Main test : callee get a call from the selected proxy of caller
					}
				}
			}
		}
		end_call(marie, pauline);
		wait_for_until(marie->lc, pauline->lc, NULL, 5, 500); // Wait between each session to proper ending calls
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void test_accounts_connections(bool_t accounts_channel_isolation_enabled) {
	// Caller
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dual_proxy_udp_rc");
	LinphoneCallParams *params;
	const bctbx_list_t *accounts;

	linphone_core_remove_supported_tag(marie->lc, "gruu");

	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "accounts_channel_isolation",
	                        (int)accounts_channel_isolation_enabled);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "reject_duplicated_calls", 0);
	linphone_core_set_media_resource_mode(marie->lc, LinphoneSharedMediaResources);
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_manager_start(marie, TRUE);
	accounts = linphone_core_get_account_list(marie->lc);
	LinphoneAccount *first_account = (LinphoneAccount *)accounts->data;
	LinphoneAccount *second_account = (LinphoneAccount *)accounts->next->data;

	const LinphoneAddress *first_contact = linphone_account_get_contact_address(first_account);
	const LinphoneAddress *second_contact = linphone_account_get_contact_address(second_account);

	/* the host part will be the same, there is only one interface */
	BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(first_contact), linphone_address_get_domain(second_contact));
	if (!accounts_channel_isolation_enabled) {
		BC_ASSERT_EQUAL(linphone_address_get_port(first_contact), linphone_address_get_port(second_contact), int, "%i");
	} else {
		/* the ports must be different */
		BC_ASSERT_NOT_EQUAL(linphone_address_get_port(first_contact), linphone_address_get_port(second_contact), int,
		                    "%i");
	}
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_set_account(params, first_account);
	/* make a call from one account to the other */
	LinphoneCall *marie_call = linphone_core_invite_address_with_params(
	    marie->lc, linphone_account_params_get_identity_address(linphone_account_get_params(second_account)), params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	LinphoneCall *marie2_call;
	const bctbx_list_t *calls = linphone_core_get_calls(marie->lc);
	if (calls->data == marie_call) marie2_call = (LinphoneCall *)calls->next->data;
	else marie2_call = (LinphoneCall *)calls->data;
	linphone_call_accept(marie2_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	first_contact = linphone_call_get_remote_contact_address(marie_call);
	second_contact = linphone_call_get_remote_contact_address(marie2_call);

	if (!accounts_channel_isolation_enabled) {
		BC_ASSERT_EQUAL(linphone_address_get_port(first_contact), linphone_address_get_port(second_contact), int, "%i");
	} else {
		/* the ports must be different */
		BC_ASSERT_NOT_EQUAL(linphone_address_get_port(first_contact), linphone_address_get_port(second_contact), int,
		                    "%i");
	}

	linphone_call_terminate(marie_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallReleased, 1));
	linphone_core_manager_destroy(marie);
}

static void two_accounts_use_different_connections(void) {
	test_accounts_connections(0);
	test_accounts_connections(1);
}

static void multiple_answers_call(void) {
	/* Scenario is this: pauline calls marie, which is registered 2 times.
	   Both linphones answer at the same time, and only one should get the
	   call running, the other should be terminated */
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneCoreManager *marie1 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_new("marie_rc");

	LinphoneCall *call1, *call2;

	bctbx_list_t *lcs = bctbx_list_append(NULL, pauline->lc);
	lcs = bctbx_list_append(lcs, marie1->lc);
	lcs = bctbx_list_append(lcs, marie2->lc);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(pauline->lc, marie1->identity));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 5000));

	// marie 1 and 2 answer at the same time
	call1 = linphone_core_get_current_call(marie1->lc);
	call2 = linphone_core_get_current_call(marie2->lc);

	if (BC_ASSERT_PTR_NOT_NULL(call1) && BC_ASSERT_PTR_NOT_NULL(call2)) {
		BC_ASSERT_EQUAL(linphone_call_accept(call1), 0, int, "%d");
		ms_sleep(1); /*sleep to make sure that the 200OK of marie1 reaches the server first*/
		BC_ASSERT_EQUAL(linphone_call_accept(call2), 0, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));

		/*Pauline will send a bye to marie2, as its 200Ok arrived second*/
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallEnd, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallReleased, 1, 10000));

		liblinphone_tester_check_rtcp(pauline, marie1);

		end_call(marie1, pauline);
	}

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	bctbx_list_free(lcs);
}

static void multiple_answers_call_with_media_relay(void) {

	/* Scenario is this: pauline calls marie, which is registered 2 times.
	 *   Both linphones answer at the same time, and only one should get the
	 *   call running, the other should be terminated
	 *  In this test, the server is expected to send ACK-BYE, unlike in multiple_answers_call()*/
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check("pauline_tcp_rc", FALSE);
	LinphoneCoreManager *marie1 = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	LinphoneCoreManager *marie2 = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);

	/* This tests a feature of the proxy (nta_msg_ackbye()) that doesn't work with gruu.
	 * Actually nta_msg_ackbye() is deprecated because it is not the task of the proxy to handle the race conditions of
	 * 200 Ok arriving at the same time. It is the job of the user-agent, see test multiple_answers_call() above.
	 */
	linphone_core_remove_supported_tag(pauline->lc, "gruu");
	linphone_core_remove_supported_tag(marie1->lc, "gruu");
	linphone_core_remove_supported_tag(marie2->lc, "gruu");

	linphone_core_manager_start(pauline, TRUE);
	linphone_core_manager_start(marie1, TRUE);
	linphone_core_manager_start(marie2, TRUE);

	LinphoneCall *call1, *call2;

	bctbx_list_t *lcs = bctbx_list_append(NULL, pauline->lc);
	lcs = bctbx_list_append(lcs, marie1->lc);
	lcs = bctbx_list_append(lcs, marie2->lc);

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie1->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie2->lc, "Natted Linphone", NULL);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(pauline->lc, marie1->identity));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));

	// marie 1 and 2 answer at the same time
	call1 = linphone_core_get_current_call(marie1->lc);
	call2 = linphone_core_get_current_call(marie2->lc);

	if (BC_ASSERT_PTR_NOT_NULL(call1) && BC_ASSERT_PTR_NOT_NULL(call2)) {
		BC_ASSERT_EQUAL(linphone_call_accept(call1), 0, int, "%d");
		ms_sleep(1); /*sleep to make sure that the 200OK of marie1 reaches the server first*/
		BC_ASSERT_EQUAL(linphone_call_accept(call2), 0, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		/*the server will send a bye to marie2, as is 200Ok arrived second*/
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallEnd, 1, 10000));

		end_call(marie1, pauline);
	}

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	bctbx_list_free(lcs);
}

static void call_with_specified_codec_bitrate(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	const char *codec = "opus";
	int rate = 48000;
	int min_bw = 24;
	int max_bw = 50;

#ifdef __arm__
	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) <
	    2) { /*2 opus codec channel + resampler is too much for a single core*/
#ifndef __ANDROID__
		codec = "speex";
		rate = 8000;
		min_bw = 20;
		max_bw = 35;
#else
		BC_PASS("Test requires at least a dual core");
		goto end;
#endif
	}
#endif
	/*Force marie to play from file: if soundcard is used and it is silient, then vbr mode will drop down the bitrate
	 Note that a play file is already set by linphone_core_manager_new() (but not used)*/
	linphone_core_set_use_files(marie->lc, TRUE);

	if (linphone_core_find_payload_type(marie->lc, codec, rate, -1) == NULL) {
		BC_PASS("opus codec not supported, test skipped.");
		goto end;
	}

	disable_all_audio_codecs_except_one(marie->lc, codec, rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec, rate);

	linphone_core_set_payload_type_bitrate(marie->lc, linphone_core_find_payload_type(marie->lc, codec, rate, -1),
	                                       max_bw);
	linphone_core_set_payload_type_bitrate(pauline->lc, linphone_core_find_payload_type(pauline->lc, codec, rate, -1),
	                                       min_bw);

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(marie, pauline);
	/*wait a bit that bitstreams are stabilized*/
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(marie), (int)(min_bw + 5 + min_bw * .1), int, "%i");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie), 10, int,
	                  "%i"); /*check that at least something is received */
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(pauline), (int)(max_bw - 5 - max_bw * .1), int,
	                  "%i");

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_with_maxptime(void) {

	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_create(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	// to make sure bandwidth for ptime 20 is 80kbits/s
	linphone_core_enable_ipv6(marie->lc, FALSE);
	linphone_core_enable_ipv6(pauline->lc, FALSE);
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);

	/*Force marie to play from file: if soundcard is used and it is silient, then vbr mode will drop down the bitrate
	 Note that a play file is already set by linphone_core_manager_new() (but not used)*/
	linphone_core_set_use_files(marie->lc, TRUE);

	disable_all_audio_codecs_except_one(marie->lc, "PCMA", 8000);
	disable_all_audio_codecs_except_one(pauline->lc, "PCMA", 8000);
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_add_custom_sdp_media_attribute(marie_params, LinphoneStreamTypeAudio, "maxptime", "40");
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_add_custom_sdp_media_attribute(pauline_params, LinphoneStreamTypeAudio, "maxptime", "40");

	linphone_core_set_upload_ptime(pauline->lc, 100);
	linphone_core_set_upload_ptime(marie->lc, 100);

	BC_ASSERT_TRUE((call_ok = call_with_params(pauline, marie, pauline_params, marie_params)));
	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(marie, pauline);
	/*wait a bit that bitstreams are stabilized*/
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	// network-birate = ((codec-birate*ptime/8) + RTP header + UDP header + IP header)*8/ptime;

	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_up_bw(pauline), 80, int, "%i");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_up_bw(pauline), 70, int,
	                  "%i"); // without maxptime=40, it should be 67
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_up_bw(marie), 80, int, "%i");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_up_bw(marie), 70, int,
	                  "%i"); // without maxptime=40, it should be 67

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void disable_all_codecs(const bctbx_list_t *elem, LinphoneCoreManager *call) {

	PayloadType *pt;

	for (; elem != NULL; elem = elem->next) {
		pt = (PayloadType *)elem->data;
		linphone_core_enable_payload_type(call->lc, pt, FALSE);
	}
}
/***
 Disable all audio codecs , sends an INVITE with RTP port 0 and payload 0.
 Wait for SIP  488 unacceptable.
 ***/
static void call_with_no_audio_codec(void) {

	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTcp) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call;

	const bctbx_list_t *elem = linphone_core_get_audio_codecs(caller->lc);

	disable_all_codecs(elem, caller);

	out_call = linphone_core_invite_address(caller->lc, callee->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallOutgoingInit, 1));

	BC_ASSERT_TRUE(wait_for_until(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void call_with_no_active_stream_on_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);

	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (marie_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
	}
	LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
	}

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", TRUE);

	stats initial_pauline_stat = pauline->stat;
	stats initial_marie_stat = marie->stat;
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_update(marie_call, marie_params);
	linphone_call_params_unref(marie_params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1));

	int pauline_defer_update =
	    !!linphone_config_get_int(linphone_core_get_config(pauline->lc), "sip", "defer_update_default", FALSE);
	BC_ASSERT_TRUE(pauline_defer_update);
	if (pauline_defer_update == TRUE) {
		LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);
		linphone_call_params_enable_audio(pauline_params, FALSE);
		linphone_call_params_enable_video(pauline_params, FALSE);
		linphone_call_accept_update(pauline_call, pauline_params);
		linphone_call_params_unref(pauline_params);
	}
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating,
	                        initial_marie_stat.number_of_LinphoneCallUpdating + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));

	if (marie_call) {
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_rparams));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_cparams));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
	}
	if (pauline_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_lparams));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_cparams));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
	}

	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_call_compatibility_mode(void) {
	char route[256];
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCore *lc_marie = marie->lc;
	LinphoneCore *lc_pauline = pauline->lc;
	stats *stat_marie = &marie->stat;
	stats *stat_pauline = &pauline->stat;
	LinphoneProxyConfig *proxy;
	LinphoneAddress *identity;
	LinphoneAddress *proxy_address;
	char *tmp;
	LinphoneSipTransports transport;

	proxy = linphone_core_get_default_proxy_config(lc_marie);
	BC_ASSERT_PTR_NOT_NULL(proxy);
	identity = linphone_address_clone(linphone_proxy_config_get_identity_address(proxy));

	proxy_address = linphone_address_new(linphone_proxy_config_get_addr(proxy));
	linphone_address_clean(proxy_address);
	tmp = linphone_address_as_string_uri_only(proxy_address);
	linphone_proxy_config_edit(proxy);
	linphone_proxy_config_set_server_addr(proxy, tmp);
	sprintf(route, "sip:%s", test_route);
	linphone_proxy_config_set_route(proxy, route);
	linphone_proxy_config_done(proxy);
	ms_free(tmp);
	linphone_address_unref(proxy_address);
	linphone_core_get_sip_transports(lc_marie, &transport);
	transport.udp_port = 0;
	transport.tls_port = 0;
	transport.dtls_port = 0;
	/*only keep tcp*/
	linphone_core_set_sip_transports(lc_marie, &transport);
	stat_marie->number_of_LinphoneRegistrationOk = 0;

	BC_ASSERT_TRUE(wait_for(lc_marie, lc_marie, &stat_marie->number_of_LinphoneRegistrationOk, 1));

	linphone_core_invite_address(lc_marie, pauline->identity);

	BC_ASSERT_TRUE(wait_for(lc_pauline, lc_marie, &stat_pauline->number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(linphone_core_is_incoming_invite_pending(lc_pauline));
	BC_ASSERT_EQUAL(stat_marie->number_of_LinphoneCallOutgoingProgress, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for(lc_pauline, lc_marie, &stat_marie->number_of_LinphoneCallOutgoingRinging, 1));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call_remote_address(lc_pauline));
	if (linphone_core_get_current_call_remote_address(lc_pauline)) {
		BC_ASSERT_TRUE(
		    linphone_address_weak_equal(identity, linphone_core_get_current_call_remote_address(lc_pauline)));

		linphone_call_accept(linphone_core_get_current_call(lc_pauline));

		BC_ASSERT_TRUE(wait_for(lc_pauline, lc_marie, &stat_pauline->number_of_LinphoneCallConnected, 1));
		BC_ASSERT_TRUE(wait_for(lc_pauline, lc_marie, &stat_marie->number_of_LinphoneCallConnected, 1));
		BC_ASSERT_TRUE(wait_for(lc_pauline, lc_marie, &stat_pauline->number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(lc_pauline, lc_marie, &stat_marie->number_of_LinphoneCallStreamsRunning, 1));

		wait_for(lc_pauline, lc_marie, &stat_marie->number_of_LinphoneCallStreamsRunning, 3);
		end_call(pauline, marie);
	}

	linphone_address_unref(identity);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

// This test is used to write down reasons on call and in logs after declining with reason.
// Call states like End/Release may depends to the reason too.
// Check theses cases to know if it is coherent or if the behaviour should change.
static void call_declined_with_reasons(void) {
	for (int reasonToTest = LinphoneReasonNone; reasonToTest <= LinphoneReasonTransferred; ++reasonToTest) {
		// create core from start on each iteration to avoid state conflicting between calls.
		LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *caller_mgr =
		    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

		LinphoneCall *in_call = NULL;
		LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);
		//------------------------------------------------------------------
		//						Reasons Checklist
		//------------------------------------------------------------------
		int incall_reason_target = reasonToTest, incall_log_status_target = reasonToTest;
		int outcall_reason_target = reasonToTest, outcall_log_status_target = reasonToTest;
		bool_t caller_end = TRUE;
		bool_t caller_release = TRUE;
		switch (reasonToTest) {
			case LinphoneReasonNone:
				caller_end = FALSE;
				incall_reason_target = LinphoneReasonDeclined;
				incall_log_status_target = LinphoneReasonDeclined;
				break;
			case LinphoneReasonNoResponse:
				caller_end = FALSE;
				outcall_reason_target = LinphoneReasonUnknown;
				break;
			case LinphoneReasonForbidden:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonDeclined: // Default
				break;
			case LinphoneReasonNotFound:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonNotAnswered:
				caller_end = FALSE;
				incall_reason_target = LinphoneReasonNone;
				outcall_reason_target = LinphoneReasonUnknown;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonBusy:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonUnsupportedContent:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonBadEvent:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonIOError:
				caller_end = FALSE;
				caller_release = FALSE;
				outcall_reason_target = LinphoneReasonNone;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonDoNotDisturb:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonBusy;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonUnauthorized:
				caller_end = FALSE;
				outcall_reason_target = LinphoneReasonNone;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonNotAcceptable:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonNoMatch:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonMovedPermanently:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonGone:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonTemporarilyUnavailable:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonAddressIncomplete:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonNotImplemented:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonBadGateway:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonSessionIntervalTooSmall:
				caller_end = FALSE;
				outcall_reason_target = LinphoneReasonUnknown;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonServerTimeout:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonUnknown:
				caller_end = FALSE;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
			case LinphoneReasonTransferred:
				caller_end = FALSE;
				outcall_reason_target = LinphoneReasonUnknown;
				incall_log_status_target = LinphoneReasonNoResponse;
				outcall_log_status_target = LinphoneReasonNoResponse;
				break;
		} // No default because we want to have all use cases from Reasons
		  //------------------------------------------------------------------
		  //						Call test
		  //------------------------------------------------------------------
		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
		BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(callee_mgr->lc));

		linphone_call_ref(out_call);
		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingRinging, 1));
		BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(callee_mgr->lc));
		if (in_call) {
			linphone_call_ref(in_call);
			linphone_call_decline(in_call, reasonToTest);

			BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEnd, 1));
			BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallEnd,
			                        (caller_end ? 1 : 0)));
			BC_ASSERT_TRUE(
			    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallReleased, 1));
			BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased,
			                        (caller_release ? 1 : 0)));

			BC_ASSERT_EQUAL(linphone_call_get_reason(in_call), incall_reason_target, int, "%d");
			BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), outcall_reason_target, int, "%d");
			BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(in_call)), incall_log_status_target,
			                int, "%d");
			BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(out_call)),
			                outcall_log_status_target, int, "%d");

			linphone_call_unref(in_call);
		}
		linphone_call_unref(out_call);
		wait_for_until(caller_mgr->lc, callee_mgr->lc, NULL, 0, 500);
		linphone_core_manager_destroy(callee_mgr);
		linphone_core_manager_destroy(caller_mgr);
	}
}

static void terminate_call_with_error(void) {
	LinphoneCall *call_callee;
	LinphoneErrorInfo *ei;
	const LinphoneErrorInfo *rei = NULL;
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const LinphoneCoreToneManagerStats *caller_tone_mgr_stats = linphone_core_get_tone_manager_stats(caller_mgr->lc);

	LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);

	linphone_call_ref(out_call);
	ei = linphone_error_info_new();
	linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 200, "Call refused for security reason", NULL);

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingProgress, 1));

	call_callee = linphone_core_get_current_call(callee_mgr->lc);
	linphone_call_ref(call_callee);
	BC_ASSERT_PTR_NOT_NULL(call_callee);

	BC_ASSERT_EQUAL(linphone_core_accept_call(callee_mgr->lc, call_callee), 0, int, "%d");
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallConnected, 1));

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_PTR_NOT_NULL(ei);
	if (ei) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(ei), 200, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(ei));
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(ei), "Call refused for security reason");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(ei), "SIP");
	}
	linphone_call_terminate_with_error_info(out_call, ei);
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_EQUAL(caller_tone_mgr_stats->number_of_startNamedTone, caller_mgr->stat.number_of_LinphoneCallEnd, int,
	                "%d");

	rei = linphone_call_get_error_info(call_callee);
	BC_ASSERT_PTR_NOT_NULL(rei);
	if (rei) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(rei), 200, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(rei));
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(rei), "Call refused for security reason");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(ei), "SIP");
	}
	BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(call_callee)),
	                LinphoneCallAcceptedElsewhere, int, "%d");

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased, 1));

	linphone_error_info_unref(ei);
	linphone_call_unref(out_call);
	linphone_call_unref(call_callee);
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
}

static void cancel_call_with_error(void) {
	LinphoneCall *call_callee;
	LinphoneErrorInfo *ei;
	const LinphoneErrorInfo *rei = NULL;
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);

	linphone_call_ref(out_call);
	ei = linphone_error_info_new();
	linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 600, "Call has been cancelled", NULL);

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingProgress, 1));

	call_callee = linphone_core_get_current_call(callee_mgr->lc);
	linphone_call_ref(call_callee);

	BC_ASSERT_PTR_NOT_NULL(call_callee);
	BC_ASSERT_PTR_NOT_NULL(ei);
	if (ei) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(ei), 600, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(ei));
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(ei), "Call has been cancelled");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(ei), "SIP");
	}
	linphone_call_terminate_with_error_info(out_call, ei);
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallReleased, 1));

	rei = linphone_call_get_error_info(call_callee);
	BC_ASSERT_PTR_NOT_NULL(rei);
	if (rei) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(rei), 600, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(rei));
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(rei), "Call has been cancelled");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(rei), "SIP");
	}
	BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(call_callee)),
	                LinphoneCallDeclinedElsewhere, int, "%d");

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased, 1));

	linphone_error_info_unref(ei);
	linphone_call_unref(out_call);
	linphone_call_unref(call_callee);
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
}

static void cancel_other_device_after_accept(void) {
	LinphoneCall *call_callee = NULL;
	LinphoneCall *call_callee_2 = NULL;
	const LinphoneErrorInfo *rei = NULL;
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *callee_mgr_2 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);
	linphone_call_ref(out_call);

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingProgress, 1));
	call_callee = linphone_core_get_current_call(callee_mgr->lc);
	if (BC_ASSERT_PTR_NOT_NULL(call_callee)) {

		linphone_call_ref(call_callee);

		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr_2->lc, &callee_mgr_2->stat.number_of_LinphoneCallIncomingReceived, 1));
		call_callee_2 = linphone_core_get_current_call(callee_mgr_2->lc);
		linphone_call_ref(call_callee_2);
		BC_ASSERT_PTR_NOT_NULL(call_callee_2);

		BC_ASSERT_EQUAL(linphone_call_accept(call_callee), 0, int, "%d");
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallConnected, 1));
		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr_2->lc, &callee_mgr_2->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr_2->lc, &callee_mgr_2->stat.number_of_LinphoneCallReleased, 1));

		wait_for_until(caller_mgr->lc, callee_mgr_2->lc, NULL, 0, 500);

		rei = linphone_call_get_error_info(call_callee_2);
		BC_ASSERT_PTR_NOT_NULL(rei);
		if (rei) {
			BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(rei), 200, int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(rei));
			BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(rei), "Call completed elsewhere");
			BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(rei), "SIP");
		}
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(call_callee_2)),
		                LinphoneCallAcceptedElsewhere, int, "%d");
	}
	linphone_call_terminate(out_call);
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased, 1));

	if (out_call) linphone_call_unref(out_call);
	if (call_callee) linphone_call_unref(call_callee);
	if (call_callee_2) linphone_call_unref(call_callee_2);
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(callee_mgr_2);
	linphone_core_manager_destroy(caller_mgr);
}

static void cancel_other_device_after_decline(void) {
	LinphoneCall *call_callee = NULL;
	LinphoneCall *call_callee_2 = NULL;
	const LinphoneErrorInfo *rei = NULL;
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *callee_mgr_2 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);
	linphone_call_ref(out_call);

	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingProgress, 1));

	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr->lc)->number_of_startRingtone, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc,
	                        (int *)&linphone_core_get_tone_manager_stats(caller_mgr->lc)->number_of_startRingbackTone,
	                        1));

	call_callee = linphone_core_get_current_call(callee_mgr->lc);
	if (BC_ASSERT_PTR_NOT_NULL(call_callee)) {
		linphone_call_ref(call_callee);

		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr_2->lc, &callee_mgr_2->stat.number_of_LinphoneCallIncomingReceived, 1));
		call_callee_2 = linphone_core_get_current_call(callee_mgr_2->lc);
		linphone_call_ref(call_callee_2);
		BC_ASSERT_PTR_NOT_NULL(call_callee_2);
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr_2->lc)->number_of_startRingtone, 1, int, "%d");

		BC_ASSERT_EQUAL(linphone_call_decline(call_callee, LinphoneReasonDeclined), 0, int, "%d");
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(caller_mgr->lc)->number_of_stopRingbackTone, 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(caller_mgr->lc)->number_of_startNamedTone, 1, int, "%d");
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased, 1));

		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr->lc)->number_of_stopRingtone, 1, int, "%d");
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallReleased, 1));

		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr_2->lc, &callee_mgr_2->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr_2->lc)->number_of_stopRingtone, 1, int, "%d");
		BC_ASSERT_TRUE(
		    wait_for(caller_mgr->lc, callee_mgr_2->lc, &callee_mgr_2->stat.number_of_LinphoneCallReleased, 1));

		rei = linphone_call_get_error_info(call_callee_2);
		BC_ASSERT_PTR_NOT_NULL(rei);
		if (rei) {
			BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(rei), 600, int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(rei));
			BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(rei), "Busy Everywhere");
			BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(rei), "SIP");
		}
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(call_callee_2)),
		                LinphoneCallDeclinedElsewhere, int, "%d");

		// There is currently no tone for LinphoneReasonDeclined, so there is no call to stopTone
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(caller_mgr->lc)->number_of_stopTone, 0, int, "%d");
	}
	if (out_call) linphone_call_unref(out_call);
	if (call_callee) linphone_call_unref(call_callee);
	if (call_callee_2) linphone_call_unref(call_callee_2);
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(callee_mgr_2);
	linphone_core_manager_destroy(caller_mgr);
}

static void cancelled_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));

	linphone_call_terminate(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	// BC_ASSERT_EQUAL(linphone_call_get_reason(out_call),LinphoneReasonCanceled, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void disable_all_audio_codecs_except_one(LinphoneCore *lc, const char *mime, int rate) {
	const bctbx_list_t *elem = linphone_core_get_audio_codecs(lc);
	PayloadType *pt;

	for (; elem != NULL; elem = elem->next) {
		pt = (PayloadType *)elem->data;
		linphone_core_enable_payload_type(lc, pt, FALSE);
	}
	pt = linphone_core_find_payload_type(lc, mime, rate, -1);
	if (BC_ASSERT_PTR_NOT_NULL(pt)) {
		linphone_core_enable_payload_type(lc, pt, TRUE);
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void disable_all_video_codecs_except_one(LinphoneCore *lc, const char *mime) {
#ifdef VIDEO_ENABLED
	const bctbx_list_t *codecs = linphone_core_get_video_codecs(lc);
	const bctbx_list_t *it = NULL;
	PayloadType *pt = NULL;

	for (it = codecs; it != NULL; it = it->next) {
		linphone_core_enable_payload_type(lc, (PayloadType *)it->data, FALSE);
	}
	pt = linphone_core_find_payload_type(lc, mime, -1, -1);
	if (BC_ASSERT_PTR_NOT_NULL(pt)) {
		linphone_core_enable_payload_type(lc, pt, TRUE);
	}
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

static void call_with_dns_time_out(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
	LinphoneSipTransports transport = {9773, 0, 0, 0};

	linphone_core_set_sip_transports(marie->lc, &transport);
	linphone_core_iterate(marie->lc);
	sal_set_dns_timeout(linphone_core_get_sal(marie->lc), 0);
	linphone_core_invite(marie->lc, "\"t\x8et\x8e\" <sip:toto@toto.com>"); /*just to use non ascii values*/

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallError, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallReleased, 1));
	linphone_core_manager_destroy(marie);
}

static void udp_call_early_cancelled_with_sal_error(void) {
	LinphoneCoreManager *laure;
	laure = linphone_core_manager_new("laure_rc_udp");
	sal_set_send_error(linphone_core_get_sal(laure->lc), -1);
	linphone_core_invite(laure->lc, "toto@sip.example.org");
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallOutgoingProgress, 1));
	linphone_core_terminate_all_calls(laure->lc);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1, 50000));
	linphone_core_manager_destroy(laure);
}

static void udp_call_terminated_with_sal_error(void) {
	LinphoneCoreManager *laure;
	laure = linphone_core_manager_new("laure_rc_udp");
	sal_set_send_error(linphone_core_get_sal(laure->lc), -1);
	linphone_core_invite(laure->lc, "toto@sip.example.org");
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallError, 1, 50000));
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1));
	linphone_core_manager_destroy(laure);
}

static void early_cancelled_call(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneSipTransports pauline_transports;
	LinphoneAddress *pauline_dest = linphone_address_new("sip:127.0.0.1;transport=tcp");
	LinphoneCall *out_call;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_default_proxy_config(marie->lc, NULL);
	linphone_core_set_default_proxy_config(pauline->lc, NULL);

	linphone_core_get_sip_transports_used(pauline->lc, &pauline_transports);
	linphone_address_set_port(pauline_dest, pauline_transports.tcp_port);

	out_call = linphone_core_invite_address(marie->lc, pauline_dest);
	linphone_address_unref(pauline_dest);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	const char *callID = linphone_call_log_get_call_id(linphone_call_get_call_log(out_call));
	BC_ASSERT_PTR_NOT_NULL(callID);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	/*
	 * Wait for pauline to receive the call. Since the call is almost immediately notified, pauline hasn't received
	 * yet the provisionnal response.
	 * In the following loop, marie is not scheduled after the call is received by pauline.
	 */
	int i = 0;
	while (i < 300) {
		linphone_core_iterate(pauline->lc);
		if (pauline->stat.number_of_LinphoneCallIncomingReceived == 0) {
			linphone_core_iterate(marie->lc);
		} else break;
		ms_usleep(20000);
		i++;
	}

	BC_ASSERT_TRUE(pauline->stat.number_of_LinphoneCallIncomingReceived == 1);
	/* This asserts that the 180 is not received: */
	BC_ASSERT_TRUE(linphone_call_get_state(out_call) == LinphoneCallOutgoingProgress);
	/*
	 * Immediately terminate the call. Since no response is received, no CANCEL can be sent.
	 * It will ring at Pauline's side.
	 * We want to verify that the CANCEL is automatically sent when the provisionnal response from Marie is received.
	 */
	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	/* now the CANCEL should have been sent and the the call at marie's side should terminate*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_called_without_any_response_base(bool_t with_network_switch) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check("pauline_rc", FALSE);

	LinphoneCall *out_call = linphone_core_invite_address(pauline->lc, marie->identity);

	/* we don't schedule marie, because we want NO response at all.*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));

	/* Wait a bit. */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, NULL, 0, 2000));

	if (with_network_switch) {
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		linphone_core_set_network_reachable(pauline->lc, TRUE);
		BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationProgress, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 2));
	}
	/* Cancel the call. */
	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneCallEnd, 1));

	/* Now schedule marie. */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	/* The call should end shortly because the proxy will cancel it*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_called_without_any_response(void) {
	call_called_without_any_response_base(FALSE);
}

static void call_called_without_any_response_with_network_switch(void) {
	call_called_without_any_response_base(TRUE);
}

static void cancelled_ringing_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const bctbx_list_t *call_history;
	LinphoneCall *out_call;

	BC_ASSERT_EQUAL(linphone_core_get_missed_calls_count(pauline->lc), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_account_get_missed_calls_count(linphone_core_get_default_account(pauline->lc)), 0, int,
	                "%d");

	BC_ASSERT_EQUAL(linphone_core_get_missed_calls_count(marie->lc), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_account_get_missed_calls_count(linphone_core_get_default_account(marie->lc)), 0, int,
	                "%d");

	out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));

	/* Wait a bit. */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, NULL, 0, 2000));

	// Make sure the call duration is increased during the ringing
	BC_ASSERT_GREATER_STRICT(linphone_call_get_duration(out_call), 0, int, "%d");

	LinphoneCall *in_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(in_call);
	if (in_call) {
		// Make sure the call duration is increased during the ringing
		BC_ASSERT_GREATER_STRICT(linphone_call_get_duration(in_call), 0, int, "%d");
	}

	linphone_call_terminate(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd, 1, int, "%d");

	call_history = linphone_core_get_call_history(marie->lc);
	if (BC_ASSERT_PTR_NOT_NULL(call_history)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(call_history), 1, int, "%i");
		LinphoneCallLog *call_log = (LinphoneCallLog *)bctbx_list_get_data(call_history);
		BC_ASSERT_EQUAL(linphone_call_log_get_status(call_log), LinphoneCallMissed, LinphoneCallStatus, "%i");
		// Make sure the call log duration is 0 as the call wasn't connected
		BC_ASSERT_EQUAL(linphone_call_log_get_duration(call_log), 0, int, "%d");
	}

	BC_ASSERT_EQUAL(linphone_core_get_missed_calls_count(pauline->lc), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_account_get_missed_calls_count(linphone_core_get_default_account(pauline->lc)), 0, int,
	                "%d");

	BC_ASSERT_EQUAL(linphone_core_get_missed_calls_count(marie->lc), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_account_get_missed_calls_count(linphone_core_get_default_account(marie->lc)), 1, int,
	                "%d");

	linphone_account_reset_missed_calls_count(linphone_core_get_default_account(marie->lc));

	BC_ASSERT_EQUAL(linphone_core_get_missed_calls_count(marie->lc), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_account_get_missed_calls_count(linphone_core_get_default_account(marie->lc)), 0, int,
	                "%d");

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_declined_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallLog *out_call_log;
	LinphoneCall *out_call;

	linphone_core_set_max_calls(marie->lc, 0);
	out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(out_call);

	/*wait until flexisip transfers the busy...*/
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallError, 1, 33000));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallError, 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonBusy, int, "%d");
	if (bctbx_list_size(linphone_core_get_call_logs(pauline->lc)) > 0) {
		BC_ASSERT_PTR_NOT_NULL(out_call_log = (LinphoneCallLog *)(linphone_core_get_call_logs(pauline->lc)->data));
		BC_ASSERT_EQUAL(linphone_call_log_get_status(out_call_log), LinphoneCallAborted, int, "%d");
	}
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_busy_when_calling_self(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, marie->identity);
	linphone_call_ref(out_call);

	/*wait until flexisip transfers the busy...*/
	BC_ASSERT_TRUE(wait_for_until(marie->lc, marie->lc, &marie->stat.number_of_LinphoneCallError, 1, 33000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallError, 1, int, "%d");

	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonBusy, int, "%d");
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
}

static void call_declined_with_error(void) {
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *in_call = NULL;
	LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);
	LinphoneFactory *factory = linphone_factory_get();
	const LinphoneErrorInfo *rcvd_ei;
	const LinphoneErrorInfo *sub_rcvd_ei;

	LinphoneErrorInfo *ei = linphone_factory_create_error_info(factory);
	LinphoneErrorInfo *reason_ei = linphone_factory_create_error_info(factory);

	linphone_error_info_set(ei, "SIP", LinphoneReasonDeclined, 603, "Decline", NULL); // ordre des arguments à vérifier
	linphone_error_info_set(reason_ei, "hardware", LinphoneReasonDeclined, 66, "J'ai plus de batterie", NULL);

	linphone_error_info_set_sub_error_info(ei, reason_ei);

	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(callee_mgr->lc));

	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(callee_mgr->lc));
	if (in_call) {
		linphone_call_ref(in_call);
		linphone_call_decline_with_error_info(in_call, ei);

		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallEnd, 1));

		rcvd_ei = linphone_call_get_error_info(out_call);
		sub_rcvd_ei = linphone_error_info_get_sub_error_info(rcvd_ei);

		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(rcvd_ei), "Decline");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(rcvd_ei), "SIP");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(sub_rcvd_ei), "J'ai plus de batterie");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(sub_rcvd_ei), "hardware");

		BC_ASSERT_EQUAL(linphone_call_get_reason(in_call), LinphoneReasonDeclined, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(in_call)), LinphoneCallDeclined, int,
		                "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonDeclined, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(out_call)), LinphoneCallDeclined, int,
		                "%d");

		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased, 1));
		linphone_call_unref(in_call);
	}
	linphone_call_unref(out_call);
	linphone_error_info_unref(reason_ei);
	linphone_error_info_unref(ei);

	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
}

static void call_declined_with_retry_after(void) {
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *in_call = NULL;
	LinphoneCall *out_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity);
	LinphoneFactory *factory = linphone_factory_get();
	const LinphoneErrorInfo *rcvd_ei;
	const LinphoneErrorInfo *sub_rcvd_ei;

	LinphoneErrorInfo *ei = linphone_factory_create_error_info(factory);
	LinphoneErrorInfo *reason_ei = linphone_factory_create_error_info(factory);

	linphone_error_info_set(ei, "SIP", LinphoneReasonDeclined, 603, "Decline", NULL); // ordre des arguments à vérifier
	linphone_error_info_set(reason_ei, "hardware", LinphoneReasonDeclined, 66, "J'ai plus de batterie", NULL);

	BC_ASSERT_TRUE(linphone_error_code_is_retry_after(linphone_error_info_get_protocol_code(ei)));
	linphone_error_info_set_retry_after(ei, 120);

	linphone_error_info_set_sub_error_info(ei, reason_ei);

	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(callee_mgr->lc));

	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(
	    wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(callee_mgr->lc));
	if (in_call) {
		linphone_call_ref(in_call);
		linphone_call_decline_with_error_info(in_call, ei);

		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallEnd, 1));

		rcvd_ei = linphone_call_get_error_info(out_call);
		sub_rcvd_ei = linphone_error_info_get_sub_error_info(rcvd_ei);

		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(rcvd_ei), "Decline");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(rcvd_ei), "SIP");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_phrase(sub_rcvd_ei), "J'ai plus de batterie");
		BC_ASSERT_STRING_EQUAL(linphone_error_info_get_protocol(sub_rcvd_ei), "hardware");
		BC_ASSERT_GREATER(linphone_error_info_get_retry_after(rcvd_ei), 0, int, "%d");

		BC_ASSERT_EQUAL(linphone_call_get_reason(in_call), LinphoneReasonDeclined, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(in_call)), LinphoneCallDeclined, int,
		                "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonDeclined, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(out_call)), LinphoneCallDeclined, int,
		                "%d");

		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_TRUE(wait_for(caller_mgr->lc, callee_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallReleased, 1));
		linphone_call_unref(in_call);
	}
	linphone_call_unref(out_call);
	linphone_error_info_unref(reason_ei);
	linphone_error_info_unref(ei);

	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
}

static void call_declined_base(bool_t use_timeout, bool_t use_earlymedia, bool_t request_timeout) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneReason reason =
	    use_timeout ? LinphoneReasonBusy : (request_timeout ? LinphoneReasonNotAnswered : LinphoneReasonDeclined);

	LinphoneCall *in_call;
	LinphoneCall *out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	const LinphoneCoreToneManagerStats *marieToneManagerStats = linphone_core_get_tone_manager_stats(marie->lc);
	const LinphoneCoreToneManagerStats *paulineToneManagerStats = linphone_core_get_tone_manager_stats(pauline->lc);
	if (use_timeout) linphone_core_set_inc_timeout(marie->lc, 1);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(in_call = linphone_core_get_current_call(marie->lc));
	if (in_call) {
		linphone_call_ref(in_call);
		if (use_earlymedia) linphone_call_accept_early_media(in_call);

		BC_ASSERT_EQUAL(marieToneManagerStats->number_of_startRingtone, 1, int, "%d");
		BC_ASSERT_TRUE(
		    wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_startRingbackTone, 1));
		if (!use_timeout && !request_timeout) linphone_call_terminate(in_call);

		if (request_timeout)
			wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1,
			               2 * liblinphone_tester_sip_timeout);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_EQUAL(marieToneManagerStats->number_of_stopRingtone, 1, int, "%d");

		if (request_timeout || use_timeout)
			BC_ASSERT_EQUAL(marieToneManagerStats->number_of_startNamedTone, 0, int, "%d");

		BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_stopRingbackTone, 1, int, "%d");
		BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_startNamedTone, 1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd, 1, int, "%d");
		BC_ASSERT_EQUAL((use_timeout || request_timeout) ? pauline->stat.number_of_LinphoneCallError
		                                                 : pauline->stat.number_of_LinphoneCallEnd,
		                1, int, "%d");
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_stopTone, 1));
		BC_ASSERT_EQUAL(linphone_call_get_reason(in_call), reason, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(in_call)),
		                (use_timeout || request_timeout) ? LinphoneCallMissed : LinphoneCallDeclined, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), reason, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(linphone_call_get_call_log(out_call)),
		                (use_timeout || request_timeout) ? LinphoneCallAborted : LinphoneCallDeclined, int, "%d");

		if (use_timeout)
			BC_ASSERT_EQUAL(linphone_call_log_get_duration(linphone_call_get_call_log(out_call)), 0, int, "%d");
		linphone_call_unref(in_call);
	}

	linphone_call_unref(out_call);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_declined(void) {
	call_declined_base(FALSE, FALSE, FALSE);
}

static void call_declined_in_early_media(void) {
	call_declined_base(FALSE, TRUE, FALSE);
}

static void call_declined_on_timeout(void) {
	call_declined_base(TRUE, FALSE, FALSE);
}

static void call_declined_on_timeout_in_early_media(void) {
	call_declined_base(TRUE, TRUE, FALSE);
}

static void call_cancelled_on_request_timeout_in_early_media(void) {
	call_declined_base(FALSE, TRUE, TRUE);
}

static void call_terminated_by_caller(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	BC_ASSERT_TRUE(call(pauline, marie));

	end_call(pauline, marie);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_terminated_by_nortp_timeout_base(bool_t on_hold) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_params = NULL;
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	int nortp_timeout = 5;
	int nortp_on_hold_timeout = 8;
	int test_timeout = (nortp_timeout + 1) * 1000;
	if (on_hold) {
		test_timeout = (nortp_on_hold_timeout + 1) * 1000;
		linphone_core_set_nortp_onhold_timeout(marie->lc, nortp_on_hold_timeout);
	} else {
		linphone_core_set_nortp_timeout(marie->lc, nortp_timeout);
	}

	BC_ASSERT_TRUE((call_ok = call_with_params(pauline, marie, pauline_params, NULL)));
	if (!call_ok) goto end;

	LinphoneCall *marie_call = NULL;
	marie_call = linphone_core_get_current_call(marie->lc);

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);
	if (on_hold) {
		/*marie in pause*/
		linphone_call_pause(marie_call);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 1, 10000));
	}

	/*marie looses the network and pauline terminates the call, no rtp sent*/
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));

	/*call terminates after nortp timeout has been reached */
	wait_for_list(lcs, NULL, 0, test_timeout);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_EQUAL(linphone_call_get_reason(marie_call), LinphoneReasonIOError, int, "%d");

end:
	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
	}
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_terminated_by_nortp_timeout(void) {
	call_terminated_by_nortp_timeout_base(FALSE);
}

static void call_terminated_by_nortp_timeout_on_hold(void) {
	call_terminated_by_nortp_timeout_base(TRUE);
}

static void call_with_no_sdp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);

	BC_ASSERT_TRUE(call(marie, pauline));

	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_lime(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_sips_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_create(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	set_lime_server_and_curve(C25519, pauline);
	linphone_core_manager_start(pauline, TRUE);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_X3dhUserCreationSuccess, 1));

	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);

	BC_ASSERT_TRUE(call(marie, pauline));

	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_ack_without_sdp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call;

	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);

	linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	call = linphone_core_get_current_call(pauline->lc);
	if (call) {
		sal_call_set_sdp_handling(
		    linphone_call_get_op_as_sal_op(call),
		    SalOpSDPSimulateError); /*this will have the effect that the SDP received in the ACK will be ignored*/
		linphone_call_accept(call);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallError, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_with_update(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "sip", "inactive_audio_on_pause", 1);
	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);

	// Invite sendonly Audio (Marie pause Pauline)
	ms_message("CONTEXT: Marie sends a re-INVITE with the audio stream direction set to sendonly to pause the call");
	int marieStreamsRunning = marie->stat.number_of_LinphoneCallStreamsRunning;
	LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendOnly);
	linphone_call_update(marie_call, params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(
	    wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, (marieStreamsRunning + 1)));

	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, NULL, 0, 2000));

	check_local_desc_stream(marie_call);

	const LinphoneCallParams *pauline_params = linphone_call_get_current_params(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(pauline_params);
	if (pauline_params) {
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(pauline_params));
		BC_ASSERT_EQUAL(linphone_call_params_get_audio_direction(pauline_params), (int)LinphoneMediaDirectionInactive,
		                int, "%d");
	}

	// Invite sendrecv Audio (Marie resumes call with Pauline)
	ms_message("CONTEXT: Marie resumes the call");
	linphone_core_enable_sdp_200_ack(marie->lc, FALSE);
	marieStreamsRunning = marie->stat.number_of_LinphoneCallStreamsRunning;
	int paulineStreamsRunning = pauline->stat.number_of_LinphoneCallStreamsRunning;
	params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendRecv);
	linphone_call_update(marie_call, params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 2));
	BC_ASSERT_TRUE(
	    wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, (marieStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (paulineStreamsRunning + 1)));
	linphone_call_params_unref(params);

	check_local_desc_stream(marie_call);
	check_local_desc_stream(pauline_call);

	const LinphoneCallParams *marie_params2 = linphone_call_get_current_params(marie_call);
	BC_ASSERT_PTR_NOT_NULL(marie_params2);
	if (marie_params2) {
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(marie_params2));
	}
	const LinphoneCallParams *pauline_params2 = linphone_call_get_current_params(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(pauline_params2);
	if (pauline_params2) {
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(pauline_params2));
	}

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, NULL, 0, 2000));

	ms_message("=============== Ending call =================");
	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_with_rtp_port_to_zero(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
	linphone_core_set_keep_stream_direction_for_rejected_stream(marie->lc, TRUE);

	linphone_core_enable_zero_rtp_port_for_stream_inactive(pauline->lc, TRUE);

	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);

	// Invite inactive Audio (Marie pause Pauline)
	ms_message("CONTEXT: Marie sends empty INVITE to pause the call");
	int marieStreamsRunning = marie->stat.number_of_LinphoneCallStreamsRunning;
	LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_enable_audio(params, FALSE);
	linphone_call_update(marie_call, params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(
	    wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, (marieStreamsRunning + 1)));

	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, NULL, 0, 2000));

	check_local_desc_stream(marie_call);
	check_result_desc_rtp_rtcp_ports(pauline_call, 0, 0);

	const LinphoneCallParams *pauline_params = linphone_call_get_current_params(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(pauline_params);
	if (pauline_params) {
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(pauline_params));
	}

	// Invite active Audio (Marie resumes call with Pauline)
	ms_message("CONTEXT: Marie sends INVITE with SDP to resume the call");
	linphone_core_enable_sdp_200_ack(marie->lc, FALSE);
	marieStreamsRunning = marie->stat.number_of_LinphoneCallStreamsRunning;
	int paulineStreamsRunning = pauline->stat.number_of_LinphoneCallStreamsRunning;
	params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_enable_audio(params, TRUE);
	linphone_call_update(marie_call, params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 2));
	BC_ASSERT_TRUE(
	    wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, (marieStreamsRunning + 1)));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        (paulineStreamsRunning + 1)));
	linphone_call_params_unref(params);

	check_local_desc_stream(marie_call);
	check_local_desc_stream(pauline_call);

	const LinphoneCallParams *marie_params2 = linphone_call_get_current_params(marie_call);
	BC_ASSERT_PTR_NOT_NULL(marie_params2);
	if (marie_params2) {
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(marie_params2));
	}
	const LinphoneCallParams *pauline_params2 = linphone_call_get_current_params(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(pauline_params2);
	if (pauline_params2) {
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(pauline_params2));
	}

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, NULL, 0, 2000));

	ms_message("=============== Ending call =================");
	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

int check_nb_media_starts(unsigned int media_type,
                          LinphoneCoreManager *caller,
                          LinphoneCoreManager *callee,
                          unsigned int caller_nb_media_starts,
                          unsigned int callee_nb_media_starts) {
	unsigned int c1_starts = 0, c2_starts = 0;
	int c1_ret = FALSE, c2_ret = FALSE;
	LinphoneCall *c1 = linphone_core_get_current_call(caller->lc);
	LinphoneCall *c2 = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (!c1 || !c2) return FALSE;
	if (media_type > 2) return FALSE;

	if (media_type == AUDIO_START) {
		c1_starts = _linphone_call_get_nb_audio_starts(c1);
		c2_starts = _linphone_call_get_nb_audio_starts(c2);
	} else if (media_type == VIDEO_START) {
		c1_starts = _linphone_call_get_nb_video_starts(c1);
		c2_starts = _linphone_call_get_nb_video_starts(c2);
	} else {
		c1_starts = _linphone_call_get_nb_text_starts(c1);
		c2_starts = _linphone_call_get_nb_text_starts(c2);
	}

	c1_ret = (c1_starts == caller_nb_media_starts) ? TRUE : FALSE;
	BC_ASSERT_EQUAL(c1_starts, caller_nb_media_starts, unsigned int, "%u");

	c2_ret = (c2_starts == callee_nb_media_starts) ? TRUE : FALSE;
	BC_ASSERT_EQUAL(c2_starts, callee_nb_media_starts, unsigned int, "%u");

	return c1_ret && c2_ret;
}

void _call_with_ice_base(LinphoneCoreManager *pauline,
                         LinphoneCoreManager *marie,
                         bool_t caller_with_ice,
                         bool_t callee_with_ice,
                         bool_t random_ports,
                         bool_t forced_relay,
                         bool_t quick_cancel) {

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);

	if (callee_with_ice) {
		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
	}
	if (caller_with_ice) {
		enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);
	}

	if (random_ports) {
		linphone_core_set_audio_port(marie->lc, -1);
		linphone_core_set_video_port(marie->lc, -1);
		linphone_core_set_text_port(marie->lc, -1);
		linphone_core_set_audio_port(pauline->lc, -1);
		linphone_core_set_video_port(pauline->lc, -1);
		linphone_core_set_text_port(pauline->lc, -1);
	}

	if (forced_relay == TRUE) {
		linphone_core_enable_forced_ice_relay(marie->lc, TRUE);
		linphone_core_enable_forced_ice_relay(pauline->lc, TRUE);
	}
	if (quick_cancel) {
		stats pauline_stats = pauline->stat;
		LinphoneCall *out_call = linphone_core_invite_address(pauline->lc, marie->identity);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit,
		                        pauline_stats.number_of_LinphoneCallOutgoingInit + 1));
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 100); // Let time to begin candidates gathering
		linphone_call_terminate(out_call);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd,
		                        pauline_stats.number_of_LinphoneCallEnd + 1));
	} else {
		if (!BC_ASSERT_TRUE(call(pauline, marie))) return;

		if (callee_with_ice && caller_with_ice) {
			/*wait for the ICE reINVITE to complete*/
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

			if (forced_relay == TRUE) {
				BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateRelayConnection));
			} else {
				BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
			}
			check_nb_media_starts(AUDIO_START, pauline, marie, 1, 1);
		}

		liblinphone_tester_check_rtcp(marie, pauline);
		/*then close the call*/
		end_call(pauline, marie);
	}
}

static void on_ack_processing(LinphoneCall *call, LinphoneHeaders *ack, bool_t is_received) {
	if (!is_received) {
		linphone_headers_add(ack, "Coucou", "me voila");
		/*We put something in user_data to indicate that we went through this callback*/
		linphone_call_set_user_data(call, (void *)1);
	} else {
		const char *ack_header;
		ack_header = linphone_headers_get_value(ack, "Coucou");
		BC_ASSERT_PTR_NOT_NULL(ack_header);
		if (ack_header) {
			BC_ASSERT_STRING_EQUAL(ack_header, "me voila");
		}
		linphone_call_set_user_data(call, (void *)1);
	}
}

static void call_created(BCTBX_UNUSED(LinphoneCore *lc), LinphoneCall *call) {
	LinphoneCallCbs *cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	linphone_call_cbs_set_ack_processing(cbs, on_ack_processing);
	linphone_call_add_callbacks(call, cbs);
	linphone_call_cbs_unref(cbs);
}

static void call_with_custom_headers(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie, *call_pauline;
	LinphoneCallParams *params;
	const LinphoneCallParams *marie_remote_params;
	const char *hvalue;
	char *pauline_remote_contact_header, *pauline_remote_contact, *marie_remote_contact, *marie_remote_contact_header;
	LinphoneAddress *marie_identity;
	char *tmp = linphone_address_as_string_uri_only(marie->identity);
	char tmp2[256];
	LinphoneCoreCbs *core_cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	snprintf(tmp2, sizeof(tmp2), "%s?uriHeader=myUriHeader", tmp);
	marie_identity = linphone_address_new(tmp2);
	ms_free(tmp);
	linphone_address_unref(marie->identity);
	marie->identity = marie_identity;

	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_add_custom_header(params, "Weather", "bad");
	linphone_call_params_add_custom_header(params, "Working", "yes");

	linphone_core_cbs_set_call_created(core_cbs, call_created);
	linphone_core_add_callbacks(marie->lc, core_cbs);
	linphone_core_add_callbacks(pauline->lc, core_cbs);

	if (!BC_ASSERT_TRUE(call_with_caller_params(pauline, marie, params))) goto end;

	call_marie = linphone_core_get_current_call(marie->lc);
	call_pauline = linphone_core_get_current_call(pauline->lc);

	BC_ASSERT_PTR_NOT_NULL(call_marie);
	BC_ASSERT_PTR_NOT_NULL(call_pauline);

	marie_remote_params = linphone_call_get_remote_params(call_marie);
	hvalue = linphone_call_params_get_custom_header(marie_remote_params, "Weather");
	BC_ASSERT_PTR_NOT_NULL(hvalue);
	BC_ASSERT_STRING_EQUAL(hvalue, "bad");
	hvalue = linphone_call_params_get_custom_header(marie_remote_params, "uriHeader");
	BC_ASSERT_PTR_NOT_NULL(hvalue);
	BC_ASSERT_STRING_EQUAL(hvalue, "myUriHeader");

	// FIXME: we have to strdup because successive calls to get_remote_params erase the returned const char*!!
	pauline_remote_contact = ms_strdup(linphone_call_get_remote_contact(call_pauline));
	pauline_remote_contact_header =
	    ms_strdup(linphone_call_params_get_custom_header(linphone_call_get_remote_params(call_pauline), "Contact"));

	marie_remote_contact = ms_strdup(linphone_call_get_remote_contact(call_marie));
	marie_remote_contact_header =
	    ms_strdup(linphone_call_params_get_custom_header(linphone_call_get_remote_params(call_marie), "Contact"));

	BC_ASSERT_PTR_NOT_NULL(pauline_remote_contact);
	BC_ASSERT_PTR_NOT_NULL(pauline_remote_contact_header);
	BC_ASSERT_PTR_NOT_NULL(marie_remote_contact);
	BC_ASSERT_PTR_NOT_NULL(marie_remote_contact_header);
	BC_ASSERT_STRING_EQUAL(pauline_remote_contact, pauline_remote_contact_header);
	BC_ASSERT_STRING_EQUAL(marie_remote_contact, marie_remote_contact_header);

	/*we need to wait for the ack to arrive*/
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 3000);

	BC_ASSERT_TRUE(linphone_call_get_user_data(call_marie) == (void *)1);
	BC_ASSERT_TRUE(linphone_call_get_user_data(call_pauline) == (void *)1);

	ms_free(pauline_remote_contact);
	ms_free(pauline_remote_contact_header);
	ms_free(marie_remote_contact);
	ms_free(marie_remote_contact_header);

	end_call(pauline, marie);

end:
	linphone_core_cbs_unref(core_cbs);
	linphone_call_params_unref(params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_custom_reserved_headers(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie, *call_pauline;
	LinphoneCallParams *init_pauline_params, *marie_params;
	const LinphoneCallParams *marie_remote_params;
	const char *hvalue;
	char *pauline_remote_contact_header, *pauline_remote_contact, *marie_remote_contact, *marie_remote_contact_header;
	LinphoneAddress *marie_identity;
	char *tmp = linphone_address_as_string_uri_only(marie->identity);
	char tmp2[256];

	const char *from_str = "sip:laure@custom.header.org";
	LinphoneAddress *from = linphone_address_new(from_str);
	init_pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_set_from_header(init_pauline_params, from_str);
	// linphone_call_params_add_custom_header(init_pauline_params,"From","<sip:laure@custom.header.org>;tag=b88752a2-33ff-4bae-a4b1-e768500a90d8");
	LinphoneCoreCbs *core_cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	snprintf(tmp2, sizeof(tmp2), "%s?uriHeader=myUriHeader", tmp);
	marie_identity = linphone_address_new(tmp2);
	ms_free(tmp);
	linphone_address_unref(marie->identity);
	marie->identity = marie_identity;

	stats initial_pauline_stat = pauline->stat;
	stats initial_marie_stat = marie->stat;

	call_pauline = linphone_core_invite_address_with_params(pauline->lc, marie->identity, init_pauline_params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived,
	                        initial_pauline_stat.number_of_LinphoneCallIncomingReceived + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit,
	                        initial_pauline_stat.number_of_LinphoneCallOutgoingInit + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging,
	                        initial_pauline_stat.number_of_LinphoneCallOutgoingRinging + 1));

	call_marie = linphone_core_get_current_call(marie->lc);
	linphone_call_accept(call_marie);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));

	marie_remote_params = linphone_call_get_remote_params(call_marie);
	hvalue = linphone_call_params_get_custom_header(marie_remote_params, "From");
	BC_ASSERT_PTR_NOT_NULL(hvalue);
	LinphoneAddress *marie_from = linphone_address_new(hvalue);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marie_from, from));

	BC_ASSERT_PTR_NOT_NULL(call_marie);
	BC_ASSERT_PTR_NOT_NULL(call_pauline);

	initial_pauline_stat = pauline->stat;
	initial_marie_stat = marie->stat;

	marie_params = linphone_core_create_call_params(marie->lc, call_marie);
	linphone_call_update(call_marie, marie_params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1));

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));

	// FIXME: we have to strdup because successive calls to get_remote_params erase the returned const char*!!
	pauline_remote_contact = ms_strdup(linphone_call_get_remote_contact(call_pauline));
	pauline_remote_contact_header =
	    ms_strdup(linphone_call_params_get_custom_header(linphone_call_get_remote_params(call_pauline), "Contact"));

	marie_remote_contact = ms_strdup(linphone_call_get_remote_contact(call_marie));
	marie_remote_contact_header =
	    ms_strdup(linphone_call_params_get_custom_header(linphone_call_get_remote_params(call_marie), "Contact"));

	BC_ASSERT_PTR_NOT_NULL(pauline_remote_contact);
	BC_ASSERT_PTR_NOT_NULL(pauline_remote_contact_header);
	BC_ASSERT_PTR_NOT_NULL(marie_remote_contact);
	BC_ASSERT_PTR_NOT_NULL(marie_remote_contact_header);
	BC_ASSERT_STRING_EQUAL(pauline_remote_contact, pauline_remote_contact_header);
	BC_ASSERT_STRING_EQUAL(marie_remote_contact, marie_remote_contact_header);

	BC_ASSERT_TRUE(linphone_address_weak_equal(
	    linphone_call_log_get_from_address(linphone_call_get_call_log(call_pauline)), from));
	BC_ASSERT_TRUE(
	    linphone_address_weak_equal(linphone_call_log_get_from_address(linphone_call_get_call_log(call_marie)), from));

	ms_free(pauline_remote_contact);
	ms_free(pauline_remote_contact_header);
	ms_free(marie_remote_contact);
	ms_free(marie_remote_contact_header);

	linphone_call_terminate(call_marie);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd,
	                        initial_pauline_stat.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd,
	                        initial_marie_stat.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased,
	                        initial_pauline_stat.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased,
	                        initial_marie_stat.number_of_LinphoneCallReleased + 1));

	linphone_address_unref(from);
	linphone_address_unref(marie_from);
	linphone_core_cbs_unref(core_cbs);
	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(init_pauline_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_custom_sdp_attributes_cb(LinphoneCore *lc,
                                               LinphoneCall *call,
                                               LinphoneCallState cstate,
                                               BCTBX_UNUSED(const char *message)) {
	if (cstate == LinphoneCallUpdatedByRemote) {
		LinphoneCallParams *params;
		const LinphoneCallParams *remote_params = linphone_call_get_remote_params(call);
		const char *value = linphone_call_params_get_custom_sdp_attribute(remote_params, "weather");
		BC_ASSERT_PTR_NOT_NULL(value);
		if (value) BC_ASSERT_STRING_EQUAL(value, "sunny");
		params = linphone_core_create_call_params(lc, call);
		linphone_call_params_clear_custom_sdp_attributes(params);
		linphone_call_params_clear_custom_sdp_media_attributes(params, LinphoneStreamTypeAudio);
		linphone_call_params_add_custom_sdp_attribute(params, "working", "no");
		BC_ASSERT_EQUAL(linphone_call_accept_update(call, params), 0, int, "%i");
		linphone_call_params_unref(params);
	}
}

static void call_with_custom_sdp_attributes(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie, *call_pauline;
	LinphoneCallParams *pauline_params;
	const LinphoneCallParams *marie_remote_params;
	const LinphoneCallParams *pauline_remote_params;
	const char *value;
	LinphoneCoreCbs *cbs;

	pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_add_custom_sdp_attribute(pauline_params, "weather", "bad");
	linphone_call_params_add_custom_sdp_attribute(pauline_params, "working", "yes");
	linphone_call_params_add_custom_sdp_attribute(pauline_params, "attribute_without_value",
	                                              NULL); /*cannot be tested yet*/
	linphone_call_params_add_custom_sdp_media_attribute(pauline_params, LinphoneStreamTypeAudio, "sleeping", "almost");
	BC_ASSERT_TRUE(call_with_caller_params(pauline, marie, pauline_params));
	linphone_call_params_unref(pauline_params);

	call_marie = linphone_core_get_current_call(marie->lc);
	call_pauline = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(call_marie);
	BC_ASSERT_PTR_NOT_NULL(call_pauline);

	marie_remote_params = linphone_call_get_remote_params(call_marie);
	value = linphone_call_params_get_custom_sdp_attribute(marie_remote_params, "weather");
	BC_ASSERT_PTR_NOT_NULL(value);
	if (value) BC_ASSERT_STRING_EQUAL(value, "bad");
	value =
	    linphone_call_params_get_custom_sdp_media_attribute(marie_remote_params, LinphoneStreamTypeAudio, "sleeping");
	BC_ASSERT_PTR_NOT_NULL(value);
	if (value) BC_ASSERT_STRING_EQUAL(value, "almost");

	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_with_custom_sdp_attributes_cb);
	linphone_core_add_callbacks(marie->lc, cbs);
	pauline_params = linphone_core_create_call_params(pauline->lc, call_pauline);
	linphone_call_params_clear_custom_sdp_attributes(pauline_params);
	linphone_call_params_clear_custom_sdp_media_attributes(pauline_params, LinphoneStreamTypeAudio);
	linphone_call_params_add_custom_sdp_attribute(pauline_params, "weather", "sunny");
	linphone_call_update(call_pauline, pauline_params);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
	linphone_call_params_unref(pauline_params);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	pauline_remote_params = linphone_call_get_remote_params(call_pauline);
	value = linphone_call_params_get_custom_sdp_attribute(pauline_remote_params, "working");
	BC_ASSERT_PTR_NOT_NULL(value);
	if (value) BC_ASSERT_STRING_EQUAL(value, "no");

	end_call(pauline, marie);

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_custom_header_or_sdp_cb(LinphoneCore *lc,
                                              LinphoneCall *call,
                                              LinphoneCallState cstate,
                                              BCTBX_UNUSED(const char *message)) {
	const char *value;
	if (cstate == LinphoneCallOutgoingInit) {
		LinphoneCallParams *params = linphone_core_create_call_params(lc, call);
		linphone_call_params_add_custom_sdp_attribute(params, "working", "maybe");
		linphone_call_set_params(call, params);
		linphone_call_params_unref(params);
	} else if (cstate == LinphoneCallIncomingReceived) {
		const LinphoneCallParams *tparams = linphone_call_get_remote_params(call);
		// Check received params
		// SDP
		value = linphone_call_params_get_custom_sdp_attribute(tparams, "working");
		BC_ASSERT_PTR_NOT_NULL(value);
		if (value) BC_ASSERT_STRING_EQUAL(value, "maybe");
		// header
		value = linphone_call_params_get_custom_header(tparams, "weather");
		BC_ASSERT_PTR_NOT_NULL(value);
		if (value) BC_ASSERT_STRING_EQUAL(value, "thunderstorm");
		// modify SDP
		LinphoneCallParams *params = linphone_core_create_call_params(lc, call);
		linphone_call_params_add_custom_sdp_attribute(params, "working", "yes");
		linphone_call_set_params(call, params);
		linphone_call_params_unref(params);
	}
}

static void call_caller_with_custom_header_or_sdp_attributes(void) {
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_caller = NULL, *call_callee = NULL;
	LinphoneCallParams *caller_params; //	*callee_params ;
	LinphoneCoreCbs *cbs;
	LinphoneCallTestParams caller_test_params = {0};
	LinphoneCallTestParams callee_test_params = {0};

	stats initial_caller = caller_mgr->stat;
	stats initial_callee = callee_mgr->stat;
	bool_t result = FALSE;
	bool_t did_receive_call;

	// Create caller params with custom header and custom SDP
	caller_params = linphone_core_create_call_params(caller_mgr->lc, NULL);
	linphone_call_params_add_custom_header(caller_params, "weather", "thunderstorm");
	linphone_call_params_add_custom_sdp_media_attribute(caller_params, LinphoneStreamTypeAudio, "sleeping", "almost");

	caller_test_params.base = (LinphoneCallParams *)caller_params;
	callee_test_params.base = NULL;

	/* TODO: This should be handled correctly inside the liblinphone library but meanwhile handle this here. */
	linphone_core_manager_wait_for_stun_resolution(caller_mgr);
	linphone_core_manager_wait_for_stun_resolution(callee_mgr);

	setup_sdp_handling(&caller_test_params, caller_mgr);
	setup_sdp_handling(&callee_test_params, callee_mgr);

	// Assign dedicated callback for caller and callee
	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_with_custom_header_or_sdp_cb);
	linphone_core_add_callbacks(callee_mgr->lc, cbs);
	linphone_core_add_callbacks(caller_mgr->lc, cbs);

	// Caller initates the call with INVITE
	//  caller params not null
	BC_ASSERT_PTR_NOT_NULL(
	    (call_caller = linphone_core_invite_address_with_params(caller_mgr->lc, callee_mgr->identity, caller_params)));

	BC_ASSERT_PTR_NULL(linphone_call_get_remote_params(
	    call_caller)); /*assert that remote params are NULL when no response is received yet*/

	// Wait for Incoming received
	did_receive_call =
	    wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived,
	             initial_callee.number_of_LinphoneCallIncomingReceived + 1);
	BC_ASSERT_EQUAL(did_receive_call, !callee_test_params.sdp_simulate_error, int, "%d");

	linphone_call_params_unref(caller_params);

	sal_default_set_sdp_handling(linphone_core_get_sal(caller_mgr->lc), SalOpSDPNormal);
	sal_default_set_sdp_handling(linphone_core_get_sal(callee_mgr->lc), SalOpSDPNormal);

	// Wait for Outgoing Progress
	if (linphone_core_get_calls_nb(callee_mgr->lc) <= 1)
		BC_ASSERT_TRUE(linphone_core_is_incoming_invite_pending(callee_mgr->lc));
	BC_ASSERT_EQUAL(caller_mgr->stat.number_of_LinphoneCallOutgoingProgress,
	                initial_caller.number_of_LinphoneCallOutgoingProgress + 1, int, "%d");

	LinphoneCallParams *default_params = linphone_core_create_call_params(callee_mgr->lc, call_callee);
	ms_message("Created default call params with video=%i", linphone_call_params_video_enabled(default_params));
	linphone_core_accept_call_with_params(callee_mgr->lc, call_callee, default_params);
	linphone_call_params_unref(default_params);

	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallConnected,
	                        initial_callee.number_of_LinphoneCallConnected + 1));
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallConnected,
	                        initial_caller.number_of_LinphoneCallConnected + 1));

	result = wait_for_until(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_caller.number_of_LinphoneCallStreamsRunning + 1, 2000) &&
	         wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_callee.number_of_LinphoneCallStreamsRunning + 1, 2000);
	BC_ASSERT_TRUE(result);

	caller_params = linphone_core_create_call_params(caller_mgr->lc, call_caller);
	linphone_call_params_clear_custom_sdp_attributes(caller_params);
	linphone_call_params_clear_custom_sdp_media_attributes(caller_params, LinphoneStreamTypeAudio);
	linphone_call_params_add_custom_sdp_attribute(caller_params, "weather", "sunny");
	linphone_core_update_call(caller_mgr->lc, call_caller, caller_params);
	linphone_call_params_unref(caller_params);

	end_call(caller_mgr, callee_mgr);

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
}

static void call_callee_with_custom_header_or_sdp_cb(LinphoneCore *lc,
                                                     LinphoneCall *call,
                                                     LinphoneCallState cstate,
                                                     BCTBX_UNUSED(const char *message)) {
	const char *value;
	if (cstate == LinphoneCallOutgoingInit) {
		LinphoneCallParams *params = linphone_core_create_call_params(lc, call);
		linphone_call_params_add_custom_sdp_attribute(params, "working", "maybe");
		linphone_call_set_params(call, params);
		linphone_call_params_unref(params);
	} else if (cstate == LinphoneCallIncomingReceived) {
		const LinphoneCallParams *tparams = linphone_call_get_remote_params(call);
		value = linphone_call_params_get_custom_sdp_attribute(tparams, "working");
		BC_ASSERT_PTR_NOT_NULL(value);
		if (value) BC_ASSERT_STRING_EQUAL(value, "maybe");
		LinphoneCallParams *params = linphone_core_create_call_params(lc, call);
		linphone_call_set_params(call, params);
		linphone_call_params_unref(params);
	}
}

static void call_callee_with_custom_header_or_sdp_attributes(void) {
	int result;
	LinphoneCoreManager *callee_mgr = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *caller_mgr =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_caller = NULL, *call_callee = NULL;
	LinphoneCallParams *callee_params, *caller_params;
	LinphoneCoreCbs *cbs;
	const char *value;
	LinphoneCallTestParams caller_test_params = {0};
	LinphoneCallTestParams callee_test_params = {0};
	stats initial_caller = caller_mgr->stat;
	stats initial_callee = callee_mgr->stat;
	bool_t did_receive_call;
	const LinphoneCallParams *caller_remote_params;

	caller_params = linphone_core_create_call_params(caller_mgr->lc, NULL);
	callee_test_params.base = NULL;
	caller_test_params.base = NULL;

	/* TODO: This should be handled correctly inside the liblinphone library but meanwhile handle this here. */
	linphone_core_manager_wait_for_stun_resolution(caller_mgr);
	linphone_core_manager_wait_for_stun_resolution(callee_mgr);

	setup_sdp_handling(&caller_test_params, caller_mgr);
	setup_sdp_handling(&callee_test_params, callee_mgr);

	// Assign dedicated callback for caller and callee
	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_callee_with_custom_header_or_sdp_cb);
	linphone_core_add_callbacks(callee_mgr->lc, cbs);
	linphone_core_add_callbacks(caller_mgr->lc, cbs);

	// Caller initates the call with INVITE
	//  caller params not null
	BC_ASSERT_PTR_NOT_NULL(
	    (call_caller = linphone_core_invite_address_with_params(caller_mgr->lc, callee_mgr->identity, caller_params)));

	BC_ASSERT_PTR_NULL(linphone_call_get_remote_params(
	    call_caller)); /*assert that remote params are NULL when no response is received yet*/

	// Wait for Incoming received
	did_receive_call =
	    wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived,
	             initial_callee.number_of_LinphoneCallIncomingReceived + 1);
	BC_ASSERT_EQUAL(did_receive_call, !callee_test_params.sdp_simulate_error, int, "%d");

	sal_default_set_sdp_handling(linphone_core_get_sal(caller_mgr->lc), SalOpSDPNormal);
	sal_default_set_sdp_handling(linphone_core_get_sal(callee_mgr->lc), SalOpSDPNormal);

	// Wait for Outgoing Progress
	if (linphone_core_get_calls_nb(callee_mgr->lc) <= 1)
		BC_ASSERT_TRUE(linphone_core_is_incoming_invite_pending(callee_mgr->lc));
	BC_ASSERT_EQUAL(caller_mgr->stat.number_of_LinphoneCallOutgoingProgress,
	                initial_caller.number_of_LinphoneCallOutgoingProgress + 1, int, "%d");

	// Create callee params with custom header and custom SDP
	callee_params = linphone_core_create_call_params(callee_mgr->lc, call_callee);
	linphone_call_params_add_custom_header(callee_params, "weather", "thunderstorm");
	linphone_call_params_add_custom_sdp_media_attribute(callee_params, LinphoneStreamTypeAudio, "sleeping", "almost");
	linphone_call_params_add_custom_sdp_attribute(callee_params, "working", "yes");
	ms_message("Created default call params with video=%i", linphone_call_params_video_enabled(callee_params));
	linphone_core_accept_call_with_params(callee_mgr->lc, call_callee, callee_params);
	linphone_call_params_unref(callee_params);

	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallConnected,
	                        initial_callee.number_of_LinphoneCallConnected + 1));
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallConnected,
	                        initial_caller.number_of_LinphoneCallConnected + 1));

	result = wait_for_until(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_caller.number_of_LinphoneCallStreamsRunning + 1, 2000) &&
	         wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_callee.number_of_LinphoneCallStreamsRunning + 1, 2000);

	BC_ASSERT_TRUE(result);

	caller_remote_params = linphone_call_get_remote_params(call_caller);
	value = linphone_call_params_get_custom_sdp_attribute(caller_remote_params, "working");
	BC_ASSERT_PTR_NOT_NULL(value);
	if (value) BC_ASSERT_STRING_EQUAL(value, "yes");
	// header
	value = linphone_call_params_get_custom_header(caller_remote_params, "weather");
	BC_ASSERT_PTR_NOT_NULL(value);
	if (value) BC_ASSERT_STRING_EQUAL(value, "thunderstorm");

	linphone_call_params_unref(caller_params);
	end_call(caller_mgr, callee_mgr);

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(callee_mgr);
	linphone_core_manager_destroy(caller_mgr);
}

void call_paused_resumed_base(bool_t multicast, bool_t with_losses, bool_t accept_video) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	const LinphoneCoreToneManagerStats *marieToneManagerStats = linphone_core_get_tone_manager_stats(marie->lc);
	const LinphoneCoreToneManagerStats *paulineToneManagerStats = linphone_core_get_tone_manager_stats(pauline->lc);

	if (accept_video) {
		LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_initiate(vpol, FALSE);
		linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
		linphone_core_set_video_activation_policy(marie->lc, vpol);
		linphone_core_set_video_activation_policy(pauline->lc, vpol);
		linphone_video_activation_policy_unref(vpol);

		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	}

	LinphoneCall *call_pauline = NULL;
	LinphoneCall *call_marie = NULL;
	RtpSession *rtp_session;
	const rtp_stats_t *stats;
	bool_t call_ok;

	linphone_core_enable_audio_multicast(pauline->lc, multicast);

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);
	linphone_call_ref(call_pauline);

	const LinphoneCallParams *marie_call_lparams = linphone_call_get_params(call_marie);
	BC_ASSERT_FALSE(linphone_call_params_ringing_disabled(marie_call_lparams));
	BC_ASSERT_TRUE(linphone_call_params_tone_indications_enabled(marie_call_lparams));

	const LinphoneCallParams *pauline_call_lparams = linphone_call_get_params(call_pauline);
	BC_ASSERT_FALSE(linphone_call_params_ringing_disabled(pauline_call_lparams));
	BC_ASSERT_TRUE(linphone_call_params_tone_indications_enabled(pauline_call_lparams));

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

	if (with_losses) {
		sal_set_send_error(linphone_core_get_sal(marie->lc), 1500); /*to trash 200ok without generating error*/
	}
	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_startRingbackTone, 1)))
		goto end;

	linphone_call_pause(call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));

	if (with_losses) {
		BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1, 1000));
		sal_set_send_error(linphone_core_get_sal(marie->lc), 0); /*to trash 200ok without generating error*/
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_stopRingbackTone, 1)))
		goto end;

	BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_startNamedTone, 1, int, "%d");

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_call_resume(call_pauline);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_stopTone, 1)))
		goto end;

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	rtp_session = linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio)->sessions.rtp_session;
	if (BC_ASSERT_PTR_NOT_NULL(rtp_session)) {
		stats = rtp_session_get_stats(rtp_session);
		BC_ASSERT_EQUAL((int)stats->cum_packet_loss, 0, int, "%d");
	}

	if (with_losses) {
		/* now we want to loose the ack*/
		linphone_call_pause(call_pauline);
		sal_set_send_error(linphone_core_get_sal(pauline->lc), 1500); /*to trash ACK without generating error*/
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 2));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 2, 1000));
		/*now try to resume, it should be OK*/
		sal_set_send_error(linphone_core_get_sal(pauline->lc), 0);
		linphone_call_resume(call_pauline);
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3, 2000));
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3, 2000));
	}

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

	if (with_losses) {
		sal_set_send_error(linphone_core_get_sal(pauline->lc), 1500); /*to trash 200ok without generating error*/
	}
	linphone_call_pause(call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausing, 1));

	if (with_losses) {
		BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 1, 1000));
		sal_set_send_error(linphone_core_get_sal(pauline->lc), 0); /*to trash 200ok without generating error*/
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

	BC_ASSERT_EQUAL(marieToneManagerStats->number_of_startNamedTone, 1, int, "%d");

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_call_resume(call_marie);

	int streams_running = (with_losses) ? 4 : 3;
	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, streams_running)))
		goto end;
	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, streams_running)))
		goto end;

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, (int *)&marieToneManagerStats->number_of_stopTone, 1)))
		goto end;

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	rtp_session = linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio)->sessions.rtp_session;
	if (BC_ASSERT_PTR_NOT_NULL(rtp_session)) {
		stats = rtp_session_get_stats(rtp_session);
		BC_ASSERT_EQUAL((int)stats->cum_packet_loss, 0, int, "%d");
	}

	if (with_losses) {
		/* now we want to loose the ack*/
		linphone_call_pause(call_marie);
		sal_set_send_error(linphone_core_get_sal(marie->lc), 1500); /*to trash ACK without generating error*/
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausing, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 2));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 2, 1000));
		/*now try to resume, it should be OK*/
		sal_set_send_error(linphone_core_get_sal(marie->lc), 0);
		linphone_call_resume(call_marie);
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                              (streams_running + 1), 2000));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                              (streams_running + 1), 2000));
	}

	end_call(pauline, marie);
end:
	if (call_pauline) linphone_call_unref(call_pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_resumed(void) {
	call_paused_resumed_base(FALSE, FALSE, FALSE);
}

static void call_paused_resumed_with_sip_packets_losses(void) {
	call_paused_resumed_base(FALSE, TRUE, FALSE);
}

void call_paused_quickly_resumed(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	const LinphoneCoreToneManagerStats *paulineToneManagerStats = linphone_core_get_tone_manager_stats(pauline->lc);

	LinphoneCall *call_pauline = NULL;
	LinphoneCall *call_marie = NULL;
	bool_t call_ok;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);
	linphone_call_ref(call_pauline);

	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);
	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_startRingbackTone, 1)))
		goto end;

	linphone_call_pause(call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));

	for (int i = 0; i < 10; i++) {
		linphone_call_resume(call_pauline);
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_stopRingbackTone, 1)))
		goto end;

	BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_startNamedTone, 1, int, "%d");

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, (int *)&paulineToneManagerStats->number_of_stopTone, 1)))
		goto end;

	/*check if video stream is not offered*/
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_pauline), 1, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_get_stream_count(call_marie), 1, int, "%i");

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_pauline, LinphoneStreamTypeVideo));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio));
	BC_ASSERT_PTR_NULL(linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo));

	end_call(pauline, marie);
end:
	if (call_pauline) linphone_call_unref(call_pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_resumed_no_register(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall *call_marie = NULL;
	RtpSession *rtp_session;
	const rtp_stats_t *stats;
	bool_t call_ok;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (!call_ok) goto end;

	call_marie = linphone_core_get_current_call(marie->lc);
	linphone_call_pause(call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausing, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, 1, int, "%d");

	LinphoneProxyConfig *proxyConfig = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, FALSE);
	linphone_proxy_config_done(proxyConfig);
	BC_ASSERT_TRUE(wait_for(marie->lc, marie->lc, &marie->stat.number_of_LinphoneRegistrationCleared, 1));

	linphone_call_resume(call_marie);

	int streams_running = 2;
	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, streams_running)))
		goto end;
	if (!BC_ASSERT_TRUE(
	        wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, streams_running)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc,
	                             (int *)&linphone_core_get_tone_manager_stats(marie->lc)->number_of_stopTone, 1)))
		goto end;

	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	rtp_session = linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio)->sessions.rtp_session;
	if (BC_ASSERT_PTR_NOT_NULL(rtp_session)) {
		stats = rtp_session_get_stats(rtp_session);
		BC_ASSERT_EQUAL((int)stats->cum_packet_loss, 0, int, "%d");
	}

	proxyConfig = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, TRUE);
	linphone_proxy_config_done(proxyConfig);
	BC_ASSERT_TRUE(wait_for(marie->lc, marie->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_by_both(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline, *call_marie;
	const rtp_stats_t *stats;
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;

	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, marie->lc);
	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_call_pause(call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	/*marie pauses the call also*/
	linphone_call_pause(call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);
	/*pauline must stay in paused state*/
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallPaused, 2, int, "%i");
	check_media_direction(pauline, call_pauline, lcs, LinphoneMediaDirectionInactive, LinphoneMediaDirectionInvalid);
	check_media_direction(marie, call_marie, lcs, LinphoneMediaDirectionInactive, LinphoneMediaDirectionInvalid);

	/*now pauline wants to resume*/
	linphone_call_resume(call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallResuming, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	/*Marie must stay in paused state*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, 2, int, "%i");

	/*now marie wants to resume also*/
	linphone_call_resume(call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallResuming, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	stats =
	    rtp_session_get_stats(linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio)->sessions.rtp_session);
	BC_ASSERT_EQUAL((int)stats->cum_packet_loss, 0, int, "%d");

	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

#define CHECK_CURRENT_LOSS_RATE()                                                                                      \
	rtcp_count_current = pauline->stat.number_of_rtcp_sent;                                                            \
	/*wait for an RTCP packet to have an accurate cumulative lost value*/                                              \
	BC_ASSERT_TRUE(                                                                                                    \
	    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_rtcp_sent, rtcp_count_current + 1, 10000));    \
	stats = rtp_session_get_stats(rtp_session);                                                                        \
	loss_percentage = stats->cum_packet_loss * 100.f / (stats->packet_recv + stats->cum_packet_loss);                  \
	BC_ASSERT_GREATER(loss_percentage, .75f * params.loss_rate, float, "%f");                                          \
	BC_ASSERT_LOWER(loss_percentage, 1.25f * params.loss_rate, float, "%f")

static void call_paused_resumed_with_loss(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline;
	const rtp_stats_t *stats;
	float loss_percentage;
	int rtcp_count_current;

	OrtpNetworkSimulatorParams params = {0};
	params.enabled = TRUE;
	params.loss_rate = 20;

	BC_ASSERT_TRUE(call(pauline, marie));
	call_pauline = linphone_core_get_current_call(pauline->lc);
	if (call_pauline) {
		RtpSession *rtp_session = linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio)->sessions.rtp_session;
		rtp_session_enable_network_simulation(rtp_session, &params);

		/*generate some traffic*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 10000);
		CHECK_CURRENT_LOSS_RATE();

		/*pause call*/
		linphone_call_pause(call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));
		/*stay in pause a little while in order to generate traffic*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 10000);
		CHECK_CURRENT_LOSS_RATE();

		/*resume*/
		linphone_call_resume(call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 10000);

		/*since stats are NOT totally reset during pause, the stats->packet_recv is computed from
		the start of call. This test ensures that the loss rate is consistent during the entire call.*/
		CHECK_CURRENT_LOSS_RATE();
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

bool_t
pause_call_1(LinphoneCoreManager *mgr_1, LinphoneCall *call_1, LinphoneCoreManager *mgr_2, LinphoneCall *call_2) {
	stats initial_call_stat_1 = mgr_1->stat;
	stats initial_call_stat_2 = mgr_2->stat;
	BC_ASSERT_PTR_NOT_NULL(call_1);
	if (!call_1) return FALSE;
	linphone_call_pause(call_1);
	BC_ASSERT_TRUE(wait_for(mgr_1->lc, mgr_2->lc, &mgr_1->stat.number_of_LinphoneCallPausing,
	                        initial_call_stat_1.number_of_LinphoneCallPausing + 1));
	BC_ASSERT_TRUE(wait_for(mgr_1->lc, mgr_2->lc, &mgr_1->stat.number_of_LinphoneCallPaused,
	                        initial_call_stat_1.number_of_LinphoneCallPaused + 1));
	BC_ASSERT_TRUE(wait_for(mgr_1->lc, mgr_2->lc, &mgr_2->stat.number_of_LinphoneCallPausedByRemote,
	                        initial_call_stat_2.number_of_LinphoneCallPausedByRemote + 1));
	BC_ASSERT_EQUAL(linphone_call_get_state(call_1), LinphoneCallPaused, int, "%d");
	BC_ASSERT_EQUAL(linphone_call_get_state(call_2), LinphoneCallPausedByRemote, int, "%d");
	return linphone_call_get_state(call_1) == LinphoneCallPaused &&
	       linphone_call_get_state(call_2) == LinphoneCallPausedByRemote;
}
#if 0
void concurrent_paused_resumed_base(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall* call_pauline,call_marie;
	const rtp_stats_t * stats;


	BC_ASSERT_TRUE(call(pauline,marie));

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);

	linphone_core_pause_call(pauline->lc,call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPausing,1));

	linphone_core_pause_call(marie->lc,call_marie);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausedByRemote,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPaused,1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_core_resume_call(pauline->lc,call_pauline);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	stats = rtp_session_get_stats(call_pauline->sessions->rtp_session);
	BC_ASSERT_EQUAL(stats->cum_packet_loss, 0, int, "%d");


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));


	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
#endif
static void call_paused_resumed_from_callee_base(bool_t timeout) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie;
	const rtp_stats_t *stats;
	bool_t call_ok;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (!call_ok) goto end;
	call_marie = linphone_core_get_current_call(marie->lc);

	linphone_core_enable_mic(marie->lc, FALSE);
	BC_ASSERT_FALSE(linphone_core_mic_enabled(marie->lc));
	linphone_call_pause(call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	BC_ASSERT_FALSE(linphone_core_mic_enabled(marie->lc));

	if (timeout) {
		linphone_core_set_network_reachable(marie->lc, FALSE);
		OrtpNetworkSimulatorParams params = {0};
		params.enabled = TRUE;
		params.loss_rate = 100;
		params.consecutive_loss_probability = 1;
		params.mode = OrtpNetworkSimulatorOutbound;
		RtpSession *rtp_session = linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio)->sessions.rtp_session;
		rtp_session_enable_network_simulation(rtp_session, &params);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1, 50000));
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1, 50000));
	} else {
		linphone_call_resume(call_marie);

		linphone_core_enable_mic(marie->lc, TRUE);
		BC_ASSERT_TRUE(linphone_core_mic_enabled(marie->lc));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

		/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
		stats =
		    rtp_session_get_stats(linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio)->sessions.rtp_session);
		BC_ASSERT_EQUAL((int)stats->cum_packet_loss, 0, int, "%d");

		end_call(pauline, marie);
	}
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_resumed_from_callee(void) {
	call_paused_resumed_from_callee_base(FALSE);
}

static void call_paused_resumed_from_callee_with_timeout(void) {
	call_paused_resumed_from_callee_base(TRUE);
}

static void _call_with_media_relay(bool_t random_ports) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	if (random_ports) {
		linphone_core_set_audio_port(marie->lc, -1);
		linphone_core_set_video_port(marie->lc, -1);
		linphone_core_set_audio_port(pauline->lc, -1);
		linphone_core_set_video_port(pauline->lc, -1);
	}

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(pauline, marie);

#ifdef VIDEO_ENABLED
	BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
	liblinphone_tester_check_rtcp(pauline, marie);
#endif
	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_media_relay(void) {
	_call_with_media_relay(FALSE);
}

static void call_with_media_relay_random_ports(void) {
	_call_with_media_relay(TRUE);
}

static void call_with_privacy(void) {
	LinphoneCall *c1, *c2;
	LinphoneCallParams *params;
	LinphoneProxyConfig *pauline_proxy;

	/* pub-gruu and privacy do not work well together. */
	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_remove_supported_tag(marie->lc, "gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline, transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc",
	                           NULL);
	linphone_core_set_supported_tag(pauline->lc, "");
	linphone_core_manager_start(pauline, TRUE);

	params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_set_privacy(params, LinphonePrivacyId);

	BC_ASSERT_TRUE(call_with_caller_params(pauline, marie, params));
	linphone_call_params_unref(params);

	c1 = linphone_core_get_current_call(pauline->lc);
	c2 = linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (c1 && c2) {
		/*make sure local identity is unchanged*/
		BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from_address(linphone_call_get_call_log(c1)),
		                                           pauline->identity));

		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2), pauline->identity));

		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)), LinphonePrivacyId, int,
		                "%d");
	}
	liblinphone_tester_check_rtcp(pauline, marie);
	end_call(pauline, marie);

	/*test proxy config privacy*/
	pauline_proxy = linphone_core_get_default_proxy_config(pauline->lc);
	linphone_proxy_config_edit(pauline_proxy);
	linphone_proxy_config_set_privacy(pauline_proxy, LinphonePrivacyId);
	linphone_proxy_config_done(pauline_proxy);

	BC_ASSERT_TRUE(call(pauline, marie));

	c1 = linphone_core_get_current_call(pauline->lc);
	c2 = linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (c1 && c2) {

		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2), pauline->identity));

		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)), LinphonePrivacyId, int,
		                "%d");
	}

	end_call(pauline, marie);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*this ones makes call with privacy without previous registration*/
static void call_with_privacy2(void) {
	LinphoneCall *c1, *c2;
	LinphoneCallParams *params;
	LinphoneProxyConfig *pauline_proxy;
	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_remove_supported_tag(marie->lc, "gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline, transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc",
	                           NULL);
	linphone_core_remove_supported_tag(pauline->lc, "gruu");
	linphone_core_manager_start(pauline, TRUE);

	params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_set_privacy(params, LinphonePrivacyId);

	pauline_proxy = linphone_core_get_default_proxy_config(pauline->lc);
	linphone_proxy_config_edit(pauline_proxy);
	linphone_proxy_config_enable_register(pauline_proxy, FALSE);
	linphone_proxy_config_done(pauline_proxy);

	BC_ASSERT_TRUE(call_with_caller_params(pauline, marie, params));
	linphone_call_params_unref(params);

	c1 = linphone_core_get_current_call(pauline->lc);
	c2 = linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	if (c1 && c2) {
		/*make sure local identity is unchanged*/
		BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from_address(linphone_call_get_call_log(c1)),
		                                           pauline->identity));
		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2), pauline->identity));

		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)), LinphonePrivacyId, int,
		                "%d");
	}

	end_call(pauline, marie);

	/*test proxy config privacy*/
	linphone_proxy_config_edit(pauline_proxy);
	linphone_proxy_config_set_privacy(pauline_proxy, LinphonePrivacyId);
	linphone_proxy_config_done(pauline_proxy);

	BC_ASSERT_TRUE(call(pauline, marie));
	c1 = linphone_core_get_current_call(pauline->lc);
	c2 = linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	if (c1 && c2) {
		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2), pauline->identity));
		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)), LinphonePrivacyId, int,
		                "%d");
	}
	end_call(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_file_player(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recordpath = bc_tester_file("record-call_with_file_player.wav");
	bool_t call_ok;
	int attempts;
	double similar = 1;
	const double threshold = 0.9;

	/*this test is actually attempted three times in case of failure, because the audio comparison at the end is very
	 * sensitive to jitter buffer drifts, which sometimes happen if the machine is unable to run the test in good
	 * realtime conditions */
	for (attempts = 0; attempts < 3; attempts++) {
		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);
		linphone_core_reset_tone_manager_stats(marie->lc);
		linphone_core_reset_tone_manager_stats(pauline->lc);

		/*make sure the record file doesn't already exists, otherwise this test will append new samples to it*/
		unlink(recordpath);
		/*caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using call's
		 * player*/
		linphone_core_set_use_files(marie->lc, TRUE);
		linphone_core_set_play_file(marie->lc, NULL);

		/*callee is recording and plays file*/
		linphone_core_set_use_files(pauline->lc, TRUE);
		linphone_core_set_play_file(pauline->lc, NULL);
		linphone_core_set_record_file(pauline->lc, recordpath);

		BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
		if (!call_ok) goto end;
		player = linphone_call_get_player(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_PTR_NOT_NULL(player);
		if (player) {
			cbs = linphone_factory_create_player_cbs(linphone_factory_get());
			linphone_player_cbs_set_eof_reached(cbs, on_player_eof);
			linphone_player_cbs_set_user_data(cbs, marie);
			linphone_player_add_callbacks(player, cbs);
			BC_ASSERT_EQUAL(linphone_player_open(player, hellopath), 0, int, "%d");
			BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
		}
		/* This assert should be modified to be at least as long as the WAV file */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_player_eof, 1, 10000));
		/*wait one second more for transmission to be fully ended (transmission time + jitter buffer)*/
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);

		end_call(marie, pauline);
		/*cannot run on iphone simulator because locks main loop beyond permitted time (should run
		on another thread) */
		BC_ASSERT_EQUAL(ms_audio_diff(hellopath, recordpath, &similar, &audio_cmp_params, NULL, NULL), 0, int, "%d");
		if (cbs) linphone_player_cbs_unref(cbs);
		if (similar >= threshold) break;
	}
	BC_ASSERT_GREATER(similar, threshold, double, "%g");
	BC_ASSERT_LOWER(similar, 1.0, double, "%g");
	if (similar >= threshold && similar <= 1.0) {
		remove(recordpath);
	}

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(recordpath);
	ms_free(hellopath);
}

static void call_with_mkv_file_player(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	char *hellomkv;
	char *hellowav;
	char *recordpath;
	bool_t call_ok;
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
	double similar = 0.0;
	const double threshold = 0.9;
#define DO_AUDIO_CMP
#endif
	hellowav = bc_tester_res("sounds/hello8000_mkv_ref.wav");
	hellomkv = bc_tester_res("sounds/hello8000.mkv");

	if (!linphone_core_file_format_supported(marie->lc, "mkv")) {
		ms_warning("Test skipped, no mkv support.");
		goto end;
	}
	recordpath = bc_tester_file("record-call_with_mkv_file_player.wav");
	/*make sure the record file doesn't already exists, otherwise this test will append new samples to it*/
	unlink(recordpath);

	/*caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using call's
	 * player*/
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, NULL);
	/*callee is recording and plays file*/
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_play_file(pauline->lc,
	                            hellowav); /*just to send something but we are not testing what is sent by pauline*/
	linphone_core_set_record_file(pauline->lc, recordpath);

	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (!call_ok) goto end;
	player = linphone_call_get_player(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(player);
	if (player) {
		cbs = linphone_factory_create_player_cbs(linphone_factory_get());
		linphone_player_cbs_set_eof_reached(cbs, on_player_eof);
		linphone_player_cbs_set_user_data(cbs, marie);
		linphone_player_add_callbacks(player, cbs);
		int res = linphone_player_open(player, hellomkv);
		// if(!ms_filter_codec_supported("opus")) {
		if (!ms_factory_codec_supported(linphone_core_get_ms_factory(marie->lc), "opus") &&
		    !ms_factory_codec_supported(linphone_core_get_ms_factory(pauline->lc), "opus")) {
			BC_ASSERT_EQUAL(res, -1, int, "%d");
			end_call(marie, pauline);
			goto end;
		}
		BC_ASSERT_EQUAL(res, 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_player_eof, 1, 12000));
		linphone_player_close(player);
		/*wait for one second more so that last RTP packets can arrive*/
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
	}
	end_call(marie, pauline);
#ifdef DO_AUDIO_CMP
	BC_ASSERT_EQUAL(ms_audio_diff(hellowav, recordpath, &similar, &audio_cmp_params, NULL, NULL), 0, int, "%d");
	BC_ASSERT_GREATER(similar, threshold, double, "%f");
	BC_ASSERT_LOWER(similar, 1.0, double, "%f");
	if (similar > threshold && similar <= 1.0) {
		remove(recordpath);
	}
#else
	/*inter-correlation process is too much CPU consuming ending in a 20 minutes test on arm...*/
	remove(recordpath);
#endif
	ms_free(recordpath);

end:
	if (cbs) linphone_player_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(hellomkv);
	ms_free(hellowav);
}

static void _call_base_with_configfile(LinphoneMediaEncryption mode,
                                       bool_t enable_video,
                                       bool_t enable_relay,
                                       LinphoneFirewallPolicy policy,
                                       bool_t enable_tunnel,
                                       const char *marie_rc,
                                       const char *pauline_rc,
                                       bool_t plays_nothing) {
	LinphoneCoreManager *marie = linphone_core_manager_new(marie_rc);
	LinphoneCoreManager *pauline = linphone_core_manager_new(pauline_rc);
	bool_t call_ok;

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	if (linphone_core_find_payload_type(pauline->lc, "h264", -1, -1) != NULL) {
		disable_all_video_codecs_except_one(pauline->lc, "h264");
		disable_all_video_codecs_except_one(marie->lc, "h264");
	}
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	if (plays_nothing) {
		/*This case was for trying to replicate an issue because
		 * zrtp_iterate() was only called when packets are received, which
		 * creates a big problem because no retransmission of HELLO packet will occur
		 * if the remote sends nothing.
		 * However it is not possible to forcibly loose the hello packet, even with network simulator.
		 * If retransmissions fail, this test will fail from time to time*/
		linphone_core_set_use_files(marie->lc, TRUE);
		linphone_core_set_play_file(marie->lc, NULL);
		linphone_core_set_play_file(pauline->lc, NULL);
		linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
		linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
	}

	if (enable_relay) {
		linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
		linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	}
	if (enable_tunnel) {
		int i;
		LinphoneTunnelConfig *tunnel_config = linphone_tunnel_config_new();
		linphone_tunnel_config_set_host(tunnel_config, "tunnel.linphone.org");
		linphone_tunnel_config_set_port(tunnel_config, 443);
		linphone_tunnel_add_server(linphone_core_get_tunnel(marie->lc), tunnel_config);
		linphone_tunnel_enable_sip(linphone_core_get_tunnel(marie->lc), FALSE);
		linphone_tunnel_set_mode(linphone_core_get_tunnel(marie->lc), LinphoneTunnelModeEnable);
		for (i = 0; i < 500; i++) {
			if (linphone_tunnel_connected(linphone_core_get_tunnel(marie->lc))) {
				linphone_core_iterate(marie->lc);
				break;
			}
			linphone_core_iterate(marie->lc);
			ms_usleep(20000);
		}
		BC_ASSERT_TRUE(linphone_tunnel_connected(linphone_core_get_tunnel(marie->lc)));
		linphone_tunnel_config_unref(tunnel_config);
	}

	if (linphone_core_media_encryption_supported(marie->lc, mode)) {
		linphone_core_set_media_encryption(marie->lc, mode);
		linphone_core_set_media_encryption(pauline->lc, mode);
		if (mode == LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a directory
			                                          to store them */
			char *path = bc_tester_file("certificates-marie");
			linphone_core_set_user_certificates_path(marie->lc, path);
			bc_free(path);
			path = bc_tester_file("certificates-pauline");
			linphone_core_set_user_certificates_path(pauline->lc, path);
			bc_free(path);
			bctbx_mkdir(linphone_core_get_user_certificates_path(marie->lc));
			bctbx_mkdir(linphone_core_get_user_certificates_path(pauline->lc));
		}

		if (policy == LinphonePolicyUseIce) {
			enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
			enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);
		}

		BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
		if (!call_ok) goto end;
		/* if caller set ZRTP or (callee set ZRTP and caller has no encryption requested), ZRTP shall take place, wait
		 * for the SAS */
		if ((linphone_core_get_media_encryption(pauline->lc) == LinphoneMediaEncryptionZRTP) ||
		    ((linphone_core_get_media_encryption(marie->lc) == LinphoneMediaEncryptionZRTP) &&
		     (linphone_core_get_media_encryption(pauline->lc) == LinphoneMediaEncryptionNone))) {
			/*wait for SAS*/
			int i;
			LinphoneCall *pauline_call;
			LinphoneCall *marie_call;
			const char *pauline_token = NULL;
			const char *marie_token = NULL;
			for (i = 0; i < 100; i++) {
				pauline_call = linphone_core_get_current_call(pauline->lc);
				marie_call = linphone_core_get_current_call(marie->lc);

				if (!pauline_call || !marie_call) {
					/*if one of the two calls was disapeering, don't crash, but report it*/
					BC_ASSERT_PTR_NOT_NULL(pauline_call);
					BC_ASSERT_PTR_NOT_NULL(marie_call);
					break;
				}
				pauline_token = linphone_call_get_authentication_token(pauline_call);
				marie_token = linphone_call_get_authentication_token(marie_call);
				if (pauline_token && marie_token) break;
				linphone_core_iterate(marie->lc);
				linphone_core_iterate(pauline->lc);
				ms_usleep(20000);
			}
			BC_ASSERT_PTR_NOT_NULL(pauline_token);
			BC_ASSERT_PTR_NOT_NULL(marie_token);
			if (marie_token && pauline_token) {
				BC_ASSERT_STRING_EQUAL(pauline_token, marie_token);
				BC_ASSERT_TRUE(strlen(pauline_token) > 0);
				BC_ASSERT_TRUE(strlen(marie_token) > 0);

				const char *pauline_local_token = linphone_call_get_local_authentication_token(pauline_call);
				const char *marie_local_token = linphone_call_get_local_authentication_token(marie_call);
				char *auth_token = (char *)bctbx_malloc(strlen(pauline_local_token) + strlen(marie_local_token) + 1);
				if (linphone_call_get_dir(pauline_call) == LinphoneCallIncoming) {
					strcat(strcpy(auth_token, pauline_local_token), marie_local_token);
				} else {
					strcat(strcpy(auth_token, marie_local_token), pauline_local_token);
				}
				BC_ASSERT_STRING_EQUAL(pauline_token, auth_token);
				bctbx_free(auth_token);

				bool_t auth_token_found = FALSE;
				for (const bctbx_list_t *it = linphone_call_get_remote_authentication_tokens(pauline_call); it;
				     it = it->next) {
					if (strcmp(marie_local_token, it->data) == 0) auth_token_found = TRUE;
				}
				BC_ASSERT_TRUE(auth_token_found);
				auth_token_found = FALSE;
				for (const bctbx_list_t *it = linphone_call_get_remote_authentication_tokens(marie_call); it;
				     it = it->next) {
					if (strcmp(pauline_local_token, it->data) == 0) auth_token_found = TRUE;
				}
				BC_ASSERT_TRUE(auth_token_found);
			}
			if (!plays_nothing) liblinphone_tester_check_rtcp(pauline, marie);
		}

		if (policy == LinphonePolicyUseIce) {
			BC_ASSERT_TRUE(check_ice(
			    pauline, marie, enable_tunnel ? LinphoneIceStateReflexiveConnection : LinphoneIceStateHostConnection));
			wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000); /*fixme to workaround a crash*/
		}
		if (enable_tunnel) {
			LinphoneTunnel *tunnel = linphone_core_get_tunnel(marie->lc);
			BC_ASSERT_PTR_NOT_NULL(tunnel);
			if (tunnel) {
				RtpTransport *tp = linphone_call_get_meta_rtp_transport(linphone_core_get_current_call(marie->lc),
				                                                        LinphoneStreamTypeAudio);
				RtpTransport *endpoint = meta_rtp_transport_get_endpoint(tp);
				BC_ASSERT_PTR_NOT_NULL(endpoint);
				if (endpoint) {
					BC_ASSERT_TRUE(linphone_tunnel_is_tunnel_rtp_transport(tunnel, endpoint));
				}
			}
		}

		if (enable_video) {
#ifdef VIDEO_ENABLED
			if (linphone_core_video_supported(marie->lc)) {
				BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
				if (policy == LinphonePolicyUseIce) {
					BC_ASSERT_TRUE(check_ice(pauline, marie,
					                         enable_tunnel ? LinphoneIceStateReflexiveConnection
					                                       : LinphoneIceStateHostConnection));
				}
				liblinphone_tester_check_rtcp(marie, pauline);

			} else {
				ms_warning("not tested because video not available");
			}
#endif
		}
		end_call(marie, pauline);
	} else {
		ms_warning("not tested because %s not available", linphone_media_encryption_to_string(mode));
	}
end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

void call_base_with_configfile(LinphoneMediaEncryption mode,
                               bool_t enable_video,
                               bool_t enable_relay,
                               LinphoneFirewallPolicy policy,
                               bool_t enable_tunnel,
                               const char *marie_rc,
                               const char *pauline_rc) {
	_call_base_with_configfile(mode, enable_video, enable_relay, policy, enable_tunnel, marie_rc, pauline_rc, FALSE);
}

void call_base_with_configfile_play_nothing(LinphoneMediaEncryption mode,
                                            bool_t enable_video,
                                            bool_t enable_relay,
                                            LinphoneFirewallPolicy policy,
                                            bool_t enable_tunnel,
                                            const char *marie_rc,
                                            const char *pauline_rc) {
	_call_base_with_configfile(mode, enable_video, enable_relay, policy, enable_tunnel, marie_rc, pauline_rc, TRUE);
}

void call_base(LinphoneMediaEncryption mode,
               bool_t enable_video,
               bool_t enable_relay,
               LinphoneFirewallPolicy policy,
               bool_t enable_tunnel) {
	call_base_with_configfile(mode, enable_video, enable_relay, policy, enable_tunnel, "marie_rc", "pauline_tcp_rc");
}

static void early_media_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));

	if (!call_ok) goto end;
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, int, "%d");

	wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);

	/*added because a bug related to early-media caused the Connected state to be reached two times*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallConnected, 1, int, "%d");

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void assert_mic_gain(LinphoneCall *call, float linear_gain) {
	AudioStream *astream = (AudioStream *)linphone_call_get_stream(call, LinphoneStreamTypeAudio);
	MSFilter *volsend;
	float cur_gain = -1;
	if (!BC_ASSERT_PTR_NOT_NULL(astream)) return;
	volsend = astream->volsend;
	if (!BC_ASSERT_PTR_NOT_NULL(volsend)) return;
	ms_filter_call_method(volsend, MS_VOLUME_GET_GAIN, &cur_gain);
	BC_ASSERT_EQUAL(cur_gain, linear_gain, float, "%f");
}

static void early_media_call_with_ringing_base(bool_t network_change) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCall *marie_call;
	LinphoneCallLog *marie_call_log;
	uint64_t connected_time = 0;
	uint64_t ended_time = 0;
	int dummy = 0;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	/*
	    Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	marie_call_log = linphone_call_get_call_log(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 5000));
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingbackTone, 1, int, "%d");

	if (linphone_core_is_incoming_invite_pending(pauline->lc)) {
		LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
		/* send a 183 to initiate the early media */
		linphone_call_accept_early_media(pauline_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 5000));
		/*
		 * By default, on early media, both caller and callee must not send sensitive data.
		 * This is enforced by muting microphones, ie applying a zero gain on the volume conntroller.
		 */
		BC_ASSERT_TRUE(linphone_call_get_all_muted(marie_call));
		BC_ASSERT_TRUE(linphone_call_get_all_muted(pauline_call));
		assert_mic_gain(marie_call, 0);
		assert_mic_gain(pauline_call, 0);

		bool_t ringWithEarlyMedia = linphone_core_get_ring_during_incoming_early_media(pauline->lc);
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopRingtone,
		                ringWithEarlyMedia ? 0 : 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_stopRingbackTone, 1, int, "%d");

		liblinphone_tester_check_rtcp(marie, pauline);

		/* this is a hack to simulate an incoming OK with a different IP address
		 * in the 'c' SDP field. */
		if (network_change) {
			_linphone_call_add_local_desc_changed_flag(marie_call, SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED);
		}

		if (linphone_core_get_current_call(pauline->lc) &&
		    linphone_call_get_state(linphone_core_get_current_call(pauline->lc)) == LinphoneCallIncomingEarlyMedia) {
			linphone_call_accept(linphone_core_get_current_call(pauline->lc));

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 1000));
			connected_time = ms_get_cur_time_ms();
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
			/* Make sure that microphone is now unmuted */
			assert_mic_gain(marie_call, 1.0f);
			assert_mic_gain(pauline_call, 1.0f);
			BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopRingtone, 1, int, "%d");

			BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));
			BC_ASSERT_FALSE(linphone_call_get_all_muted(marie_call));

			liblinphone_tester_check_rtcp(marie, pauline);
			/*just to have a call duration !=0*/
			wait_for_list(lcs, &dummy, 1, 2000);

			end_call(pauline, marie);
			ended_time = ms_get_cur_time_ms();
			BC_ASSERT_LOWER(labs((long)((linphone_call_log_get_duration(marie_call_log) * 1000) -
			                            (int64_t)(ended_time - connected_time))),
			                1500, long, "%ld");
		}
		bctbx_list_free(lcs);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_ringing(void) {
	early_media_call_with_ringing_base(FALSE);
}

static void early_media_call_with_ringing_and_network_changing(void) {
	early_media_call_with_ringing_base(TRUE);
}

static void early_media_call_with_update_base(bool_t media_change) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCall *marie_call, *pauline_call;
	LinphoneCallParams *pauline_params;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	if (media_change) {
		disable_all_audio_codecs_except_one(marie->lc, "pcmu", -1);
		disable_all_audio_codecs_except_one(pauline->lc, "pcmu", -1);
	}
	/*
	    Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 5000));
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingbackTone, 1, int, "%d");

	pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!pauline_call) goto end;
	/* send a 183 to initiate the early media */
	linphone_call_accept_early_media(pauline_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 5000));
	BC_ASSERT_TRUE(linphone_call_get_all_muted(marie_call));

	bool_t ringWithEarlyMedia = linphone_core_get_ring_during_incoming_early_media(pauline->lc);
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopRingtone,
	                ringWithEarlyMedia ? 0 : 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_stopRingbackTone, 1, int, "%d");

	pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);

	if (media_change) {
		disable_all_audio_codecs_except_one(marie->lc, "pcma", -1);
		disable_all_audio_codecs_except_one(pauline->lc, "pcma", -1);
	}
#define UPDATED_SESSION_NAME "nouveau nom de session"

	linphone_call_params_set_session_name(pauline_params, UPDATED_SESSION_NAME);
	linphone_call_update(pauline_call, pauline_params);
	linphone_call_params_unref(pauline_params);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEarlyUpdating, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEarlyUpdatedByRemote, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 2000));
	BC_ASSERT_TRUE(linphone_call_get_all_muted(marie_call));

	/*just to wait 2s*/
	liblinphone_tester_check_rtcp(marie, pauline);

	BC_ASSERT_STRING_EQUAL(linphone_call_params_get_session_name(linphone_call_get_remote_params(marie_call)),
	                       UPDATED_SESSION_NAME);

	linphone_call_accept(linphone_core_get_current_call(pauline->lc));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
	BC_ASSERT_FALSE(linphone_call_get_all_muted(marie_call));

	liblinphone_tester_check_rtcp(marie, pauline);

	end_call(pauline, marie);

end:

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_session_update(void) {
	early_media_call_with_update_base(FALSE);
}

static void early_media_call_with_codec_update(void) {
	early_media_call_with_update_base(TRUE);
}

static void call_with_early_update_failed(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCall *marie_call, *pauline_call;
	LinphoneCallParams *marie_params;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	// Start the call again
	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingInit, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1, 5000));

	if (marie_call) {
		marie_params = linphone_core_create_call_params(marie->lc, marie_call);
		linphone_call_params_enable_video(marie_params, TRUE);
		linphone_call_update(marie_call, marie_params);
		linphone_call_params_unref(marie_params);
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		linphone_call_accept(pauline_call);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));

	if (pauline_call) {
		const LinphoneCallParams *pauline_current_call_params = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(pauline_current_call_params));
	}

	if (marie_call) {
		const LinphoneCallParams *marie_current_call_params = linphone_call_get_current_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(marie_current_call_params));
	}

	liblinphone_tester_check_rtcp(marie, pauline);

	end_call(pauline, marie);

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void check_call_state(LinphoneCoreManager *mgr, LinphoneCallState state) {
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(mgr->lc));
	if (linphone_core_get_current_call(mgr->lc))
		BC_ASSERT_EQUAL(linphone_call_get_state(linphone_core_get_current_call(mgr->lc)), state, int, "%d");
}

static void call_established_with_rejected_info(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneInfoMessage *im1;
	LinphoneInfoMessage *im2;
	int dummy = 0;
	bool_t call_ok = FALSE;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (call_ok) {

		sal_enable_unconditional_answer(linphone_core_get_sal(marie->lc), TRUE);
		im1 = linphone_core_create_info_message(pauline->lc);
		linphone_call_send_info_message(linphone_core_get_current_call(pauline->lc), im1);

		wait_for_until(marie->lc, pauline->lc, &dummy, 1, 3000); /*just to sleep while iterating 1s*/
		linphone_info_message_unref(im1);

		sal_enable_unconditional_answer(linphone_core_get_sal(marie->lc), FALSE);

		im2 = linphone_core_create_info_message(pauline->lc);
		linphone_call_send_info_message(linphone_core_get_current_call(pauline->lc), im2);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_InfoReceived, 1));
		BC_ASSERT_EQUAL(marie->stat.number_of_InfoReceived, 1, int, "%d");
		linphone_info_message_unref(im2);

		check_call_state(pauline, LinphoneCallStreamsRunning);
		check_call_state(marie, LinphoneCallStreamsRunning);
		end_call(pauline, marie);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_complex_rejected_operation(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok = FALSE;
	LinphoneCallParams *params;
	LinphoneInfoMessage *info;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (call_ok) {
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMU", 8000, 1),
		                                  FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
		                                  TRUE); /*enable PCMA*/
		linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1),
		                                  FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1),
		                                  TRUE); /*enable PCMA*/

		/*just to authenticate marie*/
		info = linphone_core_create_info_message(marie->lc);
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc), info);
		linphone_info_message_unref(info);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_InfoReceived, 1));
		BC_ASSERT_EQUAL(pauline->stat.number_of_InfoReceived, 1, int, "%d");
		/*to give time for 200ok to arrive*/
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
		linphone_call_params_unref(params);

		params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));
		linphone_call_update(linphone_core_get_current_call(marie->lc), params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

		check_call_state(pauline, LinphoneCallStreamsRunning);
		check_call_state(marie, LinphoneCallStreamsRunning);

		params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
		linphone_call_params_unref(params);
		info = linphone_core_create_info_message(marie->lc);
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc), info);
		linphone_info_message_unref(info);

		sal_enable_pending_trans_checking(linphone_core_get_sal(marie->lc), FALSE); /*to allow // transactions*/
		linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1), TRUE);
		linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1),
		                                  FALSE);
		params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));
		linphone_call_update(linphone_core_get_current_call(marie->lc), params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),
		                LinphoneReasonNotAcceptable, int, "%d");

		end_call(pauline, marie);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_info_during_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok = FALSE;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (call_ok) {
		LinphoneInfoMessage *info;
		LinphoneCallParams *params;
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMU", 8000, 1),
		                                  FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
		                                  TRUE); /*enable PCMA*/
		linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1),
		                                  FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1),
		                                  TRUE); /*enable PCMA*/

		/*just to authenticate marie*/
		info = linphone_core_create_info_message(marie->lc);
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc), info);
		linphone_info_message_unref(info);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_InfoReceived, 1));
		BC_ASSERT_EQUAL(pauline->stat.number_of_InfoReceived, 1, int, "%d");
		/*to give time for 200ok to arrive*/
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		// sal_enable_pending_trans_checking(marie->lc->sal,FALSE); /*to allow // transactions*/
		info = linphone_core_create_info_message(marie->lc);
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc), info);
		linphone_info_message_unref(info);

		// sal_set_send_error(marie->lc->sal, -1); /*to avoid 491 pending to be sent*/
		params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
		linphone_call_params_unref(params);

		wait_for_until(pauline->lc, pauline->lc, NULL, 0, 2000); /*to avoid 491 pending to be sent to early*/

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		end_call(pauline, marie);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok = FALSE;

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (call_ok) {
		LinphoneCallParams *params =
		    linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMU", 8000, 1),
		                                  FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
		                                  TRUE); /*enable PCMA*/

		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),
		                LinphoneReasonNotAcceptable, int, "%d");

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, 1, int, "%d");
		check_call_state(pauline, LinphoneCallStreamsRunning);
		check_call_state(marie, LinphoneCallStreamsRunning);
		end_call(pauline, marie);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_incoming_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok = FALSE;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (call_ok) {
		LinphoneCallParams *params;
		/*wait for ACK to be transmitted before going to reINVITE*/
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMU", 8000, 1),
		                                  FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
		                                  TRUE); /*enable PCMA*/
		params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));
		linphone_call_update(linphone_core_get_current_call(marie->lc), params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(marie->lc)),
		                LinphoneReasonNotAcceptable, int, "%d");

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, 1, int, "%d");
		check_call_state(pauline, LinphoneCallStreamsRunning);
		check_call_state(marie, LinphoneCallStreamsRunning);
		end_call(pauline, marie);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_redirect(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");
	bctbx_list_t *lcs = NULL;
	char *laure_url = NULL;
	LinphoneCall *marie_call, *laure_call;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	/*
	    Marie calls Pauline, which will redirect the call to Laure via a 302
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 6000));

	if (linphone_core_get_current_call(pauline->lc)) {
		laure_url = linphone_address_as_string(laure->identity);
		linphone_call_redirect(linphone_core_get_current_call(pauline->lc), laure_url);
		ms_free(laure_url);

		/* laure should be ringing now */
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 6000));
		/* pauline should have ended the call */
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 1000));
		/* the call should still be ringing on marie's side */
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingRinging, 1, int, "%i");

		laure_call = linphone_core_get_current_call(laure->lc);
		BC_ASSERT_PTR_NOT_NULL(laure_call);
		if (laure_call) {
			linphone_call_accept(laure_call);

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

			BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));

			liblinphone_tester_check_rtcp(marie, laure);

			end_call(laure, marie);
		}
	}

	bctbx_list_free(lcs);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void call_established_with_rejected_reinvite_with_error_base(bool_t trans_pending) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok = TRUE;
	int result;

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (call_ok) {
		LinphoneCallParams *params;
		linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
		                                  TRUE); /*add PCMA*/

		if (trans_pending) {
			LinphoneInfoMessage *info = linphone_core_create_info_message(pauline->lc);
			linphone_call_send_info_message(linphone_core_get_current_call(pauline->lc), info);
			linphone_info_message_unref(info);

		} else sal_enable_unconditional_answer(linphone_core_get_sal(marie->lc), TRUE);
		params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		result = linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
		linphone_call_params_unref(params);
		if (trans_pending) BC_ASSERT_NOT_EQUAL(result, 0, int, "%d");
		else {
			BC_ASSERT_EQUAL(result, 0, int, "%d");
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		}

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),
		                LinphoneReasonTemporarilyUnavailable, int, "%d"); /*might be changed later*/

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, 1, int, "%d");
		check_call_state(pauline, LinphoneCallStreamsRunning);
		check_call_state(marie, LinphoneCallStreamsRunning);

		if (!trans_pending) sal_enable_unconditional_answer(linphone_core_get_sal(marie->lc), FALSE);

		end_call(pauline, marie);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_reinvite_with_error(void) {
	call_established_with_rejected_reinvite_with_error_base(FALSE);
}

static void call_established_with_rejected_reinvite_with_trans_pending_error(void) {
	call_established_with_rejected_reinvite_with_error_base(TRUE);
}

static void call_rejected_because_wrong_credentials_with_params(const char *user_agent, bool_t enable_auth_req_cb) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAuthInfo *good_auth_info = linphone_auth_info_clone(
	    linphone_core_find_auth_info(marie->lc, NULL, linphone_address_get_username(marie->identity), NULL));
	LinphoneAuthInfo *wrong_auth_info = linphone_auth_info_clone(good_auth_info);
	bool_t result = FALSE;
	linphone_auth_info_set_passwd(wrong_auth_info, "passecretdutout");
	linphone_auth_info_set_ha1(wrong_auth_info, NULL);
	linphone_core_clear_all_auth_info(marie->lc);

	if (user_agent) {
		linphone_core_set_user_agent(marie->lc, user_agent, NULL);
	}
	if (!enable_auth_req_cb) {
		LinphoneCoreCbs *cbs = linphone_core_get_first_callbacks(marie->lc);
		linphone_core_cbs_set_auth_info_requested(cbs, NULL);
		linphone_core_add_auth_info(marie->lc, wrong_auth_info);
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(marie->lc, marie->identity));

	result = wait_for(marie->lc, marie->lc, &marie->stat.number_of_auth_info_requested, 1);

	if (enable_auth_req_cb) {
		BC_ASSERT_TRUE(result);
		/*automatically re-initiate the call*/
		linphone_core_add_auth_info(marie->lc, wrong_auth_info);
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, marie->lc, &marie->stat.number_of_LinphoneCallError, 1));

	result = wait_for(marie->lc, marie->lc, &marie->stat.number_of_auth_info_requested, 2);
	if (user_agent && enable_auth_req_cb) {
		BC_ASSERT_TRUE(result);
	}

	/*to make sure unregister will work*/
	linphone_core_clear_all_auth_info(marie->lc);
	linphone_core_add_auth_info(marie->lc, good_auth_info);
	linphone_auth_info_unref(good_auth_info);
	linphone_auth_info_unref(wrong_auth_info);
	linphone_core_manager_destroy(marie);
}

static void call_rejected_because_wrong_credentials(void) {
	call_rejected_because_wrong_credentials_with_params(NULL, TRUE);
}

static void call_rejected_without_403_because_wrong_credentials(void) {
	call_rejected_because_wrong_credentials_with_params("tester-no-403", TRUE);
}

static void call_rejected_without_403_because_wrong_credentials_no_auth_req_cb(void) {
	call_rejected_because_wrong_credentials_with_params("tester-no-403", FALSE);
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void check_media_direction(LinphoneCoreManager *mgr,
                           LinphoneCall *call,
                           bctbx_list_t *lcs,
                           LinphoneMediaDirection audio_dir,
                           LinphoneMediaDirection video_dir) {
	BC_ASSERT_PTR_NOT_NULL(call);
	if (call) {
		const LinphoneCallParams *params;
		call = linphone_call_ref(call);    // Iterate can remove the call
		wait_for_list(lcs, NULL, 0, 5000); /*on some device, it may take 3 to 4s to get audio from mic*/
		params = linphone_call_get_current_params(call);
#ifdef VIDEO_ENABLED
		if (video_dir != LinphoneMediaDirectionInvalid) {
			int current_recv_iframe = mgr->stat.number_of_IframeDecoded;
			int expected_recv_iframe = 0;
			LinphoneCallStats *stats = linphone_call_get_video_stats(call);

			if (video_dir != LinphoneMediaDirectionInactive) {
				LinphoneCallCbs *call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
				BC_ASSERT_TRUE(linphone_call_params_video_enabled(params));
				BC_ASSERT_EQUAL(linphone_call_params_get_video_direction(params), video_dir, int, "%d");
				linphone_call_cbs_set_next_video_frame_decoded(call_cbs, linphone_call_next_video_frame_decoded_cb);
				linphone_call_add_callbacks(call, call_cbs);
				linphone_call_cbs_unref(call_cbs);
				linphone_call_request_notify_next_video_frame_decoded(call);
				linphone_call_send_vfu_request(call);
			}
			switch (video_dir) {
				case LinphoneMediaDirectionInactive:
					if (stats) {
						BC_ASSERT_LOWER((int)linphone_call_stats_get_upload_bandwidth(stats), 5, int, "%i");
					} else {
						/* it is expected that there is no stats for an inactive stream.*/
					}
					break;
				case LinphoneMediaDirectionSendOnly:
					expected_recv_iframe = 0;
					BC_ASSERT_LOWER((int)linphone_call_stats_get_download_bandwidth(stats), 5, int, "%i");
					break;
				case LinphoneMediaDirectionRecvOnly:
					BC_ASSERT_LOWER((int)linphone_call_stats_get_upload_bandwidth(stats), 5, int, "%i");
					BCTBX_NO_BREAK; /*intentionally no break*/
				case LinphoneMediaDirectionSendRecv:
					expected_recv_iframe = 1;
					break;
				default:
					break;
			}
			if (stats) linphone_call_stats_unref(stats);
			BC_ASSERT_TRUE(wait_for_list(lcs, &mgr->stat.number_of_IframeDecoded,
			                             current_recv_iframe + expected_recv_iframe, 10000));
		}
#endif
		if (audio_dir != LinphoneMediaDirectionInvalid) {
			BC_ASSERT_EQUAL(linphone_call_params_get_audio_direction(params), audio_dir, int, "%d");
			switch (audio_dir) {
				case LinphoneMediaDirectionInactive:
					BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_up_bw(mgr), 5, int, "%i");
					BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(mgr), 5, int, "%i");
					break;
				case LinphoneMediaDirectionSendOnly:
					BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_up_bw(mgr), 70, int, "%i");
					break;
				case LinphoneMediaDirectionRecvOnly:
					BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_up_bw(mgr), 5, int, "%i");
					break;
				case LinphoneMediaDirectionSendRecv:
					BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(mgr), 70, int, "%i");
					BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_up_bw(mgr), 70, int, "%i");
					break;
				default:
					break;
			}
		}
		linphone_call_unref(call);
	}
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

static void call_state_changed_callback_to_start_record(BCTBX_UNUSED(LinphoneCore *lc),
                                                        LinphoneCall *call,
                                                        LinphoneCallState state,
                                                        BCTBX_UNUSED(const char *message)) {
	if (state == /*LinphoneCallStreamsRunning*/ LinphoneCallConnected) {
		ms_message("call_recording(): start early recording into %s",
		           linphone_call_params_get_record_file(linphone_call_get_params(call)));
		linphone_call_start_recording(call);
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void record_call(const char *filename, bool_t enableVideo, const char *video_codec) {
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *pauline = NULL;
	LinphoneCallParams *marieParams = NULL;
	LinphoneCallParams *paulineParams = NULL;
	LinphoneCall *callInst = NULL;
	const char **formats, *file_format;
	char *filepath;
	int dummy = 0, i;
	bool_t call_succeeded = FALSE;

	marie = linphone_core_manager_new("marie_h264_rc");
	pauline = linphone_core_manager_new("pauline_h264_rc");

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	if (linphone_core_find_payload_type(pauline->lc, "h264", -1, -1) != NULL) {
		disable_all_video_codecs_except_one(pauline->lc, "h264");
		disable_all_video_codecs_except_one(marie->lc, "h264");
	}

#if defined(HAVE_OPENH264) && defined(__ANDROID__)
	libmsopenh264_init(linphone_core_get_ms_factory(marie->lc));
	linphone_core_reload_ms_plugins(marie->lc, NULL);
	libmsopenh264_init(linphone_core_get_ms_factory(pauline->lc));
	linphone_core_reload_ms_plugins(pauline->lc, NULL);
#endif
	marieParams = linphone_core_create_call_params(marie->lc, NULL);
	paulineParams = linphone_core_create_call_params(pauline->lc, NULL);

#ifdef VIDEO_ENABLED
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	if (enableVideo) {
		if (linphone_core_find_payload_type(marie->lc, video_codec, -1, -1) &&
		    linphone_core_find_payload_type(pauline->lc, video_codec, -1, -1)) {
			linphone_call_params_enable_video(marieParams, TRUE);
			linphone_call_params_enable_video(paulineParams, TRUE);
			disable_all_video_codecs_except_one(marie->lc, video_codec);
			disable_all_video_codecs_except_one(pauline->lc, video_codec);
		} else {
			ms_warning("call_recording(): the H264 payload has not been found. Only sound will be recorded");
			enableVideo = FALSE;
		}
	}
#endif
	bool_t early_record = FALSE; /*preliminary work to have a test starting video record in state Connected*/
	if (early_record) {
		LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		linphone_core_cbs_set_call_state_changed(cbs, call_state_changed_callback_to_start_record);
		linphone_core_add_callbacks(marie->lc, cbs);
		belle_sip_object_unref(cbs);
	}

	formats = linphone_core_get_supported_file_formats(marie->lc);

	for (i = 0, file_format = formats[0]; file_format != NULL; i++, file_format = formats[i]) {
		char *totalname = ms_strdup_printf("%s.%s", filename, file_format);
		filepath = bc_tester_file(totalname);
		ms_free(totalname);
		remove(filepath);
		linphone_call_params_set_record_file(marieParams, filepath);
		BC_ASSERT_TRUE(call_succeeded = call_with_params(marie, pauline, marieParams, paulineParams));
		BC_ASSERT_PTR_NOT_NULL(callInst = linphone_core_get_current_call(marie->lc));
		LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
		if ((call_succeeded == TRUE) && (callInst != NULL)) {
			if (!early_record) {
				ms_message("call_recording(): start recording into %s", filepath);
				linphone_call_start_recording(callInst);
			}
			if (strcmp(file_format, "mkv") == 0 && enableVideo) {
				VideoStream *pauline_vstream =
				    (VideoStream *)linphone_call_get_stream(pauline_call, LinphoneStreamTypeVideo);
				/* make sure that Pauline receives a RTCP FIR (Full Intra Request) requested by Marie's recorder.*/
				if (BC_ASSERT_PTR_NOT_NULL(pauline_vstream)) {
					BC_ASSERT_TRUE(
					    wait_for(marie->lc, pauline->lc, &pauline_vstream->ms_video_stat.counter_rcvd_fir, 1));
				}
			}
			wait_for_until(marie->lc, pauline->lc, &dummy, 1, 5000);
			linphone_call_stop_recording(callInst);
			end_call(marie, pauline);
			BC_ASSERT_EQUAL(bctbx_file_exist(filepath), 0, int, "%d");
		}
		remove(filepath);
		ms_free(filepath);
	}
	linphone_call_params_unref(paulineParams);
	linphone_call_params_unref(marieParams);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

static void audio_call_recording_test(void) {
	record_call("recording", FALSE, NULL);
}

static void call_with_in_dialog_update(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCallParams *params;
	bool_t call_ok;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	liblinphone_tester_check_rtcp(marie, pauline);
	params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_set_no_user_consent(params, TRUE);
	linphone_call_params_add_custom_sdp_attribute(params, "weather", "bad");
	linphone_call_update(marie_call, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	const LinphoneCallParams *pauline_remote_params = linphone_call_get_remote_params(pauline_call);
	const char *value = linphone_call_params_get_custom_sdp_attribute(pauline_remote_params, "weather");
	BC_ASSERT_PTR_NOT_NULL(value);
	if (value) BC_ASSERT_STRING_EQUAL(value, "bad");

	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_with_very_early_call_update(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCallParams *params;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(linphone_core_is_incoming_invite_pending(pauline->lc));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingProgress, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call_remote_address(pauline->lc));
	if (linphone_core_get_current_call_remote_address(pauline->lc)) {
		linphone_call_accept(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	}

	if (linphone_core_get_current_call(pauline->lc)) {
		params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
		linphone_call_params_unref(params);
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	end_call(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_in_dialog_codec_change_base(bool_t no_sdp) {
	int dummy = 0;
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCallParams *params;
	bool_t call_ok;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "sip", "keep_sdp_version", 1);
	if (!call_ok) goto end;

	liblinphone_tester_check_rtcp(marie, pauline);
	params = linphone_core_create_call_params(marie->lc, linphone_core_get_current_call(marie->lc));

	linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMU", 8000, 1),
	                                  FALSE); /*disable PCMU*/
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1),
	                                  FALSE); /*disable PCMU*/
	linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
	                                  TRUE); /*enable PCMA*/
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1),
	                                  TRUE); /*enable PCMA*/
	if (no_sdp) {
		linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
	}
	linphone_call_update(linphone_core_get_current_call(marie->lc), params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_STRING_EQUAL("PCMA", linphone_payload_type_get_mime_type(linphone_call_params_get_used_audio_payload_type(
	                                   linphone_call_get_current_params(linphone_core_get_current_call(marie->lc)))));
	BC_ASSERT_STRING_EQUAL("PCMA", linphone_payload_type_get_mime_type(linphone_call_params_get_used_audio_payload_type(
	                                   linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)))));
	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 5000);
	BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(marie), 70, int, "%i");
	BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(pauline), 70, int, "%i");

	end_call(marie, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_with_in_dialog_codec_change(void) {
	call_with_in_dialog_codec_change_base(FALSE);
}
static void call_with_in_dialog_codec_change_no_sdp(void) {
	call_with_in_dialog_codec_change_base(TRUE);
}

static void call_with_custom_supported_tags(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	const LinphoneCallParams *remote_params;
	const char *recv_supported;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_add_supported_tag(marie->lc, "pouet-tag");
	linphone_core_add_supported_tag(marie->lc, "truc-tag");
	linphone_core_add_supported_tag(marie->lc, "machin-tag");

	linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!pauline_call) goto end;

	remote_params = linphone_call_get_remote_params(pauline_call);
	recv_supported = linphone_call_params_get_custom_header(remote_params, "supported");
	BC_ASSERT_PTR_NOT_NULL(recv_supported);
	if (recv_supported) {
		BC_ASSERT_PTR_NOT_NULL(strstr(recv_supported, "pouet-tag, truc-tag, machin-tag"));
	}

	end_call(marie, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_log_from_taken_from_p_asserted_id(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *c1, *c2;
	LinphoneCallParams *params;
	const char *pauline_asserted_id = "\"Paupauche\" <sip:pauline@super.net>";
	LinphoneAddress *pauline_asserted_id_addr = linphone_address_new(pauline_asserted_id);
	LpConfig *marie_lp;
	bool_t call_ok;

	params = linphone_core_create_call_params(pauline->lc, NULL);

	linphone_call_params_add_custom_header(params, "P-Asserted-Identity", pauline_asserted_id);
	/*fixme, should be able to add several time the same header
	 * linphone_call_params_add_custom_header(params,"P-Asserted-Identity","\"Paupauche\" <tel:+12345>");*/

	marie_lp = linphone_core_get_config(marie->lc);
	linphone_config_set_int(marie_lp, "sip", "call_logs_use_asserted_id_instead_of_from", 1);

	BC_ASSERT_TRUE(call_ok = call_with_caller_params(pauline, marie, params));

	if (!call_ok) goto end;

	c1 = linphone_core_get_current_call(pauline->lc);
	c2 = linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	/*make sure remote identity is hidden*/
	BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_get_remote_address(c2), pauline_asserted_id_addr));
	linphone_address_unref(pauline_asserted_id_addr);
	end_call(pauline, marie);
end:
	linphone_call_params_unref(params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_log_from_taken_from_p_asserted_id_in_response(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *c1, *c2;
	LinphoneCallParams *params;
	const char *pauline_asserted_id = "\"Paupauche\" <sip:pauline@super.net>";
	LinphoneAddress *pauline_asserted_id_addr = linphone_address_new(pauline_asserted_id);

	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "call_logs_use_asserted_id_instead_of_from", 1);

	linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	c1 = linphone_core_get_current_call(marie->lc);
	c2 = linphone_core_get_current_call(pauline->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(c1) || !BC_ASSERT_PTR_NOT_NULL(c2)) goto end;

	params = linphone_core_create_call_params(pauline->lc, c2);
	linphone_call_params_add_custom_header(params, "P-Asserted-Identity", pauline_asserted_id);

	linphone_call_accept_early_media_with_params(c2, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));

	BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_get_remote_address(c1), pauline_asserted_id_addr));
	linphone_call_accept(c2);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_get_remote_address(c1), pauline_asserted_id_addr));
	linphone_address_unref(pauline_asserted_id_addr);
	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void incoming_invite_with_invalid_sdp(void) {
	LinphoneCoreManager *caller = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	LinphoneCallLog *cl;
	const bctbx_list_t *logs;

	callee_test_params.sdp_simulate_error = TRUE;
	BC_ASSERT_FALSE(call_with_params2(caller, callee, &caller_test_params, &callee_test_params, FALSE));

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(callee->lc));
	BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError, 1, int, "%d");
	/*call will be drop before presented to the application, because it is invalid*/
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	logs = linphone_core_get_call_logs(callee->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(logs), 1, int, "%i");
	if (logs) {
		const LinphoneErrorInfo *ei;
		cl = (LinphoneCallLog *)logs->data;
		BC_ASSERT_EQUAL(linphone_call_log_get_status(cl), LinphoneCallEarlyAborted, int, "%d");
		BC_ASSERT_TRUE(linphone_call_log_get_start_date(cl) != 0);
		ei = linphone_call_log_get_error_info(cl);
		BC_ASSERT_PTR_NOT_NULL(ei);
		if (ei) {
			BC_ASSERT_EQUAL(linphone_error_info_get_reason(ei), LinphoneReasonNotAcceptable, int, "%d");
		}
	}

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void outgoing_invite_with_invalid_sdp(void) {
	LinphoneCoreManager *caller = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *callee = linphone_core_manager_new("marie_rc");
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};

	caller_test_params.sdp_simulate_error = TRUE;
	BC_ASSERT_FALSE(call_with_params2(caller, callee, &caller_test_params, &callee_test_params, FALSE));

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(callee->lc));
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived, 1, int, "%d");
	BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError, 1, int, "%d");
	// actually callee does not receive error, because it just get a BYE from the other part
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallError, 0, int, "%d");
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallEnd, 1, int, "%d");

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void call_with_paused_no_sdp_on_resume(void) {
	int dummy = 0;
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *call_marie = NULL;
	LinphoneCallStats *stats;
	bool_t call_ok;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;

	liblinphone_tester_check_rtcp(marie, pauline);

	call_marie = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(call_marie);

	ms_message("== Call is OK ==");

	/* the called party pause the call */
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

	linphone_call_pause(call_marie);
	ms_message("== Call pausing ==");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	ms_message("== Call paused, marie call: %p ==", call_marie);

	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);

	linphone_call_resume(call_marie);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 3000);
	BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(marie), 70, int, "%i");
	stats = linphone_call_get_audio_stats(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(stats) > 70);
	linphone_call_stats_unref(stats);
	end_call(marie, pauline);
end:

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void early_media_without_sdp_in_200_base(bool_t use_video, bool_t use_ice) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCall *marie_call;
	LinphoneCallParams *params = NULL;
	LinphoneCallLog *marie_call_log;
	uint64_t connected_time = 0;
	uint64_t ended_time = 0;
	int dummy = 0;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	if (use_ice) {
		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
		/* We need RTP symmetric because ICE will put the STUN address in the C line, and no relay is made in this
		 * scenario.*/
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "symmetric", 1);
	}
	/*
	    Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/
	params = linphone_core_create_call_params(marie->lc, NULL);

	if (use_video) {

		linphone_call_params_enable_video(params, TRUE);

		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, FALSE);
	}

	marie_call = linphone_core_invite_address_with_params(marie->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	marie_call_log = linphone_call_get_call_log(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 5000));

	if (linphone_core_is_incoming_invite_pending(pauline->lc)) {
		LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);

		/* send a 183 to initiate the early media */
		linphone_call_accept_early_media(pauline_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 5000));

		liblinphone_tester_check_rtcp(marie, pauline);

		/* will send the 200OK _without_ SDP. We expect the early-media SDP to be used instead */
		sal_call_set_sdp_handling(linphone_call_get_op_as_sal_op(pauline_call), SalOpSDPSimulateRemove);
		linphone_call_accept(pauline_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 5000));
		connected_time = ms_get_cur_time_ms();
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));

		liblinphone_tester_check_rtcp(marie, pauline);
		/*just to have a call duration !=0*/
		wait_for_list(lcs, &dummy, 1, 2000);

		end_call(pauline, marie);
		ended_time = ms_get_cur_time_ms();
		BC_ASSERT_LOWER(labs((long)((linphone_call_log_get_duration(marie_call_log) * 1000) -
		                            (int64_t)(ended_time - connected_time))),
		                1500, long, "%ld");
	}
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_early_media_and_no_sdp_in_200(void) {
	early_media_without_sdp_in_200_base(FALSE, FALSE);
}

static void call_with_generic_cn(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;
	char *audio_file_with_silence = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recorded_file = bc_tester_file("result.wav");

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	remove(recorded_file);

	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_play_file(marie->lc, audio_file_with_silence);
	/*linphone_core_set_play_file(pauline->lc, NULL);*/
	linphone_core_set_record_file(pauline->lc, recorded_file);
	linphone_core_enable_generic_comfort_noise(marie->lc, TRUE);
	linphone_core_enable_generic_comfort_noise(pauline->lc, TRUE);
	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		const rtp_stats_t *rtps;
		AudioStream *astream;

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 8000);
		astream = (AudioStream *)linphone_call_get_stream(pauline_call, LinphoneStreamTypeAudio);
		rtps = rtp_session_get_stats(astream->ms.sessions.rtp_session);
		BC_ASSERT_TRUE(rtps->packet_recv <= 300 && rtps->packet_recv >= 200);
	}
	end_call(marie, pauline);

	if (pauline_call) {
		struct stat stbuf;
		int err;

		err = stat(recorded_file, &stbuf);
		BC_ASSERT_EQUAL(err, 0, int, "%d");
		if (err == 0) {
			BC_ASSERT_GREATER((int)stbuf.st_size, 120000, int, "%d");
		}
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	ms_free(audio_file_with_silence);
	bc_free(recorded_file);
}

static void call_state_changed_2(LinphoneCore *lc,
                                 BCTBX_UNUSED(LinphoneCall *call),
                                 LinphoneCallState cstate,
                                 BCTBX_UNUSED(const char *msg)) {
	LinphoneSipTransports sip_tr;
	if (cstate == LinphoneCallReleased) {
		/*to make sure transport is changed*/
		sip_tr.udp_port = 0;
		sip_tr.tcp_port = 45876;
		sip_tr.tls_port = 0;

		linphone_core_set_sip_transports(lc, &sip_tr);
	}
}

static void call_state_changed_3(BCTBX_UNUSED(LinphoneCore *lc),
                                 LinphoneCall *call,
                                 LinphoneCallState cstate,
                                 BCTBX_UNUSED(const char *msg)) {
	/*just to check multi listener in such situation*/
	LinphoneCallLog *clog = linphone_call_get_call_log(call);
	char *to = linphone_address_as_string(linphone_call_log_get_to_address(clog));
	char *from = linphone_address_as_string(linphone_call_log_get_from_address(clog));
	ms_message("Third call listener reports: %s call from [%s] to [%s], new state is [%s]",
	           linphone_call_log_get_dir(clog) == LinphoneCallIncoming ? "Incoming" : "Outgoing", from, to,
	           linphone_call_state_to_string(cstate));
	ms_free(to);
	ms_free(from);
}

static void call_with_transport_change_base(bool_t successfull_call) {
	LinphoneSipTransports sip_tr;
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	int bind_attempts;

	linphone_core_cbs_set_call_state_changed(cbs, call_state_changed_2);
	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_cbs_unref(cbs);
	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_state_changed_3);
	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_cbs_unref(cbs);

	for (bind_attempts = 0; bind_attempts < 5; bind_attempts++) {
		LinphoneSipTransports used_tr;
		memset(&used_tr, 0, sizeof(used_tr));
		sip_tr.udp_port = 0;
		sip_tr.tcp_port = 1024 + (bctbx_random() % 64000);
		sip_tr.tls_port = 0;
		linphone_core_set_sip_transports(marie->lc, &sip_tr);

		linphone_core_get_sip_transports_used(marie->lc, &used_tr);
		if (used_tr.tcp_port != sip_tr.tcp_port) {
			ms_error("Bind() apparently failed, retrying...");
		} else {
			ms_message("Bind successful.");
			break;
		}
	}

	if (successfull_call) {
		BC_ASSERT_TRUE(call(marie, pauline));
		end_call(marie, pauline);
	} else linphone_core_invite(marie->lc, "nexiste_pas");

	if (successfull_call) BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	if (successfull_call) {
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_transport_change_after_released(void) {
	call_with_transport_change_base(TRUE);
}
static void unsucessfull_call_with_transport_change_after_released(void) {
	call_with_transport_change_base(FALSE);
}

static void call_with_complex_late_offering(void) {
	LinphoneCallParams *params;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline;
	LinphoneCall *call_marie;
	bool_t call_ok;

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	if (linphone_core_find_payload_type(pauline->lc, "h264", -1, -1) != NULL) {
		disable_all_video_codecs_except_one(pauline->lc, "h264");
		disable_all_video_codecs_except_one(marie->lc, "h264");
	}

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);

	// Invite inactive Audio/video (Marie pause Pauline)
	ms_message("CONTEXT: Marie sends INVITE with SDP with all streams inactive");
	params = linphone_core_create_call_params(marie->lc, call_marie);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionInactive);
	linphone_call_params_set_video_direction(params, LinphoneMediaDirectionInactive);

	linphone_call_update(call_marie, params);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Marie sends INVITE without SDP
	ms_message("CONTEXT: Marie sends INVITE without SDP for setting streams in send-only mode");
	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
	params = linphone_core_create_call_params(marie->lc, call_marie);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendOnly);
	linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendOnly);
	linphone_call_update(call_marie, params);
	linphone_call_params_unref(params);

	// Pauline OK with sendonly
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 2));

	linphone_core_enable_sdp_200_ack(marie->lc, FALSE);

	// Pauline pause Marie
	ms_message("CONTEXT: Pauline pauses the call");
	linphone_call_pause(call_pauline);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));

	// Pauline resume Marie
	ms_message("CONTEXT: Pauline resumes the call");
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);
	linphone_call_resume(call_pauline);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 4));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallResuming, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 3));

	wait_for_until(pauline->lc, marie->lc, NULL, 0, 2000);

	// Marie invite inactive Audio/Video
	ms_message("CONTEXT: Marie sends INVITE with SDP with all streams inactive");
	params = linphone_core_create_call_params(marie->lc, call_marie);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionInactive);
	linphone_call_params_set_video_direction(params, LinphoneMediaDirectionInactive);

	linphone_call_update(call_marie, params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 3));
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 4));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 5));

	// Marie sends INVITE without SDP
	ms_message("CONTEXT: Marie sends INVITE without SDP in the purpose of re-enabling streams in sendrecv mode");
	linphone_core_enable_sdp_200_ack(marie->lc, TRUE);
	params = linphone_core_create_call_params(marie->lc, call_marie);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendRecv);
	linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendRecv);
	linphone_call_update(call_marie, params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 3));
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 5));

	linphone_core_enable_sdp_200_ack(marie->lc, FALSE);

	end_call(marie, pauline);

end:

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/* because SIP ALG (like in android phones) crash when seing a domain name in SDP, we prefer using SIP/TLS for both
 * participants*/
static void call_with_fqdn_in_sdp(void) {
	bool_t tls_supported = transport_supported(LinphoneTransportTls);
	LinphoneCoreManager *marie = linphone_core_manager_new(tls_supported ? "marie_sips_rc" : "marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(tls_supported ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *lp;
	bool_t call_ok;

	lp = linphone_core_get_config(marie->lc);
	linphone_config_set_string(lp, "rtp", "bind_address", "localhost");
	lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_string(lp, "rtp", "bind_address", "localhost");

	BC_ASSERT_TRUE(call_ok = call(marie, pauline));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(pauline, marie);

#ifdef VIDEO_ENABLED
	BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
	liblinphone_tester_check_rtcp(pauline, marie);
#endif
	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_rtp_io_mode(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recordpath = bc_tester_file("record-call_with_rtp_io_mode.wav");
	bool_t call_ok;
	int attempts;
	double similar = 1;
	const double threshold = 0.85;

	/*this test is actually attempted three times in case of failure, because the audio comparison at the end is very
	 * sensitive to jitter buffer drifts, which sometimes happen if the machine is unable to run the test in good
	 * realtime conditions */
	for (attempts = 0; attempts < 3; attempts++) {
		/* Make sure that the record file doesn't already exists, otherwise this test will append new samples to it. */
		unlink(recordpath);
		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);
		linphone_core_reset_tone_manager_stats(marie->lc);
		linphone_core_reset_tone_manager_stats(pauline->lc);

		/* The caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using
		 * call's player. */
		linphone_core_set_use_files(marie->lc, TRUE);
		linphone_core_set_play_file(marie->lc, NULL);
		linphone_core_set_record_file(marie->lc, recordpath);
		linphone_core_set_use_files(pauline->lc, FALSE);

		/* The callee uses the RTP IO mode with the PCMU codec to send back audio to the caller. */
		disable_all_audio_codecs_except_one(pauline->lc, "pcmu", -1);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sound", "rtp_io", 1);
		linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "rtp_local_addr",
		                           linphone_core_ipv6_enabled(pauline->lc) ? "::1" : "127.0.0.1");
		linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "rtp_remote_addr",
		                           linphone_core_ipv6_enabled(pauline->lc) ? "::1" : "127.0.0.1");
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sound", "rtp_local_port", 17076);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sound", "rtp_remote_port", 17076);
		linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "rtp_map", "pcmu/8000/1");

		BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
		if (!call_ok) goto end;
		player = linphone_call_get_player(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_PTR_NOT_NULL(player);
		if (player) {
			cbs = linphone_factory_create_player_cbs(linphone_factory_get());
			linphone_player_cbs_set_eof_reached(cbs, on_player_eof);
			linphone_player_cbs_set_user_data(cbs, marie);
			linphone_player_add_callbacks(player, cbs);
			BC_ASSERT_EQUAL(linphone_player_open(player, hellopath), 0, int, "%d");
			BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
		}

		/* This assert should be modified to be at least as long as the WAV file */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_player_eof, 1, 10000));
		/*wait for one second more so that last RTP packets can arrive*/
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
		end_call(pauline, marie);

		BC_ASSERT_EQUAL(ms_audio_diff(hellopath, recordpath, &similar, &audio_cmp_params, NULL, NULL), 0, int, "%d");
		if (similar >= threshold) break;
	}
	BC_ASSERT_GREATER(similar, threshold, double, "%g");
	BC_ASSERT_LOWER(similar, 1.0, double, "%g");
	if (similar >= threshold && similar <= 1.0) {
		remove(recordpath);
	}

end:
	if (cbs) linphone_player_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(recordpath);
	ms_free(hellopath);
}

static void generic_nack_received(const OrtpEventData *evd, stats *st) {
	if (rtcp_is_RTPFB(evd->packet)) {
		switch (rtcp_RTPFB_get_type(evd->packet)) {
			case RTCP_RTPFB_NACK:
				st->number_of_rtcp_generic_nack++;
				break;
			default:
				break;
		}
	}
}

static void call_with_generic_nack_rtcp_feedback(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *lp;
	LinphoneCall *call_marie;
	bool_t call_ok;
	OrtpNetworkSimulatorParams params = {0};

	params.enabled = TRUE;
	params.loss_rate = 10;
	params.consecutive_loss_probability = 0.75;
	params.mode = OrtpNetworkSimulatorOutbound;
	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);
	lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_int(lp, "rtp", "rtcp_fb_generic_nack_enabled", 1);

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	call_marie = linphone_core_get_current_call(marie->lc);
	if (call_marie) {
		AudioStream *astream = (AudioStream *)linphone_call_get_stream(call_marie, LinphoneStreamTypeAudio);
		rtp_session_enable_network_simulation(astream->ms.sessions.rtp_session, &params);
		ortp_ev_dispatcher_connect(media_stream_get_event_dispatcher(&astream->ms), ORTP_EVENT_RTCP_PACKET_RECEIVED,
		                           RTCP_RTPFB, (OrtpEvDispatcherCb)generic_nack_received, &marie->stat);
	}

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_rtcp_generic_nack, 5, 8000));
	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

// This is a custom structure used for the tests using custom RTP transport modifier.
// It is only used to count the number of sent and received packets
typedef struct _RtpTransportModifierData {
	uint64_t packetSentCount;
	uint64_t packetReceivedCount;
	MSQueue to_send;
	MSQueue to_recv;
} RtpTransportModifierData;

const char *XOR_KEY = "BELLEDONNECOMMUNICATIONS";

// Callback called when a packet is on it's way to be sent
// This is where we can do some changes on it, like encrypt it
static int rtptm_on_send(RtpTransportModifier *rtptm, mblk_t *msg) {
	RtpTransportModifierData *data = rtptm->data;
	rtp_header_t *rtp = (rtp_header_t *)msg->b_rptr;

	if (rtp->version == 0) {
		// This is probably a STUN packet, so don't count it (oRTP won't) and don't encrypt it either
		return (int)msgdsize(msg);
	}
	/*ms_message("rtptm_on_send: rtpm=%p seq=%u", rtptm, (int)rtp_get_seqnumber(msg));*/

	data->packetSentCount += 1;
	ms_queue_put(&data->to_send, dupmsg(msg));
	return 0; // Return 0 to drop the packet, it will be injected later
}

// Callback called when a packet is on it's way to be received
// This is where we can do some changes on it, like decrypt it
static int rtptm_on_receive(RtpTransportModifier *rtptm, mblk_t *msg) {
	RtpTransportModifierData *data = rtptm->data;
	rtp_header_t *rtp = (rtp_header_t *)msg->b_rptr;

	if (rtp->version == 0) {
		// This is probably a STUN packet, so don't count it (oRTP won't) and don't decrypt it either
		return (int)msgdsize(msg);
	}

	data->packetReceivedCount += 1;
	ms_queue_put(&data->to_recv, dupmsg(msg));
	return 0; // Return 0 to drop the packet, it will be injected later
}

static void rtptm_on_schedule(RtpTransportModifier *rtptm) {
	RtpTransportModifierData *data = rtptm->data;
	mblk_t *msg = NULL;

	while ((msg = ms_queue_get(&data->to_send)) != NULL) {
		int size = 0;
		unsigned char *src;
		int i = 0;

		// Mediastream can create a mblk_t with only the RTP header and setting the b_cont pointer to the actual RTP
		// content buffer In this scenario, the result of rtp_get_payload will be 0, and we won't be able to do our XOR
		// encryption on the payload The call to msgpullup will trigger a memcpy of the header and the payload in the
		// same buffer in the msg mblk_t
		msgpullup(msg, -1);
		// Now that the mblk_t buffer directly contains the header and the payload, we can get the size of the payload
		// and a pointer to it's start (we don't encrypt the RTP header)
		size = rtp_get_payload(msg, &src);

		// Just for fun, let's do a XOR encryption
		for (i = 0; i < size; i++) {
			src[i] ^= (unsigned char)XOR_KEY[i % strlen(XOR_KEY)];
		}

		meta_rtp_transport_modifier_inject_packet_to_send(rtptm->transport, rtptm, msg, 0);
		freemsg(msg);
	}

	msg = NULL;
	while ((msg = ms_queue_get(&data->to_recv)) != NULL) {
		int size = 0;
		unsigned char *src;
		int i = 0;

		// On the receiving side, there is no need for a msgpullup, the mblk_t contains the header and the payload in
		// the same buffer We just ask for the size and a pointer to the payload buffer
		size = rtp_get_payload(msg, &src);

		// Since we did a XOR encryption on the send side, we have to do it again to decrypt the payload
		for (i = 0; i < size; i++) {
			src[i] ^= (unsigned char)XOR_KEY[i % strlen(XOR_KEY)];
		}

		meta_rtp_transport_modifier_inject_packet_to_recv(rtptm->transport, rtptm, msg, 0);
	}
}

// This callback is called when the transport modifier is being destroyed
// It is a good place to free the resources allocated for the transport modifier
static void rtptm_destroy(BCTBX_UNUSED(RtpTransportModifier *rtptm)) {
	// Do nothing, we'll free it later because we need to access the RtpTransportModifierData structure after the call
	// is ended
}

// This is the callback called when the state of the call change
static void call_state_changed_4(BCTBX_UNUSED(LinphoneCore *lc),
                                 LinphoneCall *call,
                                 LinphoneCallState cstate,
                                 BCTBX_UNUSED(const char *msg)) {
	int i = 0;

	// To add a custom RTP transport modifier, we have to do it before the call is running, but after the RTP session is
	// created.
	if (cstate == LinphoneCallIncomingReceived || cstate == LinphoneCallOutgoingProgress) {
		RtpTransport *rtpt = NULL;
		RtpTransportModifierData *data = ms_new0(RtpTransportModifierData, 1);
		RtpTransportModifier *rtptm = ms_new0(RtpTransportModifier, 1);
		ms_queue_init(&data->to_send);
		ms_queue_init(&data->to_recv);
		rtptm->data = data;
		rtptm->t_process_on_send = rtptm_on_send;
		rtptm->t_process_on_receive = rtptm_on_receive;
		rtptm->t_process_on_schedule = rtptm_on_schedule;
		rtptm->t_destroy = rtptm_destroy;

		// Here we iterate on each meta rtp transport available
		for (i = 0; i < linphone_call_get_stream_count(call); i++) {
			MSFormatType type;

			rtpt = linphone_call_get_meta_rtp_transport(call, i);

			// If we wanted, we also could get the RTCP meta transports like this:
			// rtcpt = linphone_call_get_meta_rtcp_transport(call, i);

			// If you want to know which stream meta RTP transport is the current one, you can use
			type = linphone_call_get_stream_type(call, i);
			// Currently there is only MSAudio and MSVideo types, but this could change later
			if (type == MSAudio) {
				// And now we append our RTP transport modifier to the current list of modifiers
				meta_rtp_transport_append_modifier(rtpt, rtptm);
			} else if (type == MSVideo) {
				// Because the call of this test is audio only, we don't have to append our modifier to the meta RTP
				// transport from the video stream
			}
		}
		// We save the pointer to our RtpTransportModifier in the call user_data to be able to get to it later
		linphone_call_set_user_data(call, rtptm);
	}
}

static void custom_rtp_modifier(bool_t pauseResumeTest, bool_t recordTest) {
	// This will initialize two linphone core using information contained in the marie_rc and pauline_rc files and wait
	// for them to be correctly registered
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline = NULL;
	LinphoneCall *call_marie = NULL;
	const rtp_stats_t *stats;
	bool_t call_ok;
	LinphoneCoreCbs *cbs;
	RtpTransportModifier *rtptm_marie = NULL;
	RtpTransportModifier *rtptm_pauline = NULL;
	RtpTransportModifierData *data_marie = NULL;
	RtpTransportModifierData *data_pauline = NULL;
	// The following are only used for the record test
	LinphonePlayer *player;
	LinphonePlayerCbs *player_cbs = NULL;
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");          // File to be played
	char *recordpath = bc_tester_file("record-call_custom_rtp_modifier.wav"); // File to record the received sound
	double similar = 1;            // The factor of similarity between the played file and the one recorded
	const double threshold = 0.85; // Minimum similarity value to consider the record file equal to the one sent

	// We create a new LinphoneCoreCbs to listen only to the call state changes, in order to plug our RTP Transport
	// Modifier when the call will be established
	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_state_changed_4);
	linphone_core_add_callbacks(pauline->lc, cbs);
	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_cbs_unref(cbs);

	if (recordTest) { // When we do the record test, we need a file player to play the content of a sound file
		/*make sure the record file doesn't already exists, otherwise this test will append new samples to it*/
		unlink(recordpath);

		linphone_core_set_use_files(pauline->lc, TRUE);
		linphone_core_set_play_file(pauline->lc, NULL);
		linphone_core_set_record_file(pauline->lc, NULL);

		/*callee is recording and plays file*/
		linphone_core_set_use_files(marie->lc, TRUE);
		linphone_core_set_play_file(marie->lc, NULL);
		linphone_core_set_record_file(marie->lc, recordpath);
	}

	// Now the the call should be running (call state StreamsRunning)
	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));

	if (!call_ok) goto end;

	// Ref the call to keep the pointer valid even after the call is release
	call_pauline = linphone_call_ref(linphone_core_get_current_call(pauline->lc));
	call_marie = linphone_call_ref(linphone_core_get_current_call(marie->lc));

	// This is for the pause/resume test, we don't do it in the call record test to be able to check the recorded call
	// matches the file played
	if (pauseResumeTest) {
		// This only wait for 3 seconds in order to generate traffic for the test
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

		linphone_call_pause(call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

		/*stay in pause a little while in order to generate traffic*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

		linphone_call_resume(call_pauline);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

		/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

		/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
		stats = rtp_session_get_stats(
		    linphone_call_get_stream(call_pauline, LinphoneStreamTypeAudio)->sessions.rtp_session);
		BC_ASSERT_EQUAL((int)stats->cum_packet_loss, 0, int, "%d");

		end_call(pauline, marie);
	} else if (recordTest) {
		player = linphone_call_get_player(call_pauline);
		BC_ASSERT_PTR_NOT_NULL(player);
		if (player) {
			// This will ask pauline to play the file
			player_cbs = linphone_factory_create_player_cbs(linphone_factory_get());
			linphone_player_cbs_set_eof_reached(player_cbs, on_player_eof);
			linphone_player_cbs_set_user_data(player_cbs, pauline);
			linphone_player_add_callbacks(player, player_cbs);
			BC_ASSERT_EQUAL(linphone_player_open(player, hellopath), 0, int, "%d");
			BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
		}
		/* This assert should be modified to be at least as long as the WAV file */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_player_eof, 1, 10000));
		/*wait one second more for transmission to be fully ended (transmission time + jitter buffer)*/
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);

		end_call(pauline, marie);

		// Now we compute a similarity factor between the original file and the one we recorded on the callee side
		BC_ASSERT_EQUAL(ms_audio_diff(hellopath, recordpath, &similar, &audio_cmp_params, NULL, NULL), 0, int, "%d");

		BC_ASSERT_GREATER(similar, threshold, double, "%g");
		BC_ASSERT_LOWER(similar, 1.0, double, "%g");
		if (similar >= threshold && similar <= 1.0) {
			// If the similarity value is between perfect (1) and our threshold (0.9), then we delete the file used for
			// the record
			remove(recordpath);
		}
	} else {
		// This only wait for 3 seconds in order to generate traffic for the test
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

		// We termine the call and check the stats to see if the call is correctly ended on both sides
		end_call(pauline, marie);
	}

	// Now we can go fetch our custom structure and check the number of packets sent/received is the same on both sides
	rtptm_marie = (RtpTransportModifier *)linphone_call_get_user_data(call_marie);
	rtptm_pauline = (RtpTransportModifier *)linphone_call_get_user_data(call_pauline);
	data_marie = (RtpTransportModifierData *)rtptm_marie->data;
	data_pauline = (RtpTransportModifierData *)rtptm_pauline->data;

	BC_ASSERT_PTR_NOT_NULL(data_marie);
	BC_ASSERT_PTR_NOT_NULL(data_pauline);
	ms_message("Marie sent %i RTP packets and received %i (through our modifier)", (int)data_marie->packetSentCount,
	           (int)data_marie->packetReceivedCount);
	ms_message("Pauline sent %i RTP packets and received %i (through our modifier)", (int)data_pauline->packetSentCount,
	           (int)data_pauline->packetReceivedCount);
	// There will be a few RTP packets sent on marie's side before the call is ended at pauline's request, so we need
	// the threshold
	BC_ASSERT_TRUE(data_marie->packetSentCount - data_pauline->packetReceivedCount < 50);
	BC_ASSERT_TRUE(data_pauline->packetSentCount - data_marie->packetReceivedCount < 50);
	// At this point, we know each packet that has been processed in the send callback of our RTP modifier also go
	// through the recv callback of the remote.

	// Now we want to ensure that all sent RTP packets actually go through our RTP transport modifier and thus no packet
	// leave without being processed (by any operation we might want to do on it)
	{
		LinphoneCallStats *marie_stats = linphone_call_get_audio_stats(call_marie);
		LinphoneCallStats *pauline_stats = linphone_call_get_audio_stats(call_pauline);
		rtp_stats_t marie_rtp_stats = *linphone_call_stats_get_rtp_stats(marie_stats);
		rtp_stats_t pauline_rtp_stats = *linphone_call_stats_get_rtp_stats(pauline_stats);
		ms_message("Marie sent %i RTP packets and received %i (for real)", (int)marie_rtp_stats.packet_sent,
		           (int)marie_rtp_stats.packet_recv);
		ms_message("Pauline sent %i RTP packets and received %i (for real)", (int)pauline_rtp_stats.packet_sent,
		           (int)pauline_rtp_stats.packet_recv);
		BC_ASSERT_EQUAL(data_marie->packetReceivedCount, marie_rtp_stats.packet_recv, unsigned long long, "%llu");
		BC_ASSERT_EQUAL(data_marie->packetSentCount, marie_rtp_stats.packet_sent, unsigned long long, "%llu");
		// There can be a small difference between the number of packets received in the modifier and the number
		// processed in reception because the processing is asynchronous
		BC_ASSERT_TRUE(data_pauline->packetReceivedCount - pauline_rtp_stats.packet_recv < 20);
		BC_ASSERT_TRUE(data_pauline->packetSentCount == pauline_rtp_stats.packet_sent);
		linphone_call_stats_unref(marie_stats);
		linphone_call_stats_unref(pauline_stats);
	}

end:
	// Since we didn't free the resources of our RTP transport modifier in the rtptm_destroy callback, we'll do it here
	if (data_pauline) {
		ms_queue_flush(&data_pauline->to_send);
		ms_queue_flush(&data_pauline->to_recv);
		ms_free(data_pauline);
	}
	ms_free(rtptm_pauline);
	if (data_marie) {
		ms_queue_flush(&data_marie->to_send);
		ms_queue_flush(&data_marie->to_recv);
		ms_free(data_marie);
	}
	ms_free(rtptm_marie);

	// Unref the previously ref calls
	if (call_marie) {
		linphone_call_unref(call_marie);
	}
	if (call_pauline) {
		linphone_call_unref(call_pauline);
	}

	if (player_cbs) linphone_player_cbs_unref(player_cbs);

	// The test is finished, the linphone core are no longer needed, we can safely free them
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	bc_free(recordpath);
	bc_free(hellopath);
}

static void call_with_custom_rtp_modifier(void) {
	custom_rtp_modifier(FALSE, FALSE);
}

static void call_paused_resumed_with_custom_rtp_modifier(void) {
	custom_rtp_modifier(TRUE, FALSE);
}

static void call_record_with_custom_rtp_modifier(void) {
	custom_rtp_modifier(FALSE, TRUE);
}

static void call_logs_if_no_db_set(void) {
#ifndef HAVE_DB_STORAGE
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_call_logs_rc");

	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_call_logs(laure->lc)), 0, int, "%d");

	BC_ASSERT_TRUE(call(marie, laure));
	wait_for_until(marie->lc, laure->lc, NULL, 5, 1000);
	end_call(marie, laure);

	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_call_logs(laure->lc)), 1, int, "%d");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(laure);
#else
	BC_PASS("This test needs no database to run");
#endif
}

static void call_logs_migrate(void) {
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_call_logs_rc");
	size_t i = 0;
	int incoming_count = 0, outgoing_count = 0, missed_count = 0, aborted_count = 0, decline_count = 0,
	    video_enabled_count = 0;
	bctbx_list_t **call_logs_attr = NULL;

	// Need to reset migration flag & restore call logs in rc file
	// because when the Core started it automatically migrated the call logs & removed them from the rc
	char *logs_rc = bc_tester_res("rcfiles/laure_call_logs_rc");
	linphone_config_set_int(linphone_core_get_config(laure->lc), "misc", "call_logs_migration_done", 0);
	linphone_config_read_file(linphone_core_get_config(laure->lc), logs_rc);
	linphone_core_migrate_logs_from_rc_to_db(laure->lc);

	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_call_logs(laure->lc)), 10, int, "%d");

	// Need to reset migration flag & restore call logs in rc file
	// because when the Core started it automatically migrated the call logs & removed them from the rc
	linphone_config_set_int(linphone_core_get_config(laure->lc), "misc", "call_logs_migration_done", 0);
	linphone_config_read_file(linphone_core_get_config(laure->lc), logs_rc);
	ms_free(logs_rc);

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(laure->lc), 10, int, "%d");

	for (; i < bctbx_list_size(linphone_core_get_call_logs(laure->lc)); i++) {
		LinphoneCallLog *log = bctbx_list_nth_data(linphone_core_get_call_logs(laure->lc), (int)i);
		LinphoneCallStatus state = linphone_call_log_get_status(log);
		LinphoneCallDir direction = linphone_call_log_get_dir(log);

		if (state == LinphoneCallAborted) {
			aborted_count += 1;
		} else if (state == LinphoneCallMissed) {
			missed_count += 1;
		} else if (state == LinphoneCallDeclined) {
			decline_count += 1;
		}

		if (direction == LinphoneCallOutgoing) {
			outgoing_count += 1;
		} else {
			incoming_count += 1;
		}

		if (linphone_call_log_video_enabled(log)) {
			video_enabled_count += 1;
		}
	}
	BC_ASSERT_EQUAL(incoming_count, 5, int, "%d");
	BC_ASSERT_EQUAL(outgoing_count, 5, int, "%d");
	BC_ASSERT_EQUAL(missed_count, 1, int, "%d");
	BC_ASSERT_EQUAL(aborted_count, 3, int, "%d");
	BC_ASSERT_EQUAL(decline_count, 2, int, "%d");
	BC_ASSERT_EQUAL(video_enabled_count, 3, int, "%d");

	{
		LinphoneCallLog *log = linphone_core_get_last_outgoing_call_log(laure->lc);
		BC_ASSERT_PTR_NOT_NULL(log);
		if (log) {
			BC_ASSERT_EQUAL((int)linphone_call_log_get_start_date(log), 1441738272, int, "%d");
			linphone_call_log_unref(log);
			log = NULL;
		}
	}

	call_logs_attr = linphone_core_get_call_logs_attribute(laure->lc);
	*call_logs_attr = bctbx_list_free_with_data(*call_logs_attr, (void (*)(void *))linphone_call_log_unref);
	*call_logs_attr = linphone_core_read_call_logs_from_config_file(laure->lc);

	linphone_core_manager_destroy(laure);
}

static void call_logs_sqlite_storage(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *logs = NULL;
	LinphoneCallLog *call_log = NULL;
	LinphoneAddress *laure = NULL;
	time_t start_time = 0;

	bctbx_list_t *call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 0);
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);

	BC_ASSERT_TRUE(call(marie, pauline));
	wait_for_until(marie->lc, pauline->lc, NULL, 5, 500);
	call_log = linphone_call_get_call_log(linphone_core_get_current_call(marie->lc));
	start_time = linphone_call_log_get_start_date(call_log);
	linphone_call_log_set_ref_key(call_log, "ref_key");
	end_call(marie, pauline);

	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 1);
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);

	logs = linphone_account_get_call_logs_for_address(
	    linphone_core_get_default_account(marie->lc),
	    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc)));
	BC_ASSERT_TRUE(bctbx_list_size(logs) == 1);
	bctbx_list_free_with_data(logs, (void (*)(void *))linphone_call_log_unref);

	laure = linphone_address_new("\"Laure\" <sip:laure@sip.example.org>");
	logs = linphone_account_get_call_logs_for_address(linphone_core_get_default_account(marie->lc), laure);
	BC_ASSERT_TRUE(bctbx_list_size(logs) == 0);
	linphone_address_unref(laure);

	logs = linphone_account_get_call_logs_for_address(
	    linphone_core_get_default_account(marie->lc),
	    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc)));
	if (BC_ASSERT_TRUE(bctbx_list_size(logs) == 1)) {
		const char *call_id;
		const char *ref_key;
		call_log = (LinphoneCallLog *)bctbx_list_get_data(logs);
		ref_key = linphone_call_log_get_ref_key(call_log);
		BC_ASSERT_EQUAL(linphone_call_log_get_dir(call_log), LinphoneCallOutgoing, int, "%d");
		BC_ASSERT_LOWER(linphone_call_log_get_duration(call_log), 2, int, "%d");
		BC_ASSERT_TRUE(linphone_address_weak_equal(
		    linphone_call_log_get_from_address(call_log),
		    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(marie->lc))));
		BC_ASSERT_TRUE(linphone_address_weak_equal(
		    linphone_call_log_get_to_address(call_log),
		    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc))));
		BC_ASSERT_GREATER(linphone_call_log_get_quality(call_log), -1, float, "%.1f");
		BC_ASSERT_PTR_NOT_NULL(ref_key);
		if (ref_key) {
			BC_ASSERT_STRING_EQUAL(ref_key, "ref_key");
		}

		call_id = linphone_call_log_get_call_id(call_log);
		BC_ASSERT_PTR_NOT_NULL(call_id);
		{
			LinphoneCallLog *find_call_log = linphone_core_find_call_log_from_call_id(marie->lc, call_id);
			BC_ASSERT_PTR_NOT_NULL(find_call_log);
			if (find_call_log) linphone_call_log_unref(find_call_log);
		}

		BC_ASSERT_TRUE(linphone_address_equal(
		    linphone_call_log_get_remote_address(call_log),
		    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc))));

		BC_ASSERT_EQUAL(linphone_call_log_get_start_date(call_log), start_time, unsigned long long, "%llu");
		BC_ASSERT_EQUAL(linphone_call_log_get_status(call_log), LinphoneCallSuccess, int, "%d");
	}

	linphone_core_delete_call_log(marie->lc, (LinphoneCallLog *)bctbx_list_nth_data(logs, 0));
	bctbx_list_free_with_data(logs, (void (*)(void *))linphone_call_log_unref);

	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 0);
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	linphone_core_reset_tone_manager_stats(marie->lc);
	linphone_core_reset_tone_manager_stats(pauline->lc);
	BC_ASSERT_TRUE(call(marie, pauline));
	end_call(marie, pauline);

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	linphone_core_reset_tone_manager_stats(marie->lc);
	linphone_core_reset_tone_manager_stats(pauline->lc);
	BC_ASSERT_TRUE(call(marie, pauline));
	end_call(marie, pauline);

	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 2);
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);

	linphone_account_clear_call_logs(linphone_core_get_default_account(marie->lc));
	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(marie->lc));
	BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 0);
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void _call_with_http_proxy(bool_t use_ipv4) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bool_t call_ok;
	LinphoneCall *marie_call, *pauline_call;
	LinphoneAddress *contact_addr;
	LinphoneAddress *contact_addr2;

	if (!transport_supported(LinphoneTransportTls)) {
		ms_message("Test skipped because no tls support");
		goto end;
	}
	linphone_core_remove_supported_tag(pauline->lc,
	                                   "gruu"); /*with gruu, we have no access to the "public IP from contact*/
	linphone_core_remove_supported_tag(marie->lc, "gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneAddress *http_proxy_example_org_ip;
	LinphoneAddress *http_proxy_fqdn = linphone_address_new("sip:http-proxy.example.org:8888");

	http_proxy_example_org_ip = linphone_core_manager_resolve(marie, http_proxy_fqdn);

	linphone_core_set_http_proxy_host(pauline->lc, "http-proxy.example.org");

	if (use_ipv4) {
		LinphoneAddress *v4proxy =
		    linphone_factory_create_address(linphone_factory_get(), "sip:sipv4.example.org;transport=tls");
		LinphoneAccount *account = linphone_core_get_default_account(pauline->lc);
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		bctbx_list_t *routes = bctbx_list_append(NULL, v4proxy);
		linphone_account_params_set_server_address(params, v4proxy);
		linphone_account_params_set_routes_addresses(params, routes);
		linphone_account_set_params(account, params);
		bctbx_list_free(routes);
		linphone_address_unref(v4proxy);
		linphone_account_params_unref(params);
	}

	linphone_core_manager_start(pauline, TRUE);

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (!call_ok) goto end;

	marie_call = linphone_core_get_current_call(marie->lc);
	contact_addr = linphone_address_new(linphone_call_get_remote_contact(marie_call));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	contact_addr2 = linphone_address_new(linphone_call_get_remote_contact(pauline_call));

	/* Since we may not see the http proxy address in the "fixed" contact address because the http proxy can be
	 * behind a NAT (typically with docker), we are simply going to check that the contact addresses exposed by marie
	 * and pauline are different. If they are equal, this clearly means that the INVITE didn't went through the http
	 * proxy*/

	BC_ASSERT_STRING_NOT_EQUAL(linphone_address_get_domain(contact_addr), linphone_address_get_domain(contact_addr2));
	linphone_address_unref(contact_addr);
	linphone_address_unref(contact_addr2);
	linphone_address_unref(http_proxy_fqdn);
	linphone_address_unref(http_proxy_example_org_ip);
	end_call(marie, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_http_proxy(void) {
	_call_with_http_proxy(FALSE);
}

static void call_with_http_proxy_v4(void) {
	_call_with_http_proxy(TRUE);
}

void _call_with_rtcp_mux(bool_t caller_rtcp_mux, bool_t callee_rtcp_mux, bool_t with_ice, bool_t with_ice_reinvite) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const LinphoneCallParams *params;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	if (caller_rtcp_mux) {
		linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_mux", 1);
	}
	if (callee_rtcp_mux) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_mux", 1);
	}
	if (with_ice) {
		linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
		linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);

		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
		enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);
	}
	if (!with_ice_reinvite) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "update_call_when_ice_completed", 0);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "update_call_when_ice_completed", 0);
	}

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;

	params = linphone_call_get_remote_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(caller_rtcp_mux == (linphone_call_params_get_custom_sdp_media_attribute(
	                                       params, LinphoneStreamTypeAudio, "rtcp-mux") != NULL));
	if (caller_rtcp_mux) {
		params = linphone_call_get_remote_params(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(callee_rtcp_mux == (linphone_call_params_get_custom_sdp_media_attribute(
		                                       params, LinphoneStreamTypeAudio, "rtcp-mux") != NULL));
	}

	if (with_ice) {
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
	}
	liblinphone_tester_check_rtcp(marie, pauline);

	if (caller_rtcp_mux && callee_rtcp_mux) {
		BC_ASSERT_EQUAL(marie->stat.number_of_rtcp_received_via_mux, marie->stat.number_of_rtcp_received, int, "%i");

		BC_ASSERT_EQUAL(pauline->stat.number_of_rtcp_received_via_mux, pauline->stat.number_of_rtcp_received, int,
		                "%i");

	} else {
		BC_ASSERT_TRUE(marie->stat.number_of_rtcp_received_via_mux == 0);
		BC_ASSERT_TRUE(pauline->stat.number_of_rtcp_received_via_mux == 0);
	}

	check_media_direction(pauline, linphone_core_get_current_call(pauline->lc), lcs, LinphoneMediaDirectionSendRecv,
	                      LinphoneMediaDirectionInvalid);
	end_call(marie, pauline);

end:
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void call_with_rtcp_mux(void) {
	_call_with_rtcp_mux(TRUE, TRUE, FALSE, TRUE);
}

static void call_with_rtcp_mux_not_accepted(void) {
	_call_with_rtcp_mux(TRUE, FALSE, FALSE, TRUE);
}

static void v6_to_v4_call_without_relay(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	bctbx_list_t *lcs = NULL;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()) {
		marie = linphone_core_manager_new("marie_rc");
		pauline = linphone_core_manager_new_with_proxies_check("pauline_tcp_rc", FALSE);

		lcs = bctbx_list_append(lcs, marie->lc);
		lcs = bctbx_list_append(lcs, pauline->lc);
		linphone_core_enable_ipv6(pauline->lc, FALSE);
		/*RTP symmetric must be enabled for this test to pass, because the IPv4-only client cannot send by itself to
		 * the IPv6 address in the SDP*/
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "symmetric", 1);
		linphone_core_manager_start(pauline, TRUE);

		if (BC_ASSERT_TRUE(call(marie, pauline))) {
			check_media_direction(marie, linphone_core_get_current_call(marie->lc), lcs, LinphoneMediaDirectionSendRecv,
			                      LinphoneMediaDirectionInvalid);

			liblinphone_tester_check_rtcp(marie, pauline);
			end_call(marie, pauline);
		}
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		bctbx_list_free(lcs);

	} else ms_warning("Test skipped, dual stack not available");
}

static void v6_call_over_nat_64(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	if (!liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()) {

		marie = linphone_core_manager_new("marie_nat64_rc");
		pauline = linphone_core_manager_new("pauline_nat64_rc");

		linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
		linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);

		BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 2000));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 2000));

		BC_ASSERT_TRUE(call(marie, pauline));

		liblinphone_tester_check_rtcp(marie, pauline);
		end_call(marie, pauline);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);

	} else ms_warning("Test skipped, no ipv6 nat64 available");
}

static void my_call_state_changed_cb(LinphoneCore *lc,
                                     BCTBX_UNUSED(LinphoneCall *call),
                                     LinphoneCallState state,
                                     BCTBX_UNUSED(const char *text)) {
	if (state == LinphoneCallError) {
		linphone_core_set_network_reachable(lc, FALSE);
	}
}

/*This test simulates a case where a 404 not found is received from server when attempting to make a call,
 * and the app decides to immediately shutdown the network reachability*/
static void call_with_network_reachable_down_in_callback(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	LinphoneCall *call;

	linphone_core_cbs_set_call_state_changed(cbs, my_call_state_changed_cb);

	marie = linphone_core_manager_new("laure_rc_udp");

	linphone_core_add_callbacks(marie->lc, cbs);

	call = linphone_core_invite(marie->lc, "inexistant_username_xbfuuuf");
	BC_ASSERT_PTR_NOT_NULL(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallError, 1));

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
}

static void simple_call_with_gruu(void) {
	const LinphoneAddress *pauline_addr, *marie_addr;
	LinphoneCall *marie_call = NULL;
	LinphoneCall *pauline_call = NULL;
	LinphoneProxyConfig *pauline_cfg;
	LinphoneAddress *contact_addr;
	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_add_supported_tag(marie->lc, "gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline, "pauline_tcp_rc", NULL);
	linphone_core_add_supported_tag(pauline->lc, "gruu");
	linphone_core_manager_start(pauline, TRUE);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 1));

	pauline_cfg = linphone_core_get_default_proxy_config(pauline->lc);
	pauline_addr = linphone_proxy_config_get_contact(pauline_cfg);

	BC_ASSERT_PTR_NOT_NULL(pauline_addr);
	BC_ASSERT_TRUE(linphone_address_has_uri_param(pauline_addr, "gr"));
	BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(pauline_addr), "sip.example.org");

	linphone_core_disable_call_ringing(marie->lc, TRUE);
	linphone_core_enable_call_tone_indications(marie->lc, FALSE);

	marie_call = linphone_core_invite_address(marie->lc, pauline_addr);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (!marie_call) goto end;
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (!pauline_call) goto end;

	marie_addr = linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc));
	BC_ASSERT_TRUE(linphone_address_has_uri_param(marie_addr, "gr"));
	BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(marie_addr), "sip.example.org");
	contact_addr = linphone_address_new(linphone_call_get_remote_contact(pauline_call));
	if (!BC_ASSERT_TRUE(linphone_address_equal(contact_addr, marie_addr))) {
		char *expected = linphone_address_as_string(marie_addr);
		char *result = linphone_address_as_string(contact_addr);
		ms_error("Expected contact is [%s], result is [%s]", expected, result);
		ms_free(expected);
		ms_free(result);
	}
	linphone_address_unref(contact_addr);

	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	const LinphoneCallParams *marie_call_lparams = linphone_call_get_params(marie_call);
	BC_ASSERT_TRUE(linphone_call_params_ringing_disabled(marie_call_lparams));
	BC_ASSERT_FALSE(linphone_call_params_tone_indications_enabled(marie_call_lparams));

	const LinphoneCallParams *pauline_call_lparams = linphone_call_get_params(pauline_call);
	BC_ASSERT_FALSE(linphone_call_params_ringing_disabled(pauline_call_lparams));
	BC_ASSERT_TRUE(linphone_call_params_tone_indications_enabled(pauline_call_lparams));

	contact_addr = linphone_address_new(linphone_call_get_remote_contact(marie_call));
	if (!BC_ASSERT_TRUE(linphone_address_equal(contact_addr, pauline_addr))) {
		char *expected = linphone_address_as_string(pauline_addr);
		char *result = linphone_address_as_string(contact_addr);
		ms_error("Expected contact is [%s], result is [%s]", expected, result);
		ms_free(expected);
		ms_free(result);
	}
	linphone_address_unref(contact_addr);

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);

	// BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	// BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_call_with_gruu_only_one_device_ring(void) {
	const LinphoneAddress *pauline_addr;
	const LinphoneAddress *pauline_addr2;
	LinphoneCall *marie_call = NULL;
	LinphoneCall *pauline_call = NULL;
	LinphoneProxyConfig *pauline_cfg;
	LinphoneProxyConfig *pauline_cfg2;

	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_add_supported_tag(marie->lc, "gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline, transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc",
	                           NULL);
	linphone_core_add_supported_tag(pauline->lc, "gruu");
	linphone_core_manager_start(pauline, TRUE);
	LinphoneCoreManager *pauline2 = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline2, transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc",
	                           NULL);
	linphone_core_add_supported_tag(pauline2->lc, "gruu");
	linphone_core_manager_start(pauline2, TRUE);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline2->stat.number_of_LinphoneRegistrationOk, 1));

	pauline_cfg = linphone_core_get_default_proxy_config(pauline->lc);
	pauline_addr = linphone_proxy_config_get_contact(pauline_cfg);
	BC_ASSERT_PTR_NOT_NULL(pauline_addr);
	char *pauline_addr_str = linphone_address_as_string_uri_only(pauline_addr);
	BC_ASSERT_PTR_NOT_NULL(strstr(pauline_addr_str, "gr"));
	pauline_cfg2 = linphone_core_get_default_proxy_config(pauline2->lc);
	pauline_addr2 = linphone_proxy_config_get_contact(pauline_cfg2);
	BC_ASSERT_PTR_NOT_NULL(pauline_addr2);
	char *pauline_addr2_str = linphone_address_as_string_uri_only(pauline_addr2);
	BC_ASSERT_PTR_NOT_NULL(strstr(pauline_addr2_str, "gr"));
	BC_ASSERT_NOT_EQUAL(pauline_addr_str, pauline_addr2_str, char *, "%s"); // Not same GRUU
	ms_free(pauline_addr_str);
	ms_free(pauline_addr2_str);

	marie_call = linphone_core_invite_address(marie->lc, pauline_addr);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (!marie_call) goto end;
	BC_ASSERT_FALSE(wait_for(marie->lc, pauline2->lc, &pauline2->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (!pauline_call) goto end;

	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_call_terminate(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(marie);
}

static void async_core_stop_after_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 2000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 2000));

	BC_ASSERT_TRUE(call(marie, pauline));
	linphone_core_stop_async(marie->lc);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneGlobalShutdown, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneGlobalOff, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd, 1, int, "%d");

	linphone_core_manager_destroy_after_stop_async(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_avpf_mismatch(void) {
	LinphoneCoreManager *marie, *pauline;
	const LinphoneAddress *from;
	LinphoneCall *pauline_call;
	LinphoneProxyConfig *marie_cfg;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/* with the account manager, we might lose the identity */
	marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	{
		LinphoneAddress *marie_addr = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_cfg));
		linphone_address_set_display_name(marie_addr, "Super Marie");

		linphone_proxy_config_edit(marie_cfg);
		linphone_proxy_config_set_identity_address(marie_cfg, marie_addr);
		// Enable AVPF from the caller
		linphone_proxy_config_set_avpf_mode(marie_cfg, LinphoneAVPFEnabled);
		linphone_proxy_config_done(marie_cfg);

		linphone_address_unref(marie_addr);
	}

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);

	// Disable AVPF from the callee
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_avpf(pauline_params, FALSE);

	// Test early media
	linphone_call_accept_early_media_with_params(pauline_call, pauline_params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));

	linphone_call_accept_with_params(pauline_call, pauline_params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallConnected, 1));
	linphone_call_params_unref(pauline_params);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, int, "%d");
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	/*check that display name is correctly propagated in From */
	if (pauline_call) {
		from = linphone_call_get_remote_address(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_PTR_NOT_NULL(from);
		if (from) {
			const char *dname = linphone_address_get_display_name(from);
			BC_ASSERT_PTR_NOT_NULL(dname);
			if (dname) {
				BC_ASSERT_STRING_EQUAL(dname, "Super Marie");
			}
		}
	}

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void on_remote_recording(LinphoneCall *call, bool_t recording) {
	stats *user_stats = (stats *)linphone_call_get_user_data(call);
	if (recording) {
		user_stats->number_of_LinphoneRemoteRecordingEnabled++;
	} else {
		user_stats->number_of_LinphoneRemoteRecordingDisabled++;
	}
}

static void call_recording_with_record_aware_base(bool_t record_aware, bool_t both_recording) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"));
	char *filepath_pauline = NULL;

	if (record_aware) {
		linphone_core_set_record_aware_enabled(marie->lc, TRUE);
		linphone_core_set_record_aware_enabled(pauline->lc, TRUE);
	}

	LinphoneCallParams *marieParams = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *paulineParams = NULL;

	const char *name_marie = "record-marie.wav";
	char *filepath_marie = bc_tester_file(name_marie);
	remove(filepath_marie);
	linphone_call_params_set_record_file(marieParams, filepath_marie);

	if (both_recording) {
		paulineParams = linphone_core_create_call_params(pauline->lc, NULL);
		const char *name_pauline = "record-pauline.wav";
		filepath_pauline = bc_tester_file(name_pauline);
		remove(filepath_pauline);
		linphone_call_params_set_record_file(paulineParams, filepath_pauline);
	}

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marieParams, paulineParams));
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);

	linphone_call_set_user_data(pauline_call, &pauline->stat);
	linphone_call_set_user_data(marie_call, &marie->stat);

	// Set pauline remote recording callback
	LinphoneCallCbs *call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	linphone_call_cbs_set_remote_recording(call_cbs, on_remote_recording);
	linphone_call_add_callbacks(pauline_call, call_cbs);
	linphone_call_cbs_unref(call_cbs);

	if (both_recording) {
		// Set marie remote recording callback
		call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
		linphone_call_cbs_set_remote_recording(call_cbs, on_remote_recording);
		linphone_call_add_callbacks(marie_call, call_cbs);
		linphone_call_cbs_unref(call_cbs);
	}

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_call_start_recording(marie_call);
	BC_ASSERT_TRUE(linphone_call_is_recording(marie_call));

	if (record_aware) {
		// The recording start should trigger and update
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_TRUE(linphone_call_params_is_recording(linphone_call_get_remote_params(pauline_call)));
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneRemoteRecordingEnabled, 1, int, "%d");
	}

	linphone_call_stop_recording(marie_call);
	BC_ASSERT_FALSE(linphone_call_is_recording(marie_call));

	if (record_aware) {
		// Same when stopping the recording
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));

		BC_ASSERT_FALSE(linphone_call_params_is_recording(linphone_call_get_remote_params(pauline_call)));
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneRemoteRecordingDisabled, 1, int, "%d");
	}

	if (both_recording) {
		linphone_call_start_recording(pauline_call);
		BC_ASSERT_TRUE(linphone_call_is_recording(pauline_call));

		if (record_aware) {
			// The recording start should trigger and update
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 4));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 4));

			BC_ASSERT_TRUE(linphone_call_params_is_recording(linphone_call_get_remote_params(marie_call)));
			BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRemoteRecordingEnabled, 1, int, "%d");
		}

		linphone_call_stop_recording(pauline_call);
		BC_ASSERT_FALSE(linphone_call_is_recording(pauline_call));

		if (record_aware) {
			// Same when stopping the recording
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 2));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 5));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 5));

			BC_ASSERT_FALSE(linphone_call_params_is_recording(linphone_call_get_remote_params(marie_call)));
			BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRemoteRecordingDisabled, 1, int, "%d");
		}

		end_call(marie, pauline);

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

		remove(filepath_pauline);
		ms_free(filepath_pauline);
		linphone_call_params_unref(paulineParams);
	}

	remove(filepath_marie);
	ms_free(filepath_marie);
	linphone_call_params_unref(marieParams);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_recording_with_record_aware(void) {
	call_recording_with_record_aware_base(TRUE, FALSE);
}

static void call_recording_with_record_aware_both_recording(void) {
	call_recording_with_record_aware_base(TRUE, TRUE);
}

static void call_recording_without_record_aware(void) {
	call_recording_with_record_aware_base(FALSE, FALSE);
}

static void call_without_automatic_180_ringing(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_auto_send_ringing(pauline->lc, FALSE);

	linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_FALSE(
	    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 1000));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	int dummy = 0;
	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 2000);

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_notify_ringing(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));

	linphone_call_accept(pauline_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_automatic_180_ringing_but_early_media(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	linphone_core_enable_auto_send_ringing(pauline->lc, FALSE);

	// The 183 early media should be sent
	BC_ASSERT_TRUE(call_ok = call(pauline, marie));

	if (!call_ok) goto end;
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, int, "%d");

	wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);

	/*added because a bug related to early-media caused the Connected state to be reached two times*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallConnected, 1, int, "%d");

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_state_changed_accept_early_media(BCTBX_UNUSED(LinphoneCore *lc),
                                                  LinphoneCall *call,
                                                  LinphoneCallState cstate,
                                                  BCTBX_UNUSED(const char *msg)) {
	if (cstate == LinphoneCallIncomingReceived) {
		/* send a 183 to initiate the early media */
		linphone_call_accept_early_media(call);
	}
}

static void call_with_early_media_accepted_state_changed_callback(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCall *marie_call;

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(cbs, call_state_changed_accept_early_media);
	linphone_core_add_callbacks(pauline->lc, cbs);
	linphone_core_cbs_unref(cbs);

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	/*
	    Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));

	if (linphone_core_is_incoming_invite_pending(pauline->lc)) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 5000));
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingRinging, 0, int, "%d");

		liblinphone_tester_check_rtcp(marie, pauline);

		linphone_call_terminate(marie_call);

		bctbx_list_free(lcs);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_audio_stream_added_later_on(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	bool_t call_ok;

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_audio(marie_params, TRUE);
	linphone_call_params_enable_video(marie_params, TRUE);

	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_audio(pauline_params, FALSE);
	linphone_call_params_enable_video(pauline_params, TRUE);

	BC_ASSERT_TRUE((call_ok = call_with_params(pauline, marie, pauline_params, marie_params)));
	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	if (!call_ok) goto end;

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);

	if (marie_call) {
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_rparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_cparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
	}
	if (pauline_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_lparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_rparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_FALSE(linphone_call_params_audio_enabled(call_cparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
	}

	stats initial_pauline_stat = pauline->stat;
	stats initial_marie_stat = marie->stat;
	LinphoneCallParams *pauline_new_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_audio(pauline_new_params, TRUE);
	linphone_call_update(pauline_call, pauline_new_params);
	linphone_call_params_unref(pauline_new_params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
	                        initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating,
	                        initial_pauline_stat.number_of_LinphoneCallUpdating + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

	if (marie_call) {
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_call);
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(call_rparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_call);
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(call_cparams));
	}
	if (pauline_call) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(call_lparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(call_rparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
		BC_ASSERT_TRUE(linphone_call_params_audio_enabled(call_cparams));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
	}

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_maformed_from(void) {
	const char *crashing_invite =
	    "INVITE sip:631453@212.55.48.36:51230;transport=udp SIP/2.0\r\n"
	    "Via: SIP/2.0/UDP 212.55.48.2:5060;branch=z9hG4bKac1473882254\r\n"
	    "Max-Forwards: 19\r\n"
	    "From: \"\r\n\" <sip:mickey@example.com;user=phone>;tag=1c849167855\r\n"
	    "To: \"Bugs Bunny\" <sip:bunny@example.com>\r\n"
	    "Call-ID: 9771187781832022142418@212.55.48.2\r\n"
	    "CSeq: 1 INVITE\r\n"
	    "Contact: <sip:212.55.48.2:5060>\r\n"
	    "Supported: 100rel,sdp-anat\r\n"
	    "Allow: ACK,BYE,CANCEL,INFO,INVITE,OPTIONS,PRACK,REFER,NOTIFY,UPDATE\r\n"
	    "User-Agent: vSBC PROD/v.7.20A.258.459\r\n"
	    "Accept:application/media_control+xml,application/sdp,multipart/mixed\r\n"
	    "Recv-Info:x-broadworks-client-session-info\r\n"
	    "Content-Type: application/sdp\r\n"
	    "Content-Length: 860\r\n"
	    "\r\n"
	    "v=0\r\n"
	    "o=BroadWorks 1693848685 415741525 IN IP4 212.55.48.2\r\n"
	    "s=-\r\n"
	    "c=IN IP4 212.55.48.2\r\n"
	    "t=0 0\r\n"
	    "m=audio 15536 RTP/AVP 8 0 18 116\r\n"
	    "a=rtpmap:8 PCMA/8000\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "a=rtpmap:116 telephone-event/8000\r\n"
	    "a=ptime:20\r\n"
	    "a=3gOoBTC\r\n"
	    "a=rtpmap:18 G729/8000\r\n"
	    "a=fmtp:18 annexb=yes\r\n"
	    "m=audio 15536 RTP/SAVP 8 0 18 116\r\n"
	    "a=rtpmap:8 PCMA/8000\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "a=rtpmap:116 telephone-event/8000\r\n"
	    "a=ptime:20\r\n"
	    "a=3gOoBTC\r\n"
	    "a=rtpmap:18 G729/8000\r\n"
	    "a=fmtp:18 annexb=yes\r\n"
	    "a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:zc409/eT1JuwUPQAswLkF878WJvn5Rpo+aLUt+SI|2^31\r\n"
	    "a=crypto:2 AES_CM_128_HMAC_SHA1_32 inline:ebHN/WPPcu2E+Jm4kdx9YK58jVFDKD4uRgwFu18k|2^31\r\n"
	    "a=crypto:3 AES_256_CM_HMAC_SHA1_80 "
	    "inline:M9UR+6n8F8DZ5mh/V5vh2VKdYZ+5Hb4K3mwepx8oM9aIQYb7RzdfJE42ezOTcQ==|2^31\r\n"
	    "a=crypto:4 AES_256_CM_HMAC_SHA1_32 "
	    "inline:AIsXIk2O8tsCefUYXpqP96hNZKJR+nJZcXlCOiXZW6TDEtg/g5HQD7lcj0KJPA==|2^31\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	BC_ASSERT_TRUE(liblinphone_tester_send_data(crashing_invite, strlen(crashing_invite), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	wait_for_until(laure->lc, NULL, NULL, 0, 1000);

	linphone_core_manager_destroy(laure);
}

static void enable_specific_payloads(LinphoneCore *lc) {
	bctbx_list_t *payloads = linphone_core_get_audio_payload_types(lc);
	LinphonePayloadType *type;

	for (bctbx_list_t *it = payloads; it != NULL; it = it->next) {
		type = (LinphonePayloadType *)it->data;
		linphone_payload_type_enable(type, FALSE);
	}

	type = linphone_core_get_payload_type(lc, "speex", 8000, -1);
	if (BC_ASSERT_PTR_NOT_NULL(type)) {
		linphone_payload_type_enable(type, TRUE);
		linphone_payload_type_unref(type);
	}

	type = linphone_core_get_payload_type(lc, "pcmu", 8000, -1);
	if (BC_ASSERT_PTR_NOT_NULL(type)) {
		linphone_payload_type_enable(type, TRUE);
		linphone_payload_type_unref(type);
	}

	type = linphone_core_get_payload_type(lc, "pcma", 8000, -1);
	if (BC_ASSERT_PTR_NOT_NULL(type)) {
		linphone_payload_type_enable(type, TRUE);
		linphone_payload_type_unref(type);
	}

	bctbx_list_free_with_data(payloads, (void (*)(void *))linphone_payload_type_unref);
}

static void call_with_same_codecs_ordered_differently(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	int dummy = 0;

	// We have to test this feature with only codecs that have the same clock rate
	// Because we generate telephone-events for each clock rate and their payload type number
	// is not fixed. Which makes the isSamePayload function fail.

	// Set speex the first payload of marie
	LinphonePayloadType *speex = linphone_core_get_payload_type(marie->lc, "speex", 8000, -1);
	if (BC_ASSERT_PTR_NOT_NULL(speex)) {
		bctbx_list_t *new_order = bctbx_list_new(speex);
		linphone_core_set_audio_payload_types(marie->lc, new_order);
		bctbx_list_free(new_order);
		linphone_payload_type_unref(speex);
	}

	// Set Pauline audio payloads with the same list but inverted with speex still in first
	bctbx_list_t *payloads = linphone_core_get_audio_payload_types(pauline->lc);
	bctbx_list_t *inverted = NULL;

	for (bctbx_list_t *it = payloads; it != NULL; it = it->next) {
		LinphonePayloadType *type = (LinphonePayloadType *)bctbx_list_get_data(it);

		if (strcmp(linphone_payload_type_get_mime_type(type), "speex") == 0 &&
		    linphone_payload_type_get_clock_rate(type) == 8000) {
			speex = type;
		} else {
			inverted = bctbx_list_prepend(inverted, type);
		}
	}
	inverted = bctbx_list_prepend(inverted, speex);

	linphone_core_set_audio_payload_types(pauline->lc, inverted);

	bctbx_list_free(inverted);
	bctbx_list_free_with_data(payloads, (void (*)(void *))linphone_payload_type_unref);

	enable_specific_payloads(marie->lc);
	enable_specific_payloads(pauline->lc);

	BC_ASSERT_TRUE(call(marie, pauline));

	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 2000);

	// Disconnect pauline to trigger a reINVITE
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationProgress, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 2));

	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 2000);

	// It should not trigger an audio stream restart
	check_nb_media_starts(AUDIO_START, marie, pauline, 1, 1);

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_rejected_with_403(void) {
	const LinphoneErrorInfo *error;
	LinphoneCoreManager *mgr = linphone_core_manager_new("empty_rc");

	LinphoneAccountParams *accountParams = linphone_core_create_account_params(mgr->lc);
	LinphoneAddress *address = linphone_factory_create_address(linphone_factory_get(), "sip:bob@toto.example.org");
	linphone_account_params_set_identity_address(accountParams, address);

	LinphoneAddress *serverAddress = linphone_factory_create_address(linphone_factory_get(), "sip:toto.example.org");
	linphone_account_params_set_server_address(accountParams, serverAddress);

	linphone_account_params_set_register_enabled(accountParams, TRUE);
	LinphoneAccount *account = linphone_core_create_account(mgr->lc, accountParams);
	linphone_core_add_account(mgr->lc, account);
	BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneAccountAdded, 1));
	linphone_core_set_default_account(mgr->lc, account);
	BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneDefaultAccountChanged, 1));
	linphone_core_iterate(mgr->lc);

	LinphoneCall *call = linphone_core_invite(mgr->lc, "sip:nimportequoi@sip.example.org;transport=TLS");
	linphone_call_ref(call);

	BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallError, 1));
	BC_ASSERT_PTR_NOT_NULL(call);
	error = linphone_call_get_error_info(call);
	BC_ASSERT_PTR_NOT_NULL(error);
	int code = linphone_error_info_get_protocol_code(error);
	BC_ASSERT_EQUAL(code, 403, int, "%d");

	linphone_call_unref(call);
	linphone_address_destroy(serverAddress);
	linphone_account_params_unref(accountParams);
	linphone_account_unref(account);
	linphone_address_destroy(address);
	linphone_core_manager_destroy(mgr);
}
static void simple_call_with_display_name(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	const LinphoneAddress *from;
	LinphoneCall *pauline_call;
	LinphoneProxyConfig *marie_cfg;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new((transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc"));

	/* with the account manager, we might lose the identity */
	marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	{
		LinphoneAddress *marie_addr = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_cfg));
		char *marie_tmp_addr = NULL;
		linphone_address_set_display_name(marie_addr, "Super Marie");
		marie_tmp_addr = linphone_address_as_string(marie_addr);

		linphone_proxy_config_edit(marie_cfg);
		linphone_proxy_config_set_identity_address(marie_cfg, marie_addr);
		linphone_proxy_config_done(marie_cfg);

		ms_free(marie_tmp_addr);
		linphone_address_unref(marie_addr);
	}

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	BC_ASSERT_TRUE(call(marie, pauline));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	/*check that display name is correctly propagated in From */
	if (pauline_call) {
		from = linphone_call_get_remote_address(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_PTR_NOT_NULL(from);
		if (from) {
			const char *dname = linphone_address_get_display_name(from);
			BC_ASSERT_PTR_NOT_NULL(dname);
			if (dname) {
				BC_ASSERT_STRING_EQUAL(dname, "Super Marie");
			}
		}
	}

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);

	const bctbx_list_t *logs = linphone_core_get_call_logs(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(logs);
	LinphoneCallLog *log = (LinphoneCallLog *)bctbx_list_get_data(logs);
	BC_ASSERT_STRING_EQUAL(linphone_address_get_display_name(linphone_call_log_get_from_address(log)), "Super Marie");

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_received_with_tel_uri(void) {
	const char *invite = "INVITE sip:49.14.153.62:34247;transport=udp SIP/2.0\r\n"
	                     "Via: SIP/2.0/UDP 127.0.0.1:80;branch=z9hG4bKd1615h3de26kgeh6k1202gzb4T12045\r\n"
	                     "Call-ID: asbc16ja7z3sau4j2u2jzdx063zj6zj6336n@10.191.176.18\r\n"
	                     "From: <tel:+123456789>;tag=sbc0502naux6xjz\r\n"
	                     "To: <tel:+1234567890>\r\n"
	                     "CSeq: 1 INVITE\r\n"
	                     "Allow: UPDATE,INFO,PRACK,NOTIFY,OPTIONS,INVITE,ACK,BYE,CANCEL\r\n"
	                     "Contact: <sip:127.0.0.1:80;Dpt=eb7a-200>\r\n"
	                     "Max-Forwards: 66\r\n"
	                     "Supported: timer,100rel,histinfo,early-session\r\n"
	                     "Session-Expires: 1800\r\n"
	                     "Min-SE: 600\r\n"
	                     "P-Asserted-Identity: <tel:+919599936258>\r\n"
	                     "P-Called-Party-ID: <tel:+919403993402>\r\n"
	                     "P-Notification: caller-control\r\n"
	                     "Content-Length: 229\r\n"
	                     "Content-Type: application/sdp\r\n"
	                     "Content-Disposition: session\r\n"
	                     "\r\n"
	                     "v=0\r\n"
	                     "o=- 213464062 213464062 IN IP4 127.0.0.1\r\n"
	                     "s=SBC call\r\n"
	                     "c=IN IP4 127.0.0.1\r\n"
	                     "t=0 0\r\n"
	                     "m=audio 1023 RTP/AVP 8 96 0\r\n"
	                     "b=AS:80\r\n"
	                     "a=rtpmap:8 PCMA/8000\r\n"
	                     "a=rtpmap:96 telephone-event/8000\r\n"
	                     "a=ptime:20\r\n"
	                     "a=maxptime:20\r\n"
	                     "a=rtpmap:0 PCMU/8000\r\n"
	                     "\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);

	BC_ASSERT_TRUE(liblinphone_tester_send_data(invite, strlen(invite), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallIncomingReceived, 1));
	linphone_call_accept(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallStreamsRunning, 1));
	linphone_call_terminate(linphone_core_get_current_call(laure->lc));

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1,
	                              liblinphone_tester_sip_timeout));
	linphone_core_manager_destroy(laure);
}

static void call_with_custom_m_line_not_encrypted(void) {
	const char *invite =
	    "INVITE sip:fraya1_5c36c@218.107.193.68:40833;instance=1108cf52-f678-4a3a-88f6-2faf1dfdb9f3;transport=tls "
	    "SIP/2.0\r\n"
	    "Via: SIP/2.0/TLS 20.103.252.41;rport;branch=z9hG4bK.mgB5K2Q3HmS1QyF41ygNN15B4c\r\n"
	    "Record-Route: <sips:20.103.252.41:5061;lr>\r\n"
	    "Record-Route: <sip:10.17.4.52:5059;transport=tcp;lr>\r\n"
	    "Record-Route: <sip:10.17.4.51:5059;transport=tcp;lr>\r\n"
	    "Record-Route: <sips:20.103.252.84:5061;lr>\r\n"
	    "Record-Route: <sip:218.107.193.68:23565;transport=tls;lr>\r\n"
	    "Record-Route: <sip:192.168.75.3:2222;transport=tcp;lr>\r\n"
	    "Record-Route: <sip:192.168.75.3:5070;transport=tcp;lr>\r\n"
	    "Record-Route: <sip:192.168.31.149:5070;transport=tcp;lr>\r\n"
	    "Record-Route: <sip:192.168.31.149:44830;transport=tcp>\r\n"
	    "Record-Route: <sip:192.168.31.149:2225;transport=udp;lr>\r\n"
	    "Record-Route: <sip:192.168.31.104:5060;lr>\r\n"
	    "Via: SIP/2.0/TCP 10.17.4.51:5059;rport=38317;branch=z9hG4bK.tvBvQQUpBUa7Nm5UrHZe212BHa\r\n"
	    "Via: SIP/2.0/TLS 192.168.75.3:2224;rport=23565;branch=z9hG4bK.A6DJpwUJQ;received=218.107.193.68\r\n"
	    "Via: SIP/2.0/TCP 192.168.75.3:5070;rport;branch=z9hG4bK.cDtyF1gQZ1BcDe0rr60N5pB10F\r\n"
	    "Via: SIP/2.0/TCP 192.168.31.149:44830;rport=44830;branch=z9hG4bK.z~Md5C-XV\r\n"
	    "Via: SIP/2.0/UDP 192.168.31.104;rport;branch=z9hG4bK.HyNtp97erS931r6FyrK6B0N8vc\r\n"
	    "Via: SIP/2.0/UDP 192.168.31.123:5060;branch=z9hG4bK124035529;rport=5060\r\n"
	    "Max-Forwards: 66\r\n"
	    "From: <sip:0071818@ipgw48dd6199-d874-45da-8e61-d78107b1cbd0>;tag=745175727\r\n"
	    "To: <sip:750007101001@ipgw48dd6199-d874-45da-8e61-d78107b1cbd0>\r\n"
	    "Call-ID: 1103078726-5060-13@BJC.BGI.DB.BCD\r\n"
	    "CSeq: 120 INVITE\r\n"
	    "Contact: \"MyContact\" <sip:test@127.0.0.1:5060>\r\n"
	    "User-Agent: My User Agent\r\n"
	    "Accept: application/sdp, application/dtmf-relay, application/dtmf-relay, application/dtmf-relay, "
	    "application/dtmf-relay, application/dtmf-relay\r\n"
	    "Allow: INVITE, ACK, OPTIONS, CANCEL, BYE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE\r\n"
	    "Supported: replaces, path, timer, eventlist\r\n"
	    "Privacy: none\r\n"
	    "Content-Type: application/sdp\r\n"
	    "P-Access-Network-Info: IEEE-EUI-48;eui-48-addr=D4-EE-07-52-95-FC\r\n"
	    "P-Emergency-Info: IEEE-EUI-48;eui-48-addr=C0-74-AD-B0-01-04\r\n"
	    "P-Preferred-Identity: \"0071818\" <sip:0071818@192.168.31.104:5060>\r\n"
	    "Content-Length: 1665\r\n"
	    "\r\n"
	    "v=0\r\n"
	    "o=0071818 8000 8000 IN IP4 192.168.31.123\r\n"
	    "s=SIP Call\r\n"
	    "c=IN IP4 20.103.252.84\r\n"
	    "b=AS:1\r\n"
	    "t=0 0\r\n"
	    "a=10.0.65.0:yes\r\n"
	    "a=192.168.31.149:yes\r\n"
	    "a=nortpproxy:yes\r\n"
	    "m=audio 50024 RTP/SAVP 0 8 9 101\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "a=rtpmap:8 PCMA/8000\r\n"
	    "a=rtpmap:9 G722/8000\r\n"
	    "a=rtpmap:101 telephone-event/8000\r\n"
	    "a=fmtp:101 0-15\r\n"
	    "a=rtcp:50025\r\n"
	    "a=ptime:20\r\n"
	    "a=crypto:1 AES_CM_256_HMAC_SHA1_80 inline:QPPeFseKBO3ISJ5AUx5dFqS7XqMzjjOy8J/c2ju/B3uy5pJ5cJZmON4EeDEj1Q==\r\n"
	    "a=crypto:2 AES_CM_256_HMAC_SHA1_32 inline:R8eQpWvDNJ51JD5R/nkQBvXC7Ic8XB2ilPunDCzK4XORcRn8NE2bqXHZ+3BSCw==\r\n"
	    "a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:dkfOYs4KvuusUuZTXhIdP4avsJ+r5OxGjV0fiM1y\r\n"
	    "a=crypto:4 AES_CM_128_HMAC_SHA1_32 inline:lEO5YqWIbGNzGLVabBNsiVLyOAKR5OZ9KnTbSvyo\r\n"
	    "m=video 49228 RTP/SAVP 99 96 98 120\r\n"
	    "b=AS:2240\r\n"
	    "a=rtpmap:99 H264/90000\r\n"
	    "a=fmtp:99 profile-level-id=428028; packetization-mode=1\r\n"
	    "a=rtpmap:96 H264/90000\r\n"
	    "a=fmtp:96 profile-level-id=4D0028; packetization-mode=1\r\n"
	    "a=rtpmap:98 H264/90000\r\n"
	    "a=fmtp:98 profile-level-id=640028; packetization-mode=1\r\n"
	    "a=rtpmap:120 GS-FEC/90000\r\n"
	    "a=rtcp:49229\r\n"
	    "a=rtcp-fb:* nack\r\n"
	    "a=rtcp-fb:* nack pli\r\n"
	    "a=rtcp-fb:* ccm fir\r\n"
	    "a=gs-fec-version:2\r\n"
	    "a=gs-fec-version:1\r\n"
	    "a=crypto:1 AES_CM_256_HMAC_SHA1_80 inline:vJDsdfKR/V71cXeqy+O+N2wQKqUTu4n5ObNtFP1qvLn6qC/tOixLL53C2Willw==\r\n"
	    "a=crypto:2 AES_CM_256_HMAC_SHA1_32 inline:oBKoyre7hUC0vvMi0vGMj6qGN9lzcQa/oKOBegwnEaw5uXbwdPswKbojS4wU1w==\r\n"
	    "a=crypto:3 AES_CM_128_HMAC_SHA1_80 inline:G79dU5jRxJ6QZUIR30448PpxqnBhHmuRRyW1krLJ\r\n"
	    "a=crypto:4 AES_CM_128_HMAC_SHA1_32 inline:ac2IxyAhmOW/KEoBOSlPchlJ48O5ReIl1ilKi7z8\r\n"
	    "a=content:main\r\n"
	    "a=label:11\r\n"
	    "m=application 65488 UDP/BFCP 18868056\r\n"
	    "a=confid:1\r\n"
	    "a=userid:1\r\n"
	    "a=floorid:1 mstrm:12\r\n"
	    "a=floorctrl:c-s\r\n"
	    "\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_core_set_video_activation_policy(laure->lc, vpol);
	linphone_core_enable_video_capture(laure->lc, TRUE);
	linphone_core_enable_video_display(laure->lc, TRUE);
	linphone_video_activation_policy_unref(vpol);

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	BC_ASSERT_TRUE(liblinphone_tester_send_data(invite, strlen(invite), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallIncomingReceived, 1));
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	linphone_call_accept(laure_call);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(check_custom_m_line(laure_call, "application"));
	linphone_call_terminate(laure_call);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1,
	                              liblinphone_tester_sip_timeout));
	linphone_core_manager_destroy(laure);
}

static void call_with_custom_m_line_and_crappy_to_header(void) {
	const char *invite_template =
	    "INVITE "
	    "sip:%s@49.36.181.143:33703;transport=tcp;pn-key=7ca80b71bccdbda72957091955dec66f;aor=rsystems1%%40sip1."
	    "mircomsip.com SIP/2.0\r\n"
	    "v: SIP/2.0/UDP "
	    "10.10.11.148:5060;received=38.32.59.74;branch=z9hG4bK62f554a1-bff3-5e1f-9c30-0007327ac1d5;rport=47758\r\n"
	    "CSeq: 1 INVITE\r\n"
	    "c: application/sdp\r\n"
	    "Content-Length: 378\r\n"
	    "i: ba4214a1-bff3-5e1f-9c2e-0007327ac1d5@1AD143096\r\n"
	    "t: <sip:%s@49.36.181.143>\r\n"
	    "k: replaces\r\n"
	    "m: \"TESTTOUCH1\" "
	    "<sip:tx3touchf15b_testtouch1_1*38.32.59.74!47758_n@sip.example.org;nat=yes;nat=yes;nat=yes;received=38.64.170."
	    "3;"
	    "gr>\r\n"
	    "Organization: Vox Lucida\r\n"
	    "Max-Forwards: 66\r\n"
	    "f: \"TESTTOUCH1\" "
	    "<sip:yyyyyyzzzzz@sip.example.org>;tag=ba4214a1-bff3-5e1f-9c2d-0007327ac1d5\r\n"
	    "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING\r\n"
	    "User-Agent: TX3-VOIP/3.18.2\r\n"
	    "\r\n"
	    "v=0\r\n"
	    "o=- 1694529887 1 IN IP4 199.7.173.72\r\n"
	    "s=TX3-VOIP/3.18.2\r\n"
	    "c=IN IP4 199.7.173.72\r\n"
	    "t=0 0\r\n"
	    "m=audio 1023 RTP/AVP 8 96 0\r\n"
	    "b=AS:80\r\n"
	    "a=rtpmap:8 PCMA/8000\r\n"
	    "a=rtpmap:96 telephone-event/8000\r\n"
	    "a=ptime:20\r\n"
	    "a=maxptime:20\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "m=application 52748 RTP/AVP 97\r\n"
	    "a=rtcp-rsize\r\n"
	    "a=ssrc:1276113665 cname:bJR5ob/zXh+cMwAHMnrB1Q\r\n"
	    "a=rtpmap:97 H224/4800\r\n"
	    "a=sendrecv\r\n"
	    "a=rtcp:52749\r\n"
	    "\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");
	linphone_config_set_bool(linphone_core_get_config(laure->lc), "sip", "incoming_calls_early_media", 1);

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	const char *laure_username = linphone_address_get_username(laure->identity);
	char *invite = bctbx_strdup_printf(invite_template, laure_username, laure_username);
	bctbx_list_t *call_logs;

	BC_ASSERT_TRUE(liblinphone_tester_send_data(invite, strlen(invite), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallIncomingReceived, 1));
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));
	linphone_call_accept(laure_call);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(check_custom_m_line(laure_call, "application"));
	linphone_call_terminate(laure_call);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1,
	                              liblinphone_tester_sip_timeout));

	/* Make sure that the call-log was reported as belonging to laure's SIP account, despite the To address that does
	 * not mention sip.example.org. */
	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(laure->lc));
	if (BC_ASSERT_PTR_NOT_NULL(call_logs)) {
		BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 1);
		LinphoneCallLog *clog = (LinphoneCallLog *)call_logs->data;
		BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(linphone_call_log_get_to_address(clog)), "sip.example.org");
	}
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);
	bctbx_free(invite);
	linphone_core_manager_destroy(laure);
}

static void call_with_from_and_to_without_domain(void) {
	const char *invite_template =
	    "INVITE sip:%s@10.0.0.210:35932;transport=tls SIP/2.0\r\n"
	    "Via: SIP/2.0/TLS 10.0.0.60:5061;branch=z9hG4bK79920c86;rport\r\n"
	    "Max-Forwards: 70\r\n"
	    "From: <sip:lise@10.0.0.60>;tag=as5d5122vb\r\n"
	    "To: <sip:%s@10.0.0.210:35932;transport=tls>\r\n"
	    "Contact: <sip:lise@10.0.0.60:5061;transport=TLS>\r\n"
	    "CSeq: 102 INVITE\r\n"
	    "Call-ID: 12bc634d4dee9a6a1829a7e07ff4b02d@10.0.0.60:5061\r\n"
	    "Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE\r\n"
	    "Record-route: <sip:sip1.example.org;lr>, <sip:sip2.example.org;lr>\r\n"
	    "Supported: replaces, timer\r\n"
	    "Content-Type: application/sdp\r\n"
	    "\r\n"
	    "v=0\r\n"
	    "o=root 1242761950 1242761950 IN IP4 10.0.0.60\r\n"
	    "s=voip.ms\r\n"
	    "c=IN IP4 10.0.0.60\r\n"
	    "t=0 0\r\n"
	    "m=audio 18368 RTP/AVP 0 18 9 101\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "a=rtpmap:18 G729/8000\r\n"
	    "a=fmtp:18 annexb=no\r\n"
	    "a=rtpmap:9 G722/8000\r\n"
	    "a=rtpmap:101 telephone-event/8000\r\n"
	    "a=fmtp:101 0-16\r\n"
	    "a=ptime:20\r\n"
	    "a=sendrecv\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	const char *laure_username = linphone_address_get_username(laure->identity);
	char *invite = bctbx_strdup_printf(invite_template, laure_username, laure_username);
	bctbx_list_t *call_logs;

	BC_ASSERT_TRUE(liblinphone_tester_send_data(invite, strlen(invite), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallIncomingReceived, 1));
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	linphone_call_terminate(laure_call);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1,
	                              liblinphone_tester_sip_timeout));

	/* Make sure that the call-log was reported as belonging to laure's SIP account, despite the To address that does
	 * not mention sip.example.org. */
	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(laure->lc));
	if (BC_ASSERT_PTR_NOT_NULL(call_logs)) {
		BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 1);
		LinphoneCallLog *clog = (LinphoneCallLog *)call_logs->data;
		BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(linphone_call_log_get_to_address(clog)), "sip.example.org");
	}
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);
	bctbx_free(invite);
	linphone_core_manager_destroy(laure);
}

static void call_with_correct_local_account_in_request_uri(void) {
	const char *invite_template =
	    "INVITE sip:%s@10.0.0.210:35932;transport=tls SIP/2.0\r\n"
	    "Via: SIP/2.0/TLS 10.0.0.60:5061;branch=z9hG4bK79920c86;rport\r\n"
	    "Max-Forwards: 70\r\n"
	    "From: <sip:lise@sip.example.org>;tag=as5d5122vb\r\n"
	    "To: <sip:ghost@sip.doesntexists.com:35932;transport=tls>\r\n"
	    "Contact: <sip:lise@10.0.0.60:5061;transport=TLS>\r\n"
	    "CSeq: 102 INVITE\r\n"
	    "Call-ID: 12bc634d4dee9a6a1829a7e07ff4b02d@10.0.0.60:5061\r\n"
	    "Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE\r\n"
	    "Supported: replaces, timer\r\n"
	    "Content-Type: application/sdp\r\n"
	    "\r\n"
	    "v=0\r\n"
	    "o=root 1242761950 1242761950 IN IP4 10.0.0.60\r\n"
	    "s=voip.ms\r\n"
	    "c=IN IP4 10.0.0.60\r\n"
	    "t=0 0\r\n"
	    "m=audio 18368 RTP/AVP 0 18 9 101\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "a=rtpmap:18 G729/8000\r\n"
	    "a=fmtp:18 annexb=no\r\n"
	    "a=rtpmap:9 G722/8000\r\n"
	    "a=rtpmap:101 telephone-event/8000\r\n"
	    "a=fmtp:101 0-16\r\n"
	    "a=ptime:20\r\n"
	    "a=sendrecv\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	const char *laure_username = linphone_address_get_username(laure->identity);
	char *invite = bctbx_strdup_printf(invite_template, laure_username);
	bctbx_list_t *call_logs;

	BC_ASSERT_TRUE(liblinphone_tester_send_data(invite, strlen(invite), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallIncomingReceived, 1));
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	linphone_call_terminate(laure_call);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneCallReleased, 1,
	                              liblinphone_tester_sip_timeout));

	/* Make sure that the call-log was reported as belonging to laure's SIP account, despite the To address that does
	 * not mention sip.example.org. */
	call_logs = linphone_account_get_call_logs(linphone_core_get_default_account(laure->lc));
	if (BC_ASSERT_PTR_NOT_NULL(call_logs)) {
		BC_ASSERT_TRUE(bctbx_list_size(call_logs) == 1);
		LinphoneCallLog *clog = (LinphoneCallLog *)call_logs->data;
		BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(linphone_call_log_get_to_address(clog)), "sip.example.org");
	}
	bctbx_list_free_with_data(call_logs, (bctbx_list_free_func)linphone_call_log_unref);
	bctbx_free(invite);
	linphone_core_manager_destroy(laure);
}

void call_with_core_without_media(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc_without_media");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	// Reduce the nortp timeout to end the test faster
	linphone_core_set_nortp_timeout(pauline->lc, 3);

	disable_all_audio_codecs_except_one(pauline->lc, "opus", 48000);
	// disable_all_video_codecs_except_one(pauline->lc, "vp8");

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_video_activation_policy_unref(pol);

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	LinphoneCallParams *params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneContent *content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "sdp");
	linphone_content_set_utf8_text(
	    content,
	    "v=0\r\n"
	    "o=- 6646531434124232327 2 IN IP4 127.0.0.1\r\n"
	    "s=-\r\n"
	    "t=0 0\r\n"
	    "a=group:BUNDLE 0 1\r\n"
	    "a=extmap-allow-mixed\r\n"
	    "a=msid-semantic: WMS dee12576-f30b-4893-83d3-eac27ee10d8d\r\n"
	    "m=audio 9 UDP/TLS/RTP/SAVPF 111 63 9 0 8 13 110 126\r\n"
	    "c=IN IP4 0.0.0.0\r\n"
	    "a=rtcp:9 IN IP4 0.0.0.0\r\n"
	    "a=ice-ufrag:eQJT\r\n"
	    "a=ice-pwd:WIC+snfuYSyendsry0+rQEO1\r\n"
	    "a=ice-options:trickle\r\n"
	    "a=fingerprint:sha-256 "
	    "EE:D5:B7:7C:2B:07:E6:C3:D8:76:A8:6F:E3:CF:E0:E6:AB:42:29:DE:AD:A1:8E:55:11:E7:81:4B:4C:0D:F1:46\r\n"
	    "a=setup:actpass\r\n"
	    "a=mid:0\r\n"
	    "a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\n"
	    "a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\n"
	    "a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\n"
	    "a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\n"
	    "a=sendrecv\r\n"
	    "a=msid:dee12576-f30b-4893-83d3-eac27ee10d8d d4ad49b2-f4ab-48b9-8aa8-621361146f2c\r\n"
	    "a=rtcp-mux\r\n"
	    "a=rtcp-rsize\r\n"
	    "a=rtpmap:111 opus/48000/2\r\n"
	    "a=rtcp-fb:111 transport-cc\r\n"
	    "a=rtcp-fb:111 goog-remb\r\n"
	    "a=fmtp:111 minptime=10;useinbandfec=1\r\n"
	    "a=rtpmap:63 red/48000/2\r\n"
	    "a=fmtp:63 111/111\r\n"
	    "a=rtpmap:9 G722/8000\r\n"
	    "a=rtpmap:0 PCMU/8000\r\n"
	    "a=rtpmap:8 PCMA/8000\r\n"
	    "a=rtpmap:13 CN/8000\r\n"
	    "a=rtpmap:110 telephone-event/48000\r\n"
	    "a=rtpmap:126 telephone-event/8000\r\n"
	    "a=ssrc:2557628080 cname:mPaUVqJCMZayFJgz\r\n"
	    "a=ssrc:2557628080 msid:dee12576-f30b-4893-83d3-eac27ee10d8d d4ad49b2-f4ab-48b9-8aa8-621361146f2c\r\n"
	    "m=video 9 UDP/TLS/RTP/SAVPF 96 97 102 103 104 105 106 107 108 109 127 125 39 40 45 46 98 99 100 101 "
	    "112 113 116 117 118\r\n"
	    "c=IN IP4 0.0.0.0\r\n"
	    "a=rtcp:9 IN IP4 0.0.0.0\r\n"
	    "a=ice-ufrag:eQJT\r\n"
	    "a=ice-pwd:WIC+snfuYSyendsry0+rQEO1\r\n"
	    "a=ice-options:trickle\r\n"
	    "a=fingerprint:sha-256 "
	    "EE:D5:B7:7C:2B:07:E6:C3:D8:76:A8:6F:E3:CF:E0:E6:AB:42:29:DE:AD:A1:8E:55:11:E7:81:4B:4C:0D:F1:46\r\n"
	    "a=setup:actpass\r\n"
	    "a=mid:1\r\n"
	    "a=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\n"
	    "a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\n"
	    "a=extmap:13 urn:3gpp:video-orientation\r\n"
	    "a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\n"
	    "a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\n"
	    "a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\n"
	    "a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\n"
	    "a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\n"
	    "a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\n"
	    "a=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\n"
	    "a=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\n"
	    "a=sendrecv\r\n"
	    "a=msid:dee12576-f30b-4893-83d3-eac27ee10d8d e879805a-7d32-4872-9781-3b0a5ae539ff\r\n"
	    "a=rtcp-mux\r\n"
	    "a=rtcp-rsize\r\n"
	    "a=rtpmap:96 VP8/90000\r\n"
	    "a=rtcp-fb:96 goog-remb\r\n"
	    "a=rtcp-fb:96 transport-cc\r\n"
	    "a=rtcp-fb:96 ccm fir\r\n"
	    "a=rtcp-fb:96 nack\r\n"
	    "a=rtcp-fb:96 nack pli\r\n"
	    "a=rtpmap:97 rtx/90000\r\n"
	    "a=fmtp:97 apt=96\r\n"
	    "a=rtpmap:102 H264/90000\r\n"
	    "a=rtcp-fb:102 goog-remb\r\n"
	    "a=rtcp-fb:102 transport-cc\r\n"
	    "a=rtcp-fb:102 ccm fir\r\n"
	    "a=rtcp-fb:102 nack\r\n"
	    "a=rtcp-fb:102 nack pli\r\n"
	    "a=fmtp:102 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\r\n"
	    "a=rtpmap:103 rtx/90000\r\n"
	    "a=fmtp:103 apt=102\r\n"
	    "a=rtpmap:104 H264/90000\r\n"
	    "a=rtcp-fb:104 goog-remb\r\n"
	    "a=rtcp-fb:104 transport-cc\r\n"
	    "a=rtcp-fb:104 ccm fir\r\n"
	    "a=rtcp-fb:104 nack\r\n"
	    "a=rtcp-fb:104 nack pli\r\n"
	    "a=fmtp:104 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42001f\r\n"
	    "a=rtpmap:105 rtx/90000\r\n"
	    "a=fmtp:105 apt=104\r\n"
	    "a=rtpmap:106 H264/90000\r\n"
	    "a=rtcp-fb:106 goog-remb\r\n"
	    "a=rtcp-fb:106 transport-cc\r\n"
	    "a=rtcp-fb:106 ccm fir\r\n"
	    "a=rtcp-fb:106 nack\r\n"
	    "a=rtcp-fb:106 nack pli\r\n"
	    "a=fmtp:106 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f\r\n"
	    "a=rtpmap:107 rtx/90000\r\n"
	    "a=fmtp:107 apt=106\r\n"
	    "a=rtpmap:108 H264/90000\r\n"
	    "a=rtcp-fb:108 goog-remb\r\n"
	    "a=rtcp-fb:108 transport-cc\r\n"
	    "a=rtcp-fb:108 ccm fir\r\n"
	    "a=rtcp-fb:108 nack\r\n"
	    "a=rtcp-fb:108 nack pli\r\n"
	    "a=fmtp:108 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42e01f\r\n"
	    "a=rtpmap:109 rtx/90000\r\n"
	    "a=fmtp:109 apt=108\r\n"
	    "a=rtpmap:127 H264/90000\r\n"
	    "a=rtcp-fb:127 goog-remb\r\n"
	    "a=rtcp-fb:127 transport-cc\r\n"
	    "a=rtcp-fb:127 ccm fir\r\n"
	    "a=rtcp-fb:127 nack\r\n"
	    "a=rtcp-fb:127 nack pli\r\n"
	    "a=fmtp:127 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d001f\r\n"
	    "a=rtpmap:125 rtx/90000\r\n"
	    "a=fmtp:125 apt=127\r\n"
	    "a=rtpmap:39 H264/90000\r\n"
	    "a=rtcp-fb:39 goog-remb\r\n"
	    "a=rtcp-fb:39 transport-cc\r\n"
	    "a=rtcp-fb:39 ccm fir\r\n"
	    "a=rtcp-fb:39 nack\r\n"
	    "a=rtcp-fb:39 nack pli\r\n"
	    "a=fmtp:39 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=4d001f\r\n"
	    "a=rtpmap:40 rtx/90000\r\n"
	    "a=fmtp:40 apt=39\r\n"
	    "a=rtpmap:45 AV1/90000\r\n"
	    "a=rtcp-fb:45 goog-remb\r\n"
	    "a=rtcp-fb:45 transport-cc\r\n"
	    "a=rtcp-fb:45 ccm fir\r\n"
	    "a=rtcp-fb:45 nack\r\n"
	    "a=rtcp-fb:45 nack pli\r\n"
	    "a=fmtp:45 level-idx=5;profile=0;tier=0\r\n"
	    "a=rtpmap:46 rtx/90000\r\n"
	    "a=fmtp:46 apt=45\r\n"
	    "a=rtpmap:98 VP9/90000\r\n"
	    "a=rtcp-fb:98 goog-remb\r\n"
	    "a=rtcp-fb:98 transport-cc\r\n"
	    "a=rtcp-fb:98 ccm fir\r\n"
	    "a=rtcp-fb:98 nack\r\n"
	    "a=rtcp-fb:98 nack pli\r\n"
	    "a=fmtp:98 profile-id=0\r\n"
	    "a=rtpmap:99 rtx/90000\r\n"
	    "a=fmtp:99 apt=98\r\n"
	    "a=rtpmap:100 VP9/90000\r\n"
	    "a=rtcp-fb:100 goog-remb\r\n"
	    "a=rtcp-fb:100 transport-cc\r\n"
	    "a=rtcp-fb:100 ccm fir\r\n"
	    "a=rtcp-fb:100 nack\r\n"
	    "a=rtcp-fb:100 nack pli\r\n"
	    "a=fmtp:100 profile-id=2\r\n"
	    "a=rtpmap:101 rtx/90000\r\n"
	    "a=fmtp:101 apt=100\r\n"
	    "a=rtpmap:112 H264/90000\r\n"
	    "a=rtcp-fb:112 goog-remb\r\n"
	    "a=rtcp-fb:112 transport-cc\r\n"
	    "a=rtcp-fb:112 ccm fir\r\n"
	    "a=rtcp-fb:112 nack\r\n"
	    "a=rtcp-fb:112 nack pli\r\n"
	    "a=fmtp:112 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=64001f\r\n"
	    "a=rtpmap:113 rtx/90000\r\n"
	    "a=fmtp:113 apt=112\r\n"
	    "a=rtpmap:116 red/90000\r\n"
	    "a=rtpmap:117 rtx/90000\r\n"
	    "a=fmtp:117 apt=116\r\n"
	    "a=rtpmap:118 ulpfec/90000\r\n"
	    "a=ssrc-group:FID 489109307 1592583860\r\n"
	    "a=ssrc:489109307 cname:mPaUVqJCMZayFJgz\r\n"
	    "a=ssrc:489109307 msid:dee12576-f30b-4893-83d3-eac27ee10d8d e879805a-7d32-4872-9781-3b0a5ae539ff\r\n"
	    "a=ssrc:1592583860 cname:mPaUVqJCMZayFJgz\r\n"
	    "a=ssrc:1592583860 msid:dee12576-f30b-4893-83d3-eac27ee10d8d e879805a-7d32-4872-9781-3b0a5ae539ff\r\n");

	linphone_call_params_add_custom_content(params, content);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params_2(marie->lc, pauline->identity, params, "", NULL));
	linphone_content_unref(content);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1,
	                              liblinphone_tester_sip_timeout));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);

	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1,
	                              liblinphone_tester_sip_timeout));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));

	// Nortp timeout result in a LinphoneReasonIOError
	BC_ASSERT_EQUAL(linphone_call_get_reason(pauline_call), LinphoneReasonIOError, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static test_t call_tests[] = {
    TEST_NO_TAG("Simple double call", simple_double_call),
    TEST_NO_TAG("Simple call with no SIP transport", simple_call_with_no_sip_transport),
    TEST_NO_TAG("Simple call with UDP", simple_call_with_udp),
    TEST_NO_TAG("Simple call without soundcard", simple_call_without_soundcard),
    TEST_NO_TAG("Simple call with multipart INVITE body", simple_call_with_multipart_invite_body),
    TEST_NO_TAG("Simple call with video declined", simple_call_with_video_declined),
    TEST_NO_TAG("Call terminated automatically by linphone_core_destroy", automatic_call_termination),
    TEST_NO_TAG("Call with http proxy", call_with_http_proxy),
    TEST_NO_TAG("Call with http proxy, forced IPv4", call_with_http_proxy_v4),
    TEST_NO_TAG("Call with timed-out bye", call_with_timed_out_bye),
    TEST_NO_TAG("Direct call over IPv6", direct_call_over_ipv6),
    TEST_NO_TAG("Direct call well known port", direct_call_well_known_port_ipv4),
    TEST_NO_TAG("Direct call well known port ipv6", direct_call_well_known_port_ipv6),
    TEST_NO_TAG("Direct call with IPv4 destination and IPv6 media IP", direct_call_with_ipv4_dest_and_ipv6_media_ip),
    TEST_NO_TAG("Call IPv6 to IPv4 without relay", v6_to_v4_call_without_relay),
    TEST_NO_TAG("IPv6 call over NAT64", v6_call_over_nat_64),
    TEST_NO_TAG("Outbound call with multiple proxy possible", call_outbound_with_multiple_proxy),
    TEST_NO_TAG("Outbound call using different proxies", call_outbound_using_different_proxies),
    TEST_ONE_TAG("Outbound call using secondary account", call_outbound_using_secondary_account, "LimeX3DH"),
    TEST_NO_TAG("Audio call recording", audio_call_recording_test),
    TEST_NO_TAG("Multiple answers to a call", multiple_answers_call),
    TEST_NO_TAG("Multiple answers to a call with media relay", multiple_answers_call_with_media_relay),
    TEST_NO_TAG("Call with media relay", call_with_media_relay),
    TEST_NO_TAG("Call with media relay (random ports)", call_with_media_relay_random_ports),
    TEST_NO_TAG("Simple call compatibility mode", simple_call_compatibility_mode),
    TEST_NO_TAG("Early-media call", early_media_call),
    TEST_NO_TAG("Early-media call with ringing", early_media_call_with_ringing),
    TEST_NO_TAG("Early-media call with ringing and network changing",
                early_media_call_with_ringing_and_network_changing),
    TEST_NO_TAG("Early-media call with updated media session", early_media_call_with_session_update),
    TEST_NO_TAG("Call with early update failed", call_with_early_update_failed),
    TEST_NO_TAG("Early-media call with updated codec", early_media_call_with_codec_update),
    TEST_NO_TAG("Call terminated by caller", call_terminated_by_caller),
    TEST_NO_TAG("Call terminated by no rtp timeout", call_terminated_by_nortp_timeout),
    TEST_NO_TAG("Call terminated by no rtp timeout on hold", call_terminated_by_nortp_timeout_on_hold),
    TEST_NO_TAG("Call without SDP", call_with_no_sdp),
    TEST_ONE_TAG("Call without SDP to a lime X3DH enabled device", call_with_no_sdp_lime, "LimeX3DH"),
    TEST_NO_TAG("Call without SDP and ACK without SDP", call_with_no_sdp_ack_without_sdp),
    TEST_NO_TAG("Call paused with RTP port to 0", call_paused_with_rtp_port_to_zero),
    TEST_NO_TAG("Call paused with update", call_paused_with_update),
    TEST_NO_TAG("Call paused resumed", call_paused_resumed),
    TEST_NO_TAG("Call paused quickly resumed", call_paused_quickly_resumed),
    TEST_NO_TAG("Call paused resumed with sip packets looses", call_paused_resumed_with_sip_packets_losses),
    TEST_NO_TAG("Call paused resumed without register", call_paused_resumed_no_register),
    TEST_NO_TAG("Call paused by both parties", call_paused_by_both),
    TEST_NO_TAG("Call paused resumed with loss", call_paused_resumed_with_loss),
    TEST_NO_TAG("Call paused resumed from callee", call_paused_resumed_from_callee),
    TEST_NO_TAG("Call paused resumed from callee with timeout", call_paused_resumed_from_callee_with_timeout),
    TEST_NO_TAG("Call with file player", call_with_file_player),
    TEST_NO_TAG("Call with mkv file player", call_with_mkv_file_player),
    TEST_NO_TAG("Call with privacy", call_with_privacy),
    TEST_NO_TAG("Call with privacy 2", call_with_privacy2)};

static test_t call2_tests[] = {
    TEST_NO_TAG("Call with custom headers", call_with_custom_headers),
    TEST_NO_TAG("Call with custom reserved headers", call_with_custom_reserved_headers),
    TEST_NO_TAG("Call with custom SDP attributes", call_with_custom_sdp_attributes),
    TEST_NO_TAG("Call caller with custom header or sdp", call_caller_with_custom_header_or_sdp_attributes),
    TEST_NO_TAG("Call callee with custom header or sdp", call_callee_with_custom_header_or_sdp_attributes),
    TEST_NO_TAG("Call redirected by callee", call_redirect),
    TEST_NO_TAG("Call with specified codec bitrate", call_with_specified_codec_bitrate),
    TEST_NO_TAG("Call with maxptime", call_with_maxptime),
    TEST_NO_TAG("Call with no audio codec", call_with_no_audio_codec),
    TEST_NO_TAG("Call with no active stream on reINVITE", call_with_no_active_stream_on_reinvite),
    TEST_NO_TAG("Call with in-dialog UPDATE request", call_with_in_dialog_update),
    TEST_NO_TAG("Call with in-dialog very early call request", call_with_very_early_call_update),
    TEST_NO_TAG("Call with in-dialog codec change", call_with_in_dialog_codec_change),
    TEST_NO_TAG("Call with in-dialog codec change no sdp", call_with_in_dialog_codec_change_no_sdp),
    TEST_NO_TAG("Call with pause no SDP on resume", call_with_paused_no_sdp_on_resume),
    TEST_NO_TAG("Call with early media and no SDP in 200 Ok", call_with_early_media_and_no_sdp_in_200),
    TEST_NO_TAG("Call with custom supported tags", call_with_custom_supported_tags),
    TEST_NO_TAG("Call log from taken from asserted id", call_log_from_taken_from_p_asserted_id),
    TEST_NO_TAG("Call log from taken from asserted id in response", call_log_from_taken_from_p_asserted_id_in_response),
    TEST_NO_TAG("Call with generic CN", call_with_generic_cn),
    TEST_NO_TAG("Call with transport change after released", call_with_transport_change_after_released),
    TEST_NO_TAG("Call with FQDN in SDP", call_with_fqdn_in_sdp),
    TEST_NO_TAG("Call with RTP IO mode", call_with_rtp_io_mode),
    TEST_NO_TAG("Call with generic NACK RTCP feedback", call_with_generic_nack_rtcp_feedback),
    TEST_NO_TAG("Call with complex late offering", call_with_complex_late_offering),
    TEST_NO_TAG("Call log working if no db set", call_logs_if_no_db_set),
    TEST_NO_TAG("Call log storage migration from rc to db", call_logs_migrate),
    TEST_NO_TAG("Call log storage in sqlite database", call_logs_sqlite_storage),
    TEST_NO_TAG("Call with custom RTP Modifier", call_with_custom_rtp_modifier),
    TEST_NO_TAG("Call paused resumed with custom RTP Modifier", call_paused_resumed_with_custom_rtp_modifier),
    TEST_NO_TAG("Call record with custom RTP Modifier", call_record_with_custom_rtp_modifier),
    TEST_NO_TAG("Call with rtcp-mux", call_with_rtcp_mux),
    TEST_NO_TAG("Call with network reachable down in callback", call_with_network_reachable_down_in_callback),
    TEST_NO_TAG("Call terminated with reason", terminate_call_with_error),
    TEST_NO_TAG("Call accepted, other ringing device receive CANCEL with reason", cancel_other_device_after_accept),
    TEST_NO_TAG("Simple call with GRUU", simple_call_with_gruu),
    TEST_NO_TAG("Simple call with GRUU only one device ring", simple_call_with_gruu_only_one_device_ring),
    TEST_NO_TAG("Async core stop", async_core_stop_after_call),
    TEST_NO_TAG("Call AVPF mismatch", call_avpf_mismatch),
    TEST_NO_TAG("Call recording without record-aware", call_recording_without_record_aware),
    TEST_NO_TAG("Call recording with record-aware", call_recording_with_record_aware),
    TEST_NO_TAG("Call recording with record-aware both recording", call_recording_with_record_aware_both_recording),
    TEST_NO_TAG("Call without automatic 180 ringing", call_without_automatic_180_ringing),
    TEST_NO_TAG("Call without automatic 180 ringing but early media",
                call_without_automatic_180_ringing_but_early_media),
    TEST_NO_TAG("Call with early media accepted in state changed callback",
                call_with_early_media_accepted_state_changed_callback),
    TEST_NO_TAG("Call with same codecs ordered differently", call_with_same_codecs_ordered_differently),
    TEST_NO_TAG("Call with audio stream added later on", call_with_audio_stream_added_later_on),
    TEST_NO_TAG("Simple call with display name", simple_call_with_display_name),
    TEST_NO_TAG("Call with custom m line and crappy to header", call_with_custom_m_line_and_crappy_to_header),
    TEST_NO_TAG("Call with custom m line not encrypted", call_with_custom_m_line_not_encrypted),
    TEST_NO_TAG("Call with crappy from and to headers", call_with_from_and_to_without_domain),
    TEST_NO_TAG("Call with local account identity in request URI and not in to header",
                call_with_correct_local_account_in_request_uri),
    TEST_NO_TAG("Call with tel uri", call_received_with_tel_uri),
    TEST_NO_TAG("Two accounts not sharing same connection", two_accounts_use_different_connections)};

static test_t call_not_established_tests[] = {
    TEST_NO_TAG("Early declined call", early_declined_call),
    TEST_NO_TAG("Call declined", call_declined),
    TEST_NO_TAG("Call declined on timeout", call_declined_on_timeout),
    TEST_NO_TAG("Call declined in Early Media", call_declined_in_early_media),
    TEST_NO_TAG("Call declined on timeout in Early Media", call_declined_on_timeout_in_early_media),
    TEST_NO_TAG("Call cancelled on request timeout in Early Media", call_cancelled_on_request_timeout_in_early_media),
    TEST_NO_TAG("Call declined with error", call_declined_with_error),
    TEST_NO_TAG("Call declined with reasons", call_declined_with_reasons),
    TEST_NO_TAG("Call declined with retry after", call_declined_with_retry_after),
    TEST_NO_TAG("Cancelled call", cancelled_call),
    TEST_NO_TAG("Call cancelled without response", call_called_without_any_response),
    TEST_NO_TAG("Call cancelled without response and network switch",
                call_called_without_any_response_with_network_switch),
    TEST_NO_TAG("Early cancelled call", early_cancelled_call),
    TEST_NO_TAG("Udp call early cancelled with sal error", udp_call_early_cancelled_with_sal_error),
    TEST_NO_TAG("Udp call terminated with sal error", udp_call_terminated_with_sal_error),
    TEST_NO_TAG("Call with DNS timeout", call_with_dns_time_out),
    TEST_NO_TAG("Cancelled ringing call", cancelled_ringing_call),
    TEST_NO_TAG("Call busy when calling self", call_busy_when_calling_self),
    TEST_NO_TAG("Call rejected because of wrong credential", call_rejected_because_wrong_credentials),
    TEST_NO_TAG("Call rejected without 403 because of wrong credential",
                call_rejected_without_403_because_wrong_credentials),
    TEST_NO_TAG("Call rejected without 403 because of wrong credential and no auth req cb",
                call_rejected_without_403_because_wrong_credentials_no_auth_req_cb),
    TEST_NO_TAG("Call established with rejected INFO", call_established_with_rejected_info),
    TEST_NO_TAG("Call established with rejected RE-INVITE", call_established_with_rejected_reinvite),
    TEST_NO_TAG("Call established with rejected incoming RE-INVITE", call_established_with_rejected_incoming_reinvite),
    TEST_NO_TAG("Call established with rejected RE-INVITE in error",
                call_established_with_rejected_reinvite_with_error),
    TEST_NO_TAG("Call established with rejected RE-INVITE with trans pending error",
                call_established_with_rejected_reinvite_with_trans_pending_error),
    TEST_NO_TAG("Call established with complex rejected operation", call_established_with_complex_rejected_operation),
    TEST_NO_TAG("Call established with rejected info during re-invite",
                call_established_with_rejected_info_during_reinvite),
    TEST_NO_TAG("Incoming INVITE with invalid SDP", incoming_invite_with_invalid_sdp),
    TEST_NO_TAG("Outgoing INVITE with invalid ACK SDP", outgoing_invite_with_invalid_sdp),
    TEST_NO_TAG("Unsuccessful call with transport change after released",
                unsucessfull_call_with_transport_change_after_released),
    TEST_NO_TAG("Call with rtcp-mux not accepted", call_with_rtcp_mux_not_accepted),
    TEST_NO_TAG("Call cancelled with reason", cancel_call_with_error),
    TEST_NO_TAG("Call declined, other ringing device receive CANCEL with reason", cancel_other_device_after_decline),
    TEST_NO_TAG("Call with malformed from", call_with_maformed_from),
    TEST_NO_TAG("Call rejected with 403", call_rejected_with_403),
    TEST_NO_TAG("Call with core without media", call_with_core_without_media)};

test_suite_t call_test_suite = {"Single Call",
                                NULL,
                                NULL,
                                liblinphone_tester_before_each,
                                liblinphone_tester_after_each,
                                sizeof(call_tests) / sizeof(call_tests[0]),
                                call_tests,
                                0};

test_suite_t call2_test_suite = {"Single Call2",
                                 NULL,
                                 NULL,
                                 liblinphone_tester_before_each,
                                 liblinphone_tester_after_each,
                                 sizeof(call2_tests) / sizeof(call2_tests[0]),
                                 call2_tests,
                                 0};

test_suite_t call_not_established_test_suite = {"Single Call (Not established)",
                                                NULL,
                                                NULL,
                                                liblinphone_tester_before_each,
                                                liblinphone_tester_after_each,
                                                sizeof(call_not_established_tests) /
                                                    sizeof(call_not_established_tests[0]),
                                                call_not_established_tests,
                                                304};
