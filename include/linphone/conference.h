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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef LINPHONE_CONFERENCE_H
#define LINPHONE_CONFERENCE_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LinphoneConferenceClassLocal,
	LinphoneConferenceClassRemote
} LinphoneConferenceClass;

/**
 * @addtogroup call_control
 * @{
 */

/**
 * Take a reference on a #LinphoneConferencParams.
 * @param params The #LinphoneConferenceParams to ref. @notnil
 * @return The freshly refed #LinphoneConferenceParams. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_ref(LinphoneConferenceParams *params);

/**
 * Release a #LinphoneConferenceParams.
 * @param params The #LinphoneConferenceParams to release. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_params_unref(LinphoneConferenceParams *params);

/**
 * Clone a #LinphoneConferenceParams
 * @param params The #LinphoneConferenceParams to clone @notnil
 * @return An allocated #LinphoneConferenceParams with the same parameters than params @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_clone(const LinphoneConferenceParams *params);

/**
 * Enable video when starting a conference
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If true, video will be enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_video(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether video will be enable at conference starting
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the video will be enable at conference starting, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_video_enabled(const LinphoneConferenceParams *params);

/**
 * Enable local participant to enter the conference.
 * The local participant is the one driving the local #LinphoneCore. It uses the local sound devices.
 * The default value is TRUE. Setting to FALSE is mostly helpful when using liblinphone on a server application.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if local participant is automatically added to the conference, FALSE otherwise
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params, bool_t enable);

/**
 * Returns whether local participant has to enter the conference.
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if local participant is by default part of the conference, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_local_participant_enabled(const LinphoneConferenceParams *params);


/**
 * Take a reference on a #LinphoneConference.
 * @param conf The #LinphoneConference to ref. @notnil
 * @return The same #LinphoneConference object. @notnil
 */
LINPHONE_PUBLIC LinphoneConference *linphone_conference_ref(LinphoneConference *conf);

/**
 * Release a #LinphoneConference.
 * @param conf The #LinphoneConference to release. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_unref(LinphoneConference *conf);

/**
 * Get URIs of all participants of one conference
 * The returned bctbx_list_t contains URIs of all participant. That list must be
 * freed after use and each URI must be unref with linphone_address_unref()
 * @param conf A #LinphoneConference @notnil
 * @return \bctbx_list{LinphoneAddress} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_conference_get_participants(const LinphoneConference *conf);

/**
 * @param obj A #LinphoneConference @notnil
 * @param participant participant to remove
 * @warning The passed participant must be one of those returned by linphone_conference_get_participants()
 * @return 0 if succeeded, -1 if failed
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_remove_participant(LinphoneConference *obj, LinphoneParticipant *participant);

/**
 * Remove a participant from a conference
 * @param conf A #LinphoneConference @notnil
 * @param uri the #LinphoneAddress of the participant to remove @notnil
 * @warning The passed SIP URI must be one of the URIs returned by linphone_conference_get_participants()
 * @return 0 if succeeded, -1 if failed
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_remove_participant(LinphoneConference *conf, const LinphoneAddress *uri);

/**
 * Find a participant from a conference
 * @param obj A #LinphoneConference
 * @param uri SIP URI of the participant to search
 * @return a pointer to the participant found or nullptr
 */
LINPHONE_PUBLIC LinphoneParticipant * linphone_conference_find_participant(LinphoneConference *obj, const LinphoneAddress *uri);

/**
 * Invite participants to the conference, by supplying a list of #LinphoneAddress
 * @param conf The #LinphoneConference object. @notnil
 * @param addresses \bctbx_list{LinphoneAddress} @notnil
 * @param params #LinphoneCallParams to use for inviting the participants. @maybenil
**/
LINPHONE_PUBLIC LinphoneStatus linphone_conference_invite_participants(LinphoneConference *conf, const bctbx_list_t *addresses, const LinphoneCallParams *params);

/**
 * Join an existing call to the conference.
 * @param conf The #LinphoneConference object. @notnil
 * @param call a #LinphoneCall that has to be added to the conference. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participant(LinphoneConference *obj, LinphoneCall *call);

/**
 * Join a participant to the conference.
 * @param obj The #LinphoneConference object. @notnil
 * @param uri a #LinphoneAddress that has to be added to the conference. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participant_with_address (LinphoneConference *obj, const LinphoneAddress *uri);

/**
 * Update parameters of the conference.
 * This is typically used enable or disable the video stream in the conference.
 * @param conf The #LinphoneConference object. @notnil
 * @param params the new parameters to apply. @notnil
 */
LINPHONE_PUBLIC int linphone_conference_update_params(LinphoneConference *conf, const LinphoneConferenceParams *params);

/**
 * Get current parameters of the conference.
 * @param conf The #LinphoneConference object. @notnil
 * @return a #LinphoneConferenceParams . @notnil
 */
LINPHONE_PUBLIC const LinphoneConferenceParams * linphone_conference_get_current_params(const LinphoneConference *conf);

/**
 * Get the conference id as string
 * @param conf The #LinphoneConference object. @notnil
 * @return the conference id @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_get_ID(const LinphoneConference *conf);

/**
 * Set the conference id as string
 * @param conf The #LinphoneConference object. @notnil
 * @param conference_id the conference id to set. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_set_ID(LinphoneConference *conf, const char *conference_id);

/**
 * Free a #LinphoneConferenceParams
 * @param params #LinphoneConferenceParams to free @notnil
 * @deprecated 17/03/2017 Use linphone_conference_params_unref() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_params_free(LinphoneConferenceParams *params);

/**
 * Get the conference subject
 * @param obj the conference
 */
LINPHONE_PUBLIC const char *linphone_conference_get_subject(const LinphoneConference *obj);

/**
 * Set the conference subject
 * @param obj the conference
 * @param subject conference subject
 */
LINPHONE_PUBLIC void linphone_conference_set_subject(LinphoneConference *obj, const char *subject);

/**
 * Get number of participants
 * @param obj the conference
 */
LINPHONE_PUBLIC int linphone_conference_get_participant_count(const LinphoneConference *obj);

/**
 * Get the conference id
 * @param obj the conference
 */
//LINPHONE_PUBLIC ConferenceId linphone_conference_get_conference_id(const LinphoneConference *obj);

/************ */
/* DEPRECATED */
/* ********** */

int linphone_conference_terminate(LinphoneConference *obj);
int linphone_conference_enter(LinphoneConference *obj);
int linphone_conference_leave(LinphoneConference *obj);
bool_t linphone_conference_is_in(const LinphoneConference *obj);

LinphoneConferenceParams *linphone_conference_params_new(const LinphoneCore *core);

LinphoneConference *linphone_local_conference_new(LinphoneCore *core, LinphoneAddress * addr);
LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params);
LinphoneConference *linphone_remote_conference_new(LinphoneCore *core, LinphoneAddress * addr);
LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params);

int linphone_conference_add_participant_with_call(LinphoneConference *obj, LinphoneCall *call);
int linphone_conference_remove_participant_with_call(LinphoneConference *obj, LinphoneCall *call);
int linphone_conference_get_size(const LinphoneConference *obj);

/* This is actually only used by the ToneManager. TODO: encapsulate this better. */
AudioStream *linphone_conference_get_audio_stream(LinphoneConference *obj);

int linphone_conference_mute_microphone(LinphoneConference *obj, bool_t val);
bool_t linphone_conference_microphone_is_muted(const LinphoneConference *obj);
float linphone_conference_get_input_volume(const LinphoneConference *obj);

int linphone_conference_start_recording(LinphoneConference *obj, const char *path);
int linphone_conference_stop_recording(LinphoneConference *obj);

void linphone_conference_on_call_terminating(LinphoneConference *obj, LinphoneCall *call);

LINPHONE_PUBLIC bool_t linphone_conference_check_class(LinphoneConference *obj, LinphoneConferenceClass _class);



/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_CONFERENCE_H
