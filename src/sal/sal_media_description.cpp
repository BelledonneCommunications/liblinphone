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

#include "c-wrapper/internal/c-tools.h"
#include "sal/sal_media_description.h"
#include "sal/sal_stream_bundle.h"
#include "sal/sal_stream_description.h"

LINPHONE_BEGIN_NAMESPACE

// Called by makeLocalMediaDescription to create the local media decription
SalMediaDescription::SalMediaDescription(const SalMediaDescriptionParams &descParams) {
	params = descParams;

	streams.clear();
	bundles.clear();
	custom_sdp_attributes = nullptr;
}

SalMediaDescription::~SalMediaDescription() {
	streams.clear();
	bundles.clear();
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

SalMediaDescription::SalMediaDescription(const SalMediaDescription &other) {

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
}

SalMediaDescription &SalMediaDescription::operator=(const SalMediaDescription &other) {

	name = other.name;
	username = other.username;
	addr = other.addr;

	bandwidth = other.bandwidth;
	session_ver = other.session_ver;
	session_id = other.session_id;
	origin_addr = other.origin_addr;

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

	record = other.record;

	set_nortpproxy = other.set_nortpproxy;

	params = other.params;

	haveLimeIk = other.haveLimeIk;

	times = other.times;

	return *this;
}

SalMediaDescription::SalMediaDescription(belle_sdp_session_description_t *sdp)
    : SalMediaDescription(SalMediaDescriptionParams()) {
	belle_sdp_connection_t *cnx;
	belle_sdp_session_name_t *sname;
	belle_sip_list_t *custom_attribute_it;
	const char *value;

	dir = SalStreamSendRecv;

	PotentialCfgGraph potentialCfgGraph(sdp);

	// if received SDP has no valid capability negotiation attributes, then assume that it doesn't support capability
	// negotiation
	params.enableCapabilityNegotiationSupport(!potentialCfgGraph.empty());

	if ((cnx = belle_sdp_session_description_get_connection(sdp)) && belle_sdp_connection_get_address(cnx)) {
		addr = belle_sdp_connection_get_address(cnx);
	}
	if ((sname = belle_sdp_session_description_get_session_name(sdp)) && belle_sdp_session_name_get_value(sname)) {
		name = belle_sdp_session_name_get_value(sname);
	}

	if (belle_sdp_session_description_get_bandwidth(sdp, "AS") > 0) {
		bandwidth = belle_sdp_session_description_get_bandwidth(sdp, "AS");
	}

	for (bctbx_list_t *time_it = belle_sdp_session_description_get_time_descriptions(sdp); time_it != NULL;
	     time_it = time_it->next) {
		belle_sdp_time_description_t *time_attr = (belle_sdp_time_description_t *)time_it->data;
		belle_sdp_time_t *sdp_time = belle_sdp_time_description_get_time(time_attr);
		long long startTime = belle_sdp_time_get_start(sdp_time);
		long long stopTime = belle_sdp_time_get_stop(sdp_time);
		std::pair<time_t, time_t> timePair = std::make_pair<time_t, time_t>(
		    (startTime == 0) ? (time_t)-1 : static_cast<time_t>(startTime - SalMediaDescription::ntpToUnix),
		    (stopTime == 0) ? (time_t)-1 : static_cast<time_t>(stopTime - SalMediaDescription::ntpToUnix));
		times.push_back(timePair);
	}

	belle_sdp_origin_t *origin = belle_sdp_session_description_get_origin(sdp);
	origin_addr = belle_sdp_origin_get_address(origin);
	session_id = belle_sdp_origin_get_session_id(origin);
	session_ver = belle_sdp_origin_get_session_version(origin);

	/*in some very rare case, session attribute may set stream dir*/
	if (belle_sdp_session_description_get_attribute(sdp, "sendrecv")) {
		dir = SalStreamSendRecv;
	} else if (belle_sdp_session_description_get_attribute(sdp, "sendonly")) {
		dir = SalStreamSendOnly;
	} else if (belle_sdp_session_description_get_attribute(sdp, "recvonly")) {
		dir = SalStreamRecvOnly;
	} else if (belle_sdp_session_description_get_attribute(sdp, "inactive")) {
		dir = SalStreamInactive;
	}

	/* Get ICE remote ufrag and remote pwd, and ice_lite flag */
	value = belle_sdp_session_description_get_attribute_value(sdp, "ice-ufrag");
	if (value) ice_ufrag = L_C_TO_STRING(value);

	value = belle_sdp_session_description_get_attribute_value(sdp, "ice-pwd");
	if (value) ice_pwd = L_C_TO_STRING(value);

	value = belle_sdp_session_description_get_attribute_value(sdp, "ice-lite");
	if (value) ice_lite = true;

	/* Get session RTCP-XR attributes if any */
	sdp_parse_session_rtcp_xr_parameters(sdp, &rtcp_xr);

	/* Do we have Lime Ik attribute */
	value = belle_sdp_session_description_get_attribute_value(sdp, "Ik");
	if (value) haveLimeIk = true;

	/* get ready to parse also lime-Ik */
	value = belle_sdp_session_description_get_attribute_value(sdp, "lime-Ik");
	if (value) haveLimeIk = true;

	value = belle_sdp_session_description_get_attribute_value(sdp, "record");
	if (value) {
		if (strcmp(value, "on") == 0) {
			record = SalMediaRecordOn;
		} else if (strcmp(value, "off") == 0) {
			record = SalMediaRecordOff;
		} else if (strcmp(value, "paused") == 0) {
			record = SalMediaRecordPaused;
		}
	}

	/* Get the custom attributes, parse some of them that are relevant */
	for (custom_attribute_it = belle_sdp_session_description_get_attributes(sdp); custom_attribute_it != NULL;
	     custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		custom_sdp_attributes = sal_custom_sdp_attribute_append(
		    custom_sdp_attributes, belle_sdp_attribute_get_name(attr), belle_sdp_attribute_get_value(attr));

		if (strcasecmp(belle_sdp_attribute_get_name(attr), "group") == 0) {
			value = belle_sdp_attribute_get_value(attr);
			if (value && strncasecmp(value, "BUNDLE", strlen("BUNDLE")) == 0) {
				SalStreamBundle bundle(value + strlen("BUNDLE"));
				addNewBundle(bundle);
			}
		}
	}

	// Initialize currentStreamIdx to the size of vector streams as streams build from SDP media descriptions are
	// appended. Generally, at this point, vector streams should be empty
	if (params.capabilityNegotiationSupported()) {
		for (const auto &acap : potentialCfgGraph.getGlobalAcap()) {
			acaps[acap->index] = std::make_pair(acap->name, acap->value);
		}

		for (const auto &tcap : potentialCfgGraph.getGlobalTcap()) {
			tcaps[tcap->index] = tcap->value;
		}
	}

	unsigned int currentStreamIdx = static_cast<unsigned int>(streams.size());
	for (belle_sip_list_t *media_desc_it = belle_sdp_session_description_get_media_descriptions(sdp);
	     media_desc_it != NULL; media_desc_it = media_desc_it->next) {
		belle_sdp_media_description_t *media_desc = BELLE_SDP_MEDIA_DESCRIPTION(media_desc_it->data);

		SalStreamDescription stream;
		if (params.capabilityNegotiationSupported()) {
			SalStreamDescription::raw_capability_negotiation_attrs_t attrs;
			attrs.unparsed_cfgs = potentialCfgGraph.getUnparsedCfgForStream(currentStreamIdx);
			attrs.cfgs = potentialCfgGraph.getCfgForStream(currentStreamIdx);
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

bool SalMediaDescription::hasDir(const SalStreamDir &stream_dir) const {
	if (stream_dir == SalStreamRecvOnly) {
		return containsStreamWithDir(SalStreamRecvOnly) &&
		       !(containsStreamWithDir(SalStreamSendOnly) || containsStreamWithDir(SalStreamSendRecv));
	} else if (stream_dir == SalStreamSendOnly) {
		return containsStreamWithDir(SalStreamSendOnly) &&
		       !(containsStreamWithDir(SalStreamRecvOnly) || containsStreamWithDir(SalStreamSendRecv));
	} else if (stream_dir == SalStreamSendRecv) {
		return containsStreamWithDir(SalStreamSendRecv);
	} else {
		/*SalStreamInactive*/
		if (containsStreamWithDir(SalStreamSendOnly) || containsStreamWithDir(SalStreamSendRecv) ||
		    containsStreamWithDir(SalStreamRecvOnly))
			return false;
		else return true;
	}
	return false;
}

const SalMediaDescriptionParams &SalMediaDescription::getParams() const {
	return params;
}

bool SalMediaDescription::containsStreamWithDir(const SalStreamDir &stream_dir, const SalStreamType &type) const {
	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for (auto &stream : streams) {
		if (!stream.enabled()) continue;
		if ((stream.getType() == type) && (stream.getDirection() == stream_dir)) {
			return true;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (stream.getType() == type && stream.getDirection() == SalStreamSendRecv && stream_dir == SalStreamSendOnly &&
		    (isNullAddress(addr) || isNullAddress(stream.rtp_addr))) {
			return true;
		}
	}
	return false;
}

bool SalMediaDescription::containsStreamWithDir(const SalStreamDir &stream_dir) const {
	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for (auto &stream : streams) {
		if (!stream.enabled()) continue;
		if (stream.getDirection() == stream_dir) {
			return true;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (stream.getDirection() == SalStreamSendRecv && stream_dir == SalStreamSendOnly &&
		    (isNullAddress(addr) || isNullAddress(stream.rtp_addr))) {
			return true;
		}
	}
	return false;
}

bool SalMediaDescription::isNullAddress(const std::string &addr) const {
	return addr.compare("0.0.0.0") == 0 || addr.compare("::0") == 0;
}

void SalMediaDescription::addNewBundle(const SalStreamBundle &bundle) {
	bundles.push_back(bundle);
}

int SalMediaDescription::lookupMid(const std::string mid) const {
	size_t index;
	for (index = 0; index < streams.size(); ++index) {
		const auto &sd = streams[index];
		if (sd.getChosenConfiguration().getMid().compare(mid) == 0) {
			return static_cast<int>(index);
		}
	}
	return -1;
}

const SalStreamBundle &SalMediaDescription::getBundleFromMid(const std::string mid) const {
	const auto &bundleIt =
	    std::find_if(bundles.cbegin(), bundles.cend(), [&mid](const auto &bundle) { return (bundle.hasMid(mid)); });
	if (bundleIt != bundles.cend()) {
		return *bundleIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamBundle>();
}

std::list<int> SalMediaDescription::getTransportOwnerIndexes() const {
	std::list<int> owners;
	for (const auto &sd : streams) {
		auto ownerIdx = getIndexOfTransportOwner(sd);
		if (ownerIdx >= 0) {
			auto it = std::find(owners.begin(), owners.end(), ownerIdx);
			if (owners.empty() || (it == owners.end())) {
				owners.push_back(ownerIdx);
			}
		}
	}
	return owners;
}

// getIndexOfTransportOwner() returns the following values:
// - integer greater than or equal to 0 -> index of the stream holding the transport owner
// - -1 -> stream is not part of a bundle
// - -2 -> stream is part of a bundle but the transport owner cannot be found
int SalMediaDescription::getIndexOfTransportOwner(const SalStreamDescription &sd) const {
	std::string master_mid;
	int index;
	if (sd.getChosenConfiguration().getMid().empty() == true) return -1; /* not part of any bundle */
	/* lookup the mid in the bundle descriptions */
	const auto &bundle = getBundleFromMid(sd.getChosenConfiguration().getMid());
	if (bundle == Utils::getEmptyConstRefObject<SalStreamBundle>()) {
		ms_warning("Orphan stream with mid '%s'", L_STRING_TO_C(sd.getChosenConfiguration().getMid()));
		return -2;
	}
	master_mid = bundle.getMidOfTransportOwner();
	if (master_mid.empty()) {
		ms_warning("Orphan stream with mid '%s' because the transport owner mid cannot be found",
		           L_STRING_TO_C(sd.getChosenConfiguration().getMid()));
		return -2;
	}
	index = lookupMid(master_mid);
	if (index == -1) {
		ms_error("Stream with mid '%s' has no transport owner (mid '%s') !",
		         L_STRING_TO_C(sd.getChosenConfiguration().getMid()), L_STRING_TO_C(master_mid));
	}
	return index;
}

std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findStreamIt(SalMediaProto proto,
                                                                                    SalStreamType type) const {
	const auto &streamIt = std::find_if(streams.cbegin(), streams.cend(), [&type, &proto](const auto &stream) {
		return (stream.enabled() && (stream.getProto() == proto) && (stream.getType() == type));
	});
	return streamIt;
}

const SalStreamDescription &SalMediaDescription::findStream(SalMediaProto proto, SalStreamType type) const {
	const auto &streamIt = findStreamIt(proto, type);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxStream(SalMediaProto proto, SalStreamType type) const {
	const auto &streamIt = findStreamIt(proto, type);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

std::vector<SalStreamDescription>::const_iterator
SalMediaDescription::findStreamItWithLabel(SalStreamType type, const std::string label) const {
	const auto &streamIt = std::find_if(streams.cbegin(), streams.cend(), [&type, &label](const auto &stream) {
		return ((stream.getLabel().compare(label) == 0) && (stream.getType() == type));
	});
	return streamIt;
}

const SalStreamDescription &SalMediaDescription::findStreamWithLabel(SalStreamType type,
                                                                     const std::string label) const {
	const auto &streamIt = findStreamItWithLabel(type, label);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxStreamWithLabel(SalStreamType type, const std::string label) const {
	const auto &streamIt = findStreamItWithLabel(type, label);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

std::vector<SalStreamDescription>::const_iterator
SalMediaDescription::findStreamItWithContent(const std::string content) const {
	const auto &streamIt = std::find_if(streams.cbegin(), streams.cend(), [&content](const auto &stream) {
		return (stream.getContent().compare(content) == 0);
	});
	return streamIt;
}

const SalStreamDescription &SalMediaDescription::findStreamWithContent(const std::string content) const {
	const auto &streamIt = findStreamItWithContent(content);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxStreamWithContent(const std::string content) const {
	const auto &streamIt = findStreamItWithContent(content);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

std::vector<SalStreamDescription>::const_iterator
SalMediaDescription::findStreamItWithContent(const std::string content, const SalStreamDir direction) const {
	const auto &streamIt = std::find_if(streams.cbegin(), streams.cend(), [&content, &direction](const auto &stream) {
		return (stream.enabled() && (stream.getContent().compare(content) == 0) &&
		        (stream.getDirection() == direction));
	});
	return streamIt;
}

const SalStreamDescription &SalMediaDescription::findStreamWithContent(const std::string content,
                                                                       const SalStreamDir direction) const {
	const auto &streamIt = findStreamItWithContent(content, direction);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxStreamWithContent(const std::string content, const SalStreamDir direction) const {
	const auto &streamIt = findStreamItWithContent(content, direction);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

std::vector<SalStreamDescription>::const_iterator
SalMediaDescription::findStreamItWithContent(const std::string content, const std::string label) const {
	const auto &streamIt = std::find_if(streams.cbegin(), streams.cend(), [&content, &label](const auto &stream) {
		return ((content.empty() && stream.getContent().empty()) || stream.getContent().compare(content) == 0) &&
		       ((label.empty() && stream.getLabel().empty()) || (stream.getLabel().compare(label) == 0));
	});
	return streamIt;
}

const SalStreamDescription &SalMediaDescription::findStreamWithContent(const std::string content,
                                                                       const std::string label) const {
	const auto &streamIt = findStreamItWithContent(content, label);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxStreamWithContent(const std::string content, const std::string label) const {
	const auto &streamIt = findStreamItWithContent(content, label);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

unsigned int SalMediaDescription::nbStreamsOfType(SalStreamType type) const {
	unsigned int nb = 0;
	for (const auto &stream : streams) {
		if (stream.getType() == type) nb++;
	}
	return nb;
}

unsigned int SalMediaDescription::nbActiveStreamsOfType(SalStreamType type) const {
	unsigned int nb = 0;
	for (const auto &stream : streams) {
		if (stream.enabled() && (stream.getType() == type)) nb++;
	}
	return nb;
}

const SalStreamDescription &SalMediaDescription::getActiveStreamOfType(SalStreamType type, unsigned int idx) const {
	const auto &streamIt = std::find_if(streams.cbegin(), streams.cend(), [&type, &idx](const auto &stream) {
		return (stream.enabled() && (idx-- == 0) && (stream.getType() == type));
	});
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

const SalStreamDescription &SalMediaDescription::findSecureStreamOfType(SalStreamType type) const {
	auto idx = findIdxStream(SalProtoRtpSavpf, type);
	if (idx == -1) idx = findIdxStream(SalProtoRtpSavp, type);
	if (idx != -1) {
		return getStreamAtIdx(static_cast<unsigned int>(idx));
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

const SalStreamDescription &SalMediaDescription::findBestStream(SalStreamType type) const {
	const auto idx = findIdxBestStream(type);
	if (idx != -1) {
		return getStreamAtIdx(static_cast<unsigned int>(idx));
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxBestStream(SalStreamType type) const {
	auto idx = findIdxStream(SalProtoUdpTlsRtpSavpf, type);
	if (idx == -1) idx = findIdxStream(SalProtoUdpTlsRtpSavp, type);
	if (idx == -1) idx = findIdxStream(SalProtoRtpSavpf, type);
	if (idx == -1) idx = findIdxStream(SalProtoRtpSavp, type);
	if (idx == -1) idx = findIdxStream(SalProtoRtpAvpf, type);
	if (idx == -1) idx = findIdxStream(SalProtoRtpAvp, type);
	return idx;
}

int SalMediaDescription::findIdxStreamWithSdpAttribute(
    const SalStreamType type, const std::vector<std::pair<std::string, std::string>> &attributes) const {
	const auto &streamIt = findStreamItWithSdpAttribute(type, attributes);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

const SalStreamDescription &SalMediaDescription::findStreamWithSdpAttribute(
    const SalStreamType type, const std::vector<std::pair<std::string, std::string>> &attributes) const {
	const auto &streamIt = findStreamItWithSdpAttribute(type, attributes);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findIdxStreamWithSdpAttribute(
    const std::vector<std::pair<std::string, std::string>> &attributes) const {
	const auto &streamIt = findStreamItWithSdpAttribute(attributes);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

const SalStreamDescription &SalMediaDescription::findStreamWithSdpAttribute(
    const std::vector<std::pair<std::string, std::string>> &attributes) const {
	const auto &streamIt = findStreamItWithSdpAttribute(attributes);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findStreamItWithSdpAttribute(
    const std::vector<std::pair<std::string, std::string>> &attributes) const {
	return std::find_if(streams.cbegin(), streams.cend(), [&attributes](const auto &stream) {
		bool found = true;
		for (const auto &[attrName, attrValue] : attributes) {
			if (attrValue.empty()) {
				found &= (sal_custom_sdp_attribute_is_present(stream.custom_sdp_attributes, attrName.c_str()) == TRUE);
			} else {
				const auto foundAttrVal = sal_custom_sdp_attribute_find(stream.custom_sdp_attributes, attrName.c_str());
				if (foundAttrVal) {
					found &= (strcmp(foundAttrVal, attrValue.c_str()) == 0);
				} else {
					found = false;
				}
			}
		}
		return found;
	});
}

std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findStreamItWithSdpAttribute(
    const SalStreamType type, const std::vector<std::pair<std::string, std::string>> &attributes) const {
	return std::find_if(streams.cbegin(), streams.cend(), [&type, &attributes](const auto &stream) {
		if (type != stream.getType()) return false;

		bool found = true;
		for (const auto &[attrName, attrValue] : attributes) {
			if (attrValue.empty()) {
				found &= (sal_custom_sdp_attribute_is_present(stream.custom_sdp_attributes, attrName.c_str()) == TRUE);
			} else {
				const auto foundAttrVal = sal_custom_sdp_attribute_find(stream.custom_sdp_attributes, attrName.c_str());
				if (foundAttrVal) {
					found &= (strcmp(foundAttrVal, attrValue.c_str()) == 0);
				} else {
					found = false;
				}
			}
		}
		return found;
	});
}

std::vector<SalStreamDescription>::const_iterator SalMediaDescription::findFirstStreamItOfType(SalStreamType type,
                                                                                               int startingIdx) const {
	auto streamSize = static_cast<int>(streams.size());
	auto idx = (startingIdx < 0) ? 0 : ((startingIdx >= streamSize) ? (streamSize - 1) : startingIdx);
	const auto &streamIt = std::find_if(streams.cbegin() + idx, streams.cend(),
	                                    [&type](const auto &stream) { return (stream.getType() == type); });
	return streamIt;
}

const SalStreamDescription &SalMediaDescription::findFirstStreamOfType(SalStreamType type, int startingIdx) const {
	auto streamIt = findFirstStreamItOfType(type, startingIdx);
	if (streamIt != streams.end()) {
		return *streamIt;
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

int SalMediaDescription::findFirstStreamIdxOfType(SalStreamType type, int startingIdx) const {
	auto streamIt = findFirstStreamItOfType(type, startingIdx);
	if (streamIt != streams.end()) {
		return static_cast<int>(std::distance(streams.begin(), streamIt));
	}
	return -1;
}

const std::list<SalStreamDescription> SalMediaDescription::findAllStreamsOfType(SalStreamType type) const {
	std::list<SalStreamDescription> streamList;
	for (const auto &s : streams) {
		if (s.getType() == type) {
			streamList.push_back(s);
		}
	};
	return streamList;
}

bool SalMediaDescription::isEmpty() const {
	if (getNbActiveStreams() > 0) return false;
	return true;
}

bool SalMediaDescription::isAcceptable() const {
	for (auto &stream : streams) {
		if (!stream.isAcceptable()) return false;
	}
	return true;
}

void SalMediaDescription::setDir(SalStreamDir stream_dir) {
	for (auto &stream : streams) {
		if (!stream.enabled()) continue;
		stream.setDirection(stream_dir);
	}
}

int SalMediaDescription::getNbActiveStreams() const {
	int nb = 0;
	for (auto &stream : streams) {
		if (stream.enabled()) nb++;
	}
	return nb;
}

bool SalMediaDescription::hasIceParams() const {
	bool foundIceMediaDescParams = (!ice_ufrag.empty() && !ice_pwd.empty());
	bool foundIceCandidates = true;
	bool foundIceStreamDescParams = true;
	for (const auto &stream : streams) {
		if (!stream.enabled()) continue;
		foundIceCandidates &= stream.hasIceCandidates();
		foundIceStreamDescParams &= stream.hasIceParams();
	}
	// Return true if ice pwd and ufrag is in media description and each stream has candidates or if each stream defines
	// ice pwd and ufrag as well as candidates
	return (foundIceStreamDescParams || (foundIceMediaDescParams && foundIceCandidates));
}

bool SalMediaDescription::hasAvpf() const {
	if (streams.empty()) return false;
	for (const auto &stream : streams) {
		if (!stream.enabled()) continue;
		if (stream.hasAvpf() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasImplicitAvpf() const {
	if (streams.empty()) return false;
	for (const auto &stream : streams) {
		if (!stream.enabled()) continue;
		if (stream.hasImplicitAvpf() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasSrtp() const {
	if (streams.empty()) return false;
	for (const auto &stream : streams) {
		if (!stream.enabled()) continue;
		if (stream.hasSrtp() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasDtls() const {
	if (streams.empty()) return false;
	for (const auto &stream : streams) {
		if (!stream.enabled()) continue;
		if (stream.hasDtls() != true) return false;
	}
	return true;
}

bool SalMediaDescription::hasZrtp() const {
	if (streams.empty()) return false;
	for (const auto &stream : streams) {
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
	for (const auto &stream : streams) {
		if (!stream.enabled()) continue;
		if (stream.getRtpAddress().empty() == false) {
			if (!stream.hasIpv6()) return false;
		} else {
			if (addr.find(':') == std::string::npos) return false;
		}
	}
	return true;
}

bool SalMediaDescription::operator==(const SalMediaDescription &other) const {
	return (equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED);
}

bool SalMediaDescription::operator!=(const SalMediaDescription &other) const {
	return !(*this == other);
}

int SalMediaDescription::compareToActualConfiguration(const SalMediaDescription &otherMd) const {
	int result = globalEqual(otherMd);
	for (auto stream1 = streams.cbegin(), stream2 = otherMd.streams.cbegin();
	     (stream1 != streams.cend() && stream2 != otherMd.streams.cend()); ++stream1, ++stream2) {
		if (!stream1->enabled() && !stream2->enabled()) continue;
		result |= stream1->compareToActualConfiguration(*stream2);
	}
	return result;
}

int SalMediaDescription::compareToChosenConfiguration(const SalMediaDescription &otherMd) const {
	int result = globalEqual(otherMd);
	for (auto stream1 = streams.cbegin(), stream2 = otherMd.streams.cbegin();
	     (stream1 != streams.cend() && stream2 != otherMd.streams.cend()); ++stream1, ++stream2) {
		if (!stream1->enabled() && !stream2->enabled()) continue;
		result |= stream1->compareToChosenConfiguration(*stream2);
	}
	return result;
}

int SalMediaDescription::equal(const SalMediaDescription &otherMd) const {
	int result = globalEqual(otherMd);
	for (auto stream1 = streams.cbegin(), stream2 = otherMd.streams.cbegin();
	     (stream1 != streams.cend() && stream2 != otherMd.streams.cend()); ++stream1, ++stream2) {
		if (!stream1->enabled() && !stream2->enabled()) continue;
		result |= stream1->equal(*stream2);
	}
	return result;
}

int SalMediaDescription::globalEqual(const SalMediaDescription &otherMd) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	if (addr.compare(otherMd.addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (addr.empty() == false && otherMd.addr.empty() == false &&
	    ms_is_multicast(L_STRING_TO_C(addr)) != ms_is_multicast(L_STRING_TO_C(otherMd.addr)))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (streams.size() != otherMd.streams.size()) result |= SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	if (bandwidth != otherMd.bandwidth) result |= SAL_MEDIA_DESCRIPTION_BANDWIDTH_CHANGED;

	/* ICE */
	if (ice_ufrag.compare(otherMd.ice_ufrag) != 0 && !otherMd.ice_ufrag.empty())
		result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (ice_pwd.compare(otherMd.ice_pwd) != 0 && !otherMd.ice_pwd.empty())
		result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;

	return result;
}

const std::string SalMediaDescription::printDifferences(int result) {
	std::string out = std::string();
	if (result & SAL_MEDIA_DESCRIPTION_DIRECTION_CHANGED) {
		out.append("DIRECTION_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_DIRECTION_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CODEC_CHANGED) {
		out.append("CODEC_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED) {
		out.append("NETWORK_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED) {
		out.append("ICE_RESTART_DETECTED ");
		result &= ~SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED) {
		out.append("CRYPTO_KEYS_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED) {
		out.append("NETWORK_XXXCAST_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED) {
		out.append("CRYPTO_TYPE_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED) {
		out.append("CRYPTO_POLICY_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED) {
		out.append("STREAMS_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION) {
		out.append("FORCE_STREAM_RECONSTRUCTION ");
		result &= ~SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CONFIGURATION_CHANGED) {
		out.append("STREAM_CONFIGURATION_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CONFIGURATION_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_MIXER_TO_CLIENT_EXTENSION_CHANGED) {
		out.append("MIXER_TO_CLIENT_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_MIXER_TO_CLIENT_EXTENSION_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CLIENT_TO_MIXER_EXTENSION_CHANGED) {
		out.append("CLIENT_TO_MIXER_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_CLIENT_TO_MIXER_EXTENSION_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_FRAME_MARKING_EXTENSION_CHANGED) {
		out.append("FRAME_MARKING_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_FRAME_MARKING_EXTENSION_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_PTIME_CHANGED) {
		out.append("PTIME_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_PTIME_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_BANDWIDTH_CHANGED) {
		out.append("BANDWIDTH_CHANGED ");
		result &= ~SAL_MEDIA_DESCRIPTION_BANDWIDTH_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CONTENT_CHANGED) {
		out.append("CONTENT_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_CONTENT_CHANGED;
	}
	if (result) {
		ms_fatal("There are unhandled result bitmasks in SalMediaDescription::printDifferences(), fix it");
	}
	if (out.empty()) out = "NONE";
	return out;
}

size_t SalMediaDescription::getNbStreams() const {
	return streams.size();
}

const std::string &SalMediaDescription::getOriginAddress() const {
	return origin_addr;
}

const std::string &SalMediaDescription::getConnectionAddress() const {
	return addr;
}

const SalStreamDescription &SalMediaDescription::getStreamAtIdx(unsigned int idx) const {
	try {
		return streams.at(idx);
	} catch (std::out_of_range &) {
		lError() << "Unable to find stream at index " << idx << " because media description " << this << " has "
		         << streams.size() << " streams";
	}
	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

belle_sdp_session_description_t *SalMediaDescription::toSdp() const {
	belle_sdp_session_description_t *session_desc = belle_sdp_session_description_new();
	bool_t inet6;
	belle_sdp_origin_t *origin;
	char *escaped_username = NULL;
	if (!username.empty()) {
		escaped_username = belle_sip_uri_to_escaped_username(L_STRING_TO_C(username));
	}

	if (addr.find(':') != std::string::npos) {
		inet6 = 1;
	} else inet6 = 0;
	belle_sdp_session_description_set_version(session_desc, belle_sdp_version_create(0));

	origin = belle_sdp_origin_create(escaped_username, session_id, session_ver, "IN", inet6 ? "IP6" : "IP4",
	                                 L_STRING_TO_C(origin_addr));
	if (escaped_username) {
		bctbx_free(escaped_username);
	}

	belle_sdp_session_description_set_origin(session_desc, origin);

	belle_sdp_session_description_set_session_name(
	    session_desc, belle_sdp_session_name_create(name.empty() == false ? L_STRING_TO_C(name) : "Talk"));

	if (!hasDir(SalStreamInactive) || !ice_ufrag.empty()) {
		/*in case of sendonly, setting of the IP on cnx we give a chance to receive stun packets*/
		belle_sdp_session_description_set_connection(
		    session_desc, belle_sdp_connection_create("IN", inet6 ? "IP6" : "IP4", L_STRING_TO_C(addr)));

	} else {
		belle_sdp_session_description_set_connection(
		    session_desc, belle_sdp_connection_create("IN", inet6 ? "IP6" : "IP4", inet6 ? "::0" : "0.0.0.0"));
	}

	if (times.size() > 0) {
		for (const auto &[startTime, stopTime] : times) {
			long long ntpStartTime = (startTime < 0) ? 0 : startTime + SalMediaDescription::ntpToUnix;
			long long ntpStopTime = (stopTime < 0) ? 0 : stopTime + SalMediaDescription::ntpToUnix;
			belle_sdp_session_description_set_time_description(
			    session_desc, belle_sdp_time_description_create(ntpStartTime, ntpStopTime));
		}
	} else {
		belle_sdp_session_description_set_time_description(session_desc, belle_sdp_time_description_create(0, 0));
	}

	if (bandwidth > 0) {
		belle_sdp_session_description_set_bandwidth(session_desc, "AS", bandwidth);
	}

	if (set_nortpproxy == true)
		belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("nortpproxy", "yes"));
	if (!ice_pwd.empty())
		belle_sdp_session_description_add_attribute(session_desc,
		                                            belle_sdp_attribute_create("ice-pwd", L_STRING_TO_C(ice_pwd)));
	if (!ice_ufrag.empty())
		belle_sdp_session_description_add_attribute(session_desc,
		                                            belle_sdp_attribute_create("ice-ufrag", L_STRING_TO_C(ice_ufrag)));

	if (rtcp_xr.enabled == TRUE) {
		belle_sdp_session_description_add_attribute(session_desc, create_rtcp_xr_attribute(&rtcp_xr));
	}

	for (const auto &bundle : bundles) {
		bundle.addToSdp(session_desc);
	}

	if (record != SalMediaRecordNone) {
		belle_sdp_session_description_add_attribute(
		    session_desc, belle_sdp_attribute_create("record", sal_media_record_to_string(record)));
	}

	if (custom_sdp_attributes) {
		belle_sdp_session_description_t *custom_desc = (belle_sdp_session_description_t *)custom_sdp_attributes;
		belle_sip_list_t *l = belle_sdp_session_description_get_attributes(custom_desc);
		belle_sip_list_t *elem;
		for (elem = l; elem != NULL; elem = elem->next) {
			belle_sdp_session_description_add_attribute(session_desc, (belle_sdp_attribute_t *)elem->data);
		}
	}

	if (params.capabilityNegotiationSupported()) {
		for (const auto &[idx, nameValuePair] : acaps) {
			const auto &[name, value] = nameValuePair;
			std::string acapValue{};
			if (value.empty()) {
				acapValue = std::to_string(idx) + " " + name;
			} else {
				acapValue = std::to_string(idx) + " " + name + ":" + value;
			}

			belle_sdp_session_description_add_attribute(session_desc,
			                                            belle_sdp_attribute_create("acap", acapValue.c_str()));
		}

		std::string tcapValue;
		SalStreamDescription::tcap_map_t::key_type prevIdx = 0;
		for (const auto &[idx, value] : tcaps) {
			if (params.tcapLinesMerged()) {
				if (tcapValue.empty()) {
					tcapValue = std::to_string(idx) + " " + value;
					prevIdx = idx;
				} else {
					if (idx == (prevIdx + 1)) {
						tcapValue += " " + value;
					} else {
						belle_sdp_session_description_add_attribute(
						    session_desc, belle_sdp_attribute_create("tcap", tcapValue.c_str()));
						tcapValue = std::to_string(idx) + " " + value;
					}
					prevIdx = idx;
				}
			} else {
				tcapValue = std::to_string(idx) + " " + value;
				belle_sdp_session_description_add_attribute(session_desc,
				                                            belle_sdp_attribute_create("tcap", tcapValue.c_str()));
			}
		}

		if (params.tcapLinesMerged() && !tcapValue.empty()) {
			belle_sdp_session_description_add_attribute(session_desc,
			                                            belle_sdp_attribute_create("tcap", tcapValue.c_str()));
		}
	}

	for (const auto &stream : streams) {
		auto media_desc = stream.toSdpMediaDescription(this, session_desc);
		belle_sdp_session_description_add_media_description(session_desc, media_desc);
	}
	return session_desc;
}

void SalMediaDescription::addTcap(const unsigned int &idx, const std::string &value) {
	tcaps[idx] = value;
}

const std::string &SalMediaDescription::getTcap(const unsigned int &idx) const {
	try {
		return tcaps.at(idx);
	} catch (std::out_of_range &) {
		lError() << "Unable to find transport capability at index " << idx;
		return Utils::getEmptyConstRefObject<std::string>();
	}
}

void SalMediaDescription::addAcap(const unsigned int &idx, const std::string &name, const std::string &value) {
	acaps[idx] = std::make_pair(name, value);
}

const SalStreamDescription::acap_t &SalMediaDescription::getAcap(const unsigned int &idx) const {
	try {
		return acaps.at(idx);
	} catch (std::out_of_range &) {
		lError() << "Unable to find attribute capability at index " << idx;
		return Utils::getEmptyConstRefObject<SalStreamDescription::acap_t>();
	}
}

const SalStreamDescription::acap_map_t &SalMediaDescription::getAcaps() const {
	return acaps;
}

const SalStreamDescription::tcap_map_t &SalMediaDescription::getTcaps() const {
	return tcaps;
}
const SalStreamDescription::cfg_map SalMediaDescription::getCfgsForStream(const unsigned int &idx) const {
	SalStreamDescription::cfg_map cfgs;
	const SalStreamDescription &stream = getStreamAtIdx(idx);
	if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		cfgs = stream.getAllCfgs();
	}
	return cfgs;
}

const SalStreamDescription::acap_map_t SalMediaDescription::getAllAcapForStream(const unsigned int &idx) const {
	SalStreamDescription::acap_map_t allAcaps;
	const SalStreamDescription &stream = getStreamAtIdx(idx);
	if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		allAcaps = stream.getAcaps();
		auto globalAcaps = getAcaps();
		allAcaps.insert(globalAcaps.begin(), globalAcaps.end());
	}
	return allAcaps;
}
const SalStreamDescription::tcap_map_t SalMediaDescription::getAllTcapForStream(const unsigned int &idx) const {
	SalStreamDescription::tcap_map_t allTcaps;
	const SalStreamDescription &stream = getStreamAtIdx(idx);
	if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		allTcaps = stream.getTcaps();
		auto globalTcaps = getTcaps();
		allTcaps.insert(globalTcaps.begin(), globalTcaps.end());
	}
	return allTcaps;
}

void SalMediaDescription::addTcapToStream(const std::size_t &streamIdx,
                                          const unsigned int &idx,
                                          const std::string &value) {
	if (streamIdx < streams.size()) {
		streams[streamIdx].addTcap(idx, value);
	}
}

void SalMediaDescription::addAcapToStream(const std::size_t &streamIdx,
                                          const unsigned int &idx,
                                          const std::string &name,
                                          const std::string &value) {
	if (streamIdx < streams.size()) {
		streams[streamIdx].addAcap(idx, name, value);
	}
}

void SalMediaDescription::createPotentialConfigurationsForStream(const unsigned int &streamIdx,
                                                                 const bool delete_session_attributes,
                                                                 const bool delete_media_attributes) {

	try {
		SalStreamDescription &stream = streams.at(streamIdx);
		const auto allStreamAcaps = getAllAcapForStream(streamIdx);
		const auto allStreamTcaps = getAllTcapForStream(streamIdx);
		if (!allStreamAcaps.empty() || !allStreamTcaps.empty()) {
			if (allStreamTcaps.empty()) {
				const SalStreamDescription::tcap_map_t proto;
				stream.createPotentialConfiguration(proto, {allStreamAcaps}, delete_session_attributes,
				                                    delete_media_attributes, params.cfgLinesMerged());
			} else {
				for (const auto &protoPair : allStreamTcaps) {
					const SalStreamDescription::tcap_map_t proto{{protoPair}};
					stream.createPotentialConfiguration(proto, {allStreamAcaps}, delete_session_attributes,
					                                    delete_media_attributes, params.cfgLinesMerged());
				}
			}
		} else {
			lInfo() << "Unable to create potential configuration for stream " << streamIdx
			        << " because it doesn't have acap and tcap attributes";
		}
	} catch (std::out_of_range &) {
		lError() << "Unable to create potential configuration for stream " << streamIdx << " because it doesn't exists";
	}
}

unsigned int SalMediaDescription::getFreeTcapIdx() const {
	std::list<unsigned int> tcapIndexes;
	auto addToIndexList = [&tcapIndexes](const auto &cap) { tcapIndexes.push_back(cap.first); };
	const auto &globalTcaps = getTcaps();
	std::for_each(globalTcaps.begin(), globalTcaps.end(), addToIndexList);
	for (const auto &stream : streams) {
		const auto &streamTcaps = stream.getTcaps();
		std::for_each(streamTcaps.begin(), streamTcaps.end(), addToIndexList);
	}

	return PotentialCfgGraph::getFreeIdx(tcapIndexes);
}

unsigned int SalMediaDescription::getFreeAcapIdx() const {
	std::list<unsigned int> acapIndexes;
	auto addToIndexList = [&acapIndexes](const auto &cap) { acapIndexes.push_back(cap.first); };
	const auto &globalAcaps = getAcaps();
	std::for_each(globalAcaps.begin(), globalAcaps.end(), addToIndexList);
	for (const auto &stream : streams) {
		const auto &streamAcaps = stream.getAcaps();
		std::for_each(streamAcaps.begin(), streamAcaps.end(), addToIndexList);
	}

	return PotentialCfgGraph::getFreeIdx(acapIndexes);
}

LINPHONE_END_NAMESPACE
