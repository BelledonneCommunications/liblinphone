/*
 * conference-security-event.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "conference-event-p.h"
#include "conference-security-event.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceSecurityEventPrivate : public ConferenceEventPrivate {
public:
	ConferenceSecurityEvent::SecurityAlertType securityAlertType;
	IdentityAddress faultyDevice;
};

// -----------------------------------------------------------------------------

ConferenceSecurityEvent::ConferenceSecurityEvent (
	time_t creationTime,
	const ConferenceId &conferenceId,
	SecurityAlertType securityAlertType,
	IdentityAddress faultyDevice
) : ConferenceEvent(
	*new ConferenceSecurityEventPrivate,
	Type::ConferenceSecurityAlert,
	creationTime,
	conferenceId
) {
	L_D();
	d->securityAlertType = securityAlertType;
	d->faultyDevice = faultyDevice;
}

ConferenceSecurityEvent::SecurityAlertType ConferenceSecurityEvent::getSecurityAlertType () const {
	L_D();
	return d->securityAlertType;
}

const IdentityAddress ConferenceSecurityEvent::getFaultyDevice () const {
	L_D();
	return d->faultyDevice;
}

LINPHONE_END_NAMESPACE
