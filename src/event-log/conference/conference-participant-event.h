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

#ifndef _L_CONFERENCE_PARTICIPANT_EVENT_H_
#define _L_CONFERENCE_PARTICIPANT_EVENT_H_

#include "conference-notified-event.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceParticipantEventPrivate;
class Address;
class Conference;
class Participant;

class LINPHONE_PUBLIC ConferenceParticipantEvent : public ConferenceNotifiedEvent {
	friend class ClientConferenceEventHandler;
	friend class Conference;

public:
	ConferenceParticipantEvent(Type type,
	                           time_t creationTime,
	                           const ConferenceId &conferenceId,
	                           const std::shared_ptr<Address> &participantAddress);

	const std::shared_ptr<Address> &getParticipantAddress() const;

protected:
	ConferenceParticipantEvent(ConferenceParticipantEventPrivate &p,
	                           Type type,
	                           time_t creationTime,
	                           const ConferenceId &conferenceId,
	                           const std::shared_ptr<Address> &participantAddress);

private:
	L_DECLARE_PRIVATE(ConferenceParticipantEvent);
	L_DISABLE_COPY(ConferenceParticipantEvent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_PARTICIPANT_EVENT_H_
