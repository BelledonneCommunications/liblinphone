/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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
#include "sal/sal_stream_configuration.h"
#include "utils/payload-type-handler.h"
#include "bellesip_sal/sal_impl.h"

LINPHONE_BEGIN_NAMESPACE

#define keywordcmp(key,b) strncmp(key,b,sizeof(key))

SalStreamConfiguration::SalStreamConfiguration(){
	custom_sdp_attributes = NULL;

	pad[0] = false;
	pad[1] = false;

	payloads.clear();
	crypto.clear();
	ice_candidates.clear();
	ice_remote_candidates.clear();

}

SalStreamConfiguration::~SalStreamConfiguration(){
	PayloadTypeHandler::clearPayloadList(payloads);
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

SalStreamConfiguration::SalStreamConfiguration(const SalStreamConfiguration & other){
	proto = other.proto;
	proto_other = other.proto_other;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	for (const auto & pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	crypto_local_tag = other.crypto_local_tag;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad[0] = other.pad[0];
	pad[1] = other.pad[1];
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
	tcapIndex = other.tcapIndex;
	acapIndexes = other.acapIndexes;
	delete_media_attributes = other.delete_media_attributes;
	delete_session_attributes = other.delete_session_attributes;
}

SalStreamConfiguration &SalStreamConfiguration::operator=(const SalStreamConfiguration & other){
	proto = other.proto;
	proto_other = other.proto_other;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	PayloadTypeHandler::clearPayloadList(payloads);
	for (const auto & pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	crypto_local_tag = other.crypto_local_tag;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad[0] = other.pad[0];
	pad[1] = other.pad[1];
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
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

	return *this;
}

bool SalStreamConfiguration::isRecvOnly(const PayloadType *p) {
	return (p->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && ! (p->flags & PAYLOAD_TYPE_FLAG_CAN_SEND);
}

bool SalStreamConfiguration::isSamePayloadType(const PayloadType *p1, const PayloadType *p2) {
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

bool SalStreamConfiguration::isSamePayloadList(const std::list<PayloadType*> & l1, const std::list<PayloadType*> & l2) {
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

bool SalStreamConfiguration::operator==(const SalStreamConfiguration & other) const {
	return equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED;
}

bool SalStreamConfiguration::operator!=(const SalStreamConfiguration & other) const {
	return !(*this == other);
}

int SalStreamConfiguration::equal(const SalStreamConfiguration & other) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	/* A different proto should result in SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED but the encryption change
	   needs a stream restart for now, so use SAL_MEDIA_DESCRIPTION_CODEC_CHANGED */
	if (proto != other.proto) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	for(auto crypto1 = crypto.cbegin(), crypto2 = other.crypto.cbegin(); (crypto1 != crypto.cend() && crypto2 != other.crypto.cend()); ++crypto1, ++crypto2){
		if ((crypto1->tag != crypto2->tag)
			|| (crypto1->algo != crypto2->algo)){
			result|=SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		}
		if (crypto1->master_key.compare(crypto2->master_key)) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
	}

	if (crypto.size() != other.crypto.size()) {
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}

	if (!isSamePayloadList(payloads, other.payloads)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
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

void SalStreamConfiguration::disable(){
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	mid.clear();
	bundle_only = false;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamConfiguration::hasAvpf() const {
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

bool SalStreamConfiguration::hasImplicitAvpf() const {
	return implicit_rtcp_fb;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamConfiguration::hasSrtp() const {
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

bool SalStreamConfiguration::hasDtls() const {
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

bool SalStreamConfiguration::hasZrtpHash() const {
	return (haveZrtpHash==1);
}

const uint8_t * SalStreamConfiguration::getZrtpHash() const {
	return zrtphash;
}

bool SalStreamConfiguration::hasZrtp() const {
	return (haveZrtpHash==1);
}

bool SalStreamConfiguration::hasLimeIk() const {
	if (haveLimeIk==1) return true;
	return false;
}

const SalMediaProto & SalStreamConfiguration::getProto() const {
	return proto;
}

const std::string SalStreamConfiguration::getProtoAsString() const {
	if (proto==SalProtoOther) return proto_other;
	else return LinphonePrivate::Utils::toString(proto);
}

SalStreamDir SalStreamConfiguration::getDirection() const {
	return dir;
}

const std::list<PayloadType*> & SalStreamConfiguration::getPayloads() const {
	return payloads;
}

const int & SalStreamConfiguration::getMaxRate() const {
	return max_rate;
}

const std::string & SalStreamConfiguration::getMid() const {
	return mid;
}

bool SalStreamConfiguration::getIceMismatch() const {
	return ice_mismatch;
}

const std::string & SalStreamConfiguration::getIceUfrag() const {
	return ice_ufrag;
}

const std::string & SalStreamConfiguration::getIcePwd() const {
	return ice_pwd;
}

const SalIceCandidate & SalStreamConfiguration::getIceCandidateAtIndex(const std::size_t & idx) const {
	return ice_candidates.at(idx);
}

const SalIceRemoteCandidate & SalStreamConfiguration::getIceRemoteCandidateAtIndex(const std::size_t & idx) const {
	return ice_remote_candidates.at(idx);
}

const int & SalStreamConfiguration::getMidRtpExtHeaderId() const {
	return mid_rtp_ext_header_id;
}

SalCustomSdpAttribute * SalStreamConfiguration::getCustomSdpAttributes() const {
	return custom_sdp_attributes;
}

void SalStreamConfiguration::enableAvpfForStream() {
	for (auto & pt : payloads) {
		payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
	}
}

void SalStreamConfiguration::mergeAcaps(const std::list<std::list<unsigned int>> & acaps) {
	// Avoid adding duplicates
	for (const auto & newIdxs : acaps) {
		bool found = false;
		for (const auto & idxs : acapIndexes) {
			found |= ((idxs.size() == newIdxs.size()) && std::equal(idxs.begin(), idxs.end(), newIdxs.begin()));
		}
		if (!found) {
			acapIndexes.push_back(newIdxs);
		}
	}
}

const std::list<std::list<unsigned int>> & SalStreamConfiguration::getAcapIndexes() const {
	return acapIndexes;
}

const unsigned int & SalStreamConfiguration::getTcapIndex() const {
	return tcapIndex;
}

const std::string SalStreamConfiguration::getSdpString() const {

	std::string acapString;
	// Iterate over all acaps sets. For every set, get the index of all its members
	for (const auto & acapSet : acapIndexes) {
		// Do not append | on first element
		if (!acapString.empty()) {
			acapString.append("|");
		}
		const auto & firstAcapIdx = acapSet.front();
		for (const auto & acapIdx : acapSet) {
			if (acapIdx != firstAcapIdx) {
				acapString.append(",");
			}
			acapString.append(std::to_string(acapIdx));
		}
	}

	const std::string tcapString = std::to_string(tcapIndex);

	std::string deleteAttrs;
	if (delete_media_attributes && delete_session_attributes) {
		deleteAttrs = "-ms:";
	} else if (delete_session_attributes) {
		deleteAttrs = "-s:";
	} else if (delete_media_attributes) {
		deleteAttrs = "-m:";
	}

	const std::string sdpString = "a=" + deleteAttrs + acapString + " t=" + tcapString;
	return sdpString;
}

LINPHONE_END_NAMESPACE
