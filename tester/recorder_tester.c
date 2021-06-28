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
#include <sys/stat.h>


#ifdef _WIN32
#	include <io.h>
#	ifndef R_OK
#		define R_OK 0x2
#	endif
#	ifndef W_OK
#		define W_OK 0x6
#	endif
#   ifndef F_OK
#       define F_OK 0x0
#   endif

#	ifndef S_IRUSR
#	define S_IRUSR S_IREAD
#	endif

#	ifndef S_IWUSR
#	define S_IWUSR S_IWRITE
#	endif

#	define open _open
#	define read _read
#	define write _write
#	define close _close
#	define access _access
#	define lseek _lseek
#else /*_WIN32*/

#	ifndef O_BINARY
#	define O_BINARY 0
#	endif

#endif /*!_WIN32*/


static void record_file(const char *filename, bool_t supported_format, const char *audio_mime, const char *video_mime, LinphoneRecorderFileFormat format) {
	LinphoneCoreManager *lc_manager = linphone_core_manager_create("marie_rc");
	LinphoneRecorder *recorder;
	int res = 0;
	bool_t res2 = FALSE;

	if (strcmp(video_mime, "") != 0) {
		linphone_core_set_video_device(lc_manager->lc, liblinphone_tester_mire_id);
		if(linphone_core_find_payload_type(lc_manager->lc, video_mime, -1, -1)) {
			disable_all_video_codecs_except_one(lc_manager->lc, video_mime);
		} else {
			ms_warning("call_recording(): the %s payload has not been found. Only sound will be recorded", video_mime);
		}
	}

	LinphoneRecorderParams *params = linphone_core_create_recorder_params(lc_manager->lc);
	linphone_recorder_params_set_webcam_name(params, linphone_core_get_video_device(lc_manager->lc));
	linphone_recorder_params_set_file_format(params, format);
	linphone_recorder_params_set_video_codec(params, video_mime);
	recorder = linphone_core_create_recorder(lc_manager->lc, params);
	linphone_recorder_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(recorder);
	if(recorder == NULL) goto fail;

	res = linphone_recorder_open(recorder, filename);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	if(res == -1) goto fail;

	LinphoneRecorderState state = linphone_recorder_get_state(recorder);
	res2 = state == LinphoneRecorderPaused;
	BC_ASSERT_TRUE(res2);
	if(!res2) goto fail;
	ms_message("We check if the recorder is in stand by, res2 = %d\n", res2);
	res = linphone_recorder_start(recorder);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	if(res == -1) goto fail;

	state = linphone_recorder_get_state(recorder);
	res2 = state == LinphoneRecorderRunning;
	BC_ASSERT_TRUE(res2);
	ms_message("We check if the recorder is running, res2 = %d\n", res2);
	if(!res2) goto fail;

	wait_for_until(lc_manager->lc, NULL, NULL, 0, 5000);
	
	int duration = linphone_recorder_get_duration(recorder);
	BC_ASSERT_GREATER(duration, 5000, int , "%d");

	LinphoneContent *content = linphone_recorder_create_content(recorder);
	BC_ASSERT_PTR_NULL(content);

	linphone_recorder_close(recorder);

	state = linphone_recorder_get_state(recorder);
	res2 = state == LinphoneRecorderClosed;
	BC_ASSERT_TRUE(res2);
	ms_message("We check if the recorder is closed, res2 = %d\n", res2);
	if(!res2) goto fail;

	duration = linphone_recorder_get_duration(recorder);
	content = linphone_recorder_create_content(recorder);
	BC_ASSERT_PTR_NOT_NULL(content);
	if (content != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_content_get_file_path(content), filename);
		BC_ASSERT_EQUAL(linphone_content_get_file_duration(content), duration, int, "%d");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "audio");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "wav");
		linphone_content_unref(content);
	}

	res = access(filename, F_OK | W_OK);
	BC_ASSERT_EQUAL(res, 0, int, "%d");
	ms_message("We check if the file exists, res = %d\n", res);
	if(res == 0) {
		struct stat st;
		res = stat(filename, &st);
		BC_ASSERT_EQUAL(res, 0, int, "%d");
		ms_message("We check if the file can be opened, res = %d\n", res);
		res2 = st.st_size > 0;
		BC_ASSERT_TRUE(res2);
		ms_message("We check if the file has non zero size, res2 = %d\n", res2);
		remove(filename);
	}


	fail:
	if(recorder) linphone_recorder_unref(recorder);
	if(lc_manager) linphone_core_manager_destroy(lc_manager);
}

static void record_wav_pcm_test(void) {
	char *filename = bctbx_strdup_printf("%s/testrecordpcm.wav", bc_tester_get_writable_dir_prefix());
	const char *audio_mime = "pcm";
	const char *video_mime = "";
	record_file(filename, TRUE, audio_mime, video_mime, LinphoneRecorderFileFormatWav);
	ms_free(filename);
}

static void record_mkv_opus_h264_test(void) {
#ifdef HAVE_OPENH264
	char *filename = bctbx_strdup_printf("%s/testrecordopush264.mkv", bc_tester_get_writable_dir_prefix());
	const char *audio_mime = "opus";
	const char *video_mime = "h264";
	record_file(filename, linphone_recorder_matroska_supported(), audio_mime, video_mime, LINPHONE_RECORDER_FORMAT_MATROSKA);
	ms_free(filename);
#endif
}

static void record_mkv_opus_vp8_test(void) {
	char *filename = bctbx_strdup_printf("%s/testrecordopusvp8.mkv", bc_tester_get_writable_dir_prefix());
	const char *audio_mime = "opus";
	const char *video_mime = "vp8";
	record_file(filename, linphone_recorder_matroska_supported(), audio_mime, video_mime, LinphoneRecorderFileFormatMkv);
	ms_free(filename);
}

test_t recorder_tests[] = {
	TEST_NO_TAG("Recording wave", record_wav_pcm_test),
	TEST_NO_TAG("Recording mkv opus+h264", record_mkv_opus_h264_test),
	TEST_NO_TAG("Recording mkv opus+VP8", record_mkv_opus_vp8_test)
};

test_suite_t recorder_test_suite = {"Recorder", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								  sizeof(recorder_tests) / sizeof(test_t), recorder_tests};
