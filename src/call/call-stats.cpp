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

CallStats::CallStats(const CallStats &other) : HybridObject(other), mEncryptionStatus{other.mEncryptionStatus} {
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
	mFecStats = other.mFecStats;
	mRtpRemoteFamily = other.mRtpRemoteFamily;
	mClockrate = other.mClockrate;
	mEstimatedDownloadBandwidth = other.mEstimatedDownloadBandwidth;
	mRtcpReceivedViaMux = other.mRtcpReceivedViaMux;
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
	if (stream->sessions.fec_session) media_stream_get_local_fec_stats(stream, &mFecStats);
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
			mEncryptionStatus.setZrtpCipherAlgo(evd->info.zrtp_info.cipherAlgo);
			mEncryptionStatus.setZrtpKeyAgreementAlgo(evd->info.zrtp_info.keyAgreementAlgo);
			mEncryptionStatus.setZrtpHashAlgo(evd->info.zrtp_info.hashAlgo);
			mEncryptionStatus.setZrtpAuthTagAlgo(evd->info.zrtp_info.authTagAlgo);
			mEncryptionStatus.setZrtpSasAlgo(evd->info.zrtp_info.sasAlgo);
		} else if (evt == ORTP_EVENT_SRTP_ENCRYPTION_CHANGED) {
			if (evd->info.srtp_info.is_inner) {
				if (evd->info.srtp_info.is_send) {
					mEncryptionStatus.setInnerSrtpSendSuite(evd->info.srtp_info.suite);
					mEncryptionStatus.setInnerSrtpSendSource(evd->info.srtp_info.source);
				} else {
					mEncryptionStatus.setInnerSrtpRecvSuite(evd->info.srtp_info.suite);
					mEncryptionStatus.setInnerSrtpRecvSource(evd->info.srtp_info.source);
				}
			} else {
				if (evd->info.srtp_info.is_send) {
					mEncryptionStatus.setSrtpSendSuite(evd->info.srtp_info.suite);
					mEncryptionStatus.setSrtpSendSource(evd->info.srtp_info.source);
				} else {
					mEncryptionStatus.setSrtpRecvSuite(evd->info.srtp_info.suite);
					mEncryptionStatus.setSrtpRecvSource(evd->info.srtp_info.source);
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

uint64_t CallStats::getFecCumulativeLostPacketsNumber() const {
	return mFecStats.packets_not_recovered;
}

uint64_t CallStats::getFecRepairedPacketsNumber() const {
	return mFecStats.packets_recovered;
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

const EncryptionStatus &CallStats::getEncryptionStatus() const {
	return mEncryptionStatus;
}

void CallStats::setEstimatedDownloadBandwidth(float estimatedValue) {
	mEstimatedDownloadBandwidth = estimatedValue;
}

const ZrtpAlgo *CallStats::getZrtpAlgo() const {
	return mEncryptionStatus.getZrtpAlgo();
}

const char *CallStats::getZrtpCipherAlgo() const {
	return mEncryptionStatus.getZrtpCipherAlgo();
}

const char *CallStats::getZrtpKeyAgreementAlgo() const {
	return mEncryptionStatus.getZrtpKeyAgreementAlgo();
}

bool_t CallStats::isZrtpKeyAgreementAlgoPostQuantum() const {
	return mEncryptionStatus.isZrtpKeyAgreementAlgoPostQuantum();
}

const char *CallStats::getZrtpHashAlgo() const {
	return mEncryptionStatus.getZrtpHashAlgo();
}

const char *CallStats::getZrtpAuthTagAlgo() const {
	return mEncryptionStatus.getZrtpAuthTagAlgo();
}

const char *CallStats::getZrtpSasAlgo() const {
	return mEncryptionStatus.getZrtpSasAlgo();
}

const SrtpInfo *CallStats::getSrtpInfo(bool isInner) const {
	if (isInner == TRUE) {
		return mEncryptionStatus.getInnerSrtpInfo();
	} else {
		return mEncryptionStatus.getSrtpInfo();
	}
}

LinphoneSrtpSuite CallStats::getSrtpSuite() const {
	// When send and receive suite are different, setting is not complete (on going nego), returns invalid
	auto sendSuite = mEncryptionStatus.getSrtpSendSuite();
	auto recvSuite = mEncryptionStatus.getSrtpRecvSuite();
	if (sendSuite != recvSuite) {
		return LinphoneSrtpSuiteInvalid;
	}
	switch (sendSuite) {
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
	return mEncryptionStatus.getMediaEncryption();
}

/* RTP stats */

uint64_t CallStats::getRtpPacketSent() const {
	return mRtpStats.packet_sent;
}
uint64_t CallStats::getRtpPacketRcv() const {
	return mRtpStats.packet_recv;
}
uint64_t CallStats::getRtpSent() const {
	return mRtpStats.sent;
}
uint64_t CallStats::getRtpRecv() const {
	return mRtpStats.recv;
}
uint64_t CallStats::getRtpHwRecv() const {
	return mRtpStats.hw_recv;
}
int64_t CallStats::getRtpCumPacketLoss() const {
	return mRtpStats.cum_packet_loss;
}
uint64_t CallStats::getRtpDiscarded() const {
	return mRtpStats.discarded;
}

LINPHONE_END_NAMESPACE
