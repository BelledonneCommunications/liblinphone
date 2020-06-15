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
#include "call/call.h"
#include "c-wrapper/internal/c-tools.h"
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

template <typename _type>
inline list<_type> toStd(const bctbx_list_t *l){
	list<_type> ret;
	for(; l != NULL; l = l->next){
		ret.push_back(static_cast<_type>(l->data));
	}
	return ret;
}

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference{


Conference::Conference(
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<LinphonePrivate::ConferenceParams> params) :
	LinphonePrivate::Conference(core, myAddress, listener, params),
	m_state(LinphoneConferenceStopped){
}

bool Conference::addParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	std::shared_ptr<LinphonePrivate::Participant> p = Participant::create(this,call->getRemoteAddress());

	p->createSession(*this, nullptr, true, nullptr);
	participants.push_back(p);
//	Conference::addParticipant(call);
	m_callTable[p] = call;
	return 0;
}

int Conference::removeParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(call);
	if (!p)
		return -1;
	participants.remove(p);
	m_callTable.erase(p);
	return 0;
}

int Conference::removeParticipant (const IdentityAddress &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	if (!p)
		return -1;
	participants.remove(p);
	m_callTable.erase(p);
	return 0;
}

int Conference::terminate () {
	getCore()->deleteAudioVideoConference(getSharedFromThis());
	participants.clear();
	m_callTable.clear();
	return 0;
}

const char *Conference::stateToString (LinphoneConferenceState state) {
	switch(state) {
		case LinphoneConferenceStopped:
			return "Stopped";
		case LinphoneConferenceStarting:
			return "Starting";
		case LinphoneConferenceRunning:
			return "Ready";
		case LinphoneConferenceStartingFailed:
			return "Starting failed";
		default:
			return "Invalid state";
	}
}



void Conference::setState (LinphoneConferenceState state) {
	if (m_state != state) {
		ms_message("Switching conference [%p] into state '%s'", this, stateToString(state));
		m_state = state;
		if (m_stateChangedCb)
			m_stateChangedCb(toC(), state, m_userData);
	}
}

std::shared_ptr<LinphonePrivate::Participant> Conference::findParticipant (const std::shared_ptr<LinphonePrivate::Call> call) const {
	for (auto it = participants.begin(); it != participants.end(); it++) {
		auto pCall = m_callTable.find(*it)->second;
		if (pCall == call)
			return *it;
	}
	return nullptr;
}

const std::string &Conference::getSubject () const {
	return *(new std::string(""));
}

void Conference::setSubject (const std::string &subject) {

}

void Conference::setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) {

}

void Conference::join (const IdentityAddress &participantAddress) {
	
}

int Conference::getParticipantCount () const {
	return static_cast<int>(getParticipants().size());
}

std::shared_ptr<LinphonePrivate::Participant> Conference::getMe () const {
	return nullptr;
}

bool Conference::removeParticipant (const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	for (const auto &p : participants) {
		if (participant->getAddress() == p->getAddress()) {
			participants.remove(p);
			m_callTable.erase(p);
			return true;
		}
	}
	return false;
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
	m_state = LinphoneConferenceRunning;
	mMixerSession.reset(new MixerSession(*core.get()));

	setConferenceAddress(myAddress);

	setConferenceId(ConferenceId(myAddress, myAddress));

#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<LocalConferenceEventHandler>(this);
	addListener(eventHandler);
#endif // HAVE_ADVANCED_IM

	// Video is already enable in the conference params constructor
	confParams->enableAudio(true);

	getCore()->insertAudioVideoConference(getSharedFromThis());
}

LocalConference::~LocalConference() {
	terminate();
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
	
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(call->toC()))) {
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
			LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), call->toC());
			linphone_call_params_set_in_conference(params, TRUE);
			linphone_call_params_enable_video(params, getCurrentParams().videoEnabled());

			linphone_call_update(call->toC(), params);
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

	if (call->toC() == linphone_core_get_current_call(getCore()->getCCore()))
		L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(nullptr);
	_linphone_call_set_conf_ref(call->toC(), toC());
	mMixerSession->joinStreamsGroup(L_GET_PRIVATE(call)->getMediaSession()->getStreamsGroup());
	Conference::addParticipant(call);
	if (starting) setState(LinphoneConferenceRunning);
	if (localEndpointCanBeAdded){
		/*
		 * This needs to be done at the end, to ensure that the call in StreamsRunning state has released the local
		 * resources (mic and camera), which is done during the joinStreamsGroup() step.
		 */
		addLocalEndpoint();
	}
	return true;
}

int LocalConference::remoteParticipantsCount () {
	return (int)participants.size();
}

int LocalConference::removeParticipant (std::shared_ptr<LinphonePrivate::Call> call) {
	int err = 0;
	
	if (linphone_call_get_conference(call->toC()) != this->toC()){
		lError() << "Call " << call->toC() << " is not part of conference " << this->toC();
		return -1;
	}

	if (remoteParticipantsCount() >= 2){
		/* Kick the call out of the conference by moving to the Paused state. */
		const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				L_GET_PRIVATE(call->getParams()))->setInConference(false);
		err = call->pause();
	}
	
	Conference::removeParticipant(call);
	mMixerSession->unjoinStreamsGroup(L_GET_PRIVATE(call)->getMediaSession()->getStreamsGroup());
	_linphone_call_set_conf_ref(call->toC(), nullptr);
	/* 
	 * Handle the case where only the local participant and a unique remote participant are remaining.
	 * In this case, we kill the conference and let these two participants to connect directly thanks to a simple call.
	 * Indeed, the conference adds latency and processing that is useless to do for 1-1 conversation.
	 */
	if (remoteParticipantsCount() == 1 && isIn()){
		/* Obtain the last LinphoneCall from the list: FIXME: for the moment this list only contains remote participants so it works
		 * but it should contains all participants ideally.*/
		std::shared_ptr<LinphonePrivate::Call> remaining_call = m_callTable[(*participants.begin())];
		lInfo() << "Call [" << remaining_call << "] with " << remaining_call->getRemoteAddress().asString() << 
			" is our last call in our conference, we will reconnect directly to it.";
		LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), L_GET_C_BACK_PTR(remaining_call));
		linphone_call_params_set_in_conference(params, FALSE);
		leave();
		ms_message("Updating call to notify of conference removal.");
		err = linphone_call_update(L_GET_C_BACK_PTR(remaining_call), params);
		linphone_call_params_unref(params);
		/* invoke removeParticipant() recursively to remove this last participant. */
		bool success = removeParticipant(remaining_call);
		Conference::terminate();
		return success;
	}
	
	if (getSize() == 0) setState(LinphoneConferenceStopped);
	return err;
}

int LocalConference::removeParticipant (const IdentityAddress &addr) {
	const std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(addr);
	if (!participant)
		return -1;
	std::shared_ptr<LinphonePrivate::Call> call = m_callTable[participant];
	if (!call)
		return -1;
	return removeParticipant(call);
}

void LocalConference::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
#ifdef HAVE_ADVANCED_IM
	eventHandler->subscriptionStateChanged(event, state);
#endif // HAVE_ADVANCED_IM
}

int LocalConference::terminate () {
	leave();
	/*FIXME: very inefficient server side because it iterates on the global call list. */
	list<shared_ptr<LinphonePrivate::Call>> calls = L_GET_CPP_PTR_FROM_C_OBJECT(getCore()->getCCore())->getCalls();
	for (auto it = calls.begin(); it != calls.end(); it++) {
		shared_ptr<LinphonePrivate::Call> call(*it);
		LinphoneCall *cCall = call->toC();
		if (linphone_call_get_conference(cCall) == this->toC())
			call->terminate();
	}
	Conference::terminate();
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
			LinphoneCall *call = L_GET_C_BACK_PTR(m_callTable[participant]);
			if (call){
				const LinphoneCallParams *current_params = linphone_call_get_current_params(call);
				if ((!!linphone_call_params_video_enabled(current_params)) != newConfParams.videoEnabled()){
					lInfo() << "Re-INVITing participant to start/stop video.";
					LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), call);
					linphone_call_params_enable_video(params, newConfParams.videoEnabled());
					linphone_call_update(call, params);
					linphone_call_params_unref(params);
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
	switch (m_state) {
		case LinphoneConferenceStopped:
		case LinphoneConferenceStartingFailed:
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
			setState(LinphoneConferenceStarting);
			return true;
		case LinphoneConferenceStarting:
			Conference::addParticipant(call);
			if(focusIsReady())
				transferToFocus(call);
			else
				m_pendingCalls.push_back(call);
			return true;
		case LinphoneConferenceRunning:
			Conference::addParticipant(call);
			transferToFocus(call);
			return true;
		default:
			ms_error("Could not add call %p to the conference. Bad conference state (%s)", call.get(), stateToString(m_state));
			return false;
	}
}

int RemoteConference::removeParticipant (const IdentityAddress &addr) {
	Address refer_to_addr;
	int res;

	switch (m_state) {
		case LinphoneConferenceRunning:
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
				addr.asString().c_str(), stateToString(m_state));
			return -1;
	}
}

int RemoteConference::terminate () {
	switch (m_state) {
		case LinphoneConferenceRunning:
		case LinphoneConferenceStarting:
			m_focusCall->terminate();
			getCore()->deleteAudioVideoConference(getSharedFromThis());
			break;
		default:
			break;
	}
	return 0;
}

int RemoteConference::enter () {
	if (m_state != LinphoneConferenceRunning) {
		ms_error("Could not enter in the conference: bad conference state (%s)", stateToString(m_state));
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
	if (m_state != LinphoneConferenceRunning) {
		ms_error("Could not leave the conference: bad conference state (%s)", stateToString(m_state));
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
	if (m_state != LinphoneConferenceRunning)
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
			setState(LinphoneConferenceRunning);
			break;
		case LinphoneCallError:
			reset();
			Conference::terminate();
			setState(LinphoneConferenceStartingFailed);
			break;
		case LinphoneCallEnd:
			reset();
			Conference::terminate();
			setState(LinphoneConferenceStopped);
			break;
		default:
			break;
	}
}

void RemoteConference::onPendingCallStateChanged (std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState state) {
	switch (state) {
		case LinphoneCallStreamsRunning:
		case LinphoneCallPaused:
			if (m_state == LinphoneConferenceRunning) {
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

}//end of namespace MediaConference

LINPHONE_END_NAMESPACE

using namespace LinphonePrivate;
using namespace LinphonePrivate::MediaConference;

const char *linphone_conference_state_to_string (LinphoneConferenceState state) {
	return LinphonePrivate::MediaConference::Conference::stateToString(state);
}


LinphoneConference *linphone_local_conference_new (LinphoneCore *core, LinphoneAddress * addr) {
	return (new LinphonePrivate::MediaConference::LocalConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), nullptr, ConferenceParams::create(core)))->toC();
}

LinphoneConference *linphone_local_conference_new_with_params (LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params) {
	return (new LinphonePrivate::MediaConference::LocalConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), nullptr, ConferenceParams::toCpp(const_cast<LinphoneConferenceParams *>(params))->getSharedFromThis()))->toC();
}

LinphoneConference *linphone_remote_conference_new (LinphoneCore *core, LinphoneAddress * addr) {
	return (new LinphonePrivate::MediaConference::RemoteConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), ConferenceId(IdentityAddress(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr))), nullptr, ConferenceParams::create(core)))->toC();
}

LinphoneConference *linphone_remote_conference_new_with_params (LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params) {
	return (new LinphonePrivate::MediaConference::RemoteConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), ConferenceId(IdentityAddress(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr))), nullptr, ConferenceParams::toCpp(const_cast<LinphoneConferenceParams *>(params))->getSharedFromThis()))->toC();
}

LinphoneConference *linphone_conference_ref (LinphoneConference *conf) {
	return (LinphoneConference *)belle_sip_object_ref(conf);
}

void linphone_conference_unref (LinphoneConference *conf) {
	belle_sip_object_unref(conf);
}

LinphoneConferenceState linphone_conference_get_state (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getState();
}

const LinphoneConferenceParams * linphone_conference_get_current_params(const LinphoneConference *obj){
	return MediaConference::Conference::toCpp(obj)->getCurrentParams().toC();
}

LinphoneStatus linphone_conference_add_participant (LinphoneConference *obj, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(obj)->addParticipant(L_GET_CPP_PTR_FROM_C_OBJECT(call));
}

int linphone_conference_add_participant_with_call (LinphoneConference *obj, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(obj)->addParticipant(L_GET_CPP_PTR_FROM_C_OBJECT(call));
}

LinphoneStatus linphone_conference_remove_participant (LinphoneConference *obj, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(obj)->removeParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri));
}

int linphone_conference_remove_participant_with_call (LinphoneConference *obj, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(obj)->removeParticipant(L_GET_CPP_PTR_FROM_C_OBJECT(call));
}

int linphone_conference_update_params(LinphoneConference *obj, const LinphoneConferenceParams *params){
	return MediaConference::Conference::toCpp(obj)->update(*ConferenceParams::toCpp(params));
}

int linphone_conference_terminate (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->terminate();
}

int linphone_conference_enter (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->enter();
}

int linphone_conference_leave (LinphoneConference *obj) {
	MediaConference::Conference::toCpp(obj)->leave();
	return 0;
}

bool_t linphone_conference_is_in (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->isIn();
}

int linphone_conference_mute_microphone (LinphoneConference *obj, bool_t val) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(obj)->getAudioControlInterface();
	if (!aci) return -1;
	aci->enableMic(!val);
	return 0;
}

bool_t linphone_conference_microphone_is_muted (const LinphoneConference *obj) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(obj)->getAudioControlInterface();
	if (!aci) return FALSE;
	return aci->micEnabled() ? FALSE : TRUE;
}

float linphone_conference_get_input_volume (const LinphoneConference *obj) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(obj)->getAudioControlInterface();
	if (!aci) return 0.0;
	return aci->getRecordVolume();
}

int linphone_conference_get_size (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getSize();
}

bctbx_list_t *linphone_conference_get_participants (const LinphoneConference *obj) {
	const list<std::shared_ptr<LinphonePrivate::Participant>> &participants = MediaConference::Conference::toCpp(obj)->getParticipants();
	bctbx_list_t *participants_list = nullptr;
	for (auto it = participants.begin(); it != participants.end(); it++) {
		const Address &addr((*it)->getAddress());
		LinphoneAddress *uri = linphone_address_clone(L_GET_C_BACK_PTR(&addr));
		participants_list = bctbx_list_append(participants_list, uri);
	}
	return participants_list;
}

int linphone_conference_start_recording (LinphoneConference *obj, const char *path) {
	return MediaConference::Conference::toCpp(obj)->startRecording(path);
}

int linphone_conference_stop_recording (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->stopRecording();
}

bool_t linphone_conference_check_class (LinphoneConference *obj, LinphoneConferenceClass _class) {
	switch(_class) {
		case LinphoneConferenceClassLocal:
		return typeid(MediaConference::Conference::toCpp(obj)) ==
			   typeid(LinphonePrivate::MediaConference::LocalConference);
		case LinphoneConferenceClassRemote:
			return typeid(MediaConference::Conference::toCpp(obj)) ==
				   typeid(LinphonePrivate::MediaConference::RemoteConference);
		default:
			return FALSE;
	}
}

LinphoneStatus linphone_conference_invite_participants (LinphoneConference *obj, const bctbx_list_t *addresses, const LinphoneCallParams *params){
	return MediaConference::Conference::toCpp(obj)->inviteAddresses(toStd<const LinphoneAddress*>(addresses), params);
}

const char *linphone_conference_get_ID (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getID();
}

void linphone_conference_set_ID(LinphoneConference *obj, const char *conferenceID) {
	MediaConference::Conference::toCpp(obj)->setID(conferenceID);
}

AudioStream *linphone_conference_get_audio_stream(LinphoneConference *obj){
	return MediaConference::Conference::toCpp(obj)->getAudioStream();
}

void linphone_conference_set_state_changed_callback (LinphoneConference *obj, LinphoneConferenceStateChangedCb cb, void *user_data) {
	MediaConference::Conference::toCpp(obj)->setStateChangedCallback(cb, user_data);
}


LinphoneConferenceParams *linphone_conference_params_new (const LinphoneCore *core) {
	LinphoneConferenceParams *obj = ConferenceParams::createCObject(core);
	return obj;
}

LinphoneConferenceParams *linphone_conference_params_ref (LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->ref();
	return params;
}

void linphone_conference_params_unref (LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

void linphone_conference_params_free (LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

LinphoneConferenceParams *linphone_conference_params_clone (const LinphoneConferenceParams *params) {
	return static_cast<ConferenceParams*>(ConferenceParams::toCpp(params)->clone())->toC();
}

void linphone_conference_params_enable_video (LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableVideo(enable ? true : false);
}

bool_t linphone_conference_params_video_enabled (const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->videoEnabled() ? TRUE : FALSE;
}

void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params, bool_t enable){
	ConferenceParams::toCpp(params)->enableLocalParticipant(!!enable);
}

bool_t linphone_conference_params_local_participant_enabled(const LinphoneConferenceParams *params){
	return ConferenceParams::toCpp(params)->localParticipantEnabled();
}


