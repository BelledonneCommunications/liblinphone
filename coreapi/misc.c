
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

#include <bctoolbox/defs.h>

#include "mediastreamer2/mediastream.h"

#include "core/core-p.h"
#include "linphone/lpconfig.h"
#include "linphone/wrapper_utils.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SIGHANDLER_T
#include <signal.h>
#endif /*HAVE_SIGHANDLER_T*/

#if !defined(_WIN32_WCE)
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#if _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#endif /*_WIN32_WCE*/

#undef snprintf
#include <mediastreamer2/stun.h>

#include <math.h>
#if _MSC_VER
#define snprintf _snprintf
#define popen _popen
#define pclose _pclose
#endif

// TODO: From coreapi. Remove me later.
#include "private.h"

#include "c-wrapper/c-wrapper.h"
#include "call/call-log.h"
#include "call/call.h"
#include "conference/session/media-session-p.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-nat-policy.h"
#include "nat/stun-client.h"
#include "utils/payload-type-handler.h"

char *linphone_timestamp_to_rfc3339_string(time_t timestamp) {
	struct tm *ret;
#ifndef _WIN32
	struct tm gmt;
	ret = gmtime_r(&timestamp, &gmt);
#else
	ret = gmtime(&timestamp);
#endif
	int n = snprintf(0, 0, "%4d-%02d-%02dT%02d:%02d:%02dZ", ret->tm_year + 1900, ret->tm_mon + 1, ret->tm_mday,
	                 ret->tm_hour, ret->tm_min, ret->tm_sec);
	char *timestamp_str = (char *)ms_malloc(size_t(n + 1));
	snprintf(timestamp_str, (size_t)n + 1, "%4d-%02d-%02dT%02d:%02d:%02dZ", ret->tm_year + 1900, ret->tm_mon + 1,
	         ret->tm_mday, ret->tm_hour, ret->tm_min, ret->tm_sec);
	return timestamp_str;
}

void linphone_core_update_allocated_audio_bandwidth(LinphoneCore *lc) {
	const bctbx_list_t *elem;
	int maxbw = LinphonePrivate::PayloadTypeHandler::getMinBandwidth(linphone_core_get_download_bandwidth(lc),
	                                                                 linphone_core_get_upload_bandwidth(lc));
	int max_codec_bitrate = 0;

	for (elem = linphone_core_get_audio_codecs(lc); elem != NULL; elem = elem->next) {
		PayloadType *pt = (PayloadType *)elem->data;
		if (payload_type_enabled(pt)) {
			int pt_bitrate = LinphonePrivate::PayloadTypeHandler::getAudioPayloadTypeBandwidth(pt, maxbw);
			if (max_codec_bitrate == 0) {
				max_codec_bitrate = pt_bitrate;
			} else if (max_codec_bitrate < pt_bitrate) {
				max_codec_bitrate = pt_bitrate;
			}
		}
	}
	if (max_codec_bitrate) {
		lc->audio_bw = max_codec_bitrate;
	}
}

bool_t linphone_core_is_payload_type_usable_for_bandwidth(BCTBX_UNUSED(const LinphoneCore *lc),
                                                          const PayloadType *pt,
                                                          int bandwidth_limit) {
	double codec_band;
	const int video_enablement_limit = 99;
	bool_t ret = FALSE;

	switch (pt->type) {
		case PAYLOAD_AUDIO_CONTINUOUS:
		case PAYLOAD_AUDIO_PACKETIZED:
			codec_band = LinphonePrivate::PayloadTypeHandler::getAudioPayloadTypeBandwidth(pt, bandwidth_limit);
			ret = LinphonePrivate::PayloadTypeHandler::bandwidthIsGreater(bandwidth_limit, (int)codec_band);
			/*ms_message("Payload %s: codec_bandwidth=%g,
			 * bandwidth_limit=%i",pt->mime_type,codec_band,bandwidth_limit);*/
			break;
		case PAYLOAD_VIDEO:
			if (bandwidth_limit <= 0 ||
			    bandwidth_limit >= video_enablement_limit) { /* infinite or greater than video_enablement_limit*/
				ret = TRUE;
			} else ret = FALSE;
			break;
		case PAYLOAD_TEXT:
			ret = TRUE;
			break;
	}
	return ret;
}

bool_t lp_spawn_command_line_sync(const char *command, char **result, int *command_ret) {
#if !defined(_WIN32_WCE) && !defined(LINPHONE_WINDOWS_UNIVERSAL)
#ifndef ENABLE_MICROSOFT_STORE_APP
	FILE *f = popen(command, "r");
	if (f != NULL) {
		int err;
		*result = reinterpret_cast<char *>(ms_malloc(4096));
		err = (int)fread(*result, 1, 4096 - 1, f);
		if (err < 0) {
			ms_warning("Error reading command output:%s", strerror(errno));
			ms_free(result);
			return FALSE;
		}
		(*result)[err] = 0;
		err = pclose(f);
		if (command_ret != NULL) *command_ret = err;
		return TRUE;
	}
#endif // ENABLE_MICROSOFT_STORE_APP
#endif /*_WIN32_WCE*/
	return FALSE;
}

int linphone_parse_host_port(const char *input, char *host, size_t hostlen, int *port) {
	char tmphost[NI_MAXHOST] = {0};

	if ((sscanf(input, "[%64[^]]]:%d", tmphost, port) == 2) || (sscanf(input, "[%64[^]]]", tmphost) == 1)) {

	} else {
		const char *p1 = strchr(input, ':');
		const char *p2 = strrchr(input, ':');
		if (p1 && p2 && (p1 != p2)) { /* an ipv6 address without port*/
			strncpy(tmphost, input, sizeof(tmphost) - 1);
		} else if (sscanf(input, "%[^:]:%d", tmphost, port) != 2) {
			/*no port*/
			strncpy(tmphost, input, sizeof(tmphost) - 1);
		}
	}
	strncpy(host, tmphost, hostlen);
	return 0;
}

int parse_hostname_to_addr(const char *server, struct sockaddr_storage *ss, socklen_t *socklen, int default_port) {
	struct addrinfo hints, *res = NULL;
	char port[6];
	char host[NI_MAXHOST];
	int port_int = default_port;
	int ret;

	linphone_parse_host_port(server, host, sizeof(host), &port_int);

	snprintf(port, sizeof(port), "%d", port_int);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	ret = getaddrinfo(host, port, &hints, &res);
	if (ret != 0) {
		ms_error("getaddrinfo() failed for %s:%s : %s", host, port, gai_strerror(ret));
		return -1;
	}
	if (!res) return -1;
	memcpy(ss, res->ai_addr, (size_t)res->ai_addrlen);
	*socklen = (socklen_t)res->ai_addrlen;
	freeaddrinfo(res);
	return 0;
}

/* this functions runs a simple stun test and return the number of milliseconds to complete the tests, or -1 if the test
 * were failed.*/
int linphone_run_stun_tests(LinphoneCore *lc,
                            int audioPort,
                            int videoPort,
                            int textPort,
                            char *audioCandidateAddr,
                            int *audioCandidatePort,
                            char *videoCandidateAddr,
                            int *videoCandidatePort,
                            char *textCandidateAddr,
                            int *textCandidatePort) {
	LinphonePrivate::StunClient *client = new LinphonePrivate::StunClient(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
	int ret = client->run(audioPort, videoPort, textPort);
	strncpy(audioCandidateAddr, client->getAudioCandidate().address.c_str(), LINPHONE_IPADDR_SIZE);
	*audioCandidatePort = client->getAudioCandidate().port;
	strncpy(videoCandidateAddr, client->getVideoCandidate().address.c_str(), LINPHONE_IPADDR_SIZE);
	*videoCandidatePort = client->getVideoCandidate().port;
	strncpy(textCandidateAddr, client->getTextCandidate().address.c_str(), LINPHONE_IPADDR_SIZE);
	*textCandidatePort = client->getTextCandidate().port;
	delete client;
	return ret;
}

int linphone_core_get_edge_bw(LinphoneCore *lc) {
	int edge_bw = linphone_config_get_int(lc->config, "net", "edge_bw", 20);
	return edge_bw;
}

int linphone_core_get_edge_ptime(LinphoneCore *lc) {
	int edge_ptime = linphone_config_get_int(lc->config, "net", "edge_ptime", 100);
	return edge_ptime;
}

void linphone_core_resolve_stun_server(LinphoneCore *lc) {
	auto accounts = linphone_core_get_account_list(lc);
	for (auto item = accounts; item; item = bctbx_list_next(item)) {
		auto account = static_cast<LinphoneAccount *>(bctbx_list_get_data(item));
		auto policy = linphone_account_params_get_nat_policy(linphone_account_get_params(account));
		if (policy) linphone_nat_policy_resolve_stun_server(policy);
	}
	if (lc->nat_policy) linphone_nat_policy_resolve_stun_server(lc->nat_policy);
}

const struct addrinfo *linphone_core_get_stun_server_addrinfo(LinphoneCore *lc) {
	if (lc->nat_policy != NULL) {
		return linphone_nat_policy_get_stun_server_addrinfo(lc->nat_policy);
	} else {
		ms_error("linphone_core_get_stun_server_addrinfo(): called without nat_policy, this should not happen.");
	}
	return NULL;
}

void linphone_core_enable_forced_ice_relay(LinphoneCore *lc, bool_t enable) {
	lc->forced_ice_relay = enable;
}

bool_t linphone_core_forced_ice_relay_enabled(const LinphoneCore *lc) {
	return lc->forced_ice_relay;
}

void linphone_core_enable_short_turn_refresh(LinphoneCore *lc, bool_t enable) {
	lc->short_turn_refresh = enable;
}

const char *linphone_ice_state_to_string(LinphoneIceState state) {
	switch (state) {
		case LinphoneIceStateFailed:
			return "IceStateFailed";
		case LinphoneIceStateHostConnection:
			return "IceStateHostConnection";
		case LinphoneIceStateInProgress:
			return "IceStateInProgress";
		case LinphoneIceStateNotActivated:
			return "IceStateNotActivated";
		case LinphoneIceStateReflexiveConnection:
			return "IceStateReflexiveConnection";
		case LinphoneIceStateRelayConnection:
			return "IceStateRelayConnection";
	}
	return "invalid";
}

bool_t linphone_core_media_description_contains_video_stream(const LinphonePrivate::SalMediaDescription *md) {

	for (const auto &stream : md->streams) {
		if (stream.type == SalVideo && stream.rtp_port != 0) return TRUE;
	}
	return FALSE;
}

unsigned int linphone_core_get_audio_features(LinphoneCore *lc) {
	unsigned int ret = 0;
	const char *features = linphone_config_get_string(lc->config, "sound", "features", NULL);
	if (features) {
		char tmp[256] = {0};
		char name[256];
		char *p, *n;
		strncpy(tmp, features, sizeof(tmp) - 1);
		for (p = tmp; *p != '\0'; p++) {
			if (*p == ' ') continue;
			n = strchr(p, '|');
			if (n) *n = '\0';
			sscanf(p, "%s", name);
			ms_message("Found audio feature %s", name);
			if (strcasecmp(name, "PLC") == 0) ret |= AUDIO_STREAM_FEATURE_PLC;
			else if (strcasecmp(name, "EC") == 0) ret |= AUDIO_STREAM_FEATURE_EC;
			else if (strcasecmp(name, "EQUALIZER") == 0) ret |= AUDIO_STREAM_FEATURE_EQUALIZER;
			else if (strcasecmp(name, "VOL_SND") == 0) ret |= AUDIO_STREAM_FEATURE_VOL_SND;
			else if (strcasecmp(name, "VOL_RCV") == 0) ret |= AUDIO_STREAM_FEATURE_VOL_RCV;
			else if (strcasecmp(name, "DTMF") == 0) ret |= AUDIO_STREAM_FEATURE_DTMF;
			else if (strcasecmp(name, "DTMF_ECHO") == 0) ret |= AUDIO_STREAM_FEATURE_DTMF_ECHO;
			else if (strcasecmp(name, "MIXED_RECORDING") == 0) ret |= AUDIO_STREAM_FEATURE_MIXED_RECORDING;
			else if (strcasecmp(name, "LOCAL_PLAYING") == 0) ret |= AUDIO_STREAM_FEATURE_LOCAL_PLAYING;
			else if (strcasecmp(name, "REMOTE_PLAYING") == 0) ret |= AUDIO_STREAM_FEATURE_REMOTE_PLAYING;
#ifdef HAVE_BAUDOT
			else if ((strcasecmp(name, "BAUDOT") == 0) && (linphone_core_baudot_enabled(lc)))
				ret |= AUDIO_STREAM_FEATURE_BAUDOT;
#endif /* HAVE_BAUDOT */
			else if (strcasecmp(name, "ALL") == 0) ret |= AUDIO_STREAM_FEATURE_ALL;
			else if (strcasecmp(name, "NONE") == 0) ret = 0;
			else ms_error("Unsupported audio feature %s requested in config file.", name);
			if (!n) break;
			p = n;
		}
	} else {
		ret = AUDIO_STREAM_FEATURE_ALL;
#ifdef HAVE_BAUDOT
		if (!linphone_core_baudot_enabled(lc)) {
			ret ^= AUDIO_STREAM_FEATURE_BAUDOT;
		}
#else
		ret ^= AUDIO_STREAM_FEATURE_BAUDOT;
#endif /* !HAVE_BAUDOT */
	}

	if (ret == AUDIO_STREAM_FEATURE_ALL) {
		/*since call recording is specified before creation of the stream in linphonecore,
		 * it will be requested on demand. It is not necessary to include it all the time*/
		ret &= (unsigned int)~AUDIO_STREAM_FEATURE_MIXED_RECORDING;
	}
	return ret;
}

int linphone_core_get_local_ip_for(int type, const char *dest, char *result) {
	if (dest && dest[0] == '\0')
		dest = NULL; /*If null, bctbx_get_local_ip_for() will use an arbitrary public address instead.*/
	return bctbx_get_local_ip_for(type, dest, 5060, result, LINPHONE_IPADDR_SIZE);
}

void linphone_core_get_local_ip(LinphoneCore *lc, int af, const char *dest, char *result) {
	if (af == AF_UNSPEC) {
		if (linphone_core_ipv6_enabled(lc)) {
			bool_t has_ipv6 = linphone_core_get_local_ip_for(AF_INET6, dest, result) == 0;
			if (strcmp(result, "::1") != 0) return; /*this machine has real ipv6 connectivity*/
			if ((linphone_core_get_local_ip_for(AF_INET, dest, result) == 0) && (strcmp(result, "127.0.0.1") != 0))
				return; /*this machine has only ipv4 connectivity*/
			if (has_ipv6) {
				/*this machine has only local loopback for both ipv4 and ipv6, so prefer ipv6*/
				strncpy(result, "::1", LINPHONE_IPADDR_SIZE);
				return;
			}
		}
		/*in all other cases use IPv4*/
		af = AF_INET;
	}
	linphone_core_get_local_ip_for(af, dest, result);
}

SalReason linphone_reason_to_sal(LinphoneReason reason) {
	switch (reason) {
		case LinphoneReasonNone:
			return SalReasonNone;
		case LinphoneReasonNoResponse:
			return SalReasonRequestTimeout;
		case LinphoneReasonForbidden:
			return SalReasonForbidden;
		case LinphoneReasonDeclined:
			return SalReasonDeclined;
		case LinphoneReasonNotFound:
			return SalReasonNotFound;
		case LinphoneReasonTemporarilyUnavailable:
			return SalReasonTemporarilyUnavailable;
		case LinphoneReasonBusy:
			return SalReasonBusy;
		case LinphoneReasonNotAcceptable:
			return SalReasonNotAcceptable;
		case LinphoneReasonIOError:
			return SalReasonServiceUnavailable;
		case LinphoneReasonDoNotDisturb:
			return SalReasonDoNotDisturb;
		case LinphoneReasonUnauthorized:
			return SalReasonUnauthorized;
		case LinphoneReasonConditionalRequestFailed:
			return SalReasonConditionalRequestFailed;
		case LinphoneReasonUnsupportedContent:
			return SalReasonUnsupportedContent;
		case LinphoneReasonBadEvent:
			return SalReasonBadEvent;
		case LinphoneReasonNoMatch:
			return SalReasonNoMatch;
		case LinphoneReasonMovedPermanently:
			return SalReasonMovedPermanently;
		case LinphoneReasonGone:
			return SalReasonGone;
		case LinphoneReasonAddressIncomplete:
			return SalReasonAddressIncomplete;
		case LinphoneReasonNotImplemented:
			return SalReasonNotImplemented;
		case LinphoneReasonBadGateway:
			return SalReasonBadGateway;
		case LinphoneReasonSessionIntervalTooSmall:
			return SalReasonSessionIntervalTooSmall;
		case LinphoneReasonServerTimeout:
			return SalReasonServerTimeout;
		case LinphoneReasonNotAnswered:
			return SalReasonRequestTimeout;
		case LinphoneReasonTransferred: // It is not really used by Sal. This reason coming from managing tones on
		                                // transferred call.
			return SalReasonNone;
		case LinphoneReasonSasCheckRequired:
			return SalReasonSasCheckRequired;
		case LinphoneReasonUnknown:
			return SalReasonUnknown;
	}
	return SalReasonUnknown;
}

LinphoneReason linphone_reason_from_sal(SalReason r) {
	LinphoneReason ret = LinphoneReasonNone;
	switch (r) {
		case SalReasonNone:
			ret = LinphoneReasonNone;
			break;
		case SalReasonIOError:
			ret = LinphoneReasonIOError;
			break;
		case SalReasonUnknown:
		case SalReasonInternalError:
			ret = LinphoneReasonUnknown;
			break;
		case SalReasonBusy:
			ret = LinphoneReasonBusy;
			break;
		case SalReasonDeclined:
			ret = LinphoneReasonDeclined;
			break;
		case SalReasonDoNotDisturb:
			ret = LinphoneReasonDoNotDisturb;
			break;
		case SalReasonForbidden:
			ret = LinphoneReasonBadCredentials;
			break;
		case SalReasonNotAcceptable:
			ret = LinphoneReasonNotAcceptable;
			break;
		case SalReasonNotFound:
			ret = LinphoneReasonNotFound;
			break;
		case SalReasonRedirect:
			ret = LinphoneReasonNone;
			break;
		case SalReasonTemporarilyUnavailable:
			ret = LinphoneReasonTemporarilyUnavailable;
			break;
		case SalReasonServiceUnavailable:
			ret = LinphoneReasonIOError;
			break;
		case SalReasonRequestPending:
			ret = LinphoneReasonTemporarilyUnavailable; /*might not be exactly the perfect matching, but better than
			                                               LinphoneReasonNone*/
			break;
		case SalReasonUnauthorized:
			ret = LinphoneReasonUnauthorized;
			break;
		case SalReasonConditionalRequestFailed:
			ret = LinphoneReasonConditionalRequestFailed;
			break;
		case SalReasonUnsupportedContent:
			ret = LinphoneReasonUnsupportedContent;
			break;
		case SalReasonBadEvent:
			ret = LinphoneReasonBadEvent;
			break;
		case SalReasonNoMatch:
			ret = LinphoneReasonNoMatch;
			break;
		case SalReasonRequestTimeout:
			ret = LinphoneReasonNotAnswered;
			break;
		case SalReasonMovedPermanently:
			ret = LinphoneReasonMovedPermanently;
			break;
		case SalReasonGone:
			ret = LinphoneReasonGone;
			break;
		case SalReasonAddressIncomplete:
			ret = LinphoneReasonAddressIncomplete;
			break;
		case SalReasonNotImplemented:
			ret = LinphoneReasonNotImplemented;
			break;
		case SalReasonBadGateway:
			ret = LinphoneReasonBadGateway;
			break;
		case SalReasonSessionIntervalTooSmall:
			ret = LinphoneReasonSessionIntervalTooSmall;
			break;
		case SalReasonServerTimeout:
			ret = LinphoneReasonServerTimeout;
			break;
		case SalReasonSasCheckRequired:
			ret = LinphoneReasonSasCheckRequired;
			break;
	}
	return ret;
}

LinphoneStreamType sal_stream_type_to_linphone(SalStreamType type) {
	switch (type) {
		case SalAudio:
			return LinphoneStreamTypeAudio;
		case SalVideo:
			return LinphoneStreamTypeVideo;
		case SalText:
			return LinphoneStreamTypeText;
		case SalOther:
			return LinphoneStreamTypeUnknown;
	}
	return LinphoneStreamTypeUnknown;
}

SalStreamType linphone_stream_type_to_sal(LinphoneStreamType type) {
	switch (type) {
		case LinphoneStreamTypeAudio:
			return SalAudio;
		case LinphoneStreamTypeVideo:
			return SalVideo;
		case LinphoneStreamTypeText:
			return SalText;
		case LinphoneStreamTypeUnknown:
			return SalOther;
	}
	return SalOther;
}

/**
 * Set the name of the mediastreamer2 filter to be used for rendering video.
 * This is for advanced users of the library, mainly to workaround hardware/driver bugs.
 * @ingroup media_parameters
 **/
void linphone_core_set_video_display_filter(LinphoneCore *lc, const char *filter_name) {
	linphone_config_set_string(lc->config, "video", "displaytype", filter_name);
}

const char *linphone_core_get_video_display_filter(LinphoneCore *lc) {
	return linphone_config_get_string(lc->config, "video", "displaytype", NULL);
}

const char *linphone_core_get_default_video_display_filter(LinphoneCore *lc) {
	return ms_factory_get_default_video_renderer(lc->factory);
}

bool_t linphone_core_is_media_filter_supported(LinphoneCore *lc, const char *filtername) {
	return ms_factory_lookup_filter_by_name(lc->factory, filtername) != NULL;
}

void linphone_core_set_echo_canceller_filter_name(LinphoneCore *lc, const char *filtername) {
	linphone_config_set_string(lc->config, "sound", "ec_filter", filtername);
	if (filtername != NULL) {
		ms_factory_set_echo_canceller_filter_name(lc->factory, filtername);
	}
}

const char *linphone_core_get_echo_canceller_filter_name(const LinphoneCore *lc) {
	return linphone_config_get_string(lc->config, "sound", "ec_filter", NULL);
}

/**
 * Queue a task into the main loop. The data pointer must remain valid until the task is completed.
 * task_fun must return BELLE_SIP_STOP when job is finished.
 **/
void linphone_core_queue_task(LinphoneCore *lc,
                              belle_sip_source_func_t task_fun,
                              void *data,
                              const char *task_description) {
	belle_sip_source_t *s = lc->sal->createTimer(task_fun, data, 20, task_description);
	belle_sip_object_unref(s);
}

LinphoneToneDescription *linphone_tone_description_new(LinphoneToneID id, const char *audiofile) {
	LinphoneToneDescription *obj = ms_new0(LinphoneToneDescription, 1);
	obj->toneid = id;
	obj->audiofile = audiofile ? ms_strdup(audiofile) : NULL;
	return obj;
}

void linphone_tone_description_destroy(LinphoneToneDescription *obj) {
	if (obj->audiofile) ms_free(obj->audiofile);
	ms_free(obj);
}

void linphone_core_set_tone(LinphoneCore *lc, LinphoneToneID id, const char *audiofile) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->getToneManager().setTone(id, audiofile);
}

const MSCryptoSuite *linphone_core_generate_srtp_crypto_suites_array_from_string(LinphoneCore *lc, const char *config) {
	char *tmp = ms_strdup(config);

	char *sep;
	char *pos;
	char *nameend;
	char *nextpos;
	char *params;
	unsigned long found = 0;
	MSCryptoSuite *result = NULL;
	pos = tmp;
	do {
		sep = strchr(pos, ',');
		if (!sep) {
			sep = pos + strlen(pos);
			nextpos = NULL;
		} else {
			*sep = '\0';
			nextpos = sep + 1;
		}
		while (*pos == ' ')
			++pos;                 /*strip leading spaces*/
		params = strchr(pos, ' '); /*look for params that arrive after crypto suite name*/
		if (params) {
			while (*params == ' ')
				++params; /*strip parameters leading space*/
		}
		nameend = strchr(pos, ' ');
		if (nameend) {
			*nameend = '\0';
		}
		if (sep - pos > 0) {
			MSCryptoSuiteNameParams np;
			MSCryptoSuite suite;
			np.name = pos;
			np.params = params;
			suite = ms_crypto_suite_build_from_name_params(&np);
			if (suite != MS_CRYPTO_SUITE_INVALID) {
				result = reinterpret_cast<MSCryptoSuite *>(ms_realloc(result, (found + 2) * sizeof(MSCryptoSuite)));
				result[found] = suite;
				result[found + 1] = MS_CRYPTO_SUITE_INVALID;
				found++;
				ms_message("Configured srtp crypto suite: %s %s", np.name, np.params ? np.params : "");
			}
		}
		pos = nextpos;
	} while (pos);
	ms_free(tmp);
	if (lc->rtp_conf.srtp_suites) {
		ms_free(lc->rtp_conf.srtp_suites);
		lc->rtp_conf.srtp_suites = NULL;
	}
	lc->rtp_conf.srtp_suites = result;
	return result;
}

const MSCryptoSuite *linphone_core_get_srtp_crypto_suites_array(LinphoneCore *lc) {
	const char *config = linphone_core_get_srtp_crypto_suites(lc);
	return linphone_core_generate_srtp_crypto_suites_array_from_string(lc, config);
}

const MSCryptoSuite *linphone_core_get_all_supported_srtp_crypto_suites(LinphoneCore *lc) {
	const char *config =
	    "AES_CM_128_HMAC_SHA1_80, AES_CM_128_HMAC_SHA1_32, AES_256_CM_HMAC_SHA1_80, "
	    "AES_256_CM_HMAC_SHA1_32,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_80 "
	    "UNENCRYPTED_SRTP,AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP UNENCRYPTED_SRTP,AES_CM_128_HMAC_SHA1_80 "
	    "UNAUTHENTICATED_SRTP,AES_CM_128_HMAC_SHA1_32 UNAUTHENTICATED_SRTP,AEAD_AES_128_GCM,AEAD_AES_256_GCM";
	return linphone_core_generate_srtp_crypto_suites_array_from_string(lc, config);
}

static char *seperate_string_list(char **str) {
	char *ret;

	if (str == NULL) return NULL;
	if (*str == NULL) return NULL;
	if (**str == '\0') return NULL;

	ret = *str;
	for (; **str != '\0' && **str != ' ' && **str != ','; (*str)++)
		;
	if (**str == '\0') {
		return ret;
	} else {
		**str = '\0';
		do {
			(*str)++;
		} while (**str != '\0' && (**str == ' ' || **str == ','));
		return ret;
	}
}

bool_t linphone_core_get_post_quantum_available(void) {
	return ms_zrtp_is_PQ_available();
}

static LinphoneZrtpKeyAgreement MS2_to_Linphone_ZrtpKeyAgreement(MSZrtpKeyAgreement key_agreement) {
	switch (key_agreement) {
		case MS_ZRTP_KEY_AGREEMENT_DH2K:
			return LinphoneZrtpKeyAgreementDh2k;
		case MS_ZRTP_KEY_AGREEMENT_DH3K:
			return LinphoneZrtpKeyAgreementDh3k;
		case MS_ZRTP_KEY_AGREEMENT_EC25:
			return LinphoneZrtpKeyAgreementEc25;
		case MS_ZRTP_KEY_AGREEMENT_EC38:
			return LinphoneZrtpKeyAgreementEc38;
		case MS_ZRTP_KEY_AGREEMENT_EC52:
			return LinphoneZrtpKeyAgreementEc52;
		case MS_ZRTP_KEY_AGREEMENT_X255:
			return LinphoneZrtpKeyAgreementX255;
		case MS_ZRTP_KEY_AGREEMENT_X448:
			return LinphoneZrtpKeyAgreementX448;
		case MS_ZRTP_KEY_AGREEMENT_K255:
			return LinphoneZrtpKeyAgreementK255;
		case MS_ZRTP_KEY_AGREEMENT_K448:
			return LinphoneZrtpKeyAgreementK448;
		case MS_ZRTP_KEY_AGREEMENT_MLK1:
			return LinphoneZrtpKeyAgreementMlk1;
		case MS_ZRTP_KEY_AGREEMENT_MLK2:
			return LinphoneZrtpKeyAgreementMlk2;
		case MS_ZRTP_KEY_AGREEMENT_MLK3:
			return LinphoneZrtpKeyAgreementMlk3;
		case MS_ZRTP_KEY_AGREEMENT_KYB1:
			return LinphoneZrtpKeyAgreementKyb1;
		case MS_ZRTP_KEY_AGREEMENT_KYB2:
			return LinphoneZrtpKeyAgreementKyb2;
		case MS_ZRTP_KEY_AGREEMENT_KYB3:
			return LinphoneZrtpKeyAgreementKyb3;
		case MS_ZRTP_KEY_AGREEMENT_HQC1:
			return LinphoneZrtpKeyAgreementHqc1;
		case MS_ZRTP_KEY_AGREEMENT_HQC2:
			return LinphoneZrtpKeyAgreementHqc2;
		case MS_ZRTP_KEY_AGREEMENT_HQC3:
			return LinphoneZrtpKeyAgreementHqc3;
		case MS_ZRTP_KEY_AGREEMENT_K255_MLK512:
			return LinphoneZrtpKeyAgreementK255Mlk512;
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512:
			return LinphoneZrtpKeyAgreementK255Kyb512;
		case MS_ZRTP_KEY_AGREEMENT_K255_HQC128:
			return LinphoneZrtpKeyAgreementK255Hqc128;
		case MS_ZRTP_KEY_AGREEMENT_K448_MLK1024:
			return LinphoneZrtpKeyAgreementK448Mlk1024;
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024:
			return LinphoneZrtpKeyAgreementK448Kyb1024;
		case MS_ZRTP_KEY_AGREEMENT_K448_HQC256:
			return LinphoneZrtpKeyAgreementK448Hqc256;
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128:
			return LinphoneZrtpKeyAgreementK255Kyb512Hqc128;
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256:
			return LinphoneZrtpKeyAgreementK448Kyb1024Hqc256;
		default:
			return LinphoneZrtpKeyAgreementInvalid;
	}
}

static bctbx_list_t *MS2_to_Linphone_ZrtpKeyAgreement_array(MSZrtpKeyAgreement *key_agreements,
                                                            uint8_t key_agreements_size) {
	uint8_t i = 0;
	bctbx_list_t *linphone_key_agreements = NULL;
	for (i = 0; i < key_agreements_size; i++) {
		LinphoneZrtpKeyAgreement key_agreement = MS2_to_Linphone_ZrtpKeyAgreement(key_agreements[i]);
		if (key_agreement != LinphoneZrtpKeyAgreementInvalid) {
			linphone_key_agreements = bctbx_list_append(linphone_key_agreements, LINPHONE_INT_TO_PTR(key_agreement));
		}
	}
	return linphone_key_agreements;
}

static MSZrtpKeyAgreement Linphone_to_MS2_ZrtpKeyAgreement(LinphoneZrtpKeyAgreement key_agreement) {
	switch (key_agreement) {
		case LinphoneZrtpKeyAgreementDh2k:
			return MS_ZRTP_KEY_AGREEMENT_DH2K;
		case LinphoneZrtpKeyAgreementDh3k:
			return MS_ZRTP_KEY_AGREEMENT_DH3K;
		case LinphoneZrtpKeyAgreementEc25:
			return MS_ZRTP_KEY_AGREEMENT_EC25;
		case LinphoneZrtpKeyAgreementEc38:
			return MS_ZRTP_KEY_AGREEMENT_EC38;
		case LinphoneZrtpKeyAgreementEc52:
			return MS_ZRTP_KEY_AGREEMENT_EC52;
		case LinphoneZrtpKeyAgreementX255:
			return MS_ZRTP_KEY_AGREEMENT_X255;
		case LinphoneZrtpKeyAgreementX448:
			return MS_ZRTP_KEY_AGREEMENT_X448;
		case LinphoneZrtpKeyAgreementK255:
			return MS_ZRTP_KEY_AGREEMENT_K255;
		case LinphoneZrtpKeyAgreementK448:
			return MS_ZRTP_KEY_AGREEMENT_K448;
		case LinphoneZrtpKeyAgreementMlk1:
			return MS_ZRTP_KEY_AGREEMENT_MLK1;
		case LinphoneZrtpKeyAgreementMlk2:
			return MS_ZRTP_KEY_AGREEMENT_MLK2;
		case LinphoneZrtpKeyAgreementMlk3:
			return MS_ZRTP_KEY_AGREEMENT_MLK3;
		case LinphoneZrtpKeyAgreementKyb1:
			return MS_ZRTP_KEY_AGREEMENT_KYB1;
		case LinphoneZrtpKeyAgreementKyb2:
			return MS_ZRTP_KEY_AGREEMENT_KYB2;
		case LinphoneZrtpKeyAgreementKyb3:
			return MS_ZRTP_KEY_AGREEMENT_KYB3;
		case LinphoneZrtpKeyAgreementHqc1:
			return MS_ZRTP_KEY_AGREEMENT_HQC1;
		case LinphoneZrtpKeyAgreementHqc2:
			return MS_ZRTP_KEY_AGREEMENT_HQC2;
		case LinphoneZrtpKeyAgreementHqc3:
			return MS_ZRTP_KEY_AGREEMENT_HQC3;
		case LinphoneZrtpKeyAgreementK255Mlk512:
			return MS_ZRTP_KEY_AGREEMENT_K255_MLK512;
		case LinphoneZrtpKeyAgreementK255Kyb512:
			return MS_ZRTP_KEY_AGREEMENT_K255_KYB512;
		case LinphoneZrtpKeyAgreementK255Hqc128:
			return MS_ZRTP_KEY_AGREEMENT_K255_HQC128;
		case LinphoneZrtpKeyAgreementK448Mlk1024:
			return MS_ZRTP_KEY_AGREEMENT_K448_MLK1024;
		case LinphoneZrtpKeyAgreementK448Kyb1024:
			return MS_ZRTP_KEY_AGREEMENT_K448_KYB1024;
		case LinphoneZrtpKeyAgreementK448Hqc256:
			return MS_ZRTP_KEY_AGREEMENT_K448_HQC256;
		case LinphoneZrtpKeyAgreementK255Kyb512Hqc128:
			return MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128;
		case LinphoneZrtpKeyAgreementK448Kyb1024Hqc256:
			return MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256;
		default:
			return MS_ZRTP_KEY_AGREEMENT_INVALID;
	}
}

bctbx_list_t *linphone_core_get_zrtp_available_key_agreement_list(BCTBX_UNUSED(LinphoneCore *lc)) {
	MSZrtpKeyAgreement algos[256];
	uint8_t nb_algos = ms_zrtp_available_key_agreement(algos);
	return MS2_to_Linphone_ZrtpKeyAgreement_array(algos, nb_algos);
}

bctbx_list_t *linphone_core_get_zrtp_key_agreement_list(LinphoneCore *lc) {
	MSZrtpKeyAgreement algos[MS_MAX_ZRTP_CRYPTO_TYPES];
	uint8_t nb_algos = linphone_core_get_zrtp_key_agreement_suites(lc, algos);
	return MS2_to_Linphone_ZrtpKeyAgreement_array(algos, nb_algos);
}

void linphone_core_set_zrtp_key_agreement_suites(LinphoneCore *lc, bctbx_list_t *key_agreements) {
	// Turn the bctbx_list in a comma separated list
	size_t list_size = 0;
	bctbx_list_t *string_list = NULL;
	for (; key_agreements != NULL && list_size < 7; key_agreements = bctbx_list_next(key_agreements)) {
		// Get the linphone enum
		LinphoneZrtpKeyAgreement l_key_agreement =
		    (LinphoneZrtpKeyAgreement)(LINPHONE_PTR_TO_INT(bctbx_list_get_data(key_agreements)));
		// Convert it to MS2 enum
		MSZrtpKeyAgreement ms2_key_agreement = Linphone_to_MS2_ZrtpKeyAgreement(l_key_agreement);
		// Convert it to a string and concatenate it to what we already have
		if (ms2_key_agreement != MS_ZRTP_KEY_AGREEMENT_INVALID) {
			string_list =
			    bctbx_list_append(string_list, bctbx_strdup(ms_zrtp_key_agreement_to_string(ms2_key_agreement)));
			list_size++;
		}
	}
	// set it
	linphone_config_set_string_list(lc->config, "sip", "zrtp_key_agreements_suites", string_list);
	bctbx_list_free_with_data(string_list, bctbx_free);
}

MsZrtpCryptoTypesCount
linphone_core_get_zrtp_key_agreement_suites(LinphoneCore *lc,
                                            MSZrtpKeyAgreement keyAgreements[MS_MAX_ZRTP_CRYPTO_TYPES]) {
	char *zrtpConfig = (char *)linphone_config_get_string(lc->config, "sip", "zrtp_key_agreements_suites", NULL);
	MsZrtpCryptoTypesCount key_agreements_count = 0;
	char *entry, *origPtr;
	if (zrtpConfig == NULL) {
		return 0;
	}

	origPtr = ms_strdup(zrtpConfig);
	zrtpConfig = origPtr;
	while ((entry = seperate_string_list(&zrtpConfig))) {
		const MSZrtpKeyAgreement agreement = ms_zrtp_key_agreement_from_string(entry);
		if (agreement != MS_ZRTP_KEY_AGREEMENT_INVALID) {
			ms_message("Configured zrtp key agreement: '%s'", ms_zrtp_key_agreement_to_string(agreement));
			keyAgreements[key_agreements_count++] = agreement;
		}
	}

	ms_free(origPtr);
	return key_agreements_count;
}

MsZrtpCryptoTypesCount linphone_core_get_zrtp_cipher_suites(LinphoneCore *lc,
                                                            MSZrtpCipher ciphers[MS_MAX_ZRTP_CRYPTO_TYPES]) {
	char *zrtpConfig = (char *)linphone_config_get_string(lc->config, "sip", "zrtp_cipher_suites", NULL);
	MsZrtpCryptoTypesCount cipher_count = 0;
	char *entry, *origPtr;
	if (zrtpConfig == NULL) {
		return 0;
	}

	origPtr = ms_strdup(zrtpConfig);
	zrtpConfig = origPtr;
	while ((entry = seperate_string_list(&zrtpConfig))) {
		const MSZrtpCipher cipher = ms_zrtp_cipher_from_string(entry);
		if (cipher != MS_ZRTP_CIPHER_INVALID) {
			ms_message("Configured zrtp cipher: '%s'", ms_zrtp_cipher_to_string(cipher));
			ciphers[cipher_count++] = cipher;
		}
	}

	ms_free(origPtr);
	return cipher_count;
}

MsZrtpCryptoTypesCount linphone_core_get_zrtp_hash_suites(LinphoneCore *lc,
                                                          MSZrtpHash hashes[MS_MAX_ZRTP_CRYPTO_TYPES]) {
	char *zrtpConfig = (char *)linphone_config_get_string(lc->config, "sip", "zrtp_hash_suites", NULL);
	MsZrtpCryptoTypesCount hash_count = 0;
	char *entry, *origPtr;
	if (zrtpConfig == NULL) {
		return 0;
	}

	origPtr = ms_strdup(zrtpConfig);
	zrtpConfig = origPtr;
	while ((entry = seperate_string_list(&zrtpConfig))) {
		const MSZrtpHash hash = ms_zrtp_hash_from_string(entry);
		if (hash != MS_ZRTP_HASH_INVALID) {
			ms_message("Configured zrtp hash: '%s'", ms_zrtp_hash_to_string(hash));
			hashes[hash_count++] = hash;
		}
	}

	ms_free(origPtr);
	return hash_count;
}

MsZrtpCryptoTypesCount linphone_core_get_zrtp_auth_suites(LinphoneCore *lc,
                                                          MSZrtpAuthTag authTags[MS_MAX_ZRTP_CRYPTO_TYPES]) {
	char *zrtpConfig = (char *)linphone_config_get_string(lc->config, "sip", "zrtp_auth_suites", NULL);
	MsZrtpCryptoTypesCount auth_tag_count = 0;
	char *entry, *origPtr;
	if (zrtpConfig == NULL) {
		return 0;
	}

	origPtr = ms_strdup(zrtpConfig);
	zrtpConfig = origPtr;
	while ((entry = seperate_string_list(&zrtpConfig))) {
		const MSZrtpAuthTag authTag = ms_zrtp_auth_tag_from_string(entry);
		if (authTag != MS_ZRTP_AUTHTAG_INVALID) {
			ms_message("Configured zrtp auth tag: '%s'", ms_zrtp_auth_tag_to_string(authTag));
			authTags[auth_tag_count++] = authTag;
		}
	}

	ms_free(origPtr);
	return auth_tag_count;
}

MsZrtpCryptoTypesCount linphone_core_get_zrtp_sas_suites(LinphoneCore *lc,
                                                         MSZrtpSasType sasTypes[MS_MAX_ZRTP_CRYPTO_TYPES]) {
	char *zrtpConfig = (char *)linphone_config_get_string(lc->config, "sip", "zrtp_sas_suites", NULL);
	MsZrtpCryptoTypesCount sas_count = 0;
	char *entry, *origPtr;
	if (zrtpConfig == NULL) {
		return 0;
	}

	origPtr = ms_strdup(zrtpConfig);
	zrtpConfig = origPtr;
	while ((entry = seperate_string_list(&zrtpConfig))) {
		const MSZrtpSasType type = ms_zrtp_sas_type_from_string(entry);
		if (type != MS_ZRTP_SAS_INVALID) {
			ms_message("Configured zrtp SAS type: '%s'", ms_zrtp_sas_type_to_string(type));
			sasTypes[sas_count++] = type;
		}
	}

	ms_free(origPtr);
	return sas_count;
}

static const char **_linphone_core_get_supported_file_formats(const LinphoneCore *core) {
	LinphoneCore *mutable_core = (LinphoneCore *)core;
	static const char *mkv = "mkv";
	static const char *wav = "wav";
	static const char *mka = "mka";
	static const char *smff = "smff";
	int i = 0;
	if (core->supported_formats == NULL) {
		mutable_core->supported_formats = reinterpret_cast<const char **>(ms_malloc0(5 * sizeof(char *)));
		core->supported_formats[i++] = wav;
		if (ms_factory_lookup_filter_by_id(core->factory, MS_MKV_RECORDER_ID)) {
			core->supported_formats[i++] = mkv;
			core->supported_formats[i++] = mka;
		}
		if (ms_factory_lookup_filter_by_id(core->factory, MS_SMFF_RECORDER_ID)) {
			core->supported_formats[i++] = smff;
		}
	}
	return core->supported_formats;
}

const char **linphone_core_get_supported_file_formats(LinphoneCore *core) {
	return _linphone_core_get_supported_file_formats(core);
}

bctbx_list_t *linphone_core_get_supported_file_formats_list(const LinphoneCore *core) {
	bctbx_list_t *file_formats = NULL;
	const char **fmts = _linphone_core_get_supported_file_formats(core);
	for (; *fmts != nullptr; fmts++) {
		file_formats = bctbx_list_append(file_formats, bctbx_strdup(*fmts));
	}
	return file_formats;
}

bool_t linphone_core_file_format_supported(LinphoneCore *lc, const char *fmt) {
	const char **formats = _linphone_core_get_supported_file_formats(lc);
	for (; *formats != NULL; ++formats) {
		if (strcasecmp(*formats, fmt) == 0) return TRUE;
	}
	return FALSE;
}

bool_t linphone_core_symmetric_rtp_enabled(LinphoneCore *lc) {
	/* Clients don't really need rtp symmetric, unless they have a public IP address and want
	 * to interoperate with natted client. This case is not frequent with client apps.
	 */
	return !!linphone_config_get_int(lc->config, "rtp", "symmetric", 0);
}

LinphoneStatus linphone_core_set_network_simulator_params(LinphoneCore *lc, const OrtpNetworkSimulatorParams *params) {
	if (params != &lc->net_conf.netsim_params) lc->net_conf.netsim_params = *params;

	/* update all running streams. */
	for (const auto &call : L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCalls()) {
		std::shared_ptr<LinphonePrivate::MediaSession> ms = call->getMediaSession();
		if (ms) {
			L_GET_PRIVATE(ms)->getStreamsGroup().forEach<LinphonePrivate::MS2Stream>(
			    [params](LinphonePrivate::MS2Stream *ms2s) {
				    MediaStream *stream = ms2s->getMediaStream();
				    if (stream && stream->sessions.rtp_session) {
					    rtp_session_enable_network_simulation(stream->sessions.rtp_session, params);
				    }
			    });
		}
	}
	return 0;
}

const OrtpNetworkSimulatorParams *linphone_core_get_network_simulator_params(const LinphoneCore *lc) {
	return &lc->net_conf.netsim_params;
}

static const char *_tunnel_mode_str[3] = {"disable", "enable", "auto"};

LinphoneTunnelMode linphone_tunnel_mode_from_string(const char *string) {
	if (string != NULL) {
		int i;
		for (i = 0; i < 3 && strcmp(string, _tunnel_mode_str[i]) != 0; i++)
			;
		if (i < 3) {
			return (LinphoneTunnelMode)i;
		} else {
			ms_error("Invalid tunnel mode '%s'", string);
			return LinphoneTunnelModeDisable;
		}
	} else {
		return LinphoneTunnelModeDisable;
	}
}

const char *linphone_tunnel_mode_to_string(LinphoneTunnelMode mode) {
	switch (mode) {
		case LinphoneTunnelModeAuto:
			return "auto";
		case LinphoneTunnelModeDisable:
			return "disable";
		case LinphoneTunnelModeEnable:
			return "enable";
	}
	return "invalid";
}

/* Functions to mainpulate the LinphoneRange structure */

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneRange);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneRange,
                           belle_sip_object_t,
                           NULL, // destroy
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

LinphoneRange *linphone_range_new() {
	LinphoneRange *range = belle_sip_object_new(LinphoneRange);
	range->min = 0;
	range->max = 0;
	return range;
}

LinphoneRange *linphone_range_ref(LinphoneRange *range) {
	return (LinphoneRange *)belle_sip_object_ref(range);
}

void linphone_range_unref(LinphoneRange *range) {
	belle_sip_object_unref(range);
}

void *linphone_range_get_user_data(const LinphoneRange *range) {
	return range->user_data;
}

void linphone_range_set_user_data(LinphoneRange *range, void *data) {
	range->user_data = data;
}

int linphone_range_get_min(const LinphoneRange *range) {
	return range->min;
}

int linphone_range_get_max(const LinphoneRange *range) {
	return range->max;
}

void linphone_range_set_min(LinphoneRange *range, int min) {
	range->min = min;
}

void linphone_range_set_max(LinphoneRange *range, int max) {
	range->max = max;
}

LinphoneHeaders *linphone_headers_ref(LinphoneHeaders *obj) {
	sal_custom_header_ref((SalCustomHeader *)obj);
	return obj;
}

void linphone_headers_unref(LinphoneHeaders *obj) {
	sal_custom_header_unref((SalCustomHeader *)obj);
}

const char *linphone_headers_get_value(const LinphoneHeaders *obj, const char *header_name) {
	return sal_custom_header_find((const SalCustomHeader *)obj, header_name);
}

void linphone_headers_add(LinphoneHeaders *obj, const char *name, const char *value) {
	sal_custom_header_append((SalCustomHeader *)obj, name, value);
}

void linphone_headers_remove(LinphoneHeaders *obj, const char *name) {
	sal_custom_header_remove((SalCustomHeader *)obj, name);
}

/* For c#, the char * or const char * returned by the liblinphone API
 * are returned as IntPtr.
 * Indeed, the default Marshall serialization will use CoTaskMemFree() or free() (for non-windows)
 * in order to free the returned const char* or char *, which is not acceptable as
 * for the const char* as it will create a memory corruption.
 * We then use linphone_pointer_to_string() to convert the IntPtr into
 * a char* that can be consumed by the C# marshaller.
 */
char *linphone_pointer_to_string(const void *ptr) {
	char *ret;
	if (ptr == NULL) return NULL;
#ifdef _WIN32
	size_t len = strlen((const char *)ptr) + 1;
	ret = (char *)CoTaskMemAlloc(len);
	memcpy(ret, ptr, len);
#else
	ret = strdup((const char *)ptr);
#endif
	return ret;
}
/* return value needs to be freed with bctbx_free(). */
void *linphone_string_to_pointer(const char *ptr) {
	return bctbx_strdup(ptr);
}

LinphoneStatus linphone_force_utf8(void) {
	const char *current_setting;
	setlocale(LC_ALL, ".UTF-8");
	current_setting = setlocale(LC_ALL, NULL);
	if (current_setting == NULL) {
		lError() << "setlocale() failed.";
		return -1;
	}
	if (strstr(current_setting, ".utf-8") == NULL && strstr(current_setting, ".UTF-8") == NULL &&
	    strstr(current_setting, ".utf8") == NULL) {
		lError() << "It is unsure that UTF-8 was really set, locale is: " << current_setting;
		return -1;
	}
	return 0;
}
