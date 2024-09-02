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

#include "liblinphone_tester.h"
#include "tester_utils.h"
#include <mediastreamer2/mediastream.h>

static void eof_callback(LinphonePlayer *player) {
	LinphonePlayerCbs *cbs = linphone_player_get_current_callbacks(player);
	int *eof = (int *)linphone_player_cbs_get_user_data(cbs);
	*eof = 1;
}

static void play_file(const char *filename, bool_t supported_format, const char *audio_mime, const char *video_mime) {
	LinphoneCoreManager *lc_manager = linphone_core_manager_new("marie_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	int res;
	int eof = 0;

	bool_t audio_codec_supported =
	    (audio_mime && ms_factory_get_decoder(linphone_core_get_ms_factory((void *)lc_manager->lc), audio_mime));
	bool_t video_codec_supported =
	    (video_mime && ms_factory_get_decoder(linphone_core_get_ms_factory((void *)lc_manager->lc), video_mime));
	int expected_res = (supported_format && (audio_codec_supported || video_codec_supported)) ? 0 : -1;

	player = linphone_core_create_local_player(lc_manager->lc, linphone_core_get_ringer_device(lc_manager->lc),
	                                           linphone_core_get_default_video_display_filter(lc_manager->lc), 0);
	BC_ASSERT_PTR_NOT_NULL(player);
	if (player == NULL) goto fail;

	cbs = linphone_factory_create_player_cbs(linphone_factory_get());
	linphone_player_cbs_set_eof_reached(cbs, eof_callback);
	linphone_player_cbs_set_user_data(cbs, &eof);
	linphone_player_add_callbacks(player, cbs);
	res = linphone_player_open(player, filename);
	BC_ASSERT_EQUAL(res, expected_res, int, "%d");

	if (res == -1) goto fail;

	res = linphone_player_start(player);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	if (res == -1) goto fail;

	BC_ASSERT_TRUE(wait_for_until(lc_manager->lc, NULL, &eof, 1, (int)(linphone_player_get_duration(player) * 1.05)));

	linphone_player_close(player);

fail:
	if (cbs) linphone_player_cbs_unref(cbs);
	if (player) linphone_player_unref(player);
	if (lc_manager) linphone_core_manager_destroy(lc_manager);
}

static void wav_player_test(bool_t seek) {
	LinphoneCoreManager *lc_manager = linphone_core_manager_new("marie_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	int res;
	int eof = 0;
	int current_position;
	int duration;
	char *filename = bc_tester_res("sounds/hello8000.wav");

	player = linphone_core_create_local_player(lc_manager->lc, linphone_core_get_ringer_device(lc_manager->lc),
	                                           linphone_core_get_default_video_display_filter(lc_manager->lc), 0);
	BC_ASSERT_PTR_NOT_NULL(player);
	if (player == NULL) goto fail;

	cbs = linphone_factory_create_player_cbs(linphone_factory_get());
	linphone_player_cbs_set_eof_reached(cbs, eof_callback);
	linphone_player_cbs_set_user_data(cbs, &eof);
	linphone_player_add_callbacks(player, cbs);
	res = linphone_player_open(player, filename);
	BC_ASSERT_EQUAL(res, 0, int, "%d");

	duration = linphone_player_get_duration(player);
	BC_ASSERT_GREATER((int)duration, 20000, int, "%d");
	BC_ASSERT_LOWER((int)duration, 24000, int, "%d");

	if (res == -1) goto fail;

	res = linphone_player_start(player);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	if (res == -1) goto fail;

	wait_for_until(lc_manager->lc, NULL, NULL, 0, 2000);

	current_position = linphone_player_get_current_position(player);
	BC_ASSERT_GREATER((int)current_position, 1000, int, "%d");
	BC_ASSERT_LOWER((int)current_position, 3000, int, "%d");

	if (seek) {
		res = linphone_player_seek(player, 15000);
		BC_ASSERT_EQUAL(res, 0, int, "%d");
		if (res == -1) goto fail;

		BC_ASSERT_TRUE(wait_for_until(lc_manager->lc, NULL, &eof, 1, 8000));
	} else {
		BC_ASSERT_TRUE(
		    wait_for_until(lc_manager->lc, NULL, &eof, 1, (int)(linphone_player_get_duration(player) * 1.05)));
	}

	linphone_player_close(player);

fail:
	if (cbs) linphone_player_cbs_unref(cbs);
	if (player) linphone_player_unref(player);
	if (lc_manager) linphone_core_manager_destroy(lc_manager);
	bc_free(filename);
}

static void multimedia_player_test(const char *filename) {
	LinphoneCoreManager *lc_manager = linphone_core_manager_new("marie_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs = NULL;
	int res;
	int eof = 0;
	int current_position;
	int duration;
	char *filepath = bc_tester_res(filename);
	bool_t seek = TRUE;
	player = linphone_core_create_local_player(lc_manager->lc, linphone_core_get_ringer_device(lc_manager->lc),
	                                           linphone_core_get_default_video_display_filter(lc_manager->lc), 0);
	BC_ASSERT_PTR_NOT_NULL(player);
	if (player == NULL) goto fail;

	cbs = linphone_factory_create_player_cbs(linphone_factory_get());
	linphone_player_cbs_set_eof_reached(cbs, eof_callback);
	linphone_player_cbs_set_user_data(cbs, &eof);
	linphone_player_add_callbacks(player, cbs);
	res = linphone_player_open(player, filepath);
	BC_ASSERT_EQUAL(res, 0, int, "%d");

	duration = linphone_player_get_duration(player);
	BC_ASSERT_GREATER((int)duration, 11000, int, "%d");
	BC_ASSERT_LOWER((int)duration, 14000, int, "%d");

	if (res == -1) goto fail;

	res = linphone_player_start(player);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	if (res == -1) goto fail;

	wait_for_until(lc_manager->lc, NULL, NULL, 0, 2000);

	current_position = linphone_player_get_current_position(player);
	BC_ASSERT_GREATER((int)current_position, 1000, int, "%d");
	BC_ASSERT_LOWER((int)current_position, 3000, int, "%d");

	if (seek) {
		res = linphone_player_seek(player, 9000);
		BC_ASSERT_EQUAL(res, 0, int, "%d");
		if (res == -1) goto fail;

		BC_ASSERT_TRUE(wait_for_until(lc_manager->lc, NULL, &eof, 1, 4000));
	} else {
		BC_ASSERT_TRUE(
		    wait_for_until(lc_manager->lc, NULL, &eof, 1, (int)(linphone_player_get_duration(player) * 1.05)));
	}

	linphone_player_close(player);

fail:
	if (cbs) linphone_player_cbs_unref(cbs);
	if (player) linphone_player_unref(player);
	if (lc_manager) linphone_core_manager_destroy(lc_manager);
	bc_free(filepath);
}

static void mkv_player_test(void) {
	multimedia_player_test("sounds/recording.mkv");
}

static void smff_player_test(void) {
	multimedia_player_test("sounds/recording.smff");
}

static void wav_player_simple_test(void) {
	wav_player_test(FALSE);
}

static void wav_player_seeking_test(void) {
	wav_player_test(TRUE);
}

static void sintel_trailer_opus_h264_test(void) {
	char *filename = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	const char *audio_mime = "opus";
	const char *video_mime = "H264";
	play_file(filename, linphone_local_player_matroska_supported(), audio_mime, video_mime);
	bc_free(filename);
}

static void sintel_trailer_pcmu_h264_test(void) {
	char *filename = bc_tester_res("sounds/sintel_trailer_pcmu_h264.mkv");
	const char *audio_mime = "pcmu";
	const char *video_mime = "H264";
	play_file(filename, linphone_local_player_matroska_supported(), audio_mime, video_mime);
	bc_free(filename);
}

static void sintel_trailer_opus_vp8_test(void) {
	char *filename = bc_tester_res("sounds/sintel_trailer_opus_vp8.mkv");
	const char *audio_mime = "opus";
	const char *video_mime = "VP8";
	play_file(filename, linphone_local_player_matroska_supported(), audio_mime, video_mime);
	bc_free(filename);
}

static test_t player_tests[] = {TEST_NO_TAG("Wav file", wav_player_simple_test),
                                TEST_NO_TAG("Wav seeking", wav_player_seeking_test),
                                TEST_NO_TAG("Mkv seeking", mkv_player_test),
                                TEST_NO_TAG("SMFF seeking", smff_player_test),
                                TEST_NO_TAG("Sintel trailer opus/h264", sintel_trailer_opus_h264_test),
                                TEST_NO_TAG("Sintel trailer pcmu/h264", sintel_trailer_pcmu_h264_test),
                                TEST_NO_TAG("Sintel trailer opus/VP8", sintel_trailer_opus_vp8_test)};

test_suite_t player_test_suite = {"Player",
                                  NULL,
                                  NULL,
                                  liblinphone_tester_before_each,
                                  liblinphone_tester_after_each,
                                  sizeof(player_tests) / sizeof(test_t),
                                  player_tests,
                                  0};
