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

#ifndef _L_C_PARTICIPANT_H_
#define _L_C_PARTICIPANT_H_

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
 * Increment reference count of #LinphoneParticipant object.
 * @param participant a #LinphoneParticipant object @notnil
 * @return the same #LinphoneParticipant object @notnil
 **/
LINPHONE_PUBLIC LinphoneParticipant *linphone_participant_ref (LinphoneParticipant *participant);

/**
 * Decrement reference count of #LinphoneParticipant object.
 * @param participant a #LinphoneParticipant object @notnil
 **/
LINPHONE_PUBLIC void linphone_participant_unref (LinphoneParticipant *participant);

/**
 * Retrieve the user pointer associated with the conference participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @return The user pointer associated with the participant. @maybenil
**/
LINPHONE_PUBLIC void * linphone_participant_get_user_data(const LinphoneParticipant *participant);

/**
 * Assign a user pointer to the conference participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @param user_data The user pointer to associate with the participant. @maybenil
**/
LINPHONE_PUBLIC void linphone_participant_set_user_data(LinphoneParticipant *participant, void *user_data);

/**
 * Get the address of a conference participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @return The #LinphoneAddress of the participant @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress * linphone_participant_get_address (const LinphoneParticipant *participant);

/**
 * Tells whether a conference participant is an administrator of the conference.
 * @param participant A #LinphoneParticipant object
 * @return A boolean value telling whether the participant is an administrator
 */
LINPHONE_PUBLIC bool_t linphone_participant_is_admin (const LinphoneParticipant *participant);

/**
 * Tells whether a conference participant is the focus of the conference.
 * @param participant A #LinphoneParticipant object
 * @return A boolean value telling whether the participant is a focus of a conference
 */
LINPHONE_PUBLIC bool_t linphone_participant_is_focus (const LinphoneParticipant *participant);

/**
 * Get the security level of a participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @return The #LinphoneChatRoomSecurityLevel of the participant
 */
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_participant_get_security_level (const LinphoneParticipant *participant);

/**
 * Gets the list of devices from a chat room's participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @return List of devices. \bctbx_list{LinphoneParticipantDevice} @notnil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_participant_get_devices (const LinphoneParticipant *participant);

/**
 * Find a device in the list of devices from a chat room's participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @param address A #LinphoneAddress object @notnil
 * @return a #LinphoneParticipantDevice or NULL if not found. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipantDevice *linphone_participant_find_device (const LinphoneParticipant *participant, const LinphoneAddress *address);

/**
 * Get the timestamp of the creation of the participant.
 * @param participant A #LinphoneParticipant object @notnil
 * @return time of creation of the participant as returned by time(nullptr). For UNIX based systems it is the number of seconds since 00:00hours of the 1st of January 1970
 */
LINPHONE_PUBLIC time_t linphone_participant_get_creation_time(const LinphoneParticipant *participant);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_H_
