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

#include "conference/conference.h"
#include "core/core.h"
#include "params/media-session-params.h"
#include "participant.h"
#include "session/media-session.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

Participant::Participant(const std::shared_ptr<Conference> conference, const std::shared_ptr<const Address> &address) {
	configure(conference, address);
	creationTime = time(nullptr);
}

Participant::Participant(const std::shared_ptr<Conference> conference,
                         const std::shared_ptr<const Address> &address,
                         std::shared_ptr<CallSession> callSession)
    : Participant(conference, address) {
	session = callSession;
}

Participant::Participant(std::shared_ptr<Address> address) : mAddress(address) {
	L_ASSERT(address->getDisplayNameCstr() == nullptr);
	L_ASSERT(!address->hasUriParam("gr"));
}

Participant::~Participant() {
	lDebug() << "Destroying " << *this;
}

void Participant::configure(const std::shared_ptr<Conference> conference,
                            const std::shared_ptr<const Address> &address) {
	mAddress = Address::create(address->getUriWithoutGruu());
	setConference(conference);
	if (conference) {
		const auto &conferenceParams = conference->getCurrentParams();
		if (!conferenceParams->audioEnabled() && !conferenceParams->videoEnabled()) {
			mRole = Participant::Role::Speaker;
		}
	}
}

// =============================================================================

shared_ptr<CallSession>
Participant::createSession(const Conference &conference, const CallSessionParams *params, bool hasMedia) {
	return createSession(conference.getCore(), params, hasMedia);
}

shared_ptr<CallSession>
Participant::createSession(const std::shared_ptr<Core> &core, const CallSessionParams *params, bool hasMedia) {
	if (hasMedia && (!params || dynamic_cast<const MediaSessionParams *>(params))) {
		session = make_shared<MediaSession>(core, getSharedFromThis(), params);
	} else {
		session = make_shared<CallSession>(core, params);
	}
	return session;
}

// -----------------------------------------------------------------------------

std::shared_ptr<ParticipantDevice> Participant::addDevice(const std::shared_ptr<ParticipantDevice> &device) {
	std::shared_ptr<ParticipantDevice> newDevice = nullptr;
	const auto &session = device->getSession();
	if (session) {
		newDevice = findDevice(session, false);
		if (newDevice) {
			return newDevice;
		}
	}
	const auto &gruu = device->getAddress();
	if (gruu->isValid()) {
		newDevice = findDevice(gruu, false);
		if (newDevice) {
			return newDevice;
		}
	}

	const auto &name = device->getName();
	if (session) {
		newDevice = addDevice(session, name);
	} else if (gruu->isValid()) {
		newDevice = addDevice(gruu, name);
	} else {
		lError() << "Attempting to add " << *device << " to " << *this
		         << " that has neither call session associated nor a valid address";
		return nullptr;
	}

	if (newDevice) {
		newDevice->setState(device->getState(), false);
		newDevice->setTimeOfJoining(device->getTimeOfJoining());
		newDevice->setTimeOfDisconnection(device->getTimeOfDisconnection());
		newDevice->setJoiningMethod(device->getJoiningMethod());
		newDevice->setDisconnectionMethod(device->getDisconnectionMethod());
		newDevice->setDisconnectionReason(device->getDisconnectionReason());
	}

	return newDevice;
}

std::shared_ptr<ParticipantDevice> Participant::addDevice(const std::shared_ptr<CallSession> &session,
                                                          const std::string &name) {
	shared_ptr<ParticipantDevice> device = findDevice(session, false);
	if (device) return device;
	if (getCore() && (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn)) {
		lInfo() << "Add device " << (name.empty() ? "<no-name>" : name) << " with session " << session << " to "
		        << *this;
	} else {
		lDebug() << "Add device " << (name.empty() ? "<no-name>" : name) << " with session " << session << " to "
		         << *this;
	}
	device = ParticipantDevice::create(getSharedFromThis(), session, name);
	devices.push_back(device);
	return device;
}

std::shared_ptr<ParticipantDevice> Participant::addDevice(const std::shared_ptr<const Address> &gruu,
                                                          const string &name) {
	shared_ptr<ParticipantDevice> device = findDevice(gruu, false);
	if (device) return device;
	/* addDevice() is intensively called during Core startup, when loading chatrooms from database.
	 * we cannot afford to call Address:toString() for nothing when logs are disabled */
	if (getCore() && (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn)) {
		if (bctbx_log_level_enabled(BCTBX_LOG_DOMAIN, BCTBX_LOG_MESSAGE)) {
			lInfo() << "Add device " << (name.empty() ? "<no-name>" : name) << " with address " << *gruu << " to "
			        << *this;
		}
	} else {
		if (bctbx_log_level_enabled(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG)) {
			lDebug() << "Add device " << (name.empty() ? "<no-name>" : name) << " with address " << *gruu << " to "
			         << *this;
		}
	}
	device = ParticipantDevice::create(getSharedFromThis(), gruu, name);
	devices.push_back(device);
	return device;
}

void Participant::clearDevices() {
	devices.clear();
}

shared_ptr<ParticipantDevice>
Participant::findDevice(const LinphoneStreamType type, const std::string &label, const bool logFailure) const {
	for (const auto &device : devices) {
		const auto &deviceLabel = device->getStreamLabel(type);
		const auto &thumbnailLabel = device->getThumbnailStreamLabel();
		if (!label.empty() && ((!deviceLabel.empty() && deviceLabel.compare(label) == 0) ||
		                       ((type == LinphoneStreamTypeVideo) && (thumbnailLabel == label)))) {
			return device;
		}
	}
	if (logFailure) {
		lInfo() << "Unable to find device with label " << label << " among those belonging to " << *this;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDeviceByCallId(const std::string &callId, const bool logFailure) const {
	for (const auto &device : devices) {
		if (device->getCallId() == callId) return device;
	}
	if (logFailure) {
		lInfo() << "Unable to find device with call id " << callId << " among those belonging to " << *this;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDeviceBySsrc(uint32_t ssrc, LinphoneStreamType type) const {
	for (const auto &device : devices) {
		if (device->getSsrc(type) == ssrc) return device;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDevice(const std::shared_ptr<const Address> &gruu,
                                                      const bool logFailure) const {
	const auto &it = std::find_if(devices.cbegin(), devices.cend(),
	                              [&gruu](const auto &device) { return device->getAddress()->uriEqual(*gruu); });
	if (it != devices.cend()) {
		return *it;
	}

	if (logFailure) {
		lInfo() << "Unable to find device with address " << *gruu << " among those belonging to " << *this;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> Participant::findDevice(const shared_ptr<const CallSession> &session,
                                                      const bool logFailure) const {
	const auto &it = std::find_if(devices.cbegin(), devices.cend(),
	                              [&session](const auto &device) { return (device->getSession() == session); });

	if (it != devices.cend()) {
		return *it;
	}

	if (logFailure) {
		lInfo() << "Unable to find device with call session " << session << " among those belonging to " << *this;
	}
	return nullptr;
}

const list<shared_ptr<ParticipantDevice>> &Participant::getDevices() const {
	return devices;
}

void Participant::removeDevice(const shared_ptr<const CallSession> &session) {
	devices.erase(std::remove_if(devices.begin(), devices.end(),
	                             [&session](const auto &device) { return (device->getSession() == session); }),
	              devices.end());
}

void Participant::removeDevice(const std::shared_ptr<Address> &gruu) {
	devices.erase(
	    std::remove_if(devices.begin(), devices.end(),
	                   [&gruu](const auto &device) { return (device->getAddress()->getUri() == gruu->getUri()); }),
	    devices.end());
}

// -----------------------------------------------------------------------------

void Participant::setAddress(const std::shared_ptr<Address> &newAddr) {
	mAddress = Address::create(newAddr->getUriWithoutGruu());
}

const std::shared_ptr<Address> &Participant::getAddress() const {
	return mAddress;
}

AbstractChatRoom::SecurityLevel Participant::getSecurityLevel() const {
	return getSecurityLevelExcept(nullptr);
}

AbstractChatRoom::SecurityLevel
Participant::getSecurityLevelExcept(const std::shared_ptr<ParticipantDevice> &ignoredDevice) const {
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (!encryptionEngine) {
		lWarning() << *this << ": Asking participant security level but there is no encryption engine enabled";
		return AbstractChatRoom::SecurityLevel::ClearText;
	}

	std::list<std::string> participantDevices{};
	// build a list of participants devices address
	for (const auto &device : getDevices()) {
		participantDevices.push_back(device->getAddress()->asStringUriOnly());
	}
	if (ignoredDevice != nullptr) {
		participantDevices.remove(ignoredDevice->getAddress()->asStringUriOnly());
	}
	if (participantDevices.empty()) {
		return AbstractChatRoom::SecurityLevel::Safe; // There is no device to query status on, return safe
	}
	return encryptionEngine->getSecurityLevel(participantDevices);
}

// -----------------------------------------------------------------------------

std::shared_ptr<Core> Participant::getCore() const {
	auto conference = getConference();
	return conference ? conference->getCore() : nullptr;
}

std::shared_ptr<Conference> Participant::getConference() const {
	if (mConference.expired()) {
		lDebug() << "The conference owning participant " << *this << " has already been deleted";
	}
	return mConference.lock();
}

void Participant::setConference(const std::shared_ptr<Conference> conference) {
	mConference = conference;
}

void Participant::setAdmin(bool isAdmin) {
	isThisAdmin = isAdmin;
}

bool Participant::isAdmin() const {
	return isThisAdmin;
}

bool Participant::isFocus() const {
	return isThisFocus;
}

time_t Participant::getCreationTime() const {
	return creationTime;
}

bool Participant::getPreserveSession() const {
	return preserveSession;
}

void *Participant::getUserData() const {
	return mUserData;
}

void Participant::setUserData(void *ud) {
	mUserData = ud;
}

void Participant::setSequenceNumber(const int nb) {
	mSequence = nb;
};

int Participant::getSequenceNumber() const {
	return mSequence;
};

void Participant::setRole(Participant::Role role) {
	lInfo() << "Changing role of " << *this << " from " << Participant::roleToText(mRole) << " to "
	        << Participant::roleToText(role);
	mRole = role;
}

Participant::Role Participant::getRole() const {
	return mRole;
};

string Participant::roleToText(const Participant::Role &role) {
	std::string roleText = std::string();
	switch (role) {
		case Participant::Role::Speaker:
			roleText = "speaker";
			break;
		case Participant::Role::Listener:
			roleText = "listener";
			break;
		case Participant::Role::Unknown:
			roleText = "unknown";
			break;
	}
	return roleText;
}

Participant::Role Participant::textToRole(const string &str) {
	Participant::Role role = Participant::Role::Speaker;
	if (str.compare("speaker") == 0) {
		role = Participant::Role::Speaker;
	} else if (str.compare("listener") == 0) {
		role = Participant::Role::Listener;
	} else if (str.compare("unknown") == 0) {
		role = Participant::Role::Unknown;
	}
	return role;
}

ostream &operator<<(ostream &stream, Participant::Role role) {
	const auto &str = Participant::roleToText(role);
	return stream << str;
}

LINPHONE_END_NAMESPACE
