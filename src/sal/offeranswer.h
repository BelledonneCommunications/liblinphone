/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#ifndef OFFERANSWER_H
#define OFFERANSWER_H

#include <memory>
#include <list>
#include <vector>

#include "linphone/utils/general.h"
#include "sal/sal_stream_description.h"

/**
 This header files defines the SDP offer answer API.
 It can be used by implementations of SAL directly.
**/

LINPHONE_BEGIN_NAMESPACE

class SalMediaDescription;

class OfferAnswerEngine {

	public:
		/**
		 * Returns a media description to run the streams with, based on a local offer
		 * and the returned response (remote).
		**/
		static int  initiateOutgoing(MSFactory *factory, std::shared_ptr<SalMediaDescription> local_offer,
							const std::shared_ptr<SalMediaDescription> remote_answer,
							std::shared_ptr<SalMediaDescription> result);

		/**
		 * Returns a media description to run the streams with, based on the local capabilities and
		 * and the received offer.
		 * The returned media description is an answer and should be sent to the offerer.
		**/
		static int initiateIncoming(MSFactory* factory, const std::shared_ptr<SalMediaDescription> local_capabilities,
							std::shared_ptr<SalMediaDescription> remote_offer,
							std::shared_ptr<SalMediaDescription> result, bool_t one_matching_codec);

	private:

		static bool_t onlyTelephoneEvent(const std::list<OrtpPayloadType*> & l);
		static bool areProtoCompatibles(SalMediaProto localProto, SalMediaProto otherProto);
		static SalStreamDir computeDirIncoming(SalStreamDir local, SalStreamDir offered);
		static SalStreamDir computeConferenceStreamDir(SalStreamDir dir);
		static SalStreamDir computeDirOutgoing(SalStreamDir local, SalStreamDir answered);
		static bool_t matchCryptoAlgo(const std::vector<SalSrtpCryptoAlgo> &local, const std::vector<SalSrtpCryptoAlgo> &remote, SalSrtpCryptoAlgo & result, unsigned int* choosen_local_tag, bool_t use_local_key);
		static std::list<OrtpPayloadType*> matchPayloads(MSFactory *factory, const std::list<OrtpPayloadType*> & local, const std::list<OrtpPayloadType*> & remote, bool_t reading_response, bool_t one_matching_codec);
		static PayloadType * genericMatch(const std::list<OrtpPayloadType*> & local_payloads, const PayloadType *refpt, const std::list<OrtpPayloadType*> & remote_payloads);
		static PayloadType * findPayloadTypeBestMatch(MSFactory *factory, const std::list<OrtpPayloadType*> & local_payloads, const PayloadType *refpt, const std::list<OrtpPayloadType*> & remote_payloads, bool_t reading_response);

		static void initiateIncomingStream(MSFactory *factory, const SalStreamDescription & local_cap, const SalStreamDescription & remote_offer, SalStreamDescription & result, bool_t one_matching_codec, const char *bundle_owner_mid);

		static void initiateOutgoingStream(MSFactory* factory, const SalStreamDescription & local_offer, const SalStreamDescription & remote_answer, SalStreamDescription & result);

};

LINPHONE_END_NAMESPACE

#endif
