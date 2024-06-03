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

#ifndef _L_CHAT_MESSAGE_ENUMS_H_
#define _L_CHAT_MESSAGE_ENUMS_H_

// =============================================================================

/**
 * #LinphoneChatMessageState is used to notify if messages have been successfully delivered or not.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatMessageState {
	LinphoneChatMessageStateIdle = 0,         /**< Initial state */
	LinphoneChatMessageStateInProgress = 1,   /**< Delivery in progress */
	LinphoneChatMessageStateDelivered = 2,    /**< Message successfully delivered and acknowledged by the server */
	LinphoneChatMessageStateNotDelivered = 3, /**< Message was not delivered */
	LinphoneChatMessageStateFileTransferError =
	    4, /**< Message was received and acknowledged but cannot get file from server */
	LinphoneChatMessageStateFileTransferDone = 5, /**< File transfer has been completed successfully */
	LinphoneChatMessageStateDeliveredToUser =
	    6,                                 /**< Message successfully delivered an acknowledged by the remote user */
	LinphoneChatMessageStateDisplayed = 7, /**< Message successfully displayed to the remote user */
	LinphoneChatMessageStateFileTransferInProgress =
	    8, /**< File transfer is in progress. If message is incoming it's a download, otherwise it's an upload. */
	LinphoneChatMessageStatePendingDelivery = 9,         /**< Message is pending delivery */
	LinphoneChatMessageStateFileTransferCancelling = 10, /**< The user cancelled the file transfer */
} LinphoneChatMessageState;

/**
 * #LinphoneChatMessageDirection is used to indicate if a message is outgoing or incoming.
 * @ingroup chatroom
 */
typedef enum _LinphoneChatMessageDirection {
	LinphoneChatMessageDirectionIncoming = 0, /**< Incoming message */
	LinphoneChatMessageDirectionOutgoing = 1, /**< Outgoing message */
} LinphoneChatMessageDirection;

#endif // ifndef _L_CHAT_MESSAGE_ENUMS_H_
