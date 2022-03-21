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
 * @return The current is this participant device speaking changed callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsIsSpeakingChangedCb linphone_participant_device_cbs_get_is_speaking_changed (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the is this participant device speaking changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The is this participant device speaking changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_is_speaking_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb);

/**
 * Get the is this participant device muted callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The current is this participant device muted callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsIsMutedCb linphone_participant_device_cbs_get_is_muted (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the is this participant device muted callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The is this participant device muted callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_is_muted (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsIsMutedCb cb);

/**
 * Get the participant device conference joined callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The current participant device conference joined callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsConferenceJoinedCb linphone_participant_device_cbs_get_conference_joined (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the participant device conference joined callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The participant device conference joined callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_conference_joined (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsConferenceJoinedCb cb);

/**
 * Get the participant device conference left callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The current participant device conference left callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsConferenceLeftCb linphone_participant_device_cbs_get_conference_left (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the participant device conference left callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The participant device conference left callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_conference_left (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsConferenceLeftCb cb);

/**
 * Get the participant device stream capability changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The current participant device stream capability changed callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsStreamCapabilityChangedCb linphone_participant_device_cbs_get_stream_capability_changed (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the participant device stream capability changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The participant device stream capability changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_stream_capability_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsStreamCapabilityChangedCb cb);

/**
 * Get the participant device stream availability changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @return The current participant device stream availability changed callback.
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb linphone_participant_device_cbs_get_stream_availability_changed (const LinphoneParticipantDeviceCbs *cbs);

/**
 * Set the participant device stream availability changed callback.
 * @param[in] cbs The #LinphoneParticipantDeviceCbs object @notnil
 * @param[in] cb The participant device stream availability changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_device_cbs_set_stream_availability_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_CBS_H_
