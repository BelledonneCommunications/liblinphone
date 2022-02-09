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
#include <list>
#include <string>
#include <typeinfo>

#include <bctoolbox/defs.h>
#include <mediastreamer2/msvolume.h>

#include "linphone/core.h"

#include "conference_private.h"
#include "linphone/api/c-conference.h"
#include "linphone/wrapper_utils.h"

#include "sal/refer-op.h"
#include "account/account.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call.h"
#include "call/call-log.h"
#include "core/core-p.h"
#include "conference/params/media-session-params-p.h"
#include "conference/notify-conference-listener.h"
#include "conference/conference.h"
#include "conference/participant.h"
#include "conference/session/mixers.h"
#include "conference/session/ms2-streams.h"
#include "conference/session/media-session.h"
#include "conference/session/media-session-p.h"
#ifdef HAVE_ADVANCED_IM
#include "conference/handlers/local-audio-video-conference-event-handler.h"
#include "conference/handlers/local-conference-event-handler.h"
#include "conference/handlers/remote-conference-event-handler.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#endif // HAVE_ADVANCED_IM

// TODO: From coreapi. Remove me later.
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference{

Conference::Conference(
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	LinphonePrivate::Conference(core, myAddress, listener, params)
	{

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

	m_coreCbs = nullptr;
	m_coreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(m_coreCbs, callStateChanged);
	linphone_core_cbs_set_transfer_state_changed(m_coreCbs, transferStateChanged);
	linphone_core_cbs_set_user_data(m_coreCbs, this);
	_linphone_core_add_callbacks(getCore()->getCCore(), m_coreCbs, TRUE);
}

Conference::~Conference() {
	linphone_core_remove_callbacks(getCore()->getCCore(), m_coreCbs);
	linphone_core_cbs_unref(m_coreCbs);
}

void Conference::setInputAudioDevice(AudioDevice *audioDevice) {
	if (audioDevice) {
		const auto & currentInputDevice = getInputAudioDevice();
		// If pointer toward the new device has changed or at least one member of the audio device changed or no current audio device is set, then return true
		bool change = currentInputDevice ? ((audioDevice != currentInputDevice) || (*audioDevice != *currentInputDevice)) : true;

		if (!change) {
			lInfo() << "Ignoring request to change input audio device of conference " << getConferenceAddress() << " to [" << audioDevice->toString() << "] (" << audioDevice << ") because it is the same as the one currently used";
			return;
		}
		if (audioDevice && ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) != 0)) {
			AudioControlInterface *aci =getAudioControlInterface();
			if (aci) {
				lInfo() << "Set input audio device [" << audioDevice->toString() << "] (" << audioDevice << ") to audio control interface " << aci << " for conference " << getConferenceAddress();
				aci->setInputDevice(audioDevice);
				linphone_conference_notify_audio_device_changed(toC(), audioDevice->toC());
			} else {
				lError() << "Unable to set input audio device [" << audioDevice->toString() << "] (" << audioDevice << ") of conference " << getConferenceAddress() << " because audio control interface is NULL";
			}
		} else {
			lError() << "Unable to set input audio device to [" << audioDevice->toString() << "] (" << audioDevice << ") for conference " << getConferenceAddress() << " due to missing record capability";
		}
	} else {
		lError() << "Unable to set undefined input audio device (" << audioDevice << ") for conference " << getConferenceAddress();
	}
}

void Conference::setOutputAudioDevice(AudioDevice *audioDevice) {
	if (audioDevice) {
		const auto & currentOutputDevice = getOutputAudioDevice();
		// If pointer toward the new device has changed or at least one member of the audio device changed or no current audio device is set, then return true
		bool change = currentOutputDevice ? ((audioDevice != currentOutputDevice) || (*audioDevice != *currentOutputDevice)) : true;

		if (!change) {
			lInfo() << "Ignoring request to change output audio device of conference " << getConferenceAddress() << " to [" << audioDevice->toString() << "] (" << audioDevice << ") because it is the same as the one currently used";
			return;
		}
		if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) != 0) {
			AudioControlInterface *aci =getAudioControlInterface();
			if (aci) {
				lInfo() << "Set output audio device [" << audioDevice->toString() << "] (" << audioDevice << ") to audio control interface " << aci << " for conference " << getConferenceAddress();
				aci->setOutputDevice(audioDevice);
				linphone_conference_notify_audio_device_changed(toC(), audioDevice->toC());
			} else {
				lError() << "Unable to set output audio device [" << audioDevice->toString() << "] (" << audioDevice << ") of conference " << getConferenceAddress() << " because audio control interface is NULL";
			}
		} else {
			lError() << "Unable to set output audio device to [" << audioDevice->toString() << "] (" << audioDevice << ") for conference " << getConferenceAddress() << " due to missing play capability";
		}
	} else {
		lError() << "Unable to set undefined output audio device (" << audioDevice << ") for conference " << getConferenceAddress();
	}
}

AudioDevice* Conference::getInputAudioDevice() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getInputDevice();
	}

	lError() << "Unable to retrieve input audio device from undefined audio control interface of conference " << getConferenceAddress();
	return nullptr;
}

AudioDevice* Conference::getOutputAudioDevice() const {
	AudioControlInterface *aci = getAudioControlInterface();
	if (aci) {
		return aci->getOutputDevice();
	}

	lError() << "Unable to retrieve output audio device from undefined audio control interface of conference " << getConferenceAddress();
	return nullptr;
}

void Conference::setConferenceAddress (const ConferenceAddress &conferenceAddress) {
	if ((getState() == ConferenceInterface::State::Instantiated) || (getState() == ConferenceInterface::State::CreationPending)) {
		if (!conferenceAddress.isValid()) {
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
		lInfo() << "Conference " << this << " has been given the address " << conferenceAddress.asString();
	} else {
		lDebug() << "Cannot set the conference address of the Conference in state " << getState() << " to " << conferenceAddress;
		return;
	}

}

void Conference::setConferenceId (const ConferenceId &conferenceId) {
	LinphonePrivate::Conference::setConferenceId(conferenceId);
	getCore()->insertAudioVideoConference(getSharedFromThis());
}

bool Conference::isConferenceEnded() const {
	const auto & endTime = confParams->getEndTime();
	const auto now = time(NULL);
	const auto conferenceEnded = (endTime >= 0) && (endTime < now);
	return conferenceEnded;
}

bool Conference::isConferenceStarted() const {
	const auto & startTime = confParams->getStartTime();
	const auto now = time(NULL);
	// negative start time means immediate start
	const auto conferenceStarted = (startTime < 0) || (startTime <= now);
	return conferenceStarted;
}

bool Conference::addParticipant (const IdentityAddress &participantAddress) {
	bool success = LinphonePrivate::Conference::addParticipant(participantAddress);

	if (success == true) {
		lInfo() << "Participant with address " << participantAddress << " has been added to conference " << getConferenceAddress();
		time_t creationTime = time(nullptr);
		std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(participantAddress);
		notifyParticipantAdded(creationTime, false, p);
	} else {
		lError() << "Unable to add participant with address " << participantAddress << " to conference " << getConferenceAddress();
	}

	return 0;
}

bool Conference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	const Address &remoteAddress = *call->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	bool success = false;
	// Add a new participant only if it is not in the conference
	if (p == nullptr) {
		auto session = call->getActiveSession();
		p = Participant::create(this, remoteAddress, session);
		p->setFocus(false);
		Address toAddr;
		if (session) {
			auto op = session->getPrivate()->getOp();
			if (op) {
				toAddr = Address(op->getTo());
			}
		}
		if (toAddr.isValid()) {
			p->setPreserveSession(!toAddr.hasUriParam("conf-id"));
		} else {
			p->setPreserveSession(true);
		}

		// Pass admin information on if it is available in the contact address
		Address remoteContactAddress(call->getRemoteContact());

		if (remoteContactAddress.hasParam ("admin")) {
			bool value = Utils::stob(remoteContactAddress.getParamValue("admin"));
			p->setAdmin(value);
		}
		participants.push_back(p);

		time_t creationTime = time(nullptr);
		notifyParticipantAdded(creationTime, false, p);
		success =  true;
	} else {

		lError() << "Participant with address " << call->getRemoteAddress()->asString() << " is already part of conference " << getConferenceAddress();
		success =  false;
	}

	addParticipantDevice(call);

	return success;
}

bool Conference::addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) {
	const auto & session = call->getActiveSession();
	auto p = findParticipant(session);
	if (p) {
		// If device is not found, then add it
		if (p->findDevice(session, false) == nullptr) {
			shared_ptr<ParticipantDevice> device = p->addDevice(session);
			device->updateMediaCapabilities();
			device->updateStreamAvailabilities();
			auto op = session->getPrivate()->getOp();
			if (op) {
				device->setLayout(MediaSession::computeConferenceLayout(op->getRemoteMediaDescription()));
			}
			device->setState(ParticipantDevice::State::ScheduledForJoining);
			lInfo() << "Participant with address " << call->getRemoteAddress()->asString() << " has added device with session " << session << " (address " << device->getAddress() << ") to conference " << getConferenceAddress();
			return true;
		} else {
			lDebug() << "Participant with address " << call->getRemoteAddress()->asString() << " to conference " << getConferenceAddress() << " has already a device with session " << session;
		}
	}

	return false;
}

int Conference::removeParticipantDevice(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		// If device is not found, then add it
		if (device != nullptr) {
			device->setState(ParticipantDevice::State::ScheduledForLeaving);
			LinphoneEvent * event = device->getConferenceSubscribeEvent();
			if (event) {
				//try to terminate subscription if any, but do not wait for anser.
				LinphoneEventCbs *cbs = linphone_event_get_callbacks(event);
				linphone_event_cbs_set_user_data(cbs, nullptr);
				linphone_event_cbs_set_notify_response(cbs, nullptr);
				linphone_event_terminate(event);
			}

			lInfo() << "Removing device with session " << session << " from participant " << p->getAddress() << " in conference " << getConferenceAddress();
			p->removeDevice(session);
			shared_ptr<Call> call = getCore()->getCallByRemoteAddress (*session->getRemoteAddress());
			if (call) {
				call->terminateConference();
			}
			time_t creationTime = time(nullptr);
			notifyParticipantDeviceRemoved(creationTime, false, p, device);

			return 0;
		}
	}

	return -1;
}

int Conference::removeParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(call->getActiveSession());
	if (!p)
		return -1;
	return removeParticipant(p);
}

int Conference::removeParticipant (const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession) {
	removeParticipantDevice(session);
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	if (!p)
		return -1;
	if (p->getDevices().empty()) {
		participants.remove(p);
		time_t creationTime = time(nullptr);
		notifyParticipantRemoved(creationTime, false, p);
	}

	return 0;
}

int Conference::removeParticipant (const IdentityAddress &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	return removeParticipant(p);
}

bool Conference::removeParticipant (const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	if (!participant)
		return false;
	// Delete all devices of a participant
	auto deviceIt = participant->getDevices().begin();
	while (deviceIt != participant->getDevices().end()) {

		auto device = (*deviceIt);
		LinphoneEvent * event = device->getConferenceSubscribeEvent();
		if (event) {
			//try to terminate subscription if any, but do not wait for answer.
			LinphoneEventCbs *cbs = linphone_event_get_callbacks(event);
			linphone_event_cbs_set_user_data(cbs, nullptr);
			linphone_event_cbs_set_notify_response(cbs, nullptr);
			linphone_event_terminate(event);
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

int Conference::terminate () {
	participants.clear();
	return 0;
}

void Conference::setState (LinphonePrivate::ConferenceInterface::State state) {
	LinphonePrivate::ConferenceInterface::State previousState = getState();
	// Change state if:
	// - current state is not Deleted
	// - current state is Deleted and trying to move to Instantiated state
	if ((previousState != ConferenceInterface::State::Deleted) || ((previousState == ConferenceInterface::State::Deleted) && (state == ConferenceInterface::State::Instantiated))) {
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

void Conference::notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) {
	// Call listeners
	LinphonePrivate::Conference::notifyStateChanged(state);
}

void Conference::onConferenceTerminated (const IdentityAddress &addr) {

	// Keep a reference to the conference to be able to set the state to Deleted
	shared_ptr<Conference> ref = getSharedFromThis();
	// If core is in Global Shutdown state, then do not remove it from the map as it will be freed by Core::uninit()
	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalShutdown) {
		getCore()->deleteAudioVideoConference(ref);
	}
	setState(ConferenceInterface::State::Deleted);
}

void Conference::setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) {

}

void Conference::join (const IdentityAddress &participantAddress) {
	
}

void Conference::join () {

}

bool Conference::removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) {
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

LocalConference::LocalConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, myAddress, listener, params){

	bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
#ifdef HAVE_ADVANCED_IM
	if (eventLogEnabled) {
		eventHandler = std::make_shared<LocalAudioVideoConferenceEventHandler>(this);
		addListener(eventHandler);
	}
#endif // HAVE_ADVANCED_IM

	mMixerSession.reset(new MixerSession(*core.get()));

	setState(ConferenceInterface::State::Instantiated);

	// Update proxy contact address to add conference ID
	// Do not use myAddress directly as it may lack some parameter like gruu
	LinphoneAddress * cAddress = L_GET_C_BACK_PTR(&(myAddress.asAddress()));
	LinphoneAccount * account = linphone_core_lookup_known_account(core->getCCore(), cAddress);
	char * contactAddressStr = nullptr;
	if (account && Account::toCpp(account)->getOp()) {
		contactAddressStr = sal_address_as_string(Account::toCpp(account)->getOp()->getContactAddress());
	} else {
		contactAddressStr = ms_strdup(linphone_core_find_best_identity(core->getCCore(), const_cast<LinphoneAddress *>(cAddress)));
	}
	Address contactAddress(contactAddressStr);
	char confId[LinphonePrivate::MediaConference::LocalConference::confIdLength];
	belle_sip_random_token(confId,sizeof(confId));
	contactAddress.setUriParam("conf-id",confId);
	if (contactAddressStr) {
		ms_free(contactAddressStr);
	}

	setConferenceAddress(contactAddress);
	getMe()->setAdmin(true);
	getMe()->setFocus(true);

	if (!eventLogEnabled) {
		setConferenceId(ConferenceId(contactAddress, contactAddress));
	}
}

LocalConference::LocalConference (const shared_ptr<Core> &core, SalCallOp *op) :
	Conference(core, IdentityAddress(op->getTo()), nullptr, ConferenceParams::create(core->getCCore())) {

	LinphoneCore * lc = core->getCCore();

#ifdef HAVE_ADVANCED_IM
	bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(lc), "misc", "conference_event_log_enabled", TRUE );
	if (eventLogEnabled) {
		eventHandler = std::make_shared<LocalAudioVideoConferenceEventHandler>(this);
		addListener(eventHandler);
	}
#endif // HAVE_ADVANCED_IM

	mMixerSession.reset(new MixerSession(*core.get()));

	const auto & remoteMd = op->getRemoteMediaDescription();

	const auto times = remoteMd->times;

	// The following informations are retrieved from the received INVITE:
	// - start and end time from the SDP active time attribute
	// - conference active media:
	//    - if the SDP has at least one active audio stream, audio is enabled
	//    - if the SDP has at least one active video stream, video is enabled
	// - Subject is got from the "Subject" header in the INVITE
	const auto audioEnabled = (remoteMd->nbActiveStreamsOfType(SalAudio) > 0);
	const auto videoEnabled = (linphone_core_conference_server_enabled(lc)) ? linphone_core_video_enabled(lc) : (remoteMd->nbActiveStreamsOfType(SalVideo) > 0);
	confParams->enableAudio(audioEnabled);
	confParams->enableVideo(videoEnabled);
	confParams->setSubject(op->getSubject());
	confParams->enableLocalParticipant(false);
	confParams->enableOneParticipantConference(true);
	confParams->setStatic(true);

	MediaSessionParams msp;
	msp.enableAudio(audioEnabled);
	msp.enableVideo(videoEnabled);
	msp.getPrivate()->setInConference(true);

	if (times.size() > 0) {
		const auto startTime = times.front().first;
		const auto endTime = times.front().second;
		confParams->setStartTime(startTime);
		confParams->setEndTime(endTime);
		msp.getPrivate()->setStartTime(startTime);
		msp.getPrivate()->setEndTime(endTime);
	}

	shared_ptr<CallSession> session = getMe()->createSession(*this, &msp, true, nullptr);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));

	const auto resourceList = op->getContentInRemote (ContentType::ResourceLists);
	if (!resourceList.isEmpty()) {
		auto invitee = Conference::parseResourceLists(resourceList);
		invitedAddresses.insert(invitedAddresses.begin(), invitee.begin(), invitee.end());
	}
	invitedAddresses.push_back(IdentityAddress(op->getFrom()));

	getMe()->setAdmin(true);
	getMe()->setFocus(true);

	setState(ConferenceInterface::State::Instantiated);
}

LocalConference::LocalConference (const std::shared_ptr<Core> &core, const std::shared_ptr<ConferenceInfo> & info) :
	Conference(core, info->getOrganizer(), nullptr, ConferenceParams::create(core->getCCore())) {

	LinphoneCore * lc = core->getCCore();

#ifdef HAVE_ADVANCED_IM
	bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(lc), "misc", "conference_event_log_enabled", TRUE );
	if (eventLogEnabled) {
		eventHandler = std::make_shared<LocalAudioVideoConferenceEventHandler>(this);
		addListener(eventHandler);
	}
#endif // HAVE_ADVANCED_IM

	mMixerSession.reset(new MixerSession(*core.get()));

	const auto & conferenceAddress = info->getUri();

	confParams->enableAudio(true);
	confParams->enableVideo(linphone_core_video_enabled(lc));
	confParams->setSubject(info->getSubject());
	confParams->enableLocalParticipant(false);
	confParams->enableOneParticipantConference(true);
	confParams->setStatic(true);

	time_t startTime = info->getDateTime();
	confParams->setStartTime(startTime);

	const auto duration = info->getDuration();
	if ((duration > 0) && (startTime >= 0)) {
		time_t endTime = startTime + static_cast<time_t>(duration) * 60;
		confParams->setEndTime(endTime);
	}

	const auto & participants = info->getParticipants();
	for (const auto & p : participants) {
		invitedAddresses.push_back(p);
	}
	invitedAddresses.push_back(info->getOrganizer());

	getMe()->setAdmin(true);
	getMe()->setFocus(true);

	setState(ConferenceInterface::State::Instantiated);

	setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
	setConferenceAddress(conferenceAddress);

}

LocalConference::~LocalConference() {
	if ((state != ConferenceInterface::State::Terminated) && (state != ConferenceInterface::State::Deleted)) {
		terminate();
	}
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM
	mMixerSession.reset();
}

void LocalConference::notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) {
	// Call callbacks before calling listeners because listeners may change state
	linphone_core_notify_conference_state_changed(getCore()->getCCore(), toC(), (LinphoneConferenceState)getState());

	Conference::notifyStateChanged (state);
}

void LocalConference::confirmCreation () {

	if ((state != ConferenceInterface::State::Instantiated) && (state != ConferenceInterface::State::CreationPending)) {
		lError() << "Unable to confirm the creation of the conference in state " << state;
	}

	shared_ptr<CallSession> session = getMe()->getSession();

	if (session) {
		/* Assign a random conference address to this new conference, with domain
		 * set according to the proxy config used to receive the INVITE.
		 */
		LinphoneProxyConfig *cfg = session->getPrivate()->getDestProxy();
		if (!cfg) cfg = linphone_core_get_default_proxy_config(L_GET_C_BACK_PTR(getCore()));
		LinphoneAddress *addr = linphone_address_clone(linphone_proxy_config_get_identity_address(cfg));

		char confId[LinphonePrivate::MediaConference::LocalConference::confIdLength];
		belle_sip_random_token(confId,sizeof(confId));
		linphone_address_set_uri_param (addr, "conf-id", confId);
		Address conferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr));
		linphone_address_unref(addr);
		setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));

		const_cast<LinphonePrivate::CallSessionParamsPrivate *>(L_GET_PRIVATE(session->getParams()))->setInConference(true);
		session->getPrivate()->setConferenceId(confId);
		session->startIncomingNotification(false);

#ifdef HAVE_DB_STORAGE
		// Store into DB after the start incoming notification in order to have a valid conference address being the contact address of the call
		auto &mainDb = getCore()->getPrivate()->mainDb;
		const auto & conferenceInfo = createOrGetConferenceInfo();
		if (mainDb) {
			lInfo() << "Inserting conference information to database in order to be able to recreate the conference " << getConferenceAddress() << " in case of restart";
			mainDb->insertConferenceInfo(conferenceInfo);
		}
		auto callLog = session->getLog();
		if (callLog) {
			callLog->setConferenceInfo(conferenceInfo);
		}
#endif

	} else {
		lError() << "Unable to confirm the creation of the conference because no session was created";
	}
}

#ifdef HAVE_DB_STORAGE
std::shared_ptr<ConferenceInfo> LocalConference::createOrGetConferenceInfo() const {
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		std::shared_ptr<ConferenceInfo> conferenceInfo = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(getConferenceAddress());
		if (conferenceInfo) {
			return conferenceInfo;
		}
	}

	return createConferenceInfo(getMe()->getAddress());
}
#endif // HAVE_DB_STORAGE


void LocalConference::finalizeCreation() {

	if (getState() == ConferenceInterface::State::CreationPending) {
		const ConferenceAddress & conferenceAddress = getConferenceAddress();
		setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
		shared_ptr<CallSession> session = me->getSession();
		if (session) {
			Address addr(conferenceAddress.asAddress());
			addr.setParam("isfocus");
			if (session->getState() == CallSession::State::Idle) {
				lInfo() << " Scheduling redirection to [" << addr <<"] for Call session ["<<session<<"]" ;
				getCore()->doLater([session,addr] {
					session->redirect(addr);
				});
			} else {
					session->redirect(addr);
			}
		}
#ifdef HAVE_ADVANCED_IM
		if (eventHandler) {
			eventHandler->setConference(this);
		}
#endif // HAVE_ADVANCED_IM
	}
}

void LocalConference::subscribeReceived (LinphoneEvent *event) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->subscribeReceived(event);
		return;
	}
#endif // HAVE_ADVANCED_IM
	linphone_event_deny_subscription(event, LinphoneReasonNotAcceptable);
}

void LocalConference::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin != participant->isAdmin()) {
		participant->setAdmin(isAdmin);
		time_t creationTime = time(nullptr);
		notifyParticipantSetAdmin (creationTime, false, participant, isAdmin);
	}
}

void LocalConference::onConferenceTerminated (const IdentityAddress &addr) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->setConference(nullptr);
	}
#endif // HAVE_ADVANCED_IM
	Conference::onConferenceTerminated(addr);
}

void LocalConference::addLocalEndpoint () {
	if (confParams->localParticipantEnabled()) {
		StreamMixer *mixer = mMixerSession->getMixerByType(SalAudio);
		if (mixer) {
			mixer->enableLocalParticipant(true);
			// Get ssrc of me because it must be sent to participants through NOTIFY
			auto audioMixer = dynamic_cast<MS2AudioMixer*>(mixer);
			auto audioStream = audioMixer->getAudioStream();
			auto meSsrc = audio_stream_get_send_ssrc(audioStream);
			for (auto & device : me->getDevices()) {
				device->setSsrc(meSsrc);
			}
		}

		if (confParams->videoEnabled()){
			mixer = mMixerSession->getMixerByType(SalVideo);
			if (mixer){
#ifdef VIDEO_ENABLED
				for (auto & device : me->getDevices()) {
					auto videoMixer = dynamic_cast<MS2VideoMixer*>(mixer);
					videoMixer->setLocalParticipantLabel(device->getLabel());
				}
#endif // VIDEO_ENABLED
				mixer->enableLocalParticipant(true);

				VideoControlInterface *vci = getVideoControlInterface();
				if (vci){
					vci->setNativePreviewWindowId(getCore()->getCCore()->preview_window_id);
					vci->setNativeWindowId(getCore()->getCCore()->video_window_id);
				}
			}
		}

		if (!isIn()) {
			mIsIn = true;
			time_t creationTime = time(nullptr);
			notifyParticipantAdded(creationTime, false, getMe());
			for (auto & device : me->getDevices()) {
				notifyParticipantDeviceAdded(creationTime, false, getMe(), device);
			}
		}
	}
}

int LocalConference::inviteAddresses (const list<const LinphoneAddress *> &addresses, const LinphoneCallParams *params) {

	const auto & coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);

	const auto & outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto & inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	for (const auto &address : addresses) {
		LinphoneCall *call = linphone_core_get_call_by_remote_address2(getCore()->getCCore(), address);
		char *cAddress = linphone_address_as_string(address);
		Address cppAddress(cAddress);
		free(cAddress);
		if (!call) {
			/* Start a new call by indicating that it has to be put into the conference directly */
			LinphoneCallParams *new_params;
			if (params){
				new_params = linphone_call_params_copy(params);
			}else{
				new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
				linphone_call_params_enable_video(new_params, confParams->videoEnabled());
			}

			linphone_call_params_set_in_conference(new_params, TRUE);

			const Address & conferenceAddress = getConferenceAddress().asAddress();
			const string & confId = conferenceAddress.getUriParamValue("conf-id");
			linphone_call_params_set_conference_id(new_params, confId.c_str());

			call = linphone_core_invite_address_with_params(getCore()->getCCore(), address, new_params);

			if (!confParams->getAccount()) {
				// Set proxy configuration used for the conference
				auto callProxyCfg = linphone_call_get_dest_proxy(call);
				if (callProxyCfg) {
					auto callAccount = linphone_core_lookup_account_by_identity(getCore()->getCCore(), linphone_proxy_config_get_identity_address(callProxyCfg));
					confParams->setAccount(callAccount);
				} else {
					confParams->setAccount(linphone_core_lookup_known_account(getCore()->getCCore(), address));
				}
			}

			tryAddMeDevice();

			if (!call){
				lError() << "LocalConference::inviteAddresses(): could not invite participant";
			} else {
				addParticipant(Call::toCpp(call)->getSharedFromThis());
				const std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(cppAddress);
				participant->setPreserveSession(false);
			}
			linphone_call_params_unref(new_params);
		} else {
			/* There is already a call to this address, so simply join it to the local conference if not already done */
			if (!linphone_call_params_get_in_conference(linphone_call_get_current_params(call))) {
				addParticipant(Call::toCpp(call)->getSharedFromThis());

				const std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(cppAddress);
				participant->setPreserveSession(true);
			}
		}
		/* If the local participant is not yet created, created it and it to the conference */
		addLocalEndpoint();
		Call::toCpp(call)->setConference(toC());
	}

	// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change audio route to keep the one currently used
	if (startingConference) {
		if (outputDevice) {
			setOutputAudioDevice(outputDevice);
		}
		if (inputDevice) {
			setInputAudioDevice(inputDevice);
		}
	}

	return 0;
}

int LocalConference::participantDeviceJoined(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceJoined(p, device);
		} else {
			lDebug() << "Unable to find device with session " << session << " among devices of participant " << p->getAddress().asString() << " of conference " << getConferenceAddress();
		}
	}
	return -1;
}

int LocalConference::participantDeviceJoined(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	int success = -1;
	if ((device->updateMediaCapabilities() || (device->getState() != ParticipantDevice::State::Present)) && (getState() == ConferenceInterface::State::Created)) {
		lInfo() << "Device " << device->getAddress() << " joined conference " << getConferenceAddress();
		device->setState(ParticipantDevice::State::Present);
		device->updateStreamAvailabilities();
		time_t creationTime = time(nullptr);
		notifyParticipantDeviceJoined(creationTime, false, participant, device);
		return 0;
	}
	return success;
}

int LocalConference::participantDeviceLeft(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceLeft(p, device);
		} else {
			lWarning() << "Unable to find device with session " << session << " among devices of participant " << p->getAddress().asString() << " of conference " << getConferenceAddress();
		}
	}
	return -1;
}

int LocalConference::participantDeviceLeft(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	int success = -1;
	if ((device->updateMediaCapabilities() || (device->getState() != ParticipantDevice::State::OnHold)) && (getState() == ConferenceInterface::State::Created)) {
		lInfo() << "Device " << device->getAddress() << " left conference " << getConferenceAddress();
		device->setState(ParticipantDevice::State::OnHold);
		device->updateStreamAvailabilities();
		time_t creationTime = time(nullptr);
		notifyParticipantDeviceLeft(creationTime, false, participant, device);
		return 0;
	}
	return success;
}
int LocalConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			success = participantDeviceMediaCapabilityChanged(p, device);
		} else {
			lWarning() << "Unable to find device with session " << session << " among devices of participant " << p->getAddress().asString() << " of conference " << getConferenceAddress();
		}
	}
	return success;
}

int LocalConference::participantDeviceMediaCapabilityChanged(const IdentityAddress &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	int success = -1;
	for (const auto & d : p->getDevices()) {
		success = participantDeviceMediaCapabilityChanged(p, d);
	}
	return success;
}

int LocalConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	int success = -1;
	if (device->updateMediaCapabilities() && ((getState() == ConferenceInterface::State::CreationPending) || (getState() == ConferenceInterface::State::Created)) && (device->getState() == ParticipantDevice::State::Present)) {
		device->updateStreamAvailabilities();
		time_t creationTime = time(nullptr);
		notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, participant, device);
		return 0;
	}
	return success;
}

int LocalConference::participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session, uint32_t ssrc) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device && (device->getSsrc() != ssrc)) {
			lInfo() << "Setting ssrc of participant device " << device->getAddress().asString() << " in conference " << getConferenceAddress() << " to " << ssrc;
			device->setSsrc(ssrc);
			if (device->getState() == ParticipantDevice::State::Present) {
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, p, device);
			}
			return 0;
		}
	}
	lInfo() << "Unable to set ssrc to " << ssrc << " because participant device with session " << session << " cannot be found in conference " << getConferenceAddress();
	return success;
}

int LocalConference::getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> & device) {
	MS2AudioMixer * mixer = dynamic_cast<MS2AudioMixer*> (mMixerSession->getMixerByType(SalAudio));

	if (mixer) {
		MSAudioConference *conf = mixer->getAudioConference();
		return ms_audio_conference_get_participant_volume(conf, device->getSsrc());
	}

	return AUDIOSTREAMVOLUMES_NOT_FOUND;
}

bool LocalConference::addParticipants (const std::list<std::shared_ptr<Call>> &calls) {
	const auto & coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);
	const auto & outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto & inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	bool success = Conference::addParticipants(calls);
	// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change audio route to keep the one currently used
	// Do not change audio route if participant addition is not successful
	if (success && startingConference) {
		if (outputDevice) {
			setOutputAudioDevice(outputDevice);
		}
		if (inputDevice) {
			setInputAudioDevice(inputDevice);
		}
	}

	return success;
}

bool LocalConference::addParticipants (const std::list<IdentityAddress> &addresses) {
	return Conference::addParticipants(addresses);
}

bool LocalConference::addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) {

	bool success = Conference::addParticipantDevice(call);

	if (success) {
		call->setConference(toC());
		auto device = findParticipantDevice (call->getActiveSession());

		if (device) {
			char label[LinphonePrivate::Conference::labelLength];
			belle_sip_random_token(label,sizeof(label));
			device->setLabel(label);
		}

	}

	return success;
}

bool LocalConference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {

	const auto & remoteAddress = call->getRemoteAddress();
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(call->toC()))) {
		lError() << "Call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") is already in conference " << getConferenceAddress();
		return false;
	}

	if (confParams->getParticipantListType() == ConferenceParams::ParticipantListType::Closed) {
		auto p = std::find(invitedAddresses.begin(), invitedAddresses.end(), IdentityAddress(*remoteAddress));
		if (p == invitedAddresses.end()) {
			lError() << "Unable to add call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") because participant " << *remoteAddress << " is not in the list of allowed participants of conference " << getConferenceAddress();
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Call forbidden to join the conference", NULL);
			call->terminate(ei);
			linphone_error_info_unref(ei);
			return false;
		}
	}

#if 0
	if (!isConferenceStarted()) {
		lError() << "Unable to add call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") because participant " << *remoteAddress << " is not in the list of allowed participants of conference " << getConferenceAddress();
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Conference not started yet", NULL);
		call->terminate(ei);
		linphone_error_info_unref(ei);
		return false;
	}

	if (isConferenceEnded()) {
		lError() << "Unable to add call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") because participant " << *remoteAddress << " is not in the list of allowed participants of conference " << getConferenceAddress();
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Conference already terminated", NULL);
		call->terminate(ei);
		linphone_error_info_unref(ei);
		return false;
	}
#endif

	const Address & conferenceAddress = getConferenceAddress().asAddress();
	const string & confId = conferenceAddress.getUriParamValue("conf-id");
	const string & callConfId = call->getConferenceId();

	const auto & coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);

	const auto & outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto & inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	// Add participant only if creation is successful or call was previously part of the conference
	bool canAddParticipant = ((callConfId.compare(confId) == 0) || (getState() == ConferenceInterface::State::CreationPending) || (getState() == ConferenceInterface::State::Created));

	if (canAddParticipant) {
		auto session = call->getMediaSession();
		const auto & remoteContactAddress = session->getRemoteContactAddress();
		LinphoneCallState state = static_cast<LinphoneCallState>(call->getState());

		auto participantDevice = (remoteContactAddress && remoteContactAddress->isValid()) ? findParticipantDevice(*remoteContactAddress) : nullptr;
		if (participantDevice) {
			auto deviceSession = participantDevice->getSession();
			if (deviceSession) {
				if (session == deviceSession) {
					lWarning() << "Try to add again a participant device with session " << session;
				} else {
					lInfo() << "Already found a participant device with address " << *remoteContactAddress << ". Recreating it";
					deviceSession->terminate();
				}
			}
		}

		if (!confParams->getAccount()) {
			// Set proxy configuration used for the conference
			auto callProxyCfg = linphone_call_get_dest_proxy(call->toC());
			if (callProxyCfg) {
				auto callAccount = linphone_core_lookup_account_by_identity(getCore()->getCCore(), linphone_proxy_config_get_identity_address(callProxyCfg));
				confParams->setAccount(callAccount);
			} else {
				confParams->setAccount(linphone_core_lookup_known_account(getCore()->getCCore(), linphone_call_get_to_address(call->toC())));
			}
		}

		// Get contact address here because it may be modified by a change in the local parameters. As the participant enters the conference, in fact attributes conf-id and isfocus are added later on (based on local parameters) therefore there is no way to know if the remote client already knew that the call was in a conference or not.
		auto contactAddress = session->getContactAddress();
		tryAddMeDevice();

		// Add participant to the conference participant list
		switch(state){
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallOutgoingRinging:
			case LinphoneCallIncomingReceived:
			case LinphoneCallPausing:
			case LinphoneCallPaused:
			case LinphoneCallResuming:
			case LinphoneCallStreamsRunning:
				if (call->toC() == linphone_core_get_current_call(getCore()->getCCore()))
					L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(nullptr);
				mMixerSession->joinStreamsGroup(session->getStreamsGroup());

				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))->setInConference(true);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))->setConferenceId(confId);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))->setStartTime(confParams->getStartTime());
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))->setEndTime(confParams->getEndTime());
				if (getCurrentParams().videoEnabled()) {
					if (getCurrentParams().localParticipantEnabled()) {
						const_cast<LinphonePrivate::MediaSessionParams*>(call->getParams())->enableVideo(true);
					} else {
						if (call->getRemoteParams()) {
							const_cast<LinphonePrivate::MediaSessionParams*>(call->getParams())->enableVideo(call->getRemoteParams()->videoEnabled());
						}
					}
				} else {
					const_cast<LinphonePrivate::MediaSessionParams*>(call->getParams())->enableVideo(false);
				}

				Conference::addParticipant(call);

			break;
			default:
				lError() << "Call " << call << " (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") is in state " << Utils::toString(call->getState()) << ", hence it cannot be added to the conference right now";
				return false;
			break;
		}

		// Update call
		auto device = findParticipantDevice(session);

		switch(state){
			case LinphoneCallPausing:
				// Call cannot be resumed immediately, hence delay it until next state change
				session->delayResume();
			break;
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallOutgoingRinging:
			case LinphoneCallIncomingReceived:
			break;
			case LinphoneCallPaused:
				/*
				 * Modifying the MediaSession's params directly is a bit hacky.
				 * However, the resume() method doesn't accept parameters.
				 */
				device->setState(ParticipantDevice::State::Joining);
				// Conference resumes call that previously paused in order to add the participant
				call->resume();
			break;
			case LinphoneCallStreamsRunning:
			case LinphoneCallResuming:
			{
				if (state == LinphoneCallStreamsRunning) {
					// Calling enter here because update will lock sound resources
					enter();
				}
				device->setState(ParticipantDevice::State::Joining);

				if (contactAddress.isValid() && !contactAddress.hasParam("isfocus")) {
					const MediaSessionParams * params = session->getMediaParams();
					MediaSessionParams *currentParams = params->clone();
					call->update(currentParams);
					delete currentParams;
				}
			}
			break;
			default:
				lError() << "Call " << call << " (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") is in state " << Utils::toString(call->getState()) << ", hence the call cannot be updated following it becoming part of the conference";
				return false;
			break;
		}

		// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change audio route to keep the one currently used
		if (startingConference) {
			if (outputDevice) {
				setOutputAudioDevice(outputDevice);
			}
			if (inputDevice) {
				setInputAudioDevice(inputDevice);
			}
		}

		setState(ConferenceInterface::State::Created);
		return true;
	}

	lError() << "Unable to add call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") to conference " << getConferenceAddress();
	return false;
}

bool LocalConference::addParticipant (const IdentityAddress &participantAddress) {
#if 0
	if (!isConferenceEnded() && isConferenceStarted()) {
		if (confParams->getParticipantListType() == ConferenceParams::ParticipantListType::Closed) {
			auto p = std::find(invitedAddresses.begin(), invitedAddresses.end(), participantAddress);
			if (p == invitedAddresses.end()) {
				lError() << "Unable to add participant " << participantAddress << " because it is not in the list of allowed participants of conference " << getConferenceAddress();
				return false;
			}
		}
		bool success = Conference::addParticipant(participantAddress);
		if (success) {
			setState(ConferenceInterface::State::Created);
			enter();
		}
		return success;
	} else {
#endif
		const auto & endTime = confParams->getEndTime();
		const auto & startTime = confParams->getStartTime();
		const auto now = time(NULL);
		lError() << "Could not add participant " << participantAddress << " to the conference because the conference " << getConferenceAddress() << " is not active right now.";
		if (startTime >= 0) {
			lError() << "Expected start time (" << startTime << "): " << ctime(&startTime);
		} else {
			lError() << "Expected start time: none";
		}
		if (endTime >= 0) {
			lError() << "Expected end time (" << endTime << "): " << ctime(&endTime);
		} else {
			lError() << "Expected end time: none";
		}
		lError() << "Now: " << ctime(&now);
		return false;

#if 0
	}
#endif
	return false;
}

void LocalConference::setLocalParticipantStreamCapability(const LinphoneMediaDirection & direction, const LinphoneStreamType type) {
	if (confParams->localParticipantEnabled() && !me->getDevices().empty() && confParams->getAccount() && (type != LinphoneStreamTypeUnknown)) {
		char * devAddrStr = linphone_account_get_contact_address(confParams->getAccount()) ? linphone_address_as_string(linphone_account_get_contact_address(confParams->getAccount())) : nullptr;
		if (devAddrStr) {
			Address devAddr(devAddrStr);
			ms_free(devAddrStr);
			const auto & meDev = me->findDevice(devAddr);
			if (meDev) {
				lInfo() << "Setting direction of stream of type " << std::string(linphone_stream_type_to_string(type)) << " to " << std::string(linphone_media_direction_to_string(direction)) << " of device " << meDev->getAddress();
				meDev->setStreamCapability(direction, type);
				meDev->updateStreamAvailabilities();
				for (const auto & p : getParticipants()) {
					for (const auto & d : p->getDevices()) {
						d->updateStreamAvailabilities();
					}
				}
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, me, meDev);
			} else {
				lError() << "Unable to find device with address " << devAddr << " among those in the local participant " << me->getAddress();
			}
		}
	}
}

bool LocalConference::finalizeParticipantAddition (std::shared_ptr<LinphonePrivate::Call> call) {
	const auto & newParticipantSession = call->getMediaSession();
	const auto & device = findParticipantDevice(newParticipantSession);
	if (device) {
		if (((device->getState() != ParticipantDevice::State::Present) && (getLayout() == ConferenceLayout::Legacy) && (call->getActiveSession()->getPreviousState() == CallSession::State::Connected)) || (device->getState() == ParticipantDevice::State::Joining)) {
			device->updateMediaCapabilities();
			device->updateStreamAvailabilities();
			device->setState(ParticipantDevice::State::Present);
			const auto & p = findParticipant(call->getActiveSession());

			if (device && p) {
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceAdded(creationTime, false, p, device);
			}
		} else if (device->getState() == ParticipantDevice::State::ScheduledForJoining) {
			device->setState(ParticipantDevice::State::Joining);
			auto contactAddress = newParticipantSession->getContactAddress();

			if (contactAddress.isValid() && !contactAddress.hasParam("isfocus")) {
				getCore()->doLater([this, call] {
					const Address & conferenceAddress = getConferenceAddress().asAddress();
					const string & confId = conferenceAddress.getUriParamValue("conf-id");

					LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), call->toC());
					linphone_call_params_set_in_conference(params, TRUE);
					linphone_call_params_set_conference_id(params, confId.c_str());
					linphone_call_params_set_start_time(params, confParams->getStartTime());
					linphone_call_params_set_end_time(params, confParams->getEndTime());
					if (getCurrentParams().videoEnabled()) {
						linphone_call_params_enable_video(params, linphone_call_params_video_enabled(linphone_call_get_remote_params(call->toC())));
					} else {
						linphone_call_params_enable_video(params, FALSE);
					}

					linphone_call_update(call->toC(), params);
					linphone_call_params_unref(params);
				});
			}
		}
	}

	return true;
}

int LocalConference::removeParticipant (const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession) {
	int err = 0;

	auto op = session->getPrivate()->getOp();
	shared_ptr<Call> call = getCore()->getCallByCallId (op->getCallId());
	if (call) {
		const auto & remoteAddress = call->getRemoteAddress();
		if (linphone_call_get_conference(call->toC()) != toC()){
			lError() << "Call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") is not part of conference " << getConferenceAddress();
			return -1;
		}
	}

	CallSession::State sessionState = session->getState();

	std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(session);
	if (participant) {
		if (participant->isAdmin()) setParticipantAdminStatus(participant, false);
		Conference::removeParticipant(participant);
		mMixerSession->unjoinStreamsGroup(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getStreamsGroup());
		checkIfTerminated();
	} else {
		if ((sessionState != LinphonePrivate::CallSession::State::Released) && (sessionState != LinphonePrivate::CallSession::State::End)) {
			lError() << "Trying to remove participant " << *session->getRemoteAddress() << " with session " << session << " which is not part of conference " << getConferenceAddress();
		}
		return -1;
	}

	if (getState() != ConferenceInterface::State::TerminationPending) {

		// Detach call from conference
		if (call) {
			call->setConference(nullptr);
		}

		if (participant->getPreserveSession()) {
			// If the session is already paused,then send an update to kick the participant out of the conference, pause the call otherwise
			if (sessionState == CallSession::State::Paused) {
				lInfo() << "Updating call to notify of conference removal.";
				const MediaSessionParams * params = static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams();
				MediaSessionParams *currentParams = params->clone();
				currentParams->getPrivate()->setInConference(FALSE);
				currentParams->getPrivate()->setConferenceId("");
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->updateFromConference(currentParams);
				delete currentParams;
			} else if ((sessionState != CallSession::State::End) && (sessionState != CallSession::State::Released)) {
				lInfo() << "Pause call to notify of conference removal.";
				/* Kick the session out of the conference by moving to the Paused state. */
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
						L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams()))->setInConference(false);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
						L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams()))->setConferenceId("");
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->pauseFromConference();
			}
		} else {
			// Terminate session (i.e. send a BYE) as per RFC
			// This is the default behaviour
			if (sessionState != LinphonePrivate::CallSession::State::End) {
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->terminate();
			}
		}

		/*
		 * Handle the case where only the local participant and a unique remote participant are remaining.
		 * In this case, if the session linked to the participant has to be preserved after the conference, then destroy the conference and let these two participants to connect directly thanks to a simple call.
		 * Indeed, the conference adds latency and processing that is useless to do for 1-1 conversation.
		 */
		if (!confParams->oneParticipantConferenceEnabled() && (getParticipantCount() == 1) && (!preserveSession)) {
			std::shared_ptr<LinphonePrivate::Participant> remainingParticipant = participants.front();
			if (remainingParticipant->isAdmin()) setParticipantAdminStatus(remainingParticipant, false);
			const bool lastParticipantPreserveSession = remainingParticipant->getPreserveSession();
			if (lastParticipantPreserveSession) {

				std::shared_ptr<LinphonePrivate::MediaSession> lastSession = static_pointer_cast<LinphonePrivate::MediaSession>(remainingParticipant->getSession());

				lInfo() << "Participant [" << remainingParticipant << "] with " << lastSession->getRemoteAddress()->asString() << 
					" is the last call in conference " << getConferenceAddress() << ", we will reconnect directly to it.";

				const MediaSessionParams * params = lastSession->getMediaParams();
				// If only one participant is in the conference, the conference is destroyed.
				if (isIn()){
					lInfo() << "Updating call to notify of conference removal.";
					MediaSessionParams *currentParams = params->clone();
					// If the local participant is in, then an update is sent in order to notify that the call is exiting the conference
					currentParams->getPrivate()->setInConference(FALSE);
					currentParams->getPrivate()->setConferenceId("");
					err = lastSession->updateFromConference(currentParams);
					delete currentParams;
				} else {
					// If the local participant is not in, the call is paused as the local participant is busy
					const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
							L_GET_PRIVATE(params))->setInConference(false);

					err = lastSession->pauseFromConference();
				}

				setState(ConferenceInterface::State::TerminationPending);

				leave();

				/* invoke removeParticipant() recursively to remove this last participant. */
				bool success = Conference::removeParticipant(remainingParticipant);
				mMixerSession->unjoinStreamsGroup(lastSession->getStreamsGroup());

				// Detach call from conference
				auto lastOp = lastSession->getPrivate()->getOp();
				shared_ptr<Call> lastSessionCall = getCore()->getCallByCallId (lastOp->getCallId());

				if (lastSessionCall) {
					lastSessionCall->setConference(nullptr);
				}

				checkIfTerminated();
				return success;

			}
		}
	}

	// If call that we are trying to remove from the conference is in paused by remote state, then it temporarely left the conference therefore it must not be terminated
	if (sessionState != LinphonePrivate::CallSession::State::PausedByRemote) {
		checkIfTerminated();
	}
	
	return err;
}

void LocalConference::checkIfTerminated() {
	if (!confParams->isStatic() && (getParticipantCount() == 0)) {
		leave();
		if (getState() == ConferenceInterface::State::TerminationPending) {
			setState(ConferenceInterface::State::Terminated);
		} else {
			setState(ConferenceInterface::State::TerminationPending);
		}
	}
}


void LocalConference::chooseAnotherAdminIfNoneInConference() {
	if (participants.empty() == false) {
		const auto adminParticipant = std::find_if(participants.cbegin(), participants.cend(), [&] (const auto & p) {
			return (p->isAdmin() == true);
		});
		// If not admin participant is found
		if (adminParticipant == participants.cend()) {
			setParticipantAdminStatus(participants.front(), true);
			lInfo() << this << ": New admin designated is " << *(participants.front());
		}
	}
}

int LocalConference::removeParticipant (const IdentityAddress &addr) {
	const std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(addr);
	if (!participant)
		return -1;
	return removeParticipant(participant);
}

bool LocalConference::removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	std::shared_ptr<LinphonePrivate::CallSession> callSession = participant->getSession();
	if (!callSession)
		return false;
	return (bool)removeParticipant(callSession, false);
}

/* ConferenceInterface */
void LocalConference::setSubject (const std::string &subject) {
	if (subject.compare(getSubject()) != 0) {
		Conference::setSubject(subject);
		time_t creationTime = time(nullptr);
		notifySubjectChanged(creationTime, false, subject);
	}
}

void LocalConference::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->subscriptionStateChanged(event, state);
	}
#endif // HAVE_ADVANCED_IM
}

int LocalConference::terminate () {
	lInfo() << "Terminate conference " << getConferenceAddress();
	// Take a ref because the conference may be immediately go to deleted state if terminate is called when there are 0 participants
	const auto ref = getSharedFromThis();
	setState(ConferenceInterface::State::TerminationPending);

	auto participantIt = participants.begin();
	while (participantIt != participants.end()) {
		auto participant = *participantIt;
		std::shared_ptr<LinphonePrivate::MediaSession> session = static_pointer_cast<LinphonePrivate::MediaSession>(participant->getSession());
		participantIt++;
		if (session) {
			lInfo() << "Terminating session of participant " << *participant;
			session->terminate();
		}
	}

	return 0;
}

int LocalConference::enter () {

	if (confParams->localParticipantEnabled()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore()))
			return -1;
		if (linphone_core_get_current_call(getCore()->getCCore()))
			linphone_call_pause(linphone_core_get_current_call(getCore()->getCCore()));

		lInfo() << getMe()->getAddress().asString() << " is rejoining conference " << getConferenceAddress();
		addLocalEndpoint();
		if (me->getDevices().size() > 0) {
			participantDeviceJoined(me, me->getDevices().front());
		}
	}

	return 0;
}

void LocalConference::removeLocalEndpoint () {
	mMixerSession->enableLocalParticipant(false);

	if (isIn()) {
		mIsIn = false;

		time_t creationTime = time(nullptr);
		for (auto & device : me->getDevices()) {
			notifyParticipantDeviceRemoved(creationTime, false, getMe(), device);
		}
		notifyParticipantRemoved(creationTime, false, getMe());
	}
}

void LocalConference::leave () {
	if (isIn()) {
		lInfo() << getMe()->getAddress() << " is leaving conference " << getConferenceAddress();
		if (me->getDevices().size() > 0) {
			participantDeviceLeft(me, me->getDevices().front());
		}
		removeLocalEndpoint();
	}
}

bool LocalConference::update(const LinphonePrivate::ConferenceParamsInterface &newParameters){
	const LinphonePrivate::ConferenceParams &newConfParams = static_cast<const ConferenceParams&>(newParameters);
	/* Only adding or removing video is supported. */
	bool previousVideoEnablement = confParams->videoEnabled();
	bool previousAudioEnablement = confParams->audioEnabled();
	bool previousChatEnablement = confParams->chatEnabled();
	bool ret = MediaConference::Conference::update(newParameters);
	if (ret) {
		// Update endpoints only if audio or video settings have changed
		if ((newConfParams.videoEnabled() != previousVideoEnablement) || (newConfParams.audioEnabled() != previousAudioEnablement)){
			/* Don't forget the local participant. For simplicity, a removeLocalEndpoint()/addLocalEndpoint() does the job. */
			removeLocalEndpoint();
			addLocalEndpoint();
		}
		if ((newConfParams.chatEnabled() != previousChatEnablement) || (newConfParams.videoEnabled() != previousVideoEnablement) || (newConfParams.audioEnabled() != previousAudioEnablement)){
			time_t creationTime = time(nullptr);
			notifyAvailableMediaChanged(creationTime, false, getMediaCapabilities());
		}
		for (auto & meDev : me->getDevices()) {
			meDev->setStreamCapability((this->confParams->audioEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive), LinphoneStreamTypeAudio);
			meDev->setStreamCapability((this->confParams->videoEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive), LinphoneStreamTypeVideo);
			meDev->setStreamCapability((this->confParams->chatEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive), LinphoneStreamTypeText);
		}
	}
	return ret;
}

int LocalConference::startRecording (const char *path) {
	MS2AudioMixer * mixer = mMixerSession ? dynamic_cast<MS2AudioMixer*> (mMixerSession->getMixerByType(SalAudio)) : nullptr;
	if (mixer){
		mixer->setRecordPath(path);
		mixer->startRecording();
		//TODO: error reporting is absent.
	}else{
		lError() << "LocalConference::startRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

int LocalConference::stopRecording () {
	AudioControlInterface * aci = getAudioControlInterface();
	if (aci){
		aci->stopRecording();
	}else{
		lError() << "LocalConference::stopRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

bool LocalConference::isRecording () const {
	AudioControlInterface * aci = getAudioControlInterface();
	if (aci){
		return aci->isRecording();
	}
	return false;
}

bool LocalConference::isIn() const{
	return mIsIn;
}

AudioControlInterface *LocalConference::getAudioControlInterface()const{
	return mMixerSession ? dynamic_cast<AudioControlInterface*>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
}

VideoControlInterface * LocalConference::getVideoControlInterface() const{
	return mMixerSession ? dynamic_cast<VideoControlInterface*>(mMixerSession->getMixerByType(SalVideo)) : nullptr;
}

AudioStream *LocalConference::getAudioStream(){
	MS2AudioMixer *mixer = mMixerSession ? dynamic_cast<MS2AudioMixer*>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
	return mixer ? mixer->getAudioStream() : nullptr;
}

void LocalConference::notifyFullState () {
	++lastNotify;
	Conference::notifyFullState();
}

void LocalConference::callStateChangedCb (LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	LocalConference *conf = (LocalConference *)linphone_core_v_table_get_user_data(vtable);
	auto cppCall = Call::toCpp(call)->getSharedFromThis();
	if (conf && conf->toC() == cppCall->getConference()) {
		const auto & session = cppCall->getActiveSession();
		switch(cstate) {
			case LinphoneCallStateConnected:
				if (getState() == ConferenceInterface::State::Created) {
					enter();
				}
				break;
			case LinphoneCallStateStreamsRunning:
			{
				if(!addParticipantDevice(cppCall)) {
					// If the participant is already in the conference
					const auto & device = findParticipantDevice(session);
					const auto & deviceState = device->getState();
					if (deviceState == ParticipantDevice::State::Present) {
						participantDeviceMediaCapabilityChanged(session);
					} else if ((device->getState() == ParticipantDevice::State::Joining) || (device->getState() == ParticipantDevice::State::ScheduledForJoining)) {
						// Participants complete their addition to a conference when the call goes back to the StreamsRunning state

						if (!cppCall->mediaInProgress() || !!!linphone_config_get_int(linphone_core_get_config(getCore()->getCCore()), "sip", "update_call_when_ice_completed", TRUE)){
							// Participants complete their addition to a conference when the call goes back to the StreamsRunning state
							finalizeParticipantAddition(cppCall);
						} else {
							auto contactAddress = session->getContactAddress();
							if (contactAddress.isValid() && contactAddress.hasParam("isfocus")) {
								device->setState(ParticipantDevice::State::Joining);
							}
						}
					} else {
						participantDeviceJoined(session);
					}
				}
			}
				break;
			case LinphoneCallStatePausedByRemote:
				// The participant temporarely left the conference and put its call in pause
				// If a call in a local conference is paused by remote, it means that the remote participant temporarely left the call, hence notify that no audio and video is available
				lInfo() << "Call in conference has been put on hold by remote device, hence participant " << session->getRemoteAddress()->asString() << " temporarely left conference " << getConferenceAddress();
				participantDeviceLeft(session);
				break;
			case LinphoneCallStateUpdatedByRemote:
			{
				// If the participant is already in the conference
				const auto & device = findParticipantDevice(session);
				const auto & deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;
				if (session && ((deviceState == ParticipantDevice::State::Present) || (device->getState() == ParticipantDevice::State::Joining))) {

					const auto op = session->getPrivate()->getOp();
					// The remote participant requested to change subject
					if (sal_custom_header_find(op->getRecvCustomHeaders(), "Subject")) {
						// Handle subject change
						lInfo() << "conference " << getConferenceAddress() << " changed subject to \"" << op->getSubject() << "\"";
						setSubject(op->getSubject());
					}
				}
			}
				break;
			default:
				break;
		}

	}
}

void LocalConference::transferStateChangedCb(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	LocalConference *conf = (LocalConference *)linphone_core_v_table_get_user_data(vtable);
	auto cppCall = Call::toCpp(transfered)->getSharedFromThis();
	if (conf && conf->findParticipantDevice(cppCall->getActiveSession())) {
		lInfo() << "LocalConference::" << __func__ << " not implemented";
	}
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantAdded (creationTime,  isFullState, participant);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) {
	if (getState() != ConferenceInterface::State::TerminationPending) {
		// Increment last notify before notifying participants so that the delta can be calculated correctly
		++lastNotify;
		// Send notify only if it is not in state TerminationPending and:
		// - there is more than one participant in the conference
		// - there is only participant and it didn't have a session towards the conference manager preexisting conference or it left the conference (its session is in PausedByRemote state)
		return Conference::notifyParticipantRemoved (creationTime,  isFullState, participant);
	}

	return nullptr;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantSetAdmin (creationTime,  isFullState, participant, isAdmin);
}

shared_ptr<ConferenceSubjectEvent> LocalConference::notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifySubjectChanged (creationTime, isFullState, subject);
}

shared_ptr<ConferenceAvailableMediaEvent> LocalConference::notifyAvailableMediaChanged (time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyAvailableMediaChanged (creationTime, isFullState, mediaCapabilities);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceAdded (creationTime,  isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	if ((getState() != ConferenceInterface::State::TerminationPending) && (getParticipantCount() >= 2)) {
		++lastNotify;
		// Send notify only if it is not in state TerminationPending and:
		// - there are two or more participants in the conference
		return Conference::notifyParticipantDeviceRemoved (creationTime,  isFullState, participant, participantDevice);
	}
	return nullptr;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceJoined (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceJoined (creationTime, isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceLeft (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceLeft (creationTime, isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceMediaCapabilityChanged (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceMediaCapabilityChanged (creationTime,  isFullState, participant, participantDevice);
}

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const std::shared_ptr<LinphonePrivate::CallSession>& focusSession,
	const ConferenceAddress &confAddr,
	const ConferenceId &conferenceId,
	const std::list<IdentityAddress> &invitees,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, conferenceId.getLocalAddress(), listener, params){

	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
//	lastNotify = 0;

	focus = Participant::create(this, confAddr, focusSession);
	confParams->enableLocalParticipant(false);
	pendingSubject = confParams->getSubject();

	getMe()->setAdmin(true);

	invitedAddresses = invitees;

	setState(ConferenceInterface::State::Instantiated);

	setConferenceId(conferenceId);
	setConferenceAddress(confAddr);

	finalizeCreation();
}

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &focusAddr,
	const ConferenceId &conferenceId,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, conferenceId.getLocalAddress(), listener, params){

	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
//	lastNotify = 0;

	focus = Participant::create(this, focusAddr);
	lInfo() << "Create focus '" << focus->getAddress() << "' from address : " << focusAddr;
	pendingSubject = confParams->getSubject();
	getMe()->setAdmin(true);

	confParams->enableLocalParticipant(false);

	// Store conference ID to retrieve later the local address when the focus call goes to StreamsRunning state
	this->conferenceId = conferenceId;

	setState(ConferenceInterface::State::Instantiated);

}

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const std::shared_ptr<LinphonePrivate::Call>& focusCall,
	const ConferenceId &conferenceId,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, conferenceId.getLocalAddress(), listener, params){

	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
//	lastNotify = 0;

	focus = Participant::create(this, Address(focusCall->getRemoteContact()), focusCall->getActiveSession());
	pendingSubject = confParams->getSubject();
	setConferenceId(conferenceId);

	const auto & conferenceAddress = focus->getSession()->getRemoteContactAddress();

#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb)  {
		const auto & confInfo = mainDb->getConferenceInfoFromURI(ConferenceAddress(*conferenceAddress));
		// me is admin if the organizer is the same as me
		getMe()->setAdmin((confInfo && (confInfo->getOrganizer() == getMe()->getAddress())));
	}
#endif

	setState(ConferenceInterface::State::Instantiated);

	setConferenceAddress(*conferenceAddress);

	finalizeCreation();
}

RemoteConference::~RemoteConference () {
	terminate();
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM
}

void RemoteConference::finalizeCreation() {

	if (getState() == ConferenceInterface::State::CreationPending) {
		if (finalized) {
			lDebug() << "Conference " << this << " has already been finalized";
			return;
		} else if (focus->getSession() && (!static_pointer_cast<MediaSession>(focus->getSession())->mediaInProgress() || !!!linphone_config_get_int(linphone_core_get_config(getCore()->getCCore()), "sip", "update_call_when_ice_completed", TRUE))) {
			finalized = true;
			addListener(std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this), [](ConferenceListenerInterface * p){}));
		#ifdef HAVE_ADVANCED_IM
			bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
			if (eventLogEnabled) {
				eventHandler = std::make_shared<RemoteConferenceEventHandler>(this, this);
				eventHandler->subscribe(getConferenceId());
			}
		#endif // HAVE_ADVANCED_IM
		}
	} else {
		lError() << "Cannot finalize creation of Conference in state " << getState();
	}
}

#ifdef HAVE_DB_STORAGE
std::shared_ptr<ConferenceInfo> RemoteConference::createOrGetConferenceInfo() const {
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		std::shared_ptr<ConferenceInfo> conferenceInfo = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(getConferenceAddress());
		if (conferenceInfo) {
			return conferenceInfo;
		}
	}

	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const auto referer = (session ? L_GET_PRIVATE(session->getMediaParams())->getReferer() : nullptr);
	const auto organizer = (referer) ? IdentityAddress(*referer->getRemoteAddress()) : getMe()->getAddress();
	return createConferenceInfo(organizer);
}
#endif // HAVE_DB_STORAGE

void RemoteConference::setMainSession(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	if (focus) {
		focus->setSession(session);
	}
}

const std::shared_ptr<CallSession> RemoteConference::getMainSession() const {
	return focus ? focus->getSession() : nullptr;
}

int RemoteConference::startRecording (const char *path) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session){
		session->setRecordPath(path);
		session->startRecording();
	}else{
		lError() << "RemoteConference::startRecording(): no audio session.";
		return -1;
	}
	return 0;
}

int RemoteConference::stopRecording () {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session){
		session->stopRecording();
	}else{
		lError() << "RemoteConference::stopRecording(): no audio session.";
		return -1;
	}
	return 0;
}

bool RemoteConference::isRecording () const {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session){
		return session->isRecording();
	}
	return false;
}

void RemoteConference::notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) {
	// Call callbacks before calling listeners because listeners may change state
	linphone_core_notify_conference_state_changed(getCore()->getCCore(), toC(), (LinphoneConferenceState)getState());

	Conference::notifyStateChanged (state);
}

int RemoteConference::inviteAddresses (const list<const LinphoneAddress *> &addresses, const LinphoneCallParams *params) {
	lError() << "RemoteConference::inviteAddresses() not implemented";
	return -1;
}

int RemoteConference::participantDeviceLeft(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	lError() << "RemoteConference::participantDeviceLeft() not implemented";
	return -1;
}

int RemoteConference::participantDeviceLeft(const std::shared_ptr<LinphonePrivate::Participant> &participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	time_t creationTime = time(nullptr);
	notifyParticipantDeviceLeft(creationTime, false, participant, device);
	if (device) {
		_linphone_participant_device_notify_conference_left(device->toC());
		return 0;
	}
	return -1;
}

int RemoteConference::participantDeviceJoined(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	lError() << "RemoteConference::participantDeviceJoined() not implemented";
	return -1;
}

int RemoteConference::participantDeviceJoined(const std::shared_ptr<LinphonePrivate::Participant> &participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	time_t creationTime = time(nullptr);
	notifyParticipantDeviceJoined(creationTime, false, participant, device);
	if (device) {
		_linphone_participant_device_notify_conference_joined(device->toC());
		return 0;
	}
	return -1;
}

int RemoteConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	lError() << "RemoteConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaCapabilityChanged(const IdentityAddress &addr) {
	lError() << "RemoteConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::Participant> &participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	lError() << "RemoteConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session, uint32_t ssrc) {
	lError() << "RemoteConference::participantDeviceSsrcChanged() not implemented";
	return -1;
}

void RemoteConference::setLocalParticipantStreamCapability(const LinphoneMediaDirection & direction, const LinphoneStreamType type) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams * params = session->getMediaParams();
		MediaSessionParams *currentParams = params->clone();
		if (!currentParams->rtpBundleEnabled()) {
			currentParams->enableRtpBundle((getLayout() != ConferenceLayout::Legacy));
		}
		lInfo() << "Setting direction of stream of type " << std::string(linphone_stream_type_to_string(type)) << " to " << std::string(linphone_media_direction_to_string(direction)) << " of main session " << session;
		switch (type) {
			case LinphoneStreamTypeAudio:
				currentParams->enableAudio((direction != LinphoneMediaDirectionInactive) && (direction != LinphoneMediaDirectionInvalid));
				currentParams->setAudioDirection(direction);
				break;
			case LinphoneStreamTypeVideo:
				currentParams->enableVideo((direction != LinphoneMediaDirectionInactive) && (direction != LinphoneMediaDirectionInvalid));
				currentParams->setVideoDirection(direction);
				break;
			case LinphoneStreamTypeText:
				currentParams->enableRealtimeText((direction != LinphoneMediaDirectionInactive) && (direction != LinphoneMediaDirectionInvalid));
				break;
			case LinphoneStreamTypeUnknown:
				lError() << "Unable to set direction of stream of type " << std::string(linphone_stream_type_to_string(type));
				return;
		}
		session->update(currentParams);
		delete currentParams;
	}
}

int RemoteConference::getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> & device) {
	AudioStream *as = getAudioStream();

	if (as != nullptr) {
		return audio_stream_get_participant_volume(as, device->getSsrc());
	}

	return AUDIOSTREAMVOLUMES_NOT_FOUND;
}

bool RemoteConference::addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) {

	bool success = Conference::addParticipantDevice(call);

	if (success) {
		auto device = findParticipantDevice (call->getActiveSession());
		const auto & p = device->getParticipant();

		// In a remote conference, the participant has no session attached ot it.
		device->setSession(nullptr);
		device->setState(ParticipantDevice::State::Present);

		if (device && p) {
			time_t creationTime = time(nullptr);
			notifyParticipantDeviceAdded(creationTime, false, p, device);
		}

	}

	return success;
}

bool RemoteConference::addParticipant (const IdentityAddress &participantAddress) {
	// Search call that matches participant session
	const std::list<std::shared_ptr<Call>> &coreCalls = getCore()->getCalls();
	auto callIt = std::find_if(coreCalls.cbegin(), coreCalls.cend(), [&] (const std::shared_ptr<Call> & c) {
		return (IdentityAddress(*c->getRemoteAddress()) == participantAddress);
	});
	bool ret = false;
	if (callIt != coreCalls.cend()) {
		std::shared_ptr<Call> call = *callIt;
		ret = addParticipant(call);
	}
	return ret;
}

bool RemoteConference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {

#if 0
	if (getMe()->isAdmin() && !isConferenceEnded() && isConferenceStarted()) {
#endif
	if (getMe()->isAdmin()) {
		LinphoneAddress *addr;
		LinphoneCallParams *params;
		std::shared_ptr<LinphonePrivate::Call> focusCall = nullptr;
		switch (state) {
			case ConferenceInterface::State::None:
			case ConferenceInterface::State::Instantiated:
			case ConferenceInterface::State::CreationFailed:
			{
				lInfo() << "Calling the conference focus (" << focus->getAddress() << ")";
				addr = L_GET_C_BACK_PTR(&(focus->getAddress().asAddress()));
				if (!addr)
					return false;
				params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
				// Participant with the focus call is admin
				L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
				linphone_call_params_enable_video(params, confParams->videoEnabled());
				Conference::setSubject(pendingSubject);
				auto focusCallC = linphone_core_invite_address_with_params_2(getCore()->getCCore(), addr, params, L_STRING_TO_C(pendingSubject), nullptr);
				if(focusCallC) {
					focusCall = Call::toCpp(focusCallC)->getSharedFromThis();
					focusCall->setConference(toC());
					focus->setSession(focusCall->getActiveSession());
				}
				m_pendingCalls.push_back(call);
				linphone_call_params_unref(params);
				Conference::addParticipant(call);
			}
				return true;
			case ConferenceInterface::State::CreationPending:
			case ConferenceInterface::State::Created:
				Conference::addParticipant(call);
				if (focus->getSession()) {
					if(focusIsReady())
						transferToFocus(call);
					else
						m_pendingCalls.push_back(call);
				} else {
					lInfo() << "Calling the conference focus (" << focus->getAddress() << ")";
					addr = L_GET_C_BACK_PTR(&(focus->getAddress().asAddress()));
					if (!addr)
						return false;
					params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
					// Participant with the focus call is admin
					L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
					linphone_call_params_enable_video(params, confParams->videoEnabled());
					Conference::setSubject(pendingSubject);
					focusCall = Call::toCpp(linphone_core_invite_address_with_params_2(getCore()->getCCore(), addr, params, L_STRING_TO_C(pendingSubject), nullptr))->getSharedFromThis();
					focusCall->setConference(toC());
					focus->setSession(focusCall->getActiveSession());
					m_pendingCalls.push_back(call);
					linphone_call_params_unref(params);
				}
				return true;
			default:
				lError() << "Could not add call " << call << " to the conference. Bad conference state (" << Utils::toString(state) << ")";
				return false;
		}
	} else if (!getMe()->isAdmin()) {
		lError() << "Could not add call " << call << " to the conference because local participant " << getMe()->getAddress() << " is not admin.";
	} else {
		const auto & endTime = confParams->getEndTime();
		const auto & startTime = confParams->getStartTime();
		const auto now = time(NULL);
		lError() << "Could not add call " << call << " to the conference because the conference is not active right now.";
		if (startTime >= 0) {
			lError() << "Expected start time (" << startTime << "): " << ctime(&startTime);
		} else {
			lError() << "Expected start time: none";
		}
		if (endTime >= 0) {
			lError() << "Expected end time (" << endTime << "): " << ctime(&endTime);
		} else {
			lError() << "Expected end time: none";
		}
		lError() << "Now: " << ctime(&now);
		return false;
	}
	return false;
}

bool RemoteConference::finalizeParticipantAddition (std::shared_ptr<LinphonePrivate::Call> call) {
	lError() << "RemoteConference::finalizeParticipantAddition() not implemented";
	return false;
}

//Removes own address and existing participants from the list.
//Also removes gruu from kept addresses
list<IdentityAddress> RemoteConference::cleanAddressesList (const list<IdentityAddress> &addresses) const {
	list<IdentityAddress> cleanedList(addresses);

	cleanedList.sort();
	cleanedList.unique();
	for (auto it = cleanedList.begin(); it != cleanedList.end();) {
		if (findParticipant(*it) || (getMe()->getAddress() == *it)) {
			it = cleanedList.erase(it);
		} else {
			it++;
		}
	}
	return cleanedList;
}

bool RemoteConference::addParticipants (const std::list<std::shared_ptr<Call>> &calls) {
	const auto & coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);
	const auto & outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto & inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

#if 0
	if (getMe()->isAdmin() && !isConferenceEnded() && isConferenceStarted()) {
#endif
	if (getMe()->isAdmin()) {
		bool success = Conference::addParticipants(calls);
		// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change audio route to keep the one currently used
		// Do not change audio route if participant addition is not successful
		if (success && startingConference) {
			if (outputDevice) {
				setOutputAudioDevice(outputDevice);
			}
			if (inputDevice) {
				setInputAudioDevice(inputDevice);
			}
		}
	}

	return false;
}

bool RemoteConference::addParticipants (const list<IdentityAddress> &addresses) {
#if 0
	if (getMe()->isAdmin() && !isConferenceEnded()) {
#endif
	if (getMe()->isAdmin()) {
		if ((state == ConferenceInterface::State::Instantiated) || (state == ConferenceInterface::State::CreationPending)) {
			getCore()->createConferenceOnServer(confParams, getMe()->getAddress(), addresses);
		} else {
			SalReferOp *referOp = new SalReferOp(getCore()->getCCore()->sal.get());
			LinphoneAddress *lAddr = L_GET_C_BACK_PTR(&(getConferenceAddress().asAddress()));
			linphone_configure_op(getCore()->getCCore(), referOp, lAddr, nullptr, true);
			for (const auto &addr : addresses) {
				Address referToAddr = addr.asAddress();
				referToAddr.setParam("isfocus");
				referOp->sendRefer(referToAddr.getInternalAddress());
			}
			referOp->unref();
		}
	} else {
		const auto & endTime = confParams->getEndTime();
		const auto now = time(NULL);
		lError() << "Could not add participants to the conference because local participant " << getMe()->getAddress() << " is not admin or conference already ended (expected endtime: " << asctime(gmtime(&endTime)) << " now: " << asctime(gmtime(&now));
		return false;
	}

	return true;
}

int RemoteConference::removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	if (getMe()->isAdmin()) {
		if (p) {
			return removeParticipant(p);
		}
	} else {
		lError() << "Unable to remove participant " << p->getAddress().asString() << " because focus " << getMe()->getAddress().asString() << " is not admin";
	}
	return -1;
}

bool RemoteConference::removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	if (getMe()->isAdmin()) {
		return (bool)removeParticipant(participant->getAddress());
	} else {
		lError() << "Unable to remove participant " << participant->getAddress().asString() << " because focus " << getMe()->getAddress().asString() << " is not admin";
	}
	return false;
}

int RemoteConference::removeParticipant (const IdentityAddress &addr) {
	auto session = getMainSession();
	if (getMe()->isAdmin()) {
		std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
		if (p) {
			switch (state) {
				case ConferenceInterface::State::Created:
				case ConferenceInterface::State::TerminationPending:
				{
					if(!findParticipant(addr)) {
						lError() << "Conference: could not remove participant \'" << addr << "\': not in the participants list";
						return -1;
					}
					LinphoneCore *cCore = getCore()->getCCore();
					SalReferOp *referOp = new SalReferOp(cCore->sal.get());
					LinphoneAddress *lAddr = linphone_address_new(session->getRemoteContact().c_str());
					linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
					linphone_address_unref(lAddr);
					Address referToAddr = addr.asAddress();
					referToAddr.setMethodParam("BYE");
					auto res = referOp->sendRefer(referToAddr.getInternalAddress());
					referOp->unref();

					if (res == 0)
						return Conference::removeParticipant(p);
					else {
						lError() << "Conference: could not remove participant \'" << addr << "\': REFER with BYE has failed";
						return -1;
					}
				}
					break;
				default:
					lError() << "Could not remove participant " << addr << " from conference " << getConferenceAddress() << ". Bad conference state (" << Utils::toString(state) << ")";
					return -1;
			}
		} else {
			lWarning() << "Unable to remove participant " << addr.asString() << " because it is not part of the conference " << getConferenceAddress();
		}
	} else {
		lWarning() << "Unable to remove participant " << addr.asString() << " because local participant " << getMe()->getAddress().asString() << " is not admin.";
	}
	return -1;
}

int RemoteConference::terminate () {
	auto savedState = state;
	auto session = getMainSession();
	shared_ptr<Call> sessionCall = nullptr;
	if (session) {
		auto op = session->getPrivate()->getOp();
		sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	switch (savedState) {
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::CreationFailed:
			if (sessionCall) {
				// Conference will be deleted by terminating the session
				session->terminate();
				sessionCall->setConference(nullptr);
				return 0;
			}
			break;
		default:
			break;
	}

	if ((state != ConferenceInterface::State::Terminated) && (state != ConferenceInterface::State::Deleted)) {
		setState(ConferenceInterface::State::TerminationPending);
	}
	return 0;
}

int RemoteConference::enter () {
	const auto & conferenceAddress = getConferenceAddress();
	if (!conferenceAddress.isValid()) {
		lError() << "Could not enter in the conference because its conference address (" << conferenceAddress << ") is not valid";
		return -1;
	}

	auto session = static_pointer_cast<MediaSession>(getMainSession());

	if (session) {
		LinphoneCallState callState = static_cast<LinphoneCallState>(session->getState());
		switch (callState) {
			case LinphoneCallStreamsRunning:
				break;
			case LinphoneCallPaused:
				session->resume();
				participantDeviceJoined(me, me->getDevices().front());
				break;
			default:
				lError() << "Could not join the conference: bad focus call state (" << Utils::toString(callState) << ")";
				return -1;
		}
	} else {
		/* Start a new call by indicating that it has to be put into the conference directly */
		LinphoneCallParams *new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
		linphone_call_params_enable_video(new_params, confParams->videoEnabled());

		linphone_call_params_set_in_conference(new_params, FALSE);

		L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContactParameter("admin", Utils::toString(getMe()->isAdmin()));

		const Address & address = getConferenceAddress().asAddress();
		const string & confId = address.getUriParamValue("conf-id");
		linphone_call_params_set_conference_id(new_params, confId.c_str());

		std::string subject = getMe()->isAdmin() ? getSubject() : std::string();

		auto cCall = linphone_core_invite_address_with_params_2(getCore()->getCCore(), L_GET_C_BACK_PTR(&address), new_params, L_STRING_TO_C(subject), nullptr);

		linphone_call_params_unref(new_params);

		auto cppCall = Call::toCpp(cCall);
		cppCall->setConference(toC());
		focus->setSession(cppCall->getActiveSession());
	}
	return 0;
}

void RemoteConference::leave () {
	if (state != ConferenceInterface::State::Created) {
		lError() << "Could not leave the conference: bad conference state (" << Utils::toString(state) << ")";
	}

	auto session = static_pointer_cast<MediaSession>(getMainSession());
	LinphoneCallState callState = static_cast<LinphoneCallState>(session->getState());
	switch (callState) {
		case LinphoneCallPaused:
			lInfo() << getMe()->getAddress() << " is leaving conference " << getConferenceAddress() << " while focus call is paused.";
			break;
		case LinphoneCallStreamsRunning:
			lInfo() << getMe()->getAddress() << " is leaving conference " << getConferenceAddress() << ". Focus call is going to be paused.";
			session->pause();
			participantDeviceLeft(me, me->getDevices().front());
			break;
		default:
			lError() << getMe()->getAddress() << " cannot leave conference " << getConferenceAddress() << " because focus call is in state " << linphone_call_state_to_string(callState);
	}
}

bool RemoteConference::isIn () const {
	if (state != ConferenceInterface::State::Created)
		return false;
	auto session = getMainSession();
	if (!session) return false;
	LinphoneCallState callState = static_cast<LinphoneCallState>(session->getState());
	auto focusContactAddress = Address(session->getRemoteContact());
	return ((callState == LinphoneCallUpdatedByRemote) || (callState == LinphoneCallUpdating) || (callState == LinphoneCallStreamsRunning)) && focusContactAddress.hasUriParam("conf-id");
}

bool RemoteConference::focusIsReady () const {
	LinphoneCallState focusState;
	auto session = getMainSession();
	if (!session || !session->getRemoteContactAddress())
		return false;
	focusState = static_cast<LinphoneCallState>(session->getState());
	return (focusState == LinphoneCallStreamsRunning) || (focusState == LinphoneCallPaused);
}

bool RemoteConference::transferToFocus (std::shared_ptr<LinphonePrivate::Call> call) {
	auto session = getMainSession();
	Address referToAddr(*session->getRemoteContactAddress());
	std::shared_ptr<Participant> participant = findParticipant(call->getActiveSession());
	if (participant) {
		referToAddr.setParam("admin", Utils::toString(participant->isAdmin()));
		const auto & remoteAddress = call->getRemoteAddress();
		lInfo() << "Transfering call (local address " << call->getLocalAddress().asString() << " remote address " <<  (remoteAddress ? remoteAddress->asString() : "Unknown") << ") to focus " << referToAddr;
		if (call->transfer(referToAddr.asString()) == 0) {
			m_transferingCalls.push_back(call);
			return true;
		} else {
			lError() << "Conference: could not transfer call " << call << " to " << referToAddr.asString();
			return false;
		}
	} else {
		lError() << "Conference: could not transfer call " << call << " to " << referToAddr.asString() << " because participant with session " << call->getActiveSession() << " cannot be found  - guessed address " << referToAddr;
		return false;
	}
	return false;
}

void RemoteConference::reset () {
	auto session = getMainSession();
	if(session) {
		auto op = session->getPrivate()->getOp();
		auto sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
		if (sessionCall) sessionCall->setConference(nullptr);
	}
	m_pendingCalls.clear();
	m_transferingCalls.clear();
}

void RemoteConference::onFocusCallStateChanged (LinphoneCallState state) {
	auto session = getMainSession();
	Address focusContactAddress;
	std::shared_ptr<Call> call = nullptr;

	ConferenceId confId = getConferenceId();
	Address peerAddress(confId.getPeerAddress().asAddress());

	if (session) {
		focusContactAddress = Address(session->getRemoteContact());
		auto op = session->getPrivate()->getOp();
		call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it;
	switch (state) {
		case LinphoneCallConnected:
		case LinphoneCallUpdatedByRemote:
			if (session && (getState() == ConferenceInterface::State::Instantiated) && (focusContactAddress.hasUriParam("conf-id"))) {
				setConferenceAddress(focusContactAddress);
				if (!peerAddress.hasUriParam("conf-id")) {
					it = m_pendingCalls.begin();
					while (it != m_pendingCalls.end()) {
						std::shared_ptr<LinphonePrivate::Call> pendingCall = *it;
						LinphoneCallState pendingCallState = static_cast<LinphoneCallState>(pendingCall->getState());
						if ((pendingCallState == LinphoneCallStreamsRunning) || (pendingCallState == LinphoneCallPaused)) {
							it = m_pendingCalls.erase(it);
							transferToFocus(pendingCall);
						} else
							it++;
					}

					setConferenceId(ConferenceId(ConferenceAddress(focusContactAddress), getConferenceId().getLocalAddress()));
					if (call) {
						call->setConferenceId(focusContactAddress.getUriParamValue("conf-id"));
						if (!call->mediaInProgress() || !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()), "sip", "update_call_when_ice_completed", TRUE)){
							finalizeCreation();
						}
					}
				}
			}
			break;
		case LinphoneCallError:
			setState(ConferenceInterface::State::CreationFailed);
			if (call) call->setConference(nullptr);
			it = m_pendingCalls.begin();
			while (it != m_pendingCalls.end()) {
				std::shared_ptr<LinphonePrivate::Call> pendingCall = *it;
				pendingCall->setConference(nullptr);
				it++;
			}
			break;
		case LinphoneCallEnd:
			setState(ConferenceInterface::State::TerminationPending);
			break;
		default:
			break;
	}
}

void RemoteConference::onPendingCallStateChanged (std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState callState) {
	auto session = getMainSession();
	switch (callState) {
		case LinphoneCallStreamsRunning:
		case LinphoneCallPaused:
		{
			if (session) {
				CallSession::State focusCallState = session->getState();
				// Transfer call to focus is the conference is in creation pending or created state and if the focus call is in connected, streams running or updated by remote state
				if (((focusCallState == CallSession::State::StreamsRunning) || (focusCallState == CallSession::State::Connected) || (focusCallState == CallSession::State::UpdatedByRemote)) && ((getState() == ConferenceInterface::State::CreationPending) || (state == ConferenceInterface::State::Created))) {
					// Transfer call only if focus call remote contact address is available (i.e. the call has been correctly established and passed through state StreamsRunning)
					if (!session->getRemoteContactAddress()) {
						m_pendingCalls.remove(call);
						transferToFocus(call);
					}
				}
			}
		}
			break;
		case LinphoneCallError:
		case LinphoneCallEnd:
			m_pendingCalls.remove(call);
			Conference::removeParticipant(call);
			if ((participants.size() + m_pendingCalls.size() + m_transferingCalls.size()) == 0)
				terminate();
			break;
		default:
			break;
	}
}

void RemoteConference::onTransferingCallStateChanged (std::shared_ptr<LinphonePrivate::Call> transfered, LinphoneCallState newCallState) {
	switch (newCallState) {
		case LinphoneCallConnected:
			m_transferingCalls.push_back(transfered);
			break;
		case LinphoneCallError:
			m_transferingCalls.remove(transfered);
			Conference::removeParticipant(transfered);
			if ((participants.size() + m_pendingCalls.size() + m_transferingCalls.size()) == 0)
				terminate();
			break;
		default:
			break;
	}
}

void RemoteConference::callStateChangedCb (LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	if (Call::toCpp(call)->getActiveSession() == conf->getMainSession())
		conf->onFocusCallStateChanged(cstate);
	else {
		list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(conf->m_pendingCalls.begin(), conf->m_pendingCalls.end(), Call::toCpp(call)->getSharedFromThis());
		if (it != conf->m_pendingCalls.end())
			conf->onPendingCallStateChanged(Call::toCpp(call)->getSharedFromThis(), cstate);
	}
}

void RemoteConference::transferStateChangedCb (LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(conf->m_transferingCalls.begin(), conf->m_transferingCalls.end(), Call::toCpp(transfered)->getSharedFromThis());
	if (it != conf->m_transferingCalls.end())
		conf->onTransferingCallStateChanged(Call::toCpp(transfered)->getSharedFromThis(), new_call_state);
}

AudioControlInterface * RemoteConference::getAudioControlInterface() const{
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	return ms->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
}

VideoControlInterface * RemoteConference::getVideoControlInterface() const{
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	return ms->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
}

AudioStream *RemoteConference::getAudioStream(){
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	MS2AudioStream *stream = ms->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return stream ? (AudioStream*)stream->getMediaStream() : nullptr;
}

void RemoteConference::multipartNotifyReceived (const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->multipartNotifyReceived(content);
	}
#else
	lInfo() << "Advanced IM such as conferencing events is disabled!";
#endif // HAVE_ADVANCED_IM
}

void RemoteConference::notifyReceived (const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->notifyReceived(content);
	}
#else
	lInfo() << "Advanced IM such as conferencing events is disabled!";
#endif // HAVE_ADVANCED_IM
}

void RemoteConference::onStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	auto session = getMainSession();
	string subject = getSubject();

	shared_ptr<Call> sessionCall = nullptr;
	if (session) {
		auto op = session->getPrivate()->getOp();
		sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	switch(state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::TerminationFailed:
		case ConferenceInterface::State::Terminated:
			break;
		case ConferenceInterface::State::CreationFailed:
			reset();
			Conference::terminate();
			break;
		case ConferenceInterface::State::Created:
			if (session && getMe()->isAdmin() && (subject.compare(pendingSubject) != 0)) {
				lInfo() << "Updating main session to set conference subject " << subject;
				session->update(nullptr, CallSession::UpdateMethod::Default, subject);
			}
			break;
		case ConferenceInterface::State::TerminationPending:
			#ifdef HAVE_ADVANCED_IM
			if (eventHandler) {
				eventHandler->unsubscribe();
			}
			#endif // HAVE_ADVANCED_IM
			resetLastNotify();
			if (session) {
				auto call = getCore()->getCallByRemoteAddress(*session->getRemoteAddress());

				// Do not terminate focus call when terminating the remote conference
				// This is required because the local conference creates a remote conference for every participant and the call from the participant to the local conference is the focus call
				if (call) {
					call->setConference(nullptr);
				}
			}
			Conference::terminate();
			setState(ConferenceInterface::State::Terminated);
			break;
			break;
		case ConferenceInterface::State::Deleted:
			reset();
			break;
	}

}

void RemoteConference::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin == participant->isAdmin())
		return;

	if (!getMe()->isAdmin()) {
		lError() << "Unable to set admin status of participant " << participant->getAddress().asString() << " to " << (isAdmin ? "true" : "false") << " because focus " << getMe()->getAddress().asString() << " is not admin";
		return;
	}

	LinphoneCore *cCore = getCore()->getCCore();
	auto session = getMainSession();

	SalReferOp *referOp = new SalReferOp(cCore->sal.get());
	LinphoneAddress *lAddr = linphone_address_new(session->getRemoteContact().c_str());
	linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
	linphone_address_unref(lAddr);
	Address referToAddr = participant->getAddress().asAddress();
	referToAddr.setParam("admin", Utils::toString(isAdmin));
	referOp->sendRefer(referToAddr.getInternalAddress());
	referOp->unref();
}

void RemoteConference::setSubject (const std::string &subject) {
	if (!getMe()->isAdmin()) {
		lError() << "Unable to update conference subject because focus " << getMe()->getAddress().asString() << " is not admin";
		return;
	}

	auto session = getMainSession();
	if (session) {

		if (subject.compare(pendingSubject) != 0) {
			pendingSubject = subject;
			Conference::setSubject(subject);
			session->update(nullptr, CallSession::UpdateMethod::Default, subject);
#ifdef HAVE_DB_STORAGE
			updateSubjectInConferenceInfo(subject);
#endif // HAVE_DB_STORAGE
		}
	} else {
		pendingSubject = subject;
		lInfo() << "Unable to update subject right now because the focus session has not been established yet.";
	}


	if (getState() != ConferenceInterface::State::Created) {
		return;
	}
}

bool RemoteConference::update(const LinphonePrivate::ConferenceParamsInterface &newParameters){
	// Any remote participant can change the layout of the conference
	const auto & newLayout = static_cast<const ConferenceParams&>(newParameters).getLayout();
	bool layoutChanged =  confParams && (getLayout() != newLayout);
	bool ret = false;
	if (getMe()->isAdmin()) {
		ret = Conference::update(newParameters);
	} else if (layoutChanged) {
		auto newParametersWithLayoutChanged = ConferenceParams::create(getCurrentParams());
		newParametersWithLayoutChanged->setLayout(newLayout);
		ret = Conference::update(*newParametersWithLayoutChanged);
		updateMainSession();
	} else {
		lError() << "Unable to update conference parameters because focus " << getMe()->getAddress().asString() << " is not admin and layout keeps being " << getLayout();
	}

	return ret;
}

void RemoteConference::onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	const IdentityAddress &pAddr = event->getParticipantAddress();

#ifdef HAVE_DB_STORAGE
	const auto & participantAddress = participant->getAddress();
	updateParticipantsInConferenceInfo(participantAddress);
#endif

	if (isMe(pAddr)) {
	#ifdef HAVE_ADVANCED_IM
		bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
		if (eventLogEnabled && (getState() == ConferenceInterface::State::CreationPending)) {
			if (!eventHandler) {
				eventHandler = std::make_shared<RemoteConferenceEventHandler>(this, this);
			}
			lInfo() << "Subscribing me (address " << pAddr << ") to conference " << getConferenceAddress();
			eventHandler->subscribe(getConferenceId());
		}
	#endif // HAVE_ADVANCED_IM
	} else if (findParticipant(pAddr)) {
		lInfo() << "Addition of participant with address " << pAddr << " to conference " << getConferenceAddress() << " has been successful";
	} else {
		lWarning() << "Addition of participant with address " << pAddr << " has been failed because the participant is not part of the conference" << getConferenceAddress();
	}
}

void RemoteConference::onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	const IdentityAddress &pAddr = event->getParticipantAddress();

	if (isMe(pAddr)) {
		lInfo() << "Unsubscribing all devices of me (address " << pAddr << ") from conference " << getConferenceAddress();
		// Unsubscribe all devices of me
		std::for_each(getMe()->getDevices().cbegin(), getMe()->getDevices().cend(), [&] (const std::shared_ptr<ParticipantDevice> & device) {
			LinphoneEvent * event = device->getConferenceSubscribeEvent();
			if (event) {
				//try to terminate subscription if any, but do not wait for answer.
				LinphoneEventCbs *cbs = linphone_event_get_callbacks(event);
				linphone_event_cbs_set_user_data(cbs, nullptr);
				linphone_event_cbs_set_notify_response(cbs, nullptr);
				linphone_event_terminate(event);
			}
		});
	} else if (!findParticipant(pAddr)) {
		lInfo() << "Removal of participant with address " << pAddr << " from conference " << getConferenceAddress() << " has been successful";
	} else {
		lWarning() << "Removal of participant with address " << pAddr << " has been failed because the participant is still part of the conference" << getConferenceAddress();
	}
}

void RemoteConference::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const MediaSessionParams * params = session->getMediaParams();

	if (confParams->videoEnabled() && params->videoEnabled() && (getLayout() != ConferenceLayout::Legacy)) {
		lInfo() << "Sending re-INVITE in order to get streams for newly added participant device " << device->getAddress();
		updateMainSession();
	}
}

void RemoteConference::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const MediaSessionParams * params = session->getMediaParams();

	if (confParams->videoEnabled() && params->videoEnabled() && (getLayout() != ConferenceLayout::Legacy)) {
		lInfo() << "Sending re-INVITE in order to get streams for recently removed participant device " << device->getAddress();
		updateMainSession();
	}
}

void RemoteConference::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) {
#ifdef HAVE_DB_STORAGE
	updateSubjectInConferenceInfo(event->getSubject());
#endif // HAVE_DB_STORAGE
}

void RemoteConference::onParticipantDeviceMediaAvailabilityChanged (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	if (getLayout() != ConferenceLayout::Legacy) {
		lInfo() << "Sending re-INVITE because device " << device->getAddress() << " has changed its stream availability";
		updateMainSession();
	}
}

void RemoteConference::onFullStateReceived() {
#ifdef HAVE_DB_STORAGE
	// Store into DB after the start incoming notification in order to have a valid conference address being the contact address of the call
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		std::shared_ptr<ConferenceInfo> conferenceInfo = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(getConferenceAddress());

		if (!conferenceInfo) {
			lInfo() << "Inserting conference information to database related to conference " << getConferenceAddress();
			const auto conferenceInfo = createOrGetConferenceInfo();
			mainDb->insertConferenceInfo(conferenceInfo);
			auto  callLog = getMainSession() ? getMainSession()->getLog() : nullptr;
			if (callLog) {
				callLog->setConferenceInfo(conferenceInfo);
			}
		}
	}
#endif // HAVE_DB_STORAGE
	lInfo() << "Sending re-INVITE in order to get streams after joining conference " << getConferenceAddress();
	updateMainSession();
}

}//end of namespace MediaConference

LINPHONE_END_NAMESPACE
