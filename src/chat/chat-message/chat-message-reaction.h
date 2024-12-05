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
#ifndef CHAT_MESSAGE_REACTION_H
#define CHAT_MESSAGE_REACTION_H

#include "address/address.h"
#include "belle-sip/object++.hh"
#include "chat/chat-message/chat-message.h"
#include "linphone/api/c-types.h"
#include <chat/chat-message/chat-message-listener.h>

LINPHONE_BEGIN_NAMESPACE

class ChatMessage;
class AbstractChatRoom;

class LINPHONE_PUBLIC ChatMessageReaction
    : public bellesip::HybridObject<LinphoneChatMessageReaction, ChatMessageReaction>,
      public ChatMessageListener {
public:
	ChatMessageReaction(const std::string &messageId,
	                    const std::string &emoji,
	                    std::shared_ptr<const Address> from,
	                    const std::string &messageCallId);
	~ChatMessageReaction();
	ChatMessageReaction(const ChatMessageReaction &other) = delete;

	void onChatMessageStateChanged(const std::shared_ptr<ChatMessage> &message, ChatMessage::State state) override;

	std::string toString() const override;

	std::ostream &operator<<(std::ostream &str) {
		str << this->toString();
		return str;
	}

	bool operator==(const ChatMessageReaction &reaction) const;
	bool operator!=(const ChatMessageReaction &reaction) const;

	const std::string &getBody() const;
	std::shared_ptr<const Address> getFromAddress() const;
	void send();

	void setCallId(const std::string &id) {
		mCallId = id;
	}

	const std::string &getCallId() const {
		return mCallId;
	}

	void setChatRoom(std::shared_ptr<AbstractChatRoom> cr) {
		mChatRoom = cr;
	}
	std::shared_ptr<AbstractChatRoom> getChatRoom() const {
		return mChatRoom;
	}

private:
	std::string mMessageId;
	std::string mReaction;
	std::string mCallId;
	std::shared_ptr<const Address> mFromAddress;
	std::shared_ptr<ChatMessage> mReactionMessage;
	std::shared_ptr<AbstractChatRoom> mChatRoom;
};

LINPHONE_END_NAMESPACE
#endif
