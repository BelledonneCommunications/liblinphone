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

#include "tone-manager.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "call/call-p.h"
#include "conference/session/media-session.h"
#include "conference/session/media-session-p.h"
#include "conference/params/call-session-params-p.h"
#include "linphone/utils/general.h"
#include "core/core-p.h"
#include "conference_private.h"

LINPHONE_BEGIN_NAMESPACE

ToneManager::ToneManager(std::shared_ptr<Core> core) : CoreAccessor(core) {
    lInfo() << "[ToneManager] create ToneManager()";
	mStats = new LinphoneCoreToneManagerStats;
	*mStats = {0, 0, 0, 0, 0, 0, 0};
}

ToneManager::~ToneManager() {
    lInfo() << "[ToneManager] destroy ToneManager()";
	delete mStats;
}

std::string ToneManager::stateToString(ToneManager::State state) {
	switch(state) {
		case State::None:
			return "None";
		case State::Call:
			return "Call";
		case State::Ringback:
			return "Ringback";
		case State::Ringtone:
			return "Ringtone";
		case State::Tone:
			return "Tone";
		default:
			return "";
	}
}

void ToneManager::printDebugInfo(const std::shared_ptr<CallSession> &session) {
	auto callState = session->getState();
    auto toneState = getState(session);
	lInfo() << "[ToneManager] [" << session << "] state changed : [" << stateToString(toneState)  << ", " << Utils::toString(callState) << "]";
}

// ---------------------------------------------------
// public entrypoints for tones
// ---------------------------------------------------

void ToneManager::startRingbackTone(const std::shared_ptr<CallSession> &session) {
	printDebugInfo(session);
	if (getState(session) == State::Ringback) {
		return;
	}

	setState(session, State::Ringback);
	mStats->number_of_startRingbackTone++;
	
	if (session->getParams()->getPrivate()->getInConference()){
		lInfo() << "Skip ring back tone, call is in conference.";
		return;
	}

	if (!isAnotherSessionInState(session, State::Ringback)) {
		doStopToneToPlaySomethingElse(session);
		doStartRingbackTone(session);
	}
}

void ToneManager::startRingtone(const std::shared_ptr<CallSession> &session) {
	printDebugInfo(session);
	setState(session, State::Ringtone);
	if (!isAnotherSessionInState(session, State::Ringtone) && !isAnotherSessionInState(session, State::Ringback)) {
		doStopToneToPlaySomethingElse(session);
		doStartRingtone(session);
		mStats->number_of_startRingtone++;
	}
}

void ToneManager::startErrorTone(const std::shared_ptr<CallSession> &session, LinphoneReason reason) {
	LinphoneCore *lc = getCore()->getCCore();
	if (linphone_core_tone_indications_enabled(lc)) {
		printDebugInfo(session);
		doStopToneToPlaySomethingElse(session);
		doStartErrorTone(session, reason);
		mStats->number_of_startErrorTone++;
	}
}

void ToneManager::startNamedTone(const std::shared_ptr<CallSession> &session, LinphoneToneID toneId) {
	LinphoneCore *lc = getCore()->getCCore();
	if (linphone_core_tone_indications_enabled(lc)) {
		printDebugInfo(session);
		setState(session, State::Tone);
		doStopToneToPlaySomethingElse(session);
		doStartNamedTone(toneId);
		mStats->number_of_startNamedTone++;
	}
}

void ToneManager::goToCall(const std::shared_ptr<CallSession> &session) {
	printDebugInfo(session);
	lInfo() << "[ToneManager] " << __func__;
	doStop(session, State::Call);
}

void ToneManager::stop(const std::shared_ptr<CallSession> &session) {
	printDebugInfo(session);
	lInfo() << "[ToneManager] " << __func__;
	doStop(session, State::None);
}

void ToneManager::removeSession(const std::shared_ptr<CallSession> &session) {
	printDebugInfo(session);
	mSessions.erase(session);
	lInfo() << "[ToneManager] removeSession mSession size : " <<  mSessions.size();
}

/**
 * This start again ringtone when one call is not anymore in Ringtone state but the second call is still in this state
 * This code can't be called juste after the doStopRingtone because the first call needs to change its context (deletion or start a call)
 */
void ToneManager::update(const std::shared_ptr<CallSession> &session) {
	switch(session->getState()) {
		case CallSession::State::UpdatedByRemote:
		case CallSession::State::Updating:
		case CallSession::State::Released:
			printDebugInfo(session);
			if (isAnotherSessionInState(session, State::Ringtone)) {
				lInfo() << "[ToneManager] start again ringtone";
				doStartRingtone(nullptr);
				mStats->number_of_startRingtone++;
			}
			return;
			break;
		default:
			break;
	}
}

// ---------------------------------------------------
// linphone core public API entrypoints
// ---------------------------------------------------

void ToneManager::linphoneCorePlayDtmf(char dtmf, int duration) {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore()->getCCore();

	std::shared_ptr<CallSession> session = nullptr;
	bool hasSession = getSessionInState(State::Tone, session);
	if (hasSession) {
		doStop(session, State::None);
	}

	MSSndCard *card = linphone_core_in_call(lc)
		? lc->sound_conf.play_sndcard
		: lc->sound_conf.ring_sndcard;
	MSFilter *f = getAudioResource(ToneGenerator, card, true);

	if (f == NULL) {
		lError() << "[ToneManager] No dtmf generator at this time !";
		return;
	}

	if (duration > 0) {
		ms_filter_call_method(f, MS_DTMF_GEN_PLAY, &dtmf);
	} else {
		ms_filter_call_method(f, MS_DTMF_GEN_START, &dtmf);
	}
}

void ToneManager::linphoneCoreStopDtmf() {
	lInfo() << "[ToneManager] " << __func__;
	MSFilter *f = getAudioResource(ToneGenerator, NULL, false);
	if (f != NULL) {
		ms_filter_call_method_noarg (f, MS_DTMF_GEN_STOP);
	}
}

LinphoneStatus ToneManager::linphoneCorePlayLocal(const char *audiofile) {
	lInfo() << "[ToneManager] " << __func__;
	return playFile(audiofile);
}

void ToneManager::linphoneCoreStartDtmfStream() {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore()->getCCore();

	/*make sure ring stream is started*/
	getAudioResource(ToneGenerator, lc->sound_conf.ring_sndcard, true);
}

void ToneManager::linphoneCoreStopRinging() {
	lInfo() << "[ToneManager] " << __func__;
	doStopRingtone(nullptr);
}

void ToneManager::linphoneCoreStopDtmfStream() {
	lInfo() << "[ToneManager] " << __func__;
	doStopTone();
}

// ---------------------------------------------------
// timer
// ---------------------------------------------------

void ToneManager::createTimerToCleanTonePlayer(unsigned int delay) {
	lInfo() << "[ToneManager] " << __func__;
	if (!mTimer) {
		auto callback = [this] () -> bool {
			lInfo() << "[ToneManager] callback";
			LinphoneCore *lc = getCore()->getCCore();
			auto source = lc->ringstream ? lc->ringstream->source : nullptr;
			MSPlayerState state;
			if (source && ms_filter_call_method(source, MS_PLAYER_GET_STATE, &state) == 0) {
				if (state != MSPlayerPlaying) {
					deleteTimer();
					return false;
				}else{
					return false;
				}
			}
			return true;
		};
		mTimer = getCore()->createTimer(callback, delay, "Tone player cleanup");
	}
}

void ToneManager::deleteTimer() {
	if (mTimer) {
		lInfo() << "[ToneManager] " << __func__;
		doStopTone();
		mStats->number_of_stopTone++;
		getCore()->destroyTimer(mTimer);
		mTimer = nullptr;
	}
}

// ---------------------------------------------------
// setup tones
// ---------------------------------------------------

LinphoneToneDescription *ToneManager::getToneFromReason(LinphoneReason reason) {
	const bctbx_list_t *elem;
	for (elem = getCore()->getCCore()->tones; elem != NULL; elem = elem->next) {
		LinphoneToneDescription *tone = (LinphoneToneDescription *) elem->data;
		if (tone->reason==reason) return tone;
	}
	return NULL;
}

LinphoneToneDescription *ToneManager::getToneFromId(LinphoneToneID id) {
	const bctbx_list_t *elem;
	for (elem = getCore()->getCCore()->tones; elem != NULL; elem = elem->next) {
		LinphoneToneDescription *tone = (LinphoneToneDescription *) elem->data;
		if (tone->toneid == id) return tone;
	}
	return NULL;
}

void ToneManager::setTone(LinphoneReason reason, LinphoneToneID id, const char *audiofile) {
	LinphoneCore *lc = getCore()->getCCore();
	LinphoneToneDescription *tone = getToneFromId(id);
	if (!tone) tone = getToneFromReason(reason);

	if (tone) {
		lc->tones = bctbx_list_remove(lc->tones, tone);
		linphone_tone_description_destroy(tone);
	}
	tone = linphone_tone_description_new(reason, id, audiofile);
	lc->tones = bctbx_list_append(lc->tones, tone);
}

// ---------------------------------------------------
// callbacks file player
// ---------------------------------------------------

static void on_file_player_end(void *userData, MSFilter *f, unsigned int eventId, void *arg) {
	ToneManager *toneManager = (ToneManager *) userData;
	toneManager->onFilePlayerEnd(eventId);
}

void ToneManager::onFilePlayerEnd(unsigned int eventId) {
	switch (eventId) {
		case MS_PLAYER_EOF:
			lInfo() << "[ToneManager] " << __func__;
			doStopTone();
			break;
		default:
			break;
	}
}

// ---------------------------------------------------
// tester
// ---------------------------------------------------

LinphoneCoreToneManagerStats *ToneManager::getStats() {
	return mStats;
}

void ToneManager::resetStats() {
	*mStats = {0, 0, 0, 0, 0, 0, 0};
}

// ---------------------------------------------------
// sessions
// ---------------------------------------------------

/* set State for the session. Insert session if not present */
void ToneManager::setState(const std::shared_ptr<CallSession> &session, ToneManager::State newState) {
	if (mSessions.count(session) == 0) {
		lInfo() << "[ToneManager] add new session [" << session << "]";
	}
	mSessions[session] = newState;
}

ToneManager::State ToneManager::getState(const std::shared_ptr<CallSession> &session) {
	auto search = mSessions.find(session);
    if (search != mSessions.end()) {
        return search->second;
    } else {
		return State::None;
    }
}

bool ToneManager::isAnotherSessionInState(const std::shared_ptr<CallSession> &me, ToneManager::State state) {
	for (auto it = mSessions.begin(); it != mSessions.end(); it++) {
		if (it->second == state && it->first != me) {
			return true;
		}
	}
	return false;
}

bool ToneManager::getSessionInState(ToneManager::State state, std::shared_ptr<CallSession> &session) {
	for (auto it = mSessions.begin(); it != mSessions.end(); it++) {
		if (it->second == state) {
			session = it->first;
			return true;
		}
	}
	return false;
}

bool ToneManager::isThereACall() {
	for (auto it = mSessions.begin(); it != mSessions.end(); it++) {
		if (it->second == State::Call) {
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------
// start
// ---------------------------------------------------

void ToneManager::doStartErrorTone(const std::shared_ptr<CallSession> &session, LinphoneReason reason) {
	lInfo() << "[ToneManager] " << __func__ << " [" << Utils::toString(reason) << "]";
	LinphoneToneDescription *tone = getToneFromReason(reason);

	if (tone) {
		if (tone->audiofile) {
			setState(session, State::Tone);
			playFile(tone->audiofile);
		} else if (tone->toneid != LinphoneToneUndefined) {
			setState(session, State::Tone);
			MSDtmfGenCustomTone dtmfTone = generateToneFromId(tone->toneid);
			playTone(dtmfTone);
		}
	}
}

void ToneManager::doStartNamedTone(LinphoneToneID toneId) {
	lInfo() << "[ToneManager] " << __func__ << " [" << Utils::toString(toneId) << "]";
	LinphoneToneDescription *tone = getToneFromId(toneId);

	if (tone && tone->audiofile) {
		playFile(tone->audiofile);
	} else {
		MSDtmfGenCustomTone dtmfTone = generateToneFromId(toneId);
		playTone(dtmfTone);
	}
}

void ToneManager::doStartRingbackTone(const std::shared_ptr<CallSession> &session) {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore()->getCCore();

	if (!lc->sound_conf.play_sndcard)
		return;

	MSSndCard *ringCard = lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.play_sndcard;

	if (lc->sound_conf.remote_ring) {
		ms_snd_card_set_stream_type(ringCard, MS_SND_CARD_STREAM_VOICE);
		lc->ringstream = ring_start(lc->factory, lc->sound_conf.remote_ring, 2000, ringCard);
	}
}

void ToneManager::doStartRingtone(const std::shared_ptr<CallSession> &session) {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore()->getCCore();
	if (isAnotherSessionInState(session, State::Call)) {
		/* play a tone within the context of the current call */
		doStartNamedTone(LinphoneToneCallWaiting);
	} else {
		MSSndCard *ringcard = lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
		if (ringcard) {
			ms_snd_card_set_stream_type(ringcard, MS_SND_CARD_STREAM_RING);
			linphone_ringtoneplayer_start(lc->factory, lc->ringtoneplayer, ringcard, lc->sound_conf.local_ring, 2000);
		}
	}
}

// ---------------------------------------------------
// stop
// ---------------------------------------------------

void ToneManager::doStop(const std::shared_ptr<CallSession> &session, ToneManager::State newState) {
	switch (getState(session)) {
		case ToneManager::Ringback:
			setState(session, newState);
			doStopRingbackTone();
			mStats->number_of_stopRingbackTone++;
			break;
		case ToneManager::Ringtone:
			setState(session, newState);
			doStopRingtone(session);
			/* start again ringtone in update() in case another call is in Ringtone state */
			mStats->number_of_stopRingtone++;
			break;
		case ToneManager::Tone:
			setState(session, newState);
			doStopTone();
			mStats->number_of_stopTone++;
			break;
		case ToneManager::Call:
			setState(session, newState);
			if (isAnotherSessionInState(session, ToneManager::Ringtone)) {
				doStopTone();
			}
			break;
		default:
			lInfo() << "[ToneManager] nothing to stop";
			break;
	}
}

void ToneManager::doStopRingbackTone() {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore()->getCCore();
	if (lc->ringstream) {
		ring_stop(lc->ringstream);
		lc->ringstream = NULL;
	}
}

void ToneManager::doStopTone() {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore()->getCCore();
	if (lc->ringstream) {
		ring_stop(lc->ringstream);
		lc->ringstream = NULL;
	}

	if (isThereACall()) {
		MSFilter *f = getAudioResource(ToneGenerator, NULL, FALSE);
		if (f != NULL) ms_filter_call_method_noarg(f, MS_DTMF_GEN_STOP);
	}
}

void ToneManager::doStopToneToPlaySomethingElse(const std::shared_ptr<CallSession> &session) {
	lInfo() << "[ToneManager] " << __func__;
	if (isAnotherSessionInState(session, State::Tone)) {
		doStopTone();
	}
}


void ToneManager::doStopRingtone(const std::shared_ptr<CallSession> &session) {
	lInfo() << "[ToneManager] " << __func__;
	if (isAnotherSessionInState(session, State::Call)) {
		/* stop the tone within the context of the current call */
		doStopTone();
	} else {
		LinphoneCore *lc = getCore()->getCCore();
		if (linphone_ringtoneplayer_is_started(lc->ringtoneplayer)) {
			linphone_ringtoneplayer_stop(lc->ringtoneplayer);
		}
	}
}

// ---------------------------------------------------
// sound
// ---------------------------------------------------

LinphoneStatus ToneManager::playFile(const char *audiofile) {
	LinphoneCore *lc = getCore()->getCCore();
	MSFilter *f = getAudioResource(LocalPlayer, lc->sound_conf.play_sndcard, true);
	int loopms = -1;
	if (!f) return -1;
	ms_filter_call_method(f, MS_PLAYER_SET_LOOP, &loopms);
	if (ms_filter_call_method(f, MS_PLAYER_OPEN, (void*) audiofile) != 0) {
		return -1;
	}
	ms_filter_call_method_noarg(f, MS_PLAYER_START);

	if (lc->ringstream && lc->ringstream->source) {
		ms_filter_add_notify_callback(lc->ringstream->source, &on_file_player_end, this, FALSE);
	}
	return 0;
}

MSDtmfGenCustomTone ToneManager::generateToneFromId(LinphoneToneID toneId) {
	MSDtmfGenCustomTone def;
	memset(&def, 0, sizeof(def));
	def.amplitude = 1;
	/*these are french tones, excepted the failed one, which is USA congestion tone (does not exist in France)*/
	switch(toneId) {
		case LinphoneToneCallOnHold:
		case LinphoneToneCallWaiting:
			def.duration=300;
			def.frequencies[0]=440;
			def.interval=2000;
		break;
		case LinphoneToneBusy:
			def.duration=500;
			def.frequencies[0]=440;
			def.interval=500;
			def.repeat_count=3;
		break;
		case LinphoneToneCallLost:
			def.duration=250;
			def.frequencies[0]=480;
			def.frequencies[0]=620;
			def.interval=250;
			def.repeat_count=3;
		break;
		default:
			lWarning() << "[ToneManager] Unhandled tone id.";
	}
	return def;
}

void ToneManager::playTone(MSDtmfGenCustomTone tone) {
    LinphoneCore *lc = getCore()->getCCore();
	MSFilter *f = getAudioResource(ToneGenerator, lc->sound_conf.play_sndcard, true);
	if (f == NULL) {
		lError() << "[ToneManager] No tone generator at this time !";
		return;
	}
	if (tone.duration > 0) {
		ms_filter_call_method(f, MS_DTMF_GEN_PLAY_CUSTOM, &tone);
		if (tone.repeat_count > 0) {
			int delay = (tone.duration + tone.interval) * tone.repeat_count + 1000;
			createTimerToCleanTonePlayer((unsigned int) delay);
		}
	}
}

MSFilter *ToneManager::getAudioResource(AudioResourceType rtype, MSSndCard *card, bool create) {
	LinphoneCore *lc = getCore()->getCCore();
	LinphoneCall *call = linphone_core_get_current_call(lc);
	AudioStream *stream = NULL;
	RingStream *ringstream;
	if (call) {
		stream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
	} else if (linphone_core_is_in_conference(lc)) {
		stream = linphone_conference_get_audio_stream(lc->conf_ctx);
	}
	if (stream) {
		if (rtype == ToneGenerator) return stream->dtmfgen;
		if (rtype == LocalPlayer) return stream->local_player;
		return NULL;
	}
	if (card && lc->ringstream && card != lc->ringstream->card) {
		ring_stop(lc->ringstream);
		lc->ringstream = NULL;
	}
	if (lc->ringstream == NULL) {
		float amp = lp_config_get_float(lc->config, "sound", "dtmf_player_amp", 0.1f);
		MSSndCard *ringcard = lc->sound_conf.lsd_card
			? lc->sound_conf.lsd_card
			: card
				? card
				: lc->sound_conf.ring_sndcard;

		if (ringcard == NULL) return NULL;
		if (!create) return NULL;

		ms_snd_card_set_stream_type(ringcard, MS_SND_CARD_STREAM_DTMF);
		ringstream = lc->ringstream = ring_start(lc->factory, NULL, 0, ringcard);
		ms_filter_call_method(lc->ringstream->gendtmf, MS_DTMF_GEN_SET_DEFAULT_AMPLITUDE, &amp);
	} else {
		ringstream = lc->ringstream;
	}
	if (rtype == ToneGenerator) return ringstream->gendtmf;
	if (rtype == LocalPlayer) return ringstream->source;
	return NULL;
}

LINPHONE_END_NAMESPACE
