/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include <algorithm>

#include "bellesip_sal/sal_impl.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/misc.h"
#include "linphone/utils/utils.h"
#include "sal/sal_media_description.h"
#include "sal/sal_stream_description.h"
#include "utils/payload-type-handler.h"

LINPHONE_BEGIN_NAMESPACE

#define keywordcmp(key, b) strncmp(key, b, sizeof(key))

PotentialCfgGraph::media_description_config::key_type SalStreamDescription::actualConfigurationIndex{0};

bool SalConfigurationCmp::operator()(const PotentialCfgGraph::media_description_config::key_type &lhs,
                                     const PotentialCfgGraph::media_description_config::key_type &rhs) const {
	// Compare the key minus 1 because 0 is the actual configuration hence it has the lowest priority.
	// When iterating over the configuration map, it should be the last element taken into account
	return ((lhs - 1) < (rhs - 1));
}

SalStreamDescription::SalStreamDescription() {
	// By default, the current index points to the actual configuration
	cfgIndex = SalStreamDescription::actualConfigurationIndex;
	cfgs.clear();
	unparsed_cfgs.clear();
	already_assigned_payloads.clear();
	custom_sdp_attributes = NULL;

	ice_candidates.clear();
	ice_remote_candidates.clear();
}

SalStreamDescription::~SalStreamDescription() {
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

SalStreamDescription::SalStreamDescription(const SalStreamDescription &other) {
	name = other.name;
	type = other.type;
	typeother = other.typeother;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	acaps = other.acaps;
	tcaps = other.tcaps;
	for (const auto &cfg : other.cfgs) {
		const auto result = cfgs.insert(cfg);
		if (!result.second) cfgs[cfg.first] = cfg.second;
	}
	for (const auto &cfg : other.unparsed_cfgs) {
		const auto result = unparsed_cfgs.insert(cfg);
		if (!result.second) unparsed_cfgs[cfg.first] = cfg.second;
	}
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
	for (const auto &pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	multicast_role = other.multicast_role;

	ice_candidates = other.ice_candidates;
	ice_remote_candidates = other.ice_remote_candidates;
	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	ice_mismatch = other.ice_mismatch;

	label = other.label;
	content = other.content;

	supportedEncryption = other.supportedEncryption;

	sal_custom_sdp_attribute_free(custom_sdp_attributes);
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);

	cfgIndex = other.cfgIndex;
}

SalStreamDescription::SalStreamDescription(const SalMediaDescription *salMediaDesc,
                                           const belle_sdp_session_description_t *sdp,
                                           const belle_sdp_media_description_t *media_desc)
    : SalStreamDescription() {
	fillStreamDescriptionFromSdp(salMediaDesc, sdp, media_desc);
}

SalStreamDescription::SalStreamDescription(const SalMediaDescription *salMediaDesc,
                                           const belle_sdp_session_description_t *sdp,
                                           const belle_sdp_media_description_t *media_desc,
                                           const SalStreamDescription::raw_capability_negotiation_attrs_t &attrs)
    : SalStreamDescription(salMediaDesc, sdp, media_desc) {
	// Create potential configurations
	fillPotentialConfigurationsFromPotentialCfgGraph(attrs.cfgs);
}

void SalStreamDescription::fillStreamDescriptionFromSdp(const SalMediaDescription *salMediaDesc,
                                                        const belle_sdp_session_description_t *sdp,
                                                        const belle_sdp_media_description_t *media_desc) {
	belle_sdp_connection_t *cnx;
	belle_sdp_media_t *media;
	belle_sdp_attribute_t *attribute;
	const char *value;
	std::string mtype;

	media = belle_sdp_media_description_get_media(media_desc);

	if ((cnx = belle_sdp_media_description_get_connection(media_desc)) && belle_sdp_connection_get_address(cnx)) {
		rtp_addr = L_C_TO_STRING(belle_sdp_connection_get_address(cnx));
	}

	rtp_port = belle_sdp_media_get_media_port(media);
	mtype = belle_sdp_media_get_media_type(media);
	// Make mtype lowercase to emulate case insensitive comparison
	std::transform(mtype.begin(), mtype.end(), mtype.begin(), ::tolower);
	if (mtype.compare("audio") == 0) {
		type = SalAudio;
	} else if (mtype.compare("video") == 0) {
		type = SalVideo;
	} else if (mtype.compare("text") == 0) {
		type = SalText;
	} else {
		type = SalOther;
		typeother = mtype;
	}

	if (belle_sdp_media_description_get_bandwidth(media_desc, "AS") > 0) {
		bandwidth = belle_sdp_media_description_get_bandwidth(media_desc, "AS");
	}

	attribute = belle_sdp_media_description_get_attribute(media_desc, "label");
	if (attribute && (value = belle_sdp_attribute_get_value(attribute)) != NULL) {
		label = value;
	}

	attribute = belle_sdp_media_description_get_attribute(media_desc, "content");
	if (attribute && (value = belle_sdp_attribute_get_value(attribute)) != NULL) {
		content = value;
	}

	createActualCfg(salMediaDesc, sdp, media_desc);

	/* Get media specific RTCP attribute */
	rtcp_addr = rtp_addr;
	// Set here the RTCP port because we must know if rtcp_mux is enabled or not
	if (getActualConfiguration().rtcp_mux) {
		rtcp_port = rtp_port;
	} else {
		rtcp_port = rtp_port + 1;
	}
	attribute = belle_sdp_media_description_get_attribute(media_desc, "rtcp");
	if (attribute && (value = belle_sdp_attribute_get_value(attribute)) != NULL) {
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
}

void SalStreamDescription::fillStreamDescriptionFromSdp(
    const SalMediaDescription *salMediaDesc,
    const belle_sdp_session_description_t *sdp,
    const belle_sdp_media_description_t *media_desc,
    const SalStreamDescription::raw_capability_negotiation_attrs_t &attrs) {

	// Populate stream global parameters and actual configuration
	fillStreamDescriptionFromSdp(salMediaDesc, sdp, media_desc);

	if (salMediaDesc->getParams().capabilityNegotiationSupported()) {

		for (const auto &acap : attrs.acaps) {
			acaps[acap->index] = std::make_pair(acap->name, acap->value);
		}

		for (const auto &tcap : attrs.tcaps) {
			tcaps[tcap->index] = tcap->value;
		}

		for (const auto &cfg : attrs.unparsed_cfgs) {
			unparsed_cfgs.insert(cfg);
		}
		// Create potential configurations
		fillPotentialConfigurationsFromPotentialCfgGraph(attrs.cfgs);
	}
}

void SalStreamDescription::fillPotentialConfigurationsFromPotentialCfgGraph(
    const PotentialCfgGraph::media_description_config &sdpCfgs) {

	// Iterate over the potential configuration
	for (const auto &SDPMediaDescriptionCfgPair : sdpCfgs) {
		// This is the configuration ondex
		const auto &idx = SDPMediaDescriptionCfgPair.first;

		const auto &cfgAttributeList = SDPMediaDescriptionCfgPair.second;

		tcap_map_t protoMap;
		for (const auto &tAttr : cfgAttributeList.tcap) {
			const auto &cap = tAttr.cap.lock();
			if (cap) {
				protoMap[cap->index] = cap->value;
			} else {
				lError() << "Unable to retrieve transport capability from attribute list";
			}
		}

		std::list<acap_map_t> attrList;
		for (const auto &aAttrList : cfgAttributeList.acap) {
			acap_map_t attrs;
			for (const auto &aAttr : aAttrList) {
				const auto &cap = aAttr.cap.lock();
				if (cap) {
					const auto &capIdx = cap->index;
					const auto &capValue = cap->value;
					const auto &capName = cap->name;
					attrs.insert({capIdx, std::make_pair(capName, capValue)});
				} else {
					lError() << "Unable to retrieve transport capability from attribute list";
				}
			}
			if (!attrs.empty()) {
				attrList.push_back(attrs);
			}
		}

		addSupportedEncryptionFromSdp(protoMap, attrList);
		createPotentialConfigurationAtIdx(idx, protoMap, attrList, cfgAttributeList.delete_session_attributes,
		                                  cfgAttributeList.delete_media_attributes);
	}
}

const SalStreamDescription::tcap_map_t::value_type &SalStreamDescription::encryptionToTcap(
    const SalStreamDescription::tcap_map_t &caps, const LinphoneMediaEncryption encEnum, const bool avpf) {
	const auto &it = std::find_if(caps.cbegin(), caps.cend(), [&avpf, &encEnum](const auto &cap) {
		return (cap.second.compare(sal_media_proto_to_string(
		            linphone_media_encryption_to_sal_media_proto(encEnum, ((avpf) ? true : false)))) == 0);
	});
	if (it != caps.end()) {
		return *it;
	}

	return Utils::getEmptyConstRefObject<SalStreamDescription::tcap_map_t::value_type>();
}

void SalStreamDescription::addSupportedEncryptionFromSdp(const SalStreamDescription::tcap_map_t &protoMap,
                                                         const std::list<SalStreamDescription::acap_map_t> &attrList) {

	bool haveZrtpHash = false;
	for (const auto &attrs : attrList) {
		const auto zrtpHashIt = std::find_if(attrs.cbegin(), attrs.cend(), [](const auto &attr) {
			// attr is a map<unsigned int (attr index), std::pair<std::string (name), std::string (value)>>
			return (attr.second.first.compare("zrtp-hash") == 0);
		});

		haveZrtpHash |= (zrtpHashIt != attrs.cend());
	}

	LinphoneMediaEncryption enc = LinphoneMediaEncryptionNone;
	for (const auto &tAttr : protoMap) {
		auto protoStr = tAttr.second;
		enc = sal_media_proto_to_linphone_media_encryption(sal_media_proto_from_string(protoStr.c_str()),
		                                                   (haveZrtpHash ? true : false));
		supportedEncryption.push_front(enc);
		lInfo() << "Adding encryption " << linphone_media_encryption_to_string(enc) << " to stream " << this;
	}

	// Delete duplicates
	supportedEncryption.unique();
}

const std::list<LinphoneMediaEncryption> SalStreamDescription::getSupportedEncryptionsInPotentialCfgs() const {
	// Last element of the list is the encryption of the actual configuration
	return {supportedEncryption.cbegin(), std::prev(supportedEncryption.cend(), 1)};
}

void SalStreamDescription::createPotentialConfigurationAtIdx(
    const unsigned int &idx,
    const SalStreamDescription::tcap_map_t &tcaps,
    const std::list<SalStreamDescription::acap_map_t> &attrList,
    const bool delete_session_attributes,
    const bool delete_media_attributes) {

	auto baseCfg = createBasePotentialCfg();

	baseCfg.delete_media_attributes = delete_media_attributes;
	baseCfg.delete_session_attributes = delete_session_attributes;

	for (const auto &protoEl : tcaps) {
		const auto &protoIdx = protoEl.first;
		const auto &protoValue = protoEl.second;
		const auto proto = sal_media_proto_from_string(protoValue.c_str());
		baseCfg.tcapIndex = protoIdx;
		baseCfg.proto = proto;
		std::string protoString = (proto == SalProtoOther) ? protoValue : std::string();
		std::transform(protoString.begin(), protoString.end(), protoString.begin(), ::toupper);
		baseCfg.proto_other = protoString;
		if (baseCfg.hasAvpf()) {
			baseCfg.enableAvpfForStream();
		} else {
			baseCfg.disableAvpfForStream();
		}

		bool haveZrtpHash = false;
		for (const auto &attrs : attrList) {
			const auto zrtpHashIt = std::find_if(attrs.cbegin(), attrs.cend(), [](const auto &attr) {
				// attr is a map<unsigned int (attr index), std::pair<std::string (name), std::string (value)>>
				return (attr.second.first.compare("zrtp-hash") == 0);
			});

			haveZrtpHash |= (zrtpHashIt != attrs.cend());
		}
		const LinphoneMediaEncryption enc = sal_media_proto_to_linphone_media_encryption(proto, haveZrtpHash);
		auto cfgList = addAcapsToConfiguration(baseCfg, enc, attrList, true);
		auto cfg = cfgList.front();

		cfg.index = idx;
		insertOrMergeConfiguration(idx, cfg);
	}
}

void SalStreamDescription::createPotentialConfiguration(const SalStreamDescription::tcap_map_t &protoMap,
                                                        const std::list<SalStreamDescription::acap_map_t> &attrList,
                                                        const bool delete_session_attributes,
                                                        const bool delete_media_attributes,
                                                        bool mergeCfgLines) {

	auto baseCfg = createBasePotentialCfg();

	baseCfg.delete_media_attributes = delete_media_attributes;
	baseCfg.delete_session_attributes = delete_session_attributes;

	std::list<SalStreamDescription::cfg_map::mapped_type> cfgList;
	if (protoMap.empty()) {
		for (const auto &attrs : attrList) {
			auto cfg = baseCfg;
			std::list<unsigned int> cfgAcaps;
			for (const auto &attr : attrs) {
				const auto &capIndex = attr.first;
				cfgAcaps.push_back(capIndex);
			}
			cfg.acapIndexes.push_back(cfgAcaps);
			cfgList.push_back(cfg);
		}
	} else {
		const auto supportedEncs = getSupportedEncryptionsInPotentialCfgs();
		unsigned int idx = getFreeCfgIdx();
		for (const auto avpf : {true, false}) {
			for (const auto &enc : supportedEncs) {
				const auto &protoEl = SalStreamDescription::encryptionToTcap(protoMap, enc, avpf);
				if (protoEl != Utils::getEmptyConstRefObject<SalStreamDescription::tcap_map_t::value_type>()) {
					const auto &protoIdx = protoEl.first;
					const auto &protoValue = protoEl.second;
					const auto proto = sal_media_proto_from_string(protoValue.c_str());
					baseCfg.tcapIndex = protoIdx;
					baseCfg.proto = proto;
					std::string protoString = (proto == SalProtoOther) ? protoValue : std::string();
					std::transform(protoString.begin(), protoString.end(), protoString.begin(), ::toupper);
					baseCfg.proto_other = protoString;
					switch (proto) {
						case SalProtoRtpAvpf:
						case SalProtoRtpSavpf:
						case SalProtoUdpTlsRtpSavpf:
							baseCfg.enableAvpfForStream();
							break;
						default:
							baseCfg.disableAvpfForStream();
							break;
					}
					auto cfgListForEnc = addAcapsToConfiguration(baseCfg, enc, attrList, mergeCfgLines);
					for (auto cfg : cfgListForEnc) {
						cfg.index = idx;
						cfgList.push_back(cfg);
						idx++;
					}
				}
			}
		}
	}

	for (auto &cfg : cfgList) {
		insertOrMergeConfiguration(cfg.index, cfg);
	}
}

SalStreamConfiguration SalStreamDescription::createBasePotentialCfg() const {
	auto baseCfg = getActualConfiguration();
	// Clear encryption flags
	// ZRTP
	baseCfg.haveZrtpHash = 0;
	memset(baseCfg.zrtphash, 0, sizeof(baseCfg.zrtphash));
	// SRTP
	baseCfg.crypto.clear();
	baseCfg.acapIndexes.clear();
	// DTLS
	baseCfg.dtls_fingerprint.clear();
	baseCfg.dtls_role = SalDtlsRoleInvalid;

	return baseCfg;
}

void SalStreamDescription::insertOrMergeConfiguration(const unsigned &idx, const SalStreamConfiguration &cfg) {
	const auto sameCfg = std::find_if(cfgs.cbegin(), cfgs.cend(), [&cfg, this](const auto &currentCfg) {
		// Only potential configurations should be parsed - it is allowed to add a potential configuration identical to
		// the actual one
		return ((currentCfg.first != getActualConfigurationIndex()) && (currentCfg.second == cfg));
	});

	if (sameCfg == cfgs.cend()) {
		auto ret = cfgs.insert(std::make_pair(idx, cfg));
		const auto &success = ret.second;
		if (success == false) {
			const auto &cfgPair = ret.first;
			const auto &cfgIndex = cfgPair->first;
			auto &existingCfg = cfgPair->second;
			const auto &existingCfgTcap = existingCfg.getTcapIndex();
			const auto &newCfgTcap = cfg.getTcapIndex();
			lInfo() << "Failed to insert potential configuration " << cfgIndex << " into the configuration map";
			if (existingCfgTcap == newCfgTcap) {
				existingCfg.mergeAcaps(cfg.getAcapIndexes());
				lInfo() << "Merging attribute capabiities with the existing one because both have the same transport "
				           "protocol "
				        << existingCfgTcap;
			} else {
				lError() << "Unable to merge merging attribute capabiities with the existing configuration as they "
				            "have different transport protocol indexes - stored configuration "
				         << existingCfgTcap << " new configuration " << newCfgTcap;
			}
		}
	}
}

std::list<SalStreamConfiguration>
SalStreamDescription::addAcapsToConfiguration(const SalStreamConfiguration &baseCfg,
                                              const LinphoneMediaEncryption &enc,
                                              const std::list<SalStreamDescription::acap_map_t> &attrList,
                                              bool mergeCfgLines) const {

	std::list<SalStreamConfiguration> cfgList;
	auto cfg = baseCfg;

	for (const auto &attrs : attrList) {
		if (enc == LinphoneMediaEncryptionSRTP) {
			for (const auto &attr : attrs) {
				std::list<unsigned int> cfgAcaps;
				const auto &capNameValue = attr.second;
				const auto &capName = capNameValue.first;
				// For SRTP, only crypto attributes are valid in configurations
				if (capName.compare("crypto") == 0) {
					const auto &capIndex = attr.first;
					cfgAcaps.push_back(capIndex);
					cfg.acapIndexes.push_back(cfgAcaps);

					const auto &capValue = capNameValue.second;
					const auto keyEnc = SalStreamConfiguration::fillStrpCryptoAlgoFromString(capValue);
					if (keyEnc.algo != MS_CRYPTO_SUITE_INVALID) {
						cfg.crypto.push_back(keyEnc);
						if (!mergeCfgLines) {
							cfgList.push_back(cfg);
							cfg = baseCfg;
						}
					}
				}
			}
		} else if (enc == LinphoneMediaEncryptionDTLS) {
			std::list<unsigned int> cfgAcaps;
			for (const auto &attr : attrs) {
				const auto &capIndex = attr.first;
				const auto &capNameValue = attr.second;
				const auto &capName = capNameValue.first;
				const auto &capValue = capNameValue.second;
				if (capName.compare("fingerprint") == 0) {
					cfgAcaps.push_back(capIndex);
					cfg.dtls_fingerprint = capValue;
				} else if (capName.compare("setup") == 0) {
					cfg.dtls_role = SalStreamConfiguration::getDtlsRoleFromSetupAttribute(capValue);
					cfgAcaps.push_back(capIndex);
				} else if (capName.compare("rtcp-mux") == 0) {
					cfg.rtcp_mux = true;
					cfgAcaps.push_back(capIndex);
				} else if (capName.compare("ssrc") == 0) {
					unsigned int rtpSsrc;
					char rtcpName[256] = {0};
					const auto nb = sscanf(capValue.c_str(), "%u cname:%s", &rtpSsrc, rtcpName);
					if (nb == 2) {
						cfgAcaps.push_back(capIndex);
						cfg.rtp_ssrc = rtpSsrc;
						cfg.rtcp_cname = rtcpName;
					} else {
						lError() << "Unable to retrieve rtp ssrc and rtcp cname from atribute " << capValue;
					}
				}
			}
			cfg.acapIndexes.push_back(cfgAcaps);
			if (!mergeCfgLines && !cfg.dtls_fingerprint.empty() && (cfg.dtls_role != SalDtlsRoleInvalid) &&
			    (cfg.rtp_ssrc != 0)) {
				cfgList.push_back(cfg);
				cfg = baseCfg;
			}
		} else if (enc == LinphoneMediaEncryptionZRTP) {
			for (const auto &attr : attrs) {
				const auto &capNameValue = attr.second;
				const auto &capName = capNameValue.first;
				std::list<unsigned int> cfgAcaps;
				if (capName.compare("zrtp-hash") == 0) {
					const auto &capIndex = attr.first;
					cfgAcaps.push_back(capIndex);
					cfg.acapIndexes.push_back(cfgAcaps);

					cfg.haveZrtpHash = true;
					const auto &capValue = capNameValue.second;
					strncpy((char *)cfg.zrtphash, capValue.c_str(), capValue.size());

					cfgList.push_back(cfg);
					cfg = baseCfg;
				}
			}
		} else if (enc == LinphoneMediaEncryptionNone) {
			lInfo() << "No acap to add to potential configuration for encryption "
			        << linphone_media_encryption_to_string(enc);
		} else {
			lInfo() << "Adding acaps to potential configuration for encryption "
			        << linphone_media_encryption_to_string(enc);
			std::list<unsigned int> cfgAcaps;
			for (const auto &attr : attrs) {
				const auto &capIndex = attr.first;
				cfgAcaps.push_back(capIndex);
			}
			cfg.acapIndexes.push_back(cfgAcaps);

			if (!mergeCfgLines) {
				cfgList.push_back(cfg);
				cfg = baseCfg;
			}
		}
	}

	if (cfgList.empty()) {
		cfgList.push_back(cfg);
	}

	return cfgList;
}

const SalStreamDescription::cfg_map SalStreamDescription::getAllCfgs() const {
	return cfgs;
}

void SalStreamDescription::setProtoInCfg(SalStreamConfiguration &cfg, const std::string &str) {
	std::string protoOther;
	auto proto = SalProtoOther;
	if (!str.empty()) {
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
			proto = SalProtoOther;
			protoOther = protoAsString;
		}
	}
	cfg.proto = proto;
	cfg.proto_other = protoOther;
}

void SalStreamDescription::createActualCfg(const SalMediaDescription *salMediaDesc,
                                           const belle_sdp_session_description_t *sdp,
                                           const belle_sdp_media_description_t *media_desc) {
	belle_sdp_media_t *media;
	belle_sdp_attribute_t *attribute;
	belle_sip_list_t *custom_attribute_it;
	const char *value;

	media = belle_sdp_media_description_get_media(media_desc);

	SalStreamConfiguration actualCfg;

	/*copy dtls attributes from session descriptiun, might be overwritten stream by stream*/
	/*DTLS attributes can be defined at session level.*/
	SalDtlsRole session_role = SalDtlsRoleInvalid;
	value = belle_sdp_session_description_get_attribute_value(sdp, "setup");
	if (value) {
		if (strncmp(value, "actpass", 7) == 0) {
			session_role = SalDtlsRoleUnset;
		} else if (strncmp(value, "active", 6) == 0) {
			session_role = SalDtlsRoleIsClient;
		} else if (strncmp(value, "passive", 7) == 0) {
			session_role = SalDtlsRoleIsServer;
		}
	}
	actualCfg.dtls_role = session_role;

	value = belle_sdp_session_description_get_attribute_value(sdp, "fingerprint");
	if (value) {
		actualCfg.dtls_fingerprint = L_C_TO_STRING(value);
	}

	const std::string protoStr = belle_sdp_media_get_protocol(media);
	setProtoInCfg(actualCfg, protoStr);
	/* Read DTLS specific attributes : check is some are found in the stream description otherwise copy the session
	 * description one(which are at least set to Invalid) */
	if (((actualCfg.proto == SalProtoUdpTlsRtpSavpf) || (actualCfg.proto == SalProtoUdpTlsRtpSavp))) {
		attribute = belle_sdp_media_description_get_attribute(media_desc, "setup");
		if (attribute && (value = belle_sdp_attribute_get_value(attribute)) != NULL) {
			if (strncmp(value, "actpass", 7) == 0) {
				actualCfg.dtls_role = SalDtlsRoleUnset;
			} else if (strncmp(value, "active", 6) == 0) {
				actualCfg.dtls_role = SalDtlsRoleIsClient;
			} else if (strncmp(value, "passive", 7) == 0) {
				actualCfg.dtls_role = SalDtlsRoleIsServer;
			}
		}
		if (actualCfg.dtls_role != SalDtlsRoleInvalid &&
		    (attribute = belle_sdp_media_description_get_attribute(media_desc, "fingerprint"))) {
			actualCfg.dtls_fingerprint = belle_sdp_attribute_get_value(attribute);
		}
	}

	belle_sdp_connection_t *cnx;
	if ((cnx = belle_sdp_media_description_get_connection(media_desc)) && belle_sdp_connection_get_address(cnx)) {
		actualCfg.ttl = belle_sdp_connection_get_ttl(cnx);
	}

	actualCfg.rtcp_mux = belle_sdp_media_description_get_attribute(media_desc, "rtcp-mux") != NULL;
	actualCfg.bundle_only = belle_sdp_media_description_get_attribute(media_desc, "bundle-only") != NULL;

	attribute = belle_sdp_media_description_get_attribute(media_desc, "mid");
	if (attribute) {
		value = belle_sdp_attribute_get_value(attribute);
		if (value) actualCfg.mid = L_C_TO_STRING(value);
	}

	SalStreamDir dir = SalStreamInactive;
	if (belle_sdp_media_description_get_attribute(media_desc, "sendrecv")) {
		dir = SalStreamSendRecv;
	} else if (belle_sdp_media_description_get_attribute(media_desc, "sendonly")) {
		dir = SalStreamSendOnly;
	} else if (belle_sdp_media_description_get_attribute(media_desc, "recvonly")) {
		dir = SalStreamRecvOnly;
	} else if (belle_sdp_media_description_get_attribute(media_desc, "inactive")) {
		dir = SalStreamInactive;
	} else {
		dir = ((rtp_port == 0) && (!actualCfg.bundle_only)) ? SalStreamInactive
		                                                    : salMediaDesc->dir; /*takes default value if not present*/
	}
	actualCfg.dir = dir;

	actualCfg.payloads.clear();
	/* Get media payload types */
	sdpParsePayloadTypes(actualCfg, media_desc);

	/* Read crypto lines if any */
	if (actualCfg.hasSrtp()) {
		sdpParseMediaCryptoParameters(actualCfg, media_desc);
	}

	/* Read zrtp-hash attribute */
	if ((attribute = belle_sdp_media_description_get_attribute(media_desc, "zrtp-hash")) != NULL) {
		if ((value = belle_sdp_attribute_get_value(attribute)) != NULL) {
#if __GNUC__ > 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
			strncpy((char *)(actualCfg.zrtphash), belle_sdp_attribute_get_value(attribute), sizeof(actualCfg.zrtphash));
			actualCfg.haveZrtpHash = 1;
#if __GNUC__ > 7
#pragma GCC diagnostic pop
#endif
		}
	}

	/* Do we have Lime Ik attribute */
	if ((attribute = belle_sdp_media_description_get_attribute(media_desc, "Ik")) != NULL) {
		if (belle_sdp_attribute_get_value(attribute) != NULL) {
			actualCfg.haveLimeIk = 1;
		}
	}
	/* get ready to parse also lime-Ik */
	if ((attribute = belle_sdp_media_description_get_attribute(media_desc, "lime-Ik")) != NULL) {
		if (belle_sdp_attribute_get_value(attribute) != NULL) {
			actualCfg.haveLimeIk = 1;
		}
	}

	/* Get ICE candidate attributes if any */
	sdpParseMediaIceParameters(media_desc);

	bool has_avpf_attributes = sdpParseRtcpFbParameters(actualCfg, media_desc);

	/* Get RTCP-FB attributes if any */
	if (actualCfg.hasAvpf()) {
		actualCfg.enableAvpfForStream();
	} else if (has_avpf_attributes) {
		actualCfg.enableAvpfForStream();
		actualCfg.implicit_rtcp_fb = true;
	}

	/* Get RTCP-XR attributes if any */
	actualCfg.rtcp_xr = salMediaDesc->rtcp_xr; // Use session parameters if no stream parameters are defined
	sdp_parse_media_rtcp_xr_parameters(media_desc, &actualCfg.rtcp_xr);

	/* Get the ssrc used for conference if any */
	if ((attribute = belle_sdp_media_description_get_attribute(media_desc, "ssrc")) != NULL) {
		if ((value = belle_sdp_attribute_get_value(attribute)) != NULL) {
			sscanf(value, "%u", &actualCfg.conference_ssrc);
		}
	}

	/* Get the custom attributes, and parse some 'extmap'*/
	for (custom_attribute_it = belle_sdp_media_description_get_attributes(media_desc); custom_attribute_it != NULL;
	     custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		const char *attr_name = belle_sdp_attribute_get_name(attr);
		const char *attr_value = belle_sdp_attribute_get_value(attr);
		custom_sdp_attributes = sal_custom_sdp_attribute_append(custom_sdp_attributes, attr_name, attr_value);

		if (strcasecmp(attr_name, "extmap") == 0) {
			char *extmap_urn = (char *)bctbx_malloc0(strlen(attr_value) + 1);
			int value = 0;
			if (sscanf(attr_value, "%i %s", &value, extmap_urn) > 0) {
				if (value < 1 || value > 15) {
					lError() << "Extmap value out of range for \"" << extmap_urn << "\"";
					continue;
				}

				if (strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:sdes:mid") == 0) {
					actualCfg.mid_rtp_ext_header_id = value;
				} else if (strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:csrc-audio-level") == 0) {
					actualCfg.mixer_to_client_extension_id = value;
				} else if (strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:ssrc-audio-level") == 0) {
					actualCfg.client_to_mixer_extension_id = value;
				} else if (strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:framemarking") == 0) {
					actualCfg.frame_marking_extension_id = value;
				}
			}
			bctbx_free(extmap_urn);
		}
	}

	LinphoneMediaEncryption enc =
	    sal_media_proto_to_linphone_media_encryption(actualCfg.getProto(), (actualCfg.hasZrtpHash() ? true : false));
	supportedEncryption.push_front(enc);

	addActualConfiguration(actualCfg);
}

SalStreamDescription &SalStreamDescription::operator=(const SalStreamDescription &other) {
	name = other.name;
	type = other.type;
	typeother = other.typeother;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	acaps = other.acaps;
	tcaps = other.tcaps;
	for (const auto &cfg : other.cfgs) {
		const auto result = cfgs.insert(cfg);
		if (!result.second) cfgs[cfg.first] = cfg.second;
	}
	for (const auto &cfg : other.unparsed_cfgs) {
		const auto result = unparsed_cfgs.insert(cfg);
		if (!result.second) unparsed_cfgs[cfg.first] = cfg.second;
	}
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
	for (const auto &pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	multicast_role = other.multicast_role;

	ice_candidates = other.ice_candidates;
	ice_remote_candidates = other.ice_remote_candidates;
	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	ice_mismatch = other.ice_mismatch;

	supportedEncryption = other.supportedEncryption;

	sal_custom_sdp_attribute_free(custom_sdp_attributes);
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);

	cfgIndex = other.cfgIndex;

	label = other.label;
	content = other.content;

	return *this;
}

bool SalStreamDescription::operator==(const SalStreamDescription &other) const {
	return equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED;
}

bool SalStreamDescription::operator!=(const SalStreamDescription &other) const {
	return !(*this == other);
}

int SalStreamDescription::compareToChosenConfiguration(const SalStreamDescription &other) const {
	int result = globalEqual(other);

	result |= compareConfigurations(other, getChosenConfigurationIndex(), other.getChosenConfigurationIndex());

	return result;
}

int SalStreamDescription::compareToActualConfiguration(const SalStreamDescription &other) const {
	int result = globalEqual(other);

	result |= compareConfigurations(other, getChosenConfigurationIndex(), other.getActualConfigurationIndex());

	return result;
}

int SalStreamDescription::compareConfigurations(const SalStreamDescription &other,
                                                const SalStreamDescription::cfg_map::key_type &thisKey,
                                                const SalStreamDescription::cfg_map::key_type &otherKey) const {
	const SalStreamConfiguration &thisCfg = getConfigurationAtIndex(thisKey);
	const SalStreamConfiguration &otherCfg = other.getConfigurationAtIndex(otherKey);
	int result = (thisCfg.equal(otherCfg));

	return result;
}

int SalStreamDescription::equal(const SalStreamDescription &other) const {
	int result = globalEqual(other);

	if (cfgs.size() != other.cfgs.size()) result |= SAL_MEDIA_DESCRIPTION_CONFIGURATION_CHANGED;

	for (auto cfg1 = cfgs.cbegin(), cfg2 = other.cfgs.cbegin(); (cfg1 != cfgs.cend() && cfg2 != other.cfgs.cend());
	     ++cfg1, ++cfg2) {
		result |= cfg1->second.equal(cfg2->second);
	}

	/* ICE */
	if (ice_ufrag.compare(other.ice_ufrag) != 0 && !other.ice_ufrag.empty())
		result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (ice_pwd.compare(other.ice_pwd) != 0 && !other.ice_pwd.empty())
		result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;

	return result;
}

int SalStreamDescription::globalEqual(const SalStreamDescription &other) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	if (type != other.type) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (content.compare(other.content) != 0) result |= SAL_MEDIA_DESCRIPTION_CONTENT_CHANGED;

	// RTP
	if ((rtp_addr.compare(other.rtp_addr) != 0) && ((rtp_port != 0) || (other.rtp_port != 0)))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if ((rtp_addr.empty() == false) && (other.rtp_addr.empty() == false) &&
	    ms_is_multicast(L_STRING_TO_C(rtp_addr)) != ms_is_multicast(L_STRING_TO_C(other.rtp_addr)))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (rtp_port != other.rtp_port) {
		if ((rtp_port == 0) || (other.rtp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}

	// RTCP
	if ((rtcp_addr.compare(other.rtcp_addr) != 0) && (rtcp_port != 0) && (other.rtcp_port != 0))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if ((rtcp_addr.empty() == false) && (other.rtcp_addr.empty() == false) &&
	    ms_is_multicast(L_STRING_TO_C(rtcp_addr)) != ms_is_multicast(L_STRING_TO_C(other.rtcp_addr)))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (rtcp_port != other.rtcp_port) {
		if ((rtcp_port == 0) || (other.rtcp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}

	if (multicast_role != other.multicast_role) result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;

	return result;
}

bool SalStreamDescription::enabled() const {
	/* When the bundle-only attribute is present, a 0 rtp port doesn't mean that the stream is disabled.*/
	return rtp_port > 0 || (isBundleOnly() && (getDirection() != SalStreamInactive));
}

bool SalStreamDescription::isAcceptable() const {
	// Accept if enabled or RTP port is set to 0
	return enabled() || (rtp_port == 0);
}

void SalStreamDescription::disable() {
	rtp_port = 0;
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	cfgs[getChosenConfigurationIndex()].disable();
}

bool SalStreamDescription::hasAvpf() const {
	return getChosenConfiguration().hasAvpf();
}

bool SalStreamDescription::hasIpv6() const {
	return rtp_addr.find(':') != std::string::npos;
}

bool SalStreamDescription::hasImplicitAvpf() const {
	return getChosenConfiguration().hasImplicitAvpf();
}

bool SalStreamDescription::supportSrtp() const {
	for (const auto &cfgEl : cfgs) {
		const auto &cfg = cfgEl.second;
		if (cfg.hasSrtp()) {
			return true;
		}
	}
	return false;
}

bool SalStreamDescription::supportZrtp() const {
	for (const auto &cfgEl : cfgs) {
		const auto &cfg = cfgEl.second;
		if (cfg.hasZrtp()) {
			return true;
		}
	}
	return false;
}

bool SalStreamDescription::supportDtls() const {
	for (const auto &cfgEl : cfgs) {
		const auto &cfg = cfgEl.second;
		if (cfg.hasDtls()) {
			return true;
		}
	}
	return false;
}

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

const std::string &SalStreamDescription::getRtcpAddress() const {
	return rtcp_addr;
}

const int &SalStreamDescription::getRtcpPort() const {
	return rtcp_port;
}

const std::string &SalStreamDescription::getRtpAddress() const {
	return rtp_addr;
}

const int &SalStreamDescription::getRtpPort() const {
	return rtp_port;
}

const SalStreamType &SalStreamDescription::getType() const {
	return type;
}

const std::string SalStreamDescription::getTypeAsString() const {
	if (type == SalOther) return typeother;
	else return LinphonePrivate::Utils::toString(type);
}

void SalStreamDescription::setProto(const SalMediaProto &newProto) {
	cfgs[getChosenConfigurationIndex()].proto = newProto;
}

const SalMediaProto &SalStreamDescription::getProto() const {
	return getChosenConfiguration().getProto();
}

const std::string SalStreamDescription::getProtoAsString() const {
	return getChosenConfiguration().getProtoAsString();
}

void SalStreamDescription::setDirection(const SalStreamDir &newDir) {
	cfgs[getChosenConfigurationIndex()].dir = newDir;
}

SalStreamDir SalStreamDescription::getDirection() const {
	return getChosenConfiguration().getDirection();
}

const std::list<OrtpPayloadType *> &SalStreamDescription::getPayloads() const {
	return getChosenConfiguration().getPayloads();
}

const int &SalStreamDescription::getMaxRate() const {
	return getChosenConfiguration().getMaxRate();
}

SalCustomSdpAttribute *SalStreamDescription::getCustomSdpAttributes() const {
	return custom_sdp_attributes;
}

void SalStreamDescription::setPtime(const int &ptime, const int &maxptime) {
	if (ptime > 0) {
		cfgs[getChosenConfigurationIndex()].ptime = ptime;
	}
	if (maxptime > 0) {
		cfgs[getChosenConfigurationIndex()].maxptime = maxptime;
	}
}

const std::vector<SalSrtpCryptoAlgo> &SalStreamDescription::getCryptos() const {
	return getChosenConfiguration().crypto;
}

const SalSrtpCryptoAlgo &SalStreamDescription::getCryptoAtIndex(const size_t &idx) const {
	try {
		const auto &el = getChosenConfiguration().crypto.at(idx);
		return el;
	} catch (std::out_of_range &) {
		lError() << "Unable to crypto at index " << idx << " in the available crypto vector";
		return Utils::getEmptyConstRefObject<SalSrtpCryptoAlgo>();
	}
}

void SalStreamDescription::setCrypto(const size_t &idx, const SalSrtpCryptoAlgo &newCrypto) {
	cfgs[getChosenConfigurationIndex()].crypto[idx] = newCrypto;
}

void SalStreamDescription::setLabel(const std::string newLabel) {
	label = newLabel;
}

const std::string &SalStreamDescription::getLabel() const {
	return label;
}

void SalStreamDescription::setContent(const std::string newContent) {
	content = newContent;
}

const std::string &SalStreamDescription::getContent() const {
	return content;
}

void SalStreamDescription::setSupportedEncryptions(const std::list<LinphoneMediaEncryption> &encryptionList) {
	supportedEncryption = encryptionList;
}

void SalStreamDescription::setupRtcpFb(const bool nackEnabled, const bool tmmbrEnabled, const bool implicitRtcpFb) {
	for (auto &cfg : cfgs) {
		cfg.second.rtcp_fb.generic_nack_enabled = nackEnabled;
		cfg.second.rtcp_fb.tmmbr_enabled = tmmbrEnabled;
		cfg.second.implicit_rtcp_fb = implicitRtcpFb;
	}
}

void SalStreamDescription::setupRtcpXr(const OrtpRtcpXrConfiguration &rtcpXr) {
	for (auto &cfg : cfgs) {
		memcpy(&cfg.second.rtcp_xr, &rtcpXr, sizeof(cfg.second.rtcp_xr));
	}
}

belle_sdp_media_description_t *
SalStreamDescription::toSdpMediaDescription(const SalMediaDescription *salMediaDesc,
                                            belle_sdp_session_description_t *session_desc) const {
	belle_sdp_mime_parameter_t *mime_param;
	belle_sdp_media_description_t *media_desc;
	const char *dirStr = NULL;
	bool different_rtp_and_rtcp_addr;
	bool stream_enabled = enabled();

	const auto &actualCfg = getActualConfiguration();

	media_desc = belle_sdp_media_description_create(L_STRING_TO_C(getTypeAsString()), rtp_port, 1,
	                                                L_STRING_TO_C(getProtoAsString()), NULL);
	if (!actualCfg.payloads.empty()) {
		for (const auto &pt : actualCfg.payloads) {
			mime_param = belle_sdp_mime_parameter_create(pt->mime_type, payload_type_get_number(pt), pt->clock_rate,
			                                             pt->channels > 0 ? pt->channels : -1);
			belle_sdp_mime_parameter_set_parameters(mime_param, pt->recv_fmtp);
			if (actualCfg.ptime > 0) {
				belle_sdp_mime_parameter_set_ptime(mime_param, actualCfg.ptime);
			}
			belle_sdp_media_description_append_values_from_mime_parameter(media_desc, mime_param);
			belle_sip_object_unref(mime_param);
		}
	} else {
		/* to comply with SDP we cannot have an empty payload type number list */
		/* as it happens only when mline is declined with a zero port, it does not matter to put whatever codec*/
		belle_sip_list_t *format = belle_sip_list_append(NULL, 0);
		belle_sdp_media_set_media_formats(belle_sdp_media_description_get_media(media_desc), format);
	}

	/*only add a c= line within the stream description if address are differents*/
	if ((rtp_addr.empty() == false) && (salMediaDesc && (rtp_addr.compare(salMediaDesc->addr) != 0))) {
		bool_t inet6;
		belle_sdp_connection_t *connection;
		if (rtp_addr.find(':') != std::string::npos) {
			inet6 = true;
		} else inet6 = false;
		connection = belle_sdp_connection_create("IN", inet6 ? "IP6" : "IP4", L_STRING_TO_C(rtp_addr));
		if (ms_is_multicast(L_STRING_TO_C(rtp_addr))) {
			/*remove session cline in case of multicast*/
			belle_sdp_session_description_set_connection(session_desc, NULL);
			if (inet6 == false) belle_sdp_connection_set_ttl(connection, actualCfg.ttl);
		}
		belle_sdp_media_description_set_connection(media_desc, connection);
	}

	if (bandwidth > 0) belle_sdp_media_description_set_bandwidth(media_desc, "AS", bandwidth);

	if (actualCfg.hasSrtp()) {
		/* add crypto lines */
		for (const auto &crypto : actualCfg.crypto) {
			const auto value = SalStreamConfiguration::cryptoToSdpValue(crypto);
			if (!value.empty()) {
				belle_sdp_media_description_add_attribute(media_desc,
				                                          belle_sdp_attribute_create("crypto", value.c_str()));
			}
		}
	}

	/* insert DTLS session attribute if needed */
	addDtlsAttributesToMediaDesc(actualCfg, media_desc);

	/* insert zrtp-hash attribute if needed */
	if ((actualCfg.haveZrtpHash == 1) && (actualCfg.zrtphash[0] != 0)) {
		belle_sdp_media_description_add_attribute(
		    media_desc, belle_sdp_attribute_create("zrtp-hash", (const char *)(actualCfg.zrtphash)));
	}

	switch (actualCfg.dir) {
		case SalStreamSendRecv:
			/*dirStr="sendrecv";*/
			dirStr = NULL;
			break;
		case SalStreamRecvOnly:
			dirStr = "recvonly";
			break;
		case SalStreamSendOnly:
			dirStr = "sendonly";
			break;
		case SalStreamInactive:
			dirStr = "inactive";
			break;
	}
	if (dirStr) belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create(dirStr, NULL));

	if (actualCfg.rtcp_mux) {
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("rtcp-mux", NULL));
	}
	addMidAttributesToSdp(actualCfg, media_desc);

	if (actualCfg.mixer_to_client_extension_id != 0) {
		char *value = bctbx_strdup_printf("%i urn:ietf:params:rtp-hdrext:csrc-audio-level",
		                                  actualCfg.mixer_to_client_extension_id);
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("extmap", value));
		bctbx_free(value);
	}

	if (actualCfg.client_to_mixer_extension_id != 0) {
		char *value = bctbx_strdup_printf("%i urn:ietf:params:rtp-hdrext:ssrc-audio-level vad=on",
		                                  actualCfg.client_to_mixer_extension_id);
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("extmap", value));
		bctbx_free(value);
	}

	if (actualCfg.frame_marking_extension_id != 0) {
		char *value =
		    bctbx_strdup_printf("%i urn:ietf:params:rtp-hdrext:framemarking", actualCfg.frame_marking_extension_id);
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("extmap", value));
		bctbx_free(value);
	}

	if ((actualCfg.conference_ssrc != 0) && !actualCfg.rtcp_cname.empty()) {
		char *ssrc_attribute = ms_strdup_printf("%u cname:%s", actualCfg.conference_ssrc, actualCfg.rtcp_cname.c_str());
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("ssrc", ssrc_attribute));
		ms_free(ssrc_attribute);
	}

	if (rtp_port != 0) {
		different_rtp_and_rtcp_addr = (rtcp_addr.empty() == false) && (rtp_addr.compare(rtcp_addr) != 0);
		if ((rtcp_port != 0) && ((rtcp_port != (rtp_port + 1)) || (different_rtp_and_rtcp_addr == true))) {
			std::string rtcpAttrValue = std::to_string(rtcp_port);
			if (different_rtp_and_rtcp_addr == true) {
				rtcpAttrValue += " IN IP4 " + rtcp_addr;
			}
			belle_sdp_media_description_add_attribute(media_desc,
			                                          belle_sdp_attribute_create("rtcp", rtcpAttrValue.c_str()));
		}
	}
	if (actualCfg.set_nortpproxy == true) {
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("nortpproxy", "yes"));
	}
	if (ice_mismatch == true) {
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("ice-mismatch", NULL));
	} else {
		if (rtp_port != 0) {
			if (!ice_pwd.empty())
				belle_sdp_media_description_add_attribute(
				    media_desc, belle_sdp_attribute_create("ice-pwd", L_STRING_TO_C(ice_pwd)));
			if (!ice_ufrag.empty())
				belle_sdp_media_description_add_attribute(
				    media_desc, belle_sdp_attribute_create("ice-ufrag", L_STRING_TO_C(ice_ufrag)));
			addIceCandidatesToSdp(media_desc);
			addIceRemoteCandidatesToSdp(media_desc);
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
		const belle_sdp_attribute_t *session_attribute =
		    belle_sdp_session_description_get_attribute(session_desc, "rtcp-xr");
		belle_sdp_attribute_t *media_attribute;
		if (session_attribute != NULL) {
			belle_sip_object_marshal((belle_sip_object_t *)session_attribute, sastr, sizeof(sastr), &saoff);
		}
		media_attribute = create_rtcp_xr_attribute(&actualCfg.rtcp_xr);
		if (media_attribute != NULL) {
			belle_sip_object_marshal((belle_sip_object_t *)media_attribute, mastr, sizeof(mastr), &maoff);
		}
		if (strcmp(sastr, mastr) != 0) {
			belle_sdp_media_description_add_attribute(media_desc, media_attribute);
		} else {
			belle_sip_object_unref((belle_sip_object_t *)media_attribute);
		}
	}

	if (!label.empty()) {
		belle_sdp_attribute_t *label_attribute = belle_sdp_attribute_create("label", label.c_str());
		belle_sdp_media_description_add_attribute(media_desc, label_attribute);
	}

	if (!content.empty()) {
		belle_sdp_attribute_t *content_attribute = belle_sdp_attribute_create("content", content.c_str());
		belle_sdp_media_description_add_attribute(media_desc, content_attribute);
	}

	if (custom_sdp_attributes) {
		belle_sdp_session_description_t *custom_desc = (belle_sdp_session_description_t *)custom_sdp_attributes;
		belle_sip_list_t *l = belle_sdp_session_description_get_attributes(custom_desc);
		belle_sip_list_t *elem;
		for (elem = l; elem != NULL; elem = elem->next) {
			belle_sdp_media_description_add_attribute(media_desc, (belle_sdp_attribute_t *)elem->data);
		}
	}

	if (salMediaDesc->getParams().capabilityNegotiationSupported()) {
		for (const auto &acap : acaps) {
			const auto &idx = acap.first;
			const auto &nameValuePair = acap.second;
			const auto &name = nameValuePair.first;
			const auto &value = nameValuePair.second;
			std::string acapValue{};
			if (value.empty()) {
				acapValue = std::to_string(idx) + " " + name;
			} else {
				acapValue = std::to_string(idx) + " " + name + ":" + value;
			}
			belle_sdp_media_description_add_attribute(media_desc,
			                                          belle_sdp_attribute_create("acap", acapValue.c_str()));
		}

		std::string tcapValue;
		SalStreamDescription::tcap_map_t::key_type prevIdx = 0;
		for (const auto &tcap : tcaps) {
			const auto &idx = tcap.first;
			const auto &value = tcap.second;
			if (salMediaDesc->getParams().tcapLinesMerged()) {
				if (tcapValue.empty()) {
					tcapValue = std::to_string(idx) + " " + value;
					prevIdx = idx;
				} else {
					if (idx == (prevIdx + 1)) {
						tcapValue += " " + value;
					} else {
						belle_sdp_media_description_add_attribute(
						    media_desc, belle_sdp_attribute_create("tcap", tcapValue.c_str()));
						tcapValue = std::to_string(idx) + " " + value;
					}
					prevIdx = idx;
				}
			} else {
				tcapValue = std::to_string(idx) + " " + value;
				belle_sdp_media_description_add_attribute(media_desc,
				                                          belle_sdp_attribute_create("tcap", tcapValue.c_str()));
			}
		}

		if (salMediaDesc->getParams().tcapLinesMerged() && !tcapValue.empty()) {
			belle_sdp_media_description_add_attribute(media_desc,
			                                          belle_sdp_attribute_create("tcap", tcapValue.c_str()));
		}

		for (const auto &[cfgKey, cfg] : cfgs) {
			if (stream_enabled && (cfg.hasAvpf() || cfg.hasImplicitAvpf())) {
				for (const auto &pt : cfg.payloads) {
					/* AVPF/SAVPF profile is used so enable AVPF for all payload types. */
					payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				}
			}

			const auto &cfgSdpString = cfg.getSdpString();
			if (!cfgSdpString.empty()) {
				const auto &cfgIdx = cfg.index;
				// Add acfg or pcfg attributes if not the actual configuration or if the actual configuration refer to a
				// potential configuration (member index is not the actual configuration index)
				std::string attrName;
				if (cfgKey != getActualConfigurationIndex()) {
					attrName = "pcfg";
				} else if (cfgKey == getActualConfigurationIndex() && (cfgIdx != getActualConfigurationIndex())) {
					attrName = "acfg";
				}
				if (!attrName.empty()) {
					const auto attrValue = std::to_string(cfgIdx) + " " + cfgSdpString;
					belle_sdp_media_description_add_attribute(
					    media_desc, belle_sdp_attribute_create(attrName.c_str(), attrValue.c_str()));
				}
			}
		}
	}

	return media_desc;
}

void SalStreamDescription::addDtlsAttributesToMediaDesc(const SalStreamConfiguration &cfg,
                                                        belle_sdp_media_description_t *media_desc) const {
	if ((cfg.proto == SalProtoUdpTlsRtpSavpf) || (cfg.proto == SalProtoUdpTlsRtpSavp)) {
		if ((cfg.dtls_role != SalDtlsRoleInvalid) && (!cfg.dtls_fingerprint.empty())) {
			const auto setupAttrValue = SalStreamConfiguration::getSetupAttributeForDtlsRole(cfg.dtls_role);
			if (!setupAttrValue.empty()) {
				belle_sdp_media_description_add_attribute(media_desc,
				                                          belle_sdp_attribute_create("setup", setupAttrValue.c_str()));
			}
			belle_sdp_media_description_add_attribute(
			    media_desc, belle_sdp_attribute_create("fingerprint", L_STRING_TO_C(cfg.dtls_fingerprint)));
		}
	}
}

bool SalStreamDescription::sdpParseRtcpFbParameters(SalStreamConfiguration &cfg,
                                                    const belle_sdp_media_description_t *media_desc) {
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
				for (const auto &pt : cfg.payloads) {
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
			for (const auto &pt : cfg.payloads) {
				retval = true;
				if (payload_type_get_number(pt) == (int)pt_num) {
					applyRtcpFbAttributeToPayload(cfg, fb_attribute, pt);
				}
			}
		}
	}
	return retval;
}

void SalStreamDescription::sdpParsePayloadTypes(SalStreamConfiguration &cfg,
                                                const belle_sdp_media_description_t *media_desc) const {
	OrtpPayloadType *pt;
	PayloadTypeAvpfParams avpf_params;
	belle_sip_list_t *mime_param_it = NULL;
	belle_sdp_mime_parameter_t *mime_param;
	belle_sip_list_t *mime_params = belle_sdp_media_description_build_mime_parameters(media_desc);
	memset(&avpf_params, 0, sizeof(avpf_params));
	for (mime_param_it = mime_params; mime_param_it != NULL; mime_param_it = mime_param_it->next) {
		mime_param = BELLE_SDP_MIME_PARAMETER(mime_param_it->data);

		pt = payload_type_new();
		payload_type_set_number(pt, belle_sdp_mime_parameter_get_media_format(mime_param));
		pt->clock_rate = belle_sdp_mime_parameter_get_rate(mime_param);
		pt->mime_type = ms_strdup(belle_sdp_mime_parameter_get_type(mime_param));
		pt->channels = belle_sdp_mime_parameter_get_channel_count(mime_param);
		payload_type_set_send_fmtp(pt, belle_sdp_mime_parameter_get_parameters(mime_param));
		payload_type_set_avpf_params(pt, avpf_params);
		cfg.payloads.push_back(pt);
		cfg.ptime = belle_sdp_mime_parameter_get_ptime(mime_param);
		cfg.maxptime = belle_sdp_mime_parameter_get_max_ptime(mime_param);
		ms_message("Found payload %s/%i fmtp=%s", pt->mime_type, pt->clock_rate, pt->send_fmtp ? pt->send_fmtp : "");
	}
	if (mime_params) belle_sip_list_free_with_data(mime_params, belle_sip_object_unref);
}

void SalStreamDescription::sdpParseMediaCryptoParameters(SalStreamConfiguration &cfg,
                                                         const belle_sdp_media_description_t *media_desc) const {
	belle_sip_list_t *attribute_it;
	belle_sdp_attribute_t *attribute;

	cfg.crypto.clear();
	for (attribute_it = belle_sdp_media_description_get_attributes(media_desc); attribute_it != NULL;
	     attribute_it = attribute_it->next) {
		attribute = BELLE_SDP_ATTRIBUTE(attribute_it->data);

		if (keywordcmp("crypto", belle_sdp_attribute_get_name(attribute)) == 0 &&
		    belle_sdp_attribute_get_value(attribute) != NULL) {
			const std::string cryptoAttrVal = belle_sdp_attribute_get_value(attribute);
			const auto cryptoEl = SalStreamConfiguration::fillStrpCryptoAlgoFromString(cryptoAttrVal);
			if (cryptoEl.algo != MS_CRYPTO_SUITE_INVALID) {
				cfg.crypto.push_back(cryptoEl);
			} else {
				ms_warning("sdp has a strange a= line (%s)", belle_sdp_attribute_get_value(attribute));
			}
		}
	}
	ms_message("Found: %0zu valid crypto lines", cfg.crypto.size());
}

void SalStreamDescription::sdpParseMediaIceParameters(const belle_sdp_media_description_t *media_desc) {
	belle_sip_list_t *attribute_it;
	belle_sdp_attribute_t *attribute;
	const char *att_name;
	const char *value;

	for (attribute_it = belle_sdp_media_description_get_attributes(media_desc); attribute_it != NULL;
	     attribute_it = attribute_it->next) {
		attribute = BELLE_SDP_ATTRIBUTE(attribute_it->data);
		att_name = belle_sdp_attribute_get_name(attribute);
		value = belle_sdp_attribute_get_value(attribute);

		char addr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
		if ((keywordcmp("candidate", att_name) == 0) && (value != NULL)) {
			SalIceCandidate candidate;
			char raddr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN] = {0};
			char foundation[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN] = {0};
			char type[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN] = {0};
			char proto[4] = {0};
			int nb = sscanf(value, "%s %u %3s %u %s %d typ %s raddr %s rport %d", foundation, &candidate.componentID,
			                proto, &candidate.priority, addr, &candidate.port, type, raddr, &candidate.rport);
			candidate.addr = addr;
			candidate.raddr = raddr;
			candidate.foundation = foundation;
			candidate.type = type;
			if (strcasecmp("udp", proto) == 0 && ((nb == 7) || (nb == 9))) {
				ice_candidates.push_back(candidate);
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
					const unsigned int noCandidates = (unsigned int)ice_remote_candidates.size();
					if (candidateIdx >= noCandidates) {
						ice_remote_candidates.resize(componentID);
					}
					ice_remote_candidates[(std::vector<SalIceRemoteCandidate>::size_type)candidateIdx] =
					    remote_candidate;
				}
				ptr += offset;
				if (ptr < endptr) {
					if (*ptr == ' ') ptr += 1;
				} else break;
			}
		} else if ((keywordcmp("ice-ufrag", att_name) == 0) && (value != NULL)) {
			ice_ufrag = L_C_TO_STRING(value);
		} else if ((keywordcmp("ice-pwd", att_name) == 0) && (value != NULL)) {
			ice_pwd = L_C_TO_STRING(value);
		} else if (keywordcmp("ice-mismatch", att_name) == 0) {
			ice_mismatch = true;
		}
	}
}

void SalStreamDescription::applyRtcpFbAttributeToPayload(SalStreamConfiguration &cfg,
                                                         belle_sdp_rtcp_fb_attribute_t *fb_attribute,
                                                         OrtpPayloadType *pt) {
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
					avpf_params.rpsi_compatibility = true;
					break;
				case BELLE_SDP_RTCP_FB_NONE:
					cfg.rtcp_fb.generic_nack_enabled = true;
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
					cfg.rtcp_fb.tmmbr_enabled = true;
					break;
				default:
					break;
			}
			break;
		case BELLE_SDP_RTCP_FB_GOOG_REMB:
			cfg.rtcp_fb.goog_remb_enabled = true;
			break;
		default:
			break;
	}
	payload_type_set_avpf_params(pt, avpf_params);
}

void SalStreamDescription::addMidAttributesToSdp(const SalStreamConfiguration &cfg,
                                                 belle_sdp_media_description_t *media_desc) const {
	if (cfg.mid.empty() == false) {
		belle_sdp_media_description_add_attribute(media_desc,
		                                          belle_sdp_attribute_create("mid", L_STRING_TO_C(cfg.mid)));
	}
	if (cfg.mid_rtp_ext_header_id) {
		char *value = bctbx_strdup_printf("%i urn:ietf:params:rtp-hdrext:sdes:mid", cfg.mid_rtp_ext_header_id);
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("extmap", value));
		bctbx_free(value);
	}
	if (cfg.bundle_only) {
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("bundle-only", NULL));
	}
}

bool SalStreamDescription::isRtcpFbTrrIntTheSameForAllPayloads(const SalStreamConfiguration &cfg,
                                                               uint16_t *trr_int) const {
	bool first = true;
	for (const auto &pt : cfg.payloads) {
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

void SalStreamDescription::addRtcpFbAttributesToSdp(const SalStreamConfiguration &cfg,
                                                    belle_sdp_media_description_t *media_desc) const {
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
	if (cfg.rtcp_fb.goog_remb_enabled == TRUE) {
		add_rtcp_fb_goog_remb_attribute(media_desc, -1);
	}

	for (const auto &pt : cfg.payloads) {
		/* AVPF/SAVPF profile is used so enable AVPF for all payload types. */
		payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
		avpf_params = payload_type_get_avpf_params(pt);

		/* Add trr-int if not set generally. */
		if (general_trr_int != true && trr_int != 0) {
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

void SalStreamDescription::addIceCandidatesToSdp(belle_sdp_media_description_t *md) const {
	for (const auto &candidate : ice_candidates) {
		if ((candidate.addr.empty()) || (candidate.port == 0)) break;
		std::string iceCandidateValue = candidate.foundation + " " + std::to_string(candidate.componentID) + " UDP " +
		                                std::to_string(candidate.priority) + " " + candidate.addr.c_str() + " " +
		                                std::to_string(candidate.port) + " typ " + candidate.type;
		if (iceCandidateValue.size() > 1024) {
			ms_error("Cannot add ICE candidate attribute!");
			return;
		}
		if (!candidate.raddr.empty()) {
			iceCandidateValue += " raddr " + candidate.raddr + " rport " + std::to_string(candidate.rport);
			if (iceCandidateValue.size() > 1024) {
				ms_error("Cannot add ICE candidate attribute!");
				return;
			}
		}
		belle_sdp_media_description_add_attribute(md,
		                                          belle_sdp_attribute_create("candidate", iceCandidateValue.c_str()));
	}
}

void SalStreamDescription::addIceRemoteCandidatesToSdp(belle_sdp_media_description_t *md) const {
	std::string iceRemoteCandidateValue;

	for (size_t i = 0; i < ice_remote_candidates.size(); i++) {
		const auto &candidate = ice_remote_candidates[i];
		if ((!candidate.addr.empty()) && (candidate.port != 0)) {
			iceRemoteCandidateValue += ((i > 0) ? " " : "") + std::to_string(static_cast<unsigned int>(i + 1)) + " " +
			                           candidate.addr + " " + std::to_string(candidate.port);

			if (iceRemoteCandidateValue.size() > 1024) {
				ms_error("Cannot add ICE remote-candidates attribute!");
				return;
			}
		}
	}
	if (!iceRemoteCandidateValue.empty())
		belle_sdp_media_description_add_attribute(
		    md, belle_sdp_attribute_create("remote-candidates", iceRemoteCandidateValue.c_str()));
}

const std::list<LinphoneMediaEncryption> &SalStreamDescription::getSupportedEncryptions() const {
	return supportedEncryption;
}

const PotentialCfgGraph::media_description_config::key_type &SalStreamDescription::getChosenConfigurationIndex() const {
	return cfgIndex;
}

const PotentialCfgGraph::media_description_config::key_type &SalStreamDescription::getActualConfigurationIndex() const {
	return SalStreamDescription::actualConfigurationIndex;
}

bool SalStreamDescription::hasConfigurationAtIndex(
    const PotentialCfgGraph::media_description_config::key_type &index) const {
	const auto &elCount = cfgs.count(index);
	return (elCount != 0);
}

const SalStreamConfiguration &SalStreamDescription::getConfigurationAtIndex(
    const PotentialCfgGraph::media_description_config::key_type &index) const {
	try {
		const auto &cfg = cfgs.at(index);
		return cfg;
	} catch (std::out_of_range &) {
		lDebug() << "Unable to find configuration at index " << index << " in the available configuration map";
		return Utils::getEmptyConstRefObject<SalStreamConfiguration>();
	}
}

const SalStreamConfiguration &SalStreamDescription::getActualConfiguration() const {
	return getConfigurationAtIndex(getActualConfigurationIndex());
}

const SalStreamConfiguration &SalStreamDescription::getChosenConfiguration() const {
	return getConfigurationAtIndex(getChosenConfigurationIndex());
}

void SalStreamDescription::setZrtpHash(const uint8_t enable, uint8_t *zrtphash) {
	if (enable) {
		memcpy(cfgs[getChosenConfigurationIndex()].zrtphash, zrtphash,
		       sizeof(cfgs[getChosenConfigurationIndex()].zrtphash));
	}
	cfgs[getChosenConfigurationIndex()].haveZrtpHash = enable;
}
void SalStreamDescription::setDtls(const SalDtlsRole role, const std::string &fingerprint) {
	cfgs[getChosenConfigurationIndex()].dtls_role = role;
	cfgs[getChosenConfigurationIndex()].dtls_fingerprint = fingerprint;
}

void SalStreamDescription::setBundleOnly(const bool enable) {
	cfgs[getChosenConfigurationIndex()].bundle_only = enable;
}

bool SalStreamDescription::isBundleOnly() const {
	return getChosenConfiguration().isBundleOnly();
}

const std::string &SalStreamDescription::getMid() const {
	return getChosenConfiguration().getMid();
}

void SalStreamDescription::addActualConfiguration(const SalStreamConfiguration &cfg) {
	addConfigurationAtIndex(getActualConfigurationIndex(), cfg);
}

void SalStreamDescription::addConfigurationAtIndex(const PotentialCfgGraph::media_description_config::key_type &idx,
                                                   const SalStreamConfiguration &cfg) {
	cfgs[idx] = cfg;
}

void SalStreamDescription::addTcap(const unsigned int &idx, const std::string &value) {
	tcaps[idx] = value;
}

const std::string &SalStreamDescription::getTcap(const unsigned int &idx) const {
	try {
		return tcaps.at(idx);
	} catch (std::out_of_range &) {
		lDebug() << "Unable to find transport capability at index " << idx;
		return Utils::getEmptyConstRefObject<std::string>();
	}
}

void SalStreamDescription::addAcap(const unsigned int &idx, const std::string &name, const std::string &value) {
	acaps[idx] = std::make_pair(name, value);
}

const SalStreamDescription::acap_t &SalStreamDescription::getAcap(const unsigned int &idx) const {
	try {
		return acaps.at(idx);
	} catch (std::out_of_range &) {
		lDebug() << "Unable to find attribute capability at index " << idx;
		return Utils::getEmptyConstRefObject<SalStreamDescription::acap_t>();
	}
}

const SalStreamDescription::acap_map_t &SalStreamDescription::getAcaps() const {
	return acaps;
}

const SalStreamDescription::tcap_map_t &SalStreamDescription::getTcaps() const {
	return tcaps;
}

unsigned int SalStreamDescription::getFreeCfgIdx() const {
	std::list<unsigned int> cfgIndexes;
	auto addToIndexList = [&cfgIndexes](const auto &cfg) { cfgIndexes.push_back(cfg.first); };
	const auto &streamCfgs = getAllCfgs();
	std::for_each(streamCfgs.begin(), streamCfgs.end(), addToIndexList);

	return PotentialCfgGraph::getFreeIdx(cfgIndexes);
}

bool SalStreamDescription::getIceMismatch() const {
	return ice_mismatch;
}

const std::string &SalStreamDescription::getIceUfrag() const {
	return ice_ufrag;
}

const std::string &SalStreamDescription::getIcePwd() const {
	return ice_pwd;
}

const SalIceCandidate &SalStreamDescription::getIceCandidateAtIndex(const std::size_t &idx) const {
	try {
		return ice_candidates.at(idx);
	} catch (std::out_of_range &) {
		lError() << "Unable to Ice Candidate at index " << idx;
		return Utils::getEmptyConstRefObject<SalIceCandidate>();
	}
}

const SalIceRemoteCandidate &SalStreamDescription::getIceRemoteCandidateAtIndex(const std::size_t &idx) const {
	try {
		return ice_remote_candidates.at(idx);
	} catch (std::out_of_range &) {
		lError() << "Unable to Ice Remote Candidate at index " << idx;
		return Utils::getEmptyConstRefObject<SalIceRemoteCandidate>();
	}
}

bool SalStreamDescription::hasIceCandidates() const {
	return (!ice_candidates.empty());
}

bool SalStreamDescription::hasIceParams() const {
	// Return true if ice pwd and ufrag as well as candidates are defined
	return (!ice_ufrag.empty() && !ice_pwd.empty() && hasIceCandidates());
}
LINPHONE_END_NAMESPACE
