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
#include "sal/sal_stream_description.h"
#include "sal/sal_stream_bundle.h"

LINPHONE_BEGIN_NAMESPACE

// Called by makeLocalMediaDescription to create the local media decription
SalMediaDescription::SalMediaDescription(const bool capabilityNegotiation, const bool mergeTcaps){
	capabilityNegotiationSupported = capabilityNegotiation;
	mergeTcapLines = mergeTcaps;

	streams.clear();
	bundles.clear();
	custom_sdp_attributes = nullptr;
}

SalMediaDescription::~SalMediaDescription(){
	streams.clear();
	bundles.clear();
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

SalMediaDescription::SalMediaDescription(const SalMediaDescription & other) {
	name = other.name;
	username = other.username;
	addr = other.addr;

	bandwidth = other.bandwidth;
	session_ver = other.session_ver;
	session_id = other.session_id;

	dir = other.dir;
	streams = other.streams;
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);
	rtcp_xr = other.rtcp_xr;

	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	ice_lite = other.ice_lite;

	accept_bundles = other.accept_bundles;
	bundles = other.bundles;

	set_nortpproxy = other.set_nortpproxy;

	capabilityNegotiationSupported = other.capabilityNegotiationSupported;
	mergeTcapLines = other.mergeTcapLines;
}

SalMediaDescription::SalMediaDescription(belle_sdp_session_description_t  *sdp) : SalMediaDescription(false, false) {
	belle_sdp_connection_t* cnx;
	belle_sdp_session_name_t *sname;
	belle_sip_list_t *custom_attribute_it;
	const char* value;

	dir = SalStreamSendRecv;

	PotentialCfgGraph potentialCfgGraph(sdp);

	// if received SDP has no valid capability negotiation attributes, then assume that it doesn't support capability negotiation
	capabilityNegotiationSupported = !potentialCfgGraph.empty();

	if ( ( cnx=belle_sdp_session_description_get_connection ( sdp ) ) && belle_sdp_connection_get_address ( cnx ) ) {
		addr = belle_sdp_connection_get_address ( cnx );
	}
	if ( (sname=belle_sdp_session_description_get_session_name(sdp)) && belle_sdp_session_name_get_value(sname) ){
		name = belle_sdp_session_name_get_value(sname);
	}

	if ( belle_sdp_session_description_get_bandwidth ( sdp,"AS" ) >0 ) {
		bandwidth=belle_sdp_session_description_get_bandwidth ( sdp,"AS" );
	}

	belle_sdp_origin_t *origin = belle_sdp_session_description_get_origin(sdp);
	session_id = belle_sdp_origin_get_session_id(origin);
	session_ver = belle_sdp_origin_get_session_version(origin);

	/*in some very rare case, session attribute may set stream dir*/
	if ( belle_sdp_session_description_get_attribute ( sdp,"sendrecv" ) ) {
		dir=SalStreamSendRecv;
	} else if ( belle_sdp_session_description_get_attribute ( sdp,"sendonly" ) ) {
		dir=SalStreamSendOnly;
	} else if ( belle_sdp_session_description_get_attribute ( sdp,"recvonly" ) ) {
		dir=SalStreamRecvOnly;
	} else if ( belle_sdp_session_description_get_attribute ( sdp,"inactive" ) ) {
		dir=SalStreamInactive;
	}

	/* Get ICE remote ufrag and remote pwd, and ice_lite flag */
	value=belle_sdp_session_description_get_attribute_value(sdp,"ice-ufrag");
	if (value) ice_ufrag = L_C_TO_STRING(value);

	value=belle_sdp_session_description_get_attribute_value(sdp,"ice-pwd");
	if (value) ice_pwd = L_C_TO_STRING(value);

	value=belle_sdp_session_description_get_attribute_value(sdp,"ice-lite");
	if (value) ice_lite = true;

	/* Get session RTCP-XR attributes if any */
	sdp_parse_session_rtcp_xr_parameters(sdp, &rtcp_xr);

	/* Do we have Lime Ik attribute */
	value=belle_sdp_session_description_get_attribute_value(sdp,"Ik");
	if (value) haveLimeIk = true;

	/* get ready to parse also lime-Ik */
	value=belle_sdp_session_description_get_attribute_value(sdp,"lime-Ik");
	if (value) haveLimeIk = true;

	/* Get the custom attributes, parse some of them that are relevant */
	for (custom_attribute_it = belle_sdp_session_description_get_attributes(sdp); custom_attribute_it != NULL; custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		custom_sdp_attributes = sal_custom_sdp_attribute_append(custom_sdp_attributes, belle_sdp_attribute_get_name(attr), belle_sdp_attribute_get_value(attr));

		if (strcasecmp(belle_sdp_attribute_get_name(attr), "group") == 0){
			value = belle_sdp_attribute_get_value(attr);
			if (value && strncasecmp(value, "BUNDLE", strlen("BUNDLE")) == 0){
				SalStreamBundle bundle(value + strlen("BUNDLE"));
				addNewBundle(bundle);
			}
		}
	}

	// Initialize currentStreamIdx to the size of vector streams as streams build from SDP media descriptions are appended.
	// Generally, at this point, vector streams should be empty

	if (capabilityNegotiationSupported) {
		for (const auto & acap : potentialCfgGraph.getGlobalAcap()) {
			acaps[acap->index] = std::make_pair(acap->name, acap->value);
		}

		for (const auto & tcap : potentialCfgGraph.getGlobalTcap()) {
			tcaps[tcap->index] = tcap->value;
		}
	}

	unsigned int currentStreamIdx = static_cast<unsigned int>(streams.size());
	for ( belle_sip_list_t* media_desc_it=belle_sdp_session_description_get_media_descriptions ( sdp ); media_desc_it!=NULL; media_desc_it=media_desc_it->next ) {
		belle_sdp_media_description_t* media_desc=BELLE_SDP_MEDIA_DESCRIPTION ( media_desc_it->data );

		SalStreamDescription stream;
		if (capabilityNegotiationSupported) {
			SalStreamDescription::raw_capability_negotiation_attrs_t attrs;
			attrs.unparsed_cfgs =  potentialCfgGraph.getUnparsedCfgForStream(currentStreamIdx);
			attrs.cfgs =  potentialCfgGraph.getCfgForStream(currentStreamIdx);
			attrs.acaps = potentialCfgGraph.getMediaAcapForStream(currentStreamIdx);
			attrs.tcaps = potentialCfgGraph.getMediaTcapForStream(currentStreamIdx);
			stream.fillStreamDescriptionFromSdp(this, sdp, media_desc, attrs);
		} else {
			stream.fillStreamDescriptionFromSdp(this, sdp, media_desc);
		}
		streams.push_back(stream);
		currentStreamIdx++;
	}
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
		if (sd.getChosenConfiguration().getMid().compare(mid) == 0){
			return static_cast<int>(index);
		}
	}
	return -1;
}

const SalStreamBundle & SalMediaDescription::getBundleFromMid(const std::string mid) const {
	const auto & bundleIt = std::find_if(bundles.cbegin(), bundles.cend(), [&mid] (const auto & bundle) {
		return (bundle.hasMid(mid));
	});
	if (bundleIt != bundles.cend()) {
		return *bundleIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamBundle>();
}

int SalMediaDescription::getIndexOfTransportOwner(const SalStreamDescription & sd) const {
	std::string master_mid;
	int index;
	if (sd.getChosenConfiguration().getMid().empty() == true) return -1; /* not part of any bundle */
	/* lookup the mid in the bundle descriptions */
	const auto &bundle = getBundleFromMid(sd.getChosenConfiguration().getMid());
	if (bundle == Utils::getEmptyConstRefObject<SalStreamBundle>()) {
		ms_warning("Orphan stream with mid '%s'", L_STRING_TO_C(sd.getChosenConfiguration().getMid()));
		return -1;
	}
	master_mid = bundle.getMidOfTransportOwner();
	index = lookupMid(master_mid);
	if (index == -1){
		ms_error("Stream with mid '%s' has no transport owner (mid '%s') !", L_STRING_TO_C(sd.getChosenConfiguration().getMid()), L_STRING_TO_C(master_mid));
	}
	return index;
}

const SalStreamDescription & SalMediaDescription::findStream(SalMediaProto proto, SalStreamType type) const {
	const auto & streamIt = std::find_if(streams.cbegin(), streams.cend(), [&type, &proto] (const auto & stream) { 
		return (stream.enabled() && (stream.getProto()==proto) && (stream.getType()==type));
	});
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

unsigned int SalMediaDescription::nbActiveStreamsOfType(SalStreamType type) const {
	unsigned int nb = 0;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		if (stream.getType() == type) nb++;
	}
	return nb;
}

const SalStreamDescription & SalMediaDescription::getActiveStreamOfType(SalStreamType type, unsigned int idx) const {
	const auto & streamIt = std::find_if(streams.cbegin(), streams.cend(), [&type, &idx] (const auto & stream) { 
		return (stream.enabled() && (idx-- == 0) && (stream.getType()==type));
	});
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

const SalStreamDescription SalMediaDescription::findSecureStreamOfType(SalStreamType type) const {
	auto stream = findStream(SalProtoRtpSavpf, type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) stream = findStream(SalProtoRtpSavp, type);
	return stream;
}

const SalStreamDescription SalMediaDescription::findBestStream(SalStreamType type) const {
	auto stream = findStream(SalProtoUdpTlsRtpSavpf, type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) stream = findStream(SalProtoUdpTlsRtpSavp, type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) stream = findStream(SalProtoRtpSavpf, type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) stream = findStream(SalProtoRtpSavp, type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) stream = findStream(SalProtoRtpAvpf, type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) stream = findStream(SalProtoRtpAvp, type);
	return stream;
}

bool SalMediaDescription::isEmpty() const {
	if (getNbActiveStreams() > 0) return false;
	return true;
}

void SalMediaDescription::setDir(SalStreamDir stream_dir){
	for(auto & stream : streams){
		if (!stream.enabled()) continue;
		stream.setDirection(stream_dir);
	}
}

int SalMediaDescription::getNbActiveStreams() const {
	int nb = 0;
	for(auto & stream : streams){
		if (stream.enabled()) nb++;
	}
	return nb;
}

bool SalMediaDescription::hasIceParams() const {
	bool foundIceMediaDescParams = (!ice_ufrag.empty() && !ice_pwd.empty());
	bool foundIceCandidates = true;
	bool foundIceStreamDescParams = true;
	for(const auto & stream : streams){
		if (!stream.enabled()) continue;
		foundIceCandidates &= stream.hasIceCandidates();
		foundIceStreamDescParams &= stream.hasIceParams();
	}
	// Return true if ice pwd and ufrag is in media description and each stream has candidates or if each stream defines ice pwd and ufrag as well as candidates
	return (foundIceStreamDescParams || (foundIceMediaDescParams && foundIceCandidates));
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

bool SalMediaDescription::hasLimeIk() const {
	return haveLimeIk;
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

bool SalMediaDescription::supportCapabilityNegotiation() const {
	return capabilityNegotiationSupported;
}

bool SalMediaDescription::tcapLinesMerged() const {
	return mergeTcapLines;
}

bool SalMediaDescription::operator==(const SalMediaDescription & other) const {
	return (equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED);
}

bool SalMediaDescription::operator!=(const SalMediaDescription & other) const {
	return !(*this == other);
}

int SalMediaDescription::compareToActualConfiguration(const SalMediaDescription & otherMd) const {
	int result = globalEqual(otherMd);
	for(auto stream1 = streams.cbegin(), stream2 = otherMd.streams.cbegin(); (stream1 != streams.cend() && stream2 != otherMd.streams.cend()); ++stream1, ++stream2){
		if (!stream1->enabled() && !stream2->enabled()) continue;
		result |= stream1->compareToActualConfiguration(*stream2);
	}
	return result;
}

int SalMediaDescription::compareToChosenConfiguration(const SalMediaDescription & otherMd) const {
	int result = globalEqual(otherMd);
	for(auto stream1 = streams.cbegin(), stream2 = otherMd.streams.cbegin(); (stream1 != streams.cend() && stream2 != otherMd.streams.cend()); ++stream1, ++stream2){
		if (!stream1->enabled() && !stream2->enabled()) continue;
		result |= stream1->compareToChosenConfiguration(*stream2);
	}
	return result;
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
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED) {
		out.append("CRYPTO_TYPE_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED){
		out.append("CRYPTO_POLICY_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED){
		out.append("STREAMS_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION){
		out.append("FORCE_STREAM_RECONSTRUCTION ");
		result &= ~SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CONFIGURATION_CHANGED) {
		out.append("STREAM_CONFIGURATION_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CONFIGURATION_CHANGED;
	}
	if (result){
		ms_fatal("There are unhandled result bitmasks in SalMediaDescription::printDifferences(), fix it");
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

const SalStreamDescription & SalMediaDescription::getStreamIdx(unsigned int idx) const {
	if (idx < streams.size()) {
		return streams[idx];
	}
	lError() << "Unable to find stream at index " << idx << " because media description " << this << " has " << streams.size() << " streams";
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

belle_sdp_session_description_t * SalMediaDescription::toSdp() const {
	belle_sdp_session_description_t* session_desc=belle_sdp_session_description_new();
	bool_t inet6;
	belle_sdp_origin_t* origin;
	char *escaped_username = NULL;
	if (!username.empty()) {
		escaped_username = belle_sip_uri_to_escaped_username(L_STRING_TO_C(username));
	}

	if ( addr.find(':' ) != std::string::npos ) {
		inet6=1;
	} else inet6=0;
	belle_sdp_session_description_set_version ( session_desc,belle_sdp_version_create ( 0 ) );

	origin = belle_sdp_origin_create ( escaped_username
									  ,session_id
									  ,session_ver
									  ,"IN"
									  , inet6 ? "IP6" :"IP4"
									  ,L_STRING_TO_C(addr) );
	if (escaped_username) {
		bctbx_free(escaped_username);
	}

	belle_sdp_session_description_set_origin ( session_desc,origin );

	belle_sdp_session_description_set_session_name ( session_desc,
		belle_sdp_session_name_create ( name.empty()==false ? L_STRING_TO_C(name) : "Talk" ) );

	if ( !hasDir(SalStreamInactive) || !ice_ufrag.empty() ) {
		/*in case of sendonly, setting of the IP on cnx we give a chance to receive stun packets*/
		belle_sdp_session_description_set_connection ( session_desc
				,belle_sdp_connection_create ( "IN",inet6 ? "IP6" :"IP4",L_STRING_TO_C(addr) ) );

	} else 	{
		belle_sdp_session_description_set_connection ( session_desc
				,belle_sdp_connection_create ( "IN"
								,inet6 ? "IP6" :"IP4"
								,inet6 ? "::0" :"0.0.0.0" ) );

	}

	belle_sdp_session_description_set_time_description ( session_desc,belle_sdp_time_description_create ( 0,0 ) );

	if ( bandwidth>0 ) {
		belle_sdp_session_description_set_bandwidth ( session_desc,"AS",bandwidth );
	}

	if (set_nortpproxy == true) belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("nortpproxy","yes"));
	if (!ice_pwd.empty()) belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("ice-pwd",L_STRING_TO_C(ice_pwd)));
	if (!ice_ufrag.empty()) belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("ice-ufrag",L_STRING_TO_C(ice_ufrag)));

	if (rtcp_xr.enabled == TRUE) {
		belle_sdp_session_description_add_attribute(session_desc, create_rtcp_xr_attribute(&rtcp_xr));
	}

	for (const auto & bundle : bundles){
		bundle.addToSdp(session_desc);
	}

	if (custom_sdp_attributes) {
		belle_sdp_session_description_t *custom_desc = (belle_sdp_session_description_t *)custom_sdp_attributes;
		belle_sip_list_t *l = belle_sdp_session_description_get_attributes(custom_desc);
		belle_sip_list_t *elem;
		for (elem = l; elem != NULL; elem = elem->next) {
			belle_sdp_session_description_add_attribute(session_desc, (belle_sdp_attribute_t *)elem->data);
		}
	}

	if (supportCapabilityNegotiation()) {
		for (const auto & acap : acaps) {
			const auto & idx = acap.first;
			const auto & nameValuePair = acap.second;
			const auto & name = nameValuePair.first;
			const auto & value = nameValuePair.second;

			std::string acapValue = std::to_string(idx) + " " + name + ":" + value;
			belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("acap",acapValue.c_str()));
		}

		std::string tcapValue;
		SalStreamDescription::tcap_map_t::key_type prevIdx = 0;
		for (const auto & tcap : tcaps) {
			const auto & idx = tcap.first;
			const auto & value = tcap.second;
			if (mergeTcapLines) {
				if (tcapValue.empty()) {
					tcapValue = std::to_string(idx) + " " + value;
					prevIdx = idx;
				} else {
					if (idx == (prevIdx + 1)) {
						tcapValue += " " + value;
					} else {
						belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("tcap",tcapValue.c_str()));
						tcapValue = std::to_string(idx) + " " + value;
					}
					prevIdx = idx;
				}
			} else {
				tcapValue = std::to_string(idx) + " " + value;
				belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("tcap",tcapValue.c_str()));
			}
		}

		if (mergeTcapLines && !tcapValue.empty()) {
			belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("tcap",tcapValue.c_str()));
		}
	}

	for ( const auto & stream : streams) {
		auto media_desc = stream.toSdpMediaDescription(this, session_desc);
		belle_sdp_session_description_add_media_description(session_desc, media_desc);
	}
	return session_desc;
}

void SalMediaDescription::addTcap(const unsigned int & idx, const std::string & value) {
	tcaps[idx] = value;
}

const std::string & SalMediaDescription::getTcap(const unsigned int & idx) const {
	try {
		return tcaps.at(idx);
	} catch (std::out_of_range&) {
		lError() << "Unable to find transport capability at index " << idx;
		return Utils::getEmptyConstRefObject<std::string>();
	}
}

void SalMediaDescription::addAcap(const unsigned int & idx, const std::string & name, const std::string & value) {
	acaps[idx] = std::make_pair(name, value);
}

const SalStreamDescription::acap_t & SalMediaDescription::getAcap(const unsigned int & idx) const {
	try {
		return acaps.at(idx);
	} catch (std::out_of_range&) {
		lError() << "Unable to find attribute capability at index " << idx;
		return Utils::getEmptyConstRefObject<SalStreamDescription::acap_t>();
	}
}

const SalStreamDescription::acap_map_t & SalMediaDescription::getAcaps() const {
	return acaps;
}

const SalStreamDescription::tcap_map_t & SalMediaDescription::getTcaps() const {
	return tcaps;
}
const SalStreamDescription::cfg_map SalMediaDescription::getCfgsForStream(const unsigned int & idx) const {
	SalStreamDescription::cfg_map cfgs;
	const SalStreamDescription & stream = getStreamIdx(idx);
	if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		cfgs = stream.getAllCfgs();
	}
	return cfgs;
}

const SalStreamDescription::acap_map_t SalMediaDescription::getAllAcapForStream(const unsigned int & idx) const {
	SalStreamDescription::acap_map_t allAcaps;
	const SalStreamDescription & stream = getStreamIdx(idx);
	if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		allAcaps = stream.getAcaps();
		auto globalAcaps = getAcaps();
		allAcaps.insert(globalAcaps.begin(), globalAcaps.end());
	}
	return allAcaps;
}
const SalStreamDescription::tcap_map_t SalMediaDescription::getAllTcapForStream(const unsigned int & idx) const {
	SalStreamDescription::tcap_map_t allTcaps;
	const SalStreamDescription & stream = getStreamIdx(idx);
	if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		allTcaps = stream.getTcaps();
		auto globalTcaps = getTcaps();
		allTcaps.insert(globalTcaps.begin(), globalTcaps.end());
	}
	return allTcaps;
}

void SalMediaDescription::addTcapToStream(const std::size_t & streamIdx, const unsigned int & idx, const std::string & value) {
	if (streamIdx < streams.size()) {
		streams[streamIdx].addTcap(idx, value);
	}
}

void SalMediaDescription::addAcapToStream(const std::size_t & streamIdx, const unsigned int & idx, const std::string & name, const std::string & value) {
	if (streamIdx < streams.size()) {
		streams[streamIdx].addAcap(idx, name, value);
	}
}

void SalMediaDescription::createPotentialConfigurationsForStream(const unsigned int & streamIdx, const bool delete_session_attributes, const bool delete_media_attributes) {

	try {
		SalStreamDescription & stream = streams.at(streamIdx);
		const auto allStreamAcaps = getAllAcapForStream(streamIdx);
		const auto allStreamTcaps = getAllTcapForStream(streamIdx);
		if (!allStreamAcaps.empty() || !allStreamTcaps.empty()) {
			if (allStreamTcaps.empty()) {
				const SalStreamDescription::tcap_map_t proto;
				stream.createPotentialConfiguration(proto, {allStreamAcaps}, delete_session_attributes, delete_media_attributes);
			} else {
				for (const auto & protoPair : allStreamTcaps) {
					const SalStreamDescription::tcap_map_t proto{{protoPair}};
					stream.createPotentialConfiguration(proto, {allStreamAcaps}, delete_session_attributes, delete_media_attributes);
				}
			}
		} else {
			lInfo() << "Unable to create potential configuration for stream " << streamIdx << " because it doesn't have acap and tcap attributes";
		}
	} catch (std::out_of_range&) {
		lError() << "Unable to create potential configuration for stream " << streamIdx << " because it doesn't exists";
	}
}


unsigned int SalMediaDescription::getFreeTcapIdx() const {
	std::list<unsigned int> tcapIndexes;
	auto addToIndexList = [&tcapIndexes] (const auto & cap) {
		tcapIndexes.push_back(cap.first);
	};
	const auto & globalTcaps = getTcaps();
	std::for_each(globalTcaps.begin(), globalTcaps.end(), addToIndexList);
	for (const auto & stream : streams) {
		const auto & streamTcaps = stream.getTcaps();
		std::for_each(streamTcaps.begin(), streamTcaps.end(), addToIndexList);
	}

	return PotentialCfgGraph::getFreeIdx(tcapIndexes);
}

unsigned int SalMediaDescription::getFreeAcapIdx() const {
	std::list<unsigned int> acapIndexes;
	auto addToIndexList = [&acapIndexes] (const auto & cap) {
		acapIndexes.push_back(cap.first);
	};
	const auto & globalAcaps = getAcaps();
	std::for_each(globalAcaps.begin(), globalAcaps.end(), addToIndexList);
	for (const auto & stream : streams) {
		const auto & streamAcaps = stream.getAcaps();
		std::for_each(streamAcaps.begin(), streamAcaps.end(), addToIndexList);
	}

	return PotentialCfgGraph::getFreeIdx(acapIndexes);
}

void SalMediaDescription::addPotentialConfigurationToSdp(belle_sdp_media_description_t * & media_desc, const std::string attrName, const PotentialCfgGraph::media_description_config::value_type & cfg) const {
	const auto & cfgIdx = cfg.first;
	const auto & cfgAttr = cfg.second;

	// Sets of optional configuration
	const auto & cfgAcapSets = cfgAttr.acap;
	std::string acapString;
	// Iterate over all acaps sets. For every set, get the index of all its members
	for (const auto & acapSet : cfgAcapSets) {
		// Do not append | on first element
		if (!acapString.empty()) {
			acapString.append("|");
		}
		for (const auto & cfgAcap : acapSet) {
			const auto & firstAcap = acapSet.front().cap.lock();
			const auto & firstAcapIdx = firstAcap->index;
			const auto & acap = cfgAcap.cap.lock();
			const auto & acapIdx = acap->index;
			if (acapIdx != firstAcapIdx) {
				acapString.append(",");
			}
			if (acap) {
				acapString.append(std::to_string(acapIdx));
			}
		}
	}

	std::string tcapString;
	const auto & cfgTcaps = cfgAttr.tcap;
	for (const auto & cfgTcap : cfgTcaps) {
		// Do not append | on first element
		if (!tcapString.empty()) {
			tcapString.append("|");
		}
		const auto tcap = cfgTcap.cap.lock();
		if (tcap) {
			tcapString.append(std::to_string(tcap->index));
		}
	}

	std::string deleteAttrs;
	if (cfgAttr.delete_media_attributes && cfgAttr.delete_session_attributes) {
		deleteAttrs = "-ms:";
	} else if (cfgAttr.delete_session_attributes) {
		deleteAttrs = "-s:";
	} else if (cfgAttr.delete_media_attributes) {
		deleteAttrs = "-m:";
	}

	char buffer[1024];
	snprintf ( buffer, sizeof ( buffer )-1, "%d a=%s%s t=%s", cfgIdx, deleteAttrs.c_str(), acapString.c_str(), tcapString.c_str());
	belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create(attrName.c_str(),buffer));
	capabilityNegotiationSupported = true;
}

LINPHONE_END_NAMESPACE
