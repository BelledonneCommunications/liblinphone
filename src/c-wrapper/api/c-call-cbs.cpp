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

#include "linphone/api/c-call-cbs.h"

#include "call/call.h"

using namespace LinphonePrivate;

LinphoneCallCbs *_linphone_call_cbs_new (void) {
	return CallCbs::createCObject();
}

LinphoneCallCbs *linphone_call_cbs_ref (LinphoneCallCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_call_cbs_unref (LinphoneCallCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void *linphone_call_cbs_get_user_data (const LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->getUserData();
}

void linphone_call_cbs_set_user_data (LinphoneCallCbs *cbs, void *ud) {
	CallCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneCallCbsDtmfReceivedCb linphone_call_cbs_get_dtmf_received (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->dtmfReceivedCb;
}

void linphone_call_cbs_set_dtmf_received (LinphoneCallCbs *cbs, LinphoneCallCbsDtmfReceivedCb cb) {
	CallCbs::toCpp(cbs)->dtmfReceivedCb = cb;
}

LinphoneCallCbsEncryptionChangedCb linphone_call_cbs_get_encryption_changed (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->encryptionChangedCb;
}

void linphone_call_cbs_set_encryption_changed (LinphoneCallCbs *cbs, LinphoneCallCbsEncryptionChangedCb cb) {
	CallCbs::toCpp(cbs)->encryptionChangedCb = cb;
}

LinphoneCallCbsInfoMessageReceivedCb linphone_call_cbs_get_info_message_received (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->infoMessageReceivedCb;
}

void linphone_call_cbs_set_info_message_received (LinphoneCallCbs *cbs, LinphoneCallCbsInfoMessageReceivedCb cb) {
	CallCbs::toCpp(cbs)->infoMessageReceivedCb = cb;
}

LinphoneCallCbsStateChangedCb linphone_call_cbs_get_state_changed (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->stateChangedCb;
}

void linphone_call_cbs_set_state_changed (LinphoneCallCbs *cbs, LinphoneCallCbsStateChangedCb cb) {
	CallCbs::toCpp(cbs)->stateChangedCb = cb;
}

LinphoneCallCbsStatsUpdatedCb linphone_call_cbs_get_stats_updated (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->statsUpdatedCb;
}

void linphone_call_cbs_set_stats_updated (LinphoneCallCbs *cbs, LinphoneCallCbsStatsUpdatedCb cb) {
	CallCbs::toCpp(cbs)->statsUpdatedCb = cb;
}

LinphoneCallCbsTransferStateChangedCb linphone_call_cbs_get_transfer_state_changed (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->transferStateChangedCb;
}

void linphone_call_cbs_set_transfer_state_changed (LinphoneCallCbs *cbs, LinphoneCallCbsTransferStateChangedCb cb) {
	CallCbs::toCpp(cbs)->transferStateChangedCb = cb;
}

LinphoneCallCbsAckProcessingCb linphone_call_cbs_get_ack_processing (LinphoneCallCbs *cbs){
	return CallCbs::toCpp(cbs)->ackProcessing;
}

void linphone_call_cbs_set_ack_processing (LinphoneCallCbs *cbs, LinphoneCallCbsAckProcessingCb cb){
	CallCbs::toCpp(cbs)->ackProcessing = cb;
}

LinphoneCallCbsTmmbrReceivedCb linphone_call_cbs_get_tmmbr_received (LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->tmmbrReceivedCb;
}

void linphone_call_cbs_set_tmmbr_received (LinphoneCallCbs *cbs, LinphoneCallCbsTmmbrReceivedCb cb) {
	CallCbs::toCpp(cbs)->tmmbrReceivedCb = cb;
}

LinphoneCallCbsSnapshotTakenCb linphone_call_cbs_get_snapshot_taken(LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->snapshotTakenCb;
}

void linphone_call_cbs_set_snapshot_taken(LinphoneCallCbs *cbs, LinphoneCallCbsSnapshotTakenCb cb) {
	CallCbs::toCpp(cbs)->snapshotTakenCb = cb;
}

LinphoneCallCbsNextVideoFrameDecodedCb linphone_call_cbs_get_next_video_frame_decoded(LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->nextVideoFrameDecodedCb;
}

void linphone_call_cbs_set_next_video_frame_decoded(LinphoneCallCbs *cbs, LinphoneCallCbsNextVideoFrameDecodedCb cb) {
	CallCbs::toCpp(cbs)->nextVideoFrameDecodedCb = cb;
}

LinphoneCallCbsCameraNotWorkingCb linphone_call_cbs_get_camera_not_working(LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->cameraNotWorkingCb;
}

void linphone_call_cbs_set_camera_not_working(LinphoneCallCbs *cbs, LinphoneCallCbsCameraNotWorkingCb cb) {
	CallCbs::toCpp(cbs)->cameraNotWorkingCb = cb;
}

LinphoneCallCbsAudioDeviceChangedCb linphone_call_cbs_get_audio_device_changed(LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->audioDeviceChangedCb;
}

void linphone_call_cbs_set_audio_device_changed(LinphoneCallCbs *cbs, LinphoneCallCbsAudioDeviceChangedCb cb) {
	CallCbs::toCpp(cbs)->audioDeviceChangedCb = cb;
}

LinphoneCallCbsRemoteRecordingCb linphone_call_cbs_get_remote_recording(LinphoneCallCbs *cbs) {
	return CallCbs::toCpp(cbs)->remoteRecordingCb;
}

void linphone_call_cbs_set_remote_recording(LinphoneCallCbs *cbs, LinphoneCallCbsRemoteRecordingCb cb) {
	CallCbs::toCpp(cbs)->remoteRecordingCb = cb;
}
