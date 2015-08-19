
/*
linphone
Copyright (C) 2010  Belledonne Communications SARL
 (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef _WIN32
#include <time.h>
#endif
#include "linphonecore.h"
#include "sipsetup.h"
#include "lpconfig.h"
#include "private.h"
#include <ortp/event.h>
#include <ortp/b64.h>
#include <math.h>

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msvolume.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msjpegwriter.h"
#include "mediastreamer2/mseventqueue.h"
#include "mediastreamer2/mssndcard.h"

static const char *EC_STATE_STORE = ".linphone.ecstate";
#define EC_STATE_MAX_LEN 1048576 // 1Mo

static void linphone_call_stats_uninit(LinphoneCallStats *stats);
static void linphone_call_get_local_ip(LinphoneCall *call, const LinphoneAddress *remote_addr);

#ifdef VIDEO_ENABLED
MSWebCam *get_nowebcam_device(){
	return ms_web_cam_manager_get_cam(ms_web_cam_manager_get(),"StaticImage: Static picture");
}
#endif

static bool_t generate_b64_crypto_key(int key_length, char* key_out, size_t key_out_size) {
	int b64_size;
	uint8_t* tmp = (uint8_t*) ms_malloc0(key_length);
	if (sal_get_random_bytes(tmp, key_length)==NULL) {
		ms_error("Failed to generate random key");
		ms_free(tmp);
		return FALSE;
	}

	b64_size = b64_encode((const char*)tmp, key_length, NULL, 0);
	if (b64_size == 0) {
		ms_error("Failed to get b64 result size");
		ms_free(tmp);
		return FALSE;
	}
	if (b64_size>=key_out_size){
		ms_error("Insufficient room for writing base64 SRTP key");
		ms_free(tmp);
		return FALSE;
	}
	b64_size=b64_encode((const char*)tmp, key_length, key_out, key_out_size);
	if (b64_size == 0) {
		ms_error("Failed to b64 encode key");
		ms_free(tmp);
		return FALSE;
	}
	key_out[b64_size] = '\0';
	ms_free(tmp);
	return TRUE;
}

LinphoneCore *linphone_call_get_core(const LinphoneCall *call){
	return call->core;
}

const char* linphone_call_get_authentication_token(LinphoneCall *call){
	return call->auth_token;
}

/**
 * Returns whether ZRTP authentication token is verified.
 * If not, it must be verified by users as described in ZRTP procedure.
 * Once done, the application must inform of the results with linphone_call_set_authentication_token_verified().
 * @param call the LinphoneCall
 * @return TRUE if authentication token is verifed, false otherwise.
 * @ingroup call_control
**/
bool_t linphone_call_get_authentication_token_verified(LinphoneCall *call){
	return call->auth_token_verified;
}

static bool_t linphone_call_all_streams_encrypted(const LinphoneCall *call) {
	int number_of_encrypted_stream = 0;
	int number_of_active_stream = 0;
	if (call) {
		if (call->audiostream && media_stream_get_state((MediaStream *)call->audiostream) == MSStreamStarted) {
			number_of_active_stream++;
			if(media_stream_secured((MediaStream *)call->audiostream))
				number_of_encrypted_stream++;
		}
		if (call->videostream && media_stream_get_state((MediaStream *)call->videostream) == MSStreamStarted) {
			number_of_active_stream++;
			if (media_stream_secured((MediaStream *)call->videostream))
				number_of_encrypted_stream++;
		}
	}
	return number_of_active_stream>0 && number_of_active_stream==number_of_encrypted_stream;
}

static bool_t linphone_call_all_streams_avpf_enabled(const LinphoneCall *call) {
	int nb_active_streams = 0;
	int nb_avpf_enabled_streams = 0;
	if (call) {
		if (call->audiostream && media_stream_get_state((MediaStream *)call->audiostream) == MSStreamStarted) {
			nb_active_streams++;
			if (media_stream_avpf_enabled((MediaStream *)call->audiostream))
				nb_avpf_enabled_streams++;
		}
		if (call->videostream && media_stream_get_state((MediaStream *)call->videostream) == MSStreamStarted) {
			nb_active_streams++;
			if (media_stream_avpf_enabled((MediaStream *)call->videostream))
				nb_avpf_enabled_streams++;
		}
	}
	return ((nb_active_streams > 0) && (nb_active_streams == nb_avpf_enabled_streams));
}

static uint16_t linphone_call_get_avpf_rr_interval(const LinphoneCall *call) {
	uint16_t rr_interval = 0;
	uint16_t stream_rr_interval;
	if (call) {
		if (call->audiostream && media_stream_get_state((MediaStream *)call->audiostream) == MSStreamStarted) {
			stream_rr_interval = media_stream_get_avpf_rr_interval((MediaStream *)call->audiostream);
			if (stream_rr_interval > rr_interval) rr_interval = stream_rr_interval;
		}
		if (call->videostream && media_stream_get_state((MediaStream *)call->videostream) == MSStreamStarted) {
			stream_rr_interval = media_stream_get_avpf_rr_interval((MediaStream *)call->videostream);
			if (stream_rr_interval > rr_interval) rr_interval = stream_rr_interval;
		}
	} else {
		rr_interval = 5000;
	}
	return rr_interval;
}

static void propagate_encryption_changed(LinphoneCall *call){
	if (!linphone_call_all_streams_encrypted(call)) {
		ms_message("Some streams are not encrypted");
		call->current_params->media_encryption=LinphoneMediaEncryptionNone;
		linphone_core_notify_call_encryption_changed(call->core, call, FALSE, call->auth_token);
	} else {
		if (call->auth_token) {/* ZRTP only is using auth_token */
			call->current_params->media_encryption=LinphoneMediaEncryptionZRTP;
		} else { /* otherwise it must be DTLS as SDES doesn't go through this function */
			call->current_params->media_encryption=LinphoneMediaEncryptionDTLS;
		}
		ms_message("All streams are encrypted key exchanged using %s", call->current_params->media_encryption==LinphoneMediaEncryptionZRTP?"ZRTP":call->current_params->media_encryption==LinphoneMediaEncryptionDTLS?"DTLS":"Unknown mechanism");
		linphone_core_notify_call_encryption_changed(call->core, call, TRUE, call->auth_token);
	}
}

static void linphone_call_audiostream_encryption_changed(void *data, bool_t encrypted) {
	char status[255]={0};
	LinphoneCall *call;

	call = (LinphoneCall *)data;

	if (encrypted) {
		if (call->params->media_encryption==LinphoneMediaEncryptionZRTP) { /* if encryption is DTLS, no status to be displayed */
			snprintf(status,sizeof(status)-1,_("Authentication token is %s"),call->auth_token);
			linphone_core_notify_display_status(call->core, status);
		}
	}

	propagate_encryption_changed(call);


#ifdef VIDEO_ENABLED
	// Enable video encryption
	{
		const LinphoneCallParams *params=linphone_call_get_current_params(call);
		if (params->has_video) {
			MSZrtpParams params;
			ms_message("Trying to enable encryption on video stream");
			params.zid_file=NULL; //unused
			video_stream_enable_zrtp(call->videostream,call->audiostream,&params);
		}
	}
#endif
}


static void linphone_call_audiostream_auth_token_ready(void *data, const char* auth_token, bool_t verified) {
	LinphoneCall *call=(LinphoneCall *)data;
	if (call->auth_token != NULL)
		ms_free(call->auth_token);

	call->auth_token=ms_strdup(auth_token);
	call->auth_token_verified=verified;

	ms_message("Authentication token is %s (%s)", auth_token, verified?"verified":"unverified");
}

/**
 * Set the result of ZRTP short code verification by user.
 * If remote party also does the same, it will update the ZRTP cache so that user's verification will not be required for the two users.
 * @param call the LinphoneCall
 * @param verified whether the ZRTP SAS is verified.
 * @ingroup call_control
**/
void linphone_call_set_authentication_token_verified(LinphoneCall *call, bool_t verified){
	if (call->audiostream==NULL){
		ms_error("linphone_call_set_authentication_token_verified(): No audio stream");
	}
	if (call->audiostream->ms.sessions.zrtp_context==NULL){
		ms_error("linphone_call_set_authentication_token_verified(): No zrtp context.");
	}
	if (!call->auth_token_verified && verified){
		ms_zrtp_sas_verified(call->audiostream->ms.sessions.zrtp_context);
	}else if (call->auth_token_verified && !verified){
		ms_zrtp_sas_reset_verified(call->audiostream->ms.sessions.zrtp_context);
	}
	call->auth_token_verified=verified;
	propagate_encryption_changed(call);
}

static int get_max_codec_sample_rate(const MSList *codecs){
	int max_sample_rate=0;
	const MSList *it;
	for(it=codecs;it!=NULL;it=it->next){
		PayloadType *pt=(PayloadType*)it->data;
		int sample_rate;

		if( strcasecmp("G722",pt->mime_type) == 0 ){
			/* G722 spec says 8000 but the codec actually requires 16000 */
			sample_rate = 16000;
		}else sample_rate=pt->clock_rate;
		if (sample_rate>max_sample_rate) max_sample_rate=sample_rate;
	}
	return max_sample_rate;
}

static int find_payload_type_number(const MSList *assigned, const PayloadType *pt){
	const MSList *elem;
	const PayloadType *candidate=NULL;
	for(elem=assigned;elem!=NULL;elem=elem->next){
		const PayloadType *it=(const PayloadType*)elem->data;
		if ((strcasecmp(pt->mime_type, payload_type_get_mime(it)) == 0)
			&& (it->clock_rate==pt->clock_rate)
			&& (it->channels==pt->channels || pt->channels<=0)) {
			candidate=it;
			if ((it->recv_fmtp!=NULL && pt->recv_fmtp!=NULL && strcasecmp(it->recv_fmtp, pt->recv_fmtp)==0)
				|| (it->recv_fmtp==NULL && pt->recv_fmtp==NULL)){
				break;/*exact match*/
			}
		}
	}
	return candidate ? payload_type_get_number(candidate) : -1;
}

bool_t is_payload_type_number_available(const MSList *l, int number, const PayloadType *ignore){
	const MSList *elem;
	for (elem=l; elem!=NULL; elem=elem->next){
		const PayloadType *pt=(PayloadType*)elem->data;
		if (pt!=ignore && payload_type_get_number(pt)==number) return FALSE;
	}
	return TRUE;
}

static void linphone_core_assign_payload_type_numbers(LinphoneCore *lc, MSList *codecs){
	MSList *elem;
	int dyn_number=lc->codecs_conf.dyn_pt;
	for (elem=codecs; elem!=NULL; elem=elem->next){
		PayloadType *pt=(PayloadType*)elem->data;
		int number=payload_type_get_number(pt);

		/*check if number is duplicated: it could be the case if the remote forced us to use a mapping with a previous offer*/
		if (number!=-1 && !(pt->flags & PAYLOAD_TYPE_FROZEN_NUMBER)){
			if (!is_payload_type_number_available(codecs, number, pt)){
				ms_message("Reassigning payload type %i %s/%i because already offered.", number, pt->mime_type, pt->clock_rate);
				number=-1; /*need to be re-assigned*/
			}
		}

		if (number==-1){
			while(dyn_number<127){
				if (is_payload_type_number_available(codecs, dyn_number, NULL)){
					payload_type_set_number(pt, dyn_number);
					dyn_number++;
					break;
				}
				dyn_number++;
			}
			if (dyn_number==127){
				ms_error("Too many payload types configured ! codec %s/%i is disabled.", pt->mime_type, pt->clock_rate);
				payload_type_set_enable(pt, FALSE);
			}
		}
	}
}

static bool_t has_telephone_event_at_rate(const MSList *tev, int rate){
	const MSList *it;
	for(it=tev;it!=NULL;it=it->next){
		const PayloadType *pt=(PayloadType*)it->data;
		if (pt->clock_rate==rate) return TRUE;
	}
	return FALSE;
}

static MSList * create_telephone_events(LinphoneCore *lc, const MSList *codecs){
	const MSList *it;
	MSList *ret=NULL;
	for(it=codecs;it!=NULL;it=it->next){
		const PayloadType *pt=(PayloadType*)it->data;
		if (!has_telephone_event_at_rate(ret,pt->clock_rate)){
			PayloadType *tev=payload_type_clone(&payload_type_telephone_event);
			tev->clock_rate=pt->clock_rate;
			/*let it choose the number dynamically as for normal codecs*/
			payload_type_set_number(tev, -1);
			if (ret==NULL){
				/*But for first telephone-event, prefer the number that was configured in the core*/
				if (is_payload_type_number_available(codecs, lc->codecs_conf.telephone_event_pt, NULL)){
					payload_type_set_number(tev, lc->codecs_conf.telephone_event_pt);
				}
			}
			ret=ms_list_append(ret,tev);
		}
	}
	return ret;
}

static MSList *create_special_payload_types(LinphoneCore *lc, const MSList *codecs){
	MSList *ret=create_telephone_events(lc, codecs);
	if (linphone_core_generic_confort_noise_enabled(lc)){
		PayloadType *cn=payload_type_clone(&payload_type_cn);
		payload_type_set_number(cn, 13);
		ret=ms_list_append(ret, cn);
	}
	return ret;
}

typedef struct _CodecConstraints{
	int bandwidth_limit;
	int max_codecs;
	MSList *previously_used;
}CodecConstraints;

static MSList *make_codec_list(LinphoneCore *lc, CodecConstraints * hints, SalStreamType stype, const MSList *codecs){
	MSList *l=NULL;
	const MSList *it;
	int nb = 0;

	for(it=codecs;it!=NULL;it=it->next){
		PayloadType *pt=(PayloadType*)it->data;
		int num;

		if (!(pt->flags & PAYLOAD_TYPE_ENABLED))
			continue;
		if (hints->bandwidth_limit>0 && !linphone_core_is_payload_type_usable_for_bandwidth(lc,pt,hints->bandwidth_limit)){
			ms_message("Codec %s/%i eliminated because of audio bandwidth constraint of %i kbit/s",
					pt->mime_type,pt->clock_rate,hints->bandwidth_limit);
			continue;
		}
		if (!linphone_core_check_payload_type_usability(lc,pt)){
			continue;
		}
		pt=payload_type_clone(pt);

		/*look for a previously assigned number for this codec*/
		num=find_payload_type_number(hints->previously_used, pt);
		if (num!=-1){
			payload_type_set_number(pt,num);
			payload_type_set_flag(pt, PAYLOAD_TYPE_FROZEN_NUMBER);
		}

		l=ms_list_append(l, pt);
		nb++;
		if ((hints->max_codecs > 0) && (nb >= hints->max_codecs)) break;
	}
	if (stype==SalAudio){
		MSList *specials=create_special_payload_types(lc,l);
		l=ms_list_concat(l,specials);
	}
	linphone_core_assign_payload_type_numbers(lc, l);
	return l;
}

static void update_media_description_from_stun(SalMediaDescription *md, const StunCandidate *ac, const StunCandidate *vc){
	int i;
	for (i = 0; i < md->nb_streams; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if ((md->streams[i].type == SalAudio) && (ac->port != 0)) {
			strcpy(md->streams[0].rtp_addr,ac->addr);
			md->streams[0].rtp_port=ac->port;
			if ((ac->addr[0]!='\0' && vc->addr[0]!='\0' && strcmp(ac->addr,vc->addr)==0) || sal_media_description_get_nb_active_streams(md)==1){
				strcpy(md->addr,ac->addr);
			}
		}
		if ((md->streams[i].type == SalVideo) && (vc->port != 0)) {
			strcpy(md->streams[1].rtp_addr,vc->addr);
			md->streams[1].rtp_port=vc->port;
		}
	}
}

static int setup_encryption_key(SalSrtpCryptoAlgo *crypto, MSCryptoSuite suite, unsigned int tag){
	int keylen=0;
	crypto->tag=tag;
	crypto->algo=suite;
	switch(suite){
		case MS_AES_128_SHA1_80:
		case MS_AES_128_SHA1_32:
		case MS_AES_128_NO_AUTH:
		case MS_NO_CIPHER_SHA1_80: /*not sure for this one*/
			keylen=30;
		break;
		case MS_AES_256_SHA1_80:
		case MS_AES_256_SHA1_32:
			keylen=46;
		break;
		case MS_CRYPTO_SUITE_INVALID:
		break;
	}
	if (keylen==0 || !generate_b64_crypto_key(30, crypto->master_key, SAL_SRTP_KEY_SIZE)){
		ms_error("Could not generate SRTP key.");
		crypto->algo = 0;
		return -1;
	}
	return 0;
}
static void setup_dtls_keys(LinphoneCall *call, SalMediaDescription *md){
	int i;
	for(i=0; i<md->nb_streams; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		/* if media encryption is set to DTLS check presence of fingerprint in the call which shall have been set at stream init but it may have failed when retrieving certificate resulting in no fingerprint present and then DTLS not usable */
		if (sal_stream_description_has_dtls(&md->streams[i]) == TRUE) {
			strncpy(md->streams[i].dtls_fingerprint, call->dtls_certificate_fingerprint, sizeof(md->streams[i].dtls_fingerprint)); /* get the self fingerprint from call(it's computed at stream init) */
			md->streams[i].dtls_role = SalDtlsRoleUnset; /* if we are offering, SDP will have actpass setup attribute when role is unset, if we are responding the result mediadescription will be set to SalDtlsRoleIsClient */
		} else {
			md->streams[i].dtls_fingerprint[0] = '\0';
			md->streams[i].dtls_role = SalDtlsRoleInvalid;

		}
	}

}
static void setup_encryption_keys(LinphoneCall *call, SalMediaDescription *md){
	LinphoneCore *lc=call->core;
	int i,j;
	SalMediaDescription *old_md=call->localdesc;
	bool_t keep_srtp_keys=lp_config_get_int(lc->config,"sip","keep_srtp_keys",1);

	for(i=0; i<md->nb_streams; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		if (sal_stream_description_has_srtp(&md->streams[i]) == TRUE) {
			if (keep_srtp_keys && old_md && (sal_stream_description_active(&old_md->streams[i]) == TRUE) && (sal_stream_description_has_srtp(&old_md->streams[i]) == TRUE)) {
				int j;
				ms_message("Keeping same crypto keys.");
				for(j=0;j<SAL_CRYPTO_ALGO_MAX;++j){
					memcpy(&md->streams[i].crypto[j],&old_md->streams[i].crypto[j],sizeof(SalSrtpCryptoAlgo));
				}
			}else{
				const MSCryptoSuite *suites=linphone_core_get_srtp_crypto_suites(lc);
				for(j=0;suites!=NULL && suites[j]!=MS_CRYPTO_SUITE_INVALID && j<SAL_CRYPTO_ALGO_MAX;++j){
					setup_encryption_key(&md->streams[i].crypto[j],suites[j],j+1);
				}
			}
		}
	}
}

static void setup_rtcp_fb(LinphoneCall *call, SalMediaDescription *md) {
	MSList *pt_it;
	PayloadType *pt;
	PayloadTypeAvpfParams avpf_params;
	LinphoneCore *lc = call->core;
	int i;

	for (i = 0; i < md->nb_streams; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		md->streams[i].rtcp_fb.generic_nack_enabled = lp_config_get_int(lc->config, "rtp", "rtcp_fb_generic_nack_enabled", 0);
		md->streams[i].rtcp_fb.tmmbr_enabled = lp_config_get_int(lc->config, "rtp", "rtcp_fb_tmmbr_enabled", 0);
		for (pt_it = md->streams[i].payloads; pt_it != NULL; pt_it = pt_it->next) {
			pt = (PayloadType *)pt_it->data;
			if (call->params->avpf_enabled == TRUE) {
				payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				avpf_params = payload_type_get_avpf_params(pt);
				avpf_params.trr_interval = call->params->avpf_rr_interval;
			} else {
				payload_type_unset_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				memset(&avpf_params, 0, sizeof(avpf_params));
			}
			payload_type_set_avpf_params(pt, avpf_params);
		}
	}
}

static void setup_rtcp_xr(LinphoneCall *call, SalMediaDescription *md) {
	LinphoneCore *lc = call->core;
	int i;

	md->rtcp_xr.enabled = lp_config_get_int(lc->config, "rtp", "rtcp_xr_enabled", 1);
	if (md->rtcp_xr.enabled == TRUE) {
		const char *rcvr_rtt_mode = lp_config_get_string(lc->config, "rtp", "rtcp_xr_rcvr_rtt_mode", "all");
		if (strcasecmp(rcvr_rtt_mode, "all") == 0) md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttAll;
		else if (strcasecmp(rcvr_rtt_mode, "sender") == 0) md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttSender;
		else md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttNone;
		if (md->rtcp_xr.rcvr_rtt_mode != OrtpRtcpXrRcvrRttNone) {
			md->rtcp_xr.rcvr_rtt_max_size = lp_config_get_int(lc->config, "rtp", "rtcp_xr_rcvr_rtt_max_size", 10000);
		}
		md->rtcp_xr.stat_summary_enabled = lp_config_get_int(lc->config, "rtp", "rtcp_xr_stat_summary_enabled", 1);
		if (md->rtcp_xr.stat_summary_enabled == TRUE) {
			md->rtcp_xr.stat_summary_flags = OrtpRtcpXrStatSummaryLoss | OrtpRtcpXrStatSummaryDup | OrtpRtcpXrStatSummaryJitt | OrtpRtcpXrStatSummaryTTL;
		}
		md->rtcp_xr.voip_metrics_enabled = lp_config_get_int(lc->config, "rtp", "rtcp_xr_voip_metrics_enabled", 1);
	}
	for (i = 0; i < md->nb_streams; i++) {
		if (!sal_stream_description_active(&md->streams[i])) continue;
		memcpy(&md->streams[i].rtcp_xr, &md->rtcp_xr, sizeof(md->streams[i].rtcp_xr));
	}
}

void linphone_call_increment_local_media_description(LinphoneCall *call){
	SalMediaDescription *md=call->localdesc;
	md->session_ver++;
}

void linphone_call_update_local_media_description_from_ice_or_upnp(LinphoneCall *call){
	if (call->ice_session != NULL) {
		_update_local_media_description_from_ice(call->localdesc, call->ice_session);
		linphone_core_update_ice_state_in_call_stats(call);
	}
#ifdef BUILD_UPNP
	if(call->upnp_session != NULL) {
		linphone_core_update_local_media_description_from_upnp(call->localdesc, call->upnp_session);
		linphone_core_update_upnp_state_in_call_stats(call);
	}
#endif  //BUILD_UPNP
}

static void transfer_already_assigned_payload_types(SalMediaDescription *old, SalMediaDescription *md){
	int i;
	for(i=0;i<old->nb_streams;++i){
		md->streams[i].already_assigned_payloads=old->streams[i].already_assigned_payloads;
		old->streams[i].already_assigned_payloads=NULL;
	}
}

static const char *linphone_call_get_bind_ip_for_stream(LinphoneCall *call, int stream_index){
	const char *bind_ip = lp_config_get_string(call->core->config,"rtp","bind_address",call->af==AF_INET6 ? "::0" : "0.0.0.0");

	if (stream_index<2 && call->media_ports[stream_index].multicast_ip[0]!='\0'){
		if (call->dir==LinphoneCallOutgoing){
			/*as multicast sender, we must decide a local interface to use to send multicast, and bind to it*/
			bind_ip=call->media_localip;
		}
	}
	return bind_ip;
}

static const char *linphone_call_get_public_ip_for_stream(LinphoneCall *call, int stream_index){
	const char *public_ip=call->media_localip;

	if (stream_index<2 && call->media_ports[stream_index].multicast_ip[0]!='\0')
		public_ip=call->media_ports[stream_index].multicast_ip;
	return public_ip;
}

void linphone_call_make_local_media_description(LinphoneCore *lc, LinphoneCall *call) {
	linphone_call_make_local_media_description_with_params(lc, call, call->params);
}

void linphone_call_make_local_media_description_with_params(LinphoneCore *lc, LinphoneCall *call, LinphoneCallParams *params) {
	MSList *l;
	SalMediaDescription *old_md=call->localdesc;
	int i;
	int nb_active_streams = 0;
	SalMediaDescription *md=sal_media_description_new();
	LinphoneAddress *addr;
	const char *subject;
	CodecConstraints codec_hints={0};

	/*multicast is only set in case of outgoing call*/
	if (call->dir == LinphoneCallOutgoing && linphone_call_params_audio_multicast_enabled(params)) {
		md->streams[0].ttl=linphone_core_get_audio_multicast_ttl(lc);
		md->streams[0].multicast_role = SalMulticastSender;
	}

	if (call->dir == LinphoneCallOutgoing && linphone_call_params_video_multicast_enabled(params)) {
		md->streams[1].ttl=linphone_core_get_video_multicast_ttl(lc);
		md->streams[1].multicast_role = SalMulticastSender;
	}

	subject=linphone_call_params_get_session_name(params);

	linphone_core_adapt_to_network(lc,call->ping_time,params);

	if (call->dest_proxy) {
		addr=linphone_address_clone(linphone_proxy_config_get_identity_address(call->dest_proxy));
	} else {
		addr=linphone_address_new(linphone_core_get_identity(lc));
	}

	md->session_id=(old_md ? old_md->session_id : (rand() & 0xfff));
	md->session_ver=(old_md ? (old_md->session_ver+1) : (rand() & 0xfff));
	md->nb_streams=(call->biggestdesc ? call->biggestdesc->nb_streams : 1);

	/*re-check local ip address each time we make a new offer, because it may change in case of network reconnection*/
	linphone_call_get_local_ip(call, call->dir == LinphoneCallOutgoing ?  call->log->to : call->log->from);
	strncpy(md->addr,call->media_localip,sizeof(md->addr));
	if (linphone_address_get_username(addr)) /*might be null in case of identity without userinfo*/
		strncpy(md->username,linphone_address_get_username(addr),sizeof(md->username));
	if (subject) strncpy(md->name,subject,sizeof(md->name));

	if (params->down_bw)
		md->bandwidth=params->down_bw;
	else md->bandwidth=linphone_core_get_download_bandwidth(lc);

	/*set audio capabilities */
	strncpy(md->streams[0].rtp_addr,linphone_call_get_public_ip_for_stream(call,0),sizeof(md->streams[0].rtp_addr));
	strncpy(md->streams[0].rtcp_addr,linphone_call_get_public_ip_for_stream(call,0),sizeof(md->streams[0].rtcp_addr));
	strncpy(md->streams[0].name,"Audio",sizeof(md->streams[0].name)-1);
	md->streams[0].rtp_port=call->media_ports[0].rtp_port;
	md->streams[0].rtcp_port=call->media_ports[0].rtcp_port;
	md->streams[0].proto=get_proto_from_call_params(params);
	md->streams[0].dir=get_audio_dir_from_call_params(params);
	md->streams[0].type=SalAudio;
	if (params->down_ptime)
		md->streams[0].ptime=params->down_ptime;
	else
		md->streams[0].ptime=linphone_core_get_download_ptime(lc);
	codec_hints.bandwidth_limit=params->audio_bw;
	codec_hints.max_codecs=-1;
	codec_hints.previously_used=old_md ? old_md->streams[0].already_assigned_payloads : NULL;
	l=make_codec_list(lc, &codec_hints, SalAudio, lc->codecs_conf.audio_codecs);
	md->streams[0].max_rate=get_max_codec_sample_rate(l);
	md->streams[0].payloads=l;
	if (call->audiostream && call->audiostream->ms.sessions.rtp_session) {
		char* me = linphone_address_as_string_uri_only(call->me);
		md->streams[0].rtp_ssrc=rtp_session_get_send_ssrc(call->audiostream->ms.sessions.rtp_session);
		strncpy(md->streams[0].rtcp_cname,me,sizeof(md->streams[0].rtcp_cname));
		ms_free(me);
	}
	else
		ms_warning("Cannot get audio local ssrc for call [%p]",call);
	nb_active_streams++;

	if (params->has_video && (!params->internal_call_update || !call->current_params->video_declined)){
		strncpy(md->streams[1].rtp_addr,linphone_call_get_public_ip_for_stream(call,1),sizeof(md->streams[1].rtp_addr));
		strncpy(md->streams[1].rtcp_addr,linphone_call_get_public_ip_for_stream(call,1),sizeof(md->streams[1].rtcp_addr));
		strncpy(md->streams[1].name,"Video",sizeof(md->streams[1].name)-1);
		md->streams[1].rtp_port=call->media_ports[1].rtp_port;
		md->streams[1].rtcp_port=call->media_ports[1].rtcp_port;
		md->streams[1].proto=md->streams[0].proto;
		md->streams[1].dir=get_video_dir_from_call_params(params);
		md->streams[1].type=SalVideo;
		codec_hints.bandwidth_limit=0;
		codec_hints.max_codecs=-1;
		codec_hints.previously_used=old_md ? old_md->streams[1].already_assigned_payloads : NULL;
		l=make_codec_list(lc, &codec_hints, SalVideo, lc->codecs_conf.video_codecs);
		md->streams[1].payloads=l;
		if (call->videostream && call->videostream->ms.sessions.rtp_session) {
			char* me = linphone_address_as_string_uri_only(call->me);
			md->streams[1].rtp_ssrc=rtp_session_get_send_ssrc(call->videostream->ms.sessions.rtp_session);
			strncpy(md->streams[1].rtcp_cname,me,sizeof(md->streams[1].rtcp_cname));
			ms_free(me);
		}
		else
			ms_warning("Cannot get video local ssrc for call [%p]",call);
		nb_active_streams++;
	} else {
		ms_message("Don't put video stream on local offer for call [%p]",call);
	}

	if (md->nb_streams < nb_active_streams)
		md->nb_streams = nb_active_streams;

	/* Deactivate inactive streams. */
	for (i = nb_active_streams; i < md->nb_streams; i++) {
		md->streams[i].rtp_port = 0;
		md->streams[i].rtcp_port = 0;
		md->streams[i].proto = call->biggestdesc->streams[i].proto;
		md->streams[i].type = call->biggestdesc->streams[i].type;
		md->streams[i].dir = SalStreamInactive;
		codec_hints.bandwidth_limit=0;
		codec_hints.max_codecs=1;
		codec_hints.previously_used=NULL;
		l = make_codec_list(lc, &codec_hints, SalVideo, lc->codecs_conf.video_codecs);
		md->streams[i].payloads = l;
	}
	setup_encryption_keys(call,md);
	setup_dtls_keys(call,md);
	setup_rtcp_fb(call, md);
	setup_rtcp_xr(call, md);

	update_media_description_from_stun(md,&call->ac,&call->vc);
	call->localdesc=md;
	linphone_call_update_local_media_description_from_ice_or_upnp(call);
	linphone_address_destroy(addr);
	if (old_md){
		transfer_already_assigned_payload_types(old_md,md);
		call->localdesc_changed=sal_media_description_equals(md,old_md);
		sal_media_description_unref(old_md);
	}
}

static int find_port_offset(LinphoneCore *lc, int stream_index, int base_port){
	int offset;
	MSList *elem;
	int tried_port;
	int existing_port;
	bool_t already_used=FALSE;

	for(offset=0;offset<100;offset+=2){
		tried_port=base_port+offset;
		already_used=FALSE;
		for(elem=lc->calls;elem!=NULL;elem=elem->next){
			LinphoneCall *call=(LinphoneCall*)elem->data;
			existing_port=call->media_ports[stream_index].rtp_port;
			if (existing_port==tried_port) {
				already_used=TRUE;
				break;
			}
		}
		if (!already_used) break;
	}
	if (offset==100){
		ms_error("Could not find any free port !");
		return -1;
	}
	return offset;
}

static int select_random_port(LinphoneCore *lc, int stream_index, int min_port, int max_port) {
	MSList *elem;
	int nb_tries;
	int tried_port = 0;
	int existing_port = 0;
	bool_t already_used = FALSE;

	tried_port = (ortp_random() % (max_port - min_port) + min_port) & ~0x1;
	if (tried_port < min_port) tried_port = min_port + 2;
	for (nb_tries = 0; nb_tries < 100; nb_tries++) {
		for (elem = lc->calls; elem != NULL; elem = elem->next) {
			LinphoneCall *call = (LinphoneCall *)elem->data;
			existing_port=call->media_ports[stream_index].rtp_port;
			if (existing_port == tried_port) {
				already_used = TRUE;
				break;
			}
		}
		if (!already_used) break;
	}
	if (nb_tries == 100) {
		ms_error("Could not find any free port!");
		return -1;
	}
	return tried_port;
}

static void port_config_set_random(LinphoneCall *call, int stream_index){
	call->media_ports[stream_index].rtp_port=-1;
	call->media_ports[stream_index].rtcp_port=-1;
}

static void port_config_set(LinphoneCall *call, int stream_index, int min_port, int max_port){
	int port_offset;
	if (min_port>0 && max_port>0){
		if (min_port == max_port) {
			/* Used fixed RTP audio port. */
			port_offset=find_port_offset(call->core, stream_index, min_port);
			if (port_offset==-1) {
				port_config_set_random(call, stream_index);
				return;
			}
			call->media_ports[stream_index].rtp_port=min_port+port_offset;
		} else {
			/* Select random RTP audio port in the specified range. */
			call->media_ports[stream_index].rtp_port = select_random_port(call->core, stream_index, min_port, max_port);
		}
		call->media_ports[stream_index].rtcp_port=call->media_ports[stream_index].rtp_port+1;
	}else port_config_set_random(call,stream_index);
}

static void linphone_call_init_common(LinphoneCall *call, LinphoneAddress *from, LinphoneAddress *to){
	int min_port, max_port;
	ms_message("New LinphoneCall [%p] initialized (LinphoneCore version: %s)",call,linphone_core_get_version());
	call->state=LinphoneCallIdle;
	call->transfer_state = LinphoneCallIdle;
	call->log=linphone_call_log_new(call->dir, from, to);
	call->camera_enabled=TRUE;
	call->current_params = linphone_call_params_new();
	call->current_params->media_encryption=LinphoneMediaEncryptionNone;
	call->dtls_certificate_fingerprint = NULL;
	if (call->dir == LinphoneCallIncoming)
		call->me=to;
	 else
		call->me=from;
	linphone_address_ref(call->me);

	linphone_core_get_audio_port_range(call->core, &min_port, &max_port);
	port_config_set(call,0,min_port,max_port);

	linphone_core_get_video_port_range(call->core, &min_port, &max_port);
	port_config_set(call,1,min_port,max_port);

	linphone_call_init_stats(&call->stats[LINPHONE_CALL_STATS_AUDIO], LINPHONE_CALL_STATS_AUDIO);
	linphone_call_init_stats(&call->stats[LINPHONE_CALL_STATS_VIDEO], LINPHONE_CALL_STATS_VIDEO);
#ifdef VIDEO_ENABLED
	call->cam = call->core->video_conf.device;
#endif
}

void linphone_call_init_stats(LinphoneCallStats *stats, int type) {
	stats->type = type;
	stats->received_rtcp = NULL;
	stats->sent_rtcp = NULL;
	stats->ice_state = LinphoneIceStateNotActivated;
#ifdef BUILD_UPNP
	stats->upnp_state = LinphoneUpnpStateIdle;
#else
	stats->upnp_state = LinphoneUpnpStateNotAvailable;
#endif //BUILD_UPNP
}


static void discover_mtu(LinphoneCore *lc, const char *remote){
	int mtu;
	if (lc->net_conf.mtu==0	){
		/*attempt to discover mtu*/
		mtu=ms_discover_mtu(remote);
		if (mtu>0){
			ms_set_mtu(mtu);
			ms_message("Discovered mtu is %i, RTP payload max size is %i",
				mtu, ms_get_payload_max_size());
		}
	}
}

void linphone_call_create_op(LinphoneCall *call){
	if (call->op) sal_op_release(call->op);
	call->op=sal_op_new(call->core->sal);
	sal_op_set_user_pointer(call->op,call);
	if (call->params->referer)
		sal_call_set_referer(call->op,call->params->referer->op);
	linphone_configure_op(call->core,call->op,call->log->to,call->params->custom_headers,FALSE);
	if (call->params->privacy != LinphonePrivacyDefault)
		sal_op_set_privacy(call->op,(SalPrivacyMask)call->params->privacy);
	/*else privacy might be set by proxy */
}

/*
 * Choose IP version we are going to use for RTP socket.
 * The algorithm is as follows:
 * - if ipv6 is disabled at the core level, it is always AF_INET
 * - Otherwise, if the destination address for the call is an IPv6 address, use IPv6.
 * - Otherwise, if the call is done through a known proxy config, then use the information obtained during REGISTER
 * to know if IPv6 is supported by the server.
**/
static void linphone_call_outgoing_select_ip_version(LinphoneCall *call, LinphoneAddress *to, LinphoneProxyConfig *cfg){
	if (linphone_core_ipv6_enabled(call->core)){
		call->af=AF_INET;
		if (sal_address_is_ipv6((SalAddress*)to)){
			call->af=AF_INET6;
		}else if (cfg && cfg->op){
			call->af=sal_op_is_ipv6(cfg->op) ? AF_INET6 : AF_INET;
		}
	}else call->af=AF_INET;
}

/**
 * Fill the local ip that routes to the internet according to the destination, or guess it by other special means (upnp).
 */
static void linphone_call_get_local_ip(LinphoneCall *call, const LinphoneAddress *remote_addr){
	const char *ip;
	int af = call->af;
	const char *dest = NULL;
	if (call->dest_proxy == NULL) {
		struct addrinfo hints;
		struct addrinfo *res = NULL;
		int err;
		const char *domain = linphone_address_get_domain(remote_addr);
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_NUMERICHOST;
		err = getaddrinfo(domain, NULL, &hints, &res);
		if (err == 0) {
			dest = domain;
		}
		if (res != NULL) freeaddrinfo(res);
	}
	if (linphone_core_get_firewall_policy(call->core)==LinphonePolicyUseNatAddress
		&& (ip=linphone_core_get_nat_address_resolved(call->core))!=NULL){
		strncpy(call->media_localip,ip,LINPHONE_IPADDR_SIZE);
		return;
	}
#ifdef BUILD_UPNP
	else if (call->core->upnp != NULL && linphone_core_get_firewall_policy(call->core)==LinphonePolicyUseUpnp &&
			linphone_upnp_context_get_state(call->core->upnp) == LinphoneUpnpStateOk) {
		ip = linphone_upnp_context_get_external_ipaddress(call->core->upnp);
		strncpy(call->media_localip,ip,LINPHONE_IPADDR_SIZE);
		return;
	}
#endif //BUILD_UPNP
	/*first nominal use case*/
	linphone_core_get_local_ip(call->core, af, dest, call->media_localip);

	/*next, sometime, override from config*/
	if ((ip=lp_config_get_string(call->core->config,"rtp","bind_address",NULL)))
		strncpy(call->media_localip,ip,LINPHONE_IPADDR_SIZE);

	return;
}

static void linphone_call_destroy(LinphoneCall *obj);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneCall);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneCall, belle_sip_object_t,
	(belle_sip_object_destroy_t)linphone_call_destroy,
	NULL, // clone
	NULL, // marshal
	FALSE
);
void linphone_call_fill_media_multicast_addr(LinphoneCall *call) {
	if (linphone_call_params_audio_multicast_enabled(call->params)){
		strncpy(call->media_ports[0].multicast_ip,
				linphone_core_get_audio_multicast_addr(call->core), sizeof(call->media_ports[0].multicast_ip));
	} else
		call->media_ports[0].multicast_ip[0]='\0';

	if (linphone_call_params_video_multicast_enabled(call->params)){
		strncpy(call->media_ports[1].multicast_ip,
				linphone_core_get_video_multicast_addr(call->core), sizeof(call->media_ports[1].multicast_ip));
	} else
		call->media_ports[1].multicast_ip[0]='\0';
}
LinphoneCall * linphone_call_new_outgoing(struct _LinphoneCore *lc, LinphoneAddress *from, LinphoneAddress *to, const LinphoneCallParams *params, LinphoneProxyConfig *cfg){
	LinphoneCall *call = belle_sip_object_new(LinphoneCall);

	call->dir=LinphoneCallOutgoing;
	call->core=lc;
	linphone_call_outgoing_select_ip_version(call,to,cfg);
	linphone_call_get_local_ip(call, to);
	linphone_call_init_common(call,from,to);
	call->params = linphone_call_params_copy(params);

	linphone_call_fill_media_multicast_addr(call);

	if (linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseIce) {
		call->ice_session = ice_session_new();
		/*for backward compatibility purposes, shall be enabled by default in futur*/
		ice_session_enable_message_integrity_check(call->ice_session,lp_config_get_int(lc->config,"net","ice_session_enable_message_integrity_check",0));
		ice_session_set_role(call->ice_session, IR_Controlling);
	}
	if (linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseStun) {
		call->ping_time=linphone_core_run_stun_tests(call->core,call);
	}
#ifdef BUILD_UPNP
	if (linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseUpnp) {
		if(!lc->rtp_conf.disable_upnp) {
			call->upnp_session = linphone_upnp_session_new(call);
		}
	}
#endif //BUILD_UPNP

	discover_mtu(lc,linphone_address_get_domain (to));
	if (params->referer){
		call->referer=linphone_call_ref(params->referer);
	}
	call->dest_proxy=cfg;
	linphone_call_create_op(call);
	return call;
}

static void linphone_call_incoming_select_ip_version(LinphoneCall *call){
	if (linphone_core_ipv6_enabled(call->core)){
		call->af=sal_op_is_ipv6(call->op) ? AF_INET6 : AF_INET;
	}else call->af=AF_INET;
}

/**
 * Fix call parameters on incoming call to eg. enable AVPF if the incoming call propose it and it is not enabled locally.
 */
void linphone_call_set_compatible_incoming_call_parameters(LinphoneCall *call, const SalMediaDescription *md) {
	/* Handle AVPF, SRTP and DTLS. */
	call->params->avpf_enabled = sal_media_description_has_avpf(md);
	if (call->params->avpf_enabled == TRUE) {
		if (call->dest_proxy != NULL) {
			call->params->avpf_rr_interval = linphone_proxy_config_get_avpf_rr_interval(call->dest_proxy) * 1000;
		} else {
			call->params->avpf_rr_interval = linphone_core_get_avpf_rr_interval(call->core)*1000;
		}
	}
	if ((sal_media_description_has_dtls(md) == TRUE) && (media_stream_dtls_supported() == TRUE)) {
		call->params->media_encryption = LinphoneMediaEncryptionDTLS;
	}else if ((sal_media_description_has_srtp(md) == TRUE) && (ms_srtp_supported() == TRUE)) {
		call->params->media_encryption = LinphoneMediaEncryptionSRTP;
	}else if (call->params->media_encryption != LinphoneMediaEncryptionZRTP){
		call->params->media_encryption = LinphoneMediaEncryptionNone;
	}

}

LinphoneCall * linphone_call_new_incoming(LinphoneCore *lc, LinphoneAddress *from, LinphoneAddress *to, SalOp *op){
	LinphoneCall *call = belle_sip_object_new(LinphoneCall);
	SalMediaDescription *md;
	LinphoneFirewallPolicy fpol;
	int i;

	call->dir=LinphoneCallIncoming;
	sal_op_set_user_pointer(op,call);
	call->op=op;
	call->core=lc;

	linphone_call_incoming_select_ip_version(call);

	sal_op_cnx_ip_to_0000_if_sendonly_enable(op,lp_config_get_default_int(lc->config,"sip","cnx_ip_to_0000_if_sendonly_enabled",0));

	if (lc->sip_conf.ping_with_options){
#ifdef BUILD_UPNP
		if (lc->upnp != NULL && linphone_core_get_firewall_policy(lc)==LinphonePolicyUseUpnp &&
			linphone_upnp_context_get_state(lc->upnp) == LinphoneUpnpStateOk) {
#else //BUILD_UPNP
		{
#endif //BUILD_UPNP
			/*the following sends an option request back to the caller so that
			 we get a chance to discover our nat'd address before answering.*/
			call->ping_op=sal_op_new(lc->sal);

			linphone_configure_op(lc, call->ping_op, from, NULL, FALSE);

			sal_op_set_route(call->ping_op,sal_op_get_network_origin(op));
			sal_op_set_user_pointer(call->ping_op,call);

			sal_ping(call->ping_op,sal_op_get_from(call->ping_op), sal_op_get_to(call->ping_op));
		}
	}

	linphone_address_clean(from);
	linphone_call_get_local_ip(call, from);
	linphone_call_init_common(call, from, to);
	call->params = linphone_call_params_new();
	call->log->call_id=ms_strdup(sal_op_get_call_id(op)); /*must be known at that time*/
	call->dest_proxy = linphone_core_lookup_known_proxy(call->core, to);
	linphone_core_init_default_params(lc, call->params);

	/*
	 * Initialize call parameters according to incoming call parameters. This is to avoid to ask later (during reINVITEs) for features that the remote
	 * end apparently does not support. This features are: privacy, video
	 */
	/*set privacy*/
	call->current_params->privacy=(LinphonePrivacyMask)sal_op_get_privacy(call->op);
	/*set video support */
	md=sal_call_get_remote_media_description(op);
	call->params->has_video = linphone_core_video_enabled(lc) && lc->video_policy.automatically_accept;
	if (md) {
		// It is licit to receive an INVITE without SDP
		// In this case WE chose the media parameters according to policy.
		linphone_call_set_compatible_incoming_call_parameters(call, md);
		/* set multicast role & address if any*/
		if (!sal_call_is_offerer(op)){
			for (i=0;i<md->nb_streams;i++){
				if (md->streams[i].rtp_addr[0]!='\0' && ms_is_multicast(md->streams[i].rtp_addr)){
					md->streams[i].multicast_role = SalMulticastReceiver;
					strncpy(call->media_ports[i].multicast_ip,md->streams[i].rtp_addr,sizeof(call->media_ports[i].multicast_ip));
				}
			}
		}
	}

	fpol=linphone_core_get_firewall_policy(call->core);
	/*create the ice session now if ICE is required*/
	if (fpol==LinphonePolicyUseIce){
		if (md){
			call->ice_session = ice_session_new();
			/*for backward compatibility purposes, shall be enabled by default in futur*/
			ice_session_enable_message_integrity_check(call->ice_session,lp_config_get_int(lc->config,"net","ice_session_enable_message_integrity_check",0));
			ice_session_set_role(call->ice_session, IR_Controlled);
		}else{
			fpol=LinphonePolicyNoFirewall;
			ms_warning("ICE not supported for incoming INVITE without SDP.");
		}
	}

	/*reserve the sockets immediately*/
	linphone_call_init_media_streams(call);
	switch (fpol) {
		case LinphonePolicyUseIce:
			linphone_call_prepare_ice(call,TRUE);
			break;
		case LinphonePolicyUseStun:
			call->ping_time=linphone_core_run_stun_tests(call->core,call);
			/* No break to also destroy ice session in this case. */
			break;
		case LinphonePolicyUseUpnp:
#ifdef BUILD_UPNP
			if(!lc->rtp_conf.disable_upnp) {
				call->upnp_session = linphone_upnp_session_new(call);
				if (call->upnp_session != NULL) {
					if (linphone_core_update_upnp_from_remote_media_description(call, sal_call_get_remote_media_description(op))<0) {
						/* uPnP port mappings failed, proceed with the call anyway. */
						linphone_call_delete_upnp_session(call);
					}
				}
			}
#endif //BUILD_UPNP
			break;
		default:
			break;
	}

	discover_mtu(lc,linphone_address_get_domain(from));
	return call;
}

/*
 * Frees the media resources of the call.
 * This has to be done at the earliest, unlike signaling resources that sometimes need to be kept a bit more longer.
 * It is called by linphone_call_set_terminated() (for termination of calls signaled to the application), or directly by the destructor of LinphoneCall
 * (_linphone_call_destroy) if the call was never notified to the application.
 */
void linphone_call_free_media_resources(LinphoneCall *call){
	linphone_call_stop_media_streams(call);
	ms_media_stream_sessions_uninit(&call->sessions[0]);
	ms_media_stream_sessions_uninit(&call->sessions[1]);
	linphone_call_delete_upnp_session(call);
	linphone_call_delete_ice_session(call);
	linphone_call_stats_uninit(&call->stats[0]);
	linphone_call_stats_uninit(&call->stats[1]);
}

/*
 * Called internally when reaching the Released state, to perform cleanups to break circular references.
**/
static void linphone_call_set_released(LinphoneCall *call){
	if (call->op!=NULL) {
		/*transfer the last error so that it can be obtained even in Released state*/
		if (call->non_op_error.reason==SalReasonNone){
			const SalErrorInfo *ei=sal_op_get_error_info(call->op);
			sal_error_info_set(&call->non_op_error,ei->reason,ei->protocol_code,ei->status_string,ei->warnings);
		}
		/* so that we cannot have anymore upcalls for SAL
			concerning this call*/
		sal_op_release(call->op);
		call->op=NULL;
	}
	/*it is necessary to reset pointers to other call to prevent circular references that would result in memory never freed.*/
	if (call->referer){
		linphone_call_unref(call->referer);
		call->referer=NULL;
	}
	if (call->transfer_target){
		linphone_call_unref(call->transfer_target);
		call->transfer_target=NULL;
	}
	linphone_call_unref(call);
}

/* this function is called internally to get rid of a call that was notified to the application, because it reached the end or error state.
 It performs the following tasks:
 - remove the call from the internal list of calls
 - update the call logs accordingly
*/

static void linphone_call_set_terminated(LinphoneCall *call){
	LinphoneCore *lc=call->core;

	linphone_call_free_media_resources(call);
	linphone_call_log_completed(call);

	if (call == lc->current_call){
		ms_message("Resetting the current call");
		lc->current_call=NULL;
	}

	if (linphone_core_del_call(lc,call) != 0){
		ms_error("Could not remove the call from the list !!!");
	}
	linphone_core_conference_check_uninit(lc);
	if (call->ringing_beep){
		linphone_core_stop_dtmf(lc);
		call->ringing_beep=FALSE;
	}
}

void linphone_call_fix_call_parameters(LinphoneCall *call){
	if (sal_call_is_offerer(call->op)) {
		/*get remote params*/
		const LinphoneCallParams* rcp = linphone_call_get_remote_params(call);
		call->current_params->video_declined = call->params->has_video && !rcp->has_video;
	}
}

const char *linphone_call_state_to_string(LinphoneCallState cs){
	switch (cs){
		case LinphoneCallIdle:
			return "LinphoneCallIdle";
		case LinphoneCallIncomingReceived:
			return "LinphoneCallIncomingReceived";
		case LinphoneCallOutgoingInit:
			return "LinphoneCallOutgoingInit";
		case LinphoneCallOutgoingProgress:
			return "LinphoneCallOutgoingProgress";
		case LinphoneCallOutgoingRinging:
			return "LinphoneCallOutgoingRinging";
		case LinphoneCallOutgoingEarlyMedia:
			return "LinphoneCallOutgoingEarlyMedia";
		case LinphoneCallConnected:
			return "LinphoneCallConnected";
		case LinphoneCallStreamsRunning:
			return "LinphoneCallStreamsRunning";
		case LinphoneCallPausing:
			return "LinphoneCallPausing";
		case LinphoneCallPaused:
			return "LinphoneCallPaused";
		case LinphoneCallResuming:
			return "LinphoneCallResuming";
		case LinphoneCallRefered:
			return "LinphoneCallRefered";
		case LinphoneCallError:
			return "LinphoneCallError";
		case LinphoneCallEnd:
			return "LinphoneCallEnd";
		case LinphoneCallPausedByRemote:
			return "LinphoneCallPausedByRemote";
		case LinphoneCallUpdatedByRemote:
			return "LinphoneCallUpdatedByRemote";
		case LinphoneCallIncomingEarlyMedia:
			return "LinphoneCallIncomingEarlyMedia";
		case LinphoneCallUpdating:
			return "LinphoneCallUpdating";
		case LinphoneCallReleased:
			return "LinphoneCallReleased";
		case LinphoneCallEarlyUpdatedByRemote:
			return "LinphoneCallEarlyUpdatedByRemote";
		case LinphoneCallEarlyUpdating:
			return "LinphoneCallEarlyUpdating";
	}
	return "undefined state";
}

void linphone_call_set_state(LinphoneCall *call, LinphoneCallState cstate, const char *message){
	LinphoneCore *lc=call->core;

	if (call->state!=cstate){
		call->prevstate=call->state;
		if (call->state==LinphoneCallEnd || call->state==LinphoneCallError){
			if (cstate!=LinphoneCallReleased){
				ms_warning("Spurious call state change from %s to %s, ignored."	,linphone_call_state_to_string(call->state)
																				,linphone_call_state_to_string(cstate));
				return;
			}
		}
		ms_message("Call %p: moving from state %s to %s",call
														,linphone_call_state_to_string(call->state)
														,linphone_call_state_to_string(cstate));

		if (cstate!=LinphoneCallRefered){
			/*LinphoneCallRefered is rather an event, not a state.
			 Indeed it does not change the state of the call (still paused or running)*/
			call->state=cstate;
		}

		switch (cstate) {
		case LinphoneCallOutgoingInit:
		case LinphoneCallIncomingReceived:
#ifdef ANDROID
			ms_message("Call [%p] acquires both wifi and multicast lock",call);
			linphone_core_wifi_lock_acquire(call->core);
			linphone_core_multicast_lock_acquire(call->core); /*does no affect battery more than regular rtp traffic*/
#endif
			break;
		case LinphoneCallEnd:
		case LinphoneCallError:
			switch(call->non_op_error.reason){
			case SalReasonDeclined:
				call->log->status=LinphoneCallDeclined;
				break;
			case SalReasonRequestTimeout:
				call->log->status=LinphoneCallMissed;
				break;
			default:
				break;
			}
			linphone_call_set_terminated(call);
			break;
		case LinphoneCallConnected:
			call->log->status=LinphoneCallSuccess;
			call->log->connected_date_time=time(NULL);
			break;
		case LinphoneCallReleased:
#ifdef ANDROID
			ms_message("Call [%p] releases wifi/multicast lock",call);
			linphone_core_wifi_lock_release(call->core);
			linphone_core_multicast_lock_release(call->core);
#endif
			break;
		case LinphoneCallStreamsRunning:
			if (call->prevstate == LinphoneCallUpdating || call->prevstate == LinphoneCallUpdatedByRemote) {
				linphone_core_notify_display_status(lc,_("Call parameters were successfully modified."));
			}
			break;
		default:
			break;
		}

		if(cstate!=LinphoneCallStreamsRunning) {
			if (call->dtmfs_timer!=NULL){
				/*cancelling DTMF sequence, if any*/
				linphone_call_cancel_dtmfs(call);
			}
		}
		linphone_core_notify_call_state_changed(lc,call,cstate,message);
		linphone_reporting_call_state_updated(call);
		if (cstate==LinphoneCallReleased) {/*shall be performed after  app notification*/
			linphone_call_set_released(call);
		}
		linphone_core_soundcard_hint_check(lc);
	}
}

static void linphone_call_destroy(LinphoneCall *obj){
	ms_message("Call [%p] freed.",obj);
	if (obj->audiostream || obj->videostream){
		linphone_call_free_media_resources(obj);
	}
	if (obj->op!=NULL) {
		sal_op_release(obj->op);
		obj->op=NULL;
	}
	if (obj->biggestdesc!=NULL){
		sal_media_description_unref(obj->biggestdesc);
		obj->biggestdesc=NULL;
	}
	if (obj->resultdesc!=NULL) {
		sal_media_description_unref(obj->resultdesc);
		obj->resultdesc=NULL;
	}
	if (obj->localdesc!=NULL) {
		sal_media_description_unref(obj->localdesc);
		obj->localdesc=NULL;
	}
	if (obj->ping_op) {
		sal_op_release(obj->ping_op);
		obj->ping_op=NULL;
	}
	if (obj->refer_to){
		ms_free(obj->refer_to);
		obj->refer_to=NULL;
	}
	if (obj->referer){
		linphone_call_unref(obj->referer);
		obj->referer=NULL;
	}
	if (obj->transfer_target){
		linphone_call_unref(obj->transfer_target);
		obj->transfer_target=NULL;
	}
	if (obj->log) {
		linphone_call_log_unref(obj->log);
		obj->log=NULL;
	}
	if (obj->auth_token) {
		ms_free(obj->auth_token);
		obj->auth_token=NULL;
	}
	if (obj->dtls_certificate_fingerprint) {
		ms_free(obj->dtls_certificate_fingerprint);
		obj->dtls_certificate_fingerprint=NULL;
	}
	if (obj->dtmfs_timer) {
		linphone_call_cancel_dtmfs(obj);
	}
	if (obj->params){
		linphone_call_params_unref(obj->params);
		obj->params=NULL;
	}
	if (obj->current_params){
		linphone_call_params_unref(obj->current_params);
		obj->current_params=NULL;
	}
	if (obj->remote_params != NULL) {
		linphone_call_params_unref(obj->remote_params);
		obj->remote_params=NULL;
	}
	if (obj->me) {
		linphone_address_unref(obj->me);
		obj->me = NULL;
	}

	sal_error_info_reset(&obj->non_op_error);
}

/**
 * @addtogroup call_control
 * @{
**/

LinphoneCall * linphone_call_ref(LinphoneCall *obj){
	belle_sip_object_ref(obj);
	return obj;
}

void linphone_call_unref(LinphoneCall *obj){
	belle_sip_object_unref(obj);
}
static unsigned int linphone_call_get_n_active_streams(const LinphoneCall *call) {
	SalMediaDescription *md=NULL;
	if (call->op)
		md = sal_call_get_remote_media_description(call->op);
	if (!md)
		return 0;
	return sal_media_description_nb_active_streams_of_type(md, SalAudio) + sal_media_description_nb_active_streams_of_type(md, SalVideo);
}

/**
 * Returns current parameters associated to the call.
**/
const LinphoneCallParams * linphone_call_get_current_params(LinphoneCall *call){
	SalMediaDescription *md=call->resultdesc;
#ifdef VIDEO_ENABLED
	VideoStream *vstream;
#endif
	MS_VIDEO_SIZE_ASSIGN(call->current_params->sent_vsize, UNKNOWN);
	MS_VIDEO_SIZE_ASSIGN(call->current_params->recv_vsize, UNKNOWN);
#ifdef VIDEO_ENABLED
	vstream = call->videostream;
	if (vstream != NULL) {
		call->current_params->sent_vsize = video_stream_get_sent_video_size(vstream);
		call->current_params->recv_vsize = video_stream_get_received_video_size(vstream);
		call->current_params->sent_fps = video_stream_get_sent_framerate(vstream);
		call->current_params->received_fps = video_stream_get_received_framerate(vstream);
	}
#endif

	/* REVISITED
	 * Previous code was buggy.
	 * Relying on the mediastream's state (added by jehan: only) to know the current encryption is unreliable.
	 * For (added by jehan: both DTLS and) ZRTP it is though necessary.
	 * But for all others the current_params->media_encryption state should reflect (added by jehan: both) what is agreed by the offer/answer
	 * mechanism  (added by jehan: and encryption status from media which is much stronger than only result of offer/answer )
	 * Typically there can be inactive streams for which the media layer has no idea of whether they are encrypted or not.
	 */

	switch (call->params->media_encryption) {
	case LinphoneMediaEncryptionZRTP:
		if (linphone_call_all_streams_encrypted(call) && linphone_call_get_authentication_token(call)) {
			call->current_params->media_encryption=LinphoneMediaEncryptionZRTP;
		} else {
			call->current_params->media_encryption=LinphoneMediaEncryptionNone;
		}
		break;
	case LinphoneMediaEncryptionDTLS:
	case LinphoneMediaEncryptionSRTP:
		if (linphone_call_get_n_active_streams(call)==0 || linphone_call_all_streams_encrypted(call)) {
			call->current_params->media_encryption = call->params->media_encryption;
		} else {
			call->current_params->media_encryption=LinphoneMediaEncryptionNone;
		}
		break;
	case LinphoneMediaEncryptionNone:
		call->current_params->media_encryption=LinphoneMediaEncryptionNone;
		break;
	}

	call->current_params->avpf_enabled = linphone_call_all_streams_avpf_enabled(call);
	if (call->current_params->avpf_enabled == TRUE) {
		call->current_params->avpf_rr_interval = linphone_call_get_avpf_rr_interval(call);
	} else {
		call->current_params->avpf_rr_interval = 0;
	}
	if (md){
		const char *rtp_addr;

		SalStreamDescription *sd=sal_media_description_find_best_stream(md,SalAudio);
		call->current_params->audio_dir=sd ? media_direction_from_sal_stream_dir(sd->dir) : LinphoneMediaDirectionInactive;
		if (call->current_params->audio_dir != LinphoneMediaDirectionInactive) {
			rtp_addr = sd->rtp_addr[0]!='\0' ? sd->rtp_addr : call->resultdesc->addr;
			call->current_params->audio_multicast_enabled = ms_is_multicast(rtp_addr);
		} else
			call->current_params->audio_multicast_enabled = FALSE;

		sd=sal_media_description_find_best_stream(md,SalVideo);
		call->current_params->video_dir=sd ? media_direction_from_sal_stream_dir(sd->dir) : LinphoneMediaDirectionInactive;
		if (call->current_params->video_dir != LinphoneMediaDirectionInactive) {
			rtp_addr = sd->rtp_addr[0]!='\0' ? sd->rtp_addr : call->resultdesc->addr;
			call->current_params->video_multicast_enabled = ms_is_multicast(rtp_addr);
		} else
			call->current_params->video_multicast_enabled = FALSE;
	}

	return call->current_params;
}

/**
 * Returns call parameters proposed by remote.
 *
 * This is useful when receiving an incoming call, to know whether the remote party
 * supports video, encryption or whatever.
**/
const LinphoneCallParams * linphone_call_get_remote_params(LinphoneCall *call){
	if (call->op){
		LinphoneCallParams *cp;
		SalMediaDescription *md;
		if (call->remote_params != NULL) linphone_call_params_unref(call->remote_params);
		cp = call->remote_params = linphone_call_params_new();
		md=sal_call_get_remote_media_description(call->op);
		if (md) {
			SalStreamDescription *sd;
			unsigned int i;
			unsigned int nb_audio_streams = sal_media_description_nb_active_streams_of_type(md, SalAudio);
			unsigned int nb_video_streams = sal_media_description_nb_active_streams_of_type(md, SalVideo);

			for (i = 0; i < nb_video_streams; i++) {
				sd = sal_media_description_get_active_stream_of_type(md, SalVideo, i);
				if (sal_stream_description_active(sd) == TRUE) cp->has_video = TRUE;
				if (sal_stream_description_has_srtp(sd) == TRUE) cp->media_encryption = LinphoneMediaEncryptionSRTP;
			}
			for (i = 0; i < nb_audio_streams; i++) {
				sd = sal_media_description_get_active_stream_of_type(md, SalAudio, i);
				if (sal_stream_description_has_srtp(sd) == TRUE) cp->media_encryption = LinphoneMediaEncryptionSRTP;
			}
			if (!cp->has_video){
				if (md->bandwidth>0 && md->bandwidth<=linphone_core_get_edge_bw(call->core)){
					cp->low_bandwidth=TRUE;
				}
			}
			if (md->name[0]!='\0') linphone_call_params_set_session_name(cp,md->name);
		}
		cp->custom_headers=sal_custom_header_clone((SalCustomHeader*)sal_op_get_recv_custom_header(call->op));
		return cp;
	}
	return NULL;
}

/**
 * Returns the remote address associated to this call
 *
**/
const LinphoneAddress * linphone_call_get_remote_address(const LinphoneCall *call){
	return call->dir==LinphoneCallIncoming ? call->log->from : call->log->to;
}

/**
 * Returns the remote address associated to this call as a string.
 *
 * The result string must be freed by user using ms_free().
**/
char *linphone_call_get_remote_address_as_string(const LinphoneCall *call){
	return linphone_address_as_string(linphone_call_get_remote_address(call));
}

/**
 * Retrieves the call's current state.
**/
LinphoneCallState linphone_call_get_state(const LinphoneCall *call){
	return call->state;
}

/**
 * Returns the reason for a call termination (either error or normal termination)
**/
LinphoneReason linphone_call_get_reason(const LinphoneCall *call){
	return linphone_error_info_get_reason(linphone_call_get_error_info(call));
}

/**
 * Returns full details about call errors or termination reasons.
**/
const LinphoneErrorInfo *linphone_call_get_error_info(const LinphoneCall *call){
	if (call->non_op_error.reason!=SalReasonNone){
		return (const LinphoneErrorInfo*)&call->non_op_error;
	}else return linphone_error_info_from_sal_op(call->op);
}

/**
 * Get the user pointer associated with the LinphoneCall
 *
 * @ingroup call_control
 * @return  an opaque user pointer that can be retrieved at any time
**/
void *linphone_call_get_user_data(const LinphoneCall *call)
{
	return call->user_data;
}

/**
 * Set the user pointer associated with the LinphoneCall
 *
 * @ingroup call_control
 *
 * the user pointer is an opaque user pointer that can be retrieved at any time in the LinphoneCall
**/
void linphone_call_set_user_data(LinphoneCall *call, void *user_pointer)
{
	call->user_data = user_pointer;
}

/**
 * Returns the call log associated to this call.
**/
LinphoneCallLog *linphone_call_get_call_log(const LinphoneCall *call){
	return call->log;
}

/**
 * Returns the refer-to uri (if the call was transfered).
**/
const char *linphone_call_get_refer_to(const LinphoneCall *call){
	return call->refer_to;
}

/**
 * Returns the transferer if this call was started automatically as a result of an incoming transfer request.
 * The call in which the transfer request was received is returned in this case.
**/
LinphoneCall *linphone_call_get_transferer_call(const LinphoneCall *call){
	return call->referer;
}

/**
 * When this call has received a transfer request, returns the new call that was automatically created as a result of the transfer.
**/
LinphoneCall *linphone_call_get_transfer_target_call(const LinphoneCall *call){
	return call->transfer_target;
}

/**
 * Returns direction of the call (incoming or outgoing).
**/
LinphoneCallDir linphone_call_get_dir(const LinphoneCall *call){
	return call->log->dir;
}

/**
 * Returns the far end's user agent description string, if available.
**/
const char *linphone_call_get_remote_user_agent(LinphoneCall *call){
	if (call->op){
		return sal_op_get_remote_ua (call->op);
	}
	return NULL;
}

/**
 * Returns the far end's sip contact as a string, if available.
**/
const char *linphone_call_get_remote_contact(LinphoneCall *call){
	const LinphoneCallParams* lcp = linphone_call_get_remote_params(call);
	if( lcp ){
		// we're not using sal_op_get_remote_contact() here because the returned value is stripped from
		// params that we need, like the instanceid. Getting it from the headers will make sure we
		// get everything
		return linphone_call_params_get_custom_header(lcp, "Contact");
	}
	return NULL;
}


/**
 * Returns true if this calls has received a transfer that has not been
 * executed yet.
 * Pending transfers are executed when this call is being paused or closed,
 * locally or by remote endpoint.
 * If the call is already paused while receiving the transfer request, the
 * transfer immediately occurs.
**/
bool_t linphone_call_has_transfer_pending(const LinphoneCall *call){
	return call->refer_pending;
}

/**
 * Returns call's duration in seconds.
**/
int linphone_call_get_duration(const LinphoneCall *call){
	if (call->log->connected_date_time==0) return 0;
	return time(NULL)-call->log->connected_date_time;
}

/**
 * Returns the call object this call is replacing, if any.
 * Call replacement can occur during call transfers.
 * By default, the core automatically terminates the replaced call and accept the new one.
 * This function allows the application to know whether a new incoming call is a one that replaces another one.
**/
LinphoneCall *linphone_call_get_replaced_call(LinphoneCall *call){
	SalOp *op=sal_call_get_replaces(call->op);
	if (op){
		return (LinphoneCall*)sal_op_get_user_pointer(op);
	}
	return NULL;
}

/**
 * Indicate whether camera input should be sent to remote end.
**/
void linphone_call_enable_camera (LinphoneCall *call, bool_t enable){
#ifdef VIDEO_ENABLED
	call->camera_enabled=enable;
	if ((call->state==LinphoneCallStreamsRunning || call->state==LinphoneCallOutgoingEarlyMedia || call->state==LinphoneCallIncomingEarlyMedia)
		&& call->videostream!=NULL && video_stream_started(call->videostream) ){
		if (video_stream_get_camera(call->videostream) != linphone_call_get_video_device(call)) {
			const char *cur_cam, *new_cam;
			cur_cam = video_stream_get_camera(call->videostream) ? ms_web_cam_get_name(video_stream_get_camera(call->videostream)) : "NULL";
			new_cam = linphone_call_get_video_device(call) ? ms_web_cam_get_name(linphone_call_get_video_device(call)) : "NULL";
			ms_message("Switching video cam from [%s] to [%s] on call [%p]"	, cur_cam, new_cam, call);
			video_stream_change_camera(call->videostream, linphone_call_get_video_device(call));
		}
	}
#endif
}

/**
 * Request remote side to send us a Video Fast Update.
**/
void linphone_call_send_vfu_request(LinphoneCall *call) {
#ifdef VIDEO_ENABLED
	const LinphoneCallParams *current_params = linphone_call_get_current_params(call);
	if (current_params->avpf_enabled && call->videostream && media_stream_get_state((const MediaStream *)call->videostream) == MSStreamStarted) {
		ms_message("Request Full Intra Request on call [%p]", call);
		video_stream_send_fir(call->videostream);
	} else if (call->core->sip_conf.vfu_with_info) {
		if (LinphoneCallStreamsRunning == linphone_call_get_state(call))
			sal_call_send_vfu_request(call->op);
	} else {
		ms_message("vfu request using sip disabled from config [sip,vfu_with_info]");
	}
#endif
}


/**
 * Take a photo of currently received video and write it into a jpeg file.
 * Note that the snapshot is asynchronous, an application shall not assume that the file is created when the function returns.
 * @param call a LinphoneCall
 * @param file a path where to write the jpeg content.
 * @return 0 if successfull, -1 otherwise (typically if jpeg format is not supported).
**/
int linphone_call_take_video_snapshot(LinphoneCall *call, const char *file){
#ifdef VIDEO_ENABLED
	if (call->videostream!=NULL && call->videostream->jpegwriter!=NULL){
		return ms_filter_call_method(call->videostream->jpegwriter,MS_JPEG_WRITER_TAKE_SNAPSHOT,(void*)file);
	}
	ms_warning("Cannot take snapshot: no currently running video stream on this call.");
#endif
	return -1;
}

/**
 * Take a photo of currently captured video and write it into a jpeg file.
 * Note that the snapshot is asynchronous, an application shall not assume that the file is created when the function returns.
 * @param call a LinphoneCall
 * @param file a path where to write the jpeg content.
 * @return 0 if successfull, -1 otherwise (typically if jpeg format is not supported).
**/
int linphone_call_take_preview_snapshot(LinphoneCall *call, const char *file){
#ifdef VIDEO_ENABLED
	if (call->videostream!=NULL && call->videostream->local_jpegwriter!=NULL){
		return ms_filter_call_method(call->videostream->local_jpegwriter,MS_JPEG_WRITER_TAKE_SNAPSHOT,(void*)file);
	}
	ms_warning("Cannot take local snapshot: no currently running video stream on this call.");
	return -1;
#endif
	return -1;
}

/**
 * Returns TRUE if camera pictures are allowed to be sent to the remote party.
**/
bool_t linphone_call_camera_enabled (const LinphoneCall *call){
	return call->camera_enabled;
}


/**
 * @ingroup call_control
 * @return string value of LinphonePrivacy enum
 **/
const char* linphone_privacy_to_string(LinphonePrivacy privacy) {
	switch(privacy) {
	case LinphonePrivacyDefault: return "LinphonePrivacyDefault";
	case LinphonePrivacyUser: return "LinphonePrivacyUser";
	case LinphonePrivacyHeader: return "LinphonePrivacyHeader";
	case LinphonePrivacySession: return "LinphonePrivacySession";
	case LinphonePrivacyId: return "LinphonePrivacyId";
	case LinphonePrivacyNone: return "LinphonePrivacyNone";
	case LinphonePrivacyCritical: return "LinphonePrivacyCritical";
	default: return "Unknown privacy mode";
	}
}


/**
 * @}
**/


#ifdef TEST_EXT_RENDERER
static void rendercb(void *data, const MSPicture *local, const MSPicture *remote){
	ms_message("rendercb, local buffer=%p, remote buffer=%p",
			   local ? local->planes[0] : NULL, remote? remote->planes[0] : NULL);
}
#endif

#ifdef VIDEO_ENABLED
static void video_stream_event_cb(void *user_pointer, const MSFilter *f, const unsigned int event_id, const void *args){
	LinphoneCall* call = (LinphoneCall*) user_pointer;
	switch (event_id) {
		case MS_VIDEO_DECODER_DECODING_ERRORS:
			ms_warning("MS_VIDEO_DECODER_DECODING_ERRORS");
			if (call->videostream && (video_stream_is_decoding_error_to_be_reported(call->videostream, 5000) == TRUE)) {
				video_stream_decoding_error_reported(call->videostream);
				linphone_call_send_vfu_request(call);
			}
			break;
		case MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS:
			ms_message("MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS");
			if (call->videostream) {
				video_stream_decoding_error_recovered(call->videostream);
			}
			break;
		case MS_VIDEO_DECODER_FIRST_IMAGE_DECODED:
			ms_message("First video frame decoded successfully");
			if (call->nextVideoFrameDecoded._func != NULL)
				call->nextVideoFrameDecoded._func(call, call->nextVideoFrameDecoded._user_data);
			break;
		case MS_VIDEO_DECODER_SEND_PLI:
		case MS_VIDEO_DECODER_SEND_SLI:
		case MS_VIDEO_DECODER_SEND_RPSI:
			/* Handled internally by mediastreamer2. */
			break;
		default:
			ms_warning("Unhandled event %i", event_id);
			break;
	}
}
#endif

void linphone_call_set_next_video_frame_decoded_callback(LinphoneCall *call, LinphoneCallCbFunc cb, void* user_data) {
	call->nextVideoFrameDecoded._func = cb;
	call->nextVideoFrameDecoded._user_data = user_data;
#ifdef VIDEO_ENABLED
	if (call->videostream && call->videostream->ms.decoder)
		ms_filter_call_method_noarg(call->videostream->ms.decoder, MS_VIDEO_DECODER_RESET_FIRST_IMAGE_NOTIFICATION);
#endif
}

static void port_config_set_random_choosed(LinphoneCall *call, int stream_index, RtpSession *session){
	call->media_ports[stream_index].rtp_port=rtp_session_get_local_port(session);
	call->media_ports[stream_index].rtcp_port=rtp_session_get_local_rtcp_port(session);
}

static void _linphone_call_prepare_ice_for_stream(LinphoneCall *call, int stream_index, bool_t create_checklist){
	MediaStream *ms=stream_index == 0 ? (MediaStream*)call->audiostream : (MediaStream*)call->videostream;
	if ((linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseIce) && (call->ice_session != NULL)){
		IceCheckList *cl;
		rtp_session_set_pktinfo(ms->sessions.rtp_session, TRUE);
		cl=ice_session_check_list(call->ice_session, stream_index);
		if (cl == NULL && create_checklist) {
			cl=ice_check_list_new();
			ice_session_add_check_list(call->ice_session, cl, stream_index);
			ms_message("Created new ICE check list for stream [%i]",stream_index);
		}
		if (cl){
			ms->ice_check_list = cl;
			ice_check_list_set_rtp_session(ms->ice_check_list, ms->sessions.rtp_session);
		}
	}
}

int linphone_call_prepare_ice(LinphoneCall *call, bool_t incoming_offer){
	SalMediaDescription *remote = NULL;
	bool_t has_video=FALSE;

	if ((linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseIce) && (call->ice_session != NULL)){
		if (incoming_offer){
			remote=sal_call_get_remote_media_description(call->op);
			has_video=linphone_core_video_enabled(call->core) && linphone_core_media_description_contains_video_stream(remote);
		}else has_video=call->params->has_video;

		_linphone_call_prepare_ice_for_stream(call,0,TRUE);
		if (has_video) _linphone_call_prepare_ice_for_stream(call,1,TRUE);
		/*start ICE gathering*/
		if (incoming_offer)
			linphone_call_update_ice_from_remote_media_description(call,remote); /*this may delete the ice session*/
		if (call->ice_session && !ice_session_candidates_gathered(call->ice_session)){
			if (call->audiostream->ms.state==MSStreamInitialized)
				audio_stream_prepare_sound(call->audiostream, NULL, NULL);
#ifdef VIDEO_ENABLED
			if (has_video && call->videostream && call->videostream->ms.state==MSStreamInitialized) {
				video_stream_prepare_video(call->videostream);
			}
#endif
			if (linphone_core_gather_ice_candidates(call->core,call)<0) {
				/* Ice candidates gathering failed, proceed with the call anyway. */
				linphone_call_delete_ice_session(call);
				linphone_call_stop_media_streams_for_ice_gathering(call);
				return -1;
			}
			return 1;/*gathering in progress, wait*/
		}
	}
	return 0;
}

/*eventually join to a multicast group if told to do so*/
static void linphone_call_join_multicast_group(LinphoneCall *call, int stream_index, MediaStream *ms){
	if (call->media_ports[stream_index].multicast_ip[stream_index]!='\0'){
		media_stream_join_multicast_group(ms, call->media_ports[stream_index].multicast_ip);
	} else
		ms_error("Cannot join multicast group if multicast ip is not set for call [%p]",call);
}

static SalMulticastRole linphone_call_get_multicast_role(const LinphoneCall *call,SalStreamType type) {
	SalMulticastRole multicast_role=SalMulticastInactive;
	SalMediaDescription *remotedesc, *localdesc;
	SalStreamDescription *stream_desc = NULL;
	if (!call->op) goto end;
	remotedesc = sal_call_get_remote_media_description(call->op);
	localdesc = call->localdesc;
	if (!localdesc && !remotedesc && call->dir == LinphoneCallOutgoing) {
		/*well using call dir*/
		if ((type == SalAudio && linphone_call_params_audio_multicast_enabled(call->params))
			|| (type == SalVideo && linphone_call_params_video_multicast_enabled(call->params)))
			multicast_role=SalMulticastSender;
	} else	if (localdesc && (!remotedesc || sal_call_is_offerer(call->op))) {
		stream_desc = sal_media_description_find_best_stream(localdesc, type);
	} else if (!sal_call_is_offerer(call->op) && remotedesc)
		stream_desc = sal_media_description_find_best_stream(remotedesc, type);

	if (stream_desc)
		multicast_role=stream_desc->multicast_role;
	else
		ms_message("Cannot determine multicast role for stream type [%s] on call [%p]",sal_stream_type_to_string(type),call);


	end:
	return multicast_role;

}

static void setup_dtls_params(LinphoneCall *call, MediaStream* stream) {
	LinphoneCore *lc=call->core;
	if (call->params->media_encryption==LinphoneMediaEncryptionDTLS) {
		MSDtlsSrtpParams params;
		char *certificate, *key;
		memset(&params,0,sizeof(MSDtlsSrtpParams));
		/* TODO : search for a certificate with CNAME=sip uri(retrieved from variable me) or default : linphone-dtls-default-identity */
		/* This will parse the directory to find a matching fingerprint or generate it if not found */
		/* returned string must be freed */
		sal_certificates_chain_parse_directory(&certificate, &key, &call->dtls_certificate_fingerprint, lc->user_certificates_path, "linphone-dtls-default-identity", SAL_CERTIFICATE_RAW_FORMAT_PEM, TRUE, TRUE);

		if (key!= NULL && certificate!=NULL) {
			params.pem_certificate = (char *)certificate;
			params.pem_pkey = (char *)key;
			params.role = MSDtlsSrtpRoleUnset; /* default is unset, then check if we have a result SalMediaDescription */
			media_stream_enable_dtls(stream,&params);
			ms_free(certificate);
			ms_free(key);
		} else {
			ms_error("Unable to retrieve or generate DTLS certificate and key - DTLS disabled");
			/* TODO : check if encryption forced, if yes, stop call */
		}
	}
}

void linphone_call_init_audio_stream(LinphoneCall *call){
	LinphoneCore *lc=call->core;
	AudioStream *audiostream;
	const char *location;
	int dscp;
	char rtcp_tool[128]={0};
	char* cname;


	snprintf(rtcp_tool,sizeof(rtcp_tool)-1,"%s-%s",linphone_core_get_user_agent_name(),linphone_core_get_user_agent_version());

	if (call->audiostream != NULL) return;
	if (call->sessions[0].rtp_session==NULL){
		SalMulticastRole multicast_role = linphone_call_get_multicast_role(call,SalAudio);
		SalMediaDescription *remotedesc=NULL;
		SalStreamDescription *stream_desc = NULL;
		if (call->op) remotedesc = sal_call_get_remote_media_description(call->op);
		if (remotedesc)
				stream_desc = sal_media_description_find_best_stream(remotedesc, SalAudio);

		call->audiostream=audiostream=audio_stream_new2(linphone_call_get_bind_ip_for_stream(call,0),
				multicast_role ==  SalMulticastReceiver ? stream_desc->rtp_port : call->media_ports[0].rtp_port,
				multicast_role ==  SalMulticastReceiver ? 0 /*disabled for now*/ : call->media_ports[0].rtcp_port);
		if (multicast_role == SalMulticastReceiver)
			linphone_call_join_multicast_group(call, 0, &audiostream->ms);
		rtp_session_enable_network_simulation(call->audiostream->ms.sessions.rtp_session, &lc->net_conf.netsim_params);
		cname = linphone_address_as_string_uri_only(call->me);
		audio_stream_set_rtcp_information(call->audiostream, cname, rtcp_tool);
		ms_free(cname);
		rtp_session_set_symmetric_rtp(audiostream->ms.sessions.rtp_session,linphone_core_symmetric_rtp_enabled(lc));
		setup_dtls_params(call, &audiostream->ms);
	}else{
		call->audiostream=audio_stream_new_with_sessions(&call->sessions[0]);
	}
	audiostream=call->audiostream;
	if (call->media_ports[0].rtp_port==-1){
		port_config_set_random_choosed(call,0,audiostream->ms.sessions.rtp_session);
	}
	dscp=linphone_core_get_audio_dscp(lc);
	if (dscp!=-1)
		audio_stream_set_dscp(audiostream,dscp);
	if (linphone_core_echo_limiter_enabled(lc)){
		const char *type=lp_config_get_string(lc->config,"sound","el_type","mic");
		if (strcasecmp(type,"mic")==0)
			audio_stream_enable_echo_limiter(audiostream,ELControlMic);
		else if (strcasecmp(type,"full")==0)
			audio_stream_enable_echo_limiter(audiostream,ELControlFull);
	}

	/* equalizer location in the graph: 'mic' = in input graph, otherwise in output graph.
		Any other value than mic will default to output graph for compatibility */
	location = lp_config_get_string(lc->config,"sound","eq_location","hp");
	audiostream->eq_loc = (strcasecmp(location,"mic") == 0) ? MSEqualizerMic : MSEqualizerHP;
	ms_message("Equalizer location: %s", location);

	audio_stream_enable_gain_control(audiostream,TRUE);
	if (linphone_core_echo_cancellation_enabled(lc)){
		int len,delay,framesize;
		len=lp_config_get_int(lc->config,"sound","ec_tail_len",0);
		delay=lp_config_get_int(lc->config,"sound","ec_delay",0);
		framesize=lp_config_get_int(lc->config,"sound","ec_framesize",0);
		audio_stream_set_echo_canceller_params(audiostream,len,delay,framesize);
		if (audiostream->ec) {
			char *statestr=ms_malloc0(EC_STATE_MAX_LEN);
			if (lp_config_relative_file_exists(lc->config, EC_STATE_STORE)
				 && lp_config_read_relative_file(lc->config, EC_STATE_STORE, statestr, EC_STATE_MAX_LEN) == 0) {
				ms_filter_call_method(audiostream->ec, MS_ECHO_CANCELLER_SET_STATE_STRING, statestr);
			}
			ms_free(statestr);
		}
	}
	audio_stream_enable_automatic_gain_control(audiostream,linphone_core_agc_enabled(lc));
	{
		int enabled=lp_config_get_int(lc->config,"sound","noisegate",0);
		audio_stream_enable_noise_gate(audiostream,enabled);
	}

	audio_stream_set_features(audiostream,linphone_core_get_audio_features(lc));

	if (lc->rtptf){
		RtpTransport *meta_rtp;
		RtpTransport *meta_rtcp;

		rtp_session_get_transports(audiostream->ms.sessions.rtp_session,&meta_rtp,&meta_rtcp);
		if (meta_rtp_transport_get_endpoint(meta_rtp) == NULL) {
			meta_rtp_transport_set_endpoint(meta_rtp,lc->rtptf->audio_rtp_func(lc->rtptf->audio_rtp_func_data, call->media_ports[0].rtp_port));
		}
		if (meta_rtp_transport_get_endpoint(meta_rtcp) == NULL) {
			meta_rtp_transport_set_endpoint(meta_rtcp,lc->rtptf->audio_rtcp_func(lc->rtptf->audio_rtcp_func_data, call->media_ports[0].rtcp_port));
		}
	}

	call->audiostream_app_evq = ortp_ev_queue_new();
	rtp_session_register_event_queue(audiostream->ms.sessions.rtp_session,call->audiostream_app_evq);

	_linphone_call_prepare_ice_for_stream(call,0,FALSE);
}

void linphone_call_init_video_stream(LinphoneCall *call){
#ifdef VIDEO_ENABLED
	LinphoneCore *lc=call->core;
	char* cname;
	char rtcp_tool[128];


	snprintf(rtcp_tool,sizeof(rtcp_tool)-1,"%s-%s",linphone_core_get_user_agent_name(),linphone_core_get_user_agent_version());

	if (call->videostream == NULL){
		int video_recv_buf_size=lp_config_get_int(lc->config,"video","recv_buf_size",0);
		int dscp=linphone_core_get_video_dscp(lc);
		const char *display_filter=linphone_core_get_video_display_filter(lc);

		if (call->sessions[1].rtp_session==NULL){
			SalMulticastRole multicast_role = linphone_call_get_multicast_role(call,SalVideo);
			SalMediaDescription *remotedesc=NULL;
			SalStreamDescription *stream_desc = NULL;
			if (call->op) remotedesc = sal_call_get_remote_media_description(call->op);
			if (remotedesc)
					stream_desc = sal_media_description_find_best_stream(remotedesc, SalVideo);

			call->videostream=video_stream_new2(linphone_call_get_bind_ip_for_stream(call,1),
					multicast_role ==  SalMulticastReceiver ? stream_desc->rtp_port : call->media_ports[1].rtp_port,
					multicast_role ==  SalMulticastReceiver ?  0 /*disabled for now*/ : call->media_ports[1].rtcp_port);
			if (multicast_role == SalMulticastReceiver)
				linphone_call_join_multicast_group(call, 1, &call->videostream->ms);
			rtp_session_enable_network_simulation(call->videostream->ms.sessions.rtp_session, &lc->net_conf.netsim_params);
			cname = linphone_address_as_string_uri_only(call->me);
			video_stream_set_rtcp_information(call->videostream, cname, rtcp_tool);
			ms_free(cname);
			rtp_session_set_symmetric_rtp(call->videostream->ms.sessions.rtp_session,linphone_core_symmetric_rtp_enabled(lc));

			setup_dtls_params(call, &call->videostream->ms);
		}else{
			call->videostream=video_stream_new_with_sessions(&call->sessions[1]);
		}

		if (call->media_ports[1].rtp_port==-1){
			port_config_set_random_choosed(call,1,call->videostream->ms.sessions.rtp_session);
		}
		if (dscp!=-1)
			video_stream_set_dscp(call->videostream,dscp);
		video_stream_enable_display_filter_auto_rotate(call->videostream, lp_config_get_int(lc->config,"video","display_filter_auto_rotate",0));
		if (video_recv_buf_size>0) rtp_session_set_recv_buf_size(call->videostream->ms.sessions.rtp_session,video_recv_buf_size);

		if (display_filter != NULL)
			video_stream_set_display_filter_name(call->videostream,display_filter);
		video_stream_set_event_callback(call->videostream,video_stream_event_cb, call);

		if (lc->rtptf){
			RtpTransport *meta_rtp;
			RtpTransport *meta_rtcp;

			rtp_session_get_transports(call->videostream->ms.sessions.rtp_session,&meta_rtp,&meta_rtcp);
			if (meta_rtp_transport_get_endpoint(meta_rtp) == NULL) {
				meta_rtp_transport_set_endpoint(meta_rtp,lc->rtptf->video_rtp_func(lc->rtptf->video_rtp_func_data, call->media_ports[1].rtp_port));
			}
			if (meta_rtp_transport_get_endpoint(meta_rtcp) == NULL) {
				meta_rtp_transport_set_endpoint(meta_rtcp,lc->rtptf->video_rtcp_func(lc->rtptf->video_rtcp_func_data, call->media_ports[1].rtcp_port));
			}
		}
		call->videostream_app_evq = ortp_ev_queue_new();
		rtp_session_register_event_queue(call->videostream->ms.sessions.rtp_session,call->videostream_app_evq);
		_linphone_call_prepare_ice_for_stream(call,1,FALSE);
#ifdef TEST_EXT_RENDERER
		video_stream_set_render_callback(call->videostream,rendercb,NULL);
#endif
	}
#else
	call->videostream=NULL;
#endif
}

void linphone_call_init_media_streams(LinphoneCall *call){
	linphone_call_init_audio_stream(call);
	linphone_call_init_video_stream(call);
}


static int dtmf_tab[16]={'0','1','2','3','4','5','6','7','8','9','*','#','A','B','C','D'};

static void linphone_core_dtmf_received(LinphoneCall *call, int dtmf){
	if (dtmf<0 || dtmf>15){
		ms_warning("Bad dtmf value %i",dtmf);
		return;
	}
	linphone_core_notify_dtmf_received(call->core, call, dtmf_tab[dtmf]);
}

static void parametrize_equalizer(LinphoneCore *lc, AudioStream *st){
	if (st->equalizer){
		MSFilter *f=st->equalizer;
		int enabled=lp_config_get_int(lc->config,"sound","eq_active",0);
		const char *gains=lp_config_get_string(lc->config,"sound","eq_gains",NULL);
		ms_filter_call_method(f,MS_EQUALIZER_SET_ACTIVE,&enabled);
		if (enabled){
			if (gains){
				do{
					int bytes;
					MSEqualizerGain g;
					if (sscanf(gains,"%f:%f:%f %n",&g.frequency,&g.gain,&g.width,&bytes)==3){
						ms_message("Read equalizer gains: %f(~%f) --> %f",g.frequency,g.width,g.gain);
						ms_filter_call_method(f,MS_EQUALIZER_SET_GAIN,&g);
						gains+=bytes;
					}else break;
				}while(1);
			}
		}
	}
}

void set_mic_gain_db(AudioStream *st, float gain){
	audio_stream_set_mic_gain_db(st, gain);
}

void set_playback_gain_db(AudioStream *st, float gain){
	if (st->volrecv){
		ms_filter_call_method(st->volrecv,MS_VOLUME_SET_DB_GAIN,&gain);
	}else ms_warning("Could not apply playback gain: gain control wasn't activated.");
}

/*This function is not static because used internally in linphone-daemon project*/
void _post_configure_audio_stream(AudioStream *st, LinphoneCore *lc, bool_t muted){
	float mic_gain=lc->sound_conf.soft_mic_lev;
	float thres = 0;
	float recv_gain;
	float ng_thres=lp_config_get_float(lc->config,"sound","ng_thres",0.05);
	float ng_floorgain=lp_config_get_float(lc->config,"sound","ng_floorgain",0);
	int dc_removal=lp_config_get_int(lc->config,"sound","dc_removal",0);
	float speed;
	float force;
	int sustain;
	float transmit_thres;
	MSFilter *f=NULL;
	float floorgain;
	int spk_agc;

	if (!muted)
		set_mic_gain_db(st,mic_gain);
	else
		audio_stream_set_mic_gain(st,0);

	recv_gain = lc->sound_conf.soft_play_lev;
	if (recv_gain != 0) {
		set_playback_gain_db(st,recv_gain);
	}

	if (st->volsend){
		ms_filter_call_method(st->volsend,MS_VOLUME_REMOVE_DC,&dc_removal);
		speed=lp_config_get_float(lc->config,"sound","el_speed",-1);
		thres=lp_config_get_float(lc->config,"sound","el_thres",-1);
		force=lp_config_get_float(lc->config,"sound","el_force",-1);
		sustain=lp_config_get_int(lc->config,"sound","el_sustain",-1);
		transmit_thres=lp_config_get_float(lc->config,"sound","el_transmit_thres",-1);
		f=st->volsend;
		if (speed==-1) speed=0.03;
		if (force==-1) force=25;
		ms_filter_call_method(f,MS_VOLUME_SET_EA_SPEED,&speed);
		ms_filter_call_method(f,MS_VOLUME_SET_EA_FORCE,&force);
		if (thres!=-1)
			ms_filter_call_method(f,MS_VOLUME_SET_EA_THRESHOLD,&thres);
		if (sustain!=-1)
			ms_filter_call_method(f,MS_VOLUME_SET_EA_SUSTAIN,&sustain);
		if (transmit_thres!=-1)
				ms_filter_call_method(f,MS_VOLUME_SET_EA_TRANSMIT_THRESHOLD,&transmit_thres);

		ms_filter_call_method(st->volsend,MS_VOLUME_SET_NOISE_GATE_THRESHOLD,&ng_thres);
		ms_filter_call_method(st->volsend,MS_VOLUME_SET_NOISE_GATE_FLOORGAIN,&ng_floorgain);
	}
	if (st->volrecv){
		/* parameters for a limited noise-gate effect, using echo limiter threshold */
		floorgain = 1/pow(10,(mic_gain)/10);
		spk_agc=lp_config_get_int(lc->config,"sound","speaker_agc_enabled",0);
		ms_filter_call_method(st->volrecv, MS_VOLUME_ENABLE_AGC, &spk_agc);
		ms_filter_call_method(st->volrecv,MS_VOLUME_SET_NOISE_GATE_THRESHOLD,&ng_thres);
		ms_filter_call_method(st->volrecv,MS_VOLUME_SET_NOISE_GATE_FLOORGAIN,&floorgain);
	}
	parametrize_equalizer(lc,st);
}

static void post_configure_audio_streams(LinphoneCall *call, bool_t muted){
	AudioStream *st=call->audiostream;
	LinphoneCore *lc=call->core;
	_post_configure_audio_stream(st,lc,muted);
	if (linphone_core_dtmf_received_has_listener(lc)){
		audio_stream_play_received_dtmfs(call->audiostream,FALSE);
	}
	if (call->record_active)
		linphone_call_start_recording(call);
}

static int get_ideal_audio_bw(LinphoneCall *call, const SalMediaDescription *md, const SalStreamDescription *desc){
	int remote_bw=0;
	int upload_bw;
	int total_upload_bw=linphone_core_get_upload_bandwidth(call->core);
	const LinphoneCallParams *params=call->params;
	bool_t will_use_video=linphone_core_media_description_contains_video_stream(md);
	bool_t forced=FALSE;

	if (desc->bandwidth>0) remote_bw=desc->bandwidth;
	else if (md->bandwidth>0) {
		/*case where b=AS is given globally, not per stream*/
		remote_bw=md->bandwidth;
	}
	if (params->up_bw>0){
		forced=TRUE;
		upload_bw=params->up_bw;
	}else upload_bw=total_upload_bw;
	upload_bw=get_min_bandwidth(upload_bw,remote_bw);
	if (!will_use_video || forced) return upload_bw;

	if (bandwidth_is_greater(upload_bw,512)){
		upload_bw=100;
	}else if (bandwidth_is_greater(upload_bw,256)){
		upload_bw=64;
	}else if (bandwidth_is_greater(upload_bw,128)){
		upload_bw=40;
	}else if (bandwidth_is_greater(upload_bw,0)){
		upload_bw=24;
	}
	return upload_bw;
}

static int get_video_bw(LinphoneCall *call, const SalMediaDescription *md, const SalStreamDescription *desc){
	int remote_bw=0;
	int bw;
	if (desc->bandwidth>0) remote_bw=desc->bandwidth;
	else if (md->bandwidth>0) {
		/*case where b=AS is given globally, not per stream*/
		remote_bw=get_remaining_bandwidth_for_video(md->bandwidth,call->audio_bw);
	} else {
		remote_bw = lp_config_get_int(call->core->config, "net", "default_max_bandwidth", 1500);
	}
	bw=get_min_bandwidth(get_remaining_bandwidth_for_video(linphone_core_get_upload_bandwidth(call->core),call->audio_bw),remote_bw);
	return bw;
}

static RtpProfile *make_profile(LinphoneCall *call, const SalMediaDescription *md, const SalStreamDescription *desc, int *used_pt){
	int bw=0;
	const MSList *elem;
	RtpProfile *prof=rtp_profile_new("Call profile");
	bool_t first=TRUE;
	LinphoneCore *lc=call->core;
	int up_ptime=0;
	const LinphoneCallParams *params=call->params;

	*used_pt=-1;

	if (desc->type==SalAudio)
		bw=get_ideal_audio_bw(call,md,desc);
	else if (desc->type==SalVideo)
		bw=get_video_bw(call,md,desc);

	for(elem=desc->payloads;elem!=NULL;elem=elem->next){
		PayloadType *pt=(PayloadType*)elem->data;
		int number;
		/* make a copy of the payload type, so that we left the ones from the SalStreamDescription unchanged.
		 If the SalStreamDescription is freed, this will have no impact on the running streams*/
		pt=payload_type_clone(pt);

		if ((pt->flags & PAYLOAD_TYPE_FLAG_CAN_SEND) && first) {
			/*first codec in list is the selected one*/
			if (desc->type==SalAudio){
				/*this will update call->audio_bw*/
				linphone_core_update_allocated_audio_bandwidth_in_call(call,pt,bw);
				bw=call->audio_bw;
				if (params->up_ptime)
					up_ptime=params->up_ptime;
				else up_ptime=linphone_core_get_upload_ptime(lc);
			}
			*used_pt=payload_type_get_number(pt);
			first=FALSE;
		}
		if (pt->flags & PAYLOAD_TYPE_BITRATE_OVERRIDE){
			ms_message("Payload type [%s/%i] has explicit bitrate [%i] kbit/s", pt->mime_type, pt->clock_rate, pt->normal_bitrate/1000);
			pt->normal_bitrate=get_min_bandwidth(pt->normal_bitrate,bw*1000);
		} else pt->normal_bitrate=bw*1000;
		if (desc->ptime>0){
			up_ptime=desc->ptime;
		}
		if (up_ptime>0){
			char tmp[40];
			snprintf(tmp,sizeof(tmp),"ptime=%i",up_ptime);
			payload_type_append_send_fmtp(pt,tmp);
		}
		number=payload_type_get_number(pt);
		if (rtp_profile_get_payload(prof,number)!=NULL){
			ms_warning("A payload type with number %i already exists in profile !",number);
		}else
			rtp_profile_set_payload(prof,number,pt);
	}
	return prof;
}


static void setup_ring_player(LinphoneCore *lc, LinphoneCall *call){
	int pause_time=3000;
	audio_stream_play(call->audiostream,lc->sound_conf.ringback_tone);
	ms_filter_call_method(call->audiostream->soundread,MS_FILE_PLAYER_LOOP,&pause_time);
}

static bool_t linphone_call_sound_resources_available(LinphoneCall *call){
	LinphoneCore *lc=call->core;
	LinphoneCall *current=linphone_core_get_current_call(lc);
	return !linphone_core_is_in_conference(lc) &&
		(current==NULL || current==call);
}

static int find_crypto_index_from_tag(const SalSrtpCryptoAlgo crypto[],unsigned char tag) {
	int i;
	for(i=0; i<SAL_CRYPTO_ALGO_MAX; i++) {
		if (crypto[i].tag == tag) {
			return i;
		}
	}
	return -1;
}

static void configure_rtp_session_for_rtcp_fb(LinphoneCall *call, const SalStreamDescription *stream) {
	RtpSession *session = NULL;

	if (stream->type == SalAudio) {
		session = call->audiostream->ms.sessions.rtp_session;
	} else if (stream->type == SalVideo) {
		session = call->videostream->ms.sessions.rtp_session;
	} else {
		// Do nothing for streams that are not audio or video
		return;
	}
	if (stream->rtcp_fb.generic_nack_enabled)
		rtp_session_enable_avpf_feature(session, ORTP_AVPF_FEATURE_GENERIC_NACK, TRUE);
	else
		rtp_session_enable_avpf_feature(session, ORTP_AVPF_FEATURE_GENERIC_NACK, FALSE);
	if (stream->rtcp_fb.tmmbr_enabled)
		rtp_session_enable_avpf_feature(session, ORTP_AVPF_FEATURE_TMMBR, TRUE);
	else
		rtp_session_enable_avpf_feature(session, ORTP_AVPF_FEATURE_TMMBR, FALSE);
}

static void configure_rtp_session_for_rtcp_xr(LinphoneCore *lc, LinphoneCall *call, SalStreamType type) {
	RtpSession *session;
	const OrtpRtcpXrConfiguration *localconfig;
	const OrtpRtcpXrConfiguration *remoteconfig;
	OrtpRtcpXrConfiguration currentconfig;
	const SalStreamDescription *localstream;
	const SalStreamDescription *remotestream;
	SalMediaDescription *remotedesc = sal_call_get_remote_media_description(call->op);

	if (!remotedesc) return;

	localstream = sal_media_description_find_best_stream(call->localdesc, type);
	if (!localstream) return;
	localconfig = &localstream->rtcp_xr;
	remotestream = sal_media_description_find_best_stream(remotedesc, type);
	if (!remotestream) return;
	remoteconfig = &remotestream->rtcp_xr;

	if (localstream->dir == SalStreamInactive) return;
	else if (localstream->dir == SalStreamRecvOnly) {
		/* Use local config for unilateral parameters and remote config for collaborative parameters. */
		memcpy(&currentconfig, localconfig, sizeof(currentconfig));
		currentconfig.rcvr_rtt_mode = remoteconfig->rcvr_rtt_mode;
		currentconfig.rcvr_rtt_max_size = remoteconfig->rcvr_rtt_max_size;
	} else {
		memcpy(&currentconfig, remoteconfig, sizeof(currentconfig));
	}
	if (type == SalAudio) {
		session = call->audiostream->ms.sessions.rtp_session;
	} else {
		session = call->videostream->ms.sessions.rtp_session;
	}
	rtp_session_configure_rtcp_xr(session, &currentconfig);
}
void static start_dtls( MSMediaStreamSessions *sessions,  const SalStreamDescription *sd,const SalStreamDescription *remote) {
	if (sal_stream_description_has_dtls(sd) == TRUE) {
		/*DTLS*/
		SalDtlsRole salRole = sd->dtls_role;
		if (salRole!=SalDtlsRoleInvalid) { /* if DTLS is available at both end points */
			/* give the peer certificate fingerprint to dtls context */
			ms_dtls_srtp_set_peer_fingerprint(sessions->dtls_context, remote->dtls_fingerprint);
			ms_dtls_srtp_set_role(sessions->dtls_context, (salRole == SalDtlsRoleIsClient)?MSDtlsSrtpRoleIsClient:MSDtlsSrtpRoleIsServer); /* set the role to client */
			ms_dtls_srtp_start(sessions->dtls_context);  /* then start the engine, it will send the DTLS client Hello */
		} else {
			ms_warning("unable to start DTLS engine on stream session [%p], Dtls role in resulting media description is invalid",sessions);
		}
	}
}
void static start_dtls_on_all_streams(LinphoneCall *call) {
	SalMediaDescription *remote_desc = sal_call_get_remote_media_description(call->op);
	SalMediaDescription *result_desc = sal_call_get_final_media_description(call->op);
	if( remote_desc == NULL || result_desc == NULL ){
		/* this can happen in some tricky cases (early-media without SDP in the 200). In that case, simply skip DTLS code */
		return;
	}

	if (call->audiostream && (media_stream_get_state((const MediaStream *)call->audiostream) == MSStreamStarted))/*dtls must start at the end of ice*/
			start_dtls(&call->audiostream->ms.sessions
							,sal_media_description_find_best_stream(result_desc,SalAudio)
							,sal_media_description_find_best_stream(remote_desc,SalAudio));
#if VIDEO_ENABLED
	if (call->videostream && (media_stream_get_state((const MediaStream *)call->videostream) == MSStreamStarted))/*dtls must start at the end of ice*/
			start_dtls(&call->videostream->ms.sessions
						,sal_media_description_find_best_stream(result_desc,SalVideo)
						,sal_media_description_find_best_stream(remote_desc,SalVideo));
#endif
	return;
}

void static set_dtls_fingerprint( MSMediaStreamSessions *sessions,  const SalStreamDescription *sd,const SalStreamDescription *remote) {
	if (sal_stream_description_has_dtls(sd) == TRUE) {
		/*DTLS*/
		SalDtlsRole salRole = sd->dtls_role;
		if (salRole!=SalDtlsRoleInvalid) { /* if DTLS is available at both end points */
			/* give the peer certificate fingerprint to dtls context */
			ms_dtls_srtp_set_peer_fingerprint(sessions->dtls_context, remote->dtls_fingerprint);
		} else {
			ms_warning("unable to start DTLS engine on stream session [%p], Dtls role in resulting media description is invalid",sessions);
		}
	}
}

void static set_dtls_fingerprint_on_all_streams(LinphoneCall *call) {
	SalMediaDescription *remote_desc = sal_call_get_remote_media_description(call->op);
	SalMediaDescription *result_desc = sal_call_get_final_media_description(call->op);

	if( remote_desc == NULL || result_desc == NULL ){
		/* this can happen in some tricky cases (early-media without SDP in the 200). In that case, simply skip DTLS code */
		return;
	}

	if (call->audiostream && (media_stream_get_state((const MediaStream *)call->audiostream) == MSStreamStarted))/*dtls must start at the end of ice*/
			set_dtls_fingerprint(&call->audiostream->ms.sessions
							,sal_media_description_find_best_stream(result_desc,SalAudio)
							,sal_media_description_find_best_stream(remote_desc,SalAudio));
#if VIDEO_ENABLED
	if (call->videostream && (media_stream_get_state((const MediaStream *)call->videostream) == MSStreamStarted))/*dtls must start at the end of ice*/
			set_dtls_fingerprint(&call->videostream->ms.sessions
						,sal_media_description_find_best_stream(result_desc,SalVideo)
						,sal_media_description_find_best_stream(remote_desc,SalVideo));
#endif
	return;
}

static RtpSession * create_audio_rtp_io_session(LinphoneCall *call) {
	PayloadType *pt;
	LinphoneCore *lc = call->core;
	const char *local_ip = lp_config_get_string(lc->config, "sound", "rtp_local_addr", "127.0.0.1");
	const char *remote_ip = lp_config_get_string(lc->config, "sound", "rtp_remote_addr", "127.0.0.1");
	int local_port = lp_config_get_int(lc->config, "sound", "rtp_local_port", 17076);
	int remote_port = lp_config_get_int(lc->config, "sound", "rtp_remote_port", 17078);
	int ptnum = lp_config_get_int(lc->config, "sound", "rtp_ptnum", 0);
	const char *rtpmap = lp_config_get_string(lc->config, "sound", "rtp_map", "pcmu/8000/1");
	int symmetric = lp_config_get_int(lc->config, "sound", "rtp_symmetric", 0);
	RtpSession *rtp_session = NULL;
	pt = rtp_profile_get_payload_from_rtpmap(call->audio_profile, rtpmap);
	if (pt != NULL) {
		call->rtp_io_audio_profile = rtp_profile_new("RTP IO audio profile");
		rtp_profile_set_payload(call->rtp_io_audio_profile, ptnum, payload_type_clone(pt));
		rtp_session = ms_create_duplex_rtp_session(local_ip, local_port, -1);
		rtp_session_set_profile(rtp_session, call->rtp_io_audio_profile);
		rtp_session_set_remote_addr_and_port(rtp_session, remote_ip, remote_port, -1);
		rtp_session_enable_rtcp(rtp_session, FALSE);
		rtp_session_set_payload_type(rtp_session, ptnum);
		rtp_session_set_jitter_compensation(rtp_session, linphone_core_get_audio_jittcomp(lc));
		rtp_session_set_symmetric_rtp(rtp_session, (bool_t)symmetric);
	}
	return rtp_session;
}

static void linphone_call_start_audio_stream(LinphoneCall *call, bool_t muted, bool_t send_ringbacktone, bool_t use_arc){
	LinphoneCore *lc=call->core;
	LpConfig* conf;
	int used_pt=-1;
	char rtcp_tool[128]={0};
	const SalStreamDescription *stream;
	MSSndCard *playcard;
	MSSndCard *captcard;
	bool_t use_ec;
	bool_t mute;
	const char *playfile;
	const char *recfile;
	const SalStreamDescription *local_st_desc;
	int crypto_idx;
	MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;
	bool_t use_rtp_io = lp_config_get_int(lc->config, "sound", "rtp_io", FALSE);

	snprintf(rtcp_tool,sizeof(rtcp_tool)-1,"%s-%s",linphone_core_get_user_agent_name(),linphone_core_get_user_agent_version());

	stream = sal_media_description_find_best_stream(call->resultdesc, SalAudio);
	if (stream && stream->dir!=SalStreamInactive && stream->rtp_port!=0){
		const char *rtp_addr=stream->rtp_addr[0]!='\0' ? stream->rtp_addr : call->resultdesc->addr;
		bool_t is_multicast=ms_is_multicast(rtp_addr);
		playcard=lc->sound_conf.lsd_card ?
			lc->sound_conf.lsd_card : lc->sound_conf.play_sndcard;
		captcard=lc->sound_conf.capt_sndcard;
		playfile=lc->play_file;
		recfile=lc->rec_file;
		call->audio_profile=make_profile(call,call->resultdesc,stream,&used_pt);

		if (used_pt!=-1){
			bool_t ok = TRUE;
			call->current_params->audio_codec = rtp_profile_get_payload(call->audio_profile, used_pt);
			if (playcard==NULL) {
				ms_warning("No card defined for playback !");
			}
			if (captcard==NULL) {
				ms_warning("No card defined for capture !");
			}
			/*Replace soundcard filters by inactive file players or recorders
			 when placed in recvonly or sendonly mode*/
			if (stream->rtp_port==0
					|| stream->dir==SalStreamRecvOnly
					|| (stream->multicast_role == SalMulticastReceiver && is_multicast)){
				captcard=NULL;
				playfile=NULL;
			}else if (stream->dir==SalStreamSendOnly){
				playcard=NULL;
				/*jehan: why capture card should be null in this case ? Not very good to only rely on stream dir to detect paused state.
				 * It can also be a simple call in one way audio*/
				captcard=NULL;
				recfile=NULL;
				/*And we will eventually play "playfile" if set by the user*/
			}
			if (send_ringbacktone){
				conf = linphone_core_get_config(lc);
				captcard=NULL;
				playfile=NULL;/* it is setup later*/
				if( conf && lp_config_get_int(conf,"sound","send_ringback_without_playback", 0) == 1){
					playcard = NULL;
					recfile = NULL;
				}
			}
			/*if playfile are supplied don't use soundcards*/
			if (lc->use_files || use_rtp_io) {
				captcard=NULL;
				playcard=NULL;
			}
			if (call->params->in_conference){
				/* first create the graph without soundcard resources*/
				captcard=playcard=NULL;
			}
			if (!linphone_call_sound_resources_available(call)){
				ms_message("Sound resources are used by another call, not using soundcard.");
				captcard=playcard=NULL;
			}
			use_ec=captcard==NULL ? FALSE : linphone_core_echo_cancellation_enabled(lc);
			audio_stream_enable_echo_canceller(call->audiostream, use_ec);
			if (playcard &&  stream->max_rate>0) ms_snd_card_set_preferred_sample_rate(playcard, stream->max_rate);
			if (captcard &&  stream->max_rate>0) ms_snd_card_set_preferred_sample_rate(captcard, stream->max_rate);
			audio_stream_enable_adaptive_bitrate_control(call->audiostream,use_arc);
			media_stream_set_adaptive_bitrate_algorithm(&call->audiostream->ms,
													  ms_qos_analyzer_algorithm_from_string(linphone_core_get_adaptive_rate_algorithm(lc)));
			audio_stream_enable_adaptive_jittcomp(call->audiostream, linphone_core_audio_adaptive_jittcomp_enabled(lc));
			rtp_session_set_jitter_compensation(call->audiostream->ms.sessions.rtp_session,linphone_core_get_audio_jittcomp(lc));
			if (!call->params->in_conference && call->params->record_file){
				audio_stream_mixed_record_open(call->audiostream,call->params->record_file);
				call->current_params->record_file=ms_strdup(call->params->record_file);
			}
			/* valid local tags are > 0 */
			if (sal_stream_description_has_srtp(stream) == TRUE) {
				local_st_desc=sal_media_description_find_stream(call->localdesc,stream->proto,SalAudio);
				crypto_idx = find_crypto_index_from_tag(local_st_desc->crypto, stream->crypto_local_tag);

				if (crypto_idx >= 0) {
					ms_media_stream_sessions_set_srtp_recv_key_b64(&call->audiostream->ms.sessions, stream->crypto[0].algo,stream->crypto[0].master_key);
					ms_media_stream_sessions_set_srtp_send_key_b64(&call->audiostream->ms.sessions, stream->crypto[0].algo,local_st_desc->crypto[crypto_idx].master_key);
				} else {
					ms_warning("Failed to find local crypto algo with tag: %d", stream->crypto_local_tag);
				}
			}
			configure_rtp_session_for_rtcp_fb(call, stream);
			configure_rtp_session_for_rtcp_xr(lc, call, SalAudio);
			if (is_multicast)
				rtp_session_set_multicast_ttl(call->audiostream->ms.sessions.rtp_session,stream->ttl);

			if (use_rtp_io) {
				io.input.type = io.output.type = MSResourceRtp;
				io.input.session = io.output.session = create_audio_rtp_io_session(call);
				if (io.input.session == NULL) {
					ok = FALSE;
				}
			}else {
				if (playcard){
					io.output.type = MSResourceSoundcard;
					io.output.soundcard = playcard;
				}else{
					io.output.type = MSResourceFile;
					io.output.file = recfile;
				}
				if (captcard){
					io.input.type = MSResourceSoundcard;
					io.input.soundcard = captcard;
				}else{
					io.input.type = MSResourceFile;
					io.input.file = playfile;
				}
				
			}
			if (ok == TRUE) {
				audio_stream_start_from_io(call->audiostream,
					call->audio_profile,
					rtp_addr,
					stream->rtp_port,
					stream->rtcp_addr[0]!='\0' ? stream->rtcp_addr : call->resultdesc->addr,
					(linphone_core_rtcp_enabled(lc) && !is_multicast) ? (stream->rtcp_port ? stream->rtcp_port : stream->rtp_port+1) : 0,
					used_pt,
					&io
				);
				post_configure_audio_streams(call, muted && !send_ringbacktone);
			}

			ms_media_stream_sessions_set_encryption_mandatory(&call->audiostream->ms.sessions,linphone_core_is_media_encryption_mandatory(call->core));

			if (stream->dir==SalStreamSendOnly && playfile!=NULL){
				int pause_time=500;
				ms_filter_call_method(call->audiostream->soundread,MS_FILE_PLAYER_LOOP,&pause_time);
			}
			if (send_ringbacktone){
				setup_ring_player(lc,call);
			}

			if (call->params->in_conference){
				/*transform the graph to connect it to the conference filter */
				mute=stream->dir==SalStreamRecvOnly;
				linphone_call_add_to_conf(call, mute);
			}
			call->current_params->in_conference=call->params->in_conference;
			call->current_params->low_bandwidth=call->params->low_bandwidth;
		}else ms_warning("No audio stream accepted ?");
	}
}

#ifdef VIDEO_ENABLED
static RtpSession * create_video_rtp_io_session(LinphoneCall *call) {
	PayloadType *pt;
	LinphoneCore *lc = call->core;
	const char *local_ip = lp_config_get_string(lc->config, "video", "rtp_local_addr", "127.0.0.1");
	const char *remote_ip = lp_config_get_string(lc->config, "video", "rtp_remote_addr", "127.0.0.1");
	int local_port = lp_config_get_int(lc->config, "video", "rtp_local_port", 19076);
	int remote_port = lp_config_get_int(lc->config, "video", "rtp_remote_port", 19078);
	int ptnum = lp_config_get_int(lc->config, "video", "rtp_ptnum", 0);
	const char *rtpmap = lp_config_get_string(lc->config, "video", "rtp_map", "vp8/90000/1");
	int symmetric = lp_config_get_int(lc->config, "video", "rtp_symmetric", 0);
	RtpSession *rtp_session = NULL;
	pt = rtp_profile_get_payload_from_rtpmap(call->video_profile, rtpmap);
	if (pt != NULL) {
		call->rtp_io_video_profile = rtp_profile_new("RTP IO video profile");
		rtp_profile_set_payload(call->rtp_io_video_profile, ptnum, payload_type_clone(pt));
		rtp_session = ms_create_duplex_rtp_session(local_ip, local_port, -1);
		rtp_session_set_profile(rtp_session, call->rtp_io_video_profile);
		rtp_session_set_remote_addr_and_port(rtp_session, remote_ip, remote_port, -1);
		rtp_session_enable_rtcp(rtp_session, FALSE);
		rtp_session_set_payload_type(rtp_session, ptnum);
		rtp_session_set_symmetric_rtp(rtp_session, (bool_t)symmetric);
	}
	return rtp_session;
}
#endif

static void linphone_call_start_video_stream(LinphoneCall *call, bool_t all_inputs_muted){
#ifdef VIDEO_ENABLED
	LinphoneCore *lc=call->core;
	int used_pt=-1;
	const SalStreamDescription *vstream;
	MSFilter* source = NULL;
	bool_t reused_preview = FALSE;
	bool_t use_rtp_io = lp_config_get_int(lc->config, "video", "rtp_io", FALSE);
	MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;

	/* shutdown preview */
	if (lc->previewstream!=NULL) {
		if( lc->video_conf.reuse_preview_source == FALSE) video_preview_stop(lc->previewstream);
		else source = video_preview_stop_reuse_source(lc->previewstream);
		lc->previewstream=NULL;
	}

	vstream = sal_media_description_find_best_stream(call->resultdesc, SalVideo);
	if (vstream!=NULL && vstream->dir!=SalStreamInactive && vstream->rtp_port!=0) {
		const char *rtp_addr=vstream->rtp_addr[0]!='\0' ? vstream->rtp_addr : call->resultdesc->addr;
		const char *rtcp_addr=vstream->rtcp_addr[0]!='\0' ? vstream->rtcp_addr : call->resultdesc->addr;
		const SalStreamDescription *local_st_desc=sal_media_description_find_stream(call->localdesc,vstream->proto,SalVideo);
		bool_t is_multicast=ms_is_multicast(rtp_addr);
		call->video_profile=make_profile(call,call->resultdesc,vstream,&used_pt);

		if (used_pt!=-1){
			MediaStreamDir dir= MediaStreamSendRecv;
			bool_t is_inactive=FALSE;
			MSWebCam *cam;
			
			call->current_params->video_codec = rtp_profile_get_payload(call->video_profile, used_pt);
			call->current_params->has_video=TRUE;

			video_stream_enable_adaptive_bitrate_control(call->videostream,
													  linphone_core_adaptive_rate_control_enabled(lc));
			media_stream_set_adaptive_bitrate_algorithm(&call->videostream->ms,
													  ms_qos_analyzer_algorithm_from_string(linphone_core_get_adaptive_rate_algorithm(lc)));
			video_stream_enable_adaptive_jittcomp(call->videostream, linphone_core_video_adaptive_jittcomp_enabled(lc));
			rtp_session_set_jitter_compensation(call->videostream->ms.sessions.rtp_session, linphone_core_get_video_jittcomp(lc));
			if (lc->video_conf.preview_vsize.width!=0)
				video_stream_set_preview_size(call->videostream,lc->video_conf.preview_vsize);
			video_stream_set_fps(call->videostream,linphone_core_get_preferred_framerate(lc));
			video_stream_set_sent_video_size(call->videostream,linphone_core_get_preferred_video_size(lc));
			video_stream_enable_self_view(call->videostream,lc->video_conf.selfview);
			if (call->video_window_id != NULL)
				video_stream_set_native_window_id(call->videostream, call->video_window_id);
			else if (lc->video_window_id != NULL)
				video_stream_set_native_window_id(call->videostream, lc->video_window_id);
			if (lc->preview_window_id != NULL)
				video_stream_set_native_preview_window_id(call->videostream, lc->preview_window_id);
			video_stream_use_preview_video_window (call->videostream,lc->use_preview_window);

			if (is_multicast){
				if (vstream->multicast_role == SalMulticastReceiver)
					dir=MediaStreamRecvOnly;
				else
					dir=MediaStreamSendOnly;
			} else if (vstream->dir==SalStreamSendOnly && lc->video_conf.capture ){
				dir=MediaStreamSendOnly;
			}else if (vstream->dir==SalStreamRecvOnly && lc->video_conf.display ){
				dir=MediaStreamRecvOnly;
			}else if (vstream->dir==SalStreamSendRecv){
				if (lc->video_conf.display && lc->video_conf.capture)
					dir=MediaStreamSendRecv;
				else if (lc->video_conf.display)
					dir=MediaStreamRecvOnly;
				else
					dir=MediaStreamSendOnly;
			}else{
				ms_warning("video stream is inactive.");
				/*either inactive or incompatible with local capabilities*/
				is_inactive=TRUE;
			}
			if (all_inputs_muted){
				cam=get_nowebcam_device();
			} else {
				cam = linphone_call_get_video_device(call);
			}
			if (!is_inactive){
				if (sal_stream_description_has_srtp(vstream) == TRUE) {
					int crypto_idx = find_crypto_index_from_tag(local_st_desc->crypto, vstream->crypto_local_tag);
					if (crypto_idx >= 0) {
						ms_media_stream_sessions_set_srtp_recv_key_b64(&call->videostream->ms.sessions, vstream->crypto[0].algo,vstream->crypto[0].master_key);
						ms_media_stream_sessions_set_srtp_send_key_b64(&call->videostream->ms.sessions, vstream->crypto[0].algo,local_st_desc->crypto[crypto_idx].master_key);
					}
				}
				configure_rtp_session_for_rtcp_fb(call, vstream);
				configure_rtp_session_for_rtcp_xr(lc, call, SalVideo);

				call->log->video_enabled = TRUE;
				video_stream_set_direction (call->videostream, dir);
				ms_message("%s lc rotation:%d\n", __FUNCTION__, lc->device_rotation);
				video_stream_set_device_rotation(call->videostream, lc->device_rotation);
				video_stream_set_freeze_on_error(call->videostream, lp_config_get_int(lc->config, "video", "freeze_on_error", 0));
				if (is_multicast)
					rtp_session_set_multicast_ttl(call->videostream->ms.sessions.rtp_session,vstream->ttl);

				video_stream_use_video_preset(call->videostream, lp_config_get_string(lc->config, "video", "preset", NULL));
				if (lc->video_conf.reuse_preview_source && source) {
					ms_message("video_stream_start_with_source kept: %p", source);
					video_stream_start_with_source(call->videostream,
												   call->video_profile, rtp_addr, vstream->rtp_port,
												   rtcp_addr,
												   linphone_core_rtcp_enabled(lc) ? (vstream->rtcp_port ? vstream->rtcp_port : vstream->rtp_port+1) : 0,
												   used_pt, linphone_core_get_video_jittcomp(lc), cam, source);
					reused_preview = TRUE;
				} else {
					bool_t ok = TRUE;
					if (use_rtp_io) {
						io.input.type = io.output.type = MSResourceRtp;
						io.input.session = io.output.session = create_video_rtp_io_session(call);
						if (io.input.session == NULL) {
							ok = FALSE;
							ms_warning("Cannot create video RTP IO session");
						}
					} else {
						io.input.type = MSResourceCamera;
						io.input.camera = cam;
						io.output.type = MSResourceDefault;
					}
					if (ok) {
						video_stream_start_from_io(call->videostream,
							call->video_profile, rtp_addr, vstream->rtp_port,
							rtcp_addr,
							(linphone_core_rtcp_enabled(lc) && !is_multicast)  ? (vstream->rtcp_port ? vstream->rtcp_port : vstream->rtp_port+1) : 0,
							used_pt, &io);
					}
				}
				ms_media_stream_sessions_set_encryption_mandatory(&call->videostream->ms.sessions,linphone_core_is_media_encryption_mandatory(call->core));
			}
		}else ms_warning("No video stream accepted.");
	}else{
		ms_message("No valid video stream defined.");
	}
	if( reused_preview == FALSE && source != NULL ){
		/* destroy not-reused source filter */
		ms_warning("Video preview (%p) not reused: destroying it.", source);
		ms_filter_destroy(source);
	}
#endif
}

static void setZrtpCryptoTypesParameters(MSZrtpParams *params, LinphoneCore *lc)
{
	int i;
	const MSCryptoSuite *srtp_suites;
	MsZrtpCryptoTypesCount ciphersCount, authTagsCount;

	if (params == NULL) return;
	if (lc == NULL) return;

	srtp_suites = linphone_core_get_srtp_crypto_suites(lc);
	if (srtp_suites!=NULL) {
		for(i=0; srtp_suites[i]!=MS_CRYPTO_SUITE_INVALID && i<SAL_CRYPTO_ALGO_MAX && i<MS_MAX_ZRTP_CRYPTO_TYPES; ++i){
			switch (srtp_suites[i]) {
				case MS_AES_128_SHA1_32:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS32;
					break;
				case MS_AES_128_NO_AUTH:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					break;
				case MS_NO_CIPHER_SHA1_80:
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_AES_128_SHA1_80:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES1;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_AES_256_SHA1_80:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_AES_256_SHA1_32:
					params->ciphers[params->ciphersCount++] = MS_ZRTP_CIPHER_AES3;
					params->authTags[params->authTagsCount++] = MS_ZRTP_AUTHTAG_HS80;
					break;
				case MS_CRYPTO_SUITE_INVALID:
					break;
			}
		}
	}

	/* linphone_core_get_srtp_crypto_suites is used to determine sensible defaults; here each can be overridden */
	ciphersCount = linphone_core_get_zrtp_cipher_suites(lc, params->ciphers); /* if not present in config file, params->ciphers is not modified */
	if (ciphersCount!=0) { /* use zrtp_cipher_suites config only when present, keep config from srtp_crypto_suite otherwise */
		params->ciphersCount = ciphersCount;
	}
	params->hashesCount = linphone_core_get_zrtp_hash_suites(lc, params->hashes);
	authTagsCount = linphone_core_get_zrtp_auth_suites(lc, params->authTags); /* if not present in config file, params->authTags is not modified */
	if (authTagsCount!=0) {
		params->authTagsCount = authTagsCount; /* use zrtp_auth_suites config only when present, keep config from srtp_crypto_suite otherwise */
	}
	params->sasTypesCount = linphone_core_get_zrtp_sas_suites(lc, params->sasTypes);
	params->keyAgreementsCount = linphone_core_get_zrtp_key_agreement_suites(lc, params->keyAgreements);
}

void linphone_call_start_media_streams(LinphoneCall *call, bool_t all_inputs_muted, bool_t send_ringbacktone){
	LinphoneCore *lc=call->core;
	bool_t use_arc=linphone_core_adaptive_rate_control_enabled(lc);
#ifdef VIDEO_ENABLED
	const SalStreamDescription *vstream=sal_media_description_find_best_stream(call->resultdesc,SalVideo);
#endif

	call->current_params->audio_codec = NULL;
	call->current_params->video_codec = NULL;

	if ((call->audiostream == NULL) && (call->videostream == NULL)) {
		ms_fatal("start_media_stream() called without prior init !");
		return;
	}
#if defined(VIDEO_ENABLED)
	if (vstream!=NULL && vstream->dir!=SalStreamInactive && vstream->payloads!=NULL){
		/*when video is used, do not make adaptive rate control on audio, it is stupid.*/
		use_arc=FALSE;
	}
#endif
	ms_message("linphone_call_start_media_streams() call=[%p] local upload_bandwidth=[%i] kbit/s; local download_bandwidth=[%i] kbit/s",
		   call, linphone_core_get_upload_bandwidth(lc),linphone_core_get_download_bandwidth(lc));

	if (call->audiostream!=NULL) {
		linphone_call_start_audio_stream(call,all_inputs_muted||call->audio_muted,send_ringbacktone,use_arc);
	} else {
		ms_warning("DTLS no audio stream!");
	}
	call->current_params->has_video=FALSE;
	if (call->videostream!=NULL) {
		if (call->audiostream) audio_stream_link_video(call->audiostream,call->videostream);
		linphone_call_start_video_stream(call,all_inputs_muted);
	}

	call->all_muted=all_inputs_muted;
	call->playing_ringbacktone=send_ringbacktone;
	call->up_bw=linphone_core_get_upload_bandwidth(lc);

	/*might be moved in audio/video stream_start*/
	if (call->params->media_encryption==LinphoneMediaEncryptionZRTP) {
		MSZrtpParams params;
		memset(&params,0,sizeof(MSZrtpParams));
		/*call->current_params.media_encryption will be set later when zrtp is activated*/
		params.zid_file=lc->zrtp_secrets_cache;
		params.uri= linphone_address_as_string_uri_only((call->dir==LinphoneCallIncoming) ? call->log->from : call->log->to);
		setZrtpCryptoTypesParameters(&params,call->core);
		audio_stream_enable_zrtp(call->audiostream,&params);
#if VIDEO_ENABLED
		if (media_stream_secured((MediaStream *)call->audiostream) && media_stream_get_state((MediaStream *)call->videostream) == MSStreamStarted) {
			/*audio stream is already encrypted and video stream is active*/
			memset(&params,0,sizeof(MSZrtpParams));
			video_stream_enable_zrtp(call->videostream,call->audiostream,&params);
		}
#endif
	}

	set_dtls_fingerprint_on_all_streams(call);

	if ((call->ice_session != NULL) && (ice_session_state(call->ice_session) != IS_Completed)) {
		ice_session_start_connectivity_checks(call->ice_session);
	} else {
		/*should not start dtls until ice is completed*/
		start_dtls_on_all_streams(call);
	}

}

void linphone_call_stop_media_streams_for_ice_gathering(LinphoneCall *call){
	if(call->audiostream && call->audiostream->ms.state==MSStreamPreparing) audio_stream_unprepare_sound(call->audiostream);
#ifdef VIDEO_ENABLED
	if (call->videostream && call->videostream->ms.state==MSStreamPreparing) {
		video_stream_unprepare_video(call->videostream);
	}
#endif
}

static bool_t update_stream_crypto_params(LinphoneCall *call, const SalStreamDescription *local_st_desc, SalStreamDescription *old_stream, SalStreamDescription *new_stream, MediaStream *ms){
	int crypto_idx = find_crypto_index_from_tag(local_st_desc->crypto, new_stream->crypto_local_tag);
	if (crypto_idx >= 0) {
		if (call->localdesc_changed & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED)
			ms_media_stream_sessions_set_srtp_send_key_b64(&ms->sessions, new_stream->crypto[0].algo,local_st_desc->crypto[crypto_idx].master_key);
		if (strcmp(old_stream->crypto[0].master_key,new_stream->crypto[0].master_key)!=0){
			ms_media_stream_sessions_set_srtp_recv_key_b64(&ms->sessions, new_stream->crypto[0].algo,new_stream->crypto[0].master_key);
		}
		return TRUE;
	} else {
		ms_warning("Failed to find local crypto algo with tag: %d", new_stream->crypto_local_tag);
	}
	return FALSE;
}

void linphone_call_update_crypto_parameters(LinphoneCall *call, SalMediaDescription *old_md, SalMediaDescription *new_md) {
	SalStreamDescription *old_stream;
	SalStreamDescription *new_stream;
	const SalStreamDescription *local_st_desc;

	local_st_desc = sal_media_description_find_secure_stream_of_type(call->localdesc, SalAudio);
	old_stream = sal_media_description_find_secure_stream_of_type(old_md, SalAudio);
	new_stream = sal_media_description_find_secure_stream_of_type(new_md, SalAudio);
	if (call->audiostream && local_st_desc && old_stream && new_stream &&
		update_stream_crypto_params(call,local_st_desc,old_stream,new_stream,&call->audiostream->ms)){
	}

	start_dtls_on_all_streams(call);

#ifdef VIDEO_ENABLED
	local_st_desc = sal_media_description_find_secure_stream_of_type(call->localdesc, SalVideo);
	old_stream = sal_media_description_find_secure_stream_of_type(old_md, SalVideo);
	new_stream = sal_media_description_find_secure_stream_of_type(new_md, SalVideo);
	if (call->videostream && local_st_desc && old_stream && new_stream &&
		update_stream_crypto_params(call,local_st_desc,old_stream,new_stream,&call->videostream->ms)){
	}
#endif
}

void linphone_call_update_remote_session_id_and_ver(LinphoneCall *call) {
	SalMediaDescription *remote_desc = sal_call_get_remote_media_description(call->op);
	if (remote_desc) {
		call->remote_session_id = remote_desc->session_id;
		call->remote_session_ver = remote_desc->session_ver;
	}
}

void linphone_call_delete_ice_session(LinphoneCall *call){
	if (call->ice_session != NULL) {
		ice_session_destroy(call->ice_session);
		call->ice_session = NULL;
		if (call->audiostream != NULL) call->audiostream->ms.ice_check_list = NULL;
		if (call->videostream != NULL) call->videostream->ms.ice_check_list = NULL;
		call->stats[LINPHONE_CALL_STATS_AUDIO].ice_state = LinphoneIceStateNotActivated;
		call->stats[LINPHONE_CALL_STATS_VIDEO].ice_state = LinphoneIceStateNotActivated;
	}
}


void linphone_call_delete_upnp_session(LinphoneCall *call){
#ifdef BUILD_UPNP
	if(call->upnp_session!=NULL) {
		linphone_upnp_session_destroy(call->upnp_session);
		call->upnp_session=NULL;
	}
#endif //BUILD_UPNP
}


static void linphone_call_log_fill_stats(LinphoneCallLog *log, MediaStream *st){
	float quality=media_stream_get_average_quality_rating(st);
	if (quality>=0){
		if (log->quality!=-1){
			log->quality*=quality/5.0;
		}else log->quality=quality;
	}
}

static void linphone_call_stop_audio_stream(LinphoneCall *call) {
	if (call->audiostream!=NULL) {
		linphone_reporting_update_media_info(call, LINPHONE_CALL_STATS_AUDIO);
		media_stream_reclaim_sessions(&call->audiostream->ms,&call->sessions[0]);

		if (call->audiostream->ec){
			const char *state_str=NULL;
			ms_filter_call_method(call->audiostream->ec,MS_ECHO_CANCELLER_GET_STATE_STRING,&state_str);
			if (state_str){
				ms_message("Writing echo canceler state, %i bytes",(int)strlen(state_str));
				lp_config_write_relative_file(call->core->config, EC_STATE_STORE, state_str);
			}
		}
		audio_stream_get_local_rtp_stats(call->audiostream,&call->log->local_stats);
		linphone_call_log_fill_stats (call->log,(MediaStream*)call->audiostream);
		if (call->endpoint){
			linphone_call_remove_from_conf(call);
		}
		audio_stream_stop(call->audiostream);
		rtp_session_unregister_event_queue(call->sessions[0].rtp_session, call->audiostream_app_evq);
		ortp_ev_queue_flush(call->audiostream_app_evq);
		ortp_ev_queue_destroy(call->audiostream_app_evq);
		call->audiostream_app_evq=NULL;
		call->audiostream=NULL;
		call->current_params->audio_codec = NULL;
	}
}

static void linphone_call_stop_video_stream(LinphoneCall *call) {
#ifdef VIDEO_ENABLED
	if (call->videostream!=NULL){
		linphone_reporting_update_media_info(call, LINPHONE_CALL_STATS_VIDEO);
		media_stream_reclaim_sessions(&call->videostream->ms,&call->sessions[1]);
		linphone_call_log_fill_stats(call->log,(MediaStream*)call->videostream);
		video_stream_stop(call->videostream);
		call->videostream=NULL;
		rtp_session_unregister_event_queue(call->sessions[1].rtp_session, call->videostream_app_evq);
		ortp_ev_queue_flush(call->videostream_app_evq);
		ortp_ev_queue_destroy(call->videostream_app_evq);
		call->videostream_app_evq=NULL;
		call->current_params->video_codec = NULL;
	}
#endif
}

static void unset_rtp_profile(LinphoneCall *call, int i){
	if (call->sessions[i].rtp_session)
		rtp_session_set_profile(call->sessions[i].rtp_session,&av_profile);
}

void linphone_call_stop_media_streams(LinphoneCall *call){
	if (call->audiostream || call->videostream) {
		if (call->audiostream && call->videostream)
			audio_stream_unlink_video(call->audiostream, call->videostream);
		linphone_call_stop_audio_stream(call);
		linphone_call_stop_video_stream(call);

		if (call->core->msevq != NULL) {
			ms_event_queue_skip(call->core->msevq);
		}
	}

	if (call->audio_profile){
		rtp_profile_destroy(call->audio_profile);
		call->audio_profile=NULL;
		unset_rtp_profile(call,0);
	}
	if (call->video_profile){
		rtp_profile_destroy(call->video_profile);
		call->video_profile=NULL;
		unset_rtp_profile(call,1);
	}
	if (call->rtp_io_audio_profile) {
		rtp_profile_destroy(call->rtp_io_audio_profile);
		call->rtp_io_audio_profile = NULL;
	}
	if (call->rtp_io_video_profile) {
		rtp_profile_destroy(call->rtp_io_video_profile);
		call->rtp_io_video_profile = NULL;
	}
}



void linphone_call_enable_echo_cancellation(LinphoneCall *call, bool_t enable) {
	if (call!=NULL && call->audiostream!=NULL && call->audiostream->ec){
		bool_t bypass_mode = !enable;
		ms_filter_call_method(call->audiostream->ec,MS_ECHO_CANCELLER_SET_BYPASS_MODE,&bypass_mode);
	}
}
bool_t linphone_call_echo_cancellation_enabled(LinphoneCall *call) {
	if (call!=NULL && call->audiostream!=NULL && call->audiostream->ec){
		bool_t val;
		ms_filter_call_method(call->audiostream->ec,MS_ECHO_CANCELLER_GET_BYPASS_MODE,&val);
		return !val;
	} else {
		return linphone_core_echo_cancellation_enabled(call->core);
	}
}

void linphone_call_enable_echo_limiter(LinphoneCall *call, bool_t val){
	if (call!=NULL && call->audiostream!=NULL ) {
		if (val) {
		const char *type=lp_config_get_string(call->core->config,"sound","el_type","mic");
		if (strcasecmp(type,"mic")==0)
			audio_stream_enable_echo_limiter(call->audiostream,ELControlMic);
		else if (strcasecmp(type,"full")==0)
			audio_stream_enable_echo_limiter(call->audiostream,ELControlFull);
		} else {
			audio_stream_enable_echo_limiter(call->audiostream,ELInactive);
		}
	}
}

bool_t linphone_call_echo_limiter_enabled(const LinphoneCall *call){
	if (call!=NULL && call->audiostream!=NULL ){
		return call->audiostream->el_type !=ELInactive ;
	} else {
		return linphone_core_echo_limiter_enabled(call->core);
	}
}

/**
 * @addtogroup call_misc
 * @{
**/

/**
 * Returns the measured sound volume played locally (received from remote).
 * It is expressed in dbm0.
**/
float linphone_call_get_play_volume(LinphoneCall *call){
	AudioStream *st=call->audiostream;
	if (st && st->volrecv){
		float vol=0;
		ms_filter_call_method(st->volrecv,MS_VOLUME_GET,&vol);
		return vol;

	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

/**
 * Returns the measured sound volume recorded locally (sent to remote).
 * It is expressed in dbm0.
**/
float linphone_call_get_record_volume(LinphoneCall *call){
	AudioStream *st=call->audiostream;
	if (st && st->volsend && !call->audio_muted && call->state==LinphoneCallStreamsRunning){
		float vol=0;
		ms_filter_call_method(st->volsend,MS_VOLUME_GET,&vol);
		return vol;

	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

float linphone_call_get_speaker_volume_gain(const LinphoneCall *call) {
	if(call->audiostream) return audio_stream_get_sound_card_output_gain(call->audiostream);
	else {
		ms_error("Could not get playback volume: no audio stream");
		return -1.0f;
	}
}

void linphone_call_set_speaker_volume_gain(LinphoneCall *call, float volume) {
	if(call->audiostream) audio_stream_set_sound_card_output_gain(call->audiostream, volume);
	else ms_error("Could not set playback volume: no audio stream");
}

float linphone_call_get_microphone_volume_gain(const LinphoneCall *call) {
	if(call->audiostream) return audio_stream_get_sound_card_input_gain(call->audiostream);
	else {
		ms_error("Could not get record volume: no audio stream");
		return -1.0f;
	}
}

void linphone_call_set_microphone_volume_gain(LinphoneCall *call, float volume) {
	if(call->audiostream) audio_stream_set_sound_card_input_gain(call->audiostream, volume);
	else ms_error("Could not set record volume: no audio stream");
}

/**
 * Obtain real-time quality rating of the call
 *
 * Based on local RTP statistics and RTCP feedback, a quality rating is computed and updated
 * during all the duration of the call. This function returns its value at the time of the function call.
 * It is expected that the rating is updated at least every 5 seconds or so.
 * The rating is a floating point number comprised between 0 and 5.
 *
 * 4-5 = good quality <br>
 * 3-4 = average quality <br>
 * 2-3 = poor quality <br>
 * 1-2 = very poor quality <br>
 * 0-1 = can't be worse, mostly unusable <br>
 *
 * @return The function returns -1 if no quality measurement is available, for example if no
 * active audio stream exist. Otherwise it returns the quality rating.
**/
float linphone_call_get_current_quality(LinphoneCall *call){
	float audio_rating=-1;
	float video_rating=-1;
	float result;
	if (call->audiostream){
		audio_rating=media_stream_get_quality_rating((MediaStream*)call->audiostream)/5.0;
	}
	if (call->videostream){
		video_rating=media_stream_get_quality_rating((MediaStream*)call->videostream)/5.0;
	}
	if (audio_rating<0 && video_rating<0) result=-1;
	else if (audio_rating<0) result=video_rating*5.0;
	else if (video_rating<0) result=audio_rating*5.0;
	else result=audio_rating*video_rating*5.0;
	return result;
}

/**
 * Returns call quality averaged over all the duration of the call.
 *
 * See linphone_call_get_current_quality() for more details about quality measurement.
**/
float linphone_call_get_average_quality(LinphoneCall *call){
	if (call->audiostream){
		return audio_stream_get_average_quality_rating(call->audiostream);
	}
	return -1;
}

static void update_local_stats(LinphoneCallStats *stats, MediaStream *stream){
	const MSQualityIndicator *qi=media_stream_get_quality_indicator(stream);
	if (qi) {
		stats->local_late_rate=ms_quality_indicator_get_local_late_rate(qi);
		stats->local_loss_rate=ms_quality_indicator_get_local_loss_rate(qi);
	}
}

/**
 * Access last known statistics for audio stream, for a given call.
**/
const LinphoneCallStats *linphone_call_get_audio_stats(LinphoneCall *call) {
	LinphoneCallStats *stats=&call->stats[LINPHONE_CALL_STATS_AUDIO];
	if (call->audiostream){
		update_local_stats(stats,(MediaStream*)call->audiostream);
	}
	return stats;
}

/**
 * Access last known statistics for video stream, for a given call.
**/
const LinphoneCallStats *linphone_call_get_video_stats(LinphoneCall *call) {
	LinphoneCallStats *stats=&call->stats[LINPHONE_CALL_STATS_VIDEO];
	if (call->videostream){
		update_local_stats(stats,(MediaStream*)call->videostream);
	}
	return stats;
}

static bool_t ice_in_progress(LinphoneCallStats *stats){
	return stats->ice_state==LinphoneIceStateInProgress;
}

/**
 * Indicates whether an operation is in progress at the media side.
 * It can a bad idea to initiate signaling operations (adding video, pausing the call, removing video, changing video parameters) while
 * the media is busy in establishing the connection (typically ICE connectivity checks). It can result in failures generating loss of time
 * in future operations in the call.
 * Applications are invited to check this function after each call state change to decide whether certain operations are permitted or not.
 * @param call the call
 * @return TRUE if media is busy in establishing the connection, FALSE otherwise.
**/
bool_t linphone_call_media_in_progress(LinphoneCall *call){
	bool_t ret=FALSE;
	if (ice_in_progress(&call->stats[LINPHONE_CALL_STATS_AUDIO]) || ice_in_progress(&call->stats[LINPHONE_CALL_STATS_VIDEO]))
		ret=TRUE;
	/*TODO: could check zrtp state, upnp state*/
	return ret;
}

/**
 * Get the local loss rate since last report
 * @return The sender loss rate
**/
float linphone_call_stats_get_sender_loss_rate(const LinphoneCallStats *stats) {
	const report_block_t *srb = NULL;

	if (!stats || !stats->sent_rtcp)
		return 0.0;
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several mblk_t structure */
	if (stats->sent_rtcp->b_cont != NULL)
		msgpullup(stats->sent_rtcp, -1);
	if (rtcp_is_SR(stats->sent_rtcp))
		srb = rtcp_SR_get_report_block(stats->sent_rtcp, 0);
	else if (rtcp_is_RR(stats->sent_rtcp))
		srb = rtcp_RR_get_report_block(stats->sent_rtcp, 0);
	if (!srb)
		return 0.0;
	return 100.0 * report_block_get_fraction_lost(srb) / 256.0;
}

/**
 * Gets the remote reported loss rate since last report
 * @return The receiver loss rate
**/
float linphone_call_stats_get_receiver_loss_rate(const LinphoneCallStats *stats) {
	const report_block_t *rrb = NULL;

	if (!stats || !stats->received_rtcp)
		return 0.0;
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several mblk_t structure */
	if (stats->received_rtcp->b_cont != NULL)
		msgpullup(stats->received_rtcp, -1);
	if (rtcp_is_RR(stats->received_rtcp))
		rrb = rtcp_RR_get_report_block(stats->received_rtcp, 0);
	else if (rtcp_is_SR(stats->received_rtcp))
		rrb = rtcp_SR_get_report_block(stats->received_rtcp, 0);
	if (!rrb)
		return 0.0;
	return 100.0 * report_block_get_fraction_lost(rrb) / 256.0;
}

/**
 * Gets the local interarrival jitter
 * @return The interarrival jitter at last emitted sender report
**/
float linphone_call_stats_get_sender_interarrival_jitter(const LinphoneCallStats *stats, LinphoneCall *call) {
	const LinphoneCallParams *params;
	const PayloadType *pt;
	const report_block_t *srb = NULL;

	if (!stats || !call || !stats->sent_rtcp)
		return 0.0;
	params = linphone_call_get_current_params(call);
	if (!params)
		return 0.0;
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several mblk_t structure */
	if (stats->sent_rtcp->b_cont != NULL)
		msgpullup(stats->sent_rtcp, -1);
	if (rtcp_is_SR(stats->sent_rtcp))
		srb = rtcp_SR_get_report_block(stats->sent_rtcp, 0);
	else if (rtcp_is_RR(stats->sent_rtcp))
		srb = rtcp_RR_get_report_block(stats->sent_rtcp, 0);
	if (!srb)
		return 0.0;
	if (stats->type == LINPHONE_CALL_STATS_AUDIO)
		pt = linphone_call_params_get_used_audio_codec(params);
	else
		pt = linphone_call_params_get_used_video_codec(params);
	if (!pt || (pt->clock_rate == 0))
		return 0.0;
	return (float)report_block_get_interarrival_jitter(srb) / (float)pt->clock_rate;
}

/**
 * Gets the remote reported interarrival jitter
 * @return The interarrival jitter at last received receiver report
**/
float linphone_call_stats_get_receiver_interarrival_jitter(const LinphoneCallStats *stats, LinphoneCall *call) {
	const LinphoneCallParams *params;
	const PayloadType *pt;
	const report_block_t *rrb = NULL;

	if (!stats || !call || !stats->received_rtcp)
		return 0.0;
	params = linphone_call_get_current_params(call);
	if (!params)
		return 0.0;
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several mblk_t structure */
	if (stats->received_rtcp->b_cont != NULL)
		msgpullup(stats->received_rtcp, -1);
	if (rtcp_is_SR(stats->received_rtcp))
		rrb = rtcp_SR_get_report_block(stats->received_rtcp, 0);
	else if (rtcp_is_RR(stats->received_rtcp))
		rrb = rtcp_RR_get_report_block(stats->received_rtcp, 0);
	if (!rrb)
		return 0.0;
	if (stats->type == LINPHONE_CALL_STATS_AUDIO)
		pt = linphone_call_params_get_used_audio_codec(params);
	else
		pt = linphone_call_params_get_used_video_codec(params);
	if (!pt || (pt->clock_rate == 0))
		return 0.0;
	return (float)report_block_get_interarrival_jitter(rrb) / (float)pt->clock_rate;
}

rtp_stats_t linphone_call_stats_get_rtp_stats(const LinphoneCallStats *stats, LinphoneCall *call) {
	rtp_stats_t rtp_stats;
	memset(&rtp_stats, 0, sizeof(rtp_stats));

	if (stats && call) {
		if (stats->type == LINPHONE_CALL_STATS_AUDIO && call->audiostream != NULL)
			audio_stream_get_local_rtp_stats(call->audiostream, &rtp_stats);
	#ifdef VIDEO_ENABLED
		else if (call->videostream != NULL)
			video_stream_get_local_rtp_stats(call->videostream, &rtp_stats);
	#endif
	}
	return rtp_stats;
}

/**
 * Gets the cumulative number of late packets
 * @return The cumulative number of late packets
**/
uint64_t linphone_call_stats_get_late_packets_cumulative_number(const LinphoneCallStats *stats, LinphoneCall *call) {
	return linphone_call_stats_get_rtp_stats(stats, call).outoftime;
}

/**
 * Get the bandwidth measurement of the received stream, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param[in] stats LinphoneCallStats object
 * @return The bandwidth measurement of the received stream in kbit/s.
 */
float linphone_call_stats_get_download_bandwidth(const LinphoneCallStats *stats) {
	return stats->download_bandwidth;
}

/**
 * Get the bandwidth measurement of the sent stream, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param[in] stats LinphoneCallStats object
 * @return The bandwidth measurement of the sent stream in kbit/s.
 */
float linphone_call_stats_get_upload_bandwidth(const LinphoneCallStats *stats) {
	return stats->upload_bandwidth;
}

/**
 * Get the state of ICE processing.
 * @param[in] stats LinphoneCallStats object
 * @return The state of ICE processing.
 */
LinphoneIceState linphone_call_stats_get_ice_state(const LinphoneCallStats *stats) {
	return stats->ice_state;
}

/**
 * Get the state of uPnP processing.
 * @param[in] stats LinphoneCallStats object
 * @return The state of uPnP processing.
 */
LinphoneUpnpState linphone_call_stats_get_upnp_state(const LinphoneCallStats *stats) {
	return stats->upnp_state;
}

/**
 * Start call recording.
 * The output file where audio is recorded must be previously specified with linphone_call_params_set_record_file().
**/
void linphone_call_start_recording(LinphoneCall *call){
	if (!call->params->record_file){
		ms_error("linphone_call_start_recording(): no output file specified. Use linphone_call_params_set_record_file().");
		return;
	}
	if (call->audiostream && !call->params->in_conference){
		audio_stream_mixed_record_start(call->audiostream);
	}
	call->record_active=TRUE;
}

/**
 * Stop call recording.
**/
void linphone_call_stop_recording(LinphoneCall *call){
	if (call->audiostream && !call->params->in_conference){
		audio_stream_mixed_record_stop(call->audiostream);
	}
	call->record_active=FALSE;
}

/**
 * @}
**/

static void report_bandwidth(LinphoneCall *call, MediaStream *as, MediaStream *vs){
	bool_t as_active =  as ? (media_stream_get_state(as) == MSStreamStarted) : FALSE;
	bool_t vs_active =  vs ? (media_stream_get_state(vs) == MSStreamStarted) : FALSE;

	call->stats[LINPHONE_CALL_STATS_AUDIO].download_bandwidth=(as_active) ? (media_stream_get_down_bw(as)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_AUDIO].upload_bandwidth=(as_active) ? (media_stream_get_up_bw(as)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_VIDEO].download_bandwidth=(vs_active) ? (media_stream_get_down_bw(vs)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_VIDEO].upload_bandwidth=(vs_active) ? (media_stream_get_up_bw(vs)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_AUDIO].rtcp_download_bandwidth=(as_active) ? (media_stream_get_rtcp_down_bw(as)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_AUDIO].rtcp_upload_bandwidth=(as_active) ? (media_stream_get_rtcp_up_bw(as)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_VIDEO].rtcp_download_bandwidth=(vs_active) ? (media_stream_get_rtcp_down_bw(vs)*1e-3) : 0;
	call->stats[LINPHONE_CALL_STATS_VIDEO].rtcp_upload_bandwidth=(vs_active) ? (media_stream_get_rtcp_up_bw(vs)*1e-3) : 0;
	if (as_active) {
		call->stats[LINPHONE_CALL_STATS_AUDIO].updated|=LINPHONE_CALL_STATS_PERIODICAL_UPDATE;
		linphone_core_notify_call_stats_updated(call->core, call, &call->stats[LINPHONE_CALL_STATS_AUDIO]);
		call->stats[LINPHONE_CALL_STATS_AUDIO].updated=0;
	}
	if (vs_active) {
		call->stats[LINPHONE_CALL_STATS_VIDEO].updated|=LINPHONE_CALL_STATS_PERIODICAL_UPDATE;
		linphone_core_notify_call_stats_updated(call->core, call, &call->stats[LINPHONE_CALL_STATS_VIDEO]);
		call->stats[LINPHONE_CALL_STATS_VIDEO].updated=0;

	}

	ms_message(	"Bandwidth usage for call [%p]:\n"
				"\tRTP  audio=[d=%5.1f,u=%5.1f], video=[d=%5.1f,u=%5.1f] kbits/sec\n"
				"\tRTCP audio=[d=%5.1f,u=%5.1f], video=[d=%5.1f,u=%5.1f] kbits/sec",
				call,
				call->stats[LINPHONE_CALL_STATS_AUDIO].download_bandwidth,
				call->stats[LINPHONE_CALL_STATS_AUDIO].upload_bandwidth,
				call->stats[LINPHONE_CALL_STATS_VIDEO].download_bandwidth,
				call->stats[LINPHONE_CALL_STATS_VIDEO].upload_bandwidth,
				call->stats[LINPHONE_CALL_STATS_AUDIO].rtcp_download_bandwidth,
				call->stats[LINPHONE_CALL_STATS_AUDIO].rtcp_upload_bandwidth,
				call->stats[LINPHONE_CALL_STATS_VIDEO].rtcp_download_bandwidth,
				call->stats[LINPHONE_CALL_STATS_VIDEO].rtcp_upload_bandwidth
	);

}

static void linphone_core_disconnected(LinphoneCore *lc, LinphoneCall *call){
	char temp[256]={0};
	char *from=NULL;

	from = linphone_call_get_remote_address_as_string(call);
	snprintf(temp,sizeof(temp)-1,"Remote end %s seems to have disconnected, the call is going to be closed.",from ? from : "");
	if (from) ms_free(from);

	ms_message("On call [%p]: %s",call,temp);
	linphone_core_notify_display_warning(lc,temp);
	linphone_core_terminate_call(lc,call);
	linphone_core_play_named_tone(lc,LinphoneToneCallLost);
}

static void change_ice_media_destinations(LinphoneCall *call) {
    const char *rtp_addr;
    const char *rtcp_addr;
    int rtp_port;
    int rtcp_port;
    bool_t result;

    if (call->audiostream && ice_session_check_list(call->ice_session, 0)) {
        result = ice_check_list_selected_valid_remote_candidate(ice_session_check_list(call->ice_session, 0), &rtp_addr, &rtp_port, &rtcp_addr, &rtcp_port);
        if (result == TRUE) {
            ms_message("Change audio stream destination: RTP=%s:%d RTCP=%s:%d", rtp_addr, rtp_port, rtcp_addr, rtcp_port);
            rtp_session_set_symmetric_rtp(call->audiostream->ms.sessions.rtp_session, FALSE);
            rtp_session_set_remote_addr_full(call->audiostream->ms.sessions.rtp_session, rtp_addr, rtp_port, rtcp_addr, rtcp_port);
        }
    }
#ifdef VIDEO_ENABLED
    if (call->videostream && ice_session_check_list(call->ice_session, 1)) {
        result = ice_check_list_selected_valid_remote_candidate(ice_session_check_list(call->ice_session, 1), &rtp_addr, &rtp_port, &rtcp_addr, &rtcp_port);
        if (result == TRUE) {
            ms_message("Change video stream destination: RTP=%s:%d RTCP=%s:%d", rtp_addr, rtp_port, rtcp_addr, rtcp_port);
            rtp_session_set_symmetric_rtp(call->videostream->ms.sessions.rtp_session, FALSE);
            rtp_session_set_remote_addr_full(call->videostream->ms.sessions.rtp_session, rtp_addr, rtp_port, rtcp_addr, rtcp_port);
        }
    }
#endif
}

static void handle_ice_events(LinphoneCall *call, OrtpEvent *ev){
	OrtpEventType evt=ortp_event_get_type(ev);
	OrtpEventData *evd=ortp_event_get_data(ev);
	int ping_time;

	if (evt == ORTP_EVENT_ICE_SESSION_PROCESSING_FINISHED) {
		LinphoneCallParams *params = linphone_call_params_copy(call->current_params);
		switch (call->params->media_encryption) {
			case LinphoneMediaEncryptionZRTP:
			case LinphoneMediaEncryptionDTLS:
			/* preserve media encryption param because at that time ZRTP/SRTP-DTLS negociation may still be ongoing*/
			params->media_encryption=call->params->media_encryption;
			break;
			case LinphoneMediaEncryptionSRTP:
			case LinphoneMediaEncryptionNone:
			/*keep all values to make sure a warning will be generated by compiler if new enum value is added*/
				break;
		}

		switch (ice_session_state(call->ice_session)) {
			case IS_Completed:
				ice_session_select_candidates(call->ice_session);
				if (ice_session_role(call->ice_session) == IR_Controlling
						&& lp_config_get_int(call->core->config, "sip", "update_call_when_ice_completed", TRUE)) {
					params->internal_call_update = TRUE;
					linphone_core_update_call(call->core, call, params);
				}
				change_ice_media_destinations(call);
				start_dtls_on_all_streams(call);
				break;
			case IS_Failed:
				if (ice_session_has_completed_check_list(call->ice_session) == TRUE) {
					ice_session_select_candidates(call->ice_session);
					if (ice_session_role(call->ice_session) == IR_Controlling) {
						/* At least one ICE session has succeeded, so perform a call update. */
						params->internal_call_update = TRUE;
						linphone_core_update_call(call->core, call, params);
					}
				}
				break;
			default:
				break;
		}
		linphone_core_update_ice_state_in_call_stats(call);
		linphone_call_params_unref(params);
	} else if (evt == ORTP_EVENT_ICE_GATHERING_FINISHED) {

		if (evd->info.ice_processing_successful==TRUE) {
			ice_session_compute_candidates_foundations(call->ice_session);
			ice_session_eliminate_redundant_candidates(call->ice_session);
			ice_session_choose_default_candidates(call->ice_session);
			ping_time = ice_session_average_gathering_round_trip_time(call->ice_session);
			if (ping_time >=0) {
				call->ping_time=ping_time;
			}
		} else {
			ms_warning("No STUN answer from [%s], disabling ICE",linphone_core_get_stun_server(call->core));
			linphone_call_delete_ice_session(call);
		}
		switch (call->state) {
			case LinphoneCallUpdating:
				linphone_core_start_update_call(call->core, call);
				break;
			case LinphoneCallUpdatedByRemote:
				linphone_core_start_accept_call_update(call->core, call,call->prevstate,linphone_call_state_to_string(call->prevstate));
				break;
			case LinphoneCallOutgoingInit:
				linphone_call_stop_media_streams_for_ice_gathering(call);
				linphone_core_proceed_with_invite_if_ready(call->core, call, NULL);
				break;
			case LinphoneCallIdle:
				linphone_call_stop_media_streams_for_ice_gathering(call);
				linphone_call_update_local_media_description_from_ice_or_upnp(call);
				sal_call_set_local_media_description(call->op,call->localdesc);
				linphone_core_notify_incoming_call(call->core, call);
				break;
			default:
				break;
		}
	} else if (evt == ORTP_EVENT_ICE_LOSING_PAIRS_COMPLETED) {
		if (call->state==LinphoneCallUpdatedByRemote){
			linphone_core_start_accept_call_update(call->core, call,call->prevstate,linphone_call_state_to_string(call->prevstate));
			linphone_core_update_ice_state_in_call_stats(call);
		}
	} else if (evt == ORTP_EVENT_ICE_RESTART_NEEDED) {
		ice_session_restart(call->ice_session);
		ice_session_set_role(call->ice_session, IR_Controlling);
		linphone_core_update_call(call->core, call, call->current_params);
	}
}


/*do not change the prototype of this function, it is also used internally in linphone-daemon.*/
void linphone_call_stats_fill(LinphoneCallStats *stats, MediaStream *ms, OrtpEvent *ev){
	OrtpEventType evt=ortp_event_get_type(ev);
	OrtpEventData *evd=ortp_event_get_data(ev);

	if (evt == ORTP_EVENT_RTCP_PACKET_RECEIVED) {
		stats->round_trip_delay = rtp_session_get_round_trip_propagation(ms->sessions.rtp_session);
		if(stats->received_rtcp != NULL)
			freemsg(stats->received_rtcp);
		stats->received_rtcp = evd->packet;
		evd->packet = NULL;
		stats->updated = LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE;
		update_local_stats(stats,ms);
	} else if (evt == ORTP_EVENT_RTCP_PACKET_EMITTED) {
		memcpy(&stats->jitter_stats, rtp_session_get_jitter_stats(ms->sessions.rtp_session), sizeof(jitter_stats_t));
		if (stats->sent_rtcp != NULL)
			freemsg(stats->sent_rtcp);
		stats->sent_rtcp = evd->packet;
		evd->packet = NULL;
		stats->updated = LINPHONE_CALL_STATS_SENT_RTCP_UPDATE;
		update_local_stats(stats,ms);
	}
}

void linphone_call_stats_uninit(LinphoneCallStats *stats){
	if (stats->received_rtcp) {
		freemsg(stats->received_rtcp);
		stats->received_rtcp=NULL;
	}
	if (stats->sent_rtcp){
		freemsg(stats->sent_rtcp);
		stats->sent_rtcp=NULL;
	}
}

void linphone_call_notify_stats_updated(LinphoneCall *call, int stream_index){
	LinphoneCallStats *stats=&call->stats[stream_index];
	LinphoneCore *lc=call->core;
	if (stats->updated){
		switch(stats->updated) {
			case LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE:
			case LINPHONE_CALL_STATS_SENT_RTCP_UPDATE:
				linphone_reporting_on_rtcp_update(call, stream_index);
				break;
			default:break;
		}
		linphone_core_notify_call_stats_updated(lc, call, stats);
		stats->updated = 0;
	}
}

void linphone_call_handle_stream_events(LinphoneCall *call, int stream_index){
	MediaStream *ms=stream_index==0 ? (MediaStream *)call->audiostream : (MediaStream *)call->videostream; /*assumption to remove*/
	OrtpEvQueue *evq;
	OrtpEvent *ev;

	if (ms==NULL) return;
	/* Ensure there is no dangling ICE check list. */
	if (call->ice_session == NULL) ms->ice_check_list = NULL;

	switch(ms->type){
		case MSAudio:
			audio_stream_iterate((AudioStream*)ms);
		break;
		case MSVideo:
#ifdef VIDEO_ENABLED
			video_stream_iterate((VideoStream*)ms);
#endif
		break;
		default:
			ms_error("linphone_call_handle_stream_events(): unsupported stream type.");
			return;
		break;
	}
	/*yes the event queue has to be taken at each iteration, because ice events may perform operations re-creating the streams*/
	while ((evq=stream_index==0 ? call->audiostream_app_evq : call->videostream_app_evq)  && (NULL != (ev=ortp_ev_queue_get(evq)))){
		OrtpEventType evt=ortp_event_get_type(ev);
		OrtpEventData *evd=ortp_event_get_data(ev);

		linphone_call_stats_fill(&call->stats[stream_index],ms,ev);
		linphone_call_notify_stats_updated(call,stream_index);

		if (evt == ORTP_EVENT_ZRTP_ENCRYPTION_CHANGED){
			if (ms->type==MSAudio)
				linphone_call_audiostream_encryption_changed(call, evd->info.zrtp_stream_encrypted);
			else if (ms->type==MSVideo)
				propagate_encryption_changed(call);
		} else if (evt == ORTP_EVENT_ZRTP_SAS_READY) {
			if (ms->type==MSAudio)
				linphone_call_audiostream_auth_token_ready(call, evd->info.zrtp_sas.sas, evd->info.zrtp_sas.verified);
		} else if (evt == ORTP_EVENT_DTLS_ENCRYPTION_CHANGED) {
			if (ms->type==MSAudio)
				linphone_call_audiostream_encryption_changed(call, evd->info.dtls_stream_encrypted);
			else if (ms->type==MSVideo)
				propagate_encryption_changed(call);
		}else if ((evt == ORTP_EVENT_ICE_SESSION_PROCESSING_FINISHED) || (evt == ORTP_EVENT_ICE_GATHERING_FINISHED)
			|| (evt == ORTP_EVENT_ICE_LOSING_PAIRS_COMPLETED) || (evt == ORTP_EVENT_ICE_RESTART_NEEDED)) {
			handle_ice_events(call, ev);
		} else if (evt==ORTP_EVENT_TELEPHONE_EVENT){
			linphone_core_dtmf_received(call,evd->info.telephone_event);
		}
		ortp_event_destroy(ev);
	}
}

void linphone_call_background_tasks(LinphoneCall *call, bool_t one_second_elapsed){
	int disconnect_timeout = linphone_core_get_nortp_timeout(call->core);
	bool_t disconnected=FALSE;

	switch (call->state) {
	case LinphoneCallStreamsRunning:
	case LinphoneCallOutgoingEarlyMedia:
	case LinphoneCallIncomingEarlyMedia:
	case LinphoneCallPausedByRemote:
	case LinphoneCallPaused:
		if (one_second_elapsed){
			float audio_load=0, video_load=0;
			if (call->audiostream!=NULL){
				if (call->audiostream->ms.sessions.ticker)
					audio_load=ms_ticker_get_average_load(call->audiostream->ms.sessions.ticker);
			}
			if (call->videostream!=NULL){
				if (call->videostream->ms.sessions.ticker)
					video_load=ms_ticker_get_average_load(call->videostream->ms.sessions.ticker);
			}
			report_bandwidth(call,(MediaStream*)call->audiostream,(MediaStream*)call->videostream);
			ms_message("Thread processing load: audio=%f\tvideo=%f",audio_load,video_load);
		}
		break;
	default:
		/*no stats for other states*/
		break;
	}

#ifdef BUILD_UPNP
	linphone_upnp_call_process(call);
#endif //BUILD_UPNP

	linphone_call_handle_stream_events(call,0);
	linphone_call_handle_stream_events(call,1);
	if (call->state==LinphoneCallStreamsRunning && one_second_elapsed && call->audiostream!=NULL
		&& call->audiostream->ms.state==MSStreamStarted && disconnect_timeout>0 )
		disconnected=!audio_stream_alive(call->audiostream,disconnect_timeout);
	if (disconnected)
		linphone_core_disconnected(call->core,call);
}

void linphone_call_log_completed(LinphoneCall *call){
	LinphoneCore *lc=call->core;

	call->log->duration=linphone_call_get_duration(call); /*store duration since connected*/

	if (call->log->status==LinphoneCallMissed){
		char *info;
		lc->missed_calls++;
		info=ortp_strdup_printf(ngettext("You have missed %i call.",
										 "You have missed %i calls.", lc->missed_calls),
								lc->missed_calls);
		linphone_core_notify_display_status(lc,info);
		ms_free(info);
	}
	lc->call_logs=ms_list_prepend(lc->call_logs,linphone_call_log_ref(call->log));
	if (ms_list_size(lc->call_logs)>lc->max_call_logs){
		MSList *elem,*prevelem=NULL;
		/*find the last element*/
		for(elem=lc->call_logs;elem!=NULL;elem=elem->next){
			prevelem=elem;
		}
		elem=prevelem;
		linphone_call_log_unref((LinphoneCallLog*)elem->data);
		lc->call_logs=ms_list_remove_link(lc->call_logs,elem);
	}
	linphone_core_notify_call_log_updated(lc,call->log);
	call_logs_write_to_config_file(lc);
}

/**
 * Returns the current transfer state, if a transfer has been initiated from this call.
 * @see linphone_core_transfer_call() , linphone_core_transfer_call_to_another()
**/
LinphoneCallState linphone_call_get_transfer_state(LinphoneCall *call) {
	return call->transfer_state;
}

void linphone_call_set_transfer_state(LinphoneCall* call, LinphoneCallState state) {
	if (state != call->transfer_state) {
		LinphoneCore* lc = call->core;
		ms_message("Transfer state for call [%p] changed  from [%s] to [%s]",call
						,linphone_call_state_to_string(call->transfer_state)
						,linphone_call_state_to_string(state));
		call->transfer_state = state;
		linphone_core_notify_transfer_state_changed(lc, call, state);
	}
}

bool_t linphone_call_is_in_conference(const LinphoneCall *call) {
	return call->params->in_conference;
}

/**
 * Perform a zoom of the video displayed during a call.
 * @param call the call.
 * @param zoom_factor a floating point number describing the zoom factor. A value 1.0 corresponds to no zoom applied.
 * @param cx a floating point number pointing the horizontal center of the zoom to be applied. This value should be between 0.0 and 1.0.
 * @param cy a floating point number pointing the vertical center of the zoom to be applied. This value should be between 0.0 and 1.0.
 *
 * cx and cy are updated in return in case their coordinates were too excentrated for the requested zoom factor. The zoom ensures that all the screen is fullfilled with the video.
**/
void linphone_call_zoom_video(LinphoneCall* call, float zoom_factor, float* cx, float* cy) {
	VideoStream* vstream = call->videostream;
	if (vstream && vstream->output) {
		float zoom[3];
		float halfsize;

		if (zoom_factor < 1)
			zoom_factor = 1;
		halfsize = 0.5 * 1.0 / zoom_factor;

		if ((*cx - halfsize) < 0)
			*cx = 0 + halfsize;
		if ((*cx + halfsize) > 1)
			*cx = 1 - halfsize;
		if ((*cy - halfsize) < 0)
			*cy = 0 + halfsize;
		if ((*cy + halfsize) > 1)
			*cy = 1 - halfsize;

		zoom[0] = zoom_factor;
		zoom[1] = *cx;
		zoom[2] = *cy;
		ms_filter_call_method(vstream->output, MS_VIDEO_DISPLAY_ZOOM, &zoom);
	}else ms_warning("Could not apply zoom: video output wasn't activated.");
}

static LinphoneAddress *get_fixed_contact(LinphoneCore *lc, LinphoneCall *call , LinphoneProxyConfig *dest_proxy){
	LinphoneAddress *ctt=NULL;
	LinphoneAddress *ret=NULL;
	//const char *localip=call->localip;

	/* first use user's supplied ip address if asked*/
	if (linphone_core_get_firewall_policy(lc)==LinphonePolicyUseNatAddress){
		ctt=linphone_core_get_primary_contact_parsed(lc);
		linphone_address_set_domain(ctt,linphone_core_get_nat_address_resolved(lc));
		ret=ctt;
	} else if (call->op && sal_op_get_contact_address(call->op)!=NULL){
		/* if already choosed, don't change it */
		return NULL;
	} else if (call->ping_op && sal_op_get_contact_address(call->ping_op)) {
		/* if the ping OPTIONS request succeeded use the contact guessed from the
		 received, rport*/
		ms_message("Contact has been fixed using OPTIONS"/* to %s",guessed*/);
		ret=linphone_address_clone(sal_op_get_contact_address(call->ping_op));;
	} else 	if (dest_proxy && dest_proxy->op && sal_op_get_contact_address(dest_proxy->op)){
	/*if using a proxy, use the contact address as guessed with the REGISTERs*/
		ms_message("Contact has been fixed using proxy" /*to %s",fixed_contact*/);
		ret=linphone_address_clone(sal_op_get_contact_address(dest_proxy->op));
	} else {
		ctt=linphone_core_get_primary_contact_parsed(lc);
		if (ctt!=NULL){
			/*otherwise use supplied localip*/
			linphone_address_set_domain(ctt,NULL/*localip*/);
			linphone_address_set_port(ctt,-1/*linphone_core_get_sip_port(lc)*/);
			ms_message("Contact has not been fixed stack will do"/* to %s",ret*/);
			ret=ctt;
		}
	}
	return ret;
}

void linphone_call_set_contact_op(LinphoneCall* call) {
	LinphoneAddress *contact;

	if (call->dest_proxy == NULL) {
		/* Try to define the destination proxy if it has not already been done to have a correct contact field in the SIP messages */
		call->dest_proxy = linphone_core_lookup_known_proxy(call->core, call->log->to);
	}

	contact=get_fixed_contact(call->core,call,call->dest_proxy);
	if (contact){
		SalTransport tport=sal_address_get_transport((SalAddress*)contact);
		sal_address_clean((SalAddress*)contact); /* clean out contact_params that come from proxy config*/
		sal_address_set_transport((SalAddress*)contact,tport);
		sal_op_set_contact_address(call->op, contact);
		linphone_address_destroy(contact);
	}
}

LinphonePlayer *linphone_call_get_player(LinphoneCall *call){
	if (call->player==NULL)
		call->player=linphone_call_build_player(call);
	return call->player;
}

void linphone_call_set_new_params(LinphoneCall *call, const LinphoneCallParams *params){
	LinphoneCallParams *cp=NULL;
	if (params) cp=linphone_call_params_copy(params);
	if (call->params) linphone_call_params_unref(call->params);
	call->params=cp;
}

static int send_dtmf_handler(void *data, unsigned int revents){
	LinphoneCall *call = (LinphoneCall*)data;
	/*By default we send DTMF RFC2833 if we do not have enabled SIP_INFO but we can also send RFC2833 and SIP_INFO*/
	if (linphone_core_get_use_rfc2833_for_dtmf(call->core)!=0 || linphone_core_get_use_info_for_dtmf(call->core)==0)
	{
		/* In Band DTMF */
		if (call->audiostream!=NULL){
			audio_stream_send_dtmf(call->audiostream,*call->dtmf_sequence);
		}
		else
		{
			ms_error("Cannot send RFC2833 DTMF when we are not in communication.");
			return FALSE;
		}
	}
	if (linphone_core_get_use_info_for_dtmf(call->core)!=0){
		/* Out of Band DTMF (use INFO method) */
		sal_call_send_dtmf(call->op,*call->dtmf_sequence);
	}

	/*this check is needed because linphone_call_send_dtmf does not set the timer since its a single character*/
	if (call->dtmfs_timer!=NULL) {
		memmove(call->dtmf_sequence, call->dtmf_sequence+1, strlen(call->dtmf_sequence));
	}
	/* continue only if the dtmf sequence is not empty*/
	if (call->dtmf_sequence!=NULL&&*call->dtmf_sequence!='\0') {
		return TRUE;
	} else {
		linphone_call_cancel_dtmfs(call);
		return FALSE;
	}
}
int linphone_call_send_dtmf(LinphoneCall *call,char dtmf){
	if (call==NULL){
		ms_warning("linphone_call_send_dtmf(): invalid call, canceling DTMF.");
		return -1;
	}
	call->dtmf_sequence = &dtmf;
	send_dtmf_handler(call,0);
	call->dtmf_sequence = NULL;
	return 0;
}

int linphone_call_send_dtmfs(LinphoneCall *call,char *dtmfs) {
	if (call==NULL){
		ms_warning("linphone_call_send_dtmfs(): invalid call, canceling DTMF sequence.");
		return -1;
	}
	if (call->dtmfs_timer!=NULL){
		ms_warning("linphone_call_send_dtmfs(): a DTMF sequence is already in place, canceling DTMF sequence.");
		return -2;
	}
	if (dtmfs != NULL) {
		int delay_ms = lp_config_get_int(call->core->config,"net","dtmf_delay_ms",200);
		call->dtmf_sequence = ms_strdup(dtmfs);
		call->dtmfs_timer = sal_create_timer(call->core->sal, send_dtmf_handler, call, delay_ms, "DTMF sequence timer");
	}
	return 0;
}

void linphone_call_cancel_dtmfs(LinphoneCall *call) {
	/*nothing to do*/
	if (!call || !call->dtmfs_timer) return;

	sal_cancel_timer(call->core->sal, call->dtmfs_timer);
	belle_sip_object_unref(call->dtmfs_timer);
	call->dtmfs_timer = NULL;
	if (call->dtmf_sequence != NULL) {
		ms_free(call->dtmf_sequence);
		call->dtmf_sequence = NULL;
	}
}

void * linphone_call_get_native_video_window_id(const LinphoneCall *call) {
	if (call->video_window_id) {
		/* The video id was previously set by the app. */
		return call->video_window_id;
	}
#ifdef VIDEO_ENABLED
	else if (call->videostream) {
		/* It was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only). */
		return video_stream_get_native_window_id(call->videostream);
	}
#endif
	return 0;
}

void linphone_call_set_native_video_window_id(LinphoneCall *call, void *id) {
	call->video_window_id = id;
#ifdef VIDEO_ENABLED
	if (call->videostream) {
		video_stream_set_native_window_id(call->videostream, id);
	}
#endif
}
#ifdef VIDEO_ENABLED
MSWebCam *linphone_call_get_video_device(const LinphoneCall *call) {
	if (call->camera_enabled==FALSE)
		return get_nowebcam_device();
	else
		return call->cam;
}
#endif

void linphone_call_set_audio_route(LinphoneCall *call, LinphoneAudioRoute route) {
	if (call != NULL && call->audiostream != NULL){
		audio_stream_set_audio_route(call->audiostream, (MSAudioRoute) route);
	}
}
