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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ADVANCED_IM
#include "handlers/local-conference-event-handler.h"
#endif
#include "local-conference.h"
#include "logger/logger.h"
#include "participant.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

LocalConference::LocalConference (const shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener)
	: Conference(core, myAddress, listener) {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset(new LocalConferenceEventHandler(this));
#endif
}

LocalConference::~LocalConference () {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif
}

// -----------------------------------------------------------------------------

bool LocalConference::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	shared_ptr<Participant> participant = findParticipant(addr);
	if (participant) {
		lInfo() << "Not adding participant '" << addr.asString() << "' because it is already a participant of the LocalConference";
		return false;
	}
	participant = Participant::create(this,addr);
	participant->createSession(*this, params, hasMedia, listener);
	participants.push_back(participant);
	if (!activeParticipant)
		activeParticipant = participant;
	return true;
}

bool LocalConference::removeParticipant (const shared_ptr<Participant> &participant) {
	for (const auto &p : participants) {
		if (participant->getAddress() == p->getAddress()) {
			participants.remove(p);
			return true;
		}
	}
	return false;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded (const Address &addr) {
	shared_ptr<Participant> participant = findParticipant(addr);
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	eventHandler->notifyAllExcept(eventHandler->createNotifyParticipantAdded(addr), participant);
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantAdded,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	return event;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved (const Address &addr) {
	shared_ptr<Participant> participant = findParticipant(addr);
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	eventHandler->notifyAllExcept(eventHandler->createNotifyParticipantRemoved(addr), participant);
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantRemoved,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	return event;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin (const Address &addr, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	eventHandler->notifyAll(eventHandler->createNotifyParticipantAdminStatusChanged(addr, isAdmin));
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	return event;
}

shared_ptr<ConferenceSubjectEvent> LocalConference::notifySubjectChanged () {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	eventHandler->notifyAll(eventHandler->createNotifySubjectChanged());
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(
		time(nullptr),
		conferenceId,
		lastNotify,
		getSubject()
	);
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceAdded (const Address &addr, const Address &gruu) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	eventHandler->notifyAll(eventHandler->createNotifyParticipantDeviceAdded(addr, gruu));
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceAdded,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr,
		gruu
	);
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceRemoved (const Address &addr, const Address &gruu) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	eventHandler->notifyAll(eventHandler->createNotifyParticipantDeviceRemoved(addr, gruu));
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceRemoved,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr,
		gruu
	);
	return event;
}

void LocalConference::setLastNotify (unsigned int lastNotify) {
	this->lastNotify = lastNotify;
}

void LocalConference::setConferenceId (const ConferenceId &conferenceId) {
	this->conferenceId = conferenceId;
}

const ConferenceId &LocalConference::getConferenceId () const {
	return conferenceId;
}


LINPHONE_END_NAMESPACE
