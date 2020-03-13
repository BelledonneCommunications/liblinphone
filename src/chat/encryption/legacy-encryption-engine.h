/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_LEGACY_ENCRYPTION_ENGINE_H_
#define _L_LEGACY_ENCRYPTION_ENGINE_H_

#include "encryption-engine.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LegacyEncryptionEngine : public EncryptionEngine {
public:
	LegacyEncryptionEngine (const std::shared_ptr<Core> &core);

	ChatMessageModifier::Result processIncomingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
	ChatMessageModifier::Result processOutgoingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
	bool isEncryptionEnabledForFileTransfer (const std::shared_ptr<AbstractChatRoom> &ChatRoom) override;
	void generateFileTransferKey (const std::shared_ptr<AbstractChatRoom> &ChatRoom, const std::shared_ptr<ChatMessage> &message, FileTransferContent *fileTransferContent) override;
	int downloadingFile (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer, FileTransferContent *fileTransferContent) override;
	int uploadingFile (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encrypted_buffer, FileTransferContent *fileTransferContent) override;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LEGACY_ENCRYPTION_ENGINE_H_
