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
 * Create an object of type a #LinphoneConferenceParams.
 * @param core The #LinphoneCore to pass to the constructor. @notnil
 * @return The created #LinphoneConferenceParams. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_new(const LinphoneCore *core);

/**
 * Take a reference on a #LinphoneConferenceParams.
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
 * Free a #LinphoneConferenceParams
 * @param params #LinphoneConferenceParams to free @notnil
 * @deprecated 17/03/2017 Use linphone_conference_params_unref() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_params_free(LinphoneConferenceParams *params);

/**
 * Clone a #LinphoneConferenceParams
 * @param params The #LinphoneConferenceParams to clone @notnil
 * @return An allocated #LinphoneConferenceParams with the same parameters than params @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_clone(const LinphoneConferenceParams *params);

/**
 * Enable audio capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, audio will be enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_set_audio_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether audio capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports audio capabilities, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_is_audio_enabled(const LinphoneConferenceParams *params);

/**
 * Enable video capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, video will be enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_set_video_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether video capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports video capabilities, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_is_video_enabled(const LinphoneConferenceParams *params);

/**
 * Enable chat capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, chat is enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_set_chat_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether chat capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports chat capabilities, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_is_chat_enabled(const LinphoneConferenceParams *params);

/**
 * Enable local participant to enter the conference.
 * The local participant is the one driving the local #LinphoneCore. It uses the local sound devices.
 * The default value is TRUE. Setting to FALSE is mostly helpful when using liblinphone on a server application.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if local participant is automatically added to the conference, FALSE otherwise
 */
LINPHONE_PUBLIC void linphone_conference_params_set_local_participant_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Returns whether local participant has to enter the conference.
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if local participant is by default part of the conference, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_is_local_participant_enabled(const LinphoneConferenceParams *params);

/**
 * Enable conference with one participant.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if conference can have only one participant, FALSE otherwise
 */
LINPHONE_PUBLIC void linphone_conference_params_set_one_participant_conference_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Returns whether conference can have only one participant
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference can have only one participant, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_is_one_participant_conference_enabled(const LinphoneConferenceParams *params);


/**
 * Take a reference on a #LinphoneConference.
 * @param conference The #LinphoneConference to ref. @notnil
 * @return The same #LinphoneConference object. @notnil
 */
LINPHONE_PUBLIC LinphoneConference *linphone_conference_ref(LinphoneConference *conference);

/**
 * Release a #LinphoneConference.
 * @param conference The #LinphoneConference to release. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_unref(LinphoneConference *conference);

/**
 * Get URIs of all participants of one conference
 * The returned bctbx_list_t contains URIs of all participant. That list must be
 * freed after use and each URI must be unref with linphone_address_unref()
 * @param conference A #LinphoneConference @notnil
 * @return The list of the participants' address active in the conference. \bctbx_list{LinphoneAddress} @maybenil
 * @deprecated 10/07/2020 Use linphone_conference_get_participant_list() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bctbx_list_t *linphone_conference_get_participants(const LinphoneConference *conference);

/**
 * Get list of all participants of one conference
 * @param conference A #LinphoneConference @notnil
 * @return The list of participants of the conference. \bctbx_list{LinphoneParticipant} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_conference_get_participant_list(const LinphoneConference *conference);

/**
 * @param conference A #LinphoneConference @notnil
 * @param uri URI of the participant to remove @notnil
 * @warning The passed participant uri must be one of those returned by linphone_conference_get_participants()
 * @return 0 if succeeded, -1 if failed
 * @deprecated 10/07/2020 Use linphone_conference_remove_participant_2() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneStatus linphone_conference_remove_participant(LinphoneConference *conference, const LinphoneAddress *uri);

/**
 * @param conference A #LinphoneConference @notnil
 * @param participant participant to remove @notnil
 * @warning The passed participant must be one of those returned by linphone_conference_get_participant_list()
 * @warning This method may destroy the conference if the only remaining participant had an existing call to the local participant before the conference was created
 * @return 0 if succeeded, -1 if failed
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_remove_participant_2(LinphoneConference *conference, LinphoneParticipant *participant);

/**
 * @param conference A #LinphoneConference @notnil
 * @param call call to remove @notnil
 * @return 0 if succeeded, -1 if failed
 * @deprecated 10/07/2020 Use linphone_conference_remove_participant_2() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneStatus linphone_conference_remove_participant_3(LinphoneConference *conference, LinphoneCall *call);

/**
 * Find a participant from a conference
 * @param conference A #LinphoneConference. @notnil
 * @param uri SIP URI of the participant to search. @notnil
 * @return a pointer to the participant found or nullptr. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipant * linphone_conference_find_participant(LinphoneConference *conference, const LinphoneAddress *uri);

/**
 * Invite participants to the conference, by supplying a list of #LinphoneAddress
 * @param conference The #LinphoneConference object. @notnil
 * @param addresses A list of SIP addresses to invite. @bctbx_list{LinphoneAddress} @notnil
 * @param params #LinphoneCallParams to use for inviting the participants. @maybenil
**/
LINPHONE_PUBLIC LinphoneStatus linphone_conference_invite_participants(LinphoneConference *conference, const bctbx_list_t *addresses, const LinphoneCallParams *params);

/**
 * Join an existing call to the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @param call a #LinphoneCall that has to be added to the conference. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participant(LinphoneConference *conference, LinphoneCall *call);

/**
 * Join a participant to the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @param uri a #LinphoneAddress that has to be added to the conference. @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participant_2 (LinphoneConference *conference, const LinphoneAddress *uri);

/**
 * Update parameters of the conference.
 * This is typically used enable or disable the video stream in the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @param params the new parameters to apply. @notnil
 */
LINPHONE_PUBLIC int linphone_conference_update_params(LinphoneConference *conference, const LinphoneConferenceParams *params);

/**
 * Get current parameters of the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @return a #LinphoneConferenceParams . @notnil
 */
LINPHONE_PUBLIC const LinphoneConferenceParams * linphone_conference_get_current_params(const LinphoneConference *conference);

/**
 * Get the conference subject
 * @param conference The #LinphoneConference object. @notnil
 * @return conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_get_subject(const LinphoneConference *conference);

/**
 * Set the conference subject
 * @param conference The #LinphoneConference object. @notnil
 * @param subject conference subject @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_set_subject(LinphoneConference *conference, const char *subject);

/**
 * Get number of participants without me
 * @param conference The #LinphoneConference object. @notnil
 * @return the number of participants excluding me in a #LinphoneConference
 */
LINPHONE_PUBLIC int linphone_conference_get_participant_count(const LinphoneConference *conference);

/**
 * For a local audio video conference, this function returns the participant hosting the conference
 * For a remote audio video conference, this function returns the focus of the conference
 * @param conference The #LinphoneConference object. @notnil
 * @return a #LinphoneParticipant . @notnil
 */
LINPHONE_PUBLIC LinphoneParticipant *linphone_conference_get_me(const LinphoneConference *conference);

/**
 * Terminates conference
 * @param conference The #LinphoneConference object. @notnil
 * @return 0 if the termination is successful, -1 otherwise.
 */
LINPHONE_PUBLIC int linphone_conference_terminate(LinphoneConference *conference);

/**
 * Retrieves the user pointer that was given to linphone_conference_new()
 * @param conference #LinphoneConference object @notnil
 * @return The user data associated with the #LinphoneConference object. @maybenil
 * @ingroup initializing
**/
LINPHONE_PUBLIC void *linphone_conference_get_user_data(const LinphoneConference *conference);

/**
 * Associate a user pointer to the linphone conference.
 * @param core #LinphoneConference object @notnil
 * @param user_data The user data to associate with the #LinphoneConference object. @maybenil
 * @ingroup initializing
**/
LINPHONE_PUBLIC void linphone_conference_set_user_data(LinphoneConference *conference, void *user_data);

/**
 * Call generic OpenGL render preview for a given conference
 * @param conference The #LinphoneConference object. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_preview_ogl_render(LinphoneConference *conference);

/**
 * Call generic OpenGL render for a given conference
 * @param conference The #LinphoneConference object. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_ogl_render(LinphoneConference *conference);

/**
 * Change the admin status of a participant of a conference (you need to be an admin yourself to do this).
 * @param conference A #LinphoneConference object @notnil
 * @param participant The Participant for which to change the admin status @notnil
 * @param is_admin A boolean value telling whether the participant should now be an admin or not
 */
LINPHONE_PUBLIC void linphone_conference_set_participant_admin_status (LinphoneConference *conference, LinphoneParticipant *participant, bool_t is_admin);

/**
 * For a local conference, the local participant joins the conference
 * For a remote conference, the participant rejoins the conference after leaving it earlier on
 * @param conference A #LinphoneConference object @notnil
 * @return 0 if succeeded. Negative number if failed
 */
LINPHONE_PUBLIC int linphone_conference_enter(LinphoneConference *conference);

/**
 * For a local conference, the local participant leaves the conference
 * For a remote conference, the participant leaves the conference after joining it earlier on
 * @param conference A #LinphoneConference object @notnil
 * @return 0 if succeeded. Negative number if failed
 */
LINPHONE_PUBLIC int linphone_conference_leave(LinphoneConference *conference);

/**
 * For a local audio video conference, this function compares the address provided as argument with that of participant hosting the conference
 * For a remote audio video conference, this function compares the address provided as argument with that of the focus of the conference
 * @param conference A #LinphoneConference object @notnil
 * @param uri A #LinphoneAddress object @notnil
 * @return TRUE if the participant is me, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_conference_is_me(const LinphoneConference *conference, const LinphoneAddress * uri);

/**
 * For a local conference, it returns whether the local participant is enabled
 * For a remote conference, it return whether the remote participant has left the conference without bein removed from it
 * @param conference A #LinphoneConference object @notnil
 * @return TRUE if the local participant is in a conference, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_conference_is_in(const LinphoneConference *conference);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Get the conference id as string
 * @param conference The #LinphoneConference object. @notnil
 * @return the conference id @notnil
 * @deprecated 10/07/2020 Use linphone_conference_get_conference_address() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_conference_get_ID(const LinphoneConference *conference);

/**
 * Set the conference id as string
 * @param conference The #LinphoneConference object. @notnil
 * @param conference_id the conference id to set. @notnil
 * @deprecated 10/07/2020 Use linphone_conference_set_conference_address() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_set_ID(LinphoneConference *conference, const char *conference_id);

LinphoneConference *linphone_local_conference_new(LinphoneCore *core, LinphoneAddress * addr);
LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params);
LinphoneConference *linphone_remote_conference_new(LinphoneCore *core, LinphoneAddress * addr);
LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core, LinphoneAddress * focus, LinphoneAddress * addr, const LinphoneConferenceParams *params);


/* This is actually only used by the ToneManager. TODO: encapsulate this better. */
AudioStream *linphone_conference_get_audio_stream(LinphoneConference *conference);

int linphone_conference_mute_microphone(LinphoneConference *conference, bool_t val);
bool_t linphone_conference_microphone_is_muted(const LinphoneConference *conference);
float linphone_conference_get_input_volume(const LinphoneConference *conference);

int linphone_conference_start_recording(LinphoneConference *conference, const char *path);
int linphone_conference_stop_recording(LinphoneConference *conference);

void linphone_conference_on_call_terminating(LinphoneConference *conference, LinphoneCall *call);

LINPHONE_PUBLIC bool_t linphone_conference_check_class(LinphoneConference *conference, LinphoneConferenceClass _class);



/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_CONFERENCE_H
