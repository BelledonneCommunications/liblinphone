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

#ifndef OFFERANSWER_H
#define OFFERANSWER_H

#include <list>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "linphone/utils/general.h"
#include "sal/potential_config_graph.h"
#include "sal/sal_stream_configuration.h"

/**
 This header files defines the SDP offer answer API.
 It can be used by implementations of SAL directly.
**/

LINPHONE_BEGIN_NAMESPACE

class SalMediaDescription;

class OfferAnswerEngine {

public:
	using optional_sal_stream_configuration = std::optional<SalStreamConfiguration>;
	OfferAnswerEngine(MSFactory *factory);
	void setFactory(MSFactory *factory) {
		mMsFactory = factory;
	}
	void setOneMatchingCodecPolicy(bool value);
	void setAnswerWithOwnNumberingPolicy(bool value);
	/**
	 * Returns a media description to run the streams with, based on a local offer
	 * and the returned response (remote).
	 **/
	std::shared_ptr<SalMediaDescription> initiateOutgoing(std::shared_ptr<SalMediaDescription> local_offer,
	                                                      const std::shared_ptr<SalMediaDescription> remote_answer);

	/**
	 * Returns a media description to run the streams with, based on the local capabilities and
	 * and the received offer.
	 * The returned media description is an answer and should be sent to the offerer.
	 **/
	std::shared_ptr<SalMediaDescription> initiateIncoming(const std::shared_ptr<SalMediaDescription> local_capabilities,
	                                                      std::shared_ptr<SalMediaDescription> remote_offer);

private:
	static void verifyBundles(const std::shared_ptr<SalMediaDescription> &local,
	                          const std::shared_ptr<SalMediaDescription> &remote,
	                          std::shared_ptr<SalMediaDescription> &result);
	static bool onlyTelephoneEvent(const std::list<OrtpPayloadType *> &l);
	static bool areProtoInStreamCompatibles(const SalStreamDescription &localStream,
	                                        const SalStreamDescription &otherStream);
	static bool areProtoCompatibles(SalMediaProto localProto, SalMediaProto otherProto);
	static SalStreamDir computeDirIncoming(SalStreamDir local, SalStreamDir offered);
	static SalStreamDir computeConferenceStreamDir(SalStreamDir dir);
	static SalStreamDir computeDirOutgoing(SalStreamDir local, SalStreamDir answered);
	static bool matchCryptoAlgo(const std::vector<SalSrtpCryptoAlgo> &local,
	                            const std::vector<SalSrtpCryptoAlgo> &remote,
	                            SalSrtpCryptoAlgo &result,
	                            bool use_local_key);
	std::list<OrtpPayloadType *> matchPayloads(const std::list<OrtpPayloadType *> &local,
	                                           const std::list<OrtpPayloadType *> &remote,
	                                           bool reading_response,
	                                           bool bundle_enabled);
	static OrtpPayloadType *genericMatch(const std::list<OrtpPayloadType *> &local_payloads,
	                                     const OrtpPayloadType *refpt,
	                                     const std::list<OrtpPayloadType *> &remote_payloads);
	PayloadType *findPayloadTypeBestMatch(const std::list<OrtpPayloadType *> &local_payloads,
	                                      const OrtpPayloadType *refpt,
	                                      const std::list<OrtpPayloadType *> &remote_payloads,
	                                      bool reading_response);

	SalStreamDescription initiateIncomingStream(const SalStreamDescription &local_cap,
	                                            const SalStreamDescription &remote_offer,
	                                            const std::string &bundle_owner_mid,
	                                            const bool allowCapabilityNegotiation);
	OfferAnswerEngine::optional_sal_stream_configuration
	initiateIncomingConfiguration(const SalStreamDescription &local_cap,
	                              const SalStreamDescription &remote_offer,
	                              const SalStreamDescription &result,
	                              const std::string &bundle_owner_mid,
	                              const PotentialCfgGraph::media_description_config::key_type &localCfgIdx,
	                              const PotentialCfgGraph::media_description_config::key_type &remoteCfgIdx);

	SalStreamDescription initiateOutgoingStream(const SalStreamDescription &local_offer,
	                                            const SalStreamDescription &remote_answer,
	                                            const bool allowCapabilityNegotiation);

	OfferAnswerEngine::optional_sal_stream_configuration
	initiateOutgoingConfiguration(const SalStreamDescription &local_offer,
	                              const SalStreamDescription &remote_answer,
	                              const SalStreamDescription &result,
	                              const PotentialCfgGraph::media_description_config::key_type &localCfgIdx,
	                              const PotentialCfgGraph::media_description_config::key_type &remoteCfgIdx);

	bool fillZrtpAttributes(const SalStreamDescription &localStream,
	                        const unsigned int &localCfgIdx,
	                        const SalStreamDescription &remoteStream,
	                        const unsigned int &remoteCfgIdx,
	                        SalStreamConfiguration &resultCfg);

	static int getExtensionId(int localExtensionId, int remoteExtensionId);

	MSFactory *mMsFactory = nullptr;
	bool mUseOneMatchingCodec = false;
	bool mAnswerWithOwnNumbering = false;
};

LINPHONE_END_NAMESPACE

#endif
