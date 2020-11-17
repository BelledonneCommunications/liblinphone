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

#ifndef _L_CHAT_ROOM_ENUMS_H_
#define _L_CHAT_ROOM_ENUMS_H_

// =============================================================================
/**
 * #LinphoneChatRoomState is used to indicate the current state of a chat room.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatRoomState{
	LinphoneChatRoomStateNone, /**< Initial state */
	LinphoneChatRoomStateInstantiated, /**< Chat room is now instantiated on local */
	LinphoneChatRoomStateCreationPending, /**< One creation request was sent to the server */
	LinphoneChatRoomStateCreated, /**< Chat room was created on the server */
	LinphoneChatRoomStateCreationFailed, /**< Chat room creation failed */
	LinphoneChatRoomStateTerminationPending, /**< Wait for chat room termination */
	LinphoneChatRoomStateTerminated, /**< Chat room exists on server but not in local */
	LinphoneChatRoomStateTerminationFailed, /**< The chat room termination failed */
	LinphoneChatRoomStateDeleted, /**< Chat room was deleted on the server */
} LinphoneChatRoomState;

/**
 * #LinphoneChatRoomCapabilities is used to indicate the capabilities of a chat room.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatRoomCapabilities{
	LinphoneChatRoomCapabilitiesNone = 0, /**< No capabilities. */
	LinphoneChatRoomCapabilitiesBasic = 1 << 0 , /**< No server. It&apos;s a direct communication */
	LinphoneChatRoomCapabilitiesRealTimeText = 1 << 1, /**< Supports RTT */
	LinphoneChatRoomCapabilitiesConference = 1 << 2, /**< Use server (supports group chat) */
	LinphoneChatRoomCapabilitiesProxy = 1 << 3, /**< Special proxy chat room flag */
	LinphoneChatRoomCapabilitiesMigratable = 1 << 4, /**< Chat room migratable from Basic to Conference */
	LinphoneChatRoomCapabilitiesOneToOne = 1 << 5, /**< A communication between two participants (can be Basic or Conference) */
	LinphoneChatRoomCapabilitiesEncrypted = 1 << 6 /**< Chat room is encrypted */
} LinphoneChatRoomCapabilities;


/**
 * #LinphoneChatRoomBackend is used to indicate the backend implementation of a chat room.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatRoomBackend{
	LinphoneChatRoomBackendBasic = 1 << 0, /**< Basic (client-to-client) chat room. */
	LinphoneChatRoomBackendFlexisipChat = 1 << 1 /**< Server-based chat room. */
} LinphoneChatRoomBackend;

/**
 * #LinphoneChatRoomEncryptionBackend is used to indicate the encryption engine used by a chat room.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatRoomEncryptionBackend{
	LinphoneChatRoomEncryptionBackendNone = 0, /**< No encryption. */
	LinphoneChatRoomEncryptionBackendLime = 1 << 0 /**< Lime x3dh encryption. */
} LinphoneChatRoomEncryptionBackend;


#endif // ifndef _L_CHAT_ROOM_ENUMS_H_
