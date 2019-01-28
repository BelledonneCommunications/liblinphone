/*
 * c-participant.h
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

#ifndef _L_C_PARTICIPANT_H_
#define _L_C_PARTICIPANT_H_

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
 * Increment reference count of #LinphoneParticipant object.
 **/
LINPHONE_PUBLIC LinphoneParticipant *linphone_participant_ref (LinphoneParticipant *participant);

/**
 * Decrement reference count of #LinphoneParticipant object.
 **/
LINPHONE_PUBLIC void linphone_participant_unref (LinphoneParticipant *participant);

/**
 * Retrieve the user pointer associated with the conference participant.
 * @param[in] participant A #LinphoneParticipant object
 * @return The user pointer associated with the participant.
**/
LINPHONE_PUBLIC void * linphone_participant_get_user_data(const LinphoneParticipant *participant);

/**
 * Assign a user pointer to the conference participant.
 * @param[in] participant A #LinphoneParticipant object
 * @param[in] ud The user pointer to associate with the participant
**/
LINPHONE_PUBLIC void linphone_participant_set_user_data(LinphoneParticipant *participant, void *ud);

/**
 * Get the address of a conference participant.
 * @param[in] participant A #LinphoneParticipant object
 * @return The address of the participant
 */
LINPHONE_PUBLIC const LinphoneAddress * linphone_participant_get_address (const LinphoneParticipant *participant);

/**
 * Tells whether a conference participant is an administrator of the conference.
 * @param[in] participant A #LinphoneParticipant object
 * @return A boolean value telling whether the participant is an administrator
 */
LINPHONE_PUBLIC bool_t linphone_participant_is_admin (const LinphoneParticipant *participant);

/**
 * Get the security level of a chat room.
 * @param[in] cr A #LinphoneChatRoom object
 * @return The security level of the chat room
 */
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_participant_get_security_level (const LinphoneParticipant *participant);

/**
 * Gets the list of devices from a chat room's participant.
 * @param[in] participant A #LinphoneParticipant object
 * @return \bctbx_list{LinphoneParticipantDevice}
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_participant_get_devices (const LinphoneParticipant *participant);

/**
 * Find a device in the list of devices from a chat room's participant.
 * @param[in] participant A #LinphoneParticipant object
 * @param[in] address A #LinphoneAddress object
 * @return a #LinphoneParticipantDevice or NULL if not found
 */
LINPHONE_PUBLIC LinphoneParticipantDevice *linphone_participant_find_device (const LinphoneParticipant *participant, const LinphoneAddress *address);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_H_
