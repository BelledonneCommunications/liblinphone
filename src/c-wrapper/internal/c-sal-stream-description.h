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

#ifndef _C_SAL_STREAM_DESCRIPTION_H_
#define _C_SAL_STREAM_DESCRIPTION_H_

#include "c-wrapper/internal/c-sal.h"

typedef struct SalStreamDescription SalStreamDescription;

#ifdef __cplusplus
extern "C" {
#endif

SalStreamDescription * sal_stream_description_new(void);
void sal_stream_description_init(SalStreamDescription *sd);
void sal_stream_description_destroy(SalStreamDescription *sd);

/* Enabled means that the stream exists and is accepted as part of the session: the port value is non-zero or the stream has bundle-only attribute.
 *However, it may be marked with a=inactive, which is unrelated to the return value of this function.*/
bool_t sal_stream_description_enabled(const SalStreamDescription *sd);
void sal_stream_description_disable(SalStreamDescription *sd);
bool_t sal_stream_description_has_avpf(const SalStreamDescription *sd);
bool_t sal_stream_description_has_implicit_avpf(const SalStreamDescription *sd);
bool_t sal_stream_description_has_srtp(const SalStreamDescription *sd);
bool_t sal_stream_description_has_dtls(const SalStreamDescription *sd);
bool_t sal_stream_description_has_limeIk(const SalStreamDescription *sd);
bool_t sal_stream_description_has_ipv6(const SalStreamDescription *md);

const char *sal_stream_description_get_type_as_string(const SalStreamDescription *desc);
const char *sal_stream_description_get_proto_as_string(const SalStreamDescription *desc);
bool_t sal_stream_description_has_zrtp(const SalStreamDescription *sd);

int sal_stream_description_equals(const SalStreamDescription *sd1, const SalStreamDescription *sd2);

LINPHONE_PUBLIC const char * sal_stream_description_get_rtcp_address(SalStreamDescription *sd);
LINPHONE_PUBLIC const char * sal_stream_description_get_rtp_address(SalStreamDescription *sd);
LINPHONE_PUBLIC MSList * sal_stream_description_get_payloads(SalStreamDescription *sd);
LINPHONE_PUBLIC SalStreamDir sal_stream_description_get_direction(SalStreamDescription *sd);

#ifdef __cplusplus
}
#endif

#endif // ifndef _C_SAL_STREAM_DESCRIPTION_H_
