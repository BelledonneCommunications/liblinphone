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

#ifndef _C_SAL_MEDIA_DESCRIPTION_H_
#define _C_SAL_MEDIA_DESCRIPTION_H_

#include "ortp/rtpsession.h"
#include "linphone/api/c-types.h"
#include "c-wrapper/internal/c-sal.h"
#include "c-wrapper/internal/c-sal-stream-description.h"

typedef struct SalMediaDescription SalMediaDescription;
typedef struct SalStreamBundle SalStreamBundle;

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
LINPHONE_PUBLIC SalMediaDescription *_linphone_call_get_local_desc (const LinphoneCall *call);
LINPHONE_PUBLIC SalMediaDescription *_linphone_call_get_result_desc (const LinphoneCall *call);
LINPHONE_PUBLIC SalStreamDescription * sal_media_description_get_stream_idx(SalMediaDescription *md, unsigned int idx);
LINPHONE_PUBLIC SalStreamDescription *sal_media_description_find_stream(SalMediaDescription *md, SalMediaProto proto, SalStreamType type);
LINPHONE_PUBLIC const char * sal_media_description_get_address(SalMediaDescription *md);
LINPHONE_PUBLIC size_t sal_media_description_get_nb_streams(SalMediaDescription *md);
#ifdef __cplusplus
}
#endif

#endif // ifndef _C_SAL_MEDIA_DESCRIPTION_H_
