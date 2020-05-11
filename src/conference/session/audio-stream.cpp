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
#include "bctoolbox/defs.h"

#include "ms2-streams.h"
#include "mixers.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "call/call-p.h"
#include "conference/participant.h"
#include "conference/params/media-session-params-p.h"
#include "nat/ice-service.h"

#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msvolume.h"

#include "linphone/core.h"

#include <cmath>

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2AudioStream implementation.
 */

MS2AudioStream::MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params){
	string bindIp = getBindIp();
	mStream = audio_stream_new2(getCCore()->factory, bindIp.empty() ? nullptr : bindIp.c_str(), mPortConfig.rtpPort, mPortConfig.rtcpPort);

	/* Initialize zrtp even if we didn't explicitely set it, just in case peer offers it */
	if (linphone_core_media_encryption_supported(getCCore(), LinphoneMediaEncryptionZRTP)) {
		LinphoneCallLog *log = getMediaSession().getLog();
		const LinphoneAddress *peerAddr = linphone_call_log_get_remote_address(log);
		const LinphoneAddress *selfAddr = linphone_call_log_get_local_address(log);
		char *peerUri = ms_strdup_printf("%s:%s@%s"	, linphone_address_get_scheme(peerAddr)
													, linphone_address_get_username(peerAddr)
													, linphone_address_get_domain(peerAddr));
		char *selfUri = ms_strdup_printf("%s:%s@%s"	, linphone_address_get_scheme(selfAddr)
													, linphone_address_get_username(selfAddr)
													, linphone_address_get_domain(selfAddr));

		MSZrtpParams zrtpParams;
		zrtpCacheAccess zrtpCacheInfo = linphone_core_get_zrtp_cache_access(getCCore());

		memset(&zrtpParams, 0, sizeof(MSZrtpParams));
		/* media encryption of current params will be set later when zrtp is activated */
		zrtpParams.zidCacheDB = zrtpCacheInfo.db;
		zrtpParams.zidCacheDBMutex = zrtpCacheInfo.dbMutex;
		zrtpParams.peerUri = peerUri;
		zrtpParams.selfUri = selfUri;
		/* Get key lifespan from config file, default is 0:forever valid */
		zrtpParams.limeKeyTimeSpan = bctbx_time_string_to_sec(lp_config_get_string(linphone_core_get_config(getCCore()), "sip", "lime_key_validity", "0"));
		setZrtpCryptoTypesParameters(&zrtpParams, params.localIsOfferer);
		audio_stream_enable_zrtp(mStream, &zrtpParams);
		if (peerUri)
			ms_free(peerUri);
		if (selfUri)
			ms_free(selfUri);
	}
	initializeSessions((MediaStream*)mStream);
}

void MS2AudioStream::setZrtpCryptoTypesParameters(MSZrtpParams *params, bool localIsOfferer) {
	const MSCryptoSuite *srtpSuites = linphone_core_get_srtp_crypto_suites(getCCore());
	if (srtpSuites) {
		for(int i = 0; (srtpSuites[i] != MS_CRYPTO_SUITE_INVALID) && (i < SAL_CRYPTO_ALGO_MAX) && (i < MS_MAX_ZRTP_CRYPTO_TYPES); i++) {
			switch (srtpSuites[i]) {
				case MS_AES_128_SHA1_32:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS32;
					break;
				case MS_AES_128_NO_AUTH:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					break;
				case MS_NO_CIPHER_SHA1_80:
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_AES_128_SHA1_80:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_AES_CM_256_SHA1_80:
					lWarning() << "Deprecated crypto suite MS_AES_CM_256_SHA1_80, use MS_AES_256_SHA1_80 instead";
					BCTBX_NO_BREAK;
				case MS_AES_256_SHA1_80:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_AES_256_SHA1_32:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS32;
					break;
				case MS_CRYPTO_SUITE_INVALID:
					break;
			}
		}
	}

	/* linphone_core_get_srtp_crypto_suites is used to determine sensible defaults; here each can be overridden */
	MsZrtpCryptoTypesCount ciphersCount = linphone_core_get_zrtp_cipher_suites(getCCore(), params->ciphers); /* if not present in config file, params->ciphers is not modified */
	if (ciphersCount != 0) /* Use zrtp_cipher_suites config only when present, keep config from srtp_crypto_suite otherwise */
		params->ciphersCount = ciphersCount;
	params->hashesCount = linphone_core_get_zrtp_hash_suites(getCCore(), params->hashes);
	MsZrtpCryptoTypesCount authTagsCount = linphone_core_get_zrtp_auth_suites(getCCore(), params->authTags); /* If not present in config file, params->authTags is not modified */
	if (authTagsCount != 0)
		params->authTagsCount = authTagsCount; /* Use zrtp_auth_suites config only when present, keep config from srtp_crypto_suite otherwise */
	params->sasTypesCount = linphone_core_get_zrtp_sas_suites(getCCore(), params->sasTypes);
	params->keyAgreementsCount = linphone_core_get_zrtp_key_agreement_suites(getCCore(), params->keyAgreements);
	
	/* ZRTP Autostart: avoid starting a ZRTP session before we got peer's lime Ik */
	/* We MUST start if ZRTP is not active otherwise we break RFC compatibility */
	/* When we are not offerer, we received the lime-Ik in SDP so we can start upon ZRTP Hello reception */
	params->autoStart =  (getMediaSessionPrivate().getParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) || (!localIsOfferer);
}

void MS2AudioStream::configureAudioStream(){
	// try to get playcard from the stream if it was already set
	AudioDevice * audioDevice = getMediaSessionPrivate().getCurrentAudioDevice();
	MSSndCard * playcard = NULL;

	if (audioDevice) {
		playcard = audioDevice->getSoundCard();
	}

	// If stream doesn't have a playcard associated with it, then use the default values
	if (!playcard)
		playcard = getCCore()->sound_conf.lsd_card ? getCCore()->sound_conf.lsd_card : getCCore()->sound_conf.play_sndcard;

	if (playcard) {
		// Set the stream type immediately, as on iOS AudioUnit is instanciated very early because it is 
		// otherwise too slow to start.
		ms_snd_card_set_stream_type(playcard, MS_SND_CARD_STREAM_VOICE);
	}
	
	if (linphone_core_echo_limiter_enabled(getCCore())) {
		string type = lp_config_get_string(linphone_core_get_config(getCCore()), "sound", "el_type", "mic");
		if (type == "mic")
			audio_stream_enable_echo_limiter(mStream, ELControlMic);
		else if (type == "full")
			audio_stream_enable_echo_limiter(mStream, ELControlFull);
	}

	// Equalizer location in the graph: 'mic' = in input graph, otherwise in output graph.
	// Any other value than mic will default to output graph for compatibility.
	string location = lp_config_get_string(linphone_core_get_config(getCCore()), "sound", "eq_location", "hp");
	mStream->eq_loc = (location == "mic") ? MSEqualizerMic : MSEqualizerHP;
	lInfo() << "Equalizer location: " << location;

	audio_stream_enable_gain_control(mStream, true);
	if (linphone_core_echo_cancellation_enabled(getCCore())) {
		int len = lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "ec_tail_len", 0);
		int delay = lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "ec_delay", 0);
		int framesize = lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "ec_framesize", 0);
		audio_stream_set_echo_canceller_params(mStream, len, delay, framesize);
		if (mStream->ec) {
			char *statestr=static_cast<char *>(ms_malloc0(ecStateMaxLen));
			if (lp_config_relative_file_exists(linphone_core_get_config(getCCore()), ecStateStore)
				&& (lp_config_read_relative_file(linphone_core_get_config(getCCore()), ecStateStore, statestr, ecStateMaxLen) == 0)) {
				ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_SET_STATE_STRING, statestr);
			}
			ms_free(statestr);
		}
	}
	audio_stream_enable_automatic_gain_control(mStream, linphone_core_agc_enabled(getCCore()));
	bool_t enabled = !!lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "noisegate", 0);
	audio_stream_enable_noise_gate(mStream, enabled);
	audio_stream_set_features(mStream, linphone_core_get_audio_features(getCCore()));
}

bool MS2AudioStream::prepare(){
	MSSndCard *playcard = getCCore()->sound_conf.lsd_card ? getCCore()->sound_conf.lsd_card : getCCore()->sound_conf.play_sndcard;
	if (playcard) {
		// Set the stream type immediately, as on iOS AudioUnit is instanciated very early because it is 
		// otherwise too slow to start.
		ms_snd_card_set_stream_type(playcard, MS_SND_CARD_STREAM_VOICE);
	}
	
	if (getIceService().isActive()){
		audio_stream_prepare_sound(mStream, nullptr, nullptr);
	}
	MS2Stream::prepare();
	return false;
}

void MS2AudioStream::sessionConfirmed(const OfferAnswerContext &ctx){
	if (mStartZrtpLater){
		lInfo() << "Starting zrtp late";
		startZrtpPrimaryChannel(ctx);
		mStartZrtpLater = false;
	}
}

void MS2AudioStream::finishPrepare(){
	MS2Stream::finishPrepare();
	audio_stream_unprepare_sound(mStream);
}

MediaStream *MS2AudioStream::getMediaStream()const{
	return &mStream->ms;
}

void MS2AudioStream::setupMediaLossCheck(){
	int disconnectTimeout = linphone_core_get_nortp_timeout(getCCore());
	mMediaLostCheckTimer = getCore().createTimer( [this, disconnectTimeout]() -> bool{
			if (!audio_stream_alive(mStream, disconnectTimeout)){
				CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
				listener->onLossOfMediaDetected(getMediaSession().getSharedFromThis());
			}
			return true;
		}, 1000, "Audio stream alive check");
}

void MS2AudioStream::render(const OfferAnswerContext &params, CallSession::State targetState){
	const SalStreamDescription *stream = params.resultStreamDescription;
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	
	bool basicChangesHandled = handleBasicChanges(params, targetState);
	
	if (basicChangesHandled) {
		if (getState() == Running) {
			bool muted = mMuted;
			MS2Stream::render(params, targetState); // MS2Stream::render() may decide to unmute.
			if (muted && !mMuted) {
				lInfo() << "Early media finished, unmuting audio input...";
				enableMic(micEnabled());
			}
		}
		return;
	}
	MS2AudioMixer *audioMixer = getAudioMixer();
	int usedPt = -1;
	string onHoldFile = "";
	RtpProfile *audioProfile = makeProfile(params.resultMediaDescription, stream, &usedPt);
	if (usedPt == -1){
		lError() << "No payload types configured for this stream !";
		stop();
		return;
	}

	bool ok = true;
	if (isMain()){
		getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedAudioCodec(rtp_profile_get_payload(audioProfile, usedPt));
	}

	AudioDevice *audioDevice = getMediaSessionPrivate().getCurrentAudioDevice();
	MSSndCard *playcard = nullptr;

	// try to get currently used playcard if it was already set
	if (audioDevice) {
		playcard = audioDevice->getSoundCard();
	}

	// If stream doesn't have a playcard associated with it, then use the default values
	if (!playcard)
		playcard = getCCore()->sound_conf.lsd_card ? getCCore()->sound_conf.lsd_card : getCCore()->sound_conf.play_sndcard;

	if (!playcard)
		lWarning() << "No card defined for playback!";
	MSSndCard *captcard = getCCore()->sound_conf.capt_sndcard;
	if (!captcard)
		lWarning() << "No card defined for capture!";
	string playfile = L_C_TO_STRING(getCCore()->play_file);
	string recfile = L_C_TO_STRING(getCCore()->rec_file);
	/* Don't use file or soundcard capture when placed in recv-only mode */
	if ((stream->rtp_port == 0) || (stream->dir == SalStreamRecvOnly) || (stream->multicast_role == SalMulticastReceiver)) {
		captcard = nullptr;
		playfile = "";
	}

	if (targetState == CallSession::State::Paused) {
		// In paused state, we never use soundcard
		playcard = captcard = nullptr;
		recfile = "";
		// And we will eventually play "playfile" if set by the user
	}
	if (listener && listener->isPlayingRingbackTone(getMediaSession().getSharedFromThis())) {
		captcard = nullptr;
		playfile = ""; /* It is setup later */
		if (lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "send_ringback_without_playback", 0) == 1) {
			playcard = nullptr;
			recfile = "";
		}
	}
	// If playfile are supplied don't use soundcards
	bool useRtpIo = !!lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "rtp_io", false);
	bool useRtpIoEnableLocalOutput = !!lp_config_get_int(linphone_core_get_config(getCCore()), "sound", "rtp_io_enable_local_output", false);
	if (getCCore()->use_files || (useRtpIo && !useRtpIoEnableLocalOutput)) {
		captcard = playcard = nullptr;
	}
	if (audioMixer) {
		// Create the graph without soundcard resources.
		captcard = playcard = nullptr;
	}
	if (listener && !listener->areSoundResourcesAvailable(getMediaSession().getSharedFromThis())) {
		lInfo() << "Sound resources are used by another CallSession, not using soundcard";
		captcard = playcard = nullptr;
	}


	if (playcard) {
		ms_snd_card_set_stream_type(playcard, MS_SND_CARD_STREAM_VOICE);
	}
	configureAudioStream();
	bool useEc = captcard && linphone_core_echo_cancellation_enabled(getCCore());
	audio_stream_enable_echo_canceller(mStream, useEc);
	if (playcard && (stream->max_rate > 0))
		ms_snd_card_set_preferred_sample_rate(playcard, stream->max_rate);
	if (captcard && (stream->max_rate > 0))
		ms_snd_card_set_preferred_sample_rate(captcard, stream->max_rate);
	
	if (!audioMixer && !getMediaSessionPrivate().getParams()->getRecordFilePath().empty()) {
		audio_stream_mixed_record_open(mStream, getMediaSessionPrivate().getParams()->getRecordFilePath().c_str());
		getMediaSessionPrivate().getCurrentParams()->setRecordFilePath(getMediaSessionPrivate().getParams()->getRecordFilePath());
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
				io.output.file = recfile.empty() ? nullptr : recfile.c_str();
			}
		} else {
			io.input.type = io.output.type = MSResourceRtp;
			io.input.session = io.output.session = createRtpIoSession();
		}
		if (!io.input.session)
			ok = false;
	} else {
		if (playcard) {
			io.output.type = MSResourceSoundcard;
			io.output.soundcard = playcard;
		} else {
			io.output.type = MSResourceFile;
			io.output.file = recfile.empty() ? nullptr : recfile.c_str();
		}
		if (captcard) {
			io.input.type = MSResourceSoundcard;
			io.input.soundcard = captcard;
		} else {
			io.input.type = MSResourceFile;
			onHoldFile = playfile;
			io.input.file = nullptr; /* We prefer to use the remote_play api, that allows to play multimedia files */
		}
	}
	if (ok) {
		VideoStream *vs = getPeerVideoStream();
		if (vs) audio_stream_link_video(mStream, vs);

		if (mCurrentCaptureCard) ms_snd_card_unref(mCurrentCaptureCard);
		if (mCurrentPlaybackCard) ms_snd_card_unref(mCurrentPlaybackCard);
		mCurrentCaptureCard = ms_media_resource_get_soundcard(&io.input);
		mCurrentPlaybackCard = ms_media_resource_get_soundcard(&io.output);
		if (mCurrentCaptureCard) mCurrentCaptureCard = ms_snd_card_ref(mCurrentCaptureCard);
		if (mCurrentPlaybackCard) mCurrentPlaybackCard = ms_snd_card_ref(mCurrentPlaybackCard);

		int err = audio_stream_start_from_io(mStream, audioProfile, dest.rtpAddr.c_str(), dest.rtpPort,
			dest.rtcpAddr.c_str(), dest.rtcpPort, usedPt, &io);
		if (err == 0)
			postConfigureAudioStream((mMuted || mMicMuted) && (listener && !listener->isPlayingRingbackTone(getMediaSession().getSharedFromThis())));
		mStartCount++;
	}
	
	if ((targetState == CallSession::State::Paused) && !captcard && !playfile.empty()) {
		int pauseTime = 500;
		ms_filter_call_method(mStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
	}
	if (listener && listener->isPlayingRingbackTone(getMediaSession().getSharedFromThis()))
		setupRingbackPlayer();
	
	if (audioMixer){
		mConferenceEndpoint = ms_audio_endpoint_get_from_stream(mStream, TRUE);
		audioMixer->connectEndpoint(this, mConferenceEndpoint, (stream->dir == SalStreamRecvOnly));
	}
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setInConference(audioMixer != nullptr);
	getMediaSessionPrivate().getCurrentParams()->enableLowBandwidth(getMediaSessionPrivate().getParams()->lowBandwidthEnabled());
	
	// Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute
	if (linphone_core_media_encryption_supported(getCCore(), LinphoneMediaEncryptionZRTP) && isMain()) {
		getMediaSessionPrivate().performMutualAuthentication();
		LinphoneMediaEncryption requestedMediaEncryption = getMediaSessionPrivate().getParams()->getMediaEncryption();
		// Start ZRTP: If requested (by local config or peer giving zrtp-hash in SDP, we shall start the ZRTP engine
		if ((requestedMediaEncryption == LinphoneMediaEncryptionZRTP) || (params.remoteStreamDescription->haveZrtpHash == 1)) {
			// However, when we are receiver, if peer offers a lime-Ik attribute, we shall delay the start (and ZRTP Hello Packet sending)
			// until the ACK has been received to ensure the caller got our 200 Ok (with lime-Ik in it) before starting its ZRTP engine
			if (!params.localIsOfferer && sal_stream_description_has_limeIk(params.remoteStreamDescription)) {
				mStartZrtpLater = true;
			} else {
				startZrtpPrimaryChannel(params);
			}
		}
	}
	
	getGroup().addPostRenderHook([this, onHoldFile] {
		/* The on-hold file is to be played once both audio and video are ready */
		if (!onHoldFile.empty() && !getMediaSessionPrivate().getParams()->getPrivate()->getInConference()) {
			MSFilter *player = audio_stream_open_remote_play(mStream, onHoldFile.c_str());
			if (player) {
				int pauseTime = 500;
				ms_filter_call_method(player, MS_PLAYER_SET_LOOP, &pauseTime);
				ms_filter_call_method_noarg(player, MS_PLAYER_START);
			}
		}
	});
	
	if (targetState == CallSession::State::StreamsRunning){
		setupMediaLossCheck();
	}
	
	return;
}

void MS2AudioStream::stop(){
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
			lp_config_write_relative_file(linphone_core_get_config(getCCore()), ecStateStore, stateStr);
		}
	}
	VideoStream *vs = getPeerVideoStream();
	if (vs) audio_stream_unlink_video(mStream, vs);
	
	if (mConferenceEndpoint){
		// First disconnect from the mixer before stopping the stream.
		getAudioMixer()->disconnectEndpoint(this,mConferenceEndpoint);
		ms_audio_endpoint_release_from_stream(mConferenceEndpoint);
		mConferenceEndpoint = nullptr;
	}
	audio_stream_stop(mStream);

	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for later use, keeping the 
	 * sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = audio_stream_new_with_sessions(getCCore()->factory, &mSessions);
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);
	
	if (mCurrentCaptureCard) ms_snd_card_unref(mCurrentCaptureCard);
	if (mCurrentPlaybackCard) ms_snd_card_unref(mCurrentPlaybackCard);
	
	mCurrentCaptureCard = nullptr;
	mCurrentPlaybackCard = nullptr;
}

//To give a chance for auxilary secret to be used, primary channel (I.E audio) should be started either on 200ok if ZRTP is signaled by a zrtp-hash or when ACK is received in case calling side does not have zrtp-hash.
void MS2AudioStream::startZrtpPrimaryChannel(const OfferAnswerContext &params) {
	const SalStreamDescription *remote = params.remoteStreamDescription;
	audio_stream_start_zrtp(mStream);
	if (remote->haveZrtpHash == 1) {
		int retval = ms_zrtp_setPeerHelloHash(mSessions.zrtp_context, (uint8_t *)remote->zrtphash, strlen((const char *)(remote->zrtphash)));
		if (retval != 0)
			lError() << "ZRTP hash mismatch 0x" << hex << retval;
	}
}

void MS2AudioStream::forceSpeakerMuted (bool muted) {
	if (muted)
		audio_stream_set_spk_gain(mStream, 0);
	else
		audio_stream_set_spk_gain_db(mStream, getCCore()->sound_conf.soft_play_lev);
}

void MS2AudioStream::setRoute(LinphoneAudioRoute route){
	audio_stream_set_audio_route(mStream, (MSAudioRoute)route);
}

void MS2AudioStream::parameterizeEqualizer(AudioStream *as, LinphoneCore *lc) {
	LinphoneConfig *config = linphone_core_get_config(lc);
	const char *eqActive = lp_config_get_string(config, "sound", "eq_active", nullptr);
	if (eqActive)
		lWarning() << "'eq_active' linphonerc parameter has no effect anymore. Please use 'mic_eq_active' or 'spk_eq_active' instead";
	const char *eqGains = lp_config_get_string(config, "sound", "eq_gains", nullptr);
	if(eqGains)
		lWarning() << "'eq_gains' linphonerc parameter has no effect anymore. Please use 'mic_eq_gains' or 'spk_eq_gains' instead";
	if (as->mic_equalizer) {
		MSFilter *f = as->mic_equalizer;
		bool enabled = !!lp_config_get_int(config, "sound", "mic_eq_active", 0);
		ms_filter_call_method(f, MS_EQUALIZER_SET_ACTIVE, &enabled);
		const char *gains = lp_config_get_string(config, "sound", "mic_eq_gains", nullptr);
		if (enabled && gains) {
			bctbx_list_t *gainsList = ms_parse_equalizer_string(gains);
			for (bctbx_list_t *it = gainsList; it; it = bctbx_list_next(it)) {
				MSEqualizerGain *g = reinterpret_cast<MSEqualizerGain *>(bctbx_list_get_data(it));
				lInfo() << "Read microphone equalizer gains: " << g->frequency << "(~" << g->width << ") --> " << g->gain;
				ms_filter_call_method(f, MS_EQUALIZER_SET_GAIN, g);
			}
			if (gainsList)
				bctbx_list_free_with_data(gainsList, ms_free);
		}
	}
	if (as->spk_equalizer) {
		MSFilter *f = as->spk_equalizer;
		bool enabled = !!lp_config_get_int(config, "sound", "spk_eq_active", 0);
		ms_filter_call_method(f, MS_EQUALIZER_SET_ACTIVE, &enabled);
		const char *gains = lp_config_get_string(config, "sound", "spk_eq_gains", nullptr);
		if (enabled && gains) {
			bctbx_list_t *gainsList = ms_parse_equalizer_string(gains);
			for (bctbx_list_t *it = gainsList; it; it = bctbx_list_next(it)) {
				MSEqualizerGain *g = reinterpret_cast<MSEqualizerGain *>(bctbx_list_get_data(it));
				lInfo() << "Read speaker equalizer gains: " << g->frequency << "(~" << g->width << ") --> " << g->gain;
				ms_filter_call_method(f, MS_EQUALIZER_SET_GAIN, g);
			}
			if (gainsList)
				bctbx_list_free_with_data(gainsList, ms_free);
		}
	}
}

void MS2AudioStream::postConfigureAudioStream(AudioStream *as, LinphoneCore *lc, bool muted){
	float micGain = lc->sound_conf.soft_mic_lev;
	if (muted)
		audio_stream_set_mic_gain(as, 0);
	else
		audio_stream_set_mic_gain_db(as, micGain);
	float recvGain = lc->sound_conf.soft_play_lev;
	if (static_cast<int>(recvGain)){
		if (as->volrecv)
			ms_filter_call_method(as->volrecv, MS_VOLUME_SET_DB_GAIN, &recvGain);
		else
			lWarning() << "Could not apply playback gain: gain control wasn't activated";
	}
	LinphoneConfig *config = linphone_core_get_config(lc);
	float ngThres = lp_config_get_float(config, "sound", "ng_thres", 0.05f);
	float ngFloorGain = lp_config_get_float(config, "sound", "ng_floorgain", 0);
	if (as->volsend) {
		int dcRemoval = lp_config_get_int(config, "sound", "dc_removal", 0);
		ms_filter_call_method(as->volsend, MS_VOLUME_REMOVE_DC, &dcRemoval);
		float speed = lp_config_get_float(config, "sound", "el_speed", -1);
		float thres = lp_config_get_float(config, "sound", "el_thres", -1);
		float force = lp_config_get_float(config, "sound", "el_force", -1);
		int sustain = lp_config_get_int(config, "sound", "el_sustain", -1);
		float transmitThres = lp_config_get_float(config, "sound", "el_transmit_thres", -1);
		if (static_cast<int>(speed) == -1)
			speed = 0.03f;
		if (static_cast<int>(force) == -1)
			force = 25;
		MSFilter *f = as->volsend;
		ms_filter_call_method(f, MS_VOLUME_SET_EA_SPEED, &speed);
		ms_filter_call_method(f, MS_VOLUME_SET_EA_FORCE, &force);
		if (static_cast<int>(thres) != -1)
			ms_filter_call_method(f, MS_VOLUME_SET_EA_THRESHOLD, &thres);
		if (static_cast<int>(sustain) != -1)
			ms_filter_call_method(f, MS_VOLUME_SET_EA_SUSTAIN, &sustain);
		if (static_cast<int>(transmitThres) != -1)
			ms_filter_call_method(f, MS_VOLUME_SET_EA_TRANSMIT_THRESHOLD, &transmitThres);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_THRESHOLD, &ngThres);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_FLOORGAIN, &ngFloorGain);
	}
	if (as->volrecv) {
		/* Parameters for a limited noise-gate effect, using echo limiter threshold */
		float floorGain = (float)(1 / pow(10, micGain / 10));
		int spkAgc = lp_config_get_int(config, "sound", "speaker_agc_enabled", 0);
		MSFilter *f = as->volrecv;
		ms_filter_call_method(f, MS_VOLUME_ENABLE_AGC, &spkAgc);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_THRESHOLD, &ngThres);
		ms_filter_call_method(f, MS_VOLUME_SET_NOISE_GATE_FLOORGAIN, &floorGain);
	}
	parameterizeEqualizer(as, lc);
}

void MS2AudioStream::postConfigureAudioStream(bool muted) {
	postConfigureAudioStream(mStream, getCCore(), muted);
	forceSpeakerMuted(mSpeakerMuted);
	if (linphone_core_dtmf_received_has_listener(getCCore())) {
		audio_stream_play_received_dtmfs(mStream, false);
	}
	if (mRecordActive)
		startRecording();
}

void MS2AudioStream::setupRingbackPlayer () {
	int pauseTime = 3000;
	audio_stream_play(mStream, getCCore()->sound_conf.ringback_tone);
	ms_filter_call_method(mStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
}

void MS2AudioStream::telephoneEventReceived (int event) {
	static char dtmfTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'A', 'B', 'C', 'D' };
	if ((event < 0) || (event > 15)) {
		lWarning() << "Bad dtmf value " << event;
		return;
	}
	getMediaSessionPrivate().dtmfReceived(dtmfTab[event]);
}

void MS2AudioStream::handleEvent(const OrtpEvent *ev){
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(const_cast<OrtpEvent*>(ev));
	switch (evt){
		case ORTP_EVENT_ZRTP_ENCRYPTION_CHANGED:
			if (isMain()) getGroup().zrtpStarted(this);
		break;
		case ORTP_EVENT_ZRTP_SAS_READY:
			getGroup().authTokenReady(evd->info.zrtp_info.sas, !!evd->info.zrtp_info.verified);
		break;
		case ORTP_EVENT_TELEPHONE_EVENT:
			telephoneEventReceived(evd->info.telephone_event);
		break;
	}
}

void MS2AudioStream::enableMic(bool value){
	mMicMuted = !value;

	if (mMicMuted)
		audio_stream_set_mic_gain(mStream, 0);
	else
		audio_stream_set_mic_gain_db(mStream, getCCore()->sound_conf.soft_mic_lev);
}

bool MS2AudioStream::micEnabled()const{
	return !mMicMuted;
}

void MS2AudioStream::enableSpeaker(bool value){
	mSpeakerMuted = !value;
	forceSpeakerMuted(mSpeakerMuted);
}

bool MS2AudioStream::speakerEnabled()const{
	return !mSpeakerMuted;
}

void MS2AudioStream::sendDtmf(int dtmf){
	audio_stream_send_dtmf(mStream, (char)dtmf);
}

void MS2AudioStream::startRecording(){
	if (getMediaSessionPrivate().getParams()->getRecordFilePath().empty()) {
		lError() << "MS2AudioStream::startRecording(): no output file specified. Use MediaSessionParams::setRecordFilePath()";
		return;
	}
	if (getMediaSessionPrivate().getParams()->getPrivate()->getInConference()){
		lWarning() << "MS2AudioStream::startRecording(): not supported in conference.";
		return;
	}
	if (media_stream_get_state(&mStream->ms) == MSStreamStarted) audio_stream_mixed_record_start(mStream);
	mRecordActive = true;
}

void MS2AudioStream::stopRecording(){
	if (mRecordActive)
		audio_stream_mixed_record_stop(mStream);
	mRecordActive = false;
}

float MS2AudioStream::getPlayVolume(){
	if (mStream->volrecv) {
		float vol = 0;
		ms_filter_call_method(mStream->volrecv, MS_VOLUME_GET, &vol);
		return vol;
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MS2AudioStream::getRecordVolume(){
	if (mStream->volsend && !mMicMuted) {
		float vol = 0;
		ms_filter_call_method(mStream->volsend, MS_VOLUME_GET, &vol);
		return vol;
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MS2AudioStream::getMicGain(){
	return audio_stream_get_sound_card_input_gain(mStream);
}

void MS2AudioStream::setMicGain(float gain){
	audio_stream_set_sound_card_input_gain(mStream, gain);
}

float MS2AudioStream::getSpeakerGain(){
	return audio_stream_get_sound_card_output_gain(mStream);
}

void MS2AudioStream::setSpeakerGain(float gain){
	audio_stream_set_sound_card_output_gain(mStream, gain);
}

VideoStream *MS2AudioStream::getPeerVideoStream(){
#ifdef VIDEO_ENABLED
	MS2VideoStream *vs = getGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	return vs ? (VideoStream*)vs->getMediaStream() : nullptr;
#else
	return nullptr;
#endif
}

void MS2AudioStream::enableEchoCancellation(bool value){
	if (mStream->ec) {
		bool bypassMode = !value;
		ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_SET_BYPASS_MODE, &bypassMode);
	}
	
}

bool MS2AudioStream::echoCancellationEnabled()const{
	if (!mStream->ec)
		return !!linphone_core_echo_cancellation_enabled(getCCore());

	bool_t val;
	ms_filter_call_method(mStream->ec, MS_ECHO_CANCELLER_GET_BYPASS_MODE, &val);
	return !val;
}
	
void MS2AudioStream::setInputDevice(AudioDevice *audioDevice) {
	audio_stream_set_input_ms_snd_card(mStream, audioDevice->getSoundCard());
}

void MS2AudioStream::setOutputDevice(AudioDevice *audioDevice) {
	audio_stream_set_output_ms_snd_card(mStream, audioDevice->getSoundCard());
}

AudioDevice* MS2AudioStream::getInputDevice() const {
	MSSndCard *card = audio_stream_get_input_ms_snd_card(mStream);
	return getCore().findAudioDeviceMatchingMsSoundCard(card);
}

AudioDevice* MS2AudioStream::getOutputDevice() const {
	MSSndCard *card = audio_stream_get_output_ms_snd_card(mStream);
	return getCore().findAudioDeviceMatchingMsSoundCard(card);
}

void MS2AudioStream::finish(){
	if (mStream){
		stopRecording();	// ensure to stop recording if recording
		audio_stream_stop(mStream);
		mStream = nullptr;
	}
	MS2Stream::finish();
}

MS2AudioMixer *MS2AudioStream::getAudioMixer(){
	StreamMixer *mixer = getMixer();
	if (mixer){
		MS2AudioMixer * audioMixer = dynamic_cast<MS2AudioMixer*>(mixer);
		if (!audioMixer){
			lError() << *this << " does not have a mixer it is able to interface with.";
		}
		return audioMixer;
	}
	return nullptr;
}

MS2AudioStream::~MS2AudioStream(){
	if (mStream)
		finish();
}


LINPHONE_END_NAMESPACE
