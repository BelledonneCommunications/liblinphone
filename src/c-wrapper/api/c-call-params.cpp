/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/conference-enums.h"
#include "conference/params/call-session-params-p.h"
#include "conference/params/media-session-params-p.h"
#include "conference/session/call-session.h"
#include "core/core.h"
#include "linphone/api/c-content.h"
#include "linphone/call_params.h"

// =============================================================================

L_DECLARE_C_CLONABLE_OBJECT_IMPL(CallParams)

using namespace std;

// =============================================================================
// Internal functions.
// =============================================================================

SalMediaProto get_proto_from_call_params(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getMediaProto();
}

SalStreamDir sal_dir_from_call_params_dir(LinphoneMediaDirection cpdir) {
	switch (cpdir) {
		case LinphoneMediaDirectionInactive:
			return SalStreamInactive;
		case LinphoneMediaDirectionSendOnly:
			return SalStreamSendOnly;
		case LinphoneMediaDirectionRecvOnly:
			return SalStreamRecvOnly;
		case LinphoneMediaDirectionSendRecv:
			return SalStreamSendRecv;
		case LinphoneMediaDirectionInvalid:
			ms_error("LinphoneMediaDirectionInvalid shall not be used.");
			return SalStreamInactive;
	}
	return SalStreamSendRecv;
}

LinphoneMediaDirection media_direction_from_sal_stream_dir(SalStreamDir dir) {
	switch (dir) {
		case SalStreamInactive:
			return LinphoneMediaDirectionInactive;
		case SalStreamSendOnly:
			return LinphoneMediaDirectionSendOnly;
		case SalStreamRecvOnly:
			return LinphoneMediaDirectionRecvOnly;
		case SalStreamSendRecv:
			return LinphoneMediaDirectionSendRecv;
	}
	return LinphoneMediaDirectionSendRecv;
}

SalStreamDir get_audio_dir_from_call_params(const LinphoneCallParams *params) {
	return sal_dir_from_call_params_dir(linphone_call_params_get_audio_direction(params));
}

SalStreamDir get_video_dir_from_call_params(const LinphoneCallParams *params) {
	return sal_dir_from_call_params_dir(linphone_call_params_get_video_direction(params));
}

bool_t linphone_call_params_is_valid(const LinphoneCallParams *params) {
	return !!L_GET_CPP_PTR_FROM_C_OBJECT(params)->isValid();
}

bool_t linphone_call_params_ringing_disabled(const LinphoneCallParams *params) {
	return !!L_GET_PRIVATE_FROM_C_OBJECT(params)->ringingDisabled();
}

void linphone_call_params_disable_ringing(LinphoneCallParams *params, bool_t disable) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->disableRinging(!!disable);
}

bool_t linphone_call_params_tone_indications_enabled(const LinphoneCallParams *params) {
	return !!L_GET_PRIVATE_FROM_C_OBJECT(params)->toneIndicationsEnabled();
}

void linphone_call_params_enable_tone_indications(LinphoneCallParams *params, bool_t enable) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->enableToneIndications(!!enable);
}

bool_t linphone_call_params_is_capability_negotiation_reinvite_enabled(const LinphoneCallParams *params) {
	return linphone_call_params_capability_negotiation_reinvite_enabled(params);
}

bool_t linphone_call_params_capability_negotiation_reinvite_enabled(const LinphoneCallParams *params) {
	return !!L_GET_PRIVATE_FROM_C_OBJECT(params)->capabilityNegotiationReInviteEnabled();
}

void linphone_call_params_enable_capability_negotiation_reinvite(LinphoneCallParams *params, bool_t enable) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->enableCapabilityNegotiationReInvite(!!enable);
}

bool_t linphone_call_params_capability_negotiations_enabled(const LinphoneCallParams *params) {
	return !!L_GET_PRIVATE_FROM_C_OBJECT(params)->capabilityNegotiationEnabled();
}

void linphone_call_params_enable_capability_negotiations(LinphoneCallParams *params, bool_t enable) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->enableCapabilityNegotiation(!!enable);
}

bool_t linphone_call_params_cfg_lines_merged(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->cfgLinesMerged();
}

void linphone_call_params_enable_cfg_lines_merging(LinphoneCallParams *params, bool_t enable) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->enableCfgLinesMerging(!!enable);
}

bool_t linphone_call_params_tcap_lines_merged(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->tcapLinesMerged();
}

void linphone_call_params_enable_tcap_line_merging(LinphoneCallParams *params, bool_t enable) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->enableTcapLineMerging(!!enable);
}

bool_t linphone_call_params_is_media_encryption_supported(const LinphoneCallParams *params,
                                                          const LinphoneMediaEncryption encryption) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->isMediaEncryptionSupported(encryption);
}

bctbx_list_t *linphone_call_params_get_supported_encryptions(const LinphoneCallParams *params) {
	const auto encEnumList = L_GET_PRIVATE_FROM_C_OBJECT(params)->getSupportedEncryptions();
	bctbx_list_t *encryption_list = NULL;
	for (const auto &enc : encEnumList) {
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(enc));
	}
	return encryption_list;
}

void linphone_call_params_set_supported_encryptions(LinphoneCallParams *params, bctbx_list_t *encs) {
	std::list<LinphoneMediaEncryption> encEnumList;
	for (bctbx_list_t *enc = encs; enc != NULL; enc = enc->next) {
		encEnumList.push_back(static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(enc))));
	}

	L_GET_PRIVATE_FROM_C_OBJECT(params)->setSupportedEncryptions(encEnumList);
}

bctbx_list_t *linphone_call_params_get_srtp_suites(const LinphoneCallParams *call_params) {
	const auto suitesList = L_GET_CPP_PTR_FROM_C_OBJECT(call_params)->getSrtpSuites();
	bctbx_list_t *encryption_list = NULL;
	for (const auto &suite : suitesList) {
		encryption_list = bctbx_list_append(encryption_list, LINPHONE_INT_TO_PTR(suite));
	}
	return encryption_list;
}

void linphone_call_params_set_srtp_suites(LinphoneCallParams *call_params, bctbx_list_t *srtpSuites) {
	std::list<LinphoneSrtpSuite> suitesList;
	for (bctbx_list_t *suite = srtpSuites; suite != NULL; suite = suite->next) {
		suitesList.push_back(static_cast<LinphoneSrtpSuite>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(suite))));
	}

	L_GET_CPP_PTR_FROM_C_OBJECT(call_params)->setSrtpSuites(suitesList);
}

void linphone_call_params_set_custom_headers(LinphoneCallParams *params, const SalCustomHeader *ch) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setCustomHeaders(ch);
}

void linphone_call_params_set_custom_sdp_attributes(LinphoneCallParams *params, const SalCustomSdpAttribute *csa) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setCustomSdpAttributes(csa);
}

void linphone_call_params_set_custom_sdp_media_attributes(LinphoneCallParams *params,
                                                          LinphoneStreamType type,
                                                          const SalCustomSdpAttribute *csa) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setCustomSdpMediaAttributes(type, csa);
}

/* Test feature only. */
void linphone_call_params_set_video_download_bandwidth(LinphoneCallParams *params, int bw) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->videoDownloadBandwidth = bw;
}

// =============================================================================
// Public functions.
// =============================================================================

void linphone_call_params_add_custom_header(LinphoneCallParams *params,
                                            const char *header_name,
                                            const char *header_value) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomHeader(header_name, L_C_TO_STRING(header_value));
}

void linphone_call_params_set_from_header(LinphoneCallParams *params, const char *from_value) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setFromHeader(L_C_TO_STRING(from_value));
}

const char *linphone_call_params_get_from_header(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getFromHeader();
}

void linphone_call_params_add_custom_sdp_attribute(LinphoneCallParams *params,
                                                   const char *attribute_name,
                                                   const char *attribute_value) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomSdpAttribute(attribute_name, L_C_TO_STRING(attribute_value));
}

void linphone_call_params_add_custom_sdp_media_attribute(LinphoneCallParams *params,
                                                         LinphoneStreamType type,
                                                         const char *attribute_name,
                                                         const char *attribute_value) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomSdpMediaAttribute(type, attribute_name,
	                                                                L_C_TO_STRING(attribute_value));
}

void linphone_call_params_clear_custom_sdp_attributes(LinphoneCallParams *params) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->clearCustomSdpAttributes();
}

void linphone_call_params_clear_custom_sdp_media_attributes(LinphoneCallParams *params, LinphoneStreamType type) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->clearCustomSdpMediaAttributes(type);
}

LinphoneCallParams *_linphone_call_params_copy(const LinphoneCallParams *params) {
	return (LinphoneCallParams *)belle_sip_object_clone((const belle_sip_object_t *)params);
}

LinphoneCallParams *linphone_call_params_copy(const LinphoneCallParams *params) {
	return _linphone_call_params_copy(params);
}

bool_t linphone_call_params_early_media_sending_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->earlyMediaSendingEnabled();
}

void linphone_call_params_enable_early_media_sending(LinphoneCallParams *params, bool_t enabled) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableEarlyMediaSending(!!enabled);
}

void linphone_call_params_enable_low_bandwidth(LinphoneCallParams *params, bool_t enabled) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableLowBandwidth(!!enabled);
}

void linphone_call_params_enable_audio(LinphoneCallParams *params, bool_t enabled) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableAudio(!!enabled);
}

LinphoneStatus linphone_call_params_enable_realtime_text(LinphoneCallParams *params, bool_t yesno) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableRealtimeText(!!yesno);
	return 0;
}

void linphone_call_params_set_realtime_text_keepalive_interval(LinphoneCallParams *params, unsigned int interval) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setRealtimeTextKeepaliveInterval(interval);
}

void linphone_call_params_enable_camera(LinphoneCallParams *params, bool_t enabled) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableCamera(!!enabled);
}

void linphone_call_params_enable_screen_sharing(LinphoneCallParams *params, bool_t enabled) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableScreenSharing(!!enabled);
}

void linphone_call_params_enable_video(LinphoneCallParams *params, bool_t enabled) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableVideo(!!enabled);
}

const char *linphone_call_params_get_custom_header(const LinphoneCallParams *params, const char *header_name) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getCustomHeader(header_name);
}

const char *linphone_call_params_get_custom_sdp_attribute(const LinphoneCallParams *params,
                                                          const char *attribute_name) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getCustomSdpAttribute(attribute_name);
}

const char *linphone_call_params_get_custom_sdp_media_attribute(const LinphoneCallParams *params,
                                                                LinphoneStreamType type,
                                                                const char *attribute_name) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getCustomSdpMediaAttribute(type, attribute_name);
}

bool_t linphone_call_params_get_local_conference_mode(const LinphoneCallParams *params) {
	return linphone_call_params_get_in_conference(params);
}

LinphoneMediaEncryption linphone_call_params_get_media_encryption(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getMediaEncryption();
}

LinphonePrivacyMask linphone_call_params_get_privacy(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getPrivacy();
}

float linphone_call_params_get_received_framerate(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getReceivedFps();
}

MSVideoSize linphone_call_params_get_received_video_size(const LinphoneCallParams *params) {
	MSVideoSize vsize;
	LinphoneVideoDefinition *vdef = L_GET_CPP_PTR_FROM_C_OBJECT(params)->getReceivedVideoDefinition();
	if (vdef) {
		vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
		vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
	} else {
		vsize.width = MS_VIDEO_SIZE_UNKNOWN_W;
		vsize.height = MS_VIDEO_SIZE_UNKNOWN_H;
	}
	return vsize;
}

const LinphoneVideoDefinition *linphone_call_params_get_received_video_definition(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getReceivedVideoDefinition();
}

const char *linphone_call_params_get_record_file(const LinphoneCallParams *params) {
	return L_STRING_TO_C(L_GET_CPP_PTR_FROM_C_OBJECT(params)->getRecordFilePath());
}

const char *linphone_call_params_get_rtp_profile(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getRtpProfile();
}

float linphone_call_params_get_sent_framerate(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getSentFps();
}

MSVideoSize linphone_call_params_get_sent_video_size(const LinphoneCallParams *params) {
	MSVideoSize vsize;
	LinphoneVideoDefinition *vdef = L_GET_CPP_PTR_FROM_C_OBJECT(params)->getSentVideoDefinition();
	if (vdef) {
		vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
		vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
	} else {
		vsize.width = MS_VIDEO_SIZE_UNKNOWN_W;
		vsize.height = MS_VIDEO_SIZE_UNKNOWN_H;
	}
	return vsize;
}

const LinphoneVideoDefinition *linphone_call_params_get_sent_video_definition(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getSentVideoDefinition();
}

const char *linphone_call_params_get_session_name(const LinphoneCallParams *params) {
	return L_STRING_TO_C(L_GET_CPP_PTR_FROM_C_OBJECT(params)->getSessionName());
}

const LinphonePayloadType *linphone_call_params_get_used_audio_payload_type(const LinphoneCallParams *params) {
	return bellesip::toC(L_GET_CPP_PTR_FROM_C_OBJECT(params)->getUsedAudioPayloadType());
}

const LinphonePayloadType *linphone_call_params_get_used_video_payload_type(const LinphoneCallParams *params) {
	return bellesip::toC(L_GET_CPP_PTR_FROM_C_OBJECT(params)->getUsedVideoPayloadType());
}

const LinphonePayloadType *linphone_call_params_get_used_text_payload_type(const LinphoneCallParams *params) {
	return bellesip::toC(L_GET_CPP_PTR_FROM_C_OBJECT(params)->getUsedRealtimeTextPayloadType());
}

bool_t linphone_call_params_low_bandwidth_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->lowBandwidthEnabled();
}

int linphone_call_params_get_audio_bandwidth_limit(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getAudioBandwidthLimit();
}

void linphone_call_params_set_audio_bandwidth_limit(LinphoneCallParams *params, int bandwidth) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setAudioBandwidthLimit(bandwidth);
}

void linphone_call_params_set_media_encryption(LinphoneCallParams *params, LinphoneMediaEncryption encryption) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setMediaEncryption(encryption);
}

void linphone_call_params_set_privacy(LinphoneCallParams *params, LinphonePrivacyMask privacy) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setPrivacy(privacy);
}

void linphone_call_params_set_record_file(LinphoneCallParams *params, const char *path) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setRecordFilePath(L_C_TO_STRING(path));
}

void linphone_call_params_set_session_name(LinphoneCallParams *params, const char *name) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setSessionName(L_C_TO_STRING(name));
}

bool_t linphone_call_params_audio_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->audioEnabled();
}

bool_t linphone_call_params_realtime_text_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->realtimeTextEnabled();
}

unsigned int linphone_call_params_get_realtime_text_keepalive_interval(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->realtimeTextKeepaliveInterval();
}

bool_t linphone_call_params_camera_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->cameraEnabled();
}

bool_t linphone_call_params_screen_sharing_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->screenSharingEnabled();
}

bool_t linphone_call_params_video_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->videoEnabled();
}

bool_t linphone_call_params_fec_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->fecEnabled();
}

LinphoneMediaDirection linphone_call_params_get_audio_direction(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getAudioDirection();
}

LinphoneMediaDirection linphone_call_params_get_video_direction(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getVideoDirection();
}

void linphone_call_params_set_audio_direction(LinphoneCallParams *params, LinphoneMediaDirection dir) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setAudioDirection(dir);
}

void linphone_call_params_set_video_direction(LinphoneCallParams *params, LinphoneMediaDirection dir) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setVideoDirection(dir);
}

void linphone_call_params_set_proxy_config(LinphoneCallParams *params, LinphoneProxyConfig *proxy_config) {
	linphone_call_params_set_account(params, proxy_config->account);
}

LinphoneProxyConfig *linphone_call_params_get_proxy_config(const LinphoneCallParams *params) {
	auto account = L_GET_CPP_PTR_FROM_C_OBJECT(params)->getAccount();
	return account != nullptr ? account->getConfig() : NULL;
}

void linphone_call_params_set_account(LinphoneCallParams *params, LinphoneAccount *account) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setAccount(
	    account ? LinphonePrivate::Account::toCpp(account)->getSharedFromThis() : nullptr);
}

LinphoneAccount *linphone_call_params_get_account(const LinphoneCallParams *params) {
	auto account = L_GET_CPP_PTR_FROM_C_OBJECT(params)->getAccount();
	return account != nullptr ? account->toC() : NULL;
}

void linphone_call_params_enable_audio_multicast(LinphoneCallParams *params, bool_t yesno) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableAudioMulticast(!!yesno);
}

bool_t linphone_call_params_audio_multicast_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->audioMulticastEnabled();
}

void linphone_call_params_enable_video_multicast(LinphoneCallParams *params, bool_t yesno) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableVideoMulticast(!!yesno);
}

bool_t linphone_call_params_video_multicast_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->videoMulticastEnabled();
}

bool_t linphone_call_params_real_early_media_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->earlyMediaSendingEnabled();
}

bool_t linphone_call_params_avpf_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->avpfEnabled();
}
bool_t linphone_call_params_has_avpf_enabled_been_set(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->hasAvpfEnabledBeenSet();
}

void linphone_call_params_enable_avpf(LinphoneCallParams *params, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableAvpf(!!enable);
}

bool_t linphone_call_params_mandatory_media_encryption_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->mandatoryMediaEncryptionEnabled();
}

void linphone_call_params_enable_mandatory_media_encryption(LinphoneCallParams *params, bool_t value) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableMandatoryMediaEncryption(!!value);
}

uint16_t linphone_call_params_get_avpf_rr_interval(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->getAvpfRrInterval();
}

void linphone_call_params_set_avpf_rr_interval(LinphoneCallParams *params, uint16_t value) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setAvpfRrInterval(value);
}

void linphone_call_params_set_sent_fps(LinphoneCallParams *params, float value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setSentFps(value);
}

void linphone_call_params_set_received_fps(LinphoneCallParams *params, float value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setReceivedFps(value);
}

// =============================================================================
// Private functions.
// =============================================================================

const char *linphone_call_params_get_conference_id(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getConferenceId();
}

void linphone_call_params_set_conference_id(LinphoneCallParams *params, const char *id) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setConferenceId(L_C_TO_STRING(id));
}

bool_t linphone_call_params_get_in_conference(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getInConference();
}

void linphone_call_params_set_in_conference(LinphoneCallParams *params, bool_t value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setInConference(!!value);
}

bool_t linphone_call_params_get_internal_call_update(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getInternalCallUpdate();
}

void linphone_call_params_set_internal_call_update(LinphoneCallParams *params, bool_t value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setInternalCallUpdate(!!value);
}

bool_t linphone_call_params_implicit_rtcp_fb_enabled(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->implicitRtcpFbEnabled();
}

void linphone_call_params_enable_implicit_rtcp_fb(LinphoneCallParams *params, bool_t value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->enableImplicitRtcpFb(!!value);
}

int linphone_call_params_get_down_bandwidth(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getDownBandwidth();
}

void linphone_call_params_set_down_bandwidth(LinphoneCallParams *params, int value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setDownBandwidth(value);
}

int linphone_call_params_get_up_bandwidth(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getUpBandwidth();
}

void linphone_call_params_set_up_bandwidth(LinphoneCallParams *params, int value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setUpBandwidth(value);
}

int linphone_call_params_get_down_ptime(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getDownPtime();
}

void linphone_call_params_set_down_ptime(LinphoneCallParams *params, int value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setDownPtime(value);
}

int linphone_call_params_get_up_ptime(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getUpPtime();
}

void linphone_call_params_set_up_ptime(LinphoneCallParams *params, int value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setUpPtime(value);
}

SalCustomHeader *linphone_call_params_get_custom_headers(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getCustomHeaders();
}

bool_t linphone_call_params_has_custom_sdp_attribute(const LinphoneCallParams *params, const char *attribute_name) {
	SalCustomSdpAttribute *csa = linphone_call_params_get_custom_sdp_attributes(params);
	return sal_custom_sdp_attribute_is_present(csa, attribute_name);
}

SalCustomSdpAttribute *linphone_call_params_get_custom_sdp_attributes(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getCustomSdpAttributes();
}

bool_t linphone_call_params_has_custom_sdp_media_attribute(const LinphoneCallParams *params,
                                                           LinphoneStreamType type,
                                                           const char *attribute_name) {
	SalCustomSdpAttribute *sdp = linphone_call_params_get_custom_sdp_media_attributes(params, type);
	return sal_custom_sdp_attribute_is_present(sdp, attribute_name);
}

SalCustomSdpAttribute *linphone_call_params_get_custom_sdp_media_attributes(const LinphoneCallParams *params,
                                                                            LinphoneStreamType type) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getCustomSdpMediaAttributes(type);
}

LinphoneCall *linphone_call_params_get_referer(const LinphoneCallParams *params) {
	shared_ptr<LinphonePrivate::CallSession> session = L_GET_PRIVATE_FROM_C_OBJECT(params)->getReferer();
	if (!session) return nullptr;
	for (const auto &call : session->getCore()->getCalls()) {
		if (call->getActiveSession() == session) return call->toC();
	}
	return nullptr;
}

void linphone_call_params_set_referer(LinphoneCallParams *params, LinphoneCall *referer) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setReferer(LinphonePrivate::Call::toCpp(referer)->getActiveSession());
}

bool_t linphone_call_params_get_update_call_when_ice_completed(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getUpdateCallWhenIceCompleted();
}

void linphone_call_params_set_update_call_when_ice_completed(LinphoneCallParams *params, bool_t value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setUpdateCallWhenIceCompleted(!!value);
}

void linphone_call_params_set_sent_vsize(LinphoneCallParams *params, MSVideoSize vsize) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setSentVideoDefinition(linphone_video_definition_new(
	    static_cast<unsigned int>(vsize.width), static_cast<unsigned int>(vsize.height), nullptr));
}

void linphone_call_params_set_recv_vsize(LinphoneCallParams *params, MSVideoSize vsize) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setReceivedVideoDefinition(linphone_video_definition_new(
	    static_cast<unsigned int>(vsize.width), static_cast<unsigned int>(vsize.height), nullptr));
}

void linphone_call_params_set_sent_video_definition(LinphoneCallParams *params, LinphoneVideoDefinition *vdef) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setSentVideoDefinition(vdef);
}

void linphone_call_params_set_received_video_definition(LinphoneCallParams *params, LinphoneVideoDefinition *vdef) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setReceivedVideoDefinition(vdef);
}

bool_t linphone_call_params_get_no_user_consent(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getNoUserConsent();
}

void linphone_call_params_set_no_user_consent(LinphoneCallParams *params, bool_t value) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setNoUserConsent(!!value);
}

time_t linphone_call_params_get_start_time(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getStartTime();
}

void linphone_call_params_set_start_time(LinphoneCallParams *params, time_t time) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setStartTime(time);
}

time_t linphone_call_params_get_end_time(const LinphoneCallParams *params) {
	return L_GET_PRIVATE_FROM_C_OBJECT(params)->getEndTime();
}

void linphone_call_params_set_end_time(LinphoneCallParams *params, time_t time) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setEndTime(time);
}

const char *linphone_call_params_get_description(const LinphoneCallParams *params) {
	return L_STRING_TO_C(L_GET_PRIVATE_FROM_C_OBJECT(params)->getDescription());
}

void linphone_call_params_set_description(LinphoneCallParams *params, const char *desc) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setDescription(L_C_TO_STRING(desc));
}

void linphone_call_params_set_conference_creation(LinphoneCallParams *params, bool_t conference_creation) {
	L_GET_PRIVATE_FROM_C_OBJECT(params)->setConferenceCreation(conference_creation);
}

bctbx_list_t *linphone_call_params_get_custom_contents(const LinphoneCallParams *params) {
	const list<std::shared_ptr<LinphonePrivate::Content>> &contents =
	    L_GET_CPP_PTR_FROM_C_OBJECT(params)->getCustomContents();
	bctbx_list_t *c_contents = nullptr;
	for (auto &content : contents) {
		LinphoneContent *c_content = content->toC();
		c_contents = bctbx_list_append(c_contents, linphone_content_ref(c_content));
	}
	return c_contents;
}

void linphone_call_params_add_custom_content(LinphoneCallParams *params, LinphoneContent *content) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContent(
	    LinphonePrivate::Content::toCpp(content)->getSharedFromThis());
}

bool_t linphone_call_params_rtp_bundle_enabled(const LinphoneCallParams *params) {
	return (bool_t)L_GET_CPP_PTR_FROM_C_OBJECT(params)->rtpBundleEnabled();
}

void linphone_call_params_enable_rtp_bundle(LinphoneCallParams *params, bool_t value) {
	lError() << "linphone_call_params_enable_rtp_bundle(): is no longer supported. Use "
	            "linphone_core_enable_rtp_bundle() or linphone_account_params_enable_rtp_bundle().";
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableRtpBundle(!!value);
}

bool_t linphone_call_params_is_recording(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->isRecording();
}

void linphone_call_params_enable_mic(LinphoneCallParams *params, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->enableMic(!!enable);
}

bool_t linphone_call_params_mic_enabled(const LinphoneCallParams *params) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(params)->isMicEnabled();
}

void linphone_call_params_set_input_audio_device(LinphoneCallParams *params, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(params)->setInputAudioDevice(
		    LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

void linphone_call_params_set_output_audio_device(LinphoneCallParams *params, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		L_GET_CPP_PTR_FROM_C_OBJECT(params)->setOutputAudioDevice(
		    LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

const LinphoneAudioDevice *linphone_call_params_get_input_audio_device(const LinphoneCallParams *params) {
	auto audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(params)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

const LinphoneAudioDevice *linphone_call_params_get_output_audio_device(const LinphoneCallParams *params) {
	auto audioDevice = L_GET_CPP_PTR_FROM_C_OBJECT(params)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

void linphone_call_params_set_conference_video_layout(LinphoneCallParams *params, LinphoneConferenceLayout layout) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->setConferenceVideoLayout((LinphonePrivate::ConferenceLayout)layout);
}

LinphoneConferenceLayout linphone_call_params_get_conference_video_layout(const LinphoneCallParams *params) {
	return (LinphoneConferenceLayout)L_GET_CPP_PTR_FROM_C_OBJECT(params)->getConferenceVideoLayout();
}

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

void *linphone_call_params_get_user_data(const LinphoneCallParams *cp) {
	return L_GET_USER_DATA_FROM_C_OBJECT(cp);
}

void linphone_call_params_set_user_data(LinphoneCallParams *cp, void *ud) {
	L_SET_USER_DATA_FROM_C_OBJECT(cp, ud);
}

LinphoneCallParams *linphone_call_params_ref(LinphoneCallParams *cp) {
	belle_sip_object_ref(cp);
	return cp;
}

void linphone_call_params_unref(LinphoneCallParams *cp) {
	belle_sip_object_unref(cp);
}

// =============================================================================
// Constructor and destructor functions.
// =============================================================================

LinphoneCallParams *linphone_call_params_new(LinphoneCore *core) {
	LinphoneCallParams *params = L_INIT(CallParams);
	L_SET_CPP_PTR_FROM_C_OBJECT(params, new LinphonePrivate::MediaSessionParams());
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->initDefault(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphoneCallOutgoing);
	return params;
}

LinphoneCallParams *linphone_call_params_new_with_media_session_params(LinphonePrivate::MediaSessionParams *msp) {
	LinphoneCallParams *params = L_INIT(CallParams);
	L_SET_CPP_PTR_FROM_C_OBJECT(params, msp);
	return params;
}

LinphoneCallParams *linphone_call_params_new_for_wrapper(void) {
	return _linphone_CallParams_init();
}

/* DEPRECATED */
void linphone_call_params_destroy(LinphoneCallParams *cp) {
	linphone_call_params_unref(cp);
}
