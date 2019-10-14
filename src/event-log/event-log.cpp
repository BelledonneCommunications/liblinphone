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

#include "db/main-db.h"
#include "event-log-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

EventLog::EventLog () : BaseObject(*new EventLogPrivate) {}

EventLog::EventLog (EventLogPrivate &p, Type type, time_t creationTime) : BaseObject(p) {
	L_D();
	d->type = type;
	d->creationTime = creationTime;
}

EventLog::Type EventLog::getType () const {
	L_D();
	return d->type;
}

time_t EventLog::getCreationTime () const {
	L_D();
	return d->creationTime;
}

void EventLog::deleteFromDatabase (const shared_ptr<const EventLog> &eventLog) {
	MainDb::deleteEvent(eventLog);
}

LINPHONE_END_NAMESPACE
