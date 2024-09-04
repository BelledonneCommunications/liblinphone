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

#include "recorder.h"

#include "core/core-p.h"
#include "logger/logger.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

Recorder::Recorder(shared_ptr<Core> core, shared_ptr<const RecorderParams> params)
    : CoreAccessor(core), mParams(params) {
	init();
}

Recorder::~Recorder() {
	if (mRecorder != nullptr) ms_media_recorder_free(mRecorder);
}

Recorder *Recorder::clone() const {
	return nullptr;
}

void Recorder::init() {
	MSSndCard *card;
	if (mParams->getAudioDevice() == nullptr) {
		MSSndCardManager *cardManager = ms_factory_get_snd_card_manager(getCore()->getCCore()->factory);
		card = ms_snd_card_manager_get_card(cardManager, linphone_core_get_capture_device(getCore()->getCCore()));
	} else {
		card = mParams->getAudioDevice()->getSoundCard();
	}

	MSWebCamManager *camManager = ms_factory_get_web_cam_manager(getCore()->getCCore()->factory);
	MSWebCam *cam;
	if (mParams->getWebcamName().empty()) {
		cam = ms_web_cam_manager_get_cam(camManager, linphone_core_get_video_device(getCore()->getCCore()));
	} else {
		cam = ms_web_cam_manager_get_cam(camManager, mParams->getWebcamName().c_str());
	}

	mRecorder = ms_media_recorder_new(getCore()->getCCore()->factory, card, cam,
	                                  linphone_core_get_video_display_filter(getCore()->getCCore()),
	                                  mParams->getWindowId(), (MSFileFormat)mParams->getFileFormat(),
	                                  mParams->getVideoCodec().empty() ? NULL : mParams->getVideoCodec().c_str());
}

LinphoneStatus Recorder::open(const std::string &file) {
	mFilePath = file;
	ms_media_recorder_set_device_orientation(mRecorder, linphone_core_get_device_rotation(getCore()->getCCore()));
	return ms_media_recorder_open(mRecorder, file.c_str()) ? 0 : -1;
}

void Recorder::close() {
	pause();
	if (getState() != LinphoneRecorderClosed) ms_media_recorder_close(mRecorder);
}

const std::string &Recorder::getFile() const {
	return mFilePath;
}

LinphoneStatus Recorder::start() {
	bctbx_gettimeofday(&mStartTime, nullptr);
	getPlatformHelpers(getCore()->getCCore())->onRecordingStarted();
	return ms_media_recorder_start(mRecorder) ? 0 : -1;
}

LinphoneStatus Recorder::pause() {
	if (getState() == LinphoneRecorderRunning) {
		ms_media_recorder_pause(mRecorder);
		bctbx_gettimeofday(&mEndTime, nullptr);
		getPlatformHelpers(getCore()->getCCore())->onRecordingPaused();
		return 0;
	} else return -1;
}

LinphoneRecorderState Recorder::getState() const {
	switch (ms_media_recorder_get_state(mRecorder)) {
		case MSRecorderRunning:
			return LinphoneRecorderRunning;
		case MSRecorderPaused:
			return LinphoneRecorderPaused;
		case MSRecorderClosed:
		default:
			return LinphoneRecorderClosed;
	}
}

int Recorder::getDuration() const {
	if (getState() == LinphoneRecorderRunning) {
		struct timeval cur;
		bctbx_gettimeofday(&cur, nullptr);
		double elapsed = static_cast<double>(cur.tv_sec - mStartTime.tv_sec) * 1000 +
		                 static_cast<double>(cur.tv_usec - mStartTime.tv_usec) / 1000;
		return (int)elapsed;
	} else {
		double elapsed = static_cast<double>(mEndTime.tv_sec - mStartTime.tv_sec) * 1000 +
		                 static_cast<double>(mEndTime.tv_usec - mStartTime.tv_usec) / 1000;
		return (int)elapsed;
	}
}

float Recorder::getCaptureVolume() const {
	return ms_media_recorder_get_capture_volume(mRecorder);
}

std::shared_ptr<FileContent> Recorder::createContent() const {
	LinphoneRecorderState currentState = getState();
	if (currentState != LinphoneRecorderClosed) {
		lError() << "Cannot create Content from Recorder that isn't in Closed state, current state is " << currentState;
		return nullptr;
	}

	auto fileContent = FileContent::create<FileContent>();
	fileContent->setFilePath(mFilePath);
	fileContent->setContentType(ContentType::VoiceRecording);
	fileContent->setFileDuration(getDuration());

	string fileName = mFilePath.substr(mFilePath.find_last_of("/\\") + 1);
	fileContent->setFileName(fileName);

	return fileContent;
}

void Recorder::setParams(std::shared_ptr<RecorderParams> params) {
	if (getState() != LinphoneRecorderClosed) {
		lError() << "Cannot set Recorder [" << params << "] params, close the recording before!";
	} else {
		if (mRecorder != nullptr) ms_media_recorder_free(mRecorder);
		init();
	}
}

std::shared_ptr<const RecorderParams> Recorder::getParams() const {
	return mParams;
}

void Recorder::setUserData(void *userData) {
	mUserData = userData;
}

void *Recorder::getUserData() const {
	return mUserData;
}

LINPHONE_END_NAMESPACE
