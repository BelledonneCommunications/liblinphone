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
#include "local-conference-p.h"
#include "logger/logger.h"
#include "participant-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

LocalConference::LocalConference (const shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener)
	: Conference(*new LocalConferencePrivate, core, myAddress, listener) {
#ifdef HAVE_ADVANCED_IM
	L_D();
	d->eventHandler.reset(new LocalConferenceEventHandler(this));
#endif
}

LocalConference::~LocalConference () {
#ifdef HAVE_ADVANCED_IM
	L_D();
	d->eventHandler.reset();
#endif
}

// -----------------------------------------------------------------------------

bool LocalConference::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	L_D();
	shared_ptr<Participant> participant = findParticipant(addr);
	if (participant) {
		lInfo() << "Not adding participant '" << addr.asString() << "' because it is already a participant of the LocalConference";
		return false;
	}
	participant = make_shared<Participant>(this, addr);
	participant->getPrivate()->createSession(*this, params, hasMedia, d->listener);
	d->participants.push_back(participant);
	if (!d->activeParticipant)
		d->activeParticipant = participant;
	return true;
}

bool LocalConference::removeParticipant (const shared_ptr<Participant> &participant) {
	L_D();
	for (const auto &p : d->participants) {
		if (participant->getAddress() == p->getAddress()) {
			d->participants.remove(p);
			return true;
		}
	}
	return false;
}

LINPHONE_END_NAMESPACE
