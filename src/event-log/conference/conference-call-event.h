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

#ifndef _L_CONFERENCE_CALL_EVENT_H_
#define _L_CONFERENCE_CALL_EVENT_H_

#include "event-log/event-log.h"

#include "call/call-log.h"
#include "conference/conference-info.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceCallEventPrivate;

class LINPHONE_PUBLIC ConferenceCallEvent : public EventLog {
public:
	ConferenceCallEvent(Type type,
	                    time_t creationTime,
	                    const std::shared_ptr<CallLog> &callLog,
	                    const std::shared_ptr<ConferenceInfo> &conferenceInfo = nullptr);

	std::shared_ptr<CallLog> getCallLog() const;
	std::shared_ptr<ConferenceInfo> getConferenceInfo() const;

private:
	L_DECLARE_PRIVATE(ConferenceCallEvent);
	L_DISABLE_COPY(ConferenceCallEvent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_CALL_EVENT_H_
