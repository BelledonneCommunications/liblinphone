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

#include "linphone/api/c-call-stats.h"
#include "c-wrapper/c-wrapper.h"
#include "mediastreamer2/zrtp.h"
#include "private.h"

// =============================================================================

static void _linphone_call_stats_clone(LinphoneCallStats *dst, const LinphoneCallStats *src);
static void _linphone_call_stats_uninit(LinphoneCallStats *stats);

/**
 * The LinphoneCallStats objects carries various statistic informations regarding quality of audio or video streams.
 *
 * To receive these informations periodically and as soon as they are computed, the application is invited to place a
 *#LinphoneCoreCallStatsUpdatedCb callback in the LinphoneCoreVTable structure it passes for instantiating the
 *LinphoneCore object (see linphone_core_new() ).
 *
 * At any time, the application can access last computed statistics using linphone_call_get_audio_stats() or
 *linphone_call_get_video_stats().
 **/
struct _LinphoneCallStats {
	belle_sip_object_t base;
	void *user_data;
	LinphoneStreamType type;     /**< Type of the stream which the stats refer to */
	jitter_stats_t jitter_stats; /**<jitter buffer statistics, see oRTP documentation for details */
	mblk_t *received_rtcp;  /**<Last RTCP packet received, as a mblk_t structure. See oRTP documentation for details how
	                           to extract information from it*/
	mblk_t *sent_rtcp;      /**<Last RTCP packet sent, as a mblk_t structure. See oRTP documentation for details how to
	                           extract information from it*/
	float round_trip_delay; /**<Round trip propagation time in seconds if known, -1 if unknown.*/
	LinphoneIceState ice_state;   /**< State of ICE processing. */
	LinphoneUpnpState upnp_state; /**< State of uPnP processing. */
	float download_bandwidth;     /**<Download bandwidth measurement of received stream, expressed in kbit/s, including
	                                 IP/UDP/RTP headers*/
	float upload_bandwidth; /**<Download bandwidth measurement of sent stream, expressed in kbit/s, including IP/UDP/RTP
	                           headers*/
	float local_late_rate;  /**<percentage of packet received too late over last second*/
	float local_loss_rate;  /**<percentage of lost packet over last second*/
	int updated;            /**< Tell which RTCP packet has been updated (received_rtcp or sent_rtcp). Can be either
	                           LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE or LINPHONE_CALL_STATS_SENT_RTCP_UPDATE */
	float rtcp_download_bandwidth; /**<RTCP download bandwidth measurement of received stream, expressed in kbit/s,
	                                  including IP/UDP/RTP headers*/
	float rtcp_upload_bandwidth; /**<RTCP download bandwidth measurement of sent stream, expressed in kbit/s, including
	                                IP/UDP/RTP headers*/
	rtp_stats_t rtp_stats;       /**< RTP stats */
	int rtp_remote_family;       /**< Ip adress family of the remote destination */
	int clockrate; /*RTP clockrate of the stream, provided here for easily converting timestamp units expressed in RTCP
	                  packets in milliseconds*/
	float estimated_download_bandwidth; /**<Estimated download bandwidth measurement of received stream, expressed in
	                                       kbit/s, including IP/UDP/RTP headers*/
	bool_t rtcp_received_via_mux;       /*private flag, for non-regression test only*/
	ZrtpAlgo zrtp_algo; /**< informations on the ZRTP exchange updated once it is performed(when the SAS is available),
	                       this is valid only on the audio stream */
	SrtpInfo inner_srtp_info; /**< informations on the SRTP crypto suite and and source of key material used on this
	                             stream for inner encryption when double encryption is on */
	SrtpInfo srtp_info; /**< informations on the SRTP crypto suite and and source of key material used on this stream */
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneCallStats);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneCallStats);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneCallStats,
                           belle_sip_object_t,
                           _linphone_call_stats_uninit, // destroy
                           _linphone_call_stats_clone,  // clone
                           NULL,                        // marshal
                           FALSE);

// =============================================================================
// Private functions
// =============================================================================

LinphoneCallStats *_linphone_call_stats_new() {
	LinphoneCallStats *stats = belle_sip_object_new(LinphoneCallStats);
	return stats;
}

static void _linphone_call_stats_uninit(LinphoneCallStats *stats) {
	if (stats->received_rtcp) {
		freemsg(stats->received_rtcp);
		stats->received_rtcp = NULL;
	}
	if (stats->sent_rtcp) {
		freemsg(stats->sent_rtcp);
		stats->sent_rtcp = NULL;
	}
}

static void _linphone_call_stats_clone(LinphoneCallStats *dst, const LinphoneCallStats *src) {
	dst->type = src->type;
	dst->jitter_stats = src->jitter_stats;
	dst->received_rtcp = src->received_rtcp ? dupmsg(src->received_rtcp) : nullptr;
	dst->sent_rtcp = src->sent_rtcp ? dupmsg(src->sent_rtcp) : nullptr;
	dst->round_trip_delay = src->round_trip_delay;
	dst->ice_state = src->ice_state;
	dst->upnp_state = src->upnp_state;
	dst->download_bandwidth = src->download_bandwidth;
	dst->upload_bandwidth = src->upload_bandwidth;
	dst->local_late_rate = src->local_late_rate;
	dst->local_loss_rate = src->local_loss_rate;
	dst->updated = src->updated;
	dst->rtcp_download_bandwidth = src->rtcp_download_bandwidth;
	dst->rtcp_upload_bandwidth = src->rtcp_upload_bandwidth;
	dst->rtp_stats = src->rtp_stats;
	dst->rtp_remote_family = src->rtp_remote_family;
	dst->clockrate = src->clockrate;
	dst->rtcp_received_via_mux = src->rtcp_received_via_mux;
	dst->estimated_download_bandwidth = src->estimated_download_bandwidth;
	dst->zrtp_algo.cipher_algo = src->zrtp_algo.cipher_algo;
	dst->zrtp_algo.key_agreement_algo = src->zrtp_algo.key_agreement_algo;
	dst->zrtp_algo.hash_algo = src->zrtp_algo.hash_algo;
	dst->zrtp_algo.auth_tag_algo = src->zrtp_algo.auth_tag_algo;
	dst->zrtp_algo.sas_algo = src->zrtp_algo.sas_algo;
	dst->srtp_info.send_suite = src->srtp_info.send_suite;
	dst->srtp_info.send_source = src->srtp_info.send_source;
	dst->srtp_info.recv_suite = src->srtp_info.recv_suite;
	dst->srtp_info.recv_source = src->srtp_info.recv_source;
	dst->inner_srtp_info.send_suite = src->inner_srtp_info.send_suite;
	dst->inner_srtp_info.send_source = src->inner_srtp_info.send_source;
	dst->inner_srtp_info.recv_suite = src->inner_srtp_info.recv_suite;
	dst->inner_srtp_info.recv_source = src->inner_srtp_info.recv_source;
}

void _linphone_call_stats_set_ice_state(LinphoneCallStats *stats, LinphoneIceState state) {
	stats->ice_state = state;
}

void _linphone_call_stats_set_type(LinphoneCallStats *stats, LinphoneStreamType type) {
	stats->type = type;
}

mblk_t *_linphone_call_stats_get_received_rtcp(const LinphoneCallStats *stats) {
	return stats->received_rtcp;
}

void _linphone_call_stats_set_received_rtcp(LinphoneCallStats *stats, mblk_t *m) {
	stats->received_rtcp = m;
}

mblk_t *_linphone_call_stats_get_sent_rtcp(const LinphoneCallStats *stats) {
	return stats->sent_rtcp;
}

void _linphone_call_stats_set_sent_rtcp(LinphoneCallStats *stats, mblk_t *m) {
	stats->sent_rtcp = m;
}

int _linphone_call_stats_get_updated(const LinphoneCallStats *stats) {
	return stats->updated;
}

void _linphone_call_stats_set_updated(LinphoneCallStats *stats, int updated) {
	stats->updated = updated;
}

void _linphone_call_stats_set_rtp_stats(LinphoneCallStats *stats, const rtp_stats_t *rtpStats) {
	memcpy(&(stats->rtp_stats), rtpStats, sizeof(*rtpStats));
}

void _linphone_call_stats_set_download_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	stats->download_bandwidth = bandwidth;
}

void _linphone_call_stats_set_upload_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	stats->upload_bandwidth = bandwidth;
}

void _linphone_call_stats_set_rtcp_download_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	stats->rtcp_download_bandwidth = bandwidth;
}

void _linphone_call_stats_set_rtcp_upload_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	stats->rtcp_upload_bandwidth = bandwidth;
}

void _linphone_call_stats_set_ip_family_of_remote(LinphoneCallStats *stats, LinphoneAddressFamily family) {
	stats->rtp_remote_family = family;
}

bool_t _linphone_call_stats_rtcp_received_via_mux(const LinphoneCallStats *stats) {
	return stats->rtcp_received_via_mux;
}

bool_t _linphone_call_stats_has_received_rtcp(const LinphoneCallStats *stats) {
	return stats->received_rtcp != NULL;
}

bool_t _linphone_call_stats_has_sent_rtcp(const LinphoneCallStats *stats) {
	return stats->sent_rtcp != NULL;
}

// =============================================================================
// Public functions
// =============================================================================

LinphoneCallStats *linphone_call_stats_ref(LinphoneCallStats *stats) {
	belle_sip_object_ref(stats);
	return stats;
}

void linphone_call_stats_unref(LinphoneCallStats *stats) {
	belle_sip_object_unref(stats);
}

void *linphone_call_stats_get_user_data(const LinphoneCallStats *stats) {
	return stats->user_data;
}

void linphone_call_stats_set_user_data(LinphoneCallStats *stats, void *data) {
	stats->user_data = data;
}

void linphone_call_stats_update(LinphoneCallStats *stats, MediaStream *stream) {
	PayloadType *pt;
	RtpSession *session = stream->sessions.rtp_session;
	const MSQualityIndicator *qi = media_stream_get_quality_indicator(stream);
	if (qi) {
		stats->local_late_rate = ms_quality_indicator_get_local_late_rate(qi);
		stats->local_loss_rate = ms_quality_indicator_get_local_loss_rate(qi);
	}
	media_stream_get_local_rtp_stats(stream, &stats->rtp_stats);
	pt = rtp_profile_get_payload(rtp_session_get_profile(session), rtp_session_get_send_payload_type(session));
	stats->clockrate = pt ? pt->clock_rate : 8000;
}

/*do not change the prototype of this function, it is also used internally in linphone-daemon.*/
void linphone_call_stats_fill(LinphoneCallStats *stats, MediaStream *ms, OrtpEvent *ev) {
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(ev);
	if (ms->sessions.rtp_session) {
		if (evt == ORTP_EVENT_RTCP_PACKET_RECEIVED) {
			stats->round_trip_delay = rtp_session_get_round_trip_propagation(ms->sessions.rtp_session);
			if (stats->received_rtcp != NULL) freemsg(stats->received_rtcp);
			stats->received_rtcp = evd->packet;
			stats->rtcp_received_via_mux = evd->info.socket_type == OrtpRTPSocket;
			evd->packet = NULL;
			stats->updated = LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE;
			linphone_call_stats_update(stats, ms);
		} else if (evt == ORTP_EVENT_RTCP_PACKET_EMITTED) {
			memcpy(&stats->jitter_stats, rtp_session_get_jitter_stats(ms->sessions.rtp_session),
			       sizeof(jitter_stats_t));
			if (stats->sent_rtcp != NULL) freemsg(stats->sent_rtcp);
			stats->sent_rtcp = evd->packet;
			evd->packet = NULL;
			stats->updated = LINPHONE_CALL_STATS_SENT_RTCP_UPDATE;
			linphone_call_stats_update(stats, ms);
		} else if (evt == ORTP_EVENT_ZRTP_SAS_READY) {
			stats->zrtp_algo.cipher_algo = evd->info.zrtp_info.cipherAlgo;
			stats->zrtp_algo.key_agreement_algo = evd->info.zrtp_info.keyAgreementAlgo;
			stats->zrtp_algo.hash_algo = evd->info.zrtp_info.hashAlgo;
			stats->zrtp_algo.auth_tag_algo = evd->info.zrtp_info.authTagAlgo;
			stats->zrtp_algo.sas_algo = evd->info.zrtp_info.sasAlgo;
		} else if (evt == ORTP_EVENT_SRTP_ENCRYPTION_CHANGED) {
			if (evd->info.srtp_info.is_inner) {
				if (evd->info.srtp_info.is_send) {
					stats->inner_srtp_info.send_suite = evd->info.srtp_info.suite;
					stats->inner_srtp_info.send_source = evd->info.srtp_info.source;
				} else {
					stats->inner_srtp_info.recv_suite = evd->info.srtp_info.suite;
					stats->inner_srtp_info.recv_source = evd->info.srtp_info.source;
				}
			} else {
				if (evd->info.srtp_info.is_send) {
					stats->srtp_info.send_suite = evd->info.srtp_info.suite;
					stats->srtp_info.send_source = evd->info.srtp_info.source;
				} else {
					stats->srtp_info.recv_suite = evd->info.srtp_info.suite;
					stats->srtp_info.recv_source = evd->info.srtp_info.source;
				}
			}
		}
	}
}

LinphoneStreamType linphone_call_stats_get_type(const LinphoneCallStats *stats) {
	return stats->type;
}

float linphone_call_stats_get_sender_loss_rate(const LinphoneCallStats *stats) {
	const report_block_t *srb = NULL;

	if (!stats->sent_rtcp) {
		ms_warning("linphone_call_stats_get_sender_loss_rate(): there is no RTCP packet sent.");
		return 0.0;
	}
	RtcpParserContext parserCtx;
	const mblk_t *rtcpMessage = rtcp_parser_context_init(&parserCtx, stats->sent_rtcp);

	do {
		if (rtcp_is_SR(rtcpMessage)) srb = rtcp_SR_get_report_block(rtcpMessage, 0);
		else if (rtcp_is_RR(rtcpMessage)) srb = rtcp_RR_get_report_block(rtcpMessage, 0);
		if (srb) break;
	} while ((rtcpMessage = rtcp_parser_context_next_packet(&parserCtx)) != nullptr);
	rtcp_parser_context_uninit(&parserCtx);
	if (!srb) return 0.0;
	return 100.0f * (float)report_block_get_fraction_lost(srb) / 256.0f;
}

float linphone_call_stats_get_receiver_loss_rate(const LinphoneCallStats *stats) {
	const report_block_t *rrb = NULL;

	if (!stats->received_rtcp) {
		ms_warning("linphone_call_stats_get_receiver_loss_rate(): there is no RTCP packet received.");
		return 0.0;
	}
	RtcpParserContext parserCtx;
	const mblk_t *rtcpMessage = rtcp_parser_context_init(&parserCtx, stats->received_rtcp);

	do {
		if (rtcp_is_RR(rtcpMessage)) rrb = rtcp_RR_get_report_block(rtcpMessage, 0);
		else if (rtcp_is_SR(rtcpMessage)) rrb = rtcp_SR_get_report_block(rtcpMessage, 0);
		if (rrb) break;
	} while ((rtcpMessage = rtcp_parser_context_next_packet(&parserCtx)) != nullptr);
	rtcp_parser_context_uninit(&parserCtx);
	if (!rrb) return 0.0;
	return 100.0f * (float)report_block_get_fraction_lost(rrb) / 256.0f;
}

float linphone_call_stats_get_local_loss_rate(const LinphoneCallStats *stats) {
	return stats->local_loss_rate;
}

float linphone_call_stats_get_local_late_rate(const LinphoneCallStats *stats) {
	return stats->local_late_rate;
}

float linphone_call_stats_get_sender_interarrival_jitter(const LinphoneCallStats *stats) {
	const report_block_t *srb = NULL;

	if (!stats->sent_rtcp) {
		ms_warning("linphone_call_stats_get_sender_interarrival_jitter(): there is no RTCP packet sent.");
		return 0.0;
	}
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several
	 * mblk_t structure */
	if (stats->sent_rtcp->b_cont != NULL) msgpullup(stats->sent_rtcp, (size_t)-1);
	if (rtcp_is_SR(stats->sent_rtcp)) srb = rtcp_SR_get_report_block(stats->sent_rtcp, 0);
	else if (rtcp_is_RR(stats->sent_rtcp)) srb = rtcp_RR_get_report_block(stats->sent_rtcp, 0);
	if (!srb) return 0.0;
	if (stats->clockrate == 0) return 0.0;
	return (float)report_block_get_interarrival_jitter(srb) / (float)stats->clockrate;
}

float linphone_call_stats_get_receiver_interarrival_jitter(const LinphoneCallStats *stats) {
	const report_block_t *rrb = NULL;

	if (!stats->received_rtcp) {
		ms_warning("linphone_call_stats_get_receiver_interarrival_jitter(): there is no RTCP packet received.");
		return 0.0;
	}
	/* Perform msgpullup() to prevent crashes in rtcp_is_SR() or rtcp_is_RR() if the RTCP packet is composed of several
	 * mblk_t structure */
	if (stats->received_rtcp->b_cont != NULL) msgpullup(stats->received_rtcp, (size_t)-1);
	if (rtcp_is_SR(stats->received_rtcp)) rrb = rtcp_SR_get_report_block(stats->received_rtcp, 0);
	else if (rtcp_is_RR(stats->received_rtcp)) rrb = rtcp_RR_get_report_block(stats->received_rtcp, 0);
	if (!rrb) return 0.0;
	if (stats->clockrate == 0) return 0.0;
	return (float)report_block_get_interarrival_jitter(rrb) / (float)stats->clockrate;
}

const rtp_stats_t *linphone_call_stats_get_rtp_stats(const LinphoneCallStats *stats) {
	return &stats->rtp_stats;
}

uint64_t linphone_call_stats_get_late_packets_cumulative_number(const LinphoneCallStats *stats) {
	return linphone_call_stats_get_rtp_stats(stats)->outoftime;
}

float linphone_call_stats_get_download_bandwidth(const LinphoneCallStats *stats) {
	return stats->download_bandwidth;
}

float linphone_call_stats_get_upload_bandwidth(const LinphoneCallStats *stats) {
	return stats->upload_bandwidth;
}

float linphone_call_stats_get_rtcp_download_bandwidth(const LinphoneCallStats *stats) {
	return stats->rtcp_download_bandwidth;
}

float linphone_call_stats_get_rtcp_upload_bandwidth(const LinphoneCallStats *stats) {
	return stats->rtcp_upload_bandwidth;
}

LinphoneIceState linphone_call_stats_get_ice_state(const LinphoneCallStats *stats) {
	return stats->ice_state;
}

LinphoneUpnpState linphone_call_stats_get_upnp_state(const LinphoneCallStats *stats) {
	return stats->upnp_state;
}

LinphoneAddressFamily linphone_call_stats_get_ip_family_of_remote(const LinphoneCallStats *stats) {
	return (LinphoneAddressFamily)stats->rtp_remote_family;
}

float linphone_call_stats_get_jitter_buffer_size_ms(const LinphoneCallStats *stats) {
	return stats->jitter_stats.jitter_buffer_size_ms;
}

float linphone_call_stats_get_round_trip_delay(const LinphoneCallStats *stats) {
	return stats->round_trip_delay;
}

float linphone_call_stats_get_estimated_download_bandwidth(const LinphoneCallStats *stats) {
	return stats->estimated_download_bandwidth;
}

void linphone_call_stats_set_estimated_download_bandwidth(LinphoneCallStats *stats, float estimated_value) {
	stats->estimated_download_bandwidth = estimated_value;
}

const SrtpInfo *linphone_call_stats_get_srtp_info(const LinphoneCallStats *stats, bool_t is_inner) {
	if (is_inner == TRUE) {
		return &stats->inner_srtp_info;
	} else {
		return &stats->srtp_info;
	}
}

const ZrtpAlgo *linphone_call_stats_get_zrtp_algo(const LinphoneCallStats *stats) {
	return &stats->zrtp_algo;
}

const char *linphone_call_stats_get_zrtp_cipher_algo(const LinphoneCallStats *stats) {
	switch (stats->zrtp_algo.cipher_algo) {
		case (MS_ZRTP_CIPHER_INVALID):
			return "invalid";
		case (MS_ZRTP_CIPHER_AES1):
			return "AES-128";
		case (MS_ZRTP_CIPHER_AES2):
			return "AES-192";
		case (MS_ZRTP_CIPHER_AES3):
			return "AES-256";
		case (MS_ZRTP_CIPHER_2FS1):
			return "TwoFish-128";
		case (MS_ZRTP_CIPHER_2FS2):
			return "TwoFish-192";
		case (MS_ZRTP_CIPHER_2FS3):
			return "TwoFish-256";
		default:
			return "Unknown Algo";
	}
}
const char *linphone_call_stats_get_zrtp_key_agreement_algo(const LinphoneCallStats *stats) {
	switch (stats->zrtp_algo.key_agreement_algo) {
		case (MS_ZRTP_KEY_AGREEMENT_INVALID):
			return "invalid";
		case (MS_ZRTP_KEY_AGREEMENT_DH2K):
			return "DHM-2048";
		case (MS_ZRTP_KEY_AGREEMENT_EC25):
			return "ECDH-256";
		case (MS_ZRTP_KEY_AGREEMENT_DH3K):
			return "DHM-3072";
		case (MS_ZRTP_KEY_AGREEMENT_EC38):
			return "ECDH-384";
		case (MS_ZRTP_KEY_AGREEMENT_EC52):
			return "ECDH-521";
		case (MS_ZRTP_KEY_AGREEMENT_X255):
			return "X25519";
		case (MS_ZRTP_KEY_AGREEMENT_X448):
			return "X448";
		case (MS_ZRTP_KEY_AGREEMENT_K255):
			return "KEM-X25519";
		case (MS_ZRTP_KEY_AGREEMENT_K448):
			return "KEM-X448";
		case (MS_ZRTP_KEY_AGREEMENT_KYB1):
			return "KYBER-512";
		case (MS_ZRTP_KEY_AGREEMENT_KYB2):
			return "KYBER-768";
		case (MS_ZRTP_KEY_AGREEMENT_KYB3):
			return "KYBER-1024";
		case (MS_ZRTP_KEY_AGREEMENT_HQC1):
			return "HQC-128";
		case (MS_ZRTP_KEY_AGREEMENT_HQC2):
			return "HQC-192";
		case (MS_ZRTP_KEY_AGREEMENT_HQC3):
			return "HQC-256";
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512):
			return "X25519/Kyber512";
		case (MS_ZRTP_KEY_AGREEMENT_K255_HQC128):
			return "X25519/HQC128";
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024):
			return "X448/Kyber1024";
		case (MS_ZRTP_KEY_AGREEMENT_K448_HQC256):
			return "X448/HQC256";
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128):
			return "X25519/Kyber512/HQC128";
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256):
			return "X448/Kyber1024/HQC256";
		default:
			return "Unknown Algo";
	}
}

bool_t linphone_call_stats_is_zrtp_key_agreement_algo_post_quantum(const LinphoneCallStats *stats) {
	switch (stats->zrtp_algo.key_agreement_algo) {
		case (MS_ZRTP_KEY_AGREEMENT_KYB1):
		case (MS_ZRTP_KEY_AGREEMENT_KYB2):
		case (MS_ZRTP_KEY_AGREEMENT_KYB3):
		case (MS_ZRTP_KEY_AGREEMENT_HQC1):
		case (MS_ZRTP_KEY_AGREEMENT_HQC2):
		case (MS_ZRTP_KEY_AGREEMENT_HQC3):
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512):
		case (MS_ZRTP_KEY_AGREEMENT_K255_HQC128):
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024):
		case (MS_ZRTP_KEY_AGREEMENT_K448_HQC256):
		case (MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128):
		case (MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256):
			return TRUE;
		default:
			return FALSE;
	}
}

const char *linphone_call_stats_get_zrtp_hash_algo(const LinphoneCallStats *stats) {
	switch (stats->zrtp_algo.hash_algo) {
		case (MS_ZRTP_HASH_INVALID):
			return "invalid";
		case (MS_ZRTP_HASH_S256):
			return "SHA-256";
		case (MS_ZRTP_HASH_S384):
			return "SHA-384";
		case (MS_ZRTP_HASH_N256):
			return "SHA3-256";
		case (MS_ZRTP_HASH_N384):
			return "SHA3-384";
		case (MS_ZRTP_HASH_S512):
			return "SHA-512";
		default:
			return "Unknown Algo";
	}
}
const char *linphone_call_stats_get_zrtp_auth_tag_algo(const LinphoneCallStats *stats) {
	switch (stats->zrtp_algo.auth_tag_algo) {
		case (MS_ZRTP_AUTHTAG_INVALID):
			return "invalid";
		case (MS_ZRTP_AUTHTAG_HS32):
			return "HMAC-SHA1-32";
		case (MS_ZRTP_AUTHTAG_HS80):
			return "HMAC-SHA1-80";
		case (MS_ZRTP_AUTHTAG_SK32):
			return "Skein-32";
		case (MS_ZRTP_AUTHTAG_SK64):
			return "Skein-64";
		case (MS_ZRTP_AUTHTAG_GCM):
			return "GCM";
		default:
			return "Unknown Algo";
	}
}
const char *linphone_call_stats_get_zrtp_sas_algo(const LinphoneCallStats *stats) {
	switch (stats->zrtp_algo.sas_algo) {
		case (MS_ZRTP_SAS_INVALID):
			return "invalid";
		case (MS_ZRTP_SAS_B32):
			return "Base32";
		case (MS_ZRTP_SAS_B256):
			return "PGP-WordList";
		default:
			return "Unknown Algo";
	}
}
