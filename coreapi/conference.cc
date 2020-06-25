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

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call-p.h"
#include "conference/params/media-session-params-p.h"
#include "core/core-p.h"
#include "conference/conference.h"
#include "conference/participant.h"
#include "conference/session/mixers.h"
#include "conference/session/ms2-streams.h"
#include "conference/session/media-session.h"
#ifdef HAVE_ADVANCED_IM
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

	setState(ConferenceInterface::State::None);
}

bool Conference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	const Address * remoteContact = static_pointer_cast<MediaSession>(call->getPrivate()->getActiveSession())->getRemoteContactAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = Participant::create(this,*remoteContact, call->getPrivate()->getActiveSession());

	printf("Adding participant %p - session %p\n", p.get(), call->getPrivate()->getActiveSession().get());

	shared_ptr<ParticipantDevice> device = p->addDevice(*remoteContact);
	device->setSession(call->getPrivate()->getActiveSession());
	participants.push_back(p);
//	Conference::addParticipant(call);
	return 0;
}

int Conference::removeParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(call);
	if (!p)
		return -1;
	participants.remove(p);
	return 0;
}

int Conference::removeParticipant (const IdentityAddress &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	if (!p)
		return -1;
	participants.remove(p);
	return 0;
}

int Conference::terminate () {

	// Delete conference ID from proxy as the conference was terminated
	LinphoneAddress * cConferenceAddress = linphone_address_new(getConferenceAddress().asString().c_str());
	LinphoneProxyConfig * proxyCfg = linphone_core_lookup_known_proxy(getCore()->getCCore(), cConferenceAddress);
	linphone_address_unref(cConferenceAddress);
	char * contactAddressStr = sal_address_as_string(proxyCfg->op->getContactAddress());
	Address contactAddress(contactAddressStr);
	ms_free(contactAddressStr);
	if (contactAddress.hasParam ("conf-id")) {
		contactAddress.removeUriParam("conf-id");
		proxyCfg->op->setContactAddress(contactAddress.getInternalAddress());
	}

	participants.clear();
	getCore()->deleteAudioVideoConference(getSharedFromThis());
	return 0;
}

const char *Conference::stateToString (LinphonePrivate::ConferenceInterface::State state) {
	return Utils::toString(state).c_str();
}

void Conference::setState (LinphonePrivate::ConferenceInterface::State state) {
	LinphonePrivate::Conference::setState(state);
	// TODO Delete
	if (m_stateChangedCb) {
		m_stateChangedCb(toC(), (LinphoneChatRoomState)state, m_userData);
	}
}

std::shared_ptr<LinphonePrivate::Participant> Conference::findParticipant (const std::shared_ptr<LinphonePrivate::Call> call) const {
	for (const auto &participant : getParticipants()) {
		if (participant->getSession() == call->getPrivate()->getActiveSession())
			return participant;
	}

	return nullptr;
}

void Conference::setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) {

}

void Conference::join (const IdentityAddress &participantAddress) {
	
}

std::shared_ptr<LinphonePrivate::Participant> Conference::getMe () const {
	return nullptr;
}

bool Conference::removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) {
	bool soFarSoGood = true;
	for (const auto &p : participants)
		soFarSoGood &= removeParticipant(p);
	return soFarSoGood;
}

LocalConference::LocalConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, myAddress, listener, params){
printf("%s - Creating conference [%p]\n", __func__, this);
	setState(ConferenceInterface::State::Created);
	mMixerSession.reset(new MixerSession(*core.get()));

	char confId[6];
	belle_sip_random_token(confId,sizeof(confId));

	// Update proxy contact address to add conference ID
	// Do not use myAddress directly as it may lack some parameter like gruu
	LinphoneAddress * cAddress = linphone_address_new(myAddress.asString().c_str());
	LinphoneProxyConfig * proxyCfg = linphone_core_lookup_known_proxy(core->getCCore(), cAddress);
	char * contactAddressStr = sal_address_as_string(proxyCfg->op->getContactAddress());
	Address contactAddress(contactAddressStr);
	ms_free(contactAddressStr);
	contactAddress.setUriParam("conf-id",confId);
	proxyCfg->op->setContactAddress(contactAddress.getInternalAddress());
	linphone_address_unref(cAddress);

	setConferenceId(ConferenceId(contactAddress, contactAddress));
	setConferenceAddress(contactAddress);

#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<LocalConferenceEventHandler>(this);
	addListener(eventHandler);
#endif // HAVE_ADVANCED_IM

	// Video is already enable in the conference params constructor
	confParams->enableAudio(true);

	getCore()->insertAudioVideoConference(getSharedFromThis());
}

LocalConference::~LocalConference() {
	if (state != ConferenceInterface::State::Terminated) {
		terminate();
	}
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM
}

void LocalConference::subscribeReceived (LinphoneEvent *event) {
	shared_ptr<Conference> conf = L_GET_CPP_PTR_FROM_C_OBJECT(linphone_event_get_core(event))->findAudioVideoConference(conferenceId);
#ifdef HAVE_ADVANCED_IM
	eventHandler->subscribeReceived(event, false);
#endif // HAVE_ADVANCED_IM
}

void LocalConference::addLocalEndpoint () {
	if (!confParams->localParticipantEnabled()) return;
	
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
	mIsIn = true;
}

int LocalConference::inviteAddresses (const list<const LinphoneAddress *> &addresses, const LinphoneCallParams *params) {
	for (const auto &address : addresses) {
		LinphoneCall *call = linphone_core_get_call_by_remote_address2(getCore()->getCCore(), address);
		std::shared_ptr<LinphonePrivate::Call>  ccCall = nullptr;
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
			call = linphone_core_invite_address_with_params(getCore()->getCCore(), address, new_params);
			ccCall = L_GET_CPP_PTR_FROM_C_OBJECT(call);
			if (!call){
				ms_error("LocalConference::inviteAddresses(): could not invite participant");
			}else{
				addParticipant(ccCall);
			}
			linphone_call_params_unref(new_params);
		} else {
			ccCall = L_GET_CPP_PTR_FROM_C_OBJECT(call);
			/* There is already a call to this address, so simply join it to the local conference if not already done */
			if (!linphone_call_params_get_in_conference(linphone_call_get_current_params(call)))
				addParticipant(ccCall);
		}
		/* If the local participant is not yet created, created it and it to the conference */
		addLocalEndpoint();
	}
	return 0;
}

bool LocalConference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	LinphoneCallState state = static_cast<LinphoneCallState>(call->getState());
	
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(L_GET_C_BACK_PTR(call)))) {
		ms_error("Already in conference");
		return false;
	}

	bool starting = (getSize() == 0);
	bool localEndpointCanBeAdded = false;
	switch(state){
		case LinphoneCallOutgoingInit:
		case LinphoneCallOutgoingProgress:
		case LinphoneCallIncomingReceived:
			const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				L_GET_PRIVATE(call->getParams()))->setInConference(true);
		break;
		case LinphoneCallPaused:
			/*
			 * Modifying the MediaSession's params directly is a bit hacky.
			 * However, the resume() method doesn't accept parameters.
			 */
			const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				L_GET_PRIVATE(call->getParams()))->setInConference(true);
			const_cast<LinphonePrivate::MediaSessionParams *>(
				call->getParams())->enableVideo(getCurrentParams().videoEnabled());
			call->resume();
		break;
		case LinphoneCallStreamsRunning:
		{
			LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), L_GET_C_BACK_PTR(call));
			linphone_call_params_set_in_conference(params, TRUE);
			linphone_call_params_enable_video(params, getCurrentParams().videoEnabled());

			linphone_call_update(L_GET_C_BACK_PTR(call), params);
			linphone_call_params_unref(params);
			localEndpointCanBeAdded = true;
		}
		break;
		default:
			ms_error("Call is in state %s, it cannot be added to the conference",
			linphone_call_state_to_string(static_cast<LinphoneCallState>(call->getState())));
			return false;
		break;
	}
	if (L_GET_C_BACK_PTR(call) == linphone_core_get_current_call(getCore()->getCCore()))
		L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(nullptr);
	_linphone_call_set_conf_ref(L_GET_C_BACK_PTR(call), toC());
	mMixerSession->joinStreamsGroup(L_GET_PRIVATE(call)->getMediaSession()->getStreamsGroup());
	Conference::addParticipant(call);
	if (starting) setState(ConferenceInterface::State::Created);
	if (localEndpointCanBeAdded){
		/*
		 * This needs to be done at the end, to ensure that the call in StreamsRunning state has released the local
		 * resources (mic and camera), which is done during the joinStreamsGroup() step.
		 */
		addLocalEndpoint();
	}
	return true;
}

int LocalConference::removeParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	int err = 0;
	
	if (linphone_call_get_conference(L_GET_C_BACK_PTR(call)) != this->toC()){
		lError() << "Call " << L_GET_C_BACK_PTR(call) << " is not part of conference " << this->toC();
		return -1;
	}
	if (getParticipantCount() >= 2){
		/* Kick the call out of the conference by moving to the Paused state. */
		const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				L_GET_PRIVATE(call->getParams()))->setInConference(false);
		err = call->pause();
	}
	
	Conference::removeParticipant(call);
	mMixerSession->unjoinStreamsGroup(L_GET_PRIVATE(call)->getMediaSession()->getStreamsGroup());
	_linphone_call_set_conf_ref(L_GET_C_BACK_PTR(call), nullptr);
	/* 
	 * Handle the case where only the local participant and a unique remote participant are remaining.
	 * In this case, we kill the conference and let these two participants to connect directly thanks to a simple call.
	 * Indeed, the conference adds latency and processing that is useless to do for 1-1 conversation.
	 */
	if (getParticipantCount() == 1 && isIn()){
		/* Obtain the last LinphoneCall from the list: FIXME: for the moment this list only contains remote participants so it works
		 * but it should contains all participants ideally.*/
		std::shared_ptr<LinphonePrivate::Participant> remaining_participant = *participants.begin();
		std::shared_ptr<LinphonePrivate::CallSession> session = remaining_participant->getSession();

		const CallSessionParams * params = session->getParams();
		CallSessionParams *currentParams = params->clone();
		lInfo() << "Participant [" << remaining_participant << "] with " << session->getRemoteAddress().asString() << 
			" is our last call in our conference, we will reconnect directly to it.";

		currentParams->getPrivate()->setInConference(FALSE);
		leave();
		ms_message("Updating call to notify of conference removal.");
		err = session->update(currentParams);
		/* invoke removeParticipant() recursively to remove this last participant. */
		bool success = removeParticipant(remaining_participant->getAddress());
		Conference::terminate();
		return success;
	}
	
	if (getSize() == 0) setState(ConferenceInterface::State::Terminated);
	return err;
}

int LocalConference::removeParticipant (const IdentityAddress &addr) {
	const std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(addr);
	if (!participant)
		return -1;
	std::shared_ptr<LinphonePrivate::MediaSession> mediaSession = static_pointer_cast<LinphonePrivate::MediaSession>(participant->getSession());
	if (!mediaSession)
		return -1;

	bool ret = Conference::removeParticipant(addr);
	//mMixerSession->unjoinStreamsGroup(mediaSession->getStreamsGroup());
	mediaSession->terminate();
	return ret;
}

void LocalConference::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
#ifdef HAVE_ADVANCED_IM
	eventHandler->subscriptionStateChanged(event, state);
#endif // HAVE_ADVANCED_IM
}

int LocalConference::terminate () {
	/*FIXME: very inefficient server side because it iterates on the global call list. */
	list<shared_ptr<LinphonePrivate::Call>> calls = L_GET_CPP_PTR_FROM_C_OBJECT(getCore()->getCCore())->getCalls();
	for (auto it = calls.begin(); it != calls.end(); it++) {
		shared_ptr<LinphonePrivate::Call> call(*it);
		LinphoneCall *cCall = L_GET_C_BACK_PTR(call);
printf("%s - trying to terminate call %p contact %s to %s\n", __func__, call.get(), linphone_call_get_remote_contact(cCall), linphone_address_as_string(linphone_call_get_to_address(cCall)));
		if (linphone_call_get_conference(cCall) == this->toC()) {
printf("%s - terminating call %p\n", __func__, call.get());
			call->terminate();
		}
	}
	if (state != ConferenceInterface::State::Terminated) {
		setState(ConferenceInterface::State::TerminationPending);
	}

	lInfo() << "func " << __func__ << " CONFERENCE TERMINATED!!!";

	return 0;
}

int LocalConference::finalizeTermination () {
	leave();
	Conference::terminate();
	setState(ConferenceInterface::State::Terminated);
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
	mMixerSession->enableLocalParticipant(false);
	mIsIn = false;
}

void LocalConference::leave () {
	if (isIn())
		removeLocalEndpoint();
}

bool LocalConference::update(const LinphonePrivate::ConferenceParamsInterface &newParameters){
	const LinphonePrivate::ConferenceParams &newConfParams = static_cast<const ConferenceParams&>(newParameters);
	/* Only adding or removing video is supported. */
	bool previousVideoEnablement = confParams->videoEnabled();
	if (newConfParams.videoEnabled() != previousVideoEnablement){
		lInfo() << "LocalConference::update(): checking participants...";
		for (auto participant : participants){
			std::shared_ptr<LinphonePrivate::MediaSession> session = static_pointer_cast<LinphonePrivate::MediaSession>(participant->getSession());

			if (session){

				const MediaSessionParams * params = session->getMediaParams();
				MediaSessionParams *currentParams = params->clone();

				if ((!!currentParams->videoEnabled()) != newConfParams.videoEnabled()){
					lInfo() << "Re-INVITing participant to start/stop video.";
					currentParams->enableVideo(newConfParams.videoEnabled());
					session->update(currentParams);
				}
			}
		}
		/* Don't forget the local participant. For simplicity, a removeLocalEndpoint()/addLocalEndpoint() does the job. */
		removeLocalEndpoint();
		addLocalEndpoint();
	}
	return MediaConference::Conference::update(newParameters);
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
	return mIsIn;
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

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded (time_t creationTime,  const bool isFullState, const Address &addr) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantAdded (creationTime,  isFullState, addr);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const Address &addr) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantRemoved (creationTime,  isFullState, addr);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const Address &addr, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantSetAdmin (creationTime,  isFullState, addr, isAdmin);
}

shared_ptr<ConferenceSubjectEvent> LocalConference::notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifySubjectChanged (creationTime, isFullState, subject);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu, const std::string name) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceAdded (creationTime,  isFullState, addr, gruu, name);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceRemoved (creationTime,  isFullState, addr, gruu);
}

RemoteConference::RemoteConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	const ConferenceId &conferenceId,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	Conference(core, myAddress, listener, params){
	m_focusAddr = nullptr;
	m_focusContact = nullptr;
	m_focusCall = nullptr;
	m_coreCbs = nullptr;
	m_focusAddr = lp_config_get_string(getCore()->getCCore()->config, "misc", "conference_focus_addr", "");
	m_coreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(m_coreCbs, callStateChangedCb);
	linphone_core_cbs_set_transfer_state_changed(m_coreCbs, transferStateChanged);
	linphone_core_cbs_set_user_data(m_coreCbs, this);
	_linphone_core_add_callbacks(getCore()->getCCore(), m_coreCbs, TRUE);

	setConferenceAddress(myAddress);
	setConferenceId(conferenceId);

	// Video is already enable in the conference params constructor
	confParams->enableAudio(true);


	addListener(std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this), [](ConferenceListenerInterface * p){}));
#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<RemoteConferenceEventHandler>(this);

	getCore()->getPrivate()->remoteListEventHandler->addHandler(eventHandler.get());
#endif // HAVE_ADVANCED_IM

	getCore()->insertAudioVideoConference(getSharedFromThis());

}

RemoteConference::~RemoteConference () {
	terminate();
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM

	linphone_core_remove_callbacks(getCore()->getCCore(), m_coreCbs);
	linphone_core_cbs_unref(m_coreCbs);
}

int RemoteConference::inviteAddresses (const list<const LinphoneAddress *> &addresses, const LinphoneCallParams *params) {
	ms_error("RemoteConference::inviteAddresses() not implemented");
	return -1;
}

bool RemoteConference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	LinphoneAddress *addr;
	LinphoneCallParams *params;
	LinphoneCallLog *callLog;
	switch (state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::CreationFailed:
			Conference::addParticipant(call);
			ms_message("Calling the conference focus (%s)", m_focusAddr);
			addr = linphone_address_new(m_focusAddr);
			if (!addr)
				return false;
			params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
			linphone_call_params_enable_video(params, confParams->videoEnabled());
			m_focusCall = L_GET_CPP_PTR_FROM_C_OBJECT(linphone_core_invite_address_with_params(getCore()->getCCore(), addr, params));
			m_pendingCalls.push_back(call);
			callLog = m_focusCall->getLog();
			callLog->was_conference = TRUE;
			linphone_address_unref(addr);
			linphone_call_params_unref(params);
			setState(ConferenceInterface::State::CreationPending);
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
			ms_error("Could not add call %p to the conference. Bad conference state (%s)", call.get(), stateToString(state));
			return false;
	}
}

int RemoteConference::removeParticipant (const IdentityAddress &addr) {
	Address refer_to_addr;
	int res;

	switch (state) {
		case ConferenceInterface::State::Created:
			if(!findParticipant(addr)) {
				ms_error("Conference: could not remove participant '%s': not in the participants list", addr.asString().c_str());
				return -1;
			}
			refer_to_addr = Address(addr);
			linphone_address_set_method_param(L_GET_C_BACK_PTR(&refer_to_addr), "BYE");
			res = L_GET_PRIVATE(m_focusCall)->getOp()->refer(refer_to_addr.asString().c_str());
			if (res == 0)
				return Conference::removeParticipant(addr);
			else {
				ms_error("Conference: could not remove participant '%s': REFER with BYE has failed", addr.asString().c_str());
				return -1;
			}
		default:
			ms_error("Cannot remove %s from conference: Bad conference state (%s)",
				addr.asString().c_str(), stateToString(state));
			return -1;
	}
}

int RemoteConference::terminate () {
	switch (state) {
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::CreationPending:
			m_focusCall->terminate();
			getCore()->deleteAudioVideoConference(getSharedFromThis());
			break;
		default:
			break;
	}
	return 0;
}

int RemoteConference::finalizeTermination () {
	leave();
	Conference::terminate();
	setState(ConferenceInterface::State::Terminated);
	return 0;
}

int RemoteConference::enter () {
	if (state != ConferenceInterface::State::Created) {
		ms_error("Could not enter in the conference: bad conference state (%s)", stateToString(state));
		return -1;
	}
	LinphoneCallState callState = static_cast<LinphoneCallState>(m_focusCall->getState());
	switch (callState) {
		case LinphoneCallStreamsRunning:
			break;
		case LinphoneCallPaused:
			m_focusCall->resume();
			break;
		default:
			ms_error("Could not join the conference: bad focus call state (%s)",
				linphone_call_state_to_string(callState));
			return -1;
	}
	return 0;
}

void RemoteConference::leave () {
	if (state != ConferenceInterface::State::Created) {
		ms_error("Could not leave the conference: bad conference state (%s)", stateToString(state));
	}
	LinphoneCallState callState = static_cast<LinphoneCallState>(m_focusCall->getState());
	switch (callState) {
		case LinphoneCallPaused:
			break;
		case LinphoneCallStreamsRunning:
			m_focusCall->pause();
			break;
		default:
			ms_error("Could not leave the conference: bad focus call state (%s)",
				linphone_call_state_to_string(callState));
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
	if (call->transfer(m_focusContact) == 0) {
		m_transferingCalls.push_back(call);
		return true;
	} else {
		ms_error("Conference: could not transfer call [%p] to %s", call.get(), m_focusContact);
		return false;
	}
	return false;
}

void RemoteConference::reset () {
	m_focusAddr = nullptr;
	if(m_focusContact) {
		ms_free(m_focusContact);
		m_focusContact = nullptr;
	}
	m_focusCall = nullptr;
	m_pendingCalls.clear();
	m_transferingCalls.clear();
}

void RemoteConference::onFocusCallSateChanged (LinphoneCallState state) {
	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it;
	switch (state) {
		case LinphoneCallConnected:
			m_focusContact = ms_strdup(linphone_call_get_remote_contact(L_GET_C_BACK_PTR(m_focusCall)));
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
			setState(ConferenceInterface::State::Created);
			break;
		case LinphoneCallError:
			reset();
			Conference::terminate();
			setState(ConferenceInterface::State::CreationFailed);
			break;
		case LinphoneCallEnd:
			reset();
			Conference::terminate();
			setState(ConferenceInterface::State::Terminated);
			break;
		default:
			break;
	}
}

void RemoteConference::onPendingCallStateChanged (std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState callState) {
	switch (callState) {
		case LinphoneCallStreamsRunning:
		case LinphoneCallPaused:
			if (state == ConferenceInterface::State::Created) {
				m_pendingCalls.remove(call);
				m_transferingCalls.push_back(call);
				call->transfer(m_focusContact);
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
//			findParticipant(transfered)->m_call = nullptr;
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
	if (L_GET_CPP_PTR_FROM_C_OBJECT(call) == conf->m_focusCall)
		conf->onFocusCallSateChanged(cstate);
	else {
		list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(conf->m_pendingCalls.begin(), conf->m_pendingCalls.end(), L_GET_CPP_PTR_FROM_C_OBJECT(call));
		if (it != conf->m_pendingCalls.end())
			conf->onPendingCallStateChanged(L_GET_CPP_PTR_FROM_C_OBJECT(call), cstate);
	}
}

void RemoteConference::transferStateChanged (LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(conf->m_transferingCalls.begin(), conf->m_transferingCalls.end(), L_GET_CPP_PTR_FROM_C_OBJECT(transfered));
	if (it != conf->m_transferingCalls.end())
		conf->onTransferingCallStateChanged(L_GET_CPP_PTR_FROM_C_OBJECT(transfered), new_call_state);
}

AudioControlInterface * RemoteConference::getAudioControlInterface() const{
	if (!m_focusCall) return nullptr;
	CallPrivate *callPriv = L_GET_PRIVATE(m_focusCall);
	shared_ptr<MediaSession> ms = callPriv->getMediaSession();
	return ms->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
}

VideoControlInterface * RemoteConference::getVideoControlInterface() const{
	if (!m_focusCall) return nullptr;
	CallPrivate *callPriv = L_GET_PRIVATE(m_focusCall);
	shared_ptr<MediaSession> ms = callPriv->getMediaSession();
	return ms->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
}

AudioStream *RemoteConference::getAudioStream(){
	if (!m_focusCall) return nullptr;
	CallPrivate *callPriv = L_GET_PRIVATE(m_focusCall);
	shared_ptr<MediaSession> ms = callPriv->getMediaSession();
	MS2AudioStream *stream = ms->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return stream ? (AudioStream*)stream->getMediaStream() : nullptr;
}

void RemoteConference::notifyReceived (const string &body) {
#ifdef HAVE_ADVANCED_IM
	eventHandler->notifyReceived(body);
#else
	ms_message("Advanced IM such as group chat is disabled!");
#endif // HAVE_ADVANCED_IM
}

}//end of namespace MediaConference

LINPHONE_END_NAMESPACE

