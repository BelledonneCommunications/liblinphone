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

#include "sal/sal_media_description.h"
#include "c-wrapper/internal/c-tools.h"
#include "c-wrapper/internal/c-sal-media-description.h"
#include "c-wrapper/internal/c-sal-stream-bundle.h"
#include "tester_utils.h"


SalMediaDescription *sal_media_description_new(){
	SalMediaDescription *md= new SalMediaDescription();
	md->refcount=1;
	md->streams.clear();
	md->pad.clear();
	md->custom_sdp_attributes = nullptr;
	md->bundles = nullptr;
	return md;
}

SalStreamBundle * sal_media_description_add_new_bundle(SalMediaDescription *md){
	return md->addNewBundle();
}

int sal_media_description_lookup_mid(const SalMediaDescription *md, const char *mid){
	return md->lookupMid(L_C_TO_STRING(mid));
}

const SalStreamBundle *sal_media_description_get_bundle_from_mid(const SalMediaDescription *md, const char *mid){
	return md->getBundleFromMid(L_C_TO_STRING(mid));
}

int sal_media_description_get_index_of_transport_owner(const SalMediaDescription *md, const SalStreamDescription *sd){
	return md->getIndexOfTransportOwner(sd);
}

SalMediaDescription * sal_media_description_ref(SalMediaDescription *md){
	md->refcount++;
	return md;
}

void sal_media_description_unref(SalMediaDescription *md){
	md->refcount--;
	if (md->refcount==0){
		delete md;
	}
}

const SalStreamDescription *sal_media_description_find_stream(const SalMediaDescription *md, SalMediaProto proto, SalStreamType type){
	return md->findStream(proto, type);
}

unsigned int sal_media_description_nb_active_streams_of_type(const SalMediaDescription *md, SalStreamType type) {
	return md->nbActiveStreamsOfType(type);
}

const SalStreamDescription * sal_media_description_get_active_stream_of_type(const SalMediaDescription *md, SalStreamType type, unsigned int idx) {
	return md->getActiveStreamOfType(type, idx);
}

const SalStreamDescription * sal_media_description_find_secure_stream_of_type(const SalMediaDescription *md, SalStreamType type) {
	return md->findSecureStreamOfType(type);
}

const SalStreamDescription * sal_media_description_find_best_stream(const SalMediaDescription *md, SalStreamType type) {
	return md->findBestStream(type);
}

bool_t sal_media_description_empty(const SalMediaDescription *md){
	return md->isEmpty();
}

void sal_media_description_set_dir(SalMediaDescription *md, SalStreamDir stream_dir){
	md->setDir(stream_dir);
}

int sal_media_description_get_nb_active_streams(const SalMediaDescription *md) {
	return md->getNbActiveStreams();
}

bool_t sal_media_description_has_dir(const SalMediaDescription *md, const SalStreamDir stream_dir){
	return md->hasDir(stream_dir);
}

bool_t sal_media_description_has_avpf(const SalMediaDescription *md) {
	return md->hasAvpf();
}

bool_t sal_media_description_has_implicit_avpf(const SalMediaDescription *md) {
	return md->hasImplicitAvpf();
}

bool_t sal_media_description_has_srtp(const SalMediaDescription *md) {
	return md->hasSrtp();
}

bool_t sal_media_description_has_dtls(const SalMediaDescription *md) {
	return md->hasDtls();
}

bool_t sal_media_description_has_zrtp(const SalMediaDescription *md) {
	return md->hasZrtp();
}

bool_t sal_media_description_has_ipv6(const SalMediaDescription *md){
	return md->hasIpv6();
}

int sal_media_description_equals(const SalMediaDescription *md1, const SalMediaDescription *md2) {
	return md1->equal(*md2);
}

int sal_media_description_global_equals(const SalMediaDescription *md1, const SalMediaDescription *md2) {
	return md1->globalEqual(*md2);
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

size_t sal_media_description_get_nb_streams(const SalMediaDescription *md){
	return md->getNbStreams();
}

const char * sal_media_description_get_address(const SalMediaDescription *md){
	return L_STRING_TO_C(md->getAddress());
}

const SalStreamDescription * sal_media_description_get_stream_idx(const SalMediaDescription *md, unsigned int idx) {
	return md->getStreamIdx(idx);
}
