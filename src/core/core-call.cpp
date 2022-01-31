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
#include "account/account.h"
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

	if (!hasCalls()){
		/* 
		 * Free possibly used sound ressources now. Useful for iOS, because CallKit may cause any already running AudioUnit to stop working.
		 */
		linphone_core_stop_dtmf_stream(q->getCCore());
	}
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
		replacedSession = static_cast<CallSession *>(replacedOp->getUserPointer());
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
	if (!linphone_config_get_int(linphone_core_get_config(q->getCCore()),"sound","usage_hint",1)) return;
	MSSndCard *card = q->getCCore()->sound_conf.capt_sndcard;
	if (!card || !(ms_snd_card_get_capabilities(card) & MS_SND_CARD_CAP_IS_SLOW)) return;
	if (q->getCCore()->use_files) return;
	
	LinphoneConfig *config = linphone_core_get_config(q->getCCore());
	
	bool useRtpIo = !!linphone_config_get_int(config, "sound", "rtp_io", FALSE);
	bool useRtpIoEnableLocalOutput = !!linphone_config_get_int(config, "sound", "rtp_io_enable_local_output", FALSE);
	
	if (useRtpIo && !useRtpIoEnableLocalOutput) return;
	
	LinphoneConference *conf_ctx = getCCore()->conf_ctx;
	if (conf_ctx && ((linphone_conference_get_participant_count(conf_ctx) >= 1) || linphone_conference_is_in(conf_ctx))) return;
	if (used) lInfo() << "Notifying sound card that it is going to be used.";
	else lInfo() << "Notifying sound card that is no longer needed.";
	ms_snd_card_set_usage_hint(card, used);
}

int CorePrivate::removeCall (const shared_ptr<Call> &call) {
	L_ASSERT(call);
	auto iter = find(calls.begin(), calls.end(), call);
	if (iter == calls.end()) {
		lWarning() << "Could not find the call (local address " << call->getLocalAddress().asString() << " remote address " << call->getRemoteAddress()->asString() << ") to remove";
		return -1;
	}
	lInfo() << "Removing the call (local address " << call->getLocalAddress().asString() << " remote address " << (call->getRemoteAddress() ? call->getRemoteAddress()->asString() : "Unknown") << ") from the list attached to the core";

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

/*
 * setCurrentCall() is the good place to notify the soundcard about its planned usage.
 * The currentCall is set when a first call happens (either incoming or outgoing), regardless of its state.
 * When there are multiple calls established, only the one that uses the soundcard is the current call. It is one with which the local
 * user is interacting with sound.
 * When in a locally-mixed conference, the current call is set to null.
 * 
 * notifySoundcardUsage() is an optimization intended to "slow" soundcard, that needs several hundred of milliseconds to initialize 
 * (in practice as of today: only iOS AudioUnit).
 * notifySoundcardUsage(true) indicates that the sound card is likely to be used in the future (it may be already being used in fact), and so it should be kept
 * active even if no AudioStream is still using it, which can happen during small transition phases between OutgoingRinging and StreamsRunning.
 * notifySoundcardUsage(false) indicates that no one is going to use the soundcard in the short term, so that it can be safely shutdown, if not used by an AudioStream.
 */
void CorePrivate::setCurrentCall (const std::shared_ptr<Call> &call) {
	if (!currentCall && call){
		/* we had no current call but now we have one. */
		notifySoundcardUsage(true);
	}else if (!call || currentCall != call){
		/* the current call is reset or changed.
		 * Indeed, with CallKit the AudioUnit cannot be reused between different calls (we get silence). */
		notifySoundcardUsage(false);
	}
	currentCall = call;
}


// =============================================================================

bool Core::areSoundResourcesLocked () const {
	L_D();
	for (const auto &call : d->calls) {
		// Do not check if sound resources are locked by call if it is in a conference
		if (!call->getConference() || linphone_core_conference_server_enabled(getCCore())) {
			switch (call->getState()) {
				case CallSession::State::OutgoingInit:
				case CallSession::State::OutgoingProgress:
				case CallSession::State::OutgoingRinging:
				case CallSession::State::OutgoingEarlyMedia:
				case CallSession::State::Referred:
				case CallSession::State::IncomingEarlyMedia:
				case CallSession::State::Updating:
					lInfo() << "Call " << call << " (local address " << call->getLocalAddress().asString() << " remote address " << call->getRemoteAddress()->asString() << ") is locking sound resources because it is state " << call->getState();
					return true;
				case CallSession::State::Connected:
					// Allow to put in conference call in state connected
					return !call->getConference();
				case CallSession::State::StreamsRunning:
					if (call->mediaInProgress()) {
						lInfo() << "Call " << call << " (local address " << call->getLocalAddress().asString() << " remote address " << call->getRemoteAddress()->asString() << ") is locking sound resources because it is state " << call->getState() << " and media is in progress";
						return true;
					}
				break;
				default:
					break;
			}
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

shared_ptr<Call> Core::getCallByCallId (const string &callId) const {
	L_D();
	if (callId.empty()) {
		return nullptr;
	}

	for (const auto &call : d->calls) {
		if (!call->getLog()->getCallId().empty() && call->getLog()->getCallId() == callId) {
			return call;
		}
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

void Core::soundcardActivateAudioSession (bool actived) {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	if (card) {
		ms_snd_card_notify_audio_session_activated(card, actived);
	}
}

void Core::soundcardConfigureAudioSession () {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	if (card) {
		ms_snd_card_configure_audio_session(card);
	}
}

void Core::soundcardEnableCallkit (bool enabled) {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	if (card) {
		ms_snd_card_app_notifies_activation(card, enabled);
	}
}

void Core::soundcardAudioRouteChanged () {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	if (card) {
		ms_snd_card_notify_audio_route_changed(card);
	}
}

LinphoneStatus Core::terminateAllCalls () {
	L_D();
	auto calls = d->calls;
	while (!calls.empty()) {
		calls.front()->terminate();
		calls.pop_front();
	}
	return 0;
}

// =============================================================================

void Core::reportConferenceCallEvent (EventLog::Type type, std::shared_ptr<CallLog> &callLog, std::shared_ptr<ConferenceInfo> confInfo) {
	// TODO: This is a workaround that has to be removed ASAP
#ifdef HAVE_DB_STORAGE
	L_D();

	if (confInfo == nullptr) {
		// Let's see if we have a conference info in db with the corresponding URI
		confInfo = callLog->wasConference() ? callLog->getConferenceInfo() : d->mainDb->getConferenceInfoFromURI(ConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(callLog->getToAddress())));
	}
#endif

	// Add all calls that have been into a conference to the call logs of the core. The if below is required for calls that have been merged in a conference.
	// In fact, in such a scenario, the client that merges calls to put them in a conference will call the conference factory or the audio video conference factory directly but still its call must be added to the call logs
	if (!confInfo) {
		// Do not add calls made to the conference factory in the history
		LinphoneAccount *account = linphone_core_lookup_known_account(getCCore(), callLog->getToAddress());
		if (account) {
			string conferenceFactoryUri = Account::toCpp(account)->getAccountParams()->getConferenceFactoryUri();
			if (!conferenceFactoryUri.empty()) {
				LinphoneAddress *conference_factory_addr = linphone_address_new(conferenceFactoryUri.c_str());
				if (conference_factory_addr) {
					if (linphone_address_weak_equal(callLog->getToAddress(), conference_factory_addr)) {
						linphone_address_unref(conference_factory_addr);
						return;
					}
					linphone_address_unref(conference_factory_addr);
				}
			}
			// Do not add calls made to the audio/video conference factory in the history either
			const LinphoneAddress *audioVideoConferenceFactoryAddress = Account::toCpp(account)->getAccountParams()->getAudioVideoConferenceFactoryAddress();
			if (audioVideoConferenceFactoryAddress != nullptr) {
				if (linphone_address_weak_equal(callLog->getToAddress(), audioVideoConferenceFactoryAddress)) {
					return;
				}
			}
		}

		// For PushIncomingState call, from and to address are unknow.
		const char *usernameFrom = callLog->getFromAddress() ? linphone_address_get_username(callLog->getFromAddress()) : nullptr;
		const char *usernameTo = callLog->getToAddress() ? linphone_address_get_username(callLog->getToAddress()) : nullptr;
		if ((usernameFrom && (strstr(usernameFrom, "chatroom-") == usernameFrom))
			|| (usernameTo && (strstr(usernameTo, "chatroom-") == usernameTo))
		)
			return;
	}
	// End of workaround

#ifdef HAVE_DB_STORAGE
	auto event = make_shared<ConferenceCallEvent>(type, std::time(nullptr), callLog, confInfo);
	d->mainDb->addEvent(event);
#endif

	LinphoneCore *lc = getCCore();

	lc->call_logs = bctbx_list_prepend(lc->call_logs, linphone_call_log_ref(callLog->toC()));
	if (bctbx_list_size(lc->call_logs) > (size_t)lc->max_call_logs) {
		bctbx_list_t *elem, *prevelem = NULL;
		/*find the last element*/
		for(elem = lc->call_logs; elem != NULL; elem = elem->next) {
			prevelem = elem;
		}
		elem = prevelem;
		linphone_call_log_unref((LinphoneCallLog*) elem->data);
		lc->call_logs = bctbx_list_erase_link(lc->call_logs, elem);
	}

#ifndef HAVE_DB_STORAGE
	call_logs_write_to_config_file(lc);
#endif

	linphone_core_notify_call_log_updated(getCCore(), callLog->toC());
}

void Core::reportEarlyCallFailed (LinphoneCallDir dir, LinphoneAddress *from, LinphoneAddress *to, LinphoneErrorInfo *ei, const std::string callId) {
	auto callLog = CallLog::create(getSharedFromThis(), dir, from, to);
	callLog->setErrorInfo(ei);
	callLog->setStatus(LinphoneCallEarlyAborted);
	callLog->setCallId(callId);

	reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, callLog, nullptr);
}

LINPHONE_END_NAMESPACE
