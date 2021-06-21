/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include <bctoolbox/tester.h>

#include "tester_utils.h"
#include "call/call.h"
#include "sal/call-op.h"
#include "sal/sal_media_description.h"
#include "shared_tester_functions.h"
#include "c-wrapper/internal/c-tools.h"

using namespace LinphonePrivate;

static void check_ice_from_rtp(LinphoneCall *c1, LinphoneCall *c2, LinphoneStreamType stream_type) {
	MediaStream *ms;
	LinphoneCallStats *stats;
	switch (stream_type) {
	case LinphoneStreamTypeAudio:
		ms=linphone_call_get_stream(c1, LinphoneStreamTypeAudio);
		break;
	case LinphoneStreamTypeVideo:
		ms=linphone_call_get_stream(c1, LinphoneStreamTypeVideo);
		break;
	case LinphoneStreamTypeText:
		ms=linphone_call_get_stream(c1, LinphoneStreamTypeText);
		break;
	default:
		lError() << "Unknown stream type [" << linphone_stream_type_to_string(stream_type) << "]";
		BC_ASSERT_FALSE(stream_type >= LinphoneStreamTypeUnknown);
		return;
	}

	stats = linphone_call_get_audio_stats(c1);
	if (linphone_call_stats_get_ice_state(stats) == LinphoneIceStateHostConnection && media_stream_started(ms)) {
		struct sockaddr_storage remaddr;
		socklen_t remaddrlen = sizeof(remaddr);
		char ip[NI_MAXHOST] = { 0 };
		int port = 0;
		std::string expected_addr;
		AudioStream *astream;

		const LinphoneCallParams *cp1 = linphone_call_get_current_params(c1);
		const LinphoneCallParams *cp2 = linphone_call_get_current_params(c2);
		if (linphone_call_params_get_update_call_when_ice_completed(cp1) && linphone_call_params_get_update_call_when_ice_completed(cp2)) {
			memset(&remaddr, 0, remaddrlen);

			LinphonePrivate::SalCallOp * op = LinphonePrivate::Call::toCpp(c2)->getOp();
			const std::shared_ptr<SalMediaDescription> & result_desc = op->getFinalMediaDescription();
			const auto & result_stream = result_desc->getStreamIdx(0);
			if (result_stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				expected_addr = result_stream.getRtpAddress();
			}
			if (expected_addr.empty()) {
				expected_addr = result_desc->getAddress();
			}
			astream = (AudioStream *)linphone_call_get_stream(c1, LinphoneStreamTypeAudio);
			if ((expected_addr.find(':') == std::string::npos) && (astream->ms.sessions.rtp_session->rtp.gs.rem_addr.ss_family == AF_INET6)) {
				bctbx_sockaddr_ipv6_to_ipv4((struct sockaddr *)&astream->ms.sessions.rtp_session->rtp.gs.rem_addr, (struct sockaddr *)&remaddr, &remaddrlen);
			} else {
				memcpy(&remaddr, &astream->ms.sessions.rtp_session->rtp.gs.rem_addr, astream->ms.sessions.rtp_session->rtp.gs.rem_addrlen);
			}
			bctbx_sockaddr_to_ip_address((struct sockaddr *)&remaddr, remaddrlen, ip, sizeof(ip), &port);

			BC_ASSERT_STRING_EQUAL(ip, expected_addr.c_str());

		}
	}
	linphone_call_stats_unref(stats);
}

bool_t check_ice(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneIceState state) {
	LinphoneCall *c1,*c2;
	bool_t global_success = TRUE;
	bool_t audio_success=FALSE;
	bool_t video_success=FALSE;
	bool_t text_success=FALSE;
	bool_t audio_enabled, video_enabled, realtime_text_enabled;
	MSTimeSpec ts;

	c1=linphone_core_get_current_call(caller->lc);
	c2=linphone_core_get_current_call(callee->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (!c1 || !c2) return FALSE;
	linphone_call_ref(c1);
	linphone_call_ref(c2);

	BC_ASSERT_EQUAL(linphone_call_params_video_enabled(linphone_call_get_current_params(c1)),linphone_call_params_video_enabled(linphone_call_get_current_params(c2)), int, "%d");
	BC_ASSERT_EQUAL(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1)),linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c2)), int, "%d");
	audio_enabled=linphone_call_params_audio_enabled(linphone_call_get_current_params(c1));
	video_enabled=linphone_call_params_video_enabled(linphone_call_get_current_params(c1));
	realtime_text_enabled=linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1));
	if (audio_enabled) {
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_audio_stats(c1);
				stats2 = linphone_call_get_audio_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1)==state &&
					linphone_call_stats_get_ice_state(stats2)==state){
					audio_success=TRUE;
					check_ice_from_rtp(c1,c2,LinphoneStreamTypeAudio);
					check_ice_from_rtp(c2,c1,LinphoneStreamTypeAudio);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts,10000));
		if (stats1)
			linphone_call_stats_unref(stats1);
		if (stats2)
			linphone_call_stats_unref(stats2);
	}

	if (video_enabled){
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_video_stats(c1);
				stats2 = linphone_call_get_video_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1)==state &&
					linphone_call_stats_get_ice_state(stats2)==state){
					video_success=TRUE;
					check_ice_from_rtp(c1,c2,LinphoneStreamTypeVideo);
					check_ice_from_rtp(c2,c1,LinphoneStreamTypeVideo);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts,10000));
		if (stats1)
			linphone_call_stats_unref(stats1);
		if (stats2)
			linphone_call_stats_unref(stats2);
	}

	if (realtime_text_enabled){
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_text_stats(c1);
				stats2 = linphone_call_get_text_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1)==state &&
					linphone_call_stats_get_ice_state(stats2)==state){
					text_success=TRUE;
					check_ice_from_rtp(c1,c2,LinphoneStreamTypeText);
					check_ice_from_rtp(c2,c1,LinphoneStreamTypeText);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts,10000));
		if (stats1)
			linphone_call_stats_unref(stats1);
		if (stats2)
			linphone_call_stats_unref(stats2);
	}

	linphone_call_unref(c1);
	linphone_call_unref(c2);
	if (audio_enabled) global_success = global_success && audio_success;
	if (video_enabled) global_success = global_success && video_success;
	if (realtime_text_enabled) global_success = global_success && text_success;
	return global_success;
}

bool_t check_ice_sdp (LinphoneCall *call) {
	SalMediaDescription *desc = _linphone_call_get_local_desc(call);
	belle_sdp_session_description_t *sdp = desc->toSdp();
	const char *value=belle_sdp_session_description_get_attribute_value(sdp,"ice-ufrag");
	if (value) return TRUE;
	return FALSE;
}

bool_t is_srtp_secured (LinphoneCall *call, LinphoneStreamType ctype) {
	SalStreamType type = SalOther;
	switch(ctype){
		case LinphoneStreamTypeAudio:
			type = SalAudio;
		break;
		case LinphoneStreamTypeVideo:
			type = SalVideo;
		break;
		case LinphoneStreamTypeText:
			type = SalText;
		break;
		default:
			type = SalOther;
		break;
	}
	SalMediaDescription *desc = _linphone_call_get_result_desc(call);
	const SalStreamDescription & stream = desc->findBestStream(type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) return FALSE;
	if (stream.hasSrtp()) {
		const auto & streamCryptos = stream.getCryptos();
		for (const auto & crypto : streamCryptos) {
			const auto & algo = crypto.algo;
			//return (!ms_crypto_suite_is_unencrypted(algo) && !ms_crypto_suite_is_unauthenticated(algo));
			return (!ms_crypto_suite_is_unencrypted(algo));
		}
	}
	return FALSE;
}
