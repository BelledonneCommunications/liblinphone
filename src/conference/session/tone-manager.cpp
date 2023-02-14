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

#include "tone-manager.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "call/call.h"
#include "conference/session/media-session.h"
#include "conference/session/media-session-p.h"
#include "conference/params/call-session-params-p.h"
#include "linphone/utils/general.h"
#include "core/core-p.h"
#include "conference_private.h"

#include <bctoolbox/defs.h>

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

ToneManager::ToneManager(Core &core) : mCore(core) {
	lInfo() << "[ToneManager] create ToneManager()";
	mStats = {0, 0, 0, 0, 0, 0, 0};
	
	/* Assign default tones */
	setTone(LinphoneReasonBusy, LinphoneToneBusy, NULL);
	setTone(LinphoneReasonGone, LinphoneToneCallEnd, NULL);
	setTone(LinphoneReasonNoResponse, LinphoneToneCallEnd, NULL);
	setTone(LinphoneReasonDeclined, LinphoneToneCallEnd, NULL);
	setTone(LinphoneReasonNone, LinphoneToneCallEnd, NULL);
	setTone(LinphoneReasonTransferred, LinphoneToneCallEnd, NULL);
	setTone(LinphoneReasonIOError, LinphoneToneCallLost, NULL);
	setTone(LinphoneReasonNotAnswered, LinphoneToneCallLost, NULL);
	setTone(LinphoneReasonServerTimeout, LinphoneToneCallLost, NULL);
	setTone(LinphoneReasonUnknown, LinphoneToneCallLost, NULL);
}

ToneManager::~ToneManager() {
}


std::shared_ptr<AudioDevice> ToneManager::getOutputDevice(const shared_ptr<CallSession> &session) const{
	std::shared_ptr<AudioDevice> device = nullptr;
	if (session == mSessionRinging){
		RingStream *ringStream = linphone_ringtoneplayer_get_stream(getCore().getCCore()->ringtoneplayer);
		if (ringStream) {
			MSSndCard *card = ring_stream_get_output_ms_snd_card(ringStream);
			if (card) {
				device = getCore().findAudioDeviceMatchingMsSoundCard(card);
			}
		}
	}else{
		if (mRingStream) {
			MSSndCard *card = ring_stream_get_output_ms_snd_card(mRingStream);
			if (card) {
				device = getCore().findAudioDeviceMatchingMsSoundCard(card);
			}
		}
	}
	return device;
}

void ToneManager::setOutputDevice(const shared_ptr<CallSession> &session, const std::shared_ptr<AudioDevice> &audioDevice){
	if (mSessionRinging == session){
		RingStream * ringStream = linphone_ringtoneplayer_get_stream(getCore().getCCore()->ringtoneplayer);
		if (ringStream) {
			ring_stream_set_output_ms_snd_card(ringStream, audioDevice->getSoundCard());
		}
		return;
	}
	if (mRingStream) {
		ring_stream_set_output_ms_snd_card(mRingStream, audioDevice->getSoundCard());
		return;
	}
}

void ToneManager::startRingbackTone() {
	LinphoneCore *lc = getCore().getCCore();
	lInfo() << "[ToneManager] " << __func__;
	mStats.number_of_startRingbackTone++;
	
	if (!lc->sound_conf.play_sndcard)
		return;

	MSSndCard *ringCard = lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.play_sndcard;

	std::shared_ptr<LinphonePrivate::Call> call = getCore().getCurrentCall();
	if (call) {
		auto audioDevice = call->getOutputAudioDevice();

		// If the user changed the audio device before the ringback started, the new value will be stored in the call playback card
		// It is NULL otherwise
		if (audioDevice) {
			ringCard = audioDevice->getSoundCard();
		}
	}
	destroyRingStream();
	
	if (lc->sound_conf.remote_ring) {
		ms_snd_card_set_stream_type(ringCard, MS_SND_CARD_STREAM_VOICE);
		mRingStream = ring_start(lc->factory, lc->sound_conf.remote_ring, 2000, (lc->use_files) ? NULL : ringCard);
	}
}

void ToneManager::stopRingbackTone() {
	lInfo() << "[ToneManager] " << __func__;
	mStats.number_of_stopRingbackTone++;
	destroyRingStream();
}

void ToneManager::startRingtone() {
	LinphoneCore *lc = getCore().getCCore();
	lInfo() << "[ToneManager] " << __func__;
	mStats.number_of_startRingtone++;
	MSSndCard *ringcard = lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
	if (ringcard) {
		ms_snd_card_set_stream_type(ringcard, MS_SND_CARD_STREAM_RING);
		linphone_ringtoneplayer_start(lc->factory, lc->ringtoneplayer, ringcard, lc->sound_conf.local_ring, 2000);
	}
}

void ToneManager::stopRingtone(){
	lInfo() << "[ToneManager] " << __func__;
	mStats.number_of_stopRingtone++;
	LinphoneCore *lc = getCore().getCCore();
	if (linphone_ringtoneplayer_is_started(lc->ringtoneplayer)) {
		linphone_ringtoneplayer_stop(lc->ringtoneplayer);
	}
}

void ToneManager::startErrorTone(LinphoneReason reason) {
	lInfo() << "[ToneManager] " << __func__;
	mStats.number_of_startErrorTone++;
	LinphoneToneDescription *tone = getToneFromReason(reason);

	if (tone) {
		if (tone->audiofile) {
			playFile(tone->audiofile);
		} else if (tone->toneid != LinphoneToneUndefined) {
			MSDtmfGenCustomTone dtmfTone = generateToneFromId(tone->toneid);
			playTone(dtmfTone);
		}
	}
}

void ToneManager::startNamedTone(LinphoneToneID toneId) {
	lInfo() << "[ToneManager] " << __func__;
	mStats.number_of_startNamedTone++;
	LinphoneToneDescription *tone = getToneFromId(toneId);
	
	if (tone && tone->audiofile) {
		playFile(tone->audiofile);
	} else {
		MSDtmfGenCustomTone dtmfTone = generateToneFromId(toneId);
		playTone(dtmfTone);
	}
}

void ToneManager::stopTone() {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore().getCCore();
	mStats.number_of_stopTone++;
	MSFilter *f = getAudioResource(LocalPlayer, lc->sound_conf.play_sndcard, false);
	if(f != NULL) ms_filter_call_method_noarg(f, MS_PLAYER_CLOSE);// MS_PLAYER is used while being in call
	f = getAudioResource(ToneGenerator, NULL, FALSE);
	if (f != NULL) ms_filter_call_method_noarg(f, MS_DTMF_GEN_STOP);
}

// ---------------------------------------------------
// linphone core public API entrypoints
// ---------------------------------------------------

void ToneManager::playDtmf(char dtmf, int duration) {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore().getCCore();

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

void ToneManager::stopDtmf() {
	lInfo() << "[ToneManager] " << __func__;
	MSFilter *f = getAudioResource(ToneGenerator, NULL, false);
	if (f != NULL) {
		ms_filter_call_method_noarg (f, MS_DTMF_GEN_STOP);
	}
}

LinphoneStatus ToneManager::playLocal(const char *audiofile) {
	lInfo() << "[ToneManager] " << __func__;
	return playFile(audiofile);
}

void ToneManager::startDtmfStream() {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore().getCCore();
	
	if (inCallOrConference()){
		lWarning() << "Dtmf player stream won't be started because there is a running call or conference.";
		return;
	}

	/*make sure ring stream is started*/
	getAudioResource(ToneGenerator, lc->sound_conf.ring_sndcard, true);

	mDtmfStreamStarted = true;
}

void ToneManager::stopDtmfStream() {
	lInfo() << "[ToneManager] " << __func__;
	if (!mDtmfStreamStarted) return;
	mDtmfStreamStarted = false;
	destroyRingStream();
}

LinphoneStatus ToneManager::playFile(const char *audiofile) {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore().getCCore();
	MSFilter *f = getAudioResource(LocalPlayer, lc->sound_conf.play_sndcard, true);
	int loopms = -1;
	if (!f) return -1;
	ms_filter_call_method(f, MS_PLAYER_SET_LOOP, &loopms);
	const char * audiofileToUse = audiofile;
	std::string fileString;// Declare here for the scope of audiofileToUse
	
	if(bctbx_file_exist(audiofile) != 0){// This file doesn't exist. Check it from Platform resource.
		char * basename = bctbx_basename(audiofile);
		fileString = static_cast<PlatformHelpers *>(lc->platform_helper)->getSoundResource(basename);
		bctbx_free(basename);
		if(fileString != "")
			audiofileToUse = fileString.c_str();
	}
	
	if (ms_filter_call_method(f, MS_PLAYER_OPEN, (void*)audiofileToUse ) != 0) {
		return -1;
	}
	ms_filter_call_method_noarg(f, MS_PLAYER_START);

	return 0;
}

void ToneManager::playTone(const MSDtmfGenCustomTone &tone) {
	lInfo() << "[ToneManager] " << __func__;
	LinphoneCore *lc = getCore().getCCore();
	shared_ptr<CallSession> session;
	MSSndCard * card = NULL;

	std::shared_ptr<LinphonePrivate::Call> call = getCore().getCurrentCall();
	if (call) session = call->getActiveSession();
	if (session) {
		auto audioDevice = std::dynamic_pointer_cast<MediaSession>(session)->getPrivate()->getCurrentOutputAudioDevice();
		if (audioDevice) {
			card = audioDevice->getSoundCard();
		}
	}

	// If card is null, use the default playcard
	if (card == NULL) {
		card = lc->sound_conf.play_sndcard;
	}

	MSFilter *f = getAudioResource(ToneGenerator, card, true);
	if (f == NULL) {
		lError() << "[ToneManager] No tone generator at this time !";
		return;
	}
	if (tone.duration > 0) {
		ms_filter_call_method(f, MS_DTMF_GEN_PLAY_CUSTOM, (void*)&tone);
	}
}

void ToneManager::scheduleRingStreamDestruction(){
	if (mRingStreamTimer) getCore().destroyTimer(mRingStreamTimer);
	mRingStreamTimer = getCore().createTimer([this]() -> bool{
		if (!mRingStream) return false;
		MSPlayerState state;
		bool_t isPlaying = false;
		if ((ms_filter_call_method(mRingStream->source, MS_PLAYER_GET_STATE, &state) == 0 && state == MSPlayerPlaying)
			|| (mRingStream->gendtmf && ms_filter_call_method(mRingStream->gendtmf, MS_DTMF_GEN_IS_PLAYING, &isPlaying) == 0 && isPlaying)
		) {
			/* The player is still playing, or the tone player is still toning, so repeat the timer.*/
			return true;
		}
		/* otherwise the ring stream is no longer needed, destroy it. */
		lInfo() << "RingStream no longer needed.";
		destroyRingStream();
		return false;
	} , 1000, "Tone player cleanup");
}


void ToneManager::destroyRingStream(){
	lInfo() << "[ToneManager] " << __func__;
	if (mRingStream){
		ring_stop(mRingStream);
		mRingStream = nullptr;
	}
	
	if (mRingStreamTimer){
		getCore().destroyTimer(mRingStreamTimer);
		mRingStreamTimer = nullptr;
		mStats.number_of_stopTone++;
	}
}

MSFilter *ToneManager::getAudioResource(AudioResourceType rtype, MSSndCard *card, bool create) {
	LinphoneCore *lc = getCore().getCCore();
	LinphoneCall *call = linphone_core_get_current_call(lc);
	AudioStream *stream = NULL;
	RingStream *ringstream;
	MSFilter * audioResource = NULL;
	float tmp;
	if (call) {
		stream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
	} else if (linphone_core_is_in_conference(lc)) {
		stream = linphone_conference_get_audio_stream(lc->conf_ctx);
	}
	if (stream) {
		if (rtype == ToneGenerator) audioResource = stream->dtmfgen;
		if (rtype == LocalPlayer) audioResource = stream->local_player;
	}
	if(!audioResource) {
		if (card && mRingStream && card != mRingStream->card) {
			ring_stop(mRingStream);
			mRingStream = nullptr;
		}
		if (mRingStream == NULL) {
		#if TARGET_OS_IPHONE
			tmp = 0.007f;
		#else
			tmp = 0.1f;
		#endif
			float amp = linphone_config_get_float(lc->config, "sound", "dtmf_player_amp", tmp);
			MSSndCard *ringcard = NULL;
			
			if (!lc->use_files) {
				ringcard = lc->sound_conf.lsd_card
				? lc->sound_conf.lsd_card
				: card
					? card
					: lc->sound_conf.ring_sndcard;
				if (ringcard == NULL) return NULL;
				ms_snd_card_set_stream_type(ringcard, MS_SND_CARD_STREAM_DTMF);
			}
			if (!create) return NULL;
	
			ringstream = mRingStream = ring_start(lc->factory, NULL, 0, ringcard); // passing a NULL ringcard if core if lc->use_files is enabled
			ms_filter_call_method(mRingStream->gendtmf, MS_DTMF_GEN_SET_DEFAULT_AMPLITUDE, &amp);
			/* A ring stream was started in the purpose of playing a tone. Make sure it is destroyed once the tone is finished. */
			scheduleRingStreamDestruction();
		} else {
			ringstream = mRingStream;
		}
		if (rtype == ToneGenerator) audioResource = ringstream->gendtmf;
		if (rtype == LocalPlayer) audioResource = ringstream->source;
	}
	return audioResource;
}

void ToneManager::freeAudioResources(){
	LinphoneCore *lc = getCore().getCCore();
	if (linphone_ringtoneplayer_is_started(lc->ringtoneplayer)) {
		linphone_ringtoneplayer_stop(lc->ringtoneplayer);
	}
	destroyRingStream();
	getPlatformHelpers(lc)->stopRinging();
}


// ---------------------------------------------------
// setup tones
// ---------------------------------------------------

LinphoneToneDescription *ToneManager::getToneFromReason(LinphoneReason reason) {
	const bctbx_list_t *elem;
	for (elem = getCore().getCCore()->tones; elem != NULL; elem = elem->next) {
		LinphoneToneDescription *tone = (LinphoneToneDescription *) elem->data;
		if (tone->reason==reason) return tone;
	}
	return NULL;
}

LinphoneToneDescription *ToneManager::getToneFromId(LinphoneToneID id) {
	const bctbx_list_t *elem;
	for (elem = getCore().getCCore()->tones; elem != NULL; elem = elem->next) {
		LinphoneToneDescription *tone = (LinphoneToneDescription *) elem->data;
		if (tone->toneid == id) return tone;
	}
	return NULL;
}

void ToneManager::setTone(LinphoneReason reason, LinphoneToneID id, const char *audiofile) {
	LinphoneCore *lc = getCore().getCCore();
	LinphoneToneDescription *tone = getToneFromReason(reason);

	if (tone) {
		lc->tones = bctbx_list_remove(lc->tones, tone);
		linphone_tone_description_destroy(tone);
	}
	tone = linphone_tone_description_new(reason, id, audiofile);
	lc->tones = bctbx_list_append(lc->tones, tone);
}

const LinphoneCoreToneManagerStats *ToneManager::getStats() const{
	return &mStats;
}

void ToneManager::resetStats() {
	mStats = {0, 0, 0, 0, 0, 0, 0};
}

MSDtmfGenCustomTone ToneManager::generateToneFromId(LinphoneToneID toneId) {
	MSDtmfGenCustomTone def;
	memset(&def, 0, sizeof(def));
	def.amplitude = 1;
	/*these are french tones, excepted the failed one, which comes USA congestion on mono-frequency*/
	switch(toneId) {
		case LinphoneToneCallOnHold:
			def.repeat_count=3;
			def.duration=300;
			def.frequencies[0]=440;
			def.interval=2000;
		break;
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
			//def.frequencies[0]=480;  // Second frequency that is hide
			def.frequencies[0]=620;
			def.interval=250;
			def.repeat_count=3;
		break;
		case LinphoneToneCallEnd:
			def.duration=200;
			def.frequencies[0]=480;
			def.interval=200;
			def.repeat_count=2;
			def.amplitude = 0.5f;// This tone can be in parallel of other calls. This will be played on a lighter amplitude
		break;
		default:
			lWarning() << "[ToneManager] Unhandled tone id.";
	}
	return def;
}

bool ToneManager::shouldPlayWaitingTone(const std::shared_ptr<CallSession> &session){
	shared_ptr<Call> currentCall = getCore().getCurrentCall();
	LinphoneCore *lc = getCore().getCCore();
	
	if (linphone_core_is_in_conference(lc)) return true;
	if (currentCall != nullptr && currentCall->getActiveSession() != session) {
		switch (currentCall->getActiveSession()->getState()){
			case CallSession::State::OutgoingInit:
			case CallSession::State::OutgoingEarlyMedia:
			/*case CallSession::State::OutgoingRinging:*/
			case CallSession::State::OutgoingProgress:
			case CallSession::State::Paused:
			case CallSession::State::Pausing:
				return false;
			break;
			case CallSession::State::StreamsRunning:
			case CallSession::State::PausedByRemote:
			{
				auto params = currentCall->getCurrentParams();
				if (params->getAudioDirection() != LinphoneMediaDirectionInactive && params->audioEnabled()) {
					return true;
				}
			}
			break;
			default:
				return true;
			break;
		}
	}
	return false;
}

void ToneManager::notifyIncomingCall(const std::shared_ptr<CallSession> &session){
	shared_ptr<Call> currentCall = getCore().getCurrentCall();
	LinphoneCore *lc = getCore().getCCore();
	
	if (mSessionRinging && mSessionRinging != session){
		// Already a session in ringing state.
		return;
	}
	
	if (shouldPlayWaitingTone(session)){
		/* already in a call or in a local conference. A tone indication will be used. */
		if (linphone_core_tone_indications_enabled(lc)){
			startNamedTone(LinphoneToneCallWaiting);
			/* Setup the way this incoming call notification has to be stopped */
			mSessionRingingStopFunction = [this](){
				this->stopTone();
			};
		}
	}else{
		/* Not already in a call or conference, so play the normal ringtone. */
		if (linphone_core_is_native_ringing_enabled(lc)){
			lInfo() << "Native (ie platform dependant) ringing is enabled, so not ringing from liblinphone.";
			return;
		}
		if (linphone_core_callkit_enabled(lc)){
			lInfo() << "Callkit mode is enabled, will not play ring tone from liblinphone.";
			return;
		}
		/*
		 For ios, only one write sound card is allowed. To ensure this, free audio resources before a new incoming call.
		 */
		freeAudioResources();
		startRingtone();
		/* Setup the way this incoming call notification has to be stopped */
		mSessionRingingStopFunction = [this](){
			this->stopRingtone();
		};
	}
	mSessionRinging = session;
}

void ToneManager::notifyOutgoingCallRinging(const std::shared_ptr<CallSession> &session){
	auto currentCall = getCore().getCurrentCall();
	if ((currentCall != nullptr && currentCall->getActiveSession() != session) || linphone_core_is_in_conference(getCore().getCCore())){
		lInfo() << "Will not play ringback tone, audio is already used in a call or conference.";
		return;
	}
	if (mSessionRingingBack != session){
		mSessionRingingBack = session;
		startRingbackTone();
	}
}

void ToneManager::notifyToneIndication(LinphoneReason reason){
	if (!linphone_core_tone_indications_enabled(getCore().getCCore())) return;
	startErrorTone(reason);
}

bool ToneManager::inCallOrConference()const{
	shared_ptr<Call> currentCall = getCore().getCurrentCall();
	return currentCall != nullptr || linphone_core_is_in_conference(getCore().getCCore());
}

/*
 * Below is the main logic of the ToneManager, entirely based on Call state notifications.
 */

shared_ptr<CallSession> ToneManager::lookupRingingSession() const{
	for (auto call : getCore().getCalls()){
		auto session = call->getActiveSession();
		if (session->getState() == CallSession::State::IncomingReceived){
			return session;
		}
	}
	return nullptr;
}

void ToneManager::cleanPauseTone(){
	if (mSessionPaused){
		stopTone();
		destroyRingStream(); // the "pause" tone is exclusively played by the RingStream API.
		mSessionPaused = nullptr;
	}
}

void ToneManager::updateRingingSessions(const std::shared_ptr<CallSession> &callSession, CallSession::State state){
	shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(callSession);
	if (mSessionRinging == session){
		if (state == CallSession::State::IncomingEarlyMedia && linphone_core_get_ring_during_incoming_early_media(getCore().getCCore())) {
			/* Let continue to ring.*/	
		}else{
			/* In all other cases, stop ringing */
			lInfo() << "[ToneManager] session " << mSessionRinging << " is no longer ringing.";
			mSessionRinging = nullptr;
			if (mSessionRingingStopFunction) {
				mSessionRingingStopFunction();
				mSessionRingingStopFunction = nullptr;
			}
			/* but check if there is another session for which we should keep ringing
			 Do this outside of call state callback. */
			getCore().doLater([this](){
				auto otherSession = lookupRingingSession();
				if (otherSession) {
					lInfo() << "[ToneManager] ringing session is now " << otherSession;
					notifyIncomingCall(otherSession);
				}
			});
		}
	}
	if (mSessionRingingBack == session){
		switch (state){
			case CallSession::State::OutgoingEarlyMedia:
				if (session->getCurrentParams()->getAudioDirection() != LinphoneMediaDirectionInactive){
					stopRingbackTone();
					mSessionRingingBack = nullptr;
				}
			break;
			case CallSession::State::OutgoingRinging:
				/* keep ringing - we might have several OutgoingRinging notifications if several 180 Ringing are received.*/
			break;
			default:
				stopRingbackTone();
				mSessionRingingBack = nullptr;
			break;
			
		}
	}
	if (mSessionPaused){
		if (mSessionPaused == callSession && state != CallSession::State::Paused && state != CallSession::State::Pausing){
			cleanPauseTone();
		}
	}
}

void ToneManager::prepareForNextState(const std::shared_ptr<CallSession> &callSession, CallSession::State nextState){
	shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(callSession);
	if (!session) return;
	if (!session->toneIndicationsEnabled()) return;
	
	updateRingingSessions(callSession, nextState);
	
	switch (nextState){
		case CallSession::State::StreamsRunning:
		case CallSession::State::PausedByRemote:
			/* In all those states we'll be starting streams shortly. No tone shall be playing. */
			
			freeAudioResources();
		break;
		default:
		break;
	}
}

void ToneManager::notifyState(const std::shared_ptr<CallSession> &callSession, CallSession::State state){
	shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(callSession);
	if (!session) return;
	if (!session->toneIndicationsEnabled()) return;
	
	updateRingingSessions(callSession, state);
	
	switch (state){
		case CallSession::State::IncomingReceived:
			cleanPauseTone();
			notifyIncomingCall(session);
		break;
		case CallSession::State::OutgoingProgress:
			cleanPauseTone();
		break;
		case CallSession::State::OutgoingRinging:
			cleanPauseTone();
			notifyOutgoingCallRinging(session);
		break;
		case CallSession::State::OutgoingEarlyMedia:
			if (mSessionRingingBack == nullptr && (session->getCurrentParams()->getAudioDirection() == LinphoneMediaDirectionInactive)) {
				cleanPauseTone();
				notifyOutgoingCallRinging(session);
			}
		break;
		case CallSession::State::Pausing:
		{
			if (session->pausedByApp() && (getCore().getCallCount() == 1) && !linphone_core_is_in_conference(getCore().getCCore()) && mSessionPaused == nullptr ) {
				mSessionPaused = session;
				startNamedTone(LinphoneToneCallOnHold);
			}
		}
		break;
		case CallSession::State::End:
		case CallSession::State::Error:
		{
			LinphoneReason reason = session->getReason();
			CallSession::State pState = session->getPreviousState();
			// Do not play tone for incoming calls when declining them with Busy or DoNotDisturb reason or incoming calls not connected
			if (session->getDirection() == LinphoneCallOutgoing || (pState != CallSession::State::IncomingReceived && pState != CallSession::State::IncomingEarlyMedia && pState != CallSession::State::PushIncomingReceived && reason != LinphoneReasonBusy && reason != LinphoneReasonDoNotDisturb)) {
				notifyToneIndication(reason);
			}
		}
		break;
		default:
		break;
	}
}




LINPHONE_END_NAMESPACE
