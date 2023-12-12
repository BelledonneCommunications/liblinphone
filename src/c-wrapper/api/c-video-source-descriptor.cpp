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

#include "linphone/api/c-video-source-descriptor.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call.h"
#include "call/video-source/video-source-descriptor.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneVideoSourceDescriptor *linphone_video_source_descriptor_new() {
	return VideoSourceDescriptor::createCObject();
}

LinphoneVideoSourceDescriptor *linphone_video_source_descriptor_clone(const LinphoneVideoSourceDescriptor *descriptor) {
	return VideoSourceDescriptor::toCpp(descriptor)->clone()->toC();
}

LinphoneVideoSourceDescriptor *linphone_video_source_descriptor_ref(LinphoneVideoSourceDescriptor *descriptor) {
	VideoSourceDescriptor::toCpp(descriptor)->ref();
	return descriptor;
}

void linphone_video_source_descriptor_unref(LinphoneVideoSourceDescriptor *descriptor) {
	VideoSourceDescriptor::toCpp(descriptor)->unref();
}

LinphoneVideoSourceType linphone_video_source_descriptor_get_type(const LinphoneVideoSourceDescriptor *descriptor) {
	return static_cast<LinphoneVideoSourceType>(VideoSourceDescriptor::toCpp(descriptor)->getType());
}

LinphoneCall *linphone_video_source_descriptor_get_call(const LinphoneVideoSourceDescriptor *descriptor) {
	return VideoSourceDescriptor::toCpp(descriptor)->getCall()->toC();
}

void linphone_video_source_descriptor_set_call(LinphoneVideoSourceDescriptor *descriptor, LinphoneCall *call) {
	VideoSourceDescriptor::toCpp(descriptor)->setCall(Call::toCpp(call)->getSharedFromThis());
}

const char *linphone_video_source_descriptor_get_camera_id(const LinphoneVideoSourceDescriptor *descriptor) {
	return L_STRING_TO_C(VideoSourceDescriptor::toCpp(descriptor)->getCameraId());
}

void linphone_video_source_descriptor_set_camera_id(LinphoneVideoSourceDescriptor *descriptor, const char *camera_id) {
	VideoSourceDescriptor::toCpp(descriptor)->setCameraId(L_C_TO_STRING(camera_id));
}

const char *linphone_video_source_descriptor_get_image(const LinphoneVideoSourceDescriptor *descriptor) {
	return L_STRING_TO_C(VideoSourceDescriptor::toCpp(descriptor)->getImage());
}

void linphone_video_source_descriptor_set_image(LinphoneVideoSourceDescriptor *descriptor, const char *image_path) {
	VideoSourceDescriptor::toCpp(descriptor)->setImage(L_C_TO_STRING(image_path));
}

LinphoneVideoSourceScreenSharingType
linphone_video_source_descriptor_get_screen_sharing_type(const LinphoneVideoSourceDescriptor *descriptor) {
	return static_cast<LinphoneVideoSourceScreenSharingType>(
	    VideoSourceDescriptor::toCpp(descriptor)->getScreenSharingType());
}
void *linphone_video_source_descriptor_get_screen_sharing(const LinphoneVideoSourceDescriptor *descriptor) {
	return VideoSourceDescriptor::toCpp(descriptor)->getScreenSharing();
}
void linphone_video_source_descriptor_set_screen_sharing(LinphoneVideoSourceDescriptor *descriptor,
                                                         LinphoneVideoSourceScreenSharingType type,
                                                         void *native_data) {
	VideoSourceDescriptor::toCpp(descriptor)
	    ->setScreenSharing(static_cast<VideoSourceDescriptor::ScreenSharingType>(type), native_data);
}
