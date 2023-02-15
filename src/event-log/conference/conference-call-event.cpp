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

#include "conference-call-event.h"
#include "event-log/event-log-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceCallEventPrivate : public EventLogPrivate {
public:
	shared_ptr<CallLog> callLog;
	shared_ptr<ConferenceInfo> conferenceInfo;
};

// -----------------------------------------------------------------------------

ConferenceCallEvent::ConferenceCallEvent(Type type,
                                         time_t creationTime,
                                         const shared_ptr<CallLog> &callLog,
                                         const std::shared_ptr<ConferenceInfo> &conferenceInfo)
    : EventLog(*new ConferenceCallEventPrivate, type, creationTime) {
	L_D();
	L_ASSERT(callLog);
	L_ASSERT(type == Type::ConferenceCallStarted || type == Type::ConferenceCallConnected ||
	         type == Type::ConferenceCallEnded);
	d->callLog = callLog;
	d->conferenceInfo = conferenceInfo;
}

shared_ptr<CallLog> ConferenceCallEvent::getCallLog() const {
	L_D();
	return d->callLog;
}

std::shared_ptr<ConferenceInfo> ConferenceCallEvent::getConferenceInfo() const {
	L_D();
	return d->conferenceInfo;
}

LINPHONE_END_NAMESPACE
