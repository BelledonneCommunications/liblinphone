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
LINPHONE_PUBLIC LinphoneConferenceInfo *linphone_conference_info_new(void);

/**
 * Take a reference on a #LinphoneConferenceInfo.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return the same #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceInfo *linphone_conference_info_ref(LinphoneConferenceInfo *conference_info);

/**
 * Clone an object #LinphoneConferenceInfo.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return the cloned #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceInfo *linphone_conference_info_clone(const LinphoneConferenceInfo *conference_info);

/**
 * Release a #LinphoneConferenceInfo.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_unref(LinphoneConferenceInfo *conference_info);

/**
 * Retrieve the organizer of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The #LinphoneParticipantInfo of the conference's organizer. @maybenil
 */
LINPHONE_PUBLIC const LinphoneParticipantInfo *
linphone_conference_info_get_organizer_info(const LinphoneConferenceInfo *conference_info);

/**
 * Retrieve the organizer of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The #LinphoneAddress of the conference's organizer. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_conference_info_get_organizer(const LinphoneConferenceInfo *conference_info);

/**
 * Set the organizer of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param organizer The #LinphoneAddress of the conference's organizer. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_organizer(LinphoneConferenceInfo *conference_info,
                                                            const LinphoneAddress *organizer);

/**
 * Retrieve the list of participants as list of addresses.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The list of participants. \bctbx_list{LinphoneAddress} @maybenil
 * @deprecated 24/08/2023 use linphone_conference_info_get_participant_infos instead
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const bctbx_list_t *
linphone_conference_info_get_participants(const LinphoneConferenceInfo *conference_info);

/**
 * Retrieve the list of participants as list of participant infos.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The list of participant informations. \bctbx_list{LinphoneParticipantInfo} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t *
linphone_conference_info_get_participant_infos(const LinphoneConferenceInfo *conference_info);

/**
 * Set the list of participants.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participants The list of participants to set. \bctbx_list{LinphoneAddress} @maybenil
 * @deprecated 24/08/2023 use linphone_conference_info_set_participant_infos instead
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void
linphone_conference_info_set_participants(LinphoneConferenceInfo *conference_info, const bctbx_list_t *participants);

/**
 * Set the list of participants.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant_infos The list of participant informations to set. \bctbx_list{LinphoneParticipantInfo} @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_participant_infos(LinphoneConferenceInfo *conference_info,
                                                                    const bctbx_list_t *participant_infos);

/**
 * Add a list of participants.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant_infos The list of participant informations to add. \bctbx_list{LinphoneParticipantInfo} @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_add_participant_infos(LinphoneConferenceInfo *conference_info,
                                                                    const bctbx_list_t *participant_infos);

/**
 * Add a participant to the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant The participant (#LinphoneAddress) to add. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_add_participant(LinphoneConferenceInfo *conference_info,
                                                              const LinphoneAddress *participant);

/**
 * Add a participant to the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant_info The participant information (#LinphoneParticipantInfo) to add. This method can be called to
 * set attributes such as the role to the organizer of the conference @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_add_participant_2(LinphoneConferenceInfo *conference_info,
                                                                const LinphoneParticipantInfo *participant_info);

/**
 * Update the participant information in the conference informations
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant_info The participant information (#LinphoneParticipantInfo) to update. This method can be called
 * to change attributes such as the role to the organizer of the conference @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_update_participant(LinphoneConferenceInfo *conference_info,
                                                                 const LinphoneParticipantInfo *participant_info);

/**
 * Remove a participant from the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant The participant (#LinphoneAddress) to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_info_remove_participant(LinphoneConferenceInfo *conference_info,
                                                                 const LinphoneAddress *participant);

/**
 * Find a participant information in the conference information.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param participant The participant (#LinphoneAddress) to search. @notnil
 * @return The participant information (#LinphoneParticipantInfo). @maybenil
 */
LINPHONE_PUBLIC const LinphoneParticipantInfo *
linphone_conference_info_find_participant(LinphoneConferenceInfo *conference_info, const LinphoneAddress *participant);

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
LINPHONE_PUBLIC void linphone_conference_info_set_duration(LinphoneConferenceInfo *conference_info,
                                                           unsigned int duration);

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
 * Retrieve the subject of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The subject of the conference. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_info_get_subject_utf8(const LinphoneConferenceInfo *conference_info);

/**
 * Set the subject of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param subject The subject of the conference. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_subject_utf8(LinphoneConferenceInfo *conference_info,
                                                               const char *subject);

/**
 * Set the CCMP URI of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param uri The URI of the conference in the CCMP server. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_ccmp_uri(LinphoneConferenceInfo *conference_info, const char *uri);

/**
 * Retrieve the CCMP URI of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The URI of the conference stored in the CCMP server. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_info_get_ccmp_uri(const LinphoneConferenceInfo *conference_info);

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
LINPHONE_PUBLIC void linphone_conference_info_set_description(LinphoneConferenceInfo *conference_info,
                                                              const char *description);

/**
 * Retrieve the description of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The description of the conference. @maybenil
 */
LINPHONE_PUBLIC const char *
linphone_conference_info_get_description_utf8(const LinphoneConferenceInfo *conference_info);

/**
 * Set the description of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param description The description of the conference. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_description_utf8(LinphoneConferenceInfo *conference_info,
                                                                   const char *description);

/**
 * Retrieve the desired security level of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return The desired security level of the conference.
 */
LINPHONE_PUBLIC LinphoneConferenceSecurityLevel
linphone_conference_info_get_security_level(const LinphoneConferenceInfo *conference_info);

/**
 * Set the desired security level of the conference.
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param security_level The desired security level of the conference.
 */
LINPHONE_PUBLIC void linphone_conference_info_set_security_level(LinphoneConferenceInfo *conference_info,
                                                                 LinphoneConferenceSecurityLevel security_level);

/**
 * Set the capability of the conference.
 * The capability information represents the capability for the conference linked to the #ConferenceInfo to handle a
 * given stream type (audio, video or text).
 * @param conference_info A #LinphoneConferenceInfo object @notnil
 * @param stream_type A #LinphoneStreamType
 * @param enable the capability of the conference linked to conference information #LinphoneConferenceInfo
 */
LINPHONE_PUBLIC void linphone_conference_info_set_capability(LinphoneConferenceInfo *conference_info,
                                                             const LinphoneStreamType stream_type,
                                                             bool_t enable);

/**
 * Get the capability of the conference.
 * The capability information represents the capability for the conference linked to the #ConferenceInfo to handle a
 * given stream type (audio, video or text).
 * @param conference_info A #LinphoneConferenceInfo object @notnil
 * @param stream_type A #LinphoneStreamType
 * @return the capability of the conference linked to conference information #LinphoneConferenceInfo
 */
LINPHONE_PUBLIC bool_t linphone_conference_info_get_capability(const LinphoneConferenceInfo *conference_info,
                                                               const LinphoneStreamType stream_type);

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
LINPHONE_PUBLIC LinphoneConferenceInfoState
linphone_conference_info_get_state(const LinphoneConferenceInfo *conference_info);

/**
 * Store the ICS UID in the conference info
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @param uid the ICS UID to be associated to the #LinphoneConferenceInfo object. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_info_set_ics_uid(LinphoneConferenceInfo *conference_info, const char *uid);

/**
 * Retrieve the ICS UID linked to a conference info
 * @param conference_info The #LinphoneConferenceInfo object. @notnil
 * @return the ICS UID. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_info_get_ics_uid(const LinphoneConferenceInfo *conference_info);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_CONFERENCE_INFO_H */
