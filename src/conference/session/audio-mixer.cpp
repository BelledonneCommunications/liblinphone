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

#include <bctoolbox/defs.h>

#include "mediastreamer2/msvolume.h"

#include "linphone/core.h"
#include "mixers.h"
#include "player/call-player.h"
#include "private.h"
#include "streams.h"

LINPHONE_BEGIN_NAMESPACE

MS2AudioMixer::MS2AudioMixer(MixerSession &session) : StreamMixer(session) {
	MSAudioConferenceParams ms_conf_params;
	ms_conf_params.samplerate = linphone_config_get_int(mSession.getCCore()->config, "sound", "conference_rate", 16000);
	ms_conf_params.active_talker_callback = &MS2AudioMixer::sOnActiveTalkerChanged;
	ms_conf_params.security_level = StreamMixer::securityLevelToMsSecurityLevel(session.getSecurityLevel());
	LinphoneConfig *config = linphone_core_get_config(mSession.getCCore());
	ms_conf_params.mode = static_cast<MSConferenceMode>(
	    linphone_config_get_int(config, "sound", "conference_mode",
	                            MSConferenceModeMixer)); // this core setting is also used in MS2AudioStream::render
	ms_conf_params.user_data = this;
	mConference = ms_audio_conference_new(&ms_conf_params, mSession.getCCore()->factory);
}

MS2AudioMixer::~MS2AudioMixer() {
	if (mTimer) {
		mSession.getCore().destroyTimer(mTimer);
	}
	if (mRecordEndpoint) {
		stopRecording();
	}
	if (mPlayer) {
		mPlayer->close();
	}
	if (mLocalEndpoint) {
		removeLocalParticipant();
	}
	ms_audio_conference_destroy(mConference);
}

void MS2AudioMixer::addListener(AudioMixerListener *listener) {
	if (mTimer == nullptr) {
		// Start the monitoring of the active talker since somebody wants this information.
		mTimer = mSession.getCore().createTimer(
		    [this]() -> bool {
			    ms_audio_conference_process_events(mConference);
			    return true;
		    },
		    50, "AudioConference events timer");
	}
	mListeners.push_back(listener);
}

void MS2AudioMixer::removeListener(AudioMixerListener *listener) {
	mListeners.remove(listener);
}

void MS2AudioMixer::sOnActiveTalkerChanged(MSAudioConference *audioconf, MSAudioEndpoint *ep) {
	const MSAudioConferenceParams *params = ms_audio_conference_get_params(audioconf);
	MS2AudioMixer *zis = static_cast<MS2AudioMixer *>(params->user_data);
	zis->onActiveTalkerChanged(ep);
}

void MS2AudioMixer::onActiveTalkerChanged(MSAudioEndpoint *ep) {
	StreamsGroup *sg = (StreamsGroup *)ms_audio_endpoint_get_user_data(ep);
	for (auto &l : mListeners) {
		l->onActiveTalkerChanged(sg);
	}
}

void MS2AudioMixer::connectEndpoint(Stream *as, MSAudioEndpoint *endpoint, bool muted) {
	ms_audio_endpoint_set_user_data(endpoint, &as->getGroup());
	ms_audio_conference_add_member(mConference, endpoint);
	ms_audio_conference_mute_member(mConference, endpoint, muted);
}

void MS2AudioMixer::disconnectEndpoint(BCTBX_UNUSED(Stream *as), MSAudioEndpoint *endpoint) {
	ms_audio_endpoint_set_user_data(endpoint, nullptr);
	ms_audio_conference_remove_member(mConference, endpoint);
}

RtpProfile *MS2AudioMixer::sMakeDummyProfile(int samplerate) {
	RtpProfile *prof = rtp_profile_new("dummy");
	OrtpPayloadType *pt = payload_type_clone(&payload_type_l16_mono);
	pt->clock_rate = samplerate;
	rtp_profile_set_payload(prof, 0, pt);
	return prof;
}

void MS2AudioMixer::addLocalParticipant() {
	LinphoneCore *core = getSession().getCCore();
	AudioStream *st = audio_stream_new(core->factory, 65000, 65001, FALSE);
	MSSndCard *playcard = core->sound_conf.lsd_card ? core->sound_conf.lsd_card : core->sound_conf.play_sndcard;
	MSSndCard *captcard = core->sound_conf.capt_sndcard;

	// If playfile are supplied don't use soundcards
	if (core->use_files) {
		captcard = playcard = nullptr;
	}
	const MSAudioConferenceParams *params = ms_audio_conference_get_params(mConference);
	mLocalDummyProfile = sMakeDummyProfile(params->samplerate);
	audio_stream_start_full(st, mLocalDummyProfile, "127.0.0.1", 65000, "127.0.0.1", 65001, 0, 40, nullptr, nullptr,
	                        playcard, captcard, linphone_core_echo_cancellation_enabled(core));
	MS2AudioStream::postConfigureAudioStream(st, core, FALSE);
	mLocalParticipantStream = st;
	mLocalEndpoint = ms_audio_endpoint_get_from_stream(st, FALSE, params->mode);
	ms_message("Conference: adding local endpoint");
	ms_audio_conference_add_member(mConference, mLocalEndpoint);
	enableMic(mLocalMicEnabled);
}

void MS2AudioMixer::removeLocalParticipant() {
	if (mPlayer) mPlayer = nullptr; // Stop audio file playing
	if (mLocalEndpoint) {
		ms_audio_conference_remove_member(mConference, mLocalEndpoint);
		ms_audio_endpoint_release_from_stream(mLocalEndpoint);
		mLocalEndpoint = nullptr;
		audio_stream_stop(mLocalParticipantStream);
		mLocalParticipantStream = nullptr;
		rtp_profile_destroy(mLocalDummyProfile);
		mLocalDummyProfile = nullptr;
	}
}

void MS2AudioMixer::enableLocalParticipant(bool value) {
	/* Create a dummy audiostream in order to extract the local part of it */
	/* network address and ports have no meaning and are not used here. */
	if (value && !mLocalParticipantStream) {
		addLocalParticipant();
	} else if (!value && mLocalParticipantStream) {
		removeLocalParticipant();
	}
}

void MS2AudioMixer::setRecordPath(const std::string &path) {
	mRecordPath = path;
}

void MS2AudioMixer::enableMic(bool value) {
	mLocalMicEnabled = value;
	if (mLocalEndpoint) {
		bool coreMicrophoneEnabled = !!linphone_core_mic_enabled(mSession.getCore().getCCore());
		if (!coreMicrophoneEnabled) {
			lWarning() << "Microphone of the local participant of conference will be muted because the microphone is "
			              "disabled in the core settings";
		}
		bool enabled = (value && coreMicrophoneEnabled);
		bctbx_message("AudioMixer[%p]: mic of local participnt is [%s].", this, enabled ? "enabled" : "disabled");

		ms_audio_conference_mute_member(mConference, mLocalEndpoint, !enabled);
	}
}

bool MS2AudioMixer::micEnabled() const {
	return mLocalMicEnabled;
}

void MS2AudioMixer::enableSpeaker(BCTBX_UNUSED(bool value)) {
}

bool MS2AudioMixer::speakerEnabled() const {
	return false;
}

bool MS2AudioMixer::startRecording() {
	if (mRecordPath.empty()) {
		lError() << "MS2AudioMixer:startRecording(): no path set.";
		return false;
	}
	if (!mRecordEndpoint) {
		mRecordEndpoint = ms_audio_endpoint_new_recorder(getSession().getCCore()->factory, mRecordPath.c_str());
		ms_audio_conference_add_member(mConference, mRecordEndpoint);
	}
	return ms_audio_recorder_endpoint_start(mRecordEndpoint) != -1;
}

void MS2AudioMixer::stopRecording() {
	if (!mRecordEndpoint) {
		lWarning() << "MS2AudioMixer::stopRecording(): no record currently active";
		return;
	}
	ms_audio_recorder_endpoint_stop(mRecordEndpoint);
	ms_audio_conference_remove_member(mConference, mRecordEndpoint);
	ms_audio_endpoint_destroy(mRecordEndpoint);
	mRecordEndpoint = nullptr;
}

bool MS2AudioMixer::isRecording() {
	return mRecordEndpoint != nullptr;
}

float MS2AudioMixer::getPlayVolume() {
	AudioStream *st = mLocalParticipantStream;
	if (st && st->volrecv) {
		float vol = 0;
		ms_filter_call_method(st->volrecv, MS_VOLUME_GET, &vol);
		return vol;
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MS2AudioMixer::getRecordVolume() {
	AudioStream *st = mLocalParticipantStream;
	if (st && st->volsend && mLocalMicEnabled) {
		float vol = 0;
		ms_filter_call_method(st->volsend, MS_VOLUME_GET, &vol);
		return vol;
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MS2AudioMixer::getMicGain() {
	return 0.0;
}

void MS2AudioMixer::setMicGain(BCTBX_UNUSED(float value)) {
}

float MS2AudioMixer::getSpeakerGain() {
	return 0.0;
}

void MS2AudioMixer::setSpeakerGain(BCTBX_UNUSED(float value)) {
}

bool MS2AudioMixer::supportsTelephoneEvents() {
	return false;
}

void MS2AudioMixer::sendDtmf(BCTBX_UNUSED(int dtmf)) {
}

void MS2AudioMixer::enableEchoCancellation(BCTBX_UNUSED(bool value)) {
}

bool MS2AudioMixer::echoCancellationEnabled() const {
	return linphone_core_echo_cancellation_enabled(getSession().getCCore());
}

void MS2AudioMixer::setInputDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if (mLocalParticipantStream) {
		audio_stream_set_input_ms_snd_card(mLocalParticipantStream, audioDevice->getSoundCard());
	}
}

void MS2AudioMixer::setOutputDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if (mLocalParticipantStream) {
		audio_stream_set_output_ms_snd_card(mLocalParticipantStream, audioDevice->getSoundCard());
	}
}

std::shared_ptr<AudioDevice> MS2AudioMixer::getInputDevice() const {
	if (mLocalParticipantStream) {
		MSSndCard *card = audio_stream_get_input_ms_snd_card(mLocalParticipantStream);
		return getSession().getCore().findAudioDeviceMatchingMsSoundCard(card);
	}
	return nullptr;
}

std::shared_ptr<AudioDevice> MS2AudioMixer::getOutputDevice() const {
	if (mLocalParticipantStream) {
		MSSndCard *card = audio_stream_get_output_ms_snd_card(mLocalParticipantStream);
		return getSession().getCore().findAudioDeviceMatchingMsSoundCard(card);
	}
	return nullptr;
}

AudioStream *MS2AudioMixer::getAudioStream() {
	return mLocalParticipantStream;
}

MSAudioConference *MS2AudioMixer::getAudioConference() {
	return mConference;
}

std::shared_ptr<Player> MS2AudioMixer::getPlayer() const {
	if (!mPlayer) const_cast<MS2AudioMixer *>(this)->createPlayer();
	return mPlayer;
}

void MS2AudioMixer::createPlayer() {
	mPlayer = CallPlayer::create<CallPlayer>(getSession().getCore().getSharedFromThis(), getAudioConference());
}

void MS2AudioMixer::enableBaudotDetection(BCTBX_UNUSED(bool enabled)) {
}

void MS2AudioMixer::setBaudotMode(BCTBX_UNUSED(LinphoneBaudotMode mode)) {
}

void MS2AudioMixer::setBaudotSendingStandard(BCTBX_UNUSED(LinphoneBaudotStandard standard)) {
}

void MS2AudioMixer::setBaudotPauseTimeout(BCTBX_UNUSED(uint8_t seconds)) {
}

void MS2AudioMixer::sendBaudotCharacter(BCTBX_UNUSED(char character)) {
}

LINPHONE_END_NAMESPACE
