/*
 * c-participant-device.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _L_C_PARTICIPANT_DEVICE_H_
#define _L_C_PARTICIPANT_DEVICE_H_

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
 **/
LINPHONE_PUBLIC LinphoneParticipantDevice *linphone_participant_device_ref (LinphoneParticipantDevice *participant_device);

/**
 * Decrement reference count of #LinphoneParticipantDevice object.
 **/
LINPHONE_PUBLIC void linphone_participant_device_unref (LinphoneParticipantDevice *participant_device);

/**
 * Retrieve the user pointer associated with the participant's device.
 * @param[in] participant_device A #LinphoneParticipantDevice object
 * @return The user pointer associated with the participant's device.
**/
LINPHONE_PUBLIC void * linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device);

/**
 * Assign a user pointer to the participant's device.
 * @param[in] participant_device A #LinphoneParticipantDevice object
 * @param[in] ud The user pointer to associate with the participant's device
**/
LINPHONE_PUBLIC void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *ud);

/**
 * Get the address of a participant's device.
 * @param[in] participant_device A #LinphoneParticipantDevice object
 * @return The address of the participant's device
 */
LINPHONE_PUBLIC const LinphoneAddress * linphone_participant_device_get_address (const LinphoneParticipantDevice *participant_device);
        
/**
 * Get the security level of a participant's device.
 * @param[in] participant_device A #LinphoneParticipantDevice object
 * @return The security level of the device
*/
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_participant_device_get_security_level (const LinphoneParticipantDevice *participant_device);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_H_
