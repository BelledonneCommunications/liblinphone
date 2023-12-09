/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "conference.h"
#include "call/call.h"
#include "conference/notify-conference-listener.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "core/core.h"
#include "factory/factory.h"
#include "linphone/api/c-call.h"
#include "linphone/core.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference {

Conference::Conference(const shared_ptr<Core> &core,
                       const std::shared_ptr<Address> &myAddress,
                       CallSessionListener *listener,
                       const std::shared_ptr<LinphonePrivate::ConferenceParams> params)
    : LinphonePrivate::Conference(core, myAddress, listener, params) {

	addListener(std::make_shared<NotifyConferenceListener>(this));

	// Video is already enabled in the conference params constructor based on core settings
	// Audio must be always enabled
	confParams->enableAudio(true);

	if (confParams->getStartTime() < 0) {
		confParams->setStartTime(ms_time(NULL));
	}

	auto startTime = confParams->getStartTime();
	auto endTime = confParams->getEndTime();
	auto duration = (endTime >= 0) ? (endTime - startTime) : startTime;

	if (duration < 0) {
		lError() << "Unable to create conference due to an invalid time settings";
		lError() << "Start time (" << startTime << "): " << ctime(&startTime);
		lError() << "End time (" << endTime << "): " << ctime(&endTime);
		lError() << "Duration: " << duration << " seconds";
		setState(ConferenceInterface::State::CreationFailed);
	}

	mCoreCbs = nullptr;
	mCoreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(mCoreCbs, callStateChanged);
	linphone_core_cbs_set_transfer_state_changed(mCoreCbs, transferStateChanged);
	linphone_core_cbs_set_user_data(mCoreCbs, this);
	_linphone_core_add_callbacks(getCore()->getCCore(), mCoreCbs, TRUE);
}

Conference::~Conference() {
	linphone_core_remove_callbacks(getCore()->getCCore(), mCoreCbs);
	linphone_core_cbs_unref(mCoreCbs);
}

void Conference::setInputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (audioDevice) {
		const auto &currentInputDevice = getInputAudioDevice();
		// If pointer toward the new device has changed or at least one member of the audio device changed or no current
		// audio device is set, then return true
		bool change =
		    currentInputDevice ? ((audioDevice != currentInputDevice) || (*audioDevice != *currentInputDevice)) : true;

		if (!change) {
			lInfo() << "Ignoring request to change input audio device of conference " << *getConferenceAddress()
			        << " to [" << audioDevice->toString() << "] (" << audioDevice
			        << ") because it is the same as the one currently used";
			return;
		}
		if (audioDevice &&
		    ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) != 0)) {
			AudioControlInterface *aci = getAudioControlInterface();
			if (aci) {
				lInfo() << "Set input audio device [" << audioDevice->toString() << "] (" << audioDevice
				        << ") to audio control interface " << aci << " for conference " << *getConferenceAddress();
				aci->setInputDevice(audioDevice);
				linphone_conference_notify_audio_device_changed(toC(), audioDevice->toC());
			} else {
				lError() << "Unable to set input audio device [" << audioDevice->toString() << "] (" << audioDevice
				         << ") of conference " << *getConferenceAddress() << " because audio control interface is NULL";
			}
		} else {
			lError() << "Unable to set input audio device to [" << audioDevice->toString() << "] (" << audioDevice
			         << ") for conference " << *getConferenceAddress() << " due to missing record capability";
		}
	} else {
		lError() << "Unable to set undefined input audio device (" << audioDevice << ") for conference "
		         << *getConferenceAddress();
	}
}

void Conference::setOutputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (audioDevice) {
		const auto &currentOutputDevice = getOutputAudioDevice();
		// If pointer toward the new device has changed or at least one member of the audio device changed or no current
		// audio device is set, then return true
		bool change = currentOutputDevice
		                  ? ((audioDevice != currentOutputDevice) || (*audioDevice != *currentOutputDevice))
		                  : true;

		if (!change) {
			lInfo() << "Ignoring request to change output audio device of conference " << *getConferenceAddress()
			        << " to [" << audioDevice->toString() << "] (" << audioDevice
			        << ") because it is the same as the one currently used";
			return;
		}
		if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) != 0) {
			AudioControlInterface *aci = getAudioControlInterface();
			if (aci) {
				lInfo() << "Set output audio device [" << audioDevice->toString() << "] (" << audioDevice
				        << ") to audio control interface " << aci << " for conference " << *getConferenceAddress();
				aci->setOutputDevice(audioDevice);
				linphone_conference_notify_audio_device_changed(toC(), audioDevice->toC());
			} else {
				lError() << "Unable to set output audio device [" << audioDevice->toString() << "] (" << audioDevice
				         << ") of conference " << *getConferenceAddress() << " because audio control interface is NULL";
			}
		} else {
			lError() << "Unable to set output audio device to [" << audioDevice->toString() << "] (" << audioDevice
			         << ") for conference " << *getConferenceAddress() << " due to missing play capability";
		}
	} else {
		lError() << "Unable to set undefined output audio device (" << audioDevice << ") for conference "
		         << *getConferenceAddress();
	}
}

shared_ptr<AudioDevice> Conference::getInputAudioDevice() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getInputDevice();
	}

	lError() << "Unable to retrieve input audio device from undefined audio control interface of conference "
	         << *getConferenceAddress();
	return nullptr;
}

shared_ptr<AudioDevice> Conference::getOutputAudioDevice() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getOutputDevice();
	}

	lError() << "Unable to retrieve output audio device from undefined audio control interface of conference "
	         << *getConferenceAddress();
	return nullptr;
}

int Conference::stopRecording() {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		aci->stopRecording();
	} else {
		lError() << "LocalConference::stopRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

bool Conference::isRecording() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->isRecording();
	}
	return false;
}

bool Conference::getMicrophoneMuted() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return !aci->micEnabled();
	}
	lError() << "Unable to get status of microphone because the audio control interface of conference "
	         << *getConferenceAddress() << " cannot be found";
	return false;
}

void Conference::setMicrophoneMuted(bool muted) {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		aci->enableMic(!muted);
		for (const auto &participant : participants) {
			for (const auto &device : participant->getDevices()) {
				// If the core is holding a conference (conference server or client holding the conference because it
				// has scheduled a conference without having a conference server set), every participant device has a
				// media session associated to. In such a scenario all calls are muted one by one.
				auto deviceSession = device->getSession();
				if (deviceSession) {
					auto op = deviceSession->getPrivate()->getOp();
					shared_ptr<Call> call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
					if (call) {
						call->setMicrophoneMuted(muted);
					}
				}
			}
		}
		bool coreMicrophoneEnabled = !!linphone_core_mic_enabled(getCore()->getCCore());
		notifyLocalMutedDevices(muted || !coreMicrophoneEnabled);
	} else {
		const auto conferenceAddressStr =
		    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));
		lError() << "Unable to " << std::string(muted ? "disable" : "enable")
		         << " microphone because the audio control interface of conference " << conferenceAddressStr
		         << " cannot be found";
	}
}

float Conference::getRecordVolume() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getRecordVolume();
	}
	lError() << "Unable to get record volume because the audio control interface of conference "
	         << *getConferenceAddress() << " cannot be found";
	return 0.0;
}

void Conference::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	if ((getState() == ConferenceInterface::State::Instantiated) ||
	    (getState() == ConferenceInterface::State::CreationPending)) {
		if (!conferenceAddress || !conferenceAddress->isValid()) {
			lError() << "Cannot set the conference address to " << conferenceAddress;
			shared_ptr<CallSession> session = getMe()->getSession();
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, "SIP", LinphoneReasonUnknown, 500, "Server internal error", NULL);
			session->decline(ei);
			linphone_error_info_unref(ei);
			setState(ConferenceInterface::State::CreationFailed);
			return;
		}

		LinphonePrivate::Conference::setConferenceAddress(conferenceAddress);
		setState(ConferenceInterface::State::CreationPending);
		lInfo() << "Conference " << this << " has been given the address " << conferenceAddress;
	} else {
		lDebug() << "Cannot set the conference address of the Conference in state " << getState() << " to "
		         << conferenceAddress;
		return;
	}
}

void Conference::setConferenceId(const ConferenceId &conferenceId) {
	LinphonePrivate::Conference::setConferenceId(conferenceId);
	getCore()->insertAudioVideoConference(getSharedFromThis());
}

bool Conference::isConferenceEnded() const {
	const auto &endTime = confParams->getEndTime();
	const auto now = time(NULL);
	const auto conferenceEnded = (endTime >= 0) && (endTime < now);
	return conferenceEnded;
}

bool Conference::isConferenceStarted() const {
	const auto &startTime = confParams->getStartTime();
	const auto now = time(NULL);
	// negative start time means immediate start
	const auto conferenceStarted = (startTime < 0) || (startTime <= now);
	return conferenceStarted;
}

void Conference::fillParticipantAttributes(std::shared_ptr<Participant> &p) {
	const auto &pAddress = p->getAddress();
	const auto participantInfo =
	    std::find_if(mInvitedParticipants.cbegin(), mInvitedParticipants.cend(),
	                 [&pAddress](const auto &info) { return pAddress->weakEqual(*info->getAddress()); });
	const auto conferenceAddressStr =
	    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));

	if (participantInfo == mInvitedParticipants.cend()) {
		if (mInvitedParticipants.empty()) {
			// It is a conference created on the fly, therefore all participants are speakers
			p->setRole(Participant::Role::Speaker);
			lInfo() << "Conference " << this << " (address " << conferenceAddressStr
			        << ") has been created on the fly, either by inviting addresses or by merging existing calls "
			           "therefoe participant "
			        << *pAddress << " is given the role of " << p->getRole();
		} else {
			const bool isThereAListener =
			    std::find_if(mInvitedParticipants.cbegin(), mInvitedParticipants.cend(), [](const auto &info) {
				    return (info->getRole() == Participant::Role::Listener);
			    }) != mInvitedParticipants.cend();

			Participant::Role role = Participant::Role::Unknown;
			if (isThereAListener) {
				role = Participant::Role::Listener;
			} else {
				role = Participant::Role::Speaker;
			}
			p->setRole(role);

			lInfo() << "Unable to find participant " << *pAddress
			        << " in the list of invited participants. Assuming its role to be " << p->getRole()
			        << " in conference " << this << " (address " << conferenceAddressStr << ")";
		}
	} else {
		const auto &role = (*participantInfo)->getRole();
		if (role == Participant::Role::Unknown) {
			p->setRole(Participant::Role::Speaker);
			lInfo() << "No role was given to participant " << *pAddress << " when the conference " << this
			        << " (address " << conferenceAddressStr << ") was created. Assuming its role to be "
			        << p->getRole();
		} else {
			p->setRole(role);
		}
	}
}

bool Conference::addParticipant(BCTBX_UNUSED(const std::shared_ptr<ParticipantInfo> &info)) {
	lError() << "Conference class does not handle addParticipant() generically";
	return false;
}

bool Conference::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	bool success = LinphonePrivate::Conference::addParticipant(participantAddress);

	const auto conferenceAddressStr =
	    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));
	if (success == true) {
		lInfo() << "Participant with address " << *participantAddress << " has been added to conference "
		        << conferenceAddressStr;
		time_t creationTime = time(nullptr);
		std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(participantAddress);
		fillParticipantAttributes(p);
		notifyParticipantAdded(creationTime, false, p);
	} else {
		lError() << "Unable to add participant with address " << *participantAddress << " to conference "
		         << conferenceAddressStr;
	}

	return success;
}

bool Conference::addParticipant(std::shared_ptr<LinphonePrivate::Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	bool success = false;
	// Add a new participant only if it is not in the conference
	if (p == nullptr) {
		auto session = call->getActiveSession();
		p = Participant::create(this, remoteAddress);
		fillParticipantAttributes(p);
		p->setFocus(false);
		std::shared_ptr<Address> toAddr;
		if (session) {
			auto op = session->getPrivate()->getOp();
			if (op) {
				toAddr = Address::create(op->getTo());
			}
		}
		if (toAddr && toAddr->isValid()) {
			p->setPreserveSession(!toAddr->hasUriParam("conf-id"));
		} else {
			p->setPreserveSession(true);
		}

		// Pass admin information on if it is available in the contact address
		std::shared_ptr<Address> remoteContactAddress = Address::create(call->getRemoteContact());

		if (remoteContactAddress->hasParam("admin")) {
			bool value = Utils::stob(remoteContactAddress->getParamValue("admin"));
			p->setAdmin(value);
		}
		participants.push_back(p);

		time_t creationTime = time(nullptr);
		notifyParticipantAdded(creationTime, false, p);
		success = true;
	} else {
		lWarning() << "Participant with address " << call->getRemoteAddress()->toString()
		           << " is already part of conference " << *getConferenceAddress();
		success = false;
	}

	addParticipantDevice(call);

	return success;
}

bool Conference::addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	auto p = findParticipant(remoteAddress);
	if (p) {
		const auto &session = call->getActiveSession();
		// If device is not found, then add it
		if (p->findDevice(session, false) == nullptr) {
			shared_ptr<ParticipantDevice> device = p->addDevice(session);
			// If there is already a call for this participant, then he/she is joining the conference
			device->setState(ParticipantDevice::State::Joining);
			lInfo() << "Participant with address " << call->getRemoteAddress()->toString()
			        << " has added device with session " << session << " (address " << *device->getAddress()
			        << ") to conference " << *getConferenceAddress();
			return true;
		} else {
			lDebug() << "Participant with address " << call->getRemoteAddress()->toString() << " to conference "
			         << *getConferenceAddress() << " has already a device with session " << session;
		}
	}

	return false;
}

int Conference::removeParticipantDevice(const std::shared_ptr<LinphonePrivate::CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		// If device is not found, then add it
		if (device != nullptr) {
			device->setState(ParticipantDevice::State::ScheduledForLeaving);
			shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
			if (ev) {
				// try to terminate subscription if any, but do not wait for answer.
				ev->clearCallbacksList();
				ev->terminate();
			}

			const auto ei = session->getErrorInfo();
			device->setDisconnectionData(static_pointer_cast<LinphonePrivate::MediaSession>(session)->isTerminator(),
			                             linphone_error_info_get_protocol_code(ei), linphone_error_info_get_reason(ei));
			device->setState(ParticipantDevice::State::Left);

			time_t creationTime = time(nullptr);
			notifyParticipantDeviceRemoved(creationTime, false, p, device);

			lInfo() << "Removing device with session " << session << " from participant " << *p->getAddress()
			        << " in conference " << *getConferenceAddress();
			p->removeDevice(session);

			auto op = session->getPrivate()->getOp();
			shared_ptr<Call> call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
			if (call) {
				call->setConference(nullptr);
			}
			return 0;
		}
	}

	return -1;
}

int Conference::removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) {
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	if (!p) {
		lDebug() << "Unable to participant with address " << remoteAddress;
		return -1;
	}
	return removeParticipant(p);
}

int Conference::removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> &session,
                                  BCTBX_UNUSED(const bool preserveSession)) {
	const std::shared_ptr<Address> &pAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(pAddress);
	removeParticipantDevice(session);
	if (!p) {
		lInfo() << "Participant removal failed: Participant with address " << *pAddress
		        << " has not been found in conference " << *getConferenceAddress();
		return -1;
	}
	if (p->getDevices().empty()) {
		lInfo() << "Remove participant with address " << *pAddress << " from conference " << *getConferenceAddress();
		participants.remove(p);
		time_t creationTime = time(nullptr);
		notifyParticipantRemoved(creationTime, false, p);
		return 0;
	}
	return -1;
}

int Conference::removeParticipant(const std::shared_ptr<Address> &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	return removeParticipant(p);
}

bool Conference::removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	if (!participant) return false;
	lInfo() << "Removing participant with address " << *participant->getAddress() << " from conference "
	        << *getConferenceAddress();
	// Delete all devices of a participant
	auto deviceIt = participant->getDevices().begin();
	while (deviceIt != participant->getDevices().end()) {

		auto device = (*deviceIt);
		shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
		if (ev) {
			// try to terminate subscription if any, but do not wait for answer.
			ev->clearCallbacksList();
			ev->terminate();
		}

		auto session = device->getSession();
		deviceIt++;
		participant->removeDevice(session);

		time_t creationTime = time(nullptr);
		notifyParticipantDeviceRemoved(creationTime, false, participant, device);
	}

	participant->clearDevices();
	participants.remove(participant);
	time_t creationTime = time(nullptr);
	notifyParticipantRemoved(creationTime, false, participant);

	auto pSession = participant->getSession();

	// Detach call from conference
	shared_ptr<Call> pSessionCall = nullptr;
	if (pSession) {
		auto op = pSession->getPrivate()->getOp();
		pSessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
		if (pSessionCall) {
			pSessionCall->setConference(nullptr);
		}
	}

	return true;
}

int Conference::terminate() {
	participants.clear();
	return 0;
}

void Conference::setState(LinphonePrivate::ConferenceInterface::State state) {
	LinphonePrivate::ConferenceInterface::State previousState = getState();
	// Change state if:
	// - current state is not Deleted
	// - current state is Deleted and trying to move to Instantiated state
	if ((previousState != ConferenceInterface::State::Deleted) ||
	    ((previousState == ConferenceInterface::State::Deleted) &&
	     (state == ConferenceInterface::State::Instantiated))) {
		shared_ptr<Conference> ref = getSharedFromThis();
		LinphonePrivate::Conference::setState(state);
		if (previousState != state) {
			if (mStateChangedCb) {
				mStateChangedCb(toC(), (LinphoneConferenceState)state, mCbUserData);
			}
		}
	}

	if (state == ConferenceInterface::State::Terminated) {
		onConferenceTerminated(getConferenceAddress());
	}
}

void Conference::notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	// Call listeners
	LinphonePrivate::Conference::notifyStateChanged(state);
}

void Conference::onConferenceTerminated(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	// Keep a reference to the conference to be able to set the state to Deleted
	shared_ptr<Conference> ref = getSharedFromThis();
	// If core is in Global Shutdown state, then do not remove it from the map as it will be freed by Core::uninit()
	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalShutdown) {
		getCore()->deleteAudioVideoConference(ref);
	}
	setState(ConferenceInterface::State::Deleted);
}

void Conference::setParticipantAdminStatus(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant), BCTBX_UNUSED(bool isAdmin)) {
}

void Conference::join(BCTBX_UNUSED(const std::shared_ptr<Address> &participantAddress)) {
}

void Conference::join() {
}

bool Conference::removeParticipants(const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) {
	bool soFarSoGood = true;
	for (const auto &p : participants)
		soFarSoGood &= removeParticipant(p);
	return soFarSoGood;
}

void Conference::callStateChanged(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	Conference *conf = static_cast<Conference *>(linphone_core_v_table_get_user_data(vtable));

	if (conf) {
		conf->callStateChangedCb(lc, call, cstate, message);
	}
}

void Conference::transferStateChanged(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	Conference *conf = static_cast<Conference *>(linphone_core_v_table_get_user_data(vtable));

	if (conf) {
		conf->transferStateChangedCb(lc, transfered, new_call_state);
	}
}

std::list<std::shared_ptr<Address>> Conference::getInvitedAddresses() const {
	list<std::shared_ptr<Address>> addresses;
	for (auto &participant : mInvitedParticipants) {
		addresses.push_back(participant->getAddress());
	}
	return addresses;
}

ConferenceInfo::participant_list_t Conference::getFullParticipantList() const {
	auto participantList = mInvitedParticipants;
	// Add participants that are not part of the invitees'list
	for (const auto &p : getParticipants()) {
		const auto &pAddress = p->getAddress();
		auto pIt = std::find_if(participantList.begin(), participantList.end(), [&pAddress](const auto &participant) {
			return (pAddress->weakEqual(*participant->getAddress()));
		});
		if (pIt == participantList.end()) {
			auto participantInfo = Factory::get()->createParticipantInfo(pAddress);
			participantInfo->setRole(p->getRole());
			participantList.push_back(participantInfo);
		}
	}
	if (isIn()) {
		auto me = getMe();
		auto participantInfo = Factory::get()->createParticipantInfo(me->getAddress());
		participantInfo->setRole(me->getRole());
		participantList.push_back(participantInfo);
	}
	return participantList;
}

std::shared_ptr<Player> Conference::getPlayer() const {
	lWarning() << "Getting a player is not available for this conference.";
	return nullptr;
}

} // end of namespace MediaConference

LINPHONE_END_NAMESPACE
