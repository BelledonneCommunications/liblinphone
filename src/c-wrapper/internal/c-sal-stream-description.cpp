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

const char *sal_stream_description_get_type_as_string(const SalStreamDescription *desc){
//	return L_STRING_TO_C(desc->getTypeAsString());
	if (desc->getType()==SalOther) return L_STRING_TO_C(desc->typeother);
	else return sal_stream_type_to_string(desc->getType());
}

const char *sal_stream_description_get_proto_as_string(const SalStreamDescription *desc){
	if (desc->getProto()==SalProtoOther) return L_STRING_TO_C(desc->proto_other);
	else return sal_media_proto_to_string(desc->getProto());
	//return L_STRING_TO_C(desc->getProtoAsString());
}

SalStreamDescription sal_stream_description_create(){
	SalStreamDescription sd;
	return sd;
}

SalStreamDescription *sal_stream_description_new(){
	SalStreamDescription *sd= new SalStreamDescription();
	return sd;
}

void sal_stream_description_destroy(SalStreamDescription *sd) {
	sd->destroy();
}

int sal_stream_description_equals(const SalStreamDescription *sd1, const SalStreamDescription *sd2) {
	return sd1->equal(*sd2);
}

bool_t sal_stream_description_enabled(const SalStreamDescription *sd) {
	return sd->enabled();
}

void sal_stream_description_disable(SalStreamDescription *sd){
	sd->disable();
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool_t sal_stream_description_has_avpf(const SalStreamDescription *sd) {
	return sd->hasAvpf();
}

bool_t sal_stream_description_has_ipv6(const SalStreamDescription *sd){
	return sd->hasIpv6();
}

bool_t sal_stream_description_has_implicit_avpf(const SalStreamDescription *sd){
	return sd->hasImplicitAvpf();
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool_t sal_stream_description_has_srtp(const SalStreamDescription *sd) {
	return sd->hasSrtp();
}

bool_t sal_stream_description_has_dtls(const SalStreamDescription *sd) {
	return sd->hasDtls();
}

bool_t sal_stream_description_has_zrtp(const SalStreamDescription *sd) {
	return sd->hasZrtp();
}

bool_t sal_stream_description_has_limeIk(const SalStreamDescription *sd) {
	return sd->hasLimeIk();
}

const char * sal_stream_description_get_rtcp_address(SalStreamDescription *sd){
	return L_STRING_TO_C(sd->getRtcpAddress());
}

const char * sal_stream_description_get_rtp_address(SalStreamDescription *sd){
	return L_STRING_TO_C(sd->getRtpAddress());
}

SalStreamDir sal_stream_description_get_direction(SalStreamDescription *sd){
	return sd->getDirection();
}

MSList * sal_stream_description_get_payloads(SalStreamDescription *sd){
	return sd->getPayloads();
}
