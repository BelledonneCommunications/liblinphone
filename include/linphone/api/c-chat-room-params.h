/*
 * c-chat-room-params.h
 * Copyright (C) 2010-2019 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _L_C_CHAT_ROOM_PARAMS_H_
#define _L_C_CHAT_ROOM_PARAMS_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup chatroom
 * @{
 */

/**
 * Returns new chat room parameters.
 * @return LinphoneChatRoomParams
**/
LINPHONE_PUBLIC LinphoneChatRoomParams *linphone_chat_room_params_new(void);

/**
 * Acquire a reference to the chat room parameters.
 * @param[in] params The chat room parameters.
 * @return The same chat room parameters.
**/
LINPHONE_PUBLIC LinphoneChatRoomParams *linphone_chat_room_params_ref(LinphoneChatRoomParams *params);

/**
 * Release reference to the chat room params.
 * @param[in] params The chat room params.
**/
LINPHONE_PUBLIC void linphone_chat_room_params_unref(LinphoneChatRoomParams *params);

/**
 * @param[in] params The chat room params.
 * @return TRUE if the given parameters are valid, FALSE otherwise
 **/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_is_valid(const LinphoneChatRoomParams *params);

/**
 * Get the backend implementation of the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @return LinphoneChatRoomBackend
**/
LINPHONE_PUBLIC LinphoneChatRoomBackend linphone_chat_room_params_get_backend(const LinphoneChatRoomParams *params);

/**
 * Get the encryption implementation of the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @return LinphoneChatRoomEncryptionBackend
**/
LINPHONE_PUBLIC LinphoneChatRoomEncryptionBackend linphone_chat_room_params_get_encryption_backend(const LinphoneChatRoomParams *params);

/**
 * Get the group chat status of the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @return TRUE if group chat is enabled, FALSE if one-to-one
**/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_group_enabled(const LinphoneChatRoomParams *params);

/**
 * Get the encryption status of the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @return TRUE if encryption is enabled, FALSE otherwise
**/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_encryption_enabled(const LinphoneChatRoomParams *params);

/**
 * Get the real time text status of the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @return TRUE if real time text is enabled, FALSE otherwise
**/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_rtt_enabled(const LinphoneChatRoomParams *params);

/**
 * Set the backend implementation of these chat room parameters.
 * @param[in] params The chat room params.
 * @param[in] backend The #LinphoneChatRoomBackend enum value
**/
LINPHONE_PUBLIC void linphone_chat_room_params_set_backend(LinphoneChatRoomParams *params, LinphoneChatRoomBackend backend);

/**
 * Set the encryption backend implementation of these chat room parameters.
 * @param[in] params The chat room params.
 * @param[in] backend The #LinphoneChatRoomEncryptionBackend enum value
**/
LINPHONE_PUBLIC void linphone_chat_room_params_set_encryption_backend(LinphoneChatRoomParams *params, LinphoneChatRoomEncryptionBackend backend);

/**
 * Enables or disables group chat for the chat room associated with the given parameters.
 * @param[in] cr The chat room params.
 * @param[in] group. TRUE to enable group chat, FALSE to disable (resulting in one-to-one chat room)
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_enable_group(LinphoneChatRoomParams *params, bool_t group);

/**
 * Enables or disables encryption for the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @param[in] encrypted. TRUE to enable encryption, FALSE to disable.
**/
LINPHONE_PUBLIC void linphone_chat_room_params_enable_encryption(LinphoneChatRoomParams *params, bool_t encrypted);

/**
 * Enables or disables real time text for the chat room associated with the given parameters.
 * @param[in] params The chat room params.
 * @param[in] rtt. TRUE to enable real time text, FALSE to disable.
**/
LINPHONE_PUBLIC void linphone_chat_room_params_enable_rtt(LinphoneChatRoomParams *params, bool_t rtt);


/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_ROOM_PARAMS_H_
