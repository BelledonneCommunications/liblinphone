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

#include <bctoolbox/defs.h>

#include "liblinphone_tester.h"
#include "linphone/api/c-alert.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-dictionary.h"
#include "linphone/core.h"
#include "tester_utils.h"

#ifdef VIDEO_ENABLED

static void _video_call_with_explicit_bandwidth_limit(bool_t bandwidth_is_specific_for_video) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneCallParams *params;
	LinphoneCall *pauline_call, *marie_call;
	const int bandwidth_limit = 128;

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	disable_all_audio_codecs_except_one(marie->lc, "opus", 48000);
	disable_all_audio_codecs_except_one(pauline->lc, "opus", 48000);

	if (!bandwidth_is_specific_for_video) {
		/* Pauline has an explicit download bandwidth. */
		linphone_core_set_download_bandwidth(pauline->lc, bandwidth_limit);
		/* Set a much higher upload bandwidth for marie, so that we can check that the minimum of the two is taken
		 * for Pauline's encoder.*/
		linphone_core_set_upload_bandwidth(marie->lc, 1024);
	}

	/*set the video preset to custom so the video quality controller won't update the video size*/
	linphone_core_set_video_preset(marie->lc, "custom");
	linphone_core_set_preferred_video_definition_by_name(
	    marie->lc, "vga"); /*It would result in approxy 350kbit/s VP8 output without the bandwidth limit.*/

	linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);

	if (pauline_call && marie_call) {
		params = linphone_core_create_call_params(pauline->lc, pauline_call);
		if (bandwidth_is_specific_for_video) {
			linphone_call_params_set_video_download_bandwidth(params, bandwidth_limit);
		}
		linphone_call_accept_with_params(pauline_call, params);
		linphone_call_params_unref(params);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
		{
			VideoStream *vstream = (VideoStream *)linphone_call_get_stream(marie_call, LinphoneStreamTypeVideo);
			MSVideoConfiguration vconf;

			BC_ASSERT_PTR_NOT_NULL(vstream);

			if (vstream) {
				/* check that the encoder has been set a bitrate lower than the bandwidth limit. */
				ms_filter_call_method(vstream->ms.encoder, MS_VIDEO_ENCODER_GET_CONFIGURATION, &vconf);
				if (bandwidth_is_specific_for_video) {
					BC_ASSERT_EQUAL(vconf.required_bitrate, bandwidth_limit * 1000, int, "%d");
				} else {
					BC_ASSERT_LOWER(vconf.required_bitrate, 100000, int, "%d");
					BC_ASSERT_GREATER(vconf.required_bitrate, 80000, int, "%d");
				}
			}
		}
		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_with_explicit_bandwidth_limit(void) {
	_video_call_with_explicit_bandwidth_limit(FALSE);
}

static void video_call_with_explicit_bandwidth_limit_for_stream(void) {
	_video_call_with_explicit_bandwidth_limit(TRUE);
}

/*
 * This test simulates a network congestion in the video flow from marie to pauline.
 * The stream from pauline to marie is not under test.
 * It checks that a first TMMBR consecutive to congestion detection is received, and a second one after congestion
 *resolution is received a few seconds later. The parameters used for the network simulator correspond to a "light
 *congestion", which are the ones that translate into relatively small packet losses, hence the more difficult to detect
 *at first sight.
 *
 **/
static void video_call_with_thin_congestion(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	OrtpNetworkSimulatorParams simparams = {0};

	disable_all_video_codecs_except_one(marie->lc, "VP8");
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	/*set the video preset to custom so the video quality controller won't update the video size*/
	linphone_core_set_video_preset(marie->lc, "custom");
	linphone_core_set_preferred_framerate(marie->lc, 15);
	linphone_core_set_preferred_video_definition_by_name(marie->lc, "vga");
	linphone_core_set_upload_bandwidth(marie->lc, 430); /*It will result in approxy 350kbit/s VP8 output*/

	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.max_bandwidth = 430000; /*we first limit to 430 kbit/s*/
	simparams.max_buffer_size = (int)simparams.max_bandwidth;
	simparams.latency = 60;
	linphone_core_set_network_simulator_params(marie->lc, &simparams);

	if (BC_ASSERT_TRUE(call(marie, pauline))) {

		/* Wait ten seconds. The bandwidth estimator will confirm the 430 kbit/s available. */
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 10000);
		/* Now suddenly limit to 300 kbit/s */
		simparams.max_bandwidth = 300000;
		linphone_core_set_network_simulator_params(marie->lc, &simparams);
		/*A congestion is expected, which will result in a TMMBR to be emitted by pauline*/
		/*wait for the TMMBR*/
		BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 170000,
		                                       280000, 30000));

		/*another tmmbr with a greater value is expected once the congestion is resolved*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 250000, 50000));
		LinphoneCall *call = linphone_core_get_current_call(pauline->lc);
		if (BC_ASSERT_PTR_NOT_NULL(call)) BC_ASSERT_GREATER(linphone_call_get_current_quality(call), 4.f, float, "%f");

		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void on_tmmbr_received(LinphoneCall *call, int stream_index, int tmmbr) {
	if (stream_index == _linphone_call_get_main_video_stream_index(call)) {
		stats *stat = get_stats(linphone_call_get_core(call));
		stat->tmmbr_received_from_cb = tmmbr;
	}
}

static void call_created(BCTBX_UNUSED(LinphoneCore *lc), LinphoneCall *call) {
	LinphoneCallCbs *cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	linphone_call_cbs_set_tmmbr_received(cbs, on_tmmbr_received);
	linphone_call_add_callbacks(call, cbs);
	linphone_call_cbs_unref(cbs);
}

/*
 * This test simulates a higher bandwith available from marie than expected.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie received a TMMBR with the approximate value corresponding to the new
 *bandwidth.
 *
 **/
static void video_call_with_high_bandwidth_available(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	OrtpNetworkSimulatorParams simparams = {0};
	LinphoneCoreCbs *core_cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	disable_all_video_codecs_except_one(marie->lc, "VP8");
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_set_preferred_video_definition_by_name(marie->lc, "QVGA");
	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.max_bandwidth = 1000000;
	simparams.max_buffer_size = (int)simparams.max_bandwidth;
	simparams.latency = 60;
	linphone_core_set_network_simulator_params(marie->lc, &simparams);

	linphone_core_cbs_set_call_created(core_cbs, call_created);
	linphone_core_add_callbacks(marie->lc, core_cbs);

	if (BC_ASSERT_TRUE(call(marie, pauline))) {
		/*wait a little in order to have traffic*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, NULL, 5, 50000));

		BC_ASSERT_GREATER((float)marie->stat.last_tmmbr_value_received, 750000.f, float, "%f");
		BC_ASSERT_LOWER((float)marie->stat.last_tmmbr_value_received, 1000000.f, float, "%f");

		end_call(marie, pauline);
	}
	linphone_core_cbs_unref(core_cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_expected_fps_for_specified_bandwidth(int bandwidth, int expected_fps, const char *resolution) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	OrtpNetworkSimulatorParams simparams = {0};

	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) >= 2) {
		disable_all_video_codecs_except_one(marie->lc, "VP8");
		disable_all_video_codecs_except_one(pauline->lc, "VP8");

		// Use goog-remb in this test
		linphone_core_enable_goog_remb(marie->lc, TRUE);
		linphone_core_enable_goog_remb(pauline->lc, TRUE);

		linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);

		LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
		linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
		linphone_core_set_video_activation_policy(marie->lc, vpol);
		linphone_core_set_video_activation_policy(pauline->lc, vpol);
		linphone_video_activation_policy_unref(vpol);

		linphone_core_set_preferred_video_definition_by_name(marie->lc, resolution);
		simparams.mode = OrtpNetworkSimulatorOutbound;
		simparams.enabled = TRUE;
		simparams.max_bandwidth = (float)bandwidth;
		simparams.max_buffer_size = bandwidth;
		simparams.latency = 60;

		linphone_core_set_network_simulator_params(marie->lc, &simparams);

		if (BC_ASSERT_TRUE(call(marie, pauline))) {
			LinphoneCall *call = linphone_core_get_current_call(marie->lc);
			VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
			int count;
			/*wait some time until the target fps is reached. Indeed the bandwidth measurement may take several
			 iterations to converge to a value big enough to allow mediastreamer2 to switch to the high fps profile*/

			for (count = 0; count < 10; count++) {
				/*wait for at least the first TMMBR to arrive*/
				BC_ASSERT_TRUE(
				    wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, 10000));

				if ((int)vstream->configured_fps == expected_fps) {
					break;
				} else {
					/*target fps not reached yet, wait more time*/
					wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);
				}
			}
			BC_ASSERT_EQUAL((int)vstream->configured_fps, expected_fps, int, "%d");
			end_call(marie, pauline);
		}
	} else {
		BC_PASS("Test requires at least a dual core");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This test simulates a video call with a lower bandwidth than the required_bitrate of the lowest given configuration.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie, after receiving a TMMBR, has her fps set to the lowest given configuration.
 * This test requires at least a computer with 2 CPUs.
 *
 **/
static void video_call_expected_fps_for_low_bandwidth(void) {
	video_call_expected_fps_for_specified_bandwidth(350000, 18, "vga");
}

/*
 * This test simulates a video call with a regular bandwidth that is between a given configuration.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie, after receiving a TMMBR, has her fps set to the expected given
 *configuration. This test requires at least a computer with 2 CPUs.
 *
 **/
static void video_call_expected_fps_for_regular_bandwidth(void) {
	video_call_expected_fps_for_specified_bandwidth(580000, 25, "vga");
}

/*
 * This test simulates a video call with a higher bandwidth than the bitrate_limit of the highest given configuration.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie, after receiving a TMMBR, has her fps set to the highest given
 *configuration. This test requires at least a computer with 2 CPUs.
 *
 **/
static void video_call_expected_fps_for_high_bandwidth(void) {
	video_call_expected_fps_for_specified_bandwidth(5000000, 30, "vga");
}

static void video_call_expected_size_for_specified_bandwidth_with_congestion(
    int bandwidth, int width, int height, const char *resolution, const char *video_codec) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	OrtpNetworkSimulatorParams simparams = {0};

	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) >= 2) {
		if (linphone_core_find_payload_type(marie->lc, video_codec, -1, -1) != NULL) {
			linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(marie->lc, TRUE);
			linphone_core_enable_video_display(marie->lc, TRUE);
			linphone_core_enable_video_capture(pauline->lc, TRUE);
			linphone_core_enable_video_display(pauline->lc, TRUE);

			disable_all_video_codecs_except_one(marie->lc, video_codec);

			LinphoneVideoActivationPolicy *vpol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
			linphone_core_set_video_activation_policy(marie->lc, vpol);
			linphone_core_set_video_activation_policy(pauline->lc, vpol);
			linphone_video_activation_policy_unref(vpol);

			linphone_core_set_preferred_video_definition_by_name(marie->lc, resolution);

			simparams.mode = OrtpNetworkSimulatorOutbound;
			simparams.enabled = TRUE;
			simparams.max_bandwidth = (float)bandwidth;
			simparams.max_buffer_size = bandwidth;
			simparams.latency = 60;
			linphone_core_set_network_simulator_params(marie->lc, &simparams);

			if (BC_ASSERT_TRUE(call(marie, pauline))) {
				LinphoneCall *call = linphone_core_get_current_call(marie->lc);
				VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
				MSVideoConfiguration vconf;

				BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received,
				                                       1, (int)(0.7 * bandwidth), 50000));

				ms_filter_call_method(vstream->ms.encoder, MS_VIDEO_ENCODER_GET_CONFIGURATION, &vconf);
				BC_ASSERT_EQUAL(vconf.vsize.width * vconf.vsize.height, width * height, int, "%d");

				end_call(marie, pauline);
			}
		} else {
			BC_PASS("Cannot find provided video codec");
		}
	} else {
		BC_PASS("Test requires at least a dual core");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_expected_size_for_specified_bandwidth(
    int bandwidth, int width, int height, const char *resolution, const char *video_codec) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	OrtpNetworkSimulatorParams simparams = {0};
	int available_cpu = 0;
#ifdef _WIN32 /*fixme to be tested*/
	SYSTEM_INFO sysinfo;
	GetNativeSystemInfo(&sysinfo);
	available_cpu = sysinfo.dwNumberOfProcessors;
#elif __APPLE__ || __linux__
	available_cpu = sysconf(_SC_NPROCESSORS_CONF); /*check the number of processors configured, not just the one that
	                                                  are currently active.*/
#else
#warning "There is no code that detects the number of CPU for this platform."
	available_cpu = (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc));
#endif

	if (width * height >= 1280 * 720 && strcasecmp("vp8", video_codec) == 0) {
		/*required cpu for this test is 8
		 * https://gitlab.linphone.org/BC/public/mediastreamer2/-/blob/master/src/videofilters/vp8.c#L61 */
		if (available_cpu >= 8) {
			ms_factory_set_cpu_count(linphone_core_get_ms_factory(marie->lc), 8);
		} else {
			bctbx_error("Cannot execute this test with only [%i] cpus, requires 8", available_cpu);
			BC_PASS("Test requires at least an octo core");
			linphone_core_manager_destroy(marie);
			linphone_core_manager_destroy(pauline);
			return;
		}
	}

	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) >= 2) {
		if (linphone_core_find_payload_type(marie->lc, video_codec, -1, -1) != NULL) {
			linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(marie->lc, TRUE);
			linphone_core_enable_video_display(marie->lc, TRUE);
			linphone_core_enable_video_capture(pauline->lc, TRUE);
			linphone_core_enable_video_display(pauline->lc, TRUE);

			disable_all_video_codecs_except_one(marie->lc, video_codec);

			LinphoneVideoActivationPolicy *vpol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
			linphone_core_set_video_activation_policy(marie->lc, vpol);
			linphone_core_set_video_activation_policy(pauline->lc, vpol);
			linphone_video_activation_policy_unref(vpol);

			linphone_core_set_preferred_video_definition_by_name(marie->lc, resolution);

			simparams.mode = OrtpNetworkSimulatorOutbound;
			simparams.enabled = TRUE;
			simparams.max_bandwidth = (float)bandwidth;
			simparams.max_buffer_size = bandwidth;
			simparams.latency = 60;
			linphone_core_set_network_simulator_params(marie->lc, &simparams);

			if (BC_ASSERT_TRUE(call(marie, pauline))) {
				LinphoneCall *call = linphone_core_get_current_call(marie->lc);
				VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
				MSVideoConfiguration vconf = {0};
				LinphoneCallStats *stats;
				uint64_t start_time = bctbx_get_cur_time_ms();
				uint64_t elapsed;
				uint64_t max_time = 55000;

				while ((elapsed = bctbx_get_cur_time_ms() - start_time) < max_time) {
					wait_for_until(marie->lc, pauline->lc, NULL, 1, 1000);
					stats = linphone_call_get_video_stats(call);
					/* wait for target video size to be reached, no matter the number of TMMBR received */
					ms_filter_call_method(vstream->ms.encoder, MS_VIDEO_ENCODER_GET_CONFIGURATION, &vconf);
					if (vconf.vsize.width * vconf.vsize.height >= width * height &&
					    marie->stat.last_tmmbr_value_received >= 1 &&
					    linphone_call_stats_get_upload_bandwidth(stats) >= 250) {
						linphone_call_stats_unref(stats);
						break;
					}
					linphone_call_stats_unref(stats);
				}

				/*assert that the size is the expected one*/
				BC_ASSERT_EQUAL(vconf.vsize.width * vconf.vsize.height, width * height, int, "%d");
				/* we should have reached it before expiration of max_time */
				BC_ASSERT_LOWER((unsigned int)elapsed, (unsigned int)max_time, int, "%u");
				stats = linphone_call_get_video_stats(call);
				/* Upload bandwidth should be above 250 kbit/s all the time */
				BC_ASSERT_GREATER((int)linphone_call_stats_get_upload_bandwidth(stats), 250, int, "%i");
				/* At least a TMMBR should have been received, but possibly more */
				BC_ASSERT_GREATER((int)marie->stat.last_tmmbr_value_received, 1, int, "%i");

				linphone_call_stats_unref(stats);
				end_call(marie, pauline);
			}
		} else {
			BC_PASS("Cannot find provided video codec");
		}
	} else {
		BC_PASS("Test requires at least a dual core");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_expected_size_for_low_bandwith_vp8(void) {
	video_call_expected_size_for_specified_bandwidth_with_congestion(200000, 320, 240, "vga", "vp8");
}

static void video_call_expected_size_for_regular_bandwith_vp8(void) {
	video_call_expected_size_for_specified_bandwidth(550000, 640, 480, "vga", "vp8");
}

static void video_call_expected_size_for_high_bandwith_vp8(void) {
	video_call_expected_size_for_specified_bandwidth(5000000, 1280, 720, "vga", "vp8");
}

static void video_call_expected_size_for_low_bandwith_h264(void) {
	video_call_expected_size_for_specified_bandwidth_with_congestion(200000, 320, 240, "vga", "h264");
}

static void video_call_expected_size_for_regular_bandwith_h264(void) {
	video_call_expected_size_for_specified_bandwidth(500000, 640, 480, "vga", "h264");
}

static void video_call_expected_size_for_high_bandwith_h264(void) {
#if TARGET_OS_IPHONE == 1
	video_call_expected_size_for_specified_bandwidth(5000000, 1280, 960, "vga", "h264");
#else
	video_call_expected_size_for_specified_bandwidth(5000000, 1600, 1200, "vga", "h264");
#endif
}

static void video_call_expected_size_for_low_bandwith_h265(void) {
#if defined(__ANDROID__) || (TARGET_OS_IPHONE == 1) || defined(__arm__) || defined(_M_ARM)
	video_call_expected_size_for_specified_bandwidth_with_congestion(200000, 320, 240, "vga", "h265");
#else
	BC_PASS("H265 is not yet implemented on platforms other than mobile");
#endif
}

static void video_call_expected_size_for_regular_bandwith_h265(void) {
#if defined(__ANDROID__) || (TARGET_OS_IPHONE == 1) || defined(__arm__) || defined(_M_ARM)
	video_call_expected_size_for_specified_bandwidth(500000, 640, 480, "vga", "h265");
#else
	BC_PASS("H265 is not yet implemented on platforms other than mobile");
#endif
}

static void video_call_expected_size_for_high_bandwith_h265(void) {
#if defined(__ANDROID__) || defined(__arm__) || defined(_M_ARM)
	video_call_expected_size_for_specified_bandwidth(5000000, 1600, 1200, "vga", "h265");
#elif TARGET_OS_IPHONE == 1
	video_call_expected_size_for_specified_bandwidth(5000000, 1280, 960, "vga", "h265");
#else
	BC_PASS("H265 is not yet implemented on platforms other than mobile");
#endif
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

static void on_nack_alert(LinphoneCore *core, LinphoneAlert *alert) {

	BC_ASSERT_PTR_NOT_NULL(alert);
	int *count = (int *)linphone_core_cbs_get_user_data(linphone_core_get_current_callbacks(core));
	LinphoneAlertType type = linphone_alert_get_type(alert);

	if (type == LinphoneAlertQoSRetransmissionFailures) {
		(*count)++;
		const LinphoneDictionary *props = linphone_alert_get_informations(alert);
		float indicator = linphone_dictionary_get_float(props, "nack-performance");
		BC_ASSERT_TRUE(indicator > 0.0f);
	}
}

/*
 * This test simulates a video call with a high loss rate (50%). Without NACK and retransmission on NACK, it can't pass
 * because no I-frames can have a chance to pass. So, if the test passes, it means that retransmissions on NACK could
 * have their beneficial effect.
 */
static void call_with_retransmissions_on_nack(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie, *call_pauline;
	OrtpNetworkSimulatorParams params = {0};
	bool_t call_ok;
	int nack_alert_count = 0;
	params.enabled = TRUE;
	params.loss_rate = 50;
	params.consecutive_loss_probability = 0.2f;
	params.mode = OrtpNetworkSimulatorOutbound;
	params.latency = 500;

	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);

	linphone_core_set_network_simulator_params(marie->lc, &params);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_fb_generic_nack_enabled", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_fb_generic_nack_enabled", 1);
	linphone_core_enable_retransmission_on_nack(marie->lc, TRUE);
	linphone_core_enable_retransmission_on_nack(pauline->lc, TRUE);

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	linphone_core_enable_alerts(pauline->lc, TRUE);
	linphone_config_set_float(linphone_core_get_config(pauline->lc), "alerts", "alerts::nack_threshold", 1.0f);
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_new_alert_triggered(cbs, on_nack_alert);
	linphone_core_add_callbacks(pauline->lc, cbs);
	linphone_core_cbs_set_user_data(cbs, &nack_alert_count);
	linphone_core_cbs_unref(cbs);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	/* a VGA key frame is rather big, it has few chances to pass with such a high loss rate. */
	linphone_core_set_preferred_video_definition_by_name(marie->lc, "vga");
	linphone_core_set_download_bandwidth(marie->lc, 350); /*to make enough room for rapid avpf feedback*/

	BC_ASSERT_TRUE(call_ok = call(marie, pauline));
	if (!call_ok) goto end;
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	call_marie = linphone_core_get_current_call(marie->lc);
	call_pauline = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(call_marie);
	BC_ASSERT_PTR_NOT_NULL(call_pauline);
	if (call_marie && call_pauline) {
		VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo);
		ortp_ev_dispatcher_connect(media_stream_get_event_dispatcher(&vstream->ms), ORTP_EVENT_RTCP_PACKET_RECEIVED,
		                           RTCP_RTPFB, (OrtpEvDispatcherCb)generic_nack_received, &marie->stat);

		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_marie)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_pauline)));

		liblinphone_tester_set_next_video_frame_decoded_cb(call_marie);
		liblinphone_tester_set_next_video_frame_decoded_cb(call_pauline);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded, 1));
	}

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_rtcp_generic_nack, 5, 8000));

	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_retransmissions_on_nack_with_congestion(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie, *call_pauline;
	OrtpNetworkSimulatorParams params = {0};
	bool_t call_ok;

	params.enabled = TRUE;
	params.loss_rate = 0;
	params.max_bandwidth = 200000;
	params.mode = OrtpNetworkSimulatorOutbound;
	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);

	linphone_core_set_network_simulator_params(marie->lc, &params);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_fb_generic_nack_enabled", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_fb_generic_nack_enabled", 1);
	linphone_core_enable_retransmission_on_nack(marie->lc, TRUE);
	linphone_core_enable_retransmission_on_nack(pauline->lc, TRUE);

	disable_all_video_codecs_except_one(marie->lc, "VP8");
	disable_all_video_codecs_except_one(pauline->lc, "VP8");

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	/* a VGA key frame is rather big, it has few chances to pass with such a high loss rate. */
	linphone_core_set_preferred_video_definition_by_name(marie->lc, "vga");
	linphone_core_set_download_bandwidth(marie->lc, 350); /*to make enough room for rapid avpf feedback*/

	BC_ASSERT_TRUE(call_ok = call(marie, pauline));
	if (!call_ok) goto end;
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	call_marie = linphone_core_get_current_call(marie->lc);
	call_pauline = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(call_marie);
	BC_ASSERT_PTR_NOT_NULL(call_pauline);
	if (call_marie && call_pauline) {
		VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call_marie, LinphoneStreamTypeVideo);
		ortp_ev_dispatcher_connect(media_stream_get_event_dispatcher(&vstream->ms), ORTP_EVENT_RTCP_PACKET_RECEIVED,
		                           RTCP_RTPFB, (OrtpEvDispatcherCb)generic_nack_received, &marie->stat);

		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_marie)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(call_pauline)));

		/* Wait for a received TMMBR for a specified bitrate interval, which means that congestion was detected. */
		BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 100000,
		                                       200000, 20000));
		ms_message("Congestion TMMBR received");
		marie->stat.number_of_rtcp_generic_nack = 0;
		/* Past this point, no NACK should be generated, so except a few ones in transit, almost no nack should be
		 * received. */
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 8000);
		ms_message("Number of generic NACK received by Marie: %i", marie->stat.number_of_rtcp_generic_nack);
		BC_ASSERT_LOWER(marie->stat.number_of_rtcp_generic_nack, 10, int, "%d");
		liblinphone_tester_set_next_video_frame_decoded_cb(call_marie);
		liblinphone_tester_set_next_video_frame_decoded_cb(call_pauline);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_IframeDecoded, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_IframeDecoded, 1));
	}
	end_call(pauline, marie);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_loss_resilience(bool_t with_avpf) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	OrtpNetworkSimulatorParams simparams = {0};

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_static_image_id);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_video_activation_policy_unref(vpol);

	linphone_core_enable_adaptive_rate_control(
	    marie->lc,
	    FALSE); /* We don't want adaptive rate control here, in order to not interfere with loss recovery algorithms*/

	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.max_bandwidth = 1000000;
	simparams.max_buffer_size = (int)simparams.max_bandwidth;
	simparams.latency = 60;
	simparams.loss_rate = 70;
	linphone_core_set_network_simulator_params(marie->lc, &simparams);

	linphone_core_set_preferred_video_definition_by_name(marie->lc, "vga");
	linphone_core_set_download_bandwidth(marie->lc, 350); /*to make enough room for rapid avpf feedback*/

	if (!with_avpf) {
		linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_fb_implicit_rtcp_fb", 0);
	} else {
		/* AVPF (with implicit mode) is the default. */
	}

	if (BC_ASSERT_TRUE(call(marie, pauline))) {
		/* Given the high loss rate, no video frame should be decoded. The initial I-frame is lost.*/
		liblinphone_tester_set_next_video_frame_decoded_cb(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_FALSE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded, 1));
		/* Now modify the network simulation to remove loss rate. We should quickly get a new i-frame thanks to RTCP
		 * feedback or SIP INFO.*/
		simparams.loss_rate = 0;
		linphone_core_set_network_simulator_params(marie->lc, &simparams);
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_IframeDecoded, 1, with_avpf ? 2000 : 6000));

		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_loss_resilience_with_implicit_avpf(void) {
	video_call_loss_resilience(TRUE);
}

static void video_call_loss_resilience_without_avpf(void) {
	video_call_loss_resilience(FALSE);
}

static void video_conference_with_thin_congestion_basic(LinphoneConferenceLayout layout) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	OrtpNetworkSimulatorParams simparams = {0};
	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.max_bandwidth = 1000000;
	simparams.max_buffer_size = (int)simparams.max_bandwidth;
	simparams.latency = 60;
	linphone_core_set_network_simulator_params(marie->lc, &simparams);
	linphone_core_set_network_simulator_params(pauline->lc, &simparams);
	linphone_core_set_network_simulator_params(laure->lc, &simparams);

	LinphoneCoreCbs *core_cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_created(core_cbs, call_created);
	linphone_core_add_callbacks(laure->lc, core_cbs);

	LinphoneConference *conf = NULL;
	LinphoneConferenceParams *conf_params = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		disable_all_video_codecs_except_one(c, "VP8");

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == TRUE);

		linphone_config_set_int(linphone_core_get_config(c), "rtp", "rtcp_mux", 1);
		linphone_core_set_default_conference_layout(c, layout);
	}

	linphone_video_activation_policy_unref(pol);

	const LinphoneCallParams *negotiated_call_params = NULL;
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_set_conference_video_layout(marie_call_params, layout);
	linphone_call_params_enable_video(pauline_call_params, TRUE);
	linphone_call_params_enable_video(laure_call_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	linphone_call_params_unref(laure_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(marie_call_params);

	const char *subject = "Weekly team meeting";
	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_set_subject(conf_params, subject);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, conf, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 4, 10000));

	wait_for_list_interval(lcs, &laure->stat.last_tmmbr_value_received, 150000, 400000, 50000);

	simparams.max_bandwidth = 300000;
	linphone_core_set_network_simulator_params(pauline->lc, &simparams);
	wait_for_list_interval(lcs, &laure->stat.last_tmmbr_value_received, 20000, 40000, 30000);
	wait_for_list_interval(lcs, &laure->stat.last_tmmbr_value_received, 40000, 60000, 50000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	if (conf) {
		linphone_conference_unref(conf);
	}
	linphone_core_cbs_unref(core_cbs);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void video_conference_with_thin_congestion_layout_grid(void) {
	video_conference_with_thin_congestion_basic(LinphoneConferenceLayoutGrid);
}

static void video_conference_with_thin_congestion_layout_active_speaker(void) {
	video_conference_with_thin_congestion_basic(LinphoneConferenceLayoutActiveSpeaker);
}

static test_t call_video_quality_tests[] = {
    TEST_NO_TAG("Video call with thin congestion", video_call_with_thin_congestion),
    TEST_NO_TAG("Video call with high bandwidth available", video_call_with_high_bandwidth_available),
    TEST_NO_TAG("Video call expected FPS for low bandwidth", video_call_expected_fps_for_low_bandwidth),
    TEST_NO_TAG("Video call expected FPS for regular bandwidth", video_call_expected_fps_for_regular_bandwidth),
    TEST_NO_TAG("Video call expected FPS for high bandwidth", video_call_expected_fps_for_high_bandwidth),
    TEST_NO_TAG("Video call expected size for low bandwidth (VP8)", video_call_expected_size_for_low_bandwith_vp8),
    TEST_NO_TAG("Video call expected size for regular bandwidth (VP8)",
                video_call_expected_size_for_regular_bandwith_vp8),
    TEST_NO_TAG("Video call expected size for high bandwidth (VP8)", video_call_expected_size_for_high_bandwith_vp8),
    TEST_NO_TAG("Video call expected size for low bandwidth (H264)", video_call_expected_size_for_low_bandwith_h264),
    TEST_NO_TAG("Video call expected size for regular bandwidth (H264)",
                video_call_expected_size_for_regular_bandwith_h264),
    TEST_NO_TAG("Video call expected size for high bandwidth (H264)", video_call_expected_size_for_high_bandwith_h264),
    TEST_NO_TAG("Video call expected size for low bandwidth (H265)", video_call_expected_size_for_low_bandwith_h265),
    TEST_NO_TAG("Video call expected size for regular bandwidth (H265)",
                video_call_expected_size_for_regular_bandwith_h265),
    TEST_NO_TAG("Video call expected size for high bandwidth (H265)", video_call_expected_size_for_high_bandwith_h265),
    TEST_NO_TAG("Video call with explicit bandwidth limit", video_call_with_explicit_bandwidth_limit),
    TEST_NO_TAG("Video call with explicit bandwidth limit for the video stream",
                video_call_with_explicit_bandwidth_limit_for_stream),
    TEST_NO_TAG("Video call with retransmission on nack", call_with_retransmissions_on_nack),
    TEST_NO_TAG("Video call with retransmission on nack with congestion",
                call_with_retransmissions_on_nack_with_congestion),
    TEST_NO_TAG("Video loss rate resilience with implicit AVPF", video_call_loss_resilience_with_implicit_avpf),
    TEST_NO_TAG("Video loss rate resilience without AVPF", video_call_loss_resilience_without_avpf),
    TEST_NO_TAG("Video conference (active speaker) with thin congestion",
                video_conference_with_thin_congestion_layout_active_speaker),
    TEST_NO_TAG("Video conference (grid) with thin congestion", video_conference_with_thin_congestion_layout_grid)};

test_suite_t call_video_quality_test_suite = {"Video Call quality",
                                              NULL,
                                              NULL,
                                              liblinphone_tester_before_each,
                                              liblinphone_tester_after_each,
                                              sizeof(call_video_quality_tests) / sizeof(call_video_quality_tests[0]),
                                              call_video_quality_tests,
                                              0,
                                              8 /* cpu weight - this suite is very cpu consuming */};

#endif // ifdef VIDEO_ENABLED
