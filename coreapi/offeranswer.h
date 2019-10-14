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

#ifndef offeranswer_h
#define offeranswer_h

/**
 This header files defines the SDP offer answer API.
 It can be used by implementations of SAL directly.
**/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a media description to run the streams with, based on a local offer
 * and the returned response (remote).
**/
int offer_answer_initiate_outgoing(MSFactory *factory, const SalMediaDescription *local_offer,
									const SalMediaDescription *remote_answer,
									SalMediaDescription *result);

/**
 * Returns a media description to run the streams with, based on the local capabilities and
 * and the received offer.
 * The returned media description is an answer and should be sent to the offerer.
**/
int offer_answer_initiate_incoming(MSFactory* factory, const SalMediaDescription *local_capabilities,
						const SalMediaDescription *remote_offer,
						SalMediaDescription *result, bool_t one_matching_codec);

#ifdef __cplusplus
}
#endif

#endif
