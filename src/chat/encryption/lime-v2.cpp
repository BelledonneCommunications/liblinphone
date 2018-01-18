/*
 * encryption-chat-message-modifier.cpp
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

#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "lime-v2.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatMessageModifier::Result LimeV2::processOutgoingMessage(
    const shared_ptr<ChatMessage> &message,
    int &errorCode
) {
    shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
    // Call lime processing methods
    // Test errorCode
    return ChatMessageModifier::Result::Skipped;
}

ChatMessageModifier::Result LimeV2::processIncomingMessage(
    const shared_ptr<ChatMessage> &message,
    int &errorCode
) {
    const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
    // Call lime processing methods
    // Test errorCode
    return ChatMessageModifier::Result::Skipped;
}

bool LimeV2::encryptionEnabledForFileTransferCb(
    const shared_ptr<AbstractChatRoom> &chatRoom
) {
    // Call lime processing methods
    return false;
}

void LimeV2::generateFileTransferKeyCb(
    const shared_ptr<AbstractChatRoom> &chatRoom
) {
    // Call lime processing methods
}

int LimeV2::downloadingFileCb(
    const shared_ptr<ChatMessage> &message,
    size_t *offset,
    const uint8_t *buffer,
    size_t size,
    uint8_t *decrypted_buffer
) {
    // Call lime processing methods
    return 0;
}

int LimeV2::uploadingFileCb(
    const shared_ptr<ChatMessage> &message,
    size_t *offset,
    const uint8_t *buffer,
    size_t *size,
    uint8_t *encrypted_buffer
) {
    // Call lime processing methods
    return 0;
}

LINPHONE_END_NAMESPACE
