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

#ifndef SAL_IMPL_H_
#define SAL_IMPL_H_

#include "belle-sip/belle-sdp.h"
#include "belle-sip/belle-sip.h"
#include "c-wrapper/internal/c-sal.h"

belle_sdp_attribute_t *create_rtcp_xr_attribute(const OrtpRtcpXrConfiguration *config);
void add_rtcp_fb_ccm_attribute(belle_sdp_media_description_t *media_desc,
                               int8_t id,
                               belle_sdp_rtcp_fb_val_param_t param);
void add_rtcp_fb_nack_attribute(belle_sdp_media_description_t *media_desc,
                                int8_t id,
                                belle_sdp_rtcp_fb_val_param_t param);
void add_rtcp_fb_ack_attribute(belle_sdp_media_description_t *media_desc,
                               int8_t id,
                               belle_sdp_rtcp_fb_val_param_t param);
void add_rtcp_fb_trr_int_attribute(belle_sdp_media_description_t *media_desc, int8_t id, uint16_t trr_int);
void add_rtcp_fb_goog_remb_attribute(belle_sdp_media_description_t *media_desc, int8_t id);

void sdp_parse_session_rtcp_xr_parameters(const belle_sdp_session_description_t *session_desc,
                                          OrtpRtcpXrConfiguration *config);
void sdp_parse_media_rtcp_xr_parameters(const belle_sdp_media_description_t *media_desc,
                                        OrtpRtcpXrConfiguration *config);

bool_t _sal_compute_sal_errors(belle_sip_response_t *response, SalReason *sal_reason, char *reason, size_t reason_size);
SalReason _sal_reason_from_sip_code(int code);
/*create SalAuthInfo by copying username and realm from suth event*/
SalAuthInfo *sal_auth_info_create(belle_sip_auth_event_t *event);

#endif /* SAL_IMPL_H_ */
