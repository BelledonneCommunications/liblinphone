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

#ifndef _L_ENCRYPTION_ENGINE_ENUMS_H_
#define _L_ENCRYPTION_ENGINE_ENUMS_H_

// =============================================================================
/**
 * TODO move to encryption engine object when available
 * #LinphoneChatRoomSecurityLevel is used to indicate the encryption security level of a chat room.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatRoomSecurityLevel {
	LinphoneChatRoomSecurityLevelUnsafe = 0,    /**< Security failure */
	LinphoneChatRoomSecurityLevelClearText = 1, /**< No encryption */
	LinphoneChatRoomSecurityLevelEncrypted = 2, /**< Encrypted */
	LinphoneChatRoomSecurityLevelSafe = 3       /**< Encrypted and verified */
} LinphoneChatRoomSecurityLevel;

#endif // ifndef _L_ENCRYPTION_ENGINE_ENUMS_H_
