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


#ifndef __LINPHONE_CALL_PARAMS_H__
#define __LINPHONE_CALL_PARAMS_H__


#include "linphone/types.h"
#include "linphone/payload_type.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup call_control
 * @{
**/

/*******************************************************************************
 * Public functions                                                            *
 ******************************************************************************/

/**
 * Add a custom SIP header in the INVITE for a call.
 * @param call_params The #LinphoneCallParams to add a custom SIP header to. @notnil
 * @param header_name The name of the header to add. @notnil
 * @param header_value The content of the header to add. @maybenil
**/
LINPHONE_PUBLIC void linphone_call_params_add_custom_header(LinphoneCallParams *call_params, const char *header_name, const char *header_value);

/**
 * Copy an existing #LinphoneCallParams object to a new #LinphoneCallParams object.
 * @param call_params The #LinphoneCallParams object to copy. @notnil
 * @return A copy of the #LinphoneCallParams object. @notnil
**/
LINPHONE_PUBLIC LinphoneCallParams * linphone_call_params_copy(const LinphoneCallParams *call_params);

/**
 * Indicate whether sending of early media was enabled.
 * @param call_params #LinphoneCallParams object @notnil
 * @return A boolean value telling whether sending of early media was enabled.
**/
LINPHONE_PUBLIC bool_t linphone_call_params_early_media_sending_enabled(const LinphoneCallParams *call_params);

/**
 * Enable sending of real early media (during outgoing calls).
 * @param call_params #LinphoneCallParams object @notnil
 * @param enabled A boolean value telling whether to enable early media sending or not.
**/
LINPHONE_PUBLIC void linphone_call_params_enable_early_media_sending(LinphoneCallParams *call_params, bool_t enabled);

/**
 * Indicate low bandwith mode.
 * Configuring a call to low bandwidth mode will result in the core to activate several settings for the call in order to ensure that bitrate usage
 * is lowered to the minimum possible. Typically, ptime (packetization time) will be increased, audio codec's output bitrate will be targetted to 20kbit/s provided
 * that it is achievable by the codec selected after SDP handshake. Video is automatically disabled.
 * @param call_params #LinphoneCallParams object @notnil
 * @param enabled A boolean value telling whether to activate the low bandwidth mode or not.
**/
LINPHONE_PUBLIC void linphone_call_params_enable_low_bandwidth(LinphoneCallParams *call_params, bool_t enabled);

/**
 * Enable audio stream.
 * @param call_params #LinphoneCallParams object @notnil
 * @param enabled A boolean value telling whether to enable audio or not.
**/
LINPHONE_PUBLIC void linphone_call_params_enable_audio(LinphoneCallParams *call_params, bool_t enabled);

/**
 * Enable video stream.
 * @param call_params #LinphoneCallParams object @notnil
 * @param enabled A boolean value telling whether to enable video or not.
**/
LINPHONE_PUBLIC void linphone_call_params_enable_video(LinphoneCallParams *call_params, bool_t enabled);

/**
 * Get a custom SIP header.
 * @param call_params The #LinphoneCallParams to get the custom SIP header from. @notnil
 * @param header_name The name of the header to get. @notnil
 * @return The content of the header or NULL if not found. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_call_params_get_custom_header(const LinphoneCallParams *call_params, const char *header_name);

/**
 * Tell whether the call is part of the locally managed conference.
 * @warning If a conference server is used to manage conferences,
 * that function does not return TRUE even if the conference is running.<br/>
 * If you want to test whether the conference is running, you should test
 * whether linphone_core_get_conference() return a non-null pointer.
 * @param call_params #LinphoneCallParams object @notnil
 * @return A boolean value telling whether the call is part of the locally managed conference.
**/
LINPHONE_PUBLIC bool_t linphone_call_params_get_local_conference_mode(const LinphoneCallParams *call_params);

/**
 * Get the kind of media encryption selected for the call.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The kind of #LinphoneMediaEncryption selected for the call.
**/
LINPHONE_PUBLIC LinphoneMediaEncryption linphone_call_params_get_media_encryption(const LinphoneCallParams *call_params);

/**
 * Get requested level of privacy for the call.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The #LinphonePrivacyMask used for the call.
**/
LINPHONE_PUBLIC LinphonePrivacyMask linphone_call_params_get_privacy(const LinphoneCallParams *call_params);

/**
 * Get the framerate of the video that is received.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The actual received framerate in frames per seconds, 0 if not available.
 */
LINPHONE_PUBLIC float linphone_call_params_get_received_framerate(const LinphoneCallParams *call_params);

/**
 * Get the definition of the received video.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The received #LinphoneVideoDefinition or NULL. @maybenil
 */
LINPHONE_PUBLIC const LinphoneVideoDefinition * linphone_call_params_get_received_video_definition(const LinphoneCallParams *call_params);

/**
 * Get the path for the audio recording of the call.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The path to the audio recording of the call or NULL. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_call_params_get_record_file(const LinphoneCallParams *call_params);

/**
 * Get the RTP profile being used.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The RTP profile. @notnil
 */
LINPHONE_PUBLIC const char * linphone_call_params_get_rtp_profile(const LinphoneCallParams *call_params);

/**
 * Get the framerate of the video that is sent.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The actual sent framerate in frames per seconds, 0 if not available.
 */
LINPHONE_PUBLIC float linphone_call_params_get_sent_framerate(const LinphoneCallParams *call_params);

/**
 * Get the definition of the sent video.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The sent #LinphoneVideoDefinition or NULL. @maybenil
 */
LINPHONE_PUBLIC const LinphoneVideoDefinition * linphone_call_params_get_sent_video_definition(const LinphoneCallParams *call_params);

/**
 * Get the session name of the media session (ie in SDP).
 * Subject from the SIP message can be retrieved using linphone_call_params_get_custom_header() and is different.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The session name of the media session or NULL. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_call_params_get_session_name(const LinphoneCallParams *call_params);

/**
 * Get the audio payload type that has been selected by a call.
 * @param call_params The #LinphoneCallParams object. @notnil
 * @return The selected #LinphonePayloadType. NULL is returned if no audio payload type has been selected by the call. @maybenil
**/
LINPHONE_PUBLIC LinphonePayloadType *linphone_call_params_get_used_audio_payload_type(const LinphoneCallParams *call_params);

/**
 * Get the video payload type that has been selected by a call.
 * @param call_params The #LinphoneCallParams object. @notnil
 * @return The selected #LinphonePayloadType. NULL is returned if no video payload type has been selected by the call. @maybenil
**/
LINPHONE_PUBLIC LinphonePayloadType *linphone_call_params_get_used_video_payload_type(const LinphoneCallParams *call_params);

/**
 * Get the text payload type that has been selected by a call.
 * @param call_params The #LinphoneCallParams object. @notnil
 * @return The selected #LinphonePayloadType. NULL is returned if no text payload type has been selected by the call. @maybenil
**/
LINPHONE_PUBLIC LinphonePayloadType *linphone_call_params_get_used_text_payload_type(const LinphoneCallParams *call_params);

/**
 * Tell whether the call has been configured in low bandwidth mode or not.
 * This mode can be automatically discovered thanks to a stun server when activate_edge_workarounds=1 in section [net] of configuration file.
 * An application that would have reliable way to know network capacity may not use activate_edge_workarounds=1 but instead manually configure
 * low bandwidth mode with linphone_call_params_enable_low_bandwidth().
 * When enabled, this param may transform a call request with video in audio only mode.
 * @param call_params #LinphoneCallParams object @notnil
 * @return A boolean value telling whether the low bandwidth mode has been configured/detected.
 */
LINPHONE_PUBLIC bool_t linphone_call_params_low_bandwidth_enabled(const LinphoneCallParams *call_params);

/**
 * Refine bandwidth settings for this call by setting a bandwidth limit for audio streams.
 * As a consequence, codecs whose bitrates are not compatible with this limit won't be used.
 * @param call_params #LinphoneCallParams object @notnil
 * @param bandwidth The audio bandwidth limit to set in kbit/s.
**/
LINPHONE_PUBLIC void linphone_call_params_set_audio_bandwidth_limit(LinphoneCallParams *call_params, int bandwidth);

/**
 * Set requested media encryption for a call.
 * @param call_params #LinphoneCallParams object @notnil
 * @param encryption The #LinphoneMediaEncryption to use for the call.
**/
LINPHONE_PUBLIC void linphone_call_params_set_media_encryption(LinphoneCallParams *call_params, LinphoneMediaEncryption encryption);

/**
 * Set requested level of privacy for the call.
 * @param call_params #LinphoneCallParams object @notnil
 * @param privacy The #LinphonePrivacyMask to used for the call.
**/
LINPHONE_PUBLIC void linphone_call_params_set_privacy(LinphoneCallParams *call_params, LinphonePrivacyMask privacy);

/**
 * Enable recording of the call.
 * This function must be used before the call parameters are assigned to the call.
 * The call recording can be started and paused after the call is established with
 * linphone_call_start_recording() and linphone_call_pause_recording().
 * @param call_params #LinphoneCallParams object @notnil
 * @param path A string containing the path and filename of the file where audio/video streams are to be written.
 * The filename must have either .mkv or .wav extention. The video stream will be written only if a MKV file is given. @maybenil
**/
LINPHONE_PUBLIC void linphone_call_params_set_record_file(LinphoneCallParams *call_params, const char *path);

/**
 * Set the session name of the media session (ie in SDP).
 * Subject from the SIP message (which is different) can be set using linphone_call_params_set_custom_header().
 * @param call_params #LinphoneCallParams object @notnil
 * @param name The session name to be used. @maybenil
**/
LINPHONE_PUBLIC void linphone_call_params_set_session_name(LinphoneCallParams *call_params, const char *name);

/**
 * Tell whether audio is enabled or not.
 * @param call_params #LinphoneCallParams object @notnil
 * @return A boolean value telling whether audio is enabled or not.
**/
LINPHONE_PUBLIC bool_t linphone_call_params_audio_enabled(const LinphoneCallParams *call_params);

/**
 * Tell whether video is enabled or not.
 * @param call_params #LinphoneCallParams object @notnil
 * @return A boolean value telling whether video is enabled or not.
**/
LINPHONE_PUBLIC bool_t linphone_call_params_video_enabled(const LinphoneCallParams *call_params);

/**
 * Get the audio stream direction.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The audio stream #LinphoneMediaDirection associated with the call params.
**/
LINPHONE_PUBLIC LinphoneMediaDirection linphone_call_params_get_audio_direction(const LinphoneCallParams *call_params);

/**
 * Get the video stream direction.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The video stream #LinphoneMediaDirection associated with the call params.
**/
LINPHONE_PUBLIC LinphoneMediaDirection linphone_call_params_get_video_direction(const LinphoneCallParams *call_params);

/**
 * Set the audio stream direction.
 * @param call_params #LinphoneCallParams object @notnil
 * @param direction The audio stream #LinphoneMediaDirection associated with this call params.
**/
LINPHONE_PUBLIC void linphone_call_params_set_audio_direction(LinphoneCallParams *call_params, LinphoneMediaDirection direction);

/**
 * Set the video stream direction.
 * @param call_params #LinphoneCallParams object @notnil
 * @param direction The video stream #LinphoneMediaDirection associated with this call params.
**/
LINPHONE_PUBLIC void linphone_call_params_set_video_direction(LinphoneCallParams *call_params, LinphoneMediaDirection direction);

/**
 * Set the #LinphoneProxyConfig to use for the call
 *
 * @param params the #LinphoneCallParams @notnil
 * @param proxy_config The #LinphoneProxyConfig to use, or NULL if none has been selected. The #LinphoneCallParams keep a reference on it and remove the older if it exists. @maybenil
 **/
LINPHONE_PUBLIC void linphone_call_params_set_proxy_config(LinphoneCallParams *params, LinphoneProxyConfig *proxy_config);

/**
 * Get the #LinphoneProxyConfig that is used for the call
 *
 * @param params the #LinphoneCallParams @notnil
 * @return The selected #LinphoneProxyConfig for the call, or NULL if none has been selected. @maybenil
 **/
LINPHONE_PUBLIC LinphoneProxyConfig *linphone_call_params_get_proxy_config(const LinphoneCallParams *params);



/**
 * Set the video stream direction.
 * @param[in] params #LinphoneCallParams object
 * @param[in] enable wether or not AVPF should be enabled for this call
**/
LINPHONE_PUBLIC void linphone_call_params_enable_avpf(LinphoneCallParams *params, bool_t enable);

int linphone_call_params_get_audio_bandwidth_limit(const LinphoneCallParams *params);
bool_t linphone_call_params_real_early_media_enabled(const LinphoneCallParams *params);
bool_t linphone_call_params_avpf_enabled(const LinphoneCallParams *params);
bool_t linphone_call_params_has_avpf_enabled_been_set(const LinphoneCallParams *params);
bool_t linphone_call_params_mandatory_media_encryption_enabled(const LinphoneCallParams *params);
void linphone_call_params_enable_mandatory_media_encryption(LinphoneCallParams *params, bool_t value);
uint16_t linphone_call_params_get_avpf_rr_interval(const LinphoneCallParams *params);
void linphone_call_params_set_avpf_rr_interval(LinphoneCallParams *params, uint16_t value);


/*******************************************************************************
 * Reference and user data handling functions                                  *
 ******************************************************************************/

/**
 * Get the user data associated with the call params.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The user data associated with the call params. @maybenil
**/
LINPHONE_PUBLIC void *linphone_call_params_get_user_data(const LinphoneCallParams *call_params);

/**
 * Assign a user data to the call params.
 * @param call_params #LinphoneCallParams object @notnil
 * @param user_data The user data to associate with the call params. @maybenil
**/
LINPHONE_PUBLIC void linphone_call_params_set_user_data(LinphoneCallParams *call_params, void *user_data);

/**
 * Acquire a reference to the call params.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The same #LinphoneCallParams object @notnil
**/
LINPHONE_PUBLIC LinphoneCallParams * linphone_call_params_ref(LinphoneCallParams *call_params);

/**
 * Release a reference to the call params.
 * @param call_params #LinphoneCallParams object @notnil
**/
LINPHONE_PUBLIC void linphone_call_params_unref(LinphoneCallParams *call_params);


/**
 * Use to enable multicast rtp for audio stream.
 * * If enabled, outgoing calls put a multicast address from #linphone_core_get_video_multicast_addr() into audio cline. In case of outgoing call audio stream is sent to this multicast address.
 * <br> For incoming calls behavior is unchanged.
 * @param params #LinphoneCallParams @notnil
 * @param yesno if yes, subsequent calls will propose multicast ip set by #linphone_core_set_audio_multicast_addr()
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_enable_audio_multicast(LinphoneCallParams *params, bool_t yesno);

/**
 * Use to get multicast state of audio stream.
 * @param params #LinphoneCallParams @notnil
 * @return true if  subsequent calls will propose multicast ip set by #linphone_core_set_audio_multicast_addr()
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC bool_t linphone_call_params_audio_multicast_enabled(const LinphoneCallParams *params);

/**
 * Use to enable multicast rtp for video stream.
 * If enabled, outgoing calls put a multicast address from #linphone_core_get_video_multicast_addr() into video cline. In case of outgoing call video stream is sent to this  multicast address.
 * <br> For incoming calls behavior is unchanged.
 * @param params #LinphoneCallParams @notnil
 * @param yesno if yes, subsequent outgoing calls will propose multicast ip set by #linphone_core_set_video_multicast_addr()
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_enable_video_multicast(LinphoneCallParams *params, bool_t yesno);

/**
 * Use to get multicast state of video stream.
 * @param params #LinphoneCallParams @notnil
 * @return true if  subsequent calls will propose multicast ip set by #linphone_core_set_video_multicast_addr()
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC bool_t linphone_call_params_video_multicast_enabled(const LinphoneCallParams *params);

/**
 * Use to enable real time text following rfc4103.
 * If enabled, outgoing calls put a m=text line in SDP offer .
 * @param params #LinphoneCallParams @notnil
 * @param yesno if yes, subsequent outgoing calls will propose rtt
 * @return 0
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC LinphoneStatus linphone_call_params_enable_realtime_text(LinphoneCallParams *params, bool_t yesno);

/**
 * Use to get real time text following rfc4103.
 * @param params #LinphoneCallParams @notnil
 * @return returns true if call rtt is activated.
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC bool_t linphone_call_params_realtime_text_enabled(const LinphoneCallParams *params);

/**
 * Use to set keep alive interval for real time text following rfc4103.
 * @param params #LinphoneCallParams @notnil
 * @param interval The keep alive interval for real time text, 25000 by default.
 * @ingroup media_parameters
 **/
LINPHONE_PUBLIC void linphone_call_params_set_realtime_text_keepalive_interval(LinphoneCallParams *params, unsigned int interval);

/**
 * Use to get keep alive interval of real time text following rfc4103.
 * @param params #LinphoneCallParams @notnil
 * @return returns keep alive interval of real time text.
 * @ingroup media_parameters
 **/
LINPHONE_PUBLIC unsigned int linphone_call_params_get_realtime_text_keepalive_interval(const LinphoneCallParams *params);

/**
 * Add a custom attribute related to all the streams in the SDP exchanged within SIP messages during a call.
 * @param params The #LinphoneCallParams to add a custom SDP attribute to. @notnil
 * @param attribute_name The name of the attribute to add. @notnil
 * @param attribute_value The content value of the attribute to add. @maybenil
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_add_custom_sdp_attribute(LinphoneCallParams *params, const char *attribute_name, const char *attribute_value);

/**
 * Add a custom attribute related to a specific stream in the SDP exchanged within SIP messages during a call.
 * @param params The #LinphoneCallParams to add a custom SDP attribute to. @notnil
 * @param type The type of the stream to add a custom SDP attribute to.
 * @param attribute_name The name of the attribute to add. @notnil
 * @param attribute_value The content value of the attribute to add. @maybenil
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_add_custom_sdp_media_attribute(LinphoneCallParams *params, LinphoneStreamType type, const char *attribute_name, const char *attribute_value);

/**
 * Get a custom SDP attribute that is related to all the streams.
 * @param params The #LinphoneCallParams to get the custom SDP attribute from. @notnil
 * @param attribute_name The name of the attribute to get. @notnil
 * @return The content value of the attribute or NULL if not found. @maybenil
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC const char * linphone_call_params_get_custom_sdp_attribute(const LinphoneCallParams *params, const char *attribute_name);

/**
 * Get a custom SDP attribute that is related to a specific stream.
 * @param params The #LinphoneCallParams to get the custom SDP attribute from. @notnil
 * @param type The type of the stream to add a custom SDP attribute to.
 * @param attribute_name The name of the attribute to get. @notnil
 * @return The content value of the attribute or NULL if not found. @maybenil
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC const char * linphone_call_params_get_custom_sdp_media_attribute(const LinphoneCallParams *params, LinphoneStreamType type, const char *attribute_name);

/**
 * Clear the custom SDP attributes related to all the streams in the SDP exchanged within SIP messages during a call.
 * @param params The #LinphoneCallParams to clear the custom SDP attributes from. @notnil
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_clear_custom_sdp_attributes(LinphoneCallParams *params);

/**
 * Clear the custom SDP attributes related to a specific stream in the SDP exchanged within SIP messages during a call.
 * @param params The #LinphoneCallParams to clear the custom SDP attributes from. @notnil
 * @param type The type of the stream to clear the custom SDP attributes from.
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_clear_custom_sdp_media_attributes(LinphoneCallParams *params, LinphoneStreamType type);

/**
 * Gets a list of #LinphoneContent set if exists
 * @param params The #LinphoneCallParams to get the custom Content from. @notnil
 * @return A list of #LinphoneContent set if exists, NULL otherwise. \bctbx_list{LinphoneContent}  @maybenil @tobefreed
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC bctbx_list_t* linphone_call_params_get_custom_contents (const LinphoneCallParams *params);

/**
 * Adds a #LinphoneContent to be added to the INVITE SDP. 
 * @param params The #LinphoneCallParams in which to set the custom #LinphoneContent. @notnil
 * @param content The #LinphoneContent to be added. @notnil
 * @ingroup media_parameters
**/
LINPHONE_PUBLIC void linphone_call_params_add_custom_content (LinphoneCallParams *params, LinphoneContent *content);

/**
 * Indicates whether RTP bundle mode (also known as Media Multiplexing) is enabled.
 * See https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-54 for more information.
 * @param params the #LinphoneCallParams @notnil
 * @return a boolean indicating the enablement of rtp bundle mode.
 * @ingroup media_parameters
 */
LINPHONE_PUBLIC bool_t linphone_call_params_rtp_bundle_enabled(const LinphoneCallParams *params);

/**
 * Enables or disables RTP bundle mode (Media Multiplexing).
 * See https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-54 for more information about the feature.
 * When enabled, liblinphone will try to negociate the use of a single port for all streams.
 * It automatically enables rtcp-mux.
 * @param params the #LinphoneCallParams @notnil
 * @param value a boolean to indicate whether the feature is to be enabled.
 * @ingroup media_parameters
 */
LINPHONE_PUBLIC void linphone_call_params_enable_rtp_bundle(LinphoneCallParams *params, bool_t value);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * @brief Get the size of the video that is received.
 * @param call_params #LinphoneCallParams object @notnil
 * @return The received video size or MS_VIDEO_SIZE_UNKNOWN if not available.
 * @deprecated 22/11/2017 Use #linphone_call_params_get_received_video_definition() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED MSVideoSize linphone_call_params_get_received_video_size(const LinphoneCallParams *call_params);

/**
 * @brief Gets the size of the video that is sent.
 * @param call_params #LinphoneCalParams object @notnil
 * @return The sent video size or MS_VIDEO_SIZE_UNKNOWN if not available.
 * @deprecated 22/11/2017 Use #linphone_call_params_get_sent_video_definition() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED MSVideoSize linphone_call_params_get_sent_video_size(const LinphoneCallParams *call_params);

/**
 * Get the audio payload type that has been selected by a call.
 * @param call_params The #LinphoneCallParams object. @notnil
 * @return The selected payload type. NULL is returned if no audio payload type has been seleced by the call.
 * @deprecated 31/03/2017 Use linphone_call_params_get_used_audio_payload_type() instead.
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED const OrtpPayloadType *linphone_call_params_get_used_audio_codec(const LinphoneCallParams *call_params);

/**
 * Get the video payload type that has been selected by a call.
 * @param call_params The #LinphoneCallParams object. @notnil
 * @return The selected payload type. NULL is returned if no video payload type has been seleced by the call.
 * @deprecated 31/03/2017 Use linphone_call_params_get_used_video_payload_type() instead.
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED const OrtpPayloadType *linphone_call_params_get_used_video_codec(const LinphoneCallParams *call_params);

/**
 * Get the text payload type that has been selected by a call.
 * @param call_params The #LinphoneCallParams object. @notnil
 * @return The selected payload type. NULL is returned if no text payload type has been seleced by the call.
 * @deprecated 31/03/2017 Use linphone_call_params_get_used_text_payload_type() instead.
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED const OrtpPayloadType *linphone_call_params_get_used_text_codec(const LinphoneCallParams *call_params);

/**
 * @}
**/

#ifdef __cplusplus
}
#endif

#endif /* __LINPHONE_CALL_PARAMS_H__ */
