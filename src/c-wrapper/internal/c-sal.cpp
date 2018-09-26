/*
 * c-sal.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
This file contains SAL API functions that do not depend on the underlying implementation (like belle-sip).
**/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "c-wrapper/internal/c-sal.h"


#include <ctype.h>


const char *sal_multicast_role_to_string(SalMulticastRole role){
	switch(role){
		case SalMulticastInactive:
			return "inactive";
		case SalMulticastReceiver:
			return "receiver";
		case SalMulticastSender:
			return "sender";
		case SalMulticastSenderReceiver:
			return "sender-receiver";
	}
	return "INVALID";
}

const char* sal_transport_to_string(SalTransport transport) {
	switch (transport) {
		case SalTransportUDP:return "udp";
		case SalTransportTCP: return "tcp";
		case SalTransportTLS:return "tls";
		case SalTransportDTLS:return "dtls";
		default: {
			ms_fatal("Unexpected transport [%i]",transport);
			return NULL;
		}
	}
}

SalTransport sal_transport_parse(const char* param) {
	if (!param) return SalTransportUDP;
	if (strcasecmp("udp",param)==0) return SalTransportUDP;
	if (strcasecmp("tcp",param)==0) return SalTransportTCP;
	if (strcasecmp("tls",param)==0) return SalTransportTLS;
	if (strcasecmp("dtls",param)==0) return SalTransportDTLS;
	ms_error("Unknown transport type[%s], returning UDP", param);
	return SalTransportUDP;
}

SalMediaDescription *sal_media_description_new(){
	SalMediaDescription *md=ms_new0(SalMediaDescription,1);
	int i;
	md->refcount=1;
	for(i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		md->streams[i].dir=SalStreamInactive;
		md->streams[i].rtp_port = 0;
		md->streams[i].rtcp_port = 0;
		md->streams[i].haveZrtpHash = 0;
	}
	return md;
}

static void sal_media_description_destroy(SalMediaDescription *md){
	int i;
	for(i=0;i<SAL_MEDIA_DESCRIPTION_MAX_STREAMS;i++){
		bctbx_list_free_with_data(md->streams[i].payloads,(void (*)(void *))payload_type_destroy);
		bctbx_list_free_with_data(md->streams[i].already_assigned_payloads,(void (*)(void *))payload_type_destroy);
		md->streams[i].payloads=NULL;
		md->streams[i].already_assigned_payloads=NULL;
		sal_custom_sdp_attribute_free(md->streams[i].custom_sdp_attributes);
	}
	sal_custom_sdp_attribute_free(md->custom_sdp_attributes);
	ms_free(md);
}

SalMediaDescription * sal_media_description_ref(SalMediaDescription *md){
	md->refcount++;
	return md;
}

void sal_media_description_unref(SalMediaDescription *md){
	md->refcount--;
	if (md->refcount==0){
		sal_media_description_destroy (md);
	}
}

SalStreamDescription *sal_media_description_find_stream(SalMediaDescription *md, SalMediaProto proto, SalStreamType type){
	int i;
	for(i=0;i<SAL_MEDIA_DESCRIPTION_MAX_STREAMS;++i){
		SalStreamDescription *ss=&md->streams[i];
		if (!sal_stream_description_active(ss)) continue;
		if (ss->proto==proto && ss->type==type) return ss;
	}
	return NULL;
}

unsigned int sal_media_description_nb_active_streams_of_type(SalMediaDescription *md, SalStreamType type) {
	unsigned int i;
	unsigned int nb = 0;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; ++i) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (md->streams[i].type == type) nb++;
	}
	return nb;
}

SalStreamDescription * sal_media_description_get_active_stream_of_type(SalMediaDescription *md, SalStreamType type, unsigned int idx) {
	unsigned int i;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; ++i) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (md->streams[i].type == type) {
			if (idx-- == 0) return &md->streams[i];
		}
	}
	return NULL;
}

SalStreamDescription * sal_media_description_find_secure_stream_of_type(SalMediaDescription *md, SalStreamType type) {
	SalStreamDescription *desc = sal_media_description_find_stream(md, SalProtoRtpSavpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpSavp, type);
	return desc;
}

SalStreamDescription * sal_media_description_find_best_stream(SalMediaDescription *md, SalStreamType type) {
	SalStreamDescription *desc = sal_media_description_find_stream(md, SalProtoUdpTlsRtpSavpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoUdpTlsRtpSavp, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpSavpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpSavp, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpAvpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpAvp, type);
	return desc;
}

bool_t sal_media_description_empty(const SalMediaDescription *md){
	if (sal_media_description_get_nb_active_streams(md) > 0) return FALSE;
	return TRUE;
}

void sal_media_description_set_dir(SalMediaDescription *md, SalStreamDir stream_dir){
	int i;
	for(i=0;i<SAL_MEDIA_DESCRIPTION_MAX_STREAMS;++i){
		SalStreamDescription *ss=&md->streams[i];
		if (!sal_stream_description_active(ss)) continue;
		ss->dir=stream_dir;
	}
}

int sal_media_description_get_nb_active_streams(const SalMediaDescription *md) {
	int i;
	int nb = 0;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (sal_stream_description_active(&md->streams[i])) nb++;
	}
	return nb;
}

static bool_t is_null_address(const char *addr){
	return strcmp(addr,"0.0.0.0")==0 || strcmp(addr,"::0")==0;
}

/*check for the presence of at least one stream with requested direction */
static bool_t has_dir(const SalMediaDescription *md, SalStreamDir stream_dir){
	int i;

	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for(i=0;i<SAL_MEDIA_DESCRIPTION_MAX_STREAMS;++i){
		const SalStreamDescription *ss=&md->streams[i];
		if (!sal_stream_description_active(ss)) continue;
		if (ss->dir==stream_dir) {
			return TRUE;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (ss->dir==SalStreamSendRecv && stream_dir==SalStreamSendOnly && (is_null_address(md->addr) || is_null_address(ss->rtp_addr))){
			return TRUE;
		}
	}
	return FALSE;
}

bool_t sal_media_description_has_dir(const SalMediaDescription *md, SalStreamDir stream_dir){
	if (stream_dir==SalStreamRecvOnly){
		return has_dir(md, SalStreamRecvOnly) && !(has_dir(md,SalStreamSendOnly) || has_dir(md,SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendOnly){
		return has_dir(md, SalStreamSendOnly) && !(has_dir(md,SalStreamRecvOnly) || has_dir(md,SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendRecv){
		return has_dir(md,SalStreamSendRecv);
	}else{
		/*SalStreamInactive*/
		if (has_dir(md,SalStreamSendOnly) || has_dir(md,SalStreamSendRecv)  || has_dir(md,SalStreamRecvOnly))
			return FALSE;
		else return TRUE;
	}
	return FALSE;
}

bool_t sal_stream_description_active(const SalStreamDescription *sd) {
	return (sd->rtp_port > 0);
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool_t sal_stream_description_has_avpf(const SalStreamDescription *sd) {
	switch (sd->proto){
		case SalProtoRtpAvpf:
		case SalProtoRtpSavpf:
		case SalProtoUdpTlsRtpSavpf:
			return TRUE;
		case SalProtoRtpAvp:
		case SalProtoRtpSavp:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return FALSE;
	}
	return FALSE;
}

bool_t sal_stream_description_has_ipv6(const SalStreamDescription *sd){
	return strchr(sd->rtp_addr,':') != NULL;
}

bool_t sal_stream_description_has_implicit_avpf(const SalStreamDescription *sd){
	return sd->implicit_rtcp_fb;
}
/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool_t sal_stream_description_has_srtp(const SalStreamDescription *sd) {
	switch (sd->proto){
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
			return TRUE;
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return FALSE;
	}
	return FALSE;
}

bool_t sal_stream_description_has_dtls(const SalStreamDescription *sd) {
	switch (sd->proto){
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
			return TRUE;
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoOther:
			return FALSE;
	}
	return FALSE;
}

bool_t sal_stream_description_has_zrtp(const SalStreamDescription *sd) {
	if (sd->haveZrtpHash==1) return TRUE;
	return FALSE;
}

bool_t sal_media_description_has_avpf(const SalMediaDescription *md) {
	int i;
	if (md->nb_streams == 0) return FALSE;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (sal_stream_description_has_avpf(&md->streams[i]) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_implicit_avpf(const SalMediaDescription *md) {
    int i;
    if (md->nb_streams == 0) return FALSE;
    for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
        if (!sal_stream_description_active(&md->streams[i])) continue;
        if (sal_stream_description_has_implicit_avpf(&md->streams[i]) != TRUE) return FALSE;
    }
    return TRUE;
}

bool_t sal_media_description_has_srtp(const SalMediaDescription *md) {
	int i;
	if (md->nb_streams == 0) return FALSE;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (sal_stream_description_has_srtp(&md->streams[i]) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_dtls(const SalMediaDescription *md) {
	int i;
	if (md->nb_streams == 0) return FALSE;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (sal_stream_description_has_dtls(&md->streams[i]) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_zrtp(const SalMediaDescription *md) {
	int i;
	if (md->nb_streams == 0) return FALSE;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (sal_stream_description_has_zrtp(&md->streams[i]) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_ipv6(const SalMediaDescription *md){
	int i;
	if (md->nb_streams == 0) return FALSE;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (md->streams[i].rtp_addr[0] != '\0'){
			if (!sal_stream_description_has_ipv6(&md->streams[i])) return FALSE;
		}else{
			if (strchr(md->addr,':') == NULL) return FALSE;
		}
	}
	return TRUE;
}

/*
static bool_t fmtp_equals(const char *p1, const char *p2){
	if (p1 && p2 && strcmp(p1,p2)==0) return TRUE;
	if (p1==NULL && p2==NULL) return TRUE;
	return FALSE;
}
*/

static bool_t payload_type_equals(const PayloadType *p1, const PayloadType *p2){
	if (p1->type!=p2->type) return FALSE;
	if (strcmp(p1->mime_type,p2->mime_type)!=0) return FALSE;
	if (p1->clock_rate!=p2->clock_rate) return FALSE;
	if (p1->channels!=p2->channels) return FALSE;
	if (payload_type_get_number(p1) != payload_type_get_number(p2)) return FALSE;
	/*
	 Do not compare fmtp right now: they are modified internally when the call is started
	*/
	/*
	if (!fmtp_equals(p1->recv_fmtp,p2->recv_fmtp) ||
		!fmtp_equals(p1->send_fmtp,p2->send_fmtp))
		return FALSE;
	*/
	return TRUE;
}

static bool_t is_recv_only(PayloadType *p){
	return (p->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && ! (p->flags & PAYLOAD_TYPE_FLAG_CAN_SEND);
}

static bool_t payload_list_equals(const bctbx_list_t *l1, const bctbx_list_t *l2){
	const bctbx_list_t *e1,*e2;
	for(e1=l1,e2=l2;e1!=NULL && e2!=NULL; e1=e1->next,e2=e2->next){
		PayloadType *p1=(PayloadType*)e1->data;
		PayloadType *p2=(PayloadType*)e2->data;
		if (!payload_type_equals(p1,p2))
			return FALSE;
	}
	if (e1!=NULL){
		/*skip possible recv-only payloads*/
		for(;e1!=NULL && is_recv_only((PayloadType*)e1->data);e1=e1->next){
			ms_message("Skipping recv-only payload type...");
		}
	}
	if (e1!=NULL || e2!=NULL){
		/*means one list is longer than the other*/
		return FALSE;
	}
	return TRUE;
}

int sal_stream_description_equals(const SalStreamDescription *sd1, const SalStreamDescription *sd2) {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;
	int i;

	/* A different proto should result in SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED but the encryption change
	   needs a stream restart for now, so use SAL_MEDIA_DESCRIPTION_CODEC_CHANGED */
	if (sd1->proto != sd2->proto) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	for (i = 0; i < SAL_CRYPTO_ALGO_MAX; i++) {
		if ((sd1->crypto[i].tag != sd2->crypto[i].tag)
			|| (sd1->crypto[i].algo != sd2->crypto[i].algo)){
			result|=SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		}
		if ((strncmp(sd1->crypto[i].master_key, sd2->crypto[i].master_key, sizeof(sd1->crypto[i].master_key) - 1))) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
	}

	if (sd1->type != sd2->type) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (strcmp(sd1->rtp_addr, sd2->rtp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (sd1->rtp_addr[0]!='\0' && sd2->rtp_addr[0]!='\0' && ms_is_multicast(sd1->rtp_addr) != ms_is_multicast(sd2->rtp_addr))
			result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (sd1->rtp_port != sd2->rtp_port) {
		if ((sd1->rtp_port == 0) || (sd2->rtp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (strcmp(sd1->rtcp_addr, sd2->rtcp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (sd1->rtcp_port != sd2->rtcp_port) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (!payload_list_equals(sd1->payloads, sd2->payloads)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->bandwidth != sd2->bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->ptime != sd2->ptime) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->dir != sd2->dir) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (strcmp(sd1->ice_ufrag, sd2->ice_ufrag) != 0 && sd2->ice_ufrag[0] != '\0') result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (strcmp(sd1->ice_pwd, sd2->ice_pwd) != 0 && sd2->ice_pwd[0] != '\0') result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;


	/*DTLS*/
	if (sd1->dtls_role != sd2->dtls_role) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	if (strcmp(sd1->dtls_fingerprint, sd2->dtls_fingerprint) != 0) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	return result;
}

char * sal_media_description_print_differences(int result){
	char *out = NULL;
	if (result & SAL_MEDIA_DESCRIPTION_CODEC_CHANGED){
		out = ms_strcat_printf(out, "%s ", "CODEC_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED){
		out = ms_strcat_printf(out, "%s ", "NETWORK_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED){
		out = ms_strcat_printf(out, "%s ", "ICE_RESTART_DETECTED");
		result &= ~SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED){
		out = ms_strcat_printf(out, "%s ", "CRYPTO_KEYS_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED){
		out = ms_strcat_printf(out, "%s ", "NETWORK_XXXCAST_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED){
		out = ms_strcat_printf(out, "%s ", "STREAMS_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED){
		out = ms_strcat_printf(out, "%s ", "CRYPTO_POLICY_CHANGED");
		result &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
	}
	if (result & SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION){
		out = ms_strcat_printf(out, "%s ", "FORCE_STREAM_RECONSTRUCTION");
		result &= ~SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	}
	if (result){
		ms_fatal("There are unhandled result bitmasks in sal_media_description_print_differences(), fix it");
	}
	if (!out) out = ms_strdup("NONE");
	return out;
}

int sal_media_description_equals(const SalMediaDescription *md1, const SalMediaDescription *md2) {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;
	int i;

	if (strcmp(md1->addr, md2->addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (md1->addr[0]!='\0' && md2->addr[0]!='\0' && ms_is_multicast(md1->addr) != ms_is_multicast(md2->addr))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (md1->nb_streams != md2->nb_streams) result |= SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	if (md1->bandwidth != md2->bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (strcmp(md1->ice_ufrag, md2->ice_ufrag) != 0 && md2->ice_ufrag[0] != '\0') result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (strcmp(md1->ice_pwd, md2->ice_pwd) != 0 && md2->ice_pwd[0] != '\0') result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;

	for(i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; ++i){
		if (!sal_stream_description_active(&md1->streams[i]) && !sal_stream_description_active(&md2->streams[i])) continue;
		result |= sal_stream_description_equals(&md1->streams[i], &md2->streams[i]);
	}
	return result;
}

SalAuthInfo* sal_auth_info_new() {
	return ms_new0(SalAuthInfo,1);
}

SalAuthInfo* sal_auth_info_clone(const SalAuthInfo* auth_info) {
	SalAuthInfo* new_auth_info=sal_auth_info_new();
	new_auth_info->username=auth_info->username?ms_strdup(auth_info->username):NULL;
	new_auth_info->userid=auth_info->userid?ms_strdup(auth_info->userid):NULL;
	new_auth_info->realm=auth_info->realm?ms_strdup(auth_info->realm):NULL;
	new_auth_info->domain=auth_info->realm?ms_strdup(auth_info->domain):NULL;
	new_auth_info->password=auth_info->password?ms_strdup(auth_info->password):NULL;
	new_auth_info->algorithm=auth_info->algorithm?ms_strdup(auth_info->algorithm):NULL;
	return new_auth_info;
}

void sal_auth_info_delete(SalAuthInfo* auth_info) {
	if (auth_info->username) ms_free(auth_info->username);
	if (auth_info->userid) ms_free(auth_info->userid);
	if (auth_info->realm) ms_free(auth_info->realm);
	if (auth_info->domain) ms_free(auth_info->domain);
	if (auth_info->password) ms_free(auth_info->password);
	if (auth_info->ha1) ms_free(auth_info->ha1);
	if (auth_info->certificates) sal_certificates_chain_delete(auth_info->certificates);
	if (auth_info->key) sal_signing_key_delete(auth_info->key);
	if (auth_info->algorithm) ms_free(auth_info->algorithm);
	ms_free(auth_info);
}



const char* sal_stream_type_to_string(SalStreamType type) {
	switch (type) {
	case SalAudio: return "audio";
	case SalVideo: return "video";
	case SalText: return "text";
	default: return "other";
	}
}

const char *sal_stream_description_get_type_as_string(const SalStreamDescription *desc){
	if (desc->type==SalOther) return desc->typeother;
	else return sal_stream_type_to_string(desc->type);
}

const char* sal_media_proto_to_string(SalMediaProto type) {
	switch (type) {
	case SalProtoRtpAvp:return "RTP/AVP";
	case SalProtoRtpSavp:return "RTP/SAVP";
	case SalProtoUdpTlsRtpSavp:return "UDP/TLS/RTP/SAVP";
	case SalProtoRtpAvpf:return "RTP/AVPF";
	case SalProtoRtpSavpf:return "RTP/SAVPF";
	case SalProtoUdpTlsRtpSavpf:return "UDP/TLS/RTP/SAVPF";
	default: return "unknown";
	}
}

const char *sal_stream_description_get_proto_as_string(const SalStreamDescription *desc){
	if (desc->proto==SalProtoOther) return desc->proto_other;
	else return sal_media_proto_to_string(desc->proto);
}


const char* sal_stream_dir_to_string(SalStreamDir type) {
	switch (type) {
	case SalStreamSendRecv:return "sendrecv";
	case SalStreamSendOnly:return "sendonly";
	case SalStreamRecvOnly:return "recvonly";
	case SalStreamInactive:return "inative";
	default: return "unknown";
	}

}

const char* sal_reason_to_string(const SalReason reason) {
	switch (reason) {
	case SalReasonDeclined : return "SalReasonDeclined";
	case SalReasonBusy: return "SalReasonBusy";
	case SalReasonRedirect: return "SalReasonRedirect";
	case SalReasonTemporarilyUnavailable: return "SalReasonTemporarilyUnavailable";
	case SalReasonNotFound: return "SalReasonNotFound";
	case SalReasonDoNotDisturb: return "SalReasonDoNotDisturb";
	case SalReasonUnsupportedContent: return "SalReasonUnsupportedContent";
	case SalReasonForbidden: return "SalReasonForbidden";
	case SalReasonUnknown: return "SalReasonUnknown";
	case SalReasonServiceUnavailable: return "SalReasonServiceUnavailable";
	case SalReasonNotAcceptable: return "SalReasonNotAcceptable";
	default: return "Unkown reason";
	}
}

const char* sal_presence_status_to_string(const SalPresenceStatus status) {
	switch (status) {
	case SalPresenceOffline: return "SalPresenceOffline";
	case SalPresenceOnline: return "SalPresenceOnline";
	case SalPresenceBusy: return "SalPresenceBusy";
	case SalPresenceBerightback: return "SalPresenceBerightback";
	case SalPresenceAway: return "SalPresenceAway";
	case SalPresenceOnthephone: return "SalPresenceOnthephone";
	case SalPresenceOuttolunch: return "SalPresenceOuttolunch";
	case SalPresenceDonotdisturb: return "SalPresenceDonotdisturb";
	case SalPresenceMoved: return "SalPresenceMoved";
	case SalPresenceAltService: return "SalPresenceAltService";
	default : return "unknown";
	}

}
const char* sal_privacy_to_string(SalPrivacy privacy) {
	switch(privacy) {
	case SalPrivacyUser: return "user";
	case SalPrivacyHeader: return "header";
	case SalPrivacySession: return "session";
	case SalPrivacyId: return "id";
	case SalPrivacyNone: return "none";
	case SalPrivacyCritical: return "critical";
	default: return NULL;
	}
}

static int line_get_value(const char *input, const char *key, char *value, size_t value_size, size_t *read) {
	const char *end = strchr(input, '\n');
	char line[256] = {0};
	char key_candidate[256]; // key_candidate array must have the same size of line array to avoid potential invalid writes
	char *equal;
	size_t len;

	if (!end) len = strlen(input);
	else len = (size_t)(end + 1 - input);
	*read = len;
	strncpy(line, input, MIN(len, sizeof(line)));

	equal = strchr(line, '=');
	if (!equal) return FALSE;
	*equal = '\0';

	if (sscanf(line, "%s", key_candidate) != 1) return FALSE;
	if (strcasecmp(key, key_candidate) != 0) return FALSE;

	equal++;
	if (strlen(equal) >= value_size) equal[value_size - 1] = '\0';
	if (sscanf(equal, "%s", value) != 1) return FALSE;
	return TRUE;
}

int sal_lines_get_value(const char *data, const char *key, char *value, size_t value_size) {
	size_t read = 0;

	do {
		if (line_get_value(data, key, value, value_size, &read))
			return TRUE;
		data += read;
	} while (read != 0);
	return FALSE;
}
