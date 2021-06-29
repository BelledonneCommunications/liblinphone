
/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_C_PARTICIPANT_DEVICE_CBS_H_
#define _L_C_PARTICIPANT_DEVICE_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup conference
 * @{
 */

/**
 * Acquire a reference to the participant device callbacks object.
 * @param[in] cbs The participant device callbacks object
 * @return The same participant device callbacks object
**/
LINPHONE_PUBLIC LinphoneParticipantDeviceCbs * linphone_participant_device_cbs_ref (LinphoneParticipantDeviceCbs *cbs);

/**
 * Release reference to the participant device callbacks object.
 * @param[in] cr The participant device callbacks object
**/
LINPHONE_PUBLIC void linphone_participant_device_cbs_unref (LinphoneParticipantDeviceCbs *cbs);

/**
 * Retrieve the user pointer associated with the participant device callbacks object.
 * @param[in] cr The participant device callbacks object
 * @return The user pointer associated with the participant device callbacks object
**/
LINPHONE_PUBLIC void * linphone_participant_device_cbs_get_user_data (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Assign a user pointer to the participant device callbacks object.
 * @param[in] cr The participant device callbacks object
 * @param[in] ud The user pointer to associate with the participant device callbacks object
**/
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_user_data (LinphoneParticipantDeviceCbs *cbs, void *ud);

/**
 * Get the capture video size changed callback.
 * @param[in] cbs #LinphoneParticipantDeviceCbs object.
 * @return The current capture video size changed callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsCaptureVideoSizeChangedCb linphone_participant_device_cbs_get_capture_video_size_changed (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the capture video size changed callback.
 * @param[in] cbs #LinphoneParticipantDeviceCbs object.
 * @param[in] cb The capture video size changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_capture_video_size_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsCaptureVideoSizeChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_CBS_H_
