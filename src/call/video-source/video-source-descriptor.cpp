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

#include "video-source-descriptor.h"

#include "linphone/types.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

VideoSourceDescriptor::VideoSourceDescriptor (const VideoSourceDescriptor &other) : HybridObject(other) {
	mType = other.mType;
	mCall = other.mCall;
	mCameraId = other.mCameraId;
	mImagePath = other.mImagePath;
}

VideoSourceDescriptor* VideoSourceDescriptor::clone () const {
	return new VideoSourceDescriptor(*this);
}

// =============================================================================

VideoSourceDescriptor::Type VideoSourceDescriptor::getType () const {
	return mType;
}

shared_ptr<Call> VideoSourceDescriptor::getCall () const {
	auto call = mCall.lock();
	if (call) return call;
	return nullptr;
}

void VideoSourceDescriptor::setCall (shared_ptr<Call> call) {
	mCall = call;
	mType = call != nullptr ? VideoSourceDescriptor::Type::Call : VideoSourceDescriptor::Type::Unknown;

	mCameraId = "";
	mImagePath = "";
}

const string &VideoSourceDescriptor::getCameraId () const {
	return mCameraId;
}

void VideoSourceDescriptor::setCameraId (string cameraId) {
	mCameraId = cameraId;
	mType = !cameraId.empty() ? VideoSourceDescriptor::Type::Camera : VideoSourceDescriptor::Type::Unknown;

	mCall.reset();
	mImagePath = "";
}

const std::string &VideoSourceDescriptor::getImage () const {
	return mImagePath;
}

void VideoSourceDescriptor::setImage (std::string imagePath) {
	mImagePath = imagePath;
	mType = !imagePath.empty() ? VideoSourceDescriptor::Type::Image : VideoSourceDescriptor::Type::Unknown;

	mCall.reset();
	mCameraId = "";
}

LINPHONE_END_NAMESPACE
