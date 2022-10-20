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

#ifndef LINPHONE_RECORDER_PARAMS_H
#define LINPHONE_RECORDER_PARAMS_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup call-control
 * @{
 */

/**
 * Create a new #LinphoneRecorderParams object.
 * @return the newly created #LinphoneRecorderParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneRecorderParams* linphone_recorder_params_new(void);

/**
 * Clone a #LinphoneRecorderParams object.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the cloned #LinphoneRecorderParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneRecorderParams* linphone_recorder_params_clone(const LinphoneRecorderParams *params);

/**
 * Take a reference on a #LinphoneRecorderParams object.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the same #LinphoneRecorderParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneRecorderParams* linphone_recorder_params_ref(LinphoneRecorderParams *params);

/**
 * Release a #LinphoneRecorderParams object.
 * @param params The #LinphoneRecorderParams object. @notnil
 */
LINPHONE_PUBLIC void linphone_recorder_params_unref(LinphoneRecorderParams *params);

/**
 * Set the #LinphoneAudioDevice object.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @param device The #LinphoneAudioDevice object to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_recorder_params_set_audio_device(LinphoneRecorderParams *params, const LinphoneAudioDevice *device);

/**
 * Retrieve the #LinphoneAudioDevice object.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the #LinphoneAudioDevice object. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAudioDevice *linphone_recorder_params_get_audio_device(const LinphoneRecorderParams *params);

/**
 * Set the webcam name.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @param webcam_name The webcam name to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_recorder_params_set_webcam_name(LinphoneRecorderParams *params, const char *webcam_name);

/**
 * Retrieves the webcam name.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the webcam name. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_recorder_params_get_webcam_name(const LinphoneRecorderParams *params);

/**
 * Set the video codec.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @param video_codec The video codec to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_recorder_params_set_video_codec(LinphoneRecorderParams *params, const char *video_codec);

/**
 * Retrieves the video codec.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the video codec. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_recorder_params_get_video_codec(const LinphoneRecorderParams *params);

/**
 * Set the #LinphoneRecorderFileFormat.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @param format The #LinphoneRecorderFileFormat to set.
 */
LINPHONE_PUBLIC void linphone_recorder_params_set_file_format(LinphoneRecorderParams *params, LinphoneRecorderFileFormat format);

/**
 * Retrieves the #LinphoneRecorderFileFormat.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the #LinphoneRecorderFileFormat.
 */
LINPHONE_PUBLIC LinphoneRecorderFileFormat linphone_recorder_params_get_file_format(const LinphoneRecorderParams *params);

/**
 * Set the window id.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @param window_id The window id to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_recorder_params_set_window_id(LinphoneRecorderParams *params, void *window_id);

/**
 * Retrieves the window id.
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the window id. @maybenil
 */
LINPHONE_PUBLIC void *linphone_recorder_params_get_window_id(const LinphoneRecorderParams *params);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_RECORDER_PARAMS_H */
