/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of linphone-iphone
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

#include "conference-ephemeral-message-event.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceEphemeralMessageEventPrivate : public ConferenceEventPrivate {
public:
	long ephemeralLifetime;
};

// -----------------------------------------------------------------------------

ConferenceEphemeralMessageEvent::ConferenceEphemeralMessageEvent (
Type type,
time_t creationTime,
const ConferenceId &conferenceId,
long ephemeralLifetime
) : ConferenceEvent(
	*new ConferenceEphemeralMessageEventPrivate,
	type,
	creationTime,
	conferenceId
) {
	L_D();
	L_ASSERT(
			 type == Type::ConferenceEphemeralMessageLifetimeChanged ||
			 type == Type::ConferenceEphemeralMessageEnabled ||
			 type == Type::ConferenceEphemeralMessageDisabled
			 );
	d->ephemeralLifetime = ephemeralLifetime;
}

long ConferenceEphemeralMessageEvent::getEphemeralMessageLifetime () const {
	L_D();
	return d->ephemeralLifetime;
}

LINPHONE_END_NAMESPACE

