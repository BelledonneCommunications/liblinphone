/*
linphone
Copyright (C) 2010  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "sal/sal.h"
#include "offeranswer.h"
#include "private.h"

static bool_t only_telephone_event(const MSList *l){
	for(;l!=NULL;l=l->next){
		PayloadType *p=(PayloadType*)l->data;
		if (strcasecmp(p->mime_type,"telephone-event")!=0){
			return FALSE;
		}
	}
	return TRUE;
}

typedef struct _PayloadTypeMatcher{
	const char *mime_type;
	PayloadType *(*match_func)(const MSList *l, const PayloadType *refpt);
}PayloadTypeMatcher;

static PayloadType * opus_match(const MSList *l, const PayloadType *refpt){
	PayloadType *pt;
	const MSList *elem;
	PayloadType *candidate=NULL;

	for (elem=l;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		/*workaround a bug in earlier versions of linphone where opus/48000/1 is offered, which is uncompliant with opus rtp draft*/
		if (strcasecmp(pt->mime_type,"opus")==0 ){
			if (refpt->channels==1){
				pt->channels=1; /*so that we respond with same number of channels */
				candidate=pt;
			}else if (refpt->channels==2){
				return pt;
			}
		}
	}
	return candidate;
}

/* the reason for this matcher is for some stupid uncompliant phone that offer G729a mime type !*/
static PayloadType * g729A_match(const MSList *l, const PayloadType *refpt){
	PayloadType *pt;
	const MSList *elem;
	PayloadType *candidate=NULL;

	for (elem=l;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		if (strcasecmp(pt->mime_type,"G729")==0 && refpt->channels==pt->channels){
			candidate=pt;
		}
	}
	return candidate;
}

static PayloadType * amr_match(const MSList *l, const PayloadType *refpt){
	PayloadType *pt;
	char value[10];
	const MSList *elem;
	PayloadType *candidate=NULL;

	for (elem=l;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		if ( pt->mime_type && refpt->mime_type 
			&& strcasecmp(pt->mime_type, refpt->mime_type)==0
			&& pt->clock_rate==refpt->clock_rate
			&& pt->channels==refpt->channels) {
			int octedalign1=0,octedalign2=0;
			if (pt->recv_fmtp!=NULL && fmtp_get_value(pt->recv_fmtp,"octet-align",value,sizeof(value))){
				octedalign1=atoi(value);
			}
			if (refpt->send_fmtp!=NULL && fmtp_get_value(refpt->send_fmtp,"octet-align",value,sizeof(value))){
				octedalign2=atoi(value);
			}
			if (octedalign1==octedalign2) {
				candidate=pt;
				break; /*exact match */
			}
		}
	}
	return candidate;
}

static PayloadType * generic_match(const MSList *l, const PayloadType *refpt){
	PayloadType *pt;
	const MSList *elem;

	for (elem=l;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		
		if ( pt->mime_type && refpt->mime_type 
			&& strcasecmp(pt->mime_type, refpt->mime_type)==0
			&& pt->clock_rate==refpt->clock_rate
			&& pt->channels==refpt->channels)
			return pt;
	}
	return NULL;
}

static PayloadTypeMatcher matchers[]={
	{"opus", opus_match},
	{"G729A", g729A_match},
	{"AMR", amr_match},
	{"AMR-WB", amr_match},
	{NULL, NULL}
};


/*
 * Returns a PayloadType from the local list that matches a PayloadType offered or answered in the remote list
*/
static PayloadType * find_payload_type_best_match(const MSList *l, const PayloadType *refpt){
	PayloadTypeMatcher *m;
	for(m=matchers;m->mime_type!=NULL;++m){
		if (refpt->mime_type && strcasecmp(m->mime_type,refpt->mime_type)==0){
			return m->match_func(l,refpt);
		}
	}
	return generic_match(l,refpt);
}


static MSList *match_payloads(const MSList *local, const MSList *remote, bool_t reading_response, bool_t one_matching_codec){
	const MSList *e2,*e1;
	MSList *res=NULL;
	PayloadType *matched;
	bool_t found_codec=FALSE;

	for(e2=remote;e2!=NULL;e2=e2->next){
		PayloadType *p2=(PayloadType*)e2->data;
		matched=find_payload_type_best_match(local,p2);
		if (matched){
			PayloadType *newp;
			int local_number=payload_type_get_number(matched);
			int remote_number=payload_type_get_number(p2);

			if (one_matching_codec){
				if (strcasecmp(matched->mime_type,"telephone-event")!=0){
					if (found_codec){/* we have found a real codec already*/
						continue; /*this codec won't be added*/
					}else found_codec=TRUE;
				}
			}

			newp=payload_type_clone(matched);
			if (p2->send_fmtp){
				payload_type_append_send_fmtp(newp,p2->send_fmtp);
			}
			newp->flags|=PAYLOAD_TYPE_FLAG_CAN_RECV|PAYLOAD_TYPE_FLAG_CAN_SEND;
			if (p2->flags & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED) {
				newp->flags |= PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED;
				/* Negotiation of AVPF features (keep common features) */
				newp->avpf.features &= p2->avpf.features;
				newp->avpf.rpsi_compatibility = p2->avpf.rpsi_compatibility;
				/* Take bigger AVPF trr interval */
				if (p2->avpf.trr_interval < matched->avpf.trr_interval) {
					newp->avpf.trr_interval = matched->avpf.trr_interval;
				}
			}
			res=ms_list_append(res,newp);
			/* we should use the remote numbering even when parsing a response */
			payload_type_set_number(newp,remote_number);
			payload_type_set_flag(newp, PAYLOAD_TYPE_FROZEN_NUMBER);
			if (reading_response && remote_number!=local_number){
				ms_warning("For payload type %s, proposed number was %i but the remote phone answered %i",
						  newp->mime_type, local_number, remote_number);
				/*
				 We must add this payload type with our local numbering in order to be able to receive it.
				 Indeed despite we must sent with the remote numbering, we must be able to receive with
				 our local one.
				*/
				newp=payload_type_clone(newp);
				payload_type_set_number(newp,local_number);
				payload_type_set_flag(newp, PAYLOAD_TYPE_FROZEN_NUMBER);
				res=ms_list_append(res,newp);
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
				res=ms_list_append(res,p1);
			}
		}
	}
	return res;
}

static bool_t match_crypto_algo(const SalSrtpCryptoAlgo* local, const SalSrtpCryptoAlgo* remote,
	SalSrtpCryptoAlgo* result, unsigned int* choosen_local_tag, bool_t use_local_key) {
	int i,j;
	for(i=0; i<SAL_CRYPTO_ALGO_MAX; i++) {
		if (remote[i].algo == 0)
			break;

		/* Look for a local enabled crypto algo that matches one of the proposed by remote */
		for(j=0; j<SAL_CRYPTO_ALGO_MAX; j++) {
			if (remote[i].algo == local[j].algo) {
				result->algo = remote[i].algo;
				/* We're answering an SDP offer. Supply our master key, associated with the remote supplied tag */
				if (use_local_key) {
					strncpy(result->master_key, local[j].master_key, sizeof(result->master_key) );
					result->tag = remote[i].tag;
					*choosen_local_tag = local[j].tag;
				}
				/* We received an answer to our SDP crypto proposal. Copy matching algo remote master key to result, and memorize local tag */
				else {
					strncpy(result->master_key, remote[i].master_key, sizeof(result->master_key));
					result->tag = local[j].tag;
					*choosen_local_tag = local[j].tag;
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

static void initiate_outgoing(const SalStreamDescription *local_offer,
						const SalStreamDescription *remote_answer,
						SalStreamDescription *result){
	if (remote_answer->rtp_port!=0)
		result->payloads=match_payloads(local_offer->payloads,remote_answer->payloads,TRUE,FALSE);
	else {
		ms_message("Local stream description [%p] rejected by peer",local_offer);
		result->rtp_port=0;
		return;
	}
	result->proto=remote_answer->proto;
	result->type=local_offer->type;

	if (local_offer->rtp_addr[0]!='\0' && ms_is_multicast(local_offer->rtp_addr)) {
		/*6.2 Multicast Streams
		...
	   If a multicast stream is accepted, the address and port information
	   in the answer MUST match that of the offer.  Similarly, the
	   directionality information in the answer (sendonly, recvonly, or
	   sendrecv) MUST equal that of the offer.  This is because all
	   participants in a multicast session need to have equivalent views of
	   the parameters of the session, an underlying assumption of the
	   multicast bias of RFC 2327.*/
	  if (strcmp(local_offer->rtp_addr,remote_answer->rtp_addr) !=0 ) {
		  ms_message("Remote answered IP [%s] does not match offered [%s] for local stream description [%p]"
				  	  	  	  	  	  	  	  	  	  	  	  ,remote_answer->rtp_addr
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer->rtp_addr
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer);
		  result->rtp_port=0;
		  return;
	  }
	  if (local_offer->rtp_port!=remote_answer->rtp_port) {
		  ms_message("Remote answered rtp port [%i] does not match offered [%i] for local stream description [%p]"
				  	  	  	  	  	  	  	  	  	  	  	  ,remote_answer->rtp_port
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer->rtp_port
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer);
		  result->rtp_port=0;
		  return;
	  }
	  if (local_offer->dir!=remote_answer->dir) {
		  ms_message("Remote answered dir [%s] does not match offered [%s] for local stream description [%p]"
				  	  	  	  	  	  	  	  	  	  	  	  ,sal_stream_dir_to_string(remote_answer->dir)
				  	  	  	  	  	  	  	  	  	  	  	  ,sal_stream_dir_to_string(local_offer->dir)
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer);
		  result->rtp_port=0;
		  return;
	  }
	  if (local_offer->bandwidth!=remote_answer->bandwidth) {
		  ms_message("Remote answered bandwidth [%i] does not match offered [%i] for local stream description [%p]"
				  	  	  	  	  	  	  	  	  	  	  	  ,remote_answer->bandwidth
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer->bandwidth
				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer);
		  result->rtp_port=0;
		  return;
	  }
	  if (local_offer->ptime > 0 && local_offer->ptime!=remote_answer->ptime) {
	  		  ms_message("Remote answered ptime [%i] does not match offered [%i] for local stream description [%p]"
	  				  	  	  	  	  	  	  	  	  	  	  	  ,remote_answer->ptime
	  				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer->ptime
	  				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer);
	  	  result->rtp_port=0;
	  	  return;
	  }
	  if (local_offer->ttl > 0 && local_offer->ttl!=remote_answer->ttl) {
	  	  ms_message("Remote answered ttl [%i] does not match offered [%i] for local stream description [%p]"
	  	  				  	  	  	  	  	  	  	  	  	  	  	  ,remote_answer->ttl
	  	  				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer->ttl
	  	  				  	  	  	  	  	  	  	  	  	  	  	  ,local_offer);
	   	  result->rtp_port=0;
	   	  return;
	   }
	  result->ttl=local_offer->ttl;
	  result->dir=local_offer->dir;
	  result->multicast_role = SalMulticastSender;

	} else {
		result->dir=compute_dir_outgoing(local_offer->dir,remote_answer->dir);
	}



	if (result->payloads && !only_telephone_event(result->payloads)){
		strcpy(result->rtp_addr,remote_answer->rtp_addr);
		strcpy(result->rtcp_addr,remote_answer->rtcp_addr);
		result->rtp_port=remote_answer->rtp_port;
		result->rtcp_port=remote_answer->rtcp_port;
		result->bandwidth=remote_answer->bandwidth;
		result->ptime=remote_answer->ptime;
	}else{
		result->rtp_port=0;
	}
	if (sal_stream_description_has_srtp(result) == TRUE) {
		/* verify crypto algo */
		memset(result->crypto, 0, sizeof(result->crypto));
		if (!match_crypto_algo(local_offer->crypto, remote_answer->crypto, &result->crypto[0], &result->crypto_local_tag, FALSE))
			result->rtp_port = 0;
	}
	result->rtp_ssrc=local_offer->rtp_ssrc;
	strncpy(result->rtcp_cname,local_offer->rtcp_cname,sizeof(result->rtcp_cname));

	// Handle dtls session attribute: if both local and remote have a dtls fingerprint and a dtls setup, get the remote fingerprint into the result
	if ((local_offer->dtls_role!=SalDtlsRoleInvalid) && (remote_answer->dtls_role!=SalDtlsRoleInvalid)
			&&(strlen(local_offer->dtls_fingerprint)>0) && (strlen(remote_answer->dtls_fingerprint)>0)) {
		strncpy(result->dtls_fingerprint, remote_answer->dtls_fingerprint,sizeof(result->dtls_fingerprint));
		if (remote_answer->dtls_role==SalDtlsRoleIsClient) {
			result->dtls_role = SalDtlsRoleIsServer;
		} else {
			result->dtls_role = SalDtlsRoleIsClient;
		}
	} else {
		result->dtls_fingerprint[0] = '\0';
		result->dtls_role = SalDtlsRoleInvalid;
	}


}


static void initiate_incoming(const SalStreamDescription *local_cap,
						const SalStreamDescription *remote_offer,
						SalStreamDescription *result, bool_t one_matching_codec){
	result->payloads=match_payloads(local_cap->payloads,remote_offer->payloads, FALSE, one_matching_codec);
	result->proto=remote_offer->proto;
	result->type=local_cap->type;
	result->dir=compute_dir_incoming(local_cap->dir,remote_offer->dir);
	if (!result->payloads || only_telephone_event(result->payloads) || remote_offer->rtp_port==0 || remote_offer->dir==SalStreamRecvOnly){
		result->rtp_port=0;
		return;
	}
	if (remote_offer->rtp_addr[0]!='\0' && ms_is_multicast(remote_offer->rtp_addr)) {
		if (sal_stream_description_has_srtp(result) == TRUE) {
			ms_message("SAVP not supported for multicast address for remote stream [%p]",remote_offer);
			result->rtp_port=0;
			return;
		}
		result->dir=remote_offer->dir;
		strcpy(result->rtp_addr,remote_offer->rtp_addr);
		strcpy(result->rtcp_addr,remote_offer->rtcp_addr);
		result->rtp_port=remote_offer->rtp_port;
		/*result->rtcp_port=remote_offer->rtcp_port;*/
		result->rtcp_port=0; /* rtcp not supported yet*/
		result->bandwidth=remote_offer->bandwidth;
		result->ptime=remote_offer->ptime;
		result->ttl=remote_offer->ttl;
		result->multicast_role = SalMulticastReceiver;
	} else {
		strcpy(result->rtp_addr,local_cap->rtp_addr);
		strcpy(result->rtcp_addr,local_cap->rtcp_addr);
		result->rtp_port=local_cap->rtp_port;
		result->rtcp_port=local_cap->rtcp_port;
		result->bandwidth=local_cap->bandwidth;
		result->ptime=local_cap->ptime;
	}
	if (sal_stream_description_has_srtp(result) == TRUE) {
		/* select crypto algo */
		memset(result->crypto, 0, sizeof(result->crypto));
		if (!match_crypto_algo(local_cap->crypto, remote_offer->crypto, &result->crypto[0], &result->crypto_local_tag, TRUE))
			result->rtp_port = 0;

	}
	strcpy(result->ice_pwd, local_cap->ice_pwd);
	strcpy(result->ice_ufrag, local_cap->ice_ufrag);
	result->ice_mismatch = local_cap->ice_mismatch;
	result->ice_completed = local_cap->ice_completed;
	memcpy(result->ice_candidates, local_cap->ice_candidates, sizeof(result->ice_candidates));
	memcpy(result->ice_remote_candidates, local_cap->ice_remote_candidates, sizeof(result->ice_remote_candidates));
	strcpy(result->name,local_cap->name);
	result->rtp_ssrc=local_cap->rtp_ssrc;
	strncpy(result->rtcp_cname,local_cap->rtcp_cname,sizeof(result->rtcp_cname));

	// Handle dtls stream attribute: if both local and remote have a dtls fingerprint and a dtls setup, add the local fingerprint to the answer
	// Note: local description usually stores dtls config at session level which means it apply to all streams, check this too
	if (((local_cap->dtls_role!=SalDtlsRoleInvalid)) && (remote_offer->dtls_role!=SalDtlsRoleInvalid)
			&& (strlen(local_cap->dtls_fingerprint)>0) && (strlen(remote_offer->dtls_fingerprint)>0)) {
		strncpy(result->dtls_fingerprint, local_cap->dtls_fingerprint,sizeof(result->dtls_fingerprint));
		if (remote_offer->dtls_role==SalDtlsRoleUnset) {
			result->dtls_role = SalDtlsRoleIsClient;
		}
	} else {
		result->dtls_fingerprint[0] = '\0';
		result->dtls_role = SalDtlsRoleInvalid;
	}


}


/**
 * Returns a media description to run the streams with, based on a local offer
 * and the returned response (remote).
**/
int offer_answer_initiate_outgoing(const SalMediaDescription *local_offer,
					const SalMediaDescription *remote_answer,
					SalMediaDescription *result){
	int i,j;
	const SalStreamDescription *ls,*rs;

	for(i=0,j=0;i<local_offer->nb_streams;++i){
		ms_message("Processing for stream %i",i);
		ls=&local_offer->streams[i];
		rs=sal_media_description_find_stream((SalMediaDescription*)remote_answer,ls->proto,ls->type);
		if (rs) {
			initiate_outgoing(ls,rs,&result->streams[j]);
			memcpy(&result->streams[i].rtcp_xr, &ls->rtcp_xr, sizeof(result->streams[i].rtcp_xr));
			if ((ls->rtcp_xr.enabled == TRUE) && (rs->rtcp_xr.enabled == FALSE)) {
				result->streams[i].rtcp_xr.enabled = FALSE;
			}
			result->streams[i].rtcp_fb.generic_nack_enabled = ls->rtcp_fb.generic_nack_enabled & rs->rtcp_fb.generic_nack_enabled;
			result->streams[i].rtcp_fb.tmmbr_enabled = ls->rtcp_fb.tmmbr_enabled & rs->rtcp_fb.tmmbr_enabled;
			++j;
		}
		else ms_warning("No matching stream for %i",i);
	}
	result->nb_streams=local_offer->nb_streams;
	result->bandwidth=remote_answer->bandwidth;
	strcpy(result->addr,remote_answer->addr);
	memcpy(&result->rtcp_xr, &local_offer->rtcp_xr, sizeof(result->rtcp_xr));
	if ((local_offer->rtcp_xr.enabled == TRUE) && (remote_answer->rtcp_xr.enabled == FALSE)) {
		result->rtcp_xr.enabled = FALSE;
	}

	return 0;
}

static bool_t local_stream_not_already_used(const SalMediaDescription *result, const SalStreamDescription *stream){
	int i;
	for(i=0;i<result->nb_streams;++i){
		const SalStreamDescription *ss=&result->streams[i];
		if (strcmp(ss->name,stream->name)==0){
			ms_message("video stream already used in answer");
			return FALSE;
		}
	}
	return TRUE;
}

/*in answering mode, we consider that if we are able to make AVPF/SAVP/SAVPF, then we can do AVP as well*/
static bool_t proto_compatible(SalMediaProto local, SalMediaProto remote) {
	if (local == remote) return TRUE;
	if ((remote == SalProtoRtpAvp) && ((local == SalProtoRtpSavp) || (local == SalProtoRtpSavpf))) return TRUE;
	if ((remote == SalProtoRtpAvp) && ((local == SalProtoUdpTlsRtpSavp) || (local == SalProtoUdpTlsRtpSavpf))) return TRUE;
	if ((remote == SalProtoRtpAvpf) && (local == SalProtoRtpSavpf)) return TRUE;
	if ((remote == SalProtoRtpAvpf) && (local == SalProtoUdpTlsRtpSavpf)) return TRUE;
	return FALSE;
}

static const SalStreamDescription *find_local_matching_stream(const SalMediaDescription *result, const SalMediaDescription *local_capabilities, const SalStreamDescription *remote_stream){
	int i;
	for(i=0;i<local_capabilities->nb_streams;++i){
		const SalStreamDescription *ss=&local_capabilities->streams[i];
		if (!sal_stream_description_active(ss)) continue;
		if (ss->type==remote_stream->type && proto_compatible(ss->proto,remote_stream->proto)
			&& local_stream_not_already_used(result,ss)) return ss;
	}
	return NULL;
}

/**
 * Returns a media description to run the streams with, based on the local capabilities and
 * and the received offer.
 * The returned media description is an answer and should be sent to the offerer.
**/
int offer_answer_initiate_incoming(const SalMediaDescription *local_capabilities,
					const SalMediaDescription *remote_offer,
					SalMediaDescription *result, bool_t one_matching_codec){
	int i;
	const SalStreamDescription *ls=NULL,*rs;

	for(i=0;i<remote_offer->nb_streams;++i){
		rs=&remote_offer->streams[i];
		if (rs->proto!=SalProtoOther){
			ls=find_local_matching_stream(result,local_capabilities,rs);
		}else ms_warning("Unknown protocol for mline %i, declining",i);
		if (ls){
			initiate_incoming(ls,rs,&result->streams[i],one_matching_codec);
			// Handle global RTCP FB attributes
			result->streams[i].rtcp_fb.generic_nack_enabled = rs->rtcp_fb.generic_nack_enabled;
			result->streams[i].rtcp_fb.tmmbr_enabled = rs->rtcp_fb.tmmbr_enabled;
			// Handle media RTCP XR attribute
			memset(&result->streams[i].rtcp_xr, 0, sizeof(result->streams[i].rtcp_xr));
			if (rs->rtcp_xr.enabled == TRUE) {
				const OrtpRtcpXrConfiguration *rtcp_xr_conf = NULL;
				if (ls->rtcp_xr.enabled == TRUE) rtcp_xr_conf = &ls->rtcp_xr;
				else if (local_capabilities->rtcp_xr.enabled == TRUE) rtcp_xr_conf = &local_capabilities->rtcp_xr;
				if ((rtcp_xr_conf != NULL) && (ls->dir == SalStreamSendRecv)) {
					memcpy(&result->streams[i].rtcp_xr, rtcp_xr_conf, sizeof(result->streams[i].rtcp_xr));
				} else {
					result->streams[i].rtcp_xr.enabled = TRUE;
				}
			}
		}else {
			ms_message("Declining mline %i, no corresponding stream in local capabilities description.",i);
			/* create an inactive stream for the answer, as there where no matching stream in local capabilities */
			result->streams[i].dir=SalStreamInactive;
			result->streams[i].rtp_port=0;
			result->streams[i].type=rs->type;
			result->streams[i].proto=rs->proto;
			if (rs->type==SalOther){
				strncpy(result->streams[i].typeother,rs->typeother,sizeof(rs->typeother)-1);
			}
			if (rs->proto==SalProtoOther){
				strncpy(result->streams[i].proto_other,rs->proto_other,sizeof(rs->proto_other)-1);
			}
		}
	}
	result->nb_streams=i;
	strcpy(result->username, local_capabilities->username);
	strcpy(result->addr,local_capabilities->addr);
	result->bandwidth=local_capabilities->bandwidth;
	result->session_ver=local_capabilities->session_ver;
	result->session_id=local_capabilities->session_id;
	strcpy(result->ice_pwd, local_capabilities->ice_pwd);
	strcpy(result->ice_ufrag, local_capabilities->ice_ufrag);
	result->ice_lite = local_capabilities->ice_lite;
	result->ice_completed = local_capabilities->ice_completed;

	strcpy(result->name,local_capabilities->name);

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
