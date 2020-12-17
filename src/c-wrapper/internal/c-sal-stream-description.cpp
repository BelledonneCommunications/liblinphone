/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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
#include "c-wrapper/internal/c-sal-stream-description.h"
#include "sal/sal_stream_description.h"
#include "tester_utils.h"

static bool_t is_recv_only(PayloadType *p){
	return (p->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && ! (p->flags & PAYLOAD_TYPE_FLAG_CAN_SEND);
}

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

const char *sal_stream_description_get_type_as_string(const SalStreamDescription *desc){
	if (desc->type==SalOther) return desc->typeother.c_str();
	else return sal_stream_type_to_string(desc->type);
}

const char *sal_stream_description_get_proto_as_string(const SalStreamDescription *desc){
	if (desc->proto==SalProtoOther) return desc->proto_other.c_str();
	else return sal_media_proto_to_string(desc->proto);
}

int sal_stream_description_equals(const SalStreamDescription *sd1, const SalStreamDescription *sd2) {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	/* A different proto should result in SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED but the encryption change
	   needs a stream restart for now, so use SAL_MEDIA_DESCRIPTION_CODEC_CHANGED */
	if (sd1->proto != sd2->proto) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	int sdMinSize = (int)std::min(sd1->crypto.size(), sd2->crypto.size());
	for (int i = 0; i < sdMinSize; i++) {
		if ((sd1->crypto[i].tag != sd2->crypto[i].tag)
			|| (sd1->crypto[i].algo != sd2->crypto[i].algo)){
			result|=SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		}
		if ((strncmp(sd1->crypto[i].master_key, sd2->crypto[i].master_key, sizeof(sd1->crypto[i].master_key) - 1))) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
	}

	if (sd1->type != sd2->type) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->rtp_addr.compare(sd2->rtp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if ((sd1->rtp_addr.empty()==false) && (sd2->rtp_addr.empty()==false) && ms_is_multicast(L_STRING_TO_C(sd1->rtp_addr)) != ms_is_multicast(L_STRING_TO_C(sd2->rtp_addr)))
			result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (sd1->multicast_role != sd2->multicast_role) result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (sd1->rtp_port != sd2->rtp_port) {
		if ((sd1->rtp_port == 0) || (sd2->rtp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (sd1->rtcp_addr.compare(sd2->rtcp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (sd1->rtcp_port != sd2->rtcp_port) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (!payload_list_equals(sd1->payloads, sd2->payloads)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->bandwidth != sd2->bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->ptime != sd2->ptime) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (sd1->dir != sd2->dir) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (sd1->ice_ufrag.compare(sd2->ice_ufrag) != 0 && !sd2->ice_ufrag.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (sd1->ice_pwd.compare(sd2->ice_pwd) != 0 && !sd2->ice_pwd.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;


	/*DTLS*/
	if (sd1->dtls_role != sd2->dtls_role) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	if (sd1->dtls_fingerprint.compare(sd2->dtls_fingerprint) != 0) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	return result;
}

bool_t sal_stream_description_enabled(const SalStreamDescription *sd) {
	/* When the bundle-only attribute is present, a 0 rtp port doesn't mean that the stream is disabled.*/
	return sd->rtp_port > 0 || sd->bundle_only;
}

void sal_stream_description_disable(SalStreamDescription *sd){
	sd->rtp_port = 0;
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	sd->mid.clear();
	sd->bundle_only = FALSE;
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
	return sd->rtp_addr.find(':') != std::string::npos;
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

bool_t sal_stream_description_has_limeIk(const SalStreamDescription *sd) {
	if (sd->haveLimeIk==1) return TRUE;
	return FALSE;
}

const char * sal_stream_description_get_rtcp_address(SalStreamDescription *sd){
	return L_STRING_TO_C(sd->rtcp_addr);
}

const char * sal_stream_description_get_rtp_address(SalStreamDescription *sd){
	return L_STRING_TO_C(sd->rtp_addr);
}

MSList * sal_stream_description_get_payloads(SalStreamDescription *sd){
	return sd->payloads;
}
