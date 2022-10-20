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

#ifndef LINPHONE_VIDEO_SOURCE_DESCRIPTOR_H
#define LINPHONE_VIDEO_SOURCE_DESCRIPTOR_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup call_control
 * @{
 */

/**
 * Create a new #LinphoneVideoSourceDescriptor object.
 * @return The newly created #LinphoneVideoSourceDescriptor object. @notnil
 */
LINPHONE_PUBLIC LinphoneVideoSourceDescriptor* linphone_video_source_descriptor_new(void);

/**
 * Instantiate a new video source descriptor with values from source.
 * @param descriptor The #LinphoneVideoSourceDescriptor object to be cloned. @notnil
 * @return The newly created #LinphoneVideoSourceDescriptor object. @notnil
 */
LINPHONE_PUBLIC LinphoneVideoSourceDescriptor* linphone_video_source_descriptor_clone(const LinphoneVideoSourceDescriptor *descriptor);

/**
 * Take a reference on a #LinphoneVideoSourceDescriptor.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @return The same #LinphoneVideoSourceDescriptor object. @notnil
 */
LINPHONE_PUBLIC LinphoneVideoSourceDescriptor* linphone_video_source_descriptor_ref(LinphoneVideoSourceDescriptor *descriptor);

/**
 * Release a #LinphoneVideoSourceDescriptor.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 */
LINPHONE_PUBLIC void linphone_video_source_descriptor_unref(LinphoneVideoSourceDescriptor *descriptor);

/**
 * Gets the type of a #LinphoneVideoSourceDescriptor.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @return The #LinphoneVideoSourceType corresponding to this video source descriptor.
 */
LINPHONE_PUBLIC LinphoneVideoSourceType linphone_video_source_descriptor_get_type(const LinphoneVideoSourceDescriptor *descriptor);

/**
 * Gets the call of a #LinphoneVideoSourceDescriptor.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @return The #LinphoneCall of the video source descriptor if it's type is LinphoneVideoSourceCall, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphoneCall* linphone_video_source_descriptor_get_call(const LinphoneVideoSourceDescriptor *descriptor);

/**
 * Sets the source of a #LinphoneVideoSourceDescriptor with a call.
 *
 * Setting a #LinphoneVideoSourceDescriptor with a call will require the lib to have two calls running at the same time.
 * To do so the media resource mode has to be set to LinphoneSharedMediaResources with #linphone_core_set_media_resource_mode().
 *
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @param call The #LinphoneCall that will be used as a video source. @maybenil
 */
LINPHONE_PUBLIC void linphone_video_source_descriptor_set_call(LinphoneVideoSourceDescriptor *descriptor, LinphoneCall *call);

/**
 * Gets the camera id of a #LinphoneVideoSourceDescriptor.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @return The camera id of the video source descriptor if it's type is LinphoneVideoSourceCamera, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_video_source_descriptor_get_camera_id(const LinphoneVideoSourceDescriptor *descriptor);

/**
 * Sets the source of a #LinphoneVideoSourceDescriptor with a camera id.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @param camera_id The camera id that will be used as a video source. @maybenil
 */
LINPHONE_PUBLIC void linphone_video_source_descriptor_set_camera_id(LinphoneVideoSourceDescriptor *descriptor, const char *camera_id);

/**
 * Gets the image path of a #LinphoneVideoSourceDescriptor.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @return The image path of the video source descriptor if it's type is LinphoneVideoSourceImage, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_video_source_descriptor_get_image(const LinphoneVideoSourceDescriptor *descriptor);

/**
 * Sets the source of a #LinphoneVideoSourceDescriptor with an image path.
 * @param descriptor The #LinphoneVideoSourceDescriptor object. @notnil
 * @param image_path The image path that will be used as a video source. @maybenil
 */
LINPHONE_PUBLIC void linphone_video_source_descriptor_set_image(LinphoneVideoSourceDescriptor *descriptor, const char *image_path);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_VIDEO_SOURCE_DESCRIPTOR_H */
