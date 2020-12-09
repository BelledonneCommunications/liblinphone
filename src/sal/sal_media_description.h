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

#ifndef _SAL_MEDIA_DESCRIPTION_H_
#define _SAL_MEDIA_DESCRIPTION_H_

#include "ortp/rtpsession.h"
#include "sal/sal_stream_bundle.h"

#define SAL_MEDIA_DESCRIPTION_MAX_STREAMS 8

typedef struct SalMediaDescription{
	int refcount;
	char name[64];
	char addr[64];
	char username[64];
	int nb_streams;
	int bandwidth;
	unsigned int session_ver;
	unsigned int session_id;
	SalStreamDir dir;
	SalStreamDescription streams[SAL_MEDIA_DESCRIPTION_MAX_STREAMS];
	SalCustomSdpAttribute *custom_sdp_attributes;
	OrtpRtcpXrConfiguration rtcp_xr;
	char ice_ufrag[SAL_MEDIA_DESCRIPTION_MAX_ICE_UFRAG_LEN];
	char ice_pwd[SAL_MEDIA_DESCRIPTION_MAX_ICE_PWD_LEN];
	bctbx_list_t *bundles; /* list of SalStreamBundle */
	bool_t ice_lite;
	bool_t set_nortpproxy;
	bool_t accept_bundles; /* Set to TRUE if RTP bundles can be accepted during offer answer. This field has no appearance on the SDP.*/
	bool_t pad[1];
} SalMediaDescription;

#ifdef __cplusplus
extern "C" {
#endif

SalMediaDescription *sal_media_description_new(void);
SalMediaDescription * sal_media_description_ref(SalMediaDescription *md);
void sal_media_description_unref(SalMediaDescription *md);
bool_t sal_media_description_empty(const SalMediaDescription *md);
int sal_media_description_equals(const SalMediaDescription *md1, const SalMediaDescription *md2);
int sal_media_description_global_equals(const SalMediaDescription *md1, const SalMediaDescription *md2);
char * sal_media_description_print_differences(int result);
bool_t sal_media_description_has_dir(const SalMediaDescription *md, SalStreamDir dir);
unsigned int sal_media_description_nb_active_streams_of_type(SalMediaDescription *md, SalStreamType type);
SalStreamDescription * sal_media_description_get_active_stream_of_type(SalMediaDescription *md, SalStreamType type, unsigned int idx);
SalStreamDescription * sal_media_description_find_secure_stream_of_type(SalMediaDescription *md, SalStreamType type);
SalStreamDescription * sal_media_description_find_best_stream(SalMediaDescription *md, SalStreamType type);
void sal_media_description_set_dir(SalMediaDescription *md, SalStreamDir stream_dir);

bool_t sal_media_description_has_avpf(const SalMediaDescription *md);
bool_t sal_media_description_has_implicit_avpf(const SalMediaDescription *md);
bool_t sal_media_description_has_srtp(const SalMediaDescription *md);
bool_t sal_media_description_has_dtls(const SalMediaDescription *md);
bool_t sal_media_description_has_zrtp(const SalMediaDescription *md);
bool_t sal_media_description_has_ipv6(const SalMediaDescription *md);
int sal_media_description_get_nb_active_streams(const SalMediaDescription *md);

SalStreamBundle * sal_media_description_add_new_bundle(SalMediaDescription *md);
int sal_media_description_lookup_mid(const SalMediaDescription *md, const char *mid);
int sal_media_description_get_index_of_transport_owner(const SalMediaDescription *md, const SalStreamDescription *sd);

#ifdef __cplusplus
}
#endif



#endif // ifndef _SAL_MEDIA_DESCRIPTION_H_
