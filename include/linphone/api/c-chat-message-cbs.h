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

#ifndef _L_C_CHAT_MESSAGE_CBS_H_
#define _L_C_CHAT_MESSAGE_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup chatroom
 * @{
 */

LinphoneChatMessageCbs *linphone_chat_message_cbs_new (void);

/**
 * Acquire a reference to the chat message callbacks object.
 * @param cbs The #LinphoneChatMessageCbs object @notnil
 * @return The same chat message callbacks object
**/
LINPHONE_PUBLIC LinphoneChatMessageCbs * linphone_chat_message_cbs_ref (LinphoneChatMessageCbs *cbs);

/**
 * Release reference to the chat message callbacks object.
 * @param cbs The #LinphoneChatMessageCbs object @notnil
**/
LINPHONE_PUBLIC void linphone_chat_message_cbs_unref (LinphoneChatMessageCbs *cbs);

/**
 * Retrieve the user pointer associated with the chat message callbacks object.
 * @param cbs The #LinphoneChatMessageCbs object @notnil
 * @return The user pointer associated with the chat message callbacks object. @maybenil
**/
LINPHONE_PUBLIC void * linphone_chat_message_cbs_get_user_data (const LinphoneChatMessageCbs *cbs);

/**
 * Assign a user pointer to the chat message callbacks object.
 * @param cbs The #LinphoneChatMessageCbs object @notnil
 * @param user_data The user pointer to associate with the chat message callbacks object. @maybenil
**/
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_user_data (LinphoneChatMessageCbs *cbs, void *user_data);

/**
 * Get the message state changed callback.
 * @param cbs #LinphoneChatMessageCbs object. @notnil
 * @return The current message state changed callback.
 */
LINPHONE_PUBLIC LinphoneChatMessageCbsMsgStateChangedCb linphone_chat_message_cbs_get_msg_state_changed (const LinphoneChatMessageCbs *cbs);

/**
 * Set the message state changed callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The message state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_msg_state_changed (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsMsgStateChangedCb cb);

/**
 * Get the file transfer receive callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @return The current file transfer receive callback.
 */
LINPHONE_PUBLIC LinphoneChatMessageCbsFileTransferRecvCb linphone_chat_message_cbs_get_file_transfer_recv (const LinphoneChatMessageCbs *cbs);

/**
 * Set the file transfer receive callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The file transfer receive callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_file_transfer_recv (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsFileTransferRecvCb cb);

/**
  * Get the file transfer send callback.
  * @param cbs LinphoneChatMessageCbs object. @notnil
  * @return The current file transfer send callback.
  * @deprecated 17/08/2020 Use #linphone_chat_message_cbs_get_file_transfer_send_chunk() instead.
  */
LINPHONE_PUBLIC LinphoneChatMessageCbsFileTransferSendCb linphone_chat_message_cbs_get_file_transfer_send (const LinphoneChatMessageCbs *cbs);

/**
 * Set the file transfer send callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The file transfer send callback to be used.
 * @deprecated 17/08/2020 Use #linphone_chat_message_cbs_set_file_transfer_send_chunk() instead.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_file_transfer_send (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsFileTransferSendCb cb);

/**
  * Get the file transfer send callback.
  * @param cbs LinphoneChatMessageCbs object. @notnil
  * @return The current file transfer send callback.
  */
LINPHONE_PUBLIC LinphoneChatMessageCbsFileTransferSendChunkCb linphone_chat_message_cbs_get_file_transfer_send_chunk (const LinphoneChatMessageCbs *cbs);

/**
 * Set the file transfer send callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The file transfer send callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_file_transfer_send_chunk (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsFileTransferSendChunkCb cb);

/**
 * Get the file transfer progress indication callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @return The current file transfer progress indication callback.
 */
LINPHONE_PUBLIC LinphoneChatMessageCbsFileTransferProgressIndicationCb linphone_chat_message_cbs_get_file_transfer_progress_indication (const LinphoneChatMessageCbs *cbs);

/**
 * Set the file transfer progress indication callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The file transfer progress indication callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_file_transfer_progress_indication (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsFileTransferProgressIndicationCb cb);

/**
 * Get the participant IMDN state changed callback.
 * @param cbs #LinphoneChatMessageCbs object. @notnil
 * @return The current participant IMDN state changed callback.
 */
LINPHONE_PUBLIC LinphoneChatMessageCbsParticipantImdnStateChangedCb linphone_chat_message_cbs_get_participant_imdn_state_changed (const LinphoneChatMessageCbs *cbs);

/**
 * Set the participant IMDN state changed callback.
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The participant IMDN state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_participant_imdn_state_changed (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsParticipantImdnStateChangedCb cb);

/**
 * Get the current "ephemeral message timer started" callback. This callback is called when the message deletion timer starts (the message has been viewed).
 * @param cbs #LinphoneChatMessageCbs object. @notnil
 * @return The current ephemeral message timer started callback.
 */
LINPHONE_PUBLIC LinphoneChatMessageCbsEphemeralMessageTimerStartedCb linphone_chat_message_cbs_get_ephemeral_message_timer_started (const LinphoneChatMessageCbs *cbs);

/**
 * Set the ephemeral message timer started callback. This callback will be used when new message deletion timer starts (the message has been viewed).
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The ephemeral message timer started callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_ephemeral_message_timer_started (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsEphemeralMessageTimerStartedCb cb);

/**
 * Get the ephemeral message deleted callback. This callback is used when a message deletion timer runs out (message is deleted).
 * @param cbs #LinphoneChatMessageCbs object. @notnil
 * @return The current ephemeral message deleted callback.
 */
LINPHONE_PUBLIC LinphoneChatMessageCbsEphemeralMessageDeletedCb linphone_chat_message_cbs_get_ephemeral_message_deleted (const LinphoneChatMessageCbs *cbs);

/**
 * Set the ephemeral message deleted callback. This callback is used when new message deletion timer runs out (message is deleted).
 * @param cbs LinphoneChatMessageCbs object. @notnil
 * @param cb The ephemeral message deleted callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_message_cbs_set_ephemeral_message_deleted (LinphoneChatMessageCbs *cbs, LinphoneChatMessageCbsEphemeralMessageDeletedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_MESSAGE_CBS_H_
