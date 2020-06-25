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

#ifndef _L_CONFERENCE_ENUMS_H_
#define _L_CONFERENCE_ENUMS_H_

// =============================================================================
/**
 * #LinphoneConferenceState is used to indicate the current state of a conference.
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
	LinphoneChatRoomStateDeleted /**< Chat room was deleted on the server */
} LinphoneChatRoomState;

#endif // ifndef _L_CONFERENCE_ENUMS_H_
