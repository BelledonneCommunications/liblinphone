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


#ifndef __LINPHONE_CALL_LOG_H__
#define __LINPHONE_CALL_LOG_H__

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup call_logs
 * @{
**/

/*******************************************************************************
 * Public functions                                                            *
 ******************************************************************************/

/**
 * Get the call ID used by the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The call ID used by the call as a string. @notnil
**/
LINPHONE_PUBLIC const char * linphone_call_log_get_call_id(const LinphoneCallLog *call_log);

/**
 * Get the direction of the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The #LinphoneCallDir of the call.
**/
LINPHONE_PUBLIC LinphoneCallDir linphone_call_log_get_dir(const LinphoneCallLog *call_log);

/**
 * Get the duration of the call since connected.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The duration of the call in seconds.
**/
LINPHONE_PUBLIC int linphone_call_log_get_duration(const LinphoneCallLog *call_log);

/**
 * Get the origin address (ie from) of the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The origin #LinphoneAddress (ie from) of the call. @notnil
**/
LINPHONE_PUBLIC const LinphoneAddress * linphone_call_log_get_from_address(const LinphoneCallLog *call_log);

/**
 * Get the RTP statistics computed locally regarding the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The RTP statistics that have been computed locally for the call.
 * @donotwrap
**/
LINPHONE_PUBLIC const rtp_stats_t * linphone_call_log_get_local_stats(const LinphoneCallLog *call_log);

/**
 * Get the overall quality indication of the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The overall quality indication of the call.
**/
LINPHONE_PUBLIC float linphone_call_log_get_quality(const LinphoneCallLog *call_log);

/**
 * Get the persistent reference key associated to the call log.
 *
 * The reference key can be for example an id to an external database.
 * It is stored in the config file, thus can survive to process exits/restarts.
 *
 * @param call_log #LinphoneCallLog object @notnil
 * @return The reference key string that has been associated to the call log, or NULL if none has been associated. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_call_log_get_ref_key(const LinphoneCallLog *call_log);

/**
 * Get the local address (that is from or to depending on call direction)
 * @param call_log LinphoneCallLog object @notnil
 * @return The local #LinphoneAddress of the call @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_call_log_get_local_address(const LinphoneCallLog *call_log);

/**
 * Get the remote address (that is from or to depending on call direction).
 * @param call_log #LinphoneCallLog object @notnil
 * @return The remote #LinphoneAddress of the call. @notnil
**/
LINPHONE_PUBLIC const LinphoneAddress * linphone_call_log_get_remote_address(const LinphoneCallLog *call_log);

/**
 * Get the RTP statistics computed by the remote end and sent back via RTCP.
 * @note Not implemented yet.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The RTP statistics that have been computed by the remote end for the call.
 * @donotwrap
**/
LINPHONE_PUBLIC const rtp_stats_t * linphone_call_log_get_remote_stats(const LinphoneCallLog *call_log);

/**
 * Get the start date of the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The date of the beginning of the call.
**/
LINPHONE_PUBLIC time_t linphone_call_log_get_start_date(const LinphoneCallLog *call_log);

/**
 * Get the status of the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The #LinphoneCallStatus of the call.
**/
LINPHONE_PUBLIC LinphoneCallStatus linphone_call_log_get_status(const LinphoneCallLog *call_log);

/**
 * Get the destination address (ie to) of the call.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The destination #LinphoneAddress (ie to) of the call. @notnil
**/
LINPHONE_PUBLIC const LinphoneAddress * linphone_call_log_get_to_address(const LinphoneCallLog *call_log);

/**
 * Associate a persistent reference key to the call log.
 *
 * The reference key can be for example an id to an external database.
 * It is stored in the config file, thus can survive to process exits/restarts.
 *
 * @param call_log #LinphoneCallLog object @notnil
 * @param refkey The reference key string to associate to the call log. @maybenil
**/
LINPHONE_PUBLIC void linphone_call_log_set_ref_key(LinphoneCallLog *call_log, const char *refkey);

/**
 * Tell whether video was enabled at the end of the call or not.
 * @param call_log #LinphoneCallLog object @notnil
 * @return A boolean value telling whether video was enabled at the end of the call.
**/
LINPHONE_PUBLIC bool_t linphone_call_log_video_enabled(const LinphoneCallLog *call_log);

/**
 * Get a human readable string describing the call.
 * @note: the returned string must be freed by the application (use ms_free()).
 * @param call_log #LinphoneCallLog object @notnil
 * @return A human readable string describing the call. @notnil
**/
LINPHONE_PUBLIC char * linphone_call_log_to_str(const LinphoneCallLog *call_log);

/**
 * Tells whether that call was a call to a conference server
 * @param call_log #LinphoneCallLog object @notnil
 * @return TRUE if the call was a call to a conference server, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_call_log_was_conference(const LinphoneCallLog *call_log);

/**
 * When the call was failed, return an object describing the failure.
 * @param call_log #LinphoneCallLog object @notnil
 * @return #LinphoneErrorInfo about the error encountered by the call associated with this call log or NULL. @maybenil
**/
LINPHONE_PUBLIC const LinphoneErrorInfo *linphone_call_log_get_error_info(const LinphoneCallLog *call_log);


/*******************************************************************************
 * Reference and user data handling functions                                  *
 ******************************************************************************/

/**
 * Get the user data associated with the call log.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The user data associated with the call log. @maybenil
**/
LINPHONE_PUBLIC void *linphone_call_log_get_user_data(const LinphoneCallLog *call_log);

/**
 * Assign a user data to the call log.
 * @param call_log #LinphoneCallLog object @notnil
 * @param user_data The user data to associate with the call log. @maybenil
**/
LINPHONE_PUBLIC void linphone_call_log_set_user_data(LinphoneCallLog *call_log, void *user_data);

/**
 * Acquire a reference to the call log.
 * @param call_log #LinphoneCallLog object @notnil
 * @return The same #LinphoneCallLog object @notnil
**/
LINPHONE_PUBLIC LinphoneCallLog * linphone_call_log_ref(LinphoneCallLog *call_log);

/**
 * Release a reference to the call log.
 * @param call_log #LinphoneCallLog object @notnil
**/
LINPHONE_PUBLIC void linphone_call_log_unref(LinphoneCallLog *call_log);

/**
 * Creates a fake #LinphoneCallLog.
 * @param core #LinphoneCore object @notnil
 * @param from #LinphoneAddress of caller @notnil
 * @param to #LinphoneAddress of callee @notnil
 * @param dir #LinphoneCallDir of call
 * @param duration call length in seconds
 * @param start_time timestamp of call start time
 * @param connected_time timestamp of call connection
 * @param status #LinphoneCallStatus of call
 * @param video_enabled whether video was enabled or not for this call
 * @param quality call quality
 * @return a #LinphoneCallLog object @notnil
**/
LINPHONE_PUBLIC LinphoneCallLog *linphone_core_create_call_log(LinphoneCore *core, LinphoneAddress *from, LinphoneAddress *to, LinphoneCallDir dir, 
		int duration, time_t start_time, time_t connected_time, LinphoneCallStatus status, bool_t video_enabled, float quality);

/**
 * @}
**/

#ifdef __cplusplus
}
#endif

#endif /* __LINPHONE_CALL_LOG_H__ */
