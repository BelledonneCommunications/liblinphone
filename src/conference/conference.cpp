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
#include "logger/logger.h"
#include "participant.h"

#ifdef HAVE_ADVANCED_IM
#include "xml/resource-lists.h"
#endif

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

Conference::Conference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener
) : CoreAccessor(core) {
	this->me = Participant::create(this,myAddress);
	this->listener = listener;
}

Conference::~Conference () {
	//confListeners.clear();
}

// -----------------------------------------------------------------------------

shared_ptr<Participant> Conference::getActiveParticipant () const {
	return activeParticipant;
}

// -----------------------------------------------------------------------------

bool Conference::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	lError() << "Conference class does not handle addParticipant() generically";
	return false;
}

bool Conference::addParticipants (const list<IdentityAddress> &addresses, const CallSessionParams *params, bool hasMedia) {
	list<IdentityAddress> sortedAddresses(addresses);
	sortedAddresses.sort();
	sortedAddresses.unique();

	bool soFarSoGood = true;
	for (const auto &address : sortedAddresses)
		soFarSoGood &= addParticipant(address, params, hasMedia);
	return soFarSoGood;
}

bool Conference::canHandleParticipants () const {
	return true;
}

const ConferenceAddress &Conference::getConferenceAddress () const {
	return conferenceAddress;
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
	return subject;
}

void Conference::join () {}

void Conference::leave () {}

bool Conference::removeParticipant (const shared_ptr<Participant> &participant) {
	lError() << "Conference class does not handle removeParticipant() generically";
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
	this->subject = subject;
}

// -----------------------------------------------------------------------------

shared_ptr<Participant> Conference::findParticipant (const IdentityAddress &addr) const {

	IdentityAddress searchedAddr(addr);
	searchedAddr.setGruu("");
	for (const auto &participant : participants) {
		if (participant->getAddress() == searchedAddr)
			return participant;
	}

	return nullptr;
}

shared_ptr<Participant> Conference::findParticipant (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		if (participant->getSession() == session)
			return participant;
	}

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSession() == session)
				return device;
		}
	}

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

/*
void Conference::notifyFullState (const string &notify, const shared_ptr<ParticipantDevice> &device) {
	++lastNotify;
	for (const auto &l : confListeners) {
		l->onFullStateReceived();
	}
}
*/

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantAdded (time_t creationTime,  const bool isFullState, const Address &addr) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantAdded,
		creationTime,
		conferenceId,
		addr
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);
	for (const auto &l : confListeners) {
		l->onParticipantAdded(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const Address &addr) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		EventLog::Type::ConferenceParticipantRemoved,
		creationTime,
		conferenceId,
		addr
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);
	for (const auto &l : confListeners) {
		l->onParticipantRemoved(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const Address &addr, bool isAdmin) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
		creationTime,
		conferenceId,
		addr
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);
	for (const auto &l : confListeners) {
		l->onParticipantSetAdmin(event);
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

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu, const std::string name) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceAdded,
		creationTime,
		conferenceId,
		addr,
		gruu,
		name
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);
	for (const auto &l : confListeners) {
		l->onParticipantDeviceAdded(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceRemoved,
		creationTime,
		conferenceId,
		addr,
		gruu
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);
	for (const auto &l : confListeners) {
		l->onParticipantDeviceRemoved(event);
	}
	return event;
}

LINPHONE_END_NAMESPACE
