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


#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

#ifdef VIDEO_ENABLED

/*
 * This test simulates a network congestion in the video flow from marie to pauline.
 * The stream from pauline to marie is not under test.
 * It checks that a first TMMBR consecutive to congestion detection is received, and a second one after congestion resolution is received
 * a few seconds later.
 * The parameters used for the network simulator correspond to a "light congestion", which are the ones that translate into relatively
 * small packet losses, hence the more difficult to detect at first sight.
 *
**/
static void video_call_with_thin_congestion(void){
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneVideoPolicy pol = {0};
	OrtpNetworkSimulatorParams simparams = { 0 };

	linphone_core_set_video_device(marie->lc, "Mire: Mire (synthetic moving picture)");
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	pol.automatically_accept = TRUE;
	pol.automatically_initiate = TRUE;
	linphone_core_set_video_policy(marie->lc, &pol);
	linphone_core_set_video_policy(pauline->lc, &pol);

	/*set the video preset to custom so the video quality controller won't update the video size*/
	linphone_core_set_video_preset(marie->lc, "custom");
	linphone_core_set_preferred_video_size_by_name(marie->lc, "vga"); /*It will result in approxy 350kbit/s VP8 output*/

	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.max_bandwidth = 300000;  /*we limit to 300kbit/s*/
	simparams.max_buffer_size = (int)simparams.max_bandwidth;
	simparams.latency = 60;
	linphone_core_set_network_simulator_params(marie->lc, &simparams);

	if (BC_ASSERT_TRUE(call(marie, pauline))){
		LinphoneCall *call = linphone_core_get_current_call(pauline->lc);

		/*A congestion is expected, which will result in a TMMBR to be emitted by pauline*/
		/*wait for the TMMBR*/
		BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 170000, 280000, 15000));

		/*another tmmbr with a greater value is expected once the congestion is resolved*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 250000, 50000));
		BC_ASSERT_GREATER(linphone_call_get_current_quality(call), 4.f, float, "%f");

		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void on_tmmbr_received(LinphoneCall *call, int stream_index, int tmmbr) {
	if (stream_index == _linphone_call_get_main_video_stream_index(call)) {
		stats* stat = get_stats(linphone_call_get_core(call));
		stat->tmmbr_received_from_cb = tmmbr;
	}
}

static void call_created(LinphoneCore *lc, LinphoneCall *call) {
	LinphoneCallCbs *cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	linphone_call_cbs_set_tmmbr_received(cbs, on_tmmbr_received);
	linphone_call_add_callbacks(call, cbs);
	linphone_call_cbs_unref(cbs);
}

/*
 * This test simulates a higher bandwith available from marie than expected.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie received a TMMBR with the approximate value corresponding to the new bandwidth.
 *
**/
static void video_call_with_high_bandwidth_available(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneVideoPolicy pol = {0};
	OrtpNetworkSimulatorParams simparams = { 0 };
	LinphoneCoreCbs *core_cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	linphone_core_set_video_device(marie->lc, "Mire: Mire (synthetic moving picture)");
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	pol.automatically_accept = TRUE;
	pol.automatically_initiate = TRUE;
	linphone_core_set_video_policy(marie->lc, &pol);
	linphone_core_set_video_policy(pauline->lc, &pol);

	linphone_core_set_video_preset(marie->lc, "custom");
	linphone_core_set_preferred_video_size_by_name(marie->lc, "vga");

	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
	simparams.max_bandwidth = 1000000;
	simparams.max_buffer_size = (int)simparams.max_bandwidth;
	simparams.latency = 60;
	linphone_core_set_network_simulator_params(marie->lc, &simparams);

	linphone_core_cbs_set_call_created(core_cbs, call_created);
	linphone_core_add_callbacks(marie->lc, core_cbs);

	if (BC_ASSERT_TRUE(call(marie, pauline))){
		/*wait a little in order to have traffic*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, NULL, 5, 50000));

		BC_ASSERT_GREATER((float)marie->stat.last_tmmbr_value_received, 870000.f, float, "%f");
		BC_ASSERT_LOWER((float)marie->stat.last_tmmbr_value_received, 1150000.f, float, "%f");

		end_call(marie, pauline);
	}
	linphone_core_cbs_unref(core_cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_expected_fps_for_specified_bandwidth(int bandwidth, int fps, const char *resolution) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneVideoPolicy pol = {0};
	OrtpNetworkSimulatorParams simparams = { 0 };

	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) >= 2) {
		linphone_core_set_video_device(marie->lc, "Mire: Mire (synthetic moving picture)");
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);

		pol.automatically_accept = TRUE;
		pol.automatically_initiate = TRUE;
		linphone_core_set_video_policy(marie->lc, &pol);
		linphone_core_set_video_policy(pauline->lc, &pol);

		linphone_core_set_preferred_video_size_by_name(marie->lc, resolution);
		simparams.mode = OrtpNetworkSimulatorOutbound;
		simparams.enabled = TRUE;
		simparams.max_bandwidth = (float)bandwidth;
		simparams.max_buffer_size = bandwidth;
		simparams.latency = 60;

		linphone_core_set_network_simulator_params(marie->lc, &simparams);

		if (BC_ASSERT_TRUE(call(marie, pauline))){
			LinphoneCall *call = linphone_core_get_current_call(marie->lc);
			VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
			int count;
			/*wait some time until the target fps is reached. Indeed the bandwidth measurement may take several iterations to converge
			 to a value big enough to allow mediastreamer2 to switch to the high fps profile*/

			for (count = 0 ; count < 3; count++){
				/*wait for at least the first TMMBR to arrive*/
				BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, 10000));

				if ((int)vstream->configured_fps == fps){
					break;
				}else{
					/*target fps not reached yet, wait more time*/
					wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);
				}
			}
			BC_ASSERT_EQUAL((int)vstream->configured_fps, fps, int, "%d");
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
#if defined(__ANDROID__) || (TARGET_OS_IPHONE == 1) || defined(__arm__) || defined(_M_ARM)
	video_call_expected_fps_for_specified_bandwidth(100000, 10, "qvga");
#else
	video_call_expected_fps_for_specified_bandwidth(350000, 15, "vga");
#endif
}

/*
 * This test simulates a video call with a regular bandwidth that is between a given configuration.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie, after receiving a TMMBR, has her fps set to the expected given configuration.
 * This test requires at least a computer with 2 CPUs.
 *
**/
static void video_call_expected_fps_for_regular_bandwidth(void) {
#if defined(__ANDROID__) || (TARGET_OS_IPHONE == 1) || defined(__arm__) || defined(_M_ARM)
	video_call_expected_fps_for_specified_bandwidth(500000, 12, "vga");
#else
	video_call_expected_fps_for_specified_bandwidth(450000, 25, "vga");
#endif
}

/*
 * This test simulates a video call with a higher bandwidth than the bitrate_limit of the highest given configuration.
 * The stream from pauline to marie is not under test.
 * It checks that after a few seconds marie, after receiving a TMMBR, has her fps set to the highest given configuration.
 * This test requires at least a computer with 2 CPUs.
 *
**/
static void video_call_expected_fps_for_high_bandwidth(void) {
#if defined(__ANDROID__) || (TARGET_OS_IPHONE == 1) || defined(__arm__) || defined(_M_ARM)
	video_call_expected_fps_for_specified_bandwidth(400000, 12, "qcif");
#else
	video_call_expected_fps_for_specified_bandwidth(5000000, 30, "vga");
#endif
}

static void video_call_expected_size_for_specified_bandwidth_with_congestion(int bandwidth, int width, int height, const char *resolution, const char *video_codec) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneVideoPolicy pol = {0};
	OrtpNetworkSimulatorParams simparams = { 0 };

	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) >= 2) {
		if (linphone_core_find_payload_type(marie->lc, video_codec, -1, -1) != NULL) {
			linphone_core_set_video_device(marie->lc, "Mire: Mire (synthetic moving picture)");
			linphone_core_enable_video_capture(marie->lc, TRUE);
			linphone_core_enable_video_display(marie->lc, TRUE);
			linphone_core_enable_video_capture(pauline->lc, TRUE);
			linphone_core_enable_video_display(pauline->lc, TRUE);

			disable_all_video_codecs_except_one(marie->lc, video_codec);

			pol.automatically_accept = TRUE;
			pol.automatically_initiate = TRUE;
			linphone_core_set_video_policy(marie->lc, &pol);
			linphone_core_set_video_policy(pauline->lc, &pol);

			linphone_core_set_preferred_video_size_by_name(marie->lc, resolution);

			simparams.mode = OrtpNetworkSimulatorOutbound;
			simparams.enabled = TRUE;
			simparams.max_bandwidth = (float)bandwidth;
			simparams.max_buffer_size = bandwidth;
			simparams.latency = 60;
			linphone_core_set_network_simulator_params(marie->lc, &simparams);

			if (BC_ASSERT_TRUE(call(marie, pauline))){
				LinphoneCall *call = linphone_core_get_current_call(marie->lc);
				VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
				MSVideoConfiguration vconf;

				BC_ASSERT_TRUE(wait_for_until_interval(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, (int)(0.7*bandwidth), 50000));

				ms_filter_call_method(vstream->ms.encoder, MS_VIDEO_ENCODER_GET_CONFIGURATION, &vconf);
				BC_ASSERT_EQUAL(vconf.vsize.width*vconf.vsize.height, width*height, int, "%d");

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

static void video_call_expected_size_for_specified_bandwidth(int bandwidth, int width, int height, const char *resolution, const char *video_codec) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneVideoPolicy pol = {0};
	OrtpNetworkSimulatorParams simparams = { 0 };

	if (ms_factory_get_cpu_count(linphone_core_get_ms_factory(marie->lc)) >= 2) {
		if (linphone_core_find_payload_type(marie->lc, video_codec, -1, -1) != NULL) {
			linphone_core_set_video_device(marie->lc, "Mire: Mire (synthetic moving picture)");
			linphone_core_enable_video_capture(marie->lc, TRUE);
			linphone_core_enable_video_display(marie->lc, TRUE);
			linphone_core_enable_video_capture(pauline->lc, TRUE);
			linphone_core_enable_video_display(pauline->lc, TRUE);

			disable_all_video_codecs_except_one(marie->lc, video_codec);

			pol.automatically_accept = TRUE;
			pol.automatically_initiate = TRUE;
			linphone_core_set_video_policy(marie->lc, &pol);
			linphone_core_set_video_policy(pauline->lc, &pol);

			linphone_core_set_preferred_video_size_by_name(marie->lc, resolution);

			simparams.mode = OrtpNetworkSimulatorOutbound;
			simparams.enabled = TRUE;
			simparams.max_bandwidth = (float)bandwidth;
			simparams.max_buffer_size = bandwidth;
			simparams.latency = 60;
			linphone_core_set_network_simulator_params(marie->lc, &simparams);

			if (BC_ASSERT_TRUE(call(marie, pauline))){
				LinphoneCall *call = linphone_core_get_current_call(marie->lc);
				VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
				MSVideoConfiguration vconf;

				wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, 50000);

				marie->stat.last_tmmbr_value_received = 0;
				/* Wait until the last TMMBR was reached 10 second ago. */
				while (wait_for_until(marie->lc, pauline->lc, &marie->stat.last_tmmbr_value_received, 1, 10000)) {
					marie->stat.last_tmmbr_value_received = 0;
				}

				ms_filter_call_method(vstream->ms.encoder, MS_VIDEO_ENCODER_GET_CONFIGURATION, &vconf);

				BC_ASSERT_EQUAL(vconf.vsize.width*vconf.vsize.height, width*height, int, "%d");

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
	video_call_expected_size_for_specified_bandwidth(500000, 640, 480, "vga", "vp8");
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

static test_t call_video_quality_tests[] = {
	TEST_NO_TAG("Video call with thin congestion", video_call_with_thin_congestion),
	TEST_NO_TAG("Video call with high bandwidth available", video_call_with_high_bandwidth_available),
	TEST_NO_TAG("Video call expected FPS for low bandwidth", video_call_expected_fps_for_low_bandwidth),
	TEST_NO_TAG("Video call expected FPS for regular bandwidth", video_call_expected_fps_for_regular_bandwidth),
	TEST_NO_TAG("Video call expected FPS for high bandwidth", video_call_expected_fps_for_high_bandwidth),
	TEST_NO_TAG("Video call expected size for low bandwidth (VP8)", video_call_expected_size_for_low_bandwith_vp8),
	TEST_NO_TAG("Video call expected size for regular bandwidth (VP8)", video_call_expected_size_for_regular_bandwith_vp8),
	TEST_NO_TAG("Video call expected size for high bandwidth (VP8)", video_call_expected_size_for_high_bandwith_vp8),
	TEST_NO_TAG("Video call expected size for low bandwidth (H264)", video_call_expected_size_for_low_bandwith_h264),
	TEST_NO_TAG("Video call expected size for regular bandwidth (H264)", video_call_expected_size_for_regular_bandwith_h264),
	TEST_NO_TAG("Video call expected size for high bandwidth (H264)", video_call_expected_size_for_high_bandwith_h264),
	TEST_NO_TAG("Video call expected size for low bandwidth (H265)", video_call_expected_size_for_low_bandwith_h265),
	TEST_NO_TAG("Video call expected size for regular bandwidth (H265)", video_call_expected_size_for_regular_bandwith_h265),
	TEST_NO_TAG("Video call expected size for high bandwidth (H265)", video_call_expected_size_for_high_bandwith_h265),
};

test_suite_t call_video_quality_test_suite = {"Video Call quality", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_video_quality_tests) / sizeof(call_video_quality_tests[0]), call_video_quality_tests};

#endif // ifdef VIDEO_ENABLED
