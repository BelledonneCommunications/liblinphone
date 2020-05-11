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
#include <math.h>

#include "core-p.h"
#include "call/call.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/streams.h"
#include "logger/logger.h"

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"
#include "conference_private.h"


// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

int CorePrivate::addCall (const shared_ptr<Call> &call) {
	L_Q();
	L_ASSERT(call);
	if (!canWeAddCall())
		return -1;
	if (!hasCalls())
		notifySoundcardUsage(true);
	calls.push_back(call);
	linphone_core_notify_call_created(q->getCCore(), call->toC());
	return 0;
}

bool CorePrivate::canWeAddCall () const {
	L_Q();
	if (q->getCallCount() < static_cast<unsigned int>(q->getCCore()->max_calls))
		return true;
	lInfo() << "Maximum amount of simultaneous calls reached!";
	return false;
}

bool CorePrivate::inviteReplacesABrokenCall (SalCallOp *op) {
	CallSession *replacedSession = nullptr;
	SalCallOp *replacedOp = op->getReplaces();
	if (replacedOp)
		replacedSession = reinterpret_cast<CallSession *>(replacedOp->getUserPointer());
	for (const auto &call : calls) {
		shared_ptr<CallSession> session = call->getActiveSession();
		if (session
			&& ((session->getPrivate()->isBroken() && op->compareOp(session->getPrivate()->getOp()))
				|| (replacedSession == session.get() && op->getFrom() == replacedOp->getFrom() && op->getTo() == replacedOp->getTo())
		)) {
			session->getPrivate()->replaceOp(op);
			return true;
		}
	}

	return false;
}

bool CorePrivate::isAlreadyInCallWithAddress (const Address &addr) const {
	for (const auto &call : calls) {
		if (call->isOpConfigured() && call->getRemoteAddress()->weakEqual(addr))
			return true;
	}
	return false;
}

void CorePrivate::iterateCalls (time_t currentRealTime, bool oneSecondElapsed) const {
	// Make a copy of the list af calls because it may be altered during calls to the Call::iterate method
	list<shared_ptr<Call>> savedCalls(calls);
	for (const auto &call : savedCalls) {
		call->iterate(currentRealTime, oneSecondElapsed);
	}
}

void CorePrivate::notifySoundcardUsage (bool used) {
	L_Q();
	MSSndCard *card = q->getCCore()->sound_conf.capt_sndcard;
	if (!!lp_config_get_int(linphone_core_get_config(q->getCCore()),"sound","usage_hint",1) && card && (ms_snd_card_get_capabilities(card) & MS_SND_CARD_CAP_IS_SLOW))
		ms_snd_card_set_usage_hint(card, used);
}

int CorePrivate::removeCall (const shared_ptr<Call> &call) {
	L_ASSERT(call);
	auto iter = find(calls.begin(), calls.end(), call);
	if (iter == calls.end()) {
		lWarning() << "Could not find the call to remove";
		return -1;
	}
	calls.erase(iter);
	return 0;
}

void CorePrivate::setVideoWindowId (bool preview, void *id) {
#ifdef VIDEO_ENABLED
	L_Q();
	if (q->getCCore()->conf_ctx){
		MediaConference::Conference *conf = MediaConference::Conference::toCpp(q->getCCore()->conf_ctx);
		if (conf->isIn()){
			lInfo() << "There is a conference, video window " << id << "is assigned to the conference.";
			if (!preview){
				conf->getVideoControlInterface()->setNativeWindowId(id);
			}else{
				conf->getVideoControlInterface()->setNativePreviewWindowId(id);
			}
			return;
		}
	}
	for (const auto &call : calls) {
		shared_ptr<MediaSession> ms = dynamic_pointer_cast<MediaSession>(call->getActiveSession());
		if (ms){
			if (preview){
				ms->setNativePreviewWindowId(id);
			}else{
				ms->setNativeVideoWindowId(id);
			}
		}
	}
#endif
}


// =============================================================================

bool Core::areSoundResourcesLocked () const {
	L_D();
	for (const auto &call : d->calls) {
		if (call->mediaInProgress())
			return true;
		switch (call->getState()) {
			case CallSession::State::OutgoingInit:
			case CallSession::State::OutgoingProgress:
			case CallSession::State::OutgoingRinging:
			case CallSession::State::OutgoingEarlyMedia:
			case CallSession::State::Connected:
			case CallSession::State::Referred:
			case CallSession::State::IncomingEarlyMedia:
			case CallSession::State::Updating:
				lInfo() << "Call " << call << " is locking sound resources";
				return true;
			default:
				break;
		}
	}
	return false;
}

shared_ptr<Call> Core::getCallByRemoteAddress (const Address &addr) const {
	L_D();
	for (const auto &call : d->calls) {
		if (call->getRemoteAddress()->weakEqual(addr))
			return call;
	}
	return nullptr;
}

const list<shared_ptr<Call>> &Core::getCalls () const {
	L_D();
	return d->calls;
}

unsigned int Core::getCallCount () const {
	L_D();
	return static_cast<unsigned int>(d->calls.size());
}

shared_ptr<Call> Core::getCurrentCall () const {
	L_D();
	return d->currentCall;
}

LinphoneStatus Core::pauseAllCalls () {
	L_D();
	for (const auto &call : d->calls) {
		if ((call->getState() == CallSession::State::StreamsRunning) || (call->getState() == CallSession::State::PausedByRemote))
			call->pause();
	}
	return 0;
}

void Core::soundcardHintCheck () {
	L_D();
	bool noNeedForSound = true;
	// Check if the remaining calls are paused
	for (const auto &call : d->calls) {
		if ((call->getState() != CallSession::State::Pausing)
			&& (call->getState() != CallSession::State::Paused)
			&& (call->getState() != CallSession::State::End)
			&& (call->getState() != CallSession::State::Error)) {
			noNeedForSound = false;
			break;
		}
	}
	// If no more calls or all calls are paused, we can free the soundcard
	LinphoneConfig *config = linphone_core_get_config(L_GET_C_BACK_PTR(this));
	bool useRtpIo = !!lp_config_get_int(config, "sound", "rtp_io", FALSE);
	bool useRtpIoEnableLocalOutput = !!lp_config_get_int(config, "sound", "rtp_io_enable_local_output", FALSE);
	
	LinphoneConference *conf_ctx = getCCore()->conf_ctx;
	if (conf_ctx && linphone_conference_get_size(conf_ctx) >= 1) return;
	
	if ((!d->hasCalls() || noNeedForSound)
		&& (!L_GET_C_BACK_PTR(getSharedFromThis())->use_files && (!useRtpIo || (useRtpIo && useRtpIoEnableLocalOutput)))) {
		lInfo() << "Notifying soundcard that we don't need it anymore for calls";
		d->notifySoundcardUsage(false);
	}
}

void Core::soundcardAudioSessionActivated (bool actived) {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	if (card) {
		ms_snd_card_notify_audio_session_activated(card, actived);
	}
}

void Core::soundcardCallkitEnabled (bool enabled) {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	if (card) {
		ms_snd_card_app_notifies_activation(card, enabled);
	}
}

LinphoneStatus Core::terminateAllCalls () {
	L_D();
	while (!d->calls.empty()) {
		d->calls.front()->terminate();
	}
	return 0;
}

LINPHONE_END_NAMESPACE
