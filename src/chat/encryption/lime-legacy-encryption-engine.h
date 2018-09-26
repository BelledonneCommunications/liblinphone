/*
 * lime-legacy-encryption-engine.h
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

#ifndef _L_LIME_LEGACY_ENCRYPTION_ENGINE_H_
#define _L_LIME_LEGACY_ENCRYPTION_ENGINE_H_

#include "encryption-engine.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LimeLegacyEncryptionEngine : public EncryptionEngine {
public:
	ChatMessageModifier::Result processIncomingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
	ChatMessageModifier::Result processOutgoingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
	bool encryptionEnabledForFileTransfer (const std::shared_ptr<AbstractChatRoom> &ChatRoom) override;
	void generateFileTransferKey (const std::shared_ptr<AbstractChatRoom> &ChatRoom, const std::shared_ptr<ChatMessage> &message) override;
	int downloadingFile (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer) override;
	int uploadingFile (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encrypted_buffer) override;
	void cleanDb () override;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LIME_LEGACY_ENCRYPTION_ENGINE_H_
