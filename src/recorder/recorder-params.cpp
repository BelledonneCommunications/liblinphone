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

#include "recorder-params.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

RecorderParams::RecorderParams(shared_ptr<const AudioDevice> device,
                               const string &webcamName,
                               void *windowId,
                               LinphoneMediaFileFormat format,
                               const string &videoCodec)
    : mAudioDevice(device), mWebcamName(webcamName), mWindowId(windowId), mFormat(format), mVideoCodec(videoCodec) {
}

RecorderParams::RecorderParams(const RecorderParams &other) : HybridObject(other) {
	mAudioDevice = other.mAudioDevice;
	mWebcamName = other.mWebcamName;
	mVideoCodec = other.mVideoCodec;
	mFormat = other.mFormat;
	mWindowId = other.mWindowId;
}

RecorderParams *RecorderParams::clone() const {
	return new RecorderParams(*this);
}

void RecorderParams::setAudioDevice(shared_ptr<const AudioDevice> audioDevice) {
	mAudioDevice = audioDevice;
}

void RecorderParams::setWebcamName(const string &webcamName) {
	mWebcamName = webcamName;
}

void RecorderParams::setVideoCodec(const string &videoCodec) {
	mVideoCodec = videoCodec;
}

void RecorderParams::setFileFormat(LinphoneMediaFileFormat format) {
	mFormat = format;
}

void RecorderParams::setWindowId(void *windowId) {
	mWindowId = windowId;
}

shared_ptr<const AudioDevice> RecorderParams::getAudioDevice() const {
	return mAudioDevice;
}

const string &RecorderParams::getWebcamName() const {
	return mWebcamName;
}

const string &RecorderParams::getVideoCodec() const {
	return mVideoCodec;
}

LinphoneMediaFileFormat RecorderParams::getFileFormat() const {
	return mFormat;
}

void *RecorderParams::getWindowId() const {
	return mWindowId;
}

LINPHONE_END_NAMESPACE
