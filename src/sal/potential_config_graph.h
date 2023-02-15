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

#ifndef POTENTIAL_CONFIG_GRAPH_H
#define POTENTIAL_CONFIG_GRAPH_H

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bctoolbox/utils.hh"
#include "belle-sip/belle-sip.h"
#include "logger/logger.h"
#include "utils/general-internal.h"

LINPHONE_BEGIN_NAMESPACE

typedef enum config_type { ACFG, PCFG } config_type_t;

typedef enum capability_type { ATTRIBUTE, TRANSPORT_PROTOCOL, EXTENDED } capability_type_t;

std::string capabilityToAttributeName(const capability_type_t cap);

struct capability {
	unsigned int index = 0; // Invalid index
	std::string value;
	capability_type_t type = ATTRIBUTE;
};

struct acapability : public capability {
	std::string name;
};

template <class cap_type>
struct config_capability {
	std::weak_ptr<cap_type> cap;
	bool mandatory = false; // Capability is mandatory
};

struct config_attribute {
	// list of list of capabilities
	// each element of the outer list stores a list of capabilities (mandatory and optional) to create a media session -
	// in SDP terms, it represent a comma-separated continguous sequence of indexes
	std::list<std::list<config_capability<acapability>>> acap;
	std::list<config_capability<capability>> tcap;
	bool delete_media_attributes = false;   // Delete SDP media attributes
	bool delete_session_attributes = false; // Delete SDP session attributes
};

#ifdef _WIN32
// Disable C4251 triggered by types derived from STL.
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // ifdef _WIN32
class LINPHONE_INTERNAL_PUBLIC PotentialCfgGraph {

public:
	using media_description_base_cap = std::list<std::shared_ptr<capability>>;
	using session_description_base_cap = std::map<unsigned int, media_description_base_cap>;
	using media_description_acap = std::list<std::shared_ptr<acapability>>;
	using session_description_acap = std::map<unsigned int, media_description_acap>;
	using media_description_config = std::map<unsigned int, config_attribute>;
	using session_description_config = std::map<unsigned int, media_description_config>;
	using media_description_unparsed_config = std::map<unsigned int, std::string>;
	using session_description_unparsed_config = std::map<unsigned int, media_description_unparsed_config>;

	static unsigned int getFreeIdx(const std::list<unsigned int> &l);

	explicit PotentialCfgGraph();
	explicit PotentialCfgGraph(const belle_sdp_session_description_t *session_desc);
	PotentialCfgGraph &operator=(const PotentialCfgGraph &other);
	const session_description_config &getAllCfg() const;
	const session_description_unparsed_config &getUnparsedCfgs() const;
	const session_description_acap &getStreamAcap() const;
	const session_description_base_cap &getStreamTcap() const;

	const media_description_config &getCfgForStream(const session_description_config::key_type &idx) const;
	const media_description_unparsed_config &
	getUnparsedCfgForStream(const session_description_unparsed_config::key_type &idx) const;
	const media_description_acap &getGlobalAcap() const;
	const media_description_base_cap &getGlobalTcap() const;
	const media_description_acap &getMediaAcapForStream(const session_description_acap::key_type &idx) const;
	const media_description_base_cap &getMediaTcapForStream(const session_description_base_cap::key_type &idx) const;
	const media_description_acap getAllAcapForStream(const session_description_acap::key_type &idx) const;
	const media_description_base_cap getAllTcapForStream(const session_description_base_cap::key_type &idx) const;

	bool addGlobalAcap(const unsigned int &idx, const std::string &name, const std::string &value);
	bool addGlobalTcap(const unsigned int &idx, const std::string &value);

	bool addAcapToStream(const session_description_acap::key_type &streamIdx,
	                     const unsigned int &capIdx,
	                     const std::string &capName,
	                     const std::string &capValue);
	bool addTcapToStream(const session_description_base_cap::key_type &streamIdx,
	                     const unsigned int &capIdx,
	                     const std::string &capValue);

	void addCfg(const session_description_config::key_type &streamIdx,
	            const media_description_config::key_type &cfgIdx,
	            const std::list<std::map<unsigned int, bool>> &acapIdxs,
	            std::list<unsigned int> &tcapIdx,
	            const bool delete_media_attributes,
	            const bool delete_session_attributes);

	void processSessionDescription(const belle_sdp_session_description_t *session_desc);

	unsigned int getFreeTcapIdx() const;
	unsigned int getFreeAcapIdx() const;
	unsigned int getFreeCfgIdx(const session_description_config::key_type &idx) const;

	bool empty() const;

protected:
private:
	// configuration list
	// Each element of the vector is a media session
	media_description_acap globalAcap;
	media_description_base_cap globalTcap;
	session_description_config cfgs;
	session_description_unparsed_config unparsed_cfgs;
	session_description_acap acap;
	session_description_base_cap tcap;

	template <class cap_type>
	const std::pair<std::list<std::list<config_capability<cap_type>>>, bool>
	parseIdxList(const std::string &idxList, const std::list<std::shared_ptr<cap_type>> &availableCaps) const;

	// Session
	const belle_sip_list_t *getSessionCapabilityAttributes(const belle_sdp_session_description_t *session_desc,
	                                                       const capability_type_t cap);

	// Media
	void processMediaDescription(const unsigned int &idx, const belle_sdp_media_description_t *media_desc);
	const belle_sip_list_t *getMediaCapabilityAttributes(const belle_sdp_media_description_t *media_desc,
	                                                     const capability_type_t cap);

	// Attribute capabilities
	media_description_acap getSessionDescriptionACapabilities(const belle_sdp_session_description_t *session_desc);
	media_description_acap getMediaDescriptionACapabilities(const belle_sdp_media_description_t *media_desc);
	media_description_acap createACapabilitiesList(const belle_sip_list_t *caps_attr, const capability_type_t cap);

	// Transport capabilities
	media_description_base_cap getSessionDescriptionTCapabilities(const belle_sdp_session_description_t *session_desc);
	media_description_base_cap getMediaDescriptionTCapabilities(const belle_sdp_media_description_t *media_desc);
	media_description_base_cap createTCapabilitiesList(const belle_sip_list_t *caps_attr, const capability_type_t cap);

	// Configuration
	bool processMediaCfg(const unsigned int &idx,
	                     const belle_sdp_media_description_t *media_desc,
	                     const config_type_t cfgType);
	bool processMediaAcfg(const unsigned int &idx, const belle_sdp_media_description_t *media_desc);
	bool processMediaPcfg(const unsigned int &idx, const belle_sdp_media_description_t *media_desc);
	// TODO: should attribute have const? belle_sdp_pcfg_attribute_get_configs takes a non const
	media_description_config::mapped_type createPConfigFromAttribute(belle_sdp_pcfg_attribute_t *attribute,
	                                                                 const media_description_acap &mediaAcap,
	                                                                 const media_description_base_cap &mediaTcap);
	// TODO: should attribute have const? belle_sdp_acfg_attribute_get_configs takes a non const
	media_description_config::mapped_type createAConfigFromAttribute(belle_sdp_acfg_attribute_t *attribute,
	                                                                 const media_description_acap &mediaAcap,
	                                                                 const media_description_base_cap &mediaTcap);
	media_description_config::mapped_type processConfig(const belle_sip_list_t *configList,
	                                                    const media_description_acap &mediaAcap,
	                                                    const media_description_base_cap &mediaTcap) const;
	capability_type_t capabilityTypeFromAttrParam(const std::string &attrParam) const;
	unsigned int getElementIdx(const std::string &index) const;

	// streamIdx -> index of the stream to search for capabilities
	// acapIdxs -> list of map of attribute capability indexes and mandatory attributes
	// tcapIdx -> map of transport protocol indexes
	// delete_media_attributes -> delete media attributes flags
	// delete_session_attributes -> delete session attributes flags
	config_attribute createCfgAttr(const PotentialCfgGraph::session_description_config::key_type &streamIdx,
	                               const std::list<std::map<unsigned int, bool>> &acapIdxs,
	                               std::list<unsigned int> &tcapIdx,
	                               const bool delete_media_attributes,
	                               const bool delete_session_attributes) const;

	bool canFindAcapWithIdx(const unsigned int &index) const;
	bool canFindTcapWithIdx(const unsigned int &index) const;

	void addAcapListToCfg(const session_description_config::key_type &streamIdx,
	                      const media_description_config::key_type &cfgIdx,
	                      const std::map<unsigned int, bool> &acapIdx);
	void addTcapListToCfg(const session_description_config::key_type &streamIdx,
	                      const media_description_config::key_type &cfgIdx,
	                      std::list<unsigned int> &tcapIdx);

	std::list<config_capability<acapability>> createAcapList(const session_description_config::key_type &streamIdx,
	                                                         const std::map<unsigned int, bool> &acapIdx) const;
	std::list<config_capability<capability>> createTcapList(const session_description_config::key_type &streamIdx,
	                                                        const std::list<unsigned int> &tcapIdx) const;
};

#ifdef _WIN32
#pragma warning(pop)
#endif // ifdef _WIN32

template <class cap_type>
const std::pair<std::list<std::list<config_capability<cap_type>>>, bool>
PotentialCfgGraph::parseIdxList(const std::string &idxList,
                                const std::list<std::shared_ptr<cap_type>> &availableCaps) const {
	const char configDelim = '|';
	const auto attrCapList = bctoolbox::Utils::split(idxList, configDelim);
	bool mandatory = true;

	const char startOptDelim = '[';
	const char endOptDelim = ']';
	std::list<std::list<config_capability<cap_type>>> capList;
	bool success = true;
	for (const auto &config : attrCapList) {
		const char capDelim = ',';
		const auto capIdList = bctoolbox::Utils::split(config, capDelim);
		std::list<config_capability<cap_type>> caps;
		for (const auto &index : capIdList) {
			lDebug() << "configuration is " << config << " index is " << index;
			const auto startOptPos = index.find(startOptDelim);
			const auto endOptPos = index.find(endOptDelim);
			if (startOptPos != std::string::npos) {
				mandatory = false;
			}
			auto idx = getElementIdx(index);
			config_capability<cap_type> cfg;
			cfg.mandatory = mandatory;
			auto capIt = std::find_if(availableCaps.cbegin(), availableCaps.cend(),
			                          [&idx](const std::shared_ptr<cap_type> &cap) { return (cap->index == idx); });
			if (capIt == availableCaps.cend()) {
				lError() << "Unable to find capability with index " << idx << " - skipping it";
				// Configuration is not valid - clear all capabilities
				caps.clear();
				success = false;
				break;
			} else {
				cfg.cap = *capIt;
				caps.push_back(cfg);
			}

			if (endOptPos != std::string::npos) {
				mandatory = true;
			}
		}

		if (!caps.empty()) {
			capList.push_back(caps);
		}
	}

	return std::make_pair(capList, success);
}

LINPHONE_END_NAMESPACE

#endif // POTENTIAL_CONFIG_GRAPH_H
