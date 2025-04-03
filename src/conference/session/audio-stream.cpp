/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include <cmath>

#include "bctoolbox/defs.h"

#include "mediastreamer2/flowcontrol.h"
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msvolume.h"

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/client-conference.h"
#include "conference/conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "core/core.h"
#include "linphone/core.h"
#include "media-session-p.h"
#include "media-session.h"
#include "mixers.h"
#include "ms2-streams.h"
#include "nat/ice-service.h"

using namespace ::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2AudioStream implementation.
 */

MS2AudioStream::MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params) {
	string bindIp = getBindIp();
	mStream = audio_stream_new2(getCCore()->factory, bindIp.empty() ? nullptr : bindIp.c_str(), mPortConfig.rtpPort,
	                            mPortConfig.rtcpPort);
	mIsOfferer = params.localIsOfferer;
	mStream->disable_record_on_mute = getCCore()->sound_conf.disable_record_on_mute;

	/* initialize ZRTP if it supported as default encryption or as optional encryption and capability negotiation is
	 * enabled */
	if (!mSessions.zrtp_context && getMediaSessionPrivate().isMediaEncryptionAccepted(LinphoneMediaEncryptionZRTP)) {
		initZrtp();
	}
	initializeSessions((MediaStream *)mStream);
}

void MS2AudioStream::audioStreamIsSpeakingCb(uint32_t speakerSsrc, bool_t isSpeaking) {
	getMediaSession().notifySpeakingDevice(speakerSsrc, isSpeaking);
}

void MS2AudioStream::sAudioStreamIsSpeakingCb(void *userData, uint32_t speakerSsrc, bool_t isSpeaking) {
	MS2AudioStream *as = static_cast<MS2AudioStream *>(userData);
	as->audioStreamIsSpeakingCb(speakerSsrc, isSpeaking);
}

void MS2AudioStream::audioStreamIsMutedCb(uint32_t ssrc, bool_t muted) {
	getMediaSession().notifyMutedDevice(ssrc, muted);
}

void MS2AudioStream::sAudioStreamIsMutedCb(void *userData, uint32_t ssrc, bool_t muted) {
	MS2AudioStream *as = static_cast<MS2AudioStream *>(userData);
	as->audioStreamIsMutedCb(ssrc, muted);
}

void MS2AudioStream::audioStreamActiveSpeakerCb(uint32_t ssrc) {
	const auto conference = getCore().findConference(getMediaSession().getSharedFromThis(), false);
	if (conference) {
		const auto clientConference = dynamic_pointer_cast<ClientConference>(conference);
		if (clientConference) clientConference->notifyLouderSpeaker(ssrc);
	}
}

void MS2AudioStream::sAudioStreamActiveSpeakerCb(void *userData, uint32_t ssrc) {
	MS2AudioStream *as = static_cast<MS2AudioStream *>(userData);
	as->audioStreamActiveSpeakerCb(ssrc);
}

void MS2AudioStream::configure(BCTBX_UNUSED(const OfferAnswerContext &params)) {
}

void MS2AudioStream::initZrtp() {
	auto peerAddr = getMediaSession().getRemoteAddress()->clone()->toSharedPtr();
	auto selfAddr = getMediaSession().getLocalAddress()->clone()->toSharedPtr();
	peerAddr->clean();
	selfAddr->clean();
	char *peerUri = bctbx_strdup(peerAddr->asStringUriOnly().c_str());
	char *selfUri = bctbx_strdup(selfAddr->asStringUriOnly().c_str());

	MSZrtpParams zrtpParams;
	zrtpCacheAccess zrtpCacheInfo = linphone_core_get_zrtp_cache_access(getCCore());

	memset(&zrtpParams, 0, sizeof(MSZrtpParams));
	/* media encryption of current params will be set later when zrtp is activated */
	zrtpParams.zidCacheDB = zrtpCacheInfo.db;
	zrtpParams.zidCacheDBMutex = zrtpCacheInfo.dbMutex;
	zrtpParams.peerUri = peerUri;
	zrtpParams.selfUri = selfUri;
	zrtpParams.acceptGoClear = !!linphone_core_zrtp_go_clear_enabled(getCCore());
	setZrtpCryptoTypesParameters(&zrtpParams, mIsOfferer);
	audio_stream_enable_zrtp(mStream, &zrtpParams);
	if (peerUri) bctbx_free(peerUri);
	if (selfUri) bctbx_free(selfUri);
}

void MS2AudioStream::setZrtpCryptoTypesParameters(MSZrtpParams *params, bool localIsOfferer) {
	const MSCryptoSuite *srtpSuites = linphone_core_get_srtp_crypto_suites_array(getCCore());
	if (srtpSuites) {
		bool aes1 = false;
		bool aes3 = false;
		bool hs32 = false;
		bool hs80 = false;
		bool gcm = false;
		for (int i = 0; (srtpSuites[i] != MS_CRYPTO_SUITE_INVALID) && (i < MS_MAX_ZRTP_CRYPTO_TYPES); i++) {
			switch (srtpSuites[i]) {
				case MS_AES_128_SHA1_32:
					if (!aes1) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					if (!hs32) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS32;
					aes1 = true;
					hs32 = true;
					break;
				case MS_AES_128_SHA1_80_NO_AUTH:
				case MS_AES_128_SHA1_32_NO_AUTH:
					if (!aes1) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					aes1 = true;
					break;
				case MS_AES_128_SHA1_80_SRTP_NO_CIPHER:
				case MS_AES_128_SHA1_80_SRTCP_NO_CIPHER:
				case MS_AES_128_SHA1_80_NO_CIPHER:
					if (!hs80) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					hs80 = true;
					break;
				case MS_AES_128_SHA1_80:
					if (!aes1) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					if (!hs80) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					hs80 = true;
					aes1 = true;
					break;
				case MS_AES_CM_256_SHA1_80:
					lWarning() << "Deprecated crypto suite MS_AES_CM_256_SHA1_80, use MS_AES_256_SHA1_80 instead";
					BCTBX_NO_BREAK;
				case MS_AES_256_SHA1_80:
					if (!aes3) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					if (!hs80) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					hs80 = true;
					aes3 = true;
					break;
				case MS_AES_256_SHA1_32:
					if (!aes3) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					if (!hs80) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS32;
					hs32 = true;
					aes3 = true;
					break;
				case MS_AEAD_AES_128_GCM:
					if (!aes1) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					if (!gcm) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_GCM;
					gcm = true;
					aes1 = true;
					break;
				case MS_AEAD_AES_256_GCM:
					if (!aes3) params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					if (!gcm) params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_GCM;
					gcm = true;
					aes1 = true;
					break;
				case MS_CRYPTO_SUITE_INVALID:
					break;
			}
		}
	}

	/* linphone_core_get_zrtp_cipher_suites is used to determine sensible defaults; here each can be overridden */
	MsZrtpCryptoTypesCount ciphersCount = linphone_core_get_zrtp_cipher_suites(
	    getCCore(), params->ciphers); /* if not present in config file, params->ciphers is not modified */
	if (ciphersCount !=
	    0) /* Use zrtp_cipher_suites config only when present, keep config from srtp_crypto_suite otherwise */
		params->ciphersCount = ciphersCount;
	params->hashesCount = linphone_core_get_zrtp_hash_suites(getCCore(), params->hashes);
	MsZrtpCryptoTypesCount authTagsCount = linphone_core_get_zrtp_auth_suites(
	    getCCore(), params->authTags); /* If not present in config file, params->authTags is not modified */
	if (authTagsCount != 0)
		params->authTagsCount = authTagsCount; /* Use zrtp_auth_suites config only when present, keep config from
		                                          srtp_crypto_suite otherwise */
	params->sasTypesCount = linphone_core_get_zrtp_sas_suites(getCCore(), params->sasTypes);
	params->keyAgreementsCount = linphone_core_get_zrtp_key_agreement_suites(getCCore(), params->keyAgreements);

	/* ZRTP Autostart: avoid starting a ZRTP session before we got peer's lime Ik */
	/* We MUST start if ZRTP is not active otherwise we break RFC compatibility */
	/* When we are not offerer, we received the lime-Ik in SDP so we can start upon ZRTP Hello reception */
	params->autoStart =
	    (getMediaSessionPrivate().getNegotiatedMediaEncryption() != LinphoneMediaEncryptionZRTP) || (!localIsOfferer);
}

void MS2AudioStream::configureAudioStream() {
	if (linphone_core_echo_limiter_enabled(getCCore())) {
		string type = linphone_config_get_string(linphone_core_get_config(getCCore()), "sound", "el_type", "mic");
		if (type == "mic") audio_stream_enable_echo_limiter(mStream, ELControlMic);
		else if (type == "full") audio_stream_enable_echo_limiter(mStream, ELControlFull);
	}

	// Equalizer location in the graph: 'mic' = in input graph, otherwise in output graph.
	// Any other value than mic will default to output graph for compatibility.
	string location = linphone_config_get_string(linphone_core_get_config(getCCore()), "sound", "eq_location", "hp");
	mStream->eq_loc = (location == "mic") ? MSEqualizerMic : MSEqualizerHP;
	lInfo() << "Equalizer location: " << location;

	audio_stream_enable_gain_control(mStream, true);
	if (linphone_core_echo_cancellation_enabled(getCCore())) {
		int len = linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "ec_tail_len", 0);
		int delay = linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "ec_delay", 0);
		int framesize = linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "ec_framesize", 0);
		audio_stream_set_echo_canceller_params(mStream, len, delay, framesize);
		// If a positive delay has been measured by the echo canceller calibration, then we cannot rely on the hardware
		// echo canceller (if any), so we force the use of a software one.
		audio_stream_force_software_echo_canceller(mStream, delay > 0);
		if (mStream->ec) {
			char *statestr = static_cast<char *>(ms_malloc0(ecStateMaxLen));
			if (linphone_config_relative_file_exists(linphone_core_get_config(getCCore()), ecStateStore) &&
			    (linphone_config_read_relative_file(linphone_core_get_config(getCCore()), ecStateStore, statestr,
			                                        ecStateMaxLen) == 0)) {
				ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_SET_STATE_STRING, statestr);
			}
			ms_free(statestr);
		}
	}
	audio_stream_enable_automatic_gain_control(mStream, linphone_core_agc_enabled(getCCore()));
	bool_t enabled = !!linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "noisegate", 0);
	audio_stream_enable_noise_gate(mStream, enabled);
	audio_stream_set_features(mStream, linphone_core_get_audio_features(getCCore()));
}

bool MS2AudioStream::prepare() {
	if (getIceService().isActive()) {
		audio_stream_prepare_sound(mStream, nullptr, nullptr);
	}
	MS2Stream::prepare();
	return false;
}

void MS2AudioStream::sessionConfirmed(const OfferAnswerContext &ctx) {
	if (mStartZrtpLater) {
		lInfo() << "Starting zrtp late";
		startZrtpPrimaryChannel(ctx);
		mStartZrtpLater = false;
	}
}

void MS2AudioStream::finishPrepare() {
	MS2Stream::finishPrepare();
	audio_stream_unprepare_sound(mStream);
}

MediaStream *MS2AudioStream::getMediaStream() const {
	return mStream ? &mStream->ms : nullptr;
}

void MS2AudioStream::setupMediaLossCheck(bool_t isPaused) {
	int disconnectTimeout = linphone_core_get_nortp_timeout(getCCore());
	if (isPaused) {
		disconnectTimeout = linphone_core_get_nortp_onhold_timeout(getCCore());
	}
	if (disconnectTimeout == 0) {
		lInfo() << "No RTP timeout disabled";
		return;
	}
	mMediaLostCheckTimer = getCore().createTimer(
	    [this, disconnectTimeout]() -> bool {
		    if (!audio_stream_alive(mStream, disconnectTimeout)) {
			    getMediaSession().notifyLossOfMediaDetected();
		    }
		    return true;
	    },
	    1000, "Audio stream alive check");
}

void MS2AudioStream::audioRouteChangeCb(void *userData,
                                        bool_t needReloadSoundDevices,
                                        char *newInputDevice,
                                        char *newOutputDevice) {
	Core *core = static_cast<Core *>(userData);

	std::string newInput, newOutput;
	if (newInputDevice) newInput = std::string(newInputDevice);
	if (newOutputDevice) newOutput = std::string(newOutputDevice);

	core->doLater([core, newInput, newOutput, needReloadSoundDevices]() {
		if (needReloadSoundDevices) {
			linphone_core_reload_sound_devices(core->getCCore());
		}

		// Make sure that the current device the core is using match the reality of the IOS audio route. If not, set it
		// properly
		bool inputRequiresUpdate = !newInput.empty();
		bool outputRequiresUpdate = !newOutput.empty();

		if (inputRequiresUpdate || outputRequiresUpdate) {
			auto devices = core->getExtendedAudioDevices();
			for (auto device : devices) {
				std::string deviceName = device->getDeviceName();

				if (inputRequiresUpdate && newInput == deviceName) {
					core->setInputAudioDevice(device);
					inputRequiresUpdate = false;
				}
				if (outputRequiresUpdate && newOutput == deviceName) {
					core->setOutputAudioDevice(device);
					outputRequiresUpdate = false;
				}
			}
		}
		if (inputRequiresUpdate) {
			ms_warning("Current audio route input is '%s', but we could not find the matching device in the linphone "
			           "devices list",
			           newInput.c_str());
		}
		if (outputRequiresUpdate) {
			ms_warning("Current audio route output is '%s', but we could not find the matching device in the linphone "
			           "devices list",
			           newOutput.c_str());
		}

		// Notify the filter that the audio route changed
		core->soundcardAudioRouteChanged();
	});
}

void MS2AudioStream::configureConference() {
	// When in conference and it is remote, always enable the local mix
	const auto conference = getCore().findConference(getMediaSession().getSharedFromThis(), false);
	if (conference) {
		const auto remoteConference = dynamic_pointer_cast<ClientConference>(conference);
		if (remoteConference) { // we are a client, enable local mix of several audio stream
			// This has to be called before audio_stream_start so that the AudioStream can configure it's filters
			// properly
			media_stream_enable_conference_local_mix(&mStream->ms, TRUE);
		} else { // when conference is local(we are a server), retrieve the full packet mode in the config
			LinphoneConfig *config = linphone_core_get_config(getCCore());
			if (static_cast<MSConferenceMode>(linphone_config_get_int(
			        config, "sound", "conference_mode", MSConferenceModeMixer)) == MSConferenceModeRouterFullPacket) {
				media_stream_enable_transfer_mode(&mStream->ms, TRUE);
			}
		}
	}
}
void MS2AudioStream::render(const OfferAnswerContext &params, CallSession::State targetState) {
	const auto &stream = params.getResultStreamDescription();

	bool basicChangesHandled = handleBasicChanges(params, targetState);

	auto outputAudioDevice = getMediaSessionPrivate().getCurrentOutputAudioDevice();
	MS2AudioMixer *audioMixer = getAudioMixer();
	MSSndCard *playcard = nullptr;
	// try to get currently used playcard if it was already set
	if (outputAudioDevice) {
		playcard = outputAudioDevice->getSoundCard();
	}

	auto expectedStreamType =
	    (targetState == CallSession::State::IncomingEarlyMedia) ? MS_SND_CARD_STREAM_RING : MS_SND_CARD_STREAM_VOICE;

	auto conference = getCore().findConference(getMediaSession().getSharedFromThis(), false);
	if (conference) {
		if (conference->getCurrentParams()->getSecurityLevel() == ConferenceParamsInterface::SecurityLevel::EndToEnd) {
			if (getMediaSessionPrivate().getParams()->getPrivate()->getInConference()) {
				lInfo() << "MS2Audiostream::render End2End encrypted local conference";
				setEktMode(MS_EKT_TRANSFER);
			} else {
				lInfo() << "MS2Audiostram::render End2End encrypted client conference";
				setEktMode(MS_EKT_ENABLED);
			}
		}
	}

	if (basicChangesHandled) {
		if (getState() == Running) {
			bool muted = mMuted;
			MS2Stream::render(params, targetState); // MS2Stream::render() may decide to unmute.
			if (muted && !mMuted) {
				if (audioMixer) {
					lInfo() << "Early media finished, unmuting audio input and will connect audio to conference.";
					mRestartStreamRequired = true;
				} else {
					lInfo() << "Early media finished, unmuting audio input...";
					enableMic(micEnabled());
				}
			}

			if (!mRestartStreamRequired && playcard) {
				auto streamType = ms_snd_card_get_stream_type(playcard);
				mRestartStreamRequired = (streamType != expectedStreamType);
				if (mRestartStreamRequired)
					lInfo() << "Restarting stream because the stream type " << streamType << " of current play card "
					        << std::string(ms_snd_card_get_name(playcard)) << " doesn't match the expected one "
					        << expectedStreamType << "...";
			}

			if (mRestartStreamRequired) {
				stop();
				mRestartStreamRequired = false;
			} else {
				return;
			}
		} else {
			mRestartStreamRequired = false;
			return;
		}
	}

	int usedPt = -1;
	string onHoldFile = "";
	RtpProfile *audioProfile = makeProfile(params.resultMediaDescription, stream, &usedPt);
	if (usedPt == -1) {
		lError() << "No payload types configured for this stream !";
		stop();
		return;
	}

	bool ok = true;
	if (isMain()) {
		OrtpPayloadType *pt = rtp_profile_get_payload(audioProfile, usedPt);
		getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedAudioCodec(
		    pt ? PayloadType::create(getCore().getSharedFromThis(), pt) : nullptr);
	}

	auto streamDirection = stream.getDirection();
	if (streamDirection == SalStreamSendOnly) media_stream_set_direction(&mStream->ms, MediaStreamSendOnly);
	else if (streamDirection == SalStreamRecvOnly) media_stream_set_direction(&mStream->ms, MediaStreamRecvOnly);
	else if (streamDirection == SalStreamSendRecv) media_stream_set_direction(&mStream->ms, MediaStreamSendRecv);

	// If stream doesn't have a playcard associated with it, then use the default values
	if (!playcard)
		playcard =
		    getCCore()->sound_conf.lsd_card ? getCCore()->sound_conf.lsd_card : getCCore()->sound_conf.play_sndcard;

	if (!playcard) lWarning() << "No card defined for playback!";

	auto inputAudioDevice = getMediaSessionPrivate().getCurrentInputAudioDevice();
	MSSndCard *captcard = nullptr;
	// try to get currently used playcard if it was already set
	if (inputAudioDevice) {
		captcard = inputAudioDevice->getSoundCard();
	}

	// If stream doesn't have a playcard associated with it, then use the default values
	if (!captcard) captcard = getCCore()->sound_conf.capt_sndcard;

	if (!captcard) lWarning() << "No card defined for capture!";
	string playfile = L_C_TO_STRING(getCCore()->play_file);
	string recfile = L_C_TO_STRING(getCCore()->rec_file);
	string onHoldMusicFile = L_C_TO_STRING(getCCore()->on_hold_music_file);
	/* Don't use file or soundcard capture when placed in recv-only mode */
	if ((stream.rtp_port == 0) || (stream.getDirection() == SalStreamRecvOnly) ||
	    (stream.multicast_role == SalMulticastReceiver)) {
		captcard = nullptr;
		playfile = "";
	}

	bool pause = targetState == CallSession::State::Paused;
	if (pause) {
		// In paused state, we never use soundcard
		playcard = captcard = nullptr;
		recfile = "";
		// And we will eventually play "playfile" if set by the user
	}
	if (getMediaSession().isPlayingRingbackTone()) {
		captcard = nullptr;
		playfile = ""; /* It is setup later */
		if (linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "send_ringback_without_playback",
		                            0) == 1) {
			playcard = nullptr;
			recfile = "";
		}
	}
	// If playfile are supplied don't use soundcards
	bool useRtpIo = !!linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "rtp_io", false);
	bool useRtpIoEnableLocalOutput =
	    !!linphone_config_get_int(linphone_core_get_config(getCCore()), "sound", "rtp_io_enable_local_output", false);
	bool useFiles = getCCore()->use_files;
	if (useFiles || (useRtpIo && !useRtpIoEnableLocalOutput)) {
		captcard = playcard = nullptr;
	}
	if (audioMixer) {
		// Create the graph without soundcard resources.
		captcard = playcard = nullptr;
	}
	if (!getMediaSession().areSoundResourcesAvailable()) {
		lInfo() << "Sound resources are used by another CallSession, not using soundcard";
		captcard = playcard = nullptr;
		if (targetState == CallSession::State::OutgoingEarlyMedia) {
			// Restart will be required upon transitioning to StreamsRunning state to take into account that sound
			// resources may have been released meanwhile.
			mRestartStreamRequired = true;
			lInfo() << "Soundcard usage will be checked again when moving to StreamsRunning.";
		}
	}

	if (playcard) {
		lInfo() << "Call state " << targetState << ", using "
		        << ((expectedStreamType == MS_SND_CARD_STREAM_RING) ? "ring" : "voice") << " stream";
		ms_snd_card_set_stream_type(playcard, expectedStreamType);
	}

	configureAudioStream();
	bool useEc = captcard && linphone_core_echo_cancellation_enabled(getCCore());
	audio_stream_enable_echo_canceller(mStream, useEc);
	if (playcard && (stream.getMaxRate() > 0)) ms_snd_card_set_preferred_sample_rate(playcard, stream.getMaxRate());
	if (captcard && (stream.getMaxRate() > 0)) ms_snd_card_set_preferred_sample_rate(captcard, stream.getMaxRate());

	if (!audioMixer && !getMediaSessionPrivate().getParams()->getRecordFilePath().empty()) {
		setRecordPath(getMediaSessionPrivate().getParams()->getRecordFilePath());
		getMediaSessionPrivate().getCurrentParams()->setRecordFilePath(
		    getMediaSessionPrivate().getParams()->getRecordFilePath());
	}

	MS2Stream::render(params, targetState);
	RtpAddressInfo dest;
	getRtpDestination(params, &dest);
	/* Now start the stream */
	MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;
	if (useRtpIo) {
		if (useRtpIoEnableLocalOutput) {
			io.input.type = MSResourceRtp;
			io.input.session = createRtpIoSession();
			if (playcard) {
				io.output.type = MSResourceSoundcard;
				io.output.soundcard = playcard;
			} else {
				io.output.type = MSResourceFile;
				io.output.file = recfile.empty() || !useFiles ? nullptr : recfile.c_str();
			}
		} else {
			io.input.type = io.output.type = MSResourceRtp;
			io.input.session = io.output.session = createRtpIoSession();
		}
		if (!io.input.session) ok = false;
	} else {
		if (playcard) {
			io.output.type = MSResourceSoundcard;
			io.output.soundcard = playcard;
		} else {
			io.output.type = MSResourceFile;
			io.output.file = recfile.empty() || !useFiles ? nullptr : recfile.c_str();
		}
		if (captcard) {
			io.input.type = MSResourceSoundcard;
			io.input.soundcard = captcard;
		} else {
			io.input.type = MSResourceFile;

			// We need to use onHoldFile when paused
			onHoldFile = pause ? onHoldMusicFile : "";
			io.input.file =
			    pause || !useFiles
			        ? nullptr
			        : playfile.c_str(); /* We prefer to use the remote_play api, that allows to play multimedia files */
		}
	}

	if (ok) {
		if (mCurrentCaptureCard) ms_snd_card_unref(mCurrentCaptureCard);
		if (mCurrentPlaybackCard) ms_snd_card_unref(mCurrentPlaybackCard);

		mCurrentCaptureCard = ms_media_resource_get_soundcard(&io.input);
		mCurrentPlaybackCard = ms_media_resource_get_soundcard(&io.output);
		if (mCurrentCaptureCard) mCurrentCaptureCard = ms_snd_card_ref(mCurrentCaptureCard);
		if (mCurrentPlaybackCard) mCurrentPlaybackCard = ms_snd_card_ref(mCurrentPlaybackCard);

		const auto &streamCfg = stream.getActualConfiguration();

		if (streamCfg.getMixerToClientExtensionId() > 0) {
			// This has to be called before audio_stream_start so that the AudioStream can configure it's filters
			// properly
			audio_stream_set_mixer_to_client_extension_id(mStream, streamCfg.getMixerToClientExtensionId());
		}

		if (streamCfg.getClientToMixerExtensionId() > 0) {
			// This has to be called before audio_stream_start so that the AudioStream can configure it's filters
			// properly
			audio_stream_set_client_to_mixer_extension_id(mStream, streamCfg.getClientToMixerExtensionId());
		}

		// Specific configuration for conference
		configureConference();

		audio_stream_set_is_speaking_callback(mStream, &MS2AudioStream::sAudioStreamIsSpeakingCb, this);
		audio_stream_set_is_muted_callback(mStream, &MS2AudioStream::sAudioStreamIsMutedCb, this);

		if (conference) {
			audio_stream_set_active_speaker_callback(mStream, &MS2AudioStream::sAudioStreamActiveSpeakerCb, this);

			// Enable Voice Activity Detection
			auto features = audio_stream_get_features(mStream);
			features |= AUDIO_STREAM_FEATURE_VAD;
			audio_stream_set_features(mStream, features);
		}

		audio_stream_set_audio_route_changed_callback(mStream, &MS2AudioStream::audioRouteChangeCb, &getCore());

		int err = audio_stream_start_from_io(mStream, audioProfile, dest.rtpAddr.c_str(), dest.rtpPort,
		                                     dest.rtcpAddr.c_str(), dest.rtcpPort, usedPt, &io);
		VideoStream *vs = getPeerVideoStream();
		if (vs) audio_stream_link_video(mStream, vs);
		if (err == 0) postConfigureAudioStream((mMuted || mMicMuted) && !getMediaSession().isPlayingRingbackTone());
		mInternalStats.number_of_starts++;

#ifdef HAVE_BAUDOT
		if (mStream->baudot_detector) {
			ms_filter_add_notify_callback(mStream->baudot_detector, sBaudotDetectorEventNotified, this, false);
		}
#endif /* HAVE_BAUDOT */
	}

	if ((targetState == CallSession::State::Paused) && !captcard && !playfile.empty()) {
		int pauseTime = 500;
		ms_filter_call_method(mStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
	}
	if (getMediaSession().isPlayingRingbackTone()) setupRingbackPlayer();

	std::shared_ptr<ParticipantDevice> device = nullptr;
	if (conference) {
		device = conference->findParticipantDevice(getMediaSession().getSharedFromThis());
	}

	if (audioMixer && !mMuted) {
		const auto &audioConfParams = ms_audio_conference_get_params(audioMixer->getAudioConference());
		mConferenceEndpoint = ms_audio_endpoint_get_from_stream(mStream, TRUE, audioConfParams->mode);
		audioMixer->connectEndpoint(this, mConferenceEndpoint, (stream.getDirection() == SalStreamRecvOnly));
	}
	getMediaSessionPrivate().getCurrentParams()->enableLowBandwidth(
	    getMediaSessionPrivate().getParams()->lowBandwidthEnabled());

	// Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute
	if (getMediaSessionPrivate().isMediaEncryptionAccepted(LinphoneMediaEncryptionZRTP) && isMain()) {
		getMediaSessionPrivate().performMutualAuthentication();
		LinphoneMediaEncryption requestedMediaEncryption = getMediaSessionPrivate().getNegotiatedMediaEncryption();
		// Start ZRTP: If requested (by local config or peer giving zrtp-hash in SDP, we shall start the ZRTP engine
		if ((requestedMediaEncryption == LinphoneMediaEncryptionZRTP) ||
		    (params.getRemoteStreamDescription().getChosenConfiguration().hasZrtpHash() == 1)) {
			// However, when we are receiver, if peer offers a lime-Ik attribute, we shall delay the start (and ZRTP
			// Hello Packet sending) until the ACK has been received to ensure the caller got our 200 Ok (with
			// lime-Ik in it) before starting its ZRTP engine
			if (!params.localIsOfferer && params.remoteMediaDescription->hasLimeIk()) {
				mStartZrtpLater = true;
			} else {
				startZrtpPrimaryChannel(params);
			}
		}
	}

	if (!onHoldFile.empty() && !getMediaSessionPrivate().getParams()->getPrivate()->getInConference()) {
		lInfo() << "On hold multimedia file specified, will be started shortly after all streams are rendered.";
		getGroup().addPostRenderHook([this, onHoldFile] {
			/* The on-hold file is to be played once both audio and video are ready */
			MSFilter *player = audio_stream_open_remote_play(mStream, onHoldFile.c_str());
			if (player) {
				int pauseTime = 500;
				ms_filter_call_method(player, MS_PLAYER_SET_LOOP, &pauseTime);
				ms_filter_call_method_noarg(player, MS_PLAYER_START);
			}
		});
	}

	setupMediaLossCheck(targetState == CallSession::State::Paused);

	return;
}

void MS2AudioStream::stop() {
	if (mMediaLostCheckTimer) {
		getCore().destroyTimer(mMediaLostCheckTimer);
		mMediaLostCheckTimer = nullptr;
	}
	MS2Stream::stop();
	if (mStream->ec) {
		char *stateStr = nullptr;
		ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_GET_STATE_STRING, &stateStr);
		if (stateStr) {
			lInfo() << "Writing echo canceler state, " << (int)strlen(stateStr) << " bytes";
			linphone_config_write_relative_file(linphone_core_get_config(getCCore()), ecStateStore, stateStr);
		}
	}
	VideoStream *vs = getPeerVideoStream();
	if (vs) audio_stream_unlink_video(mStream, vs);

	if (mConferenceEndpoint) {
		// First disconnect from the mixer before stopping the stream.
		getAudioMixer()->disconnectEndpoint(this, mConferenceEndpoint);
		ms_audio_endpoint_release_from_stream(mConferenceEndpoint);
		mConferenceEndpoint = nullptr;
	}
	audio_stream_stop(mStream);

	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for
	 * later use, keeping the sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = audio_stream_new_with_sessions(getCCore()->factory, &mSessions);
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);

	if (mCurrentCaptureCard) ms_snd_card_unref(mCurrentCaptureCard);
	if (mCurrentPlaybackCard) ms_snd_card_unref(mCurrentPlaybackCard);

	mCurrentCaptureCard = nullptr;
	mCurrentPlaybackCard = nullptr;
}

void MS2AudioStream::startZrtp() {
	if (!mSessions.zrtp_context) {
		initZrtp();
		// Copy newly created zrtp context into mSessions
		MediaStream *ms = getMediaStream();
		media_stream_reclaim_sessions(ms, &mSessions);
	}
	audio_stream_start_zrtp(mStream);
}
// To give a chance for auxilary secret to be used, primary channel (I.E audio) should be started either on 200ok if
// ZRTP is signaled by a zrtp-hash or when ACK is received in case calling side does not have zrtp-hash.
void MS2AudioStream::startZrtpPrimaryChannel(const OfferAnswerContext &params) {
	startZrtp();

	const auto &remote = params.getRemoteStreamDescription();
	if (remote.getChosenConfiguration().hasZrtpHash() == 1) {
		int retval =
		    ms_zrtp_setPeerHelloHash(mSessions.zrtp_context, (uint8_t *)remote.getChosenConfiguration().getZrtpHash(),
		                             strlen((const char *)(remote.getChosenConfiguration().getZrtpHash())));
		if (retval != 0) lError() << "ZRTP hash mismatch 0x" << hex << retval;
	}
}

void MS2AudioStream::forceSpeakerMuted(bool muted) {
	if (muted) audio_stream_set_spk_gain(mStream, 0);
	else audio_stream_set_spk_gain_db(mStream, getCCore()->sound_conf.soft_play_lev);
}

void MS2AudioStream::parameterizeEqualizer(AudioStream *as, LinphoneCore *lc) {
	LinphoneConfig *config = linphone_core_get_config(lc);
	const char *eqActive = linphone_config_get_string(config, "sound", "eq_active", nullptr);
	if (eqActive)
		lWarning() << "'eq_active' linphonerc parameter has no effect anymore. Please use 'mic_eq_active' or "
		              "'spk_eq_active' instead";
	const char *eqGains = linphone_config_get_string(config, "sound", "eq_gains", nullptr);
	if (eqGains)
		lWarning() << "'eq_gains' linphonerc parameter has no effect anymore. Please use 'mic_eq_gains' or "
		              "'spk_eq_gains' instead";
	if (as->mic_equalizer) {
		MSFilter *f = as->mic_equalizer;
		bool enabled = !!linphone_config_get_int(config, "sound", "mic_eq_active", 0);
		ms_filter_call_method(f, MS_EQUALIZER_SET_ACTIVE, &enabled);
		const char *gains = linphone_config_get_string(config, "sound", "mic_eq_gains", nullptr);
		if (enabled && gains) {
			bctbx_list_t *gainsList = ms_parse_equalizer_string(gains);
			for (bctbx_list_t *it = gainsList; it; it = bctbx_list_next(it)) {
				MSEqualizerGain *g = static_cast<MSEqualizerGain *>(bctbx_list_get_data(it));
				lInfo() << "Read microphone equalizer gains: " << g->frequency << "(~" << g->width << ") --> "
				        << g->gain;
				ms_filter_call_method(f, MS_EQUALIZER_SET_GAIN, g);
			}
			if (gainsList) bctbx_list_free_with_data(gainsList, ms_free);
		}
	}
	if (as->spk_equalizer) {
		MSFilter *f = as->spk_equalizer;
		bool enabled = !!linphone_config_get_int(config, "sound", "spk_eq_active", 0);
		ms_filter_call_method(f, MS_EQUALIZER_SET_ACTIVE, &enabled);
		const char *gains = linphone_config_get_string(config, "sound", "spk_eq_gains", nullptr);
		if (enabled && gains) {
			bctbx_list_t *gainsList = ms_parse_equalizer_string(gains);
			for (bctbx_list_t *it = gainsList; it; it = bctbx_list_next(it)) {
				MSEqualizerGain *g = static_cast<MSEqualizerGain *>(bctbx_list_get_data(it));
				lInfo() << "Read speaker equalizer gains: " << g->frequency << "(~" << g->width << ") --> " << g->gain;
				ms_filter_call_method(f, MS_EQUALIZER_SET_GAIN, g);
			}
			if (gainsList) bctbx_list_free_with_data(gainsList, ms_free);
		}
	}
}

void MS2AudioStream::configureFlowControl(AudioStream *as, LinphoneCore *lc) {
	if (as->flowcontrol) {
		LinphoneConfig *config = linphone_core_get_config(lc);
		MSAudioFlowControlConfig cfg;
		memset(&cfg, 0, sizeof(cfg));
		string strategy = linphone_config_get_string(config, "sound", "flow_control_strategy", "soft");
		if (strategy == "soft") cfg.strategy = MSAudioFlowControlSoft;
		else if (strategy == "basic") {
			cfg.strategy = MSAudioFlowControlBasic;
		} else {
			lError() << "Unsupported flow_control_strategy '" << strategy << "'";
			return;
		}
		cfg.silent_threshold = linphone_config_get_float(config, "sound", "flow_control_silence_threshold", 0.02f);
		ms_filter_call_method(as->flowcontrol, MS_AUDIO_FLOW_CONTROL_SET_CONFIG, &cfg);
	}
}

void MS2AudioStream::enableMicOnAudioStream(AudioStream *as, LinphoneCore *lc, bool enabled) {
	enabled = enabled && linphone_core_mic_enabled(lc);
	bctbx_message("AudioStream[%p]: mic is [%s].", as, enabled ? "enabled" : "disabled");
	audio_stream_enable_mic(as, enabled);
}

void MS2AudioStream::postConfigureAudioStream(AudioStream *as, LinphoneCore *lc, bool muted) {
	/* Set soft gains */
	audio_stream_set_mic_gain_db(as, lc->sound_conf.soft_mic_lev);
	audio_stream_set_spk_gain_db(as, lc->sound_conf.soft_play_lev);
	/* Set microphone enablement */
	enableMicOnAudioStream(as, lc, !muted);

	LinphoneConfig *config = linphone_core_get_config(lc);
	float ngThres = linphone_config_get_float(config, "sound", "ng_thres", 0.05f);
	float ngFloorGain = linphone_config_get_float(config, "sound", "ng_floorgain", 0);
	if (as->volsend) {
		int dcRemoval = linphone_config_get_int(config, "sound", "dc_removal", 0);
		ms_filter_call_method(as->volsend, MS_VOLUME_REMOVE_DC, &dcRemoval);
		float speed = linphone_config_get_float(config, "sound", "el_speed", -1);
		float thres = linphone_config_get_float(config, "sound", "el_thres", -1);
		float force = linphone_config_get_float(config, "sound", "el_force", -1);
		int sustain = linphone_config_get_int(config, "sound", "el_sustain", -1);
		float transmitThres = linphone_config_get_float(config, "sound", "el_transmit_thres", -1);
		if (static_cast<int>(speed) == -1) speed = 0.03f;
		if (static_cast<int>(force) == -1) force = 25;
		MSFilter *f = as->volsend;
		ms_filter_call_method(f, MS_VOLUME_SET_EA_SPEED, &speed);
		ms_filter_call_method(f, MS_VOLUME_SET_EA_FORCE, &force);
		if (static_cast<int>(thres) != -1) ms_filter_call_method(f, MS_VOLUME_SET_EA_THRESHOLD, &thres);
		if (static_cast<int>(sustain) != -1) ms_filter_call_method(f, MS_VOLUME_SET_EA_SUSTAIN, &sustain);
		if (static_cast<int>(transmitThres) != -1)
			ms_filter_call_method(f, MS_VOLUME_SET_EA_TRANSMIT_THRESHOLD, &transmitThres);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_THRESHOLD, &ngThres);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_FLOORGAIN, &ngFloorGain);
	}
	if (as->volrecv) {
		/* Parameters for a limited noise-gate effect, using echo limiter threshold */
		float floorGain = (float)(1 / pow(10, lc->sound_conf.soft_mic_lev / 10));
		int spkAgc = linphone_config_get_int(config, "sound", "speaker_agc_enabled", 0);
		MSFilter *f = as->volrecv;
		ms_filter_call_method(f, MS_VOLUME_ENABLE_AGC, &spkAgc);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_THRESHOLD, &ngThres);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_FLOORGAIN, &floorGain);
	}
	parameterizeEqualizer(as, lc);
	configureFlowControl(as, lc);
}

void MS2AudioStream::postConfigureAudioStream(bool muted) {
	postConfigureAudioStream(mStream, getCCore(), muted);
	forceSpeakerMuted(mSpeakerMuted);
	if (linphone_core_dtmf_received_has_listener(getCCore())) {
		audio_stream_play_received_dtmfs(mStream, false);
	}
	if (mRecordActive) startRecording();
}

void MS2AudioStream::setupRingbackPlayer() {
	int pauseTime = 3000;
	audio_stream_play(mStream, getCCore()->sound_conf.ringback_tone);
	ms_filter_call_method(mStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
}

void MS2AudioStream::telephoneEventReceived(int event) {
	static char dtmfTab[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'A', 'B', 'C', 'D'};
	if ((event < 0) || (event > 15)) {
		lWarning() << "Bad dtmf value " << event;
		return;
	}
	getMediaSessionPrivate().dtmfReceived(dtmfTab[event]);
}

void MS2AudioStream::handleEvent(const OrtpEvent *ev) {
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(const_cast<OrtpEvent *>(ev));
	list<string> sasList{};
	switch (evt) {
		case ORTP_EVENT_ZRTP_ENCRYPTION_CHANGED:
			if (isMain()) getGroup().zrtpStarted(this);
			break;
		case ORTP_EVENT_ZRTP_SAS_READY:
			for (int i = 0; i < 3; i++) {
				sasList.emplace_back(evd->info.zrtp_info.incorrect_sas[i]);
			}
			getGroup().authTokensReady(std::move(sasList), evd->info.zrtp_info.sas, !!evd->info.zrtp_info.verified,
			                           !!evd->info.zrtp_info.cache_mismatch);
			break;
		case ORTP_EVENT_TELEPHONE_EVENT:
			telephoneEventReceived(evd->info.telephone_event);
			break;
		default:
			break;
	}
}

void MS2AudioStream::enableMic(bool value) {
	mMicMuted = !value;
	if (mStream) {
		enableMicOnAudioStream(mStream, getCCore(), value);
	}
}

bool MS2AudioStream::micEnabled() const {
	return !mMicMuted;
}

void MS2AudioStream::enableSpeaker(bool value) {
	mSpeakerMuted = !value;
	forceSpeakerMuted(mSpeakerMuted);
}

bool MS2AudioStream::speakerEnabled() const {
	return !mSpeakerMuted;
}

bool MS2AudioStream::supportsTelephoneEvents() {
	return audio_stream_supports_telephone_events(mStream);
}

void MS2AudioStream::sendDtmf(int dtmf) {
	audio_stream_send_dtmf(mStream, (char)dtmf);
}

void MS2AudioStream::setRecordPath(const std::string &path) {
	audio_stream_set_mixed_record_file(mStream, path.c_str());
}

bool MS2AudioStream::startRecording() {
	if (getMediaSessionPrivate().getParams()->getRecordFilePath().empty()) {
		lError() << "MS2AudioStream::startRecording(): no output file specified. Use "
		            "MediaSessionParams::setRecordFilePath()";
		return false;
	} else {
		lInfo() << "MS2AudioStream::startRecording(): output file "
		        << getMediaSessionPrivate().getParams()->getRecordFilePath();
	}
	if (getMediaSessionPrivate().getParams()->getPrivate()->getInConference()) {
		lWarning() << "MS2AudioStream::startRecording(): not supported in conference.";
		return false;
	}
	if (media_stream_get_state(&mStream->ms) == MSStreamStarted) {
		if (audio_stream_mixed_record_start(mStream) != -1) {
			mRecordActive = true;
			return true;
		}
	}

	return false;
}

void MS2AudioStream::stopRecording() {
	if (mRecordActive) audio_stream_mixed_record_stop(mStream);
	mRecordActive = false;
}

float MS2AudioStream::getPlayVolume() {
	if (mStream->volrecv) {
		float vol = 0;
		ms_filter_call_method(mStream->volrecv, MS_VOLUME_GET, &vol);
		return vol;
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MS2AudioStream::getRecordVolume() {
	if (mStream->volsend && !mMicMuted) {
		float vol = 0;
		ms_filter_call_method(mStream->volsend, MS_VOLUME_GET, &vol);
		return vol;
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MS2AudioStream::getMicGain() {
	return audio_stream_get_sound_card_input_gain(mStream);
}

void MS2AudioStream::setMicGain(float gain) {
	audio_stream_set_sound_card_input_gain(mStream, gain);
}

float MS2AudioStream::getSpeakerGain() {
	return audio_stream_get_sound_card_output_gain(mStream);
}

void MS2AudioStream::setSpeakerGain(float gain) {
	audio_stream_set_sound_card_output_gain(mStream, gain);
}

VideoStream *MS2AudioStream::getPeerVideoStream() {
#ifdef VIDEO_ENABLED
	MS2VideoStream *vs = getGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	return vs ? (VideoStream *)vs->getMediaStream() : nullptr;
#else
	return nullptr;
#endif
}

void MS2AudioStream::enableEchoCancellation(bool value) {
	if (mStream->ec) {
		bool bypassMode = !value;
		ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_SET_BYPASS_MODE, &bypassMode);
	}
}

bool MS2AudioStream::echoCancellationEnabled() const {
	if (!mStream->ec) return !!linphone_core_echo_cancellation_enabled(getCCore());

	bool_t val;
	ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_GET_BYPASS_MODE, &val);
	return !val;
}

void MS2AudioStream::setSoundCardType(MSSndCard *soundcard) {
	if (soundcard) {
		auto expectedStreamType = MS_SND_CARD_STREAM_VOICE;
		switch (getMediaSession().getState()) {
			case CallSession::State::IncomingReceived:
			case CallSession::State::IncomingEarlyMedia:
				expectedStreamType = MS_SND_CARD_STREAM_RING;
				break;
			default:
				expectedStreamType = MS_SND_CARD_STREAM_VOICE;
				break;
		}
		lInfo() << "[MS2AudioStream] setting type of soundcard " << soundcard << " to "
		        << ((expectedStreamType == MS_SND_CARD_STREAM_RING) ? "ring" : "voice");
		ms_snd_card_set_stream_type(soundcard, expectedStreamType);
	}
}

int MS2AudioStream::restartStream(RestartReason reason) {
	// Schedule a restart in order to avoid multiple reinitialisation when changing at the same time both devices.
	// Also, it allows to avoid to restart the stream if it has been already restart.
	const char *const reasonToText = (reason == RestartReason::OutputChanged ? "output" : "input");
	if (getState() == Running) {
		if (!mRestartStreamRequired) {
			lInfo() << *this << "restart required for updating " << reasonToText;
			mRestartStreamRequired = true;
			getCore().doLater([&]() {
				if (mRestartStreamRequired && getState() == Running) // Still need to be restarted. If false, then a
				                                                     // restart has been already done on an update.
					render(getGroup().getCurrentOfferAnswerContext().scopeStreamToIndex(getIndex()),
					       getGroup().getCurrentSessionState());
			});
			return 0;
		} else {
			lInfo() << *this << " restart already required (now for updating " << reasonToText << ")";
		}
	}
	return -1;
}

void MS2AudioStream::setInputDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (!mStream) return;
	auto soundcard = audioDevice ? audioDevice->getSoundCard() : nullptr;
	setSoundCardType(soundcard);
	if (audio_stream_set_input_ms_snd_card(mStream, soundcard) < 0 && mCurrentCaptureCard) {
		// New device couldn't update the stream, request to stop it
		// Due to missing implementation of MS_AUDIO_CAPTURE_SET_INTERNAL_ID and MS_AUDIO_PLAYBACK_SET_INTERNAL_ID
		restartStream(RestartReason::InputChanged);
	}
}

void MS2AudioStream::setOutputDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (!mStream) return;
	auto soundcard = audioDevice ? audioDevice->getSoundCard() : nullptr;
	setSoundCardType(soundcard);
	if (audio_stream_set_output_ms_snd_card(mStream, soundcard) < 0 && mCurrentPlaybackCard) {
		// New device couldn't update the stream, request to stop it
		// Due to missing implementation of MS_AUDIO_CAPTURE_SET_INTERNAL_ID and MS_AUDIO_PLAYBACK_SET_INTERNAL_ID
		restartStream(RestartReason::OutputChanged);
	}
}

shared_ptr<AudioDevice> MS2AudioStream::getInputDevice() const {
	if (!mStream) return nullptr;
	MSSndCard *card = audio_stream_get_input_ms_snd_card(mStream);
	return getCore().findAudioDeviceMatchingMsSoundCard(card);
}

shared_ptr<AudioDevice> MS2AudioStream::getOutputDevice() const {
	if (!mStream) return nullptr;
	MSSndCard *card = audio_stream_get_output_ms_snd_card(mStream);
	return getCore().findAudioDeviceMatchingMsSoundCard(card);
}

void MS2AudioStream::finish() {
	if (mStream) {
		stopRecording(); // ensure to stop recording if recording
		auto oldStream = mStream;
		mStream = nullptr;
		audio_stream_stop(oldStream);
	}
	MS2Stream::finish();
}

MS2AudioMixer *MS2AudioStream::getAudioMixer() {
	StreamMixer *mixer = getMixer();
	if (mixer) {
		MS2AudioMixer *audioMixer = dynamic_cast<MS2AudioMixer *>(mixer);
		if (!audioMixer) {
			lError() << *this << " does not have a mixer it is able to interface with.";
		}
		return audioMixer;
	}
	return nullptr;
}

static MSBaudotMode linphone_call_baudot_standard_to_ms_baudot_mode(LinphoneBaudotStandard standard) {
	MSBaudotMode mode = MSBaudotModeTty45;
	switch (standard) {
		case LinphoneBaudotStandardUs:
			mode = MSBaudotModeTty45;
			break;
		case LinphoneBaudotStandardEurope:
			mode = MSBaudotModeTty50;
			break;
	}
	return mode;
}

void MS2AudioStream::enableBaudotDetection(bool enabled) {
	if (mStream) audio_stream_enable_baudot_detection(mStream, enabled ? TRUE : FALSE);
}

void MS2AudioStream::setBaudotMode(LinphoneBaudotMode mode) {
	mBaudotMode = mode;
	applyBaudotModeAndStandard();
}

void MS2AudioStream::setBaudotSendingStandard(LinphoneBaudotStandard standard) {
	mBaudotSendingStandard = standard;
	applyBaudotModeAndStandard();
}

void MS2AudioStream::setBaudotPauseTimeout(uint8_t seconds) {
	if (mStream) audio_stream_set_baudot_pause_timeout(mStream, seconds);
}

void MS2AudioStream::applyBaudotModeAndStandard() const {
	if (mStream) {
		switch (mBaudotMode) {
			case LinphoneBaudotModeVoice:
				audio_stream_enable_baudot_decoding(mStream, FALSE);
				audio_stream_set_baudot_sending_mode(mStream, MSBaudotModeVoice);
				break;
			case LinphoneBaudotModeTty:
				audio_stream_enable_baudot_decoding(mStream, TRUE);
				audio_stream_set_baudot_sending_mode(
				    mStream, linphone_call_baudot_standard_to_ms_baudot_mode(mBaudotSendingStandard));
				break;
			case LinphoneBaudotModeHearingCarryOver:
				audio_stream_enable_baudot_decoding(mStream, FALSE);
				audio_stream_set_baudot_sending_mode(
				    mStream, linphone_call_baudot_standard_to_ms_baudot_mode(mBaudotSendingStandard));
				break;
			case LinphoneBaudotModeVoiceCarryOver:
				audio_stream_enable_baudot_decoding(mStream, TRUE);
				audio_stream_set_baudot_sending_mode(mStream, MSBaudotModeVoice);
				break;
		}
	}
}

void MS2AudioStream::sendBaudotCharacter(char character) {
	audio_stream_send_baudot_character(mStream, character);
}

#ifdef HAVE_BAUDOT
void MS2AudioStream::baudotDetectorEventNotified(BCTBX_UNUSED(MSFilter *f), unsigned int id, void *arg) {
	if (id == MS_BAUDOT_DETECTOR_STATE_EVENT) {
		MSBaudotDetectorState state = *reinterpret_cast<MSBaudotDetectorState *>(arg);
		ms_message("Baudot detector state change notified: %s",
		           state == MSBaudotDetectorStateTty45 ? "TTY45" : "TTY50");
		getMediaSession().notifyBaudotDetected((state == MSBaudotDetectorStateTty50) ? MSBaudotStandardEurope
		                                                                             : MSBaudotStandardUs);
	} else if (id == MS_BAUDOT_DETECTOR_CHARACTER_EVENT) {
		char receivedCharacter = *reinterpret_cast<char *>(arg);
		getMediaSession().notifyBaudotCharacterReceived(receivedCharacter);
	}
}

void MS2AudioStream::sBaudotDetectorEventNotified(void *userData, MSFilter *f, unsigned int id, void *arg) {
	MS2AudioStream *zis = static_cast<MS2AudioStream *>(userData);
	zis->baudotDetectorEventNotified(f, id, arg);
}
#endif /* HAVE_BAUDOT */

std::string MS2AudioStream::getLabel() const {
	return std::string();
}

MS2AudioStream::~MS2AudioStream() {
	if (mStream) finish();
}

LINPHONE_END_NAMESPACE
