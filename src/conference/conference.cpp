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

#include "conference.h"
#include "conference/participant-device.h"
#include "conference/session/call-session-p.h"
#include "content/content.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core.h"
#include "logger/logger.h"
#include "participant.h"

#ifdef HAVE_ADVANCED_IM
#include "xml/resource-lists.h"
#endif

#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE


ConferenceParams::ConferenceParams(const LinphoneCore *core) {
	if(core) {
		const LinphoneVideoPolicy *policy = linphone_core_get_video_policy(core);
		if(policy->automatically_initiate) m_enableVideo = true;
	}
}

Conference::Conference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<ConferenceParams> params
) : CoreAccessor(core) {
	this->me = Participant::create(this,myAddress);
	this->me->setFocus(true);
	this->listener = listener;
	this->update(*params);
	this->confParams->setMe(myAddress);
}

Conference::~Conference () {
	confListeners.clear();
}

// -----------------------------------------------------------------------------

shared_ptr<Participant> Conference::getActiveParticipant () const {
	return activeParticipant;
}

void Conference::clearParticipants () {
	participants.clear();
}

// -----------------------------------------------------------------------------

bool Conference::addParticipant (std::shared_ptr<Call> call) {
	lError() << "Conference class does not handle addParticipant() generically";
	return false;
}

bool Conference::addParticipant (const IdentityAddress &participantAddress) {
	shared_ptr<Participant> participant = findParticipant(participantAddress);
	if (participant) {
		lInfo() << "Not adding participant '" << participantAddress.asString() << "' because it is already a participant of the Conference";
		return false;
	}
	participant = Participant::create(this,participantAddress);
	// TODO: Use conference parameters to fill args
	participant->createSession(*this, nullptr, (confParams->chatEnabled() == false), listener);
	participant->setFocus(participantAddress == getConferenceAddress());
	participant->setPreserveSession(false);
	participants.push_back(participant);
	if (!activeParticipant)
		activeParticipant = participant;
	return true;


}

bool Conference::addParticipants (const std::list<IdentityAddress> &addresses) {
	list<IdentityAddress> sortedAddresses(addresses);
	sortedAddresses.sort();
	sortedAddresses.unique();

	bool soFarSoGood = true;
	for (const auto &address : sortedAddresses)
		soFarSoGood &= addParticipant(address);
	return soFarSoGood;
}

const ConferenceAddress Conference::getConferenceAddress () const {
	return confParams->getConferenceAddress();
}

void Conference::setConferenceAddress (const ConferenceAddress &conferenceAddress) {
	return confParams->setConferenceAddress(conferenceAddress);
}

shared_ptr<Participant> Conference::getMe () const {
	return me;
}

int Conference::getParticipantCount () const {
	return static_cast<int>(getParticipants().size());
}

const list<shared_ptr<Participant>> &Conference::getParticipants () const {
	return participants;
}

const string &Conference::getSubject () const {
	return confParams->getSubject();
}

void Conference::join (const IdentityAddress &participantAddress) {}

void Conference::join () {}

void Conference::leave () {}

bool Conference::update(const ConferenceParamsInterface &newParameters) {
	confParams = ConferenceParams::create(static_cast<const ConferenceParams&>(newParameters));
	return true;
};

bool Conference::removeParticipant (const shared_ptr<Participant> &participant) {
	for (const auto &p : participants) {
		if (participant->getAddress() == p->getAddress()) {
			participants.remove(p);
			return true;
		}
	}
	return false;
}

bool Conference::removeParticipants (const list<shared_ptr<Participant>> &participants) {
	bool soFarSoGood = true;
	for (const auto &p : participants)
		soFarSoGood &= removeParticipant(p);
	return soFarSoGood;
}

void Conference::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	lError() << "Conference class does not handle setParticipantAdminStatus() generically";
}

void Conference::setSubject (const string &subject) {
	confParams->setSubject(subject);
}

// -----------------------------------------------------------------------------

shared_ptr<Participant> Conference::findParticipant (const IdentityAddress &addr) const {

	IdentityAddress searchedAddr(addr);
	searchedAddr.setGruu("");
	for (const auto &participant : participants) {
		if (participant->getAddress() == searchedAddr) {
			return participant;
		}
	}

	lInfo() << "Unable to find participant in conference " << this << " with address " << addr.asString();
	return nullptr;
}

shared_ptr<Participant> Conference::findParticipant (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		if (participant->getSession() == session)
			return participant;
	}
	lInfo() << "Unable to find participant in conference " << this << " with call session " << session;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSession() == session)
				return device;
		}
	}

	lInfo() << "Unable to find participant device in conference " << this << " with call session " << session;

	return nullptr;
}

// -----------------------------------------------------------------------------

bool Conference::isMe (const IdentityAddress &addr) const {
	IdentityAddress cleanedAddr(addr);
	cleanedAddr.setGruu("");
	IdentityAddress cleanedMeAddr(me->getAddress());
	cleanedMeAddr.setGruu("");
	return cleanedMeAddr == cleanedAddr;
}

// -----------------------------------------------------------------------------

string Conference::getResourceLists (const list<IdentityAddress> &addresses) const {
#ifdef HAVE_ADVANCED_IM
	Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
	Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();
	for (const auto &addr : addresses) {
		Xsd::ResourceLists::EntryType entry = Xsd::ResourceLists::EntryType(addr.asString());
		l.getEntry().push_back(entry);
	}
	rl.getList().push_back(l);

	Xsd::XmlSchema::NamespaceInfomap map;
	stringstream xmlBody;
	serializeResourceLists(xmlBody, rl, map);
	return xmlBody.str();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return "";
#endif
}

// -----------------------------------------------------------------------------

list<IdentityAddress> Conference::parseResourceLists (const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if ((content.getContentType() == ContentType::ResourceLists)
		&& ((content.getContentDisposition().weakEqual(ContentDisposition::RecipientList))
			|| (content.getContentDisposition().weakEqual(ContentDisposition::RecipientListHistory))
		)
	) {
		istringstream data(content.getBodyAsString());
		unique_ptr<Xsd::ResourceLists::ResourceLists> rl(Xsd::ResourceLists::parseResourceLists(
			data,
			Xsd::XmlSchema::Flags::dont_validate
		));
		list<IdentityAddress> addresses;
		for (const auto &l : rl->getList()) {
			for (const auto &entry : l.getEntry()) {
				IdentityAddress addr(entry.getUri());
				addresses.push_back(move(addr));
			}
		}
		return addresses;
	}
	return list<IdentityAddress>();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return list<IdentityAddress>();
#endif
}

void Conference::setLastNotify (unsigned int lastNotify) {
	this->lastNotify = lastNotify;
}

void Conference::setConferenceId (const ConferenceId &conferenceId) {
	this->conferenceId = conferenceId;
}

const ConferenceId &Conference::getConferenceId () const {
	return conferenceId;
}

void Conference::resetLastNotify () {
	setLastNotify(0);
}

void Conference::notifyFullState () {
	for (const auto &l : confListeners) {
		l->onFullStateReceived();
	}
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantAdded,
		creationTime,
		conferenceId,
		participant->getAddress()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantAdded(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantRemoved,
		creationTime,
		conferenceId,
		participant->getAddress()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantRemoved(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
		creationTime,
		conferenceId,
		participant->getAddress()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantSetAdmin(event, participant);
	}
	return event;
}

shared_ptr<ConferenceSubjectEvent> Conference::notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject) {
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(
		creationTime,
		conferenceId,
		subject
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onSubjectChanged(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceAdded,
		creationTime,
		conferenceId,
		participant->getAddress(),
		participantDevice->getAddress(),
		participantDevice->getName()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceAdded(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceRemoved,
		creationTime,
		conferenceId,
		participant->getAddress(),
		participantDevice->getAddress(),
		participantDevice->getName()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceRemoved(event, participantDevice);
	}
	return event;
}

void Conference::setState (LinphonePrivate::ConferenceInterface::State state) {
	if (this->state != state) {
		if (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalStartup) {
			lDebug() << "Switching conference [" << this << "] from state " << this->state << " to " << state;
		} else {
			lInfo() << "Switching conference [" << this << "] from state " << this->state << " to " << state;
		}
		this->state = state;
		notifyStateChanged(state);
	}
}

void Conference::notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) {
	for (const auto &l : confListeners) {
		l->onStateChanged(state);
	}
}

LINPHONE_END_NAMESPACE
