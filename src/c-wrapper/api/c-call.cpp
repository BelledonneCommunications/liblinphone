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

#include "linphone/api/c-call.h"
#include "c-wrapper/c-wrapper.h"
#include "call/audio-device/audio-device.h"
#include "call/call.h"
#include "conference/conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/session/ms2-streams.h"
#include "core/core-p.h"
#include "linphone/api/c-call-cbs.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-call-stats.h"
#include "linphone/wrapper_utils.h"
#include "player/call-player.h"
#include "private_functions.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

/*This function is not static because used internally in linphone-daemon project*/
void _post_configure_audio_stream(AudioStream *st, LinphoneCore *lc, bool_t muted) {
	MS2AudioStream::postConfigureAudioStream(st, lc, !!muted);
}

// =============================================================================
// Private functions.
// =============================================================================

void _linphone_call_set_conf_ref(LinphoneCall *call, LinphoneConference *ref) {
	Call::toCpp(call)->setConference(Conference::toCpp(ref)->getSharedFromThis());
}

MSAudioEndpoint *_linphone_call_get_endpoint(const LinphoneCall *call) {
	return Call::toCpp(call)->getEndpoint();
}

void _linphone_call_set_endpoint(LinphoneCall *call, MSAudioEndpoint *endpoint) {
	Call::toCpp(call)->setEndpoint(endpoint);
}

MediaStream *linphone_call_get_stream(LinphoneCall *call, LinphoneStreamType type) {
	return Call::toCpp(call)->getMediaStream(type);
}

LinphonePrivate::SalCallOp *linphone_call_get_op(const LinphoneCall *call) {
	return Call::toCpp(call)->getOp();
}

const char *linphone_call_get_local_tag(const LinphoneCall *call) {
	return sal_call_get_local_tag(linphone_call_get_op(call));
}

const char *linphone_call_get_remote_tag(const LinphoneCall *call) {
	return sal_call_get_remote_tag(linphone_call_get_op(call));
}

LinphoneAccount *linphone_call_get_dest_account(const LinphoneCall *call) {
	return Call::toCpp(call)->getDestAccount()->toC();
}

IceSession *linphone_call_get_ice_session(const LinphoneCall *call) {
	return Call::toCpp(call)->getIceSession();
}

bool_t linphone_call_get_all_muted(const LinphoneCall *call) {
	return Call::toCpp(call)->getAllMuted();
}

void linphone_call_set_ekt(const LinphoneCall *call, const MSEKTParametersSet *ekt_params) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setEkt(ekt_params);
}

void linphone_call_notify_state_changed(LinphoneCall *call, LinphoneCallState cstate, const char *message) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_state_changed, cstate, message);
	linphone_core_notify_call_state_changed(linphone_call_get_core(call), call, cstate, message);
}

void linphone_call_notify_dtmf_received(LinphoneCall *call, int dtmf) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_dtmf_received, dtmf);
	linphone_core_notify_dtmf_received(linphone_call_get_core(call), call, dtmf);
}

void linphone_call_notify_goclear_ack_sent(LinphoneCall *call) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Call, Call::toCpp(call), linphone_call_cbs_get_goclear_ack_sent);
	linphone_core_notify_call_goclear_ack_sent(linphone_call_get_core(call), call);
}

void linphone_call_notify_security_level_downgraded(LinphoneCall *call) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Call, Call::toCpp(call), linphone_call_cbs_get_security_level_downgraded);
}

void linphone_call_notify_encryption_changed(LinphoneCall *call, bool_t on, const char *authentication_token) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_encryption_changed, on,
	                                  authentication_token);
	linphone_core_notify_call_encryption_changed(linphone_call_get_core(call), call, on, authentication_token);
}

void linphone_call_notify_authentication_token_verified(LinphoneCall *call, bool_t verified) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_authentication_token_verified,
	                                  verified);
};

void linphone_call_notify_send_master_key_changed(LinphoneCall *call, const char *master_key) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_send_master_key_changed,
	                                  master_key);
	linphone_core_notify_call_send_master_key_changed(linphone_call_get_core(call), call, master_key);
}

void linphone_call_notify_receive_master_key_changed(LinphoneCall *call, const char *master_key) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_receive_master_key_changed,
	                                  master_key);
	linphone_core_notify_call_receive_master_key_changed(linphone_call_get_core(call), call, master_key);
}

void linphone_call_notify_transfer_state_changed(LinphoneCall *call, LinphoneCallState cstate) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_transfer_state_changed, cstate);
	linphone_core_notify_transfer_state_changed(linphone_call_get_core(call), call, cstate);
}

void linphone_call_notify_refer_requested(LinphoneCall *call, const LinphoneAddress *refer_to) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_refer_requested, refer_to);
}

void linphone_call_notify_stats_updated(LinphoneCall *call, const LinphoneCallStats *stats) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_stats_updated, stats);
	linphone_core_notify_call_stats_updated(linphone_call_get_core(call), call, stats);
}

void linphone_call_notify_info_message_received(LinphoneCall *call, const LinphoneInfoMessage *msg) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_info_message_received, msg);
	linphone_core_notify_info_received(linphone_call_get_core(call), call, msg);
}

void linphone_call_notify_ack_processing(LinphoneCall *call, LinphoneHeaders *msg, bool_t is_received) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_ack_processing, msg, is_received);
}

void linphone_call_notify_tmmbr_received(LinphoneCall *call, int stream_index, int tmmbr) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_tmmbr_received, stream_index,
	                                  tmmbr);
}

void linphone_call_notify_snapshot_taken(LinphoneCall *call, const char *file_path) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_snapshot_taken, file_path);
}

void linphone_call_notify_next_video_frame_decoded(LinphoneCall *call) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Call, Call::toCpp(call), linphone_call_cbs_get_next_video_frame_decoded);
}

void linphone_call_notify_camera_not_working(LinphoneCall *call, const char *camera_name) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_camera_not_working, camera_name);
}

void linphone_call_notify_video_display_error_occurred(LinphoneCall *call, int error_code) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_video_display_error_occurred,
	                                  error_code);
}

void linphone_call_notify_audio_device_changed(LinphoneCall *call, LinphoneAudioDevice *audioDevice) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_audio_device_changed, audioDevice);
	linphone_core_notify_audio_device_changed(linphone_call_get_core(call), audioDevice);
}

void linphone_call_notify_remote_recording(LinphoneCall *call, bool_t recording) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_remote_recording, recording);
}

void linphone_call_notify_baudot_detected(LinphoneCall *call, LinphoneBaudotStandard standard) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Call, Call::toCpp(call), linphone_call_cbs_get_baudot_detected, standard);
}

// =============================================================================
// Public functions.
// =============================================================================

LinphoneCore *linphone_call_get_core(const LinphoneCall *call) {
	return Call::toCpp(call)->getCore()->getCCore();
}

LinphoneCallState linphone_call_get_state(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return static_cast<LinphoneCallState>(Call::toCpp(call)->getState());
}

bool_t linphone_call_asked_to_autoanswer(LinphoneCall *call) {
	// return TRUE if the unique(for the moment) incoming call asked to be autoanswered
	if (call) return linphone_call_get_op(call)->autoAnswerAsked();
	return FALSE;
}

const LinphoneAddress *linphone_call_get_remote_address(const LinphoneCall *call) {
	return Call::toCpp(call)->getRemoteAddress()->toC();
}

const LinphoneAddress *linphone_call_get_to_address(const LinphoneCall *call) {
	return Call::toCpp(call)->getToAddress()->toC();
}

const LinphoneAddress *linphone_call_get_request_address(const LinphoneCall *call) {
	return Call::toCpp(call)->getActiveSession()->getRequestAddress()->toC();
}

const char *linphone_call_get_to_header(const LinphoneCall *call, const char *name) {
	return Call::toCpp(call)->getToHeader(name);
}

char *linphone_call_get_remote_address_as_string(const LinphoneCall *call) {
	return Call::toCpp(call)->getRemoteAddress() ? ms_strdup(Call::toCpp(call)->getRemoteAddress()->asString().c_str())
	                                             : nullptr;
}

const LinphoneAddress *linphone_call_get_diversion_address(const LinphoneCall *call) {
	auto diversionAddress = Call::toCpp(call)->getDiversionAddress();
	return diversionAddress && diversionAddress->isValid() ? diversionAddress->toC() : nullptr;
}

LinphoneCallDir linphone_call_get_dir(const LinphoneCall *call) {
	return linphone_call_log_get_dir(linphone_call_get_call_log(call));
}

LinphoneCallLog *linphone_call_get_call_log(const LinphoneCall *call) {
	return Call::toCpp(call)->getLog()->toC();
}

const char *linphone_call_get_refer_to(const LinphoneCall *call) {
	return L_STRING_TO_C(Call::toCpp(call)->getReferTo());
}

const LinphoneAddress *linphone_call_get_refer_to_address(const LinphoneCall *call) {
	const auto &referToAddress = Call::toCpp(call)->getReferToAddress();
	return referToAddress && referToAddress->isValid() ? referToAddress->toC() : nullptr;
}

const LinphoneAddress *linphone_call_get_referred_by_address(const LinphoneCall *call) {
	auto addr = Call::toCpp(call)->getReferredBy();
	return toC(addr);
}

bool_t linphone_call_has_transfer_pending(const LinphoneCall *call) {
	return Call::toCpp(call)->hasTransferPending();
}

LinphoneCall *linphone_call_get_transferer_call(const LinphoneCall *call) {
	shared_ptr<Call> referer = Call::toCpp(call)->getReferer();
	if (!referer) return nullptr;
	return referer->toC();
}

LinphoneCall *linphone_call_get_transfer_target_call(const LinphoneCall *call) {
	shared_ptr<Call> transferTarget = Call::toCpp(call)->getTransferTarget();
	if (!transferTarget) return nullptr;
	return transferTarget->toC();
}

LinphoneCall *linphone_call_get_replaced_call(LinphoneCall *call) {
	shared_ptr<Call> replacedCall = Call::toCpp(call)->getReplacedCall();
	if (!replacedCall) return nullptr;
	return replacedCall->toC();
}

int linphone_call_get_duration(const LinphoneCall *call) {
	return Call::toCpp(call)->getDuration();
}

const LinphoneCallParams *linphone_call_get_current_params(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return L_GET_C_BACK_PTR(Call::toCpp(call)->getCurrentParams());
}

const LinphoneCallParams *linphone_call_get_remote_params(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	const LinphonePrivate::MediaSessionParams *remoteParams = Call::toCpp(call)->getRemoteParams();
	return remoteParams ? L_GET_C_BACK_PTR(remoteParams) : nullptr;
}

void linphone_call_enable_camera(LinphoneCall *call, bool_t enable) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->enableCamera(!!enable);
}

bool_t linphone_call_camera_enabled(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->cameraEnabled();
}

LinphoneStatus linphone_call_take_video_snapshot(LinphoneCall *call, const char *file) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->takeVideoSnapshot(L_C_TO_STRING(file));
}

LinphoneStatus linphone_call_take_preview_snapshot(LinphoneCall *call, const char *file) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->takePreviewSnapshot(L_C_TO_STRING(file));
}

LinphoneReason linphone_call_get_reason(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getReason();
}

const LinphoneErrorInfo *linphone_call_get_error_info(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getErrorInfo();
}

const char *linphone_call_get_remote_user_agent(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return L_STRING_TO_C(Call::toCpp(call)->getRemoteUserAgent());
}

const LinphoneAddress *linphone_call_get_remote_contact_address(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	const auto &remoteContactAddress = Call::toCpp(call)->getRemoteContactAddress();
	return remoteContactAddress ? remoteContactAddress->toC() : nullptr;
}

const char *linphone_call_get_remote_contact(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return L_STRING_TO_C(Call::toCpp(call)->getRemoteContact());
}

const char *linphone_call_get_authentication_token(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return L_STRING_TO_C(Call::toCpp(call)->getAuthenticationToken());
}

const char *linphone_call_get_local_authentication_token(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return L_STRING_TO_C(Call::toCpp(call)->forgeLocalAuthenticationToken());
}

const bctbx_list_t *linphone_call_get_remote_authentication_tokens(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	const Call *callCpp = Call::toCpp(call);
	auto remoteTokens = callCpp->getRemoteAuthenticationTokens();
	if (remoteTokens.size() == 3) {
		callCpp->storeAndSortRemoteAuthToken(callCpp->forgeRemoteAuthenticationToken());
	}
	return callCpp->getCListRemoteAuthenticationTokens();
}

bool_t linphone_call_get_authentication_token_verified(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getAuthenticationTokenVerified();
}

void linphone_call_skip_zrtp_authentication(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->skipZrtpAuthentication();
}

void linphone_call_check_authentication_token_selected(LinphoneCall *call, const char *selected_value) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->checkAuthenticationTokenSelected(selected_value);
}

void linphone_call_set_authentication_token_verified(LinphoneCall *call, bool_t verified) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setAuthenticationTokenVerified(!!verified);
}

bool_t linphone_call_get_zrtp_cache_mismatch_flag(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getZrtpCacheMismatch();
}

void linphone_call_send_vfu_request(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->sendVfuRequest();
}

void linphone_call_set_next_video_frame_decoded_callback(LinphoneCall *call, LinphoneCallCbFunc cb, void *ud) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setNextVideoFrameDecodedCallback(cb, ud);
}

void linphone_call_request_notify_next_video_frame_decoded(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->requestNotifyNextVideoFrameDecoded();
}

LinphoneCallState linphone_call_get_transfer_state(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return static_cast<LinphoneCallState>(Call::toCpp(call)->getTransferState());
}

void linphone_call_zoom_video(LinphoneCall *call, float zoom_factor, float *cx, float *cy) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->zoomVideo(zoom_factor, cx, cy);
}

void linphone_call_zoom(LinphoneCall *call, float zoom_factor, float cx, float cy) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->zoomVideo(zoom_factor, cx, cy);
}

LinphoneStatus linphone_call_send_dtmf(LinphoneCall *call, char dtmf) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->sendDtmf(dtmf);
}

LinphoneStatus linphone_call_send_dtmfs(LinphoneCall *call, const char *dtmfs) {
	CallLogContextualizer logContextualizer(call);
	if (!call) {
		ms_warning("linphone_call_send_dtmfs(): invalid call, canceling DTMF sequence");
		return -1;
	}
	return Call::toCpp(call)->sendDtmfs(dtmfs);
}

void linphone_call_cancel_dtmfs(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	if (!call) return;
	Call::toCpp(call)->cancelDtmfs();
}

bool_t linphone_call_is_in_conference(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return !!Call::toCpp(call)->isInConference();
}

LinphoneConference *linphone_call_get_conference(const LinphoneCall *call) {
	auto conference = Call::toCpp(call)->getConference();
	return conference ? conference->toC() : NULL;
}

int linphone_call_get_stream_count(const LinphoneCall *call) {
	return Call::toCpp(call)->getStreamCount();
}

MSFormatType linphone_call_get_stream_type(const LinphoneCall *call, int stream_index) {
	return Call::toCpp(call)->getStreamType(stream_index);
}

RtpTransport *linphone_call_get_meta_rtp_transport(const LinphoneCall *call, int stream_index) {
	return Call::toCpp(call)->getMetaRtpTransport(stream_index);
}

RtpTransport *linphone_call_get_meta_rtcp_transport(const LinphoneCall *call, int stream_index) {
	return Call::toCpp(call)->getMetaRtcpTransport(stream_index);
}

LinphoneStatus linphone_call_pause(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->pause();
}

LinphoneStatus linphone_call_resume(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->resume();
}

LinphoneStatus linphone_call_terminate(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->terminate();
}

LinphoneStatus linphone_call_terminate_with_error_info(LinphoneCall *call, const LinphoneErrorInfo *ei) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->terminate(ei);
}

LinphoneStatus linphone_call_redirect(LinphoneCall *call, const char *redirect_uri) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->redirect(redirect_uri);
}

LinphoneStatus linphone_call_redirect_to(LinphoneCall *call, LinphoneAddress *redirect_address) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->redirect(Address::toCpp(redirect_address)->getSharedFromThis());
}

LinphoneStatus linphone_call_decline(LinphoneCall *call, LinphoneReason reason) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->decline(reason);
}

LinphoneStatus linphone_call_decline_with_error_info(LinphoneCall *call, const LinphoneErrorInfo *ei) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->decline(ei);
}

LinphoneStatus linphone_call_accept(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->accept(nullptr);
}

LinphoneStatus linphone_call_accept_with_params(LinphoneCall *call, const LinphoneCallParams *params) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->accept(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_accept_early_media(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->acceptEarlyMedia();
}

LinphoneStatus linphone_call_accept_early_media_with_params(LinphoneCall *call, const LinphoneCallParams *params) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->acceptEarlyMedia(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_update(LinphoneCall *call, const LinphoneCallParams *params) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->update(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_defer_update(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->deferUpdate();
}

LinphoneStatus linphone_call_accept_update(LinphoneCall *call, const LinphoneCallParams *params) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->acceptUpdate(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

void linphone_call_accept_transfer(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->executeTransfer();
}

LinphoneStatus linphone_call_transfer(LinphoneCall *call, const char *referTo) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->transfer(referTo);
}

LinphoneStatus linphone_call_transfer_to(LinphoneCall *call, const LinphoneAddress *referTo) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->transfer(*Address::toCpp(referTo)->getSharedFromThis());
}

LinphoneStatus linphone_call_transfer_to_another(LinphoneCall *call, LinphoneCall *dest) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->transfer(Call::toCpp(dest)->getSharedFromThis());
}

void *linphone_call_get_native_video_window_id(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getNativeVideoWindowId();
}

void *linphone_call_create_native_video_window_id(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->createNativeVideoWindowId();
}

void linphone_call_set_native_video_window_id(LinphoneCall *call, void *id) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setNativeVideoWindowId(id);
}

void linphone_call_enable_echo_cancellation(LinphoneCall *call, bool_t enable) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->enableEchoCancellation(!!enable);
}

bool_t linphone_call_echo_cancellation_enabled(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->echoCancellationEnabled();
}

void linphone_call_enable_echo_limiter(LinphoneCall *call, bool_t val) {
	Call::toCpp(call)->enableEchoLimiter(!!val);
}

bool_t linphone_call_echo_limiter_enabled(const LinphoneCall *call) {
	return Call::toCpp(call)->echoLimiterEnabled();
}

LinphoneChatRoom *linphone_call_get_chat_room(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	shared_ptr<LinphonePrivate::AbstractChatRoom> acr = Call::toCpp(call)->getChatRoom();
	if (acr) return acr->toC();
	return nullptr;
}

float linphone_call_get_play_volume(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getPlayVolume();
}

float linphone_call_get_record_volume(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getRecordVolume();
}

float linphone_call_get_speaker_volume_gain(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getSpeakerVolumeGain();
}

void linphone_call_set_speaker_volume_gain(LinphoneCall *call, float volume) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setSpeakerVolumeGain(volume);
}

float linphone_call_get_microphone_volume_gain(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getMicrophoneVolumeGain();
}

void linphone_call_set_microphone_volume_gain(LinphoneCall *call, float volume) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setMicrophoneVolumeGain(volume);
}

bool_t linphone_call_get_speaker_muted(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getSpeakerMuted();
}

void linphone_call_set_speaker_muted(LinphoneCall *call, bool_t muted) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setSpeakerMuted(!!muted);
}

bool_t linphone_call_get_microphone_muted(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getMicrophoneMuted();
}

void linphone_call_set_microphone_muted(LinphoneCall *call, bool_t muted) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setMicrophoneMuted(!!muted);
}

float linphone_call_get_current_quality(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getCurrentQuality();
}

float linphone_call_get_average_quality(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getAverageQuality();
}

void linphone_call_start_recording(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->startRecording();
}

void linphone_call_stop_recording(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->stopRecording();
}

bool_t linphone_call_is_recording(LinphoneCall *call) {
	return Call::toCpp(call)->isRecording();
}

LinphonePlayer *linphone_call_get_player(LinphoneCall *call) {
	std::shared_ptr<Player> player = Call::toCpp(call)->getPlayer();
	return player ? player->toC() : nullptr;
}

bool_t linphone_call_media_in_progress(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->mediaInProgress();
}

void linphone_call_ogl_render(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->oglRender();
}

LinphoneStatus linphone_call_send_info_message(LinphoneCall *call, const LinphoneInfoMessage *info) {
	CallLogContextualizer logContextualizer(call);
	SalBodyHandler *body_handler = sal_body_handler_from_content(linphone_info_message_get_content(info));
	linphone_call_get_op(call)->setSentCustomHeaders(linphone_info_message_get_headers(info));
	return linphone_call_get_op(call)->sendInfo(body_handler);
}

LinphoneCallStats *linphone_call_get_stats(LinphoneCall *call, LinphoneStreamType type) {
	auto stats = Call::toCpp(call)->getStats(type);
	if (stats) {
		stats->ref();
		return stats->toC();
	}
	return NULL;
}

LinphoneCallStats *linphone_call_get_audio_stats(LinphoneCall *call) {
	auto stats = Call::toCpp(call)->getAudioStats();
	if (stats) {
		stats->ref();
		return stats->toC();
	}
	return NULL;
}

LinphoneCallStats *linphone_call_get_video_stats(LinphoneCall *call) {
	auto stats = Call::toCpp(call)->getVideoStats();
	if (stats) {
		stats->ref();
		return stats->toC();
	}
	return NULL;
}

LinphoneCallStats *linphone_call_get_text_stats(LinphoneCall *call) {
	auto stats = Call::toCpp(call)->getTextStats();
	if (stats) {
		stats->ref();
		return stats->toC();
	}
	return NULL;
}

void linphone_call_add_callbacks(LinphoneCall *call, LinphoneCallCbs *cbs) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->addCallbacks(CallCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_call_remove_callbacks(LinphoneCall *call, LinphoneCallCbs *cbs) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->removeCallbacks(CallCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneCallCbs *linphone_call_get_current_callbacks(const LinphoneCall *call) {
	return Call::toCpp(call)->getCurrentCallbacks()->toC();
}

const bctbx_list_t *linphone_call_get_callbacks_list(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return Call::toCpp(call)->getCCallbacksList();
}

void linphone_call_set_params(LinphoneCall *call, const LinphoneCallParams *params) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setParams(L_GET_CPP_PTR_FROM_C_OBJECT(params));
}

const LinphoneCallParams *linphone_call_get_params(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	return L_GET_C_BACK_PTR(Call::toCpp(call)->getParams());
}

void linphone_call_enable_baudot_detection(LinphoneCall *call, bool_t enabled) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->enableBaudotDetection(!!enabled);
}

void linphone_call_set_baudot_mode(LinphoneCall *call, LinphoneBaudotMode mode) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setBaudotMode(mode);
}

void linphone_call_set_baudot_sending_standard(LinphoneCall *call, LinphoneBaudotStandard standard) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setBaudotSendingStandard(standard);
}

void linphone_call_set_baudot_pause_timeout(LinphoneCall *call, uint8_t seconds) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setBaudotPauseTimeout(seconds);
}

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneCall *linphone_call_ref(LinphoneCall *call) {
	Call::toCpp(call)->ref();
	return call;
}

void linphone_call_unref(LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->unref();
}

void *linphone_call_get_user_data(const LinphoneCall *call) {
	return Call::toCpp(call)->getUserData();
}

void linphone_call_set_user_data(LinphoneCall *call, void *ud) {
	Call::toCpp(call)->setUserData(ud);
}

// =============================================================================
// Constructor and destructor functions.
// =============================================================================

LinphoneCall *linphone_call_new_outgoing(LinphoneCore *lc,
                                         const LinphoneAddress *from,
                                         const LinphoneAddress *to,
                                         const LinphoneCallParams *params,
                                         LinphoneAccount *account) {
	LinphoneCall *lcall = Call::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
	Call::toCpp(lcall)->configure(
	    LinphoneCallOutgoing, Address::toCpp(from)->getSharedFromThis(), Address::toCpp(to)->getSharedFromThis(),
	    account ? Account::toCpp(account)->getSharedFromThis() : nullptr, nullptr, L_GET_CPP_PTR_FROM_C_OBJECT(params));
	return lcall;
}

LinphoneCall *linphone_call_new_incoming(LinphoneCore *lc,
                                         const LinphoneAddress *from,
                                         const LinphoneAddress *to,
                                         LinphonePrivate::SalCallOp *op) {
	LinphoneCall *lcall = Call::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
	Call::toCpp(lcall)->configure(LinphoneCallIncoming, Address::toCpp(from)->getSharedFromThis(),
	                              Address::toCpp(to)->getSharedFromThis(), nullptr, op, nullptr);
	Call::toCpp(lcall)->initiateIncoming();
	return lcall;
}

LinphoneCall *linphone_call_new_incoming_with_callid(LinphoneCore *lc, const char *callid) {
	LinphoneCall *lcall = Call::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
	Call::toCpp(lcall)->configure(LinphoneCallIncoming, callid);
	return lcall;
}

void linphone_call_notify_ringing(LinphoneCall *call) {
	Call::toCpp(call)->notifyRinging();
}

void linphone_call_start_push_incoming_notification(LinphoneCall *call) {
	Call::toCpp(call)->startPushIncomingNotification();
}

void linphone_call_start_basic_incoming_notification(LinphoneCall *call) {
	Call::toCpp(call)->startBasicIncomingNotification();
}

bool_t linphone_call_is_op_configured(const LinphoneCall *call) {
	return Call::toCpp(call)->isOpConfigured();
}

void linphone_call_set_input_audio_device(LinphoneCall *call, LinphoneAudioDevice *audio_device) {
	CallLogContextualizer logContextualizer(call);
	if (audio_device) {
		Call::toCpp(call)->setInputAudioDevice(LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

void linphone_call_set_output_audio_device(LinphoneCall *call, LinphoneAudioDevice *audio_device) {
	CallLogContextualizer logContextualizer(call);
	if (audio_device) {
		Call::toCpp(call)->setOutputAudioDevice(LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

const LinphoneAudioDevice *linphone_call_get_input_audio_device(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	auto audioDevice = Call::toCpp(call)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}
const LinphoneAudioDevice *linphone_call_get_output_audio_device(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	auto audioDevice = Call::toCpp(call)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

void linphone_call_set_video_source(LinphoneCall *call, const LinphoneVideoSourceDescriptor *descriptor) {
	CallLogContextualizer logContextualizer(call);
	Call::toCpp(call)->setVideoSource(descriptor ? VideoSourceDescriptor::toCpp(descriptor)->getSharedFromThis()
	                                             : nullptr);
}

const LinphoneVideoSourceDescriptor *linphone_call_get_video_source(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
	auto descriptor = Call::toCpp(call)->getVideoSource();
	return descriptor == nullptr ? NULL : descriptor->toC();
}

void linphone_call_confirm_go_clear(const LinphoneCall *call) {
	CallLogContextualizer logContextualizer(call);
#ifdef HAVE_GOCLEAR
	Call::toCpp(call)->confirmGoClear();
#else // HAVE_GOCLEAR
	(void)call;
#endif
}

LinphoneEvent *linphone_call_create_notify(LinphoneCall *call, const char *event) {
	CallLogContextualizer logContextualizer(call);
	auto ev = Call::toCpp(call)->createNotify(event);
	if (ev) ev->ref();
	return toC(ev);
}
