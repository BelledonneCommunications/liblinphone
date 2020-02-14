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

#ifndef _L_CHAT_MESSAGE_ENUMS_H_
#define _L_CHAT_MESSAGE_ENUMS_H_

// =============================================================================

/**
 * #LinphoneChatMessageState is used to notify if messages have been successfully delivered or not.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatMessageState{
	LinphoneChatMessageStateIdle, /**< Initial state */
	LinphoneChatMessageStateInProgress, /**< Delivery in progress */
	LinphoneChatMessageStateDelivered, /**< Message successfully delivered and acknowledged by the server */
	LinphoneChatMessageStateNotDelivered, /**< Message was not delivered */
	LinphoneChatMessageStateFileTransferError, /**< Message was received and acknowledged but cannot get file from server */
	LinphoneChatMessageStateFileTransferDone, /**< File transfer has been completed successfully */
	LinphoneChatMessageStateDeliveredToUser, /**< Message successfully delivered an acknowledged by the remote user */
	LinphoneChatMessageStateDisplayed, /**< Message successfully displayed to the remote user */
	LinphoneChatMessageStateFileTransferInProgress, /** <File transfer is in progress. If message is incoming it's a download, otherwise it's an upload. */
} LinphoneChatMessageState;


/**
 * #LinphoneChatMessageDirection is used to indicate if a message is outgoing or incoming.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatMessageDirection{
	LinphoneChatMessageDirectionIncoming, /**< Incoming message */
	LinphoneChatMessageDirectionOutgoing, /**< Outgoing message */
} LinphoneChatMessageDirection;


#endif // ifndef _L_CHAT_MESSAGE_ENUMS_H_
