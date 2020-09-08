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

#ifndef _L_C_EVENT_LOG_H_
#define _L_C_EVENT_LOG_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup events
 * @{
 */

// -----------------------------------------------------------------------------
// EventLog.
// -----------------------------------------------------------------------------

/**
 * Increment reference count of #LinphoneEventLog object.
 * @param event_log A #LinphoneEventLog object @notnil
 * @return the same #LinphoneEventLog object @notnil
 **/
LINPHONE_PUBLIC LinphoneEventLog *linphone_event_log_ref (LinphoneEventLog *event_log);

/**
 * Decrement reference count of #LinphoneEventLog object. When dropped to zero, memory is freed.
 * @param event_log A #LinphoneEventLog object @notnil
 **/
LINPHONE_PUBLIC void linphone_event_log_unref (LinphoneEventLog *event_log);

/**
 * Returns the type of a event log.
 * @param event_log A #LinphoneEventLog object @notnil
 * @return The #LinphoneEventLogType type
 */
LINPHONE_PUBLIC LinphoneEventLogType linphone_event_log_get_type (const LinphoneEventLog *event_log);

/**
 * Returns the creation time of a event log.
 * @param event_log A #LinphoneEventLog object @notnil
 * @return The event creation time
 */
LINPHONE_PUBLIC time_t linphone_event_log_get_creation_time (const LinphoneEventLog *event_log);

/**
 * Delete event log from database.
 * @param event_log A #LinphoneEventLog object @notnil
 */
LINPHONE_PUBLIC void linphone_event_log_delete_from_database (LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the peer address of a conference event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The peer #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_log_get_peer_address (const LinphoneEventLog *event_log);

/**
 * Returns the local address of a conference event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The local #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_log_get_local_address (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceNotifiedEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the notify id of a conference notified event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The conference notify id.
 */
LINPHONE_PUBLIC unsigned int linphone_event_log_get_notify_id (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceCallEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the call of a conference call event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The conference #LinphoneCall. @maybenil
 */
LINPHONE_PUBLIC LinphoneCall *linphone_event_log_get_call (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceChatMessageEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the chat message of a conference chat message event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The conference #LinphoneChatMessage. @maybenil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_event_log_get_chat_message (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceParticipantEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the participant address of a conference participant event.
 * @param event_log A ConferenceParticipantEvent object. @notnil
 * @return The conference participant #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_log_get_participant_address (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceParticipantDeviceEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the device address of a conference participant device event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The conference device #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_log_get_device_address (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceSecurityEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the type of security event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The #LinphoneSecurityEventType type.
 */
LINPHONE_PUBLIC LinphoneSecurityEventType linphone_event_log_get_security_event_type (const LinphoneEventLog *event_log);

/**
 * Returns the faulty device address of a conference security event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The #LinphoneAddress of the faulty device. @maybenil
 */
LINPHONE_PUBLIC LinphoneAddress *linphone_event_log_get_security_event_faulty_device_address (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceSubjectEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the subject of a conference subject event.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_event_log_get_subject (const LinphoneEventLog *event_log);

// -----------------------------------------------------------------------------
// ConferenceEphemeralMessageEvent.
// -----------------------------------------------------------------------------

/**
 * Returns the ephemeral message lifetime of a conference ephemeral message event.
 * Ephemeral lifetime means the time before an ephemeral message which has been viewed gets deleted.
 * @param event_log A #LinphoneEventLog object. @notnil
 * @return The ephemeral message lifetime.
 */
LINPHONE_PUBLIC long linphone_event_log_get_ephemeral_message_lifetime (const LinphoneEventLog *event_log);
/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_EVENT_LOG_H_
