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

#ifndef _L_C_CALL_CBS_H_
#define _L_C_CALL_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup call_control
 * @{
 */

/**
 * Acquire a reference to the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The same #LinphoneCallCbs object.
 */
LINPHONE_PUBLIC LinphoneCallCbs *linphone_call_cbs_ref(LinphoneCallCbs *cbs);

/**
 * Release reference to the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 */
LINPHONE_PUBLIC void linphone_call_cbs_unref(LinphoneCallCbs *cbs);

/**
 * Retrieve the user pointer associated with the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The user pointer associated with the #LinphoneCallCbs object or NULL. @maybenil
 */
LINPHONE_PUBLIC void *linphone_call_cbs_get_user_data(const LinphoneCallCbs *cbs);

/**
 * Assign a user pointer to the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] user_data The user pointer to associate with the #LinphoneCallCbs object. @maybenil
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_user_data(LinphoneCallCbs *cbs, void *user_data);

/**
 * Get the dtmf received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current dtmf received callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsDtmfReceivedCb linphone_call_cbs_get_dtmf_received(LinphoneCallCbs *cbs);

/**
 * Set the dtmf received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The dtmf received callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_dtmf_received(LinphoneCallCbs *cbs, LinphoneCallCbsDtmfReceivedCb cb);

/**
 * Get the GoClear Ack sent callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The GoClear Ack sent callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsGoClearAckSentCb linphone_call_cbs_get_goclear_ack_sent(LinphoneCallCbs *cbs);

/**
 * Set the GoClear Ack sent callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The GoClear Ack sent callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_goclear_ack_sent(LinphoneCallCbs *cbs, LinphoneCallCbsGoClearAckSentCb cb);

/**
 * Get the security level downgraded callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current security level downgraded callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsSecurityLevelDowngradedCb
linphone_call_cbs_get_security_level_downgraded(LinphoneCallCbs *cbs);

/**
 * Set the security level downgraded callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The security level downgraded callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_security_level_downgraded(LinphoneCallCbs *cbs,
                                                                     LinphoneCallCbsSecurityLevelDowngradedCb cb);

/**
 * Get the encryption changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current encryption changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsEncryptionChangedCb linphone_call_cbs_get_encryption_changed(LinphoneCallCbs *cbs);

/**
 * Set the encryption changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The encryption changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_encryption_changed(LinphoneCallCbs *cbs,
                                                              LinphoneCallCbsEncryptionChangedCb cb);

/**
 * Get the authentication token verified callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current authentication token verified callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsAuthenticationTokenVerifiedCb
linphone_call_cbs_get_authentication_token_verified(LinphoneCallCbs *cbs);

/**
 * Set the authentication token verified callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The authentication token verified callback to be used.
 */
LINPHONE_PUBLIC void
linphone_call_cbs_set_authentication_token_verified(LinphoneCallCbs *cbs,
                                                    LinphoneCallCbsAuthenticationTokenVerifiedCb cb);

/**
 * Get the send master key changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current send master key changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsSendMasterKeyChangedCb
linphone_call_cbs_get_send_master_key_changed(LinphoneCallCbs *cbs);

/**
 * Set the send master key changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The send master key changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_send_master_key_changed(LinphoneCallCbs *cbs,
                                                                   LinphoneCallCbsSendMasterKeyChangedCb cb);

/**
 * Get the receive master key changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current receive master key changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsReceiveMasterKeyChangedCb
linphone_call_cbs_get_receive_master_key_changed(LinphoneCallCbs *cbs);

/**
 * Set the receive master key changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The receive master key changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_receive_master_key_changed(LinphoneCallCbs *cbs,
                                                                      LinphoneCallCbsReceiveMasterKeyChangedCb cb);

/**
 * Get the info message received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current info message received callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsInfoMessageReceivedCb linphone_call_cbs_get_info_message_received(LinphoneCallCbs *cbs);

/**
 * Set the info message received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The info message received callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_info_message_received(LinphoneCallCbs *cbs,
                                                                 LinphoneCallCbsInfoMessageReceivedCb cb);

/**
 * Get the state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current state changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsStateChangedCb linphone_call_cbs_get_state_changed(LinphoneCallCbs *cbs);

/**
 * Set the state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_state_changed(LinphoneCallCbs *cbs, LinphoneCallCbsStateChangedCb cb);

/**
 * Get the stats updated callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current stats updated callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsStatsUpdatedCb linphone_call_cbs_get_stats_updated(LinphoneCallCbs *cbs);

/**
 * Set the stats updated callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The stats updated callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_stats_updated(LinphoneCallCbs *cbs, LinphoneCallCbsStatsUpdatedCb cb);

/**
 * Get the transfer state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current transfer state changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsTransferStateChangedCb
linphone_call_cbs_get_transfer_state_changed(LinphoneCallCbs *cbs);

/**
 * Set the transfer state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The transfer state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_transfer_state_changed(LinphoneCallCbs *cbs,
                                                                  LinphoneCallCbsTransferStateChangedCb cb);

/**
 * Get the refer requested callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The refer requested callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsReferRequestedCb linphone_call_cbs_get_refer_requested(LinphoneCallCbs *cbs);

/**
 * Set the refer requested callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The refer requested callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_refer_requested(LinphoneCallCbs *cbs, LinphoneCallCbsReferRequestedCb cb);

/**
 * Get the ACK processing callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current ack processing callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsAckProcessingCb linphone_call_cbs_get_ack_processing(LinphoneCallCbs *cbs);

/**
 * Set ACK processing callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The ack processing callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_ack_processing(LinphoneCallCbs *cbs, LinphoneCallCbsAckProcessingCb cb);

/**
 * Get the TMMBR received callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The current TMMBR received callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsTmmbrReceivedCb linphone_call_cbs_get_tmmbr_received(LinphoneCallCbs *cbs);

/**
 * Set the TMMBR received callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The TMMBR received callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_tmmbr_received(LinphoneCallCbs *cbs, LinphoneCallCbsTmmbrReceivedCb cb);

/**
 * Get the snapshot taken callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The current snapshot taken callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsSnapshotTakenCb linphone_call_cbs_get_snapshot_taken(LinphoneCallCbs *cbs);

/**
 * Set the snapshot taken callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The snapshot taken callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_snapshot_taken(LinphoneCallCbs *cbs, LinphoneCallCbsSnapshotTakenCb cb);

/**
 * Get the next video frame decoded callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The current next video frame decoded callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsNextVideoFrameDecodedCb
linphone_call_cbs_get_next_video_frame_decoded(LinphoneCallCbs *cbs);

/**
 * Set the next video frame decoded callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The next video frame decoded callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_next_video_frame_decoded(LinphoneCallCbs *cbs,
                                                                    LinphoneCallCbsNextVideoFrameDecodedCb cb);

/**
 * Get the camera not working callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The camera not working callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsCameraNotWorkingCb linphone_call_cbs_get_camera_not_working(LinphoneCallCbs *cbs);

/**
 * Set the camera not working callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The camera not working callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_camera_not_working(LinphoneCallCbs *cbs,
                                                              LinphoneCallCbsCameraNotWorkingCb cb);

/**
 * Get the callback that will be used to notify that there are errors from the video rendering.
 * Check #LinphoneCallCbsVideoDisplayErrorOccurredCb for more details.
 *
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The failing rendering callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsVideoDisplayErrorOccurredCb
linphone_call_cbs_get_video_display_error_occurred(LinphoneCallCbs *cbs);

/**
 * Set the callback that will be used to notify that there are errors from the video rendering.
 * Check #LinphoneCallCbsVideoDisplayErrorOccurredCb for more details.
 *
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The failing rendering callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_video_display_error_occurred(LinphoneCallCbs *cbs,
                                                                        LinphoneCallCbsVideoDisplayErrorOccurredCb cb);

/**
 * Get the audio device changed callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The audio device changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsAudioDeviceChangedCb linphone_call_cbs_get_audio_device_changed(LinphoneCallCbs *cbs);

/**
 * Set the audio device changed callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The audio device changedcallback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_audio_device_changed(LinphoneCallCbs *cbs,
                                                                LinphoneCallCbsAudioDeviceChangedCb cb);

/**
 * Set the call remote recording callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The call remote recording callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_remote_recording(LinphoneCallCbs *cbs, LinphoneCallCbsRemoteRecordingCb cb);

/**
 * Get the call remote recording callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @return The call remote recording callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsRemoteRecordingCb linphone_call_cbs_get_remote_recording(LinphoneCallCbs *cbs);

/**
 * Set the Baudot detected callback.
 * @param[in] cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb the Baudot detected callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_baudot_detected(LinphoneCallCbs *cbs, LinphoneCallCbsBaudotDetectedCb cb);

/**
 * Get the Baudot detected callback.
 * @param[in] cbs The #LinphoneCallCbs object. @notnil
 * @return The Baudot detected callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsBaudotDetectedCb linphone_call_cbs_get_baudot_detected(LinphoneCallCbs *cbs);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CALL_CBS_H_
