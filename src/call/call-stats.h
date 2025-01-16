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

#ifndef _L_CALL_STATS_H_
#define _L_CALL_STATS_H_

#include "belle-sip/object++.hh"

#include "c-wrapper/c-wrapper.h"
#include "encryption-status.h"
#include "linphone/api/c-call-stats.h"
#include "linphone/api/c-types.h"
#include "private_structs.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class LINPHONE_PUBLIC CallStats : public bellesip::HybridObject<LinphoneCallStats, CallStats>, public UserDataAccessor {
public:
	CallStats() = default;
	CallStats(const CallStats &other);
	~CallStats();

	CallStats *clone() const override;

	void update(MediaStream *stream);
	void fill(MediaStream *ms, OrtpEvent *ev);

	/* Getters */
	LinphoneStreamType getType() const;
	float getSenderLossRate() const;
	float getReceiverLossRate() const;
	float getLocalLossRate() const;
	float getLocalLateRate() const;
	float getSenderInterarrivalJitter() const;
	float getReceiverInterarrivalJitter() const;
	const rtp_stats_t *getRtpStats() const;
	uint64_t getLatePacketsCumulativeNumber() const;
	uint64_t getFecCumulativeLostPacketsNumber() const;
	uint64_t getFecRepairedPacketsNumber() const;
	float getDownloadBandwidth() const;
	float getUploadBandwidth() const;
	float getFecDownloadBandwidth() const;
	float getFecUploadBandwidth() const;
	float getRtcpDownloadBandwidth() const;
	float getRtcpUploadBandwidth() const;
	LinphoneIceState getIceState() const;
	LinphoneUpnpState getUpnpState() const;
	LinphoneAddressFamily getIpFamilyOfRemote() const;
	float getJitterBufferSizeMs() const;
	float getRoundTripDelay() const;
	float getEstimatedDownloadBandwidth() const;
	mblk_t *getReceivedRtcp() const;
	mblk_t *getSentRtcp() const;
	int getUpdated() const;
	bool_t rtcpReceivedViaMux() const;
	bool_t hasReceivedRtcp() const;
	bool_t hasSentRtcp() const;
	const EncryptionStatus &getEncryptionStatus() const;

	/* Setters */
	void setEstimatedDownloadBandwidth(float estimatedValue);
	void setIceState(LinphoneIceState state);
	void setType(LinphoneStreamType type);
	void setReceivedRtcp(mblk_t *m);
	void setSentRtcp(mblk_t *m);
	void setUpdated(int updated);
	void setRtpStats(const rtp_stats_t *rtpStats);
	void setDownloadBandwidth(float bandwidth);
	void setUploadBandwidth(float bandwidth);
	void setFecDownloadBandwidth(float bandwidth);
	void setFecUploadBandwidth(float bandwidth);
	void setRtcpDownloadBandwidth(float bandwidth);
	void setRtcpUploadBandwidth(float bandwidth);
	void setIpFamilyOfRemote(LinphoneAddressFamily family);

	/* ZRTP stats */
	const ZrtpAlgo *getZrtpAlgo() const;
	const char *getZrtpCipherAlgo() const;
	const char *getZrtpKeyAgreementAlgo() const;
	bool_t isZrtpKeyAgreementAlgoPostQuantum() const;
	const char *getZrtpHashAlgo() const;
	const char *getZrtpAuthTagAlgo() const;
	const char *getZrtpSasAlgo() const;

	/* SRTP stats */
	const SrtpInfo *getSrtpInfo(bool isInner) const;
	LinphoneSrtpSuite getSrtpSuite() const;
	LinphoneMediaEncryption getSrtpSource() const;

	/* RTP stats */
	uint64_t getRtpPacketSent() const;
	uint64_t getRtpPacketRcv() const;
	uint64_t getRtpSent() const;
	uint64_t getRtpRecv() const;
	uint64_t getRtpHwRecv() const;
	int64_t getRtpCumPacketLoss() const;
	uint64_t getRtpDiscarded() const;

private:
	EncryptionStatus mEncryptionStatus;
	LinphoneStreamType mType;        /**< Type of the stream which the stats refer to */
	jitter_stats_t mJitterStats;     /**<jitter buffer statistics, see oRTP documentation for details */
	mblk_t *mReceivedRtcp = nullptr; /**<Last RTCP packet received, as a mblk_t structure. See oRTP documentation for
	                                    details how to extract information from it*/
	mblk_t *mSentRtcp = nullptr;  /**<Last RTCP packet sent, as a mblk_t structure. See oRTP documentation for details
	                                 how to  extract information from it*/
	float mRoundTripDelay;        /**<Round trip propagation time in seconds if known, -1 if unknown.*/
	LinphoneIceState mIceState;   /**< State of ICE processing. */
	LinphoneUpnpState mUpnpState; /**< State of uPnP processing. */
	float mDownloadBandwidth;     /**<Download bandwidth measurement of received stream, expressed in kbit/s, including
	                                       IP/UDP/RTP headers*/
	float mUploadBandwidth; /**<Download bandwidth measurement of sent stream, expressed in kbit/s, including IP/UDP/RTP
	                                       headers*/
	float mFecDownloadBandwidth; /**<Download bandwidth measurement of received stream for FEC only, expressed in
	                                       kbit/s, including IP/UDP/RTP headers*/
	float mFecUploadBandwidth;   /**<Download bandwidth measurement of sent stream for FEC only, expressed in kbit/s,
	                                       including IP/UDP/RTP   headers*/
	float mLocalLateRate;        /**<percentage of packet received too late over last second*/
	float mLocalLossRate;        /**<percentage of lost packet over last second*/
	int mUpdated; /**< Tell which RTCP packet has been updated (received_rtcp or sent_rtcp). Can be either
	                                       LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE or
	                LINPHONE_CALL_STATS_SENT_RTCP_UPDATE */
	float mRtcpDownloadBandwidth; /**<RTCP download bandwidth measurement of received stream, expressed in kbit/s,
	                                       including IP/UDP/RTP headers*/
	float mRtcpUploadBandwidth;   /**<RTCP download bandwidth measurement of sent stream, expressed in kbit/s, including
	                                         IP/UDP/RTP headers*/
	rtp_stats_t mRtpStats;        /**< RTP stats */
	fec_stats_t mFecStats;        /**< FEC stats */
	int mRtpRemoteFamily;         /**< Ip adress family of the remote destination */
	int mClockrate; /*RTP clockrate of the stream, provided here for easily converting timestamp units expressed in RTCP
	                                       packets in milliseconds*/
	float mEstimatedDownloadBandwidth; /**<Estimated download bandwidth measurement of received stream, expressed in
	                                       kbit/s, including IP/UDP/RTP headers*/
	bool_t mRtcpReceivedViaMux;        /*private flag, for non-regression test only*/
};

LINPHONE_END_NAMESPACE

#endif // _L_CALL_STATS_H_
