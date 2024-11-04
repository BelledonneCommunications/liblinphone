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

#include "core/core-p.h"
#include "db/main-db-key-p.h"
#include "db/main-db-p.h"
#include "db/main-db.h"
#include "event-log-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void EventLogPrivate::resetStorageId() {
	dbKey.resetStorageId();
}

EventLog::EventLog() : BaseObject(*new EventLogPrivate) {
}

EventLog::EventLog(EventLogPrivate &p, Type type, time_t creationTime) : BaseObject(p) {
	L_D();
	d->type = type;
	d->creationTime = creationTime;
}

EventLog::~EventLog() {
	L_D();
	d->resetStorageId();
}

EventLog::Type EventLog::getType() const {
	L_D();
	return d->type;
}

time_t EventLog::getCreationTime() const {
	L_D();
	return d->creationTime;
}

void EventLog::deleteFromDatabase(const shared_ptr<const EventLog> &eventLog) {
	MainDb::deleteEvent(eventLog);
}

std::ostream &operator<<(std::ostream &lhs, EventLog::Type e) {
	switch (e) {
		case EventLog::Type::None:
			lhs << "None";
			break;
		case EventLog::Type::ConferenceCreated:
			lhs << "ConferenceCreated";
			break;
		case EventLog::Type::ConferenceTerminated:
			lhs << "ConferenceTerminated";
			break;
		case EventLog::Type::ConferenceCallStarted:
			lhs << "ConferenceCallStarted";
			break;
		case EventLog::Type::ConferenceCallConnected:
			lhs << "ConferenceCallConnected";
			break;
		case EventLog::Type::ConferenceCallEnded:
			lhs << "ConferenceCallEnded";
			break;
		case EventLog::Type::ConferenceChatMessage:
			lhs << "ConferenceChatMessage";
			break;
		case EventLog::Type::ConferenceChatMessageReaction:
			lhs << "ConferenceChatMessageReaction";
			break;
		case EventLog::Type::ConferenceParticipantAdded:
			lhs << "ConferenceParticipantAdded";
			break;
		case EventLog::Type::ConferenceParticipantRemoved:
			lhs << "ConferenceParticipantRemoved";
			break;
		case EventLog::Type::ConferenceParticipantRoleUnknown:
			lhs << "ConferenceParticipantRoleUnknown";
			break;
		case EventLog::Type::ConferenceParticipantRoleSpeaker:
			lhs << "ConferenceParticipantRoleSpeaker";
			break;
		case EventLog::Type::ConferenceParticipantRoleListener:
			lhs << "ConferenceParticipantRoleListener";
			break;
		case EventLog::Type::ConferenceAllowedParticipantListChanged:
			lhs << "ConferenceAllowedParticipantListChanged";
			break;
		case EventLog::Type::ConferenceParticipantSetAdmin:
			lhs << "ConferenceParticipantSetAdmin";
			break;
		case EventLog::Type::ConferenceParticipantUnsetAdmin:
			lhs << "ConferenceParticipantUnsetAdmin";
			break;
		case EventLog::Type::ConferenceParticipantDeviceAdded:
			lhs << "ConferenceParticipantDeviceAdded";
			break;
		case EventLog::Type::ConferenceParticipantDeviceRemoved:
			lhs << "ConferenceParticipantDeviceRemoved";
			break;
		case EventLog::Type::ConferenceParticipantDeviceJoiningRequest:
			lhs << "ConferenceParticipantDeviceJoiningRequest";
			break;
		case EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged:
			lhs << "ConferenceParticipantDeviceMediaCapabilityChanged";
			break;
		case EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged:
			lhs << "ConferenceParticipantDeviceMediaAvailabilityChanged";
			break;
		case EventLog::Type::ConferenceParticipantDeviceStatusChanged:
			lhs << "ConferenceParticipantDeviceStatusChanged";
			break;
		case EventLog::Type::ConferenceAvailableMediaChanged:
			lhs << "ConferenceAvailableMediaChanged";
			break;
		case EventLog::Type::ConferenceSubjectChanged:
			lhs << "ConferenceSubjectChanged";
			break;
		case EventLog::Type::ConferenceSecurityEvent:
			lhs << "ConferenceSecurityEvent";
			break;
		case EventLog::Type::ConferenceEphemeralMessageLifetimeChanged:
			lhs << "ConferenceEphemeralMessageLifetimeChanged";
			break;
		case EventLog::Type::ConferenceEphemeralMessageEnabled:
			lhs << "ConferenceEphemeralMessageEnabled";
			break;
		case EventLog::Type::ConferenceEphemeralMessageDisabled:
			lhs << "ConferenceEphemeralMessageDisabled";
			break;
		case EventLog::Type::ConferenceEphemeralMessageManagedByAdmin:
			lhs << "ConferenceEphemeralMessageManagedByAdmin";
			break;
		case EventLog::Type::ConferenceEphemeralMessageManagedByParticipants:
			lhs << "ConferenceEphemeralMessageManagedByParticipants";
			break;
	}
	return lhs;
}

LINPHONE_END_NAMESPACE
