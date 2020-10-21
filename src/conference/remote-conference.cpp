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
	CallSessionListener *listener,
	const std::shared_ptr<ConferenceParams> params
) : Conference(core, myAddress, listener, params) {
	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
	lastNotify = 0;
	this->confParams->enableLocalParticipant(false);

	// FIXME: Not very nice to have an empty deleter
	addListener(std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this), [](ConferenceListenerInterface * p){}));
#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<RemoteConferenceEventHandler>(this, this);
#endif
}

RemoteConference::~RemoteConference () {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM
}

// -----------------------------------------------------------------------------

void RemoteConference::onConferenceCreated (const ConferenceAddress &) {}

void RemoteConference::onConferenceTerminated (const IdentityAddress &) {
#ifdef HAVE_ADVANCED_IM
	eventHandler->unsubscribe();
#endif
}

void RemoteConference::onFirstNotifyReceived (const IdentityAddress &) {}

void RemoteConference::onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &, const std::shared_ptr<Participant> &) {}

void RemoteConference::onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &, const std::shared_ptr<Participant> &) {}

void RemoteConference::onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &, const std::shared_ptr<Participant> &) {}

void RemoteConference::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &) {}

void RemoteConference::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &, const std::shared_ptr<ParticipantDevice> &) {}

void RemoteConference::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &, const std::shared_ptr<ParticipantDevice> &) {}

void RemoteConference::onFullStateReceived() {

	time_t creationTime = time(nullptr);

	// Subject event
	shared_ptr<ConferenceSubjectEvent> sEvent = make_shared<ConferenceSubjectEvent>(
		creationTime,
		conferenceId,
		getSubject()
	);
	sEvent->setFullState(true);
	sEvent->setNotifyId(lastNotify);
	for (const auto &l : confListeners) {
		l->onSubjectChanged(sEvent);
	}

	// Loop through the participants
	for (const auto &p : getParticipants()) {
		shared_ptr<ConferenceParticipantEvent> pEvent = make_shared<ConferenceParticipantEvent>(
			EventLog::Type::ConferenceParticipantAdded,
			creationTime,
			conferenceId,
			p->getAddress()
		);
		pEvent->setFullState(true);
		pEvent->setNotifyId(lastNotify);
		for (const auto &l : confListeners) {
			l->onParticipantAdded(pEvent, p);
		}

		shared_ptr<ConferenceParticipantEvent> aEvent = make_shared<ConferenceParticipantEvent>(
			p->isAdmin() ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
			creationTime,
			conferenceId,
			p->getAddress()
		);
		aEvent->setFullState(true);
		aEvent->setNotifyId(lastNotify);
		for (const auto &l : confListeners) {
			l->onParticipantSetAdmin(aEvent, p);
		}

		// Loop through the devices
		for (const auto &d : p->getDevices()) {
			shared_ptr<ConferenceParticipantDeviceEvent> dEvent = make_shared<ConferenceParticipantDeviceEvent>(
				EventLog::Type::ConferenceParticipantDeviceAdded,
				creationTime,
				conferenceId,
				p->getAddress(),
				d->getAddress(),
				d->getName()
			);
			dEvent->setFullState(true);
			dEvent->setNotifyId(lastNotify);
			for (const auto &l : confListeners) {
				l->onParticipantDeviceAdded(dEvent, d);
			}
		}
	}

}


LINPHONE_END_NAMESPACE
