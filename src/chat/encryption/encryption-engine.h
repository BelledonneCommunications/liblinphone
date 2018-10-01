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
#include "core/core-accessor.h"
#include "linphone/lpconfig.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class ChatMessage;

using EncryptionParameter = std::pair<std::string, std::string>;

class EncryptionEngine : public CoreAccessor {
public:
	enum class EngineType {
		Undefined = -1,
		LimeX3DH = 0,
	};

	virtual ~EncryptionEngine () = default;

	virtual ChatMessageModifier::Result processOutgoingMessage (
		const std::shared_ptr<ChatMessage> &message,
		int &errorCode
	) { return ChatMessageModifier::Result::Skipped; }

	virtual ChatMessageModifier::Result processIncomingMessage (
		const std::shared_ptr<ChatMessage> &message,
		int &errorCode
	) { return ChatMessageModifier::Result::Skipped; }

	virtual bool encryptionEnabledForFileTransfer (
		const std::shared_ptr<AbstractChatRoom> &ChatRoom
	) { return false; }

	virtual void generateFileTransferKey (
		const std::shared_ptr<AbstractChatRoom> &ChatRoom,
		const std::shared_ptr<ChatMessage> &message
	) {}

	virtual int downloadingFile (
		const std::shared_ptr<ChatMessage> &message,
		size_t offset,
		const uint8_t *buffer,
		size_t size,
		uint8_t *decryptedBuffer
	) { return 0; }

	virtual int uploadingFile (
		const std::shared_ptr<ChatMessage> &message,
		size_t offset,
		const uint8_t *buffer,
		size_t *size,
		uint8_t *encryptedBuffer
	) { return 0; }

	virtual void mutualAuthentication (
		MSZrtpContext *zrtpContext,
		SalMediaDescription *localMediaDescription,
		SalMediaDescription *remoteMediaDescription,
		LinphoneCallDir direction
	) {}

	virtual void authenticationVerified (
		MSZrtpContext *zrtpContext,
		SalMediaDescription *remoteMediaDescription,
		const char *peerDeviceId
	) {}

	virtual void authenticationRejected (
		SalMediaDescription *remoteMediaDescription,
		const char *peerDeviceId
	) {}

	virtual void addSecurityEventInChatrooms (
		const IdentityAddress &peerDeviceAddr,
		ConferenceSecurityEvent::SecurityEventType securityEventType
	) {}

	virtual std::shared_ptr<ConferenceSecurityEvent> onDeviceAdded (
		const IdentityAddress &newDeviceAddr,
		std::shared_ptr<Participant> participant,
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		ChatRoom::SecurityLevel currentSecurityLevel
	) { return nullptr; }

	virtual void cleanDb () {}
	virtual void update () {}
	virtual EngineType getEngineType () { return EngineType::Undefined; }
	virtual AbstractChatRoom::SecurityLevel getSecurityLevel (const std::string &deviceId) const { return AbstractChatRoom::SecurityLevel::ClearText; }
	virtual std::list<EncryptionParameter> getEncryptionParameters () { return std::list<EncryptionParameter>(); }

protected:
	EncryptionEngine (const std::shared_ptr<Core> &core) : CoreAccessor(core) {}

	EngineType engineType;
};

LINPHONE_END_NAMESPACE

#endif // #define _L_ENCRYPTION_ENGINE_H_
