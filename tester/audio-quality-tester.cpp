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

#include <map>
#include <vector>

#include <bctoolbox/defs.h>

#include "address/address.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/basic-chat-room.h"
#include "content/content-type.h"
#include "content/content.h"
#include "content/file-content.h"
#include "core/core.h"
// TODO: Remove me later.
#include "liblinphone_tester.h"
#include "private.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;
class FmtpManager {
public:
	FmtpManager() {
	}
	void setFmtp(const std::string &key, const std::string &value) {
		mFmtp[key] = value;
	}
	std::string toString() {
		return toString(mFmtp);
	}
	std::string toString(std::map<std::string, std::string> data) {
		std::string merge;
		for (auto i : data) {
			if (merge != "") merge += ';';
			if (i.second != "") merge += i.first + "=" + i.second;
			else merge += i.first;
		}
		return merge;
	}
	std::map<std::string, std::string> mFmtp;
};

#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
static void completion_cb(BCTBX_UNUSED(void *user_data), int percentage) {
	fprintf(stdout, "%i %% completed\r", percentage);
	fflush(stdout);
}
#endif

static void audio_call_with_soundcard(const char *codec_name, int clock_rate, bool_t stereo) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphonePayloadType *pt;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/*make sure we have the requested codec */
	pt = linphone_core_get_payload_type(marie->lc, codec_name, clock_rate, 2);
	if (!pt) {
		ms_warning("%s not available, stereo with %s not tested.", codec_name, codec_name);
		goto end;
	}
	if (stereo) linphone_payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	linphone_payload_type_unref(pt);

	pt = linphone_core_get_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (stereo) linphone_payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	linphone_payload_type_unref(pt);

	disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);

	linphone_core_set_use_files(pauline->lc, TRUE);

	/*stereo is supported only without volume control, echo canceller...*/
	linphone_config_set_string(linphone_core_get_config(marie->lc), "sound", "features", "REMOTE_PLAYING");
	linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "features", "REMOTE_PLAYING");

	if (!BC_ASSERT_TRUE(call(pauline, marie))) goto end;
	{
		AudioStream *as =
		    (AudioStream *)linphone_call_get_stream(linphone_core_get_current_call(marie->lc), LinphoneStreamTypeAudio);
		/*
		 * Make sure that the soundcard was really configured to output wideband.
		 */
		if (clock_rate == 48000) {
			int samplerate = 0;
			int nchannels = 0;
			ms_filter_call_method(as->soundread, MS_FILTER_GET_SAMPLE_RATE, &samplerate);
			ms_filter_call_method(as->soundread, MS_FILTER_GET_NCHANNELS, &nchannels);
			BC_ASSERT_GREATER(samplerate, 44100, int, "%i");
			BC_ASSERT_EQUAL(nchannels, stereo ? 2 : 1, int, "%i");
			samplerate = 0;
			nchannels = 0;
			ms_filter_call_method(as->soundwrite, MS_FILTER_GET_SAMPLE_RATE, &samplerate);
			ms_filter_call_method(as->soundread, MS_FILTER_GET_NCHANNELS, &nchannels);
			BC_ASSERT_GREATER(samplerate, 44100, int, "%i");
			BC_ASSERT_EQUAL(nchannels, stereo ? 2 : 1, int, "%i");
		}
	}
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);
	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void
audio_call_stereo_call(const char *codec_name, int clock_rate, int bitrate_override, bool_t stereo, bool_t plc) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphonePayloadType *pt;
	char *stereo_file = bc_tester_res("sounds/vrroom.wav");
	char *recordpath = bc_tester_file("stereo-record.wav");
	bool_t audio_cmp_failed = FALSE;
	OrtpNetworkSimulatorParams simparams = {0};
	unlink(recordpath);

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/*make sure we have the requested codec */
	pt = linphone_core_get_payload_type(marie->lc, codec_name, clock_rate, 2);
	if (!pt) {
		ms_warning("%s not available, stereo with %s not tested.", codec_name, codec_name);
		goto end;
	}
	if (stereo) linphone_payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	if (bitrate_override) linphone_payload_type_set_normal_bitrate(pt, bitrate_override);
	linphone_payload_type_unref(pt);
	pt = linphone_core_get_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (stereo) linphone_payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	if (bitrate_override) linphone_payload_type_set_normal_bitrate(pt, bitrate_override);
	linphone_payload_type_unref(pt);

	disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);

	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, stereo_file);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_record_file(pauline->lc, recordpath);

	linphone_core_set_mtu(marie->lc, 4000);
	linphone_core_set_mtu(pauline->lc, 4000);

	/*stereo is supported only without volume control, echo canceller...*/
	linphone_config_set_string(linphone_core_get_config(marie->lc), "sound", "features", "REMOTE_PLAYING");
	linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "features", "REMOTE_PLAYING");
	if (plc) {
		linphone_config_set_string(linphone_core_get_config(marie->lc), "sound", "features", "PLC");
		linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "features", "PLC");
		// When PLC is enabled, for a 50% loss
		simparams.mode = OrtpNetworkSimulatorOutbound;
		simparams.enabled = TRUE;
		simparams.consecutive_loss_probability = 0.000001f; // Ensure to have fec in n+1 packets
		simparams.loss_rate = 50;
		linphone_core_set_network_simulator_params(marie->lc, &simparams);
	}

	if (!BC_ASSERT_TRUE(call(pauline, marie))) goto end;
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 6000);
	end_call(pauline, marie);

	if (clock_rate != 48000) {
		ms_warning("Similarity checking not implemented for files not having the same sampling rate");
	} else {
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
		double similar;
		double min_threshold = .75f; /*should be above 0.8 in best conditions*/
		double max_threshold = 1.f;
		if (!stereo) {
			/*when opus doesn't transmit stereo, the cross correlation is around 0.6 : as expected, it is not as good as
			 * in full stereo mode*/
			min_threshold = .4f;
			max_threshold = .68f;
		}
		BC_ASSERT_EQUAL(ms_audio_diff(stereo_file, recordpath, &similar, &audio_cmp_params, completion_cb, NULL), 0,
		                int, "%d");
		BC_ASSERT_GREATER(similar, min_threshold, double, "%g");
		BC_ASSERT_LOWER(similar, max_threshold, double, "%g");
		if (similar < min_threshold || similar > max_threshold) {
			audio_cmp_failed = TRUE;
		}
#endif
	}
	if (!audio_cmp_failed) unlink(recordpath);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(stereo_file);
	bc_free(recordpath);
}

static void audio_stereo_call_l16(void) {
	audio_call_stereo_call("L16", 44100, 0, TRUE, FALSE);
}
static void audio_stereo_call_l16_plc(void) {
	audio_call_stereo_call("L16", 44100, 0, TRUE, TRUE);
}

static void audio_stereo_call_opus(void) {
	audio_call_stereo_call("opus", 48000, 150, TRUE, FALSE);
}

static void audio_mono_call_opus(void) {
	/*actually a call where input/output is made with stereo but opus transmits everything as mono*/
	audio_call_stereo_call("opus", 48000, 150, FALSE, FALSE);
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
// Opus issue : Results can contain random silences on FEC. Their number depend on the gain (a very low gain leads to
// get a silence on all FEC) This cannot be solved by the similarity alone because of the randomness behaviour. This
// test is about having a better result with high fec on 2 tries from a first one with the worst fec (not 0)
static void audio_call_loss_resilience(const char *codec_name,
                                       int clock_rate,
                                       int bitrate_override,
                                       int jitterBufferMs,
                                       bool_t stereo,
                                       std::pair<double, double> threshold) {
	((void)(threshold));
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
	LinphoneCoreManager *marie = nullptr, *pauline = nullptr;
	char *playFile = bc_tester_res("sounds/continuous_48000_stereo.wav");
	char *referenceFile = nullptr;
	double similarity = 0.0;
	double similarityRef = 0.0;

	OrtpNetworkSimulatorParams simparams = {0};
	LinphonePayloadType *mariePt = NULL, *paulinePt = NULL;
	int sampleLength = 6000;
	std::string recordFileNameRoot = "loss-record.wav", refRecordFileName;
	FmtpManager marieFmtp, paulineFmtp;
	std::vector<std::string> useinbandfec = {"1"};
	std::vector<float> lossRates = {50.0f};
	std::vector<std::string> packetLossPercentage = {"99"};
	MSAudioDiffParams audioCmpParams = audio_cmp_params;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	if (!BC_ASSERT_PTR_NOT_NULL(marie) || !BC_ASSERT_PTR_NOT_NULL(pauline)) goto end;

	/*make sure we have opus*/
	mariePt = linphone_core_get_payload_type(marie->lc, codec_name, clock_rate, 2);
	paulinePt = linphone_core_get_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (!BC_ASSERT_PTR_NOT_NULL(mariePt) || !BC_ASSERT_PTR_NOT_NULL(paulinePt)) {
		ms_warning("%s not available, stereo with %s and fec not tested.", codec_name, codec_name);
		goto end;
	}

	if (stereo) {
		marieFmtp.setFmtp("stereo", "1");
		marieFmtp.setFmtp("sprop-stereo", "1");
	}
	disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);
	if (bitrate_override) linphone_payload_type_set_normal_bitrate(mariePt, bitrate_override);
	if (bitrate_override) linphone_payload_type_set_normal_bitrate(paulinePt, bitrate_override);
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, playFile);
	linphone_core_set_use_files(pauline->lc, TRUE);
	/*stereo is supported only without volume control, echo canceller...*/
	linphone_config_set_string(linphone_core_get_config(marie->lc), "sound", "features", "REMOTE_PLAYING");
	linphone_config_set_string(linphone_core_get_config(pauline->lc), "sound", "features", "REMOTE_PLAYING");
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "jitter_buffer_min_size", jitterBufferMs);
	linphone_core_set_audio_jittcomp(pauline->lc, jitterBufferMs);
	linphone_core_enable_audio_adaptive_jittcomp(pauline->lc, TRUE);
	linphone_core_enable_adaptive_rate_control(
	    marie->lc,
	    FALSE); // We don't want adaptive rate control here, in order to not interfere with loss recovery algorithms
	linphone_core_enable_adaptive_rate_control(pauline->lc, FALSE);
	linphone_core_enable_generic_comfort_noise(marie->lc, FALSE);
	linphone_core_enable_generic_comfort_noise(pauline->lc, FALSE);

	// Make a call with almost no fec in order to compare the final result
	marieFmtp.setFmtp("useinbandfec", "1");
	marieFmtp.setFmtp("packetlosspercentage", "1");
	refRecordFileName = "result_ref_" + recordFileNameRoot;
	referenceFile = bc_tester_file(refRecordFileName.c_str());
	unlink(referenceFile);
	linphone_core_set_record_file(pauline->lc, referenceFile);
	paulineFmtp = marieFmtp;
	linphone_payload_type_set_recv_fmtp(mariePt, marieFmtp.toString().c_str());
	linphone_payload_type_set_recv_fmtp(paulinePt, paulineFmtp.toString().c_str());
	if (BC_ASSERT_TRUE(call(pauline, marie))) {
		wait_for_until(marie->lc, pauline->lc, NULL, 0, sampleLength + jitterBufferMs);
		end_call(pauline, marie);
	}

	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.consecutive_loss_probability = 0.000001f; // Ensure to have fec in n+1 packets
	for (size_t inbandIndex = 0; inbandIndex < useinbandfec.size();
	     ++inbandIndex) { // Loop to test the impact of useinbandfec
		marieFmtp.setFmtp("useinbandfec", useinbandfec[inbandIndex]);
		for (size_t lossRateIndex = 0; lossRateIndex < lossRates.size();
		     ++lossRateIndex) { // Loop to test the impact of loosing packets
			simparams.loss_rate = lossRates[lossRateIndex];
			linphone_core_set_network_simulator_params(marie->lc, &simparams);
			for (size_t packetLossIndex = 0; packetLossIndex < packetLossPercentage.size(); ++packetLossIndex) {
				double similarityMin = 1.0, similarityMax = 0.0;
				marieFmtp.setFmtp("packetlosspercentage", packetLossPercentage[packetLossIndex]);
				paulineFmtp = marieFmtp;
				linphone_payload_type_set_recv_fmtp(mariePt, marieFmtp.toString().c_str());
				linphone_payload_type_set_recv_fmtp(paulinePt, paulineFmtp.toString().c_str());
				for (int loopIndex = 0; loopIndex < 2; ++loopIndex) {
					std::string recordFileName = useinbandfec[inbandIndex] + "_" +
					                             std::to_string(lossRates[lossRateIndex]) + "_" +
					                             std::to_string(loopIndex) + "_" +
					                             packetLossPercentage[packetLossIndex] + "_out_" + recordFileNameRoot;
					char *recordPath = bc_tester_file(recordFileName.c_str());
					unlink(recordPath);
					linphone_core_set_record_file(pauline->lc, recordPath);
					if (BC_ASSERT_TRUE(call(pauline, marie))) {
						wait_for_until(marie->lc, pauline->lc, NULL, 0, sampleLength + jitterBufferMs);
						end_call(pauline, marie);
						BC_ASSERT_EQUAL(
						    ms_audio_diff(recordPath, referenceFile, &similarity, &audioCmpParams, NULL, NULL), 0, int,
						    "%d");
					}
					similarityMin = min(similarityMin, similarity);
					similarityMax = max(similarityMax, similarity);
					unlink(recordPath);
					bc_free(recordPath);
				}
				BC_ASSERT_GREATER(similarityMax, similarityRef, double, "%g");
				similarityRef = similarityMin; // Min is used if we want to test more than 1 packetLossPercentage
			}
		}
	}
	unlink(referenceFile);
end:

	linphone_payload_type_unref(mariePt);
	linphone_payload_type_unref(paulinePt);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bc_free(referenceFile);
	bc_free(playFile);
#else
	BC_PASS("audio_call_loss_resilience disabled on this platform");
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

static void audio_call_loss_resilience_opus() {
	audio_call_loss_resilience("opus", 48000, 120, 1000, TRUE, std::pair<double, double>(0.7, 1.0));
}

/**
 * - Make a call using opus 48kHz with high bit rate (60kb/s)
 * - Constraint the bandwidth(to 24kb/s) on Marie's outbound to provoque a congestion
 *   	-> check the congestion is detected and requested bitrate is decrease via TMMBR
 *   	-> check a second TMMBR with a higher bandwidth is received: this is the end of the congestion
 * - Wait 20s to assert the ABE will not give false positive
 * - Release the constraint
 * 	-> Check the current (monitored on the last 3 s) upload bandwidth is under 30kb/s
 * - the audio bandwidth estimator should detect it and set the requested bitrate to its orignal value (via TMMBR again)
 *   	-> check Marie receive a TMMBR above 60kb/s (it shall be actually much more)
 * - wait a little while and check the mean upload bandwidth is now above 45kb/s
 * - enable video, check this has disabled the ABE on pauline's side
 * - constraint the bandwidth, wait for congestion
 * - release the constraint and check the VBE is working and that Marie's ABE did not duplicated packets.
 */
static void audio_bandwidth_estimation_base(bool srtp) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphonePayloadType *pt;
	OrtpNetworkSimulatorParams params = {0};
	int lastTMMBRvalue = 0;
	const abe_stats_t *marie_abe_stats = NULL;
	int marie_sent_dup = 0;

	char *stereo_file = bc_tester_res("sounds/vrroom.wav");

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (srtp) {
		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	}

	disable_all_video_codecs_except_one(marie->lc, "VP8");
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_video_activation_policy_unref(pol);

	/*set the video preset to custom so the video quality controller won't update the video size*/
	linphone_core_set_video_preset(marie->lc, "custom");
	linphone_core_set_preferred_framerate(marie->lc, 10);
	linphone_core_set_preferred_video_definition_by_name(marie->lc, "vga");
	linphone_core_set_upload_bandwidth(marie->lc, 430); /*It will result in approxy 350kbit/s VP8 output*/

	/*make sure we have opus, and force the usage of it in stereo, 48kHz at 60 kb/s */
	pt = linphone_core_get_payload_type(marie->lc, "opus", 48000, 2);
	if (!pt) {
		ms_warning("Opus stereo not available, skip test");
		goto end;
	}
	linphone_payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	linphone_payload_type_set_normal_bitrate(pt, 60);
	linphone_payload_type_unref(pt);
	disable_all_audio_codecs_except_one(marie->lc, "opus", 48000);

	pt = linphone_core_get_payload_type(pauline->lc, "opus", 48000, 2);
	linphone_payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	linphone_payload_type_set_normal_bitrate(pt, 60);
	linphone_payload_type_unref(pt);
	disable_all_audio_codecs_except_one(pauline->lc, "opus", 48000);

	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, stereo_file);

	if (!BC_ASSERT_TRUE(call(pauline, marie))) goto end;

	// Lower Marie's outbound bitrate on the network
	params.enabled = TRUE;
	params.loss_rate = 0;
	params.max_bandwidth = 24000;
	params.mode = OrtpNetworkSimulatorOutbound;
	params.max_buffer_size = 72000;
	params.latency = 60;
	linphone_core_set_network_simulator_params(marie->lc, &params);

	// a TMMBR should arrive when the congestion is detected
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, 15000));
	lastTMMBRvalue = marie->stat.last_tmmbr_value_received;
	// a second TMMBR should arrive when the congestion is resolved but still low
	BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received,
	                                       (int)(lastTMMBRvalue * 1.2), 30000, 30000));
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_up_bw(marie), 30, int, "%i");
	lastTMMBRvalue = marie->stat.last_tmmbr_value_received;
	// Wait 20 seconds, we shall not have any false positive: check we do not receive a TMMBR with a higher bandwidth
	// than the last one
	BC_ASSERT_FALSE(
	    wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, lastTMMBRvalue + 1, 20000));
	// Set the bw constraint high enough to release the congestion
	params.max_bandwidth = 500000;
	params.max_bandwidth = 1000000;
	linphone_core_set_network_simulator_params(marie->lc, &params);
	// ABE should detect it and send a TMMBR, bandwidth is likely to be huge, just check we are back above the 60kb/s
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 60000, 25000));
	// Wait a little while for the output bitrate to stabilize and check the new setting is effective
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 4000);
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_up_bw(marie), 45, int, "%i");

	marie_abe_stats = rtp_session_get_audio_bandwidth_estimator_stats(
	    linphone_call_get_stream(linphone_core_get_current_call(marie->lc), LinphoneStreamTypeAudio)
	        ->sessions.rtp_session);
	marie_sent_dup = marie_abe_stats->sent_dup;
	BC_ASSERT_GREATER(marie_sent_dup, 0, int, "%d");

#ifdef VIDEO_ENABLED
	// Add video : This will enable a sendOnly video stream from Marie to Pauline
	BC_ASSERT_TRUE(request_video(marie, pauline, TRUE));
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	// Check the audio bandwidth estimator is now
	// - disabled on Pauline side(as VBE is enabled)
	// - enabled on Marie's side as Video is send only so the VBE is disabled
	BC_ASSERT_FALSE(linphone_call_get_stream(linphone_core_get_current_call(pauline->lc), LinphoneStreamTypeAudio)
	                    ->sessions.rtp_session->audio_bandwidth_estimator_enabled);
	BC_ASSERT_TRUE(linphone_call_get_stream(linphone_core_get_current_call(marie->lc), LinphoneStreamTypeAudio)
	                   ->sessions.rtp_session->audio_bandwidth_estimator_enabled);

	// Set the constraint back (with a 200 kb/s constraint, that shall trigger a congestion with the video enabled)
	// The audio is not bandwidth controlled so 60kb/s goes to it.
	// This should generate a TMMBR requestion the video to use 200*0.7(congestion coeff) - 60(audio bandwidth) = 95
	// kb/s
	params.max_bandwidth = 200000;
	params.max_buffer_size = 400000;
	linphone_core_set_network_simulator_params(marie->lc, &params);
	BC_ASSERT_TRUE(
	    wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 60000, 140000, 30000));
	lastTMMBRvalue = marie->stat.last_tmmbr_value_received;
	BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received,
	                                       (int)(lastTMMBRvalue * 1.2), 220000, 30000));

	// Remove the bw constraint
	params.enabled = FALSE;
	linphone_core_set_network_simulator_params(marie->lc, &params);
	// VBE should detect it and send a TMMBR
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 250000, 25000));

	// Check the ABE was never active while the video is on (no new duplicates sent)
	BC_ASSERT_EQUAL(marie_abe_stats->sent_dup, marie_sent_dup, int, "%d");
#endif

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(stereo_file);
}

static void audio_bandwidth_estimation() {
	audio_bandwidth_estimation_base(false);
}
static void audio_bandwidth_estimation_on_secure_call() {
	audio_bandwidth_estimation_base(true);
}

static void audio_call_with_opus_and_soundcard(void) {
	audio_call_with_soundcard("opus", 48000, TRUE);
}

test_t audio_quality_tests[] = {
    TEST_NO_TAG("Audio loss rate resilience opus", audio_call_loss_resilience_opus),
    TEST_NO_TAG("Simple stereo call with L16", audio_stereo_call_l16),
    TEST_NO_TAG("Simple stereo call with L16 (PLC)", audio_stereo_call_l16_plc),
    TEST_NO_TAG("Simple stereo call with opus", audio_stereo_call_opus),
    TEST_NO_TAG("Simple mono call with opus", audio_mono_call_opus),
    TEST_NO_TAG("Audio bandwidth estimation", audio_bandwidth_estimation),
    TEST_NO_TAG("Audio bandwidth estimation on secure call", audio_bandwidth_estimation_on_secure_call),
    TEST_NO_TAG("Audio call with opus and soundcard", audio_call_with_opus_and_soundcard)};

test_suite_t audio_quality_test_suite = {"Audio Call quality",
                                         NULL,
                                         NULL,
                                         liblinphone_tester_before_each,
                                         liblinphone_tester_after_each,
                                         sizeof(audio_quality_tests) / sizeof(audio_quality_tests[0]),
                                         audio_quality_tests,
                                         750,
                                         2};
