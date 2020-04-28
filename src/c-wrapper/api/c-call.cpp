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

#include "linphone/api/c-call.h"
#include "linphone/api/c-call-cbs.h"
#include "linphone/api/c-call-stats.h"
#include "linphone/wrapper_utils.h"

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-room/real-time-text-chat-room.h"
#include "conference/params/media-session-params-p.h"
#include "conference/session/ms2-streams.h"
#include "core/core-p.h"
#include "call/audio-device/audio-device.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;


// =============================================================================
// TODO: To remove!
// =============================================================================

void linphone_call_init_media_streams (LinphoneCall *call) {
	Call::toCpp(call)->initializeMediaStreams();
}

/*This function is not static because used internally in linphone-daemon project*/
void _post_configure_audio_stream (AudioStream *st, LinphoneCore *lc, bool_t muted) {
	MS2AudioStream::postConfigureAudioStream(st, lc, !!muted);
}

void linphone_call_stop_media_streams (LinphoneCall *call) {
	Call::toCpp(call)->stopMediaStreams();
}

/* Internal version that does not play tone indication*/
int _linphone_call_pause (LinphoneCall *call) {
	return Call::toCpp(call)->pause();
}


// =============================================================================
// Private functions.
// =============================================================================

void _linphone_call_set_conf_ref (LinphoneCall *call, LinphoneConference *ref) {
	Call::toCpp(call)->setConference(ref);
}

MSAudioEndpoint *_linphone_call_get_endpoint (const LinphoneCall *call) {
	return Call::toCpp(call)->getEndpoint();
}

void _linphone_call_set_endpoint (LinphoneCall *call, MSAudioEndpoint *endpoint) {
	Call::toCpp(call)->setEndpoint(endpoint);
}

MediaStream *linphone_call_get_stream (LinphoneCall *call, LinphoneStreamType type) {
	return Call::toCpp(call)->getMediaStream(type);
}

LinphonePrivate::SalCallOp * linphone_call_get_op (const LinphoneCall *call) {
	return Call::toCpp(call)->getOp();
}

LinphoneProxyConfig * linphone_call_get_dest_proxy (const LinphoneCall *call) {
	return Call::toCpp(call)->getDestProxy();
}

IceSession * linphone_call_get_ice_session (const LinphoneCall *call) {
	return Call::toCpp(call)->getIceSession();
}

bool_t linphone_call_get_all_muted (const LinphoneCall *call) {
	return Call::toCpp(call)->getAllMuted();
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...) \
for (bctbx_list_t *it = Call::toCpp(call)->getCallbacksList(); it; it = bctbx_list_next(it)) { \
	Call::toCpp(call)->setCurrentCbs(reinterpret_cast<LinphoneCallCbs *>(bctbx_list_get_data(it))); \
	LinphoneCallCbs ## cbName ## Cb cb = linphone_call_cbs_get_ ## functionName (Call::toCpp(call)->getCurrentCbs()); \
	if (cb) \
		cb(__VA_ARGS__); \
}

void linphone_call_notify_state_changed (LinphoneCall *call, LinphoneCallState cstate, const char *message) {
	NOTIFY_IF_EXIST(StateChanged, state_changed, call, cstate, message)
	linphone_core_notify_call_state_changed(linphone_call_get_core(call), call, cstate, message);
}

void linphone_call_notify_dtmf_received (LinphoneCall *call, int dtmf) {
	NOTIFY_IF_EXIST(DtmfReceived, dtmf_received, call, dtmf)
	linphone_core_notify_dtmf_received(linphone_call_get_core(call), call, dtmf);
}

void linphone_call_notify_encryption_changed (LinphoneCall *call, bool_t on, const char *authentication_token) {
	NOTIFY_IF_EXIST(EncryptionChanged, encryption_changed, call, on, authentication_token)
	linphone_core_notify_call_encryption_changed(linphone_call_get_core(call), call, on, authentication_token);
}

void linphone_call_notify_transfer_state_changed (LinphoneCall *call, LinphoneCallState cstate) {
	NOTIFY_IF_EXIST(TransferStateChanged, transfer_state_changed, call, cstate)
	linphone_core_notify_transfer_state_changed(linphone_call_get_core(call), call, cstate);
}

void linphone_call_notify_stats_updated (LinphoneCall *call, const LinphoneCallStats *stats) {
	NOTIFY_IF_EXIST(StatsUpdated, stats_updated, call, stats)
	linphone_core_notify_call_stats_updated(linphone_call_get_core(call), call, stats);
}

void linphone_call_notify_info_message_received (LinphoneCall *call, const LinphoneInfoMessage *msg) {
	NOTIFY_IF_EXIST(InfoMessageReceived, info_message_received, call, msg)
	linphone_core_notify_info_received(linphone_call_get_core(call), call, msg);
}

void linphone_call_notify_ack_processing (LinphoneCall *call, LinphoneHeaders *msg, bool_t is_received) {
	NOTIFY_IF_EXIST(AckProcessing, ack_processing, call, msg, is_received)
}

void linphone_call_notify_tmmbr_received (LinphoneCall *call, int stream_index, int tmmbr) {
	NOTIFY_IF_EXIST(TmmbrReceived, tmmbr_received, call, stream_index, tmmbr)
}

void linphone_call_notify_snapshot_taken(LinphoneCall *call, const char *file_path) {
	NOTIFY_IF_EXIST(SnapshotTaken, snapshot_taken, call, file_path)
}

void linphone_call_notify_next_video_frame_decoded(LinphoneCall *call) {
	NOTIFY_IF_EXIST(NextVideoFrameDecoded, next_video_frame_decoded, call)
}

void linphone_call_notify_camera_not_working(LinphoneCall *call, const char *camera_name) {
	NOTIFY_IF_EXIST(CameraNotWorking, camera_not_working, call, camera_name);
}

void linphone_call_notify_audio_device_changed(LinphoneCall *call, LinphoneAudioDevice *audioDevice) {
	NOTIFY_IF_EXIST(AudioDeviceChanged, audio_device_changed, call, audioDevice);
}

// =============================================================================
// Public functions.
// =============================================================================

LinphoneCore *linphone_call_get_core (const LinphoneCall *call) {
	return Call::toCpp(call)->getCore()->getCCore();
}

LinphoneCallState linphone_call_get_state (const LinphoneCall *call) {
	return static_cast<LinphoneCallState>(Call::toCpp(call)->getState());
}

bool_t linphone_call_asked_to_autoanswer (LinphoneCall *call) {
	//return TRUE if the unique(for the moment) incoming call asked to be autoanswered
	if (call)
		return linphone_call_get_op(call)->autoAnswerAsked();
	return FALSE;
}

const LinphoneAddress *linphone_call_get_remote_address (const LinphoneCall *call) {
	return L_GET_C_BACK_PTR(&Call::toCpp(call)->getRemoteAddress());
}

const LinphoneAddress *linphone_call_get_to_address (const LinphoneCall *call) {
	return L_GET_C_BACK_PTR(&Call::toCpp(call)->getToAddress());
}

const char *linphone_call_get_to_header (LinphoneCall *call, const char *name) {
	return Call::toCpp(call)->getToHeader(name).c_str();
}

char *linphone_call_get_remote_address_as_string (const LinphoneCall *call) {
	return ms_strdup(Call::toCpp(call)->getRemoteAddress().asString().c_str());
}

const LinphoneAddress *linphone_call_get_diversion_address (const LinphoneCall *call) {
	const LinphonePrivate::Address &diversionAddress = Call::toCpp(call)->getDiversionAddress();
	return diversionAddress.isValid() ? L_GET_C_BACK_PTR(&diversionAddress) : nullptr;
}

LinphoneCallDir linphone_call_get_dir (const LinphoneCall *call) {
	return linphone_call_log_get_dir(linphone_call_get_call_log(call));
}

LinphoneCallLog *linphone_call_get_call_log (const LinphoneCall *call) {
	return Call::toCpp(call)->getLog();
}

const char *linphone_call_get_refer_to (LinphoneCall *call) {
	return Call::toCpp(call)->getReferTo().c_str();
}

bool_t linphone_call_has_transfer_pending (const LinphoneCall *call) {
	return Call::toCpp(call)->hasTransferPending();
}

LinphoneCall *linphone_call_get_transferer_call (const LinphoneCall *call) {
	shared_ptr<Call> referer = Call::toCpp(call)->getReferer();
	if (!referer)
		return nullptr;
	return referer->toC();
}

LinphoneCall *linphone_call_get_transfer_target_call (const LinphoneCall *call) {
	shared_ptr<Call> transferTarget = Call::toCpp(call)->getTransferTarget();
	if (!transferTarget)
		return nullptr;
	return transferTarget->toC();
}

LinphoneCall *linphone_call_get_replaced_call (LinphoneCall *call) {
	shared_ptr<Call> replacedCall = Call::toCpp(call)->getReplacedCall();
	if (!replacedCall)
		return nullptr;
	return replacedCall->toC();
}

int linphone_call_get_duration (const LinphoneCall *call) {
	return Call::toCpp(call)->getDuration();
}

const LinphoneCallParams *linphone_call_get_current_params (LinphoneCall *call) {
	return L_GET_C_BACK_PTR(Call::toCpp(call)->getCurrentParams());
}

const LinphoneCallParams *linphone_call_get_remote_params(LinphoneCall *call) {
	const LinphonePrivate::MediaSessionParams *remoteParams = Call::toCpp(call)->getRemoteParams();
	return remoteParams ? L_GET_C_BACK_PTR(remoteParams) : nullptr;
}

void linphone_call_enable_camera (LinphoneCall *call, bool_t enable) {
	Call::toCpp(call)->enableCamera(!!enable);
}

bool_t linphone_call_camera_enabled (const LinphoneCall *call) {
	return Call::toCpp(call)->cameraEnabled();
}

LinphoneStatus linphone_call_take_video_snapshot (LinphoneCall *call, const char *file) {
	return Call::toCpp(call)->takeVideoSnapshot(L_C_TO_STRING(file));
}

LinphoneStatus linphone_call_take_preview_snapshot (LinphoneCall *call, const char *file) {
	return Call::toCpp(call)->takePreviewSnapshot(L_C_TO_STRING(file));
}

LinphoneReason linphone_call_get_reason (const LinphoneCall *call) {
	return Call::toCpp(call)->getReason();
}

const LinphoneErrorInfo *linphone_call_get_error_info (const LinphoneCall *call) {
	return Call::toCpp(call)->getErrorInfo();
}

const char *linphone_call_get_remote_user_agent (LinphoneCall *call) {
	return Call::toCpp(call)->getRemoteUserAgent().c_str();
}

const char * linphone_call_get_remote_contact (LinphoneCall *call) {
	return Call::toCpp(call)->getRemoteContact().c_str();
}

const char *linphone_call_get_authentication_token (LinphoneCall *call) {
	return Call::toCpp(call)->getAuthenticationToken().c_str();
}

bool_t linphone_call_get_authentication_token_verified (const LinphoneCall *call) {
	return Call::toCpp(call)->getAuthenticationTokenVerified();
}

void linphone_call_set_authentication_token_verified (LinphoneCall *call, bool_t verified) {
	Call::toCpp(call)->setAuthenticationTokenVerified(!!verified);
}

void linphone_call_send_vfu_request (LinphoneCall *call) {
	Call::toCpp(call)->sendVfuRequest();
}

void linphone_call_set_next_video_frame_decoded_callback (LinphoneCall *call, LinphoneCallCbFunc cb, void *ud) {
	Call::toCpp(call)->setNextVideoFrameDecodedCallback(cb, ud);
}

void linphone_call_request_notify_next_video_frame_decoded(LinphoneCall *call){
	Call::toCpp(call)->requestNotifyNextVideoFrameDecoded();
}

LinphoneCallState linphone_call_get_transfer_state (LinphoneCall *call) {
	return static_cast<LinphoneCallState>(Call::toCpp(call)->getTransferState());
}

void linphone_call_zoom_video (LinphoneCall* call, float zoom_factor, float* cx, float* cy) {
	Call::toCpp(call)->zoomVideo(zoom_factor, cx, cy);
}

void linphone_call_zoom (LinphoneCall *call, float zoom_factor, float cx, float cy) {
	Call::toCpp(call)->zoomVideo(zoom_factor, cx, cy);
}

LinphoneStatus linphone_call_send_dtmf (LinphoneCall *call, char dtmf) {
	if (!call) {
		ms_warning("linphone_call_send_dtmf(): invalid call, canceling DTMF");
		return -1;
	}
	return Call::toCpp(call)->sendDtmf(dtmf);
}

LinphoneStatus linphone_call_send_dtmfs (LinphoneCall *call, const char *dtmfs) {
	if (!call) {
		ms_warning("linphone_call_send_dtmfs(): invalid call, canceling DTMF sequence");
		return -1;
	}
	return Call::toCpp(call)->sendDtmfs(dtmfs);
}

void linphone_call_cancel_dtmfs (LinphoneCall *call) {
	if (!call)
		return;
	Call::toCpp(call)->cancelDtmfs();
}

bool_t linphone_call_is_in_conference (const LinphoneCall *call) {
	return !!Call::toCpp(call)->isInConference();
}

LinphoneConference *linphone_call_get_conference (const LinphoneCall *call) {
	return Call::toCpp(call)->getConference();
}

void linphone_call_set_audio_route (LinphoneCall *call, LinphoneAudioRoute route) {
	Call::toCpp(call)->setAudioRoute(route);
}

int linphone_call_get_stream_count (const LinphoneCall *call) {
	return Call::toCpp(call)->getStreamCount();
}

MSFormatType linphone_call_get_stream_type (const LinphoneCall *call, int stream_index) {
	return Call::toCpp(call)->getStreamType(stream_index);
}

RtpTransport *linphone_call_get_meta_rtp_transport (const LinphoneCall *call, int stream_index) {
	return Call::toCpp(call)->getMetaRtpTransport(stream_index);
}

RtpTransport *linphone_call_get_meta_rtcp_transport (const LinphoneCall *call, int stream_index) {
	return Call::toCpp(call)->getMetaRtcpTransport(stream_index);
}

LinphoneStatus linphone_call_pause (LinphoneCall *call) {
	return Call::toCpp(call)->pause();
}

LinphoneStatus linphone_call_resume (LinphoneCall *call) {
	return Call::toCpp(call)->resume();
}

LinphoneStatus linphone_call_terminate (LinphoneCall *call) {
	return Call::toCpp(call)->terminate();
}

LinphoneStatus linphone_call_terminate_with_error_info (LinphoneCall *call , const LinphoneErrorInfo *ei) {
	return Call::toCpp(call)->terminate(ei);
}

LinphoneStatus linphone_call_redirect (LinphoneCall *call, const char *redirect_uri) {
	return Call::toCpp(call)->redirect(redirect_uri);
}

LinphoneStatus linphone_call_decline (LinphoneCall *call, LinphoneReason reason) {
	return Call::toCpp(call)->decline(reason);
}

LinphoneStatus linphone_call_decline_with_error_info (LinphoneCall *call, const LinphoneErrorInfo *ei) {
	return Call::toCpp(call)->decline(ei);
}

LinphoneStatus linphone_call_accept (LinphoneCall *call) {
	return Call::toCpp(call)->accept(nullptr);
}

LinphoneStatus linphone_call_accept_with_params (LinphoneCall *call, const LinphoneCallParams *params) {
	return Call::toCpp(call)->accept(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_accept_early_media (LinphoneCall* call) {
	return Call::toCpp(call)->acceptEarlyMedia();
}

LinphoneStatus linphone_call_accept_early_media_with_params (LinphoneCall *call, const LinphoneCallParams *params) {
	return Call::toCpp(call)->acceptEarlyMedia(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_update (LinphoneCall *call, const LinphoneCallParams *params) {
	return Call::toCpp(call)->update(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_defer_update (LinphoneCall *call) {
	return Call::toCpp(call)->deferUpdate();
}

LinphoneStatus linphone_call_accept_update (LinphoneCall *call, const LinphoneCallParams *params) {
	return Call::toCpp(call)->acceptUpdate(params ? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
}

LinphoneStatus linphone_call_transfer (LinphoneCall *call, const char *referTo) {
	return Call::toCpp(call)->transfer(referTo);
}

LinphoneStatus linphone_call_transfer_to_another (LinphoneCall *call, LinphoneCall *dest) {
	return Call::toCpp(call)->transfer(Call::toCpp(dest)->getSharedFromThis());
}

void *linphone_call_get_native_video_window_id (const LinphoneCall *call) {
	return Call::toCpp(call)->getNativeVideoWindowId();
}

void linphone_call_set_native_video_window_id (LinphoneCall *call, void *id) {
	Call::toCpp(call)->setNativeVideoWindowId(id);
}

void linphone_call_enable_echo_cancellation (LinphoneCall *call, bool_t enable) {
	Call::toCpp(call)->enableEchoCancellation(!!enable);
}

bool_t linphone_call_echo_cancellation_enabled (const LinphoneCall *call) {
	return Call::toCpp(call)->echoCancellationEnabled();
}

void linphone_call_enable_echo_limiter (LinphoneCall *call, bool_t val) {
	Call::toCpp(call)->enableEchoLimiter(!!val);
}

bool_t linphone_call_echo_limiter_enabled (const LinphoneCall *call) {
	return Call::toCpp(call)->echoLimiterEnabled();
}

LinphoneChatRoom *linphone_call_get_chat_room (LinphoneCall *call) {
	shared_ptr<LinphonePrivate::RealTimeTextChatRoom> acr = Call::toCpp(call)->getChatRoom();
	if (acr)
		return L_GET_C_BACK_PTR(acr);
	return nullptr;
}

float linphone_call_get_play_volume (const LinphoneCall *call) {
	return Call::toCpp(call)->getPlayVolume();
}

float linphone_call_get_record_volume (const LinphoneCall *call) {
	return Call::toCpp(call)->getRecordVolume();
}

float linphone_call_get_speaker_volume_gain (const LinphoneCall *call) {
	return Call::toCpp(call)->getSpeakerVolumeGain();
}

void linphone_call_set_speaker_volume_gain( LinphoneCall *call, float volume) {
	Call::toCpp(call)->setSpeakerVolumeGain(volume);
}

float linphone_call_get_microphone_volume_gain (const LinphoneCall *call) {
	return Call::toCpp(call)->getMicrophoneVolumeGain();
}

void linphone_call_set_microphone_volume_gain (LinphoneCall *call, float volume) {
	Call::toCpp(call)->setMicrophoneVolumeGain(volume);
}

bool_t linphone_call_get_speaker_muted (const LinphoneCall *call) {
	return Call::toCpp(call)->getSpeakerMuted();
}

void linphone_call_set_speaker_muted (LinphoneCall *call, bool_t muted) {
	Call::toCpp(call)->setSpeakerMuted(!!muted);
}

bool_t linphone_call_get_microphone_muted (const LinphoneCall *call) {
	return Call::toCpp(call)->getMicrophoneMuted();
}

void linphone_call_set_microphone_muted (LinphoneCall *call, bool_t muted) {
	Call::toCpp(call)->setMicrophoneMuted(!!muted);
}

float linphone_call_get_current_quality (const LinphoneCall *call) {
	return Call::toCpp(call)->getCurrentQuality();
}

float linphone_call_get_average_quality (const LinphoneCall *call) {
	return Call::toCpp(call)->getAverageQuality();
}

void linphone_call_start_recording (LinphoneCall *call) {
	Call::toCpp(call)->startRecording();
}

void linphone_call_stop_recording (LinphoneCall *call) {
	Call::toCpp(call)->stopRecording();
}

bool_t linphone_call_is_recording (LinphoneCall *call) {
	return Call::toCpp(call)->isRecording();
}

LinphonePlayer *linphone_call_get_player (LinphoneCall *call) {
	return Call::toCpp(call)->getPlayer();
}

bool_t linphone_call_media_in_progress (const LinphoneCall *call) {
	return Call::toCpp(call)->mediaInProgress();
}

void linphone_call_ogl_render (const LinphoneCall *call) {
	Call::toCpp(call)->oglRender();
}

LinphoneStatus linphone_call_send_info_message (LinphoneCall *call, const LinphoneInfoMessage *info) {
	SalBodyHandler *body_handler = sal_body_handler_from_content(linphone_info_message_get_content(info));
	linphone_call_get_op(call)->setSentCustomHeaders(linphone_info_message_get_headers(info));
	return linphone_call_get_op(call)->sendInfo(body_handler);
}

LinphoneCallStats *linphone_call_get_stats (LinphoneCall *call, LinphoneStreamType type) {
	return Call::toCpp(call)->getStats(type);
}

LinphoneCallStats *linphone_call_get_audio_stats (LinphoneCall *call) {
	return Call::toCpp(call)->getAudioStats();
}

LinphoneCallStats *linphone_call_get_video_stats (LinphoneCall *call) {
	return Call::toCpp(call)->getVideoStats();
}

LinphoneCallStats *linphone_call_get_text_stats (LinphoneCall *call) {
	return Call::toCpp(call)->getTextStats();
}

void linphone_call_add_callbacks (LinphoneCall *call, LinphoneCallCbs *cbs) {
	Call::toCpp(call)->addCallbacks(cbs);
}

void linphone_call_remove_callbacks (LinphoneCall *call, LinphoneCallCbs *cbs) {
	Call::toCpp(call)->removeCallbacks(cbs);
}

LinphoneCallCbs *linphone_call_get_current_callbacks (const LinphoneCall *call) {
	return Call::toCpp(call)->getCurrentCbs();
}

const bctbx_list_t *linphone_call_get_callbacks_list(const LinphoneCall *call) {
	return Call::toCpp(call)->getCallbacksList();
}

void linphone_call_set_params (LinphoneCall *call, const LinphoneCallParams *params) {
	Call::toCpp(call)->setParams(L_GET_CPP_PTR_FROM_C_OBJECT(params));
}

const LinphoneCallParams *linphone_call_get_params (LinphoneCall *call) {
	return L_GET_C_BACK_PTR(Call::toCpp(call)->getParams());
}

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneCall *linphone_call_ref (LinphoneCall *call) {
	belle_sip_object_ref(call);
	return call;
}

void linphone_call_unref (LinphoneCall *call) {
	belle_sip_object_unref(call);
}

void *linphone_call_get_user_data (const LinphoneCall *call) {
	//return Wrapper::getUserData(LinphonePrivate::Utils::getPtr(Call::toCpp(call)));
	return nullptr;
}

void linphone_call_set_user_data (LinphoneCall *call, void *ud) {
	//LinphonePrivate::Wrapper::setUserData(LinphonePrivate::Utils::getPtr(Call::toCpp(call)),ud);
}

// =============================================================================
// Constructor and destructor functions.
// =============================================================================

LinphoneCall *linphone_call_new_outgoing (LinphoneCore *lc, const LinphoneAddress *from, const LinphoneAddress *to, const LinphoneCallParams *params, LinphoneProxyConfig *cfg) {
	shared_ptr<Call> call = make_shared<Call>(L_GET_CPP_PTR_FROM_C_OBJECT(lc), LinphoneCallOutgoing,
	*L_GET_CPP_PTR_FROM_C_OBJECT(from), *L_GET_CPP_PTR_FROM_C_OBJECT(to),
	cfg, nullptr, L_GET_CPP_PTR_FROM_C_OBJECT(params));
	
	return call->toC();
}

LinphoneCall *linphone_call_new_incoming (LinphoneCore *lc, const LinphoneAddress *from, const LinphoneAddress *to, LinphonePrivate::SalCallOp *op) {
	shared_ptr<Call> call = make_shared<Call>(L_GET_CPP_PTR_FROM_C_OBJECT(lc), LinphoneCallIncoming,
	*L_GET_CPP_PTR_FROM_C_OBJECT(from), *L_GET_CPP_PTR_FROM_C_OBJECT(to),
	nullptr, op, nullptr);
	
	return call->toC();
}

void linphone_call_set_input_audio_device(LinphoneCall *call, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		Call::toCpp(call)->setInputAudioDevice(LinphonePrivate::AudioDevice::toCpp(audio_device));
	}
}

void linphone_call_set_output_audio_device(LinphoneCall *call, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		Call::toCpp(call)->setOutputAudioDevice(LinphonePrivate::AudioDevice::toCpp(audio_device));
	}
}

const LinphoneAudioDevice* linphone_call_get_input_audio_device(const LinphoneCall *call) {
	LinphonePrivate::AudioDevice *audioDevice = Call::toCpp(call)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}
const LinphoneAudioDevice* linphone_call_get_output_audio_device(const LinphoneCall *call) {
	LinphonePrivate::AudioDevice *audioDevice = Call::toCpp(call)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}
