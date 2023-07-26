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

#ifndef _L_IMDN_MESSAGE_H_
#define _L_IMDN_MESSAGE_H_

#include "chat/chat-message/notification-message.h"
#include "chat/notification/imdn.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ImdnMessagePrivate;
class Imdn;

class LINPHONE_PUBLIC ImdnMessage : public NotificationMessage {
public:
	friend class ChatRoom;
	friend class Imdn;

	L_OVERRIDE_SHARED_FROM_THIS(ImdnMessage);

	virtual ~ImdnMessage() = default;

private:
	struct Context {
		Context(const std::shared_ptr<AbstractChatRoom> &chatRoom,
		        const std::list<std::shared_ptr<ChatMessage>> &deliveredMessages,
		        const std::list<std::shared_ptr<ChatMessage>> &displayedMessages)
		    : mChatRoom(chatRoom), deliveredMessages(deliveredMessages), displayedMessages(displayedMessages) {
		}
		Context(const std::shared_ptr<AbstractChatRoom> &chatRoom,
		        const std::list<Imdn::MessageReason> &nonDeliveredMessages)
		    : mChatRoom(chatRoom), nonDeliveredMessages(nonDeliveredMessages) {
		}

		std::shared_ptr<AbstractChatRoom> mChatRoom;
		std::list<std::shared_ptr<ChatMessage>> deliveredMessages;
		std::list<std::shared_ptr<ChatMessage>> displayedMessages;
		std::list<Imdn::MessageReason> nonDeliveredMessages;
	};

	ImdnMessage(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	            const std::list<std::shared_ptr<ChatMessage>> &deliveredMessages,
	            const std::list<std::shared_ptr<ChatMessage>> &displayedMessages);
	ImdnMessage(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	            const std::list<Imdn::MessageReason> &nonDeliveredMessages);
	ImdnMessage(const std::shared_ptr<ImdnMessage> &message);
	ImdnMessage(const Context &context);

	L_DECLARE_PRIVATE(ImdnMessage);
	L_DISABLE_COPY(ImdnMessage);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_IMDN_MESSAGE_H_
