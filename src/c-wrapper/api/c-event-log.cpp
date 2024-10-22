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

#include "linphone/api/c-event-log.h"

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-message/chat-message.h"
#include "conference/conference-id.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "event-log/events.h"

// =============================================================================

using namespace std;

L_DECLARE_C_OBJECT_IMPL(EventLog);

// -----------------------------------------------------------------------------
// Helpers.
// -----------------------------------------------------------------------------

static bool isConferenceType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceCallEnded:
		case LinphoneEventLogTypeConferenceCallStarted:
		case LinphoneEventLogTypeConferenceChatMessage:
		case LinphoneEventLogTypeConferenceCreated:
		case LinphoneEventLogTypeConferenceTerminated:
		case LinphoneEventLogTypeConferenceAllowedParticipantListChanged:
		case LinphoneEventLogTypeConferenceParticipantAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceRemoved:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaCapabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaAvailabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceStatusChanged:
		case LinphoneEventLogTypeConferenceParticipantRemoved:
		case LinphoneEventLogTypeConferenceParticipantSetAdmin:
		case LinphoneEventLogTypeConferenceParticipantUnsetAdmin:
		case LinphoneEventLogTypeConferenceSubjectChanged:
		case LinphoneEventLogTypeConferenceEphemeralMessageLifetimeChanged:
		case LinphoneEventLogTypeConferenceEphemeralMessageManagedByAdmin:
		case LinphoneEventLogTypeConferenceEphemeralMessageManagedByParticipants:
		case LinphoneEventLogTypeConferenceEphemeralMessageEnabled:
		case LinphoneEventLogTypeConferenceEphemeralMessageDisabled:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceCallType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceCallEnded:
		case LinphoneEventLogTypeConferenceCallStarted:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceChatMessageType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceChatMessage:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceNotifiedType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceAllowedParticipantListChanged:
		case LinphoneEventLogTypeConferenceParticipantAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceRemoved:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaCapabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaAvailabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceStatusChanged:
		case LinphoneEventLogTypeConferenceParticipantRemoved:
		case LinphoneEventLogTypeConferenceParticipantSetAdmin:
		case LinphoneEventLogTypeConferenceParticipantUnsetAdmin:
		case LinphoneEventLogTypeConferenceSubjectChanged:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceParticipantType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceParticipantAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceRemoved:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaCapabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaAvailabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceStatusChanged:
		case LinphoneEventLogTypeConferenceParticipantRemoved:
		case LinphoneEventLogTypeConferenceParticipantSetAdmin:
		case LinphoneEventLogTypeConferenceParticipantUnsetAdmin:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceParticipantDeviceType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceParticipantDeviceAdded:
		case LinphoneEventLogTypeConferenceParticipantDeviceRemoved:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaCapabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceMediaAvailabilityChanged:
		case LinphoneEventLogTypeConferenceParticipantDeviceStatusChanged:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceSubjectType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceSubjectChanged:
			return true;

		default:
			break;
	}

	return false;
}

static bool isConferenceSecurityType(LinphoneEventLogType type) {
	return (type == LinphoneEventLogTypeConferenceSecurityEvent);
}

static bool isConferenceEphemeralMessageType(LinphoneEventLogType type) {
	switch (type) {
		case LinphoneEventLogTypeConferenceEphemeralMessageLifetimeChanged:
		case LinphoneEventLogTypeConferenceEphemeralMessageEnabled:
		case LinphoneEventLogTypeConferenceEphemeralMessageDisabled:
			return true;

		default:
			break;
	}

	return false;
}

// -----------------------------------------------------------------------------
// EventLog.
// -----------------------------------------------------------------------------

LinphoneEventLog *linphone_event_log_ref(LinphoneEventLog *event_log) {
	belle_sip_object_ref(event_log);
	return event_log;
}

void linphone_event_log_unref(LinphoneEventLog *event_log) {
	belle_sip_object_unref(event_log);
}

LinphoneEventLogType linphone_event_log_get_type(const LinphoneEventLog *event_log) {
	return static_cast<LinphoneEventLogType>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log)->getType());
}

time_t linphone_event_log_get_creation_time(const LinphoneEventLog *event_log) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(event_log)->getCreationTime();
}

void linphone_event_log_delete_from_database(LinphoneEventLog *event_log) {
	LinphonePrivate::EventLog::deleteFromDatabase(L_GET_CPP_PTR_FROM_C_OBJECT(event_log));
}

// -----------------------------------------------------------------------------
// ConferenceEvent.
// -----------------------------------------------------------------------------

const LinphoneAddress *linphone_event_log_get_peer_address(const LinphoneEventLog *event_log) {
	if (!isConferenceType(linphone_event_log_get_type(event_log))) return nullptr;

	const std::shared_ptr<LinphonePrivate::Address> &addr =
	    static_pointer_cast<const LinphonePrivate::ConferenceEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getConferenceId()
	        .getPeerAddress();

	return addr->toC();
}

const LinphoneAddress *linphone_event_log_get_local_address(const LinphoneEventLog *event_log) {
	if (!isConferenceType(linphone_event_log_get_type(event_log))) return nullptr;

	const std::shared_ptr<LinphonePrivate::Address> &addr =
	    static_pointer_cast<const LinphonePrivate::ConferenceEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getConferenceId()
	        .getLocalAddress();

	return addr->toC();
}

// -----------------------------------------------------------------------------
// ConferenceNotifiedEvent.
// -----------------------------------------------------------------------------

unsigned int linphone_event_log_get_notify_id(const LinphoneEventLog *event_log) {
	if (!isConferenceNotifiedType(linphone_event_log_get_type(event_log))) return 0;

	return static_pointer_cast<const LinphonePrivate::ConferenceNotifiedEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	    ->getNotifyId();
}

// -----------------------------------------------------------------------------
// ConferenceCallEvent.
// -----------------------------------------------------------------------------

LinphoneCallLog *linphone_event_log_get_call_log(const LinphoneEventLog *event_log) {
	if (!isConferenceCallType(linphone_event_log_get_type(event_log))) return nullptr;

	std::shared_ptr<LinphonePrivate::CallLog> callLog =
	    static_pointer_cast<const LinphonePrivate::ConferenceCallEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getCallLog();
	if (callLog) {
		return callLog->toC();
	}
	return NULL;
}

LinphoneConferenceInfo *linphone_event_log_get_conference_info(const LinphoneEventLog *event_log) {
	if (!isConferenceCallType(linphone_event_log_get_type(event_log))) return nullptr;

	std::shared_ptr<LinphonePrivate::ConferenceInfo> confInfo =
	    static_pointer_cast<const LinphonePrivate::ConferenceCallEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getConferenceInfo();
	if (confInfo) {
		return confInfo->toC();
	}
	return NULL;
}

// -----------------------------------------------------------------------------
// ConferenceChatMessageEvent.
// -----------------------------------------------------------------------------

LinphoneChatMessage *linphone_event_log_get_chat_message(const LinphoneEventLog *event_log) {
	if (!isConferenceChatMessageType(linphone_event_log_get_type(event_log))) return nullptr;

	return L_GET_C_BACK_PTR(
	    static_pointer_cast<const LinphonePrivate::ConferenceChatMessageEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getChatMessage());
}

// -----------------------------------------------------------------------------
// ConferenceParticipantEvent.
// -----------------------------------------------------------------------------

const LinphoneAddress *linphone_event_log_get_participant_address(const LinphoneEventLog *event_log) {
	if (!isConferenceParticipantType(linphone_event_log_get_type(event_log))) return nullptr;

	const std::shared_ptr<LinphonePrivate::Address> &addr =
	    static_pointer_cast<const LinphonePrivate::ConferenceParticipantEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getParticipantAddress();

	return addr->toC();
}

// -----------------------------------------------------------------------------
// ConferenceParticipantDeviceEvent.
// -----------------------------------------------------------------------------

const LinphoneAddress *linphone_event_log_get_device_address(const LinphoneEventLog *event_log) {
	if (!isConferenceParticipantDeviceType(linphone_event_log_get_type(event_log))) return nullptr;

	const std::shared_ptr<LinphonePrivate::Address> &addr =
	    static_pointer_cast<const LinphonePrivate::ConferenceParticipantDeviceEvent>(
	        L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getDeviceAddress();

	return addr->toC();
}

// -----------------------------------------------------------------------------
// ConferenceSecurityEvent.
// -----------------------------------------------------------------------------

LINPHONE_PUBLIC LinphoneSecurityEventType
linphone_event_log_get_security_event_type(const LinphoneEventLog *event_log) {
	if (!isConferenceSecurityType(linphone_event_log_get_type(event_log))) return LinphoneSecurityEventTypeNone;

	const auto securityEvent =
	    static_pointer_cast<const LinphonePrivate::ConferenceSecurityEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log));
	return static_cast<LinphoneSecurityEventType>(securityEvent->getSecurityEventType());
}

LINPHONE_PUBLIC const LinphoneAddress *
linphone_event_log_get_security_event_faulty_device_address(const LinphoneEventLog *event_log) {
	if (!isConferenceSecurityType(linphone_event_log_get_type(event_log))) return nullptr;

	return static_pointer_cast<const LinphonePrivate::ConferenceSecurityEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	    ->getFaultyDeviceAddress()
	    ->toC();
}

// -----------------------------------------------------------------------------
// ConferenceSubjectEvent.
// -----------------------------------------------------------------------------

LINPHONE_PUBLIC const char *linphone_event_log_get_subject(const LinphoneEventLog *event_log) {
	if (!isConferenceSubjectType(linphone_event_log_get_type(event_log))) return nullptr;

	return L_STRING_TO_C(
	    static_pointer_cast<const LinphonePrivate::ConferenceSubjectEvent>(L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	        ->getSubject());
}

// -----------------------------------------------------------------------------
// ConferenceEphemeralMessageEvent.
// -----------------------------------------------------------------------------

LINPHONE_PUBLIC long linphone_event_log_get_ephemeral_message_lifetime(const LinphoneEventLog *event_log) {
	if (!isConferenceEphemeralMessageType(linphone_event_log_get_type(event_log))) return -1;

	return static_pointer_cast<const LinphonePrivate::ConferenceEphemeralMessageEvent>(
	           L_GET_CPP_PTR_FROM_C_OBJECT(event_log))
	    ->getEphemeralMessageLifetime();
}
