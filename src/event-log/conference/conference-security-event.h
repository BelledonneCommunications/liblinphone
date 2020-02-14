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

#ifndef _L_CONFERENCE_SECURITY_EVENT_H_
#define _L_CONFERENCE_SECURITY_EVENT_H_

#include <string>

#include "conference-event.h"
#include "linphone/enums/security-event-enums.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceSecurityEventPrivate;

class LINPHONE_PUBLIC ConferenceSecurityEvent : public ConferenceEvent {

public:

/*TODO rename it "Type" as the name is already clear enough with namespaces*/

	enum class SecurityEventType{
		None = LinphoneSecurityEventTypeNone,
		SecurityLevelDowngraded = LinphoneSecurityEventTypeSecurityLevelDowngraded,
		ParticipantMaxDeviceCountExceeded = LinphoneSecurityEventTypeParticipantMaxDeviceCountExceeded,
		EncryptionIdentityKeyChanged = LinphoneSecurityEventTypeEncryptionIdentityKeyChanged,
		ManInTheMiddleDetected = LinphoneSecurityEventTypeManInTheMiddleDetected
	};

	//casting to int to get rid of the enum compare warning.
	//Here we are comparing two enums serving the same purpose
	static_assert((int)ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected == (int)LinphoneSecurityEventTypeManInTheMiddleDetected, "LinphoneSecurityEventType and ConferenceSecurityEvent::SecurityEventType are not synchronized, fix this !");

	ConferenceSecurityEvent (
		time_t creationTime,
		const ConferenceId &conferenceId,
		SecurityEventType securityEventType,
		const IdentityAddress &faultyDevice
	);

	ConferenceSecurityEvent (
		time_t creationTime,
		const ConferenceId &conferenceId,
		SecurityEventType securityEventType
	);

	SecurityEventType getSecurityEventType () const;
	const IdentityAddress &getFaultyDeviceAddress () const;

private:
	L_DECLARE_PRIVATE(ConferenceSecurityEvent);
	L_DISABLE_COPY(ConferenceSecurityEvent);
};

std::ostream& operator<<(std::ostream& lhs, ConferenceSecurityEvent::SecurityEventType e);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_SECURITY_EVENT_H_
