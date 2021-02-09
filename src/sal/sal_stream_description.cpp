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

#include <algorithm>

#include "linphone/utils/utils.h"
#include "c-wrapper/internal/c-tools.h"
#include "sal/sal_stream_description.h"
#include "utils/payload-type-handler.h"
#include "bellesip_sal/sal_impl.h"

LINPHONE_BEGIN_NAMESPACE

#define keywordcmp(key,b) strncmp(key,b,sizeof(key))

bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type SalStreamDescription::actualConfigurationIndex{ 0 };

bool SalConfigurationCmp::operator()(const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type& lhs, const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type& rhs) const { 
	// Compare the key minus 1 because 0 is the actual configuration hence it has the lowest priority.
	// When iterating over the configuration map, it should be the last element taken into account
	return ((lhs-1) < (rhs-1));
}

SalStreamDescription::SalStreamDescription(){
	// By default, the current index points to the actual configuration
	cfgIndex = SalStreamDescription::actualConfigurationIndex;
	cfgs.clear();
	already_assigned_payloads.clear();
}

SalStreamDescription::~SalStreamDescription(){
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
}

SalStreamDescription::SalStreamDescription(const SalStreamDescription & other){
	name = other.name;
	type = other.type;
	typeother = other.typeother;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	for (const auto & cfg : other.cfgs) {
		cfgs.insert(cfg);
	}
	for (const auto & pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	multicast_role = other.multicast_role;
}

SalStreamDescription::SalStreamDescription(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc) : SalStreamDescription() {
	fillStreamDescription(salMediaDesc, media_desc);
}

SalStreamDescription::SalStreamDescription(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc, const SalStreamDescription::raw_capability_negotiation_attrs_t & attrs) : SalStreamDescription(salMediaDesc, media_desc) {
	// Create potential configurations
	fillPotentialConfigurations(attrs);
}

void SalStreamDescription::fillStreamDescription(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc) {
	belle_sdp_connection_t* cnx;
	belle_sdp_media_t* media;
	belle_sdp_attribute_t* attribute;
	const char* value;
	std::string mtype;

	media=belle_sdp_media_description_get_media ( media_desc );

	if ( ( cnx=belle_sdp_media_description_get_connection ( media_desc ) ) && belle_sdp_connection_get_address ( cnx ) ) {
		rtp_addr = L_C_TO_STRING(belle_sdp_connection_get_address ( cnx ));
	}

	rtp_port=belle_sdp_media_get_media_port ( media );

	mtype = belle_sdp_media_get_media_type ( media );
	// Make mtype lowercase to emulate case insensitive comparison
	std::transform(mtype.begin(), mtype.end(), mtype.begin(), ::tolower);
	if ( mtype.compare( "audio" ) == 0 ) {
		type=SalAudio;
	} else if (mtype.compare( "video" ) == 0 ) {
		type=SalVideo;
	} else if (mtype.compare( "text" ) == 0 ) {
		type=SalText;
	} else {
		type=SalOther;
		typeother = mtype;
	}

	/* Get media specific RTCP attribute */
	rtcp_addr = rtp_addr;
	rtcp_port = rtp_port + 1;

	attribute=belle_sdp_media_description_get_attribute(media_desc,"rtcp");
	if (attribute && (value=belle_sdp_attribute_get_value(attribute))!=NULL){
		char *tmp = (char *)ms_malloc0(strlen(value));
		int nb = sscanf(value, "%d IN IP4 %s", &rtcp_port, tmp);
		if (nb == 1) {
			/* SDP rtcp attribute only contains the port */
		} else if (nb == 2) {
			rtcp_addr = L_C_TO_STRING(tmp);
		} else {
			ms_warning("sdp has a strange a=rtcp line (%s) nb=%i", value, nb);
		}
		ms_free(tmp);
	}

	if ( belle_sdp_media_description_get_bandwidth ( media_desc,"AS" ) >0 ) {
		bandwidth=belle_sdp_media_description_get_bandwidth ( media_desc,"AS" );
	}

	createActualCfg(salMediaDesc, media_desc);
}

void SalStreamDescription::fillStreamDescription(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc, const SalStreamDescription::raw_capability_negotiation_attrs_t & attrs) {

	// Populate stream global parameters and actual configuration
	fillStreamDescription(salMediaDesc, media_desc);

	// Create potential configurations
	fillPotentialConfigurations(attrs);

}

void SalStreamDescription::fillPotentialConfigurations(const SalStreamDescription::raw_capability_negotiation_attrs_t & attrs) {

	for (const auto & acap : attrs.acaps) {
		acaps[acap->index] = std::make_pair(acap->name, acap->value);
	}

	for (const auto & tcap : attrs.tcaps) {
		tcaps[tcap->index] = tcap->value;
	}

	// Iterate over the potential configuration
	for (const auto & SDPMediaDescriptionCfgPair : attrs.cfgs) {
		// This is the configuration ondex
		const auto & idx = SDPMediaDescriptionCfgPair.first;
		const auto potentialCfgList = createPotentialConfiguration(SDPMediaDescriptionCfgPair);
		for (const auto & cfg : potentialCfgList) {
			auto ret = cfgs.insert(std::make_pair(idx, cfg));
			const auto & success = ret.second;
			if (success == false) {
				const auto & cfgPair = ret.first;
				const auto & cfgIndex = cfgPair->first;
				auto & existingCfg = cfgPair->second;
				const auto & existingCfgTcap = existingCfg.getTcapIndex();
				const auto & newCfgTcap = cfg.getTcapIndex();
				lInfo() << "Failed to insert potential configuration " << cfgIndex << " into the configuration map";
				if (existingCfgTcap == newCfgTcap) {
					existingCfg.mergeAcaps(cfg.getAcapIndexes());
					lInfo() << "Merging attribute capabiities with the existing one because both have the same transport protocol " << existingCfgTcap;
				} else {
					lError() << "Unable to merge merging attribute capabiities with the existing configuration as they have different transport protocol indexes - stored configuration " << existingCfgTcap << " new configuration " << newCfgTcap;
				}
			}
		}
	}
}

std::list<SalStreamDescription::cfg_map::mapped_type> SalStreamDescription::createPotentialConfiguration(const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::value_type & SDPMediaDescriptionCfgPair) {

	const auto & idx = SDPMediaDescriptionCfgPair.first;
	const auto & cfgAttributeList = SDPMediaDescriptionCfgPair.second;

	std::map<unsigned int, std::pair<SalMediaProto, std::string>> protoMap;
	for (const auto & tAttr : cfgAttributeList.tcap) {
		const auto & cap = tAttr.cap.lock();
		if (cap) {
			const auto & protoValue = string_to_sal_media_proto(cap->value.c_str());

			std::string protoString = (protoValue == SalProtoOther) ? cap->value : std::string();
			std::transform(protoString.begin(), protoString.end(), protoString.begin(), ::toupper);
			protoMap[cap->index] = std::make_pair(protoValue, protoString);
		} else {
			lError() << "Unable to retrieve transport capability from attribute list";
		}
	}

	std::list<std::multimap<std::string, std::pair<unsigned int, std::string>>> attrList;
	for (const auto & aAttrList : cfgAttributeList.acap) {
		std::multimap<std::string, std::pair<unsigned int, std::string>> attrs;
		for (const auto & aAttr : aAttrList) {
			const auto & cap = aAttr.cap.lock();
			if (cap) {
				const auto & capIdx = cap->index;
				const auto & capValue = cap->value;
				const auto & capName = cap->name;
				attrs.insert({capName, std::make_pair(capIdx, capValue)});
			} else {
				lError() << "Unable to retrieve transport capability from attribute list";
			}
		}
		if (!attrs.empty()) {
			attrList.push_back(attrs);
		}
	}

	std::list<SalStreamDescription::cfg_map::mapped_type> cfgList;
	for (const auto & protoEl : protoMap) {
		const auto & protoIdx = protoEl.first;
		const auto & protoPair = protoEl.second;
		auto baseCfg = getActualConfiguration();
		baseCfg.index = idx;
		baseCfg.delete_media_attributes = cfgAttributeList.delete_media_attributes;
		baseCfg.delete_session_attributes = cfgAttributeList.delete_session_attributes;
		const auto proto = protoPair.first;
		baseCfg.tcapIndex = protoIdx;
		baseCfg.proto = proto;
		baseCfg.proto_other = protoPair.second;
		switch (proto) {
			case SalProtoRtpAvpf:
			case SalProtoRtpSavpf:
			case SalProtoUdpTlsRtpSavpf:
				baseCfg.enableAvpfForStream();
				break;
			default:
				break;
		}

		for (const auto & attrs : attrList) {
			switch (proto) {
				case SalProtoRtpSavpf:
				case SalProtoRtpSavp:
				{
					auto cfg = baseCfg;
					std::list<unsigned int> cfgAcaps;
					const auto cryptoAttrIts = attrs.equal_range("crypto");
					for (auto it=cryptoAttrIts.first; it!=cryptoAttrIts.second; ++it) {
						const auto & capIndexValue = it->second;
						const auto & capIndex = capIndexValue.first;
						cfgAcaps.push_back(capIndex);
						const auto & capValue = capIndexValue.second;
						int tag;
						char name[257]={0}, masterKey[129]={0}, parameters[257]={0};
						const auto nb = sscanf ( capValue.c_str(), "%d %256s inline:%128s %256s",
									&tag,
									name,
									masterKey, parameters );

						if ( nb >= 3 ) {
							MSCryptoSuiteNameParams np;
							np.name=name;
							np.params=parameters[0]!='\0' ? parameters : NULL;
							const auto cs=ms_crypto_suite_build_from_name_params(&np);

							if (cs==MS_CRYPTO_SUITE_INVALID){
								ms_warning ( "Failed to parse crypto-algo: '%s'", name);
							} else {
								SalSrtpCryptoAlgo cryptoEl;
								cryptoEl.tag = tag;
								cryptoEl.master_key = masterKey;
								// Erase all characters after | if it is found
								size_t sep = cryptoEl.master_key.find("|");
								if (sep != std::string::npos) cryptoEl.master_key.erase(cryptoEl.master_key.begin() + static_cast<long>(sep), cryptoEl.master_key.end());
								cryptoEl.algo = cs;
								cfg.crypto.push_back(cryptoEl);
							}
						} else {
							lError() << "Unable to extract cryto infor,ations from crypto argument value " << capValue;
						}
					}
					cfg.acapIndexes.push_back(cfgAcaps);
					cfgList.push_back(cfg);
				}
					break;
				case SalProtoUdpTlsRtpSavp:
				case SalProtoUdpTlsRtpSavpf:
				{
					const auto fingerprintAttrIts = attrs.equal_range("fingerprint");
					for (auto it=fingerprintAttrIts.first; it!=fingerprintAttrIts.second; ++it) {
						const auto & capIndexValue = it->second;
						auto cfg = baseCfg;
						const auto & capIndex = capIndexValue.first;
						std::list<unsigned int> cfgAcaps{capIndex};
						cfg.acapIndexes.push_back(cfgAcaps);
						const auto & capValue = capIndexValue.second;
						cfg.dtls_role = SalDtlsRoleUnset;
						cfg.dtls_fingerprint = capValue;
						cfgList.push_back(cfg);
					}
				}
					break;
				case SalProtoRtpAvpf:
				case SalProtoRtpAvp:
				{
					const auto zrtphashAttrIts = attrs.equal_range("zrtp-hash");
					for (auto it=zrtphashAttrIts.first; it!=zrtphashAttrIts.second; ++it) {
						const auto & capIndexValue = it->second;
						auto cfg = baseCfg;
						const auto & capIndex = capIndexValue.first;
						std::list<unsigned int> cfgAcaps{capIndex};
						cfg.acapIndexes.push_back(cfgAcaps);
						const auto & capValue = capIndexValue.second;
						cfg.haveZrtpHash = true;
						memcpy(cfg.zrtphash, (uint8_t *)capValue.c_str(), sizeof(cfg.zrtphash));
						cfgList.push_back(cfg);
					}
				}
					break;
				default:
					lInfo() << "No acap added to potential configuration with transport protocol " << proto;
					break;
			}
		}
	}

	return cfgList;

}

const SalStreamDescription::cfg_map SalStreamDescription::getAllCfgs() const {
	return cfgs;
}

void SalStreamDescription::setProtoInCfg(SalStreamConfiguration & cfg, const std::string & str) {
	std::string protoOther;
	auto proto=SalProtoOther;
	if ( !str.empty() ) {
		auto protoAsString = str;
		// Make mtype lowercase to emulate case insensitive comparison
		std::transform(protoAsString.begin(), protoAsString.end(), protoAsString.begin(), ::toupper);
		if (protoAsString.compare("RTP/AVP") == 0) {
			proto = SalProtoRtpAvp;
		} else if (protoAsString.compare("RTP/SAVP") == 0) {
			proto = SalProtoRtpSavp;
		} else if (protoAsString.compare("RTP/AVPF") == 0) {
			proto = SalProtoRtpAvpf;
		} else if (protoAsString.compare("RTP/SAVPF") == 0) {
			proto = SalProtoRtpSavpf;
		} else if (protoAsString.compare("UDP/TLS/RTP/SAVP") == 0) {
			proto = SalProtoUdpTlsRtpSavp;
		} else if (protoAsString.compare("UDP/TLS/RTP/SAVPF") == 0) {
			proto = SalProtoUdpTlsRtpSavpf;
		} else {
			proto=SalProtoOther;
			protoOther = protoAsString;
		}
	}
	cfg.proto = proto;
	cfg.proto_other = protoOther;
}

void SalStreamDescription::createActualCfg(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc) {
	belle_sdp_connection_t* cnx;
	belle_sdp_media_t* media;
	belle_sdp_attribute_t* attribute;
	belle_sip_list_t *custom_attribute_it;
	const char* value;

	media=belle_sdp_media_description_get_media ( media_desc );

	SalStreamConfiguration actualCfg;
	actualCfg.custom_sdp_attributes = NULL;

	const std::string protoStr = belle_sdp_media_get_protocol ( media );
	setProtoInCfg(actualCfg, protoStr);
	/* Read DTLS specific attributes : check is some are found in the stream description otherwise copy the session description one(which are at least set to Invalid) */
	if (((actualCfg.proto == SalProtoUdpTlsRtpSavpf) || (actualCfg.proto == SalProtoUdpTlsRtpSavp))) {
		attribute=belle_sdp_media_description_get_attribute(media_desc,"setup");
		if (attribute && (value=belle_sdp_attribute_get_value(attribute))!=NULL){
			if (strncmp(value, "actpass", 7) == 0) {
				actualCfg.dtls_role = SalDtlsRoleUnset;
			} else if (strncmp(value, "active", 6) == 0) {
				actualCfg.dtls_role = SalDtlsRoleIsClient;
			} else if (strncmp(value, "passive", 7) == 0) {
				actualCfg.dtls_role = SalDtlsRoleIsServer;
			}
		}
		if (actualCfg.dtls_role != SalDtlsRoleInvalid && (attribute=belle_sdp_media_description_get_attribute(media_desc,"fingerprint"))) {
			actualCfg.dtls_fingerprint = belle_sdp_attribute_get_value(attribute);
		}
	}

	if ( ( cnx=belle_sdp_media_description_get_connection ( media_desc ) ) && belle_sdp_connection_get_address ( cnx ) ) {
		actualCfg.ttl = belle_sdp_connection_get_ttl(cnx);
	}

	SalStreamDir dir=SalStreamInactive;
	if ( belle_sdp_media_description_get_attribute ( media_desc,"sendrecv" ) ) {
		dir=SalStreamSendRecv;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"sendonly" ) ) {
		dir=SalStreamSendOnly;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"recvonly" ) ) {
		dir=SalStreamRecvOnly;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"inactive" ) ) {
		dir=SalStreamInactive;
	} else {
		dir=salMediaDesc->dir; /*takes default value if not present*/
	}
	actualCfg.dir = dir;

	actualCfg.rtcp_mux = belle_sdp_media_description_get_attribute(media_desc, "rtcp-mux") != NULL;
	actualCfg.bundle_only = belle_sdp_media_description_get_attribute(media_desc, "bundle-only") != NULL;

	attribute = belle_sdp_media_description_get_attribute(media_desc, "mid");
	if (attribute){
		value = belle_sdp_attribute_get_value(attribute);
		if (value)
			actualCfg.mid = L_C_TO_STRING(value);
	}

	actualCfg.payloads.clear();
	/* Get media payload types */
	sdpParsePayloadTypes(actualCfg, media_desc);

	/* Read crypto lines if any */
	if (actualCfg.hasSrtp()) {
		sdpParseMediaCryptoParameters(actualCfg, media_desc);
	}

	/* Read zrtp-hash attribute */
	if ((attribute=belle_sdp_media_description_get_attribute(media_desc,"zrtp-hash"))!=NULL) {
		if ((value=belle_sdp_attribute_get_value(attribute))!=NULL) {
		#if __GNUC__ > 7
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wstringop-truncation"
		#endif
			strncpy((char *)(actualCfg.zrtphash), belle_sdp_attribute_get_value(attribute),sizeof(actualCfg.zrtphash));
			actualCfg.haveZrtpHash = 1;
		#if __GNUC__ > 7
			#pragma GCC diagnostic pop
		#endif
		}
	}

	/* Do we have Lime Ik attribute */
	if ((attribute=belle_sdp_media_description_get_attribute(media_desc,"Ik"))!=NULL) {
		if (belle_sdp_attribute_get_value(attribute)!=NULL) {
			actualCfg.haveLimeIk = 1;
		}
	}
	/* get ready to parse also lime-Ik */
	if ((attribute=belle_sdp_media_description_get_attribute(media_desc,"lime-Ik"))!=NULL) {
		if (belle_sdp_attribute_get_value(attribute)!=NULL) {
			actualCfg.haveLimeIk = 1;
		}
	}

	/* Get ICE candidate attributes if any */
	sdpParseMediaIceParameters(actualCfg, media_desc);

	bool has_avpf_attributes = sdpParseRtcpFbParameters(actualCfg, media_desc);

	/* Get RTCP-FB attributes if any */
	if (actualCfg.hasAvpf()) {
		actualCfg.enableAvpfForStream();
	}else if (has_avpf_attributes ){
		actualCfg.enableAvpfForStream();
		actualCfg.implicit_rtcp_fb = TRUE;
	}

	/* Get RTCP-XR attributes if any */
	actualCfg.rtcp_xr = salMediaDesc->rtcp_xr; // Use session parameters if no stream parameters are defined
	sdp_parse_media_rtcp_xr_parameters(media_desc, &actualCfg.rtcp_xr);

	/* Get the custom attributes, and parse some 'extmap'*/
	for (custom_attribute_it = belle_sdp_media_description_get_attributes(media_desc); custom_attribute_it != NULL; custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		const char *attr_name = belle_sdp_attribute_get_name(attr);
		const char *attr_value = belle_sdp_attribute_get_value(attr);
		actualCfg.custom_sdp_attributes = sal_custom_sdp_attribute_append(actualCfg.custom_sdp_attributes, attr_name, attr_value);

		if (strcasecmp(attr_name, "extmap") == 0){
			char *extmap_urn = (char*)bctbx_malloc0(strlen(attr_value) + 1);
			int rtp_ext_header_id = 0;
			if (sscanf(attr_value, "%i %s", &rtp_ext_header_id, extmap_urn) > 0
				&& strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:sdes:mid") == 0){
				actualCfg.mid_rtp_ext_header_id = rtp_ext_header_id;
			}
			bctbx_free(extmap_urn);
		}
	}

	addActualConfiguration(actualCfg);
}

SalStreamDescription &SalStreamDescription::operator=(const SalStreamDescription & other){
	name = other.name;
	type = other.type;
	typeother = other.typeother;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	for (const auto & cfg : other.cfgs) {
		cfgs.insert(cfg);
	}
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
	for (const auto & pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	multicast_role = other.multicast_role;

	return *this;
}

bool SalStreamDescription::operator==(const SalStreamDescription & other) const {
	return equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED;
}

bool SalStreamDescription::operator!=(const SalStreamDescription & other) const {
	return !(*this == other);
}

int SalStreamDescription::equal(const SalStreamDescription & other) const {
	int result = (getChosenConfiguration().equal(other.getChosenConfiguration()));

	if (type != other.type) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	// RTP
	if (rtp_addr.compare(other.rtp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if ((rtp_addr.empty()==false) && (other.rtp_addr.empty()==false) && ms_is_multicast(L_STRING_TO_C(rtp_addr)) != ms_is_multicast(L_STRING_TO_C(other.rtp_addr)))
			result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (rtp_port != other.rtp_port) {
		if ((rtp_port == 0) || (other.rtp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}

	// RTCP
	if (rtcp_addr.compare(other.rtcp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (rtcp_port != other.rtcp_port) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;

	if (multicast_role != other.multicast_role) result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	return result;
}

bool SalStreamDescription::enabled() const {
	/* When the bundle-only attribute is present, a 0 rtp port doesn't mean that the stream is disabled.*/
	return rtp_port > 0 || isBundleOnly();
}

void SalStreamDescription::disable(){
	rtp_port = 0;
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	cfgs[getChosenConfigurationIndex()].disable();
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamDescription::hasAvpf() const {
	return getChosenConfiguration().hasAvpf();
}

bool SalStreamDescription::hasIpv6() const {
	return rtp_addr.find(':') != std::string::npos;
}

bool SalStreamDescription::hasImplicitAvpf() const {
	return getChosenConfiguration().hasImplicitAvpf();
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamDescription::hasSrtp() const {
	return getChosenConfiguration().hasSrtp();
}

bool SalStreamDescription::hasDtls() const {
	return getChosenConfiguration().hasDtls();
}

bool SalStreamDescription::hasZrtp() const {
	return getChosenConfiguration().hasZrtp();
}

bool SalStreamDescription::hasLimeIk() const {
	return getChosenConfiguration().hasLimeIk();
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

void SalStreamDescription::setProto(const SalMediaProto & newProto) {
	cfgs[getChosenConfigurationIndex()].proto = newProto;
}

const SalMediaProto & SalStreamDescription::getProto() const {
	return getChosenConfiguration().getProto();
}

const std::string SalStreamDescription::getProtoAsString() const {
	return getChosenConfiguration().getProtoAsString();
}

void SalStreamDescription::setDirection(const SalStreamDir & newDir) {
	cfgs[getChosenConfigurationIndex()].dir = newDir;
}

SalStreamDir SalStreamDescription::getDirection() const {
	return getChosenConfiguration().getDirection();
}

const std::list<PayloadType*> & SalStreamDescription::getPayloads() const {
	return getChosenConfiguration().getPayloads();
}

const int & SalStreamDescription::getMaxRate() const {
	return getChosenConfiguration().getMaxRate();
}

SalCustomSdpAttribute * SalStreamDescription::getCustomSdpAttributes() const {
	return getChosenConfiguration().getCustomSdpAttributes();
}

void SalStreamDescription::setPtime(const int & ptime, const int & maxptime) {
	if (ptime > 0) {
		cfgs[getChosenConfigurationIndex()].ptime = ptime;
	}
	if (maxptime > 0) {
		cfgs[getChosenConfigurationIndex()].maxptime = maxptime;
	}
}

const std::vector<SalSrtpCryptoAlgo> & SalStreamDescription::getCryptos() const {
	return getChosenConfiguration().crypto;
}

const SalSrtpCryptoAlgo & SalStreamDescription::getCryptoAtIndex(const size_t & idx) const {
	try {
		const auto & el = getChosenConfiguration().crypto.at(idx);
		return el;
	} catch (std::out_of_range&) {
		lError() << "Unable to crypto at index " << idx << " in the available crypto vector";
		return Utils::getEmptyConstRefObject<SalSrtpCryptoAlgo>();
	}
}

void SalStreamDescription::setCrypto(const size_t & idx, const SalSrtpCryptoAlgo & newCrypto) {
	cfgs[getChosenConfigurationIndex()].crypto[idx] = newCrypto;
}

void SalStreamDescription::setupRtcpFb(const bool nackEnabled, const bool tmmbrEnabled, const bool implicitRtcpFb) {
	for (auto & cfg : cfgs) {
		cfg.second.rtcp_fb.generic_nack_enabled = nackEnabled;
		cfg.second.rtcp_fb.tmmbr_enabled = tmmbrEnabled;
		cfg.second.implicit_rtcp_fb = implicitRtcpFb;
	}
}

void SalStreamDescription::setupRtcpXr(const OrtpRtcpXrConfiguration & rtcpXr) {
	for (auto & cfg : cfgs) {
		memcpy(&cfg.second.rtcp_xr, &rtcpXr, sizeof(cfg.second.rtcp_xr));
	}
}

belle_sdp_media_description_t * SalStreamDescription::toSdpMediaDescription(const SalMediaDescription * salMediaDesc, belle_sdp_session_description_t *session_desc) const {
	belle_sdp_mime_parameter_t* mime_param;
	belle_sdp_media_description_t* media_desc;
	char buffer[1024];
	const char* dirStr=NULL;
	bool_t different_rtp_and_rtcp_addr;
	bool_t stream_enabled = enabled();

	const auto & actualCfg = getActualConfiguration();

	media_desc = belle_sdp_media_description_create ( L_STRING_TO_C(getTypeAsString())
				 ,rtp_port
				 ,1
				 ,L_STRING_TO_C(getProtoAsString())
				 ,NULL );
	if (!actualCfg.payloads.empty()) {
		for (const auto & pt : actualCfg.payloads) {
			mime_param= belle_sdp_mime_parameter_create ( pt->mime_type
					, payload_type_get_number ( pt )
					, pt->clock_rate
					, pt->channels>0 ? pt->channels : -1 );
			belle_sdp_mime_parameter_set_parameters ( mime_param,pt->recv_fmtp );
			if ( actualCfg.ptime>0 ) {
				belle_sdp_mime_parameter_set_ptime ( mime_param,actualCfg.ptime );
			}
			belle_sdp_media_description_append_values_from_mime_parameter ( media_desc,mime_param );
			belle_sip_object_unref ( mime_param );
		}
	} else {
		/* to comply with SDP we cannot have an empty payload type number list */
		/* as it happens only when mline is declined with a zero port, it does not matter to put whatever codec*/
		belle_sip_list_t* format = belle_sip_list_append(NULL,0);
		belle_sdp_media_set_media_formats(belle_sdp_media_description_get_media(media_desc),format);
	}

	/*only add a c= line within the stream description if address are differents*/
	if ((rtp_addr.empty()==false) && (salMediaDesc && (rtp_addr.compare(salMediaDesc->addr)!=0))){
		bool_t inet6;
		belle_sdp_connection_t *connection;
		if (rtp_addr.find(':')!=std::string::npos){
			inet6=TRUE;
		}else inet6=FALSE;
		connection = belle_sdp_connection_create("IN", inet6 ? "IP6" : "IP4", L_STRING_TO_C(rtp_addr));
		if (ms_is_multicast(L_STRING_TO_C(rtp_addr))) {
			/*remove session cline in case of multicast*/
			belle_sdp_session_description_set_connection(session_desc,NULL);
			if (inet6 == FALSE)
				belle_sdp_connection_set_ttl(connection,actualCfg.ttl);
		}
		belle_sdp_media_description_set_connection(media_desc,connection);
	}

	if ( bandwidth>0 )
		belle_sdp_media_description_set_bandwidth ( media_desc,"AS",bandwidth );

	if (actualCfg.hasSrtp()) {
		/* add crypto lines */
		for ( const auto & crypto : actualCfg.crypto ) {
			const auto value = SalStreamConfiguration::cryptoToSdpValue(crypto);
			if (!value.empty()) {
				belle_sdp_media_description_add_attribute( media_desc,belle_sdp_attribute_create ("crypto", value.c_str()));
			}
		}
	}

	/* insert DTLS session attribute if needed */
	if ((actualCfg.proto == SalProtoUdpTlsRtpSavpf) || (actualCfg.proto == SalProtoUdpTlsRtpSavp)) {
		char* ssrc_attribute = ms_strdup_printf("%u cname:%s",actualCfg.rtp_ssrc,L_STRING_TO_C(actualCfg.rtcp_cname));
		if ((actualCfg.dtls_role != SalDtlsRoleInvalid) && (!actualCfg.dtls_fingerprint.empty())) {
			switch(actualCfg.dtls_role) {
				case SalDtlsRoleIsClient:
					belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("setup","active"));
					break;
				case SalDtlsRoleIsServer:
					belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("setup","passive"));
					break;
				case SalDtlsRoleUnset:
				default:
					belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("setup","actpass"));
					break;
			}
			belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("fingerprint",L_STRING_TO_C(actualCfg.dtls_fingerprint)));
		}
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("ssrc",ssrc_attribute));
		ms_free(ssrc_attribute);
	}

	/* insert zrtp-hash attribute if needed */
	if (actualCfg.haveZrtpHash == 1) {
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("zrtp-hash", (const char *)(actualCfg.zrtphash)));
	}

	switch ( actualCfg.dir ) {
		case SalStreamSendRecv:
			/*dirStr="sendrecv";*/
			dirStr=NULL;
			break;
		case SalStreamRecvOnly:
			dirStr="recvonly";
			break;
		case SalStreamSendOnly:
			dirStr="sendonly";
			break;
		case SalStreamInactive:
			dirStr="inactive";
			break;
	}
	if ( dirStr ) belle_sdp_media_description_add_attribute ( media_desc,belle_sdp_attribute_create ( dirStr,NULL ) );

	if (actualCfg.rtcp_mux){
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create ("rtcp-mux",NULL ) );
	}
	addMidAttributesToSdp(actualCfg, media_desc);

	if (rtp_port != 0) {
		different_rtp_and_rtcp_addr = (rtcp_addr.empty() == false) && (rtp_addr.compare(rtcp_addr) != 0);
		if ((rtcp_port != (rtp_port + 1)) || (different_rtp_and_rtcp_addr == TRUE)) {
			if (different_rtp_and_rtcp_addr == TRUE) {
				snprintf(buffer, sizeof(buffer), "%u IN IP4 %s", rtcp_port, rtcp_addr.c_str());
			} else {
				snprintf(buffer, sizeof(buffer), "%u",rtcp_port);
			}
			belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("rtcp",buffer));
		}
	}
	if (actualCfg.set_nortpproxy == TRUE) {
		belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("nortpproxy","yes"));
	}
	if (actualCfg.ice_mismatch == TRUE) {
		belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("ice-mismatch",NULL));
	} else {
		if (rtp_port != 0) {
			if (!actualCfg.ice_pwd.empty())
				belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("ice-pwd",L_STRING_TO_C(actualCfg.ice_pwd)));
			if (!actualCfg.ice_ufrag.empty())
				belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("ice-ufrag",L_STRING_TO_C(actualCfg.ice_ufrag)));
			addIceCandidatesToSdp(actualCfg, media_desc);
			addIceRemoteCandidatesToSdp(actualCfg, media_desc);
		}
	}

	if (stream_enabled && (actualCfg.hasAvpf() || actualCfg.hasImplicitAvpf())) {
		addRtcpFbAttributesToSdp(actualCfg, media_desc);
	}

	if (stream_enabled && (actualCfg.rtcp_xr.enabled == TRUE)) {
		char sastr[1024] = {0};
		char mastr[1024] = {0};
		size_t saoff = 0;
		size_t maoff = 0;
		const belle_sdp_attribute_t *session_attribute = belle_sdp_session_description_get_attribute(session_desc, "rtcp-xr");
		belle_sdp_attribute_t *media_attribute;
		if (session_attribute != NULL) {
			belle_sip_object_marshal((belle_sip_object_t*)session_attribute, sastr, sizeof(sastr), &saoff);
		}
		media_attribute = create_rtcp_xr_attribute(&actualCfg.rtcp_xr);
		if (media_attribute != NULL) {
			belle_sip_object_marshal((belle_sip_object_t*)media_attribute, mastr, sizeof(mastr), &maoff);
		}
		if (strcmp(sastr, mastr) != 0) {
			belle_sdp_media_description_add_attribute(media_desc, media_attribute);
		} else {
			belle_sip_object_unref((belle_sip_object_t*)media_attribute);
		}
	}

	if (actualCfg.custom_sdp_attributes) {
		belle_sdp_session_description_t *custom_desc = (belle_sdp_session_description_t *)actualCfg.custom_sdp_attributes;
		belle_sip_list_t *l = belle_sdp_session_description_get_attributes(custom_desc);
		belle_sip_list_t *elem;
		for (elem = l; elem != NULL; elem = elem->next) {
			belle_sdp_media_description_add_attribute(media_desc, (belle_sdp_attribute_t *)elem->data);
		}
	}

	/*
	 * rfc5576
	 * 4.1.  The "ssrc" Media Attribute
	 * <ssrc-id> is the synchronization source (SSRC) ID of the
	 * source being described, interpreted as a 32-bit unsigned integer in
	 * network byte order and represented in decimal.*/

	for (const auto & acap : acaps) {
		const auto & idx = acap.first;
		const auto & nameValuePair = acap.second;
		const auto & name = nameValuePair.first;
		const auto & value = nameValuePair.second;
		char buffer[1024];
		snprintf ( buffer, sizeof ( buffer )-1, "%d %s:%s", idx, name.c_str(), value.c_str());
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("acap",buffer));
	}

	for (const auto & tcap : tcaps) {
		const auto & idx = tcap.first;
		const auto & value = tcap.second;
		char buffer[1024];
		snprintf ( buffer, sizeof ( buffer )-1, "%d %s", idx, value.c_str());
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("tcap",buffer));
	}

	for (const auto & cfgPair : cfgs) {
		const auto & cfg = cfgPair.second;
		const auto & cfgSdpString = cfg.getSdpString();
		if (!cfgSdpString.empty()) {
			const auto & cfgIdx = cfg.index;
			// Add acfg or pcfg attributes if not the actual configuration or if the actual configuration refer to a potential configuration (member index is not the actual configuration index)
			std::string attrName;
			const auto & cfgKey = cfgPair.first;
			if (cfgKey != getActualConfigurationIndex()) {
				attrName = "pcfg";
			} else if (cfgKey == getActualConfigurationIndex() && (cfgIdx != getActualConfigurationIndex())) {
				attrName = "acfg";
			}
			if (!attrName.empty()) {
				const auto attrValue = std::to_string(cfgIdx) + " " + cfgSdpString;
				belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create(attrName.c_str(),attrValue.c_str()));
			}
		}
	}

	return media_desc;
}

bool SalStreamDescription::sdpParseRtcpFbParameters(SalStreamConfiguration & cfg, const belle_sdp_media_description_t *media_desc) {
	belle_sip_list_t *it;
	belle_sdp_attribute_t *attribute;
	belle_sdp_rtcp_fb_attribute_t *fb_attribute;
	int8_t pt_num;
	bool retval = false;

	/* Handle rtcp-fb attributes that concern all payload types. */
	for (it = belle_sdp_media_description_get_attributes(media_desc); it != NULL; it = it->next) {
		attribute = BELLE_SDP_ATTRIBUTE(it->data);
		if (keywordcmp("rtcp-fb", belle_sdp_attribute_get_name(attribute)) == 0) {
			fb_attribute = BELLE_SDP_RTCP_FB_ATTRIBUTE(attribute);
			if (belle_sdp_rtcp_fb_attribute_get_id(fb_attribute) == -1) {
				for (const auto & pt : cfg.payloads) {
					applyRtcpFbAttributeToPayload(cfg, fb_attribute, pt);
					retval = true;
				}
			}
		}
	}

	/* Handle rtcp-fb attributes that are specefic to a payload type. */
	for (it = belle_sdp_media_description_get_attributes(media_desc); it != NULL; it = it->next) {
		attribute = BELLE_SDP_ATTRIBUTE(it->data);
		if (keywordcmp("rtcp-fb", belle_sdp_attribute_get_name(attribute)) == 0) {
			fb_attribute = BELLE_SDP_RTCP_FB_ATTRIBUTE(attribute);
			pt_num = belle_sdp_rtcp_fb_attribute_get_id(fb_attribute);
			for (const auto & pt : cfg.payloads) {
				retval = true;
				if (payload_type_get_number(pt) == (int)pt_num) {
					applyRtcpFbAttributeToPayload(cfg, fb_attribute, pt);
				}
			}
		}
	}
	return retval;
}

void SalStreamDescription::sdpParsePayloadTypes(SalStreamConfiguration & cfg, const belle_sdp_media_description_t *media_desc) const {
	PayloadType *pt;
	PayloadTypeAvpfParams avpf_params;
	belle_sip_list_t* mime_param_it=NULL;
	belle_sdp_mime_parameter_t* mime_param;
	belle_sip_list_t* mime_params=belle_sdp_media_description_build_mime_parameters ( media_desc );
	memset(&avpf_params, 0, sizeof(avpf_params));
	for ( mime_param_it=mime_params
						; mime_param_it!=NULL
			; mime_param_it=mime_param_it->next ) {
		mime_param=BELLE_SDP_MIME_PARAMETER ( mime_param_it->data );

		pt=payload_type_new();
		payload_type_set_number ( pt,belle_sdp_mime_parameter_get_media_format ( mime_param ) );
		pt->clock_rate=belle_sdp_mime_parameter_get_rate ( mime_param );
		pt->mime_type=ms_strdup ( belle_sdp_mime_parameter_get_type ( mime_param ) );
		pt->channels=belle_sdp_mime_parameter_get_channel_count ( mime_param );
		payload_type_set_send_fmtp ( pt,belle_sdp_mime_parameter_get_parameters ( mime_param ) );
		payload_type_set_avpf_params(pt, avpf_params);
		cfg.payloads.push_back(pt);
		cfg.ptime=belle_sdp_mime_parameter_get_ptime ( mime_param );
		cfg.maxptime=belle_sdp_mime_parameter_get_max_ptime ( mime_param );
		ms_message ( "Found payload %s/%i fmtp=%s",pt->mime_type,pt->clock_rate,
						pt->send_fmtp ? pt->send_fmtp : "" );
	}
	if ( mime_params ) belle_sip_list_free_with_data ( mime_params,belle_sip_object_unref );
}

void SalStreamDescription::sdpParseMediaCryptoParameters(SalStreamConfiguration & cfg, const belle_sdp_media_description_t *media_desc) const {
	belle_sip_list_t *attribute_it;
	belle_sdp_attribute_t *attribute;
	char tmp[257]={0}, tmp2[129]={0}, parameters[257]={0};
	int nb;

	cfg.crypto.clear();
	for ( attribute_it=belle_sdp_media_description_get_attributes ( media_desc )
						; attribute_it!=NULL;
			attribute_it=attribute_it->next ) {
		attribute=BELLE_SDP_ATTRIBUTE ( attribute_it->data );

		if ( keywordcmp ( "crypto",belle_sdp_attribute_get_name ( attribute ) ) ==0 && belle_sdp_attribute_get_value ( attribute ) !=NULL ) {
			SalSrtpCryptoAlgo cryptoEl;
			nb = sscanf ( belle_sdp_attribute_get_value ( attribute ), "%d %256s inline:%128s %256s",
							&cryptoEl.tag,
							tmp,
							tmp2, parameters );

			if ( nb >= 3 ) {
				MSCryptoSuite cs;
				MSCryptoSuiteNameParams np;

				np.name=tmp;
				np.params=parameters[0]!='\0' ? parameters : NULL;
				cs=ms_crypto_suite_build_from_name_params(&np);
				if (cs==MS_CRYPTO_SUITE_INVALID){
					ms_warning ( "Failed to parse crypto-algo: '%s'", tmp );
					cryptoEl.algo = MS_CRYPTO_SUITE_INVALID;
				}else{
					cryptoEl.master_key = tmp2;
					size_t sep = cryptoEl.master_key.find("|");
					// Erase all characters after | if it is found
					if (sep != std::string::npos) cryptoEl.master_key.erase(cryptoEl.master_key.begin() + static_cast<long>(sep), cryptoEl.master_key.end());
					cryptoEl.algo = cs;
					ms_message ( "Found valid crypto line (tag:%d algo:'%s' key:'%s'",
									cryptoEl.tag,
									tmp,
									cryptoEl.master_key.c_str() );
					cfg.crypto.push_back(cryptoEl);
				}

			}else{
				ms_warning ( "sdp has a strange a= line (%s) nb=%i",belle_sdp_attribute_get_value ( attribute ),nb );
			}
		}
	}
	ms_message("Found: %u valid crypto lines", static_cast<unsigned int>(cfg.crypto.size()) );
}

void SalStreamDescription::sdpParseMediaIceParameters(SalStreamConfiguration & cfg, const belle_sdp_media_description_t *media_desc) {
	belle_sip_list_t *attribute_it;
	belle_sdp_attribute_t *attribute;
	const char *att_name;
	const char *value;

	for (attribute_it = belle_sdp_media_description_get_attributes(media_desc); attribute_it != NULL; attribute_it=attribute_it->next) {
		attribute=BELLE_SDP_ATTRIBUTE(attribute_it->data);
		att_name = belle_sdp_attribute_get_name(attribute);
		value = belle_sdp_attribute_get_value(attribute);

		char addr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
		if ((keywordcmp("candidate", att_name) == 0)
				&& (value != NULL)) {
			SalIceCandidate candidate;
			char raddr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
			char foundation[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
			char type[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
			char proto[4];
			int nb = sscanf(value, "%s %u %3s %u %s %d typ %s raddr %s rport %d",
				foundation, &candidate.componentID, proto, &candidate.priority, addr, &candidate.port,
				type, raddr, &candidate.rport);
			candidate.addr = addr;
			candidate.raddr = raddr;
			candidate.foundation = foundation;
			candidate.type = type;
			if (strcasecmp("udp",proto)==0 && ((nb == 7) || (nb == 9))) {
				cfg.ice_candidates.push_back(candidate);
			} else {
				ms_error("ice: Failed parsing a=candidate SDP attribute");
			}
		} else if ((keywordcmp("remote-candidates", att_name) == 0) && (value != NULL)) {
			SalIceRemoteCandidate candidate;
			unsigned int componentID;
			int offset;
			const char *ptr = value;
			const char *endptr = value + strlen(ptr);
			while (3 == sscanf(ptr, "%u %s %u%n", &componentID, addr, &candidate.port, &offset)) {
				candidate.addr = addr;
				if (componentID > 0) {
					SalIceRemoteCandidate remote_candidate;
					remote_candidate.addr = candidate.addr;
					remote_candidate.port = candidate.port;
					const unsigned int candidateIdx = componentID - 1;
					const unsigned int noCandidates = (unsigned int)cfg.ice_remote_candidates.size();
					if (candidateIdx >= noCandidates) {
						cfg.ice_remote_candidates.resize(componentID);
					}
					cfg.ice_remote_candidates[(std::vector<SalIceRemoteCandidate>::size_type)candidateIdx] = remote_candidate;
				}
				ptr += offset;
				if (ptr < endptr) {
					if (*ptr == ' ') ptr += 1;
				} else break;
			}
		} else if ((keywordcmp("ice-ufrag", att_name) == 0) && (value != NULL)) {
			cfg.ice_ufrag = L_C_TO_STRING(value);
		} else if ((keywordcmp("ice-pwd", att_name) == 0) && (value != NULL)) {
			cfg.ice_pwd = L_C_TO_STRING(value);
		} else if (keywordcmp("ice-mismatch", att_name) == 0) {
			cfg.ice_mismatch = TRUE;
		}
	}
}

void SalStreamDescription::applyRtcpFbAttributeToPayload(SalStreamConfiguration & cfg, belle_sdp_rtcp_fb_attribute_t *fb_attribute, PayloadType *pt) {
	PayloadTypeAvpfParams avpf_params = payload_type_get_avpf_params(pt);
	switch (belle_sdp_rtcp_fb_attribute_get_type(fb_attribute)) {
		case BELLE_SDP_RTCP_FB_ACK:
			if (belle_sdp_rtcp_fb_attribute_get_param(fb_attribute) == BELLE_SDP_RTCP_FB_RPSI) {
				avpf_params.features |= PAYLOAD_TYPE_AVPF_RPSI;
			}
			break;
		case BELLE_SDP_RTCP_FB_NACK:
			switch (belle_sdp_rtcp_fb_attribute_get_param(fb_attribute)) {
				case BELLE_SDP_RTCP_FB_PLI:
					avpf_params.features |= PAYLOAD_TYPE_AVPF_PLI;
					break;
				case BELLE_SDP_RTCP_FB_SLI:
					avpf_params.features |= PAYLOAD_TYPE_AVPF_SLI;
					break;
				case BELLE_SDP_RTCP_FB_RPSI:
					/* Linphone uses positive feeback for RPSI. However first versions handling
					 * AVPF wrongly declared RPSI as negative feedback, so this is kept for compatibility
					 * with these versions but will probably be removed at some point in time. */
					avpf_params.features |= PAYLOAD_TYPE_AVPF_RPSI;
					avpf_params.rpsi_compatibility = TRUE;
					break;
				case BELLE_SDP_RTCP_FB_NONE:
					cfg.rtcp_fb.generic_nack_enabled = TRUE;
					break;
				default:
					break;
			}
			break;
		case BELLE_SDP_RTCP_FB_TRR_INT:
			avpf_params.trr_interval = belle_sdp_rtcp_fb_attribute_get_trr_int(fb_attribute);
			break;
		case BELLE_SDP_RTCP_FB_CCM:
			switch (belle_sdp_rtcp_fb_attribute_get_param(fb_attribute)) {
				case BELLE_SDP_RTCP_FB_FIR:
					avpf_params.features |= PAYLOAD_TYPE_AVPF_FIR;
					break;
				case BELLE_SDP_RTCP_FB_TMMBR:
					cfg.rtcp_fb.tmmbr_enabled = TRUE;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	payload_type_set_avpf_params(pt, avpf_params);
}

void SalStreamDescription::addMidAttributesToSdp(const SalStreamConfiguration & cfg, belle_sdp_media_description_t *media_desc) const {
	if (cfg.mid.empty() == false){
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("mid", L_STRING_TO_C(cfg.mid)));
	}
	if (cfg.mid_rtp_ext_header_id){
		char *value = bctbx_strdup_printf("%i urn:ietf:params:rtp-hdrext:sdes:mid", cfg.mid_rtp_ext_header_id);
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("extmap", value));
		bctbx_free(value);
	}
	if (cfg.bundle_only){
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("bundle-only", NULL));
	}
}

bool SalStreamDescription::isRtcpFbTrrIntTheSameForAllPayloads(const SalStreamConfiguration & cfg, uint16_t *trr_int) const {
	bool first = true;
	for (const auto & pt : cfg.payloads) {
		if (payload_type_get_flags(pt) & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED) {
			if (first == true) {
				*trr_int = payload_type_get_avpf_params(pt).trr_interval;
				first = false;
			} else if (payload_type_get_avpf_params(pt).trr_interval != *trr_int) {
				return false;
			}
		}
	}
	return true;
}

void SalStreamDescription::addRtcpFbAttributesToSdp(const SalStreamConfiguration & cfg, belle_sdp_media_description_t *media_desc) const {
	PayloadTypeAvpfParams avpf_params;
	bool general_trr_int;
	uint16_t trr_int = 0;

	general_trr_int = isRtcpFbTrrIntTheSameForAllPayloads(cfg, &trr_int);
	if (general_trr_int == true && trr_int != 0) {
		add_rtcp_fb_trr_int_attribute(media_desc, -1, trr_int);
	}
	if (cfg.rtcp_fb.generic_nack_enabled == TRUE) {
		add_rtcp_fb_nack_attribute(media_desc, -1, BELLE_SDP_RTCP_FB_NONE);
	}
	if (cfg.rtcp_fb.tmmbr_enabled == TRUE) {
		add_rtcp_fb_ccm_attribute(media_desc, -1, BELLE_SDP_RTCP_FB_TMMBR);
	}

	for (const auto & pt : cfg.payloads) {

		/* AVPF/SAVPF profile is used so enable AVPF for all payload types. */
		payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
		avpf_params = payload_type_get_avpf_params(pt);

		/* Add trr-int if not set generally. */
		if (general_trr_int != TRUE && trr_int != 0) {
			add_rtcp_fb_trr_int_attribute(media_desc, (int8_t)payload_type_get_number(pt), avpf_params.trr_interval);
		}

		/* Add rtcp-fb attributes according to the AVPF features of the payload types. */
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_PLI) {
			add_rtcp_fb_nack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_PLI);
		}
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_SLI) {
			add_rtcp_fb_nack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_SLI);
		}
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_RPSI) {
			if (avpf_params.rpsi_compatibility == TRUE) {
				add_rtcp_fb_nack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_RPSI);
			} else {
				add_rtcp_fb_ack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_RPSI);
			}
		}
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_FIR) {
			add_rtcp_fb_ccm_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_FIR);
		}
	}
}

void SalStreamDescription::addIceCandidatesToSdp(const SalStreamConfiguration & cfg, belle_sdp_media_description_t *md) const {
	char buffer[1024];
	int nb;

	for (const auto & candidate : cfg.ice_candidates) {
		if ((candidate.addr.empty()) || (candidate.port == 0)) break;
		nb = snprintf(buffer, sizeof(buffer), "%s %u UDP %u %s %d typ %s",
			candidate.foundation.c_str(), candidate.componentID, candidate.priority, candidate.addr.c_str(), candidate.port, candidate.type.c_str());
		if (nb < 0) {
			ms_error("Cannot add ICE candidate attribute!");
			return;
		}
		if (!candidate.raddr.empty()) {
			nb = snprintf(buffer + nb, sizeof(buffer) - static_cast<size_t>(nb), " raddr %s rport %d", candidate.raddr.c_str(), candidate.rport);
			if (nb < 0) {
				ms_error("Cannot add ICE candidate attribute!");
				return;
			}
		}
		belle_sdp_media_description_add_attribute(md,belle_sdp_attribute_create("candidate",buffer));
	}
}

void SalStreamDescription::addIceRemoteCandidatesToSdp(const SalStreamConfiguration & cfg, belle_sdp_media_description_t *md) const {
	char buffer[1024];
	char *ptr = buffer;
	int offset = 0;

	buffer[0] = '\0';
	for (size_t i = 0; i < cfg.ice_remote_candidates.size(); i++) {
		const auto & candidate = cfg.ice_remote_candidates[i];
		if ((!candidate.addr.empty()) && (candidate.port != 0)) {
			offset = snprintf(ptr, static_cast<size_t>(buffer + sizeof(buffer) - ptr), "%s%u %s %d", (i > 0) ? " " : "", static_cast<unsigned int>(i + 1), candidate.addr.c_str(), candidate.port);
			if (offset < 0) {
				ms_error("Cannot add ICE remote-candidates attribute!");
				return;
			}
			ptr += offset;
		}
	}
	if (buffer[0] != '\0') belle_sdp_media_description_add_attribute(md,belle_sdp_attribute_create("remote-candidates",buffer));
}

const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type & SalStreamDescription::getChosenConfigurationIndex() const {
	return cfgIndex;
}

const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type & SalStreamDescription::getActualConfigurationIndex() const {
	return SalStreamDescription::actualConfigurationIndex;
}

const SalStreamConfiguration & SalStreamDescription::getConfigurationAtIndex(const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type & index) const {
	try {
		const auto & cfg = cfgs.at(index);
		return cfg;
	} catch (std::out_of_range&) {
		lError() << "Unable to configuration at index " << index << " in the available configuration map";
		return Utils::getEmptyConstRefObject<SalStreamConfiguration>();
	}
}
const SalStreamConfiguration & SalStreamDescription::getActualConfiguration() const {
	return getConfigurationAtIndex(getActualConfigurationIndex());
}

const SalStreamConfiguration & SalStreamDescription::getChosenConfiguration() const {
	return getConfigurationAtIndex(getChosenConfigurationIndex());
}

void SalStreamDescription::setZrtpHash(const uint8_t enable, uint8_t* zrtphash) {
	if (enable) {
		memcpy(cfgs[getChosenConfigurationIndex()].zrtphash, zrtphash, sizeof(cfgs[getChosenConfigurationIndex()].zrtphash));
	}
	cfgs[getChosenConfigurationIndex()].haveZrtpHash = enable;
}
void SalStreamDescription::setDtls(const SalDtlsRole role, const std::string & fingerprint) {
	cfgs[getChosenConfigurationIndex()].dtls_role = role;
	cfgs[getChosenConfigurationIndex()].dtls_fingerprint = fingerprint;
}

void SalStreamDescription::setBundleOnly(const bool enable) {
	cfgs[getChosenConfigurationIndex()].bundle_only = enable;
}

bool SalStreamDescription::isBundleOnly() const {
	return getChosenConfiguration().isBundleOnly();
}

void SalStreamDescription::addActualConfiguration(const SalStreamConfiguration & cfg) {
	addConfigurationAtIndex(getActualConfigurationIndex(), cfg);
}

void SalStreamDescription::addConfigurationAtIndex(const bellesip::SDP::SDPPotentialCfgGraph::media_description_config::key_type & idx, const SalStreamConfiguration & cfg) {
	cfgs[idx] = cfg;
}

void SalStreamDescription::addTcap(const unsigned int & idx, const std::string & value) {
	tcaps[idx] = value;
}

const std::string & SalStreamDescription::getTcap(const unsigned int & idx) const {
	try {
		return tcaps.at(idx);
	} catch (std::out_of_range&) {
		lError() << "Unable to find transport capability at index " << idx;
		return Utils::getEmptyConstRefObject<std::string>();
	}
}

void SalStreamDescription::addAcap(const unsigned int & idx, const std::string & name, const std::string & value) {
	acaps[idx] = std::make_pair(name, value);
}

const std::pair<std::string, std::string> & SalStreamDescription::getAcap(const unsigned int & idx) const {
	try {
		return acaps.at(idx);
	} catch (std::out_of_range&) {
		lError() << "Unable to find attribute capability at index " << idx;
		return Utils::getEmptyConstRefObject<std::pair<std::string, std::string>>();
	}
}


LINPHONE_END_NAMESPACE
