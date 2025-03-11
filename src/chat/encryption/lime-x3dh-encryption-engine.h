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

#ifndef _L_LIME_X3DH_ENCRYPTION_ENGINE_H_
#define _L_LIME_X3DH_ENCRYPTION_ENGINE_H_

#include "belle-sip/belle-sip.h"
#include "belle-sip/http-listener.h"
#include "core/core-listener.h"
#include "encryption-engine.h"
#include "lime/lime.hpp"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LimeX3dhEncryptionEngine : public EncryptionEngine, public CoreListener, private LimeX3dhUtils {
public:
	LimeX3dhEncryptionEngine(const std::string &db_access, const std::shared_ptr<Core> core);

	~LimeX3dhEncryptionEngine();

	// EncryptionEngine overrides

	ChatMessageModifier::Result processIncomingMessage(const std::shared_ptr<ChatMessage> &message,
	                                                   int &errorCode) override;

	ChatMessageModifier::Result processOutgoingMessage(const std::shared_ptr<ChatMessage> &message,
	                                                   int &errorCode) override;

	void generateFileTransferKey(const std::shared_ptr<AbstractChatRoom> &ChatRoom,
	                             const std::shared_ptr<ChatMessage> &message,
	                             const std::shared_ptr<FileTransferContent> &fileTransferContent) override;

	int downloadingFile(const std::shared_ptr<ChatMessage> &message,
	                    size_t offset,
	                    const uint8_t *buffer,
	                    size_t size,
	                    uint8_t *decrypted_buffer,
	                    const std::shared_ptr<FileTransferContent> &fileTransferContent) override;

	int uploadingFile(const std::shared_ptr<ChatMessage> &message,
	                  size_t offset,
	                  const uint8_t *buffer,
	                  size_t *size,
	                  uint8_t *encrypted_buffer,
	                  const std::shared_ptr<FileTransferContent> &fileTransferContent) override;

	int cancelFileTransfer(const std::shared_ptr<FileTransferContent> &fileTransferContent) override;

	void mutualAuthentication(MSZrtpContext *zrtpContext,
	                          const std::shared_ptr<SalMediaDescription> &localMediaDescription,
	                          const std::shared_ptr<SalMediaDescription> &remoteMediaDescription,
	                          LinphoneCallDir direction) override;

	void authenticationVerified(MSZrtpContext *zrtpContext,
	                            const std::shared_ptr<SalMediaDescription> &remoteMediaDescription,
	                            const char *peerDeviceId) override;

	void authenticationRejected(const char *peerDeviceId) override;

	void addSecurityEventInChatrooms(const std::shared_ptr<Address> &peerDeviceAddr,
	                                 ConferenceSecurityEvent::SecurityEventType securityEventType) override;

	std::shared_ptr<ConferenceSecurityEvent> onDeviceAdded(const std::shared_ptr<Address> &newDeviceAddr,
	                                                       std::shared_ptr<Participant> participant,
	                                                       const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                                       ChatRoom::SecurityLevel currentSecurityLevel) override;

	bool isEncryptionEnabledForFileTransfer(const std::shared_ptr<AbstractChatRoom> &ChatRoom) override;
	AbstractChatRoom::SecurityLevel getSecurityLevel(const std::string &deviceId) const override;
	AbstractChatRoom::SecurityLevel getSecurityLevel(const std::list<std::string> &deviceIds) const override;
	EncryptionEngine::EngineType getEngineType() override;
	std::list<EncryptionParameter> getEncryptionParameters(const std::shared_ptr<Account> &account) override;

	// CoreListener overrides

	void onNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) override;

	void onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
	                                       LinphoneRegistrationState state,
	                                       const std::string &message) override;

	void onServerUrlChanged(std::shared_ptr<Account> &account, const std::string &limeServerUrl) override;

	void staleSession(const std::string localDeviceId, const std::string peerDeviceId) override;

	void setTestForceDecryptionFailureFlag(bool flag) override;

	void rawEncrypt(
	    const std::string &localDeviceId,
	    const std::list<std::string> &recipientDevices,
	    const std::vector<uint8_t> &plainMessage,
	    const std::vector<uint8_t> &associatedData,
	    const std::function<void(const bool status, std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts)>
	        &callback) const override;

	bool rawDecrypt(const std::string &localDeviceId,
	                const std::string &senderDeviceId,
	                const std::vector<uint8_t> &associatedData,
	                const std::vector<uint8_t> &cipherText,
	                std::vector<uint8_t> &plainText) const override;

	void createLimeUser(std::shared_ptr<Account> &account, const std::string &gruu) override;

	bool participantListRequired() const override;

private:
	lime::limeCallback setLimeUserCreationCallback(const std::string &localDeviceId, std::shared_ptr<Account> &account);
	void update(const std::string localDeviceId);
	std::vector<lime::CurveId>
	getAllConfiguredAlgos(); /**< return a vector with all supported algorithm - from all users, in no specific order */
	std::shared_ptr<lime::LimeManager>
	    limeManager;         /**< the actual lime manager - only one is intanciated, it will manage all lime users */
	lime::CurveId coreCurve; /**< default base algo setting found in core, this is deprecated so it can store one
	                            algorithm only */
	std::unordered_map<std::string, std::vector<lime::CurveId>>
	    usersAlgos;            /**< each user(GRUU) maps to an ordered list of supported base algorithms */
	bool forceFailure = false; /**< test purpose */
};

LINPHONE_END_NAMESPACE

#endif // _L_LIME_X3DH_ENCRYPTION_ENGINE_H_
