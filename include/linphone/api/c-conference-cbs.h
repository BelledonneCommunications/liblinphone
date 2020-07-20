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

#ifndef _L_C_CONFERENCE_CBS_H_
#define _L_C_CONFERENCE_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup conferencing
 * @{
 */

/**
 * Acquire a reference to the conference callbacks object.
 * @param[in] cbs The conference callbacks object
 * @return The same conference callbacks object
**/
LINPHONE_PUBLIC LinphoneConferenceCbs * linphone_conference_cbs_ref (LinphoneConferenceCbs *cbs);

/**
 * Release reference to the conference callbacks object.
 * @param[in] cr The conference callbacks object
**/
LINPHONE_PUBLIC void linphone_conference_cbs_unref (LinphoneConferenceCbs *cbs);

/**
 * Retrieve the user pointer associated with the conference callbacks object.
 * @param[in] cr The conference callbacks object
 * @return The user pointer associated with the conference callbacks object
**/
LINPHONE_PUBLIC void * linphone_conference_cbs_get_user_data (const LinphoneConferenceCbs *cbs);

/**
 * Assign a user pointer to the conference callbacks object.
 * @param[in] cr The conference callbacks object
 * @param[in] ud The user pointer to associate with the conference callbacks object
**/
LINPHONE_PUBLIC void linphone_conference_cbs_set_user_data (LinphoneConferenceCbs *cbs, void *ud);

/**
 * Get the participant added callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current participant added callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsParticipantAddedCb linphone_conference_cbs_get_participant_added (const LinphoneConferenceCbs *cbs);

/**
 * Set the participant added callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The participant added callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_participant_added (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAddedCb cb);

/**
 * Get the participant removed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current participant removed callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsParticipantRemovedCb linphone_conference_cbs_get_participant_removed (const LinphoneConferenceCbs *cbs);

/**
 * Set the participant removed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The participant removed callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_participant_removed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantRemovedCb cb);

/**
 * Get the participant device added callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current participant device added callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsParticipantDeviceAddedCb linphone_conference_cbs_get_participant_device_added (const LinphoneConferenceCbs *cbs);

/**
 * Set the participant device added callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The participant device added callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_participant_device_added (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceAddedCb cb);

/**
 * Get the participant device removed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current participant device removed callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsParticipantDeviceRemovedCb linphone_conference_cbs_get_participant_device_removed (const LinphoneConferenceCbs *cbs);

/**
 * Set the participant device removed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The participant device removed callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_participant_device_removed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceRemovedCb cb);

/**
 * Get the participant admin status changed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current participant admin status changed callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsParticipantAdminStatusChangedCb linphone_conference_cbs_get_participant_admin_status_changed (const LinphoneConferenceCbs *cbs);

/**
 * Set the participant admin status changed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The participant admin status changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_participant_admin_status_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAdminStatusChangedCb cb);

/**
 * Get the state changed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current state changed callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsStateChangedCb linphone_conference_cbs_get_state_changed (const LinphoneConferenceCbs *cbs);

/**
 * Set the state changed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_state_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsStateChangedCb cb);

/**
 * Get the subject changed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @return The current subject changed callback.
 */
LINPHONE_PUBLIC LinphoneConferenceCbsSubjectChangedCb linphone_conference_cbs_get_subject_changed (const LinphoneConferenceCbs *cbs);

/**
 * Set the subject changed callback.
 * @param[in] cbs #LinphoneConferenceCbs object.
 * @param[in] cb The subject changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_conference_cbs_set_subject_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsSubjectChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CONFERENCE_CBS_H_
