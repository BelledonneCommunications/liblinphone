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

#ifndef LINPHONE_RECORDER_H
#define LINPHONE_RECORDER_H

#include "linphone/api/c-recorder-params.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup call-control
 * @{
 */

/**
 * Create a new #LinphoneRecorder object.
 * @param core The #LinphoneCore object. @notnil
 * @param params The #LinphoneRecorderParams object. @notnil
 * @return the newly created #LinphoneRecorder object. @notnil
 */
LINPHONE_PUBLIC LinphoneRecorder *linphone_recorder_new(LinphoneCore *core, const LinphoneRecorderParams *params);

/**
 * Take a reference on a #LinphoneRecorder object.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return the same #LinphoneRecorder object. @notnil
 */
LINPHONE_PUBLIC LinphoneRecorder *linphone_recorder_ref(LinphoneRecorder *recorder);

/**
 * Release a #LinphoneRecorder object.
 * @param recorder The #LinphoneRecorder object. @notnil
 */
LINPHONE_PUBLIC void linphone_recorder_unref(LinphoneRecorder *recorder);

/**
 * Opens a file for recording.
 * If the file already exists, it will open in append mode, otherwise it is created.

 * @param recorder The #LinphoneRecorder object. @notnil
 * @param file The path to the file to open. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_recorder_open(LinphoneRecorder *recorder, const char *file);

/**
 * Closes the opened file.
 * @param recorder The #LinphoneRecorder object. @notnil
 */
LINPHONE_PUBLIC void linphone_recorder_close(LinphoneRecorder *recorder);

/**
 * Gets the file used for recording.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return the file used for the recording if any. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_recorder_get_file(const LinphoneRecorder *recorder);

/**
 * Starts the recording into the opened file.
 * @param recorder The #LinphoneRecorder object. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_recorder_start(LinphoneRecorder *recorder);

/**
 * Pauses the recording.
 * @param recorder The #LinphoneRecorder object. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_recorder_pause(LinphoneRecorder *recorder);

/**
 * Gets the current state of the recorder.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return the current #LinphoneRecorderState.
 */
LINPHONE_PUBLIC LinphoneRecorderState linphone_recorder_get_state(const LinphoneRecorder *recorder);

/**
 * Gets the duration of the recording.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return the duration of the recording, in milliseconds.
 */
LINPHONE_PUBLIC int linphone_recorder_get_duration(const LinphoneRecorder *recorder);

/**
 * Get linear volume when capturing audio.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return Linear volume.
 */
LINPHONE_PUBLIC float linphone_recorder_get_capture_volume(const LinphoneRecorder *recorder);

/**
 * Create a #LinphoneContent object from the recording, for example to send it within a #LinphoneChatMessage.
 * @warning Recorder must be in Closed state!
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return the #LinphoneContent matching the recording, or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneContent *linphone_recorder_create_content(LinphoneRecorder *recorder);

/**
 * Sets the #LinphoneRecorderParams object.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @param params The #LinphoneRecorderParams object to set. @notnil
 */
LINPHONE_PUBLIC void linphone_recorder_set_params(LinphoneRecorder *recorder, LinphoneRecorderParams *params);

/**
 * Retrieves the #LinphoneRecorderParams object.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return The #LinphoneRecorderParams object. @notnil
 */
LINPHONE_PUBLIC const LinphoneRecorderParams *linphone_recorder_get_params(const LinphoneRecorder *recorder);

/**
 * Sets the user data.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @param user_data The user data to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_recorder_set_user_data(LinphoneRecorder *recorder, void *user_data);

/**
 * Retrieves the user data.
 * @param recorder The #LinphoneRecorder object. @notnil
 * @return the user data to retrieve. @maybenil
 */
LINPHONE_PUBLIC void *linphone_recorder_get_user_data(const LinphoneRecorder *recorder);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_RECORDER_H */
