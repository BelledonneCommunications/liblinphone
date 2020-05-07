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
#include "handlers/remote-conference-event-handler.h"
#endif
#include "logger/logger.h"
#include "participant.h"
#include "remote-conference.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener
) : Conference(core, myAddress, listener) {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset(new RemoteConferenceEventHandler(this));
#endif
}

RemoteConference::~RemoteConference () {
	eventHandler.reset();
}

// -----------------------------------------------------------------------------

bool RemoteConference::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	shared_ptr<Participant> participant = findParticipant(addr);
	if (participant) {
		lInfo() << "Not adding participant '" << addr.asString() << "' because it is already a participant of the RemoteConference";
		return false;
	}
	participant = Participant::create(this,addr);
	participant->createSession(*this, params, hasMedia, listener);
	participants.push_back(participant);
	if (!activeParticipant)
		activeParticipant = participant;
	return true;
}

bool RemoteConference::removeParticipant (const shared_ptr<Participant> &participant) {
	for (const auto &p : participants) {
		if (participant->getAddress() == p->getAddress()) {
			participants.remove(p);
			return true;
		}
	}
	return false;
}

// -----------------------------------------------------------------------------

void RemoteConference::onConferenceCreated (const IdentityAddress &) {}

void RemoteConference::onConferenceTerminated (const IdentityAddress &) {
#ifdef HAVE_ADVANCED_IM
	eventHandler->unsubscribe();
#endif
}

void RemoteConference::onFirstNotifyReceived (const IdentityAddress &) {}

void RemoteConference::onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &, bool) {}

void RemoteConference::onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &, bool) {}

void RemoteConference::onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &, bool) {}

void RemoteConference::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &, bool) {}

void RemoteConference::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &, bool) {}

void RemoteConference::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &, bool) {}

LINPHONE_END_NAMESPACE
