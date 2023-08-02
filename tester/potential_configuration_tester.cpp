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

#include "liblinphone_tester.h"
#include "sal/potential_config_graph.h"

using namespace LinphonePrivate;

struct acapParts {
	const std::string name;
	const std::string value;
};

struct acapCfgParts {
	const unsigned int idx;
	const bool mandatory;
};

static std::map<int, std::string> fillTcapMap(const bctbx_list_t *currentTcap, const int &expProtoCap) {
	std::map<int, std::string> protoList;
	for (auto it = currentTcap; it != NULL; it = it->next) {
		auto attr = static_cast<belle_sdp_tcap_attribute_t *>(bctbx_list_get_data(it));
		auto id = belle_sdp_tcap_attribute_get_id(attr);
		auto tcapList = belle_sdp_tcap_attribute_get_protos(attr);
		auto protoId = id;
		for (auto list = tcapList; list != NULL; list = list->next) {
			auto proto = static_cast<const char *>(bctbx_list_get_data(list));
			protoList[protoId] = proto;
			protoId++;
		}
	}
	BC_ASSERT_EQUAL(protoList.size(), expProtoCap, std::size_t, "%0zu");
	return protoList;
}

static void checkAcap(const PotentialCfgGraph::media_description_acap acap,
                      const std::size_t &expNoAcap,
                      const std::map<int, acapParts> &expAcapAttrs) {
	BC_ASSERT_EQUAL(acap.size(), expNoAcap, std::size_t, "%0zu");
	for (const auto &cap : acap) {
		auto id = cap->index;
		auto expIt = expAcapAttrs.find(id);
		BC_ASSERT_TRUE(expIt != expAcapAttrs.end());
		if (expIt != expAcapAttrs.end()) {
			const auto &value = cap->value;
			const auto &expValue = expAcapAttrs.at(id).value;
			BC_ASSERT_STRING_EQUAL(value.c_str(), expValue.c_str());

			const auto &name = cap->name;
			const auto &expName = expAcapAttrs.at(id).name;
			BC_ASSERT_STRING_EQUAL(name.c_str(), expName.c_str());
		}
	}
}

static void checkTcap(const PotentialCfgGraph::media_description_base_cap tcap,
                      const std::size_t &expNoTcap,
                      std::map<int, std::string> &expTcapProtos) {
	BC_ASSERT_EQUAL(tcap.size(), expNoTcap, std::size_t, "%0zu");
	for (const auto &cap : tcap) {
		auto id = cap->index;
		auto expIt = expTcapProtos.find(id);
		BC_ASSERT_TRUE(expIt != expTcapProtos.end());
		if (expIt != expTcapProtos.end()) {
			auto proto = cap->value;
			auto expProto = expTcapProtos[id];
			BC_ASSERT_STRING_EQUAL(proto.c_str(), expProto.c_str());
		};
	}
}

static void checkCfg(const PotentialCfgGraph::media_description_config &cfg,
                     const std::size_t &expNoCfg,
                     std::map<int, std::list<acapCfgParts>> expCfgAcapAttrs,
                     std::map<int, std::list<unsigned int>> expCfgTcapAttrs,
                     std::map<int, acapParts> expAcapAttrs,
                     std::map<int, std::string> &expTcapProtos,
                     const bool expDeleteMediaAttributes,
                     const bool expDeleteSessionAttributes) {

	unsigned int noCfg = 0;
	for (const auto &cfgPair : cfg) {

		// Check ID exists in the expected config list
		auto cfgId = cfgPair.first;

		auto expCfgAcap = expCfgAcapAttrs.find(cfgId);
		BC_ASSERT_TRUE(expCfgAcap != expCfgAcapAttrs.end());

		auto expCfgTcap = expCfgTcapAttrs.find(cfgId);
		BC_ASSERT_TRUE(expCfgTcap != expCfgTcapAttrs.end());

		auto cfgAttr = cfgPair.second;

		BC_ASSERT_EQUAL(cfgAttr.delete_media_attributes, expDeleteMediaAttributes, int, "%0d");
		BC_ASSERT_EQUAL(cfgAttr.delete_session_attributes, expDeleteSessionAttributes, int, "%0d");

		if (expCfgAcap != expCfgAcapAttrs.end()) {
			for (const auto &acapCfg : cfgAttr.acap) {
				// Check acap
				const auto &acapReference = expCfgAcap->second;
				for (const auto &acap : acapCfg) {
					// Get shared pointer to capability
					const auto capPtr = acap.cap.lock();
					BC_ASSERT_PTR_NOT_NULL(capPtr.get());
					if (capPtr) {
						checkAcap({capPtr}, 1, expAcapAttrs);
						const auto &acapRef =
						    std::find_if(acapReference.begin(), acapReference.end(),
						                 [&capPtr](const acapCfgParts &part) { return (part.idx == capPtr->index); });
						BC_ASSERT_TRUE(acapRef != acapReference.end());
						if (acapRef != acapReference.end()) {
							BC_ASSERT_EQUAL(acapRef->idx, capPtr->index, int, "%0d");
							BC_ASSERT_EQUAL(acapRef->mandatory, acap.mandatory, int, "%0d");
						}
					}
				}
			}
		}

		if (expCfgTcap != expCfgTcapAttrs.end()) {
			// Check acap
			const auto &tcapCfg = cfgAttr.tcap;
			const auto &tcapReference = expCfgTcap->second;
			for (const auto &tcap : tcapCfg) {
				// Get shared pointer to capability
				const auto capPtr = tcap.cap.lock();
				BC_ASSERT_PTR_NOT_NULL(capPtr.get());
				if (capPtr) {
					checkTcap({capPtr}, 1, expTcapProtos);
					const auto &tcapRef = std::find(tcapReference.begin(), tcapReference.end(), capPtr->index);
					BC_ASSERT_TRUE(tcapRef != tcapReference.end());
				}
			}
		}

		const auto &acapSize = cfgAttr.acap.size();
		const auto acapValuesInCfg = (acapSize == 0) ? 1 : acapSize;
		const auto &tcapSize = cfgAttr.tcap.size();
		const auto tcapValuesInCfg = (tcapSize == 0) ? 1 : tcapSize;
		noCfg += (unsigned int)(tcapValuesInCfg * acapValuesInCfg);
	}
	BC_ASSERT_EQUAL(noCfg, expNoCfg, std::size_t, "%0zu");
}

static void base_test_with_potential_config(const char *src,
                                            const std::map<int, acapParts> &expAcapAttrs,
                                            const std::map<int, std::list<acapCfgParts>> &expCfgAcapAttrs,
                                            const std::map<int, std::list<unsigned int>> &expCfgTcapAttrs,
                                            const int expGlobalProtoCap,
                                            const int expGlobalTcap,
                                            const int expGlobalAcap,
                                            const std::vector<int> expMediaProtoCap,
                                            const std::vector<int> expMediaTcap,
                                            const std::vector<int> expMediaAcap,
                                            const std::vector<int> expCfg,
                                            const std::vector<bool> expDeleteMediaAttributes,
                                            const std::vector<bool> expDeleteSessionAttributes) {
	belle_sdp_session_description_t *sessionDescription = belle_sdp_session_description_parse(src);
	const auto mediaDescriptions = belle_sdp_session_description_get_media_descriptions(sessionDescription);
	const auto noMediaDescriptions = belle_sip_list_size(mediaDescriptions);

	PotentialCfgGraph graph(sessionDescription);

	auto notZero = [](const int &val) { return (val != 0); };

	BC_ASSERT_EQUAL(graph.getStreamAcap().size(), std::count_if(expMediaAcap.begin(), expMediaAcap.end(), notZero),
	                std::size_t, "%0zu");
	BC_ASSERT_EQUAL(graph.getStreamTcap().size(), std::count_if(expMediaTcap.begin(), expMediaTcap.end(), notZero),
	                std::size_t, "%0zu");
	BC_ASSERT_EQUAL(graph.getAllCfg().size(), std::count_if(expCfg.begin(), expCfg.end(), notZero), std::size_t,
	                "%0zu");

	// ACAP
	const auto globalAcap = belle_sdp_session_description_find_attributes_with_name(sessionDescription, "acap");
	const auto noGlobalAcap = belle_sip_list_size(globalAcap);
	belle_sip_list_free_with_data(const_cast<belle_sip_list_t *>(globalAcap), (void (*)(void *))belle_sip_object_unref);
	BC_ASSERT_EQUAL(noGlobalAcap, expGlobalAcap, std::size_t, "%0zu");

	const auto parsedGlobalAcap = graph.getGlobalAcap().size();
	BC_ASSERT_EQUAL(parsedGlobalAcap, expGlobalAcap, std::size_t, "%0zu");

	// TCAP
	const auto globalTcap = belle_sdp_session_description_find_attributes_with_name(sessionDescription, "tcap");
	BC_ASSERT_EQUAL(belle_sip_list_size(globalTcap), expGlobalTcap, std::size_t, "%0zu");
	auto protoList = fillTcapMap(globalTcap, expGlobalProtoCap);
	belle_sip_list_free_with_data(const_cast<belle_sip_list_t *>(globalTcap), (void (*)(void *))belle_sip_object_unref);
	auto noGlobalProtoCap = protoList.size();

	const auto parsedGlobalTcap = graph.getGlobalTcap().size();
	BC_ASSERT_EQUAL(parsedGlobalTcap, expGlobalProtoCap, std::size_t, "%0zu");

	auto mediaDescriptionElem = mediaDescriptions;
	for (std::size_t idx = 0; idx < noMediaDescriptions; idx++) {
		BC_ASSERT_PTR_NOT_NULL(mediaDescriptionElem);
		auto mediaDescription = static_cast<belle_sdp_media_description_t *>(bctbx_list_get_data(mediaDescriptionElem));

		// ACAP
		if (idx < expMediaAcap.size()) {
			const auto mediaAcap = belle_sdp_media_description_find_attributes_with_name(mediaDescription, "acap");
			const auto noMediaAcap = belle_sip_list_size(mediaAcap);
			belle_sip_list_free_with_data(const_cast<belle_sip_list_t *>(mediaAcap),
			                              (void (*)(void *))belle_sip_object_unref);
			BC_ASSERT_EQUAL(noMediaAcap, expMediaAcap.at(idx), std::size_t, "%0zu");
			if (expMediaAcap.at(idx) != 0) {
				const auto mediaAcapSize = graph.getMediaAcapForStream(static_cast<unsigned int>(idx)).size();
				BC_ASSERT_EQUAL(mediaAcapSize, expMediaAcap.at(idx), std::size_t, "%0zu");
				const auto acap = graph.getAllAcapForStream(static_cast<unsigned int>(idx));
				checkAcap(acap, (noGlobalAcap + noMediaAcap), expAcapAttrs);
			}
		}

		// TCAP
		if (idx < expMediaTcap.size()) {
			const auto mediaTcap = belle_sdp_media_description_find_attributes_with_name(mediaDescription, "tcap");
			BC_ASSERT_EQUAL(belle_sip_list_size(mediaTcap), expMediaTcap.at(idx), std::size_t, "%0zu");
			auto mediaProtoList = fillTcapMap(mediaTcap, expMediaProtoCap.at(idx));
			belle_sip_list_free_with_data(const_cast<belle_sip_list_t *>(mediaTcap),
			                              (void (*)(void *))belle_sip_object_unref);
			auto noMediaProtoCap = mediaProtoList.size();
			protoList.insert(mediaProtoList.begin(), mediaProtoList.end());

			if (expMediaTcap.at(idx) != 0) {
				const auto mediaTcapSize = graph.getMediaTcapForStream(static_cast<unsigned int>(idx)).size();
				BC_ASSERT_EQUAL(mediaTcapSize, expMediaProtoCap.at(idx), std::size_t, "%0zu");
				const auto tcap = graph.getAllTcapForStream(static_cast<unsigned int>(idx));
				checkTcap(tcap, (noGlobalProtoCap + noMediaProtoCap), protoList);
			}
		}

		// CFG
		if ((idx < expCfg.size()) && (expCfg.at(idx) != 0)) {
			auto cfgs = graph.getCfgForStream(static_cast<unsigned int>(idx));
			checkCfg(cfgs, expCfg.at(idx), expCfgAcapAttrs, expCfgTcapAttrs, expAcapAttrs, protoList,
			         expDeleteMediaAttributes.at(idx), expDeleteSessionAttributes.at(idx));
		}

		mediaDescriptionElem = mediaDescriptionElem->next;
	}

	belle_sip_object_unref(sessionDescription);
}

static const std::map<int, acapParts> expAcapAttrs = {
    {1, {"key-mgmt", "mikey AQAFgM"}},
    {20, {"ptime", "30"}},
    {59, {"crypto", "10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4"}},
    {10021, {"crypto", "1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4"}},
    {1001, {"crypto", "5 AES_CM_192_HMAC_SHA1_32 inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4"}},
    {8, {"ptime", "40"}},
    {9, {"ptime", "20"}},
    {4, {"ptime", "10"}},
};

static const std::map<int, std::list<acapCfgParts>> expCfgAcapAttrs = {
    {1, {{1, true}, {1001, true}}},
    {1475, {{20, true}, {59, true}}},
    {425, {{10021, true}, {8, true}}},
    {36825, {{4, true}, {9, true}}},
};

static const std::map<int, std::list<unsigned int>> expCfgTcapAttrs = {
    {1, {1}},
    {1475, {10}},
    {425, {66}},
    {36825, {65}},
};

static const char *simpleSdpWithNoCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_no_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithNoCapabilities, expAcapAttrs, expCfgAcapAttrs, expCfgTcapAttrs, 0, 0,
	                                0, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {false, false}, {false, false});
}

static const char *simpleSdpWithSingleCapabilityInSession =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=tcap:1 RTP/SAVP\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_single_capability_in_session(void) {
	base_test_with_potential_config(simpleSdpWithSingleCapabilityInSession, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 1, 1, 1, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {false, false},
	                                {false, false});
}

static const char *simpleSdpWithMultipleProtosOnSameLineInSession =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_multiple_capabilities_on_same_line_in_session(void) {
	base_test_with_potential_config(simpleSdpWithMultipleProtosOnSameLineInSession, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 3, 2, 2, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {false, false},
	                                {false, false});
}

static const char *simpleSdpWithMultipleCapabilitiesInSession =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:1 RTP/SAVP\r\n"
    "a=tcap:2 RTP/SAVPF\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_multiple_capabilities_in_session(void) {
	base_test_with_potential_config(simpleSdpWithMultipleCapabilitiesInSession, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 2, 2, 3, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {false, false},
	                                {false, false});
}

static const char *simpleSdpWithSingleCapabilityInMedia =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=tcap:1 RTP/SAVP\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_single_capability_in_media(void) {
	base_test_with_potential_config(simpleSdpWithSingleCapabilityInMedia, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 0, 0, 0, {1}, {1}, {1}, {0}, {false}, {false});
}

static const char *simpleSdpWithMultipleCapabilitiesInMedia =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:1 RTP/SAVP\r\n"
    "a=tcap:2 RTP/SAVPF\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_multiple_capabilities_in_media(void) {
	base_test_with_potential_config(simpleSdpWithMultipleCapabilitiesInMedia, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 0, 0, 0, {2}, {2}, {3}, {0}, {false}, {false});
}

static const char *simpleSdpWithMultipleProtosOnSameLineInMedia =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_multiple_capabilities_on_same_line_in_media(void) {
	base_test_with_potential_config(simpleSdpWithMultipleProtosOnSameLineInMedia, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 0, 0, 0, {3}, {2}, {2}, {0}, {false}, {false});
}

static const char *simpleSdpWithSingleCapability =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=tcap:1 RTP/SAVP\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_single_capability(void) {
	base_test_with_potential_config(simpleSdpWithSingleCapability, expAcapAttrs, expCfgAcapAttrs, expCfgTcapAttrs, 1, 1,
	                                1, {1}, {1}, {1}, {0}, {false}, {false});
}

static const char *simpleSdpWithMultipleCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:1 RTP/SAVP\r\n"
    "a=tcap:2 RTP/SAVPF\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_multiple_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithMultipleCapabilities, expAcapAttrs, expCfgAcapAttrs, expCfgTcapAttrs,
	                                1, 1, 1, {2}, {2}, {3}, {0}, {false}, {false});
}

static const char *simpleSdpWithMultipleProtosOnSameLine =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "b=AS:380\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_multiple_capabilities_on_same_line(void) {
	base_test_with_potential_config(simpleSdpWithMultipleProtosOnSameLine, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {2}, {0}, {false}, {false});
}

static const char *simpleSdpWithOnePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_acfg(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialAConfig, expAcapAttrs, expCfgAcapAttrs, expCfgTcapAttrs, 1,
	                                1, 1, {3}, {2}, {3}, {1}, {false}, {false});
}

static const char *simpleSdpWithOnePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialPConfig, expAcapAttrs, expCfgAcapAttrs, expCfgTcapAttrs, 1,
	                                1, 1, {3}, {2}, {3}, {1}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,1 t=1\r\n"
    "a=acfg:1475 a=20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialAConfig, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=20,59 t=10\r\n"
    "a=pcfg:1 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialPConfig, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {false}, {false});
}

static const char *simpleSdpWithCapabilitiesReferredInAConfigBeforeDefinition =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acfg:1475 a=20,59 t=10\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_capability_referenced_by_acfg_before_defined(void) {
	base_test_with_potential_config(simpleSdpWithCapabilitiesReferredInAConfigBeforeDefinition, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 0, 0, 2, {4}, {3}, {2}, {2}, {false}, {false});
}

static const char *simpleSdpWithCapabilitiesReferredInPConfigBeforeDefinition =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=pcfg:1475 a=20,59 t=10\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=pcfg:1 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_capability_referenced_by_pcfg_before_defined(void) {
	base_test_with_potential_config(simpleSdpWithCapabilitiesReferredInPConfigBeforeDefinition, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 0, 0, 2, {4}, {3}, {2}, {2}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialAConfigInMultipleStreams =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:36825 a=9,4 t=65\r\n"
    "a=acfg:425 a=10021,8 t=66\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,1 t=1\r\n"
    "a=acfg:1475 a=20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_in_multiple_streams(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialAConfigInMultipleStreams, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 2, {3, 3}, {2, 2}, {3, 3}, {2, 2},
	                                {false, false}, {false, false});
}

static const char *simpleSdpWithMultiplePotentialPConfigInMultipleStreams =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=pcfg:36825 a=9,4 t=65\r\n"
    "a=pcfg:425 a=10021,8 t=66\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=20,59 t=10\r\n"
    "a=pcfg:1 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_in_multiple_streams(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialPConfigInMultipleStreams, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 2, {3, 3}, {2, 2}, {3, 3}, {2, 2},
	                                {false, false}, {false, false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:36825 a=59,4 t=1\r\n"
    "a=acfg:425 a=10021,8 t=66\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,1 t=49\r\n"
    "a=acfg:1475 a=20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_invalid_reference_to_potential_capability_in_acfg(void) {
	base_test_with_potential_config(simpleSdpWithInvalidPotentialReferenceInAConfig, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 1, 1, 2, {3, 3}, {2, 2}, {3, 3}, {1, 1}, {false, false},
	                                {false, false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=pcfg:36825 a=9,4 t=65\r\n"
    "a=pcfg:425 a=10021,8 t=1\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=20,9 t=10\r\n"
    "a=pcfg:1 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_invalid_reference_to_potential_capability_in_pcfg(void) {
	base_test_with_potential_config(simpleSdpWithInvalidPotentialReferenceInPConfig, expAcapAttrs, expCfgAcapAttrs,
	                                expCfgTcapAttrs, 1, 1, 2, {3, 3}, {2, 2}, {3, 3}, {1, 1}, {false, false},
	                                {false, false});
}

static const char *simpleSdpWithOnePotentialAConfigWithAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=20,59|1001 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static const std::map<int, std::list<acapCfgParts>> expCfgAcapAttrsWithAlternatives = {
    {1, {{1, true}, {1001, true}, {59, true}, {20, true}}},
    {1475, {{20, true}, {59, true}, {1001, true}}},
    {425, {{10021, true}, {8, true}}},
    {36825, {{4, true}, {9, true}}},
    {999, {{10021, true}, {1, true}, {1001, true}}},
    {2, {{1001, true}, {1, true}}},
};

static void test_with_one_acfg_with_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialAConfigWithAlternatives, expAcapAttrs,
	                                expCfgAcapAttrsWithAlternatives, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2},
	                                {false}, {false});
}

static const char *simpleSdpWithOnePotentialPConfigWithAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=20,59|1001 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg_with_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialPConfigWithAlternatives, expAcapAttrs,
	                                expCfgAcapAttrsWithAlternatives, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2},
	                                {false}, {false});
}

static const std::map<int, std::list<unsigned int>> expCfgTcapAttrsWithAlternatives = {
    {1, {1, 19}}, {1475, {10}}, {425, {10}}, {36825, {65, 66, 49}}, {999, {2, 10}}, {2, {1}},
};

static const char *simpleSdpWithMultiplePotentialAConfigWithAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,1|59|20 t=1|19\r\n"
    "a=acfg:1475 a=20,59|1001 t=10\r\n"
    "a=acfg:36825 a=4|9 t=65|49|66\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_with_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialAConfigWithAlternatives, expAcapAttrs,
	                                expCfgAcapAttrsWithAlternatives, expCfgTcapAttrsWithAlternatives, 1, 1, 1, {6}, {4},
	                                {5}, {14}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialPConfigWithAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:999 a=1,10021|1001 t=2|10\r\n"
    "a=pcfg:2 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_with_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialPConfigWithAlternatives, expAcapAttrs,
	                                expCfgAcapAttrsWithAlternatives, expCfgTcapAttrsWithAlternatives, 1, 1, 1, {3}, {2},
	                                {4}, {5}, {false}, {false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInAConfigWithAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:36825 a=59,4|9 t=2\r\n"
    "a=acfg:425 a=10021,8|20|8 t=10\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=acap:4 ptime:10\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acfg:1 a=1001,1|59 t=49\r\n"
    "a=acfg:1475 a=20,59|1001 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_invalid_reference_to_potential_capability_in_acfg_with_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithInvalidPotentialReferenceInAConfigWithAlternatives, expAcapAttrs,
	                                expCfgAcapAttrsWithAlternatives, expCfgTcapAttrsWithAlternatives, 1, 1, 2, {3, 3},
	                                {2, 2}, {3, 3}, {2, 2}, {false, false}, {false, false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInPConfigWithAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=pcfg:36825 a=9,4|59 t=65|66|49\r\n"
    "a=pcfg:425 a=10021,8|8 t=1\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=pcfg:1475 a=20,9|10021,8 t=10\r\n"
    "a=pcfg:1 a=1001,1|4 t=1|19\r\n"
    "a=pcfg:2 a=1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_invalid_reference_to_potential_capability_in_pcfg_with_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithInvalidPotentialReferenceInPConfigWithAlternatives, expAcapAttrs,
	                                expCfgAcapAttrsWithAlternatives, expCfgTcapAttrsWithAlternatives, 1, 1, 2, {3, 3},
	                                {2, 2}, {3, 3}, {3, 3}, {false, false}, {false, false});
}

static const char *simpleSdpWithOnePotentialAConfigWithOptionalCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=[20,59] t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static const std::map<int, std::list<acapCfgParts>> expCfgAcapAttrsWithOptionals = {
    {1, {{1, false}, {1001, true}, {59, false}, {20, true}}},
    {1475, {{20, false}, {59, false}, {1001, false}}},
    {425, {{10021, true}, {8, false}}},
    {36825, {{4, false}, {9, false}}},
    {999, {{10021, true}, {1, true}, {1001, false}}},
    {2, {{1001, true}, {1, true}}},
};

static const std::map<int, std::list<unsigned int>> expCfgTcapAttrsWithOptionals = {
    {1, {1, 19}}, {1475, {10}}, {425, {10}}, {36825, {65, 66, 49}}, {999, {2, 10}}, {2, {1}},
};

static void test_with_one_acfg_with_optional_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialAConfigWithOptionalCapabilities, expAcapAttrs,
	                                expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1, {3}, {2}, {3},
	                                {1}, {false}, {false});
}

static const char *simpleSdpWithOnePotentialPConfigWithOptionalCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=[20,59,1001] t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg_with_optional_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialPConfigWithOptionalCapabilities, expAcapAttrs,
	                                expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1, {3}, {2}, {3},
	                                {1}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialAConfigWithOptionalCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,[1],20,[59] t=1\r\n"
    "a=acfg:1475 a=[20,59] t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_with_optional_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialAConfigWithOptionalCapabilities, expAcapAttrs,
	                                expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1, {3}, {2}, {3},
	                                {2}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialPConfigWithOptionalCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=[20,59,1001] t=10\r\n"
    "a=pcfg:1 a=1001,[1] t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_with_optional_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialPConfigWithOptionalCapabilities, expAcapAttrs,
	                                expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1, {3}, {2}, {3},
	                                {2}, {false}, {false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInAConfigWithOptionalCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:36825 a=[9] t=1\r\n"
    "a=acfg:425 a=10021,[8] t=10\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=20,[1,59],1001 t=1\r\n"
    "a=acfg:1475 a=[20,59] t=20\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_invalid_reference_to_potential_capability_in_acfg_with_optional_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithInvalidPotentialReferenceInAConfigWithOptionalCapabilities,
	                                expAcapAttrs, expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 2,
	                                {3, 3}, {2, 2}, {3, 3}, {1, 1}, {false, false}, {false, false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInPConfigWithOptionalCapabilities =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=pcfg:36825 a=[9,4] t=49\r\n"
    "a=pcfg:425 a=10021,[8] t=20\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=20,9 t=10\r\n"
    "a=pcfg:1 a=1001,[1] t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_invalid_reference_to_potential_capability_in_pcfg_with_optional_capabilities(void) {
	base_test_with_potential_config(simpleSdpWithInvalidPotentialReferenceInPConfigWithOptionalCapabilities,
	                                expAcapAttrs, expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 2,
	                                {3, 3}, {2, 2}, {3, 3}, {1, 1}, {false, false}, {false, false});
}

static const char *simpleSdpWithOnePotentialAConfigWithOptionalCapabilitiesAndAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=[20]|[59,1001] t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_acfg_with_optional_capabilities_and_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialAConfigWithOptionalCapabilitiesAndAlternatives,
	                                expAcapAttrs, expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1,
	                                {3}, {2}, {3}, {2}, {false}, {false});
}

static const char *simpleSdpWithOnePotentialPConfigWithOptionalCapabilitiesAndAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=[1001,59]|[20] t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg_with_optional_capabilities_and_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithOnePotentialPConfigWithOptionalCapabilitiesAndAlternatives,
	                                expAcapAttrs, expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1,
	                                {3}, {2}, {3}, {2}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialAConfigWithOptionalCapabilitiesAndAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=1001,20|[59]|[1] t=1|19\r\n"
    "a=acfg:1475 a=[1001,59] t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_with_optional_capabilities_and_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialAConfigWithOptionalCapabilitiesAndAlternatives,
	                                expAcapAttrs, expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1,
	                                {3}, {2}, {3}, {7}, {false}, {false});
}

static const char *simpleSdpWithMultiplePotentialPConfigWithOptionalCapabilitiesAndAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=[1001,59] t=10\r\n"
    "a=pcfg:1 a=1001,20 t=1|19\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_with_optional_capabilities_and_alternatives(void) {
	base_test_with_potential_config(simpleSdpWithMultiplePotentialPConfigWithOptionalCapabilitiesAndAlternatives,
	                                expAcapAttrs, expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 1,
	                                {3}, {2}, {3}, {3}, {false}, {false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInAConfigWithOptionalCapabilitiesAndAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:36825 a=[59,4]|9 t=1\r\n"
    "a=acfg:425 a=[8],10021|20 t=10\r\n"
    "a=acfg:999 a=[1001],10021|59 t=10\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=[1001,1,10021]|59 t=49\r\n"
    "a=acfg:1475 a=[20] t=10\r\n"
    "a=acfg:999 a=1,10021 t=2|987\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void
test_with_invalid_reference_to_potential_capability_in_acfg_with_optional_capabilities_and_alternatives(void) {
	base_test_with_potential_config(
	    simpleSdpWithInvalidPotentialReferenceInAConfigWithOptionalCapabilitiesAndAlternatives, expAcapAttrs,
	    expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 2, {3, 3}, {2, 2}, {3, 3}, {2, 2},
	    {false, false}, {false, false});
}

static const char *simpleSdpWithInvalidPotentialReferenceInPConfigWithOptionalCapabilitiesAndAlternatives =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=pcfg:36825 a=[9,4]|59 t=65|66|49\r\n"
    "a=pcfg:425 a=10021,[8]|[8] t=1\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=[20,59,1001] t=10\r\n"
    "a=pcfg:1 a=1001,[1]|4 t=1|19\r\n"
    "a=pcfg:999 a=1001,1,1021|9 t=2\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void
test_with_invalid_reference_to_potential_capability_in_pcfg_with_optional_capabilities_and_alternatives(void) {
	base_test_with_potential_config(
	    simpleSdpWithInvalidPotentialReferenceInPConfigWithOptionalCapabilitiesAndAlternatives, expAcapAttrs,
	    expCfgAcapAttrsWithOptionals, expCfgTcapAttrsWithOptionals, 1, 1, 2, {3, 3}, {2, 2}, {3, 3}, {3, 3},
	    {false, false}, {false, false});
}

static const char *simpleSdpWithMediaDeleteAttributeInOnePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=-m:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_acfg_with_media_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaDeleteAttributeInOnePotentialAConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {1}, {true}, {false});
}

static const char *simpleSdpWithMediaDeleteAttributeInOnePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-m:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg_with_media_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaDeleteAttributeInOnePotentialPConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {1}, {true}, {false});
}

static const char *simpleSdpWithMediaDeleteAttributeInMultiplePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=-m:1001,1 t=1\r\n"
    "a=acfg:1475 a=-m:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_with_media_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaDeleteAttributeInMultiplePotentialAConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {true}, {false});
}

static const char *simpleSdpWithMediaDeleteAttributeInMultiplePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-m:20,59 t=10\r\n"
    "a=pcfg:1 a=-m:1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_with_media_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaDeleteAttributeInMultiplePotentialPConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {true}, {false});
}

static const char *simpleSdpWithSessionDeleteAttributeInOnePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=-s:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_acfg_with_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithSessionDeleteAttributeInOnePotentialAConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {1}, {false}, {true});
}

static const char *simpleSdpWithSessionDeleteAttributeInOnePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-s:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg_with_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithSessionDeleteAttributeInOnePotentialPConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {1}, {false}, {true});
}

static const char *simpleSdpWithSessionDeleteAttributeInMultiplePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=-s:1001,1 t=1\r\n"
    "a=acfg:1475 a=-s:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_with_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithSessionDeleteAttributeInMultiplePotentialAConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {false}, {true});
}

static const char *simpleSdpWithSessionDeleteAttributeInMultiplePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-s:20,59 t=10\r\n"
    "a=pcfg:1 a=-s:1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_with_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithSessionDeleteAttributeInMultiplePotentialPConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {false}, {true});
}

static const char *simpleSdpWithMediaSessionDeleteAttributeInOnePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=-ms:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_acfg_with_media_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaSessionDeleteAttributeInOnePotentialAConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {1}, {true}, {true});
}

static const char *simpleSdpWithMediaSessionDeleteAttributeInOnePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-ms:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_one_pcfg_with_media_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaSessionDeleteAttributeInOnePotentialPConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {1}, {true}, {true});
}

static const char *simpleSdpWithMediaSessionDeleteAttributeInMultiplePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1 a=-ms:1001,1 t=1\r\n"
    "a=acfg:1475 a=-ms:20,59 t=10\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_acfg_with_media_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaSessionDeleteAttributeInMultiplePotentialAConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {true}, {true});
}

static const char *simpleSdpWithMediaSessionDeleteAttributeInMultiplePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-ms:20,59 t=10\r\n"
    "a=pcfg:1 a=-ms:1001,1 t=1\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_multiple_pcfg_with_media_session_delete_attribute(void) {
	base_test_with_potential_config(simpleSdpWithMediaSessionDeleteAttributeInMultiplePotentialPConfig, expAcapAttrs,
	                                expCfgAcapAttrs, expCfgTcapAttrs, 1, 1, 1, {3}, {2}, {3}, {2}, {true}, {true});
}

static const char *complexSdpWithMultiplePotentialPConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:91 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/AVP RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:7799 RTP/AVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:36825 a=-ms:[9,4] t=66|49|65\r\n"
    "a=pcfg:425 a=-ms:10021|[8] t=10\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:97 key-mgmt:mikey RJGGgneojf\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 UDP/TLS/RTP/SAVP RTP/AVP RTP/AVPF RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:1475 a=-m:[20,59]|[1001] t=10\r\n"
    "a=pcfg:1 a=-m:[1001,1,59,20] t=1|19\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=pcfg:5761 a=-m:[97],[59],1001,20 t=1|2|3|4|5\r\n"
    "a=pcfg:4601 a=-m:[97,59]|10021 t=91|92\r\n"
    "m=text 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:9999 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:9799 RTP/AVP\r\n"
    "a=tcap:3799 RTP/AVPF\r\n"
    "a=acap:89 ptime:40\r\n"
    "a=acap:44 ptime:10\r\n"
    "a=acap:91 ptime:20\r\n"
    "a=tcap:799 UDP/TLS/RTP/SAVP UDP/TLS/RTP/SAVPF\r\n"
    "a=pcfg:999 a=-s:10021,[1001]|89 t=9999|799\r\n"
    "a=pcfg:2 a=-s:1001,91 t=10000\r\n"
    "a=pcfg:1000 a=-s:[44]|89|[91] t=92\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static const std::map<int, std::list<acapCfgParts>> expCfgAcapAttrsComplexSdp = {
    {1, {{1, false}, {1001, false}, {59, false}, {20, false}}},
    {1475, {{20, false}, {59, false}, {1001, false}}},
    {425, {{10021, true}, {8, false}}},
    {36825, {{4, false}, {9, false}}},
    {999, {{10021, true}, {89, true}, {1001, false}}},
    {1000, {{44, false}, {89, true}, {91, false}}},
    {2, {{1001, true}, {91, true}}},
    {5761, {{97, false}, {1001, true}, {59, false}, {20, true}}},
    {4601, {{97, false}, {59, false}, {10021, true}}},
};

static const std::map<int, std::list<unsigned int>> expCfgTcapAttrsComplexSdp = {
    {1, {1, 19}},     {1475, {10}}, {425, {10}},  {36825, {65, 66, 49}}, {999, {9999, 799}}, {5761, {1, 2, 3, 4, 5}},
    {4601, {91, 92}}, {1000, {92}}, {2, {10000}},
};

static const std::map<int, acapParts> expAcapAttrsComplexSdp = {
    {1, {"key-mgmt", "mikey AQAFgM"}},
    {20, {"ptime", "30"}},
    {59, {"crypto", "10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4"}},
    {10021, {"crypto", "1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4"}},
    {1001, {"crypto", "5 AES_CM_192_HMAC_SHA1_32 inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4"}},
    {8, {"ptime", "40"}},
    {9, {"ptime", "20"}},
    {4, {"ptime", "10"}},
    {89, {"ptime", "40"}},
    {91, {"ptime", "20"}},
    {44, {"ptime", "10"}},
    {97, {"key-mgmt", "mikey RJGGgneojf"}},
};

static void test_with_complex_sdp_and_multiple_pcfg(void) {
	base_test_with_potential_config(complexSdpWithMultiplePotentialPConfig, expAcapAttrsComplexSdp,
	                                expCfgAcapAttrsComplexSdp, expCfgTcapAttrsComplexSdp, 3, 2, 2, {5, 6, 6}, {3, 2, 4},
	                                {3, 4, 3}, {5, 13, 8}, {true, true, false}, {true, false, true});
}

static const char *complexSdpWithMultiplePotentialAConfig =
    "v=0\r\n"
    "o=jehan-mac 1239 1239 IN IP6 2a01:e35:1387:1020:6233:4bff:fe0b:5663\r\n"
    "s=SIP Talk\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "t=0 0\r\n"
    "a=ice-pwd:31ec21eb38b2ec6d36e8dc7b\r\n"
    "a=acap:1001 crypto:5 AES_CM_192_HMAC_SHA1_32 "
    "inline:CY/Dizd1QrlobZtgnigr0hWE+oDSx4S1F51Zpo4aZamN+8ZMdp8|2^20|1:4\r\n"
    "a=tcap:91 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:10 UDP/TLS/RTP/SAVP\r\n"
    "a=acap:10021 crypto:1 AES_CM_256_HMAC_SHA1_80 inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:65 RTP/AVP RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:7799 RTP/AVPF\r\n"
    "a=acap:8 ptime:40\r\n"
    "a=acap:4 ptime:10\r\n"
    "a=acap:9 ptime:20\r\n"
    "a=tcap:49 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:36825 a=-ms:[9,4] t=66|49|65\r\n"
    "a=acfg:425 a=-ms:10021|[8] t=10\r\n"
    "m=video 8078 RTP/AVP 99 97 98\r\n"
    "c=IN IP4 192.168.0.18\r\n"
    "b=AS:380\r\n"
    "a=acap:1 key-mgmt:mikey AQAFgM\r\n"
    "a=acap:97 key-mgmt:mikey RJGGgneojf\r\n"
    "a=acap:59 crypto:10 MS_AES_256_SHA1_80 inline:HjdHIU446fe64hnu6K446rkyMjA7fQp9CnVubGVz|2^20|1:4\r\n"
    "a=acap:20 ptime:30\r\n"
    "a=tcap:1 UDP/TLS/RTP/SAVP RTP/AVP RTP/AVPF RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:19 UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:1475 a=-m:[20,59]|[1001] t=10\r\n"
    "a=acfg:1 a=-m:[1001,1,59,20] t=1|19\r\n"
    "a=rtcp-fb:98 nack rpsi\r\n"
    "a=rtcp-xr:rcvr-rtt=all:10\r\n"
    "a=rtpmap:99 MP4V-ES/90000\r\n"
    "a=fmtp:99 profile-level-id=3\r\n"
    "a=rtpmap:97 theora/90000\r\n"
    "a=rtpmap:98 H263-1998/90000\r\n"
    "a=acfg:5761 a=-m:[97],[59],1001,20 t=1|2|3|4|5\r\n"
    "a=acfg:4601 a=-m:[97,59]|10021 t=91|92\r\n"
    "m=text 7078 RTP/AVP 111 110 3 0 8 101\r\n"
    "a=rtpmap:111 speex/16000\r\n"
    "a=fmtp:111 vbr=on\r\n"
    "a=rtpmap:110 speex/8000\r\n"
    "a=fmtp:110 vbr=on\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-11\r\n"
    "a=tcap:9999 RTP/SAVP RTP/SAVPF\r\n"
    "a=tcap:9799 RTP/AVP\r\n"
    "a=tcap:3799 RTP/AVPF\r\n"
    "a=acap:89 ptime:40\r\n"
    "a=acap:44 ptime:10\r\n"
    "a=acap:91 ptime:20\r\n"
    "a=tcap:799 UDP/TLS/RTP/SAVP UDP/TLS/RTP/SAVPF\r\n"
    "a=acfg:999 a=-s:10021,[1001]|89 t=9999|799\r\n"
    "a=acfg:2 a=-s:1001,91 t=10000\r\n"
    "a=acfg:1000 a=-s:[44]|89|[91] t=92\r\n"
    "a=fmtp:98 CIF=1;QCIF=1\r\n";

static void test_with_complex_sdp_and_multiple_acfg(void) {
	base_test_with_potential_config(complexSdpWithMultiplePotentialAConfig, expAcapAttrsComplexSdp,
	                                expCfgAcapAttrsComplexSdp, expCfgTcapAttrsComplexSdp, 3, 2, 2, {5, 6, 6}, {3, 2, 4},
	                                {3, 4, 3}, {5, 13, 8}, {true, true, false}, {true, false, true});
}

test_t potential_configuration_graph_tests[] = {
    TEST_NO_TAG("SDP with no capabilities", test_no_capabilities),
    TEST_NO_TAG("SDP with single capability in session", test_single_capability_in_session),
    TEST_NO_TAG("SDP with multiple capabilities in session", test_multiple_capabilities_in_session),
    TEST_NO_TAG("SDP with multiple capabilities on same line in session",
                test_multiple_capabilities_on_same_line_in_session),
    TEST_NO_TAG("SDP with single capability in media", test_single_capability_in_media),
    TEST_NO_TAG("SDP with multiple capabilities in media", test_multiple_capabilities_in_media),
    TEST_NO_TAG("SDP with multiple capabilities on same line in media",
                test_multiple_capabilities_on_same_line_in_media),
    TEST_NO_TAG("SDP with single capability", test_single_capability),
    TEST_NO_TAG("SDP with multiple capabilities", test_multiple_capabilities),
    TEST_NO_TAG("SDP with multiple capabilities on same line", test_multiple_capabilities_on_same_line),
    TEST_NO_TAG("SDP with one acfg", test_with_one_acfg),
    TEST_NO_TAG("SDP with one pcfg", test_with_one_pcfg),
    TEST_NO_TAG("SDP with multiple acfg", test_with_multiple_acfg),
    TEST_NO_TAG("SDP with multiple pcfg", test_with_multiple_pcfg),
    TEST_NO_TAG("SDP with capability referenced by acfg before it is defined",
                test_with_capability_referenced_by_acfg_before_defined),
    TEST_NO_TAG("SDP with capability referenced by pcfg before it is defined",
                test_with_capability_referenced_by_pcfg_before_defined),
    TEST_NO_TAG("SDP with multiple acfg in multiple streams", test_with_multiple_acfg_in_multiple_streams),
    TEST_NO_TAG("SDP with multiple pcfg in multiple streams", test_with_multiple_pcfg_in_multiple_streams),
    TEST_NO_TAG("SDP with invalid reference in acfg", test_with_invalid_reference_to_potential_capability_in_acfg),
    TEST_NO_TAG("SDP with invalid reference in pcfg", test_with_invalid_reference_to_potential_capability_in_pcfg),
    TEST_NO_TAG("SDP with one acfg with alternatives", test_with_one_acfg_with_alternatives),
    TEST_NO_TAG("SDP with one pcfg with alternatives", test_with_one_pcfg_with_alternatives),
    TEST_NO_TAG("SDP with multiple acfg with alternatives", test_with_multiple_acfg_with_alternatives),
    TEST_NO_TAG("SDP with multiple pcfg with alternatives", test_with_multiple_pcfg_with_alternatives),
    TEST_NO_TAG("SDP with invalid reference in acfg with alternatives",
                test_with_invalid_reference_to_potential_capability_in_acfg_with_alternatives),
    TEST_NO_TAG("SDP with invalid reference in pcfg with alternatives",
                test_with_invalid_reference_to_potential_capability_in_pcfg_with_alternatives),
    TEST_NO_TAG("SDP with one acfg with optional capabilities", test_with_one_acfg_with_optional_capabilities),
    TEST_NO_TAG("SDP with one pcfg with optional capabilities", test_with_one_pcfg_with_optional_capabilities),
    TEST_NO_TAG("SDP with multiple acfg with optional capabilities",
                test_with_multiple_acfg_with_optional_capabilities),
    TEST_NO_TAG("SDP with multiple pcfg with optional capabilities",
                test_with_multiple_pcfg_with_optional_capabilities),
    TEST_NO_TAG("SDP with invalid reference in acfg with optional capabilities",
                test_with_invalid_reference_to_potential_capability_in_acfg_with_optional_capabilities),
    TEST_NO_TAG("SDP with invalid reference in pcfg with optional capabilities",
                test_with_invalid_reference_to_potential_capability_in_pcfg_with_optional_capabilities),
    TEST_NO_TAG("SDP with one acfg with optional capabilities and alternatives",
                test_with_one_acfg_with_optional_capabilities_and_alternatives),
    TEST_NO_TAG("SDP with one pcfg with optional capabilities and alternatives",
                test_with_one_pcfg_with_optional_capabilities_and_alternatives),
    TEST_NO_TAG("SDP with multiple acfg with optional capabilities and alternatives",
                test_with_multiple_acfg_with_optional_capabilities_and_alternatives),
    TEST_NO_TAG("SDP with multiple pcfg with optional capabilities and alternatives",
                test_with_multiple_pcfg_with_optional_capabilities_and_alternatives),
    TEST_NO_TAG(
        "SDP with invalid reference in acfg with optional capabilities and alternatives",
        test_with_invalid_reference_to_potential_capability_in_acfg_with_optional_capabilities_and_alternatives),
    TEST_NO_TAG(
        "SDP with invalid reference in pcfg with optional capabilities and alternatives",
        test_with_invalid_reference_to_potential_capability_in_pcfg_with_optional_capabilities_and_alternatives),
    TEST_NO_TAG("SDP with one acfg with media delete attribute", test_with_one_acfg_with_media_delete_attribute),
    TEST_NO_TAG("SDP with one pcfg with media delete attribute", test_with_one_pcfg_with_media_delete_attribute),
    TEST_NO_TAG("SDP with multiple acfg with media delete attribute",
                test_with_multiple_acfg_with_media_delete_attribute),
    TEST_NO_TAG("SDP with multiple pcfg with media delete attribute",
                test_with_multiple_pcfg_with_media_delete_attribute),
    TEST_NO_TAG("SDP with one acfg with session delete attribute", test_with_one_acfg_with_session_delete_attribute),
    TEST_NO_TAG("SDP with one pcfg with session delete attribute", test_with_one_pcfg_with_session_delete_attribute),
    TEST_NO_TAG("SDP with multiple acfg with session delete attribute",
                test_with_multiple_acfg_with_session_delete_attribute),
    TEST_NO_TAG("SDP with multiple pcfg with session delete attribute",
                test_with_multiple_pcfg_with_session_delete_attribute),
    TEST_NO_TAG("SDP with one acfg with media and session delete attribute",
                test_with_one_acfg_with_media_session_delete_attribute),
    TEST_NO_TAG("SDP with one pcfg with media and session delete attribute",
                test_with_one_pcfg_with_media_session_delete_attribute),
    TEST_NO_TAG("SDP with multiple acfg with media and session delete attribute",
                test_with_multiple_acfg_with_media_session_delete_attribute),
    TEST_NO_TAG("SDP with multiple pcfg with media and session delete attribute",
                test_with_multiple_pcfg_with_media_session_delete_attribute),
    TEST_NO_TAG("SDP with complex SDP and multiple pcfgs", test_with_complex_sdp_and_multiple_pcfg),
    TEST_NO_TAG("SDP with complex SDP and multiple acfgs", test_with_complex_sdp_and_multiple_acfg),

};

test_suite_t potential_configuration_graph_test_suite = {"Potential configuration graph",
                                                         NULL,
                                                         NULL,
                                                         liblinphone_tester_before_each,
                                                         liblinphone_tester_after_each,
                                                         sizeof(potential_configuration_graph_tests) /
                                                             sizeof(potential_configuration_graph_tests[0]),
                                                         potential_configuration_graph_tests,
                                                         0};
