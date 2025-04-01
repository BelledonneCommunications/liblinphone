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

#include <set>
#include <sys/stat.h>
#include <vector>

#include "bctoolbox/list.h"
#include "call/call.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
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
	linphone_core_set_preferred_video_definition_by_name(lc, "VGA");
}
static void disable_all_audio_codecs(LinphoneCore *lc) {
	LinphonePayloadType *pt;
	bctbx_list_t *codecs = linphone_core_get_audio_payload_types(lc);
	bctbx_list_t *elem;
	int index = 0;
	for (elem = codecs; elem != NULL; elem = elem->next, ++index) {
		pt = (LinphonePayloadType *)elem->data;
		linphone_payload_type_enable(pt, FALSE);
	}
	bctbx_list_free_with_data(codecs, (bctbx_list_free_func)linphone_payload_type_unref);
}
typedef struct _flexfec_tests_params {

	LinphoneMediaEncryption encryption_mode;
	bool_t ice;
	bool_t audio_enabled;
	bool_t video_enabled_and_disabled;

} flexfec_tests_params;

static void video_call_with_flexfec_base(flexfec_tests_params params) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 8.f;
	network_params.mode = OrtpNetworkSimulatorOutbound;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");
	fec_stats *fec_stats = NULL;

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
		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
		enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);
	}
	enable_rtp_bundle(marie->lc, TRUE);
	enable_rtp_bundle(pauline->lc, TRUE);

	linphone_core_enable_fec(marie->lc, TRUE);
	linphone_core_enable_fec(pauline->lc, TRUE);

	if (!params.audio_enabled) {
		disable_all_audio_codecs(marie->lc);
		disable_all_audio_codecs(pauline->lc);
	}

	uint64_t expected_recovered_packets = 7;
	LinphonePayloadType *pt_marie = linphone_core_get_payload_type(marie->lc, "AV1", -1, -1);
	LinphonePayloadType *pt_pauline = linphone_core_get_payload_type(marie->lc, "AV1", -1, -1);
	if (pt_marie && pt_pauline) {
		disable_all_video_codecs_except_one(marie->lc, "AV1");
		disable_all_video_codecs_except_one(pauline->lc, "AV1");
	} else {
		disable_all_video_codecs_except_one(marie->lc, "VP8");
		disable_all_video_codecs_except_one(pauline->lc, "VP8");
		expected_recovered_packets = 50;
	}
	linphone_payload_type_unref(pt_marie);
	linphone_payload_type_unref(pt_pauline);

	enable_video_stream(marie->lc, pol);
	enable_video_stream(pauline->lc, pol);

	linphone_video_activation_policy_unref(pol);

	BC_ASSERT_TRUE(call(marie, pauline));
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	VideoStream *vstream = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);
	LinphoneCallStats *call_stats = NULL;
	uint64_t repaired_packets = 0;
	uint64_t cumulative_lost_packets = 0;
	fec_stats = fec_stream_get_stats(vstream->ms.fec_stream);

	BC_ASSERT_TRUE(linphone_call_params_fec_enabled(linphone_call_get_current_params(pauline_call)));
	BC_ASSERT_TRUE(linphone_call_params_fec_enabled(linphone_call_get_current_params(marie_call)));

	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, NULL, 0, 20000));
	if (vstream->ms.fec_stream) {
		fec_stats = fec_stream_get_stats(vstream->ms.fec_stream);
		BC_ASSERT_TRUE(wait_for_until_for_uint64(marie->lc, pauline->lc, &fec_stats->packets_recovered,
		                                         expected_recovered_packets, 25000));
		ms_message("%s recovered %0d packets. The expected value is %0d", linphone_core_get_identity(marie->lc),
		           static_cast<int>(fec_stats->packets_recovered), static_cast<int>(expected_recovered_packets));
		call_stats = linphone_call_get_video_stats(marie_call);
		repaired_packets = linphone_call_stats_get_fec_repaired_packets_number(call_stats);
		cumulative_lost_packets = linphone_call_stats_get_fec_cumulative_lost_packets_number(call_stats);
		BC_ASSERT_TRUE(repaired_packets == fec_stats->packets_recovered);
		BC_ASSERT_TRUE(cumulative_lost_packets == fec_stats->packets_not_recovered);
		BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_fec_upload_bandwidth(call_stats), 0., float, "%f");
		BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_fec_download_bandwidth(call_stats), 0., float, "%f");
		if (call_stats) linphone_call_stats_unref(call_stats);
		call_stats = NULL;

		if (params.video_enabled_and_disabled) {

			stats initial_marie_stat = marie->stat;
			stats initial_pauline_stat = pauline->stat;

			// now disable video
			pauline_call = linphone_core_get_current_call(pauline->lc);
			LinphoneCallParams *new_params = linphone_core_create_call_params(pauline->lc, pauline_call);
			linphone_call_params_enable_video(new_params, FALSE);
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

			// wait
			wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

			BC_ASSERT_FALSE(linphone_call_params_fec_enabled(linphone_call_get_current_params(pauline_call)));
			BC_ASSERT_FALSE(linphone_call_params_fec_enabled(linphone_call_get_current_params(marie_call)));

			initial_marie_stat = marie->stat;
			initial_pauline_stat = pauline->stat;

			call_stats = linphone_call_get_video_stats(marie_call);
			repaired_packets = linphone_call_stats_get_fec_repaired_packets_number(call_stats);
			cumulative_lost_packets = linphone_call_stats_get_fec_cumulative_lost_packets_number(call_stats);
			BC_ASSERT_TRUE(repaired_packets == 0);
			BC_ASSERT_TRUE(cumulative_lost_packets == 0);
			BC_ASSERT_EQUAL(linphone_call_stats_get_fec_upload_bandwidth(call_stats), 0., float, "%f");
			BC_ASSERT_EQUAL(linphone_call_stats_get_fec_download_bandwidth(call_stats), 0., float, "%f");
			if (call_stats) linphone_call_stats_unref(call_stats);
			call_stats = NULL;

			// now enable video again
			new_params = linphone_core_create_call_params(marie->lc, marie_call);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_update(pauline_call, new_params);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
			                        initial_pauline_stat.number_of_LinphoneCallUpdating + 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
			                        initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
			                        initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning,
			                        initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1));

			// Check video parameters
			pauline_call_params = linphone_call_get_current_params(pauline_call);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(pauline_call_params));
			marie_call_params = linphone_call_get_current_params(marie_call);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_call_params));

			BC_ASSERT_TRUE(linphone_call_params_fec_enabled(pauline_call_params));
			BC_ASSERT_TRUE(linphone_call_params_fec_enabled(marie_call_params));

			vstream = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);
			fec_stats = fec_stream_get_stats(vstream->ms.fec_stream);
			BC_ASSERT_TRUE(wait_for_until_for_uint64(marie->lc, pauline->lc, &fec_stats->packets_recovered,
			                                         expected_recovered_packets, 45000));
			ms_message("%s recovered %0d packets. The expected value is %0d", linphone_core_get_identity(marie->lc),
			           static_cast<int>(fec_stats->packets_recovered), static_cast<int>(expected_recovered_packets));

			call_stats = linphone_call_get_video_stats(marie_call);
			repaired_packets = linphone_call_stats_get_fec_repaired_packets_number(call_stats);
			cumulative_lost_packets = linphone_call_stats_get_fec_cumulative_lost_packets_number(call_stats);
			BC_ASSERT_TRUE(repaired_packets == fec_stats->packets_recovered);
			BC_ASSERT_TRUE(cumulative_lost_packets == fec_stats->packets_not_recovered);
			BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_fec_upload_bandwidth(call_stats), 0., float, "%f");
			BC_ASSERT_GREATER_STRICT(linphone_call_stats_get_fec_download_bandwidth(call_stats), 0., float, "%f");
			if (call_stats) linphone_call_stats_unref(call_stats);
			call_stats = NULL;
		}
	} else {
		BC_FAIL("FEC not enabled.");
	}
	end_call(marie, pauline);

	if (call_stats) linphone_call_stats_unref(call_stats);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_with_flexfec_enable_and_disable_video(void) {
	flexfec_tests_params params{LinphoneMediaEncryptionNone, FALSE, TRUE, TRUE};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_ice(void) {
	flexfec_tests_params params{LinphoneMediaEncryptionNone, TRUE, FALSE, FALSE};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_srtp(void) {
	flexfec_tests_params params{LinphoneMediaEncryptionSRTP, FALSE, FALSE, FALSE};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_dtls(void) {
	flexfec_tests_params params{LinphoneMediaEncryptionDTLS, FALSE, FALSE, FALSE};
	video_call_with_flexfec_base(params);
}
static void video_call_with_flexfec_and_zrtp(void) {
	flexfec_tests_params params{LinphoneMediaEncryptionZRTP, FALSE, TRUE, FALSE};
	video_call_with_flexfec_base(params);
}

// Iterate during max_iteration*delay_ms until the participant received at least tmmbr_min_count TMMBR
static void wait_for_tmmbr_count(const int max_iteration,
                                 const int delay_ms,
                                 const size_t tmmbr_min_count,
                                 LinphoneCoreManager *participant,
                                 bctbx_list_t *lcs) {
	int last_tmmbr_value = 0;
	std::set<int> tmmbr_received = {participant->stat.last_tmmbr_value_received};
	const int dummy = 0;
	for (int i = 0; i < max_iteration; i++) {
		wait_for_list(lcs, &dummy, 1, delay_ms);
		last_tmmbr_value = participant->stat.last_tmmbr_value_received;
		tmmbr_received.insert(last_tmmbr_value);
		if (tmmbr_received.size() > tmmbr_min_count) break;
	}
}
static void video_call_with_flexfec_bandwidth_variation(void) {
	/*This test checks that the FEC level adapts to the current bandwidth, for a given the loss rate. During the step 1,
	 * the available bandwidth is high. The test waits for few seconds to let the video quality controller set the
	 * appropriate video encoder configuration and FEC level. The FEC level must be high (L, D) = (5, 5), (4, 4) or (3,
	 * 3). During step 2 the bandwidth of Pauline is significantly reduced. The congestion must be detected and the
	 * video quality controller must adapt the encoder configuration to reduce the upload bandwidth. The FEC level must
	 * not exceed (5, 5) in 1D interleaved, and could even be (0, 0). It is not possible to infer the exact value of the
	 * FEC level because the estimation of the loss rate is too noisy. When the congestion is resolved, the
	 * configuration is updated for low bandwidth. Then the step 3 starts, with high bandwidth for Pauline. The video
	 * bandwidth estimator of Marie accumulates progessively enough measurements to send a TMMBR with the new reliable
	 * estimation of the high bandwidth, then the video encoder and the FEC of Pauline are updated. The FEC must reach
	 * again the high values (5, 5), (4, 4) or (3, 3).*/

	flexfec_tests_params params{
	    LinphoneMediaEncryptionNone,
	    FALSE,
	    FALSE,
	};

	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 3.f;
	network_params.mode = OrtpNetworkSimulatorOutbound;
	network_params.max_bandwidth = 5000000.f;
	network_params.max_buffer_size = (int)network_params.max_bandwidth;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	// VP8 instead of AV1, otherwise
	// - the video bandwidth estimator doesn't get enough packets to be reliable
	// - as the size of the packets are equal with AV1 and FEC, the overhead is small, then the FEC level doesn't
	// decrease a lot in case of low bandwidth
	disable_all_video_codecs_except_one(marie->lc, "VP8");
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

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

	linphone_core_set_network_simulator_params(pauline->lc, &network_params);
	linphone_core_set_network_simulator_params(marie->lc, &network_params);

	if (params.ice) {
		enable_stun_in_core(marie, TRUE, TRUE);
		enable_stun_in_core(pauline, TRUE, TRUE);
	}
	enable_rtp_bundle(marie->lc, TRUE);
	enable_rtp_bundle(pauline->lc, TRUE);

	linphone_core_enable_fec(marie->lc, TRUE);
	linphone_core_enable_fec(pauline->lc, TRUE);

	params.audio_enabled = TRUE;

	if (!params.audio_enabled) {
		disable_all_audio_codecs(marie->lc);
		disable_all_audio_codecs(pauline->lc);
	}

	enable_video_stream(marie->lc, pol);
	enable_video_stream(pauline->lc, pol);

	linphone_video_activation_policy_unref(pol);

	BC_ASSERT_TRUE(call(marie, pauline));

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	VideoStream *pauline_vstream = (VideoStream *)linphone_call_get_stream(pauline_call, LinphoneStreamTypeVideo);

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	BC_ASSERT_PTR_NOT_NULL(pauline_vstream->ms.sessions.fec_session);
	if (pauline_vstream->ms.sessions.fec_session == NULL) {
		BC_FAIL("No FEC session");
		end_call(marie, pauline);
		bctbx_list_free(lcs);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		return;
	}

	const rtp_stats *stats_fec_session_pauline = rtp_session_get_stats(pauline_vstream->ms.sessions.fec_session);
	uint64_t prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
	const rtp_stats *stats_rtp_session_pauline = rtp_session_get_stats(pauline_vstream->ms.sessions.rtp_session);
	uint64_t prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
	const fec_stats *fec_stats_pauline = fec_stream_get_stats(pauline_vstream->ms.fec_stream);
	uint64_t prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
	uint64_t prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
	const int dummy = 0;

	// 1. Set enough bandwidth at start to enable FEC and reach level 3, 4 or 5

	// check that fec is disabled at start
	BC_ASSERT_PTR_NOT_NULL(pauline_vstream->ms.fec_stream);
	BC_ASSERT_FALSE(fec_stream_enabled(pauline_vstream->ms.fec_stream));

	// Wait for first TMMBR
	wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, 20000);

	// wait for bitrate stabilization after first tmmbr, to enable FEC and stabilize parameters
	wait_for_list(lcs, &dummy, 1, 21000);

	// measure FEC parameters
	prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
	prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
	prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
	prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
	wait_for_list(lcs, &dummy, 1, 1000);

	float rtp_packets = (float)(stats_rtp_session_pauline->packet_sent - prev_rtp_sent_number);
	float fec_packets = (float)(stats_fec_session_pauline->packet_sent - prev_fec_sent_number);
	float current_overhead = fec_stream_get_overhead(pauline_vstream->ms.fec_stream);
	int it = 0;
	while ((fec_packets / rtp_packets < 0.35f || current_overhead > 0.95f) && it < 10) {
		wait_for_list(lcs, &dummy, 1, 4000);
		prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
		prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
		prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
		prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
		wait_for_list(lcs, &dummy, 1, 1000);
		rtp_packets = (float)(stats_rtp_session_pauline->packet_sent - prev_rtp_sent_number);
		fec_packets = (float)(stats_fec_session_pauline->packet_sent - prev_fec_sent_number);
		current_overhead = fec_stream_get_overhead(pauline_vstream->ms.fec_stream);
		it++;
	}

	// the expected FEC parameters (L, D) are (5, 5) or (4, 4) or (3, 3) in 2D

	// check that FEC is enabled
	BC_ASSERT_TRUE(pauline_vstream->ms.fec_stream != NULL);
	BC_ASSERT_TRUE(fec_stream_enabled(pauline_vstream->ms.fec_stream));
	// check the ratio between number of repair packets and source packets
	BC_ASSERT_GREATER(fec_packets / rtp_packets, 0.35f, float, "%f");
	if (fec_packets / rtp_packets < 0.35f) {
		BC_FAIL("Wrong parameters for FEC with high bandwidth");
		end_call(marie, pauline);
		bctbx_list_free(lcs);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		return;
	}
	// check the number of rows and columns repair packets, should be equal
	int fec_row_packets =
	    static_cast<int>(fec_stats_pauline->row_repair_sent) - static_cast<int>(prev_fec_row_sent_number);
	int fec_col_packets =
	    static_cast<int>(fec_stats_pauline->col_repair_sent) - static_cast<int>(prev_fec_col_sent_number);
	BC_ASSERT_LOWER(abs(fec_row_packets - fec_col_packets), 5, int, "%d");

	// 2. Decrease max bandwidth to reduce FEC level and video encoder configuration
	network_params.max_bandwidth = 300000.f;
	linphone_core_set_network_simulator_params(pauline->lc, &network_params);
	ortp_message("Update network parameters: reduce bw for pauline to %f", network_params.max_bandwidth);

	wait_for_tmmbr_count(20, 2000, 1, pauline, lcs);

	// measure FEC parameters
	prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
	prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
	prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
	prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
	uint64_t prev_fec_recv_number = stats_fec_session_pauline->packet_recv;
	wait_for_list(lcs, &dummy, 1, 5000);

	// the expected FEC parameters (L, D) are (0, 0) or (10, 0) or (5, 5) in 1D
	// check the ratio between number of repair packets and source packets
	rtp_packets = (float)(stats_rtp_session_pauline->packet_sent - prev_rtp_sent_number);
	fec_packets = (float)(stats_fec_session_pauline->packet_sent - prev_fec_sent_number);
	BC_ASSERT_LOWER(fec_packets / rtp_packets, 0.25f, float, "%f");

	// check that Pauline still receives FEC packets from Marie
	BC_ASSERT_TRUE(stats_fec_session_pauline->packet_recv > prev_fec_recv_number + 10);

	// 3. Increase max bandwidth to increase FEC level
	network_params.max_bandwidth = 5000000.f;
	linphone_core_set_network_simulator_params(pauline->lc, &network_params);
	ortp_message("Update network parameters: increase bw for pauline to %f", network_params.max_bandwidth);

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 20000);

	// measure FEC parameters
	prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
	prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
	prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
	prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
	wait_for_list(lcs, &dummy, 1, 1000);

	// sometimes the loss rate is locally low, then the FEC paremeters are low
	// if the current FEC level is low, wait a bit longer before the test
	rtp_packets = (float)(stats_rtp_session_pauline->packet_sent - prev_rtp_sent_number);
	fec_packets = (float)(stats_fec_session_pauline->packet_sent - prev_fec_sent_number);
	current_overhead = fec_stream_get_overhead(pauline_vstream->ms.fec_stream);
	it = 0;
	while ((fec_packets / rtp_packets < 0.35f || current_overhead > 0.95f) && it < 30) {
		wait_for_list(lcs, &dummy, 1, 4000);
		prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
		prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
		prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
		prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
		wait_for_list(lcs, &dummy, 1, 1000);
		rtp_packets = (float)(stats_rtp_session_pauline->packet_sent - prev_rtp_sent_number);
		fec_packets = (float)(stats_fec_session_pauline->packet_sent - prev_fec_sent_number);
		current_overhead = fec_stream_get_overhead(pauline_vstream->ms.fec_stream);
		it++;
	}

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 20000);
	prev_rtp_sent_number = stats_rtp_session_pauline->packet_sent;
	prev_fec_sent_number = stats_fec_session_pauline->packet_sent;
	prev_fec_row_sent_number = fec_stats_pauline->row_repair_sent;
	prev_fec_col_sent_number = fec_stats_pauline->col_repair_sent;
	wait_for_list(lcs, &dummy, 1, 1000);

	// the expected FEC parameters (L, D) are (5, 5) or (4, 4) or (3, 3) in 2D

	// check that FEC is enabled
	BC_ASSERT_TRUE(fec_stream_enabled(pauline_vstream->ms.fec_stream));
	BC_ASSERT_TRUE(stats_fec_session_pauline->packet_sent > prev_fec_sent_number + 10);

	// check the ratio between number of repair packets and source packets
	rtp_packets = (float)(stats_rtp_session_pauline->packet_sent - prev_rtp_sent_number);
	fec_packets = (float)(stats_fec_session_pauline->packet_sent - prev_fec_sent_number);
	BC_ASSERT_GREATER(fec_packets / rtp_packets, 0.35f, float, "%f");
	// check the number of rows and columns repair packets, should be equal
	fec_row_packets = static_cast<int>(fec_stats_pauline->row_repair_sent) - static_cast<int>(prev_fec_row_sent_number);
	fec_col_packets = static_cast<int>(fec_stats_pauline->col_repair_sent) - static_cast<int>(prev_fec_col_sent_number);
	BC_ASSERT_LOWER(abs(fec_row_packets - fec_col_packets), 5, int, "%d");

	end_call(marie, pauline);
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
int findAnomaly(std::vector<float> &nums, float lim_inf, float lim_sup) {
	int low_val_number = 0;
	for (float value : nums) {
		if (value < lim_inf) {
			low_val_number++;
		} else if (value > lim_sup) {
			low_val_number++;
		}
	}
	return low_val_number;
}
int video_call_fps_measurement(bool_t withFEC) {

	flexfec_tests_params params{
	    LinphoneMediaEncryptionNone,
	    FALSE,
	    FALSE,
	};

	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 5.f;
	network_params.max_bandwidth = 5000000.f;
	network_params.mode = OrtpNetworkSimulatorOutbound;
	network_params.consecutive_loss_probability = 0.7f;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	LinphonePayloadType *pt_marie = linphone_core_get_payload_type(marie->lc, "AV1", -1, -1);
	LinphonePayloadType *pt_pauline = linphone_core_get_payload_type(marie->lc, "AV1", -1, -1);
	if (pt_marie && pt_pauline) {
		disable_all_video_codecs_except_one(marie->lc, "AV1");
		disable_all_video_codecs_except_one(pauline->lc, "AV1");
	} else {
		disable_all_video_codecs_except_one(marie->lc, "VP8");
		disable_all_video_codecs_except_one(pauline->lc, "VP8");
	}
	linphone_payload_type_unref(pt_marie);
	linphone_payload_type_unref(pt_pauline);

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

	enable_rtp_bundle(marie->lc, TRUE);
	enable_rtp_bundle(pauline->lc, TRUE);

	linphone_core_enable_fec(pauline->lc, TRUE);
	if (withFEC) {
		linphone_core_enable_fec(marie->lc, TRUE);
	} else {
		linphone_core_enable_fec(marie->lc, FALSE);
	}

	enable_video_stream(marie->lc, pol);
	enable_video_stream(pauline->lc, pol);

	linphone_video_activation_policy_unref(pol);

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	VideoStream *vstream = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);

	if (withFEC) {
		BC_ASSERT_TRUE(linphone_call_params_fec_enabled(linphone_call_get_current_params(marie_call)));
		BC_ASSERT_TRUE(linphone_call_params_fec_enabled(linphone_call_get_current_params(pauline_call)));
	} else {
		BC_ASSERT_FALSE(linphone_call_params_fec_enabled(linphone_call_get_current_params(marie_call)));
		BC_ASSERT_FALSE(linphone_call_params_fec_enabled(linphone_call_get_current_params(pauline_call)));
	}

	// wait enough to start recovering lost packets with FEC
	if (vstream->ms.fec_stream) {
		fec_stats *stats = fec_stream_get_stats(vstream->ms.fec_stream);
		uint64_t expected_recovered_packets = 5;
		BC_ASSERT_TRUE(wait_for_until_for_uint64(marie->lc, pauline->lc, &stats->packets_recovered,
		                                         expected_recovered_packets, 60000));
	} else {
		const int dummy = 0;
		wait_for_list(lcs, &dummy, 1, 2000);
	}

	// measure FPS during time interval
	MSTimeSpec ts;
	liblinphone_tester_clock_start(&ts);
	int time_to_wait = 10000;
	std::vector<float> fps;
	do {
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 50);
		fps.push_back(video_stream_get_received_framerate(vstream));
		ortp_message("fps measured on video output = %f", fps.back());
	} while (!liblinphone_tester_clock_elapsed(&ts, time_to_wait));
	end_call(marie, pauline);
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	// detects anomalies
	float lim_inf = 20.f;
	float lim_sup = 40.f;
	int anomaly_count = findAnomaly(fps, lim_inf, lim_sup);
	ortp_message("anomaly count: %d, nb of values: %d", anomaly_count, (int)fps.size());
	if (anomaly_count > (int)fps.size() - 10) {
		ortp_message("Warning: too much anomalies detected, %d for %d measurements", anomaly_count, (int)fps.size());
	}

	return anomaly_count;
}
static void video_call_with_flexfec_check_fps() {
	int anomaly_with_fec = 0;
	int anomaly_without_fec = 0;
	bool test_again = true;
	int test_count = 1;
	// the test is rerun in the rare case of failure due to the short delay in measurement.
	while (test_again) {
		ortp_message("run test %d", test_count);
		anomaly_with_fec = video_call_fps_measurement(TRUE);
		anomaly_without_fec = video_call_fps_measurement(FALSE);
		if (anomaly_without_fec <= anomaly_with_fec) {
			ortp_message("test failed: anomaly_without_fec (%d) <= anomaly_with_fec (%d)", anomaly_without_fec,
			             anomaly_with_fec);
		}
		test_again = ((anomaly_without_fec <= anomaly_with_fec + 10) || (anomaly_with_fec > 150)) && (test_count < 3);
		test_count++;
	}
	BC_ASSERT_GREATER_STRICT(anomaly_without_fec, anomaly_with_fec + 10, int, "%d");
}

static test_t call_flexfec_tests[] = {
    TEST_NO_TAG("Video call with flexfec and video on and off", video_call_with_flexfec_enable_and_disable_video),
    TEST_NO_TAG("Video call with flexfec and ice", video_call_with_flexfec_and_ice),
    TEST_NO_TAG("Video call with flexfec and srtp", video_call_with_flexfec_and_srtp),
    TEST_NO_TAG("Video call with flexfec and dtls", video_call_with_flexfec_and_dtls),
    TEST_NO_TAG("Video call with flexfec and zrtp", video_call_with_flexfec_and_zrtp),
    TEST_NO_TAG("Video call with flexfec bandwidth variation", video_call_with_flexfec_bandwidth_variation),
    TEST_NO_TAG("Video call with flexfec check fps", video_call_with_flexfec_check_fps),
};

test_suite_t call_flexfec_suite = {"Call with FlexFec",
                                   NULL,
                                   NULL,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(call_flexfec_tests) / sizeof(call_flexfec_tests[0]),
                                   call_flexfec_tests};
#endif // VIDEO_ENABLED
