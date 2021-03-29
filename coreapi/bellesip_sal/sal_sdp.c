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
#include "sal_impl.h"
#include "c-wrapper/internal/c-tools.h"

inline OrtpRtcpXrStatSummaryFlag operator|=(OrtpRtcpXrStatSummaryFlag a, OrtpRtcpXrStatSummaryFlag b) {
	int ia = static_cast<int>(a);
	int ib = static_cast<int>(b);
	return static_cast<OrtpRtcpXrStatSummaryFlag>(ia |= ib);
}

void add_rtcp_fb_trr_int_attribute(belle_sdp_media_description_t *media_desc, int8_t id, uint16_t trr_int) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_TRR_INT);
	belle_sdp_rtcp_fb_attribute_set_trr_int(attribute, trr_int);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

void add_rtcp_fb_ack_attribute(belle_sdp_media_description_t *media_desc, int8_t id, belle_sdp_rtcp_fb_val_param_t param) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_ACK);
	belle_sdp_rtcp_fb_attribute_set_param(attribute, param);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

void add_rtcp_fb_nack_attribute(belle_sdp_media_description_t *media_desc, int8_t id, belle_sdp_rtcp_fb_val_param_t param) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_NACK);
	belle_sdp_rtcp_fb_attribute_set_param(attribute, param);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

void add_rtcp_fb_ccm_attribute(belle_sdp_media_description_t *media_desc, int8_t id, belle_sdp_rtcp_fb_val_param_t param) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_CCM);
	belle_sdp_rtcp_fb_attribute_set_param(attribute, param);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

belle_sdp_attribute_t * create_rtcp_xr_attribute(const OrtpRtcpXrConfiguration *config) {
	belle_sdp_rtcp_xr_attribute_t *attribute = belle_sdp_rtcp_xr_attribute_new();
	if (config->rcvr_rtt_mode != OrtpRtcpXrRcvrRttNone) {
		if (config->rcvr_rtt_mode == OrtpRtcpXrRcvrRttAll) belle_sdp_rtcp_xr_attribute_set_rcvr_rtt_mode(attribute, "all");
		else if (config->rcvr_rtt_mode == OrtpRtcpXrRcvrRttSender) belle_sdp_rtcp_xr_attribute_set_rcvr_rtt_mode(attribute, "sender");
		belle_sdp_rtcp_xr_attribute_set_rcvr_rtt_max_size(attribute, config->rcvr_rtt_max_size);
	}
	belle_sdp_rtcp_xr_attribute_set_stat_summary(attribute, (config->stat_summary_enabled == TRUE));
	if (config->stat_summary_enabled == TRUE) {
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryLoss) belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "loss");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryDup) belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "dup");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryJitt) belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "jitt");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryTTL) belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "TTL");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryHL) belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "HL");
	}
	belle_sdp_rtcp_xr_attribute_set_voip_metrics(attribute, (config->voip_metrics_enabled == TRUE));
	return BELLE_SDP_ATTRIBUTE(attribute);
}

static void sal_init_rtcp_xr_description(OrtpRtcpXrConfiguration *config) {
	config->enabled = FALSE;
	config->rcvr_rtt_mode = OrtpRtcpXrRcvrRttNone;
	config->rcvr_rtt_max_size = -1;
	config->stat_summary_flags = OrtpRtcpXrStatSummaryNone;
	config->voip_metrics_enabled = FALSE;
}

static void sdp_parse_rtcp_xr_parameters(const belle_sdp_attribute_t *attribute, OrtpRtcpXrConfiguration *config) {
	if (attribute != NULL) {
		const belle_sdp_rtcp_xr_attribute_t *xr_attr;
		const char *rcvr_rtt_mode;
		sal_init_rtcp_xr_description(config);
		xr_attr = BELLE_SDP_RTCP_XR_ATTRIBUTE(attribute);
		rcvr_rtt_mode = belle_sdp_rtcp_xr_attribute_get_rcvr_rtt_mode(xr_attr);
		if (rcvr_rtt_mode != NULL) {
			if (strcasecmp(rcvr_rtt_mode, "all") == 0) {
				config->rcvr_rtt_mode = OrtpRtcpXrRcvrRttAll;
			} else if (strcasecmp(rcvr_rtt_mode, "sender") == 0) {
				config->rcvr_rtt_mode = OrtpRtcpXrRcvrRttSender;
			}
			config->rcvr_rtt_max_size = belle_sdp_rtcp_xr_attribute_get_rcvr_rtt_max_size(xr_attr);
		}
		config->stat_summary_enabled = (belle_sdp_rtcp_xr_attribute_has_stat_summary(xr_attr) != 0);
		if (config->stat_summary_enabled) {
			const belle_sip_list_t *stat_summary_flag_it;
			for (stat_summary_flag_it = belle_sdp_rtcp_xr_attribute_get_stat_summary_flags(xr_attr); stat_summary_flag_it != NULL; stat_summary_flag_it = stat_summary_flag_it->next ) {
				const char *flag = (const char *)stat_summary_flag_it->data;
				if (flag != NULL) {
					if (strcasecmp(flag, "loss") == 0) config->stat_summary_flags |= OrtpRtcpXrStatSummaryLoss;
					else if (strcasecmp(flag, "dup") == 0) config->stat_summary_flags |= OrtpRtcpXrStatSummaryDup;
					else if (strcasecmp(flag, "jitt") == 0) config->stat_summary_flags |= OrtpRtcpXrStatSummaryJitt;
					else if (strcasecmp(flag, "TTL") == 0) config->stat_summary_flags |= OrtpRtcpXrStatSummaryTTL;
					else if (strcasecmp(flag, "HL") == 0) config->stat_summary_flags |= OrtpRtcpXrStatSummaryHL;
				}
			}
		}
		config->voip_metrics_enabled = (belle_sdp_rtcp_xr_attribute_has_voip_metrics(xr_attr) != 0);
		config->enabled = TRUE;
	}
}

void sdp_parse_session_rtcp_xr_parameters(const belle_sdp_session_description_t *session_desc, OrtpRtcpXrConfiguration *config) {
	const belle_sdp_attribute_t *attribute = belle_sdp_session_description_get_attribute(session_desc, "rtcp-xr");
	sdp_parse_rtcp_xr_parameters(attribute, config);
}

void sdp_parse_media_rtcp_xr_parameters(const belle_sdp_media_description_t *media_desc, OrtpRtcpXrConfiguration *config) {
	const belle_sdp_attribute_t *attribute = belle_sdp_media_description_get_attribute(media_desc, "rtcp-xr");
	sdp_parse_rtcp_xr_parameters(attribute, config);
}

static SalStreamDescription * sdp_to_stream_description(SalMediaDescription *md, belle_sdp_media_description_t *media_desc) {
	SalStreamDescription *stream;
	belle_sdp_connection_t* cnx;
	belle_sdp_media_t* media;
	belle_sdp_attribute_t* attribute;
	belle_sip_list_t *custom_attribute_it;
	const char* value;
	const char *mtype,*proto;
	bool_t has_avpf_attributes;

	stream=&md->streams[md->nb_streams];
	media=belle_sdp_media_description_get_media ( media_desc );

	proto = belle_sdp_media_get_protocol ( media );
	stream->proto=SalProtoOther;
	if ( proto ) {
		if (strcasecmp(proto, "RTP/AVP") == 0) {
			stream->proto = SalProtoRtpAvp;
		} else if (strcasecmp(proto, "RTP/SAVP") == 0) {
			stream->proto = SalProtoRtpSavp;
		} else if (strcasecmp(proto, "RTP/AVPF") == 0) {
			stream->proto = SalProtoRtpAvpf;
		} else if (strcasecmp(proto, "RTP/SAVPF") == 0) {
			stream->proto = SalProtoRtpSavpf;
		} else if (strcasecmp(proto, "UDP/TLS/RTP/SAVP") == 0) {
			stream->proto = SalProtoUdpTlsRtpSavp;
		} else if (strcasecmp(proto, "UDP/TLS/RTP/SAVPF") == 0) {
			stream->proto = SalProtoUdpTlsRtpSavpf;
		} else {
			strncpy(stream->proto_other,proto,sizeof(stream->proto_other)-1);
		}
	}
	if ( ( cnx=belle_sdp_media_description_get_connection ( media_desc ) ) && belle_sdp_connection_get_address ( cnx ) ) {
		strncpy ( stream->rtp_addr,belle_sdp_connection_get_address ( cnx ), sizeof ( stream->rtp_addr ) -1 );
		stream->ttl=belle_sdp_connection_get_ttl(cnx);
	}

	stream->rtp_port=belle_sdp_media_get_media_port ( media );

	mtype = belle_sdp_media_get_media_type ( media );
	if ( strcasecmp ( "audio", mtype ) == 0 ) {
		stream->type=SalAudio;
	} else if ( strcasecmp ( "video", mtype ) == 0 ) {
		stream->type=SalVideo;
	} else if ( strcasecmp ( "text", mtype ) == 0 ) {
		stream->type=SalText;
	} else {
		stream->type=SalOther;
		strncpy ( stream->typeother,mtype,sizeof ( stream->typeother )-1 );
	}

	if ( belle_sdp_media_description_get_bandwidth ( media_desc,"AS" ) >0 ) {
		stream->bandwidth=belle_sdp_media_description_get_bandwidth ( media_desc,"AS" );
	}

	if ( belle_sdp_media_description_get_attribute ( media_desc,"sendrecv" ) ) {
		stream->dir=SalStreamSendRecv;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"sendonly" ) ) {
		stream->dir=SalStreamSendOnly;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"recvonly" ) ) {
		stream->dir=SalStreamRecvOnly;
	} else if ( belle_sdp_media_description_get_attribute ( media_desc,"inactive" ) ) {
		stream->dir=SalStreamInactive;
	} else {
		stream->dir=md->dir; /*takes default value if not present*/
	}

	stream->rtcp_mux = belle_sdp_media_description_get_attribute(media_desc, "rtcp-mux") != NULL;
	stream->bundle_only = belle_sdp_media_description_get_attribute(media_desc, "bundle-only") != NULL;

	attribute = belle_sdp_media_description_get_attribute(media_desc, "mid");
	if (attribute){
		value = belle_sdp_attribute_get_value(attribute);
		if (value)
			strncpy(stream->mid, value, sizeof(stream->mid) - 1);
	}

	/* Get media payload types */
	sdp_parse_payload_types(media_desc, stream);

	/* Get media specific RTCP attribute */
	stream->rtcp_port = stream->rtp_port + 1;
	strncpy(stream->rtcp_addr, stream->rtp_addr, sizeof(stream->rtcp_addr) - 1);
	attribute=belle_sdp_media_description_get_attribute(media_desc,"rtcp");
	if (attribute && (value=belle_sdp_attribute_get_value(attribute))!=NULL){
		char *tmp = (char *)ms_malloc0(strlen(value));
		int nb = sscanf(value, "%d IN IP4 %s", &stream->rtcp_port, tmp);
		if (nb == 1) {
			/* SDP rtcp attribute only contains the port */
		} else if (nb == 2) {
			strncpy(stream->rtcp_addr, tmp, sizeof(stream->rtcp_addr));
			stream->rtcp_addr[sizeof(stream->rtcp_addr) - 1] = '\0';
		} else {
			ms_warning("sdp has a strange a=rtcp line (%s) nb=%i", value, nb);
		}
		ms_free(tmp);
	}

	/* Read DTLS specific attributes : check is some are found in the stream description otherwise copy the session description one(which are at least set to Invalid) */
	if (((stream->proto == SalProtoUdpTlsRtpSavpf) || (stream->proto == SalProtoUdpTlsRtpSavp))) {
		attribute=belle_sdp_media_description_get_attribute(media_desc,"setup");
		if (attribute && (value=belle_sdp_attribute_get_value(attribute))!=NULL){
			if (strncmp(value, "actpass", 7) == 0) {
				stream->dtls_role = SalDtlsRoleUnset;
			} else if (strncmp(value, "active", 6) == 0) {
				stream->dtls_role = SalDtlsRoleIsClient;
			} else if (strncmp(value, "passive", 7) == 0) {
				stream->dtls_role = SalDtlsRoleIsServer;
			}
		}
		if (stream->dtls_role != SalDtlsRoleInvalid && (attribute=belle_sdp_media_description_get_attribute(media_desc,"fingerprint"))) {
			strncpy(stream->dtls_fingerprint, belle_sdp_attribute_get_value(attribute),sizeof(stream->dtls_fingerprint));
		}
	}

	/* Read crypto lines if any */
	if (sal_stream_description_has_srtp(stream)) {
		sdp_parse_media_crypto_parameters(media_desc, stream);
	}

	/* Read zrtp-hash attribute */
	if ((attribute=belle_sdp_media_description_get_attribute(media_desc,"zrtp-hash"))!=NULL) {
		if ((value=belle_sdp_attribute_get_value(attribute))!=NULL) {
			strncpy((char *)(stream->zrtphash), belle_sdp_attribute_get_value(attribute),sizeof(stream->zrtphash));
			stream->haveZrtpHash = 1;
		}
	}

	/* Get ICE candidate attributes if any */
	sdp_parse_media_ice_parameters(media_desc, stream);

	has_avpf_attributes = sdp_parse_rtcp_fb_parameters(media_desc, stream);

	/* Get RTCP-FB attributes if any */
	if (sal_stream_description_has_avpf(stream)) {
		enable_avpf_for_stream(stream);
	}else if (has_avpf_attributes ){
		enable_avpf_for_stream(stream);
		stream->implicit_rtcp_fb = TRUE;
	}

	/* Get RTCP-XR attributes if any */
	stream->rtcp_xr = md->rtcp_xr;	// Use session parameters if no stream parameters are defined
	sdp_parse_media_rtcp_xr_parameters(media_desc, &stream->rtcp_xr);

	/* Get the custom attributes, and parse some 'extmap'*/
	for (custom_attribute_it = belle_sdp_media_description_get_attributes(media_desc); custom_attribute_it != NULL; custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		const char *attr_name = belle_sdp_attribute_get_name(attr);
		const char *attr_value = belle_sdp_attribute_get_value(attr);
		stream->custom_sdp_attributes = sal_custom_sdp_attribute_append(stream->custom_sdp_attributes, attr_name, attr_value);

		if (strcasecmp(attr_name, "extmap") == 0){
			char *extmap_urn = (char*)bctbx_malloc0(strlen(attr_value) + 1);
			int rtp_ext_header_id = 0;
			if (sscanf(attr_value, "%i %s", &rtp_ext_header_id, extmap_urn) > 0
				&& strcasecmp(extmap_urn, "urn:ietf:params:rtp-hdrext:sdes:mid") == 0){
				stream->mid_rtp_ext_header_id = rtp_ext_header_id;
			}
			bctbx_free(extmap_urn);
		}
	}

	md->nb_streams++;
	return stream;
}

static void add_bundles(SalMediaDescription *desc, const char *ids){
	char *tmp = (char*)ms_malloc0(strlen(ids) + 1);
	int err;
	SalStreamBundle *bundle = sal_media_description_add_new_bundle(desc);
	do{
		int consumed = 0;
		err = sscanf(ids, "%s%n", tmp, &consumed);
		if (err > 0){
			bundle->mids = bctbx_list_append(bundle->mids, bctbx_strdup(tmp));
			ids += consumed;
		}else break;
	}while( *ids != '\0');
	ms_free(tmp);
}


int sdp_to_media_description( belle_sdp_session_description_t  *session_desc, SalMediaDescription *desc ) {
	belle_sdp_connection_t* cnx;
	belle_sip_list_t* media_desc_it;
	belle_sdp_media_description_t* media_desc;
	belle_sdp_session_name_t *sname;
	belle_sip_list_t *custom_attribute_it;
	const char* value;
	SalDtlsRole session_role=SalDtlsRoleInvalid;
	int i;

	desc->nb_streams = 0;
	desc->dir = SalStreamSendRecv;

	if ( ( cnx=belle_sdp_session_description_get_connection ( session_desc ) ) && belle_sdp_connection_get_address ( cnx ) ) {
		strncpy ( desc->addr,belle_sdp_connection_get_address ( cnx ),sizeof ( desc->addr ) -1  );
	}
	if ( (sname=belle_sdp_session_description_get_session_name(session_desc)) && belle_sdp_session_name_get_value(sname) ){
		strncpy(desc->name,belle_sdp_session_name_get_value(sname),sizeof(desc->name) - 1);
	}

	if ( belle_sdp_session_description_get_bandwidth ( session_desc,"AS" ) >0 ) {
		desc->bandwidth=belle_sdp_session_description_get_bandwidth ( session_desc,"AS" );
	}

	belle_sdp_origin_t *origin = belle_sdp_session_description_get_origin(session_desc);
	desc->session_id = belle_sdp_origin_get_session_id(origin);
	desc->session_ver = belle_sdp_origin_get_session_version(origin);

	/*in some very rare case, session attribute may set stream dir*/
	if ( belle_sdp_session_description_get_attribute ( session_desc,"sendrecv" ) ) {
		desc->dir=SalStreamSendRecv;
	} else if ( belle_sdp_session_description_get_attribute ( session_desc,"sendonly" ) ) {
		desc->dir=SalStreamSendOnly;
	} else if ( belle_sdp_session_description_get_attribute ( session_desc,"recvonly" ) ) {
		desc->dir=SalStreamRecvOnly;
	} else if ( belle_sdp_session_description_get_attribute ( session_desc,"inactive" ) ) {
		desc->dir=SalStreamInactive;
	}

	/*DTLS attributes can be defined at session level.*/
	value=belle_sdp_session_description_get_attribute_value(session_desc,"setup");
	if (value){
		if (strncmp(value, "actpass", 7) == 0) {
			session_role = SalDtlsRoleUnset;
		} else if (strncmp(value, "active", 6) == 0) {
			session_role = SalDtlsRoleIsClient;
		} else if (strncmp(value, "passive", 7) == 0) {
			session_role = SalDtlsRoleIsServer;
		}
	}
	value=belle_sdp_session_description_get_attribute_value(session_desc,"fingerprint");
	/*copy dtls attributes to every streams, might be overwritten stream by stream*/
	for (i=0;i<SAL_MEDIA_DESCRIPTION_MAX_STREAMS;i++) {
		if (value)
			strncpy(desc->streams[i].dtls_fingerprint, value, sizeof(desc->streams[i].dtls_fingerprint));
		desc->streams[i].dtls_role=session_role; /*set or reset value*/
	}

	/* Get ICE remote ufrag and remote pwd, and ice_lite flag */
	value=belle_sdp_session_description_get_attribute_value(session_desc,"ice-ufrag");
	if (value) strncpy(desc->ice_ufrag, value, sizeof(desc->ice_ufrag) - 1);

	value=belle_sdp_session_description_get_attribute_value(session_desc,"ice-pwd");
	if (value) strncpy(desc->ice_pwd, value, sizeof(desc->ice_pwd)-1);

	value=belle_sdp_session_description_get_attribute_value(session_desc,"ice-lite");
	if (value) desc->ice_lite = TRUE;

	/* Get session RTCP-XR attributes if any */
	sdp_parse_session_rtcp_xr_parameters(session_desc, &desc->rtcp_xr);

	/* Do we have Lime Ik attribute */
	value = belle_sdp_session_description_get_attribute_value(session_desc,"Ik");
	if (value) desc->haveLimeIk = TRUE;

	/* get ready to parse also lime-Ik */
	value = belle_sdp_session_description_get_attribute_value(session_desc,"lime-Ik");
	if (value) desc->haveLimeIk = TRUE;

	/* Get the custom attributes, parse some of them that are relevant */
	for (custom_attribute_it = belle_sdp_session_description_get_attributes(session_desc); custom_attribute_it != NULL; custom_attribute_it = custom_attribute_it->next) {
		belle_sdp_attribute_t *attr = (belle_sdp_attribute_t *)custom_attribute_it->data;
		desc->custom_sdp_attributes = sal_custom_sdp_attribute_append(desc->custom_sdp_attributes, belle_sdp_attribute_get_name(attr), belle_sdp_attribute_get_value(attr));

		if (strcasecmp(belle_sdp_attribute_get_name(attr), "group") == 0){
			value = belle_sdp_attribute_get_value(attr);
			if (value && strncasecmp(value, "BUNDLE", strlen("BUNDLE")) == 0){
				add_bundles(desc, value + strlen("BUNDLE"));
			}
		}
	}

	for ( media_desc_it=belle_sdp_session_description_get_media_descriptions ( session_desc )
						; media_desc_it!=NULL
			; media_desc_it=media_desc_it->next ) {
		if (desc->nb_streams==SAL_MEDIA_DESCRIPTION_MAX_STREAMS){
			ms_warning("Cannot convert mline at position [%i] from SDP to SalMediaDescription",desc->nb_streams);
			break;
		}
		media_desc=BELLE_SDP_MEDIA_DESCRIPTION ( media_desc_it->data );
		sdp_to_stream_description(desc, media_desc);
	}
	return 0;
}
