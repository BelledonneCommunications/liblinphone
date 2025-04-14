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

#include <bctoolbox/tester.h>

#include "mediastreamer2/msmire.h"

#include "c-wrapper/internal/c-tools.h"
#include "call/call.h"
#include "conference/conference-info.h"
#include "conference/conference-scheduler.h"
#include "conference/params/media-session-params.h"
#include "conference/participant-device.h"
#include "conference/participant-info.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "conference/sip-conference-scheduler.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-conference-scheduler.h"
#include "linphone/api/c-participant.h"
#include "sal/call-op.h"
#include "sal/sal_media_description.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"
#ifdef HAVE_LIME_X3DH
#include "lime/lime.hpp"
#endif // HAVE_LIME_X3DH

using namespace std;
#include <sstream>

using namespace LinphonePrivate;
#ifdef HAVE_LIME_X3DH
namespace {
std::vector<std::pair<lime::CurveId, std::string>> parseLimeIks(const char *attr) {
	std::vector<std::pair<lime::CurveId, std::string>> algoIks;
	std::stringstream lineStream{std::string(attr)};
	std::string algoIkString;

	// Split line by semi colon fields
	while (std::getline(lineStream, algoIkString, ';')) {
		// split the field found by colon
		std::string algoString, Ikb64;
		std::stringstream algoIkStream(algoIkString);
		std::getline(algoIkStream, algoString, ':');
		std::getline(algoIkStream, Ikb64);

		auto algo = lime::string2CurveId(algoString);
		// check the algo is valid
		if (algo == lime::CurveId::unset) {
			lWarning() << "Invalid lime algo[" << algoString << "] skip it";
		} else {
			algoIks.emplace_back(algo, Ikb64);
		}
	}
	return algoIks;
}
} // anonymous namespace
#endif // HAVE_LIME_X3DH

void check_lime_ik(LinphoneCoreManager *mgr, BCTBX_UNUSED(LinphoneCall *call)) {

	if (!linphone_core_lime_x3dh_enabled(mgr->lc)) {
		return;
	}

#ifdef HAVE_LIME_X3DH
	// Do not check Ik if database is encrypted
	if (is_filepath_encrypted(mgr->lime_database_path)) {
		return;
	}
	const LinphoneCallParams *call_parameters = linphone_call_get_params(call);
	LinphoneAccount *call_account = linphone_call_params_get_account(call_parameters);
	BC_ASSERT_PTR_NOT_NULL(call_account);
	char *refIk = NULL;
	if (call_account) {
		const LinphoneAccountParams *call_account_params = linphone_account_get_params(call_account);
		const char *lime_server_url = linphone_account_params_get_lime_server_url(call_account_params);
		bool_t lime_server_found = (lime_server_url != NULL);
		if (!lime_server_found) return;
		const LinphoneAddress *call_account_contact = linphone_account_get_contact_address(call_account);
		char *call_account_contact_str = linphone_address_as_string_uri_only(call_account_contact);
		auto coreDefaultCurve =
		    lime::string2CurveId(linphone_config_get_string(mgr->lc->config, "lime", "curve", "c25519"));
		refIk = lime_get_userIk(mgr, call_account_contact_str, static_cast<uint8_t>(coreDefaultCurve));
		BC_ASSERT_PTR_NOT_NULL(refIk);

		SalMediaDescription *desc = _linphone_call_get_local_desc(call);
		belle_sdp_session_description_t *sdp = desc->toSdp();
		const char *ik = belle_sdp_session_description_get_attribute_value(sdp, "Ik");
		BC_ASSERT_PTR_NOT_NULL(ik); // for now we always have an Ik as the default is c25519 but at some point (see
		                            // comment in lime-x3dh-encryption-engine.cpp) it will not be true
		BC_ASSERT_PTR_NOT_NULL(refIk);
		if (refIk && ik) BC_ASSERT_STRING_EQUAL(refIk, ik);
		if (refIk) ms_free(refIk);

		// Check lime-Iks param
		const char *limeIks = belle_sdp_session_description_get_attribute_value(sdp, "lime-Iks");
		if (limeIks) { // We do have a lime-Iks : parse it and check it matches what's in db
			auto Iks = parseLimeIks(limeIks);
			for (const auto &Ik : Iks) {
				auto refIk = lime_get_userIk(mgr, call_account_contact_str, static_cast<uint8_t>(Ik.first));
				BC_ASSERT_PTR_NOT_NULL(refIk);
				if (refIk) {
					BC_ASSERT_STRING_EQUAL(refIk, Ik.second.c_str());
					bctbx_free(refIk);
				}
			}
		}
		ms_free(call_account_contact_str);
	}
#endif // HAVE_LIME_X3DH
}

static void check_ice_from_rtp(LinphoneCall *c1, LinphoneCall *c2, LinphoneStreamType stream_type) {
	MediaStream *ms;
	LinphoneCallStats *stats;
	switch (stream_type) {
		case LinphoneStreamTypeAudio:
			ms = linphone_call_get_stream(c1, LinphoneStreamTypeAudio);
			break;
		case LinphoneStreamTypeVideo:
			ms = linphone_call_get_stream(c1, LinphoneStreamTypeVideo);
			break;
		case LinphoneStreamTypeText:
			ms = linphone_call_get_stream(c1, LinphoneStreamTypeText);
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
		char ip[NI_MAXHOST] = {0};
		int port = 0;
		std::string expected_addr;
		AudioStream *astream;

		const LinphoneCallParams *cp1 = linphone_call_get_current_params(c1);
		const LinphoneCallParams *cp2 = linphone_call_get_current_params(c2);
		if (linphone_call_params_get_update_call_when_ice_completed(cp1) &&
		    linphone_call_params_get_update_call_when_ice_completed(cp2)) {
			memset(&remaddr, 0, remaddrlen);

			LinphonePrivate::SalCallOp *op = LinphonePrivate::Call::toCpp(c2)->getOp();
			const std::shared_ptr<SalMediaDescription> &result_desc = op->getFinalMediaDescription();
			const auto &result_stream = result_desc->getStreamAtIdx(0);
			if (result_stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				expected_addr = result_stream.getRtpAddress();
			}
			if (expected_addr.empty()) {
				expected_addr = result_desc->getConnectionAddress();
			}
			astream = (AudioStream *)linphone_call_get_stream(c1, LinphoneStreamTypeAudio);
			if ((expected_addr.find(':') == std::string::npos) &&
			    (astream->ms.sessions.rtp_session->rtp.gs.rem_addr.ss_family == AF_INET6)) {
				bctbx_sockaddr_ipv6_to_ipv4((struct sockaddr *)&astream->ms.sessions.rtp_session->rtp.gs.rem_addr,
				                            (struct sockaddr *)&remaddr, &remaddrlen);
			} else {
				memcpy(&remaddr, &astream->ms.sessions.rtp_session->rtp.gs.rem_addr,
				       astream->ms.sessions.rtp_session->rtp.gs.rem_addrlen);
			}
			bctbx_sockaddr_to_ip_address((struct sockaddr *)&remaddr, remaddrlen, ip, sizeof(ip), &port);

			BC_ASSERT_STRING_EQUAL(ip, expected_addr.c_str());
		}
	}
	linphone_call_stats_unref(stats);
}

bool_t check_ice(LinphoneCoreManager *caller, LinphoneCoreManager *callee, LinphoneIceState state) {
	LinphoneCall *c1, *c2;
	bool_t global_success = TRUE;
	bool_t audio_success = FALSE;
	bool_t video_success = FALSE;
	bool_t text_success = FALSE;
	bool_t audio_enabled, video_enabled, realtime_text_enabled;
	MSTimeSpec ts;

	c1 = linphone_core_get_call_by_remote_address2(caller->lc, callee->identity);
	c2 = linphone_core_get_call_by_remote_address2(callee->lc, caller->identity);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (!c1 || !c2) return FALSE;
	linphone_call_ref(c1);
	linphone_call_ref(c2);

	BC_ASSERT_EQUAL(linphone_call_params_video_enabled(linphone_call_get_current_params(c1)),
	                linphone_call_params_video_enabled(linphone_call_get_current_params(c2)), int, "%d");
	BC_ASSERT_EQUAL(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1)),
	                linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c2)), int, "%d");
	audio_enabled = linphone_call_params_audio_enabled(linphone_call_get_current_params(c1));
	video_enabled = linphone_call_params_video_enabled(linphone_call_get_current_params(c1));
	realtime_text_enabled = linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1));
	if (audio_enabled) {
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_audio_stats(c1);
				stats2 = linphone_call_get_audio_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1) == state &&
				    linphone_call_stats_get_ice_state(stats2) == state) {
					audio_success = TRUE;
					check_ice_from_rtp(c1, c2, LinphoneStreamTypeAudio);
					check_ice_from_rtp(c2, c1, LinphoneStreamTypeAudio);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts, 10000));
		if (stats1) linphone_call_stats_unref(stats1);
		if (stats2) linphone_call_stats_unref(stats2);
	}

	if (video_enabled) {
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_video_stats(c1);
				stats2 = linphone_call_get_video_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1) == state &&
				    linphone_call_stats_get_ice_state(stats2) == state) {
					video_success = TRUE;
					check_ice_from_rtp(c1, c2, LinphoneStreamTypeVideo);
					check_ice_from_rtp(c2, c1, LinphoneStreamTypeVideo);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts, 10000));
		if (stats1) linphone_call_stats_unref(stats1);
		if (stats2) linphone_call_stats_unref(stats2);
	}

	if (realtime_text_enabled) {
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_text_stats(c1);
				stats2 = linphone_call_get_text_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1) == state &&
				    linphone_call_stats_get_ice_state(stats2) == state) {
					text_success = TRUE;
					check_ice_from_rtp(c1, c2, LinphoneStreamTypeText);
					check_ice_from_rtp(c2, c1, LinphoneStreamTypeText);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts, 10000));
		if (stats1) linphone_call_stats_unref(stats1);
		if (stats2) linphone_call_stats_unref(stats2);
	}
	linphone_call_unref(c1);
	linphone_call_unref(c2);
	if (audio_enabled) global_success = global_success && audio_success;
	if (video_enabled) global_success = global_success && video_success;
	if (realtime_text_enabled) global_success = global_success && text_success;
	return global_success;
}

bool_t check_ice_sdp(LinphoneCall *call) {
	SalMediaDescription *desc = _linphone_call_get_local_desc(call);
	belle_sdp_session_description_t *sdp = desc->toSdp();
	const char *value = belle_sdp_session_description_get_attribute_value(sdp, "ice-ufrag");
	if (value) return TRUE;
	return FALSE;
}

bool_t is_srtp_secured(LinphoneCall *call, LinphoneStreamType ctype) {
	SalStreamType type = SalOther;
	switch (ctype) {
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
	const SalStreamDescription &stream = desc->findBestStream(type);
	if (stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) return FALSE;
	if (stream.hasSrtp()) {
		const auto &streamCryptos = stream.getCryptos();
		for (const auto &crypto : streamCryptos) {
			const auto &algo = crypto.algo;
			// return (!ms_crypto_suite_is_unencrypted(algo) && !ms_crypto_suite_is_unauthenticated(algo));
			return (!ms_crypto_suite_is_unencrypted(algo));
		}
	}
	return FALSE;
}

void check_media_stream(LinphoneCall *call, bool_t is_null) {

	LinphonePrivate::Call *c = LinphonePrivate::Call::toCpp(call);
	const auto &params = linphone_call_get_current_params(call);

	if (is_null || (params && !linphone_call_params_audio_enabled(params))) {
		BC_ASSERT_PTR_NULL(c->getMediaStream(LinphoneStreamTypeAudio));
	} else {
		BC_ASSERT_PTR_NOT_NULL(c->getMediaStream(LinphoneStreamTypeAudio));
	}

	if (is_null || (params && !linphone_call_params_video_enabled(params))) {
		BC_ASSERT_PTR_NULL(c->getMediaStream(LinphoneStreamTypeVideo));
	} else {
		BC_ASSERT_PTR_NOT_NULL(c->getMediaStream(LinphoneStreamTypeVideo));
	}

	if (is_null || (params && !linphone_call_params_realtime_text_enabled(params))) {
		BC_ASSERT_PTR_NULL(c->getMediaStream(LinphoneStreamTypeText));
	} else {
		BC_ASSERT_PTR_NOT_NULL(c->getMediaStream(LinphoneStreamTypeText));
	}
}

void check_result_desc_rtp_rtcp_ports(LinphoneCall *call, int rtp_port, int rtcp_port) {
	SalMediaDescription *desc = _linphone_call_get_result_desc(call);
	for (auto &stream : desc->streams) {
		BC_ASSERT_EQUAL(stream.rtp_port, rtp_port, int, "%d");
		BC_ASSERT_EQUAL(stream.rtcp_port, rtcp_port, int, "%d");
	}
}

void check_local_desc_stream(LinphoneCall *call) {
	const auto &desc = _linphone_call_get_local_desc(call);
	const auto &core = linphone_call_get_core(call);
	const auto &params = linphone_call_get_params(call);

	const auto &audioStream = desc->findBestStream(SalAudio);
	if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		const auto &streamDir = audioStream.getDirection();
		const auto &callParamsAudioDir = sal_dir_from_call_params_dir(linphone_call_params_get_audio_direction(params));
		if (params && linphone_call_params_audio_enabled(params)) {
			BC_ASSERT_EQUAL(streamDir, callParamsAudioDir, int, "%d");
		} else {
			BC_ASSERT_EQUAL(streamDir,
			                linphone_core_get_keep_stream_direction_for_rejected_stream(core) ? callParamsAudioDir
			                                                                                  : SalStreamInactive,
			                int, "%d");
			BC_ASSERT_EQUAL(audioStream.rtp_port, 0, int, "%d");
			BC_ASSERT_EQUAL(audioStream.rtcp_port, 0, int, "%d");
		}
	}

	const auto &videoStream = desc->findBestStream(SalVideo);
	if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		const auto &streamDir = videoStream.getDirection();
		const auto &callParamsVideoDir = sal_dir_from_call_params_dir(linphone_call_params_get_video_direction(params));
		if (params && linphone_call_params_video_enabled(params)) {
			BC_ASSERT_EQUAL(streamDir, callParamsVideoDir, int, "%d");
		} else {
			BC_ASSERT_EQUAL(streamDir,
			                linphone_core_get_keep_stream_direction_for_rejected_stream(core) ? callParamsVideoDir
			                                                                                  : SalStreamInactive,
			                int, "%d");
			BC_ASSERT_EQUAL(videoStream.rtp_port, 0, int, "%d");
			BC_ASSERT_EQUAL(videoStream.rtcp_port, 0, int, "%d");
		}
	}

	const auto &textStream = desc->findBestStream(SalText);
	if (textStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		const auto &streamDir = textStream.getDirection();
		if (params && linphone_call_params_realtime_text_enabled(params)) {
			BC_ASSERT_EQUAL(streamDir, SalStreamSendRecv, int, "%d");
		} else {
			BC_ASSERT_EQUAL(streamDir,
			                linphone_core_get_keep_stream_direction_for_rejected_stream(core) ? SalStreamSendRecv
			                                                                                  : SalStreamInactive,
			                int, "%d");
			BC_ASSERT_EQUAL(textStream.rtp_port, 0, int, "%d");
			BC_ASSERT_EQUAL(textStream.rtcp_port, 0, int, "%d");
		}
	}
}

void _check_call_media_ip_consistency(LinphoneCall *call) {
	// The offer media server address must be of the same type as the result
	const SalMediaDescription *call_ref_desc = (linphone_call_get_dir(call) == LinphoneCallIncoming)
	                                               ? _linphone_call_get_remote_desc(call)
	                                               : _linphone_call_get_local_desc(call);
	bool ref_is_ipv6 = ms_is_ipv6(call_ref_desc->addr.c_str());
	const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
	bool result_is_ipv6 = ms_is_ipv6(call_result_desc->addr.c_str());
	BC_ASSERT_FALSE(ref_is_ipv6 ^ result_is_ipv6);
}

void _linphone_call_check_max_nb_streams(const LinphoneCall *call,
                                         const size_t nb_audio_streams,
                                         const size_t nb_video_streams,
                                         const size_t nb_text_streams) {
	const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
	BC_ASSERT_PTR_NOT_NULL(call_result_desc);
	if (call_result_desc) {
		BC_ASSERT_LOWER(call_result_desc->getNbStreams(), nb_audio_streams + nb_video_streams + nb_text_streams, size_t,
		                "%zu");
		BC_ASSERT_LOWER(call_result_desc->nbStreamsOfType(SalAudio), nb_audio_streams, size_t, "%zu");
		BC_ASSERT_LOWER(call_result_desc->nbStreamsOfType(SalVideo), nb_video_streams, size_t, "%zu");
		BC_ASSERT_LOWER(call_result_desc->nbStreamsOfType(SalText), nb_text_streams, size_t, "%zu");
	}
}

void _linphone_call_check_nb_streams(const LinphoneCall *call,
                                     const size_t nb_audio_streams,
                                     const size_t nb_video_streams,
                                     const size_t nb_text_streams) {
	const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
	BC_ASSERT_PTR_NOT_NULL(call_result_desc);
	if (call_result_desc) {
		BC_ASSERT_EQUAL(call_result_desc->getNbStreams(), nb_audio_streams + nb_video_streams + nb_text_streams, size_t,
		                "%zu");
		BC_ASSERT_EQUAL(call_result_desc->nbStreamsOfType(SalAudio), nb_audio_streams, size_t, "%zu");
		BC_ASSERT_EQUAL(call_result_desc->nbStreamsOfType(SalVideo), nb_video_streams, size_t, "%zu");
		BC_ASSERT_EQUAL(call_result_desc->nbStreamsOfType(SalText), nb_text_streams, size_t, "%zu");
	}
}

size_t _linphone_call_get_nb_audio_steams(const LinphoneCall *call) {
	const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
	return call_result_desc->nbStreamsOfType(SalAudio);
}

size_t _linphone_call_get_nb_video_steams(const LinphoneCall *call) {
	const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
	return call_result_desc->nbStreamsOfType(SalVideo);
}

size_t _linphone_call_get_nb_text_steams(const LinphoneCall *call) {
	const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
	return call_result_desc->nbStreamsOfType(SalText);
}

LinphoneConferenceLayout _linphone_participant_device_get_layout(const LinphoneParticipantDevice *participant_device) {
	const auto &session =
	    static_pointer_cast<MediaSession>(LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSession());
	if (session) {
		return (LinphoneConferenceLayout)session->getRemoteParams()->getConferenceVideoLayout();
	}
	return LinphoneConferenceLayoutGrid;
}

bool_t _linphone_participant_device_get_audio_enabled(const LinphoneParticipantDevice *participant_device) {
	const auto &session =
	    static_pointer_cast<MediaSession>(LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSession());
	if (session) {
		return (session->getCurrentParams()->audioEnabled()) ? TRUE : FALSE;
	}
	return FALSE;
}

bool_t _linphone_participant_device_get_video_enabled(const LinphoneParticipantDevice *participant_device) {
	const auto &session =
	    static_pointer_cast<MediaSession>(LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSession());
	if (session) {
		return (session->getCurrentParams()->videoEnabled()) ? TRUE : FALSE;
	}
	return FALSE;
}

bool_t _linphone_participant_device_get_real_time_text_enabled(const LinphoneParticipantDevice *participant_device) {
	const auto &session =
	    static_pointer_cast<MediaSession>(LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSession());
	if (session) {
		return (session->getCurrentParams()->realtimeTextEnabled()) ? TRUE : FALSE;
	}
	return FALSE;
}

void _linphone_call_check_nb_active_streams(const LinphoneCall *call,
                                            const size_t nb_audio_streams,
                                            const size_t nb_video_streams,
                                            const size_t nb_text_streams) {
	BC_ASSERT_EQUAL(Call::toCpp(call)->getMediaStreamsNb(LinphoneStreamTypeAudio), nb_audio_streams, size_t, "%zu");
	BC_ASSERT_EQUAL(Call::toCpp(call)->getMediaStreamsNb(LinphoneStreamTypeVideo), nb_video_streams, size_t, "%zu");
	BC_ASSERT_EQUAL(Call::toCpp(call)->getMediaStreamsNb(LinphoneStreamTypeText), nb_text_streams, size_t, "%zu");
}

void check_video_conference(bctbx_list_t *lcs,
                            LinphoneCoreManager *lc1,
                            LinphoneCoreManager *lc2,
                            LinphoneConferenceLayout layout) {
	LinphoneCall *call1 = linphone_core_get_current_call(lc1->lc);
	LinphoneCall *call2 = linphone_core_get_current_call(lc2->lc);
	BC_ASSERT_PTR_NOT_NULL(call1);
	BC_ASSERT_PTR_NOT_NULL(call2);
	if (call1 && call2) {
		VideoStream *vstream1s = (VideoStream *)linphone_call_get_stream(call1, LinphoneStreamTypeVideo);
		BC_ASSERT_PTR_NOT_NULL(vstream1s);
		VideoStream *vstream2s = (VideoStream *)linphone_call_get_stream(call2, LinphoneStreamTypeVideo);
		BC_ASSERT_PTR_NOT_NULL(vstream2s);
		BC_ASSERT_TRUE(vstream1s && vstream1s->source && ms_filter_get_id(vstream1s->source) == MS_MIRE_ID);
		BC_ASSERT_TRUE(vstream2s && vstream2s->source && ms_filter_get_id(vstream2s->source) == MS_MIRE_ID);
		MSMireControl c1 = {{0, 5, 10, 15, 20, 25}};
		MSMireControl c2 = {{100, 120, 140, 160, 180, 200}};

		if (vstream1s && vstream1s->source && ms_filter_get_id(vstream1s->source) == MS_MIRE_ID) {
			ms_filter_call_method(vstream1s->source, MS_MIRE_SET_COLOR, &c1);
		}
		if (vstream2s && vstream2s->source && ms_filter_get_id(vstream2s->source) == MS_MIRE_ID) {
			ms_filter_call_method(vstream2s->source, MS_MIRE_SET_COLOR, &c2);
		}

		wait_for_list(lcs, NULL, 5, liblinphone_tester_sip_timeout);

		LinphoneConference *conference1 = linphone_call_get_conference(call1);
		BC_ASSERT_PTR_NOT_NULL(conference1);
		if (conference1) {
			size_t nb = static_cast<size_t>(linphone_conference_get_participant_count(conference1) + 2);
			BC_ASSERT_EQUAL(Call::toCpp(call1)->getMediaStreamsNb(LinphoneStreamTypeVideo), nb, size_t, "%zu");
		}

		LinphoneConference *conference2 = linphone_call_get_conference(call1);
		BC_ASSERT_PTR_NOT_NULL(conference2);
		if (conference2) {
			size_t nb = static_cast<size_t>(linphone_conference_get_participant_count(conference2) + 2);
			BC_ASSERT_EQUAL(Call::toCpp(call2)->getMediaStreamsNb(LinphoneStreamTypeVideo), nb, size_t, "%zu");
		}

		linphone_call_check_rtp_sessions(call1);
		linphone_call_check_rtp_sessions(call2);
		BC_ASSERT_TRUE(linphone_call_compare_video_color(call1, c2, MediaStreamRecvOnly, vstream2s->label));
		BC_ASSERT_TRUE(linphone_call_compare_video_color(call2, c1, MediaStreamRecvOnly, vstream1s->label));
		if (layout != LinphoneConferenceLayoutGrid) {
			BC_ASSERT_TRUE(linphone_call_compare_video_color(call2, c1, MediaStreamSendRecv, ""));
		}
	}
}

void check_video_conference_with_local_participant(bctbx_list_t *participants,
                                                   LinphoneConferenceLayout layout,
                                                   bool_t local_participant) {
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCall *call = linphone_core_get_current_call(m->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			const LinphoneCallParams *call_params = linphone_call_get_current_params(call);
			const bool_t video_enabled = linphone_call_params_video_enabled(call_params);

			size_t nb = ((bctbx_list_size(participants) + (local_participant ? 2 : 1)));
			if (!video_enabled) {
				if (layout == LinphoneConferenceLayoutActiveSpeaker) {
					// Only thumbnail corresponding to the participant is going to be inactivated
					nb--;
				} else if (layout == LinphoneConferenceLayoutGrid) {
					// main and thumbnail corresponding to the participant are going to be inactivated
					nb -= 2;
				}
			}
			BC_ASSERT_EQUAL(Call::toCpp(call)->getMediaStreamsNb(LinphoneStreamTypeVideo), nb, size_t, "%zu");
			if (video_enabled) {
				linphone_call_check_rtp_sessions(call);
			}
		}
	}
}

LinphoneCoreManager *_linphone_conference_video_change(bctbx_list_t *lcs,
                                                       LinphoneCoreManager *mgr1,
                                                       LinphoneCoreManager *mgr2,
                                                       LinphoneCoreManager *mgr3) {
	MSMireControl c1 = {{0, 5, 10, 15, 20, 25}};
	MSMireControl c3 = {{100, 120, 140, 160, 180, 200}};

	stats initial_mgr1_stat = mgr1->stat;
	stats initial_mgr2_stat = mgr2->stat;
	stats initial_mgr3_stat = mgr3->stat;
	for (LinphoneCoreManager *mgr : {mgr1, mgr2, mgr3}) {
		LinphoneCall *call = linphone_core_get_current_call(mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (!call) return NULL;
		VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
		if (mgr != mgr2) { // mgr2 is audio only
			bool filterIsMire = (vstream && vstream->source && ms_filter_get_id(vstream->source) == MS_MIRE_ID);
			BC_ASSERT_TRUE(filterIsMire);
			if (filterIsMire) {
				if (mgr == mgr1) ms_filter_call_method(vstream->source, MS_MIRE_SET_COLOR, &c1);
				else ms_filter_call_method(vstream->source, MS_MIRE_SET_COLOR, &c3);
			}
		} else {
			BC_ASSERT_PTR_NULL(vstream);
		}
	}

	LinphoneCall *call1 = linphone_core_get_current_call(mgr1->lc);
	LinphoneConference *confMgr1 = linphone_call_get_conference(call1);
	LinphoneConference *confMgr3 = linphone_call_get_conference(linphone_core_get_current_call(mgr3->lc));

	// mgr3 speaks and mgr1's video change
	linphone_core_enable_mic(mgr1->lc, FALSE);
	linphone_core_enable_mic(mgr2->lc, FALSE);
	linphone_core_enable_mic(mgr3->lc, TRUE);
	lInfo() << __func__ << ": " << linphone_core_get_identity(mgr3->lc) << " speaks";
	BC_ASSERT_TRUE(wait_for_list(lcs, &mgr1->stat.number_of_active_speaker_participant_device_changed,
	                             initial_mgr1_stat.number_of_active_speaker_participant_device_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &mgr2->stat.number_of_active_speaker_participant_device_changed,
	                             initial_mgr2_stat.number_of_active_speaker_participant_device_changed + 1,
	                             liblinphone_tester_sip_timeout));
	linphone_call_check_rtp_sessions(call1);
	BC_ASSERT_TRUE(linphone_call_compare_video_color(call1, c3, MediaStreamSendRecv, ""));
	BC_ASSERT_FALSE(linphone_call_compare_video_color(call1, c1, MediaStreamSendRecv, ""));

	// mgr1 should see mgr3 as active speaker
	LinphoneParticipantDevice *device = linphone_conference_get_active_speaker_participant_device(confMgr1);
	if (BC_ASSERT_PTR_NOT_NULL(device)) {
		const LinphoneAddress *addrMgr1 = linphone_participant_device_get_address(device);

		LinphoneParticipant *participant = linphone_conference_get_me(confMgr3);
		bctbx_list_t *devices = linphone_participant_get_devices(participant);
		const LinphoneAddress *addrMgr3 =
		    linphone_participant_device_get_address((LinphoneParticipantDevice *)devices->data);
		BC_ASSERT_TRUE(linphone_address_equal(addrMgr1, addrMgr3));

		bctbx_list_free_with_data(devices, (bctbx_list_free_func)linphone_participant_device_unref);
	}

	// mgr2 which is in audio only should also see mgr3 in active speaker
	LinphoneConference *confMgr2 = linphone_call_get_conference(linphone_core_get_current_call(mgr2->lc));
	device = linphone_conference_get_active_speaker_participant_device(confMgr2);
	if (BC_ASSERT_PTR_NOT_NULL(device)) {
		const LinphoneAddress *addrMgr2 = linphone_participant_device_get_address(device);

		LinphoneParticipant *participant = linphone_conference_get_me(confMgr3);
		bctbx_list_t *devices = linphone_participant_get_devices(participant);
		const LinphoneAddress *addrMgr3 =
		    linphone_participant_device_get_address((LinphoneParticipantDevice *)devices->data);

		BC_ASSERT_TRUE(linphone_address_equal(addrMgr2, addrMgr3));

		bctbx_list_free_with_data(devices, (bctbx_list_free_func)linphone_participant_device_unref);
	}

	// mgr3 speaks until mgr2's video change
	linphone_core_enable_mic(mgr3->lc, FALSE);
	linphone_core_enable_mic(mgr2->lc, TRUE);
	linphone_core_enable_mic(mgr1->lc, FALSE);
	lInfo() << __func__ << ": " << linphone_core_get_identity(mgr2->lc) << " speaks";
	BC_ASSERT_TRUE(wait_for_list(lcs, &mgr1->stat.number_of_active_speaker_participant_device_changed,
	                             initial_mgr1_stat.number_of_active_speaker_participant_device_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &mgr3->stat.number_of_active_speaker_participant_device_changed,
	                             initial_mgr3_stat.number_of_active_speaker_participant_device_changed + 1,
	                             liblinphone_tester_sip_timeout));
	/* mg2 is audio-only, so this shall not affect video on other participants */

	// mgr1 still receives the video of the last active speaker that has video
	linphone_call_check_rtp_sessions(call1);
	BC_ASSERT_TRUE(linphone_call_compare_video_color(call1, c3, MediaStreamSendRecv, ""));
	BC_ASSERT_FALSE(linphone_call_compare_video_color(call1, c1, MediaStreamSendRecv, ""));

	// mgr1 should see mgr2 as active speaker even though it has no video as it's speaking
	device = linphone_conference_get_active_speaker_participant_device(confMgr1);
	if (BC_ASSERT_PTR_NOT_NULL(device)) {
		const LinphoneAddress *addrMgr1 = linphone_participant_device_get_address(device);

		LinphoneParticipant *participant = linphone_conference_get_me(confMgr2);
		bctbx_list_t *devices = linphone_participant_get_devices(participant);
		const LinphoneAddress *addrMgr2 =
		    linphone_participant_device_get_address((LinphoneParticipantDevice *)devices->data);

		BC_ASSERT_TRUE(linphone_address_equal(addrMgr1, addrMgr2));

		bctbx_list_free_with_data(devices, (bctbx_list_free_func)linphone_participant_device_unref);
	}

	// mgr1 speaks and mgr1's video not change
	lInfo() << __func__ << ": " << linphone_core_get_identity(mgr1->lc) << " speaks";
	linphone_core_enable_mic(mgr1->lc, TRUE);
	linphone_core_enable_mic(mgr2->lc, FALSE);
	linphone_core_enable_mic(mgr3->lc, FALSE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &mgr2->stat.number_of_active_speaker_participant_device_changed,
	                             initial_mgr2_stat.number_of_active_speaker_participant_device_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &mgr3->stat.number_of_active_speaker_participant_device_changed,
	                             initial_mgr3_stat.number_of_active_speaker_participant_device_changed + 2,
	                             liblinphone_tester_sip_timeout));
	linphone_call_check_rtp_sessions(call1);
	BC_ASSERT_TRUE(linphone_call_compare_video_color(call1, c3, MediaStreamSendRecv, ""));
	BC_ASSERT_FALSE(linphone_call_compare_video_color(call1, c1, MediaStreamSendRecv, ""));

	// mgr3 should see mgr1 as active speaker
	device = linphone_conference_get_active_speaker_participant_device(confMgr3);
	if (BC_ASSERT_PTR_NOT_NULL(device)) {
		const LinphoneAddress *addrMgr3 = linphone_participant_device_get_address(device);

		LinphoneParticipant *participant = linphone_conference_get_me(confMgr1);
		bctbx_list_t *devices = linphone_participant_get_devices(participant);
		const LinphoneAddress *addrMgr1 =
		    linphone_participant_device_get_address((LinphoneParticipantDevice *)devices->data);

		BC_ASSERT_TRUE(linphone_address_equal(addrMgr3, addrMgr1));

		bctbx_list_free_with_data(devices, (bctbx_list_free_func)linphone_participant_device_unref);
	}

	return mgr1;
}

const char *_linphone_call_get_subject(LinphoneCall *call) {
	SalCallOp *op = Call::toCpp(call)->getOp();
	return L_STRING_TO_C(op->getSubject());
}

static std::string get_ice_default_candidate(LinphoneCoreManager *m) {
	std::string rtpAddress;
	rtpAddress =
	    _linphone_call_get_local_desc(linphone_core_get_current_call(m->lc))->getStreamAtIdx(0).getRtpAddress();
	if (!rtpAddress.empty()) {
		return rtpAddress;
	} else {
		std::string cAddress =
		    _linphone_call_get_local_desc(linphone_core_get_current_call(m->lc))->getConnectionAddress();
		return cAddress;
	}
}

static bool address_in_list(const std::string &ip, const bctbx_list_t *addresses) {
	if (ip.empty()) return FALSE;
	for (; addresses != NULL; addresses = addresses->next) {
		if (ip.compare((const char *)addresses->data) == 0) return true;
	}
	return false;
}

static void check_expected_candidate_type(LinphoneCoreManager *m,
                                          TesterIceCandidateType expected_type,
                                          const bctbx_list_t *local_addresses) {
	std::string ip = get_ice_default_candidate(m);
	const struct addrinfo *ai = linphone_core_get_stun_server_addrinfo(m->lc);
	std::string relayIP;

	// bctbx_message("default-candidate=%s", ip.c_str());
	if (ai) {
		char rawip[64] = {0};
		struct sockaddr_storage ss;
		socklen_t slen = (socklen_t)ai->ai_addrlen;
		memcpy(&ss, ai->ai_addr, ai->ai_addrlen);
		bctbx_sockaddr_remove_v4_mapping((struct sockaddr *)&ss, (struct sockaddr *)&ss, &slen);
		bctbx_sockaddr_to_ip_address((struct sockaddr *)&ss, slen, rawip, sizeof(rawip), NULL);
		relayIP = rawip;
	}
	switch (expected_type) {
		case TesterIceCandidateHost:
			BC_ASSERT_TRUE(address_in_list(ip, local_addresses));
			break;
		case TesterIceCandidateSflrx:
			BC_ASSERT_FALSE(address_in_list(ip, local_addresses));
			BC_ASSERT_STRING_NOT_EQUAL(ip.c_str(), relayIP.c_str());
			BC_ASSERT_TRUE(ip != relayIP);
			break;
		case TesterIceCandidateRelay:
			BC_ASSERT_STRING_EQUAL(ip.c_str(), relayIP.c_str());
			break;
	}
}

void liblinphone_tester_check_ice_default_candidates(LinphoneCoreManager *marie,
                                                     TesterIceCandidateType marie_expected_type,
                                                     LinphoneCoreManager *pauline,
                                                     TesterIceCandidateType pauline_expected_type) {
	bctbx_list_t *local_addresses = linphone_fetch_local_addresses();

	check_expected_candidate_type(marie, marie_expected_type, local_addresses);
	check_expected_candidate_type(pauline, pauline_expected_type, local_addresses);
	bctbx_list_free_with_data(local_addresses, bctbx_free);
}

int liblinphone_tester_send_data(const void *buffer, size_t length, const char *dest_ip, int dest_port, int sock_type) {
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	int err = 0;
	std::ostringstream service;
	bctbx_socket_t sock;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = sock_type;
	service << dest_port;

	err = bctbx_getaddrinfo(dest_ip, service.str().c_str(), &hints, &res);
	if (err != 0) {
		bctbx_error("liblinphone_tester_send_data: bctbx_getaddrinfo() failed: %s", gai_strerror(err));
		return -1;
	}
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock == (bctbx_socket_t)-1) {
		bctbx_error("liblinphone_tester_send_data: socket creation failed.");
		return -1;
	}
	err = bctbx_connect(sock, (struct sockaddr *)res->ai_addr, (socklen_t)res->ai_addrlen);
	if (err != 0) {
		bctbx_error("liblinphone_tester_send_data: connection failed: %s", getSocketError());
		goto end;
	}
	err = (int)bctbx_send(sock, buffer, length, 0);
	if (err == -1) {
		bctbx_error("liblinphone_tester_send_data: send() failed: %s", getSocketError());
		goto end;
	}
end:
	bctbx_freeaddrinfo(res);
	bctbx_socket_close(sock);
	return err;
}

bool_t linphone_conference_type_is_full_state(const char *text) {
	std::string data(text);
	return ((data.find("state=\"full\"") != std::string::npos) &&
	        (data.find("state=\"partial\"") == std::string::npos) &&
	        (data.find("state=\"deleted\"") == std::string::npos))
	           ? TRUE
	           : FALSE;
}

void linphone_conference_info_check_participant_info(const std::shared_ptr<ParticipantInfo> &info,
                                                     int sequence_number) {
	const auto &sequence = info->getSequenceNumber();
	BC_ASSERT_GREATER(sequence, 0, int, "%0d");
	BC_ASSERT_EQUAL(sequence, sequence_number, int, "%d");
}

void linphone_conference_info_check_participant(const LinphoneConferenceInfo *conference_info,
                                                LinphoneAddress *address,
                                                int sequence_number) {
	const auto &participantInfo = LinphonePrivate::ConferenceInfo::toCpp(conference_info)
	                                  ->findParticipant(Address::toCpp(address)->getSharedFromThis());
	linphone_conference_info_check_participant_info(participantInfo, sequence_number);
}

void linphone_conference_info_check_organizer(const LinphoneConferenceInfo *conference_info, int sequence_number) {
	const auto &organizer = LinphonePrivate::ConferenceInfo::toCpp(conference_info)->getOrganizer();
	linphone_conference_info_check_participant_info(organizer, sequence_number);
}

bool_t check_custom_m_line(LinphoneCall *call, const char *m_line) {
	SalStreamType type = SalOther;
	const std::string m_line_str(m_line);
	SalMediaDescription *result_desc = _linphone_call_get_result_desc(call);
	const SalStreamDescription &result_stream = result_desc->findBestStream(type);
	bool_t result_stream_found = (result_stream == Utils::getEmptyConstRefObject<SalStreamDescription>());
	BC_ASSERT_TRUE(result_stream_found);
	std::string result_stream_type_string;
	if (result_stream_found) {
		result_stream_type_string = result_stream.getTypeAsString();
	}
	bool_t result_stream_type_ok =
	    (!result_stream_type_string.empty() || (result_stream_type_string.compare(m_line_str) == 0));
	BC_ASSERT_TRUE(result_stream_type_ok);

	SalMediaDescription *local_desc = _linphone_call_get_local_desc(call);
	const SalStreamDescription &local_stream = local_desc->findBestStream(type);
	bool_t local_stream_found = (local_stream == Utils::getEmptyConstRefObject<SalStreamDescription>());
	BC_ASSERT_TRUE(local_stream_found);
	std::string local_stream_type_string;
	if (local_stream_found) {
		local_stream_type_string = local_stream.getTypeAsString();
	}
	bool_t local_stream_type_ok =
	    (!local_stream_type_string.empty() || (local_stream_type_string.compare(m_line_str) == 0));
	BC_ASSERT_TRUE(local_stream_type_ok);

	SalMediaDescription *remote_desc = _linphone_call_get_remote_desc(call);
	const SalStreamDescription &remote_stream = remote_desc->findBestStream(type);
	bool_t remote_stream_found = (remote_stream == Utils::getEmptyConstRefObject<SalStreamDescription>());
	std::string remote_stream_type_string;
	if (remote_stream_found) {
		remote_stream_type_string = remote_stream.getTypeAsString();
	}
	bool_t remote_stream_type_ok = (!remote_stream_found || !remote_stream_type_string.empty() ||
	                                (remote_stream_type_string.compare(m_line_str) == 0));
	BC_ASSERT_TRUE(remote_stream_type_ok);

	bool_t ret = result_stream_found && result_stream_type_ok && local_stream_found && local_stream_type_ok &&
	             remote_stream_type_ok;
	return ret;
}

bool check_conference_ssrc(LinphoneConference *local_conference, LinphoneConference *remote_conference) {
	bool ret = true;
	if (!local_conference) ret = false;
	if (!remote_conference) ret = false;
	if (local_conference && remote_conference) {
		bctbx_list_t *local_conference_participants = linphone_conference_get_participant_list(local_conference);
		for (bctbx_list_t *itp = local_conference_participants; itp; itp = bctbx_list_next(itp)) {
			LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
			const LinphoneAddress *p_address = linphone_participant_get_address(p);
			bctbx_list_t *local_devices = NULL;
			LinphoneParticipant *remote_participant =
			    (linphone_conference_is_me(remote_conference, p_address))
			        ? linphone_conference_get_me(remote_conference)
			        : linphone_conference_find_participant(remote_conference, p_address);
			if (!remote_participant) ret = false;
			if (remote_participant) {
				local_devices = linphone_participant_get_devices(p);
				for (bctbx_list_t *itd = local_devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					LinphoneParticipantDevice *remote_device = linphone_participant_find_device(
					    remote_participant, linphone_participant_device_get_address(d));
					if (!remote_device) ret = false;
					if (remote_device) {
						for (const auto type :
						     {LinphoneStreamTypeAudio, LinphoneStreamTypeVideo, LinphoneStreamTypeText}) {
							for (const auto device : {d, remote_device}) {
								LinphoneMediaDirection media_direction =
								    linphone_participant_device_get_stream_capability(device, type);
								// The thumbnail video stream can only be sendonly (or recvonly from the server
								// standpoint) or inactive. Henceherefore a client that can only receive video, will
								// have to set it to inactive
								uint32_t stream_ssrc = linphone_participant_device_get_ssrc(device, type);
								if (media_direction == LinphoneMediaDirectionInactive) {
									if (stream_ssrc != 0) {
										ret = false;
									}
								} else {
									if (stream_ssrc == 0) {
										ret = false;
									}
								}

								if (type == LinphoneStreamTypeVideo) {
									bool_t stream_available =
									    linphone_participant_device_get_stream_availability(device, type);
									if (!stream_available) {
										continue;
									}
									auto cppDevice = ParticipantDevice::toCpp(device)->getSharedFromThis();
									bool thumbnail_available = cppDevice->getThumbnailStreamAvailability();
									uint32_t thumbnail_ssrc = cppDevice->getThumbnailStreamSsrc();
									if (thumbnail_available) {
										if (thumbnail_ssrc == 0) {
											ret = false;
										}
										if (thumbnail_ssrc == stream_ssrc) {
											ret = false;
										}
									}
								}
							}
						}
					}
				}
			}
			if (local_devices) {
				bctbx_list_free_with_data(local_devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}
		bctbx_list_free_with_data(local_conference_participants, (void (*)(void *))linphone_participant_unref);
	}
	return ret;
}

static bool_t screen_sharing_enabled_in_stream(const std::shared_ptr<SalMediaDescription> md) {
	if (md) {
		SalStreamDescription stream = md->findStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute);
		if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
			const SalStreamDir dir = stream.getDirection();
			return (dir == SalStreamSendRecv) || (dir == SalStreamSendOnly);
		}
	}
	return false;
}

bool_t screen_sharing_enabled_in_local_description(LinphoneCall *call) {
	SalCallOp *op = Call::toCpp(call)->getOp();
	if (op) {
		const std::shared_ptr<SalMediaDescription> &local_desc = op->getLocalMediaDescription();
		return screen_sharing_enabled_in_stream(local_desc);
	}
	return FALSE;
}

bool_t screen_sharing_enabled_in_remote_description(LinphoneCall *call) {
	SalCallOp *op = Call::toCpp(call)->getOp();
	if (op) {
		const std::shared_ptr<SalMediaDescription> &remote_desc = op->getRemoteMediaDescription();
		return screen_sharing_enabled_in_stream(remote_desc);
	}
	return FALSE;
}

bool_t screen_sharing_enabled_in_negotiated_description(LinphoneCall *call) {
	SalCallOp *op = Call::toCpp(call)->getOp();
	if (op) {
		LinphoneCore *core = linphone_call_get_core(call);
		bool_t is_conference_server = linphone_core_conference_server_enabled(core);
		const std::shared_ptr<SalMediaDescription> &desc =
		    is_conference_server ? op->getRemoteMediaDescription() : op->getLocalMediaDescription();
		int streamIdx = desc->findIdxStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute);
		if (streamIdx != -1) {
			const std::shared_ptr<SalMediaDescription> &negotiated_desc = op->getFinalMediaDescription();
			SalStreamDescription stream = negotiated_desc->getStreamAtIdx(streamIdx);
			if (stream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				const SalStreamDir dir = stream.getDirection();
				return (dir == SalStreamSendRecv) || (dir == SalStreamSendOnly);
			}
		}
	}
	return FALSE;
}

void check_chat_message_properties(LinphoneChatMessage *msg) {
	BC_ASSERT_PTR_NOT_NULL(msg);
	if (!msg) return;
	std::shared_ptr<ChatMessage> cppMsg = L_GET_CPP_PTR_FROM_C_OBJECT(msg);
	auto content = cppMsg->getInternalContent();
	if (content.getContentType() == ContentType::Encrypted) {
		auto contentList = cppMsg->getProperty("content-list");
		BC_ASSERT_TRUE(contentList.isValid());
	}
}

bool_t check_screen_sharing_call_sdp(LinphoneCall *call, bool_t screen_sharing_enabled) {
	bool ret = (call != nullptr);
	BC_ASSERT_PTR_NOT_NULL(call);
	if (call) {
		LinphonePrivate::SalCallOp *op = LinphonePrivate::Call::toCpp(call)->getOp();
		bool_t is_in_conference = linphone_call_is_in_conference(call);
		const std::shared_ptr<SalMediaDescription> &content_desc =
		    is_in_conference ? op->getRemoteMediaDescription() : op->getLocalMediaDescription();
		int screenSharingStreamIdx =
		    content_desc->findIdxStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute);
		if (screenSharingStreamIdx == -1) {
			ret &= (screen_sharing_enabled == FALSE);
		} else {
			const std::shared_ptr<SalMediaDescription> &label_desc =
			    is_in_conference ? op->getLocalMediaDescription() : op->getRemoteMediaDescription();
			SalStreamDescription screen_sharing_stream = label_desc->getStreamAtIdx(screenSharingStreamIdx);
			const SalStreamDir screen_sharing_direction = screen_sharing_stream.getDirection();
			int thumbnailStreamIdx =
			    content_desc->findIdxStreamWithContent(MediaSessionPrivate::ThumbnailVideoContentAttribute);
			if (thumbnailStreamIdx == -1) {
				// If no thumbnail stream is found, then verify that the screen sharing label is not empty
				ret &= screen_sharing_enabled ? (screen_sharing_direction != SalStreamInactive)
				                              : (screen_sharing_direction == SalStreamInactive);
			} else {
				SalStreamDescription thumbnail_stream = label_desc->getStreamAtIdx(thumbnailStreamIdx);
				const std::string thumbnail_label = thumbnail_stream.getLabel();
				const std::string screen_sharing_label = screen_sharing_stream.getLabel();
				// Verify that the label of the screen sharing and thumbnail streams are different
				ret &= screen_sharing_enabled ? (thumbnail_label.compare(screen_sharing_label) != 0)
				                              : (thumbnail_label.compare(screen_sharing_label) == 0);
			}
		}
	}
	return ret ? TRUE : FALSE;
}

bool_t check_screen_sharing_sdp(LinphoneCoreManager *mgr1, LinphoneCoreManager *mgr2, bool_t screen_sharing_enabled) {
	bool_t ret = TRUE;
	LinphoneCall *mgr1_call = linphone_core_get_call_by_remote_address2(mgr1->lc, mgr2->identity);
	ret &= check_screen_sharing_call_sdp(mgr1_call, screen_sharing_enabled);
	LinphoneCall *mgr2_call = linphone_core_get_call_by_remote_address2(mgr2->lc, mgr1->identity);
	ret &= check_screen_sharing_call_sdp(mgr2_call, screen_sharing_enabled);
	return ret;
}

const char *_linphone_call_get_local_rtp_address(const LinphoneCall *call) {
	return _linphone_call_get_local_desc(call)->getConnectionAddress().c_str();
}

const char *_linphone_call_get_remote_rtp_address(const LinphoneCall *call) {
	return _linphone_call_get_remote_desc(call)->getConnectionAddress().c_str();
}

void check_session_error(LinphoneConferenceScheduler *scheduler, LinphoneReason reason) {
	auto cppScheduler = ConferenceScheduler::toCpp(scheduler);
	auto sipConferenceScheduler = dynamic_cast<SIPConferenceScheduler *>(cppScheduler);
	if (BC_ASSERT_PTR_NOT_NULL(sipConferenceScheduler)) {
		auto session = sipConferenceScheduler->getSession();
		if (BC_ASSERT_PTR_NOT_NULL(session)) {
			auto errorInfo = session->getErrorInfo();
			if (BC_ASSERT_PTR_NOT_NULL(errorInfo)) {
				BC_ASSERT_EQUAL(linphone_error_info_get_reason(errorInfo), reason, int, "%i");
			}
		}
	}
}
