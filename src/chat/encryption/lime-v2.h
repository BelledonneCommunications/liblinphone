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

#ifndef _L_LIME_V2_H_
#define _L_LIME_V2_H_

#include "encryption-engine-listener.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LimeV2 : public EncryptionEngineListener {
public:
    ChatMessageModifier::Result processIncomingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
    ChatMessageModifier::Result processOutgoingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
    bool encryptionEnabledForFileTransferCb (const std::shared_ptr<AbstractChatRoom> &ChatRoom) override;
    void generateFileTransferKeyCb (const std::shared_ptr<AbstractChatRoom> &ChatRoom) override;
    int downloadingFileCb (const std::shared_ptr<ChatMessage> &message, size_t *offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer) override;
    int uploadingFileCb (const std::shared_ptr<ChatMessage> &message, size_t *offset, const uint8_t *buffer, size_t *size, uint8_t *encrypted_buffer) override;
};

LINPHONE_END_NAMESPACE

#endif // _L_LIME_V2_H_
