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

#ifndef _L_C_PARTICIPANT_IMDN_STATE_H_
#define _L_C_PARTICIPANT_IMDN_STATE_H_

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
 * Increment reference count of LinphoneParticipantImdnState object.
 * @param state the #LinphoneParticipantImdnState object @notnil
 * @return the same #LinphoneParticipantImdnState object @notnil
 **/
LINPHONE_PUBLIC LinphoneParticipantImdnState *linphone_participant_imdn_state_ref (LinphoneParticipantImdnState *state);

/**
 * Decrement reference count of LinphoneParticipantImdnState object.
 * @param state the #LinphoneParticipantImdnState object @notnil
 **/
LINPHONE_PUBLIC void linphone_participant_imdn_state_unref (LinphoneParticipantImdnState *state);

/**
 * Retrieve the user pointer associated with a LinphoneParticipantImdnState.
 * @param state A LinphoneParticipantImdnState object @notnil
 * @return The user pointer associated with the LinphoneParticipantImdnState. @maybenil
**/
LINPHONE_PUBLIC void *linphone_participant_imdn_state_get_user_data(const LinphoneParticipantImdnState *state);

/**
 * Assign a user pointer to a LinphoneParticipantImdnState.
 * @param state A LinphoneParticipantImdnState object @notnil
 * @param user_data The user pointer to associate with the LinphoneParticipantImdnState. @maybenil
**/
LINPHONE_PUBLIC void linphone_participant_imdn_state_set_user_data(LinphoneParticipantImdnState *state, void *user_data);

/**
 * Get the participant concerned by a LinphoneParticipantImdnState.
 * @param state A LinphoneParticipantImdnState object @notnil
 * @return The #LinphoneParticipant concerned by the LinphoneParticipantImdnState @notnil
 */
LINPHONE_PUBLIC const LinphoneParticipant *linphone_participant_imdn_state_get_participant (
	const LinphoneParticipantImdnState *state
);

/**
 * Get the chat message state the participant is in.
 * @param state A LinphoneParticipantImdnState object @notnil
 * @return The #LinphoneChatMessageState the participant is in
 */
LINPHONE_PUBLIC LinphoneChatMessageState linphone_participant_imdn_state_get_state (const LinphoneParticipantImdnState *state);

/**
 * Get the timestamp at which a participant has reached the state described by a LinphoneParticipantImdnState.
 * @param state A LinphoneParticipantImdnState object @notnil
 * @return The timestamp at which the participant has reached the state described in the LinphoneParticipantImdnState
 */
LINPHONE_PUBLIC time_t linphone_participant_imdn_state_get_state_change_time (const LinphoneParticipantImdnState *state);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_IMDN_STATE_H_
