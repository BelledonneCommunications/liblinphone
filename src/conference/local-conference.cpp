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

LINPHONE_END_NAMESPACE
