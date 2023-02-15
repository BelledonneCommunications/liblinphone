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

#include "conference-ephemeral-message-event.h"
#include "conference-notified-event-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceEphemeralMessageEventPrivate : public ConferenceNotifiedEventPrivate {
public:
	long ephemeralLifetime;
};

// -----------------------------------------------------------------------------

ConferenceEphemeralMessageEvent::ConferenceEphemeralMessageEvent(Type type,
                                                                 time_t creationTime,
                                                                 const ConferenceId &conferenceId,
                                                                 long ephemeralLifetime)
    : ConferenceNotifiedEvent(*new ConferenceEphemeralMessageEventPrivate, type, creationTime, conferenceId) {
	L_D();
	L_ASSERT(type == Type::ConferenceEphemeralMessageLifetimeChanged ||
	         type == Type::ConferenceEphemeralMessageManagedByAdmin ||
	         type == Type::ConferenceEphemeralMessageManagedByParticipants ||
	         type == Type::ConferenceEphemeralMessageEnabled || type == Type::ConferenceEphemeralMessageDisabled);
	d->ephemeralLifetime = ephemeralLifetime;
}

long ConferenceEphemeralMessageEvent::getEphemeralMessageLifetime() const {
	L_D();
	return d->ephemeralLifetime;
}

LINPHONE_END_NAMESPACE
