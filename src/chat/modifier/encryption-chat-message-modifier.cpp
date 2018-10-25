/*
 * encryption-chat-message-modifier.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/chat-room.h"
#include "chat/encryption/encryption-engine.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core.h"
#include "encryption-chat-message-modifier.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatMessageModifier::Result EncryptionChatMessageModifier::encode (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
	if (message->getCore()->getEncryptionEngine() != nullptr && (message->getChatRoom()->getCapabilities() & ClientGroupChatRoom::Capabilities::Encrypted)) {
		ChatMessageModifier::Result result = message->getCore()->getEncryptionEngine()->processOutgoingMessage(message, errorCode);
		if (result == ChatMessageModifier::Result::Suspended || result == ChatMessageModifier::Result::Done) {
			message->setIsSecured(true);
		}
		return result;
	}
	return ChatMessageModifier::Result::Skipped;
}

ChatMessageModifier::Result EncryptionChatMessageModifier::decode (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
	if (message->getCore()->getEncryptionEngine() != nullptr && (message->getChatRoom()->getCapabilities() & ClientGroupChatRoom::Capabilities::Encrypted)) {
		ChatMessageModifier::Result result = message->getCore()->getEncryptionEngine()->processIncomingMessage(message, errorCode);
		if (result == ChatMessageModifier::Result::Suspended || result == ChatMessageModifier::Result::Done) {
			message->setIsSecured(true);
		}
		return result;
	}
	return ChatMessageModifier::Result::Skipped;
}

LINPHONE_END_NAMESPACE
