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

shared_ptr<ParticipantDevice> Participant::addDevice (const IdentityAddress &gruu, const string &name) {
	shared_ptr<ParticipantDevice> device = findDevice(gruu, false);
	if (device)
		return device;
	device = ParticipantDevice::create(this, gruu, name);
	devices.push_back(device);
	return device;
}

void Participant::clearDevices () {
	devices.clear();
}

shared_ptr<ParticipantDevice> Participant::findDevice (const IdentityAddress &gruu, const bool logFailure) const {
	for (const auto &device : devices) {
		if (device->getAddress() == gruu)
			return device;
	}
	if (logFailure) {
		lInfo() << "Unable to find device with address " << gruu.asString();
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDevice (const shared_ptr<const CallSession> &session, const bool logFailure) {
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
	bool isSafe = true;
	for (const auto &device : getDevices()) {
		if (ignoredDevice != device) {
			auto level = device->getSecurityLevel();
			// Note: the algorithm implemented is not actually doing what it says and we may exit on the first Unsafe device
			// while we also have a ClearText one
			// It actually never occurs because in a ciphered chatroom, no one can be set as ClearText except the local
			// device when it turns off lime after joining the chatroom and this status is thus intercepted before landing here.
			switch (level) {
				case AbstractChatRoom::SecurityLevel::Unsafe:
					return level; // if one device is Unsafe the whole participant is Unsafe
				case AbstractChatRoom::SecurityLevel::ClearText:
					return level; // if one device is ClearText the whole participant is ClearText
				case AbstractChatRoom::SecurityLevel::Encrypted:
					isSafe = false; // if one device is Encrypted the whole participant is Encrypted
					break;
				case AbstractChatRoom::SecurityLevel::Safe:
					break; // if all devices are Safe the whole participant is Safe
			}
		}
	}
	if (isSafe)
		return AbstractChatRoom::SecurityLevel::Safe;
	else
		return AbstractChatRoom::SecurityLevel::Encrypted;
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
