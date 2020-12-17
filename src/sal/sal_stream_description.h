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
#include "c-wrapper/internal/c-sal.h"
#include "ortp/rtpsession.h"

typedef struct SalStreamDescription {
	std::string name; /*unique name of stream, in order to ease offer/answer model algorithm*/
	SalMediaProto proto;
	SalStreamType type;
	std::string typeother;
	std::string proto_other;
	std::string rtp_addr;
	std::string rtcp_addr;
	unsigned int rtp_ssrc;
	std::string rtcp_cname;
	int rtp_port;
	int rtcp_port;
	MSList *payloads; /**<list of PayloadType */
	MSList *already_assigned_payloads; /**<list of PayloadType offered in the past, used for correct allocation of payload type numbers*/
	int bandwidth;
	int ptime;
	int maxptime;
	SalStreamDir dir;
	std::vector<SalSrtpCryptoAlgo> crypto;
	unsigned int crypto_local_tag;
	int max_rate;
	bool_t bundle_only;
	bool_t implicit_rtcp_fb;
	std::vector<bool_t> pad; /* Use me */
	OrtpRtcpFbConfiguration rtcp_fb;
	OrtpRtcpXrConfiguration rtcp_xr;
	SalCustomSdpAttribute *custom_sdp_attributes;
	std::vector<SalIceCandidate> ice_candidates;
	std::vector<SalIceRemoteCandidate> ice_remote_candidates;
	std::string ice_ufrag;
	std::string ice_pwd;
	std::string mid; /* Media line identifier for RTP bundle mode */
	int mid_rtp_ext_header_id; /* Identifier for the MID field in the RTP extension header */
	bool_t ice_mismatch;
	bool_t set_nortpproxy; /*Formely set by ICE to indicate to the proxy that it has nothing to do*/
	bool_t rtcp_mux;
	uint8_t haveZrtpHash; /**< flag for zrtp hash presence */
	uint8_t haveLimeIk; /**< flag for lime Ik presence */
	uint8_t zrtphash[128];
	std::string dtls_fingerprint;
	SalDtlsRole dtls_role;
	int ttl; /*for multicast -1 to disable*/
	SalMulticastRole multicast_role;
} SalStreamDescription;

#endif // ifndef _SAL_STREAM_DESCRIPTION_H_
