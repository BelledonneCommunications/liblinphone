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

#include "c-wrapper/internal/c-tools.h"
#include "c-wrapper/internal/c-sal.h"
#include "c-wrapper/internal/c-sal-stream-bundle.h"
#include "sal/sal_media_description.h"
#include "sal/sal.h"
#include "offeranswer.h"
#include "private.h"

#include "utils/payload-type-handler.h"

static bool_t only_telephone_event(const bctbx_list_t *l){
	for(;l!=NULL;l=l->next){
		PayloadType *p=(PayloadType*)l->data;
		if (strcasecmp(p->mime_type,"telephone-event")!=0){
			return FALSE;
		}
	}
	return TRUE;
}


static PayloadType * opus_match(MSOfferAnswerContext *ctx, const bctbx_list_t *local_payloads, const PayloadType *refpt, const bctbx_list_t *remote_payloads, bool_t reading_response){
	PayloadType *pt;
	const bctbx_list_t *elem;
	PayloadType *legacy_opus=NULL;

	for (elem=local_payloads;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		/*workaround a bug in earlier versions of linphone where opus/48000/1 is offered, which is uncompliant with opus rtp draft*/
		if (strcasecmp(pt->mime_type,"opus")==0 ){
			if (refpt->channels==1){
				legacy_opus=pt;
			}else if (refpt->channels==2){
				return payload_type_clone(pt);
			}
		}
	}
	if (legacy_opus){
		legacy_opus = payload_type_clone(legacy_opus);
		legacy_opus->channels=1; /*so that we respond with same number of channels */
		return legacy_opus;
	}
	return NULL;
}

static MSOfferAnswerContext *opus_offer_answer_create_context(void){
	static MSOfferAnswerContext opus_oa = {opus_match, NULL, NULL};
	return &opus_oa;
}

MSOfferAnswerProvider opus_offer_answer_provider={
	"opus",
	opus_offer_answer_create_context
};

/* the reason for this matcher is for some stupid uncompliant phone that offer G729a mime type !*/
static PayloadType * g729A_match(MSOfferAnswerContext *ctx, const bctbx_list_t *local_payloads, const PayloadType *refpt, const bctbx_list_t *remote_payloads, bool_t reading_response){
	PayloadType *pt;
	const bctbx_list_t *elem;
	PayloadType *candidate=NULL;

	for (elem=local_payloads;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		if (strcasecmp(pt->mime_type,"G729")==0 && refpt->channels==pt->channels){
			candidate=pt;
		}
	}
	return candidate ? payload_type_clone(candidate) : NULL;
}

static MSOfferAnswerContext *g729a_offer_answer_create_context(void){
	static MSOfferAnswerContext g729_oa = {g729A_match, NULL, NULL};
	return &g729_oa;
}

MSOfferAnswerProvider g729a_offer_answer_provider={
	"G729A",
	g729a_offer_answer_create_context
};

static PayloadType * red_match(MSOfferAnswerContext *ctx, const bctbx_list_t *local_payloads, const PayloadType *refpt, const bctbx_list_t *remote_payloads, bool_t reading_response) {
	const bctbx_list_t *elem_local, *elem_remote;
	PayloadType *red = NULL;

	for (elem_local = local_payloads; elem_local != NULL; elem_local = elem_local->next) {
		PayloadType *pt = (PayloadType*)elem_local->data;
		
		if (strcasecmp(pt->mime_type, payload_type_t140_red.mime_type) == 0) {
			red = payload_type_clone(pt);
			
			for (elem_remote = remote_payloads; elem_remote != NULL; elem_remote = elem_remote->next) {
				PayloadType *pt2 = (PayloadType*)elem_remote->data;
				if (strcasecmp(pt2->mime_type, payload_type_t140.mime_type) == 0) {
					int t140_payload_number = payload_type_get_number(pt2);
					char *red_fmtp = ms_strdup_printf("%i/%i/%i", t140_payload_number, t140_payload_number, t140_payload_number);
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

static MSOfferAnswerContext *red_offer_answer_create_context(void){
	static MSOfferAnswerContext red_oa = {red_match, NULL, NULL};
	return &red_oa;
}

MSOfferAnswerProvider red_offer_answer_provider={
	"red",
	red_offer_answer_create_context
};

static PayloadType * generic_match(const bctbx_list_t *local_payloads, const PayloadType *refpt, const bctbx_list_t *remote_payloads){
	PayloadType *pt;
	const bctbx_list_t *elem;

	for (elem=local_payloads;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		if ( pt->mime_type && refpt->mime_type 
			&& strcasecmp(pt->mime_type, refpt->mime_type)==0
			&& pt->clock_rate==refpt->clock_rate
			&& pt->channels==refpt->channels)
			return payload_type_clone(pt);
	}
	return NULL;
}


void linphone_core_register_offer_answer_providers(LinphoneCore *lc){
	MSFactory *factory = lc->factory;
	ms_factory_register_offer_answer_provider(factory, &red_offer_answer_provider);
	ms_factory_register_offer_answer_provider(factory, &g729a_offer_answer_provider);
	ms_factory_register_offer_answer_provider(factory, &opus_offer_answer_provider);
}

/*
 * Returns a PayloadType from the local list that matches a PayloadType offered or answered in the remote list
*/
static PayloadType * find_payload_type_best_match(MSFactory *factory, const bctbx_list_t *local_payloads, const PayloadType *refpt,
						  const bctbx_list_t *remote_payloads, bool_t reading_response){
	PayloadType *ret = NULL;
	MSOfferAnswerContext *ctx = NULL;

	// When a stream is inactive, refpt->mime_type might be null
	if (refpt->mime_type && (ctx = ms_factory_create_offer_answer_context(factory, refpt->mime_type))) {
		ms_message("Doing offer/answer processing with specific provider for codec [%s]", refpt->mime_type); 
		ret = ms_offer_answer_context_match_payload(ctx, local_payloads, refpt, remote_payloads, reading_response);
		ms_offer_answer_context_destroy(ctx);
		return ret;
	}
	return generic_match(local_payloads, refpt, remote_payloads);
}


static bctbx_list_t *match_payloads(MSFactory *factory, const bctbx_list_t *local, const bctbx_list_t *remote, bool_t reading_response, bool_t one_matching_codec){
	const bctbx_list_t *e2,*e1;
	bctbx_list_t *res=NULL;
	PayloadType *matched;
	bool_t found_codec=FALSE;

	for(e2=remote;e2!=NULL;e2=e2->next){
		PayloadType *p2=(PayloadType*)e2->data;
		matched=find_payload_type_best_match(factory, local, p2, remote, reading_response);
		if (matched){
			int local_number=payload_type_get_number(matched);
			int remote_number=payload_type_get_number(p2);

			if (one_matching_codec){
				if (strcasecmp(matched->mime_type,"telephone-event")!=0){
					if (found_codec){/* we have found a real codec already*/
						continue; /*this codec won't be added*/
					}else found_codec=TRUE;
				}
			}

			if (p2->send_fmtp){
				payload_type_append_send_fmtp(matched,p2->send_fmtp);
			}
			payload_type_set_flag(matched, PAYLOAD_TYPE_FLAG_CAN_RECV|PAYLOAD_TYPE_FLAG_CAN_SEND);
			if (matched->flags & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED && p2->flags & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED) {
				payload_type_set_flag(matched, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				/* Negotiation of AVPF features (keep common features) */
				matched->avpf.features &= p2->avpf.features;
				matched->avpf.rpsi_compatibility = p2->avpf.rpsi_compatibility;
				/* Take bigger AVPF trr interval */
				if (p2->avpf.trr_interval > matched->avpf.trr_interval) {
					matched->avpf.trr_interval = p2->avpf.trr_interval;
				}
			}else{
				payload_type_unset_flag(matched, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
			}
			res=bctbx_list_append(res,matched);
			/* we should use the remote numbering even when parsing a response */
			payload_type_set_number(matched,remote_number);
			payload_type_set_flag(matched, PAYLOAD_TYPE_FROZEN_NUMBER);
			if (reading_response && remote_number!=local_number){
				ms_warning("For payload type %s, proposed number was %i but the remote phone answered %i",
						  matched->mime_type, local_number, remote_number);
				/*
				 We must add this payload type with our local numbering in order to be able to receive it.
				 Indeed despite we must sent with the remote numbering, we must be able to receive with
				 our local one.
				*/
				matched=payload_type_clone(matched);
				payload_type_set_number(matched,local_number);
				payload_type_set_flag(matched, PAYLOAD_TYPE_FLAG_CAN_RECV);
				payload_type_set_flag(matched, PAYLOAD_TYPE_FROZEN_NUMBER);
				res=bctbx_list_append(res,matched);
			}
		}else{
			if (p2->channels>0)
				ms_message("No match for %s/%i/%i",p2->mime_type,p2->clock_rate,p2->channels);
			else ms_message("No match for %s/%i",p2->mime_type,p2->clock_rate);
		}
	}
	if (reading_response){
		/* add remaning local payload as CAN_RECV only so that if we are in front of a non-compliant equipment we are still able to decode the RTP stream*/
		for(e1=local;e1!=NULL;e1=e1->next){
			PayloadType *p1=(PayloadType*)e1->data;
			bool_t found=FALSE;
			for(e2=res;e2!=NULL;e2=e2->next){
				PayloadType *p2=(PayloadType*)e2->data;
				if (payload_type_get_number(p2)==payload_type_get_number(p1)){
					found=TRUE;
					break;
				}
			}
			if (!found){
				ms_message("Adding %s/%i for compatibility, just in case.",p1->mime_type,p1->clock_rate);
				p1=payload_type_clone(p1);
				payload_type_set_flag(p1, PAYLOAD_TYPE_FLAG_CAN_RECV);
				payload_type_set_flag(p1, PAYLOAD_TYPE_FROZEN_NUMBER);
				res=bctbx_list_append(res,p1);
			}
		}
	}
	return res;
}

static bool_t match_crypto_algo(const std::vector<SalSrtpCryptoAlgo> &local, const std::vector<SalSrtpCryptoAlgo> &remote,
	SalSrtpCryptoAlgo & result, unsigned int* choosen_local_tag, bool_t use_local_key) {
	for(const auto & rc : remote) {
		if (rc.algo == 0)
			break;

		/* Look for a local enabled crypto algo that matches one of the proposed by remote */
		for(const auto & lc : local) {
			if (rc.algo == lc.algo) {
				result.algo = rc.algo;
				/* We're answering an SDP offer. Supply our master key, associated with the remote supplied tag */
				if (use_local_key) {
					strncpy(result.master_key, lc.master_key, sizeof(result.master_key) );
					result.tag = rc.tag;
					*choosen_local_tag = lc.tag;
				}
				/* We received an answer to our SDP crypto proposal. Copy matching algo remote master key to result, and memorize local tag */
				else {
					strncpy(result.master_key, rc.master_key, sizeof(result.master_key));
					result.tag = lc.tag;
					*choosen_local_tag = lc.tag;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}



static SalStreamDir compute_dir_outgoing(SalStreamDir local, SalStreamDir answered){
	SalStreamDir res=local;
	if (local==SalStreamSendRecv){
		if (answered==SalStreamRecvOnly){
			res=SalStreamSendOnly;
		}else if (answered==SalStreamSendOnly){
			res=SalStreamRecvOnly;
		}
	}
	if (answered==SalStreamInactive){
		res=SalStreamInactive;
	}
	return res;
}

static SalStreamDir compute_dir_incoming(SalStreamDir local, SalStreamDir offered){
	SalStreamDir res=SalStreamSendRecv;
	if (local==SalStreamSendRecv){
		if (offered==SalStreamSendOnly)
			res=SalStreamRecvOnly;
		else if (offered==SalStreamRecvOnly)
			res=SalStreamSendOnly;
		else if (offered==SalStreamInactive)
			res=SalStreamInactive;
		else
			res=SalStreamSendRecv;
	}else if (local==SalStreamSendOnly){
		if (offered==SalStreamRecvOnly || offered==SalStreamSendRecv)
			res=SalStreamSendOnly;
		else res=SalStreamInactive;
	}else if (local==SalStreamRecvOnly){
		if (offered==SalStreamSendOnly || offered==SalStreamSendRecv)
			res=SalStreamRecvOnly;
		else
			res=SalStreamInactive;
	}else res=SalStreamInactive;
	return res;
}

static void initiate_outgoing(MSFactory* factory, const SalStreamDescription & local_offer,
						const SalStreamDescription & remote_answer,
						SalStreamDescription & result){
	if (remote_answer.enabled())
		result.payloads=match_payloads(factory, local_offer.payloads,remote_answer.payloads,TRUE,FALSE);
	else {
		ms_message("Local stream description [%p] rejected by peer",&local_offer);
		result.rtp_port=0;
		return;
	}
	result.proto=remote_answer.proto;
	result.type=local_offer.type;

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
		if (local_offer.rtp_addr.compare(remote_answer.rtp_addr) !=0 ) {
			ms_message("Remote answered IP [%s] does not match offered [%s] for local stream description [%p]"
																,L_STRING_TO_C(remote_answer.rtp_addr)
																,L_STRING_TO_C(local_offer.rtp_addr)
																,&local_offer);
			result.rtp_port=0;
			return;
		}
		if (local_offer.rtp_port!=remote_answer.rtp_port) {
			ms_message("Remote answered rtp port [%i] does not match offered [%i] for local stream description [%p]"
																,remote_answer.rtp_port
																,local_offer.rtp_port
																,&local_offer);
			result.rtp_port=0;
			return;
		}
		if (local_offer.dir!=remote_answer.dir) {
			ms_message("Remote answered dir [%s] does not match offered [%s] for local stream description [%p]"
																,sal_stream_dir_to_string(remote_answer.dir)
																,sal_stream_dir_to_string(local_offer.dir)
																,&local_offer);
			result.rtp_port=0;
			return;
		}
		if (local_offer.bandwidth!=remote_answer.bandwidth) {
			ms_message("Remote answered bandwidth [%i] does not match offered [%i] for local stream description [%p]"
																,remote_answer.bandwidth
																,local_offer.bandwidth
																,&local_offer);
			result.rtp_port=0;
			return;
		}
		if (local_offer.ptime > 0 && local_offer.ptime!=remote_answer.ptime) {
			ms_message("Remote answered ptime [%i] does not match offered [%i] for local stream description [%p]"
																,remote_answer.ptime
																,local_offer.ptime
																,&local_offer);
			result.rtp_port=0;
			return;
		}
		if (local_offer.ttl > 0 && local_offer.ttl!=remote_answer.ttl) {
			ms_message("Remote answered ttl [%i] does not match offered [%i] for local stream description [%p]"
																		,remote_answer.ttl
																		,local_offer.ttl
																		,&local_offer);
			result.rtp_port=0;
			return;
		}
		result.ttl=local_offer.ttl;
		result.dir=local_offer.dir;
		result.multicast_role = SalMulticastSender;
	} else {
		result.dir=compute_dir_outgoing(local_offer.dir,remote_answer.dir);
	}

	result.rtcp_mux = remote_answer.rtcp_mux && local_offer.rtcp_mux;
	if (remote_answer.mid.empty() == false){
		if (local_offer.mid.empty() == false){
			result.mid = remote_answer.mid;
			result.mid_rtp_ext_header_id = remote_answer.mid_rtp_ext_header_id;
			result.bundle_only = remote_answer.bundle_only;
			result.rtcp_mux = TRUE; /* RTCP mux must be enabled in bundle mode. */
		}else{
			ms_error("The remote has set a mid in an answer while we didn't offered it.");
		}
	}

	if (result.payloads && !only_telephone_event(result.payloads)){
		result.rtp_addr=remote_answer.rtp_addr;
		result.rtcp_addr=remote_answer.rtcp_addr;
		result.rtp_port=remote_answer.rtp_port;
		result.rtcp_port=remote_answer.rtcp_port;
		result.bandwidth=remote_answer.bandwidth;
		result.ptime=remote_answer.ptime;
		result.maxptime=remote_answer.maxptime;
	}else{
		result.disable();
	}
	if (result.hasSrtp() == TRUE) {
		/* verify crypto algo */
		result.crypto.clear();
		SalSrtpCryptoAlgo crypto_result;
		if (!match_crypto_algo(local_offer.crypto, remote_answer.crypto, crypto_result, &result.crypto_local_tag, FALSE)) {
			result.disable();
		}
		if (result.crypto.empty()) {
			result.crypto.resize(1);
		}
		result.crypto[0] = crypto_result;
	}
	result.rtp_ssrc=local_offer.rtp_ssrc;
	result.rtcp_cname=local_offer.rtcp_cname;

	// Handle dtls session attribute: if both local and remote have a dtls fingerprint and a dtls setup, get the remote fingerprint into the result
	if ((local_offer.dtls_role!=SalDtlsRoleInvalid) && (remote_answer.dtls_role!=SalDtlsRoleInvalid)
			&&(!local_offer.dtls_fingerprint.empty()) && (!remote_answer.dtls_fingerprint.empty())) {
		result.dtls_fingerprint = remote_answer.dtls_fingerprint;
		if (remote_answer.dtls_role==SalDtlsRoleIsClient) {
			result.dtls_role = SalDtlsRoleIsServer;
		} else {
			result.dtls_role = SalDtlsRoleIsClient;
		}
	} else {
		result.dtls_fingerprint.clear();
		result.dtls_role = SalDtlsRoleInvalid;
	}
	result.implicit_rtcp_fb = local_offer.implicit_rtcp_fb && remote_answer.implicit_rtcp_fb;
}


static void initiate_incoming(MSFactory *factory, const SalStreamDescription & local_cap,
						const SalStreamDescription & remote_offer,
						SalStreamDescription & result, bool_t one_matching_codec, const char *bundle_owner_mid){
	result.payloads=match_payloads(factory, local_cap.payloads,remote_offer.payloads, FALSE, one_matching_codec);
	result.proto=remote_offer.proto;
	result.type=local_cap.type;
	result.dir=compute_dir_incoming(local_cap.dir,remote_offer.dir);
	
	if (!result.payloads || only_telephone_event(result.payloads) || !remote_offer.enabled()){
		result.rtp_port=0;
		return;
	}
	if (remote_offer.rtp_addr.empty() == false && ms_is_multicast(L_STRING_TO_C(remote_offer.rtp_addr))) {
		if (result.hasSrtp() == TRUE) {
			ms_message("SAVP not supported for multicast address for remote stream [%p]",&remote_offer);
			result.rtp_port=0;
			return;
		}
		result.dir=remote_offer.dir;
		result.rtp_addr=remote_offer.rtp_addr;
		result.rtcp_addr=remote_offer.rtcp_addr;
		result.rtp_port=remote_offer.rtp_port;
		/*result.rtcp_port=remote_offer.rtcp_port;*/
		result.rtcp_port=0; /* rtcp not supported yet*/
		result.bandwidth=remote_offer.bandwidth;
		result.ptime=remote_offer.ptime;
		result.maxptime=remote_offer.maxptime;
		result.ttl=remote_offer.ttl;
		result.multicast_role = SalMulticastReceiver;
	} else {
		result.rtp_addr=local_cap.rtp_addr;
		result.rtcp_addr=local_cap.rtcp_addr;
		result.rtp_port=local_cap.rtp_port;
		result.rtcp_port=local_cap.rtcp_port;
		result.bandwidth=local_cap.bandwidth;
		result.ptime=local_cap.ptime;
		result.maxptime=local_cap.maxptime;
	}
	
	result.rtcp_mux = remote_offer.rtcp_mux && local_cap.rtcp_mux;
	
	/* Handle RTP bundle negociation */
	if (!remote_offer.mid.empty() && bundle_owner_mid){
		result.mid = remote_offer.mid;
		result.mid_rtp_ext_header_id = remote_offer.mid_rtp_ext_header_id;
		
		if (remote_offer.mid.compare(bundle_owner_mid) != 0){
			/* The stream is a secondary one part of a bundle.
			 * In this case it must set the bundle-only attribute, and set port to zero.*/
			result.bundle_only = TRUE;
			result.rtp_port = 0;
		}
		result.rtcp_mux = TRUE; /* RTCP mux must be enabled in bundle mode. */
	}

	if (result.hasSrtp() == TRUE) {
		/* select crypto algo */
		result.crypto.clear();
		SalSrtpCryptoAlgo crypto_result;
		if (!match_crypto_algo(local_cap.crypto, remote_offer.crypto, crypto_result, &result.crypto_local_tag, TRUE)) {
			result.disable();
			ms_message("No matching crypto algo for remote stream's offer [%p]",&remote_offer);
		}
		if (result.crypto.empty()) {
			result.crypto.resize(1);
		}
		result.crypto[0] = crypto_result;
	}

	// add our zrtp hash if remote gave one but also when our side has set ZRTP as active to help peer starting earlier if it has ZRTP capabilities
	// haveZrtpHash is set in local_cap when ZRTP is active on our side.
	if ((remote_offer.haveZrtpHash == 1) || (local_cap.haveZrtpHash == 1)) {
		if (local_cap.zrtphash[0] != 0) { /* if ZRTP is available, set the zrtp hash even if it is not selected */
			strncpy((char *)(result.zrtphash), (char *)(local_cap.zrtphash), sizeof(result.zrtphash));
			result.haveZrtpHash =  1;
		}
	}

	result.ice_pwd = local_cap.ice_pwd;
	result.ice_ufrag = local_cap.ice_ufrag;
	result.ice_mismatch = local_cap.ice_mismatch;
	result.set_nortpproxy = local_cap.set_nortpproxy;
	result.ice_candidates = local_cap.ice_candidates;
	result.ice_remote_candidates = local_cap.ice_remote_candidates;
	result.name = local_cap.name;
	result.rtp_ssrc=local_cap.rtp_ssrc;
	result.rtcp_cname=local_cap.rtcp_cname;

	// Handle dtls stream attribute: if both local and remote have a dtls fingerprint and a dtls setup, add the local fingerprint to the answer
	// Note: local description usually stores dtls config at session level which means it apply to all streams, check this too
	if (((local_cap.dtls_role!=SalDtlsRoleInvalid)) && (remote_offer.dtls_role!=SalDtlsRoleInvalid)
			&& (!local_cap.dtls_fingerprint.empty()) && (!remote_offer.dtls_fingerprint.empty())) {
		result.dtls_fingerprint = local_cap.dtls_fingerprint;
		if (remote_offer.dtls_role==SalDtlsRoleUnset) {
			result.dtls_role = SalDtlsRoleIsClient;
		}
	} else {
		result.dtls_fingerprint.clear();
		result.dtls_role = SalDtlsRoleInvalid;
	}
	result.implicit_rtcp_fb = local_cap.implicit_rtcp_fb && remote_offer.implicit_rtcp_fb;
}

static bool are_proto_compatibles(SalMediaProto localProto, SalMediaProto otherProto)
{
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
int offer_answer_initiate_outgoing(MSFactory *factory, std::shared_ptr<SalMediaDescription> local_offer,
					const std::shared_ptr<SalMediaDescription> remote_answer,
					std::shared_ptr<SalMediaDescription> result){
	size_t i;

	for(i=0;i<local_offer->streams.size();++i){
		ms_message("Processing for stream %zu",i);
		SalStreamDescription & ls = local_offer->streams[i];
		const SalStreamDescription & rs = remote_answer->streams[i];
		if ((i < remote_answer->streams.size()) && rs.type == ls.type && are_proto_compatibles(ls.proto, rs.proto))
		{
			if (ls.proto != rs.proto && ls.hasAvpf()) {
				ls.proto = rs.proto;
				ms_warning("Received a downgraded AVP answer for our AVPF offer");
			}
			if (i <= result->streams.size()) {
				result->streams.resize((i + 1));
			}
			initiate_outgoing(factory, ls,rs,result->streams[i]);
			memcpy(&result->streams[i].rtcp_xr, &ls.rtcp_xr, sizeof(result->streams[i].rtcp_xr));
			if ((ls.rtcp_xr.enabled == TRUE) && (rs.rtcp_xr.enabled == FALSE)) {
				result->streams[i].rtcp_xr.enabled = FALSE;
			}
			result->streams[i].rtcp_fb.generic_nack_enabled = ls.rtcp_fb.generic_nack_enabled & rs.rtcp_fb.generic_nack_enabled;
			result->streams[i].rtcp_fb.tmmbr_enabled = ls.rtcp_fb.tmmbr_enabled & rs.rtcp_fb.tmmbr_enabled;
		}
		else ms_warning("No matching stream for %zu",i);
	}
	result->bandwidth=remote_answer->bandwidth;
	result->addr=remote_answer->addr;
	result->ice_pwd = local_offer->ice_pwd;
	result->ice_ufrag = local_offer->ice_ufrag;
	memcpy(&result->rtcp_xr, &local_offer->rtcp_xr, sizeof(result->rtcp_xr));
	if ((local_offer->rtcp_xr.enabled == TRUE) && (remote_answer->rtcp_xr.enabled == FALSE)) {
		result->rtcp_xr.enabled = FALSE;
	}
	/* TODO: check that the bundle answer is compliant with our offer.
	 * For now, just check the presence of a bundle response. */
	if (local_offer->bundles){
		if (remote_answer->bundles){
			/* Copy the bundle offering to the result media description. */
			result->bundles = bctbx_list_copy_with_data(remote_answer->bundles, (bctbx_list_copy_func) sal_stream_bundle_clone);
		}
	}else if (remote_answer->bundles){
		ms_error("Remote answerer is proposing bundles, which we did not offer.");
	}

	return 0;
}

/**
 * Returns a media description to run the streams with, based on the local capabilities and
 * and the received offer.
 * The returned media description is an answer and should be sent to the offerer.
**/
int offer_answer_initiate_incoming(MSFactory *factory, const std::shared_ptr<SalMediaDescription> local_capabilities,
					std::shared_ptr<SalMediaDescription> remote_offer,
					std::shared_ptr<SalMediaDescription> result, bool_t one_matching_codec){
	size_t i;

	if (remote_offer->bundles && local_capabilities->accept_bundles){
		/* Copy the bundle offering to the result media description. */
		result->bundles = bctbx_list_copy_with_data(remote_offer->bundles, (bctbx_list_copy_func) sal_stream_bundle_clone);
	}
	
	for(i=0;i<remote_offer->streams.size();++i){

		if (i >= local_capabilities->streams.size()) {
			local_capabilities->streams.resize((i + 1));
		}
		const SalStreamDescription & ls = local_capabilities->streams[i];
		SalStreamDescription & rs = remote_offer->streams[i];
		if (i >= result->streams.size()) {
			result->streams.resize((i + 1));
		}
		if (rs.type == ls.type && are_proto_compatibles(ls.proto, rs.proto))
		{
			if (ls.proto != rs.proto && rs.hasAvpf())	{
				rs.proto = ls.proto;
				ms_warning("Sending a downgraded AVP answer for the received AVPF offer");
			}
			const char *bundle_owner_mid = NULL;
			if (local_capabilities->accept_bundles){
				int owner_index = remote_offer->getIndexOfTransportOwner(rs);
				if (owner_index != -1){
					bundle_owner_mid = L_STRING_TO_C(remote_offer->streams[(size_t)owner_index].mid);
				}
			}
			initiate_incoming(factory, ls,rs,result->streams[i],one_matching_codec, bundle_owner_mid);
			// Handle global RTCP FB attributes
			result->streams[i].rtcp_fb.generic_nack_enabled = rs.rtcp_fb.generic_nack_enabled;
			result->streams[i].rtcp_fb.tmmbr_enabled = rs.rtcp_fb.tmmbr_enabled;
			// Handle media RTCP XR attribute
			memset(&result->streams[i].rtcp_xr, 0, sizeof(result->streams[i].rtcp_xr));
			if (rs.rtcp_xr.enabled == TRUE) {
				const OrtpRtcpXrConfiguration *rtcp_xr_conf = NULL;
				if (ls.rtcp_xr.enabled == TRUE) rtcp_xr_conf = &ls.rtcp_xr;
				else if (local_capabilities->rtcp_xr.enabled == TRUE) rtcp_xr_conf = &local_capabilities->rtcp_xr;
				if ((rtcp_xr_conf != NULL) && (ls.dir == SalStreamSendRecv)) {
					memcpy(&result->streams[i].rtcp_xr, rtcp_xr_conf, sizeof(result->streams[i].rtcp_xr));
				} else {
					result->streams[i].rtcp_xr.enabled = TRUE;
				}
			}
		}else {
			ms_message("Declining mline %zu, no corresponding stream in local capabilities description.",i);
			/* create an inactive stream for the answer, as there where no matching stream in local capabilities */
			result->streams[i].dir=SalStreamInactive;
			result->streams[i].rtp_port=0;
			result->streams[i].type=rs.type;
			result->streams[i].proto=rs.proto;
			if (rs.type==SalOther){
				result->streams[i].typeother = rs.typeother;
			}
			if (rs.proto==SalProtoOther){
				result->streams[i].proto_other = rs.proto_other;
			}
		}
		result->streams[i].custom_sdp_attributes = sal_custom_sdp_attribute_clone(ls.custom_sdp_attributes);
	}
	result->username=local_capabilities->username;
	result->addr=local_capabilities->addr;
	result->bandwidth=local_capabilities->bandwidth;
	result->session_ver=local_capabilities->session_ver;
	result->session_id=local_capabilities->session_id;
	result->ice_pwd = local_capabilities->ice_pwd;
	result->ice_ufrag = local_capabilities->ice_ufrag;
	result->ice_lite = local_capabilities->ice_lite;
	result->set_nortpproxy = local_capabilities->set_nortpproxy;
	result->custom_sdp_attributes = sal_custom_sdp_attribute_clone(local_capabilities->custom_sdp_attributes);

	result->name=local_capabilities->name;

	// Handle session RTCP XR attribute
	memset(&result->rtcp_xr, 0, sizeof(result->rtcp_xr));
	if (remote_offer->rtcp_xr.enabled == TRUE) {
		if ((local_capabilities->rtcp_xr.enabled == TRUE) && (local_capabilities->dir == SalStreamSendRecv)) {
			memcpy(&result->rtcp_xr, &local_capabilities->rtcp_xr, sizeof(result->rtcp_xr));
		} else {
			result->rtcp_xr.enabled = TRUE;
		}
	}

	return 0;
}
