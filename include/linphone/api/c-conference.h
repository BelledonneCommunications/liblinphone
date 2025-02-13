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

#include "mediastreamer2/mediastream.h"

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

typedef enum { LinphoneConferenceClassLocal, LinphoneConferenceClassRemote } LinphoneConferenceClass;

/**
 * @addtogroup conference
 * @{
 */

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
 * The returned bctbx_list_t contains URIs of all participants. That list must be
 * freed after use and each URI must be unref with linphone_address_unref()
 * @warning The returned list does not include me.
 * @param conference A #LinphoneConference @notnil
 * @return The list of the participants' address active in the conference. \bctbx_list{LinphoneAddress} @maybenil
 * @tobefreed
 * @deprecated 10/07/2020 Use linphone_conference_get_participant_list() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bctbx_list_t *
linphone_conference_get_participants(const LinphoneConference *conference);

/**
 * Get list of all participants of a conference
 * @warning The returned list does not include me.
 * @param conference A #LinphoneConference @notnil
 * @return The list of participants of the conference. \bctbx_list{LinphoneParticipant} @maybenil @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_conference_get_participant_list(const LinphoneConference *conference);

/**
 * Get list of all participant devices of a conference including me if it is in
 * @param conference A #LinphoneConference @notnil
 * @return The list of participant devices of the conference. \bctbx_list{LinphoneParticipantDevice} @maybenil
 * @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_conference_get_participant_device_list(const LinphoneConference *conference);

/**
 * Get the participant that is currently screen sharing
 * @param conference A #LinphoneConference. @notnil
 * @return a pointer to the participant found or nullptr. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipant *
linphone_conference_get_screen_sharing_participant(const LinphoneConference *conference);

/**
 * Get the participant device that is currently screen sharing
 * @param conference A #LinphoneConference. @notnil
 * @return a pointer to the participant device found or nullptr. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipantDevice *
linphone_conference_get_screen_sharing_participant_device(const LinphoneConference *conference);

/**
 * @param conference A #LinphoneConference @notnil
 * @param uri URI of the participant to remove @notnil
 * @warning The passed participant uri must be one of those returned by linphone_conference_get_participants()
 * @return 0 if succeeded, -1 if failed
 * @deprecated 10/07/2020 Use linphone_conference_remove_participant_2() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneStatus
linphone_conference_remove_participant(LinphoneConference *conference, const LinphoneAddress *uri);

/**
 * @param conference A #LinphoneConference @notnil
 * @param participant participant to remove @notnil
 * @warning The passed participant must be one of those returned by linphone_conference_get_participant_list()
 * @warning This method may destroy the conference if the only remaining participant had an existing call to the local
 * participant before the conference was created
 * @return 0 if succeeded, -1 if failed
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_remove_participant_2(LinphoneConference *conference,
                                                                        LinphoneParticipant *participant);

/**
 * @param conference A #LinphoneConference @notnil
 * @param call call to remove @notnil
 * @return 0 if succeeded, -1 if failed
 * @deprecated 10/07/2020 Use linphone_conference_remove_participant_2() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneStatus
linphone_conference_remove_participant_3(LinphoneConference *conference, LinphoneCall *call);

/**
 * Find a participant from a conference
 * @param conference A #LinphoneConference. @notnil
 * @param uri SIP URI of the participant to search. @notnil
 * @return a pointer to the participant found or nullptr. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipant *linphone_conference_find_participant(LinphoneConference *conference,
                                                                          const LinphoneAddress *uri);

/**
 * Invite participants to the conference, by supplying a list of #LinphoneAddress
 * If the conference is in the state LinphoneConferenceStateCreationPending, then the conference will start on the input
 * and output audio devices used for the currently active call, if any
 * This method will call every address supplied and add it to a conference on devices hosting a conference. On the other
 *hand, if the conference is created on a server, this method will allow to create either a chat only conference or an
 *ad-hoc audio video conference
 * @param conference The #LinphoneConference object. @notnil
 * @param addresses A list of SIP addresses to invite. @bctbx_list{LinphoneAddress} @notnil
 * @param params #LinphoneCallParams to use for inviting the participants. @maybenil
 * @warning The #LinphoneCallParams are only honored by conference servers and devices that host the conference locally.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_conference_invite_participants(LinphoneConference *conference,
                                                                       const bctbx_list_t *addresses,
                                                                       const LinphoneCallParams *params);

/**
 * Add participants to the conference, by supplying a list of #LinphoneCall. If the conference is in the state
 *LinphoneConferenceStateCreationPending, then the conference will start on the input and output audio devices used for
 *the currently active call, if any
 * @param conference The #LinphoneConference object. @notnil
 * @param calls A list of calls to add to the conference. @bctbx_list{LinphoneCall} @notnil
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participants(LinphoneConference *conference,
                                                                    const bctbx_list_t *calls);

/**
 * Add participants to the conference, by supplying a list of #LinphoneAddress.
 * @param conference The #LinphoneConference object. @notnil
 * @param addresses A list of calls to add to the conference. @bctbx_list{LinphoneAddress} @notnil
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participants_2(LinphoneConference *conference,
                                                                      const bctbx_list_t *addresses);

/**
 * Join an existing call to the conference.
 * If the conference is in the state LinphoneConferenceStateCreationPending, then the conference will start on the input
 * and output audio devices used for the currently active call, if any
 * @param conference The #LinphoneConference object. @notnil
 * @param call a #LinphoneCall that has to be added to the conference. @notnil
 * @warning This function guarantees that the local endpoint is added to the conference only if one of calls added is in
 * state StreamsRunning. It is highly recommended to call linphone_confererence_enter() to guarantee that the local
 * endpoint is added to the conference.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participant(LinphoneConference *conference, LinphoneCall *call);

/**
 * Join a participant to the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @param uri a #LinphoneAddress that has to be added to the conference. @notnil
 * @warning This function guarantees that the local endpoint is added to the conference only if there is a call state
 * StreamsRunning towards one of the addresses. It is highly recommended to call linphone_confererence_enter() to
 * guarantee that the local endpoint is added to the conference.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_conference_add_participant_2(LinphoneConference *conference,
                                                                     const LinphoneAddress *uri);

/**
 * Returns the conference identifier
 * @warning This method returns a NULL pointer if the Conference is in the Instantiated state
 * @param conference The #LinphoneConference object. @notnil
 * @return the conference identifier. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_get_identifier(const LinphoneConference *conference);

/**
 * Update parameters of the conference.
 * This is typically used enable or disable the video stream in the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @param params the new parameters to apply. @notnil
 */
LINPHONE_PUBLIC int linphone_conference_update_params(LinphoneConference *conference,
                                                      const LinphoneConferenceParams *params);

/**
 * Get current parameters of the conference.
 * @param conference The #LinphoneConference object. @notnil
 * @return a #LinphoneConferenceParams . @notnil
 */
LINPHONE_PUBLIC const LinphoneConferenceParams *
linphone_conference_get_current_params(const LinphoneConference *conference);

/**
 * Get the conference subject
 * @param conference The #LinphoneConference object. @notnil
 * @return conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_get_subject(const LinphoneConference *conference);

/**
 * Get the conference subject as an UTF-8 string.
 * @param conference The #LinphoneConference object. @notnil
 * @return conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_get_subject_utf8(const LinphoneConference *conference);

/**
 * Set the conference subject
 * @param conference The #LinphoneConference object. @notnil
 * @param subject conference subject @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_set_subject(LinphoneConference *conference, const char *subject);

/**
 * Set the conference subject as an UTF-8 string.
 * @param conference The #LinphoneConference object. @notnil
 * @param subject conference subject @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_set_subject_utf8(LinphoneConference *conference, const char *subject);

/**
 * Get the conference username
 * @param conference The #LinphoneConference object. @notnil
 * @return conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_get_username(const LinphoneConference *conference);

/**
 * Set the conference username
 * @param conference The #LinphoneConference object. @notnil
 * @param username conference subject @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_set_username(LinphoneConference *conference, const char *username);

/**
 * Set stream capability on 'me' device of a local conference
 * @param conference The #LinphoneConference object. @notnil
 * @param direction the direction of stream of type stream_type
 * @param stream_type A #LinphoneStreamType
 */
LINPHONE_PUBLIC void linphone_conference_set_local_participant_stream_capability(LinphoneConference *conference,
                                                                                 const LinphoneMediaDirection direction,
                                                                                 const LinphoneStreamType stream_type);

/**
 * Get the conference duration
 * @param conference The #LinphoneConference object. @notnil
 * @return conference duration. @maybenil
 */
LINPHONE_PUBLIC int linphone_conference_get_duration(const LinphoneConference *conference);

/**
 * Get the conference start time
 * @param conference The #LinphoneConference object. @notnil
 * @return conference start time. @maybenil
 */
LINPHONE_PUBLIC time_t linphone_conference_get_start_time(const LinphoneConference *conference);

/**
 * Get number of participants without me
 * @param conference The #LinphoneConference object. @notnil
 * @return the number of participants excluding me in a #LinphoneConference
 */
LINPHONE_PUBLIC int linphone_conference_get_participant_count(const LinphoneConference *conference);

/**
 * For a local audio video conference, this function returns the participant hosting the conference
 * For a remote audio video conference, this function returns the local participant of the conference
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
 * Retrieves the user pointer that was given to linphone_conference_set_user_data()
 * @param conference #LinphoneConference object @notnil
 * @return The user data associated with the #LinphoneConference object. @maybenil
 * @ingroup initializing
 **/
LINPHONE_PUBLIC void *linphone_conference_get_user_data(const LinphoneConference *conference);

/**
 * Associate a user pointer to the linphone conference.
 * @param conference #LinphoneConference object @notnil
 * @param user_data The user data to associate with the #LinphoneConference object. @maybenil
 * @ingroup initializing
 **/
LINPHONE_PUBLIC void linphone_conference_set_user_data(LinphoneConference *conference, void *user_data);

/**
 * Change the admin status of a participant of a conference (you need to be an admin yourself to do this).
 * @param conference A #LinphoneConference object @notnil
 * @param participant The Participant for which to change the admin status @notnil
 * @param is_admin A boolean value telling whether the participant should now be an admin or not
 */
LINPHONE_PUBLIC void linphone_conference_set_participant_admin_status(LinphoneConference *conference,
                                                                      LinphoneParticipant *participant,
                                                                      bool_t is_admin);

/**
 * For a local conference, the local participant joins the conference
 * For a client conference, the participant rejoins the conference after leaving it earlier on
 * @param conference A #LinphoneConference object @notnil
 * @return 0 if succeeded. Negative number if failed
 */
LINPHONE_PUBLIC int linphone_conference_enter(LinphoneConference *conference);

/**
 * For a local conference, the local participant leaves the conference
 * For a client conference, the participant leaves the conference after joining it earlier on
 * @param conference A #LinphoneConference object @notnil
 * @return 0 if succeeded. Negative number if failed
 */
LINPHONE_PUBLIC int linphone_conference_leave(LinphoneConference *conference);

/**
 * For a local audio video conference, this function compares the address provided as argument with that of participant
 * hosting the conference For a remote audio video conference, this function compares the address provided as argument
 * with that of the local participant of the conference
 * @param conference A #LinphoneConference object @notnil
 * @param uri A #LinphoneAddress object @notnil
 * @return TRUE if the participant is me, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_conference_is_me(const LinphoneConference *conference, const LinphoneAddress *uri);

/**
 * For a local conference, it returns whether the local participant is enabled
 * For a client conference, it return whether the remote participant has left the conference without bein removed from
 * it
 * @param conference A #LinphoneConference object @notnil
 * @return TRUE if the local participant is in a conference, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_conference_is_in(const LinphoneConference *conference);

/**
 * Sets the given #LinphoneAudioDevice as input for this conference only.
 * @param conference The #LinphoneConference @notnil
 * @param audio_device The #LinphoneAudioDevice. NULL does nothing. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_set_input_audio_device(LinphoneConference *conference,
                                                                LinphoneAudioDevice *audio_device);

/**
 * Sets the given #LinphoneAudioDevice as output for this conference only.
 * @param conference The #LinphoneConference @notnil
 * @param audio_device The #LinphoneAudioDevice. NULL does nothing. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_set_output_audio_device(LinphoneConference *conference,
                                                                 LinphoneAudioDevice *audio_device);

/**
 * Gets the current input device for this conference.
 * @param conference The #LinphoneConference @notnil
 * @return the #LinphoneAudioDevice used by this conference as input or NULL if there is currently no soundcard
 * configured (depending on the state of the conference) @maybenil
 */
LINPHONE_PUBLIC const LinphoneAudioDevice *
linphone_conference_get_input_audio_device(const LinphoneConference *conference);

/**
 * Gets the current output device for this conference.
 * @param conference The #LinphoneConference @notnil
 * @return the #LinphoneAudioDevice used by this conference as output or NULL if there is currently no soundcard
 * configured (depending on the state of the conference) @maybenil
 */
LINPHONE_PUBLIC const LinphoneAudioDevice *
linphone_conference_get_output_audio_device(const LinphoneConference *conference);

/**
 * Retrieves the volume of a specific participant
 * @param conference A #LinphoneConference object @notnil
 * @param device The Participant @notnil
 * @return The volume of the participant expressed in dbm0.
 */
LINPHONE_PUBLIC int linphone_conference_get_participant_device_volume(LinphoneConference *conference,
                                                                      LinphoneParticipantDevice *device);

/**
 * Starts recording the conference.
 * @param conference A #LinphoneConference object @notnil
 * @param path Where to record the conference @notnil
 * @return 0 if succeeded. Negative number in case of failure.
 */
LINPHONE_PUBLIC int linphone_conference_start_recording(LinphoneConference *conference, const char *path);

/**
 * Stops the conference recording.
 * @param conference A #LinphoneConference object @notnil
 * @return 0 if succeeded. Negative number in case of failure.
 */
LINPHONE_PUBLIC int linphone_conference_stop_recording(LinphoneConference *conference);

/**
 * Gets whether the conference is currently being recorded.
 * @param conference A #LinphoneConference object @notnil
 * @return TRUE if conference is being recorded, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_conference_is_recording(const LinphoneConference *conference);

/**
 * Gets the call that is controlling a conference.
 * - for the local conference, it will return NULL
 * - for the client conference, it will return call associated to the conference
 *
 * @param conference The #LinphoneConference @notnil
 * @return the #LinphoneCall controlling the conference or NULL if none or local conference @maybenil
 */
LINPHONE_PUBLIC LinphoneCall *linphone_conference_get_call(const LinphoneConference *conference);

/**
 * Retrieves the volume of a specific participant
 * @param conference A #LinphoneConference object @notnil
 * @return TRUE if the microphone is muted, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_get_microphone_muted(const LinphoneConference *conference);

/**
 * Get microphone muted state.
 * Note that the microphone may be disabled globally if FALSE was given to
 * linphone_core_enable_mic().
 * @param conference A #LinphoneConference object @notnil
 * @param muted The microphone muted state
 **/
LINPHONE_PUBLIC void linphone_conference_set_microphone_muted(LinphoneConference *conference, bool_t muted);

/**
 * Retrieves the volume of a specific participant
 * @param conference A #LinphoneConference object @notnil
 * @return The volume of the participant expressed in dbm0.
 */
LINPHONE_PUBLIC float linphone_conference_get_input_volume(const LinphoneConference *conference);

/**
 * Add a listener in order to be notified of #LinphoneConference events. Once an event is received, registred
 #LinphoneConferenceCbs are
 * invoked sequencially.
 * @param conference #LinphoneConference object. @notnil
 * @param cbs A #LinphoneConferenceCbs object holding the callbacks you need. A reference is taken by the
 #LinphoneConference until you invoke linphone_conference_remove_callbacks(). @notnil

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
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * LinphoneConferenceCbs that is calling the callback.
 * @param conference #LinphoneConference object. @notnil
 * @param cbs The LinphoneConferenceCbs object. @notnil
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_conference_set_current_callbacks(LinphoneConference *conference,
                                                               LinphoneConferenceCbs *cbs);

/**
 * Sets the current LinphoneConferenceCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * LinphoneConferenceCbs that is calling the callback.
 * @param conference #LinphoneConference object. @notnil
 * @return The #LinphoneConferenceCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneConferenceCbs *linphone_conference_get_current_callbacks(const LinphoneConference *conference);

/**
 * Returns core for a #LinphoneConference
 * @param conference #LinphoneConference object. @notnil
 * @return back pointer to #LinphoneCore object. @notnil
 **/
LINPHONE_PUBLIC LinphoneCore *linphone_conference_get_core(const LinphoneConference *conference);

/**
 * Returns the #LinphoneChatRoom linked to the #LinphoneConference
 * @param conference #LinphoneConference object. @notnil
 * @return back pointer to #LinphoneChatRoom object. @maybenil
 **/
LINPHONE_PUBLIC LinphoneChatRoom *linphone_conference_get_chat_room(const LinphoneConference *conference);

/**
 * Get the conference address of the conference.
 * This function may be return a NULL pointer if called before the conference switches to the Created state
 * @param conference A #LinphoneConference object. @notnil
 * @return The conference address of the conference. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_conference_get_conference_address(const LinphoneConference *conference);

/**
 * Set the conference address
 * @param conference The #LinphoneConference object. @notnil
 * @param address the conference address to set. @maybenil
 * @warning This is only allowed for a client conference if it is in state CreationPending or Instantiated
 */
LINPHONE_PUBLIC void linphone_conference_set_conference_address(LinphoneConference *conference,
                                                                LinphoneAddress *address);

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
LINPHONE_PUBLIC LinphoneParticipantDevice *
linphone_conference_get_active_speaker_participant_device(const LinphoneConference *conference);

/**
 * Gets a player associated with the conference to play a local file and stream it to the remote peers.
 * @param conference #LinphoneConference object. @notnil
 * @return A #LinphonePlayer object. @maybenil
 */
LINPHONE_PUBLIC LinphonePlayer *linphone_conference_get_player(LinphoneConference *conference);

/**
 * Gets the #LinphoneConferenceInfo object associated with a conference
 * @param conference #LinphoneConference object. @notnil
 * @return A #LinphoneConferenceInfo object. @maybenil
 */
LINPHONE_PUBLIC const LinphoneConferenceInfo *linphone_conference_get_info(LinphoneConference *conference);

/**
 * Gets the #LinphoneAccount object associated with the conference
 * @param conference #LinphoneConference object. @notnil
 * @return A #LinphoneAccount object. @maybenil
 */
LINPHONE_PUBLIC LinphoneAccount *linphone_conference_get_account(LinphoneConference *conference);

/************ */
/* DEPRECATED */
/* ********** */
LinphoneConference *linphone_local_conference_new(LinphoneCore *core, LinphoneAddress *addr);
LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core,
                                                              LinphoneAddress *addr,
                                                              const LinphoneConferenceParams *params);
LinphoneConference *linphone_remote_conference_new(LinphoneCore *core, LinphoneAddress *addr);
LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core,
                                                               LinphoneAddress *focus,
                                                               LinphoneAddress *addr,
                                                               const LinphoneConferenceParams *params);

/* This is actually only used by the ToneManager. TODO: encapsulate this better. */
AudioStream *linphone_conference_get_audio_stream(LinphoneConference *conference);

void linphone_conference_on_call_terminating(LinphoneConference *conference, LinphoneCall *call);

LINPHONE_PUBLIC bool_t linphone_conference_check_class(LinphoneConference *conference, LinphoneConferenceClass _class);

void linphone_conference_notify_audio_device_changed(LinphoneConference *conference, LinphoneAudioDevice *audio_device);

/**
 * A function to convert a #LinphoneConferenceState into a string
 */
LINPHONE_PUBLIC char *linphone_conference_state_to_string(LinphoneConferenceState state);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CONFERENCE_H_
