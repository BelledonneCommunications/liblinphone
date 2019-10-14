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
	L_DECLARE_ENUM(SecurityEventType, L_ENUM_VALUES_SECURITY_EVENT_TYPE);

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

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_SECURITY_EVENT_H_
