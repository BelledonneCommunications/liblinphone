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

#ifndef _L_C_CALL_H_
#define _L_C_CALL_H_

#include <ortp/rtpsession.h>

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup call_control
 * @{
 */

/**
 * Acquires a reference to the call.
 * An application that wishes to retain a pointer to call object
 * must use this function to unsure the pointer remains
 * valid. Once the application no more needs this pointer,
 * it must call linphone_call_unref().
 * @param call The #LinphoneCall object. @notnil
 * @return The same #LinphoneCall object. @notnil
 **/
LINPHONE_PUBLIC LinphoneCall *linphone_call_ref(LinphoneCall *call);

/**
 * Releases reference to the call.
 * @param call The #LinphoneCall object. @notnil
 **/
LINPHONE_PUBLIC void linphone_call_unref(LinphoneCall *call);

/**
 * Retrieves the user pointer associated with the call.
 * @param call The #LinphoneCall object. @notnil
 * @return The user pointer associated with the call. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_call_get_user_data(const LinphoneCall *call);

/**
 * Assigns a user pointer to the call.
 * @param call The #LinphoneCall object. @notnil
 * @param user_data The user pointer to associate with the call. @maybenil
 **/
LINPHONE_PUBLIC void linphone_call_set_user_data(LinphoneCall *call, void *user_data);

/**
 * Gets the core that has created the specified call.
 * @param call #LinphoneCall object. @notnil
 * @return The #LinphoneCore object that has created the specified call. @notnil
 */
LINPHONE_PUBLIC LinphoneCore *linphone_call_get_core(const LinphoneCall *call);

/**
 * Retrieves the call's current state.
 * @param call #LinphoneCall object. @notnil
 * @return the current #LinphoneCallState of this call.
 **/
LINPHONE_PUBLIC LinphoneCallState linphone_call_get_state(const LinphoneCall *call);

/**
 * Tells whether a call has been asked to autoanswer
 * @param call #LinphoneCall object. @notnil
 * @return A boolean value telling whether the call has been asked to autoanswer
 **/
LINPHONE_PUBLIC bool_t linphone_call_asked_to_autoanswer(LinphoneCall *call);

/**
 * Returns the remote address associated to this call
 * @param call #LinphoneCall object. @notnil
 * @return The #LinphoneAddress of the remote end of the call. @notnil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_remote_address(const LinphoneCall *call);

/**
 * Returns the to address with its headers associated to this call
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneAddress matching the TO of the call. @notnil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_to_address(const LinphoneCall *call);

/**
 * The address to which the call has been sent, taken directly from the SIP request-URI of the INVITE.
 * Usually equal to the `To` field, except when e.g. using a fallback contact address.
 * You should probably use getToAddress() instead, unless you know what you're doing.
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneAddress matching the URI of the INVITE request. @notnil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_request_address(const LinphoneCall *call);

/**
 * Returns the value of the header name
 * @param call #LinphoneCall object. @notnil
 * @param header_name the name of the header to check. @notnil
 * @return the value of the header if exists. @maybenil
 * @deprecated 27/10/2020. Use linphone_call_params_get_custom_header() on linphone_call_get_remote_params() instead.
 **/
LINPHONE_PUBLIC const char *linphone_call_get_to_header(const LinphoneCall *call, const char *header_name);

/**
 * Returns the diversion address associated to this call
 * @param call #LinphoneCall object. @notnil
 * @return the diversion address as #LinphoneAddress or NULL. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_diversion_address(const LinphoneCall *call);

/**
 * Returns direction of the call (incoming or outgoing).
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneCallDir
 **/
LINPHONE_PUBLIC LinphoneCallDir linphone_call_get_dir(const LinphoneCall *call);

/**
 * Gets the call log associated to this call.
 * @param call #LinphoneCall object. @notnil
 * @return The #LinphoneCallLog associated with the specified #LinphoneCall. @notnil
 **/
LINPHONE_PUBLIC LinphoneCallLog *linphone_call_get_call_log(const LinphoneCall *call);

/**
 * Gets the refer-to uri (if the call was transferred).
 * @param call #LinphoneCall object. @notnil
 * @return The refer-to uri of the call (if it was transferred). @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_refer_to(const LinphoneCall *call);

/**
 * Gets the refer-to uri (if the call was transferred).
 * @param call #LinphoneCall object. @notnil
 * @return The refer-to uri of the call (if it was transferred). @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_refer_to_address(const LinphoneCall *call);

/**
 * Gets the referred-by address, which is set when an incoming call is received
 * as a consequence of a call transfer operation by a third party.
 * The referred-by address is the sip identity of the one who initiated the transfer.
 * @param call The #LinphoneCall @notnil
 * @return The referred-by address @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_referred_by_address(const LinphoneCall *call);

/**
 * Returns if this calls has received a transfer that has not been
 * executed yet.
 * Pending transfers are executed when this call is being paused or closed,
 * locally or by remote endpoint.
 * If the call is already paused while receiving the transfer request, the
 * transfer immediately occurs.
 * @param call #LinphoneCall object. @notnil
 * @return TRUE if transfer is pending, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_has_transfer_pending(const LinphoneCall *call);

/**
 * Gets the transferer if this call was started automatically as a result of an incoming transfer request.
 * The call in which the transfer request was received is returned in this case.
 * @param call #LinphoneCall object. @notnil
 * @return The transferer #LinphoneCall if the specified call was started automatically as a result of an incoming
 *transfer request, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC LinphoneCall *linphone_call_get_transferer_call(const LinphoneCall *call);

/**
 * When this call has received a transfer request, returns the new call that was automatically created as a result of
 *the transfer.
 * @param call #LinphoneCall object. @notnil
 * @return the transfer #LinphoneCall created. @maybenil
 **/
LINPHONE_PUBLIC LinphoneCall *linphone_call_get_transfer_target_call(const LinphoneCall *call);

/**
 * Returns the call object this call is replacing, if any.
 * Call replacement can occur during call transfers.
 * By default, the core automatically terminates the replaced call and accept the new one.
 * This function allows the application to know whether a new incoming call is a one that replaces another one.
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneCall object this call is replacing or NULL. @maybenil
 **/
LINPHONE_PUBLIC LinphoneCall *linphone_call_get_replaced_call(LinphoneCall *call);

/**
 * Returns call's duration in seconds.
 * @param call #LinphoneCall object. @notnil
 * @return the call's duration in seconds.
 **/
LINPHONE_PUBLIC int linphone_call_get_duration(const LinphoneCall *call);

/**
 * Returns current parameters associated to the call.
 * @param call #LinphoneCall object. @notnil
 * @return the current #LinphoneCallParams of this call. @notnil
 **/
LINPHONE_PUBLIC const LinphoneCallParams *linphone_call_get_current_params(const LinphoneCall *call);

/**
 * Returns call parameters proposed by remote.
 *
 * This is useful when receiving an incoming call, to know whether the remote party
 * supports video, encryption or whatever.
 *
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneCallParams suggested by the remote or NULL. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneCallParams *linphone_call_get_remote_params(const LinphoneCall *call);

/**
 * Indicates whether camera input should be sent to remote end.
 * @param call #LinphoneCall object. @notnil
 * @param enabled wether or not to send local video stream.
 **/
LINPHONE_PUBLIC void linphone_call_enable_camera(LinphoneCall *call, bool_t enabled);

/**
 * Returns if camera pictures are allowed to be sent to the remote party.
 * @param call #LinphoneCall object. @notnil
 * @return TRUE if local video stream is being sent, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_camera_enabled(const LinphoneCall *call);

/**
 * Takes a photo of currently received video and write it into a jpeg file.
 * Note that the snapshot is asynchronous, an application shall not assume that the file is created when the function
 *returns.
 * @param call #LinphoneCall object. @notnil
 * @param file_path a path where to write the jpeg content. @notnil
 * @return 0 if successful, -1 otherwise (typically if jpeg format is not supported).
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_take_video_snapshot(LinphoneCall *call, const char *file_path);

/**
 * Takes a photo of currently captured video and write it into a jpeg file.
 * Note that the snapshot is asynchronous, an application shall not assume that the file is created when the function
 *returns.
 * @param call #LinphoneCall object. @notnil
 * @param file_path a path where to write the jpeg content. @notnil
 * @return 0 if successful, -1 otherwise (typically if jpeg format is not supported).
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_take_preview_snapshot(LinphoneCall *call, const char *file_path);

/**
 * Returns the reason for a call termination (either error or normal termination)
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneReason of the call termination.
 **/
LINPHONE_PUBLIC LinphoneReason linphone_call_get_reason(const LinphoneCall *call);

/**
 * Returns full details about call errors or termination reasons.
 * @param call #LinphoneCall object. @notnil on which we want the information error
 * @return #LinphoneErrorInfo object holding the reason error. @notnil
 */
LINPHONE_PUBLIC const LinphoneErrorInfo *linphone_call_get_error_info(const LinphoneCall *call);

/**
 * Returns the far end's user agent description string, if available.
 * @param call #LinphoneCall object. @notnil
 * @return the remote user agent or NULL. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_remote_user_agent(LinphoneCall *call);

/**
 * Returns the far end's sip contact as an address, if available.
 * @param call #LinphoneCall object. @notnil
 * @return the remote contact as a #LinphoneAddress or NULL. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_get_remote_contact_address(LinphoneCall *call);

/**
 * Returns the far end's sip contact as a string, if available.
 * @param call #LinphoneCall object. @notnil
 * @return the remote contact or NULL. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_remote_contact(LinphoneCall *call);

/**
 * Returns the ZRTP authentication token to verify.
 * @param call The #LinphoneCall object @notnil
 * @return the authentication token to verify or NULL if ZRTP isn't enabled. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_authentication_token(const LinphoneCall *call);

/**
 * Returns the local ZRTP authentication token to verify by the remote.
 * @param call The #LinphoneCall object @notnil
 * @return the local authentication token to verify or NULL if ZRTP isn't enabled. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_local_authentication_token(const LinphoneCall *call);

/**
 * Returns a list of 4 remote ZRTP authentication tokens. The user needs to select one.
 * @param call The #LinphoneCall object @notnil
 * @return the authentication tokens to verify or NULL if ZRTP isn't enabled. \bctbx_list{char *} @maybenil
 **/
LINPHONE_PUBLIC const bctbx_list_t *linphone_call_get_remote_authentication_tokens(const LinphoneCall *call);

/**
 * Returns whether ZRTP authentication token is verified.
 * If not, it must be verified by users as described in ZRTP procedure.
 * Once done, the application must inform of the results with linphone_call_set_authentication_token_verified().
 * @param call The #LinphoneCall object @notnil
 * @return TRUE if authentication token is verifed, false otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_get_authentication_token_verified(const LinphoneCall *call);

/**
 * If the user skips the ZRTP authentication check, stop the security alert.
 * @param call The #LinphoneCall object @notnil
 */
LINPHONE_PUBLIC void linphone_call_skip_zrtp_authentication(LinphoneCall *call);

/**
 * Verifies that the half ZRTP short authentication string (SAS) selected by the user is correct, and set the
 *verification result accordingly. If the remote party also verifies his/her half code correctly, the ZRTP cache will be
 *updated to prevent future verification requirements for these two users.
 * @param call The #LinphoneCall object @notnil
 * @param selected_value The ZRTP SAS selected by the user, or an empty string if the user cannot find the SAS
 **/
LINPHONE_PUBLIC void linphone_call_check_authentication_token_selected(LinphoneCall *call, const char *selected_value);

/**
 * Sets the result of ZRTP short code verification by user.
 * If remote party also does the same, it will update the ZRTP cache so that user's verification will not be required
 *for the two users.
 * @param call The #LinphoneCall object @notnil
 * @param verified whether the ZRTP SAS is verified.
 **/
LINPHONE_PUBLIC void linphone_call_set_authentication_token_verified(LinphoneCall *call, bool_t verified);

/**
 * Returns whether ZRTP cache mismatch.
 * If mismatch, the ZRTP authentication token must be verified by users as described in ZRTP procedure.
 * @param call The #LinphoneCall object @notnil
 * @return TRUE if ZRTP cache mismatch, false otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_get_zrtp_cache_mismatch_flag(const LinphoneCall *call);

/**
 * Requests remote side to send us a Video Fast Update.
 * @param call #LinphoneCall object. @notnil
 **/
LINPHONE_PUBLIC void linphone_call_send_vfu_request(LinphoneCall *call);

/**
 * Requests the callback passed to linphone_call_cbs_set_next_video_frame_decoded() to be called the next time the video
 *decoder properly decodes a video frame.
 * @param call the #LinphoneCall @notnil
 **/
LINPHONE_PUBLIC void linphone_call_request_notify_next_video_frame_decoded(LinphoneCall *call);

/**
 * Returns the current transfer state, if a transfer has been initiated from this call.
 * @see linphone_core_transfer_call() , linphone_core_transfer_call_to_another()
 * @param call #LinphoneCall object. @notnil
 * @return the #LinphoneCallState.
 **/
LINPHONE_PUBLIC LinphoneCallState linphone_call_get_transfer_state(LinphoneCall *call);

/**
 * Performs a zoom of the video displayed during a call.
 * The zoom ensures that all the screen is fullfilled with the video.
 * @param call The #LinphoneCall object @notnil
 * @param zoom_factor a floating point number describing the zoom factor. A value 1.0 corresponds to no zoom applied.
 * @param cx a floating point number pointing the horizontal center of the zoom to be applied. This value should be
 *between 0.0 and 1.0.
 * @param cy a floating point number pointing the vertical center of the zoom to be applied. This value should be
 *between 0.0 and 1.0.
 **/
LINPHONE_PUBLIC void linphone_call_zoom(LinphoneCall *call, float zoom_factor, float cx, float cy);

/**
 * Sends the specified dtmf.
 *
 * The dtmf is automatically played to the user.
 * @see linphone_core_set_use_rfc2833_for_dtmf()
 * @param call The #LinphoneCall object @notnil
 * @param dtmf The dtmf name specified as a char, such as '0', '#' etc...
 * @return 0 if successful, -1 on error.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_send_dtmf(LinphoneCall *call, char dtmf);

/**
 * Sends a list of dtmf.
 *
 * The dtmfs are automatically sent to remote, separated by some needed customizable delay.
 * Sending is canceled if the call state changes to something not LinphoneCallStreamsRunning.
 * @see linphone_core_set_use_rfc2833_for_dtmf()
 * @param call The #LinphoneCall object @notnil
 * @param dtmfs A dtmf sequence such as '123#123123' @notnil
 * @return -2 if there is already a DTMF sequence, -1 if call is not ready, 0 otherwise.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_send_dtmfs(LinphoneCall *call, const char *dtmfs);

/**
 * Stops current DTMF sequence sending.
 *
 * Please note that some DTMF could be already sent,
 * depending on when this function call is delayed from linphone_call_send_dtmfs(). This
 * function will be automatically called if call state change to anything but LinphoneCallStreamsRunning.
 * @param call The #LinphoneCall object @notnil
 **/
LINPHONE_PUBLIC void linphone_call_cancel_dtmfs(LinphoneCall *call);

/**
 * Returns the associated conference object if any.
 * @param call The #LinphoneCall object @notnil
 * @return A pointer on #LinphoneConference or NULL if the call is not part of any conference. @maybenil
 */
LINPHONE_PUBLIC LinphoneConference *linphone_call_get_conference(const LinphoneCall *call);

/**
 * Returns the number of stream for the given call.
 * @param call The #LinphoneCall object @notnil
 * @return the amount of streams for this call.
 **/
LINPHONE_PUBLIC int linphone_call_get_stream_count(const LinphoneCall *call);

/**
 * @brief Returns the type of stream for the given stream index.
 * @return the type (MSAudio, MSVideo, MSText) of the stream of given index.
 * @donotwrap
 **/
LINPHONE_PUBLIC MSFormatType linphone_call_get_stream_type(const LinphoneCall *call, int stream_index);

/**
 * @brief Returns the meta rtp transport for the given stream index.
 * @return a pointer to the meta rtp transport if it exists, NULL otherwise.
 * @donotwrap
 **/
LINPHONE_PUBLIC RtpTransport *linphone_call_get_meta_rtp_transport(const LinphoneCall *call, int stream_index);

/**
 * @brief Returns the meta rtcp transport for the given stream index.
 * @return a pointer to the meta rtcp transport if it exists, NULL otherwise.
 * @donotwrap
 **/
LINPHONE_PUBLIC RtpTransport *linphone_call_get_meta_rtcp_transport(const LinphoneCall *call, int stream_index);

/**
 * Pauses the call. If a music file has been setup using linphone_core_set_play_file(),
 * this file will be played to the remote user.
 * The only way to resume a paused call is to call linphone_call_resume().
 * @param call #LinphoneCall object. @notnil
 * @return 0 on success, -1 on failure
 * @see linphone_call_resume()
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_pause(LinphoneCall *call);

/**
 * Resumes a call.
 * The call needs to have been paused previously with linphone_call_pause().
 * @param call #LinphoneCall object. @notnil
 * @return 0 on success, -1 on failure
 * @see linphone_call_pause()
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_resume(LinphoneCall *call);

/**
 * Terminates (ends) a call.
 * @param call #LinphoneCall object. @notnil
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_terminate(LinphoneCall *call);

/**
 * Terminates a call with additional information, serialized as a Reason header.
 * @param call The #LinphoneCall object @notnil
 * @param ei #LinphoneErrorInfo @maybenil
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_terminate_with_error_info(LinphoneCall *call, const LinphoneErrorInfo *ei);

/**
 * Redirects the specified call to the given redirect Address.
 * @param call The #LinphoneCall object @notnil
 * @param redirect_address The #LinphoneAddress to redirect the call to @notnil
 * @return 0 if successful, -1 on error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_call_redirect_to(LinphoneCall *call, LinphoneAddress *redirect_address);

/**
 * Declines a pending incoming call, with a reason.
 * @param call A #LinphoneCall object that must be in the IncomingReceived state @notnil
 * @param reason The reason for rejecting the call: #LinphoneReasonDeclined or #LinphoneReasonBusy
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_decline(LinphoneCall *call, LinphoneReason reason);

/**
 * Declines a pending incoming call, with a #LinphoneErrorInfo object.
 * @param call A #LinphoneCall object that must be in the IncomingReceived state @notnil
 * @param ei #LinphoneErrorInfo containing more information on the call rejection. @maybenil
 * @return 0 on success, -1 on failure
 */
LINPHONE_PUBLIC int linphone_call_decline_with_error_info(LinphoneCall *call, const LinphoneErrorInfo *ei);

/**
 * Accepts an incoming call.
 *
 * Basically the application is notified of incoming calls within the
 * call_state_changed callback of the #LinphoneCoreVTable structure, where it will receive
 * a #LinphoneCallStateIncoming event with the associated #LinphoneCall object.
 * The application can later accept the call using this method.
 * @param call The #LinphoneCall object @notnil
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_accept(LinphoneCall *call);

/**
 * Accepts an incoming call, with parameters.
 *
 * Basically the application is notified of incoming calls within the
 * call_state_changed callback of the #LinphoneCoreVTable structure, where it will receive
 * a #LinphoneCallStateIncoming event with the associated #LinphoneCall object.
 * The application can later accept the call using this method.
 * @param call A #LinphoneCall object @notnil
 * @param params The specific parameters for this call, for example whether video is accepted or not. Use NULL to use
 *default parameters. @maybenil
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_accept_with_params(LinphoneCall *call, const LinphoneCallParams *params);

/**
 * Accepts an early media session for an incoming call.
 * This is identical as calling linphone_call_accept_early_media_with_params() with NULL parameters.
 * @param call A #LinphoneCall object @notnil
 * @return 0 if successful, -1 otherwise
 * @see linphone_call_accept_early_media_with_params()
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_accept_early_media(LinphoneCall *call);

/**
 * When receiving an incoming, accepts to start a media session as early-media.
 * This means the call is not accepted but audio & video streams can be established if the remote party supports early
 *media. However, unlike after call acceptance, mic and camera input are not sent during early-media, though received
 *audio & video are played normally. The call can then later be fully accepted using linphone_call_accept() or
 *linphone_call_accept_with_params().
 * @param call A #LinphoneCall object @notnil
 * @param params The call parameters to use (can be NULL). @maybenil
 * @return 0 if successful, -1 otherwise
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_accept_early_media_with_params(LinphoneCall *call,
                                                                            const LinphoneCallParams *params);

/**
 * Updates a running call according to supplied call parameters or parameters changed in the LinphoneCore.
 *  It triggers a SIP reINVITE in order to perform a new offer/answer of media capabilities.
 * Changing the size of the transmitted video after calling #linphone_core_set_preferred_video_size() can be used by
 *passing NULL as params argument. In case no changes are requested through the #LinphoneCallParams argument, then this
 *argument can be omitted and set to NULL. WARNING: Updating a call in the #LinphoneCallStatePaused state will still
 *result in a paused call even if the media directions set in the params are sendrecv. To resume a paused call, you need
 *to call linphone_call_resume().
 *
 * @param call A #LinphoneCall object @notnil
 * @param params The new call parameters to use (may be NULL). @maybenil
 * @return 0 if successful, -1 otherwise.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_update(LinphoneCall *call, const LinphoneCallParams *params);

/**
 * When receiving a #LinphoneCallStateUpdatedByRemote state notification, prevent #LinphoneCore from performing an
 *automatic answer.
 *
 * When receiving a #LinphoneCallStateUpdatedByRemote state notification (ie an incoming reINVITE), the default
 *behaviour of #LinphoneCore is defined by the "defer_update_default" option of the "sip" section of the config. If this
 *option is 0 (the default) then the #LinphoneCore automatically answers the reINIVTE with call parameters unchanged.
 *However when for example when the remote party updated the call to propose a video stream, it can be useful to prompt
 *the user before answering. This can be achieved by calling linphone_core_defer_call_update() during the call state
 *notification, to deactivate the automatic answer that would just confirm the audio but reject the video. Then, when
 *the user responds to dialog prompt, it becomes possible to call linphone_call_accept_update() to answer the reINVITE,
 *with video possibly enabled in the #LinphoneCallParams argument.
 *
 * The #LinphoneCallStateUpdatedByRemote notification can also happen when receiving an INVITE without SDP. In such
 *case, an unchanged offer is made in the 200Ok, and when the ACK containing the SDP answer is received,
 *#LinphoneCallStateUpdatedByRemote is triggered to notify the application of possible changes in the media session.
 *However in such case defering the update has no meaning since we just generated an offer.
 *
 * @param call A #LinphoneCall object @notnil
 * @return 0 if successful, -1 if the linphone_call_defer_update() was done outside a valid #LinphoneCallUpdatedByRemote
 *notification
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_defer_update(LinphoneCall *call);

/**
 * Accepts call modifications initiated by other end through an incoming reINVITE or UPDATE request.
 *
 * linphone_call_accept_update() is typically used in response to a #LinphoneCallStateUpdatedByRemote state
 *notification. When such notification arrives, the application has several choices:
 * - use linphone_call_defer_update() so that it can have the time to prompt the user,
 * and then later use linphone_call_accept_update() to answer
 * - doing nothing, in which case linphone_call_accept_update() is internally called automatically to generate a default
 * response.
 * - immediately linphone_call_accept_update() to generate a response.
 *
 * An application may use linphone_call_get_remote_params() to get information about the call
 * parameters proposed by the other party (for example when he wants to add a video stream), in order to decide
 * what to do, like for example requesting the end-user approval.
 *
 * The params argument must be constructed with linphone_core_create_call_params().
 * linphone_core_create_call_params() will initialize the returned #LinphoneCallParams according to
 * the offered parameters, previously used local parameters and local policies.
 * For example, if the remote user-agent wants to add a video stream but the #LinphoneVideoActivationPolicy
 * of the #LinphoneCore is to not automatically accept video, then the #LinphoneCallParams will get its video
 * stream disabled.
 * The application is free to modify the #LinphoneCallParams before passing it to
 * linphone_call_accept_update().
 * Using null as params argument is allowed and equivalent to passing a #LinphoneCallParams
 * created by linphone_core_create_call_params() and left unmodified.
 *
 * The answer generated by linphone_call_accept_update() at first follows RFC3264 SDP offer
 * answer model rules. The supplied #LinphoneCallParams allows to give application instructions
 * about the response to generate, such as accepting or not the enablement of new stream.
 * linphone_call_accept_update() does not have the pretention to control every aspect
 * of the generation of the answer of an SDP offer/answer procedure.
 *
 * @param call A #LinphoneCall object @notnil
 * @param params A #LinphoneCallParams object describing the call parameters to accept. @maybenil
 * @return 0 if successful, -1 otherwise (actually when this function call is performed outside ot
 *#LinphoneCallStateUpdatedByRemote state)
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_accept_update(LinphoneCall *call, const LinphoneCallParams *params);

/**
 * Performs a simple call transfer to the specified destination.
 * The remote endpoint is expected to issue a new call to the specified destination.
 * The current call remains active and thus can be later paused or terminated.
 * It is possible to follow the progress of the transfer provided that transferee sends notification about it.
 * In this case, the 'transfer state changed' callback of the #LinphoneCoreCbs is invoked to notify of the state of the
 *new call to the other party. The notified states are #LinphoneCallStateOutgoingInit ,
 *#LinphoneCallStateOutgoingProgress, #LinphoneCallStateOutgoingRinging and #LinphoneCallStateConnected.
 * @param call The call to be transferred @notnil
 * @param refer_to The #LinphoneAddress the call is to be referred to. @notnil
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_transfer_to(LinphoneCall *call, const LinphoneAddress *refer_to);

/**
 * Transfers a call to destination of another running call. This is used for "attended transfer" scenarios.
 * The transferred call is supposed to be in paused state, so that it is able to accept the transfer immediately.
 * The destination call is a call previously established to introduce the transferred person.
 * This method will send a transfer request to the transferred person. The phone of the transferred is then
 * expected to automatically call to the destination of the transfer. The receiver of the transfer will then
 *automatically close the call with us (the 'dest' call). It is possible to follow the progress of the transfer provided
 *that transferee sends notification about it. In this case, the "transfer state changed" callback of the
 * #LinphoneCoreCbs is invoked to notify of the state of the new call to the other party. The notified states are
 *#LinphoneCallStateOutgoingInit , #LinphoneCallStateOutgoingProgress, #LinphoneCallStateOutgoingRinging and
 *#LinphoneCallStateConnected.
 * @param call A running call you want to transfer @notnil
 * @param dest A running call whose remote person will receive the transfer @notnil
 * @return 0 on success, -1 on failure
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_transfer_to_another(LinphoneCall *call, LinphoneCall *dest);

/**
 * @}
 */

/**
 * @addtogroup media_parameters
 * @{
 */

/**
 * Get the native window handle of the video window, casted as an unsigned long.
 * @see linphone_core_set_native_video_window_id() for a general discussion about window IDs.
 * @param call the #LinphoneCall object @notnil
 * @return the native video window id (type may vary depending on platform). @maybenil
 **/
LINPHONE_PUBLIC void *linphone_call_get_native_video_window_id(const LinphoneCall *call);

/**
 * Create a native video window id where the video is to be displayed.
 * @see linphone_core_set_native_video_window_id() for a general discussion about window IDs.
 * @param call the #LinphoneCall object @notnil
 * @return the native video window id (type may vary depending on platform). @maybenil
 **/
LINPHONE_PUBLIC void *linphone_call_create_native_video_window_id(const LinphoneCall *call);

/**
 * Set the native video window id where the video is to be displayed.
 * For MacOS, Linux, Windows: if not set or 0 a window will be automatically created, unless the special id -1 is given.
 * @see linphone_core_set_native_video_window_id() for a general discussion about window IDs.
 * @param call the #LinphoneCall object @notnil
 * @param window_id the native video window id. @maybenil
 **/
LINPHONE_PUBLIC void linphone_call_set_native_video_window_id(LinphoneCall *call, void *window_id);

/**
 * Enables or disables echo cancellation for this call.
 * @see linphone_core_enable_echo_cancellation().
 * @param call the #LinphoneCall object @notnil
 * @param enable wether to enable echo cancellation or not.
 **/
LINPHONE_PUBLIC void linphone_call_enable_echo_cancellation(LinphoneCall *call, bool_t enable);

/**
 * Returns if echo cancellation is enabled.
 * @see linphone_core_enable_echo_cancellation().
 * @param call the #LinphoneCall object @notnil
 * @return TRUE if echo cancellation is enabled, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_echo_cancellation_enabled(const LinphoneCall *call);

/**
 * Enables or disables echo limiter for this call.
 * @see linphone_core_enable_echo_limiter().
 * @param call the #LinphoneCall object @notnil
 * @param enable wether to enable echo limiter or not.
 **/
LINPHONE_PUBLIC void linphone_call_enable_echo_limiter(LinphoneCall *call, bool_t enable);

/**
 * Returns if echo limiter is enabled.
 * @see linphone_core_enable_echo_limiter().
 * @param call the #LinphoneCall object @notnil
 * @return TRUE if echo limiter is enabled, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_echo_limiter_enabled(const LinphoneCall *call);

/**
 * @}
 */

/**
 * @addtogroup call_misc
 * @{
 */

/**
 * Obtain a chat room for real time messaging from a call if not already existing, else return existing one.
 * No reference is given to the caller: the chat room will be deleted when the call is ended.
 * The call must have been accepted with a real time text stream (see linphone_call_params_enable_realtime_text()).
 * @param call #LinphoneCall object @notnil
 * @return #LinphoneChatRoom where real time messaging can take place or NULL if chat room couldn't be created.
 * @maybenil
 */
LINPHONE_PUBLIC LinphoneChatRoom *linphone_call_get_chat_room(LinphoneCall *call);

/**
 * Gets the mesured playback volume level (received from remote) in dbm0.
 * @param call The call. @notnil
 * @return float Volume level in dbm0.
 */
LINPHONE_PUBLIC float linphone_call_get_play_volume(const LinphoneCall *call);

/**
 * Gets the mesured record volume level (sent to remote) in dbm0.
 * @param call The call. @notnil
 * @return float Volume level in dbm0.
 */
LINPHONE_PUBLIC float linphone_call_get_record_volume(const LinphoneCall *call);

/**
 * Gets speaker volume gain.
 * If the sound backend supports it, the returned gain is equal to the gain set
 * with the system mixer.
 * @param call The call. @notnil
 * @return Percentage of the max supported volume gain. Valid values are in [ 0.0 : 1.0 ].
 * In case of failure, a negative value is returned
 */
LINPHONE_PUBLIC float linphone_call_get_speaker_volume_gain(const LinphoneCall *call);

/**
 * Sets speaker volume gain.
 * If the sound backend supports it, the new gain will synchronized with the system mixer.
 * @param call The call. @notnil
 * @param volume Percentage of the max supported gain. Valid values are in [ 0.0 : 1.0 ].
 */
LINPHONE_PUBLIC void linphone_call_set_speaker_volume_gain(LinphoneCall *call, float volume);

/**
 * Gets microphone volume gain.
 * If the sound backend supports it, the returned gain is equal to the gain set
 * with the system mixer.
 * @param call The #LinphoneCall object. @notnil
 * @return double Percentage of the max supported volume gain. Valid values are in [ 0.0 : 1.0 ].
 * In case of failure, a negative value is returned
 */
LINPHONE_PUBLIC float linphone_call_get_microphone_volume_gain(const LinphoneCall *call);

/**
 * Sets microphone volume gain.
 * If the sound backend supports it, the new gain will synchronized with the system mixer.
 * @param call The #LinphoneCall object. @notnil
 * @param volume Percentage of the max supported gain. Valid values are in [ 0.0 : 1.0 ].
 */
LINPHONE_PUBLIC void linphone_call_set_microphone_volume_gain(LinphoneCall *call, float volume);

/**
 * Gets speaker muted state.
 * @param call The #LinphoneCall object. @notnil
 * @return The speaker muted state.
 **/
LINPHONE_PUBLIC bool_t linphone_call_get_speaker_muted(const LinphoneCall *call);

/**
 * Sets speaker muted state.
 * @param call The #LinphoneCall object. @notnil
 * @param muted The speaker muted state.
 **/
LINPHONE_PUBLIC void linphone_call_set_speaker_muted(LinphoneCall *call, bool_t muted);

/**
 * Gets microphone muted state.
 * Note that the microphone may be disabled globally if FALSE was given to
 * linphone_core_enable_mic().
 * @param call The #LinphoneCall object. @notnil
 * @return The microphone muted state.
 * @warning This method returns state of the mute capability of the call passed as argument. If this call is part of a
 *conference, it is strongly recommended to call linphone_conference_get_microphone_muted() to know whether this device
 *is muted or not.
 **/
LINPHONE_PUBLIC bool_t linphone_call_get_microphone_muted(const LinphoneCall *call);

/**
 * Sets microphone muted state.
 * The boolean value given is applied logical-and with the value given to
 * linphone_core_enable_mic().
 * @param call The #LinphoneCall object. @notnil
 * @param muted The microphone muted state.
 * @warning This method only mutes the call passed as argument. If this call is part of a conference, it is strongly
 *recommended to call linphone_conference_set_microphone_muted() to ensure that the setting is correctly apply across
 *all participants and the conference callbacks are called.
 **/
LINPHONE_PUBLIC void linphone_call_set_microphone_muted(LinphoneCall *call, bool_t muted);

/**
 * Obtains real-time quality rating of the call
 *
 * Based on local RTP statistics and RTCP feedback, a quality rating is computed and updated
 * during all the duration of the call. This function returns its value at the time of the function call.
 * It is expected that the rating is updated at least every 5 seconds or so.
 * The rating is a floating point number comprised between 0 and 5.
 *
 * 4-5 = good quality <br>
 * 3-4 = average quality <br>
 * 2-3 = poor quality <br>
 * 1-2 = very poor quality <br>
 * 0-1 = can't be worse, mostly unusable <br>
 *
 * @param call The #LinphoneCall object. @notnil
 * @return The function returns -1 if no quality measurement is available, for example if no
 * active audio stream exist. Otherwise it returns the quality rating.
 **/
LINPHONE_PUBLIC float linphone_call_get_current_quality(const LinphoneCall *call);

/**
 * Returns call quality averaged over all the duration of the call.
 *
 * See linphone_call_get_current_quality() for more details about quality measurement.
 * @param call The #LinphoneCall object. @notnil
 * @return the call average quality since tbe beginning of the call.
 **/
LINPHONE_PUBLIC float linphone_call_get_average_quality(const LinphoneCall *call);

/**
 * Starts call recording.
 * Video record is only available if this function is called in state StreamRunning.
 * The output file where audio is recorded must be previously specified with linphone_call_params_set_record_file().
 * @param call The #LinphoneCall object. @notnil
 **/
LINPHONE_PUBLIC void linphone_call_start_recording(LinphoneCall *call);

/**
 * Stops call recording.
 * @param call The #LinphoneCall object. @notnil
 **/
LINPHONE_PUBLIC void linphone_call_stop_recording(LinphoneCall *call);

/**
 * Gets a player associated with the call to play a local file and stream it to the remote peer.
 * @param call #LinphoneCall object. @notnil
 * @return A #LinphonePlayer object. @maybenil
 */
LINPHONE_PUBLIC LinphonePlayer *linphone_call_get_player(LinphoneCall *call);

/**
 * Indicates whether an operation is in progress at the media side.
 * It can be a bad idea to initiate signaling operations (adding video, pausing the call, removing video, changing video
 *parameters) while the media is busy in establishing the connection (typically ICE connectivity checks). It can result
 *in failures generating loss of time in future operations in the call. Applications are invited to check this function
 *after each call state change to decide whether certain operations are permitted or not.
 * @param call the #LinphoneCall @notnil
 * @return TRUE if media is busy in establishing the connection, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_call_media_in_progress(const LinphoneCall *call);

/**
 * Calls generic OpenGL render for a given call.
 * @param call The #LinphoneCall. @notnil
 */
LINPHONE_PUBLIC void linphone_call_ogl_render(const LinphoneCall *call);

/**
 * Returns the local tag of the #LinphoneCall
 * @param call The #LinphoneCall object @notnil
 * @return the local tag. @notnil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_local_tag(const LinphoneCall *call);

/**
 * Returns the remote tag of the #LinphoneCall
 * @param call The #LinphoneCall object @notnil
 * @return the remote tag. @notnil
 **/
LINPHONE_PUBLIC const char *linphone_call_get_remote_tag(const LinphoneCall *call);

/**
 * Sends an info message through an established call
 * @param call the #LinphoneCall @notnil
 * @param info the #LinphoneInfoMessage to send @notnil
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_send_info_message(LinphoneCall *call, const LinphoneInfoMessage *info);

/**
 * Returns a copy of the call statistics for a particular stream type.
 * @param call the #LinphoneCall @notnil
 * @param type the #LinphoneStreamType
 * @return a #LinphoneCallStats object for the given stream or NULL if stream isn't available. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC LinphoneCallStats *linphone_call_get_stats(LinphoneCall *call, LinphoneStreamType type);

/**
 * Returns a copy of the call statistics for the audio stream.
 * @param call the #LinphoneCall @notnil
 * @return a #LinphoneCallStats object for the audio stream or NULL if it isn't available. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC LinphoneCallStats *linphone_call_get_audio_stats(LinphoneCall *call);

/**
 * Returns a copy of the call statistics for the video stream.
 * @param call the #LinphoneCall @notnil
 * @return a #LinphoneCallStats object for the video stream or NULL if it isn't available. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC LinphoneCallStats *linphone_call_get_video_stats(LinphoneCall *call);

/**
 * Returns a copy of the call statistics for the text stream.
 * @param call the #LinphoneCall @notnil
 * @return a #LinphoneCallStats object for the text stream or NULL if it isn't available. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC LinphoneCallStats *linphone_call_get_text_stats(LinphoneCall *call);

/**
 * Adds a listener in order to be notified of #LinphoneCall events. Once an event is received, registred
 * #LinphoneCallCbs are invoked sequencially.
 * @param call #LinphoneCall object. @notnil to monitor.
 * @param cbs A #LinphoneCallCbs object holding the callbacks you need. A reference is taken by the #LinphoneCall until
 * you invoke linphone_call_remove_callbacks(). @notnil
 */
LINPHONE_PUBLIC void linphone_call_add_callbacks(LinphoneCall *call, LinphoneCallCbs *cbs);

/**
 * Removes a listener from a #LinphoneCall
 * @param call #LinphoneCall object. @notnil
 * @param cbs #LinphoneCallCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_call_remove_callbacks(LinphoneCall *call, LinphoneCallCbs *cbs);

/**
 * Gets the currently invoked LinphoneCallCbs if any.
 * This is meant only to be called from a callback to be able to get the user_data associated with the #LinphoneCallCbs
 * that is calling the callback.
 * @param call #LinphoneCall object. @notnil
 * @return The #LinphoneCallCbs that has called the last callback @maybenil
 */
LINPHONE_PUBLIC LinphoneCallCbs *linphone_call_get_current_callbacks(const LinphoneCall *call);

/**
 * Sets call parameters - advanced and not recommended feature - use with caution.
 * Local call parameters applicable to an outgoing or incoming shall usually be passed to
 *linphone_core_invite_address_with_params() or linphone_call_accept_with_params(). However, in some cases it might be
 *desirable from a software design standpoint to modify local parameters outside of the application layer, typically in
 *the purpose of implementing a custom logic including special headers in INVITE or 200Ok requests, driven by a
 *call_state_changed listener method. This function accepts to assign a new #LinphoneCallParams only in
 *#LinphoneCallStateOutgoingInit and #LinphoneCallStateIncomingReceived states.
 *
 * @param call the #LinphoneCall object @notnil
 * @param params the #LinphoneCallParams object @notnil
 **/
LINPHONE_PUBLIC void linphone_call_set_params(LinphoneCall *call, const LinphoneCallParams *params);

/**
 * Returns read-only local parameters associated with the call.
 * This is typically the parameters passed at call initiation to linphone_core_invite_address_with_params() or
 *linphone_call_accept_with_params(), or some default parameters if no #LinphoneCallParams was explicitely passed during
 *call initiation.
 * @param call the #LinphoneCall object @notnil
 * @return the call's local parameters. @notnil
 **/
LINPHONE_PUBLIC const LinphoneCallParams *linphone_call_get_params(const LinphoneCall *call);

/**
 * Sets the given #LinphoneAudioDevice as input for this call only.
 * @param call The #LinphoneCall @notnil
 * @param audio_device The #LinphoneAudioDevice. NULL does nothing. @maybenil
 */
LINPHONE_PUBLIC void linphone_call_set_input_audio_device(LinphoneCall *call, LinphoneAudioDevice *audio_device);

/**
 * Sets the given #LinphoneAudioDevice as output for this call only.
 * @param call The #LinphoneCall @notnil
 * @param audio_device The #LinphoneAudioDevice. NULL does nothing. @maybenil
 */
LINPHONE_PUBLIC void linphone_call_set_output_audio_device(LinphoneCall *call, LinphoneAudioDevice *audio_device);

/**
 * Gets the current input device for this call.
 * @param call The #LinphoneCall @notnil
 * @return the #LinphoneAudioDevice used by this call as input or NULL if there is currently no soundcard configured
 * (depending on the state of the call) @maybenil
 */
LINPHONE_PUBLIC const LinphoneAudioDevice *linphone_call_get_input_audio_device(const LinphoneCall *call);

/**
 * Gets the current output device for this call.
 * @param call The #LinphoneCall @notnil
 * @return the #LinphoneAudioDevice used by this call as output or NULL if there is currently no soundcard configured
 * (depending on the state of the call) @maybenil
 */
LINPHONE_PUBLIC const LinphoneAudioDevice *linphone_call_get_output_audio_device(const LinphoneCall *call);

/**
 * Starts the process of replying 180 Ringing.
 *
 * This function is used in conjonction with linphone_core_enable_auto_send_ringing().
 * If the automatic sending of the 180 Ringing is disabled, this function needs to be called manually before the call
 * timeouts.
 *
 * @param call The #LinphoneCall @notnil
 */
LINPHONE_PUBLIC void linphone_call_notify_ringing(LinphoneCall *call);

/**
 * Sets the video source of a call.
 * @param call The #LinphoneCall @notnil
 * @param descriptor The #LinphoneVideoSourceDescriptor describing the video source to set @maybenil
 */
LINPHONE_PUBLIC void linphone_call_set_video_source(LinphoneCall *call,
                                                    const LinphoneVideoSourceDescriptor *descriptor);

/**
 * Gets the video source of a call.
 * @param call The #LinphoneCall @notnil
 * @return The #LinphoneVideoSourceDescriptor describing the video source that is set @maybenil
 */
LINPHONE_PUBLIC const LinphoneVideoSourceDescriptor *linphone_call_get_video_source(const LinphoneCall *call);

/**
 * Method to be called after the user confirm that he/she is notifed of the on going Go Clear procedure.
 * @warning this operation must be imperatevely initiate by a user action on sending of the GoClear ACK
 * @param call The #LinphoneCall @notnil
 */
LINPHONE_PUBLIC void linphone_call_confirm_go_clear(const LinphoneCall *call);

/**
 * Accepts and execute a transfer (ie an incoming REFER request), to use when [sip] 'auto_accept_refer' property is
 * false. Default behaviour is to accept and execute the transfer automatically.
 * @param call The #LinphoneCall to be transferred @notnil
 */
LINPHONE_PUBLIC void linphone_call_accept_transfer(LinphoneCall *call);

/**
 * Create a #LinphoneEvent in order to send NOTIFY requests through the SIP dialog created by the call.
 * The call state must have passed through #LinphoneCallStateConnected.
 * @param call The #LinphoneCall object @notnil
 * @param event The event type to be notified.
 * @return a new #LinphoneEvent @maybenil
 */
LINPHONE_PUBLIC LinphoneEvent *linphone_call_create_notify(LinphoneCall *call, const char *event);

/**
 * Indicates whether received Baudot tones should be detected.
 * The Baudot functionality is to be enabled first by calling linphone_core_enable_baudot().
 * @param call #LinphoneCall object. @notnil
 * @param enabled wether or not to detect received Baudot tones.
 **/
LINPHONE_PUBLIC void linphone_call_enable_baudot_detection(LinphoneCall *call, bool_t enabled);

/**
 * Defines the Baudot mode for the call.
 * The Baudot functionality is to be enabled first by calling linphone_core_enable_baudot().
 * @param call #LinphoneCall object. @notnil
 * @param mode The Baudot mode to use for the call.
 */
LINPHONE_PUBLIC void linphone_call_set_baudot_mode(LinphoneCall *call, LinphoneBaudotMode mode);

/**
 * Defines the Baudot standard to use for sending Baudot tones in the call.
 * The Baudot functionality is to be enabled first by calling linphone_core_enable_baudot().
 * @param call #LinphoneCall object. @notnil
 * @param standard The Baudot standard to use for sending Baudot tones.
 */
LINPHONE_PUBLIC void linphone_call_set_baudot_sending_standard(LinphoneCall *call, LinphoneBaudotStandard standard);

/**
 * Set the Baudot significant pause timeout after which a LETTERS tone is retransmitted before resuming transmission (in
 * seconds). Default is 5s.
 * The Baudot functionality is to be enabled first by calling linphone_core_enable_baudot().
 * @param[in] call #LinphoneCall object. @notnil
 * @param[in] seconds The significant pause timeout in seconds.
 */
LINPHONE_PUBLIC void linphone_call_set_baudot_pause_timeout(LinphoneCall *call, uint8_t seconds);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Redirect the specified call to the given redirect URI.
 * @param call The #LinphoneCall object @notnil
 * @param redirect_uri The URI to redirect the call to @notnil
 * @return 0 if successful, -1 on error.
 * @deprecated 27/10/2020. Use linphone_call_redirect_to() instead.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_call_redirect(LinphoneCall *call, const char *redirect_uri);

/**
 * Performs a simple call transfer to the specified destination.
 * The remote endpoint is expected to issue a new call to the specified destination.
 * The current call remains active and thus can be later paused or terminated.
 * It is possible to follow the progress of the transfer provided that transferee sends notification about it.
 * In this case, the transfer_state_changed callback of the #LinphoneCoreVTable is invoked to notify of the state of the
 *new call at the other party. The notified states are #LinphoneCallOutgoingInit , #LinphoneCallOutgoingProgress,
 *#LinphoneCallOutgoingRinging and #LinphoneCallConnected.
 * @param call The call to be transferred @notnil
 * @param refer_to The destination the call is to be referred to. @notnil
 * @return 0 on success, -1 on failure
 * @deprecated 27/10/2020. Use linphone_call_transfer_to() instead.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_call_transfer(LinphoneCall *call, const char *refer_to);

/**
 * Returns the remote address associated to this call as a string.
 * The result string must be freed by user using ms_free().
 * @param call #LinphoneCall object. @notnil
 * @return the remote address as a string. @maybenil @tobefreed
 * @deprecated 06/07/2020 use linphone_call_get_remote_address() instead.
 **/
LINPHONE_PUBLIC LINPHONE_DEPRECATED char *linphone_call_get_remote_address_as_string(const LinphoneCall *call);

/**
 * @deprecated 23/05/2018
 * @donotwrap
 **/
LINPHONE_PUBLIC LINPHONE_DEPRECATED void
linphone_call_set_next_video_frame_decoded_callback(LinphoneCall *call, LinphoneCallCbFunc cb, void *user_data);

/**
 * Perform a zoom of the video displayed during a call.
 * @param call The #LinphoneCall object @notnil
 * @param zoom_factor a floating point number describing the zoom factor. A value 1.0 corresponds to no zoom applied.
 * @param cx a floating point number pointing the horizontal center of the zoom to be applied. This value should be
 *between 0.0 and 1.0.
 * @param cy a floating point number pointing the vertical center of the zoom to be applied. This value should be
 *between 0.0 and 1.0.
 * @deprecated 16/10/2017 use linphone_call_zoom instead
 * @donotwrap
 * cx and cy are updated in return in case their coordinates were too excentrated for the requested zoom factor. The
 *zoom ensures that all the screen is fullfilled with the video.
 **/
LINPHONE_PUBLIC LINPHONE_DEPRECATED void
linphone_call_zoom_video(LinphoneCall *call, float zoom_factor, float *cx, float *cy);

/**
 * Return TRUE if this call is currently part of a conference
 * @param call The #LinphoneCall object @notnil
 * @return TRUE if part of a conference.
 * @deprecated 21/09/2017 Use linphone_call_get_conference() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t linphone_call_is_in_conference(const LinphoneCall *call);

/**
 * Returns whether or not the call is currently being recorded
 * @param call #LinphoneCall for which we can to know the recording state @notnil
 * @return TRUE if recording is in progress, FALSE otherwise
 * @deprecated 15/09/2021 Use linphone_call_params_is_recording() instead.
 **/
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t linphone_call_is_recording(LinphoneCall *call);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CALL_H_
