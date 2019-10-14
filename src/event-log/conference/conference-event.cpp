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

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

ConferenceEvent::ConferenceEvent (Type type, time_t creationTime, const ConferenceId &conferenceId) :
	EventLog(*new ConferenceEventPrivate, type, creationTime) {
	L_D();
	L_ASSERT(type == Type::ConferenceCreated || type == Type::ConferenceTerminated);
	d->conferenceId = conferenceId;
}

ConferenceEvent::ConferenceEvent (
	ConferenceEventPrivate &p,
	Type type,
	time_t creationTime,
	const ConferenceId &conferenceId
) : EventLog(p, type, creationTime) {
	L_D();
	d->conferenceId = conferenceId;
}

const ConferenceId &ConferenceEvent::getConferenceId () const {
	L_D();
	return d->conferenceId;
}

LINPHONE_END_NAMESPACE
