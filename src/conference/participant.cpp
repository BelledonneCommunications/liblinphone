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

#include "object/object-p.h"
#include "participant.h"

#include "participant.h"
#include "params/media-session-params.h"
#include "session/media-session.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

Participant::Participant (Conference *conference, const IdentityAddress &address) : Object(*new ObjectPrivate) {
	mConference = conference;
	addr = address.getAddressWithoutGruu();
}

Participant::~Participant() {
}

// =============================================================================

shared_ptr<CallSession> Participant::createSession (
	const Conference &conference, const CallSessionParams *params, bool hasMedia, CallSessionListener *listener
) {
	if (hasMedia && (!params || dynamic_cast<const MediaSessionParams *>(params))) {
		session = make_shared<MediaSession>(conference.getCore(), getSharedFromThis(), params, listener);
	} else {
		session = make_shared<CallSession>(conference.getCore(), params, listener);
	}
	return session;
}

// -----------------------------------------------------------------------------

shared_ptr<ParticipantDevice> Participant::addDevice (const IdentityAddress &gruu, const string &name) {
	shared_ptr<ParticipantDevice> device = findDevice(gruu);
	if (device)
		return device;
	device = make_shared<ParticipantDevice>(this, gruu, name);
	devices.push_back(device);
	return device;
}

void Participant::clearDevices () {
	devices.clear();
}

shared_ptr<ParticipantDevice> Participant::findDevice (const IdentityAddress &gruu) const {
	for (const auto &device : devices) {
		if (device->getAddress() == gruu)
			return device;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDevice (const shared_ptr<const CallSession> &session) {
	for (const auto &device : devices) {
		if (device->getSession() == session)
			return device;
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

AbstractChatRoom::SecurityLevel Participant::getSecurityLevel () const {
	bool isSafe = true;
	for (const auto &device : getDevices()) {
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
	if (isSafe)
		return AbstractChatRoom::SecurityLevel::Safe;
	else
		return AbstractChatRoom::SecurityLevel::Encrypted;
}

// -----------------------------------------------------------------------------

bool Participant::isAdmin () const {
	return isThisAdmin;
}

LINPHONE_END_NAMESPACE
