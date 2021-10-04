
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

#ifndef _L_C_PARTICIPANT_CBS_H_
#define _L_C_PARTICIPANT_CBS_H_

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
 * Acquire a reference to the participant  callbacks object.
 * @param[in] cbs The #LinphoneParticipantCbs object @notnil
 * @return The same participant callbacks object
 */
LINPHONE_PUBLIC LinphoneParticipantCbs * linphone_participant_cbs_ref (LinphoneParticipantCbs *cbs);

/**
 * Release reference to the participant callbacks object.
 * @param[in] cbs The #LinphoneParticipantCbs object @notnil
 */
LINPHONE_PUBLIC void linphone_participant_cbs_unref (LinphoneParticipantCbs *cbs);

/**
 * Retrieve the user pointer associated with the participant callbacks object.
 * @param[in] cbs The #LinphoneParticipantCbs object @notnil
 * @return The user pointer associated with the participant callbacks object
 */
LINPHONE_PUBLIC void * linphone_participant_cbs_get_user_data (const LinphoneParticipantCbs *cbs);

/**
 * Assign a user pointer to the participant callbacks object.
 * @param[in] cbs The #LinphoneParticipantCbs object @notnil
 * @param[in] ud The user pointer to associate with the participant callbacks object @maybenil
 */
LINPHONE_PUBLIC void linphone_participant_cbs_set_user_data (LinphoneParticipantCbs *cbs, void *ud);

/**
 * Get the is this participant speaking changed callback.
 * @param[in] cbs The #LinphoneParticipantCbs object @notnil
 * @return The current is this particiapnt speaking changed callback.
 */
LINPHONE_PUBLIC LinphoneParticipantCbsIsThisSpeakingChangedCb linphone_participant_cbs_get_is_this_speaking_changed (const LinphoneParticipantCbs *cbs);

/**
 * Set the is this participant speaking changed callback.
 * @param[in] cbs The #LinphoneParticipantCbs object @notnil
 * @param[in] cb The is this participant speaking changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_participant_cbs_set_is_this_speaking_changed (LinphoneParticipantCbs *cbs, LinphoneParticipantCbsIsThisSpeakingChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_CBS_H_

