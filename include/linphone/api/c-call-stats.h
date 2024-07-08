/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_C_CALL_STATS_H_
#define _L_C_CALL_STATS_H_

#include <ortp/rtp.h>

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup call_misc
 * @{
 */

#define LINPHONE_CALL_STATS_AUDIO ((int)LinphoneStreamTypeAudio)
#define LINPHONE_CALL_STATS_VIDEO ((int)LinphoneStreamTypeVideo)
#define LINPHONE_CALL_STATS_TEXT ((int)LinphoneStreamTypeText)

#define LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE                                                                       \
	(1 << 0) /**< received_rtcp field of LinphoneCallStats object has been updated */
#define LINPHONE_CALL_STATS_SENT_RTCP_UPDATE                                                                           \
	(1 << 1) /**< sent_rtcp field of LinphoneCallStats object has been updated */
#define LINPHONE_CALL_STATS_PERIODICAL_UPDATE (1 << 2) /**< Every seconds LinphoneCallStats object has been updated */

/**
 * Increment refcount.
 * @param stats #LinphoneCallStats object @notnil
 * @return the same #LinphoneCallStats object @notnil
 * @ingroup misc
 **/
LINPHONE_PUBLIC LinphoneCallStats *linphone_call_stats_ref(LinphoneCallStats *stats);

/**
 * Decrement refcount and possibly free the object.
 * @param stats #LinphoneCallStats object @notnil
 * @ingroup misc
 **/
LINPHONE_PUBLIC void linphone_call_stats_unref(LinphoneCallStats *stats);

/**
 * Gets the user data in the #LinphoneCallStats object
 * @param[in] stats the #LinphoneCallStats
 * @return the user data. @maybenil
 * @ingroup misc
 */
LINPHONE_PUBLIC void *linphone_call_stats_get_user_data(const LinphoneCallStats *stats);

/**
 * Sets the user data in the #LinphoneCallStats object
 * @param[in] stats the #LinphoneCallStats object
 * @param[in] data the user data. @maybenil
 * @ingroup misc
 */
LINPHONE_PUBLIC void linphone_call_stats_set_user_data(LinphoneCallStats *stats, void *data);

/**
 * Get the type of the stream the stats refer to.
 * @param stats #LinphoneCallStats object @notnil
 * @return The #LinphoneStreamType the stats refer to
 */
LINPHONE_PUBLIC LinphoneStreamType linphone_call_stats_get_type(const LinphoneCallStats *stats);

/**
 * Get the local loss rate since last report
 * @param stats #LinphoneCallStats object @notnil
 * @return The sender loss rate
 **/
LINPHONE_PUBLIC float linphone_call_stats_get_sender_loss_rate(const LinphoneCallStats *stats);

/**
 * Gets the remote reported loss rate since last report
 * @param stats #LinphoneCallStats object @notnil
 * @return The receiver loss rate
 **/
LINPHONE_PUBLIC float linphone_call_stats_get_receiver_loss_rate(const LinphoneCallStats *stats);

/**
 * Get the local loss rate since last report
 * @param stats #LinphoneCallStats object @notnil
 * @return The local loss rate
 **/
LINPHONE_PUBLIC float linphone_call_stats_get_local_loss_rate(const LinphoneCallStats *stats);

/**
 * Gets the local late rate since last report
 * @param stats #LinphoneCallStats object @notnil
 * @return The local late rate
 **/
LINPHONE_PUBLIC float linphone_call_stats_get_local_late_rate(const LinphoneCallStats *stats);

/**
 * Gets the local interarrival jitter
 * @param stats #LinphoneCallStats object @notnil
 * @return The interarrival jitter at last emitted sender report
 **/
LINPHONE_PUBLIC float linphone_call_stats_get_sender_interarrival_jitter(const LinphoneCallStats *stats);

/**
 * Gets the remote reported interarrival jitter
 * @param stats #LinphoneCallStats object @notnil
 * @return The interarrival jitter at last received receiver report
 **/
LINPHONE_PUBLIC float linphone_call_stats_get_receiver_interarrival_jitter(const LinphoneCallStats *stats);

LINPHONE_PUBLIC const rtp_stats_t *linphone_call_stats_get_rtp_stats(const LinphoneCallStats *stats);

/**
 * Gets the cumulative number of late packets
 * @param stats #LinphoneCallStats object @notnil
 * @return The cumulative number of late packets
 **/
LINPHONE_PUBLIC uint64_t linphone_call_stats_get_late_packets_cumulative_number(const LinphoneCallStats *stats);

/**
 * If the FEC is enabled, gets the cumulative number of lost source packets of the RTP session that have not been
 *repaired by the current FEC stream.
 * @param stats #LinphoneCallStats object @notnil
 * @return The cumulative number of lost packets
 **/
LINPHONE_PUBLIC uint64_t linphone_call_stats_get_fec_cumulative_lost_packets_number(const LinphoneCallStats *stats);

/**
 * If the FEC is enabled, gets the cumulative number of source packets of the RTP session that have been repaired by the
 *current FEC stream.
 * @param stats #LinphoneCallStats object @notnil
 * @return The cumulative number of repaired packets
 **/
LINPHONE_PUBLIC uint64_t linphone_call_stats_get_fec_repaired_packets_number(const LinphoneCallStats *stats);

/**
 * Get the bandwidth measurement of the received stream, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The bandwidth measurement of the received stream in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_download_bandwidth(const LinphoneCallStats *stats);

/**
 * Get the bandwidth measurement of the sent stream, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The bandwidth measurement of the sent stream in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_upload_bandwidth(const LinphoneCallStats *stats);

/**
 * Get the bandwidth measurement of the part of the received stream dedicated to FEC, expressed in kbit/s, including
 * IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The bandwidth measurement of the received FEC stream in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_fec_download_bandwidth(const LinphoneCallStats *stats);

/**
 * Get the bandwidth measurement of the part of the sent stream dedicated to FEC, expressed in kbit/s, including
 * IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The bandwidth measurement of the sent stream in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_fec_upload_bandwidth(const LinphoneCallStats *stats);

/**
 * Get the bandwidth measurement of the received RTCP, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The bandwidth measurement of the received RTCP in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_rtcp_download_bandwidth(const LinphoneCallStats *stats);

/**
 * Get the bandwidth measurement of the sent RTCP, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The bandwidth measurement of the sent RTCP in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_rtcp_upload_bandwidth(const LinphoneCallStats *stats);

/**
 * Get the state of ICE processing.
 * @param stats #LinphoneCallStats object @notnil
 * @return The #LinphoneIceState of ICE processing
 */
LINPHONE_PUBLIC LinphoneIceState linphone_call_stats_get_ice_state(const LinphoneCallStats *stats);

/**
 * Get the state of uPnP processing.
 * @param stats #LinphoneCallStats object @notnil
 * @return The #LinphoneUpnpState of uPnP processing.
 */
LINPHONE_PUBLIC LinphoneUpnpState linphone_call_stats_get_upnp_state(const LinphoneCallStats *stats);

/**
 * Get the IP address family of the remote peer.
 * @param stats #LinphoneCallStats object @notnil
 * @return The IP address family #LinphoneAddressFamily of the remote peer.
 */
LINPHONE_PUBLIC LinphoneAddressFamily linphone_call_stats_get_ip_family_of_remote(const LinphoneCallStats *stats);

/**
 * Get the jitter buffer size in ms.
 * @param stats #LinphoneCallStats object @notnil
 * @return The jitter buffer size in ms.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_jitter_buffer_size_ms(const LinphoneCallStats *stats);

/**
 * Get the round trip delay in s.
 * @param stats #LinphoneCallStats object @notnil
 * @return The round trip delay in s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_round_trip_delay(const LinphoneCallStats *stats);

/**
 * Get the estimated bandwidth measurement of the received stream, expressed in kbit/s, including IP/UDP/RTP headers.
 * @param stats #LinphoneCallStats object @notnil
 * @return The estimated bandwidth measurement of the received stream in kbit/s.
 */
LINPHONE_PUBLIC float linphone_call_stats_get_estimated_download_bandwidth(const LinphoneCallStats *stats);

void linphone_call_stats_set_estimated_download_bandwidth(LinphoneCallStats *stats, float estimated_value);

/**
 * Get the ZRTP algorithm statistics details (cipher)
 * @param stats #LinphoneCallStats object @notnil
 * @return The cipher algo
 */
LINPHONE_PUBLIC const char *linphone_call_stats_get_zrtp_cipher_algo(const LinphoneCallStats *stats);

/**
 * Get the ZRTP algorithm statistics details (key agreeement)
 * @param stats #LinphoneCallStats object @notnil
 * @return The key agreement algo
 */
LINPHONE_PUBLIC const char *linphone_call_stats_get_zrtp_key_agreement_algo(const LinphoneCallStats *stats);

/**
 * Did ZRTP used a Post Quantum algorithm to perform a key exchange
 * @param stats #LinphoneCallStats object @notnil
 * @return TRUE if the ZRTP key exchange was performed using a PQ algo
 *         FALSE otherwise: ZRTP exchange not completed or not using a PQ algo
 */
LINPHONE_PUBLIC bool_t linphone_call_stats_is_zrtp_key_agreement_algo_post_quantum(const LinphoneCallStats *stats);

/**
 * Get the ZRTP algorithm statistics details (hash function)
 * @param stats #LinphoneCallStats object @notnil
 * @return The hash algo
 */
LINPHONE_PUBLIC const char *linphone_call_stats_get_zrtp_hash_algo(const LinphoneCallStats *stats);

/**
 * Get the ZRTP algorithm statistics details (authentication method)
 * @param stats #LinphoneCallStats object @notnil
 * @return The auth tag algo
 */
LINPHONE_PUBLIC const char *linphone_call_stats_get_zrtp_auth_tag_algo(const LinphoneCallStats *stats);

/**
 * Get the ZRTP algorithm statistics details (SAS display)
 * @param stats #LinphoneCallStats object @notnil
 * @return The sas algo
 */
LINPHONE_PUBLIC const char *linphone_call_stats_get_zrtp_sas_algo(const LinphoneCallStats *stats);

/**
 * Get the SRTP Cryto suite in use
 * @param stats #LinphoneCallStats object @notnil
 * @return The SRTP crypto suite currently in use #LinphoneSrtpSuite @notnil
 */
LINPHONE_PUBLIC LinphoneSrtpSuite linphone_call_stats_get_srtp_suite(const LinphoneCallStats *stats);

/**
 * Get the method used for SRTP key exchange
 * @param stats #LinphoneCallStats object @notnil
 * @return The #LinphoneMediaEncryption method used to exchange the SRTP keys @notnil
 */
LINPHONE_PUBLIC LinphoneMediaEncryption linphone_call_stats_get_srtp_source(const LinphoneCallStats *stats);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CALL_STATS_H_
