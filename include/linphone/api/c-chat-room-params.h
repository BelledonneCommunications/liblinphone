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
 * @deprecated 20/05/2024. Use linphone_conference_params_new() instead.
 **/
LINPHONE_PUBLIC LinphoneChatRoomParams *linphone_chat_room_params_new(void);
LinphoneChatRoomParams *linphone_chat_room_params_new_with_core(LinphoneCore *core);
LinphoneChatRoomParams *linphone_chat_room_params_new_and_init(LinphoneCore *core);

LinphoneChatRoomParams *linphone_chat_room_params_clone(const LinphoneChatRoomParams *params);

/**
 * Acquire a reference to the chat room parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return The same #LinphoneChatRoomParams object. @notnil
 * @deprecated 20/05/2024. Use linphone_conference_params_clone() instead.
 **/
LINPHONE_PUBLIC LinphoneChatRoomParams *linphone_chat_room_params_ref(LinphoneChatRoomParams *params);

/**
 * Release reference to the chat room params.
 * @param params The #LinphoneChatRoomParams object. @notnil
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_unref(LinphoneChatRoomParams *params);

/**
 * Returns whether the given parameters are valid or not.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return TRUE if the given parameters are valid, FALSE otherwise
 * @deprecated 20/05/2024. Use linphone_conference_params_is_valid() instead.
 **/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_is_valid(const LinphoneChatRoomParams *params);

/**
 * Get the backend implementation of the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return the #LinphoneChatRoomBackend
 * @deprecated 20/05/2024. Use linphone_chat_params_get_backend() instead.
 **/
LINPHONE_PUBLIC LinphoneChatRoomBackend linphone_chat_room_params_get_backend(const LinphoneChatRoomParams *params);

/**
 * Get the encryption implementation of the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return the #LinphoneChatRoomEncryptionBackend
 * @deprecated 20/05/2024. Use linphone_chat_params_get_encryption_backend() instead.
 **/
LINPHONE_PUBLIC LinphoneChatRoomEncryptionBackend
linphone_chat_room_params_get_encryption_backend(const LinphoneChatRoomParams *params);

/**
 * Get the group chat status of the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return TRUE if group chat is enabled, FALSE if one-to-one
 * @deprecated 20/05/2024. Use linphone_conference_params_group_enabled() instead.
 **/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_group_enabled(const LinphoneChatRoomParams *params);

/**
 * Get the ephemeral message mode of the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return the ephemeral message mode #LinphoneChatRoomEphemeralMode
 * @deprecated 20/05/2024. Use linphone_chat_params_get_ephemeral_mode() instead.
 **/
LINPHONE_PUBLIC LinphoneChatRoomEphemeralMode
linphone_chat_room_params_get_ephemeral_mode(const LinphoneChatRoomParams *params);

/**
 * Set lifetime (in seconds) for all new ephemral messages in the chat room.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_room_params_ephemeral_enabled()
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param time The ephemeral lifetime, default is disabled (0)
 * @deprecated 20/05/2024. Use linphone_chat_params_set_ephemeral_lifetime() instead.
 */
LINPHONE_PUBLIC void linphone_chat_room_params_set_ephemeral_lifetime(LinphoneChatRoomParams *params, long time);

/**
 * Get lifetime (in seconds) for all new ephemeral messages in the chat room.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_room_params_ephemeral_enabled()
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return the ephemeral lifetime (in seconds)
 * @deprecated 20/05/2024. Use linphone_chat_params_get_ephemeral_lifetime() instead.
 */
LINPHONE_PUBLIC long linphone_chat_room_params_get_ephemeral_lifetime(const LinphoneChatRoomParams *params);

/**
 * Get the encryption status of the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return TRUE if encryption is enabled, FALSE otherwise
 * @deprecated 20/05/2024. Use linphone_chat_params_encryption_enabled() instead.
 **/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_encryption_enabled(const LinphoneChatRoomParams *params);

/**
 * Get the real time text status of the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return TRUE if real time text is enabled, FALSE otherwise
 * @deprecated 20/05/2024. Use linphone_chat_params_rtt_enabled() instead.
 **/
LINPHONE_PUBLIC bool_t linphone_chat_room_params_rtt_enabled(const LinphoneChatRoomParams *params);

/**
 * Set the backend implementation of these chat room parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param backend The #LinphoneChatRoomBackend enum value
 * @deprecated 20/05/2024. Use linphone_chat_params_set_backend() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_set_backend(LinphoneChatRoomParams *params,
                                                           LinphoneChatRoomBackend backend);

/**
 * Set the encryption backend implementation of these chat room parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param backend The #LinphoneChatRoomEncryptionBackend enum value
 * @deprecated 20/05/2024. Use linphone_chat_params_set_encryption_backend() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_set_encryption_backend(LinphoneChatRoomParams *params,
                                                                      LinphoneChatRoomEncryptionBackend backend);

/**
 * Enables or disables group chat for the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param group TRUE to enable group chat, FALSE to disable (resulting in one-to-one chat room)
 * @deprecated 20/05/2024. Use linphone_conference_params_enable_group() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_enable_group(LinphoneChatRoomParams *params, bool_t group);

/**
 * Enables or disables forcing of ephemeral messages for the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param mode Ephemeral message mode #LinphoneChatRoomEphemeralMode.
 * @deprecated 20/05/2024. Use linphone_chat_params_set_ephemeral_mode() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_set_ephemeral_mode(LinphoneChatRoomParams *params,
                                                                  LinphoneChatRoomEphemeralMode mode);

/**
 * Enables or disables encryption for the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param encrypted TRUE to enable encryption, FALSE to disable.
 * @deprecated 20/05/2024. Use linphone_conference_params_enable_encryption() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_enable_encryption(LinphoneChatRoomParams *params, bool_t encrypted);

/**
 * Enables or disables real time text for the chat room associated with the given parameters.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param rtt TRUE to enable real time text, FALSE to disable.
 * @deprecated 20/05/2024. Use linphone_chat_params_enable_rtt() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_enable_rtt(LinphoneChatRoomParams *params, bool_t rtt);

/**
 * Get the subject of the chat room.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @return The subject. @maybenil
 * @deprecated 20/05/2024. Use linphone_conference_params_get_subject() instead.
 **/
LINPHONE_PUBLIC const char *linphone_chat_room_params_get_subject(const LinphoneChatRoomParams *params);

/**
 * Set the subject of the chat room.
 * @param params The #LinphoneChatRoomParams object. @notnil
 * @param subject The subject to set. @maybenil
 * @deprecated 20/05/2024. Use linphone_conference_params_set_subject() instead.
 **/
LINPHONE_PUBLIC void linphone_chat_room_params_set_subject(LinphoneChatRoomParams *params, const char *subject);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_ROOM_PARAMS_H_
