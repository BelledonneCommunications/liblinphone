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

#include <algorithm>
#include <math.h>

#include "account/account.h"
#include "call/call.h"
#include "conference/conference.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/streams.h"
#include "core-p.h"
#include "linphone/api/c-call-log.h"
#include "logger/logger.h"

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

int CorePrivate::addCall(const shared_ptr<Call> &call) {
	L_Q();
	L_ASSERT(call);

	auto callLog = call->getLog();
	if (callLog) {
		const auto &callId = callLog->getCallId();
		if (!callId.empty() && lastPushReceivedCallId == callId) {
			lInfo() << "Call ID matches last push received Call-ID, stopping push background task";
			lastPushReceivedCallId = "";
			pushReceivedBackgroundTask.stop();
			static_cast<PlatformHelpers *>(getCCore()->platform_helper)->stopPushService();
		}
	}

	if (!canWeAddCall()) return -1;

	if (!hasCalls()) {
		/*
		 * Free possibly used sound ressources now. Useful for iOS, because CallKit may cause any already running
		 * AudioUnit to stop working.
		 */
		linphone_core_stop_dtmf_stream(q->getCCore());
	}
	calls.push_back(call);

	linphone_core_notify_call_created(q->getCCore(), call->toC());
	return 0;
}

bool CorePrivate::canWeAddCall() const {
	L_Q();
	if (q->getCallCount() < static_cast<unsigned int>(q->getCCore()->max_calls)) return true;
	lInfo() << "Maximum amount of simultaneous calls reached!";
	return false;
}

bool CorePrivate::inviteReplacesABrokenCall(SalCallOp *op) {
	CallSession *replacedSession = nullptr;
	SalCallOp *replacedOp = op->getReplaces();
	if (replacedOp) replacedSession = static_cast<CallSession *>(replacedOp->getUserPointer());
	for (const auto &call : calls) {
		shared_ptr<CallSession> session = call->getActiveSession();
		if (session && ((session->getPrivate()->isBroken() && op->compareOp(session->getPrivate()->getOp())) ||
		                (replacedSession == session.get() && op->getFrom() == replacedOp->getFrom() &&
		                 op->getTo() == replacedOp->getTo()))) {
			session->getPrivate()->replaceOp(op);
			return true;
		}
	}

	return false;
}

bool CorePrivate::isAlreadyInCallWithAddress(const std::shared_ptr<Address> &addr) const {
	for (const auto &call : calls) {
		if (call->isOpConfigured() && call->getRemoteAddress()->weakEqual(*addr)) return true;
	}
	return false;
}

void CorePrivate::iterateCalls(time_t currentRealTime, bool oneSecondElapsed) const {
	// Make a copy of the list af calls because it may be altered during calls to the Call::iterate method
	list<shared_ptr<Call>> savedCalls(calls);
	for (const auto &call : savedCalls) {
		call->iterate(currentRealTime, oneSecondElapsed);
	}
}

void CorePrivate::notifySoundcardUsage(bool used) {
	L_Q();

	if (!linphone_config_get_int(linphone_core_get_config(q->getCCore()), "sound", "usage_hint", 1)) return;
	if (q->getCCore()->use_files) return;

	LinphoneConfig *config = linphone_core_get_config(q->getCCore());
	bool useRtpIo = !!linphone_config_get_int(config, "sound", "rtp_io", FALSE);
	bool useRtpIoEnableLocalOutput = !!linphone_config_get_int(config, "sound", "rtp_io_enable_local_output", FALSE);
	if (useRtpIo && !useRtpIoEnableLocalOutput) return;

	LinphoneConference *conf_ctx = getCCore()->conf_ctx;
	if (conf_ctx && ((linphone_conference_get_participant_count(conf_ctx) >= 1) || linphone_conference_is_in(conf_ctx)))
		return;

	MSSndCard *capture_card = q->getCCore()->sound_conf.capt_sndcard;
	if (capture_card) {
		if (used) lInfo() << "Notifying capture sound card that it is going to be used.";
		else lInfo() << "Notifying capture sound card that is no longer needed.";
		ms_snd_card_set_usage_hint(capture_card, used);
	}

	MSSndCard *playback_card = q->getCCore()->sound_conf.play_sndcard;
	if (playback_card) {
		if (used) lInfo() << "Notifying playback sound card that it is going to be used.";
		else lInfo() << "Notifying playback sound card that is no longer needed.";
		ms_snd_card_set_usage_hint(playback_card, used);
	}
}

int CorePrivate::removeCall(const shared_ptr<Call> &call) {
	L_ASSERT(call);
	auto iter = find(calls.begin(), calls.end(), call);
	if (iter == calls.end()) {
		lWarning() << "Could not find the call (local address " << call->getLocalAddress()->toString()
		           << " remote address " << call->getRemoteAddress()->toString() << ") to remove";
		return -1;
	}
	lInfo() << "Removing the call (local address " << call->getLocalAddress()->toString() << " remote address "
	        << (call->getRemoteAddress() ? call->getRemoteAddress()->toString() : "Unknown")
	        << ") from the list attached to the core";

	calls.erase(iter);
	return 0;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void CorePrivate::setVideoWindowId(bool preview, void *id) {
#ifdef VIDEO_ENABLED
	L_Q();
	if (q->getCCore()->conf_ctx) {
		Conference *conf = Conference::toCpp(q->getCCore()->conf_ctx);
		if (conf->isIn() && conf->getVideoControlInterface()) {
			lInfo() << "There is a conference " << conf->getConferenceAddress() << ", video window " << id
			        << "is assigned to the conference.";
			if (!preview) {
				conf->getVideoControlInterface()->setNativeWindowId(id);
			} else {
				conf->getVideoControlInterface()->setNativePreviewWindowId(id);
			}
			return;
		}
	}
	for (const auto &call : calls) {
		shared_ptr<MediaSession> ms = dynamic_pointer_cast<MediaSession>(call->getActiveSession());
		if (ms) {
			if (preview) {
				ms->setNativePreviewWindowId(id);
			} else {
				ms->setNativeVideoWindowId(id);
			}
		}
	}
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

/*
 * setCurrentCall() is the good place to notify the soundcard about its planned usage.
 * The currentCall is set when a first call happens (either incoming or outgoing), regardless of its state.
 * When there are multiple calls established, only the one that uses the soundcard is the current call. It is one with
 * which the local user is interacting with sound. When in a locally-mixed conference, the current call is set to null.
 *
 * notifySoundcardUsage() is an optimization intended to "slow" soundcard, that needs several hundred of milliseconds to
 * initialize (in practice as of today: only iOS AudioUnit). notifySoundcardUsage(true) indicates that the sound card is
 * likely to be used in the future (it may be already being used in fact), and so it should be kept active even if no
 * AudioStream is still using it, which can happen during small transition phases between OutgoingRinging and
 * StreamsRunning. notifySoundcardUsage(false) indicates that no one is going to use the soundcard in the short term, so
 * that it can be safely shutdown, if not used by an AudioStream.
 */
void CorePrivate::setCurrentCall(const std::shared_ptr<Call> &call) {
	if (!currentCall && call) {
		/* we had no current call but now we have one. */
		notifySoundcardUsage(true);
	} else if (!call || currentCall != call) {
#if TARGET_OS_IPHONE
		/* the current call is reset or changed.
		 * Indeed, with CallKit the AudioUnit cannot be reused between different calls (we get silence). */
		notifySoundcardUsage(false);
#endif
	}
	currentCall = call;
}

// =============================================================================

bool Core::areSoundResourcesLocked() const {
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
					lInfo() << "Call " << call << " (local address " << call->getLocalAddress()->toString()
					        << " remote address " << call->getRemoteAddress()->toString()
					        << ") is locking sound resources because it is state " << call->getState();
					return true;
				case CallSession::State::Connected:
					// Allow to put in conference call in state connected
					return !call->getConference();
				case CallSession::State::StreamsRunning:
					if (call->mediaInProgress()) {
						lInfo() << "Call " << call << " (local address " << call->getLocalAddress()->toString()
						        << " remote address " << call->getRemoteAddress()->toString()
						        << ") is locking sound resources because it is state " << call->getState()
						        << " and media is in progress";
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

shared_ptr<Call> Core::getCallByRemoteAddress(const std::shared_ptr<const Address> &addr) const {
	L_D();
	for (const auto &call : d->calls) {
		if (call->getRemoteAddress()->weakEqual(*addr)) return call;
	}
	return nullptr;
}

shared_ptr<Call> Core::getCallByCallId(const string &callId) const {
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

const list<shared_ptr<Call>> &Core::getCalls() const {
	L_D();
	return d->calls;
}

unsigned int Core::getCallCount() const {
	L_D();
	return static_cast<unsigned int>(d->calls.size());
}

shared_ptr<Call> Core::getCurrentCall() const {
	L_D();
	return d->currentCall;
}

LinphoneStatus Core::pauseAllCalls() {
	L_D();
	for (const auto &call : d->calls) {
		if ((call->getState() == CallSession::State::StreamsRunning) ||
		    (call->getState() == CallSession::State::PausedByRemote))
			call->pause();
	}
	return 0;
}

void Core::soundcardActivateAudioSession(bool actived) {
	MSSndCard *card = getCCore()->sound_conf.play_sndcard;
	if (card) {
		ms_snd_card_notify_audio_session_activated(card, actived);
	}
}

void Core::soundcardConfigureAudioSession() {
	MSSndCard *card = getCCore()->sound_conf.play_sndcard;
	if (card) {
		ms_snd_card_configure_audio_session(card);
	}
}

void Core::soundcardEnableCallkit(bool enabled) {
	MSSndCard *card = getCCore()->sound_conf.play_sndcard;
	if (card) {
		ms_snd_card_app_notifies_activation(card, enabled);
	}
}

void Core::soundcardAudioRouteChanged() {
	MSSndCard *card = getCCore()->sound_conf.play_sndcard;
	if (card) {
		ms_snd_card_notify_audio_route_changed(card);
	}
}

LinphoneStatus Core::terminateAllCalls() {
	L_D();
	auto calls = d->calls;
	while (!calls.empty()) {
		calls.front()->terminate();
		calls.pop_front();
	}
	return 0;
}

// =============================================================================

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Core::reportConferenceCallEvent(EventLog::Type type,
                                     std::shared_ptr<CallLog> &callLog,
                                     std::shared_ptr<ConferenceInfo> confInfo) {
	std::shared_ptr<Address> to = callLog->getToAddress() ? callLog->getToAddress() : nullptr;
#ifdef HAVE_DB_STORAGE
	L_D();

	if (d->mainDb == nullptr) return;

	if (confInfo == nullptr) {
		// Let's see if we have a conference info in db with the corresponding URI
		confInfo = callLog->wasConference() ? callLog->getConferenceInfo() : d->mainDb->getConferenceInfoFromURI(to);
	}
#endif

	// TODO: This is a workaround that has to be removed ASAP
	// Add all calls that have been into a conference to the call logs of the core. The if below is required for calls
	// that have been merged in a conference. In fact, in such a scenario, the client that merges calls to put them in a
	// conference will call the conference factory or the audio video conference factory directly but still its call
	// must be added to the call logs
	std::shared_ptr<Address> from = callLog->getFromAddress() ? callLog->getFromAddress() : nullptr;
	if (!confInfo && to) {
		// Do not add calls made to the conference factory in the history
		auto account = lookupKnownAccount(to, true);
		if (account) {
			const auto &conferenceFactoryUri = account->getAccountParams()->getConferenceFactoryAddress();
			if (conferenceFactoryUri && conferenceFactoryUri->isValid()) {
				if (to->weakEqual(*conferenceFactoryUri)) {
					return;
				}
			}
			// Do not add calls made to the audio/video conference factory in the history either
			const auto &audioVideoConferenceFactoryAddress =
			    account->getAccountParams()->getAudioVideoConferenceFactoryAddress();
			if (audioVideoConferenceFactoryAddress != nullptr) {
				if (to->weakEqual(*audioVideoConferenceFactoryAddress)) {
					return;
				}
			}
		}
	}

	if (from && to) {
		// For PushIncomingState call, from and to address are unknow.
		const std::string usernameFrom = from ? from->getUsername() : std::string();
		const std::string usernameTo = to ? to->getUsername() : std::string();
		if ((!usernameFrom.empty() && (usernameFrom.find("chatroom-") != std::string::npos)) ||
		    (!usernameTo.empty() && (usernameTo.find("chatroom-") != std::string::npos)))
			return;
	}
	// End of workaround

#ifdef HAVE_DB_STORAGE
	auto event = make_shared<ConferenceCallEvent>(type, std::time(nullptr), callLog, confInfo);
	d->mainDb->addEvent(event);
#endif

	LinphoneCore *lc = getCCore();

	// Check if we already have this log in cache
	if (lc->call_logs != NULL) {
		for (bctbx_list_t *it = lc->call_logs; it != NULL; it = it->next) {
			LinphoneCallLog *log = (LinphoneCallLog *)it->data;
			if (bctbx_strcmp(linphone_call_log_get_call_id(log), callLog->getCallId().c_str()) == 0) {
				lc->call_logs = bctbx_list_remove(lc->call_logs, log);
				linphone_call_log_unref(log);
				break;
			}
		}
	}

	lc->call_logs = bctbx_list_prepend(lc->call_logs, linphone_call_log_ref(callLog->toC()));

	if (bctbx_list_size(lc->call_logs) > (size_t)lc->max_call_logs) {
		bctbx_list_t *elem, *prevelem = NULL;
		/*find the last element*/
		for (elem = lc->call_logs; elem != NULL; elem = elem->next) {
			prevelem = elem;
		}
		elem = prevelem;
		linphone_call_log_unref((LinphoneCallLog *)elem->data);
		lc->call_logs = bctbx_list_erase_link(lc->call_logs, elem);
	}

#ifndef HAVE_DB_STORAGE
	call_logs_write_to_config_file(lc);
#endif

	linphone_core_notify_call_log_updated(getCCore(), callLog->toC());
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

void Core::reportEarlyCallFailed(LinphoneCallDir dir,
                                 const std::shared_ptr<Address> &from,
                                 const std::shared_ptr<Address> &to,
                                 LinphoneErrorInfo *ei,
                                 const std::string callId) {
	auto callLog = CallLog::create(getSharedFromThis(), dir, from, to);
	callLog->setErrorInfo(ei);
	callLog->setStatus(LinphoneCallEarlyAborted);
	callLog->setCallId(callId);

	reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, callLog, nullptr);
}

LINPHONE_END_NAMESPACE
