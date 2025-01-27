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

#include <bctoolbox/defs.h>

#include "c-wrapper/internal/c-sal.h"
#include "c-wrapper/internal/c-tools.h"
#include "offeranswer.h"
#include "private.h"
#include "sal/sal.h"
#include "sal/sal_media_description.h"
#include "sal/sal_stream_bundle.h"
#include "sal/sal_stream_configuration.h"
#include "utils/payload-type-handler.h"

static OrtpPayloadType *opus_match(BCTBX_UNUSED(MSOfferAnswerContext *ctx),
                                   const bctbx_list_t *local_payloads,
                                   const OrtpPayloadType *refpt,
                                   BCTBX_UNUSED(const bctbx_list_t *remote_payloads),
                                   BCTBX_UNUSED(bool_t reading_response)) {
	OrtpPayloadType *pt;
	const bctbx_list_t *elem;
	OrtpPayloadType *legacy_opus = NULL;

	for (elem = local_payloads; elem != NULL; elem = elem->next) {
		pt = (OrtpPayloadType *)elem->data;

		/*workaround a bug in earlier versions of linphone where opus/48000/1 is offered, which is uncompliant with opus
		 * rtp draft*/
		if (strcasecmp(pt->mime_type, "opus") == 0) {
			if (refpt->channels == 1) {
				legacy_opus = pt;
			} else if (refpt->channels == 2) {
				return payload_type_clone(pt);
			}
		}
	}
	if (legacy_opus) {
		legacy_opus = payload_type_clone(legacy_opus);
		legacy_opus->channels = 1; /*so that we respond with same number of channels */
		return legacy_opus;
	}
	return NULL;
}

static MSOfferAnswerContext *opus_offer_answer_create_context(void) {
	static MSOfferAnswerContext opus_oa = {opus_match, NULL, NULL};
	return &opus_oa;
}

MSOfferAnswerProvider opus_offer_answer_provider = {"opus", opus_offer_answer_create_context};

/* the reason for this matcher is for some stupid uncompliant phone that offer G729a mime type !*/
static OrtpPayloadType *g729A_match(BCTBX_UNUSED(MSOfferAnswerContext *ctx),
                                    const bctbx_list_t *local_payloads,
                                    const OrtpPayloadType *refpt,
                                    BCTBX_UNUSED(const bctbx_list_t *remotoe_payloads),
                                    BCTBX_UNUSED(bool_t reading_resonse)) {
	OrtpPayloadType *pt;
	const bctbx_list_t *elem;
	OrtpPayloadType *candidate = NULL;

	for (elem = local_payloads; elem != NULL; elem = elem->next) {
		pt = (OrtpPayloadType *)elem->data;

		if (strcasecmp(pt->mime_type, "G729") == 0 && refpt->channels == pt->channels) {
			candidate = pt;
		}
	}
	return candidate ? payload_type_clone(candidate) : NULL;
}

static MSOfferAnswerContext *g729a_offer_answer_create_context(void) {
	static MSOfferAnswerContext g729_oa = {g729A_match, NULL, NULL};
	return &g729_oa;
}

MSOfferAnswerProvider g729a_offer_answer_provider = {"G729A", g729a_offer_answer_create_context};

static OrtpPayloadType *red_match(BCTBX_UNUSED(MSOfferAnswerContext *ctx),
                                  const bctbx_list_t *local_payloads,
                                  BCTBX_UNUSED(const OrtpPayloadType *refpt),
                                  const bctbx_list_t *remote_payloads,
                                  BCTBX_UNUSED(bool_t reading_response)) {
	const bctbx_list_t *elem_local, *elem_remote;
	OrtpPayloadType *red = NULL;

	for (elem_local = local_payloads; elem_local != NULL; elem_local = elem_local->next) {
		OrtpPayloadType *pt = (OrtpPayloadType *)elem_local->data;

		if (strcasecmp(pt->mime_type, payload_type_t140_red.mime_type) == 0) {
			red = payload_type_clone(pt);

			for (elem_remote = remote_payloads; elem_remote != NULL; elem_remote = elem_remote->next) {
				OrtpPayloadType *pt2 = (OrtpPayloadType *)elem_remote->data;
				if (strcasecmp(pt2->mime_type, payload_type_t140.mime_type) == 0) {
					int t140_payload_number = payload_type_get_number(pt2);
					char *red_fmtp =
					    ms_strdup_printf("%i/%i/%i", t140_payload_number, t140_payload_number, t140_payload_number);
					/*modify the local payload and the return value*/
					payload_type_set_recv_fmtp(pt, red_fmtp);
					payload_type_set_recv_fmtp(red, red_fmtp);
					ms_free(red_fmtp);
					break;
				}
			}
			break;
		}
	}
	return red;
}

static MSOfferAnswerContext *red_offer_answer_create_context(void) {
	static MSOfferAnswerContext red_oa = {red_match, NULL, NULL};
	return &red_oa;
}

MSOfferAnswerProvider red_offer_answer_provider = {"red", red_offer_answer_create_context};

void linphone_core_register_offer_answer_providers(LinphoneCore *lc) {
	MSFactory *factory = lc->factory;
	ms_factory_register_offer_answer_provider(factory, &red_offer_answer_provider);
	ms_factory_register_offer_answer_provider(factory, &g729a_offer_answer_provider);
	ms_factory_register_offer_answer_provider(factory, &opus_offer_answer_provider);
}

LINPHONE_BEGIN_NAMESPACE

OfferAnswerEngine::OfferAnswerEngine(MSFactory *factory) : mMsFactory(factory) {
}

void OfferAnswerEngine::setOneMatchingCodecPolicy(bool value) {
	mUseOneMatchingCodec = value;
}

void OfferAnswerEngine::setAnswerWithOwnNumberingPolicy(bool value) {
	mAnswerWithOwnNumbering = value;
}

void OfferAnswerEngine::verifyBundles(const std::shared_ptr<SalMediaDescription> &local,
                                      const std::shared_ptr<SalMediaDescription> &remote,
                                      std::shared_ptr<SalMediaDescription> &result) {
	// Reject streams belonging to a bundle if the offerer tagged m section has been rejected (RFC8843 - Section 7.3.3)
	// It is not possible to do it while doing the offer-answer because the offerer-tagged stream may not be the first
	// of the bundle presented in the SDP We also must ensure that the result media description is coherent with the
	// local capabilities and the received offer
	for (size_t i = 0; i < result->streams.size(); ++i) {
		if (local->streams.size() > i) {
			auto &s = result->streams[i];
			int result_owner_index = result->getIndexOfTransportOwner(s);
			auto &ls = local->streams[i];
			int local_owner_index = local->getIndexOfTransportOwner(ls);
			auto &rs = remote->streams[i];
			int remote_owner_index = remote->getIndexOfTransportOwner(rs);
			// Disable stream if
			// - it belongs to a bundle and it is not the same as the one in local and remote SDP
			// - it doesn't belong to a bundle but both the offer and the answer do
			if (((result_owner_index >= 0) &&
			     ((local_owner_index != result_owner_index) || (remote_owner_index != result_owner_index))) ||
			    ((result_owner_index < 0) && (local_owner_index >= 0) && (remote_owner_index >= 0))) {
				s.disable();
			}
		}
	}
}

bool OfferAnswerEngine::onlyTelephoneEvent(const std::list<OrtpPayloadType *> &l) {
	for (const auto &p : l) {
		if (strcasecmp(p->mime_type, "telephone-event") != 0) {
			return false;
		}
	}
	return true;
}

OrtpPayloadType *OfferAnswerEngine::genericMatch(const std::list<OrtpPayloadType *> &local_payloads,
                                                 const OrtpPayloadType *refpt,
                                                 BCTBX_UNUSED(const std::list<OrtpPayloadType *> &remote_payloads)) {
	for (const auto &pt : local_payloads) {
		if (pt->mime_type && refpt->mime_type && strcasecmp(pt->mime_type, refpt->mime_type) == 0 &&
		    pt->clock_rate == refpt->clock_rate && pt->channels == refpt->channels)
			return payload_type_clone(pt);
	}
	return NULL;
}

/*
 * Returns a PayloadType from the local list that matches a OrtpPayloadType offered or answered in the remote list
 */
PayloadType *OfferAnswerEngine::findPayloadTypeBestMatch(const std::list<OrtpPayloadType *> &local_payloads,
                                                         const PayloadType *refpt,
                                                         const std::list<OrtpPayloadType *> &remote_payloads,
                                                         bool reading_response) {
	PayloadType *ret = NULL;
	MSOfferAnswerContext *ctx = NULL;

	// When a stream is inactive, refpt->mime_type might be null
	if (refpt->mime_type && (ctx = ms_factory_create_offer_answer_context(mMsFactory, refpt->mime_type))) {
		ms_message("Doing offer/answer processing with specific provider for codec [%s]", refpt->mime_type);
		auto local_payloads_list = Utils::listToBctbxList(local_payloads);
		auto remote_payloads_list = Utils::listToBctbxList(remote_payloads);
		ret = ms_offer_answer_context_match_payload(ctx, local_payloads_list, refpt, remote_payloads_list,
		                                            reading_response);
		bctbx_list_free(local_payloads_list);
		bctbx_list_free(remote_payloads_list);
		ms_offer_answer_context_destroy(ctx);
		return ret;
	}
	return OfferAnswerEngine::genericMatch(local_payloads, refpt, remote_payloads);
}

std::list<OrtpPayloadType *> OfferAnswerEngine::matchPayloads(const std::list<OrtpPayloadType *> &local,
                                                              const std::list<OrtpPayloadType *> &remote,
                                                              bool reading_response,
                                                              bool bundle_enabled) {
	std::list<OrtpPayloadType *> res;
	OrtpPayloadType *matched;
	bool found_codec = false;

	for (const auto &p2 : remote) {
		matched = findPayloadTypeBestMatch(local, p2, remote, reading_response);
		if (matched) {
			int local_number = payload_type_get_number(matched);
			int remote_number = payload_type_get_number(p2);

			if (mUseOneMatchingCodec) {
				if (strcasecmp(matched->mime_type, "telephone-event") != 0) {
					if (found_codec) { /* we have found a real codec already*/
						continue;      /*this codec won't be added*/
					} else found_codec = true;
				}
			}

			if (strcasecmp(matched->mime_type, "flexfec") == 0) {
				if (bundle_enabled == false) {
					continue;
				}
			}

			if (p2->send_fmtp) {
				payload_type_append_send_fmtp(matched, p2->send_fmtp);
			}
			payload_type_set_flag(matched, PAYLOAD_TYPE_FLAG_CAN_RECV | PAYLOAD_TYPE_FLAG_CAN_SEND);
			if (matched->flags & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED && p2->flags & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED) {
				payload_type_set_flag(matched, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				/* Negotiation of AVPF features (keep common features) */
				matched->avpf.features &= p2->avpf.features;
				matched->avpf.rpsi_compatibility = p2->avpf.rpsi_compatibility;
				/* Take bigger AVPF trr interval */
				if (p2->avpf.trr_interval > matched->avpf.trr_interval) {
					matched->avpf.trr_interval = p2->avpf.trr_interval;
				}
			} else {
				payload_type_unset_flag(matched, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
			}
			res.push_back(matched);
			/* we should use the remote numbering, even when parsing a response, but we need for testing
			 * to reproduce the minority case where we answer with our own numbering.
			 * This is tolerated by RFC3264.
			 */
			if (!reading_response && mAnswerWithOwnNumbering) {
				payload_type_set_number(matched, local_number);
			} else {
				payload_type_set_number(matched, remote_number);
			}
			payload_type_set_flag(matched, PAYLOAD_TYPE_FROZEN_NUMBER);
			if (reading_response && remote_number != local_number) {
				ms_warning("For payload type %s, proposed number was %i but the remote phone answered %i",
				           matched->mime_type, local_number, remote_number);
				/*
				 We must add this payload type with our local numbering in order to be able to receive it.
				 Indeed despite we must sent with the remote numbering, we must be able to receive with
				 our local one.
				*/
				matched = payload_type_clone(matched);
				payload_type_set_number(matched, local_number);
				payload_type_set_flag(matched, PAYLOAD_TYPE_FLAG_CAN_RECV);
				payload_type_unset_flag(matched, PAYLOAD_TYPE_FLAG_CAN_SEND);
				payload_type_set_flag(matched, PAYLOAD_TYPE_FROZEN_NUMBER);
				res.push_back(matched);
			}
		} else {
			if (p2->channels > 0) ms_message("No match for %s/%i/%i", p2->mime_type, p2->clock_rate, p2->channels);
			else ms_message("No match for %s/%i", p2->mime_type, p2->clock_rate);
		}
	}
	if (reading_response) {
		/* add remaning local payload as CAN_RECV only so that if we are in front of a non-compliant equipment we are
		 * still able to decode the RTP stream*/
		bool found = false;
		for (const auto &p1 : local) {
			for (const auto &p2 : remote) {
				if (payload_type_get_number(p2) == payload_type_get_number(p1)) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			const auto &p1 = local.front();
			if (p1) {
				ms_message("Adding %s/%i for compatibility, just in case.", p1->mime_type, p1->clock_rate);
				PayloadType *cloned_p1 = payload_type_clone(p1);
				payload_type_set_flag(cloned_p1, PAYLOAD_TYPE_FLAG_CAN_RECV);
				payload_type_set_flag(cloned_p1, PAYLOAD_TYPE_FROZEN_NUMBER);
				res.push_back(cloned_p1);
			} else {
				ms_error("Unable to add payload for compatibilty. Local has %zu payloads available", local.size());
			}
		}
	} else {
		/* case of generating an answer */
		/* resort result list to put high-priority codecs first */
		res.sort([](const OrtpPayloadType *p1, const OrtpPayloadType *p2) -> bool {
			if (p1->flags & PAYLOAD_TYPE_PRIORITY_BONUS && !(p2->flags & PAYLOAD_TYPE_PRIORITY_BONUS)) {
				return true;
			}
			return false;
		});
	}
	return res;
}

bool OfferAnswerEngine::matchCryptoAlgo(const std::vector<SalSrtpCryptoAlgo> &local,
                                        const std::vector<SalSrtpCryptoAlgo> &remote,
                                        SalSrtpCryptoAlgo &result,
                                        bool use_local_key) {
	for (const auto &rc : remote) {
		if (rc.algo == 0) break;
		/* Look for a local enabled crypto algo that matches one of the proposed by remote */
		for (const auto &lc : local) {
			if (rc.algo == lc.algo) {
				result.algo = rc.algo;
				/* We're answering an SDP offer. Supply our master key, associated with the remote supplied tag */
				if (use_local_key) {
					result.master_key = lc.master_key;
					result.tag = rc.tag;
				}
				/* We received an answer to our SDP crypto proposal. Copy matching algo remote master key to result, and
				   memorize local tag */
				else {
					result.master_key = rc.master_key;
					result.tag = lc.tag;
				}
				return true;
			}
		}
	}
	return false;
}

SalStreamDir OfferAnswerEngine::computeDirOutgoing(SalStreamDir local, SalStreamDir answered) {
	SalStreamDir res = local;
	if (local == SalStreamSendRecv) {
		if (answered == SalStreamRecvOnly) {
			res = SalStreamSendOnly;
		} else if (answered == SalStreamSendOnly) {
			res = SalStreamRecvOnly;
		}
	}
	if (answered == SalStreamInactive) {
		res = SalStreamInactive;
	}
	return res;
}

SalStreamDir OfferAnswerEngine::computeDirIncoming(SalStreamDir local, SalStreamDir offered) {
	SalStreamDir res = SalStreamSendRecv;
	if (local == SalStreamSendRecv) {
		if (offered == SalStreamSendOnly) res = SalStreamRecvOnly;
		else if (offered == SalStreamRecvOnly) res = SalStreamSendOnly;
		else if (offered == SalStreamInactive) res = SalStreamInactive;
		else res = SalStreamSendRecv;
	} else if (local == SalStreamSendOnly) {
		if (offered == SalStreamRecvOnly || offered == SalStreamSendRecv) res = SalStreamSendOnly;
		else res = SalStreamInactive;
	} else if (local == SalStreamRecvOnly) {
		if (offered == SalStreamSendOnly || offered == SalStreamSendRecv) res = SalStreamRecvOnly;
		else res = SalStreamInactive;
	} else res = SalStreamInactive;
	return res;
}

SalStreamDir OfferAnswerEngine::computeConferenceStreamDir(SalStreamDir dir) {
	SalStreamDir res = SalStreamSendRecv;
	switch (dir) {
		case SalStreamSendRecv:
			res = SalStreamSendRecv;
			break;
		case SalStreamSendOnly:
			res = SalStreamRecvOnly;
			break;
		case SalStreamRecvOnly:
			res = SalStreamSendOnly;
			break;
		case SalStreamInactive:
			res = SalStreamInactive;
			break;
	}
	return res;
}

SalStreamDescription OfferAnswerEngine::initiateOutgoingStream(const SalStreamDescription &local_offer,
                                                               const SalStreamDescription &remote_answer,
                                                               const bool allowCapabilityNegotiation) {

	SalStreamDescription result;
	result.setLabel(local_offer.getLabel());
	result.setContent(local_offer.getContent());
	result.type = local_offer.getType();
	if (local_offer.rtp_addr.empty() == false && ms_is_multicast(L_STRING_TO_C(local_offer.rtp_addr))) {
		/*6.2 Multicast Streams
		...
		If a multicast stream is accepted, the address and port information
		in the answer MUST match that of the offer.  Similarly, the
		directionality information in the answer (sendonly, recvonly, or
		sendrecv) MUST equal that of the offer.  This is because all
		participants in a multicast session need to have equivalent views of
		the parameters of the session, an underlying assumption of the
		multicast bias of RFC 2327.*/
		if (local_offer.rtp_addr.compare(remote_answer.rtp_addr) != 0) {
			ms_message("[Initiate Outgoing Stream] Remote answered IP [%s] does not match offered [%s] for local "
			           "stream description [%p]",
			           L_STRING_TO_C(remote_answer.rtp_addr), L_STRING_TO_C(local_offer.rtp_addr), &local_offer);
			result.rtp_port = 0;
			return result;
		}
		if (local_offer.rtp_port != remote_answer.rtp_port) {
			ms_message("[Initiate Outgoing Stream] Remote answered rtp port [%i] does not match offered [%i] for local "
			           "stream description [%p]",
			           remote_answer.rtp_port, local_offer.rtp_port, &local_offer);
			result.rtp_port = 0;
			return result;
		}
		if (local_offer.getDirection() != remote_answer.getDirection()) {
			ms_message("[Initiate Outgoing Stream] Remote answered dir [%s] does not match offered [%s] for local "
			           "stream description [%p]",
			           sal_stream_dir_to_string(remote_answer.getDirection()),
			           sal_stream_dir_to_string(local_offer.getDirection()), &local_offer);
			result.rtp_port = 0;
			return result;
		}
		if (local_offer.bandwidth != remote_answer.bandwidth) {
			ms_message("[Initiate Outgoing Stream] Remote answered bandwidth [%i] does not match offered [%i] for "
			           "local stream description [%p]",
			           remote_answer.bandwidth, local_offer.bandwidth, &local_offer);
			result.rtp_port = 0;
			return result;
		}
		result.multicast_role = SalMulticastSender;
	}

	if (remote_answer.enabled()) {
		OfferAnswerEngine::optional_sal_stream_configuration resultNegCfg;

		const auto remoteCfgIdx = remote_answer.getActualConfigurationIndex();
		unsigned int localCfgIdx = local_offer.getActualConfigurationIndex();
		if (allowCapabilityNegotiation) {
			auto answerUnparsedCfgs = remote_answer.unparsed_cfgs;
			// Keep only acfg in unparsed configuration list
			if (!answerUnparsedCfgs.empty()) {
				for (auto cfgIt = answerUnparsedCfgs.begin(); cfgIt != answerUnparsedCfgs.end();) {
					const auto &cfgLine = cfgIt->second;
					if (cfgLine.find("acfg") == std::string::npos) {
						lWarning() << "Unparsed configuration line " << cfgLine << " is not defining a acfg";
						cfgIt = answerUnparsedCfgs.erase(cfgIt);
					} else {
						++cfgIt;
					}
				}
			}

			// remote answer has only one configuration (the actual configuration). It contains acfg attribute if a
			// potential configuration has been selected from the offer. If not, the actual configuration from the local
			// offer has been selected
			if (answerUnparsedCfgs.empty()) {
				lInfo() << "[Initiate Outgoing Stream] Answerer chose offerer's actual configuration at index "
				        << localCfgIdx;
				localCfgIdx = local_offer.getActualConfigurationIndex();
				resultNegCfg = OfferAnswerEngine::initiateOutgoingConfiguration(local_offer, remote_answer, result,
				                                                                localCfgIdx, remoteCfgIdx);
			} else {
				if (answerUnparsedCfgs.size() > 1) {
					lError() << "[Initiate Outgoing Stream] The answer must contain only one potential configuration - "
					            "found "
					         << answerUnparsedCfgs.size()
					         << " instead - trying to use them if default negotiation with actual configuration failed";
				}
				for (const auto &cfg : answerUnparsedCfgs) {
					if (resultNegCfg) {
						break;
					} else {
						localCfgIdx = cfg.first;
						const auto cfgLine = cfg.second;
						// Perform negotiations only with acfg
						resultNegCfg = OfferAnswerEngine::initiateOutgoingConfiguration(
						    local_offer, remote_answer, result, localCfgIdx, remoteCfgIdx);
					}
				}
			}

			if (resultNegCfg) {
				lInfo() << "[Initiate Outgoing Stream] Found match between answerer's configuration and offerer "
				           "configuration at index "
				        << localCfgIdx;
			} else {
				lWarning() << "[Initiate Outgoing Stream] No match found between answerer's configuration and any of "
				              "offerers available configurations";
			}

		} else {
			localCfgIdx = local_offer.getActualConfigurationIndex();
			resultNegCfg = OfferAnswerEngine::initiateOutgoingConfiguration(local_offer, remote_answer, result,
			                                                                localCfgIdx, remoteCfgIdx);
		}

		if (resultNegCfg) {
			auto resultCfg = resultNegCfg.value();
			result.addActualConfiguration(resultCfg);
			remote_answer.cfgIndex = remoteCfgIdx;
			local_offer.cfgIndex = localCfgIdx;

			// finalize stream settings based on result configuration
			if (!resultCfg.payloads.empty() && !OfferAnswerEngine::onlyTelephoneEvent(resultCfg.payloads)) {
				result.rtp_addr = remote_answer.rtp_addr;
				result.rtp_port = remote_answer.rtp_port;
				result.rtcp_addr = remote_answer.rtcp_addr;
				result.rtcp_port = remote_answer.rtcp_port;
				result.bandwidth = remote_answer.bandwidth;
			} else {
				lWarning() << "Disable stream " << &result << " because "
				           << ((resultCfg.payloads.empty()) ? "payload is empty"
				                                            : " found event other than telephone one");
				result.disable();
			}

			lInfo() << "[Initiate Outgoing Stream] Found matching configurations: local offered configuration index "
			        << local_offer.cfgIndex << " remote configuration index " << remote_answer.cfgIndex;
		} else {
			lDebug() << "[Initiate Outgoing Stream] Unable to find a suitable configuration for stream of type "
			         << std::string(sal_stream_type_to_string(result.type));
			result.disable();
		}
	} else {
		lDebug() << "[Initiate Outgoing Stream] Remote stream is inactive";
		result.disable();
	}

	return result;
}

int OfferAnswerEngine::getExtensionId(int localExtensionId, int remoteExtensionId) {
	// If any of the two is set to 0 then one (or both) of the party does not support the extension.
	if (localExtensionId == 0 || remoteExtensionId == 0) return 0;

	// If they are not configured with the same id, then use the remote
	if (localExtensionId != remoteExtensionId) return remoteExtensionId;

	return localExtensionId;
}

OfferAnswerEngine::optional_sal_stream_configuration OfferAnswerEngine::initiateOutgoingConfiguration(
    const SalStreamDescription &local_offer,
    const SalStreamDescription &remote_answer,
    const SalStreamDescription &result,
    const PotentialCfgGraph::media_description_config::key_type &localCfgIdx,
    const PotentialCfgGraph::media_description_config::key_type &remoteCfgIdx) {
	SalStreamConfiguration resultCfg = result.getActualConfiguration();
	const SalStreamConfiguration &localCfg = local_offer.getConfigurationAtIndex(localCfgIdx);
	const SalStreamConfiguration &remoteCfg = remote_answer.getConfigurationAtIndex(remoteCfgIdx);

	if ((localCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>()) ||
	    (remoteCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>())) {
		lWarning() << "[Initiate Outgoing Configuration] Unable to find valid configurations to compare against - "
		              "local configuration found: "
		           << (localCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>())
		           << " remote configuration found: "
		           << (remoteCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>());
		return std::nullopt;
	}

	resultCfg.delete_media_attributes = localCfg.delete_media_attributes;
	resultCfg.delete_session_attributes = localCfg.delete_session_attributes;

	resultCfg.rtcp_mux = remoteCfg.rtcp_mux && localCfg.rtcp_mux;

	/* Bundle */
	bool bundle_enabled = false;
	if (remoteCfg.mid.empty() == false) {
		if (localCfg.mid.empty() == false) {
			resultCfg.mid = remoteCfg.mid;
			resultCfg.mid_rtp_ext_header_id = remoteCfg.mid_rtp_ext_header_id;
			resultCfg.bundle_only = remoteCfg.bundle_only;
			resultCfg.rtcp_mux = true; /* RTCP mux must be enabled in bundle mode. */
			bundle_enabled = true;
		} else {
			lError() << "The remote configuration at index " << remoteCfgIdx
			         << " has set a mid in an answer while local configuration " << localCfgIdx << " didn't offer it.";
		}
	}

	const auto &availableEncs = local_offer.getSupportedEncryptions();
	if (remoteCfg != Utils::getEmptyConstRefObject<SalStreamConfiguration>()) {
		resultCfg.payloads = matchPayloads(localCfg.payloads, remoteCfg.payloads, true, bundle_enabled);
	} else {
		lWarning() << "[Initiate Outgoing Configuration] Remote configuration has not been found";
		return std::nullopt;
	}

	if (OfferAnswerEngine::areProtoCompatibles(localCfg.getProto(), remoteCfg.getProto())) {
		if (localCfg.getProto() != remoteCfg.getProto() && localCfg.hasAvpf()) {
			lWarning() << "[Initiate Outgoing Configuration] Received a downgraded AVP answer (transport protocol "
			           << sal_media_proto_to_string(remoteCfg.getProto())
			           << " of the remote answer stream configuration at index " << remoteCfgIdx
			           << " for our AVPF offer (transport protocol " << sal_media_proto_to_string(localCfg.getProto())
			           << " of local offered stream configuration at index " << localCfgIdx << ")";
			const_cast<SalStreamConfiguration &>(localCfg).proto = remoteCfg.getProto();
		}
		resultCfg.proto = remoteCfg.getProto();
	} else {
		lWarning() << "[Initiate Outgoing Configuration] The transport protocol "
		           << sal_media_proto_to_string(localCfg.getProto())
		           << " of local offered stream configuration at index " << localCfgIdx
		           << " is not compatible with the transport protocol "
		           << sal_media_proto_to_string(remoteCfg.getProto()) << " of the remote stream configuration at index "
		           << remoteCfgIdx;
		return std::nullopt;
	}

	if (local_offer.rtp_addr.empty() == false && ms_is_multicast(L_STRING_TO_C(local_offer.rtp_addr))) {
		if (localCfg.ptime > 0 && localCfg.ptime != remoteCfg.ptime) {
			lWarning() << "[Initiate Outgoing Configuration] Remote answered ptime [" << remoteCfg.ptime
			           << "] does not match offered [" << localCfg.ptime << "] for local configuration " << localCfgIdx
			           << " [" << &local_offer << "]";
			return std::nullopt;
		}
		if (localCfg.ttl > 0 && localCfg.ttl != remoteCfg.ttl) {
			lWarning() << "[Initiate Outgoing Configuration] Remote answered ttl [" << remoteCfg.ttl
			           << "] does not match offered [" << localCfg.ttl << "] for local configuration " << localCfgIdx
			           << " [" << &local_offer << "]";
			return std::nullopt;
		}
		resultCfg.ttl = localCfg.ttl;
		resultCfg.dir = localCfg.getDirection();
	} else {
		resultCfg.dir = OfferAnswerEngine::computeDirOutgoing(localCfg.getDirection(), remoteCfg.getDirection());
	}

	resultCfg.rtcp_mux = remoteCfg.rtcp_mux && localCfg.rtcp_mux;
	if (remoteCfg.mid.empty() == false) {
		if (localCfg.mid.empty() == false) {
			resultCfg.mid = remoteCfg.mid;
			resultCfg.mid_rtp_ext_header_id = remoteCfg.mid_rtp_ext_header_id;
			resultCfg.bundle_only = remoteCfg.bundle_only;
			resultCfg.rtcp_mux = true; /* RTCP mux must be enabled in bundle mode. */
		} else {
			lError() << "[Initiate Outgoing Configuration] The remote configuration at index " << remoteCfgIdx
			         << " has set a mid in an answer while local configuration " << localCfgIdx << " didn't offer it.";
		}
	}

	resultCfg.mixer_to_client_extension_id =
	    getExtensionId(localCfg.mixer_to_client_extension_id, remoteCfg.mixer_to_client_extension_id);
	resultCfg.client_to_mixer_extension_id =
	    getExtensionId(localCfg.client_to_mixer_extension_id, remoteCfg.client_to_mixer_extension_id);
	resultCfg.frame_marking_extension_id =
	    getExtensionId(localCfg.frame_marking_extension_id, remoteCfg.frame_marking_extension_id);

	resultCfg.conference_ssrc = remoteCfg.conference_ssrc;

	if (!resultCfg.payloads.empty() && !OfferAnswerEngine::onlyTelephoneEvent(resultCfg.payloads)) {
		resultCfg.ptime = remoteCfg.ptime;
		resultCfg.maxptime = remoteCfg.maxptime;
	} else {
		return std::nullopt;
	}

	if (resultCfg.hasSrtp() == true) {
		const auto srtpFound = std::find(availableEncs.cbegin(), availableEncs.cend(), LinphoneMediaEncryptionSRTP);
		if (srtpFound == availableEncs.cend()) {
			lWarning() << "[Initiate Outgoing Configuration] Found matching payloads but SRTP is not supported";
			return std::nullopt;
		} else {
			auto &crypto = resultCfg.crypto;
			/* verify crypto algo */
			crypto.clear();
			SalSrtpCryptoAlgo crypto_result;
			if (!OfferAnswerEngine::matchCryptoAlgo(localCfg.crypto, remoteCfg.crypto, crypto_result, false)) {
				return std::nullopt;
			}
			crypto.emplace(crypto.begin(), crypto_result);
		}
	}
	resultCfg.rtp_ssrc = localCfg.rtp_ssrc;
	resultCfg.rtcp_cname = localCfg.rtcp_cname;

	if (!OfferAnswerEngine::fillZrtpAttributes(local_offer, localCfgIdx, remote_answer, remoteCfgIdx, resultCfg)) {
		return std::nullopt;
	}

	// Handle dtls session attribute: if both local and remote have a dtls fingerprint and a dtls setup, get the remote
	// fingerprint into the result
	if ((localCfg.dtls_role != SalDtlsRoleInvalid) && (remoteCfg.dtls_role != SalDtlsRoleInvalid) &&
	    (!localCfg.dtls_fingerprint.empty()) && (!remoteCfg.dtls_fingerprint.empty())) {
		const auto dtlsFound = std::find(availableEncs.cbegin(), availableEncs.cend(), LinphoneMediaEncryptionDTLS);
		if (dtlsFound == availableEncs.cend()) {
			lWarning() << "[Initiate Outgoing Configuration] DTLS is not supported even tough negotiation succeded";
			return std::nullopt;
		} else {
			resultCfg.dtls_fingerprint = remoteCfg.dtls_fingerprint;
			if (remoteCfg.dtls_role == SalDtlsRoleIsClient) {
				resultCfg.dtls_role = SalDtlsRoleIsServer;
			} else {
				resultCfg.dtls_role = SalDtlsRoleIsClient;
			}
		}
	} else {
		resultCfg.dtls_fingerprint.clear();
		resultCfg.dtls_role = SalDtlsRoleInvalid;
	}
	resultCfg.implicit_rtcp_fb = localCfg.implicit_rtcp_fb && remoteCfg.implicit_rtcp_fb;

	resultCfg.acapIndexes = remoteCfg.acapIndexes;
	resultCfg.tcapIndex = remoteCfg.tcapIndex;
	resultCfg.index = remoteCfg.index;

	return resultCfg;
}

SalStreamDescription OfferAnswerEngine::initiateIncomingStream(const SalStreamDescription &local_cap,
                                                               const SalStreamDescription &remote_offer,
                                                               const std::string &bundle_owner_mid,
                                                               const bool allowCapabilityNegotiation) {
	SalStreamDescription result;
	result.name = local_cap.name;
	result.type = local_cap.getType();
	if (result.type == SalOther) result.typeother = remote_offer.typeother;

	auto remoteCfgIdx = remote_offer.getActualConfigurationIndex();
	auto localCfgIdx = local_cap.getActualConfigurationIndex();

	result.ice_pwd = local_cap.ice_pwd;
	result.ice_ufrag = local_cap.ice_ufrag;
	result.ice_mismatch = local_cap.ice_mismatch;
	result.ice_candidates = local_cap.ice_candidates;
	result.ice_remote_candidates = local_cap.ice_remote_candidates;

	OfferAnswerEngine::optional_sal_stream_configuration resultNegCfg;
	if (allowCapabilityNegotiation) {
		for (const auto &remoteCfg : remote_offer.getAllCfgs()) {
			for (const auto &localCfg : local_cap.getAllCfgs()) {
				if (resultNegCfg) {
					break;
				} else {
					localCfgIdx = localCfg.first;
					remoteCfgIdx = remoteCfg.first;
					resultNegCfg = initiateIncomingConfiguration(local_cap, remote_offer, result, bundle_owner_mid,
					                                             localCfgIdx, remoteCfgIdx);
				}
			}
			if (resultNegCfg) {
				break;
			}
		}
	} else {
		localCfgIdx = local_cap.getActualConfigurationIndex();
		remoteCfgIdx = remote_offer.getActualConfigurationIndex();
		resultNegCfg =
		    initiateIncomingConfiguration(local_cap, remote_offer, result, bundle_owner_mid, localCfgIdx, remoteCfgIdx);
	}

	if (remote_offer.getLabel().empty()) {
		result.setLabel(local_cap.getLabel());
	} else {
		// Offer made by local conference
		result.setLabel(remote_offer.getLabel());
	}
	result.setContent(local_cap.getContent());

	if (resultNegCfg) {
		auto resultCfg = resultNegCfg.value();
		result.addActualConfiguration(resultCfg);

		remote_offer.cfgIndex = remoteCfgIdx;
		local_cap.cfgIndex = localCfgIdx;

		if (remote_offer.rtp_addr.empty() == false && ms_is_multicast(L_STRING_TO_C(remote_offer.rtp_addr))) {
			result.rtp_addr = remote_offer.rtp_addr;
			result.rtcp_addr = remote_offer.rtcp_addr;
			result.rtp_port = remote_offer.rtp_port;
			/*result.rtcp_port=remote_offer.rtcp_port;*/
			result.rtcp_port = 0; /* rtcp not supported yet*/
			result.bandwidth = remote_offer.bandwidth;
			result.multicast_role = SalMulticastReceiver;
		} else {
			result.rtp_addr = local_cap.rtp_addr;
			result.rtcp_addr = local_cap.rtcp_addr;
			result.rtp_port = local_cap.rtp_port;
			result.rtcp_port = local_cap.rtcp_port;
			result.bandwidth = local_cap.bandwidth;
		}

		if (resultCfg.bundle_only == true) {
			/* The stream is a secondary one part of a bundle.
			 * In this case it must set the bundle-only attribute, and set port to zero.*/
			result.rtp_port = 0;
		}
		if (resultCfg.rtcp_mux == true) {
			// RTCP multiplexing is enabled, therefore all traffic goes through the RTP port.
			result.rtcp_port = 0;
		}
		lInfo() << "Found matching configurations: local configuration index " << local_cap.cfgIndex
		        << " remote offered configuration index " << remote_offer.cfgIndex;
	} else {
		lDebug() << "[Initiate Incoming Stream] Unable to find a suitable configuration for stream of type "
		         << std::string(sal_stream_type_to_string(result.type));

		// Copy remote proto as it must not change event when a stream is rejected
		auto &cfg = result.cfgs[result.getActualConfigurationIndex()];
		const auto &remoteCfg = remote_offer.getActualConfiguration();
		cfg.proto = remoteCfg.proto;
		result.disable();
	}

	return result;
}

OfferAnswerEngine::optional_sal_stream_configuration OfferAnswerEngine::initiateIncomingConfiguration(
    const SalStreamDescription &local_cap,
    const SalStreamDescription &remote_offer,
    const SalStreamDescription &result,
    const std::string &bundle_owner_mid,
    const PotentialCfgGraph::media_description_config::key_type &localCfgIdx,
    const PotentialCfgGraph::media_description_config::key_type &remoteCfgIdx) {

	SalStreamConfiguration resultCfg;
	if (result.hasConfigurationAtIndex(result.getActualConfigurationIndex())) {
		resultCfg = result.getActualConfiguration();
	}
	const SalStreamConfiguration &localCfg = local_cap.getConfigurationAtIndex(localCfgIdx);
	const SalStreamConfiguration &remoteCfg = remote_offer.getConfigurationAtIndex(remoteCfgIdx);

	if ((localCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>()) ||
	    (remoteCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>())) {
		lWarning() << "[Initiate Incoming Configuration] Unable to find valid configurations to compare against - "
		              "local configuration found: "
		           << (localCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>())
		           << " remote configuration found: "
		           << (remoteCfg == Utils::getEmptyConstRefObject<SalStreamConfiguration>());
		return std::nullopt;
	}
	/* Handle RTP bundle negociation */
	bool bundle_enabled = false;
	if (!localCfg.mid.empty() && !remoteCfg.mid.empty() && !bundle_owner_mid.empty() &&
	    (localCfg.mid_rtp_ext_header_id != 0) && (remoteCfg.mid_rtp_ext_header_id != 0)) {
		resultCfg.mid = remoteCfg.mid;
		resultCfg.mid_rtp_ext_header_id = remoteCfg.mid_rtp_ext_header_id;

		if (remoteCfg.mid.compare(bundle_owner_mid) != 0) {
			/* The stream is a secondary one part of a bundle.
			 * In this case it must set the bundle-only attribute, and set port to zero.*/
			resultCfg.bundle_only = true;
		}
		bundle_enabled = true;
		resultCfg.rtcp_mux = true; /* RTCP mux must be enabled in bundle mode. */
	}

	const auto &availableEncs = local_cap.getSupportedEncryptions();
	resultCfg.payloads = matchPayloads(localCfg.payloads, remoteCfg.payloads, false, bundle_enabled);
	if (OfferAnswerEngine::areProtoCompatibles(localCfg.getProto(), remoteCfg.getProto())) {
		if (localCfg.getProto() != remoteCfg.getProto() && remoteCfg.hasAvpf()) {
			lWarning() << "[Initiate Incoming Configuration] Sending a downgraded AVP answer (transport protocol "
			           << sal_media_proto_to_string(remoteCfg.getProto())
			           << " of the remote offered stream configuration at index " << remoteCfgIdx
			           << ") for the received AVPF offer (transport protocol "
			           << sal_media_proto_to_string(localCfg.getProto()) << " of local stream configuration at index "
			           << localCfgIdx << ")";
			const_cast<SalStreamConfiguration &>(remoteCfg).proto = localCfg.getProto();
		}
		resultCfg.proto = remoteCfg.getProto();
	} else {
		lWarning() << "[Initiate Incoming Configuration] The transport protocol "
		           << sal_media_proto_to_string(localCfg.getProto()) << " of local stream configuration at index "
		           << localCfgIdx << " is not compatible with the transport protocol "
		           << sal_media_proto_to_string(remoteCfg.getProto())
		           << " of the remote offered stream configuration at index " << remoteCfgIdx;
		return std::nullopt;
	}
	resultCfg.dir = OfferAnswerEngine::computeDirIncoming(localCfg.getDirection(), remoteCfg.getDirection());
	resultCfg.delete_media_attributes = localCfg.delete_media_attributes;
	resultCfg.delete_session_attributes = localCfg.delete_session_attributes;

	if (remote_offer.rtp_addr.empty() == false && ms_is_multicast(L_STRING_TO_C(remote_offer.rtp_addr))) {
		if (resultCfg.hasSrtp() == true) {
			lWarning()
			    << "[Initiate Incoming Configuration] SAVP not supported for multicast address for remote stream ["
			    << &remote_offer << "]";
			return std::nullopt;
		}
		resultCfg.dir = remoteCfg.getDirection();
		resultCfg.ptime = remoteCfg.ptime;
		resultCfg.maxptime = remoteCfg.maxptime;
		resultCfg.ttl = remoteCfg.ttl;
	} else {
		resultCfg.ptime = localCfg.ptime;
		resultCfg.maxptime = localCfg.maxptime;
	}

	if (resultCfg.payloads.empty() || OfferAnswerEngine::onlyTelephoneEvent(resultCfg.payloads) ||
	    !remote_offer.enabled()) {
		lWarning() << "[Initiate Incoming Configuration] Unable to find suitable payloads";
		return std::nullopt;
	}

	resultCfg.rtcp_mux = remoteCfg.rtcp_mux && localCfg.rtcp_mux;

	resultCfg.mixer_to_client_extension_id =
	    getExtensionId(localCfg.mixer_to_client_extension_id, remoteCfg.mixer_to_client_extension_id);
	resultCfg.client_to_mixer_extension_id =
	    getExtensionId(localCfg.client_to_mixer_extension_id, remoteCfg.client_to_mixer_extension_id);
	resultCfg.frame_marking_extension_id =
	    getExtensionId(localCfg.frame_marking_extension_id, remoteCfg.frame_marking_extension_id);

	resultCfg.conference_ssrc = localCfg.conference_ssrc;

	if (resultCfg.hasSrtp() == true) {
		const auto srtpFound = std::find(availableEncs.cbegin(), availableEncs.cend(), LinphoneMediaEncryptionSRTP);
		if (srtpFound == availableEncs.cend()) {
			lWarning() << "[Initiate Incoming Configuration] Found matching payloads but SRTP is not supported";
			return std::nullopt;
		} else {
			auto &crypto = resultCfg.crypto;
			/* select crypto algo */
			crypto.clear();
			SalSrtpCryptoAlgo crypto_result;
			if (!OfferAnswerEngine::matchCryptoAlgo(localCfg.crypto, remoteCfg.crypto, crypto_result, true)) {
				lError() << "[Initiate Incoming Configuration] No matching crypto algo for remote stream's offer ["
				         << &remote_offer << "]";
				return std::nullopt;
			}
			crypto.emplace(crypto.begin(), crypto_result);
		}
	}

	if (!OfferAnswerEngine::fillZrtpAttributes(local_cap, localCfgIdx, remote_offer, remoteCfgIdx, resultCfg)) {
		return std::nullopt;
	}

	resultCfg.set_nortpproxy = localCfg.set_nortpproxy;
	resultCfg.rtp_ssrc = localCfg.rtp_ssrc;
	resultCfg.rtcp_cname = localCfg.rtcp_cname;

	// Handle dtls stream attribute: if both local and remote have a dtls fingerprint and a dtls setup, add the local
	// fingerprint to the answer Note: local description usually stores dtls config at session level which means it
	// apply to all streams, check this too
	if (((localCfg.dtls_role != SalDtlsRoleInvalid)) && (remoteCfg.dtls_role != SalDtlsRoleInvalid) &&
	    (!localCfg.dtls_fingerprint.empty()) && (!remoteCfg.dtls_fingerprint.empty())) {
		const auto dtlsFound = std::find(availableEncs.cbegin(), availableEncs.cend(), LinphoneMediaEncryptionDTLS);
		if (dtlsFound == availableEncs.cend()) {
			lWarning() << "[Initiate Incoming Configuration] DTLS is not supported even though negotiation succeeded";
			return std::nullopt;
		} else {
			resultCfg.dtls_fingerprint = localCfg.dtls_fingerprint;
			if (remoteCfg.dtls_role == SalDtlsRoleUnset) {
				resultCfg.dtls_role = SalDtlsRoleIsClient;
			}
		}
	} else {
		resultCfg.dtls_fingerprint.clear();
		resultCfg.dtls_role = SalDtlsRoleInvalid;
	}
	resultCfg.implicit_rtcp_fb = localCfg.implicit_rtcp_fb && remoteCfg.implicit_rtcp_fb;
	resultCfg.tcapIndex = remoteCfg.tcapIndex;
	resultCfg.acapIndexes = remoteCfg.acapIndexes;
	resultCfg.index = remoteCfg.index;

	return resultCfg;
}

bool OfferAnswerEngine::areProtoInStreamCompatibles(const SalStreamDescription &localStream,
                                                    const SalStreamDescription &otherStream) {

	for (const auto &otherCfg : otherStream.getAllCfgs()) {
		for (const auto &localCfg : localStream.getAllCfgs()) {
			const auto compatible =
			    OfferAnswerEngine::areProtoCompatibles(localCfg.second.getProto(), otherCfg.second.getProto());
			if (compatible) {
				return true;
			}
		}
	}
	return false;
}

bool OfferAnswerEngine::areProtoCompatibles(SalMediaProto localProto, SalMediaProto otherProto) {
	switch (localProto) {
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
			return (otherProto == SalProtoRtpAvp || otherProto == SalProtoRtpAvpf);
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
			return (otherProto == SalProtoRtpSavp || otherProto == SalProtoRtpSavpf);
		case SalProtoUdpTlsRtpSavp:
		case SalProtoUdpTlsRtpSavpf:
			return (otherProto == SalProtoUdpTlsRtpSavp || otherProto == SalProtoUdpTlsRtpSavpf);
		default:
			return (otherProto == localProto);
	}
}

/**
 * Returns a media description to run the streams with, based on a local offer
 * and the returned response (remote).
 **/
std::shared_ptr<SalMediaDescription>
OfferAnswerEngine::initiateOutgoing(std::shared_ptr<SalMediaDescription> local_offer,
                                    const std::shared_ptr<SalMediaDescription> remote_answer) {
	size_t i;

	auto result = std::make_shared<SalMediaDescription>(local_offer->getParams());
	const bool capabilityNegotiation = result->getParams().capabilityNegotiationSupported();

	for (i = 0; i < local_offer->streams.size(); ++i) {
		ms_message("Processing for stream %zu", i);
		SalStreamDescription &ls = local_offer->streams[i];
		const SalStreamDescription &rs = remote_answer->streams[i];
		if (i < remote_answer->streams.size()) {
			SalStreamDescription stream;
			if ((rs.getType() == ls.getType()) && OfferAnswerEngine::areProtoInStreamCompatibles(ls, rs)) {
				stream = OfferAnswerEngine::initiateOutgoingStream(ls, rs, capabilityNegotiation);
				SalStreamConfiguration actualCfg = stream.getActualConfiguration();
				memcpy(&actualCfg.rtcp_xr, &ls.getChosenConfiguration().rtcp_xr,
				       sizeof(stream.getChosenConfiguration().rtcp_xr));
				if ((ls.getChosenConfiguration().rtcp_xr.enabled == TRUE) &&
				    (rs.getChosenConfiguration().rtcp_xr.enabled == FALSE)) {
					actualCfg.rtcp_xr.enabled = FALSE;
				}
				actualCfg.rtcp_fb.generic_nack_enabled = ls.getChosenConfiguration().rtcp_fb.generic_nack_enabled &
				                                         rs.getChosenConfiguration().rtcp_fb.generic_nack_enabled;
				actualCfg.rtcp_fb.tmmbr_enabled = ls.getChosenConfiguration().rtcp_fb.tmmbr_enabled &
				                                  rs.getChosenConfiguration().rtcp_fb.tmmbr_enabled;
				actualCfg.rtcp_fb.goog_remb_enabled = ls.getChosenConfiguration().rtcp_fb.goog_remb_enabled &
				                                      rs.getChosenConfiguration().rtcp_fb.goog_remb_enabled;
				stream.addActualConfiguration(actualCfg);
			}
			result->streams.push_back(stream);
		} else {
			ms_warning("No matching stream for %zu", i);
		}
	}
	// t= field on the final description must have the same value as the offer
	// RFC3264 - Section 6
	// The "t=" line in the answer MUST equal that of the offer. The time of the session cannot be negotiated.
	result->times = local_offer->times;
	result->bandwidth = remote_answer->bandwidth;
	result->origin_addr = remote_answer->origin_addr;
	result->addr = remote_answer->addr;
	result->ice_pwd = local_offer->ice_pwd;
	result->ice_ufrag = local_offer->ice_ufrag;
	memcpy(&result->rtcp_xr, &local_offer->rtcp_xr, sizeof(result->rtcp_xr));
	if ((local_offer->rtcp_xr.enabled == TRUE) && (remote_answer->rtcp_xr.enabled == FALSE)) {
		result->rtcp_xr.enabled = FALSE;
	}
	/* TODO: check that the bundle answer is compliant with our offer.
	 * For now, just check the presence of a bundle response. */
	if (!local_offer->bundles.empty()) {
		if (!remote_answer->bundles.empty()) {
			for (auto &s : result->streams) {
				SalStreamBundle bundle;
				auto &cfg = s.cfgs[s.getChosenConfigurationIndex()];
				const auto &mid = cfg.mid;
				if (!mid.empty()) {
					if (!result->bundles.empty()) {
						bundle = result->bundles.front();
						// Delete first element
						result->bundles.erase(result->bundles.begin());
					}
					bundle.addStream(cfg, mid);
					result->bundles.push_front(bundle);
				}
			}
		}
	} else if (!remote_answer->bundles.empty()) {
		ms_error("Remote answerer is proposing bundles, which we did not offer.");
	}

	verifyBundles(local_offer, remote_answer, result);

	if (local_offer->record != SalMediaRecordNone && remote_answer->record != SalMediaRecordNone) {
		result->record = remote_answer->record;
	}

	return result;
}

/**
 * Returns a media description to run the streams with, based on the local capabilities and
 * and the received offer.
 * The returned media description is an answer and should be sent to the offerer.
 **/
std::shared_ptr<SalMediaDescription>
OfferAnswerEngine::initiateIncoming(const std::shared_ptr<SalMediaDescription> local_capabilities,
                                    std::shared_ptr<SalMediaDescription> remote_offer) {
	auto result = std::make_shared<SalMediaDescription>(local_capabilities->getParams());
	size_t i = 0;

	if (remote_offer->record != SalMediaRecordNone && local_capabilities->record != SalMediaRecordNone) {
		result->record = local_capabilities->record;
	}

	const bool capabilityNegotiation = result->getParams().capabilityNegotiationSupported();
	for (auto &rs : remote_offer->streams) {
		SalStreamDescription &ls = local_capabilities->streams[i];
		SalStreamDescription stream;
		SalStreamConfiguration actualCfg;

		if ((local_capabilities->streams.size() > i) && (rs.getType() == ls.getType()) &&
		    OfferAnswerEngine::areProtoInStreamCompatibles(ls, rs)) {
			std::string bundle_owner_mid;
			if (local_capabilities->accept_bundles) {
				int owner_index = remote_offer->getIndexOfTransportOwner(rs);
				if (owner_index >= 0) {
					bundle_owner_mid = remote_offer->streams[(size_t)owner_index].getChosenConfiguration().getMid();
				}
			}
			stream = OfferAnswerEngine::initiateIncomingStream(ls, rs, bundle_owner_mid, capabilityNegotiation);
			// Get an up to date actual configuration as it may have changed
			actualCfg = stream.getActualConfiguration();
			// Handle global RTCP FB attributes
			actualCfg.rtcp_fb.generic_nack_enabled = rs.getChosenConfiguration().rtcp_fb.generic_nack_enabled;
			actualCfg.rtcp_fb.tmmbr_enabled = rs.getChosenConfiguration().rtcp_fb.tmmbr_enabled;
			actualCfg.rtcp_fb.goog_remb_enabled = rs.getChosenConfiguration().rtcp_fb.goog_remb_enabled;
			// Handle media RTCP XR attribute
			memset(&actualCfg.rtcp_xr, 0, sizeof(actualCfg.rtcp_xr));
			if (rs.getChosenConfiguration().rtcp_xr.enabled == TRUE) {
				const OrtpRtcpXrConfiguration *rtcp_xr_conf = NULL;
				if (ls.getChosenConfiguration().rtcp_xr.enabled == TRUE)
					rtcp_xr_conf = &ls.getChosenConfiguration().rtcp_xr;
				else if (local_capabilities->rtcp_xr.enabled == TRUE) rtcp_xr_conf = &local_capabilities->rtcp_xr;
				if (rtcp_xr_conf != NULL) {
					memcpy(&actualCfg.rtcp_xr, rtcp_xr_conf, sizeof(actualCfg.rtcp_xr));
				} else {
					actualCfg.rtcp_xr.enabled = FALSE;
				}
			}
		} else {
			ms_message("Declining mline %zu, no corresponding stream in local capabilities description.", i);
			/* create an inactive stream for the answer, as there where no matching stream in local capabilities */
			actualCfg.dir = SalStreamInactive;
			stream.rtp_port = 0;
			stream.type = rs.getType();
			actualCfg.proto = rs.getProto();
			if (rs.getType() == SalOther) {
				stream.typeother = rs.typeother;
			}
			if (rs.getProto() == SalProtoOther) {
				actualCfg.proto_other = rs.getChosenConfiguration().proto_other;
			}
		}
		if (local_capabilities->streams.size() > i) {
			stream.custom_sdp_attributes = sal_custom_sdp_attribute_clone(ls.custom_sdp_attributes);
		}
		stream.addActualConfiguration(actualCfg);
		result->streams.push_back(stream);
		i++;
	}
	result->username = local_capabilities->username;
	result->addr = local_capabilities->addr;
	// t= field on the answer must have the same value as the offer
	// RFC3264 - Section 6
	// The "t=" line in the answer MUST equal that of the offer. The time of the session cannot be negotiated.
	result->times = remote_offer->times;
	result->bandwidth = local_capabilities->bandwidth;
	result->origin_addr = local_capabilities->origin_addr;
	result->session_ver = local_capabilities->session_ver;
	result->session_id = local_capabilities->session_id;
	result->ice_pwd = local_capabilities->ice_pwd;
	result->ice_ufrag = local_capabilities->ice_ufrag;
	result->ice_lite = local_capabilities->ice_lite;
	result->set_nortpproxy = local_capabilities->set_nortpproxy;
	result->custom_sdp_attributes = sal_custom_sdp_attribute_clone(local_capabilities->custom_sdp_attributes);

	result->name = local_capabilities->name;

	// Handle session RTCP XR attribute
	memset(&result->rtcp_xr, 0, sizeof(result->rtcp_xr));
	if (remote_offer->rtcp_xr.enabled == TRUE) {
		if ((local_capabilities->rtcp_xr.enabled == TRUE) && (local_capabilities->dir == SalStreamSendRecv)) {
			memcpy(&result->rtcp_xr, &local_capabilities->rtcp_xr, sizeof(result->rtcp_xr));
		} else {
			result->rtcp_xr.enabled = FALSE;
		}
	}

	for (auto &s : result->streams) {
		SalStreamBundle bundle;
		auto &cfg = s.cfgs[s.getChosenConfigurationIndex()];
		const auto &mid = cfg.mid;
		if (!mid.empty()) {
			if (!result->bundles.empty()) {
				bundle = result->bundles.front();
				// Delete first element in order to update the bundle
				result->bundles.erase(result->bundles.begin());
			}
			bundle.addStream(cfg, mid);
			result->bundles.push_front(bundle);
		}
	}

	verifyBundles(local_capabilities, remote_offer, result);

	return result;
}

bool OfferAnswerEngine::fillZrtpAttributes(const SalStreamDescription &localStream,
                                           const unsigned int &localCfgIdx,
                                           const SalStreamDescription &remoteStream,
                                           const unsigned int &remoteCfgIdx,
                                           SalStreamConfiguration &resultCfg) {

	const SalStreamConfiguration &localCfg = localStream.getConfigurationAtIndex(localCfgIdx);
	const SalStreamConfiguration &remoteCfg = remoteStream.getConfigurationAtIndex(remoteCfgIdx);

	const bool isLocalActualCfg = (localStream.getActualConfigurationIndex() == localCfgIdx);
	const bool isRemoteActualCfg = (remoteStream.getActualConfigurationIndex() == remoteCfgIdx);

	if ((!isLocalActualCfg || !isRemoteActualCfg) && (localCfg.haveZrtpHash != remoteCfg.haveZrtpHash)) {
		lWarning() << "[Fill ZRTP Attributes] No matching zrtp attribute for remote configuration " << remoteCfgIdx
		           << " (hash \"" << (char *)remoteCfg.zrtphash << "\") and local configuration " << localCfgIdx
		           << " (hash \"" << (char *)localCfg.zrtphash << "\")";
		return false;
	}
	// add our zrtp hash if remote gave one but also when our side has set ZRTP as active to help peer starting earlier
	// if it has ZRTP capabilities haveZrtpHash is set in localStream when ZRTP is active on our side. If local or
	// remote configuration is not the actual one, ensure that both have zrtp hashes
	if ((remoteCfg.haveZrtpHash == 1) || (localCfg.haveZrtpHash == 1)) {
		const auto &availableEncs = localStream.getSupportedEncryptions();
		// For actual configurations, all encryptions are supported
		const auto zrtpFound = (!isLocalActualCfg || !isRemoteActualCfg)
		                           ? (std::find(availableEncs.cbegin(), availableEncs.cend(),
		                                        LinphoneMediaEncryptionZRTP) != availableEncs.cend())
		                           : true;
		if (!zrtpFound) {
			lWarning() << "[Fill ZRTP Attributes] ZRTP encryption is not supported by the local configuration - ZRTP "
			              "attribute for remote configuration "
			           << remoteCfgIdx << " (hash \"" << (char *)remoteCfg.zrtphash << "\") and local configuration "
			           << localCfgIdx << " (hash \"" << (char *)localCfg.zrtphash << "\")";
			return false;
		} else {
			if (localCfg.zrtphash[0] != 0) { /* if ZRTP is available, set the zrtp hash even if it is not selected */
				strncpy((char *)(resultCfg.zrtphash), (char *)(localCfg.zrtphash), sizeof(resultCfg.zrtphash));
			}
			resultCfg.haveZrtpHash = ((localCfg.zrtphash[0] != 0) || (remoteCfg.zrtphash[0] != 0)) ? 1 : 0;
		}
	}

	return true;
}

LINPHONE_END_NAMESPACE
