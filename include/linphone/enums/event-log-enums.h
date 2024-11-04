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
typedef enum _EventLogType {
	LinphoneEventLogTypeNone = 0,                  /**< No defined event */
	LinphoneEventLogTypeConferenceCreated = 1,     /**< Conference (created) event */
	LinphoneEventLogTypeConferenceTerminated = 2,  /**< Conference (terminated) event */
	LinphoneEventLogTypeConferenceCallStarted = 3, /**< Conference call (start) event */
	LinphoneEventLogTypeConferenceConnected = 21,  /**< Conference call (connected) event */
	LinphoneEventLogTypeConferenceCallEnded = 4,   /**< Conference call (end) event */
	LinphoneEventLogTypeConferenceChatMessage = 5, /**< Conference chat message event */
	LinphoneEventLogTypeConferenceAllowedParticipantListChanged =
	    28,                                                      /**< Conference allowed participant changed event */
	LinphoneEventLogTypeConferenceParticipantAdded = 6,          /**< Conference participant (added) event */
	LinphoneEventLogTypeConferenceParticipantRemoved = 7,        /**< Conference participant (removed) event */
	LinphoneEventLogTypeConferenceParticipantRoleUnknown = 25,   /**< Conference participant (role unknown) event */
	LinphoneEventLogTypeConferenceParticipantRoleSpeaker = 26,   /**< Conference participant (role speaker) event */
	LinphoneEventLogTypeConferenceParticipantRoleListener = 27,  /**< Conference participant (role listener) event */
	LinphoneEventLogTypeConferenceParticipantSetAdmin = 8,       /**< Conference participant (set admin) event */
	LinphoneEventLogTypeConferenceParticipantUnsetAdmin = 9,     /**< Conference participant (unset admin) event */
	LinphoneEventLogTypeConferenceParticipantDeviceAdded = 10,   /**< Conference participant device (added) event */
	LinphoneEventLogTypeConferenceParticipantDeviceRemoved = 11, /**< Conference participant device (removed) event */
	LinphoneEventLogTypeConferenceParticipantDeviceJoiningRequest =
	    29, /**< Conference participant device (joining request) event */
	LinphoneEventLogTypeConferenceParticipantDeviceMediaCapabilityChanged =
	    17, /**< Conference participant device (media capability changed) event */
	LinphoneEventLogTypeConferenceParticipantDeviceMediaAvailabilityChanged =
	    23, /**< Conference participant device (media availability changed) event */
	LinphoneEventLogTypeConferenceParticipantDeviceStatusChanged =
	    22,                                                   /**< Conference participant device (left) event */
	LinphoneEventLogTypeConferenceSubjectChanged = 12,        /**< Conference subject event */
	LinphoneEventLogTypeConferenceAvailableMediaChanged = 18, /**< Conference available media event */
	LinphoneEventLogTypeConferenceSecurityEvent = 13,         /**< Conference encryption security event*/
	LinphoneEventLogTypeConferenceEphemeralMessageLifetimeChanged =
	    14, /**< Conference ephemeral message (ephemeral message lifetime changed) event */
	LinphoneEventLogTypeConferenceEphemeralMessageEnabled =
	    15, /**< Conference ephemeral message (ephemeral message enabled) event */
	LinphoneEventLogTypeConferenceEphemeralMessageDisabled =
	    16, /**< Conference ephemeral message (ephemeral message disabled) event */
	LinphoneEventLogTypeConferenceEphemeralMessageManagedByAdmin =
	    19, /**< Conference ephemeral message (ephemeral message settings managed by admin) event */
	LinphoneEventLogTypeConferenceEphemeralMessageManagedByParticipants =
	    20, /**< Conference ephemeral message (ephemeral message settings managed by participants) event */
	LinphoneEventLogTypeConferenceChatMessageReaction = 24, /**< Reaction event to a chat message */
} LinphoneEventLogType;

#endif // ifndef _L_EVENT_LOG_ENUMS_H_
