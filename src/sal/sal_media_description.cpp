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

#include <algorithm>

#include "c-wrapper/internal/c-tools.h"
#include "sal/sal_media_description.h"

LINPHONE_BEGIN_NAMESPACE

SalMediaDescription::SalMediaDescription(){
	init();
}

SalMediaDescription::~SalMediaDescription(){
	destroy();
}

SalMediaDescription::SalMediaDescription(const SalMediaDescription & other){
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
	bundles = other.bundles;

	pad = other.pad;
	set_nortpproxy = other.set_nortpproxy;

}

void SalMediaDescription::init() {
	streams.clear();
	bundles.clear();
	pad.clear();
	custom_sdp_attributes = nullptr;
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
			return false;
		else return true;
	}
	return false;
}

bool SalMediaDescription::containsStreamWithDir(const SalStreamDir & stream_dir) const{
	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for(auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getDirection()==stream_dir) {
			return true;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (stream.getDirection()==SalStreamSendRecv && stream_dir==SalStreamSendOnly && (isNullAddress(addr) || isNullAddress(stream.rtp_addr))){
			return true;
		}
	}
	return false;
}

bool SalMediaDescription::isNullAddress(const std::string & addr) const {
	return addr.compare("0.0.0.0")==0 || addr.compare("::0")==0;
}

void SalMediaDescription::addNewBundle(const SalStreamBundle & bundle){
	bundles.push_back(bundle);
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

const std::list<SalStreamBundle>::const_iterator SalMediaDescription::getBundleFromMid(const std::string mid) const {
	const auto & bundleIt = std::find_if(bundles.cbegin(), bundles.cend(), [&mid] (const auto & bundle) {
		return (bundle.hasMid(mid));
	});
	return bundleIt;
}

int SalMediaDescription::getIndexOfTransportOwner(const SalStreamDescription & sd) const {
	std::string master_mid;
	int index;
	if (sd.mid.empty() == true) return -1; /* not part of any bundle */
	/* lookup the mid in the bundle descriptions */
	const auto &bundle = getBundleFromMid(sd.mid);
	if (bundle == bundles.cend()) {
		ms_warning("Orphan stream with mid '%s'", L_STRING_TO_C(sd.mid));
		return -1;
	}
	master_mid = bundle->getMidOfTransportOwner();
	index = lookupMid(master_mid);
	if (index == -1){
		ms_error("Stream with mid '%s' has no transport owner (mid '%s') !", L_STRING_TO_C(sd.mid), L_STRING_TO_C(master_mid));
	}
	return index;
}

void SalMediaDescription::destroy(){
	streams.clear();
	bundles.clear();
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

const std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findStream(SalMediaProto proto, SalStreamType type) const {
	return std::find_if(streams.cbegin(), streams.cend(), [&type, &proto] (const auto & stream) { 
		return (stream.enabled() && (stream.proto==proto) && (stream.getType()==type));
	});
}

unsigned int SalMediaDescription::nbActiveStreamsOfType(SalStreamType type) const {
	unsigned int nb = 0;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getType() == type) nb++;
	}
	return nb;
}

const std::vector<SalStreamDescription>::const_iterator SalMediaDescription::getActiveStreamOfType(SalStreamType type, unsigned int idx) const {
	return std::find_if(streams.cbegin(), streams.cend(), [&type, &idx] (const auto & stream) { 
		return (stream.enabled() && (idx-- == 0) && (stream.getType()==type));
	});
}

const std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findSecureStreamOfType(SalStreamType type) const {
	std::vector<SalStreamDescription>::const_iterator desc = findStream(SalProtoRtpSavpf, type);
	if (desc == streams.cend()) desc = findStream(SalProtoRtpSavp, type);
	return desc;
}

const std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findBestStream(SalStreamType type) const {
	std::vector<SalStreamDescription>::const_iterator desc = findStream(SalProtoUdpTlsRtpSavpf, type);
	if (desc == streams.cend()) desc = findStream(SalProtoUdpTlsRtpSavp, type);
	if (desc == streams.cend()) desc = findStream(SalProtoRtpSavpf, type);
	if (desc == streams.cend()) desc = findStream(SalProtoRtpSavp, type);
	if (desc == streams.cend()) desc = findStream(SalProtoRtpAvpf, type);
	if (desc == streams.cend()) desc = findStream(SalProtoRtpAvp, type);
	return desc;
}

bool SalMediaDescription::isEmpty() const {
	if (getNbActiveStreams() > 0) return false;
	return true;
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

bool SalMediaDescription::hasAvpf() const {
	if (streams.empty()) return false;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasAvpf() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasImplicitAvpf() const {
	if (streams.empty()) return false;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasImplicitAvpf() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasSrtp() const {
	if (streams.empty()) return false;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasSrtp()) return true;
	}
	return false;
}

bool SalMediaDescription::hasDtls() const {
	if (streams.empty()) return false;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasDtls() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasZrtp() const {
	if (streams.empty()) return false;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.hasZrtp() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasIpv6() const {
	if (streams.empty()) return false;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getRtpAddress().empty() == false){
			if (!stream.hasIpv6()) return false;
		}else{
			if (addr.find(':') == std::string::npos) return false;
		}
	}
	return true;
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

const std::vector<SalStreamDescription>::const_iterator SalMediaDescription::getStreamIdx(unsigned int idx) const {
	if (idx < streams.size()) {
		return streams.cbegin() + idx;
	}
	return streams.cend();
}

LINPHONE_END_NAMESPACE
