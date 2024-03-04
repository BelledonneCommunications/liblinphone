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

#include <bctoolbox/defs.h>

#include "conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant-device.h"
#include "conference/participant-info.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"
#include "core/core.h"
#include "logger/logger.h"
#include "participant.h"
#include "private_functions.h"

#ifdef HAVE_ADVANCED_IM
#include "xml/resource-lists.h"
#endif

#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::string Conference::SecurityModeParameter = "conference-security-mode";

Conference::Conference(const shared_ptr<Core> &core,
                       const std::shared_ptr<Address> &myAddress,
                       CallSessionListener *listener,
                       const std::shared_ptr<ConferenceParams> params)
    : CoreAccessor(core) {
	this->me = Participant::create(this, myAddress);
	this->listener = listener;
	this->update(*params);
	this->confParams->setMe(myAddress);
	this->startTime = ms_time(nullptr);
}

Conference::~Conference() {
	confListeners.clear();
}

// -----------------------------------------------------------------------------

time_t Conference::getStartTime() const {
	return startTime;
}

int Conference::getDuration() const {
	return (int)(ms_time(nullptr) - startTime);
}

shared_ptr<Participant> Conference::getActiveParticipant() const {
	return activeParticipant;
}

void Conference::clearParticipants() {
	me->clearDevices();
	participants.clear();
}

// -----------------------------------------------------------------------------

bool Conference::addParticipant(BCTBX_UNUSED(const std::shared_ptr<ParticipantInfo> &info)) {
	lError() << "Conference class does not handle addParticipant() generically";
	return false;
}

bool Conference::addParticipant(BCTBX_UNUSED(std::shared_ptr<Call> call)) {
	lError() << "Conference class does not handle addParticipant() generically";
	return false;
}

bool Conference::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	shared_ptr<Participant> participant = findParticipant(participantAddress);
	if (participant) {
		lWarning() << "Not adding participant '" << *participantAddress
		           << "' because it is already a participant of the Conference";
		return false;
	}
	participant = Participant::create(this, participantAddress);
	participant->createSession(*this, nullptr, (confParams->chatEnabled() == false), listener);
	const auto confAddr = getConferenceAddress();
	bool isFocus = participantAddress && confAddr && (*participantAddress == *confAddr);
	participant->setFocus(isFocus);
	participant->setPreserveSession(false);
	participants.push_back(participant);
	if (!activeParticipant) activeParticipant = participant;
	return true;
}

const std::shared_ptr<CallSession> Conference::getMainSession() const {
	return me->getSession();
}

bool Conference::addParticipants(const std::list<std::shared_ptr<Address>> &addresses) {
	list<std::shared_ptr<Address>> sortedAddresses(addresses);
	sortedAddresses.sort([](const auto &addr1, const auto &addr2) { return *addr1 < *addr2; });
	sortedAddresses.unique([](const auto &addr1, const auto &addr2) { return addr1->weakEqual(*addr2); });

	bool soFarSoGood = true;
	for (const auto &address : sortedAddresses)
		soFarSoGood &= addParticipant(address);
	return soFarSoGood;
}

bool Conference::addParticipants(const std::list<std::shared_ptr<Call>> &calls) {
	list<std::shared_ptr<Call>> sortedCalls(calls);
	sortedCalls.sort();
	sortedCalls.unique();

	bool soFarSoGood = true;
	for (const auto &call : sortedCalls)
		soFarSoGood &= addParticipant(call);

	return soFarSoGood;
}

LinphoneStatus Conference::updateMainSession(bool modifyParams) {
	LinphoneStatus ret = -1;
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams *params = session->getMediaParams();
		if (params->rtpBundleEnabled()) {
			MediaSessionParams *currentParams = params->clone();
			currentParams->getPrivate()->setInternalCallUpdate(false);

			// Update parameters based on conference capabilities
			if (!confParams->audioEnabled()) {
				currentParams->enableAudio(confParams->audioEnabled());
			}
			if (!confParams->videoEnabled()) {
				currentParams->enableVideo(confParams->videoEnabled());
			}

			bool wasScreenSharingEnabled = currentParams->screenSharingEnabled();
			if (modifyParams) {
				const auto screenSharingParticipant = getScreenSharingParticipant();
				bool screenSharingEnabled = false;
				if (getCachedScreenSharingDevice()) {
					// Disable screen sharing if no participant is screen sharing or the participant who is sharing its
					// screen is not the me participant
					screenSharingEnabled = screenSharingParticipant && isMe(screenSharingParticipant->getAddress());
				} else {
					// If no one was screen sharing, we can try again to enabled screen sharing
					screenSharingEnabled = wasScreenSharingEnabled;
				}
				currentParams->enableScreenSharing(screenSharingEnabled);
			}
			ret = session->update(currentParams);
			// Restore the screen sharing flag as it was before and change local parameters
			currentParams->enableScreenSharing(wasScreenSharingEnabled);
			session->getPrivate()->setParams(currentParams);
		} else {
			lWarning() << "Unable to update session " << session << " of conference " << *getConferenceAddress()
			           << " because RTP bundle is disabled";
			ret = 0;
		}
	}
	return ret;
}

ConferenceLayout Conference::getLayout() const {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	return session ? session->getParams()->getConferenceVideoLayout()
	               : (ConferenceLayout)linphone_core_get_default_conference_layout(getCore()->getCCore());
}

void Conference::setLayout(const ConferenceLayout layout) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session && (getLayout() != layout)) {
		lInfo() << "Changing layout of conference " << getConferenceAddress() << " from " << getLayout() << " to "
		        << layout;
		const_cast<LinphonePrivate::CallSessionParams *>(session->getParams())->setConferenceVideoLayout(layout);
		updateMainSession();
	}
}

const std::shared_ptr<Address> &Conference::getConferenceAddress() const {
	return confParams->getConferenceAddress();
}

void Conference::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	confParams->setConferenceAddress(conferenceAddress);
}

shared_ptr<Participant> Conference::getMe() const {
	return me;
}

int Conference::getParticipantCount() const {
	return static_cast<int>(getParticipants().size());
}

const list<shared_ptr<Participant>> &Conference::getParticipants() const {
	return participants;
}

const list<shared_ptr<ParticipantDevice>> Conference::getParticipantDevices() const {
	list<shared_ptr<ParticipantDevice>> devices;
	for (const auto &p : participants) {
		const auto &d = p->getDevices();
		if (!d.empty()) {
			devices.insert(devices.end(), d.begin(), d.end());
		}
	}
	if (isIn()) {
		const auto &d = getMe()->getDevices();
		if (!d.empty()) {
			devices.insert(devices.begin(), d.begin(), d.end());
		}
	}

	return devices;
}

void Conference::setCachedScreenSharingDevice() {
	cachedScreenSharingDevice = getScreenSharingDevice();
}

void Conference::resetCachedScreenSharingDevice() {
	cachedScreenSharingDevice = nullptr;
}

std::shared_ptr<ParticipantDevice> Conference::getCachedScreenSharingDevice() const {
	return cachedScreenSharingDevice;
}

const std::shared_ptr<Participant> Conference::getScreenSharingParticipant() const {
	const auto device = getScreenSharingDevice();
	return (device) ? device->getParticipant() : nullptr;
}

const std::shared_ptr<ParticipantDevice> Conference::getScreenSharingDevice() const {
	const auto devices = getParticipantDevices();
	const auto screenSharingDeviceIt =
	    std::find_if(devices.cbegin(), devices.cend(), [](const auto &d) { return d->screenSharingEnabled(); });
	return (screenSharingDeviceIt == devices.cend()) ? nullptr : (*screenSharingDeviceIt);
}

const string &Conference::getSubject() const {
	return confParams->getSubject();
}

const string &Conference::getUtf8Subject() const {
	return confParams->getUtf8Subject();
}

const string &Conference::getUsername() const {
	return mUsername;
}

void Conference::join(BCTBX_UNUSED(const std::shared_ptr<Address> &participantAddress)) {
}

void Conference::join() {
}

void Conference::leave() {
}

void Conference::setLocalParticipantStreamCapability(BCTBX_UNUSED(const LinphoneMediaDirection &direction),
                                                     BCTBX_UNUSED(const LinphoneStreamType type)) {
}

bool Conference::update(const ConferenceParamsInterface &newParameters) {
	const LinphonePrivate::ConferenceParams &newConfParams = static_cast<const ConferenceParams &>(newParameters);
	if (confParams && ((confParams->getConferenceFactoryAddress() != newConfParams.getConferenceFactoryAddress()) ||
	                   (confParams->getConferenceAddress() != newConfParams.getConferenceAddress()))) {
		lError() << "Trying to change frozen conference parameters:";
		lError() << " -  factory address: actual " << confParams->getConferenceFactoryAddress() << " new value "
		         << newConfParams.getConferenceFactoryAddress();
		lError() << " -  conference address: actual " << confParams->getConferenceAddress() << " new value "
		         << newConfParams.getConferenceAddress();
		return false;
	}
	confParams = ConferenceParams::create(newConfParams);
	return true;
};

bool Conference::removeParticipant(const shared_ptr<Participant> &participant) {
	for (const auto &p : participants) {
		if (*participant->getAddress() == *p->getAddress()) {
			participants.remove(p);
			return true;
		}
	}
	return false;
}

bool Conference::removeParticipants(const list<shared_ptr<Participant>> &participants) {
	bool soFarSoGood = true;
	for (const auto &p : participants)
		soFarSoGood &= removeParticipant(p);
	return soFarSoGood;
}

void Conference::setParticipantAdminStatus(BCTBX_UNUSED(const shared_ptr<Participant> &participant),
                                           BCTBX_UNUSED(bool isAdmin)) {
	lError() << "Conference class does not handle setParticipantAdminStatus() generically";
}

void Conference::setUtf8Subject(const string &subject) {
	setSubject(Utils::utf8ToLocale(subject));
}

void Conference::setSubject(const string &subject) {
	confParams->setSubject(subject);
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceStateChanged(time_t creationTime,
                                                const bool isFullState,
                                                const std::shared_ptr<Participant> &participant,
                                                const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceStatusChanged, creationTime, conferenceId, participant->getAddress(),
	    participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceStateChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceScreenSharingChanged(time_t creationTime,
                                                        const bool isFullState,
                                                        const std::shared_ptr<Participant> &participant,
                                                        const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceStatusChanged, creationTime, conferenceId, participant->getAddress(),
	    participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceScreenSharingChanged(event, participantDevice);
	}
	return event;
}

void Conference::notifySpeakingDevice(uint32_t ssrc, bool isSpeaking) {
	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
				_linphone_participant_device_notify_is_speaking_changed(device->toC(), isSpeaking);
				for (const auto &l : confListeners) {
					l->onParticipantDeviceIsSpeakingChanged(device, isSpeaking);
				}
				return;
			}
		}
	}
	for (const auto &device : getMe()->getDevices()) {
		if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
			_linphone_participant_device_notify_is_speaking_changed(device->toC(), isSpeaking);
			for (const auto &l : confListeners) {
				l->onParticipantDeviceIsSpeakingChanged(device, isSpeaking);
			}
			return;
		}
	}
	lDebug() << "IsSpeaking: unable to notify speaking device because there is no device found.";
}

void Conference::notifyMutedDevice(uint32_t ssrc, bool muted) {
	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
				_linphone_participant_device_notify_is_muted(device->toC(), muted);
				for (const auto &l : confListeners) {
					l->onParticipantDeviceIsMuted(device, muted);
				}
				pendingParticipantsMutes.erase(ssrc);
				return;
			}
		}
	}
	for (const auto &device : getMe()->getDevices()) {
		if (device->getSsrc(LinphoneStreamTypeAudio) == ssrc) {
			_linphone_participant_device_notify_is_muted(device->toC(), muted);
			for (const auto &l : confListeners) {
				l->onParticipantDeviceIsMuted(device, muted);
			}
			pendingParticipantsMutes.erase(ssrc);
			return;
		}
	}
	pendingParticipantsMutes[ssrc] = muted;
	lDebug() << "IsMuted: unable to notify muted device because there is no device found - queuing it waiting to match "
	            "a device to ssrc "
	         << ssrc;
}

void Conference::notifyLocalMutedDevices(bool muted) {
	for (const auto &device : getMe()->getDevices()) {
		_linphone_participant_device_notify_is_muted(device->toC(), muted);
		for (const auto &l : confListeners) {
			l->onParticipantDeviceIsMuted(device, muted);
		}
	}
}

const std::map<uint32_t, bool> &Conference::getPendingParticipantsMutes() const {
	return pendingParticipantsMutes;
}

void Conference::setUsername(const string &username) {
	mUsername = username;
}

// -----------------------------------------------------------------------------

shared_ptr<Participant> Conference::findParticipant(const std::shared_ptr<Address> &addr) const {
	for (const auto &participant : participants) {
		if (participant->getAddress()->weakEqual(*addr)) {
			return participant;
		}
	}

	lWarning() << "Unable to find participant in conference "
	           << (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<unknown address>"))
	           << " (" << this << ") with address " << *addr;
	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDeviceByLabel(const LinphoneStreamType type,
                                                                       const std::string &label) const {
	for (const auto &participant : participants) {
		auto device = participant->findDevice(type, label, false);
		if (device) return device;
	}

	lDebug() << "Unable to find participant device in conference "
	         << (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<unknown address>"))
	         << " with " << std::string(linphone_stream_type_to_string(type)) << " label " << label;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDeviceBySsrc(uint32_t ssrc, LinphoneStreamType type) const {
	for (const auto &participant : participants) {
		auto device = participant->findDeviceBySsrc(ssrc, type);
		if (device) {
			return device;
		}
	}

	lDebug() << "Unable to find participant device in conference "
	         << (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<unknown address>"))
	         << " with ssrc " << ssrc;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice(const std::shared_ptr<Address> &pAddr,
                                                                const std::shared_ptr<Address> &dAddr) const {
	for (const auto &participant : participants) {
		if (pAddr->weakEqual(*participant->getAddress())) {
			auto device = participant->findDevice(dAddr, false);
			if (device) {
				return device;
			}
		}
	}

	lDebug() << "Unable to find participant device in conference "
	         << (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<unknown-address>"))
	         << " with device address " << dAddr->toString() << " belonging to participant " << pAddr->toString();

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice(const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		auto device = participant->findDevice(session, false);
		if (device) {
			return device;
		}
	}

	lDebug() << "Unable to find participant device in conference "
	         << (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<unknown-address>"))
	         << " with call session " << session;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::getActiveSpeakerParticipantDevice() const {
	return activeSpeakerDevice;
}

std::map<ConferenceMediaCapabilities, bool> Conference::getMediaCapabilities() const {
	std::map<ConferenceMediaCapabilities, bool> mediaCapabilities;
	mediaCapabilities[ConferenceMediaCapabilities::Audio] = confParams->audioEnabled();
	mediaCapabilities[ConferenceMediaCapabilities::Video] = confParams->videoEnabled();
	mediaCapabilities[ConferenceMediaCapabilities::Text] = confParams->chatEnabled();
	return mediaCapabilities;
}

// -----------------------------------------------------------------------------

bool Conference::isMe(const std::shared_ptr<Address> &addr) const {
	Address cleanedAddr = addr->getUriWithoutGruu();
	Address cleanedMeAddr = me->getAddress()->getUriWithoutGruu();
	return cleanedMeAddr == cleanedAddr;
}

// -----------------------------------------------------------------------------

void Conference::setLastNotify(unsigned int lastNotify) {
	this->lastNotify = lastNotify;
}

void Conference::setConferenceId(const ConferenceId &conferenceId) {
	this->conferenceId = conferenceId;
}

const ConferenceId &Conference::getConferenceId() const {
	return conferenceId;
}

void Conference::resetLastNotify() {
	setLastNotify(0);
}

void Conference::notifyFullState() {
	for (const auto &l : confListeners) {
		l->onFullStateReceived();
	}
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantAdded(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    EventLog::Type::ConferenceParticipantAdded, creationTime, conferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantAdded(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantRemoved(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    EventLog::Type::ConferenceParticipantRemoved, creationTime, conferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantRemoved(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent>
Conference::notifyParticipantSetRole(time_t creationTime,
                                     const bool isFullState,
                                     const std::shared_ptr<Participant> &participant,
                                     Participant::Role role) {
	EventLog::Type eventType = EventLog::Type::None;
	switch (role) {
		case Participant::Role::Speaker:
			eventType = EventLog::Type::ConferenceParticipantRoleSpeaker;
			break;
		case Participant::Role::Listener:
			eventType = EventLog::Type::ConferenceParticipantRoleListener;
			break;
		case Participant::Role::Unknown:
			eventType = EventLog::Type::ConferenceParticipantRoleUnknown;
			break;
	}
	shared_ptr<ConferenceParticipantEvent> event =
	    make_shared<ConferenceParticipantEvent>(eventType, creationTime, conferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantSetRole(event, participant);
	}
	return event;
}

shared_ptr<ConferenceParticipantEvent> Conference::notifyParticipantSetAdmin(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    isAdmin ? EventLog::Type::ConferenceParticipantSetAdmin : EventLog::Type::ConferenceParticipantUnsetAdmin,
	    creationTime, conferenceId, participant->getAddress());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantSetAdmin(event, participant);
	}
	return event;
}

shared_ptr<ConferenceSubjectEvent>
Conference::notifySubjectChanged(time_t creationTime, const bool isFullState, const std::string subject) {
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(creationTime, conferenceId, subject);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onSubjectChanged(event);
	}
	return event;
}

shared_ptr<ConferenceAvailableMediaEvent> Conference::notifyAvailableMediaChanged(
    time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	shared_ptr<ConferenceAvailableMediaEvent> event =
	    make_shared<ConferenceAvailableMediaEvent>(creationTime, conferenceId, mediaCapabilities);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onAvailableMediaChanged(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceAdded(time_t creationTime,
                                         const bool isFullState,
                                         const std::shared_ptr<Participant> &participant,
                                         const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceAdded, creationTime, conferenceId, participant->getAddress(),
	    participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceAdded(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceRemoved(time_t creationTime,
                                           const bool isFullState,
                                           const std::shared_ptr<Participant> &participant,
                                           const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceRemoved, creationTime, conferenceId, participant->getAddress(),
	    participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceRemoved(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent>
Conference::notifyParticipantDeviceMediaCapabilityChanged(time_t creationTime,
                                                          const bool isFullState,
                                                          const std::shared_ptr<Participant> &participant,
                                                          const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged, creationTime, conferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceMediaCapabilityChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceMediaAvailabilityChanged(
    time_t creationTime,
    const bool isFullState,
    const std::shared_ptr<Participant> &participant,
    const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged, creationTime, conferenceId,
	    participant->getAddress(), participantDevice->getAddress(), participantDevice->getName());
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceMediaAvailabilityChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent>
Conference::notifyEphemeralModeChanged(time_t creationTime, const bool isFullState, const EventLog::Type type) {
	L_ASSERT((type == EventLog::Type::ConferenceEphemeralMessageManagedByAdmin) ||
	         (type == EventLog::Type::ConferenceEphemeralMessageManagedByParticipants));
	shared_ptr<ConferenceEphemeralMessageEvent> event =
	    make_shared<ConferenceEphemeralMessageEvent>(type, creationTime, conferenceId, 0);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onEphemeralModeChanged(event);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent>
Conference::notifyEphemeralMessageEnabled(time_t creationTime, const bool isFullState, const bool enable) {

	shared_ptr<ConferenceEphemeralMessageEvent> event =
	    make_shared<ConferenceEphemeralMessageEvent>((enable) ? EventLog::Type::ConferenceEphemeralMessageEnabled
	                                                          : EventLog::Type::ConferenceEphemeralMessageDisabled,
	                                                 creationTime, getConferenceId(), 0);

	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onEphemeralMessageEnabled(event);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent>
Conference::notifyEphemeralLifetimeChanged(time_t creationTime, const bool isFullState, const long lifetime) {
	shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
	    EventLog::Type::ConferenceEphemeralMessageLifetimeChanged, creationTime, conferenceId, lifetime);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onEphemeralLifetimeChanged(event);
	}
	return event;
}

void Conference::setState(LinphonePrivate::ConferenceInterface::State state) {
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

void Conference::notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	for (const auto &l : confListeners) {
		l->onStateChanged(state);
	}
}

void Conference::notifyActiveSpeakerParticipantDevice(const std::shared_ptr<ParticipantDevice> &participantDevice) {
	activeSpeakerDevice = participantDevice;
	for (const auto &l : confListeners) {
		l->onActiveSpeakerParticipantDevice(participantDevice);
	}
}

std::shared_ptr<ConferenceInfo> Conference::createOrGetConferenceInfo() const {
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		std::shared_ptr<ConferenceInfo> conferenceInfo =
		    getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(getConferenceAddress());
		if (conferenceInfo) {
			return conferenceInfo;
		}
	}
#endif // HAVE_DB_STORAGE
	return createConferenceInfo();
}

std::shared_ptr<ConferenceInfo> Conference::createConferenceInfo() const {
	return nullptr;
}

const std::shared_ptr<ParticipantDevice> Conference::getFocusOwnerDevice() const {
	std::shared_ptr<ParticipantDevice> focusOwnerDevice = nullptr;
	const auto devices = getParticipantDevices();
	const auto deviceIt = std::find_if(devices.cbegin(), devices.cend(), [](const auto &device) {
		return (device->getJoiningMethod() == ParticipantDevice::JoiningMethod::FocusOwner);
	});
	if (deviceIt != devices.cend()) {
		focusOwnerDevice = (*deviceIt);
	}
	return focusOwnerDevice;
}

const std::shared_ptr<ConferenceInfo> Conference::getUpdatedConferenceInfo() const {
	auto conferenceInfo = createOrGetConferenceInfo();

	const auto focusOwnerDevice = getFocusOwnerDevice();
	if (focusOwnerDevice) {
		const auto &organizer = focusOwnerDevice->getParticipant()->getAddress();
		if (organizer) {
			auto organizerInfo = ParticipantInfo::create(Address::create(organizer->getUri()));
			for (const auto &[name, value] : organizer->getParams()) {
				organizerInfo->addParameter(name, value);
			}
			conferenceInfo->setOrganizer(organizerInfo);
		}
	}

	// Update me only if he/she is already in the list of participants info
	const auto &meAddress = getMe()->getAddress();
	const auto &currentParticipants = conferenceInfo->getParticipants();
	const auto meInfoIt =
	    std::find_if(currentParticipants.begin(), currentParticipants.end(),
	                 [&meAddress](const auto &p) { return (meAddress->weakEqual(*p->getAddress())); });
	if (meInfoIt != currentParticipants.end()) {
		updateParticipantInfoInConferenceInfo(conferenceInfo, getMe());
	}

	for (const auto &participant : getParticipants()) {
		updateParticipantInfoInConferenceInfo(conferenceInfo, participant);
	}

	// Update start time and duration as this information can be sent through the SUBSCRIBE/NOTIFY dialog. In fact, if a
	// client dials a conference without prior knowledge (for example it is given an URI to call), the start and end
	// time are initially estimated as there is no conference information associated to that URI.
	time_t startTime = confParams->getStartTime();
	time_t endTime = confParams->getEndTime();
	conferenceInfo->setDateTime(startTime);
	if ((startTime >= 0) && (endTime >= 0) && (endTime > startTime)) {
		unsigned int duration = (static_cast<unsigned int>(endTime - startTime)) / 60;
		conferenceInfo->setDuration(duration);
	}

	conferenceInfo->setSecurityLevel(confParams->getSecurityLevel());
	conferenceInfo->setSubject(confParams->getSubject());

	return conferenceInfo;
}

std::shared_ptr<ConferenceInfo> Conference::createConferenceInfoWithCustomParticipantList(
    const std::shared_ptr<Address> &organizer, const ConferenceInfo::participant_list_t invitedParticipants) const {
	std::shared_ptr<ConferenceInfo> info = ConferenceInfo::create();
	if (organizer) {
		auto organizerInfo = ParticipantInfo::create(Address::create(organizer->getUri()));
		for (const auto &[name, value] : organizer->getParams()) {
			organizerInfo->addParameter(name, value);
		}
		info->setOrganizer(organizerInfo);
	}
	for (const auto &participant : invitedParticipants) {
		info->addParticipant(participant);
	}

	const auto &conferenceAddress = getConferenceAddress();
	if (conferenceAddress && conferenceAddress->isValid()) {
		info->setUri(conferenceAddress);
	}

	time_t startTime = confParams->getStartTime();
	time_t endTime = confParams->getEndTime();
	info->setDateTime(startTime);
	if ((startTime >= 0) && (endTime >= 0) && (endTime > startTime)) {
		unsigned int duration = (static_cast<unsigned int>(endTime - startTime)) / 60;
		info->setDuration(duration);
	}

	info->setSubject(confParams->getSubject());
	info->setSecurityLevel(confParams->getSecurityLevel());

	return info;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Conference::updateSecurityLevelInConferenceInfo(const ConferenceParams::SecurityLevel &level) const {
#ifdef HAVE_DB_STORAGE
	if ((getState() == ConferenceInterface::State::CreationPending) ||
	    (getState() == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			info->setSecurityLevel(level);

			// Store into DB after the start incoming notification in order to have a valid conference address being the
			// contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo() << "Updating conference information of conference " << *getConferenceAddress()
				        << " because its security level has been changed to " << level;
				mainDb->insertConferenceInfo(info);
			}
		}
	}
#endif // HAVE_DB_STORAGE
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Conference::updateSubjectInConferenceInfo(const std::string &subject) const {
#ifdef HAVE_DB_STORAGE
	if ((getState() == ConferenceInterface::State::CreationPending) ||
	    (getState() == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			info->setSubject(subject);

			// Store into DB after the start incoming notification in order to have a valid conference address being the
			// contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo() << "Updating conference information of conference " << *getConferenceAddress()
				        << " because its subject has been changed to " << subject;
				mainDb->insertConferenceInfo(info);
			}
		}
	}
#endif // HAVE_DB_STORAGE
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Conference::updateParticipantRoleInConferenceInfo(const std::shared_ptr<Participant> &participant) const {
#ifdef HAVE_DB_STORAGE
	if ((getState() == ConferenceInterface::State::CreationPending) ||
	    (getState() == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			const auto &address = participant->getAddress();
			const auto &newRole = participant->getRole();
			const auto &participantInfo = info->findParticipant(address);
			if (participantInfo) {
				auto newParticipantInfo = participantInfo->clone()->toSharedPtr();
				newParticipantInfo->setRole(newRole);

				info->updateParticipant(newParticipantInfo);

				// Store into DB after the start incoming notification in order to have a valid conference address being
				// the contact address of the call
				auto &mainDb = getCore()->getPrivate()->mainDb;
				if (mainDb) {
					lInfo() << "Updating conference information of conference " << *getConferenceAddress()
					        << " because the role of participant " << *address << " changed to " << newRole;
					mainDb->insertConferenceInfo(info);
				}
			} else {
				lError() << "Unable to update role of participant " << *address << " to " << newRole
				         << " because it cannot be found in the conference info linked to conference "
				         << *getConferenceAddress();
			}
		}
	}
#endif // HAVE_DB_STORAGE
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

bool Conference::updateParticipantInfoInConferenceInfo(std::shared_ptr<ConferenceInfo> &info,
                                                       const std::shared_ptr<Participant> &participant) const {
	bool update = false;
	const auto &participantAddress = participant->getAddress();
	const auto &currentParticipants = info->getParticipants();
	const auto participantInfoIt =
	    std::find_if(currentParticipants.begin(), currentParticipants.end(), [&participantAddress](const auto &p) {
		    return (participantAddress->weakEqual(*p->getAddress()));
	    });

	const auto &participantRole = participant->getRole();
	if (participantInfoIt == currentParticipants.end()) {
		auto participantInfo = ParticipantInfo::create(participantAddress);
		participantInfo->setRole(participantRole);
		info->addParticipant(participantInfo);
		update = true;
	} else {
		auto participantInfo = (*participantInfoIt)->clone()->toSharedPtr();
		if (participantInfo->getRole() != participantRole) {
			participantInfo->setRole(participantRole);
			info->updateParticipant(participantInfo);
			update = true;
		}
	}
	return update;
}

void Conference::updateParticipantInConferenceInfo(const std::shared_ptr<Participant> &participant) const {
	const auto &participantAddress = participant->getAddress();
	if (!participant) {
		lError() << "Conference " << *getConferenceAddress()
		         << " received a request to update the conference info to add participant with address "
		         << *participantAddress << " but it looks like he/she is not part of this conference";
		return;
	}

#ifdef HAVE_DB_STORAGE
	if ((getState() == ConferenceInterface::State::CreationPending) ||
	    (getState() == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();
		if (info) {
			bool update = updateParticipantInfoInConferenceInfo(info, participant);

			// Store into DB after the start incoming notification in order to have a valid conference address being
			// the contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb && update) {
				lInfo() << "Updating conference information of conference " << *getConferenceAddress()
				        << " because participant " << *participantAddress
				        << " has been added or has modified its informations";
				mainDb->insertConferenceInfo(info);
			}
		}
	}
#endif // HAVE_DB_STORAGE
}

bool Conference::updateMinatureRequestedFlag() const {
	auto oldMinaturesRequested = thumbnailsRequested;
	int thumbnailAvailableCount = 0;
	for (const auto &p : participants) {
		for (const auto &d : p->getDevices()) {
			auto dir = d->getThumbnailStreamCapability();
			if ((dir == LinphoneMediaDirectionSendOnly) || (dir == LinphoneMediaDirectionSendRecv)) {
				thumbnailAvailableCount++;
			}
		}
	}
	int max_thumbnails = linphone_core_get_conference_max_thumbnails(getCore()->getCCore());
	thumbnailsRequested = (thumbnailAvailableCount <= max_thumbnails) && sessionParamsAllowThumbnails();
	bool changed = (oldMinaturesRequested != thumbnailsRequested);
	for (const auto &p : participants) {
		for (const auto &d : p->getDevices()) {
			// Even if the request of thumbnails has not changed, it may be possible that the stream availabilities have
			// changed if one participant devices stqrts or stops sharing its screen
			auto availabilityChanges = d->updateStreamAvailabilities();
			changed |= (availabilityChanges.find(LinphoneStreamTypeVideo) != availabilityChanges.cend());
		}
	}

	return changed;
}

bool Conference::areThumbnailsRequested(bool update) const {
	if (update) {
		updateMinatureRequestedFlag();
	}
	return thumbnailsRequested;
}

std::pair<bool, LinphoneMediaDirection>
Conference::getMainStreamVideoDirection(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                        BCTBX_UNUSED(bool localIsOfferer),
                                        BCTBX_UNUSED(bool useLocalParams)) const {
	lWarning() << __func__ << " not implemented";
	return std::make_pair(false, LinphoneMediaDirectionInactive);
}

LINPHONE_END_NAMESPACE
