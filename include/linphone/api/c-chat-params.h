/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef LINPHONE_CHAT_PARAMS_H
#define LINPHONE_CHAT_PARAMS_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup conference
 * @{
 */

/**
 * Create an object of type a #LinphoneChatParams.
 * @param core The #LinphoneCore to pass to the constructor. @notnil
 * @return The created #LinphoneChatParams. @notnil
 */
LINPHONE_PUBLIC LinphoneChatParams *linphone_chat_params_new(const LinphoneCore *core);

/**
 * Take a reference on a #LinphoneChatParams.
 * @param params The #LinphoneChatParams to ref. @notnil
 * @return The freshly refed #LinphoneChatParams. @notnil
 */
LINPHONE_PUBLIC LinphoneChatParams *linphone_chat_params_ref(LinphoneChatParams *params);

/**
 * Release a #LinphoneChatParams.
 * @param params The #LinphoneChatParams to release. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_params_unref(LinphoneChatParams *params);

/**
 * Free a #LinphoneChatParams
 * @param params #LinphoneChatParams to free @notnil
 * @deprecated 17/03/2017 Use linphone_chat_params_unref() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_params_free(LinphoneChatParams *params);

/**
 * Clone a #LinphoneChatParams
 * @param params The #LinphoneChatParams to clone @notnil
 * @return An allocated #LinphoneChatParams with the same parameters than params @notnil
 */
LINPHONE_PUBLIC LinphoneChatParams *linphone_chat_params_clone(const LinphoneChatParams *params);

/**
 * Get the backend implementation of the text capabilities of the chat associated with the given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @return the #LinphoneChatRoomBackend
 **/
LINPHONE_PUBLIC LinphoneChatRoomBackend linphone_chat_params_get_backend(const LinphoneChatParams *params);

/**
 * Get the encryption implementation of the text capabilities of the chat associated with the given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @return the #LinphoneChatRoomEncryptionBackend
 **/
LINPHONE_PUBLIC LinphoneChatRoomEncryptionBackend
linphone_chat_params_get_encryption_backend(const LinphoneChatParams *params);

/**
 * Get the ephemeral message mode of the text capabilities of the chat associated with the given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @return the ephemeral message mode #LinphoneChatRoomEphemeralMode
 **/
LINPHONE_PUBLIC LinphoneChatRoomEphemeralMode linphone_chat_params_get_ephemeral_mode(const LinphoneChatParams *params);

/**
 * Set lifetime (in seconds) for all new ephemral messages in the text capabilities of the chat.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_params_ephemeral_enabled()
 * @param params The #LinphoneChatParams object. @notnil
 * @param time The ephemeral lifetime, default is disabled (0)
 */
LINPHONE_PUBLIC void linphone_chat_params_set_ephemeral_lifetime(LinphoneChatParams *params, long time);

/**
 * Get lifetime (in seconds) for all new ephemeral messages in the text capabilities of the chat.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_params_ephemeral_enabled()
 * @param params The #LinphoneChatParams object. @notnil
 * @return the ephemeral lifetime (in seconds)
 */
LINPHONE_PUBLIC long linphone_chat_params_get_ephemeral_lifetime(const LinphoneChatParams *params);

/**
 * Get the encryption status of the text capabilities of the chat associated with the given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @return TRUE if encryption is enabled, FALSE otherwise
 **/
LINPHONE_PUBLIC bool_t linphone_chat_params_encryption_enabled(const LinphoneChatParams *params);

/**
 * Get the real time text status of the text capabilities of the chat associated with the given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @return TRUE if real time text is enabled, FALSE otherwise
 **/
LINPHONE_PUBLIC bool_t linphone_chat_params_rtt_enabled(const LinphoneChatParams *params);

/**
 * Set the backend implementation of these text capabilities of the chat parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @param backend The #LinphoneChatRoomBackend enum value
 **/
LINPHONE_PUBLIC void linphone_chat_params_set_backend(LinphoneChatParams *params, LinphoneChatRoomBackend backend);

/**
 * Set the encryption backend implementation of these text capabilities of the chat parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @param backend The #LinphoneChatRoomEncryptionBackend enum value
 **/
LINPHONE_PUBLIC void linphone_chat_params_set_encryption_backend(LinphoneChatParams *params,
                                                                 LinphoneChatRoomEncryptionBackend backend);

/**
 * Enables or disables forcing of ephemeral messages for the text capabilities of the chat associated with the
 *given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @param mode Ephemeral message mode #LinphoneChatRoomEphemeralMode.
 **/
LINPHONE_PUBLIC void linphone_chat_params_set_ephemeral_mode(LinphoneChatParams *params,
                                                             LinphoneChatRoomEphemeralMode mode);

/**
 * Enables or disables real time text for the text capabilities of the chat associated with the given parameters.
 * @param params The #LinphoneChatParams object. @notnil
 * @param rtt TRUE to enable real time text, FALSE to disable.
 **/
LINPHONE_PUBLIC void linphone_chat_params_enable_rtt(LinphoneChatParams *params, bool_t rtt);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_CHAT_PARAMS_H
