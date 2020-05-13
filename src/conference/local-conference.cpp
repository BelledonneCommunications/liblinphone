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

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/event.h"

#ifdef HAVE_ADVANCED_IM
#include "handlers/local-conference-event-handler.h"
#endif
#include "local-conference.h"
#include "logger/logger.h"
#include "participant.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

LocalConference::LocalConference (const shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener)
	: Conference(core, myAddress, listener) {
#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<LocalConferenceEventHandler>(this);
	addListener(eventHandler);
#endif
}

LocalConference::~LocalConference () {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif
	confListeners.clear();
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

/*
void LocalConference::notifyFullState (const string &notify, const shared_ptr<ParticipantDevice> &device) {
	++lastNotify;
	for (const auto &l : confListeners) {
		l->onFullStateReceived();
	}
}
*/

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded (const Address &addr) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantAdded,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	for (const auto &l : confListeners) {
		l->onParticipantAdded(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved (const Address &addr) {
	shared_ptr<Participant> participant = findParticipant(addr);
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantRemoved,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	for (const auto &l : confListeners) {
		l->onParticipantRemoved(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin (const Address &addr, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr
	);
	for (const auto &l : confListeners) {
		l->onParticipantSetAdmin(event);
	}
	return event;
}

shared_ptr<ConferenceSubjectEvent> LocalConference::notifySubjectChanged () {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(
		time(nullptr),
		conferenceId,
		lastNotify,
		getSubject()
	);
	for (const auto &l : confListeners) {
		l->onSubjectChanged(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceAdded (const Address &addr, const Address &gruu) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceAdded,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr,
		gruu
	);
	for (const auto &l : confListeners) {
		l->onParticipantDeviceAdded(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceRemoved (const Address &addr, const Address &gruu) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceRemoved,
		time(nullptr),
		conferenceId,
		lastNotify,
		addr,
		gruu
	);
	for (const auto &l : confListeners) {
		l->onParticipantDeviceRemoved(event);
	}
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

void LocalConference::subscribeReceived (LinphoneEvent *event) {
#ifdef HAVE_ADVANCED_IM
	shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(linphone_event_get_core(event))->findChatRoom(conferenceId);
	eventHandler->subscribeReceived(event, !!(chatRoom->getCapabilities() & AbstractChatRoom::Capabilities::OneToOne));
#endif
}

LINPHONE_END_NAMESPACE
