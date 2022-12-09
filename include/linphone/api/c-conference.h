/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone 
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_C_CONFERENCE_H_
#define _L_C_CONFERENCE_H_

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
 * Add a listener in order to be notified of #LinphoneConference events. Once an event is received, registred #LinphoneConferenceCbs are
 * invoked sequencially.
 * @param conference #LinphoneConference object. @notnil
 * @param cbs A #LinphoneConferenceCbs object holding the callbacks you need. A reference is taken by the #LinphoneConference until you invoke linphone_conference_remove_callbacks(). @notnil

 */
LINPHONE_PUBLIC void linphone_conference_add_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs);

/**
 * Remove a listener from a LinphoneConference
 * @param conference #LinphoneConference object. @notnil
 * @param cbs #LinphoneConferenceCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_remove_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs);

/**
 * Gets the current LinphoneConferenceCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the LinphoneConferenceCbs that is calling the callback.
 * @param conference #LinphoneConference object. @notnil
 * @param cbs The LinphoneConferenceCbs object. @notnil
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_conference_set_current_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs);

/**
 * Sets the current LinphoneConferenceCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the LinphoneConferenceCbs that is calling the callback.
 * @param conference #LinphoneConference object. @notnil
 * @return The #LinphoneConferenceCbs that has called the last callback. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceCbs *linphone_conference_get_current_callbacks(const LinphoneConference *conference);

/**
 * Returns core for a #LinphoneConference
 * @param conference #LinphoneConference object. @notnil
 * @return back pointer to #LinphoneCore object. @notnil
 * Returns back pointer to #LinphoneCore object.
**/
LINPHONE_PUBLIC LinphoneCore* linphone_conference_get_core(const LinphoneConference *conference);

/**
 * Get the conference address of the conference.
 * @param conference A #LinphoneConference object. @notnil
 * @return The conference address of the conference. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_conference_get_conference_address (const LinphoneConference *conference);

/**
 * Set the conference address
 * @param conference The #LinphoneConference object. @notnil
 * @param address the conference address to set. @notnil
 * @warning This is only allowed for a remote conference if it is in state CreationPending or Instantiated
 */
LINPHONE_PUBLIC void linphone_conference_set_conference_address(LinphoneConference *conference, LinphoneAddress *address);

/**
 * Set the conference factory address of the conference. 
 * By default when creating a new conference, the factory address will come from the current proxy configuration.
 * If NULL then the conference will be local else it will be a remote conference.
 * @param conference The #LinphoneConference object. @notnil
 * @param address the conference factory address. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_conference_factory_address(LinphoneConferenceParams *params, const LinphoneAddress *address);

/**
 * Get the conference factory address of the conference that has been set.
 * @param conference The #LinphoneConference object. @notnil
 * @return the factory address conference description. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_conference_params_get_conference_factory_address(const LinphoneConferenceParams *params);

/**
 * Set the description of the conference
 * @param conference The #LinphoneConference object. @notnil
 * @param description the conference description. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_description(LinphoneConferenceParams *params, const char * description);

/**
 * Get conference description
 * @param conference The #LinphoneConference object. @notnil
 * @return the conference description. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_conference_params_get_description(const LinphoneConferenceParams *params);

/**
 * Set the subject of the conference
 * @param conference The #LinphoneConference object. @notnil
 * @param subject the conference subject. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_subject(LinphoneConferenceParams *params, const char * subject);

/**
 * Get conference subject
 * @param conference The #LinphoneConference object. @notnil
 * @return the conference subject. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_conference_params_get_subject(const LinphoneConferenceParams *params);

/**
 * Set the conference start time
 * @param conference The #LinphoneConference object. @notnil
 * @param start the conference start time as the number of seconds between the desired start time and the 1st of January 1970. In order to program an immediate start of a conference, then program the start time to 0
 */
LINPHONE_PUBLIC void linphone_conference_params_set_start_time(LinphoneConferenceParams *params, time_t start);

/**
 * Get the start time of the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @return start time of a conference as time_t type or 0 for immediate start of a conference. For UNIX based systems it is the number of seconds since 00:00hours of the 1st of January 1970
 */
LINPHONE_PUBLIC time_t linphone_conference_params_get_start_time(const LinphoneConferenceParams *params);

/**
 * Set the conference end time
 * @param conference The #LinphoneConference object. @notnil
 * @param end the conference end time as the number of seconds between the desired end time and the 1st of January 1970. In order to program an undefined end of a conference, then program the end time to 0
 */
LINPHONE_PUBLIC void linphone_conference_params_set_end_time(LinphoneConferenceParams *params, time_t end);

/**
 * Get the end time of the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @return end time of a conference as time_t type or 0 for open end of a conference. For UNIX based systems it is the number of seconds since 00:00hours of the 1st of January 1970
 */
LINPHONE_PUBLIC time_t linphone_conference_params_get_end_time(const LinphoneConferenceParams *params);

/**
 * Set the participant list type
 * @param conference The #LinphoneConference object. @notnil
 * @param type Participant list type #LinphoneConferenceParticipantListType. This allows to restrict the access to the conference to a selected set of participants
 */
LINPHONE_PUBLIC void linphone_conference_params_set_participant_list_type(LinphoneConferenceParams *params, LinphoneConferenceParticipantListType type);

/**
 * Get the participant list type
 * @param conference The #LinphoneConference object. @notnil
 * @return participant list type #LinphoneConferenceParticipantListType.
 */
LINPHONE_PUBLIC LinphoneConferenceParticipantListType linphone_conference_params_get_participant_list_type(const LinphoneConferenceParams *params);

/**
 * Get the current state of the conference
 * @param conference The #LinphoneConference object. @notnil
 * @return the #LinphoneConferenceState of the conference.
 */
LINPHONE_PUBLIC LinphoneConferenceState linphone_conference_get_state(const LinphoneConference *conference);

/**
 * Get the currently active speaker participant device
 * @param conference the #LinphoneConference object. @notnil
 * @return the #LinphoneParticipantDevice currently displayed as active speaker. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipantDevice* linphone_conference_get_active_speaker_participant_device(const LinphoneConference *conference);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CONFERENCE_H_
