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
#include "sal_impl.h"

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

void add_rtcp_fb_ack_attribute(belle_sdp_media_description_t *media_desc,
                               int8_t id,
                               belle_sdp_rtcp_fb_val_param_t param) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_ACK);
	belle_sdp_rtcp_fb_attribute_set_param(attribute, param);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

void add_rtcp_fb_nack_attribute(belle_sdp_media_description_t *media_desc,
                                int8_t id,
                                belle_sdp_rtcp_fb_val_param_t param) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_NACK);
	belle_sdp_rtcp_fb_attribute_set_param(attribute, param);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

void add_rtcp_fb_ccm_attribute(belle_sdp_media_description_t *media_desc,
                               int8_t id,
                               belle_sdp_rtcp_fb_val_param_t param) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_CCM);
	belle_sdp_rtcp_fb_attribute_set_param(attribute, param);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

void add_rtcp_fb_goog_remb_attribute(belle_sdp_media_description_t *media_desc, int8_t id) {
	belle_sdp_rtcp_fb_attribute_t *attribute = belle_sdp_rtcp_fb_attribute_new();
	belle_sdp_rtcp_fb_attribute_set_id(attribute, id);
	belle_sdp_rtcp_fb_attribute_set_type(attribute, BELLE_SDP_RTCP_FB_GOOG_REMB);
	belle_sdp_media_description_add_attribute(media_desc, BELLE_SDP_ATTRIBUTE(attribute));
}

belle_sdp_attribute_t *create_rtcp_xr_attribute(const OrtpRtcpXrConfiguration *config) {
	belle_sdp_rtcp_xr_attribute_t *attribute = belle_sdp_rtcp_xr_attribute_new();
	if (config->rcvr_rtt_mode != OrtpRtcpXrRcvrRttNone) {
		if (config->rcvr_rtt_mode == OrtpRtcpXrRcvrRttAll)
			belle_sdp_rtcp_xr_attribute_set_rcvr_rtt_mode(attribute, "all");
		else if (config->rcvr_rtt_mode == OrtpRtcpXrRcvrRttSender)
			belle_sdp_rtcp_xr_attribute_set_rcvr_rtt_mode(attribute, "sender");
		belle_sdp_rtcp_xr_attribute_set_rcvr_rtt_max_size(attribute, config->rcvr_rtt_max_size);
	}
	belle_sdp_rtcp_xr_attribute_set_stat_summary(attribute, (config->stat_summary_enabled == TRUE));
	if (config->stat_summary_enabled == TRUE) {
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryLoss)
			belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "loss");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryDup)
			belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "dup");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryJitt)
			belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "jitt");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryTTL)
			belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "TTL");
		if (config->stat_summary_flags & OrtpRtcpXrStatSummaryHL)
			belle_sdp_rtcp_xr_attribute_add_stat_summary_flag(attribute, "HL");
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
			for (stat_summary_flag_it = belle_sdp_rtcp_xr_attribute_get_stat_summary_flags(xr_attr);
			     stat_summary_flag_it != NULL; stat_summary_flag_it = stat_summary_flag_it->next) {
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

void sdp_parse_session_rtcp_xr_parameters(const belle_sdp_session_description_t *session_desc,
                                          OrtpRtcpXrConfiguration *config) {
	const belle_sdp_attribute_t *attribute = belle_sdp_session_description_get_attribute(session_desc, "rtcp-xr");
	sdp_parse_rtcp_xr_parameters(attribute, config);
}

void sdp_parse_media_rtcp_xr_parameters(const belle_sdp_media_description_t *media_desc,
                                        OrtpRtcpXrConfiguration *config) {
	const belle_sdp_attribute_t *attribute = belle_sdp_media_description_get_attribute(media_desc, "rtcp-xr");
	sdp_parse_rtcp_xr_parameters(attribute, config);
}
