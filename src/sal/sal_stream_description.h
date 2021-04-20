/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef _SAL_STREAM_DESCRIPTION_H_
#define _SAL_STREAM_DESCRIPTION_H_

#include <vector>
#include "linphone/utils/general.h"
#include "c-wrapper/internal/c-sal.h"
#include "ortp/rtpsession.h"

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN 64
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_FOUNDATION_LEN 32
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_TYPE_LEN 6

typedef struct SalIceCandidate {
	std::string addr;
	std::string raddr;
	std::string foundation;
	std::string type;
	unsigned int componentID = 0;
	unsigned int priority = 0;
	int port = 0;
	int rport = 0;
} SalIceCandidate;

typedef struct SalIceRemoteCandidate {
	std::string addr;
	int port = 0;
} SalIceRemoteCandidate;

/*sufficient for 256bit keys encoded in base 64*/
#define SAL_SRTP_KEY_SIZE 128

typedef struct SalSrtpCryptoAlgo {
	unsigned int tag = 0;
	MSCryptoSuite algo = MS_CRYPTO_SUITE_INVALID;
	std::string master_key;
} SalSrtpCryptoAlgo;

LINPHONE_BEGIN_NAMESPACE

class SalMediaDescription;

class LINPHONE_PUBLIC SalStreamDescription {

	public:

		SalStreamDescription();
		SalStreamDescription(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc);
		SalStreamDescription(const SalStreamDescription & other);
		virtual ~SalStreamDescription();
		SalStreamDescription &operator=(const SalStreamDescription& other);
		int equal(const SalStreamDescription & other) const;
		bool operator==(const SalStreamDescription & other) const;
		bool operator!=(const SalStreamDescription & other) const;
		belle_sdp_media_description_t * toSdpMediaDescription(const SalMediaDescription * salMediaDesc, belle_sdp_session_description_t *session_desc) const;
		bool enabled() const;
		void disable();

		/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
		bool hasAvpf() const;
		bool hasIpv6() const;
		bool hasImplicitAvpf() const;

		/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
		bool hasSrtp() const;
		bool hasDtls() const;
		bool hasZrtp() const;
		bool hasLimeIk() const;

		bool isMain() const;

		const std::string & getRtcpAddress() const;
		const int & getRtcpPort() const;
		const std::string & getRtpAddress() const;
		const int & getRtpPort() const;
		const std::list<PayloadType*> & getPayloads() const;

		const SalStreamType & getType() const;
		const std::string getTypeAsString() const;
		const SalMediaProto & getProto() const;
		const std::string getProtoAsString() const;
		SalStreamDir getDirection() const;

		bool main = false;
		std::string name; /*unique name of stream, in order to ease offer/answer model algorithm*/
		SalMediaProto proto = SalProtoRtpAvp;
		SalStreamType type = SalAudio;
		std::string typeother;
		std::string proto_other;
		std::string rtp_addr;
		std::string rtcp_addr;
		unsigned int rtp_ssrc = 0;
		std::string rtcp_cname;
		int rtp_port = 0;
		int rtcp_port = 0;
		std::list<PayloadType*> payloads; /**<list of PayloadType */
		std::list<PayloadType*> already_assigned_payloads; /**<list of PayloadType offered in the past, used for correct allocation of payload type numbers*/
		int bandwidth = 0;
		int ptime = 0;
		int maxptime = 0;
		SalStreamDir dir = SalStreamInactive;
		std::vector<SalSrtpCryptoAlgo> crypto;
		unsigned int crypto_local_tag = 0;
		int max_rate = 0;
		bool bundle_only = false;
		bool implicit_rtcp_fb = false;
		bool pad[2]; /* Use me */
		OrtpRtcpFbConfiguration rtcp_fb;
		OrtpRtcpXrConfiguration rtcp_xr;
		SalCustomSdpAttribute *custom_sdp_attributes = nullptr;
		std::vector<SalIceCandidate> ice_candidates;
		std::vector<SalIceRemoteCandidate> ice_remote_candidates;
		std::string ice_ufrag;
		std::string ice_pwd;
		std::string mid; /* Media line identifier for RTP bundle mode */
		int mid_rtp_ext_header_id = 0; /* Identifier for the MID field in the RTP extension header */
		bool ice_mismatch = false;
		bool set_nortpproxy = false; /*Formely set by ICE to indicate to the proxy that it has nothing to do*/
		bool rtcp_mux = false;
		uint8_t haveZrtpHash = 0; /**< flag for zrtp hash presence */
		uint8_t zrtphash[128];
		std::string dtls_fingerprint;
		SalDtlsRole dtls_role = SalDtlsRoleInvalid;
		int ttl = 0; /*for multicast -1 to disable*/
		SalMulticastRole multicast_role = SalMulticastInactive;

	private:
		bool isRecvOnly(const PayloadType *p) const;
		bool isSamePayloadType(const PayloadType *p1, const PayloadType *p2) const;
		bool isSamePayloadList(const std::list<PayloadType*> & l1, const std::list<PayloadType*> & l2) const;

		void addIceRemoteCandidatesToSdp(belle_sdp_media_description_t *md) const;
		void addIceCandidatesToSdp(belle_sdp_media_description_t *md) const;
		void addRtcpFbAttributesToSdp(belle_sdp_media_description_t *media_desc) const;
		bool_t isRtcpFbTrrIntTheSameForAllPayloads(uint16_t *trr_int) const;
		void addMidAttributesToSdp(belle_sdp_media_description_t *media_desc) const;
		void applyRtcpFbAttributeToPayload(belle_sdp_rtcp_fb_attribute_t *fb_attribute, PayloadType *pt);
		bool_t sdpParseRtcpFbParameters(const belle_sdp_media_description_t *media_desc);
		void sdpParsePayloadTypes(const belle_sdp_media_description_t *media_desc);
		void sdpParseMediaCryptoParameters(const belle_sdp_media_description_t *media_desc);
		void sdpParseMediaIceParameters(const belle_sdp_media_description_t *media_desc);
		void enableAvpfForStream();

};

LINPHONE_END_NAMESPACE

#endif // ifndef _SAL_STREAM_DESCRIPTION_H_
