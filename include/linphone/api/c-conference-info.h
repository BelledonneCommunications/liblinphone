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

#ifndef LINPHONE_CONFERENCE_INFO_H
#define LINPHONE_CONFERENCE_INFO_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup conference
 * @{
 */

/**
 * Create a new #LinphoneConferenceInfo object.
 * @return The newly created #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceInfo* linphone_conference_info_new(void);

/**
 * Take a reference on a #LinphoneConferenceInfo.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return the same #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceInfo* linphone_conference_info_ref(LinphoneConferenceInfo *conference_info);

/**
 * Clone an object #LinphoneConferenceInfo.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return the cloned #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceInfo* linphone_conference_info_clone(const LinphoneConferenceInfo *conference_info);

/**
 * Release a #LinphoneConferenceInfo.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_unref(LinphoneConferenceInfo *conference_info);

/**
 * Retrieve the organizer of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The #LinphoneAddress of the conference's organizer. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_conference_info_get_organizer(const LinphoneConferenceInfo *conference_info);

/**
 * Set the organizer of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param organizer The #LinphoneAddress of the conference's organizer. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_organizer(LinphoneConferenceInfo *conference_info, LinphoneAddress *organizer);

/**
 * Retrieve the list of participants.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The list of participants. \bctbx_list{LinphoneAddress} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_conference_info_get_participants(const LinphoneConferenceInfo *conference_info);

/**
 * Set the list of participants.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participants The list of participants to set. \bctbx_list{LinphoneAddress} @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_participants(LinphoneConferenceInfo *conference_info, const bctbx_list_t *participants);

/**
 * Add a participant to the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant The participant (#LinphoneAddress) to add. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_add_participant(LinphoneConferenceInfo *conference_info, LinphoneAddress *participant);

/**
 * Remove a participant from the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant The participant (#LinphoneAddress) to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_remove_participant(LinphoneConferenceInfo *conference_info, LinphoneAddress *participant);

/**
 * Retrieve the URI of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The URI of the conference (#LinphoneAddress). @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_conference_info_get_uri(const LinphoneConferenceInfo *conference_info);

/**
 * Retrieve the date and time of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The date and time of the conference.
 */
LINPHONE_PUBLIC time_t linphone_conference_info_get_date_time(const LinphoneConferenceInfo *conference_info);

/**
 * Set the date and time of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param datetime The date and time of the conference.
 */
LINPHONE_PUBLIC void linphone_conference_info_set_date_time(LinphoneConferenceInfo *conference_info, time_t datetime);

/**
 * Retrieve the duration (in minutes) of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The duration of the conference.
 */
LINPHONE_PUBLIC unsigned int linphone_conference_info_get_duration(const LinphoneConferenceInfo *conference_info);

/**
 * Set the duration (in minutes) of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param duration The duration of the conference.
 */
LINPHONE_PUBLIC void linphone_conference_info_set_duration(LinphoneConferenceInfo *conference_info, unsigned int duration);

/**
 * Retrieve the subject of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The subject of the conference. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_info_get_subject(const LinphoneConferenceInfo *conference_info);

/**
 * Set the subject of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param subject The subject of the conference. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_subject(LinphoneConferenceInfo *conference_info, const char *subject);

/**
 * Retrieve the description of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The description of the conference. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_info_get_description(const LinphoneConferenceInfo *conference_info);

/**
 * Set the description of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param description The description of the conference. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_description(LinphoneConferenceInfo *conference_info, const char *description);

/**
 * Retrieve the conference as an Icalendar string.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The conference as an Icalendar string. The returned char* must be freed by the caller. @maybenil @tobefreed
 */
LINPHONE_PUBLIC char *linphone_conference_info_get_icalendar_string(const LinphoneConferenceInfo *conference_info);

/**
 * Retrieve the state of the conference info.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return #LinphoneConferenceInfoState object. @maybenil
 */
LINPHONE_PUBLIC LinphoneConferenceInfoState linphone_conference_info_get_state(const LinphoneConferenceInfo *conference_info);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_CONFERENCE_INFO_H */
