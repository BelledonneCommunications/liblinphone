/*
 * cpim-chat-message-modifier.h
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

#ifndef _L_ENCRYPTION_ENGINE_LISTENER_H_
#define _L_ENCRYPTION_ENGINE_LISTENER_H_

// system includes
#include <memory>

// include includes
#include "chat/modifier/chat-message-modifier.h"

// local includes

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class ChatMessage;

class EncryptionEngineListener {
public:
	enum class EngineType {
		Undefined = -1,
		LimeV2 = 0,
		Another = 1,
		YetAnother = 2
	};

	virtual ~EncryptionEngineListener () = default;

	virtual ChatMessageModifier::Result processOutgoingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) { return ChatMessageModifier::Result::Skipped; }
	virtual ChatMessageModifier::Result processIncomingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) { return ChatMessageModifier::Result::Skipped; }
	virtual void update (LinphoneConfig *lpconfig) {};
	virtual bool encryptionEnabledForFileTransferCb (const std::shared_ptr<AbstractChatRoom> &ChatRoom) { return false; }
	virtual void generateFileTransferKeyCb (const std::shared_ptr<AbstractChatRoom> &ChatRoom, const std::shared_ptr<ChatMessage> &message) {}
	virtual int downloadingFileCb (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decryptedBuffer) { return 0; }
	virtual int uploadingFileCb (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *encryptedBuffer) { return 0; }
	virtual EncryptionEngineListener::EngineType getEngineType () { return EngineType::Undefined; }

protected:
	EncryptionEngineListener::EngineType engineType;
};

LINPHONE_END_NAMESPACE

#endif // #define _L_ENCRYPTION_ENGINELISTENER_H_
