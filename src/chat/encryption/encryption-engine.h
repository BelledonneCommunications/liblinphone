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

#include "chat/chat-room/chat-room.h"
#include "chat/modifier/chat-message-modifier.h"
#include "core/core-accessor.h"
#include "linphone/lpconfig.h"
#include "sal/sal.h"
#include "sal/sal_media_description.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

#define ERROR_FILE_TRANFER_AUTHENTICATION_FAILED -0x1001

class AbstractChatRoom;
class ChatMessage;
class Account;

class LimeX3dhUtils {
public:
	static bool isMessageEncrypted(const Content &internalContent);
};

using EncryptionParameter = std::pair<std::string, std::string>;

class EncryptionEngine : public CoreAccessor {
public:
	enum class EngineType { Undefined = -1, LimeX3dh = 0, LimeX3dhServer = 1 };

	virtual ~EncryptionEngine() = default;

	virtual ChatMessageModifier::Result
	processOutgoingMessage(BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &message), BCTBX_UNUSED(int &errorCode)) {
		return ChatMessageModifier::Result::Skipped;
	}

	virtual ChatMessageModifier::Result
	processIncomingMessage(BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &message), BCTBX_UNUSED(int &errorCode)) {
		return ChatMessageModifier::Result::Skipped;
	}

	virtual bool isEncryptionEnabledForFileTransfer(BCTBX_UNUSED(const std::shared_ptr<AbstractChatRoom> &ChatRoom)) {
		return false;
	}

	virtual void
	generateFileTransferKey(BCTBX_UNUSED(const std::shared_ptr<AbstractChatRoom> &ChatRoom),
	                        BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &message),
	                        BCTBX_UNUSED(const std::shared_ptr<FileTransferContent> &fileTransferContent)) {
	}

	virtual int downloadingFile(BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &message),
	                            BCTBX_UNUSED(size_t offset),
	                            BCTBX_UNUSED(const uint8_t *buffer),
	                            BCTBX_UNUSED(size_t size),
	                            BCTBX_UNUSED(uint8_t *decryptedBuffer),
	                            BCTBX_UNUSED(const std::shared_ptr<FileTransferContent> &fileTransferContent)) {
		return 0;
	}

	virtual int uploadingFile(BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &message),
	                          BCTBX_UNUSED(size_t offset),
	                          BCTBX_UNUSED(const uint8_t *buffer),
	                          BCTBX_UNUSED(size_t *size),
	                          BCTBX_UNUSED(uint8_t *encryptedBuffer),
	                          BCTBX_UNUSED(const std::shared_ptr<FileTransferContent> &fileTransferContent)) {
		return 0;
	}

	virtual int cancelFileTransfer(BCTBX_UNUSED(const std::shared_ptr<FileTransferContent> &fileTransferContent)) {
		return 0;
	}

	virtual void mutualAuthentication(BCTBX_UNUSED(MSZrtpContext *zrtpContext),
	                                  BCTBX_UNUSED(const std::shared_ptr<SalMediaDescription> &localMediaDescription),
	                                  BCTBX_UNUSED(const std::shared_ptr<SalMediaDescription> &remoteMediaDescription),
	                                  BCTBX_UNUSED(LinphoneCallDir direction)) {
	}

	virtual void
	authenticationVerified(BCTBX_UNUSED(MSZrtpContext *zrtpContext),
	                       BCTBX_UNUSED(const std::shared_ptr<SalMediaDescription> &remoteMediaDescription),
	                       BCTBX_UNUSED(const char *peerDeviceId)) {
	}

	virtual void authenticationRejected(BCTBX_UNUSED(const char *peerDeviceId)) {
	}

	virtual void
	addSecurityEventInChatrooms(BCTBX_UNUSED(const std::shared_ptr<Address> &peerDeviceAddr),
	                            BCTBX_UNUSED(ConferenceSecurityEvent::SecurityEventType securityEventType)) {
	}

	virtual std::shared_ptr<ConferenceSecurityEvent>
	onDeviceAdded(BCTBX_UNUSED(const std::shared_ptr<Address> &newDeviceAddr),
	              BCTBX_UNUSED(std::shared_ptr<Participant> participant),
	              BCTBX_UNUSED(const std::shared_ptr<AbstractChatRoom> &chatRoom),
	              BCTBX_UNUSED(ChatRoom::SecurityLevel currentSecurityLevel)) {
		return nullptr;
	}

	virtual EngineType getEngineType() {
		return EngineType::Undefined;
	}
	virtual AbstractChatRoom::SecurityLevel getSecurityLevel(BCTBX_UNUSED(const std::string &deviceId)) const {
		return AbstractChatRoom::SecurityLevel::ClearText;
	}
	virtual AbstractChatRoom::SecurityLevel
	getSecurityLevel(BCTBX_UNUSED(const std::list<std::string> &deviceIds)) const {
		return AbstractChatRoom::SecurityLevel::ClearText;
	}
	virtual std::list<EncryptionParameter>
	getEncryptionParameters(BCTBX_UNUSED(const std::shared_ptr<Account> &account)) {
		return std::list<EncryptionParameter>();
	}

	virtual void staleSession(BCTBX_UNUSED(const std::string localDeviceId),
	                          BCTBX_UNUSED(const std::string peerDeviceId)){};

	virtual void setTestForceDecryptionFailureFlag(BCTBX_UNUSED(bool flag)) {
	}

	virtual void onServerUrlChanged(BCTBX_UNUSED(std::shared_ptr<Account> &account),
	                                BCTBX_UNUSED(const std::string &limeServerUrl)) {
	}

	virtual void rawEncrypt(
	    BCTBX_UNUSED(const std::string &localDeviceId),
	    BCTBX_UNUSED(const std::list<std::string> &recipientDevices),
	    BCTBX_UNUSED(const std::vector<uint8_t> &plainMessage),
	    BCTBX_UNUSED(const std::vector<uint8_t> &associatedData),
	    BCTBX_UNUSED(const std::function<void(const bool status,
	                                          std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts)>
	                     &callback)) const {};

	virtual bool rawDecrypt(BCTBX_UNUSED(const std::string &localDeviceId),
	                        BCTBX_UNUSED(const std::string &senderDeviceId),
	                        BCTBX_UNUSED(const std::vector<uint8_t> &associatedData),
	                        BCTBX_UNUSED(const std::vector<uint8_t> &cipherText),
	                        BCTBX_UNUSED(std::vector<uint8_t> &plainText)) const {
		return false;
	}

	virtual void createLimeUser(BCTBX_UNUSED(std::shared_ptr<Account> &account),
	                            BCTBX_UNUSED(const std::string &gruu)){};

	virtual bool participantListRequired() const {
		return false;
	}

protected:
	EncryptionEngine(const std::shared_ptr<Core> &core) : CoreAccessor(core) {
	}

	EngineType engineType;
};

LINPHONE_END_NAMESPACE

#endif // #define _L_ENCRYPTION_ENGINE_H_
