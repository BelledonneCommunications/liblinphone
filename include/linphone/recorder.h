/*
recorder.h
Copyright (C) 2010-2018 Belledonne Communications SARL

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

#ifndef LINPHONE_RECORDER_H_
#define LINPHONE_RECORDER_H_


#include "linphone/types.h"
#include "mediastreamer2/msinterfaces.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup call_control
 * @{
 */

/**
 * Acquire a reference to the recorder.
 * @param[in] recorder #LinphoneRecorder object.
 * @return The same #LinphoneRecorder object.
**/
LINPHONE_PUBLIC LinphoneRecorder * linphone_recorder_ref(LinphoneRecorder *recorder);

/**
 * Release reference to the recorder.
 * @param[in] recorder #LinphoneRecorder object.
**/
LINPHONE_PUBLIC void linphone_recorder_unref(LinphoneRecorder *recorder);

/**
 * Retrieve the user pointer associated with the recorder.
 * @param[in] recorder #LinphoneRecorder object.
 * @return The user pointer associated with the recorder.
**/
LINPHONE_PUBLIC void *linphone_recorder_get_user_data(const LinphoneRecorder *recorder);

/**
 * Assign a user pointer to the recorder.
 * @param[in] recorder #LinphoneRecorder object.
 * @param[in] ud The user pointer to associate with the recorder.
**/
LINPHONE_PUBLIC void linphone_recorder_set_user_data(LinphoneRecorder *recorder, void *ud);

/**
 * Open a file for playing.
 * @param[in] obj #LinphoneRecorder object
 * @param[in] filename The path to the file to open
 */
LINPHONE_PUBLIC LinphoneStatus linphone_recorder_open(LinphoneRecorder *obj, const char *filename);

/**
 * Start playing a file that has been opened with linphone_recorder_open().
 * @param[in] obj #LinphoneRecorder object
 * @return 0 on success, a negative value otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_recorder_start(LinphoneRecorder *obj);

/**
 * Pause the playing of a file.
 * @param[in] obj #LinphoneRecorder object
 * @return 0 on success, a negative value otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_recorder_pause(LinphoneRecorder *obj);

/**
 * Get the current state of a recorder.
 * @param[in] obj #LinphoneRecorder object
 * @return The current state of the recorder.
 */
LINPHONE_PUBLIC LinphoneRecorderState linphone_recorder_get_state(LinphoneRecorder *obj);

/**
 * Close the opened file.
 * @param[in] obj #LinphoneRecorder object
 */
LINPHONE_PUBLIC void linphone_recorder_close(LinphoneRecorder *obj);

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_RECORDER_H_ */
