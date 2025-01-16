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
#include "call/call-stats.h"
#include "mediastreamer2/zrtp.h"
#include "private.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

// -----------------------------------------------------------------------------

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

// =============================================================================
// Private functions
// =============================================================================

LinphoneCallStats *_linphone_call_stats_new() {
	return CallStats::createCObject();
}

void _linphone_call_stats_set_ice_state(LinphoneCallStats *stats, LinphoneIceState state) {
	CallStats::toCpp(stats)->setIceState(state);
}

void _linphone_call_stats_set_type(LinphoneCallStats *stats, LinphoneStreamType type) {
	CallStats::toCpp(stats)->setType(type);
}

mblk_t *_linphone_call_stats_get_received_rtcp(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getReceivedRtcp();
}

void _linphone_call_stats_set_received_rtcp(LinphoneCallStats *stats, mblk_t *m) {
	CallStats::toCpp(stats)->setReceivedRtcp(m);
}

mblk_t *_linphone_call_stats_get_sent_rtcp(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getSentRtcp();
}

void _linphone_call_stats_set_sent_rtcp(LinphoneCallStats *stats, mblk_t *m) {
	CallStats::toCpp(stats)->setSentRtcp(m);
}

int _linphone_call_stats_get_updated(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getUpdated();
}

void _linphone_call_stats_set_updated(LinphoneCallStats *stats, int updated) {
	CallStats::toCpp(stats)->setUpdated(updated);
}

void _linphone_call_stats_set_rtp_stats(LinphoneCallStats *stats, const rtp_stats_t *rtpStats) {
	CallStats::toCpp(stats)->setRtpStats(rtpStats);
}

void _linphone_call_stats_set_download_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	CallStats::toCpp(stats)->setDownloadBandwidth(bandwidth);
}

void _linphone_call_stats_set_upload_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	CallStats::toCpp(stats)->setUploadBandwidth(bandwidth);
}

void _linphone_call_stats_set_fec_download_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	CallStats::toCpp(stats)->setFecDownloadBandwidth(bandwidth);
}

void _linphone_call_stats_set_fec_upload_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	CallStats::toCpp(stats)->setFecUploadBandwidth(bandwidth);
}

void _linphone_call_stats_set_rtcp_download_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	CallStats::toCpp(stats)->setRtcpDownloadBandwidth(bandwidth);
}

void _linphone_call_stats_set_rtcp_upload_bandwidth(LinphoneCallStats *stats, float bandwidth) {
	CallStats::toCpp(stats)->setRtcpUploadBandwidth(bandwidth);
}

void _linphone_call_stats_set_ip_family_of_remote(LinphoneCallStats *stats, LinphoneAddressFamily family) {
	CallStats::toCpp(stats)->setIpFamilyOfRemote(family);
}

bool_t _linphone_call_stats_rtcp_received_via_mux(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->rtcpReceivedViaMux();
}

bool_t _linphone_call_stats_has_received_rtcp(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->hasReceivedRtcp();
}

bool_t _linphone_call_stats_has_sent_rtcp(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->hasSentRtcp();
}

// =============================================================================
// Public functions
// =============================================================================

LinphoneCallStats *linphone_call_stats_ref(LinphoneCallStats *stats) {
	CallStats::toCpp(stats)->ref();
	return stats;
}

void linphone_call_stats_unref(LinphoneCallStats *stats) {
	CallStats::toCpp(stats)->unref();
}

void *linphone_call_stats_get_user_data(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getUserData();
}

void linphone_call_stats_set_user_data(LinphoneCallStats *stats, void *data) {
	CallStats::toCpp(stats)->setUserData(data);
}

void linphone_call_stats_update(LinphoneCallStats *stats, MediaStream *stream) {
	CallStats::toCpp(stats)->update(stream);
}

/*do not change the prototype of this function, it is also used internally in linphone-daemon.*/
void linphone_call_stats_fill(LinphoneCallStats *stats, MediaStream *ms, OrtpEvent *ev) {
	CallStats::toCpp(stats)->fill(ms, ev);
}

LinphoneStreamType linphone_call_stats_get_type(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getType();
}

float linphone_call_stats_get_sender_loss_rate(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getSenderLossRate();
}

float linphone_call_stats_get_receiver_loss_rate(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getReceiverLossRate();
}

float linphone_call_stats_get_local_loss_rate(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getLocalLossRate();
}

float linphone_call_stats_get_local_late_rate(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getLocalLateRate();
}

float linphone_call_stats_get_sender_interarrival_jitter(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getSenderInterarrivalJitter();
}

float linphone_call_stats_get_receiver_interarrival_jitter(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getReceiverInterarrivalJitter();
}

const rtp_stats_t *linphone_call_stats_get_rtp_stats(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpStats();
}

uint64_t linphone_call_stats_get_late_packets_cumulative_number(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getLatePacketsCumulativeNumber();
}

uint64_t linphone_call_stats_get_fec_cumulative_lost_packets_number(const LinphoneCallStats *stats) {
	if (stats) return CallStats::toCpp(stats)->getFecCumulativeLostPacketsNumber();
	else return 0;
}

uint64_t linphone_call_stats_get_fec_repaired_packets_number(const LinphoneCallStats *stats) {
	if (stats) return CallStats::toCpp(stats)->getFecRepairedPacketsNumber();
	else return 0;
}

float linphone_call_stats_get_download_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getDownloadBandwidth();
}

float linphone_call_stats_get_upload_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getUploadBandwidth();
}

float linphone_call_stats_get_fec_download_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getFecDownloadBandwidth();
}

float linphone_call_stats_get_fec_upload_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getFecUploadBandwidth();
}

float linphone_call_stats_get_rtcp_download_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtcpDownloadBandwidth();
}

float linphone_call_stats_get_rtcp_upload_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtcpUploadBandwidth();
}

LinphoneIceState linphone_call_stats_get_ice_state(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getIceState();
}

LinphoneUpnpState linphone_call_stats_get_upnp_state(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getUpnpState();
}

LinphoneAddressFamily linphone_call_stats_get_ip_family_of_remote(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getIpFamilyOfRemote();
}

float linphone_call_stats_get_jitter_buffer_size_ms(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getJitterBufferSizeMs();
}

float linphone_call_stats_get_round_trip_delay(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRoundTripDelay();
}

float linphone_call_stats_get_estimated_download_bandwidth(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getEstimatedDownloadBandwidth();
}

void linphone_call_stats_set_estimated_download_bandwidth(LinphoneCallStats *stats, float estimated_value) {
	CallStats::toCpp(stats)->setEstimatedDownloadBandwidth(estimated_value);
}

const SrtpInfo *linphone_call_stats_get_srtp_info(const LinphoneCallStats *stats, bool_t is_inner) {
	return CallStats::toCpp(stats)->getSrtpInfo(is_inner);
}

const ZrtpAlgo *linphone_call_stats_get_zrtp_algo(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getZrtpAlgo();
}

const char *linphone_call_stats_get_zrtp_cipher_algo(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getZrtpCipherAlgo();
}
const char *linphone_call_stats_get_zrtp_key_agreement_algo(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getZrtpKeyAgreementAlgo();
}

bool_t linphone_call_stats_is_zrtp_key_agreement_algo_post_quantum(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->isZrtpKeyAgreementAlgoPostQuantum();
}

const char *linphone_call_stats_get_zrtp_hash_algo(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getZrtpHashAlgo();
}
const char *linphone_call_stats_get_zrtp_auth_tag_algo(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getZrtpAuthTagAlgo();
}
const char *linphone_call_stats_get_zrtp_sas_algo(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getZrtpSasAlgo();
}
LinphoneSrtpSuite linphone_call_stats_get_srtp_suite(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getSrtpSuite();
}
LinphoneMediaEncryption linphone_call_stats_get_srtp_source(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getSrtpSource();
}
uint64_t linphone_call_stats_get_rtp_packet_sent(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpPacketSent();
}
uint64_t linphone_call_stats_get_rtp_packet_recv(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpPacketRcv();
}
uint64_t linphone_call_stats_get_rtp_sent(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpSent();
}
uint64_t linphone_call_stats_get_rtp_recv(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpRecv();
}
uint64_t linphone_call_stats_get_rtp_hw_recv(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpHwRecv();
}
int64_t linphone_call_stats_get_rtp_cum_packet_loss(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpCumPacketLoss();
}
uint64_t linphone_call_stats_get_rtp_discarded(const LinphoneCallStats *stats) {
	return CallStats::toCpp(stats)->getRtpDiscarded();
}
