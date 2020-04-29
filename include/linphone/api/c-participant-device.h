/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#ifndef _L_C_PARTICIPANT_DEVICE_H_
#define _L_C_PARTICIPANT_DEVICE_H_

#include "time.h"

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

/**
 * Increment reference count of #LinphoneParticipantDevice object.
 * @param participant_device the #LinphoneParticipantDevice object @notnil
 * @return the same #LinphoneParticipantDevice object @notnil
 **/
LINPHONE_PUBLIC LinphoneParticipantDevice *linphone_participant_device_ref (LinphoneParticipantDevice *participant_device);

/**
 * Decrement reference count of #LinphoneParticipantDevice object.
 * @param participant_device the #LinphoneParticipantDevice object @notnil
 **/
LINPHONE_PUBLIC void linphone_participant_device_unref (LinphoneParticipantDevice *participant_device);

/**
 * Retrieve the user pointer associated with the participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The user pointer associated with the participant's device. @maybenil
**/
LINPHONE_PUBLIC void * linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device);

/**
 * Assign a user pointer to the participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @param user_data The user pointer to associate with the participant's device. @maybenil
**/
LINPHONE_PUBLIC void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *user_data);

/**
 * Get the address of a participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The #LinphoneAddress of the participant's device @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress* linphone_participant_device_get_address (const LinphoneParticipantDevice *participant_device);

/**
 * Get the security level of a participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The #LinphoneChatRoomSecurityLevel of the device
*/
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_participant_device_get_security_level (const LinphoneParticipantDevice *participant_device);

/**
 * Return the name of the device or NULL.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return the name of the device or NULL. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_participant_device_get_name (const LinphoneParticipantDevice *participant_device);

/**
 * Return whether the participant device is in a conference or not.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return a boolean to state whether the device is in a conference
 */
LINPHONE_PUBLIC bool_t linphone_participant_device_is_in_conference (const LinphoneParticipantDevice *participant_device);

/**
 * Get the timestamp the device joined a conference.
 * @param participant A #LinphoneParticipantDevice object @notnil
 * @return time of joining a conference as returned by time(nullptr). For UNIX based systems it is the number of seconds since 00:00hours of the 1st of January 1970
 */
LINPHONE_PUBLIC time_t linphone_participant_device_get_time_of_joining(const LinphoneParticipantDevice *participant_device);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_H_
