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
#include "conference/params/media-session-params-p.h"
#include "core/core-p.h"
#include "conference/session/mixers.h"
#include "conference/session/ms2-streams.h"
#include "conference/session/media-session.h"

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


Conference::Conference(LinphoneCore *core, const ConferenceParams *params) :
	m_core(core),
	m_currentParams((new ConferenceParams(*params))->toSharedPtr()),
	m_state(LinphoneConferenceStopped){
}

int Conference::addParticipant (LinphoneCall *call) {
	Participant *p = new Participant(call);
	m_participants.push_back(p);
	return 0;
}

int Conference::removeParticipant (LinphoneCall *call) {
	Participant *p = findParticipant(call);
	if (!p)
		return -1;
	delete p;
	m_participants.remove(p);
	return 0;
}

int Conference::removeParticipant (const LinphoneAddress *uri) {
	Participant *p = findParticipant(uri);
	if (!p)
		return -1;
	delete p;
	m_participants.remove(p);
	return 0;
}

int Conference::terminate () {
	for (auto it = m_participants.begin(); it != m_participants.end(); it++)
		delete *it;
	m_participants.clear();
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
		if (m_currentParams->m_stateChangedCb)
			m_currentParams->m_stateChangedCb(toC(), state, m_currentParams->m_userData);
	}
}

Conference::Participant *Conference::findParticipant (const LinphoneCall *call) const {
	for (auto it = m_participants.begin(); it != m_participants.end(); it++) {
		if ((*it)->getCall() == call)
			return *it;
	}
	return nullptr;
}

Conference::Participant *Conference::findParticipant (const LinphoneAddress *uri) const {
	for (auto it = m_participants.begin(); it != m_participants.end(); it++) {
		if (linphone_address_equal((*it)->getUri(), uri))
			return *it;
	}
	return nullptr;
}




LocalConference::LocalConference (LinphoneCore *core, const ConferenceParams *params) :
	Conference(core, params){
	m_state = LinphoneConferenceRunning;
	mMixerSession.reset(new MixerSession(*L_GET_CPP_PTR_FROM_C_OBJECT(core)));
}

LocalConference::~LocalConference() {
	terminate();
}

void LocalConference::addLocalEndpoint () {
	if (!m_currentParams->localParticipantEnabled()) return;
	
	StreamMixer *mixer = mMixerSession->getMixerByType(SalAudio);
	if (mixer) mixer->enableLocalParticipant(true);
	
	if (m_currentParams->videoEnabled()){
		mixer = mMixerSession->getMixerByType(SalVideo);
		if (mixer){
			mixer->enableLocalParticipant(true);
			VideoControlInterface *vci = getVideoControlInterface();
			if (vci){
				vci->setNativePreviewWindowId(m_core->preview_window_id);
				vci->setNativeWindowId(m_core->video_window_id);
			}
		}
	}
	mIsIn = true;
}

int LocalConference::inviteAddresses (const list<const LinphoneAddress *> &addresses, const LinphoneCallParams *params) {
	for (const auto &address : addresses) {
		LinphoneCall *call = linphone_core_get_call_by_remote_address2(m_core, address);
		if (!call) {
			/* Start a new call by indicating that it has to be put into the conference directly */
			LinphoneCallParams *new_params;
			if (params){
				new_params = linphone_call_params_copy(params);
			}else{
				new_params = linphone_core_create_call_params(m_core, nullptr);
				linphone_call_params_enable_video(new_params, m_currentParams->videoEnabled());
			}
			linphone_call_params_set_in_conference(new_params, TRUE);
			call = linphone_core_invite_address_with_params(m_core, address, new_params);
			if (!call){
				ms_error("LocalConference::inviteAddresses(): could not invite participant");
			}else{
				addParticipant(call);
			}
			linphone_call_params_unref(new_params);
		} else {
			/* There is already a call to this address, so simply join it to the local conference if not already done */
			if (!linphone_call_params_get_in_conference(linphone_call_get_current_params(call)))
				addParticipant(call);
		}
		/* If the local participant is not yet created, created it and it to the conference */
		addLocalEndpoint();
	}
	return 0;
}

int LocalConference::addParticipant (LinphoneCall *call) {
	LinphoneCallState state = linphone_call_get_state(call);
	
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(call))) {
		ms_error("Already in conference");
		return -1;
	}
	bool starting = (getSize() == 0);
	bool localEndpointCanBeAdded = false;
	switch(state){
		case LinphoneCallOutgoingInit:
		case LinphoneCallOutgoingProgress:
		case LinphoneCallIncomingReceived:
			const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				L_GET_PRIVATE(Call::toCpp(call)->getParams()))->setInConference(true);
		break;
		case LinphoneCallPaused:
			const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				L_GET_PRIVATE(Call::toCpp(call)->getParams()))->setInConference(true);
			linphone_call_resume(call);
		break;
		case LinphoneCallStreamsRunning:
		{
			LinphoneCallParams *params = linphone_core_create_call_params(m_core, call);
			linphone_call_params_set_in_conference(params, TRUE);
			linphone_call_params_enable_video(params, FALSE);

			linphone_call_update(call, params);
			linphone_call_params_unref(params);
			localEndpointCanBeAdded = true;
		}
		break;
		default:
			ms_error("Call is in state %s, it cannot be added to the conference",
			linphone_call_state_to_string(linphone_call_get_state(call)));
			return -1;
		break;
	}
	if (call == linphone_core_get_current_call(m_core))
		L_GET_PRIVATE_FROM_C_OBJECT(m_core)->setCurrentCall(nullptr);
	_linphone_call_set_conf_ref(call, toC());
	mMixerSession->joinStreamsGroup(Call::toCpp(call)->getMediaSession()->getStreamsGroup());
	Conference::addParticipant(call);
	if (starting) setState(LinphoneConferenceRunning);
	if (localEndpointCanBeAdded){
		/*
		 * This needs to be done at the end, to ensure that the call in StreamsRunning state has released the local
		 * resources (mic and camera), which is done during the joinStreamsGroup() step.
		 */
		addLocalEndpoint();
	}
	return 0;
}

int LocalConference::remoteParticipantsCount () {
	return (int)m_participants.size();
}

int LocalConference::removeParticipant (LinphoneCall *call) {
	int err = 0;
	if (linphone_call_get_conference(call) != this->toC()){
		lError() << "Call " << call << " is not part of conference " << this->toC();
		return -1;
	}
	Conference::removeParticipant(call);
	bool conferenceGoesEmpty = remoteParticipantsCount() == 0 && isIn();
	switch(linphone_call_get_state(call)){
		case LinphoneCallStreamsRunning:
			if (conferenceGoesEmpty){
				/* 
				 * Special case: the last remote participant is in StreamsRunning state and we are still in the conference.
				 * Then directly reconnect to it, as a normal call.
				 */
				lInfo() << "Call [" << call << "] with " << Call::toCpp(call)->getRemoteAddress().asString() <<
					" is our last call in our conference, we will reconnect directly to it.";
				LinphoneCallParams *params = linphone_core_create_call_params(m_core, call);
				linphone_call_params_set_in_conference(params, FALSE);
				leave();
				ms_message("Updating call to notify of conference removal.");
				err = linphone_call_update(call, params);
				linphone_call_params_unref(params);
				conferenceGoesEmpty = false; // already empty as we have just left.
			}else{
				/* Kick the call out of the conference by moving to the Paused state. */
				err = _linphone_call_pause(call);
			}
		break;
		case LinphoneCallEnd:
		break;
		default:
			/*
			 * Normally there should be nothing special to do for a call in a transcient state, but log a warning.
			 */
			lWarning() << "Unhandled case " << linphone_call_state_to_string(linphone_call_get_state(call)) << " for a call in conference.";
		break;
	}
	if (conferenceGoesEmpty){
		leave();
	}
	mMixerSession->unjoinStreamsGroup(Call::toCpp(call)->getMediaSession()->getStreamsGroup());
	_linphone_call_set_conf_ref(call, nullptr);
	if (getSize() == 0) setState(LinphoneConferenceStopped);
	return err;
}

int LocalConference::removeParticipant (const LinphoneAddress *uri) {
	const Participant *participant = findParticipant(uri);
	if (!participant)
		return -1;
	LinphoneCall *call = participant->getCall();
	if (!call)
		return -1;
	return removeParticipant(call);
}

int LocalConference::terminate () {
	leave();
	/*FIXME: very inefficient server side because it iterates on the global call list. */
	list<shared_ptr<LinphonePrivate::Call>> calls = L_GET_CPP_PTR_FROM_C_OBJECT(m_core)->getCalls();
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
	if (linphone_core_sound_resources_locked(m_core))
		return -1;
	if (linphone_core_get_current_call(m_core))
		_linphone_call_pause(linphone_core_get_current_call(m_core));

	addLocalEndpoint();
	return 0;
}

void LocalConference::removeLocalEndpoint () {
	mMixerSession->enableLocalParticipant(false);
	mIsIn = false;
}

int LocalConference::leave () {
	if (isIn())
		removeLocalEndpoint();
	return 0;
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



RemoteConference::RemoteConference (LinphoneCore *core, const ConferenceParams *params) :
	Conference(core, params) {
	m_focusAddr = nullptr;
	m_focusContact = nullptr;
	m_focusCall = nullptr;
	m_coreCbs = nullptr;
	m_focusAddr = lp_config_get_string(m_core->config, "misc", "conference_focus_addr", "");
	m_coreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(m_coreCbs, callStateChangedCb);
	linphone_core_cbs_set_transfer_state_changed(m_coreCbs, transferStateChanged);
	linphone_core_cbs_set_user_data(m_coreCbs, this);
	_linphone_core_add_callbacks(m_core, m_coreCbs, TRUE);
}

RemoteConference::~RemoteConference () {
	terminate();
	linphone_core_remove_callbacks(m_core, m_coreCbs);
	linphone_core_cbs_unref(m_coreCbs);
}

int RemoteConference::inviteAddresses (const list<const LinphoneAddress *> &addresses, const LinphoneCallParams *params) {
	ms_error("RemoteConference::inviteAddresses() not implemented");
	return -1;
}

int RemoteConference::addParticipant (LinphoneCall *call) {
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
				return -1;
			params = linphone_core_create_call_params(m_core, nullptr);
			linphone_call_params_enable_video(params, m_currentParams->videoEnabled());
			m_focusCall = linphone_core_invite_address_with_params(m_core, addr, params);
			m_pendingCalls.push_back(call);
			callLog = linphone_call_get_call_log(m_focusCall);
			callLog->was_conference = TRUE;
			linphone_address_unref(addr);
			linphone_call_params_unref(params);
			setState(LinphoneConferenceStarting);
			return 0;
		case LinphoneConferenceStarting:
			Conference::addParticipant(call);
			if(focusIsReady())
				transferToFocus(call);
			else
				m_pendingCalls.push_back(call);
			return 0;
		case LinphoneConferenceRunning:
			Conference::addParticipant(call);
			transferToFocus(call);
			return 0;
		default:
			ms_error("Could not add call %p to the conference. Bad conference state (%s)", call, stateToString(m_state));
			return -1;
	}
}

int RemoteConference::removeParticipant (const LinphoneAddress *uri) {
	char *refer_to;
	LinphoneAddress *refer_to_addr;
	int res;

	switch (m_state) {
		case LinphoneConferenceRunning:
			if(!findParticipant(uri)) {
				char *tmp = linphone_address_as_string(uri);
				ms_error("Conference: could not remove participant '%s': not in the participants list", tmp);
				ms_free(tmp);
				return -1;
			}
			refer_to_addr = linphone_address_clone(uri);
			linphone_address_set_method_param(refer_to_addr, "BYE");
			refer_to = linphone_address_as_string(refer_to_addr);
			linphone_address_unref(refer_to_addr);
			res = linphone_call_get_op(m_focusCall)->refer(refer_to);
			ms_free(refer_to);
			if (res == 0)
				return Conference::removeParticipant(uri);
			else {
				char *tmp = linphone_address_as_string(uri);
				ms_error("Conference: could not remove participant '%s': REFER with BYE has failed", tmp);
				ms_free(tmp);
				return -1;
			}
		default:
			ms_error("Cannot remove %s from conference: Bad conference state (%s)",
				linphone_address_as_string(uri), stateToString(m_state));
			return -1;
	}
}

int RemoteConference::terminate () {
	switch (m_state) {
		case LinphoneConferenceRunning:
		case LinphoneConferenceStarting:
			linphone_call_terminate(m_focusCall);
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
	LinphoneCallState callState = linphone_call_get_state(m_focusCall);
	switch (callState) {
		case LinphoneCallStreamsRunning:
			break;
		case LinphoneCallPaused:
			linphone_call_resume(m_focusCall);
			break;
		default:
			ms_error("Could not join the conference: bad focus call state (%s)",
				linphone_call_state_to_string(callState));
			return -1;
	}
	return 0;
}

int RemoteConference::leave () {
	if (m_state != LinphoneConferenceRunning) {
		ms_error("Could not leave the conference: bad conference state (%s)", stateToString(m_state));
		return -1;
	}
	LinphoneCallState callState = linphone_call_get_state(m_focusCall);
	switch (callState) {
		case LinphoneCallPaused:
			break;
		case LinphoneCallStreamsRunning:
			linphone_call_pause(m_focusCall);
			break;
		default:
			ms_error("Could not leave the conference: bad focus call state (%s)",
				linphone_call_state_to_string(callState));
			return -1;
	}
	return 0;
}

bool RemoteConference::isIn () const {
	if (m_state != LinphoneConferenceRunning)
		return false;
	LinphoneCallState callState = linphone_call_get_state(m_focusCall);
	return callState == LinphoneCallStreamsRunning;
}

bool RemoteConference::focusIsReady () const {
	LinphoneCallState focusState;
	if (!m_focusCall)
		return false;
	focusState = linphone_call_get_state(m_focusCall);
	return (focusState == LinphoneCallStreamsRunning) || (focusState == LinphoneCallPaused);
}

bool RemoteConference::transferToFocus (LinphoneCall *call) {
	if (linphone_call_transfer(call, m_focusContact) == 0) {
		m_transferingCalls.push_back(call);
		return true;
	} else {
		ms_error("Conference: could not transfer call [%p] to %s", call, m_focusContact);
		return false;
	}
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
	list<LinphoneCall *>::iterator it;
	switch (state) {
		case LinphoneCallConnected:
			m_focusContact = linphone_call_get_remote_contact(m_focusCall);
			it = m_pendingCalls.begin();
			while (it != m_pendingCalls.end()) {
				LinphoneCall *pendingCall = *it;
				LinphoneCallState pendingCallState = linphone_call_get_state(pendingCall);
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

void RemoteConference::onPendingCallStateChanged (LinphoneCall *call, LinphoneCallState state) {
	switch (state) {
		case LinphoneCallStreamsRunning:
		case LinphoneCallPaused:
			if (m_state == LinphoneConferenceRunning) {
				m_pendingCalls.remove(call);
				m_transferingCalls.push_back(call);
				linphone_call_transfer(call, m_focusContact);
			}
			break;
		case LinphoneCallError:
		case LinphoneCallEnd:
			m_pendingCalls.remove(call);
			Conference::removeParticipant(call);
			if ((m_participants.size() + m_pendingCalls.size() + m_transferingCalls.size()) == 0)
				terminate();
			break;
		default:
			break;
	}
}

void RemoteConference::onTransferingCallStateChanged (LinphoneCall* transfered, LinphoneCallState newCallState) {
	switch (newCallState) {
		case LinphoneCallConnected:
			m_transferingCalls.push_back(transfered);
			findParticipant(transfered)->m_call = nullptr;
			break;
		case LinphoneCallError:
			m_transferingCalls.remove(transfered);
			Conference::removeParticipant(transfered);
			if ((m_participants.size() + m_pendingCalls.size() + m_transferingCalls.size()) == 0)
				terminate();
			break;
		default:
			break;
	}
}

void RemoteConference::callStateChangedCb (LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	if (call == conf->m_focusCall)
		conf->onFocusCallSateChanged(cstate);
	else {
		list<LinphoneCall *>::iterator it = find(conf->m_pendingCalls.begin(), conf->m_pendingCalls.end(), call);
		if (it != conf->m_pendingCalls.end())
			conf->onPendingCallStateChanged(call, cstate);
	}
}

void RemoteConference::transferStateChanged (LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	list<LinphoneCall *>::iterator it = find(conf->m_transferingCalls.begin(), conf->m_transferingCalls.end(), transfered);
	if (it != conf->m_transferingCalls.end())
		conf->onTransferingCallStateChanged(transfered, new_call_state);
}

AudioControlInterface * RemoteConference::getAudioControlInterface() const{
	if (!m_focusCall) return nullptr;
	shared_ptr<MediaSession> ms = Call::toCpp(m_focusCall)->getMediaSession();
	return ms->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
}

VideoControlInterface * RemoteConference::getVideoControlInterface() const{
	if (!m_focusCall) return nullptr;
	shared_ptr<MediaSession> ms = Call::toCpp(m_focusCall)->getMediaSession();
	return ms->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
}


AudioStream *RemoteConference::getAudioStream(){
	if (!m_focusCall) return nullptr;
	shared_ptr<MediaSession> ms = Call::toCpp(m_focusCall)->getMediaSession();
	MS2AudioStream *stream = ms->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return stream ? (AudioStream*)stream->getMediaStream() : nullptr;
}

}//end of namespace MediaConference

LINPHONE_END_NAMESPACE

using namespace LinphonePrivate;
using namespace LinphonePrivate::MediaConference;

const char *linphone_conference_state_to_string (LinphoneConferenceState state) {
	return MediaConference::Conference::stateToString(state);
}


LinphoneConference *linphone_local_conference_new (LinphoneCore *core) {
	return (new MediaConference::LocalConference(core))->toC();
}

LinphoneConference *linphone_local_conference_new_with_params (LinphoneCore *core, const LinphoneConferenceParams *params) {
	return (new MediaConference::LocalConference(core, ConferenceParams::toCpp(params)))->toC();
}

LinphoneConference *linphone_remote_conference_new (LinphoneCore *core) {
	return (new MediaConference::RemoteConference(core))->toC();
}

LinphoneConference *linphone_remote_conference_new_with_params (LinphoneCore *core, const LinphoneConferenceParams *params) {
	return (new MediaConference::RemoteConference(core, ConferenceParams::toCpp(params)))->toC();
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

int linphone_conference_add_participant (LinphoneConference *obj, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(obj)->addParticipant(call);
}

LinphoneStatus linphone_conference_remove_participant (LinphoneConference *obj, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(obj)->removeParticipant(uri);
}

int linphone_conference_remove_participant_with_call (LinphoneConference *obj, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(obj)->removeParticipant(call);
}

int linphone_conference_terminate (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->terminate();
}

int linphone_conference_enter (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->enter();
}

int linphone_conference_leave (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->leave();
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
	const list<MediaConference::Conference::Participant *> &participants = MediaConference::Conference::toCpp(obj)->getParticipants();
	bctbx_list_t *participants_list = nullptr;
	for (auto it = participants.begin(); it != participants.end(); it++) {
		LinphoneAddress *uri = linphone_address_clone((*it)->getUri());
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
			return typeid(MediaConference::Conference::toCpp(obj)) == typeid(LocalConference);
		case LinphoneConferenceClassRemote:
			return typeid(MediaConference::Conference::toCpp(obj)) == typeid(RemoteConference);
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

void linphone_conference_params_set_state_changed_callback (LinphoneConferenceParams *params, LinphoneConferenceStateChangedCb cb, void *user_data) {
	ConferenceParams::toCpp(params)->setStateChangedCallback(cb, user_data);
}

