/*
 * encryption-engine.h
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

#ifndef _L_ENCRYPTION_ENGINE_H_
#define _L_ENCRYPTION_ENGINE_H_

#include <memory>

#include "chat/chat-room/client-group-chat-room.h"
#include "chat/modifier/chat-message-modifier.h"
#include "linphone/lpconfig.h"
#include "sal/sal.h"
#include "core/core.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class ChatMessage;

class EncryptionEngine {
public:
	enum class EngineType {
		Undefined = -1,
		LimeX3DHEncryptionEngine = 0,
		//Another = 1,
		//YetAnother = 2
	};

	virtual ~EncryptionEngine () = default;

	virtual ChatMessageModifier::Result processOutgoingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) { return ChatMessageModifier::Result::Skipped; }
	virtual ChatMessageModifier::Result processIncomingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) { return ChatMessageModifier::Result::Skipped; }
	virtual bool encryptionEnabledForFileTransfer (const std::shared_ptr<AbstractChatRoom> &ChatRoom) { return false; }
	virtual void generateFileTransferKey (const std::shared_ptr<AbstractChatRoom> &ChatRoom, const std::shared_ptr<ChatMessage> &message) {}
	virtual int downloadingFile (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decryptedBuffer) { return 0; }
	virtual int uploadingFile (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encryptedBuffer) { return 0; }

	virtual void update (LinphoneConfig *lpconfig) {}
	virtual EncryptionEngine::EngineType getEngineType () { return EngineType::Undefined; }
	virtual AbstractChatRoom::SecurityLevel getSecurityLevel (const std::string &deviceId) const { return AbstractChatRoom::SecurityLevel::Unsafe; }
	virtual std::list<std::pair<std::string, std::string>> getEncryptionParameters (std::shared_ptr<Core> core) { return {}; }
	virtual void mutualAuthentication (SalMediaDescription *localMediaDescription, SalMediaDescription *remoteMediaDescription, MSZrtpContext *zrtpContext, LinphoneCallDir direction) {}
	virtual void authenticationVerified (const char *peerDeviceId, SalMediaDescription *remoteMediaDescription, MSZrtpContext *zrtpContext, std::shared_ptr<Core> core) {}
	virtual void authenticationRejected (const char *peerDeviceId, SalMediaDescription *remoteMediaDescription, MSZrtpContext *zrtpContext, std::shared_ptr<Core> core) {}
	virtual void addSecurityEventInChatrooms (const IdentityAddress &peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType securityEventType, std::shared_ptr<Core> core) {}
	virtual void cleanDb () {}

protected:
	EncryptionEngine::EngineType engineType;
};

LINPHONE_END_NAMESPACE

#endif // #define _L_ENCRYPTION_ENGINE_H_
