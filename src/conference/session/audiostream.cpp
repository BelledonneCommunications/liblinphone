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
#include <bctoolbox/defs.h>

#include "streams.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "call/call-p.h"
#include "conference/participant.h"

#include "linphone/core.h"

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2AudioStream implementation.
 */

MS2AudioStream::MS2AudioStream(StreamsGroup &sg, const StreamParams &params) : MS2Stream(sg, params){
	
	mStream = audio_stream_new2(getCCore()->factory, getBindIp().c_str(), mPortConfig.rtpPort, mPortConfig.rtcpPort);
	initializeSessions((MediaStream*)mStream);

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
		setZrtpCryptoTypesParameters(&zrtpParams, params.mRemoteStreamDescription ? params.mRemoteStreamDescription->haveZrtpHash : false);
		audio_stream_enable_zrtp(mStream, &zrtpParams);
		if (peerUri)
			ms_free(peerUri);
		if (selfUri)
			ms_free(selfUri);
	}
}

void MS2AudioStream::setZrtpCryptoTypesParameters(MSZrtpParams *params, bool haveRemoteZrtpHash) {
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
	
	params->autoStart =  (getMediaSessionPrivate().getParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) && (haveRemoteZrtpHash == false) ;
}


void MS2AudioStream::initialize(){
	MSSndCard *playcard = getCCore()->sound_conf.lsd_card ? getCCore()->sound_conf.lsd_card : getCCore()->sound_conf.play_sndcard;
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
	
	MS2Stream::initialize();
}

MediaStream *MS2AudioStream::getMediaStream()const{
	return &mStream->ms;
}

void MS2AudioStream::start(SalStreamDescription *stream, CallSession::State targetState){
	if (stream->dir != SalStreamInactive && stream->rtp_port != 0) {
		int usedPt = -1;
		onHoldFile = "";
		audioProfile = makeProfile(resultDesc, stream, &usedPt);
		if (usedPt == -1)
			lWarning() << "No audio stream accepted?";
		else {
			const char *rtpAddr = (stream->rtp_addr[0] != '\0') ? stream->rtp_addr : resultDesc->addr;
			bool isMulticast = !!ms_is_multicast(rtpAddr);
			bool ok = true;
			getCurrentParams()->getPrivate()->setUsedAudioCodec(rtp_profile_get_payload(audioProfile, usedPt));
			getCurrentParams()->enableAudio(true);
			MSSndCard *playcard = q->getCore()->getCCore()->sound_conf.lsd_card ? q->getCore()->getCCore()->sound_conf.lsd_card : q->getCore()->getCCore()->sound_conf.play_sndcard;
			if (!playcard)
				lWarning() << "No card defined for playback!";
			MSSndCard *captcard = q->getCore()->getCCore()->sound_conf.capt_sndcard;
			if (!captcard)
				lWarning() << "No card defined for capture!";
			string playfile = L_C_TO_STRING(q->getCore()->getCCore()->play_file);
			string recfile = L_C_TO_STRING(q->getCore()->getCCore()->rec_file);
			/* Don't use file or soundcard capture when placed in recv-only mode */
			if ((stream->rtp_port == 0) || (stream->dir == SalStreamRecvOnly) || ((stream->multicast_role == SalMulticastReceiver) && isMulticast)) {
				captcard = nullptr;
				playfile = "";
			}
			if (targetState == CallSession::State::Paused) {
				// In paused state, we never use soundcard
				playcard = captcard = nullptr;
				recfile = "";
				// And we will eventually play "playfile" if set by the user
			}
			if (listener && listener->isPlayingRingbackTone(q->getSharedFromThis())) {
				captcard = nullptr;
				playfile = ""; /* It is setup later */
				if (lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "send_ringback_without_playback", 0) == 1) {
					playcard = nullptr;
					recfile = "";
				}
			}
			// If playfile are supplied don't use soundcards
			bool useRtpIo = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "rtp_io", false);
			bool useRtpIoEnableLocalOutput = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "rtp_io_enable_local_output", false);
			if (q->getCore()->getCCore()->use_files || (useRtpIo && !useRtpIoEnableLocalOutput)) {
				captcard = playcard = nullptr;
			}
			if (getParams()->getPrivate()->getInConference()) {
				// First create the graph without soundcard resources
				captcard = playcard = nullptr;
			}
			if (listener && !listener->areSoundResourcesAvailable(q->getSharedFromThis())) {
				lInfo() << "Sound resources are used by another CallSession, not using soundcard";
				captcard = playcard = nullptr;
			}

			if (playcard) {
				ms_snd_card_set_stream_type(playcard, MS_SND_CARD_STREAM_VOICE);
			}
			media_stream_set_max_network_bitrate(&audioStream->ms, linphone_core_get_upload_bandwidth(q->getCore()->getCCore()) * 1000);
			bool useEc = captcard && linphone_core_echo_cancellation_enabled(q->getCore()->getCCore());
			audio_stream_enable_echo_canceller(audioStream, useEc);
			if (playcard && (stream->max_rate > 0))
				ms_snd_card_set_preferred_sample_rate(playcard, stream->max_rate);
			if (captcard && (stream->max_rate > 0))
				ms_snd_card_set_preferred_sample_rate(captcard, stream->max_rate);
			rtp_session_enable_rtcp_mux(audioStream->ms.sessions.rtp_session, stream->rtcp_mux);
			if (!getParams()->getPrivate()->getInConference() && !getParams()->getRecordFilePath().empty()) {
				audio_stream_mixed_record_open(audioStream, getParams()->getRecordFilePath().c_str());
				getCurrentParams()->setRecordFilePath(getParams()->getRecordFilePath());
			}
			// Valid local tags are > 0
			if (sal_stream_description_has_srtp(stream)) {
				const SalStreamDescription *localStreamDesc = sal_media_description_find_stream(localDesc, stream->proto, SalAudio);
				int cryptoIdx = Sal::findCryptoIndexFromTag(localStreamDesc->crypto, static_cast<unsigned char>(stream->crypto_local_tag));
				if (cryptoIdx >= 0) {
					ms_media_stream_sessions_set_srtp_recv_key_b64(&audioStream->ms.sessions, stream->crypto[0].algo, stream->crypto[0].master_key);
					ms_media_stream_sessions_set_srtp_send_key_b64(&audioStream->ms.sessions, stream->crypto[0].algo, localStreamDesc->crypto[cryptoIdx].master_key);
				} else
					lWarning() << "Failed to find local crypto algo with tag: " << stream->crypto_local_tag;
			}
			configureRtpSessionForRtcpFb(stream);
			configureRtpSessionForRtcpXr(SalAudio);
			bool videoWillBeUsed = false;
#if defined(VIDEO_ENABLED)
			const SalStreamDescription *vstream = sal_media_description_find_best_stream(resultDesc, SalVideo);
			if (vstream && (vstream->dir != SalStreamInactive) && vstream->payloads) {
				/* When video is used, do not make adaptive rate control on audio, it is stupid */
				videoWillBeUsed = true;
			}
#endif
			configureAdaptiveRateControl(&audioStream->ms, getCurrentParams()->getUsedAudioCodec(), videoWillBeUsed);
			if (isMulticast)
				rtp_session_set_multicast_ttl(audioStream->ms.sessions.rtp_session, stream->ttl);
			MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;
			if (useRtpIo) {
				if (useRtpIoEnableLocalOutput) {
					io.input.type = MSResourceRtp;
					io.input.session = createAudioRtpIoSession();
					if (playcard) {
						io.output.type = MSResourceSoundcard;
						io.output.soundcard = playcard;
					} else {
						io.output.type = MSResourceFile;
						io.output.file = recfile.empty() ? nullptr : recfile.c_str();
					}
				} else {
					io.input.type = io.output.type = MSResourceRtp;
					io.input.session = io.output.session = createAudioRtpIoSession();
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
				currentCaptureCard = ms_media_resource_get_soundcard(&io.input);
				currentPlayCard = ms_media_resource_get_soundcard(&io.output);

				int err = audio_stream_start_from_io(audioStream, audioProfile, rtpAddr, stream->rtp_port,
					(stream->rtcp_addr[0] != '\0') ? stream->rtcp_addr : resultDesc->addr,
					(linphone_core_rtcp_enabled(q->getCore()->getCCore()) && !isMulticast) ? (stream->rtcp_port ? stream->rtcp_port : stream->rtp_port + 1) : 0,
					usedPt, &io);
				if (err == 0)
					postConfigureAudioStreams((audioMuted || microphoneMuted) && (listener && !listener->isPlayingRingbackTone(q->getSharedFromThis())));
			}
			ms_media_stream_sessions_set_encryption_mandatory(&audioStream->ms.sessions, isEncryptionMandatory());
			if ((targetState == CallSession::State::Paused) && !captcard && !playfile.empty()) {
				int pauseTime = 500;
				ms_filter_call_method(audioStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
			}
			if (listener && listener->isPlayingRingbackTone(q->getSharedFromThis()))
				setupRingbackPlayer();
			if (getParams()->getPrivate()->getInConference() && listener) {
				// Transform the graph to connect it to the conference filter
				bool mute = (stream->dir == SalStreamRecvOnly);
				listener->onCallSessionConferenceStreamStarting(q->getSharedFromThis(), mute);
			}
			getCurrentParams()->getPrivate()->setInConference(getParams()->getPrivate()->getInConference());
			getCurrentParams()->enableLowBandwidth(getParams()->lowBandwidthEnabled());
			// Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute
			SalMediaDescription *remote = op->getRemoteMediaDescription();
			const SalStreamDescription *remoteStream = sal_media_description_find_best_stream(remote, SalAudio);
			if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), LinphoneMediaEncryptionZRTP)) {
				// Perform mutual authentication if instant messaging encryption is enabled
				auto encryptionEngine = q->getCore()->getEncryptionEngine();
				//Is call direction really relevant ? might be linked to offerer/answerer rather than call direction ?
				LinphoneCallDir direction = this->getPublic()->CallSession::getDirection();
				if (encryptionEngine && audioStream->ms.sessions.zrtp_context) {
					encryptionEngine->mutualAuthentication(
														   audioStream->ms.sessions.zrtp_context,
														   op->getLocalMediaDescription(),
														   op->getRemoteMediaDescription(),
														   direction
														   );
				}
				
				//Start zrtp if remote has offered it or if local is configured for zrtp and is the offerrer. If not, defered when ACK is received
				if ((getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP && op->isOfferer()) || (remoteStream->haveZrtpHash == 1)) {
					startZrtpPrimaryChannel(remoteStream);
				}
			}
		}
	}
	
}

void MS2AudioStream::stop(){
	if (!mStream) return;
	
	MS2Stream::stop();
	if (mStream->ec) {
		char *stateStr = nullptr;
		ms_filter_call_method(audioStream->ec, MS_ECHO_CANCELLER_GET_STATE_STRING, &stateStr);
		if (stateStr) {
			lInfo() << "Writing echo canceler state, " << (int)strlen(stateStr) << " bytes";
			lp_config_write_relative_file(getCCore()), ecStateStore, stateStr);
		}
	}
	
	audio_stream_stop(mStream);
	mStream = nullptr;
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);
	
	
	currentCaptureCard = nullptr;
	currentPlayCard = nullptr;
}

MS2AudioStream::~MS2AudioStream(){
}


LINPHONE_END_NAMESPACE
