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

#include "linphone/utils/utils.h"
#include "c-wrapper/internal/c-tools.h"
#include "sal/sal_stream_description.h"

SalStreamDescription::SalStreamDescription(){
	init();
}

SalStreamDescription::~SalStreamDescription(){
	destroy();
}

SalStreamDescription::SalStreamDescription(const SalStreamDescription & other){
	name = other.name;
	proto = other.proto;
	type = other.type;
	typeother = other.typeother;
	proto_other = other.proto_other;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	for (const auto & pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	for (const auto & pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	crypto_local_tag = other.crypto_local_tag;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad = other.pad;
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);
	ice_candidates = other.ice_candidates;
	ice_remote_candidates = other.ice_remote_candidates;
	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	mid = other.mid;
	mid_rtp_ext_header_id = other.mid_rtp_ext_header_id;
	ice_mismatch = other.ice_mismatch;
	set_nortpproxy = other.set_nortpproxy;
	rtcp_mux = other.rtcp_mux;
	haveZrtpHash = other.haveZrtpHash;
	haveLimeIk = other.haveLimeIk;
	memcpy(zrtphash, other.zrtphash, sizeof(zrtphash));
	dtls_fingerprint = other.dtls_fingerprint;
	dtls_role = other.dtls_role;
	ttl = other.ttl;
	multicast_role = other.multicast_role;
}

SalStreamDescription &SalStreamDescription::operator=(const SalStreamDescription & other){
	destroy();
	name = other.name;
	proto = other.proto;
	type = other.type;
	typeother = other.typeother;
	proto_other = other.proto_other;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	for (const auto & pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	for (const auto & pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	crypto_local_tag = other.crypto_local_tag;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad = other.pad;
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);
	ice_candidates = other.ice_candidates;
	ice_remote_candidates = other.ice_remote_candidates;
	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	mid = other.mid;
	mid_rtp_ext_header_id = other.mid_rtp_ext_header_id;
	ice_mismatch = other.ice_mismatch;
	set_nortpproxy = other.set_nortpproxy;
	rtcp_mux = other.rtcp_mux;
	haveZrtpHash = other.haveZrtpHash;
	haveLimeIk = other.haveLimeIk;
	memcpy(zrtphash, other.zrtphash, sizeof(zrtphash));
	dtls_fingerprint = other.dtls_fingerprint;
	dtls_role = other.dtls_role;
	ttl = other.ttl;
	multicast_role = other.multicast_role;

	return *this;
}

void SalStreamDescription::init() {
	custom_sdp_attributes = NULL;

	payloads.clear();
	already_assigned_payloads.clear();
	pad.clear();
	crypto.clear();
	ice_candidates.clear();
	ice_remote_candidates.clear();
}

void SalStreamDescription::destroy() {
	for (auto & pt : payloads) {
		payload_type_destroy(pt);
	}
	payloads.clear();
	for (auto & pt : already_assigned_payloads) {
		payload_type_destroy(pt);
	}
	already_assigned_payloads.clear();
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

bool SalStreamDescription::isRecvOnly(const PayloadType *p) const {
	return (p->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && ! (p->flags & PAYLOAD_TYPE_FLAG_CAN_SEND);
}

bool SalStreamDescription::isSamePayloadType(const PayloadType *p1, const PayloadType *p2) const {
	if (p1->type!=p2->type) return false;
	if (strcmp(p1->mime_type,p2->mime_type)!=0) return false;
	if (p1->clock_rate!=p2->clock_rate) return false;
	if (p1->channels!=p2->channels) return false;
	if (payload_type_get_number(p1) != payload_type_get_number(p2)) return false;
	/*
	 Do not compare fmtp right now: they are modified internally when the call is started
	*/
	/*
	if (!fmtp_equals(p1->recv_fmtp,p2->recv_fmtp) ||
		!fmtp_equals(p1->send_fmtp,p2->send_fmtp))
		return false;
	*/
	return true;
}

bool SalStreamDescription::isSamePayloadList(const std::list<PayloadType*> & l1, const std::list<PayloadType*> & l2) const {
	auto p1 = l1.cbegin();
	auto p2 = l2.cbegin();
	for(; (p1 != l1.cend() && p2 != l2.cend()); ++p1, ++p2){
		if (!isSamePayloadType(*p1,*p2))
			return false;
	}
	if (p1!=l1.cend()){
		/*skip possible recv-only payloads*/
		for(;p1!=l1.cend() && isRecvOnly(*p1);++p1){
			ms_message("Skipping recv-only payload type...");
		}
	}
	if (p1!=l1.cend() || p2!=l2.cend()){
		/*means one list is longer than the other*/
		return false;
	}
	return true;
}

bool SalStreamDescription::operator==(const SalStreamDescription & other) const {
	return equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED;
}

int SalStreamDescription::equal(const SalStreamDescription & other) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	/* A different proto should result in SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED but the encryption change
	   needs a stream restart for now, so use SAL_MEDIA_DESCRIPTION_CODEC_CHANGED */
	if (proto != other.proto) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	for(auto crypto1 = crypto.cbegin(), crypto2 = other.crypto.cbegin(); (crypto1 != crypto.cend() && crypto2 != other.crypto.cend()); ++crypto1, ++crypto2){
		if ((crypto1->tag != crypto2->tag)
			|| (crypto1->algo != crypto2->algo)){
			result|=SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		}
		if ((strncmp(crypto1->master_key, crypto2->master_key, sizeof(crypto1->master_key) - 1))) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
	}

	if (crypto.size() != other.crypto.size()) {
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}

	if (type != other.type) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (rtp_addr.compare(other.rtp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if ((rtp_addr.empty()==false) && (other.rtp_addr.empty()==false) && ms_is_multicast(L_STRING_TO_C(rtp_addr)) != ms_is_multicast(L_STRING_TO_C(other.rtp_addr)))
			result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (multicast_role != other.multicast_role) result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (rtp_port != other.rtp_port) {
		if ((rtp_port == 0) || (other.rtp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (rtcp_addr.compare(other.rtcp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (rtcp_port != other.rtcp_port) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (!isSamePayloadList(payloads, other.payloads)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (bandwidth != other.bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (ptime != other.ptime) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (dir != other.dir) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (ice_ufrag.compare(other.ice_ufrag) != 0 && !other.ice_ufrag.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (ice_pwd.compare(other.ice_pwd) != 0 && !other.ice_pwd.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;


	/*DTLS*/
	if (dtls_role != other.dtls_role) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	if (dtls_fingerprint.compare(other.dtls_fingerprint) != 0) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	return result;
}

bool SalStreamDescription::enabled() const {
	/* When the bundle-only attribute is present, a 0 rtp port doesn't mean that the stream is disabled.*/
	return rtp_port > 0 || bundle_only;
}

void SalStreamDescription::disable(){
	rtp_port = 0;
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	mid.clear();
	bundle_only = false;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamDescription::hasAvpf() const {
	switch (proto){
		case SalProtoRtpAvpf:
		case SalProtoRtpSavpf:
		case SalProtoUdpTlsRtpSavpf:
			return true;
		case SalProtoRtpAvp:
		case SalProtoRtpSavp:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamDescription::hasIpv6() const {
	return rtp_addr.find(':') != std::string::npos;
}

bool SalStreamDescription::hasImplicitAvpf() const {
	return implicit_rtcp_fb;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamDescription::hasSrtp() const {
	switch (proto){
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
			return true;
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamDescription::hasDtls() const {
	switch (proto){
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
			return true;
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamDescription::hasZrtp() const {
	if (haveZrtpHash==1) return true;
	return false;
}

bool SalStreamDescription::hasLimeIk() const {
	if (haveLimeIk==1) return true;
	return false;
}

const std::string & SalStreamDescription::getRtcpAddress() const {
	return rtcp_addr;
}

const int & SalStreamDescription::getRtcpPort() const {
	return rtcp_port;
}

const std::string & SalStreamDescription::getRtpAddress() const {
	return rtp_addr;
}

const int & SalStreamDescription::getRtpPort() const {
	return rtp_port;
}

const SalStreamType & SalStreamDescription::getType() const {
	return type;
}

const std::string SalStreamDescription::getTypeAsString() const {
	if (type==SalOther) return typeother;
	else return LinphonePrivate::Utils::toString(type);
}

const SalMediaProto & SalStreamDescription::getProto() const {
	return proto;
}

const std::string SalStreamDescription::getProtoAsString() const {
	if (proto==SalProtoOther) return proto_other;
	else return LinphonePrivate::Utils::toString(proto);
}

SalStreamDir SalStreamDescription::getDirection() const {
	return dir;
}

const std::list<PayloadType*> & SalStreamDescription::getPayloads() const {
	return payloads;
}
