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

#ifndef LINPHONE_ERROR_INFO_H
#define LINPHONE_ERROR_INFO_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup misc
 * @{
 */

/**
 * Create an empty #LinphoneErrorInfo object.
 * The #LinphoneErrorInfo object carries these fields:
 * - a #LinphoneReason enum member giving overall signification of the error reported.
 * - the "protocol" name in which the protocol reason code has meaning, for example SIP or Q.850
 * - the "protocol code", an integer referencing the kind of error reported
 * - the "phrase", a text phrase describing the error
 * - the "warning", the content of warning headers if any
 * - a sub "LinphoneErrorInfo" may be provided if a SIP response includes a Reason header (RFC3326).
 * @return a new #LinphoneErrorInfo object @notnil
**/
LINPHONE_PUBLIC LinphoneErrorInfo *linphone_error_info_new(void);

/**
 * Increment refcount.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return the same #LinphoneErrorInfo object @notnil
**/
LINPHONE_PUBLIC LinphoneErrorInfo *linphone_error_info_ref(LinphoneErrorInfo *error_info);

/**
 * Decrement refcount and possibly free the object.
 * @param error_info #LinphoneErrorInfo object @notnil
**/
LINPHONE_PUBLIC void linphone_error_info_unref(LinphoneErrorInfo *error_info);

/**
* Get Retry-After delay second from the error info.
* @param error_info #LinphoneErrorInfo object @notnil
* @return The Retry-After delay second
**/
LINPHONE_PUBLIC int linphone_error_info_get_retry_after(const LinphoneErrorInfo *error_info);

/**
 * Get reason code from the error info.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return A #LinphoneReason object
**/
LINPHONE_PUBLIC LinphoneReason linphone_error_info_get_reason(const LinphoneErrorInfo *error_info);

/**
 * Get pointer to chained #LinphoneErrorInfo set in sub_ei.
 * It corresponds to a Reason header in a received SIP response.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return #LinphoneErrorInfo pointer defined in the ei object. @maybenil
 */
LINPHONE_PUBLIC LinphoneErrorInfo* linphone_error_info_get_sub_error_info(const LinphoneErrorInfo *error_info);

/**
 * Get textual phrase from the error info.
 * This is the text that is provided by the peer in the protocol (SIP).
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return The error phrase @maybenil
**/
LINPHONE_PUBLIC const char * linphone_error_info_get_phrase(const LinphoneErrorInfo *error_info);
	
/**
 * Get protocol from the error info.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return The protocol. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_error_info_get_protocol(const LinphoneErrorInfo *error_info);

/**
 * Provides additional information regarding the failure.
 * With SIP protocol, the content of "Warning" headers are returned.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return More details about the failure. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_error_info_get_warnings(const LinphoneErrorInfo *error_info);


/**
 * Get the status code from the low level protocol (ex a SIP status code).
 * @param error_info #LinphoneErrorInfo object @notnil
 * @return The status code
**/
LINPHONE_PUBLIC int linphone_error_info_get_protocol_code(const LinphoneErrorInfo *error_info);

/**
 * Assign information to a #LinphoneErrorInfo object.
 * @param error_info    #LinphoneErrorInfo object @notnil
 * @param protocol      protocol name @maybenil
 * @param reason        reason  from #LinphoneReason enum
 * @param code          protocol code
 * @param status        description of the reason @maybenil
 * @param warning       warning message @maybenil
 */
LINPHONE_PUBLIC void linphone_error_info_set(LinphoneErrorInfo *error_info, const char *protocol, LinphoneReason reason, int code, const char *status, const char *warning);
	
/**
 * Set the sub_ei in #LinphoneErrorInfo to another #LinphoneErrorInfo. 
 * Used when a reason header is to be added in a SIP response. The first level #LinphoneErrorInfo defines the SIP response code and phrase,
 * the second (sub) #LinphoneErroInfo defining the content of the Reason header.
 * @param error_info #LinphoneErrorInfo object to which the other #LinphoneErrorInfo will be appended as ei->sub_ei. @notnil
 * @param appended_error_info #LinphoneErrorInfo to append @maybenil
 */
LINPHONE_PUBLIC void linphone_error_info_set_sub_error_info(LinphoneErrorInfo *error_info, LinphoneErrorInfo *appended_error_info);

/**
* Assign retry-after value to a #LinphoneErrorInfo object.
* @param error_info #LinphoneErrorInfo object @notnil
* @param retry_after the retry-after value
*/
LINPHONE_PUBLIC void linphone_error_info_set_retry_after(LinphoneErrorInfo *error_info, int retry_after);

/**
 * Assign reason #LinphoneReason to a #LinphoneErrorInfo object.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @param reason reason from #LinphoneReason enum
 */
LINPHONE_PUBLIC void linphone_error_info_set_reason(LinphoneErrorInfo *error_info, LinphoneReason reason);

/**
 * Assign protocol name to a #LinphoneErrorInfo object.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @param protocol the protocol name @maybenil
 */
LINPHONE_PUBLIC void linphone_error_info_set_protocol(LinphoneErrorInfo *error_info, const char *protocol);

/**
 * Assign protocol code to a #LinphoneErrorInfo object.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @param code the protocol code
 */
LINPHONE_PUBLIC void linphone_error_info_set_protocol_code(LinphoneErrorInfo *error_info, int code);

/**
 * Assign phrase to a #LinphoneErrorInfo object.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @param phrase the phrase explaining the error @maybenil
 */
LINPHONE_PUBLIC void linphone_error_info_set_phrase(LinphoneErrorInfo *error_info, const char *phrase);

/**
 * Assign warnings to a #LinphoneErrorInfo object.
 * @param error_info #LinphoneErrorInfo object @notnil
 * @param warnings the warnings @maybenil
 */
LINPHONE_PUBLIC void linphone_error_info_set_warnings(LinphoneErrorInfo *error_info, const char *warnings);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ERROR_INFO_H */
