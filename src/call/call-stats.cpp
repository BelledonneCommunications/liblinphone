/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "call-stats.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

CallStats::CallStats(const CallStats &other) : HybridObject(other) {
	mType = other.mType;
	mJitterStats = other.mJitterStats;
	mReceivedRtcp = other.mReceivedRtcp ? dupmsg(other.mReceivedRtcp) : nullptr;
	mSentRtcp = other.mSentRtcp ? dupmsg(other.mSentRtcp) : nullptr;
	mRoundTripDelay = other.mRoundTripDelay;
	mIceState = other.mIceState;
	mUpnpState = other.mUpnpState;
	mDownloadBandwidth = other.mDownloadBandwidth;
	mUploadBandwidth = other.mUploadBandwidth;
	mFecDownloadBandwidth = other.mFecDownloadBandwidth;
	mFecUploadBandwidth = other.mFecUploadBandwidth;
	mLocalLateRate = other.mLocalLateRate;
	mLocalLossRate = other.mLocalLossRate;
	mUpdated = other.mUpdated;
	mRtcpDownloadBandwidth = other.mRtcpDownloadBandwidth;
	mRtcpUploadBandwidth = other.mRtcpUploadBandwidth;
	mRtpStats = other.mRtpStats;
	mRtpRemoteFamily = other.mRtpRemoteFamily;
	mClockrate = other.mClockrate;
	mEstimatedDownloadBandwidth = other.mEstimatedDownloadBandwidth;
	mRtcpReceivedViaMux = other.mRtcpReceivedViaMux;
	mZrtpAlgo.cipher_algo = other.mZrtpAlgo.cipher_algo;
	mZrtpAlgo.key_agreement_algo = other.mZrtpAlgo.key_agreement_algo;
	mZrtpAlgo.hash_algo = other.mZrtpAlgo.hash_algo;
	mZrtpAlgo.auth_tag_algo = other.mZrtpAlgo.auth_tag_algo;
	mZrtpAlgo.sas_algo = other.mZrtpAlgo.sas_algo;
	mSrtpInfo.send_suite = other.mSrtpInfo.send_suite;
	mSrtpInfo.send_source = other.mSrtpInfo.send_source;
	mSrtpInfo.recv_suite = other.mSrtpInfo.recv_suite;
	mSrtpInfo.recv_source = other.mSrtpInfo.recv_source;
	mInnerSrtpInfo.send_suite = other.mInnerSrtpInfo.send_suite;
	mInnerSrtpInfo.send_source = other.mInnerSrtpInfo.send_source;
	mInnerSrtpInfo.recv_suite = other.mInnerSrtpInfo.recv_suite;
	mInnerSrtpInfo.recv_source = other.mInnerSrtpInfo.recv_source;
}

CallStats::~CallStats() {
	if (mReceivedRtcp) {
		freemsg(mReceivedRtcp);
		mReceivedRtcp = nullptr;
	}
	if (mSentRtcp) {
		freemsg(mSentRtcp);
		mSentRtcp = nullptr;
	}
}

CallStats *CallStats::clone() const {
	return new CallStats(*this);
}

void CallStats::update(MediaStream *stream) {
	PayloadType *pt;
	RtpSession *session = stream->sessions.rtp_session;
	const MSQualityIndicator *qi = media_stream_get_quality_indicator(stream);
	if (qi) {
		mLocalLateRate = ms_quality_indicator_get_local_late_rate(qi);
		mLocalLossRate = ms_quality_indicator_get_local_loss_rate(qi);
	}
	media_stream_get_local_rtp_stats(stream, &mRtpStats);
	pt = rtp_profile_get_payload(rtp_session_get_profile(session), rtp_session_get_send_payload_type(session));
	mClockrate = pt ? pt->clock_rate : 8000;
}

void CallStats::fill(MediaStream *ms, OrtpEvent *ev) {
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(ev);
	if (ms->sessions.rtp_session) {
		if (evt == ORTP_EVENT_RTCP_PACKET_RECEIVED) {
			mRoundTripDelay = rtp_session_get_round_trip_propagation(ms->sessions.rtp_session);
			if (mReceivedRtcp != NULL) freemsg(mReceivedRtcp);
			mReceivedRtcp = evd->packet;
			mRtcpReceivedViaMux = evd->info.socket_type == OrtpRTPSocket;
			evd->packet = NULL;
			mUpdated = LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE;
			update(ms);
		} else if (evt == ORTP_EVENT_RTCP_PACKET_EMITTED) {
			memcpy(&mJitterStats, rtp_session_get_jitter_stats(ms->sessions.rtp_session), sizeof(jitter_stats_t));
			if (mSentRtcp != NULL) freemsg(mSentRtcp);
			mSentRtcp = evd->packet;
			evd->packet = NULL;
			mUpdated = LINPHONE_CALL_STATS_SENT_RTCP_UPDATE;
			update(ms);
		} else if (evt == ORTP_EVENT_ZRTP_SAS_READY) {
			mZrtpAlgo.cipher_algo = evd->info.zrtp_info.cipherAlgo;
			mZrtpAlgo.key_agreement_algo = evd->info.zrtp_info.keyAgreementAlgo;
			mZrtpAlgo.hash_algo = evd->info.zrtp_info.hashAlgo;
			mZrtpAlgo.auth_tag_algo = evd->info.zrtp_info.authTagAlgo;
			mZrtpAlgo.sas_algo = evd->info.zrtp_info.sasAlgo;
		} else if (evt == ORTP_EVENT_SRTP_ENCRYPTION_CHANGED) {
			if (evd->info.srtp_info.is_inner) {
				if (evd->info.srtp_info.is_send) {
					mInnerSrtpInfo.send_suite = evd->info.srtp_info.suite;
					mInnerSrtpInfo.send_source = evd->info.srtp_info.source;
				} else {
					mInnerSrtpInfo.recv_suite = evd->info.srtp_info.suite;
					mInnerSrtpInfo.recv_source = evd->info.srtp_info.source;
				}
			} else {
				if (evd->info.srtp_info.is_send) {
					mSrtpInfo.send_suite = evd->info.srtp_info.suite;
					mSrtpInfo.send_source = evd->info.srtp_info.source;
				} else {
					mSrtpInfo.recv_suite = evd->info.srtp_info.suite;
					mSrtpInfo.recv_source = evd->info.srtp_info.source;
				}
			}
		}
	}
}

LinphoneStreamType CallStats::getType() const {
	return mType;
}

float CallStats::getSenderLossRate() const {
	const report_block_t *srb = NULL;

	if (!mSentRtcp) {
		lWarning() << __func__ << ": there is no RTCP packet sent.";
		return 0.0;
	}
	RtcpParserContext parserCtx;
	const mblk_t *rtcpMessage = rtcp_parser_context_init(&parserCtx, mSentRtcp);

	do {
		if (rtcp_is_SR(rtcpMessage)) srb = rtcp_SR_get_report_block(rtcpMessage, 0);
		else if (rtcp_is_RR(rtcpMessage)) srb = rtcp_RR_get_report_block(rtcpMessage, 0);
		if (srb) break;
	} while ((rtcpMessage = rtcp_parser_context_next_packet(&parserCtx)) != nullptr);
	rtcp_parser_context_uninit(&parserCtx);
	if (!srb) return 0.0;
	return 100.0f * (float)report_block_get_fraction_lost(srb) / 256.0f;
}

float CallStats::getReceiverLossRate() const {
	const report_block_t *rrb = NULL;

	if (!mReceivedRtcp) {
		lWarning() << __func__ << ": there is no RTCP packet received.";
		return 0.0;
	}
	RtcpParserContext parserCtx;
	const mblk_t *rtcpMessage = rtcp_parser_context_init(&parserCtx, mReceivedRtcp);

	do {
		if (rtcp_is_RR(rtcpMessage)) rrb = rtcp_RR_get_report_block(rtcpMessage, 0);
		else if (rtcp_is_SR(rtcpMessage)) rrb = rtcp_SR_get_report_block(rtcpMessage, 0);
		if (rrb) break;
	} while ((rtcpMessage = rtcp_parser_context_next_packet(&parserCtx)) != nullptr);
	rtcp_parser_context_uninit(&parserCtx);
	if (!rrb) return 0.0;
	return 100.0f * (float)report_block_get_fraction_lost(rrb) / 256.0f;
}

float CallStats::getLocalLossRate() const {
	return mLocalLossRate;
}

float CallStats::getLocalLateRate() const {
	return mLocalLateRate;
}

float CallStats::getSenderInterarrivalJitter() const {
	const report_block_t *srb = NULL;

	if (!mSentRtcp) {
		lWarning() << __func__ << ": there is no RTCP packet sent.";
		return 0.0;
	}
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several
	 * mblk_t structure */
	if (mSentRtcp->b_cont != NULL) msgpullup(mSentRtcp, (size_t)-1);
	if (rtcp_is_SR(mSentRtcp)) srb = rtcp_SR_get_report_block(mSentRtcp, 0);
	else if (rtcp_is_RR(mSentRtcp)) srb = rtcp_RR_get_report_block(mSentRtcp, 0);
	if (!srb) return 0.0;
	if (mClockrate == 0) return 0.0;
	return (float)report_block_get_interarrival_jitter(srb) / (float)mClockrate;
}

float CallStats::getReceiverInterarrivalJitter() const {
	const report_block_t *rrb = NULL;

	if (!mReceivedRtcp) {
		ms_warning("linphone_call_stats_get_receiver_interarrival_jitter(): there is no RTCP packet received.");
		return 0.0;
	}
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several
	 * mblk_t structure */
	if (mReceivedRtcp->b_cont != NULL) msgpullup(mReceivedRtcp, (size_t)-1);
	if (rtcp_is_SR(mReceivedRtcp)) rrb = rtcp_SR_get_report_block(mReceivedRtcp, 0);
	else if (rtcp_is_RR(mReceivedRtcp)) rrb = rtcp_RR_get_report_block(mReceivedRtcp, 0);
	if (!rrb) return 0.0;
	if (mClockrate == 0) return 0.0;
	return (float)report_block_get_interarrival_jitter(rrb) / (float)mClockrate;
}

const rtp_stats_t *CallStats::getRtpStats() const {
	return &mRtpStats;
}

uint64_t CallStats::getLatePacketsCumulativeNumber() const {
	return mRtpStats.outoftime;
}

float CallStats::getDownloadBandwidth() const {
	return mDownloadBandwidth;
}

float CallStats::getUploadBandwidth() const {
	return mUploadBandwidth;
}

float CallStats::getFecDownloadBandwidth() const {
	return mFecDownloadBandwidth;
}

float CallStats::getFecUploadBandwidth() const {
	return mFecUploadBandwidth;
}

float CallStats::getRtcpDownloadBandwidth() const {
	return mRtcpDownloadBandwidth;
}

float CallStats::getRtcpUploadBandwidth() const {
	return mRtcpUploadBandwidth;
}

LinphoneIceState CallStats::getIceState() const {
	return mIceState;
}

LinphoneUpnpState CallStats::getUpnpState() const {
	return mUpnpState;
}

LinphoneAddressFamily CallStats::getIpFamilyOfRemote() const {
	return (LinphoneAddressFamily)mRtpRemoteFamily;
}

float CallStats::getJitterBufferSizeMs() const {
	return mJitterStats.jitter_buffer_size_ms;
}

float CallStats::getRoundTripDelay() const {
	return mRoundTripDelay;
}

float CallStats::getEstimatedDownloadBandwidth() const {
	return mEstimatedDownloadBandwidth;
}

void CallStats::setIceState(LinphoneIceState state) {
	mIceState = state;
}

void CallStats::setType(LinphoneStreamType type) {
	mType = type;
}

mblk_t *CallStats::getReceivedRtcp() const {
	return mReceivedRtcp;
}

void CallStats::setReceivedRtcp(mblk_t *m) {
	mReceivedRtcp = m;
}

mblk_t *CallStats::getSentRtcp() const {
	return mSentRtcp;
}

void CallStats::setSentRtcp(mblk_t *m) {
	mSentRtcp = m;
}

int CallStats::getUpdated() const {
	return mUpdated;
}

void CallStats::setUpdated(int updated) {
	mUpdated = updated;
}

void CallStats::setRtpStats(const rtp_stats_t *rtpStats) {
	memcpy(&mRtpStats, rtpStats, sizeof(*rtpStats));
}

void CallStats::setDownloadBandwidth(float bandwidth) {
	mDownloadBandwidth = bandwidth;
}

void CallStats::setUploadBandwidth(float bandwidth) {
	mUploadBandwidth = bandwidth;
}

void CallStats::setFecDownloadBandwidth(float bandwidth) {
	mFecDownloadBandwidth = bandwidth;
}

void CallStats::setFecUploadBandwidth(float bandwidth) {
	mFecUploadBandwidth = bandwidth;
}

void CallStats::setRtcpDownloadBandwidth(float bandwidth) {
	mRtcpDownloadBandwidth = bandwidth;
}

void CallStats::setRtcpUploadBandwidth(float bandwidth) {
	mRtcpUploadBandwidth = bandwidth;
}

void CallStats::setIpFamilyOfRemote(LinphoneAddressFamily family) {
	mRtpRemoteFamily = family;
}

bool_t CallStats::rtcpReceivedViaMux() const {
	return mRtcpReceivedViaMux;
}

bool_t CallStats::hasReceivedRtcp() const {
	return mReceivedRtcp != NULL;
}

bool_t CallStats::hasSentRtcp() const {
	return mSentRtcp != NULL;
}

void CallStats::setEstimatedDownloadBandwidth(float estimatedValue) {
	mEstimatedDownloadBandwidth = estimatedValue;
}

const ZrtpAlgo *CallStats::getZrtpAlgo() const {
	return &mZrtpAlgo;
}

const char *CallStats::getZrtpCipherAlgo() const {
	switch (mZrtpAlgo.cipher_algo) {
		case (MS_ZRTP_CIPHER_INVALID):
			return "invalid";
		case (MS_ZRTP_CIPHER_AES1):
			return "AES-128";
		case (MS_ZRTP_CIPHER_AES2):
			return "AES-192";
		case (MS_ZRTP_CIPHER_AES3):
			return "AES-256";
		case (MS_ZRTP_CIPHER_2FS1):
			return "TwoFish-128";
		case (MS_ZRTP_CIPHER_2FS2):
			return "TwoFish-192";
		case (MS_ZRTP_CIPHER_2FS3):
			return "TwoFish-256";
		default:
			return "Unknown Algo";
	}
}

const char *CallStats::getZrtpKeyAgreementAlgo() const {
	switch (mZrtpAlgo.key_agreement_algo) {
		case (MS_ZRTP_KEY_AGREEMENT_INVALID):
			return "invalid";
		case (MS_ZRTP_KEY_AGREEMENT_DH2K):
			return "DHM-2048";
		case (MS_ZRTP_KEY_AGREEMENT_EC25):
			return "ECDH-256";
		case (MS_ZRTP_KEY_AGREEMENT_DH3K):
			return "DHM-3072";
		case (MS_ZRTP_KEY_AGREEMENT_EC38):
			return "ECDH-384";
		case (MS_ZRTP_KEY_AGREEMENT_EC52):
			return "ECDH-521";
		case (MS_ZRTP_KEY_AGREEMENT_X255):
			return "X25519";
		case (MS_ZRTP_KEY_AGREEMENT_X448):
			return "X448";
		case (MS_ZRTP_KEY_AGREEMENT_K255):
			return "KEM-X25519";
		case (MS_ZRTP_KEY_AGREEMENT_K448):
			return "KEM-X448";
		case (MS_ZRTP_KEY_AGREEMENT_KYB1):
			return "KYBER-512";
		case (MS_ZRTP_KEY_AGREEMENT_KYB2):
			return "KYBER-768";
		case (MS_ZRTP_KEY_AGREEMENT_KYB3):
			return "KYBER-1024";
		case (MS_ZRTP_KEY_AGREEMENT_HQC1):
			return "HQC-128";
		case (MS_ZRTP_KEY_AGREEMENT_HQC2):
			return "HQC-192";
		case (MS_ZRTP_KEY_AGREEMENT_HQC3):
			return "HQC-256";
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512):
			return "X25519/Kyber512";
		case (MS_ZRTP_KEY_AGREEMENT_K255_HQC128):
			return "X25519/HQC128";
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024):
			return "X448/Kyber1024";
		case (MS_ZRTP_KEY_AGREEMENT_K448_HQC256):
			return "X448/HQC256";
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128):
			return "X25519/Kyber512/HQC128";
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256):
			return "X448/Kyber1024/HQC256";
		default:
			return "Unknown Algo";
	}
}

bool_t CallStats::isZrtpKeyAgreementAlgoPostQuantum() const {
	switch (mZrtpAlgo.key_agreement_algo) {
		case (MS_ZRTP_KEY_AGREEMENT_KYB1):
		case (MS_ZRTP_KEY_AGREEMENT_KYB2):
		case (MS_ZRTP_KEY_AGREEMENT_KYB3):
		case (MS_ZRTP_KEY_AGREEMENT_HQC1):
		case (MS_ZRTP_KEY_AGREEMENT_HQC2):
		case (MS_ZRTP_KEY_AGREEMENT_HQC3):
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512):
		case (MS_ZRTP_KEY_AGREEMENT_K255_HQC128):
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024):
		case (MS_ZRTP_KEY_AGREEMENT_K448_HQC256):
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128):
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256):
			return TRUE;
		default:
			return FALSE;
	}
}

const char *CallStats::getZrtpHashAlgo() const {
	switch (mZrtpAlgo.hash_algo) {
		case (MS_ZRTP_HASH_INVALID):
			return "invalid";
		case (MS_ZRTP_HASH_S256):
			return "SHA-256";
		case (MS_ZRTP_HASH_S384):
			return "SHA-384";
		case (MS_ZRTP_HASH_N256):
			return "SHA3-256";
		case (MS_ZRTP_HASH_N384):
			return "SHA3-384";
		case (MS_ZRTP_HASH_S512):
			return "SHA-512";
		default:
			return "Unknown Algo";
	}
}

const char *CallStats::getZrtpAuthTagAlgo() const {
	switch (mZrtpAlgo.auth_tag_algo) {
		case (MS_ZRTP_AUTHTAG_INVALID):
			return "invalid";
		case (MS_ZRTP_AUTHTAG_HS32):
			return "HMAC-SHA1-32";
		case (MS_ZRTP_AUTHTAG_HS80):
			return "HMAC-SHA1-80";
		case (MS_ZRTP_AUTHTAG_SK32):
			return "Skein-32";
		case (MS_ZRTP_AUTHTAG_SK64):
			return "Skein-64";
		case (MS_ZRTP_AUTHTAG_GCM):
			return "GCM";
		default:
			return "Unknown Algo";
	}
}

const char *CallStats::getZrtpSasAlgo() const {
	switch (mZrtpAlgo.sas_algo) {
		case (MS_ZRTP_SAS_INVALID):
			return "invalid";
		case (MS_ZRTP_SAS_B32):
			return "Base32";
		case (MS_ZRTP_SAS_B256):
			return "PGP-WordList";
		default:
			return "Unknown Algo";
	}
}

const SrtpInfo *CallStats::getSrtpInfo(bool_t is_inner) const {
	if (is_inner == TRUE) {
		return &mInnerSrtpInfo;
	} else {
		return &mSrtpInfo;
	}
}

LinphoneSrtpSuite CallStats::getSrtpSuite() const {
	// When send and receive suite are different, setting is not complete (on going nego), returns invalid
	if (mSrtpInfo.send_suite != mSrtpInfo.recv_suite) {
		return LinphoneSrtpSuiteInvalid;
	}
	switch (mSrtpInfo.send_suite) {
		case (MS_AES_128_SHA1_80):
			return LinphoneSrtpSuiteAESCM128HMACSHA180;
		case (MS_AES_256_SHA1_80):
		case (MS_AES_CM_256_SHA1_80):
			return LinphoneSrtpSuiteAES256CMHMACSHA180;
		case (MS_AES_128_SHA1_32):
			return LinphoneSrtpSuiteAESCM128HMACSHA132;
		case (MS_AES_256_SHA1_32):
			return LinphoneSrtpSuiteAES256CMHMACSHA132;
		case (MS_AEAD_AES_128_GCM):
			return LinphoneSrtpSuiteAEADAES128GCM;
		case (MS_AEAD_AES_256_GCM):
			return LinphoneSrtpSuiteAEADAES256GCM;
		case (MS_CRYPTO_SUITE_INVALID):
		default:
			return LinphoneSrtpSuiteInvalid;
	}
}

LinphoneMediaEncryption CallStats::getSrtpSource() const {
	// When send and receive suite are different, setting is not complete (on going nego), returns invalid
	if (mSrtpInfo.send_source != mSrtpInfo.recv_source) {
		return LinphoneMediaEncryptionNone;
	}
	switch (mSrtpInfo.send_source) {
		case (MSSrtpKeySourceSDES):
			return LinphoneMediaEncryptionSRTP;
		case (MSSrtpKeySourceZRTP):
			return LinphoneMediaEncryptionZRTP;
		case (MSSrtpKeySourceDTLS):
			return LinphoneMediaEncryptionDTLS;
		case (MSSrtpKeySourceUnknown):
		case (MSSrtpKeySourceUnavailable):
		case (MSSrtpKeySourceEKT):
		default:
			return LinphoneMediaEncryptionNone;
	}
}

LINPHONE_END_NAMESPACE