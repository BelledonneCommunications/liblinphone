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

#ifndef _L_ENCRYPTION_ENGINE_H_
#define _L_ENCRYPTION_ENGINE_H_

#include <bctoolbox/defs.h>

#include <memory>

#include "chat/chat-room/client-group-chat-room.h"
#include "chat/modifier/chat-message-modifier.h"
#include "core/core-accessor.h"
#include "linphone/lpconfig.h"
#include "sal/sal_media_description.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

#define ERROR_FILE_TRANFER_AUTHENTICATION_FAILED -0x1001

class AbstractChatRoom;
class ChatMessage;
class Account;

using EncryptionParameter = std::pair<std::string, std::string>;

class EncryptionEngine : public CoreAccessor {
public:
	enum class EngineType {
		Undefined = -1,
		LimeX3dh = 0,
		LimeX3dhServer = 1
	};

	virtual ~EncryptionEngine () = default;

	virtual ChatMessageModifier::Result processOutgoingMessage (
		UNUSED(const std::shared_ptr<ChatMessage> &message),
		UNUSED(int &errorCode)
	) { return ChatMessageModifier::Result::Skipped; }

	virtual ChatMessageModifier::Result processIncomingMessage (
		UNUSED(const std::shared_ptr<ChatMessage> &message),
		UNUSED(int &errorCode)
	) { return ChatMessageModifier::Result::Skipped; }

	virtual bool isEncryptionEnabledForFileTransfer (
		UNUSED(const std::shared_ptr<AbstractChatRoom> &ChatRoom)
	) { return false; }

	virtual void generateFileTransferKey (
		UNUSED(const std::shared_ptr<AbstractChatRoom> &ChatRoom),
		UNUSED(const std::shared_ptr<ChatMessage> &message),
		UNUSED(FileTransferContent *fileTransferContent)
	) {}

	virtual int downloadingFile (
		UNUSED(const std::shared_ptr<ChatMessage> &message),
		UNUSED(size_t offset),
		UNUSED(const uint8_t *buffer),
		UNUSED(size_t size),
		UNUSED(uint8_t *decryptedBuffer),
		UNUSED(FileTransferContent *fileTransferContent)
	) { return 0; }

	virtual int uploadingFile (
		UNUSED(const std::shared_ptr<ChatMessage> &message),
		UNUSED(size_t offset),
		UNUSED(const uint8_t *buffer),
		UNUSED(size_t *size),
		UNUSED(uint8_t *encryptedBuffer),
		UNUSED(FileTransferContent *fileTransferContent)
	) { return 0; }

	virtual int cancelFileTransfer (
		UNUSED(FileTransferContent *fileTransferContent)
	) { return 0; }

	virtual void mutualAuthentication (
		UNUSED(MSZrtpContext *zrtpContext),
		UNUSED(const std::shared_ptr<SalMediaDescription> & localMediaDescription),
		UNUSED(const std::shared_ptr<SalMediaDescription> & remoteMediaDescription),
		UNUSED(LinphoneCallDir direction)
	) {}

	virtual void authenticationVerified (
		UNUSED(MSZrtpContext *zrtpContext),
		UNUSED(const std::shared_ptr<SalMediaDescription> & remoteMediaDescription),
		UNUSED(const char *peerDeviceId)
	) {}

	virtual void authenticationRejected (
		UNUSED(const char *peerDeviceId)
	) {}

	virtual void addSecurityEventInChatrooms (
		UNUSED(const IdentityAddress &peerDeviceAddr),
		UNUSED(ConferenceSecurityEvent::SecurityEventType securityEventType)
	) {}

	virtual std::shared_ptr<ConferenceSecurityEvent> onDeviceAdded (
		UNUSED(const IdentityAddress &newDeviceAddr),
		UNUSED(std::shared_ptr<Participant> participant),
		UNUSED(const std::shared_ptr<AbstractChatRoom> &chatRoom),
		UNUSED(ChatRoom::SecurityLevel currentSecurityLevel)
	) { return nullptr; }

	virtual void cleanDb () {}
	virtual void update () {}
	virtual EngineType getEngineType () { return EngineType::Undefined; }
	virtual AbstractChatRoom::SecurityLevel getSecurityLevel (UNUSED(const std::string &deviceId)) const { return AbstractChatRoom::SecurityLevel::ClearText; }
	virtual AbstractChatRoom::SecurityLevel getSecurityLevel (UNUSED(const std::list<std::string> &deviceIds)) const { return AbstractChatRoom::SecurityLevel::ClearText; }
	virtual std::list<EncryptionParameter> getEncryptionParameters () { return std::list<EncryptionParameter>(); }

	virtual void staleSession (UNUSED(const std::string localDeviceId), UNUSED(const std::string peerDeviceId)) {};

	virtual void setTestForceDecryptionFailureFlag(
		UNUSED(bool flag)
	) {}

	virtual void onServerUrlChanged(
		UNUSED(const std::shared_ptr<Account> & account),
		UNUSED(const std::string& limeServerUrl)
	) {}

protected:
	EncryptionEngine (const std::shared_ptr<Core> &core) : CoreAccessor(core) {}

	EngineType engineType;
};

LINPHONE_END_NAMESPACE

#endif // #define _L_ENCRYPTION_ENGINE_H_
