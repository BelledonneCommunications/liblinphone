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

#include "c-wrapper/internal/c-sal.h"
#include "ortp/rtpsession.h"

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_CANDIDATES 20

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_REMOTE_CANDIDATES 2

#define SAL_CRYPTO_ALGO_MAX 4

class SalStreamDescription {
public:
	std::string getTypeAsString() const;

	std::string name; /*unique name of stream, in order to ease offer/answer model algorithm*/
	SalMediaProto proto;
	SalStreamType type;
	std::string typeother;
	char proto_other[32];
	char rtp_addr[64];
	char rtcp_addr[64];
	unsigned int rtp_ssrc;
	char rtcp_cname[256];
	int rtp_port;
	int rtcp_port;
	MSList *payloads; /**<list of PayloadType */
	MSList *already_assigned_payloads; /**<list of PayloadType offered in the past, used for correct allocation of payload type numbers*/
	int bandwidth;
	int ptime;
	int maxptime;
	SalStreamDir dir;
	SalSrtpCryptoAlgo crypto[SAL_CRYPTO_ALGO_MAX];
	unsigned int crypto_local_tag;
	int max_rate;
	bool_t bundle_only;
	bool_t implicit_rtcp_fb;
	bool_t pad[2]; /* Use me */
	OrtpRtcpFbConfiguration rtcp_fb;
	OrtpRtcpXrConfiguration rtcp_xr;
	SalCustomSdpAttribute *custom_sdp_attributes;
	SalIceCandidate ice_candidates[SAL_MEDIA_DESCRIPTION_MAX_ICE_CANDIDATES];
	SalIceRemoteCandidate ice_remote_candidates[SAL_MEDIA_DESCRIPTION_MAX_ICE_REMOTE_CANDIDATES];
	char ice_ufrag[SAL_MEDIA_DESCRIPTION_MAX_ICE_UFRAG_LEN];
	char ice_pwd[SAL_MEDIA_DESCRIPTION_MAX_ICE_PWD_LEN];
	char mid[32]; /* Media line identifier for RTP bundle mode */
	int mid_rtp_ext_header_id; /* Identifier for the MID field in the RTP extension header */
	bool_t ice_mismatch;
	bool_t set_nortpproxy; /*Formely set by ICE to indicate to the proxy that it has nothing to do*/
	bool_t rtcp_mux;
	uint8_t haveZrtpHash; /**< flag for zrtp hash presence */
	uint8_t haveLimeIk; /**< flag for lime Ik presence */
	uint8_t zrtphash[128];
	char dtls_fingerprint[256];
	SalDtlsRole dtls_role;
	int ttl; /*for multicast -1 to disable*/
	SalMulticastRole multicast_role;
};

#ifdef __cplusplus
extern "C" {
#endif

/* Enabled means that the stream exists and is accepted as part of the session: the port value is non-zero or the stream has bundle-only attribute.
 *However, it may be marked with a=inactive, which is unrelated to the return value of this function.*/
bool_t sal_stream_description_enabled(const SalStreamDescription *sd);
void sal_stream_description_disable(SalStreamDescription *sd);
bool_t sal_stream_description_has_avpf(const SalStreamDescription *sd);
bool_t sal_stream_description_has_implicit_avpf(const SalStreamDescription *sd);
bool_t sal_stream_description_has_srtp(const SalStreamDescription *sd);
bool_t sal_stream_description_has_dtls(const SalStreamDescription *sd);
bool_t sal_stream_description_has_limeIk(const SalStreamDescription *sd);
bool_t sal_stream_description_has_ipv6(const SalStreamDescription *md);

const char *sal_stream_description_get_type_as_string(const SalStreamDescription *desc);
const char *sal_stream_description_get_proto_as_string(const SalStreamDescription *desc);
bool_t sal_stream_description_has_zrtp(const SalStreamDescription *sd);

int sal_stream_description_equals(const SalStreamDescription *sd1, const SalStreamDescription *sd2);

#ifdef __cplusplus
}
#endif

#endif // ifndef _SAL_STREAM_DESCRIPTION_H_
