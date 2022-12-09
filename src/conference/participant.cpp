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

#include <algorithm>

#include "participant.h"
#include "params/media-session-params.h"
#include "session/media-session.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

Participant::Participant (Conference *conference, const IdentityAddress &address) {
	configure(conference, address);
	creationTime = time(nullptr);
}

Participant::Participant (Conference *conference, const IdentityAddress &address, std::shared_ptr<CallSession> callSession) : Participant(conference, address) {
	session = callSession;
}

Participant::Participant () {
}

void Participant::configure (Conference *conference, const IdentityAddress &address) {
	mConference = conference;
	addr = address.getAddressWithoutGruu();
}

Participant::~Participant() {
	clearDevices();
}

// =============================================================================

shared_ptr<CallSession> Participant::createSession (
	const Conference &conference, const CallSessionParams *params, bool hasMedia, CallSessionListener *listener
) {
	session = createSession(conference.getCore(), params, hasMedia, listener);
	return session;
}

shared_ptr<CallSession> Participant::createSession (
	const std::shared_ptr<Core> &core, const CallSessionParams *params, bool hasMedia, CallSessionListener *listener
) {
	if (hasMedia && (!params || dynamic_cast<const MediaSessionParams *>(params))) {
		session = make_shared<MediaSession>(core, getSharedFromThis(), params, listener);
	} else {
		session = make_shared<CallSession>(core, params, listener);
	}
	return session;
}

// -----------------------------------------------------------------------------

std::shared_ptr<ParticipantDevice> Participant::addDevice (const std::shared_ptr<LinphonePrivate::CallSession> &session, const std::string &name) {
	shared_ptr<ParticipantDevice> device = findDevice(session, false);
	if (device)
		return device;
	if (getCore() && (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn)) {
		lInfo() << "Add device " << (name.empty() ? "<no-name>" : name) << " with session " << session << " to participant " << getAddress().asString();
	} else {
		lDebug() << "Add device " << (name.empty() ? "<no-name>" : name) << " with session " << session << " to participant " << getAddress().asString();
	}
	device = ParticipantDevice::create(getSharedFromThis(), session, name);
	devices.push_back(device);
	return device;
}

std::shared_ptr<ParticipantDevice> Participant::addDevice (const IdentityAddress &gruu, const string &name) {
	shared_ptr<ParticipantDevice> device = findDevice(gruu, false);
	if (device)
		return device;
	if (getCore() && (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn)) {
		lInfo() << "Add device " << (name.empty() ? "<no-name>" : name) << " with address " << gruu.asString() << " to participant " << getAddress().asString();
	} else {
		lDebug() << "Add device " << (name.empty() ? "<no-name>" : name) << " with address " << gruu.asString() << " to participant " << getAddress().asString();
	}
	device = ParticipantDevice::create(getSharedFromThis(), gruu, name);
	devices.push_back(device);
	return device;
}

void Participant::clearDevices () {
	devices.clear();
}

shared_ptr<ParticipantDevice> Participant::findDevice (const std::string &label, const bool logFailure) const {
	for (const auto &device : devices) {
		const auto & deviceLabel = device->getLabel();
		if (!label.empty() && !deviceLabel.empty() && (deviceLabel.compare(label) == 0))
			return device;
	}
	if (logFailure) {
		lInfo() << "Unable to find device with label " << label;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDeviceByCallId (const std::string &callId, const bool logFailure) const {
	for (const auto &device : devices) {
		if (device->getCallId() == callId)
			return device;
	}
	if (logFailure) {
		lInfo() << "Unable to find device with call id " << callId;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDevice (const IdentityAddress &gruu, const bool logFailure) const {
	for (const auto &device : devices) {
		if (device->getAddress() == gruu)
			return device;
	}
	if (logFailure) {
		lInfo() << "Unable to find device with address " << gruu;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDevice (const shared_ptr<const CallSession> &session, const bool logFailure) const {
	for (const auto &device : devices) {
		if (device->getSession() == session)
			return device;
	}
	if (logFailure) {
		lInfo() << "Unable to find device with call session " << session;
	}
	return nullptr;
}

const list<shared_ptr<ParticipantDevice>> &Participant::getDevices () const {
	return devices;
}

void Participant::removeDevice (const shared_ptr<const CallSession> &session) {
	for (auto it = devices.begin(); it != devices.end(); it++) {
		if ((*it)->getSession() == session) {
			devices.erase(it);
			return;
		}
	}
}

void Participant::removeDevice (const IdentityAddress &gruu) {
	for (auto it = devices.begin(); it != devices.end(); it++) {
		if ((*it)->getAddress() == gruu) {
			devices.erase(it);
			return;
		}
	}
}

// -----------------------------------------------------------------------------

const IdentityAddress& Participant::getAddress () const {
	return addr;
}

AbstractChatRoom::SecurityLevel Participant::getSecurityLevel() const {
	return getSecurityLevelExcept(nullptr);
}

AbstractChatRoom::SecurityLevel Participant::getSecurityLevelExcept(const std::shared_ptr<ParticipantDevice> & ignoredDevice) const {
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (!encryptionEngine) {
		lWarning() << "Asking participant security level but there is no encryption engine enabled";
		return AbstractChatRoom::SecurityLevel::ClearText;
	}

	std::list<std::string> participantDevices{};
	// build a list of participants devices address
	for (const auto &device : getDevices()) {
		participantDevices.push_back(device->getAddress().asString());
	}
	if (ignoredDevice != nullptr) {
		participantDevices.remove(ignoredDevice->getAddress().asString());
	}
	if (participantDevices.empty()) {
		return AbstractChatRoom::SecurityLevel::Safe; // There is no device to query status on, return safe
	}
	return  encryptionEngine->getSecurityLevel(participantDevices);
}

// -----------------------------------------------------------------------------

bool Participant::isAdmin () const {
	return isThisAdmin;
}

bool Participant::isFocus () const {
	return isThisFocus;
}

time_t Participant::getCreationTime () const {
	return creationTime;
}

bool Participant::getPreserveSession () const {
	return preserveSession;
}

void *Participant::getUserData () const{
	return mUserData;
}

void Participant::setUserData (void *ud) {
	mUserData = ud;
}

LINPHONE_END_NAMESPACE
