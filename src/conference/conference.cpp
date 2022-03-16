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
#include "conference/params/media-session-params-p.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session.h"
#include "content/content.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core.h"
#include "core/core-p.h"
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

Conference::Conference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<ConferenceParams> params
) : CoreAccessor(core) {
	this->me = Participant::create(this,myAddress);
	this->listener = listener;
	this->update(*params);
	this->confParams->setMe(myAddress);
	this->startTime = ms_time(nullptr);
}

Conference::~Conference () {
	confListeners.clear();
}

// -----------------------------------------------------------------------------

bool Conference::tryAddMeDevice() {
	if (confParams->localParticipantEnabled() && me->getDevices().empty() && confParams->getAccount()) {
		char * devAddrStr = linphone_account_get_contact_address(confParams->getAccount()) ? linphone_address_as_string(linphone_account_get_contact_address(confParams->getAccount())) : nullptr;
		if (devAddrStr) {
			Address devAddr(devAddrStr);
			ms_free(devAddrStr);
			auto meDev = me->addDevice(devAddr);
			const auto & meSession = me->getSession();

			char label[Conference::labelLength];
			belle_sip_random_token(label,sizeof(label));
			meDev->setLayout(getLayout());
			meDev->setLabel(label);
			meDev->setSession(meSession);

			// Initialize media directions
			meDev->setStreamCapability((confParams->audioEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive), LinphoneStreamTypeAudio);
			meDev->setStreamCapability((confParams->videoEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive), LinphoneStreamTypeVideo);
			meDev->setStreamCapability((confParams->chatEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive), LinphoneStreamTypeText);

			meDev->updateStreamAvailabilities();

			return true;
		}
	}

	return false;
}

time_t Conference::getStartTime() const {
	return startTime;
}

int Conference::getDuration() const {
	return (int)(ms_time(nullptr) - startTime);
}

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
		lWarning() << "Not adding participant '" << participantAddress.asString() << "' because it is already a participant of the Conference";
		return false;
	}
	participant = Participant::create(this,participantAddress);
	participant->createSession(*this, nullptr, (confParams->chatEnabled() == false), listener);
	participant->setFocus(participantAddress == getConferenceAddress());
	participant->setPreserveSession(false);
	participants.push_back(participant);
	if (!activeParticipant)
		activeParticipant = participant;
	return true;


}

const std::shared_ptr<CallSession> Conference::getMainSession() const {
	return me->getSession();
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

bool Conference::addParticipants (const std::list<std::shared_ptr<Call>> &calls) {
	list<std::shared_ptr<Call>> sortedCalls(calls);
	sortedCalls.sort();
	sortedCalls.unique();

	bool soFarSoGood = true;
	for (const auto &call : sortedCalls)
		soFarSoGood &= addParticipant(call);

	return soFarSoGood;
}

ConferenceLayout Conference::getLayout() const {
	return confParams ? confParams->getLayout() : (ConferenceLayout)linphone_core_get_default_conference_layout(getCore()->getCCore());
}

LinphoneStatus Conference::updateMainSession() {
	LinphoneStatus ret = -1;
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams * params = session->getMediaParams();
		MediaSessionParams *currentParams = params->clone();
		currentParams->getPrivate()->setInternalCallUpdate(false);
		if (!currentParams->rtpBundleEnabled()) {
			currentParams->enableRtpBundle((getLayout() != ConferenceLayout::Legacy));
		}
		ret = session->update(currentParams);
		delete currentParams;
	}
	return ret;
}

void Conference::setLayout(const ConferenceLayout layout) {
	if (getLayout() != layout) {
		lInfo() << "Changing layout of conference " << getConferenceAddress() << " from " << getLayout() << " to " << layout;
		confParams->setLayout(layout);
		updateMainSession();
	}
}

const ConferenceAddress & Conference::getConferenceAddress () const {
	return confParams->getConferenceAddress();
}

void Conference::setConferenceAddress (const ConferenceAddress &conferenceAddress) {
	confParams->setConferenceAddress(conferenceAddress);
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

const list<shared_ptr<ParticipantDevice>> Conference::getParticipantDevices () const {
	list<shared_ptr<ParticipantDevice>> devices;
	for (const auto & p : participants) {
		const auto & d = p->getDevices();
		if (!d.empty()) {
			devices.insert(devices.begin(), d.begin(), d.end());
		}
	}
	if (isIn()) {
		const auto & d = getMe()->getDevices();
		if (!d.empty()) {
			devices.insert(devices.begin(), d.begin(), d.end());
		}
	}

	return devices;
}

const string &Conference::getSubject () const {
	return confParams->getSubject();
}

const string &Conference::getUsername () const {
	return mUsername;
}

void Conference::join (const IdentityAddress &participantAddress) {}

void Conference::join () {}

void Conference::leave () {}

void Conference::setLocalParticipantStreamCapability(const LinphoneMediaDirection & direction, const LinphoneStreamType type) {}

bool Conference::update(const ConferenceParamsInterface &newParameters) {
	const LinphonePrivate::ConferenceParams &newConfParams = static_cast<const ConferenceParams&>(newParameters);
	if (confParams && ((confParams->getConferenceFactoryAddress() != newConfParams.getConferenceFactoryAddress()) || (confParams->getConferenceAddress() != newConfParams.getConferenceAddress()))) {
		lError() << "Trying to change frozen conference parameters:";
		lError() << " -  factory address: actual " << confParams->getConferenceFactoryAddress() << " new value " << newConfParams.getConferenceFactoryAddress();
		lError() << " -  conference address: actual " << confParams->getConferenceAddress() << " new value " << newConfParams.getConferenceAddress();
		return false;
	}
	confParams = ConferenceParams::create(newConfParams);

	for (const auto &device : getMe()->getDevices()) {
		device->setLayout(getLayout());
	}
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
#ifdef HAVE_DB_STORAGE
	updateSubjectInConferenceInfo(subject);
#endif // HAVE_DB_STORAGE
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceLeft (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceStatusChanged,
		creationTime,
		conferenceId,
		participant->getAddress(),
		participantDevice->getAddress(),
		participantDevice->getName()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceLeft(event, participantDevice);
	}
	if (participantDevice) {
		_linphone_participant_device_notify_conference_left(participantDevice->toC());
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceJoined (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceStatusChanged,
		creationTime,
		conferenceId,
		participant->getAddress(),
		participantDevice->getAddress(),
		participantDevice->getName()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceJoined(event, participantDevice);
	}
	if (participantDevice) {
		_linphone_participant_device_notify_conference_joined(participantDevice->toC());
	}
	return event;
}

void Conference::notifySpeakingDevice (uint32_t ssrc, bool isSpeaking) {
	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSsrc() == ssrc) {
				_linphone_participant_device_notify_is_speaking_changed(device->toC(), isSpeaking);
				for (const auto &l : confListeners) {
					l->onParticipantDeviceIsSpeakingChanged(device, isSpeaking);
				}
				return;
			}
		}
	}
	for (const auto &device : getMe()->getDevices()) {
		if (device->getSsrc() == ssrc) {
			_linphone_participant_device_notify_is_speaking_changed(device->toC(), isSpeaking);
			for (const auto &l : confListeners) {
				l->onParticipantDeviceIsSpeakingChanged(device, isSpeaking);
			}
			return;
		}
	}
	lDebug() << "IsSpeaking: unable to notify speaking device because there is no device found.";
}

void Conference::notifyMutedDevice (uint32_t ssrc, bool muted) {
	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSsrc() == ssrc) {
				_linphone_participant_device_notify_is_muted(device->toC(), muted);
				for (const auto &l : confListeners) {
					l->onParticipantDeviceIsMuted(device, muted);
				}
				return;
			}
		}
	}
	for (const auto &device : getMe()->getDevices()) {
		if (device->getSsrc() == ssrc) {
			_linphone_participant_device_notify_is_muted(device->toC(), muted);
			for (const auto &l : confListeners) {
				l->onParticipantDeviceIsMuted(device, muted);
			}
			return;
		}
	}
	lDebug() << "IsMuted: unable to notify muted device because there is no device found.";
}

void Conference::setUsername (const string &username) {
	mUsername = username;
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

	lWarning() << "Unable to find participant in conference " << getConferenceAddress() << " (" << this << ") with address " << addr.asString();
	return nullptr;
}

shared_ptr<Participant> Conference::findParticipant (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		if (participant->getSession() == session)
			return participant;
	}
	lWarning() << "Unable to find participant in conference " << getConferenceAddress() << " (" << this << ") with call session " << session;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDeviceByLabel (const std::string &label) const {
	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getLabel() == label)
				return device;
		}
	}

	lDebug() << "Unable to find participant device in conference " << getConferenceAddress() << " with label " << label;

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice (const IdentityAddress &addr) const {

	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getAddress() == addr)
				return device;
		}
	}

	lDebug() << "Unable to find participant device in conference " << getConferenceAddress() << " with address " << addr.asString();

	return nullptr;
}

shared_ptr<ParticipantDevice> Conference::findParticipantDevice (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSession() == session)
				return device;
		}
	}

	lDebug() << "Unable to find participant device in conference " << getConferenceAddress() << " with call session " << session;

	return nullptr;
}

std::map<ConferenceMediaCapabilities, bool> Conference::getMediaCapabilities() const {
	std::map<ConferenceMediaCapabilities, bool> mediaCapabilities;
	mediaCapabilities[ConferenceMediaCapabilities::Audio] = confParams->audioEnabled();
	mediaCapabilities[ConferenceMediaCapabilities::Video] = confParams->videoEnabled();
	mediaCapabilities[ConferenceMediaCapabilities::Text] = confParams->chatEnabled();
	return mediaCapabilities;
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

string Conference::getResourceLists (const list<IdentityAddress> &addresses) {
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

shared_ptr<ConferenceAvailableMediaEvent> Conference::notifyAvailableMediaChanged (time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	shared_ptr<ConferenceAvailableMediaEvent> event = make_shared<ConferenceAvailableMediaEvent>(
		creationTime,
		conferenceId,
		mediaCapabilities
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onAvailableMediaChanged(event);
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

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceMediaCapabilityChanged (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged,
		creationTime,
		conferenceId,
		participant->getAddress(),
		participantDevice->getAddress(),
		participantDevice->getName()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceMediaCapabilityChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> Conference::notifyParticipantDeviceMediaAvailabilityChanged (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged,
		creationTime,
		conferenceId,
		participant->getAddress(),
		participantDevice->getAddress(),
		participantDevice->getName()
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onParticipantDeviceMediaAvailabilityChanged(event, participantDevice);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent> Conference::notifyEphemeralModeChanged (time_t creationTime,  const bool isFullState, const EventLog::Type type) {
	L_ASSERT((type == EventLog::Type::ConferenceEphemeralMessageManagedByAdmin) || (type == EventLog::Type::ConferenceEphemeralMessageManagedByParticipants));
	shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
		type,
		creationTime,
		conferenceId,
		0
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onEphemeralModeChanged(event);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent> Conference::notifyEphemeralMessageEnabled (time_t creationTime,  const bool isFullState, const bool enable) {

	shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
		(enable) ? EventLog::Type::ConferenceEphemeralMessageEnabled : EventLog::Type::ConferenceEphemeralMessageDisabled,
		creationTime,
		getConferenceId(),
		0
	);

	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onEphemeralMessageEnabled(event);
	}
	return event;
}

shared_ptr<ConferenceEphemeralMessageEvent> Conference::notifyEphemeralLifetimeChanged (time_t creationTime,  const bool isFullState, const long lifetime) {
	shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
		EventLog::Type::ConferenceEphemeralMessageLifetimeChanged,
		creationTime,
		conferenceId,
		lifetime
	);
	event->setFullState(isFullState);
	event->setNotifyId(lastNotify);

	for (const auto &l : confListeners) {
		l->onEphemeralLifetimeChanged(event);
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

#ifdef HAVE_DB_STORAGE
std::shared_ptr<ConferenceInfo> Conference::createOrGetConferenceInfo() const {
	return nullptr;
}

std::shared_ptr<ConferenceInfo> Conference::createConferenceInfo(const IdentityAddress & organizer, const std::list<IdentityAddress> invitedParticipants) const {
	std::shared_ptr<ConferenceInfo> info = ConferenceInfo::create();
	info->setOrganizer(organizer);
	info->setParticipants(invitedParticipants);

	const auto & conferenceAddress = getConferenceAddress();
	if (conferenceAddress.isValid()) {
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

	return info;
}

void Conference::updateSubjectInConferenceInfo(const std::string & subject) const {
	if ((getState() == ConferenceInterface::State::CreationPending) || (getState() == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			info->setSubject(subject);

			// Store into DB after the start incoming notification in order to have a valid conference address being the contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo() << "Updating conference information of conference " << getConferenceAddress() << " because its subject has been changed to " << subject;
				mainDb->insertConferenceInfo(info);
			}
		}
	}
}

void Conference::updateParticipantsInConferenceInfo(const IdentityAddress & participantAddress) const {
	if ((getState() == ConferenceInterface::State::CreationPending) || (getState() == ConferenceInterface::State::Created)) {
		auto info = createOrGetConferenceInfo();

		if (info) {
			std::list<IdentityAddress> currentParticipants = info->getParticipants();
			const auto participantAddressIt = std::find(currentParticipants.begin(), currentParticipants.end(), participantAddress);
			if (participantAddressIt == currentParticipants.end()) {
				currentParticipants.push_back(participantAddress);
				info->setParticipants(currentParticipants);


				// Store into DB after the start incoming notification in order to have a valid conference address being the contact address of the call
				auto &mainDb = getCore()->getPrivate()->mainDb;
				if (mainDb) {
					lInfo() << "Updating conference information of conference " << getConferenceAddress() << " because participant " << participantAddress << " has been added";
					mainDb->insertConferenceInfo(info);
				}
			}
		}
	}
}
#endif // HAVE_DB_STORAGE

LINPHONE_END_NAMESPACE
