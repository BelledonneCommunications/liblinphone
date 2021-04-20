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

#include "linphone/utils/utils.h"
#include "c-wrapper/internal/c-tools.h"
#include "sal/sal_stream_description.h"
#include "utils/payload-type-handler.h"
#include "bellesip_sal/sal_impl.h"

LINPHONE_BEGIN_NAMESPACE

#define keywordcmp(key,b) strncmp(key,b,sizeof(key))

SalStreamDescription::SalStreamDescription(){
	custom_sdp_attributes = NULL;

	pad[0] = false;
	pad[1] = false;

	payloads.clear();
	already_assigned_payloads.clear();
	crypto.clear();
	ice_candidates.clear();
	ice_remote_candidates.clear();

	rtcp_fb.generic_nack_enabled = TRUE;
	rtcp_fb.tmmbr_enabled = TRUE;
}

SalStreamDescription::~SalStreamDescription(){
	PayloadTypeHandler::clearPayloadList(payloads);
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
}

SalStreamDescription::SalStreamDescription(const SalStreamDescription & other){
	name = other.name;
	proto = other.proto;
	type = other.type;
	main = other.main;
	typeother = other.typeother;
	proto_other = other.proto_other;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	for (const auto & pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	for (const auto & pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	crypto_local_tag = other.crypto_local_tag;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad[0] = other.pad[0];
	pad[1] = other.pad[1];
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);
	ice_candidates = other.ice_candidates;
	ice_remote_candidates = other.ice_remote_candidates;
	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	mid = other.mid;
	mid_rtp_ext_header_id = other.mid_rtp_ext_header_id;
	ice_mismatch = other.ice_mismatch;
	set_nortpproxy = other.set_nortpproxy;
	rtcp_mux = other.rtcp_mux;
	haveZrtpHash = other.haveZrtpHash;
	memcpy(zrtphash, other.zrtphash, sizeof(zrtphash));
	dtls_fingerprint = other.dtls_fingerprint;
	dtls_role = other.dtls_role;
	ttl = other.ttl;
	multicast_role = other.multicast_role;
}

SalStreamDescription::SalStreamDescription(const SalMediaDescription * salMediaDesc, const belle_sdp_media_description_t *media_desc) : SalStreamDescription() {
	belle_sdp_connection_t* cnx;
	belle_sdp_media_t* media;
	belle_sdp_attribute_t* attribute;
	belle_sip_list_t *custom_attribute_it;
	const char* value;
	const char *mtype,*protoStr;
	bool_t has_avpf_attributes;

	media=belle_sdp_media_description_get_media ( media_desc );

	custom_sdp_attributes = NULL;

	protoStr = belle_sdp_media_get_protocol ( media );
	proto=SalProtoOther;
	if ( protoStr ) {
		if (strcasecmp(protoStr, "RTP/AVP") == 0) {
			proto = SalProtoRtpAvp;
		} else if (strcasecmp(protoStr, "RTP/SAVP") == 0) {
			proto = SalProtoRtpSavp;
		} else if (strcasecmp(protoStr, "RTP/AVPF") == 0) {
			proto = SalProtoRtpAvpf;
		} else if (strcasecmp(protoStr, "RTP/SAVPF") == 0) {
			proto = SalProtoRtpSavpf;
		} else if (strcasecmp(protoStr, "UDP/TLS/RTP/SAVP") == 0) {
			proto = SalProtoUdpTlsRtpSavp;
		} else if (strcasecmp(protoStr, "UDP/TLS/RTP/SAVPF") == 0) {
			proto = SalProtoUdpTlsRtpSavpf;
		} else {
			proto_other = proto;
		}
	}
	if ( ( cnx=belle_sdp_media_description_get_connection ( media_desc ) ) && belle_sdp_connection_get_address ( cnx ) ) {
		rtp_addr = L_C_TO_STRING(belle_sdp_connection_get_address ( cnx ));
		ttl=belle_sdp_connection_get_ttl(cnx);
	}

	rtp_port=belle_sdp_media_get_media_port ( media );

	mtype = belle_sdp_media_get_media_type ( media );
	if ( strcasecmp ( "audio", mtype ) == 0 ) {
		type=SalAudio;
	} else if ( strcasecmp ( "video", mtype ) == 0 ) {
		type=SalVideo;
	} else if ( strcasecmp ( "text", mtype ) == 0 ) {
		type=SalText;
	} else {
		type=SalOther;
		typeother = mtype;
	}

	if ( belle_sdp_media_description_get_bandwidth ( media_desc,"AS" ) >0 ) {
		bandwidth=belle_sdp_media_description_get_bandwidth ( media_desc,"AS" );
	}

	if ( belle_sdp_media_description_get_attribute ( media_desc,"sendrecv" ) ) {
		dir=SalStreamSendRecv;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"sendonly" ) ) {
		dir=SalStreamSendOnly;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"recvonly" ) ) {
		dir=SalStreamRecvOnly;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"inactive" ) ) {
		dir=SalStreamInactive;
	} else {
		dir=salMediaDesc->dir; /*takes default value if not present*/
	}

	rtcp_mux = belle_sdp_media_description_get_attribute(media_desc, "rtcp-mux") != NULL;
	bundle_only = belle_sdp_media_description_get_attribute(media_desc, "bundle-only") != NULL;

	attribute = belle_sdp_media_description_get_attribute(media_desc, "mid");
	if (attribute){
		value = belle_sdp_attribute_get_value(attribute);
		if (value) {
			mid = L_C_TO_STRING(value);
		}
	}

	main = false;
	attribute = belle_sdp_media_description_get_attribute(media_desc, "main");
	if (attribute){
		value = belle_sdp_attribute_get_value(attribute);
		if (value) {
			// If value is 0, then main is false, true otherwise
			main = (strcmp(value, "0") != 0);
		}
	}

	payloads.clear();
	already_assigned_payloads.clear();
	/* Get media payload types */
	sdpParsePayloadTypes(media_desc);

	/* Get media specific RTCP attribute */
	rtcp_addr = rtp_addr;
	rtcp_port = rtp_port + 1;

	attribute=belle_sdp_media_description_get_attribute(media_desc,"rtcp");
	if (attribute && (value=belle_sdp_attribute_get_value(attribute))!=NULL){
		char *tmp = (char *)ms_malloc0(strlen(value));
		int nb = sscanf(value, "%d IN IP4 %s", &rtcp_port, tmp);
		if (nb == 1) {
			/* SDP rtcp attribute only contains the port */
		} else if (nb == 2) {
			rtcp_addr = L_C_TO_STRING(tmp);
		} else {
			ms_warning("sdp has a strange a=rtcp line (%s) nb=%i", value, nb);
		}
		ms_free(tmp);
	}

	/* Read DTLS specific attributes : check is some are found in the stream description otherwise copy the session description one(which are at least set to Invalid) */
	if (((proto == SalProtoUdpTlsRtpSavpf) || (proto == SalProtoUdpTlsRtpSavp))) {
		attribute=belle_sdp_media_description_get_attribute(media_desc,"setup");
		if (attribute && (value=belle_sdp_attribute_get_value(attribute))!=NULL){
			if (strncmp(value, "actpass", 7) == 0) {
				dtls_role = SalDtlsRoleUnset;
			} else if (strncmp(value, "active", 6) == 0) {
				dtls_role = SalDtlsRoleIsClient;
			} else if (strncmp(value, "passive", 7) == 0) {
				dtls_role = SalDtlsRoleIsServer;
			}
		}
		if (dtls_role != SalDtlsRoleInvalid && (attribute=belle_sdp_media_description_get_attribute(media_desc,"fingerprint"))) {
			dtls_fingerprint = belle_sdp_attribute_get_value(attribute);
		}
	}

	/* Read crypto lines if any */
	if (hasSrtp()) {
		sdpParseMediaCryptoParameters(media_desc);
	}

	/* Read zrtp-hash attribute */
	if ((attribute=belle_sdp_media_description_get_attribute(media_desc,"zrtp-hash"))!=NULL) {
		if ((value=belle_sdp_attribute_get_value(attribute))!=NULL) {
		#if __GNUC__ > 7
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wstringop-truncation"
		#endif
			strncpy((char *)(zrtphash), belle_sdp_attribute_get_value(attribute),sizeof(zrtphash));
			haveZrtpHash = 1;
		#if __GNUC__ > 7
			#pragma GCC diagnostic pop
		#endif
		}
	}

	/* Get ICE candidate attributes if any */
	sdpParseMediaIceParameters(media_desc);

	has_avpf_attributes = sdpParseRtcpFbParameters(media_desc);

	/* Get RTCP-FB attributes if any */
	if (hasAvpf()) {
		enableAvpfForStream();
	}else if (has_avpf_attributes ){
		enableAvpfForStream();
		implicit_rtcp_fb = TRUE;
	}

	/* Get RTCP-XR attributes if any */
	rtcp_xr = salMediaDesc->rtcp_xr;	// Use session parameters if no stream parameters are defined
	sdp_parse_media_rtcp_xr_parameters(media_desc, &rtcp_xr);

	/* Get the custom attributes, and parse some 'extmap'*/
	for (custom_attribute_it = belle_sdp_media_description_get_attributes(media_desc); custom_attribute_it != NULL; custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		const char *attr_name = belle_sdp_attribute_get_name(attr);
		const char *attr_value = belle_sdp_attribute_get_value(attr);
		custom_sdp_attributes = sal_custom_sdp_attribute_append(custom_sdp_attributes, attr_name, attr_value);

		if (strcasecmp(attr_name, "extmap") == 0){
			char *extmap_urn = (char*)bctbx_malloc0(strlen(attr_value) + 1);
			int rtp_ext_header_id = 0;
			if (sscanf(attr_value, "%i %s", &rtp_ext_header_id, extmap_urn) > 0
				&& strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:sdes:mid") == 0){
				mid_rtp_ext_header_id = rtp_ext_header_id;
			}
			bctbx_free(extmap_urn);
		}
	}

}

SalStreamDescription &SalStreamDescription::operator=(const SalStreamDescription & other){
	name = other.name;
	proto = other.proto;
	type = other.type;
	main = other.main;
	typeother = other.typeother;
	proto_other = other.proto_other;
	rtp_addr = other.rtp_addr;
	rtcp_addr = other.rtcp_addr;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	rtp_port = other.rtp_port;
	rtcp_port = other.rtcp_port;
	PayloadTypeHandler::clearPayloadList(payloads);
	for (const auto & pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	PayloadTypeHandler::clearPayloadList(already_assigned_payloads);
	for (const auto & pt : other.already_assigned_payloads) {
		already_assigned_payloads.push_back(payload_type_clone(pt));
	}
	bandwidth = other.bandwidth;
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	crypto_local_tag = other.crypto_local_tag;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad[0] = other.pad[0];
	pad[1] = other.pad[1];
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	sal_custom_sdp_attribute_free(custom_sdp_attributes);
	custom_sdp_attributes = sal_custom_sdp_attribute_clone(other.custom_sdp_attributes);
	ice_candidates = other.ice_candidates;
	ice_remote_candidates = other.ice_remote_candidates;
	ice_ufrag = other.ice_ufrag;
	ice_pwd = other.ice_pwd;
	mid = other.mid;
	mid_rtp_ext_header_id = other.mid_rtp_ext_header_id;
	ice_mismatch = other.ice_mismatch;
	set_nortpproxy = other.set_nortpproxy;
	rtcp_mux = other.rtcp_mux;
	haveZrtpHash = other.haveZrtpHash;
	memcpy(zrtphash, other.zrtphash, sizeof(zrtphash));
	dtls_fingerprint = other.dtls_fingerprint;
	dtls_role = other.dtls_role;
	ttl = other.ttl;
	multicast_role = other.multicast_role;

	return *this;
}

bool SalStreamDescription::isRecvOnly(const PayloadType *p) const {
	return (p->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && ! (p->flags & PAYLOAD_TYPE_FLAG_CAN_SEND);
}

bool SalStreamDescription::isSamePayloadType(const PayloadType *p1, const PayloadType *p2) const {
	if (p1->type!=p2->type) return false;
	if (strcmp(p1->mime_type,p2->mime_type)!=0) return false;
	if (p1->clock_rate!=p2->clock_rate) return false;
	if (p1->channels!=p2->channels) return false;
	if (payload_type_get_number(p1) != payload_type_get_number(p2)) return false;
	/*
	 Do not compare fmtp right now: they are modified internally when the call is started
	*/
	/*
	if (!fmtp_equals(p1->recv_fmtp,p2->recv_fmtp) ||
		!fmtp_equals(p1->send_fmtp,p2->send_fmtp))
		return false;
	*/
	return true;
}

bool SalStreamDescription::isSamePayloadList(const std::list<PayloadType*> & l1, const std::list<PayloadType*> & l2) const {
	auto p1 = l1.cbegin();
	auto p2 = l2.cbegin();
	for(; (p1 != l1.cend() && p2 != l2.cend()); ++p1, ++p2){
		if (!isSamePayloadType(*p1,*p2))
			return false;
	}
	if (p1!=l1.cend()){
		/*skip possible recv-only payloads*/
		for(;p1!=l1.cend() && isRecvOnly(*p1);++p1){
			ms_message("Skipping recv-only payload type...");
		}
	}
	if (p1!=l1.cend() || p2!=l2.cend()){
		/*means one list is longer than the other*/
		return false;
	}
	return true;
}

bool SalStreamDescription::operator==(const SalStreamDescription & other) const {
	return equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED;
}

bool SalStreamDescription::operator!=(const SalStreamDescription & other) const {
	return !(*this == other);
}

int SalStreamDescription::equal(const SalStreamDescription & other) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	/* A different proto should result in SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED but the encryption change
	   needs a stream restart for now, so use SAL_MEDIA_DESCRIPTION_CODEC_CHANGED */
	if (proto != other.proto) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	for(auto crypto1 = crypto.cbegin(), crypto2 = other.crypto.cbegin(); (crypto1 != crypto.cend() && crypto2 != other.crypto.cend()); ++crypto1, ++crypto2){
		if ((crypto1->tag != crypto2->tag)
			|| (crypto1->algo != crypto2->algo)){
			result|=SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		}
		if (crypto1->master_key.compare(crypto2->master_key)) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
	}

	if (crypto.size() != other.crypto.size()) {
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}

	if (type != other.type) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (rtp_addr.compare(other.rtp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if ((rtp_addr.empty()==false) && (other.rtp_addr.empty()==false) && ms_is_multicast(L_STRING_TO_C(rtp_addr)) != ms_is_multicast(L_STRING_TO_C(other.rtp_addr)))
			result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (multicast_role != other.multicast_role) result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (rtp_port != other.rtp_port) {
		if ((rtp_port == 0) || (other.rtp_port == 0)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
		else result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	}
	if (rtcp_addr.compare(other.rtcp_addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (rtcp_port != other.rtcp_port) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (!isSamePayloadList(payloads, other.payloads)) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (bandwidth != other.bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (ptime != other.ptime) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	if (dir != other.dir) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (ice_ufrag.compare(other.ice_ufrag) != 0 && !other.ice_ufrag.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (ice_pwd.compare(other.ice_pwd) != 0 && !other.ice_pwd.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;


	/*DTLS*/
	if (dtls_role != other.dtls_role) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	if (dtls_fingerprint.compare(other.dtls_fingerprint) != 0) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	return result;
}

bool SalStreamDescription::enabled() const {
	/* When the bundle-only attribute is present, a 0 rtp port doesn't mean that the stream is disabled.*/
	return rtp_port > 0 || bundle_only;
}

void SalStreamDescription::disable(){
	rtp_port = 0;
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	mid.clear();
	bundle_only = false;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamDescription::hasAvpf() const {
	switch (proto){
		case SalProtoRtpAvpf:
		case SalProtoRtpSavpf:
		case SalProtoUdpTlsRtpSavpf:
			return true;
		case SalProtoRtpAvp:
		case SalProtoRtpSavp:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamDescription::hasIpv6() const {
	return rtp_addr.find(':') != std::string::npos;
}

bool SalStreamDescription::hasImplicitAvpf() const {
	return implicit_rtcp_fb;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamDescription::hasSrtp() const {
	switch (proto){
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
			return true;
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamDescription::hasDtls() const {
	switch (proto){
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
			return true;
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamDescription::hasZrtp() const {
	if (haveZrtpHash==1) return true;
	return false;
}

	if (haveLimeIk==1) return true;
	return false;
}

bool SalStreamDescription::isMain() const {
	return main;
}

const std::string & SalStreamDescription::getRtcpAddress() const {
	return rtcp_addr;
}

const int & SalStreamDescription::getRtcpPort() const {
	return rtcp_port;
}

const std::string & SalStreamDescription::getRtpAddress() const {
	return rtp_addr;
}

const int & SalStreamDescription::getRtpPort() const {
	return rtp_port;
}

const SalStreamType & SalStreamDescription::getType() const {
	return type;
}

const std::string SalStreamDescription::getTypeAsString() const {
	if (type==SalOther) return typeother;
	else return LinphonePrivate::Utils::toString(type);
}

const SalMediaProto & SalStreamDescription::getProto() const {
	return proto;
}

const std::string SalStreamDescription::getProtoAsString() const {
	if (proto==SalProtoOther) return proto_other;
	else return LinphonePrivate::Utils::toString(proto);
}

SalStreamDir SalStreamDescription::getDirection() const {
	return dir;
}

const std::list<PayloadType*> & SalStreamDescription::getPayloads() const {
	return payloads;
}

belle_sdp_media_description_t * SalStreamDescription::toSdpMediaDescription(const SalMediaDescription * salMediaDesc, belle_sdp_session_description_t *session_desc) const {
	belle_sdp_mime_parameter_t* mime_param;
	belle_sdp_media_description_t* media_desc;
	char buffer[1024];
	const char* dirStr=NULL;
	bool_t different_rtp_and_rtcp_addr;
	bool_t stream_enabled = enabled();

	media_desc = belle_sdp_media_description_create ( L_STRING_TO_C(getTypeAsString())
				 ,rtp_port
				 ,1
				 ,L_STRING_TO_C(getProtoAsString())
				 ,NULL );
	if (!payloads.empty()) {
		for (const auto & pt : payloads) {
			mime_param= belle_sdp_mime_parameter_create ( pt->mime_type
					, payload_type_get_number ( pt )
					, pt->clock_rate
					, pt->channels>0 ? pt->channels : -1 );
			belle_sdp_mime_parameter_set_parameters ( mime_param,pt->recv_fmtp );
			if ( ptime>0 ) {
				belle_sdp_mime_parameter_set_ptime ( mime_param,ptime );
			}
			belle_sdp_media_description_append_values_from_mime_parameter ( media_desc,mime_param );
			belle_sip_object_unref ( mime_param );
		}
	} else {
		/* to comply with SDP we cannot have an empty payload type number list */
		/* as it happens only when mline is declined with a zero port, it does not matter to put whatever codec*/
		belle_sip_list_t* format = belle_sip_list_append(NULL,0);
		belle_sdp_media_set_media_formats(belle_sdp_media_description_get_media(media_desc),format);
	}

	/*only add a c= line within the stream description if address are differents*/
	if ((rtp_addr.empty()==false) && (salMediaDesc && (rtp_addr.compare(salMediaDesc->addr)!=0))){
		bool_t inet6;
		belle_sdp_connection_t *connection;
		if (rtp_addr.find(':')!=std::string::npos){
			inet6=TRUE;
		}else inet6=FALSE;
		connection = belle_sdp_connection_create("IN", inet6 ? "IP6" : "IP4", L_STRING_TO_C(rtp_addr));
		if (ms_is_multicast(L_STRING_TO_C(rtp_addr))) {
			/*remove session cline in case of multicast*/
			belle_sdp_session_description_set_connection(session_desc,NULL);
			if (inet6 == FALSE)
				belle_sdp_connection_set_ttl(connection,ttl);
		}
		belle_sdp_media_description_set_connection(media_desc,connection);
	}

	if ( bandwidth>0 )
		belle_sdp_media_description_set_bandwidth ( media_desc,"AS",bandwidth );

	if (hasSrtp()) {
		/* add crypto lines */
		for ( const auto & crypto : crypto ) {
			MSCryptoSuiteNameParams desc;
			if (ms_crypto_suite_to_name_params(crypto.algo,&desc)==0){
				if (desc.params)
					snprintf ( buffer, sizeof ( buffer )-1, "%d %s inline:%s %s", crypto.tag, desc.name, crypto.master_key.c_str(),desc.params);
				else
					snprintf ( buffer, sizeof ( buffer )-1, "%d %s inline:%s", crypto.tag, desc.name, crypto.master_key.c_str() );

				belle_sdp_media_description_add_attribute( media_desc,belle_sdp_attribute_create ("crypto", buffer));
			}else break;
		}
	}

	/* insert DTLS session attribute if needed */
	if ((proto == SalProtoUdpTlsRtpSavpf) || (proto == SalProtoUdpTlsRtpSavp)) {
		char* ssrc_attribute = ms_strdup_printf("%u cname:%s",rtp_ssrc,L_STRING_TO_C(rtcp_cname));
		if ((dtls_role != SalDtlsRoleInvalid) && (!dtls_fingerprint.empty())) {
			switch(dtls_role) {
				case SalDtlsRoleIsClient:
					belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("setup","active"));
					break;
				case SalDtlsRoleIsServer:
					belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("setup","passive"));
					break;
				case SalDtlsRoleUnset:
				default:
					belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("setup","actpass"));
					break;
			}
			belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("fingerprint",L_STRING_TO_C(dtls_fingerprint)));
		}
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("ssrc",ssrc_attribute));
		ms_free(ssrc_attribute);
	}

	belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("main",(main ? "1" : "0")));

	/* insert zrtp-hash attribute if needed */
	if (haveZrtpHash == 1) {
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("zrtp-hash", (const char *)(zrtphash)));
	}

	switch ( dir ) {
		case SalStreamSendRecv:
			/*dirStr="sendrecv";*/
			dirStr=NULL;
			break;
		case SalStreamRecvOnly:
			dirStr="recvonly";
			break;
		case SalStreamSendOnly:
			dirStr="sendonly";
			break;
		case SalStreamInactive:
			dirStr="inactive";
			break;
	}
	if ( dirStr ) belle_sdp_media_description_add_attribute ( media_desc,belle_sdp_attribute_create ( dirStr,NULL ) );

	if (rtcp_mux){
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create ("rtcp-mux",NULL ) );
	}
	addMidAttributesToSdp(media_desc);

	if (rtp_port != 0) {
		different_rtp_and_rtcp_addr = (rtcp_addr.empty() == false) && (rtp_addr.compare(rtcp_addr) != 0);
		if ((rtcp_port != (rtp_port + 1)) || (different_rtp_and_rtcp_addr == TRUE)) {
			if (different_rtp_and_rtcp_addr == TRUE) {
				snprintf(buffer, sizeof(buffer), "%u IN IP4 %s", rtcp_port, rtcp_addr.c_str());
			} else {
				snprintf(buffer, sizeof(buffer), "%u",rtcp_port);
			}
			belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("rtcp",buffer));
		}
	}
	if (set_nortpproxy == TRUE) {
		belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("nortpproxy","yes"));
	}
	if (ice_mismatch == TRUE) {
		belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("ice-mismatch",NULL));
	} else {
		if (rtp_port != 0) {
			if (!ice_pwd.empty())
				belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("ice-pwd",L_STRING_TO_C(ice_pwd)));
			if (!ice_ufrag.empty())
				belle_sdp_media_description_add_attribute(media_desc,belle_sdp_attribute_create ("ice-ufrag",L_STRING_TO_C(ice_ufrag)));
			addIceCandidatesToSdp(media_desc);
			addIceRemoteCandidatesToSdp(media_desc);
		}
	}

	if (stream_enabled && (hasAvpf() || hasImplicitAvpf())) {
		addRtcpFbAttributesToSdp(media_desc);
	}

	if (stream_enabled && (rtcp_xr.enabled == TRUE)) {
		char sastr[1024] = {0};
		char mastr[1024] = {0};
		size_t saoff = 0;
		size_t maoff = 0;
		const belle_sdp_attribute_t *session_attribute = belle_sdp_session_description_get_attribute(session_desc, "rtcp-xr");
		belle_sdp_attribute_t *media_attribute;
		if (session_attribute != NULL) {
			belle_sip_object_marshal((belle_sip_object_t*)session_attribute, sastr, sizeof(sastr), &saoff);
		}
		media_attribute = create_rtcp_xr_attribute(&rtcp_xr);
		if (media_attribute != NULL) {
			belle_sip_object_marshal((belle_sip_object_t*)media_attribute, mastr, sizeof(mastr), &maoff);
		}
		if (strcmp(sastr, mastr) != 0) {
			belle_sdp_media_description_add_attribute(media_desc, media_attribute);
		} else {
			belle_sip_object_unref((belle_sip_object_t*)media_attribute);
		}
	}

	if (custom_sdp_attributes) {
		belle_sdp_session_description_t *custom_desc = (belle_sdp_session_description_t *)custom_sdp_attributes;
		belle_sip_list_t *l = belle_sdp_session_description_get_attributes(custom_desc);
		belle_sip_list_t *elem;
		for (elem = l; elem != NULL; elem = elem->next) {
			belle_sdp_media_description_add_attribute(media_desc, (belle_sdp_attribute_t *)elem->data);
		}
	}

	/*
	 * rfc5576
	 * 4.1.  The "ssrc" Media Attribute
	 * <ssrc-id> is the synchronization source (SSRC) ID of the
	 * source being described, interpreted as a 32-bit unsigned integer in
	 * network byte order and represented in decimal.*/

	return media_desc;
}

bool_t SalStreamDescription::sdpParseRtcpFbParameters(const belle_sdp_media_description_t *media_desc) {
	belle_sip_list_t *it;
	belle_sdp_attribute_t *attribute;
	belle_sdp_rtcp_fb_attribute_t *fb_attribute;
	int8_t pt_num;
	bool_t retval = FALSE;

	/* Handle rtcp-fb attributes that concern all payload types. */
	for (it = belle_sdp_media_description_get_attributes(media_desc); it != NULL; it = it->next) {
		attribute = BELLE_SDP_ATTRIBUTE(it->data);
		if (keywordcmp("rtcp-fb", belle_sdp_attribute_get_name(attribute)) == 0) {
			fb_attribute = BELLE_SDP_RTCP_FB_ATTRIBUTE(attribute);
			if (belle_sdp_rtcp_fb_attribute_get_id(fb_attribute) == -1) {
				for (const auto & pt : payloads) {
					applyRtcpFbAttributeToPayload(fb_attribute, pt);
					retval = TRUE;
				}
			}
		}
	}

	/* Handle rtcp-fb attributes that are specefic to a payload type. */
	for (it = belle_sdp_media_description_get_attributes(media_desc); it != NULL; it = it->next) {
		attribute = BELLE_SDP_ATTRIBUTE(it->data);
		if (keywordcmp("rtcp-fb", belle_sdp_attribute_get_name(attribute)) == 0) {
			fb_attribute = BELLE_SDP_RTCP_FB_ATTRIBUTE(attribute);
			pt_num = belle_sdp_rtcp_fb_attribute_get_id(fb_attribute);
			for (const auto & pt : payloads) {
				retval = TRUE;
				if (payload_type_get_number(pt) == (int)pt_num) {
					applyRtcpFbAttributeToPayload(fb_attribute, pt);
				}
			}
		}
	}
	return retval;
}

void SalStreamDescription::sdpParsePayloadTypes(const belle_sdp_media_description_t *media_desc) {
	PayloadType *pt;
	PayloadTypeAvpfParams avpf_params;
	belle_sip_list_t* mime_param_it=NULL;
	belle_sdp_mime_parameter_t* mime_param;
	belle_sip_list_t* mime_params=belle_sdp_media_description_build_mime_parameters ( media_desc );
	memset(&avpf_params, 0, sizeof(avpf_params));
	for ( mime_param_it=mime_params
						; mime_param_it!=NULL
			; mime_param_it=mime_param_it->next ) {
		mime_param=BELLE_SDP_MIME_PARAMETER ( mime_param_it->data );

		pt=payload_type_new();
		payload_type_set_number ( pt,belle_sdp_mime_parameter_get_media_format ( mime_param ) );
		pt->clock_rate=belle_sdp_mime_parameter_get_rate ( mime_param );
		pt->mime_type=ms_strdup ( belle_sdp_mime_parameter_get_type ( mime_param ) );
		pt->channels=belle_sdp_mime_parameter_get_channel_count ( mime_param );
		payload_type_set_send_fmtp ( pt,belle_sdp_mime_parameter_get_parameters ( mime_param ) );
		payload_type_set_avpf_params(pt, avpf_params);
		payloads.push_back(pt);
		ptime=belle_sdp_mime_parameter_get_ptime ( mime_param );
		maxptime=belle_sdp_mime_parameter_get_max_ptime ( mime_param );
		ms_message ( "Found payload %s/%i fmtp=%s",pt->mime_type,pt->clock_rate,
						pt->send_fmtp ? pt->send_fmtp : "" );
	}
	if ( mime_params ) belle_sip_list_free_with_data ( mime_params,belle_sip_object_unref );
}

void SalStreamDescription::sdpParseMediaCryptoParameters(const belle_sdp_media_description_t *media_desc) {
	belle_sip_list_t *attribute_it;
	belle_sdp_attribute_t *attribute;
	char tmp[257]={0}, tmp2[129]={0}, parameters[257]={0};
	int nb;

	crypto.clear();
	for ( attribute_it=belle_sdp_media_description_get_attributes ( media_desc )
						; attribute_it!=NULL;
			attribute_it=attribute_it->next ) {
		attribute=BELLE_SDP_ATTRIBUTE ( attribute_it->data );

		if ( keywordcmp ( "crypto",belle_sdp_attribute_get_name ( attribute ) ) ==0 && belle_sdp_attribute_get_value ( attribute ) !=NULL ) {
			SalSrtpCryptoAlgo cryptoEl;
			nb = sscanf ( belle_sdp_attribute_get_value ( attribute ), "%d %256s inline:%128s %256s",
							&cryptoEl.tag,
							tmp,
							tmp2, parameters );

			if ( nb >= 3 ) {
				MSCryptoSuite cs;
				MSCryptoSuiteNameParams np;

				np.name=tmp;
				np.params=parameters[0]!='\0' ? parameters : NULL;
				cs=ms_crypto_suite_build_from_name_params(&np);
				if (cs==MS_CRYPTO_SUITE_INVALID){
					ms_warning ( "Failed to parse crypto-algo: '%s'", tmp );
					cryptoEl.algo = MS_CRYPTO_SUITE_INVALID;
				}else{
					cryptoEl.master_key = tmp2;
					size_t sep = cryptoEl.master_key.find("|");
					// Erase all characters after | if it is found
					if (sep != std::string::npos) cryptoEl.master_key.erase(cryptoEl.master_key.begin() + static_cast<long>(sep), cryptoEl.master_key.end());
					cryptoEl.algo = cs;
					ms_message ( "Found valid crypto line (tag:%d algo:'%s' key:'%s'",
									cryptoEl.tag,
									tmp,
									cryptoEl.master_key.c_str() );
					crypto.push_back(cryptoEl);
				}

			}else{
				ms_warning ( "sdp has a strange a= line (%s) nb=%i",belle_sdp_attribute_get_value ( attribute ),nb );
			}
		}
	}
	ms_message("Found: %u valid crypto lines", static_cast<unsigned int>(crypto.size()) );
}

void SalStreamDescription::sdpParseMediaIceParameters(const belle_sdp_media_description_t *media_desc) {
	belle_sip_list_t *attribute_it;
	belle_sdp_attribute_t *attribute;
	const char *att_name;
	const char *value;

	for (attribute_it = belle_sdp_media_description_get_attributes(media_desc); attribute_it != NULL; attribute_it=attribute_it->next) {
		attribute=BELLE_SDP_ATTRIBUTE(attribute_it->data);
		att_name = belle_sdp_attribute_get_name(attribute);
		value = belle_sdp_attribute_get_value(attribute);

		char addr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
		if ((keywordcmp("candidate", att_name) == 0)
				&& (value != NULL)) {
			SalIceCandidate candidate;
			char raddr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
			char foundation[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
			char type[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
			char proto[4];
			int nb = sscanf(value, "%s %u %3s %u %s %d typ %s raddr %s rport %d",
				foundation, &candidate.componentID, proto, &candidate.priority, addr, &candidate.port,
				type, raddr, &candidate.rport);
			candidate.addr = addr;
			candidate.raddr = raddr;
			candidate.foundation = foundation;
			candidate.type = type;
			if (strcasecmp("udp",proto)==0 && ((nb == 7) || (nb == 9))) {
				ice_candidates.push_back(candidate);
			} else {
				ms_error("ice: Failed parsing a=candidate SDP attribute");
			}
		} else if ((keywordcmp("remote-candidates", att_name) == 0) && (value != NULL)) {
			SalIceRemoteCandidate candidate;
			unsigned int componentID;
			int offset;
			const char *ptr = value;
			const char *endptr = value + strlen(ptr);
			while (3 == sscanf(ptr, "%u %s %u%n", &componentID, addr, &candidate.port, &offset)) {
				candidate.addr = addr;
				if (componentID > 0) {
					SalIceRemoteCandidate remote_candidate;
					remote_candidate.addr = candidate.addr;
					remote_candidate.port = candidate.port;
					const unsigned int candidateIdx = componentID - 1;
					const unsigned int noCandidates = (unsigned int)ice_remote_candidates.size();
					if (candidateIdx >= noCandidates) {
						ice_remote_candidates.resize(componentID);
					}
					ice_remote_candidates[(std::vector<SalIceRemoteCandidate>::size_type)candidateIdx] = remote_candidate;
				}
				ptr += offset;
				if (ptr < endptr) {
					if (*ptr == ' ') ptr += 1;
				} else break;
			}
		} else if ((keywordcmp("ice-ufrag", att_name) == 0) && (value != NULL)) {
			ice_ufrag = L_C_TO_STRING(value);
		} else if ((keywordcmp("ice-pwd", att_name) == 0) && (value != NULL)) {
			ice_pwd = L_C_TO_STRING(value);
		} else if (keywordcmp("ice-mismatch", att_name) == 0) {
			ice_mismatch = TRUE;
		}
	}
}

void SalStreamDescription::enableAvpfForStream() {
	for (auto & pt : payloads) {
		payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
	}
}

void SalStreamDescription::applyRtcpFbAttributeToPayload(belle_sdp_rtcp_fb_attribute_t *fb_attribute, PayloadType *pt) {
	PayloadTypeAvpfParams avpf_params = payload_type_get_avpf_params(pt);
	switch (belle_sdp_rtcp_fb_attribute_get_type(fb_attribute)) {
		case BELLE_SDP_RTCP_FB_ACK:
			if (belle_sdp_rtcp_fb_attribute_get_param(fb_attribute) == BELLE_SDP_RTCP_FB_RPSI) {
				avpf_params.features |= PAYLOAD_TYPE_AVPF_RPSI;
			}
			break;
		case BELLE_SDP_RTCP_FB_NACK:
			switch (belle_sdp_rtcp_fb_attribute_get_param(fb_attribute)) {
				case BELLE_SDP_RTCP_FB_PLI:
					avpf_params.features |= PAYLOAD_TYPE_AVPF_PLI;
					break;
				case BELLE_SDP_RTCP_FB_SLI:
					avpf_params.features |= PAYLOAD_TYPE_AVPF_SLI;
					break;
				case BELLE_SDP_RTCP_FB_RPSI:
					/* Linphone uses positive feeback for RPSI. However first versions handling
					 * AVPF wrongly declared RPSI as negative feedback, so this is kept for compatibility
					 * with these versions but will probably be removed at some point in time. */
					avpf_params.features |= PAYLOAD_TYPE_AVPF_RPSI;
					avpf_params.rpsi_compatibility = TRUE;
					break;
				case BELLE_SDP_RTCP_FB_NONE:
					rtcp_fb.generic_nack_enabled = TRUE;
					break;
				default:
					break;
			}
			break;
		case BELLE_SDP_RTCP_FB_TRR_INT:
			avpf_params.trr_interval = belle_sdp_rtcp_fb_attribute_get_trr_int(fb_attribute);
			break;
		case BELLE_SDP_RTCP_FB_CCM:
			switch (belle_sdp_rtcp_fb_attribute_get_param(fb_attribute)) {
				case BELLE_SDP_RTCP_FB_FIR:
					avpf_params.features |= PAYLOAD_TYPE_AVPF_FIR;
					break;
				case BELLE_SDP_RTCP_FB_TMMBR:
					rtcp_fb.tmmbr_enabled = TRUE;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	payload_type_set_avpf_params(pt, avpf_params);
}

void SalStreamDescription::addMidAttributesToSdp(belle_sdp_media_description_t *media_desc) const {
	if (mid.empty() == false){
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("mid", L_STRING_TO_C(mid)));
	}
	if (mid_rtp_ext_header_id){
		char *value = bctbx_strdup_printf("%i urn:ietf:params:rtp-hdrext:sdes:mid", mid_rtp_ext_header_id);
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("extmap", value));
		bctbx_free(value);
	}
	if (bundle_only){
		belle_sdp_media_description_add_attribute(media_desc, belle_sdp_attribute_create("bundle-only", NULL));
	}
}

bool_t SalStreamDescription::isRtcpFbTrrIntTheSameForAllPayloads(uint16_t *trr_int) const {
	bool_t first = TRUE;
	for (const auto & pt : payloads) {
		if (payload_type_get_flags(pt) & PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED) {
			if (first == TRUE) {
				*trr_int = payload_type_get_avpf_params(pt).trr_interval;
				first = FALSE;
			} else if (payload_type_get_avpf_params(pt).trr_interval != *trr_int) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

void SalStreamDescription::addRtcpFbAttributesToSdp(belle_sdp_media_description_t *media_desc) const {
	PayloadTypeAvpfParams avpf_params;
	bool_t general_trr_int;
	uint16_t trr_int = 0;

	general_trr_int = isRtcpFbTrrIntTheSameForAllPayloads(&trr_int);
	if (general_trr_int == TRUE && trr_int != 0) {
		add_rtcp_fb_trr_int_attribute(media_desc, -1, trr_int);
	}
	if (rtcp_fb.generic_nack_enabled == TRUE) {
		add_rtcp_fb_nack_attribute(media_desc, -1, BELLE_SDP_RTCP_FB_NONE);
	}
	if (rtcp_fb.tmmbr_enabled == TRUE) {
		add_rtcp_fb_ccm_attribute(media_desc, -1, BELLE_SDP_RTCP_FB_TMMBR);
	}

	for (const auto & pt : payloads) {

		/* AVPF/SAVPF profile is used so enable AVPF for all payload types. */
		payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
		avpf_params = payload_type_get_avpf_params(pt);

		/* Add trr-int if not set generally. */
		if (general_trr_int != TRUE && trr_int != 0) {
			add_rtcp_fb_trr_int_attribute(media_desc, (int8_t)payload_type_get_number(pt), avpf_params.trr_interval);
		}

		/* Add rtcp-fb attributes according to the AVPF features of the payload types. */
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_PLI) {
			add_rtcp_fb_nack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_PLI);
		}
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_SLI) {
			add_rtcp_fb_nack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_SLI);
		}
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_RPSI) {
			if (avpf_params.rpsi_compatibility == TRUE) {
				add_rtcp_fb_nack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_RPSI);
			} else {
				add_rtcp_fb_ack_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_RPSI);
			}
		}
		if (avpf_params.features & PAYLOAD_TYPE_AVPF_FIR) {
			add_rtcp_fb_ccm_attribute(media_desc, (int8_t)payload_type_get_number(pt), BELLE_SDP_RTCP_FB_FIR);
		}
	}
}

void SalStreamDescription::addIceCandidatesToSdp(belle_sdp_media_description_t *md) const {
	char buffer[1024];
	int nb;

	for (const auto & candidate : ice_candidates) {
		if ((candidate.addr.empty()) || (candidate.port == 0)) break;
		nb = snprintf(buffer, sizeof(buffer), "%s %u UDP %u %s %d typ %s",
			candidate.foundation.c_str(), candidate.componentID, candidate.priority, candidate.addr.c_str(), candidate.port, candidate.type.c_str());
		if (nb < 0) {
			ms_error("Cannot add ICE candidate attribute!");
			return;
		}
		if (!candidate.raddr.empty()) {
			nb = snprintf(buffer + nb, sizeof(buffer) - static_cast<size_t>(nb), " raddr %s rport %d", candidate.raddr.c_str(), candidate.rport);
			if (nb < 0) {
				ms_error("Cannot add ICE candidate attribute!");
				return;
			}
		}
		belle_sdp_media_description_add_attribute(md,belle_sdp_attribute_create("candidate",buffer));
	}
}

void SalStreamDescription::addIceRemoteCandidatesToSdp(belle_sdp_media_description_t *md) const {
	char buffer[1024];
	char *ptr = buffer;
	int offset = 0;

	buffer[0] = '\0';
	for (size_t i = 0; i < ice_remote_candidates.size(); i++) {
		const auto & candidate = ice_remote_candidates[i];
		if ((!candidate.addr.empty()) && (candidate.port != 0)) {
			offset = snprintf(ptr, static_cast<size_t>(buffer + sizeof(buffer) - ptr), "%s%u %s %d", (i > 0) ? " " : "", static_cast<unsigned int>(i + 1), candidate.addr.c_str(), candidate.port);
			if (offset < 0) {
				ms_error("Cannot add ICE remote-candidates attribute!");
				return;
			}
			ptr += offset;
		}
	}
	if (buffer[0] != '\0') belle_sdp_media_description_add_attribute(md,belle_sdp_attribute_create("remote-candidates",buffer));
}

LINPHONE_END_NAMESPACE
