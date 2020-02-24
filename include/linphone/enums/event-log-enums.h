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

#ifndef _L_EVENT_LOG_ENUMS_H_
#define _L_EVENT_LOG_ENUMS_H_

// =============================================================================

// -----------------------------------------------------------------------------
// EventLog.
// -----------------------------------------------------------------------------

/**
 * #LinphoneEventLogType is used to indicate the type of an event. Useful for cast.
 * @ingroup events
 */
typedef enum _EventLogType{
	LinphoneEventLogTypeNone, /**< No defined event */
	LinphoneEventLogTypeConferenceCreated, /**< Conference (created) event */
	LinphoneEventLogTypeConferenceTerminated, /**< Conference (terminated) event */
	LinphoneEventLogTypeConferenceCallStart, /**< Conference call (start) event */
	LinphoneEventLogTypeConferenceCallEnd, /**< Conference call (end) event */
	LinphoneEventLogTypeConferenceChatMessage, /**< Conference chat message event */
	LinphoneEventLogTypeConferenceParticipantAdded, /**< Conference participant (added) event */
	LinphoneEventLogTypeConferenceParticipantRemoved, /**< Conference participant (removed) event */
	LinphoneEventLogTypeConferenceParticipantSetAdmin, /**< Conference participant (set admin) event */
	LinphoneEventLogTypeConferenceParticipantUnsetAdmin, /**< Conference participant (unset admin) event */
	LinphoneEventLogTypeConferenceParticipantDeviceAdded, /**< Conference participant device (added) event */
	LinphoneEventLogTypeConferenceParticipantDeviceRemoved, /**< Conference participant device (removed) event */
	LinphoneEventLogTypeConferenceSubjectChanged, /**< Conference subject event */

	LinphoneEventLogTypeConferenceSecurityEvent, /**< Conference encryption security event*/
	LinphoneEventLogTypeConferenceEphemeralMessageLifetimeChanged, /**< Conference ephemeral message (ephemeral message lifetime changed) event */
	LinphoneEventLogTypeConferenceEphemeralMessageEnabled, /**< Conference ephemeral message (ephemeral message enabled) event */
	LinphoneEventLogTypeConferenceEphemeralMessageDisabled, /**< Conference ephemeral message (ephemeral message disabled) event */
} LinphoneEventLogType;

#endif // ifndef _L_EVENT_LOG_ENUMS_H_
