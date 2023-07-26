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

#include "chat/chat-message/imdn-message-p.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "content/content-disposition.h"
#include "logger/logger.h"
#include "sip-tools/sip-headers.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

void ImdnMessagePrivate::setState(ChatMessage::State newState) {
	L_Q();

	if (newState == ChatMessage::State::Delivered) {
		for (const auto &message : context.deliveredMessages)
			message->getPrivate()->updateInDb();
		for (const auto &message : context.displayedMessages)
			message->getPrivate()->updateInDb();
		context.mChatRoom->getImdnHandler()->onImdnMessageDelivered(q->getSharedFromThis());
	} else if (newState == ChatMessage::State::NotDelivered) {
		// TODO: Maybe we should retry sending the IMDN message if we get an error here
		context.mChatRoom->getImdnHandler()->onImdnMessageNotDelivered(q->getSharedFromThis());
	}
}

// -----------------------------------------------------------------------------

ImdnMessage::ImdnMessage(const shared_ptr<AbstractChatRoom> &chatRoom,
                         const list<shared_ptr<ChatMessage>> &deliveredMessages,
                         const list<shared_ptr<ChatMessage>> &displayedMessages)
    : ImdnMessage(Context(chatRoom, deliveredMessages, displayedMessages)) {
}

ImdnMessage::ImdnMessage(const shared_ptr<AbstractChatRoom> &chatRoom,
                         const list<Imdn::MessageReason> &nonDeliveredMessages)
    : ImdnMessage(Context(chatRoom, nonDeliveredMessages)) {
}

ImdnMessage::ImdnMessage(const std::shared_ptr<ImdnMessage> &message) : ImdnMessage(message->getPrivate()->context) {
}

ImdnMessage::ImdnMessage(const Context &context) : NotificationMessage(*new ImdnMessagePrivate(context)) {
	L_D();
	for (const auto &message : d->context.deliveredMessages) {
		// Don't send IMDN if the message we send it for has no Message-ID
		const string &imdnMessageId = message->getImdnMessageId();
		if (imdnMessageId.empty()) {
			lWarning() << "Skipping delivery IMDN as message doesn't have a Message-ID";
			continue;
		}

		auto content = Content::create();
		content->setContentDisposition(ContentDisposition::Notification);
		content->setContentType(ContentType::Imdn);
		content->setBodyFromUtf8(
		    Imdn::createXml(imdnMessageId, message->getTime(), Imdn::Type::Delivery, LinphoneReasonNone));
		addContent(content);
	}
	for (const auto &message : d->context.displayedMessages) {
		// Don't send IMDN if the message we send it for has no Message-ID
		const string &imdnMessageId = message->getImdnMessageId();
		if (imdnMessageId.empty()) {
			lWarning() << "Skipping displayed IMDN as message doesn't have a Message-ID";
			continue;
		}

		auto content = Content::create();
		content->setContentDisposition(ContentDisposition::Notification);
		content->setContentType(ContentType::Imdn);
		content->setBodyFromUtf8(
		    Imdn::createXml(imdnMessageId, message->getTime(), Imdn::Type::Display, LinphoneReasonNone));
		addContent(content);
	}
	for (const auto &mr : d->context.nonDeliveredMessages) {
		// Don't send IMDN if the message we send it for has no Message-ID
		const string &imdnMessageId = mr.message->getImdnMessageId();
		if (imdnMessageId.empty()) {
			lWarning() << "Skipping not delivered IMDN as message doesn't have a Message-ID";
			continue;
		}

		auto content = Content::create();
		content->setContentDisposition(ContentDisposition::Notification);
		content->setContentType(ContentType::Imdn);
		content->setBodyFromUtf8(
		    Imdn::createXml(imdnMessageId, mr.message->getTime(), Imdn::Type::Delivery, mr.reason));
		addContent(content);
	}

	d->addSalCustomHeader(PriorityHeader::HeaderName, PriorityHeader::NonUrgent);
	if (!d->context.nonDeliveredMessages.empty()) d->setEncryptionPrevented(true);
}

LINPHONE_END_NAMESPACE
