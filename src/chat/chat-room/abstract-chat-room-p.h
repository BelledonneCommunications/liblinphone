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

#ifndef _L_ABSTRACT_CHAT_ROOM_P_H_
#define _L_ABSTRACT_CHAT_ROOM_P_H_

#include "abstract-chat-room.h"
#include "chat/chat-room/chat-room-listener.h"
#include "conference/session/call-session-listener.h"
#include "object/object-p.h"

// =============================================================================

L_DECL_C_STRUCT_PREFIX_LESS(SalMessage);

LINPHONE_BEGIN_NAMESPACE

class SalOp;

class AbstractChatRoomPrivate : public ObjectPrivate, public ChatRoomListener, public CallSessionListener {
public:
	virtual void setCreationTime (time_t creationTime) = 0;
	virtual void setLastUpdateTime (time_t lastUpdateTime) = 0;

	virtual void setState (AbstractChatRoom::State newState) = 0;

	virtual void sendChatMessage (const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void addEvent (const std::shared_ptr<EventLog> &eventLog) = 0;

	virtual void addTransientEvent (const std::shared_ptr<EventLog> &eventLog) = 0;
	virtual void removeTransientEvent (const std::shared_ptr<EventLog> &eventLog) = 0;

	virtual void sendDeliveryNotifications (const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void notifyChatMessageReceived (const std::shared_ptr<ChatMessage> &chatMessage) = 0;
	virtual void notifyUndecryptableChatMessageReceived (const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual LinphoneReason onSipMessageReceived (SalOp *op, const SalMessage *message) = 0;
	virtual void onChatMessageReceived (const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void addTransientChatMessage (const std::shared_ptr<ChatMessage> &message) = 0;
	virtual void removeTransientChatMessage (const std::shared_ptr<ChatMessage> &message) = 0;

	virtual void setIsEmpty (const bool empty) = 0;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ABSTRACT_CHAT_ROOM_P_H_
