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

#include <mediastreamer2/msvolume.h>

#include "linphone/core.h"

#include "conference_private.h"

#include "sal/refer-op.h"
#include "account/account.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call.h"
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

	// Video is already enable in the conference params constructor
	confParams->enableAudio(true);

}

Conference::~Conference() {
	bctbx_list_free_with_data(mCallbacks, (void(*)(void *))belle_sip_object_unref);
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
			shared_ptr<CallSession> session = getMe()->getSession();
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, "SIP", LinphoneReasonUnknown, 500, "Server internal error", NULL);
			session->decline(ei);
			linphone_error_info_unref(ei);
			setState(ConferenceInterface::State::CreationFailed);
			return;
		}

		LinphonePrivate::Conference::setConferenceAddress(conferenceAddress);

		lInfo() << "The Conference has been given the address " << conferenceAddress.asString();
	} else {
		lError() << "Cannot set the conference address of the Conference in state " << getState();
		return;
	}

}

void Conference::setConferenceId (const ConferenceId &conferenceId) {
	LinphonePrivate::Conference::setConferenceId(conferenceId);
	getCore()->insertAudioVideoConference(getSharedFromThis());
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
		p = Participant::create(this,remoteAddress, call->getActiveSession());
		p->setFocus(false);
		p->setPreserveSession(true);
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
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(call->getActiveSession());
	if (p) {
		const Address * remoteContact = static_pointer_cast<MediaSession>(call->getActiveSession())->getRemoteContactAddress();
		if (remoteContact) {
			// If device is not found, then add it
			if (p->findDevice(*remoteContact, false) == nullptr) {
				lInfo() << "Adding device with address " << remoteContact->asString() << " to participant " << p->getAddress();
				shared_ptr<ParticipantDevice> device = p->addDevice(*remoteContact);
				call->setConference(toC());
				device->setSession(call->getActiveSession());

				time_t creationTime = time(nullptr);
				notifyParticipantDeviceAdded(creationTime, false, p, device);

				lInfo() << "Participant with address " << call->getRemoteAddress()->asString() << " has added device " << remoteContact->asString() << " to conference " << getConferenceAddress();

				return true;
			}
		} else {
			lError() << "Unable to add device to participant with address " << call->getRemoteAddress()->asString() << " in conference " << getConferenceAddress();
		}
	}

	return false;
}

int Conference::removeParticipantDevice(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	if (p) {
		const Address * remoteContact = static_pointer_cast<MediaSession>(session)->getRemoteContactAddress();
		if (remoteContact) {
			std::shared_ptr<ParticipantDevice> device = p->findDevice(*remoteContact);
			// If device is not found, then add it
			if (device != nullptr) {

				LinphoneEvent * event = device->getConferenceSubscribeEvent();
				if (event) {
					//try to terminate subscription if any, but do not wait for anser.
					LinphoneEventCbs *cbs = linphone_event_get_callbacks(event);
					linphone_event_cbs_set_user_data(cbs, nullptr);
					linphone_event_cbs_set_notify_response(cbs, nullptr);
					linphone_event_terminate(event);
				}

				lInfo() << "Removing device with address " << remoteContact->asString() << " to participant " << p.get();
				p->removeDevice(*remoteContact);
				shared_ptr<Call> call = getCore()->getCallByRemoteAddress (*session->getRemoteAddress());
				if (call) {
					call->terminateConference();
				}

				time_t creationTime = time(nullptr);
				notifyParticipantDeviceRemoved(creationTime, false, p, device);

				return 0;
			}
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

	checkIfTerminated();

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
	std::for_each(participant->getDevices().cbegin(), participant->getDevices().cend(), [&] (const std::shared_ptr<ParticipantDevice> & device) {
		LinphoneEvent * event = device->getConferenceSubscribeEvent();
		if (event) {
			//try to terminate subscription if any, but do not wait for answer.
			LinphoneEventCbs *cbs = linphone_event_get_callbacks(event);
			linphone_event_cbs_set_user_data(cbs, nullptr);
			linphone_event_cbs_set_notify_response(cbs, nullptr);
			linphone_event_terminate(event);
		}

		time_t creationTime = time(nullptr);
		notifyParticipantDeviceRemoved(creationTime, false, participant, device);
	});
	participant->clearDevices();
	participants.remove(participant);
	time_t creationTime = time(nullptr);
	notifyParticipantRemoved(creationTime, false, participant);

	bool checkTermination = true;
	auto pSession = participant->getSession();
	// If participant that we are trying to remove from the conference is in paused by remote state, then it temporarely left the conference therefore it must not be terminated
	if (pSession->getState() == LinphonePrivate::CallSession::State::PausedByRemote) {
		checkTermination = false;
	}

	// Detach call from conference
	shared_ptr<Call> pSessionCall = getCore()->getCallByRemoteAddress (*pSession->getRemoteAddress());
	if (pSessionCall) {
		pSessionCall->setConference(nullptr);
	}

	if (checkTermination) {
		checkIfTerminated();
	}

	return true;
}

void Conference::checkIfTerminated() {
	if (getParticipantCount() == 0) {
		if (getState() == ConferenceInterface::State::TerminationPending) {
			setState(ConferenceInterface::State::Terminated);
		} else {
			setState(ConferenceInterface::State::TerminationPending);
		}
	}
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
				mStateChangedCb(toC(), (LinphoneConferenceState)state, mUserData);
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

bctbx_list_t *Conference::getCallbacksList () const {
	return mCallbacks;
}

LinphoneConferenceCbs *Conference::getCurrentCbs () const{
	return mCurrentCbs;
}

void Conference::setCurrentCbs (LinphoneConferenceCbs *cbs) {
	mCurrentCbs = cbs;
}

void Conference::addCallbacks (LinphoneConferenceCbs *cbs) {
	mCallbacks = bctbx_list_append(mCallbacks, belle_sip_object_ref(cbs));
}

void Conference::removeCallbacks (LinphoneConferenceCbs *cbs) {
	mCallbacks = bctbx_list_remove(mCallbacks, cbs);
	belle_sip_object_unref(cbs);
}

void *Conference::getUserData () const{
	return userData;
}

void Conference::setUserData (void *ud) {
	userData = ud;
}

LocalConference::LocalConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, myAddress, listener, params){

#ifdef HAVE_ADVANCED_IM
	bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
	if (eventLogEnabled) {
		eventHandler = std::make_shared<LocalAudioVideoConferenceEventHandler>(this);
		addListener(eventHandler);
	}
#endif // HAVE_ADVANCED_IM

	setState(ConferenceInterface::State::Instantiated);
	mMixerSession.reset(new MixerSession(*core.get()));

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
	char confId[6];
	belle_sip_random_token(confId,sizeof(confId));
	contactAddress.setUriParam("conf-id",confId);
	if (contactAddressStr) {
		ms_free(contactAddressStr);
	}

	setConferenceAddress(contactAddress);
	setState(ConferenceInterface::State::CreationPending);
	getMe()->setAdmin(true);
	getMe()->setFocus(true);
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

void LocalConference::finalizeCreation() {
	if (getState() == ConferenceInterface::State::CreationPending) {
		const ConferenceAddress & conferenceAddress = getConferenceAddress ();
		setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
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
		eventHandler->subscribeReceived(event, false);
	}
#endif // HAVE_ADVANCED_IM
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
	StreamMixer *mixer = mMixerSession->getMixerByType(SalAudio);
	if (mixer) mixer->enableLocalParticipant(true);

	if (confParams->videoEnabled()){
		mixer = mMixerSession->getMixerByType(SalVideo);
		if (mixer){
			mixer->enableLocalParticipant(true);
			VideoControlInterface *vci = getVideoControlInterface();
			if (vci){
				vci->setNativePreviewWindowId(getCore()->getCCore()->preview_window_id);
				vci->setNativeWindowId(getCore()->getCCore()->video_window_id);
			}
		}
	}

	if (!isIn()) {
		confParams->enableLocalParticipant(true);
		time_t creationTime = time(nullptr);
		notifyParticipantAdded(creationTime, false, getMe());
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

int LocalConference::participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(session);
	int success = -1;
	if (p) {
		const Address * remoteContact = session->getRemoteContactAddress();
		if (remoteContact) {
			std::shared_ptr<ParticipantDevice> device = p->findDevice(*remoteContact);
			if (device) {
				success = participantDeviceMediaChanged(p, device);
			} else {
				lWarning() << "Unable to find device with address " << remoteContact->asString() << " among devices of participant " << p->getAddress().asString();
			}
		}
	}
	return success;
}

int LocalConference::participantDeviceMediaChanged(const IdentityAddress &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	int success = -1;
	for (const auto & d : p->getDevices()) {
		success = participantDeviceMediaChanged(p, d);
	}
	return success;
}

int LocalConference::participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	int success = -1;
	if (device->updateMedia()) {
		time_t creationTime = time(nullptr);
		notifyParticipantDeviceMediaChanged(creationTime, false, participant, device);
		return 0;
	}
	return success;
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

bool LocalConference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(call->toC()))) {
		lError() << "Already in conference";
		return false;
	}

	const Address & conferenceAddress = getConferenceAddress().asAddress();
	const string & confId = conferenceAddress.getUriParamValue("conf-id");
	const string & callConfId = call->getConferenceId();

	const auto & coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);

	const auto & outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto & inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	// Add participant only if creation is successful or call was previously part of the conference
	bool canAddParticipant = ((callConfId.compare(confId) == 0) || ((getParticipantCount() == 0) ? (getState() == ConferenceInterface::State::CreationPending) : (getState() == ConferenceInterface::State::Created)));
	if (canAddParticipant) {
		LinphoneCallState state = static_cast<LinphoneCallState>(call->getState());
		bool localEndpointCanBeAdded = false;

		switch(state){
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallIncomingReceived:
			case LinphoneCallPausing:
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
					L_GET_PRIVATE(call->getParams()))->setInConference(true);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
					L_GET_PRIVATE(call->getParams()))->setConferenceId(confId);
			break;
			case LinphoneCallPaused:
			{
				/*
				 * Modifying the MediaSession's params directly is a bit hacky.
				 * However, the resume() method doesn't accept parameters.
				 */
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
					L_GET_PRIVATE(call->getParams()))->setInConference(true);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
					L_GET_PRIVATE(call->getParams()))->setConferenceId(confId);
				const_cast<LinphonePrivate::MediaSessionParams *>(
					call->getParams())->enableVideo(getCurrentParams().videoEnabled());
				// Conference resumes call that previously paused in order to add the participant
				call->resume();
			}
			break;
			case LinphoneCallResuming:
			case LinphoneCallStreamsRunning:
			{
				LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), call->toC());
				linphone_call_params_set_in_conference(params, TRUE);
				linphone_call_params_enable_video(params, getCurrentParams().videoEnabled());
				linphone_call_params_set_conference_id(params, confId.c_str());

				linphone_call_update(call->toC(), params);
				linphone_call_params_unref(params);
				// Add local endpoint if the call was not previously in the conference
				localEndpointCanBeAdded = (callConfId.compare(confId) != 0);
			}
			break;
			default:
				lError() << "Call " << call << " is in state " << Utils::toString(call->getState()) << ", it cannot be added to the conference";
				return false;
			break;
		}

		if (call->toC() == linphone_core_get_current_call(getCore()->getCCore()))
			L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(nullptr);
		mMixerSession->joinStreamsGroup(call->getMediaSession()->getStreamsGroup());
		Conference::addParticipant(call);
		if (localEndpointCanBeAdded){
			/*
			 * This needs to be done at the end, to ensure that the call in StreamsRunning state has released the local
			 * resources (mic and camera), which is done during the joinStreamsGroup() step.
			 */
			enter();

			// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change audio route to keep the one currently used
			if (startingConference) {
				if (outputDevice) {
					setOutputAudioDevice(outputDevice);
				}
				if (inputDevice) {
					setInputAudioDevice(inputDevice);
				}
			}

		}
		setState(ConferenceInterface::State::Created);
		return true;
	}

	lError() << "Unable to add participant to conference " << getConferenceAddress() << " because it is in state " << linphone_conference_state_to_string(static_cast<LinphoneConferenceState>(getState()));
	return false;
}

bool LocalConference::addParticipant (const IdentityAddress &participantAddress) {
	bool success = Conference::addParticipant(participantAddress);
	setState(ConferenceInterface::State::Created);
	enter();
	return success;
}

int LocalConference::removeParticipant (const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession) {
	int err = 0;

	shared_ptr<Call> call = getCore()->getCallByRemoteAddress (*session->getRemoteAddress());
	if (call) {
		if (linphone_call_get_conference(call->toC()) != toC()){
			lError() << "Call " << call->toC() << " is not part of conference " << getConferenceAddress();
			return -1;
		}
	}

	std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(session);
	if (!participant) {
		lError() << "Trying to remove participant " << *session->getRemoteAddress() << " with session " << session << " which is not part of conference " << getConferenceAddress();
		return -1;
	}

	CallSession::State sessionState = session->getState();

	if (getState() != ConferenceInterface::State::TerminationPending) {

		const auto & op = session->getPrivate()->getOp();
		const auto account = linphone_core_lookup_known_account(getCore()->getCCore(), L_GET_C_BACK_PTR(&(me->getAddress().asAddress())));
		char * contactAddressStr = nullptr;
		if (op) {
			contactAddressStr = sal_address_as_string(op->getContactAddress());
		} else if (account && Account::toCpp(account)->getOp()) {
			contactAddressStr = sal_address_as_string(Account::toCpp(account)->getOp()->getContactAddress());
		} else {
			contactAddressStr = ms_strdup(linphone_core_get_identity(getCore()->getCCore()));
		}

		Address contactAddress(contactAddressStr);
		ms_free(contactAddressStr);

		if (participant->getPreserveSession()) {
			// If the session is already paused,then send an update to kick the participant out of the conference, pause the call otherwise
			if (sessionState == CallSession::State::Paused) {
				const MediaSessionParams * params = static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams();
				MediaSessionParams *currentParams = params->clone();
				currentParams->getPrivate()->setInConference(FALSE);
				lInfo() << "Updating call to notify of conference removal.";
				if (contactAddress.hasUriParam("conf-id")) {
					err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->updateFromConference(currentParams);
				}
			} else {
				/* Kick the session out of the conference by moving to the Paused state. */
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
						L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams()))->setInConference(false);

				if (contactAddress.hasUriParam("conf-id")) {
					err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->pauseFromConference();
				}
			}
		} else {
			// Terminate session (i.e. send a BYE) as per RFC
			// This is the default behaviour
			if ((sessionState != LinphonePrivate::CallSession::State::Released) && contactAddress.hasUriParam("conf-id")) {
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->terminate();
			}
		}
		// Detach call from conference
		if (call) {
			call->setConference(nullptr);
		}
	}
	
	// If conference is in termination pending state, all call sessions are about be kicked out of the conference hence unjoin streams
	if (participant) {
		if (participant->isAdmin()) setParticipantAdminStatus(participant, false);
		Conference::removeParticipant(participant);
		mMixerSession->unjoinStreamsGroup(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getStreamsGroup());
	}

	// If conference is in termination pending state, terminate method is already taking care of state kicking participants out of the conference
	// No need to kick out last remainign participant as it will be done soon
	if ((getState() != ConferenceInterface::State::TerminationPending) && (!preserveSession)) {
		/*
		 * Handle the case where only the local participant and a unique remote participant are remaining.
		 * In this case, if the session linked to the participant has to be preserved after the conference, then destroy the conference and let these two participants to connect directly thanks to a simple call.
		 * Indeed, the conference adds latency and processing that is useless to do for 1-1 conversation.
		 */

		if (!confParams->oneParticipantConferenceEnabled() && (getParticipantCount() == 1)) {
			std::shared_ptr<LinphonePrivate::Participant> remainingParticipant = participants.front();
			if (remainingParticipant->isAdmin()) setParticipantAdminStatus(remainingParticipant, false);
			const bool lastParticipantPreserveSession = remainingParticipant->getPreserveSession();
			if (lastParticipantPreserveSession) {

				std::shared_ptr<LinphonePrivate::MediaSession> lastSession = static_pointer_cast<LinphonePrivate::MediaSession>(remainingParticipant->getSession());

				const auto & op = lastSession->getPrivate()->getOp();
				const auto account = linphone_core_lookup_known_account(getCore()->getCCore(), L_GET_C_BACK_PTR(&(me->getAddress().asAddress())));
				char * contactAddressStr = nullptr;
				if (op) {
					contactAddressStr = sal_address_as_string(op->getContactAddress());
				} else if (account && Account::toCpp(account)->getOp()) {
					contactAddressStr = sal_address_as_string(Account::toCpp(account)->getOp()->getContactAddress());
				} else {
					contactAddressStr = ms_strdup(linphone_core_get_identity(getCore()->getCCore()));
				}

				Address contactAddress(contactAddressStr);
				ms_free(contactAddressStr);

				lInfo() << "Participant [" << remainingParticipant << "] with " << lastSession->getRemoteAddress()->asString() << 
					" is our last call in our conference, we will reconnect directly to it.";

				const MediaSessionParams * params = lastSession->getMediaParams();
				// If only one participant is in the conference, the conference is destroyed.
				if (isIn()){
					MediaSessionParams *currentParams = params->clone();
					// If the local participant is in, then an update is sent in order to notify that the call is exiting the conference
					currentParams->getPrivate()->setInConference(FALSE);
					lInfo() << "Updating call to notify of conference removal.";
					if (contactAddress.hasUriParam("conf-id")) {
						err = lastSession->updateFromConference(currentParams);
					} else {
						const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
							L_GET_PRIVATE(params))->setInConference(false);
					}
					delete currentParams;
				} else {
					// If the local participant is not in, the call is paused as the local participant is busy
					const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
							L_GET_PRIVATE(params))->setInConference(false);

					if (contactAddress.hasUriParam("conf-id")) {
						err = lastSession->pauseFromConference();
					}
				}

				setState(ConferenceInterface::State::TerminationPending);

				leave();

				/* invoke removeParticipant() recursively to remove this last participant. */
				bool success = Conference::removeParticipant(remainingParticipant);
				mMixerSession->unjoinStreamsGroup(lastSession->getStreamsGroup());

				// Detach call from conference
				shared_ptr<Call> lastSessionCall = getCore()->getCallByRemoteAddress (*lastSession->getRemoteAddress());
				if (lastSessionCall) {
					lastSessionCall->setConference(nullptr);
				}
				return success;
			}

			bool_t designateAdmin = !linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "allow_conference_without_admin", TRUE );
			// Choose another admin if local participant is not in or it is not admin
			if (designateAdmin && (!isIn() || !getMe()->isAdmin())) {
				chooseAnotherAdminIfNoneInConference();
			}
		}

	}

	// If call that we are trying to remove from the conference is in paused by remote state, then it temporarely left the conference therefore it must not be terminated
	if ((getParticipantCount() == 0) && (sessionState != LinphonePrivate::CallSession::State::PausedByRemote)){
		leave();
		if (getState() == ConferenceInterface::State::TerminationPending) {
			setState(ConferenceInterface::State::Terminated);
		} else {
			setState(ConferenceInterface::State::TerminationPending);
		}
	}
	
	return err;
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
	Conference::setSubject(subject);
	time_t creationTime = time(nullptr);
	notifySubjectChanged(creationTime, false, subject);
}

void LocalConference::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->subscriptionStateChanged(event, state);
	}
#endif // HAVE_ADVANCED_IM
}

int LocalConference::terminate () {
	setState(ConferenceInterface::State::TerminationPending);
	for (auto participant : participants){
		for (const auto & device : participant->getDevices()) {
			std::shared_ptr<LinphonePrivate::MediaSession> session = static_pointer_cast<LinphonePrivate::MediaSession>(device->getSession());
			if (session) {
				lInfo() << "Terminating session of participant " << *participant;
				session->terminate();
			}
		}
	}

	return 0;
}

int LocalConference::enter () {
	if (linphone_core_sound_resources_locked(getCore()->getCCore()))
		return -1;
	if (linphone_core_get_current_call(getCore()->getCCore()))
		linphone_call_pause(linphone_core_get_current_call(getCore()->getCCore()));

	addLocalEndpoint();

	return 0;
}

void LocalConference::removeLocalEndpoint () {
	confParams->enableLocalParticipant(false);
	mMixerSession->enableLocalParticipant(false);

	time_t creationTime = time(nullptr);
	notifyParticipantRemoved(creationTime, false, getMe());
}

void LocalConference::leave () {
	if (isIn()) {
		lInfo() << getMe()->getAddress() << " is leaving conference " << getConferenceAddress();
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
	// Update media conference parameter here in order to properly
	if (newConfParams.videoEnabled() != previousVideoEnablement){
		lInfo() << "LocalConference::update(): checking participants...";
		for (auto participant : participants){
			std::shared_ptr<LinphonePrivate::MediaSession> session = static_pointer_cast<LinphonePrivate::MediaSession>(participant->getSession());

			if (session){

				const MediaSessionParams * params = session->getMediaParams();
				MediaSessionParams *currentParams = params->clone();

				if ((!!currentParams->videoEnabled()) != newConfParams.videoEnabled()){
					lInfo() << "Re-INVITing participant " << participant->getAddress().asString() << " to " << (newConfParams.videoEnabled() ? "start" : "stop") << " video.";
					currentParams->enableVideo(newConfParams.videoEnabled());
					session->update(currentParams);
				}
			}
		}
	}
	// Update endpoints only if audio or video settings have changed
	if ((newConfParams.videoEnabled() != previousVideoEnablement) || (newConfParams.audioEnabled() != previousAudioEnablement)){
		/* Don't forget the local participant. For simplicity, a removeLocalEndpoint()/addLocalEndpoint() does the job. */
		removeLocalEndpoint();
		addLocalEndpoint();
	}
	if ((newConfParams.chatEnabled() != previousChatEnablement) || (newConfParams.videoEnabled() != previousVideoEnablement) || (newConfParams.audioEnabled() != previousAudioEnablement)){
		std::map<ConferenceMediaCapabilities, bool> mediaCapabilities;
		mediaCapabilities[ConferenceMediaCapabilities::Audio] = newConfParams.audioEnabled();
		mediaCapabilities[ConferenceMediaCapabilities::Video] = newConfParams.videoEnabled();
		mediaCapabilities[ConferenceMediaCapabilities::Text] = newConfParams.chatEnabled();
		time_t creationTime = time(nullptr);
		notifyAvailableMediaChanged(creationTime, false, mediaCapabilities);
	}
	return ret;
}

int LocalConference::startRecording (const char *path) {
	MS2AudioMixer * mixer = dynamic_cast<MS2AudioMixer*> (mMixerSession->getMixerByType(SalAudio));
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
	AudioControlInterface * aci = dynamic_cast<MS2AudioMixer*> (mMixerSession->getMixerByType(SalAudio));
	if (aci){
		aci->stopRecording();
	}else{
		lError() << "LocalConference::stopRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

bool LocalConference::isIn() const{
	return confParams->localParticipantEnabled();
}

AudioControlInterface *LocalConference::getAudioControlInterface()const{
	return dynamic_cast<AudioControlInterface*>(mMixerSession->getMixerByType(SalAudio));
}

VideoControlInterface * LocalConference::getVideoControlInterface() const{
	return dynamic_cast<VideoControlInterface*>(mMixerSession->getMixerByType(SalVideo));
}

AudioStream *LocalConference::getAudioStream(){
	MS2AudioMixer *mixer = dynamic_cast<MS2AudioMixer*>(mMixerSession->getMixerByType(SalAudio));
	return mixer ? mixer->getAudioStream() : nullptr;
}

void LocalConference::notifyFullState () {
	++lastNotify;
	Conference::notifyFullState();
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

	bool preserveSession = true;
	auto participantIt = std::find_if(participants.cbegin(), participants.cend(), [&] (const std::shared_ptr<Participant> & p) {
	return (p->getSession() != participant->getSession());
	});
	if (participantIt != participants.cend()) {
		const std::shared_ptr<Participant> & p = *participantIt;
		preserveSession = p->getPreserveSession();
	}

	// Increment last notify before notifying participants so that the delta can be calculated correctly
	if ((getState() != ConferenceInterface::State::TerminationPending) && ((getParticipantCount() > 2) || ((getParticipantCount() == 2) && ((participant->getDevices().empty() == false) || !preserveSession)))) {
		++lastNotify;
		// Send notify only if it is not in state TerminationPending and:
		// - there are more than two participants in the conference
		// - there are two participants and the list of devices of the current participant is not empty
		// - there are two participants and the remaining participant didn't have a session towards the conference manager preexisting conference
		return Conference::notifyParticipantDeviceRemoved (creationTime,  isFullState, participant, participantDevice);
	}
	return nullptr;
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceMediaChanged (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceMediaChanged (creationTime,  isFullState, participant, participantDevice);
}

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &focus,
	const ConferenceId &conferenceId,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, conferenceId.getLocalAddress(), listener, params){

	setState(ConferenceInterface::State::Instantiated);

	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
//	lastNotify = 0;

	m_focusContact = nullptr;
	m_focusCall = nullptr;
	m_coreCbs = nullptr;
	m_coreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(m_coreCbs, callStateChangedCb);
	linphone_core_cbs_set_transfer_state_changed(m_coreCbs, transferStateChanged);
	linphone_core_cbs_set_user_data(m_coreCbs, this);
	_linphone_core_add_callbacks(getCore()->getCCore(), m_coreCbs, TRUE);

	getMe()->setAdmin(true);

	confParams->enableLocalParticipant(false);

	// Store conference ID to retrieve later the local address when the focus call goes to StreamsRunning state
	this->conferenceId = conferenceId;
	setConferenceAddress(focus);
}

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const std::shared_ptr<LinphonePrivate::Call>& focusCall,
	const ConferenceId &conferenceId,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, conferenceId.getLocalAddress(), listener, params){

	setState(ConferenceInterface::State::Instantiated);

	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
//	lastNotify = 0;

	m_focusContact = ms_strdup(focusCall->getRemoteContact().c_str());

	m_focusCall = focusCall;
	m_coreCbs = nullptr;
	m_coreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(m_coreCbs, callStateChangedCb);
	linphone_core_cbs_set_transfer_state_changed(m_coreCbs, transferStateChanged);
	linphone_core_cbs_set_user_data(m_coreCbs, this);
	_linphone_core_add_callbacks(getCore()->getCCore(), m_coreCbs, TRUE);

	setConferenceId(conferenceId);

	setConferenceAddress(Address(m_focusContact));
	setState(ConferenceInterface::State::CreationPending);
	finalizeCreation();
}

RemoteConference::~RemoteConference () {
	terminate();
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM

	ms_free(m_focusContact);

	linphone_core_remove_callbacks(getCore()->getCCore(), m_coreCbs);
	linphone_core_cbs_unref(m_coreCbs);
}

void RemoteConference::finalizeCreation() {
	if (getState() == ConferenceInterface::State::CreationPending) {

		addListener(std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this), [](ConferenceListenerInterface * p){}));
	#ifdef HAVE_ADVANCED_IM
		bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
		if (eventLogEnabled) {
			eventHandler = std::make_shared<RemoteConferenceEventHandler>(this, this);
			eventHandler->subscribe(getConferenceId());
		}
	#endif // HAVE_ADVANCED_IM
	} else {
		lError() << "Cannot finalize creation of Conference in state " << getState();
	}
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

int RemoteConference::participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) {
	lError() << "RemoteConference::participantDeviceMediaChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaChanged(const IdentityAddress &addr) {
	lError() << "RemoteConference::participantDeviceMediaChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::Participant> &participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	lError() << "RemoteConference::participantDeviceMediaChanged() not implemented";
	return -1;
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
	if (getMe()->isAdmin()) {
		LinphoneAddress *addr;
		LinphoneCallParams *params;
		LinphoneCallLog *callLog;
		switch (state) {
			case ConferenceInterface::State::None:
			case ConferenceInterface::State::Instantiated:
			case ConferenceInterface::State::CreationFailed:
				lInfo() << "Calling the conference focus (" << getConferenceAddress() << ")";
				addr = L_GET_C_BACK_PTR(&(getConferenceAddress().asAddress()));
				if (!addr)
					return false;
				params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
				// Participant with the focus call is admin
				L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
				linphone_call_params_enable_video(params, confParams->videoEnabled());
				m_focusCall = Call::toCpp(linphone_core_invite_address_with_params(getCore()->getCCore(), addr, params))->getSharedFromThis();
				m_focusCall->setConference(toC());
				m_pendingCalls.push_back(call);
				callLog = m_focusCall->getLog();
				callLog->was_conference = TRUE;
				linphone_call_params_unref(params);
				setState(ConferenceInterface::State::CreationPending);
				Conference::addParticipant(call);
				return true;
			case ConferenceInterface::State::CreationPending:
				Conference::addParticipant(call);
				if(focusIsReady())
					transferToFocus(call);
				else
					m_pendingCalls.push_back(call);
				return true;
			case ConferenceInterface::State::Created:
				Conference::addParticipant(call);
				transferToFocus(call);
				return true;
			default:
				lError() << "Could not add call " << call << " to the conference. Bad conference state (" << Utils::toString(state) << ")";
				return false;
		}
	} else {
		lError() << "Could not add call " << call << " to the conference because local participant " << getMe()->getAddress() << " is not admin";
		return false;
	}
	return false;
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
	if (getMe()->isAdmin()) {
		Address refer_to_addr;
		int res;
		std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
		if (p) {
			switch (state) {
				case ConferenceInterface::State::Created:
				case ConferenceInterface::State::TerminationPending:
					if(!findParticipant(addr)) {
						lError() << "Conference: could not remove participant \'" << addr << "\': not in the participants list";
						return -1;
					}
					refer_to_addr = addr.asAddress();
					linphone_address_set_method_param(L_GET_C_BACK_PTR(&refer_to_addr), "BYE");
					res = m_focusCall->getOp()->refer(refer_to_addr.asString().c_str());
					if (res == 0)
						return Conference::removeParticipant(p);
					else {
						lError() << "Conference: could not remove participant \'" << addr << "\': REFER with BYE has failed";
						return -1;
					}
				default:
					lError() << "Could not remove participant " << addr << " from conference " << getConferenceAddress() << ". Bad conference state (" << Utils::toString(state) << ")";
					return -1;
			}
		} else {
			lWarning() << "Unable to remove participant " << addr.asString() << " because it is not part of the conference " << getConferenceAddress();
		}
	} else {
		lWarning() << "Unable to remove participant " << addr.asString() << " because focus " << getMe()->getAddress().asString() << " is not admin";
	}
	return -1;
}

int RemoteConference::terminate () {
	switch (state) {
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::CreationFailed:
			if (m_focusCall) {
				m_focusCall->setConference(nullptr);
				m_focusCall->terminate();
			}
			setState(ConferenceInterface::State::TerminationPending);
			break;
		case ConferenceInterface::State::TerminationPending:
			if (m_focusCall) {
				m_focusCall->setConference(nullptr);
				// Do not terminate focus call when terminating the remote conference
				// This is required because the local conference creates a remote conference for every participant and the call from the participant to the local conference is the focus call
				m_focusCall = nullptr;
			}
			Conference::terminate();
			setState(ConferenceInterface::State::Terminated);
			break;
		default:
			break;
	}
	return 0;
}

int RemoteConference::enter () {
	if (state != ConferenceInterface::State::Created) {
		lError() << "Could not enter in the conference: bad conference state (" << Utils::toString(state) << ")";
		return -1;
	}

	time_t creationTime = time(nullptr);
	LinphoneCallState callState = static_cast<LinphoneCallState>(m_focusCall->getState());
	switch (callState) {
		case LinphoneCallStreamsRunning:
			break;
		case LinphoneCallPaused:
			m_focusCall->resume();
			notifyParticipantAdded(creationTime, false, getMe());
			break;
		default:
			lError() << "Could not join the conference: bad focus call state (" << Utils::toString(callState) << ")";
			return -1;
	}
	return 0;
}

void RemoteConference::leave () {
	if (state != ConferenceInterface::State::Created) {
		lError() << "Could not leave the conference: bad conference state (" << Utils::toString(state) << ")";
	}

	LinphoneCallState callState = static_cast<LinphoneCallState>(m_focusCall->getState());
	switch (callState) {
		case LinphoneCallPaused:
			lInfo() << getMe()->getAddress() << " is leaving conference " << getConferenceAddress() << " while focus call is paused.";
			break;
		case LinphoneCallStreamsRunning:
			lInfo() << getMe()->getAddress() << " is leaving conference " << getConferenceAddress() << ". Focus call is going to be paused.";
			m_focusCall->pause();
			break;
		default:
			lError() << getMe()->getAddress() << " cannot leave conference " << getConferenceAddress() << " because focus call is in state " << linphone_call_state_to_string(callState);
	}
}

bool RemoteConference::isIn () const {
	if (state != ConferenceInterface::State::Created)
		return false;
	LinphoneCallState callState = static_cast<LinphoneCallState>(m_focusCall->getState());
	return callState == LinphoneCallStreamsRunning;
}

bool RemoteConference::focusIsReady () const {
	LinphoneCallState focusState;
	if (!m_focusCall)
		return false;
	focusState = static_cast<LinphoneCallState>(m_focusCall->getState());
	return (focusState == LinphoneCallStreamsRunning) || (focusState == LinphoneCallPaused);
}

bool RemoteConference::transferToFocus (std::shared_ptr<LinphonePrivate::Call> call) {
	Address referToAddr(m_focusContact);
	//std::string referToAddr(m_focusCall->getRemoteContact());
	std::shared_ptr<Participant> participant = findParticipant(call->getActiveSession());
	referToAddr.setParam("admin", Utils::toString(participant->isAdmin()));
	if (call->transfer(referToAddr.asString()) == 0) {
		m_transferingCalls.push_back(call);
		return true;
	} else {
		lError() << "Conference: could not transfer call " << call << " to " << m_focusContact;
		return false;
	}
	return false;
}

void RemoteConference::reset () {
	if(m_focusContact) {
		ms_free(m_focusContact);
		m_focusContact = nullptr;
	}
	if(m_focusCall) {
		m_focusCall->setConference(nullptr);
		m_focusCall = nullptr;
	}
	m_pendingCalls.clear();
	m_transferingCalls.clear();
}

void RemoteConference::onFocusCallStateChanged (LinphoneCallState state) {
	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it;
	switch (state) {
		case LinphoneCallConnected:
		case LinphoneCallUpdatedByRemote:
		{
			Address focusContactAddress(m_focusCall->getRemoteContact());
			ConferenceId confId = getConferenceId();
			Address peerAddress(confId.getPeerAddress().asAddress());
			if ((getState() == ConferenceInterface::State::CreationPending) && (focusContactAddress.hasUriParam("conf-id")) && (!peerAddress.hasUriParam("conf-id"))) {
				m_focusContact = ms_strdup(linphone_call_get_remote_contact(m_focusCall->toC()));
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

				setConferenceId(ConferenceId(ConferenceAddress(m_focusContact), getConferenceId().getLocalAddress()));
				m_focusCall->setConferenceId(focusContactAddress.getUriParamValue("conf-id"));
				finalizeCreation();
			}
		}
			break;
		case LinphoneCallError:
			setState(ConferenceInterface::State::CreationFailed);
			m_focusCall->setConference(nullptr);
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
	switch (callState) {
		case LinphoneCallStreamsRunning:
		case LinphoneCallPaused:
		{
			CallSession::State focusCallState = m_focusCall->getState();
			// Transfer call to focus is the conference is in creation pending or created state and if the focus call is in connected, streams running or updated by remote state
			if (((focusCallState == CallSession::State::StreamsRunning) || (focusCallState == CallSession::State::Connected) || (focusCallState == CallSession::State::UpdatedByRemote)) && ((getState() == ConferenceInterface::State::CreationPending) || (state == ConferenceInterface::State::Created))) {
				// Transfer call only if focus call remote contact address is available (i.e. the call has been correctly established and passed through state StreamsRunning)
				if (m_focusCall && (m_focusCall->getRemoteContact().empty() == false)) {
					m_pendingCalls.remove(call);
					transferToFocus(call);
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
	if (Call::toCpp(call)->getSharedFromThis() == conf->m_focusCall)
		conf->onFocusCallStateChanged(cstate);
	else {
		list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(conf->m_pendingCalls.begin(), conf->m_pendingCalls.end(), Call::toCpp(call)->getSharedFromThis());
		if (it != conf->m_pendingCalls.end())
			conf->onPendingCallStateChanged(Call::toCpp(call)->getSharedFromThis(), cstate);
	}
}

void RemoteConference::transferStateChanged (LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(conf->m_transferingCalls.begin(), conf->m_transferingCalls.end(), Call::toCpp(transfered)->getSharedFromThis());
	if (it != conf->m_transferingCalls.end())
		conf->onTransferingCallStateChanged(Call::toCpp(transfered)->getSharedFromThis(), new_call_state);
}

AudioControlInterface * RemoteConference::getAudioControlInterface() const{
	if (!m_focusCall) return nullptr;
	shared_ptr<MediaSession> ms = m_focusCall->getMediaSession();
	return ms->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
}

VideoControlInterface * RemoteConference::getVideoControlInterface() const{
	if (!m_focusCall) return nullptr;
	shared_ptr<MediaSession> ms = m_focusCall->getMediaSession();
	return ms->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
}

AudioStream *RemoteConference::getAudioStream(){
	if (!m_focusCall) return nullptr;
	shared_ptr<MediaSession> ms = m_focusCall->getMediaSession();
	MS2AudioStream *stream = ms->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return stream ? (AudioStream*)stream->getMediaStream() : nullptr;
}

void RemoteConference::notifyReceived (const string &body) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->notifyReceived(body);
	}
#else
	lInfo() << "Advanced IM such as conferencing events is disabled!";
#endif // HAVE_ADVANCED_IM
}

void RemoteConference::onStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	shared_ptr<CallSession> session = nullptr;
	string subject = getSubject();
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
			if (getMe()->isAdmin() && (subject.empty() == false)) {
				session = m_focusCall->getActiveSession();
				if (session) {
					session->update(nullptr, subject);
				}
			}
			break;
		case ConferenceInterface::State::TerminationPending:
			#ifdef HAVE_ADVANCED_IM
			if (eventHandler) {
				eventHandler->unsubscribe();
			}
			#endif // HAVE_ADVANCED_IM
			resetLastNotify();
			terminate();
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

	SalReferOp *referOp = new SalReferOp(cCore->sal.get());
	LinphoneAddress *lAddr = linphone_address_new(m_focusCall->getRemoteContact().c_str());
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

	Conference::setSubject(subject);

	if (getState() != ConferenceInterface::State::Created) {
		lInfo() << "Participants will be notified of the conference state changed once the conference is in state created";
		return;
	}

	shared_ptr<CallSession> session = m_focusCall->getActiveSession();
	if (session) {
		session->update(nullptr, subject);
	}
}

bool RemoteConference::update(const LinphonePrivate::ConferenceParamsInterface &newParameters){
	if (!getMe()->isAdmin()) {
		lError() << "Unable to update conference parameters because focus " << getMe()->getAddress().asString() << " is not admin";
		return false;
	}

	return Conference::update(newParameters);
}

void RemoteConference::onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	const IdentityAddress &pAddr = event->getParticipantAddress();

	if (isMe(pAddr)) {
	#ifdef HAVE_ADVANCED_IM
		bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
		if (eventLogEnabled) {
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

}//end of namespace MediaConference

LINPHONE_END_NAMESPACE
