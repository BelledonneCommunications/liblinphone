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

#ifndef LINPHONE_PAYLOAD_TYPE_H_
#define LINPHONE_PAYLOAD_TYPE_H_

#include "linphone/types.h"

/**
 * @addtogroup media_parameters
 * @{
**/


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Instantiates a new payload type with values from source.
 * @param[in] source The #LinphonePayloadType object to be cloned. @notnil
 * @return The newly created #LinphonePayloadType object. @notnil
 */
LINPHONE_PUBLIC LinphonePayloadType *linphone_payload_type_clone(const LinphonePayloadType *orig);

/**
 * Take a reference on a #LinphonePayloadType.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return the same #LinphonePayloadType object @notnil
 */
LINPHONE_PUBLIC LinphonePayloadType *linphone_payload_type_ref(LinphonePayloadType *payload_type);

/**
 * Release a reference on a #LinphonePayloadType.
 * @param payload_type the #LinphonePayloadType object @notnil
 */
LINPHONE_PUBLIC void linphone_payload_type_unref(LinphonePayloadType *payload_type);

/**
 * Get the type of a payload type.
 * @param payload_type The payload type. @notnil
 * @return The type of the payload e.g. PAYLOAD_AUDIO_CONTINUOUS or PAYLOAD_VIDEO.
 */
LINPHONE_PUBLIC int linphone_payload_type_get_type(const LinphonePayloadType *payload_type);

/**
 * Enable/disable a payload type.
 * @param payload_type The payload type to enable/disable. @notnil
 * @param enabled Set TRUE for enabling and FALSE for disabling.
 * @return 0 for success, -1 for failure.
 */
LINPHONE_PUBLIC int linphone_payload_type_enable(LinphonePayloadType *payload_type, bool_t enabled);

/**
 * Check whether a palyoad type is enabled.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return TRUE if enabled, FALSE if disabled.
 */
LINPHONE_PUBLIC bool_t linphone_payload_type_enabled(const LinphonePayloadType *payload_type);

/**
 * Return a string describing a payload type. The format of the string is
 * &lt;mime_type&gt;/&lt;clock_rate&gt;/&lt;channels&gt;.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The description of the payload type. Must be release after use. @notnil @tobefreed
 */
LINPHONE_PUBLIC char *linphone_payload_type_get_description(const LinphonePayloadType *payload_type);

/**
 * Get a description of the encoder used to provide a payload type.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The description of the encoder. Can be NULL if the payload type is not supported by Mediastreamer2. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_payload_type_get_encoder_description(const LinphonePayloadType *payload_type);

/**
 * Get the normal bitrate in bits/s.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The normal bitrate in bits/s or -1 if an error has occured.
 */
LINPHONE_PUBLIC int linphone_payload_type_get_normal_bitrate(const LinphonePayloadType *payload_type);

/**
 * Change the normal bitrate of a payload type..
 * @param payload_type the #LinphonePayloadType object @notnil
 * @param bitrate The new bitrate in kbits/s.
 */
LINPHONE_PUBLIC void linphone_payload_type_set_normal_bitrate(LinphonePayloadType *payload_type, int bitrate);

/**
 * Get the mime type.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The mime type. @notnil
 */
LINPHONE_PUBLIC const char * linphone_payload_type_get_mime_type(const LinphonePayloadType *payload_type);

/**
 * Get the number of channels.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The number of channels.
 */
LINPHONE_PUBLIC int linphone_payload_type_get_channels(const LinphonePayloadType *payload_type);

/**
 * Returns the payload type number assigned for this codec.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The number of the payload type.
**/
LINPHONE_PUBLIC int linphone_payload_type_get_number(const LinphonePayloadType *payload_type);

/**
 * Force a number for a payload type. The #LinphoneCore does payload type number assignment automatically.
 * This function is mainly to be used for tests, in order to override the automatic assignment mechanism.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @param number The number to assign to the payload type.
**/
LINPHONE_PUBLIC void linphone_payload_type_set_number(LinphonePayloadType *payload_type, int number);

/**
 * Get the format parameters for incoming streams.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The format parameters as string. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_payload_type_get_recv_fmtp(const LinphonePayloadType *payload_type);

/**
 * Set the format parameters for incoming streams.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @param recv_fmtp The new format parameters as string. The string will be copied. @maybenil
 */
LINPHONE_PUBLIC void linphone_payload_type_set_recv_fmtp(LinphonePayloadType *payload_type, const char *recv_fmtp);

/**
 * Get the format parameters for outgoing streams.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The format parameters as string. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_payload_type_get_send_fmtp(const LinphonePayloadType *payload_type);

/**
 * Set the format parameters for outgoing streams.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @param send_fmtp The new format parameters as string. The string will be copied. @maybenil
 */
LINPHONE_PUBLIC void linphone_payload_type_set_send_fmtp(LinphonePayloadType *payload_type, const char *send_fmtp);

/**
 * Get the clock rate of a payload type.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return The clock rate in Hz.
 */
LINPHONE_PUBLIC int linphone_payload_type_get_clock_rate(const LinphonePayloadType *payload_type);

/**
 * Tells whether the specified payload type represents a variable bitrate codec.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return TRUE if the payload type represents a VBR codec, FALSE instead.
 */
LINPHONE_PUBLIC bool_t linphone_payload_type_is_vbr(const LinphonePayloadType *payload_type);

/**
 * Check whether the payload is usable according the bandwidth targets set in the core.
 * @param payload_type the #LinphonePayloadType object @notnil
 * @return TRUE if the payload type is usable.
 */
LINPHONE_PUBLIC bool_t linphone_payload_type_is_usable(const LinphonePayloadType *payload_type);


#ifdef __cplusplus
}
#endif

/**
 * @}
**/

#endif /* LINPHONE_PAYLOAD_TYPE_H_ */
