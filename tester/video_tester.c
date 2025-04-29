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

#ifdef VIDEO_ENABLED

#include "liblinphone_tester.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "tester_utils.h"
#include <mediastreamer2/msqrcodereader.h>

static void enable_disable_camera_after_camera_switches(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const char *currentCamId = (char *)linphone_core_get_video_device(marie->lc);
	const char **cameras = linphone_core_get_video_devices(marie->lc);
	const char *newCamId = NULL;
	int i;

	video_call_base_2(marie, pauline, FALSE, LinphoneMediaEncryptionNone, TRUE, TRUE);

	for (i = 0; cameras[i] != NULL; ++i) {
		if (strcmp(cameras[i], currentCamId) != 0) {
			newCamId = cameras[i];
			break;
		}
	}

	if (newCamId) {
		LinphoneCall *call = linphone_core_get_current_call(marie->lc);
		ms_message("Switching from [%s] to [%s]", currentCamId, newCamId);
		linphone_core_set_video_device(marie->lc, newCamId);
		BC_ASSERT_STRING_EQUAL(newCamId, ms_web_cam_get_string_id(_linphone_call_get_video_device(call)));
		linphone_call_enable_camera(call, FALSE);
		linphone_core_iterate(marie->lc);
		linphone_call_enable_camera(call, TRUE);
		BC_ASSERT_STRING_EQUAL(newCamId, ms_web_cam_get_string_id(_linphone_call_get_video_device(call)));
	}

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void snapshot_taken_cb(BCTBX_UNUSED(LinphoneCore *lc), const char *result) {
	char *filename = bc_tester_file("test_snapshot.jpeg");
	BC_ASSERT_STRING_EQUAL(filename, result);
	ms_free(filename);
}

static void camera_switches_while_only_preview(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	const char *camId = liblinphone_tester_mire_id;
	float fps = 0.0f;
	MSWebCam *cam =
	    ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(marie->lc)), camId);

	if (cam == NULL) {
		MSWebCamDesc *desc = ms_mire_webcam_desc_get();
		if (desc) {
			cam = ms_web_cam_new(desc);
			ms_web_cam_manager_add_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(marie->lc)), cam);
		}
	}
	linphone_core_set_video_device(marie->lc, camId);
	linphone_core_iterate(marie->lc);

	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(marie->lc);
	linphone_core_cbs_set_snapshot_taken(cbs, snapshot_taken_cb);

	linphone_core_enable_video_preview(marie->lc, TRUE);
	linphone_core_iterate(marie->lc); // Let time to the core to set new values

	char *filename = bc_tester_file("test_snapshot.jpeg");
	remove(filename);
	linphone_core_take_preview_snapshot(marie->lc, filename);
	linphone_core_iterate(marie->lc);
	linphone_core_iterate(marie->lc);

	VideoStream *vs = (VideoStream *)linphone_core_get_preview_stream(marie->lc);
	if (BC_ASSERT_PTR_NOT_NULL(vs) && BC_ASSERT_PTR_NOT_NULL(vs->source)) {
		ms_filter_call_method(vs->source, MS_FILTER_SET_FPS, (void *)&fps); // Simulate camera deficiency
		BC_ASSERT_TRUE(vs->cam == cam);
		wait_for_until(marie->lc, NULL, NULL, 0, 6000);
		BC_ASSERT_TRUE(vs->cam != cam);
	}

	remove(filename);
	ms_free(filename);
	linphone_core_manager_destroy(marie);
}

typedef struct struct_qrcode_callback_data {
	int qrcode_found;
	char *text;
} qrcode_callback_data;

static void qrcode_found_cb(LinphoneCore *lc, const char *result) {
	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(lc);
	qrcode_callback_data *found = (qrcode_callback_data *)linphone_core_cbs_get_user_data(cbs);
	found->qrcode_found = TRUE;
	if (result) {
		if (found->text) ms_free(found->text);
		found->text = ms_strdup(result);
	}
}

static void qrcode_reset_cb(qrcode_callback_data *qrcode_cb_data) {
	qrcode_cb_data->qrcode_found = FALSE;
	if (qrcode_cb_data->text) ms_free(qrcode_cb_data->text);
	qrcode_cb_data->text = NULL;
}

typedef struct struct_image_rect {
	int x;
	int y;
	int w;
	int h;
} image_rect;

static void _decode_qrcode(const char *image_path, image_rect *rect, bool_t image_from_res) {
	qrcode_callback_data qrcode_data;
	char *qrcode_image;
	LinphoneCoreManager *lcm = NULL;
	MSFactory *factory = NULL;
	factory = ms_factory_new_with_voip();
	if (!ms_factory_lookup_filter_by_name(factory, "MSQRCodeReader")) {
		ms_error("QRCode support is not built-in");
		goto end;
	}

	lcm = linphone_core_manager_create("empty_rc");
	LinphoneCoreCbs *cbs = NULL;
	qrcode_data.qrcode_found = FALSE;
	qrcode_data.text = NULL;
	linphone_core_manager_start(lcm, FALSE);
	if (image_from_res) qrcode_image = bc_tester_res(image_path);
	else qrcode_image = bctbx_strdup(image_path);

	linphone_core_set_video_device(lcm->lc, liblinphone_tester_static_image_id);
	linphone_core_set_static_picture(lcm->lc, qrcode_image);

	linphone_core_enable_qrcode_video_preview(lcm->lc, TRUE);
	cbs = linphone_core_get_current_callbacks(lcm->lc);
	linphone_core_cbs_set_qrcode_found(cbs, qrcode_found_cb);
	linphone_core_cbs_set_user_data(cbs, &qrcode_data);
	if (rect) {
		linphone_core_set_qrcode_decode_rect(lcm->lc, rect->x, rect->y, rect->w, rect->h);
	}
	linphone_core_enable_video_preview(lcm->lc, TRUE);

	BC_ASSERT_TRUE(wait_for_until(lcm->lc, NULL, &qrcode_data.qrcode_found, TRUE, 3000));
	if (qrcode_data.qrcode_found) {
		if (BC_ASSERT_PTR_NOT_NULL(qrcode_data.text)) {
			ms_message("QRCode decode: %s", qrcode_data.text);
			BC_ASSERT_STRING_EQUAL(qrcode_data.text, "https://www.linphone.org/");
		}
	}

	if (qrcode_data.text) ms_free(qrcode_data.text);
	if (qrcode_image) ms_free(qrcode_image);

	linphone_core_enable_video_preview(lcm->lc, FALSE);
	linphone_core_manager_destroy(lcm);
end:
	ms_factory_destroy(factory);
}

static void decode_qrcode_from_image(void) {
	_decode_qrcode("images/linphonesiteqr.jpg", NULL, TRUE);
}

static void decode_qrcode_from_zone(void) {
	image_rect rect;
	rect.x = 332;
	rect.y = 470;
	rect.w = 268;
	rect.h = 262;
	_decode_qrcode("images/linphonesiteqr_captured.jpg", &rect, TRUE);
}

static void decode_qrcode_from_image_when_enabled(LinphoneCore *lc,
                                                  char *image_path,
                                                  char *expected_qrcode_text,
                                                  qrcode_callback_data *qrcode_data,
                                                  bool_t enabled_expected,
                                                  bool_t QRCode_found_expected) {
	linphone_core_set_video_device(lc, liblinphone_tester_static_image_id);
	linphone_core_set_static_picture(lc, image_path);
	qrcode_reset_cb(qrcode_data);

	if (enabled_expected) BC_ASSERT_TRUE(linphone_core_qrcode_video_preview_enabled(lc));
	else BC_ASSERT_FALSE(linphone_core_qrcode_video_preview_enabled(lc));
	if (QRCode_found_expected) {
		BC_ASSERT_TRUE(wait_for_until(lc, NULL, &qrcode_data->qrcode_found, TRUE, 3000));
	} else {
		BC_ASSERT_FALSE(wait_for_until(lc, NULL, &qrcode_data->qrcode_found, TRUE, 3000));
	}
	if (qrcode_data->qrcode_found) {
		if (enabled_expected && QRCode_found_expected) {
			if (BC_ASSERT_PTR_NOT_NULL(qrcode_data->text)) {
				ms_message("QRCode decode: %s", qrcode_data->text);
				BC_ASSERT_STRING_EQUAL(qrcode_data->text, expected_qrcode_text);
			}
		}
	}
}

static void decode_several_qrcodes(void) {
	qrcode_callback_data qrcode_data;
	char *qrcode_image = bc_tester_res("images/linphonesiteqr.jpg");
	char *no_qrcode_image = bc_tester_res("images/nowebcamCIF.jpg");
	char *qrcode_text = "https://www.linphone.org/";
	LinphoneCoreManager *lcm = NULL;
	MSFactory *factory = NULL;
	factory = ms_factory_new_with_voip();
	if (!ms_factory_lookup_filter_by_name(factory, "MSQRCodeReader")) {
		ms_error("QRCode support is not built-in");
		goto end;
	}

	lcm = linphone_core_manager_create("empty_rc");
	linphone_core_manager_start(lcm, FALSE);

	linphone_core_set_video_device(lcm->lc, liblinphone_tester_static_image_id);
	linphone_core_set_static_picture(lcm->lc, qrcode_image);
	linphone_core_enable_video_preview(lcm->lc, TRUE);

	linphone_core_enable_qrcode_video_preview(lcm->lc, TRUE);
	LinphoneCoreCbs *cbs = NULL;
	qrcode_data.qrcode_found = FALSE;
	qrcode_data.text = NULL;
	cbs = linphone_core_get_current_callbacks(lcm->lc);
	linphone_core_cbs_set_qrcode_found(cbs, qrcode_found_cb);
	linphone_core_cbs_set_user_data(cbs, &qrcode_data);

	// scan QRCode
	decode_qrcode_from_image_when_enabled(lcm->lc, qrcode_image, qrcode_text, &qrcode_data, TRUE, TRUE);

	// cannot scan again because the delay is too short
	qrcode_reset_cb(&qrcode_data);
	BC_ASSERT_FALSE(wait_for_until(lcm->lc, NULL, &qrcode_data.qrcode_found, TRUE, 1000));

	// no QRCode to scan
	decode_qrcode_from_image_when_enabled(lcm->lc, no_qrcode_image, NULL, &qrcode_data, TRUE, FALSE);

	// scan QR code
	decode_qrcode_from_image_when_enabled(lcm->lc, qrcode_image, qrcode_text, &qrcode_data, TRUE, TRUE);

	// disable QRCode search, do no scan current QRCode
	linphone_core_enable_qrcode_video_preview(lcm->lc, FALSE);
	wait_for_until(lcm->lc, NULL, NULL, 0, 1000);
	decode_qrcode_from_image_when_enabled(lcm->lc, qrcode_image, NULL, &qrcode_data, FALSE, FALSE);

	// enable QRCode search, scan QRCode
	linphone_core_enable_qrcode_video_preview(lcm->lc, TRUE);
	decode_qrcode_from_image_when_enabled(lcm->lc, qrcode_image, qrcode_text, &qrcode_data, TRUE, TRUE);

	// enable QRCode search, scan QRCode again
	decode_qrcode_from_image_when_enabled(lcm->lc, qrcode_image, qrcode_text, &qrcode_data, TRUE, TRUE);

	if (qrcode_data.text) ms_free(qrcode_data.text);
	if (qrcode_image) ms_free(qrcode_image);
	if (no_qrcode_image) ms_free(no_qrcode_image);
	linphone_core_enable_video_preview(lcm->lc, FALSE);
	linphone_core_manager_destroy(lcm);
end:
	ms_factory_destroy(factory);
}

#if defined(QRCODE_ENABLED) && defined(JPEG_ENABLED)
static void encode_qrcode(void) {
	unsigned int w = 200;
	unsigned int h = 150;
	LinphoneFactory *factory = linphone_factory_get();
	char *file_path = NULL;

	file_path = bc_tester_file("video_tester_qrcode.jpg");

	int error = linphone_factory_write_qrcode_file(factory, file_path, "https://www.linphone.org/", w, h, 0);

	if (error != 0) {
		BC_ASSERT_EQUAL(error, 0, int, "%i");
		goto end;
	}

	// Decode the encoded qrcode from file
	_decode_qrcode(file_path, NULL, FALSE);
end:
	if (file_path) free(file_path);
}

#endif

static void preview_memory(const char *display_filter) {
	LinphoneCoreManager *marie = linphone_core_manager_new("empty_rc");
	bool_t ok = TRUE;
	linphone_core_set_video_display_filter(marie->lc, display_filter);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_set_native_preview_window_id(marie->lc, LINPHONE_VIDEO_DISPLAY_AUTO);
	linphone_core_set_native_video_window_id(marie->lc, LINPHONE_VIDEO_DISPLAY_AUTO);
	uint64_t memory_ref = 0;
	int excessCount = 0;
	for (int count = 0; ok && count < 50; ++count) {
		if (count == 3) { // Ignore first allocations to ignore preallocations/caches
			memory_ref = bc_tester_get_memory_consumption();
			ms_message("Memory consumption first reference: %lld KB", (long long)memory_ref / 1024);
		} else if (count > 3) { // Bad if more than 10% of memory
			// end = !BC_ASSERT_TRUE(bc_tester_get_memory_consumption() <= 1.1 * memory_ref);
			uint64_t current_memory = bc_tester_get_memory_consumption();
			ok = (current_memory <= 1.1 * memory_ref);
			if (!ok) {
				ms_warning("Excess memory consumption : %lld KB / %lld KB [count=%d]", (long long)current_memory / 1024,
				           (long long)memory_ref / 1024, count);
				if (++excessCount <= 3) { // Let 3 attempts on growing memory.
					memory_ref = current_memory;
					ok = TRUE;
				} else {
					BC_ASSERT_TRUE(current_memory <= 1.1 * memory_ref); // Make assert
				}
			}
		}
		ms_message("Starting preview");
		linphone_core_enable_video_preview(marie->lc, TRUE);
		wait_for_until(marie->lc, NULL, NULL, 0, 200); // Let some time to open and to print something.
		ms_message("Stopping preview");
		linphone_core_enable_video_preview(marie->lc, FALSE);
		wait_for_until(marie->lc, NULL, NULL, 0, 50); // Closing window
	}
	linphone_core_manager_destroy(marie);
}

static void preview_memory_default(void) {
	preview_memory(ms_factory_get_default_video_renderer(NULL));
}

static void preview_memory_msogl(void) {
#if !defined(__ANDROID__) && !defined(__APPLE__)
	if (strcmp(ms_factory_get_default_video_renderer(NULL), "MSOGL") != 0) preview_memory("MSOGL");
#endif
}

static test_t video_tests[] = {
    TEST_NO_TAG("Enable/disable camera after camera switches", enable_disable_camera_after_camera_switches),
    TEST_ONE_TAG("Decode QRCode from image", decode_qrcode_from_image, "QRCode"),
    TEST_ONE_TAG("Decode QRCode from zone", decode_qrcode_from_zone, "QRCode"),
    TEST_ONE_TAG("Decode several QRCodes", decode_several_qrcodes, "QRCode"),
#if defined(QRCODE_ENABLED) && defined(JPEG_ENABLED)
    TEST_ONE_TAG("Encode QRCode", encode_qrcode, "QRCode"),
#endif
    TEST_NO_TAG("Fallback camera while preview is only enabled", camera_switches_while_only_preview),

    TEST_ONE_TAG("Preview memory default", preview_memory_default, "skip"),
    TEST_ONE_TAG("Preview memory MSOGL", preview_memory_msogl, "skip")};

test_suite_t video_test_suite = {"Video",
                                 NULL,
                                 NULL,
                                 liblinphone_tester_before_each,
                                 liblinphone_tester_after_each,
                                 sizeof(video_tests) / sizeof(video_tests[0]),
                                 video_tests,
                                 0};

#endif // ifdef VIDEO_ENABLED
