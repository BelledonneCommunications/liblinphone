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

#include "conference-event-p.h"
#include "conference-security-event.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceSecurityEventPrivate : public ConferenceEventPrivate {
public:
	ConferenceSecurityEvent::SecurityEventType securityEventType;
	IdentityAddress faultyDevice;
};

// -----------------------------------------------------------------------------

ConferenceSecurityEvent::ConferenceSecurityEvent (
	time_t creationTime,
	const ConferenceId &conferenceId,
	SecurityEventType securityEventType,
	const IdentityAddress &faultyDevice
) : ConferenceEvent(
	*new ConferenceSecurityEventPrivate,
	Type::ConferenceSecurityEvent,
	creationTime,
	conferenceId
) {
	L_D();
	d->securityEventType = securityEventType;
	d->faultyDevice = faultyDevice;
}

ConferenceSecurityEvent::ConferenceSecurityEvent (
	time_t creationTime,
	const ConferenceId &conferenceId,
	SecurityEventType securityEventType
) : ConferenceEvent(
	*new ConferenceSecurityEventPrivate,
	Type::ConferenceSecurityEvent,
	creationTime,
	conferenceId
) {
	L_D();
	d->securityEventType = securityEventType;
}

ConferenceSecurityEvent::SecurityEventType ConferenceSecurityEvent::getSecurityEventType () const {
	L_D();
	return d->securityEventType;
}

const IdentityAddress &ConferenceSecurityEvent::getFaultyDeviceAddress () const {
	L_D();
	return d->faultyDevice;
}

std::ostream& operator<<(std::ostream& lhs, ConferenceSecurityEvent::SecurityEventType e) {
	switch(e) {
		case ConferenceSecurityEvent::SecurityEventType::None:
			lhs << "None"; break;
		case ConferenceSecurityEvent::SecurityEventType::SecurityLevelDowngraded:
			lhs << "SecurityLevelDowngraded"; break;
		case ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded:
			lhs << "ParticipantMaxDeviceCountExceeded"; break;
		case ConferenceSecurityEvent::SecurityEventType::EncryptionIdentityKeyChanged:
			lhs << "EncryptionIdentityKeyChanged"; break;
		case ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected:
			lhs << "ManInTheMiddleDetected"; break;
	}
	return lhs;
}

LINPHONE_END_NAMESPACE
