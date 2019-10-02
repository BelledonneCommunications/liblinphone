/*
    liblinphone_tester - liblinphone test suite
    Copyright (C) 2013  Belledonne Communications SARL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "liblinphone_tester.h"
#include "tester_utils.h"
#include <mediastreamer2/mediastream.h>

static void eof_callback(LinphonePlayer *player) {
	LinphonePlayerCbs *cbs = linphone_player_get_callbacks(player);
	int *eof = (int *)linphone_player_cbs_get_user_data(cbs);
	*eof = 1;
}

static void play_file(const char *filename, bool_t supported_format, const char *audio_mime, const char *video_mime) {
	LinphoneCoreManager *lc_manager = linphone_core_manager_new("marie_rc");
	LinphonePlayer *player;
	LinphonePlayerCbs *cbs;
	int res;
	int eof = 0;

	bool_t audio_codec_supported = (audio_mime && ms_factory_get_decoder(linphone_core_get_ms_factory((void *)lc_manager->lc), audio_mime));
	bool_t video_codec_supported = (video_mime && ms_factory_get_decoder(linphone_core_get_ms_factory((void *)lc_manager->lc), video_mime));
	int expected_res = (supported_format && (audio_codec_supported || video_codec_supported)) ? 0 : -1;

	player = linphone_core_create_local_player(lc_manager->lc, linphone_core_get_ringer_device(lc_manager->lc),
						   linphone_core_get_default_video_display_filter(lc_manager->lc), 0);
	BC_ASSERT_PTR_NOT_NULL(player);
	if(player == NULL) goto fail;

	cbs = linphone_player_get_callbacks(player);
	linphone_player_cbs_set_eof_reached(cbs, eof_callback);
	linphone_player_cbs_set_user_data(cbs, &eof);
	res = linphone_player_open(player, filename);
	BC_ASSERT_EQUAL(res, expected_res, int, "%d");

	if(res == -1) goto fail;

	res = linphone_player_start(player);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	if(res == -1) goto fail;

	BC_ASSERT_TRUE(wait_for_until(lc_manager->lc, NULL, &eof, 1, (int)(linphone_player_get_duration(player) * 1.05)));

	linphone_player_close(player);

	fail:
	if(player) linphone_player_unref(player);
	if(lc_manager) linphone_core_manager_destroy(lc_manager);
}

static void sintel_trailer_opus_h264_test(void) {
	char *filename = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	const char *audio_mime = "opus";
	const char *video_mime = "H264";
	play_file(filename, linphone_local_player_matroska_supported(), audio_mime, video_mime);
	ms_free(filename);
}

static void sintel_trailer_pcmu_h264_test(void) {
	char *filename = bc_tester_res("sounds/sintel_trailer_pcmu_h264.mkv");
	const char *audio_mime = "pcmu";
	const char *video_mime = "H264";
	play_file(filename, linphone_local_player_matroska_supported(), audio_mime, video_mime);
	ms_free(filename);
}

static void sintel_trailer_opus_vp8_test(void) {
	char *filename = bc_tester_res("sounds/sintel_trailer_opus_vp8.mkv");
	const char *audio_mime = "opus";
	const char *video_mime = "VP8";
	play_file(filename, linphone_local_player_matroska_supported(), audio_mime, video_mime);
	ms_free(filename);
}

test_t player_tests[] = {
	TEST_NO_TAG("Sintel trailer opus/h264", sintel_trailer_opus_h264_test),
	TEST_NO_TAG("Sintel trailer pcmu/h264", sintel_trailer_pcmu_h264_test),
	TEST_NO_TAG("Sintel trailer opus/VP8", sintel_trailer_opus_vp8_test)
};

test_suite_t player_test_suite = {"Player", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								  sizeof(player_tests) / sizeof(test_t), player_tests};
