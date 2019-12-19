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

#ifndef _L_CONFERENCE_EPHEMERAL_MESSAGE_EVENT_H_
#define _L_CONFERENCE_EPHEMERAL_MESSAGE_EVENT_H_

#include "conference-event-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceEphemeralMessageEventPrivate;

class LINPHONE_PUBLIC ConferenceEphemeralMessageEvent : public ConferenceEvent {
public:
	ConferenceEphemeralMessageEvent (
							Type type,
							time_t creationTime,
							const ConferenceId &conferenceId,
							long ephemeralLifetime
							);
	
	long getEphemeralMessageLifetime() const;
	
private:
	L_DECLARE_PRIVATE(ConferenceEphemeralMessageEvent);
	L_DISABLE_COPY(ConferenceEphemeralMessageEvent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_EPHEMERAL_MESSAGE_EVENT_H_
