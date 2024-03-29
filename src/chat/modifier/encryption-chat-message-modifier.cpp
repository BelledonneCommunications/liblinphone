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

#include "encryption-chat-message-modifier.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/chat-room.h"
#include "chat/encryption/encryption-engine.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatMessageModifier::Result EncryptionChatMessageModifier::encode(const shared_ptr<ChatMessage> &message,
                                                                  int &errorCode) {
	auto imee = message->getCore()->getEncryptionEngine();
	if (imee != nullptr) {
		ChatMessageModifier::Result result = imee->processOutgoingMessage(message, errorCode);
		if (result == ChatMessageModifier::Result::Suspended || result == ChatMessageModifier::Result::Done) {
			message->setIsSecured(true);
		}
		return result;
	}
	return ChatMessageModifier::Result::Skipped;
}

ChatMessageModifier::Result EncryptionChatMessageModifier::decode(const shared_ptr<ChatMessage> &message,
                                                                  int &errorCode) {
	auto imee = message->getCore()->getEncryptionEngine();
	if (imee != nullptr) {
		ChatMessageModifier::Result result = imee->processIncomingMessage(message, errorCode);
		if (result == ChatMessageModifier::Result::Suspended || result == ChatMessageModifier::Result::Done) {
			message->setIsSecured(true);
		}
		return result;
	}
	return ChatMessageModifier::Result::Skipped;
}

LINPHONE_END_NAMESPACE
