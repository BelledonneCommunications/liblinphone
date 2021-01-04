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

#include "c-wrapper/internal/c-tools.h"
#include "sal/sal_media_description.h"
#include "c-wrapper/internal/c-sal-stream-description.h"
#include "c-wrapper/internal/c-sal-stream-bundle.h"

SalMediaDescription::SalMediaDescription(){
	init();
}

SalMediaDescription::~SalMediaDescription(){
	destroy();
}

SalMediaDescription::SalMediaDescription(const SalMediaDescription & other){
	refcount = other.refcount;
	name = other.name;
	username = other.username;
	addr = other.addr;

	bandwidth = other.bandwidth;
	session_ver = other.session_ver;
	session_id = other.session_id;

	dir = other.dir;
	streams = other.streams;
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);
	rtcp_xr = other.rtcp_xr;

	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	ice_lite = other.ice_lite;

	accept_bundles = other.accept_bundles;
	bundles = bctbx_list_copy_with_data(other.bundles, (bctbx_list_copy_func)sal_stream_bundle_clone);

	pad = other.pad;
	set_nortpproxy = other.set_nortpproxy;

}

void SalMediaDescription::init() {
	refcount=1;
	streams.clear();
	pad.clear();
	custom_sdp_attributes = nullptr;
	bundles = nullptr;
}

bool SalMediaDescription::hasDir(const SalStreamDir & stream_dir) const {
	if (stream_dir==SalStreamRecvOnly){
		return containsStreamWithDir(SalStreamRecvOnly) && !(containsStreamWithDir(SalStreamSendOnly) || containsStreamWithDir(SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendOnly){
		return containsStreamWithDir(SalStreamSendOnly) && !(containsStreamWithDir(SalStreamRecvOnly) || containsStreamWithDir(SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendRecv){
		return containsStreamWithDir(SalStreamSendRecv);
	}else{
		/*SalStreamInactive*/
		if (containsStreamWithDir(SalStreamSendOnly) || containsStreamWithDir(SalStreamSendRecv)  || containsStreamWithDir(SalStreamRecvOnly))
			return FALSE;
		else return TRUE;
	}
	return FALSE;
}

bool SalMediaDescription::containsStreamWithDir(const SalStreamDir & stream_dir) const{
	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for(auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getDirection()==stream_dir) {
			return TRUE;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (stream.getDirection()==SalStreamSendRecv && stream_dir==SalStreamSendOnly && (isNullAddress(addr) || isNullAddress(stream.rtp_addr))){
			return TRUE;
		}
	}
	return FALSE;
}

bool SalMediaDescription::isNullAddress(const std::string & addr) const {
	return addr.compare("0.0.0.0")==0 || addr.compare("::0")==0;
}

SalStreamBundle * SalMediaDescription::addNewBundle(){
	SalStreamBundle *bundle = sal_stream_bundle_new();
	bundles = bctbx_list_append(bundles, bundle);
	return bundle;
}

int SalMediaDescription::lookupMid(const std::string mid) const {
	size_t index;
	for (index = 0 ; index < streams.size(); ++index){
		const auto & sd = streams[index];
		if (sd.mid.compare(mid) == 0){
			return static_cast<int>(index);
		}
	}
	return -1;
}

const SalStreamBundle *SalMediaDescription::getBundleFromMid(const std::string mid) const {
	const bctbx_list_t *elem;
	for (elem = bundles; elem != NULL; elem = elem->next){
		SalStreamBundle *bundle = (SalStreamBundle *)elem->data;
		if (sal_stream_bundle_has_mid(bundle, L_STRING_TO_C(mid))) return bundle;
	}
	return nullptr;
}

int SalMediaDescription::getIndexOfTransportOwner(const SalStreamDescription *sd) const {
	const SalStreamBundle *bundle;
	std::string master_mid;
	int index;
	if (sd->mid.empty() == true) return -1; /* not part of any bundle */
	/* lookup the mid in the bundle descriptions */
	bundle = getBundleFromMid(sd->mid);
	if (!bundle) {
		ms_warning("Orphan stream with mid '%s'", L_STRING_TO_C(sd->mid));
		return -1;
	}
	master_mid = sal_stream_bundle_get_mid_of_transport_owner(bundle);
	index = lookupMid(master_mid);
	if (index == -1){
		ms_error("Stream with mid '%s' has no transport owner (mid '%s') !", L_STRING_TO_C(sd->mid), L_STRING_TO_C(master_mid));
	}
	return index;
}

void SalMediaDescription::destroy(){
	streams.clear();
	bctbx_list_free_with_data(bundles, (void (*)(void*)) sal_stream_bundle_destroy);
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

SalMediaDescription * SalMediaDescription::ref(){
	refcount++;
	return this;
}

void SalMediaDescription::unref(){
	refcount--;
	if (refcount==0){
		destroy ();
	}
}

const SalStreamDescription * SalMediaDescription::findStream(SalMediaProto proto, SalStreamType type) const {
	for(auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.proto==proto && stream.getType()==type) return &stream;
	}
	return nullptr;
}

unsigned int SalMediaDescription::nbActiveStreamsOfType(SalStreamType type) const {
	unsigned int nb = 0;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getType() == type) nb++;
	}
	return nb;
}

const SalStreamDescription * SalMediaDescription::getActiveStreamOfType(SalStreamType type, unsigned int idx) const {
	for(auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getType() == type) {
			if (idx-- == 0) return &stream;
		}
	}
	return nullptr;
}

const SalStreamDescription * SalMediaDescription::findSecureStreamOfType(SalStreamType type) const {
	const SalStreamDescription *desc = findStream(SalProtoRtpSavpf, type);
	if (desc == nullptr) desc = findStream(SalProtoRtpSavp, type);
	return desc;
}

const SalStreamDescription * SalMediaDescription::findBestStream(SalStreamType type) const {
	const SalStreamDescription *desc = findStream(SalProtoUdpTlsRtpSavpf, type);
	if (desc == nullptr) desc = findStream(SalProtoUdpTlsRtpSavp, type);
	if (desc == nullptr) desc = findStream(SalProtoRtpSavpf, type);
	if (desc == nullptr) desc = findStream(SalProtoRtpSavp, type);
	if (desc == nullptr) desc = findStream(SalProtoRtpAvpf, type);
	if (desc == nullptr) desc = findStream(SalProtoRtpAvp, type);
	return desc;
}

bool_t SalMediaDescription::isEmpty() const {
	if (getNbActiveStreams() > 0) return FALSE;
	return TRUE;
}

void SalMediaDescription::setDir(SalStreamDir stream_dir){
	for(auto & stream : streams){
		if (!stream.enabled()) continue;
		stream.dir=stream_dir;
	}
}

int SalMediaDescription::getNbActiveStreams() const {
	int nb = 0;
	for(auto & stream : streams){
		if (stream.enabled()) nb++;
	}
	return nb;
}

bool_t SalMediaDescription::hasAvpf() const {
	if (streams.empty()) return FALSE;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasAvpf() != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t SalMediaDescription::hasImplicitAvpf() const {
	if (streams.empty()) return FALSE;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasImplicitAvpf() != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t SalMediaDescription::hasSrtp() const {
	if (streams.empty()) return FALSE;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasSrtp()) return TRUE;
	}
	return FALSE;
}

bool_t SalMediaDescription::hasDtls() const {
	if (streams.empty()) return FALSE;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasDtls() != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t SalMediaDescription::hasZrtp() const {
	if (streams.empty()) return FALSE;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasZrtp() != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t SalMediaDescription::hasIpv6() const {
	if (streams.empty()) return FALSE;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getRtpAddress().empty() == false){
			if (!stream.hasIpv6()) return FALSE;
		}else{
			if (addr.find(':') == std::string::npos) return FALSE;
		}
	}
	return TRUE;
}

bool SalMediaDescription::operator==(const SalMediaDescription & other) const {
	return (equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED);
}

int SalMediaDescription::equal(const SalMediaDescription & otherMd) const {
	int result = globalEqual(otherMd);
	for(auto stream1 = streams.cbegin(), stream2 = otherMd.streams.cbegin(); (stream1 != streams.cend() && stream2 != otherMd.streams.cend()); ++stream1, ++stream2){
		if (!stream1->enabled() && !stream2->enabled()) continue;
		result |= stream1->equal(*stream2);
	}
	return result;
}

int SalMediaDescription::globalEqual(const SalMediaDescription & otherMd) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	if (addr.compare(otherMd.addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (addr.empty() == false && otherMd.addr.empty() == false && ms_is_multicast(L_STRING_TO_C(addr)) != ms_is_multicast(L_STRING_TO_C(otherMd.addr)))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (streams.size() != otherMd.streams.size()) result |= SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	if (bandwidth != otherMd.bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (ice_ufrag.compare(otherMd.ice_ufrag) != 0 && !otherMd.ice_ufrag.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (ice_pwd.compare(otherMd.ice_pwd) != 0 && !otherMd.ice_pwd.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;

	return result;
}

const std::string SalMediaDescription::printDifferences(int result) {
	std::string out = std::string();
	if (result & SAL_MEDIA_DESCRIPTION_CODEC_CHANGED){
		out.append("CODEC_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED){
		out.append("NETWORK_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED){
		out.append("ICE_RESTART_DETECTED ");
		result &= ~SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED){
		out.append("CRYPTO_KEYS_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED){
		out.append("NETWORK_XXXCAST_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED){
		out.append("STREAMS_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED){
		out.append("CRYPTO_POLICY_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION){
		out.append("FORCE_STREAM_RECONSTRUCTION ");
		result &= ~SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	}
	if (result){
		ms_fatal("There are unhandled result bitmasks in SalMediaDescription::print_differences(), fix it");
	}
	if (out.empty()) out = "NONE";
	return out;
}

size_t SalMediaDescription::getNbStreams() const {
	return streams.size();
}

const std::string & SalMediaDescription::getAddress() const {
	return addr;
}

const SalStreamDescription * SalMediaDescription::getStreamIdx(unsigned int idx) const {
	if (idx < streams.size()) {
		return &(streams[idx]);
	}
	return nullptr;
}
