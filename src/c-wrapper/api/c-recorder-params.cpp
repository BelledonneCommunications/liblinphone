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

#include <ctype.h>

#include "c-wrapper/internal/c-tools.h"
#include "call/audio-device/audio-device.h"
#include "linphone/api/c-recorder-params.h"
#include "recorder/recorder-params.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneRecorderParams *linphone_recorder_params_new() {
	return RecorderParams::createCObject(nullptr, "", nullptr, LinphoneMediaFileFormatUnknown, "");
}

LinphoneRecorderParams *linphone_recorder_params_clone(const LinphoneRecorderParams *params) {
	return RecorderParams::toCpp(params)->clone()->toC();
}

LinphoneRecorderParams *linphone_recorder_params_ref(LinphoneRecorderParams *params) {
	RecorderParams::toCpp(params)->ref();
	return params;
}

void linphone_recorder_params_unref(LinphoneRecorderParams *params) {
	RecorderParams::toCpp(params)->unref();
}

void linphone_recorder_params_set_audio_device(LinphoneRecorderParams *params, const LinphoneAudioDevice *device) {
	RecorderParams::toCpp(params)->setAudioDevice(device ? AudioDevice::toCpp(device)->getSharedFromThis() : nullptr);
}

const LinphoneAudioDevice *linphone_recorder_params_get_audio_device(const LinphoneRecorderParams *params) {
	return RecorderParams::toCpp(params)->getAudioDevice() ? RecorderParams::toCpp(params)->getAudioDevice()->toC()
	                                                       : nullptr;
}

void linphone_recorder_params_set_webcam_name(LinphoneRecorderParams *params, const char *webcam_name) {
	RecorderParams::toCpp(params)->setWebcamName(L_C_TO_STRING(webcam_name));
}

const char *linphone_recorder_params_get_webcam_name(const LinphoneRecorderParams *params) {
	return L_STRING_TO_C(RecorderParams::toCpp(params)->getWebcamName());
}

void linphone_recorder_params_set_video_codec(LinphoneRecorderParams *params, const char *video_codec) {
	RecorderParams::toCpp(params)->setVideoCodec(L_C_TO_STRING(video_codec));
}

const char *linphone_recorder_params_get_video_codec(const LinphoneRecorderParams *params) {
	return L_STRING_TO_C(RecorderParams::toCpp(params)->getVideoCodec());
}

void linphone_recorder_params_set_file_format(LinphoneRecorderParams *params, LinphoneMediaFileFormat format) {
	RecorderParams::toCpp(params)->setFileFormat(format);
}

LinphoneMediaFileFormat linphone_recorder_params_get_file_format(const LinphoneRecorderParams *params) {
	return RecorderParams::toCpp(params)->getFileFormat();
}

void linphone_recorder_params_set_window_id(LinphoneRecorderParams *params, void *window_id) {
	RecorderParams::toCpp(params)->setWindowId(window_id);
}

void *linphone_recorder_params_get_window_id(const LinphoneRecorderParams *params) {
	return RecorderParams::toCpp(params)->getWindowId();
}
