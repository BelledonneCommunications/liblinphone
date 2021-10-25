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
 * Create a new participant device callbacks object.
 * @return The #LinphoneParticipantDeviceCbs object. @notnil
**/
LinphoneParticipantDeviceCbs *linphone_participant_device_cbs_new (void);

/**
 * Acquire a reference to the participant device callbacks object.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The same participant device callbacks object
**/
LINPHONE_PUBLIC LinphoneParticipantDeviceCbs * linphone_participant_device_cbs_ref (LinphoneParticipantDeviceCbs *cbs);

/**
 * Release reference to the participant device callbacks object.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
**/
LINPHONE_PUBLIC void linphone_participant_device_cbs_unref (LinphoneParticipantDeviceCbs *cbs);

/**
 * Retrieve the user pointer associated with the participant device callbacks object.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The user pointer associated with the participant device callbacks object
**/
LINPHONE_PUBLIC void * linphone_participant_device_cbs_get_user_data (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Assign a user pointer to the participant device callbacks object.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] ud The user pointer to associate with the participant device callbacks object @maybenil
**/
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_user_data (LinphoneParticipantDeviceCbs *cbs, void *ud);

/**
 * Get the is this participant device speaking changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The current is this particiapnt device speaking changed callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsIsSpeakingChangedCb linphone_participant_device_cbs_get_is_speaking_changed (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the is this participant device speaking changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The is this participant device speaking changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_is_speaking_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb);
	
	
/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_CBS_H_
