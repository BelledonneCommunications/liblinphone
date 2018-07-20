
/*
linphone
Copyright (C) 2014 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "c-wrapper/c-wrapper.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

#include <mediastreamer2/msmediarecorder.h>
#include <mediastreamer2/mssndcard.h>
#include <mediastreamer2/mswebcam.h>


BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneRecorder);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneRecorder, belle_sip_object_t,
	_linphone_recorder_destroy, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

static int _recorder_open(LinphoneRecorder *obj, const char *filename);
static int _recorder_start(LinphoneRecorder *obj);
static int _recorder_pause(LinphoneRecorder *obj);
static MSRecorderState _recorder_get_state(LinphoneRecorder *obj);
static void _recorder_close(LinphoneRecorder *obj);
static void _recorder_destroy(LinphoneRecorder *obj);
static void _recorder_remove_file(LinphoneRecorder *obj, const char *filename);

LinphoneRecorder *linphone_core_create_recorder(LinphoneCore *lc, const char *sound_card_name, const char *web_cam_name, const char *video_display_name, void *window_id, MSFileFormat format, const char *video_codec) {
	LinphoneRecorder *obj = linphone_recorder_new();
	MSSndCard *snd_card;
	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(lc->factory);
	MSWebCam *web_cam;
	MSWebCamManager *web_cam_manager = ms_factory_get_web_cam_manager(lc->factory);
	if (sound_card_name == NULL) sound_card_name = linphone_core_get_ringer_device(lc);
	snd_card = ms_snd_card_manager_get_card(snd_card_manager, sound_card_name);
	if (web_cam_name == NULL) web_cam_name = linphone_core_get_video_device(lc);
	web_cam = ms_web_cam_manager_get_cam(web_cam_manager, web_cam_name);
	if (video_display_name == NULL) video_display_name = linphone_core_get_video_display_filter(lc);
	obj->impl = ms_media_recorder_new(lc->factory, snd_card, web_cam, video_display_name, window_id, format, video_codec);
	obj->open = _recorder_open;
	obj->start = _recorder_start;
	obj->pause = _recorder_pause;
	obj->get_state = _recorder_get_state;
	obj->close = _recorder_close;
	obj->destroy = _recorder_destroy;
	obj->remove_file = _recorder_remove_file;
	return obj;
}

bool_t linphone_recorder_matroska_supported(void) {
	return ms_media_recorder_matroska_supported();
}

static int _recorder_open(LinphoneRecorder *obj, const char *filename) {
	return ms_media_recorder_open((MSMediaRecorder *)obj->impl, filename) ? 0 : -1;
}

static int _recorder_start(LinphoneRecorder *obj) {
	return ms_media_recorder_start((MSMediaRecorder *)obj->impl) ? 0 : -1;
}

static int _recorder_pause(LinphoneRecorder *obj) {
	ms_media_recorder_pause((MSMediaRecorder *)obj->impl);
	return 0;
}

static MSRecorderState _recorder_get_state(LinphoneRecorder *obj) {
	return ms_media_recorder_get_state((MSMediaRecorder *)obj->impl);
}

static void _recorder_destroy(LinphoneRecorder *obj) {
	ms_media_recorder_free((MSMediaRecorder *)obj->impl);
}

static void _recorder_close(LinphoneRecorder *obj) {
	ms_media_recorder_close((MSMediaRecorder *)obj->impl);
}

static void _recorder_remove_file(LinphoneRecorder *obj, const char *filename) {
	ms_media_recorder_remove_file((MSMediaRecorder *)obj->impl, filename);
}

LinphoneRecorder * linphone_recorder_new(void) {
	LinphoneRecorder *recorder = belle_sip_object_new(LinphoneRecorder);
	return recorder;
}

LinphoneRecorder * linphone_recorder_ref(LinphoneRecorder *recorder) {
	belle_sip_object_ref(recorder);
	return recorder;
}

void linphone_recorder_unref(LinphoneRecorder *recorder) {
	belle_sip_object_unref(recorder);
}

void *linphone_recorder_get_user_data(const LinphoneRecorder *recorder) {
	return recorder->user_data;
}

void linphone_recorder_set_user_data(LinphoneRecorder *recorder, void *ud) {
	recorder->user_data = ud;
}

LinphoneStatus linphone_recorder_open(LinphoneRecorder *obj, const char *filename){
	return obj->open(obj,filename);
}

LinphoneStatus linphone_recorder_start(LinphoneRecorder *obj){
	return obj->start(obj);
}

LinphoneStatus linphone_recorder_pause(LinphoneRecorder *obj){
	return obj->pause(obj);
}

LinphoneRecorderState linphone_recorder_get_state(LinphoneRecorder *obj){
	switch (obj->get_state(obj)) {
		case MSRecorderClosed:
		default:
			return LinphoneRecorderClosed;
		case MSRecorderPaused:
			return LinphoneRecorderPaused;
		case MSRecorderRunning:
			return LinphoneRecorderRunning;
	}
}

void linphone_recorder_remove_file(LinphoneRecorder *obj, const char *filename){
	obj->remove_file(obj, filename);
}

void linphone_recorder_close(LinphoneRecorder *obj){
	obj->close(obj);
}

void linphone_recorder_destroy(LinphoneRecorder *obj) {
	_linphone_recorder_destroy(obj);
}

void _linphone_recorder_destroy(LinphoneRecorder *recorder) {
	if(recorder->destroy) recorder->destroy(recorder);
}
