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

static bool_t is_null_address(const std::string & addr){
	return addr.compare("0.0.0.0")==0 || addr.compare("::0")==0;
}

/*check for the presence of at least one stream with requested direction */
static bool_t has_dir(const SalMediaDescription *md, SalStreamDir stream_dir){
	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for(auto & stream : md->streams){
		const SalStreamDescription *ss=&stream;
		if (!sal_stream_description_enabled(ss)) continue;
		if (ss->dir==stream_dir) {
			return TRUE;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (ss->dir==SalStreamSendRecv && stream_dir==SalStreamSendOnly && (is_null_address(md->addr) || is_null_address(ss->rtp_addr))){
			return TRUE;
		}
	}
	return FALSE;
}

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
	SalStreamBundle *bundle = sal_stream_bundle_new();
	md->bundles = bctbx_list_append(md->bundles, bundle);
	return bundle;
}

int sal_stream_bundle_has_mid(const SalStreamBundle *bundle, const char *mid){
	const bctbx_list_t *elem;
	for (elem = bundle->mids; elem != NULL; elem = elem->next){
		const char *m = (const char *) elem->data;
		if (strcmp(m, mid) == 0) return TRUE;
	}
	return FALSE;
}


int sal_media_description_lookup_mid(const SalMediaDescription *md, const char *mid){
	int index;
	for (index = 0 ; index < md->nb_streams; ++index){
		const SalStreamDescription * sd = &md->streams[(size_t)index];
		if (sd->mid.compare(mid) == 0){
			return index;
		}
	}
	return -1;
}

const SalStreamBundle *sal_media_description_get_bundle_from_mid(const SalMediaDescription *md, const char *mid){
	const bctbx_list_t *elem;
	for (elem = md->bundles; elem != NULL; elem = elem->next){
		SalStreamBundle *bundle = (SalStreamBundle *)elem->data;
		if (sal_stream_bundle_has_mid(bundle, mid)) return bundle;
	}
	return NULL;
}

int sal_media_description_get_index_of_transport_owner(const SalMediaDescription *md, const SalStreamDescription *sd){
	const SalStreamBundle *bundle;
	const char *master_mid;
	int index;
	if (sd->mid.empty() == true) return -1; /* not part of any bundle */
	/* lookup the mid in the bundle descriptions */
	bundle = sal_media_description_get_bundle_from_mid(md, L_STRING_TO_C(sd->mid));
	if (!bundle) {
		ms_warning("Orphan stream with mid '%s'", L_STRING_TO_C(sd->mid));
		return -1;
	}
	master_mid = sal_stream_bundle_get_mid_of_transport_owner(bundle);
	index = sal_media_description_lookup_mid(md, master_mid);
	if (index == -1){
		ms_error("Stream with mid '%s' has no transport owner (mid '%s') !", L_STRING_TO_C(sd->mid), master_mid);
	}
	return index;
}

static void sal_media_description_destroy(SalMediaDescription *md){
	for(auto & stream : md->streams){
		sal_stream_description_destroy(&stream);
	}
	md->streams.clear();
	bctbx_list_free_with_data(md->bundles, (void (*)(void*)) sal_stream_bundle_destroy);
	sal_custom_sdp_attribute_free(md->custom_sdp_attributes);
	delete md;
}

SalMediaDescription * sal_media_description_ref(SalMediaDescription *md){
	md->refcount++;
	return md;
}

void sal_media_description_unref(SalMediaDescription *md){
	md->refcount--;
	if (md->refcount==0){
		sal_media_description_destroy (md);
	}
}

SalStreamDescription *sal_media_description_find_stream(SalMediaDescription *md, SalMediaProto proto, SalStreamType type){
	for(auto & stream : md->streams){
		SalStreamDescription *ss=&stream;
		if (!sal_stream_description_enabled(ss)) continue;
		if (ss->proto==proto && ss->type==type) return ss;
	}
	return NULL;
}

unsigned int sal_media_description_nb_active_streams_of_type(SalMediaDescription *md, SalStreamType type) {
	unsigned int nb = 0;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (stream.type == type) nb++;
	}
	return nb;
}

SalStreamDescription * sal_media_description_get_active_stream_of_type(SalMediaDescription *md, SalStreamType type, unsigned int idx) {
	for(auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (stream.type == type) {
			if (idx-- == 0) return &stream;
		}
	}
	return NULL;
}

SalStreamDescription * sal_media_description_find_secure_stream_of_type(SalMediaDescription *md, SalStreamType type) {
	SalStreamDescription *desc = sal_media_description_find_stream(md, SalProtoRtpSavpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpSavp, type);
	return desc;
}

SalStreamDescription * sal_media_description_find_best_stream(SalMediaDescription *md, SalStreamType type) {
	SalStreamDescription *desc = sal_media_description_find_stream(md, SalProtoUdpTlsRtpSavpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoUdpTlsRtpSavp, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpSavpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpSavp, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpAvpf, type);
	if (desc == NULL) desc = sal_media_description_find_stream(md, SalProtoRtpAvp, type);
	return desc;
}

bool_t sal_media_description_empty(const SalMediaDescription *md){
	if (sal_media_description_get_nb_active_streams(md) > 0) return FALSE;
	return TRUE;
}

void sal_media_description_set_dir(SalMediaDescription *md, SalStreamDir stream_dir){
	for(auto & stream : md->streams){
		SalStreamDescription *ss=&stream;
		if (!sal_stream_description_enabled(ss)) continue;
		stream.dir=stream_dir;
	}
}

int sal_media_description_get_nb_active_streams(const SalMediaDescription *md) {
	int nb = 0;
	for(auto & stream : md->streams){
		if (sal_stream_description_enabled(&stream)) nb++;
	}
	return nb;
}

bool_t sal_media_description_has_dir(const SalMediaDescription *md, SalStreamDir stream_dir){
	if (stream_dir==SalStreamRecvOnly){
		return has_dir(md, SalStreamRecvOnly) && !(has_dir(md,SalStreamSendOnly) || has_dir(md,SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendOnly){
		return has_dir(md, SalStreamSendOnly) && !(has_dir(md,SalStreamRecvOnly) || has_dir(md,SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendRecv){
		return has_dir(md,SalStreamSendRecv);
	}else{
		/*SalStreamInactive*/
		if (has_dir(md,SalStreamSendOnly) || has_dir(md,SalStreamSendRecv)  || has_dir(md,SalStreamRecvOnly))
			return FALSE;
		else return TRUE;
	}
	return FALSE;
}

bool_t sal_media_description_has_avpf(const SalMediaDescription *md) {
	if (md->nb_streams == 0) return FALSE;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (sal_stream_description_has_avpf(&stream) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_implicit_avpf(const SalMediaDescription *md) {
	if (md->nb_streams == 0) return FALSE;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (sal_stream_description_has_implicit_avpf(&stream) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_srtp(const SalMediaDescription *md) {
	if (md->nb_streams == 0) return FALSE;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (sal_stream_description_has_srtp(&stream)) return TRUE;
	}
	return FALSE;
}

bool_t sal_media_description_has_dtls(const SalMediaDescription *md) {
	if (md->nb_streams == 0) return FALSE;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (sal_stream_description_has_dtls(&stream) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_zrtp(const SalMediaDescription *md) {
	if (md->nb_streams == 0) return FALSE;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (sal_stream_description_has_zrtp(&stream) != TRUE) return FALSE;
	}
	return TRUE;
}

bool_t sal_media_description_has_ipv6(const SalMediaDescription *md){
	if (md->nb_streams == 0) return FALSE;
	for(const auto & stream : md->streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (stream.rtp_addr.empty() == false){
			if (!sal_stream_description_has_ipv6(&stream)) return FALSE;
		}else{
			if (md->addr.find(':') == std::string::npos) return FALSE;
		}
	}
	return TRUE;
}

int sal_media_description_equals(const SalMediaDescription *md1, const SalMediaDescription *md2) {
	int result = sal_media_description_global_equals(md1, md2);
	for(auto stream1 = md1->streams.cbegin(), stream2 = md2->streams.cbegin(); (stream1 != md1->streams.cend() && stream2 != md2->streams.cend()); ++stream1, ++stream2){
		if (!sal_stream_description_enabled(&(*stream1)) && !sal_stream_description_enabled(&(*stream2))) continue;
		result |= sal_stream_description_equals(&(*stream1), &(*stream2));
	}
	return result;
}

int sal_media_description_global_equals(const SalMediaDescription *md1, const SalMediaDescription *md2) {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	if (md1->addr.compare(md2->addr) != 0) result |= SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
	if (md1->addr.empty() == false && md2->addr.empty() == false && ms_is_multicast(L_STRING_TO_C(md1->addr)) != ms_is_multicast(L_STRING_TO_C(md2->addr)))
		result |= SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED;
	if (md1->nb_streams != md2->nb_streams) result |= SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
	if (md1->bandwidth != md2->bandwidth) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;

	/* ICE */
	if (md1->ice_ufrag.compare(md2->ice_ufrag) != 0 && !md2->ice_ufrag.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;
	if (md1->ice_pwd.compare(md2->ice_pwd) != 0 && !md2->ice_pwd.empty()) result |= SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED;

	return result;
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

int sal_media_description_get_nb_streams(SalMediaDescription *md){
	return md->nb_streams;
}

const char * sal_media_description_get_address(SalMediaDescription *md){
	if (md->addr.empty()) {
		return NULL;
	}
	return L_STRING_TO_C(md->addr);
}

SalStreamDescription * sal_media_description_get_stream_idx(SalMediaDescription *md, unsigned int idx) {
	if (idx < (unsigned int)md->streams.size()) {
		return &(md->streams[idx]);
	}
	return NULL;
}
