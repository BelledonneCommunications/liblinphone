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

#ifndef _L_C_PARTICIPANT_DEVICE_H_
#define _L_C_PARTICIPANT_DEVICE_H_

#include "time.h"

#include "linphone/api/c-types.h"
#include "linphone/api/c-participant-device-cbs.h"
#include "linphone/wrapper_utils.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

/**
 * Increment reference count of #LinphoneParticipantDevice object.
 * @param participant_device the #LinphoneParticipantDevice object @notnil
 * @return the same #LinphoneParticipantDevice object @notnil
 **/
LINPHONE_PUBLIC LinphoneParticipantDevice *linphone_participant_device_ref (LinphoneParticipantDevice *participant_device);

/**
 * Decrement reference count of #LinphoneParticipantDevice object.
 * @param participant_device the #LinphoneParticipantDevice object @notnil
 **/
LINPHONE_PUBLIC void linphone_participant_device_unref (LinphoneParticipantDevice *participant_device);

/**
 * Retrieve the user pointer associated with the participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The user pointer associated with the participant's device. @maybenil
**/
LINPHONE_PUBLIC void * linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device);

/**
 * Assign a user pointer to the participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @param user_data The user pointer to associate with the participant's device. @maybenil
**/
LINPHONE_PUBLIC void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *user_data);

/**
 * Get the address of a participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The #LinphoneAddress of the participant's device @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress* linphone_participant_device_get_address (const LinphoneParticipantDevice *participant_device);

/**
 * Get the state of a participant device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The #LinphoneParticipantDeviceState of the device
*/
LINPHONE_PUBLIC LinphoneParticipantDeviceState linphone_participant_device_get_state (const LinphoneParticipantDevice *participant_device);

/**
 * Get the security level of a participant's device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return The #LinphoneChatRoomSecurityLevel of the device
*/
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_participant_device_get_security_level (const LinphoneParticipantDevice *participant_device);

/**
 * Return the name of the device or NULL.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return the name of the device or NULL. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_participant_device_get_name (const LinphoneParticipantDevice *participant_device);

/**
 * Return whether the participant device is in a conference or not.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return a boolean to state whether the device is in a conference
 */
LINPHONE_PUBLIC bool_t linphone_participant_device_is_in_conference (const LinphoneParticipantDevice *participant_device);

/**
 * Get the timestamp the device joined a conference.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return time of joining a conference as returned by time(nullptr). For UNIX based systems it is the number of seconds since 00:00hours of the 1st of January 1970
 */
LINPHONE_PUBLIC time_t linphone_participant_device_get_time_of_joining(const LinphoneParticipantDevice *participant_device);

/**
 * Get the timestamp the device left a conference.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return time of disconnection a conference as returned by time(nullptr). For UNIX based systems it is the number of seconds since 00:00hours of the 1st of January 1970
 */
LINPHONE_PUBLIC time_t linphone_participant_device_get_time_of_disconnection(const LinphoneParticipantDevice *participant_device);

/**
 * Get the joining method or it the device is the focus owner
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return joining method or focus owner #LinphoneParticipantDeviceJoiningMethod
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceJoiningMethod linphone_participant_device_get_joining_method (const LinphoneParticipantDevice *participant_device);

/**
 * Get the disconnection method
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return disconnection method #LinphoneParticipantDeviceDisconnectionMethod
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceDisconnectionMethod linphone_participant_device_get_disconnection_method (const LinphoneParticipantDevice *participant_device);

/**
 * Get the disconnection reason
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return disconnection reason @maybenil
 */
LINPHONE_PUBLIC const char * linphone_participant_device_get_disconnection_reason (const LinphoneParticipantDevice *participant_device);

/**
 * Get the stream capability of the device.
 * The capability information represents the capability for the #ParticipantDevice to handle a given stream type (audio, video or text).
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @param stream_type A #LinphoneStreamType
 * @return the capability of stream of type stream_type of the device #LinphoneMediaDirection
 */
LINPHONE_PUBLIC LinphoneMediaDirection linphone_participant_device_get_stream_capability(const LinphoneParticipantDevice *participant_device, const LinphoneStreamType stream_type);

/**
 * Get the stream availability of the device.
 * The availability information represents whether a given stream type is currently available to be presented in the conference for a #LinphoneParticipantDevice
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @param stream_type A #LinphoneStreamType
 * @return TRUE if the stream of type stream_type is available for device, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_participant_device_get_stream_availability(const LinphoneParticipantDevice *participant_device, const LinphoneStreamType stream_type);

/**
 * Get the audio stream SSRC of the device.
 * @param participant A #LinphoneParticipantDevice object @notnil
 * @param stream_type A #LinphoneStreamType
 * @return the audio stream SSRC of the device
 */
LINPHONE_PUBLIC uint32_t linphone_participant_device_get_ssrc(const LinphoneParticipantDevice *participant_device, const LinphoneStreamType stream_type);

/**
 * Add a listener in order to be notified of #LinphoneParticipantDevice events. Once an event is received, registred #LinphoneParticipantDeviceCbs are
 * invoked sequencially.
 * @param participant_device #LinphoneParticipantDevice object. @notnil
 * @param cbs A #LinphoneParticipantDeviceCbs object holding the callbacks you need. A reference is taken by the #LinphoneParticipantDevice until you invoke linphone_participant_device_remove_callbacks(). @notnil
 */
LINPHONE_PUBLIC void linphone_participant_device_add_callbacks(LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs);

/**
 * Remove a listener from a #LinphoneParticipantDevice
 * @param participant_device #LinphoneParticipantDevice object. @notnil
 * @param cbs #LinphoneParticipantDeviceCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_participant_device_remove_callbacks(LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs);

/**
 * Gets the current LinphoneParticipantDeviceCbs.
 * @param participant_device #LinphoneParticipantDevice object. @notnil
 * @return The LinphoneParticipantDeviceCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceCbs *linphone_participant_device_get_current_callbacks(const LinphoneParticipantDevice *participant_device);

/**
 * Set window ID for a device.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @param window_id the window ID of the device @maybenil
 */
LINPHONE_PUBLIC void linphone_participant_device_set_native_video_window_id(LinphoneParticipantDevice *participant_device, void * window_id);

/**
 * Get window ID.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return the window ID of the device @maybenil
 */
LINPHONE_PUBLIC void * linphone_participant_device_get_native_video_window_id(const LinphoneParticipantDevice *participant_device);

/**
 * Create a window ID and return it.
 * @param participant_device A #LinphoneParticipantDevice object @notnil
 * @return the window ID of the device @maybenil
 */
LINPHONE_PUBLIC void * linphone_participant_device_create_native_video_window_id(const LinphoneParticipantDevice *participant_device);

/**
 * Return whether the participant device is speaking or not.
 * @param participant_device The #LinphoneParticipantDeviceCbs object @notnil
 * @return TRUE if the participant device is speaking, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_participant_device_get_is_speaking (const LinphoneParticipantDevice *participant_device);

/**
 * Return whether the participant device is muted or not.
 * @param participant_device The #LinphoneParticipantDeviceCbs object @notnil
 * @return TRUE if the participant device is muted, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_participant_device_get_is_muted (const LinphoneParticipantDevice *participant_device);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_H_
