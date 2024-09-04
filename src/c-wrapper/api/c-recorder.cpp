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

#include "linphone/api/c-recorder.h"

#include "c-wrapper/c-wrapper.h"
#include "core/core.h"
#include "linphone/misc.h"
#include "recorder/recorder-params.h"
#include "recorder/recorder.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneRecorder *linphone_recorder_new(LinphoneCore *core, const LinphoneRecorderParams *params) {
	return Recorder::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core),
	                               RecorderParams::toCpp(params)->getSharedFromThis());
}

LinphoneRecorder *linphone_recorder_ref(LinphoneRecorder *recorder) {
	Recorder::toCpp(recorder)->ref();
	return recorder;
}

void linphone_recorder_unref(LinphoneRecorder *recorder) {
	Recorder::toCpp(recorder)->unref();
}

LinphoneStatus linphone_recorder_open(LinphoneRecorder *recorder, const char *file) {
	return Recorder::toCpp(recorder)->open(L_C_TO_STRING(file));
}

void linphone_recorder_close(LinphoneRecorder *recorder) {
	Recorder::toCpp(recorder)->close();
}

const char *linphone_recorder_get_file(const LinphoneRecorder *recorder) {
	return L_STRING_TO_C(Recorder::toCpp(recorder)->getFile());
}

LinphoneStatus linphone_recorder_start(LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->start();
}

LinphoneStatus linphone_recorder_pause(LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->pause();
}

LinphoneRecorderState linphone_recorder_get_state(const LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->getState();
}

int linphone_recorder_get_duration(const LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->getDuration();
}

float linphone_recorder_get_capture_volume(const LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->getCaptureVolume();
}

LinphoneContent *linphone_recorder_create_content(LinphoneRecorder *recorder) {
	auto fileContent = Recorder::toCpp(recorder)->createContent();
	if (fileContent != nullptr) {
		fileContent->ref();
		return fileContent->toC();
	}
	return nullptr;
}

void linphone_recorder_set_params(LinphoneRecorder *recorder, LinphoneRecorderParams *params) {
	Recorder::toCpp(recorder)->setParams(RecorderParams::toCpp(params)->getSharedFromThis());
}

const LinphoneRecorderParams *linphone_recorder_get_params(const LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->getParams()->toC();
}

void linphone_recorder_set_user_data(LinphoneRecorder *recorder, void *user_data) {
	Recorder::toCpp(recorder)->setUserData(user_data);
}

void *linphone_recorder_get_user_data(const LinphoneRecorder *recorder) {
	return Recorder::toCpp(recorder)->getUserData();
}

bool_t linphone_recorder_matroska_supported(void) {
	return ms_media_recorder_matroska_supported();
}
