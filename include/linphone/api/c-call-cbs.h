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
LINPHONE_PUBLIC LinphoneCallCbs *linphone_call_cbs_ref (LinphoneCallCbs *cbs);

/**
 * Release reference to the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 */
LINPHONE_PUBLIC void linphone_call_cbs_unref (LinphoneCallCbs *cbs);

/**
 * Retrieve the user pointer associated with the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The user pointer associated with the #LinphoneCallCbs object or NULL. @maybenil
 */
LINPHONE_PUBLIC void *linphone_call_cbs_get_user_data (const LinphoneCallCbs *cbs);

/**
 * Assign a user pointer to the #LinphoneCallCbs object.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] user_data The user pointer to associate with the #LinphoneCallCbs object. @maybenil
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_user_data (LinphoneCallCbs *cbs, void *user_data);

/**
 * Get the dtmf received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current dtmf received callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsDtmfReceivedCb linphone_call_cbs_get_dtmf_received (LinphoneCallCbs *cbs);

/**
 * Set the dtmf received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The dtmf received callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_dtmf_received (LinphoneCallCbs *cbs, LinphoneCallCbsDtmfReceivedCb cb);

/**
 * Get the encryption changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current encryption changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsEncryptionChangedCb linphone_call_cbs_get_encryption_changed (LinphoneCallCbs *cbs);

/**
 * Set the encryption changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The encryption changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_encryption_changed (LinphoneCallCbs *cbs, LinphoneCallCbsEncryptionChangedCb cb);

/**
 * Get the info message received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current info message received callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsInfoMessageReceivedCb linphone_call_cbs_get_info_message_received (LinphoneCallCbs *cbs);

/**
 * Set the info message received callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The info message received callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_info_message_received (LinphoneCallCbs *cbs, LinphoneCallCbsInfoMessageReceivedCb cb);

/**
 * Get the state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current state changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsStateChangedCb linphone_call_cbs_get_state_changed (LinphoneCallCbs *cbs);

/**
 * Set the state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_state_changed (LinphoneCallCbs *cbs, LinphoneCallCbsStateChangedCb cb);

/**
 * Get the stats updated callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current stats updated callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsStatsUpdatedCb linphone_call_cbs_get_stats_updated (LinphoneCallCbs *cbs);

/**
 * Set the stats updated callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The stats updated callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_stats_updated (LinphoneCallCbs *cbs, LinphoneCallCbsStatsUpdatedCb cb);

/**
 * Get the transfer state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current transfer state changed callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsTransferStateChangedCb linphone_call_cbs_get_transfer_state_changed (LinphoneCallCbs *cbs);

/**
 * Set the transfer state changed callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The transfer state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_transfer_state_changed (LinphoneCallCbs *cbs, LinphoneCallCbsTransferStateChangedCb cb);

/**
 * Get the ACK processing callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @return The current ack processing callback.
 */
LINPHONE_PUBLIC LinphoneCallCbsAckProcessingCb linphone_call_cbs_get_ack_processing (LinphoneCallCbs *cbs);

/**
 * Set ACK processing callback.
 * @param cbs #LinphoneCallCbs object. @notnil
 * @param[in] cb The ack processing callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_ack_processing (LinphoneCallCbs *cbs, LinphoneCallCbsAckProcessingCb cb);

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
LINPHONE_PUBLIC LinphoneCallCbsNextVideoFrameDecodedCb linphone_call_cbs_get_next_video_frame_decoded(LinphoneCallCbs *cbs);

/**
 * Set the next video frame decoded callback.
 * @param cbs The #LinphoneCallCbs object. @notnil
 * @param[in] cb The next video frame decoded callback to be used.
 */
LINPHONE_PUBLIC void linphone_call_cbs_set_next_video_frame_decoded(LinphoneCallCbs *cbs, LinphoneCallCbsNextVideoFrameDecodedCb cb);

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
LINPHONE_PUBLIC void linphone_call_cbs_set_camera_not_working(LinphoneCallCbs *cbs, LinphoneCallCbsCameraNotWorkingCb cb);

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
LINPHONE_PUBLIC void linphone_call_cbs_set_audio_device_changed(LinphoneCallCbs *cbs, LinphoneCallCbsAudioDeviceChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CALL_CBS_H_
