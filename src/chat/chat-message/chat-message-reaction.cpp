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

#include "chat-message-reaction.h"
#include "chat-message-p.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "core/core-p.h"
#include "db/main-db-p.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatMessageReaction::ChatMessageReaction(const string &messageId,
                                         const string &emoji,
                                         std::shared_ptr<const Address> from,
                                         const string &messageCallId)
    : messageId(messageId), callId(messageCallId), fromAddress(from) {
	string toUtf8 = Utils::localeToUtf8(emoji);
	reaction = toUtf8;
}

ChatMessageReaction::~ChatMessageReaction() {
	reactionMessage = nullptr;
}

ChatMessageReaction::ChatMessageReaction(const ChatMessageReaction &other) : HybridObject(other) {
	messageId = other.messageId;
	reactionMessage = other.reactionMessage;
	reaction = other.getBody();
	fromAddress = other.getFromAddress();
	callId = other.getCallId();
}

const string &ChatMessageReaction::getBody() const {
	return reaction;
}

std::shared_ptr<const Address> ChatMessageReaction::getFromAddress() const {
	return fromAddress;
}

void ChatMessageReaction::onChatMessageStateChanged(const shared_ptr<ChatMessage> &message, ChatMessage::State state) {
	if (!message->isReaction()) {
		lError() << "[Chat Message Reaction] Received chat message state changed callback for non-reaction message";
		return;
	}

	if (state == ChatMessage::State::Delivered) {
		lInfo() << "[Chat Message Reaction] Reaction [" << reaction << "] for message ID [" << messageId
		        << "] has been delivered";
		auto originalMessage = message->getReactionToMessage();
		if (!originalMessage) {
			lError() << "[Chat Message Reaction] Failed to find original message";
			return;
		}

		setCallId(message->getPrivate()->getCallId());
		// Now that reaction was went and Call ID is known, store it in DB
		message->getPrivate()->storeInDb();

		LinphoneChatMessage *msg = L_GET_C_BACK_PTR(originalMessage);
		LinphoneChatRoom *cr = message->getChatRoom()->toC();

		if (reaction.empty()) {
			lInfo() << "[Chat Message Reaction] Sending empty reaction to chat message ID [" << messageId
			        << "] to remove any previously existing reaction";
			const LinphoneAddress *address = fromAddress->toC();
			unique_ptr<MainDb> &mainDb = message->getChatRoom()->getCore()->getPrivate()->mainDb;
			mainDb->removeConferenceChatMessageReactionEvent(messageId, fromAddress);

			_linphone_chat_message_notify_reaction_removed(msg, address);
			linphone_core_notify_message_reaction_removed(message->getCore()->getCCore(), cr, msg, address);
			linphone_core_notify_message_reaction_removed_private(message->getCore()->getCCore(), cr, msg, address,
			                                                      getCallId().c_str());
		} else {
			LinphoneChatMessageReaction *reaction = getSharedFromThis()->toC();
			_linphone_chat_message_notify_new_message_reaction(msg, reaction);
			_linphone_chat_room_notify_new_reaction_received(cr, msg, reaction);
			linphone_core_notify_new_message_reaction(message->getCore()->getCCore(), cr, msg, reaction);
		}

		message->removeListener(getSharedFromThis());
	} else if (state == ChatMessage::State::NotDelivered) {
		lError() << "[Chat Message Reaction] Can't send reaction [" << reaction << "] for message ID [" << messageId
		         << "], an error occurred";
		message->removeListener(getSharedFromThis());
	}
}

void ChatMessageReaction::send() {
	if (!chatRoom) {
		lError() << "[Chat Message Reaction] Can't send reaction [" << reaction << "] for message ID [" << messageId
		         << "], no chat room was provided, make sure to use linphone_chat_message_create_reaction()";
		return;
	}

	reactionMessage = chatRoom->createChatMessage();
	reactionMessage->addListener(getSharedFromThis());
	reactionMessage->getPrivate()->setReactionToMessageId(messageId);

	auto content = Content::create();
	content->setContentType(ContentType::PlainText);
	content->setBodyFromUtf8(reaction);
	reactionMessage->addContent(content);

	// Do not store the reaction in DB for now, won't know Call ID at this time, wait for above callback
	reactionMessage->setToBeStored(false);

	lInfo() << "[Chat Message Reaction] Sending reaction [" << reaction << "] for message ID [" << messageId << "]";
	reactionMessage->send();
}

bool ChatMessageReaction::operator==(const ChatMessageReaction &chatMessageReaction) const {
	return chatMessageReaction.reaction == reaction;
}

bool ChatMessageReaction::operator!=(const ChatMessageReaction &chatMessageReaction) const {
	return !(*this == chatMessageReaction);
}

string ChatMessageReaction::toString() const {
	std::ostringstream ss;
	ss << "Reaction [" << reaction << "] for message ID [" << messageId << "] from [" << fromAddress << "]";
	return ss.str();
}

LINPHONE_END_NAMESPACE
