/*
 * lime-legacy-encryption-engine.cpp
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

#include "linphone/core.h"

#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "lime-legacy-encryption-engine.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

LimeLegacyEncryptionEngine::LimeLegacyEncryptionEngine (const shared_ptr<Core> &core) : EncryptionEngine(core) {}

ChatMessageModifier::Result LimeLegacyEncryptionEngine::processOutgoingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	LinphoneImEncryptionEngine *imee = linphone_core_get_im_encryption_engine(chatRoom->getCore()->getCCore());
	if (!imee)
		return ChatMessageModifier::Result::Skipped;

	LinphoneImEncryptionEngineCbsOutgoingMessageCb cbProcessOutgoingMessage =
		linphone_im_encryption_engine_cbs_get_process_outgoing_message(
			linphone_im_encryption_engine_get_callbacks(imee)
		);

	if (!cbProcessOutgoingMessage)
		return ChatMessageModifier::Result::Skipped;

	errorCode = cbProcessOutgoingMessage(imee, L_GET_C_BACK_PTR(chatRoom), L_GET_C_BACK_PTR(message));
	if (errorCode == -1)
		return ChatMessageModifier::Result::Skipped;

	if (errorCode != 0 && errorCode != 1) {
		return ChatMessageModifier::Result::Error;
	}

	if (errorCode == 1)
		return ChatMessageModifier::Result::Suspended;

	return ChatMessageModifier::Result::Done;
}

ChatMessageModifier::Result LimeLegacyEncryptionEngine::processIncomingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	LinphoneImEncryptionEngine *imee = linphone_core_get_im_encryption_engine(chatRoom->getCore()->getCCore());
	if (!imee)
		return ChatMessageModifier::Result::Skipped;

	LinphoneImEncryptionEngineCbsIncomingMessageCb cbProcessIncomingMessage =
		linphone_im_encryption_engine_cbs_get_process_incoming_message(
			linphone_im_encryption_engine_get_callbacks(imee)
		);

	if (!cbProcessIncomingMessage)
		return ChatMessageModifier::Result::Skipped;

	errorCode = cbProcessIncomingMessage(imee, L_GET_C_BACK_PTR(chatRoom), L_GET_C_BACK_PTR(message));
	if (errorCode == -1)
		return ChatMessageModifier::Result::Skipped;

	if (errorCode != 0 && errorCode != 1) {
		return ChatMessageModifier::Result::Error;
	}

	if (errorCode == 1)
		return ChatMessageModifier::Result::Suspended;

	return ChatMessageModifier::Result::Done;
}

bool LimeLegacyEncryptionEngine::encryptionEnabledForFileTransfer (const shared_ptr<AbstractChatRoom> &chatRoom) {
	LinphoneImEncryptionEngine *imee = linphone_core_get_im_encryption_engine(chatRoom->getCore()->getCCore());
	LinphoneImEncryptionEngineCbs *imee_cbs = linphone_im_encryption_engine_get_callbacks(imee);
	LinphoneImEncryptionEngineCbsIsEncryptionEnabledForFileTransferCb is_encryption_enabled_for_file_transfer_cb =
		linphone_im_encryption_engine_cbs_get_is_encryption_enabled_for_file_transfer(imee_cbs);
	if (is_encryption_enabled_for_file_transfer_cb) {
		return is_encryption_enabled_for_file_transfer_cb(imee, L_GET_C_BACK_PTR(chatRoom));
	}
	return false;
}

void LimeLegacyEncryptionEngine::generateFileTransferKey (const shared_ptr<AbstractChatRoom> &chatRoom, const shared_ptr<ChatMessage> &message) {
	LinphoneImEncryptionEngine *imee = linphone_core_get_im_encryption_engine(chatRoom->getCore()->getCCore());
	LinphoneImEncryptionEngineCbs *imee_cbs = linphone_im_encryption_engine_get_callbacks(imee);
	LinphoneImEncryptionEngineCbsGenerateFileTransferKeyCb generate_file_transfer_key_cb =
		linphone_im_encryption_engine_cbs_get_generate_file_transfer_key(imee_cbs);
	if (generate_file_transfer_key_cb) {
		generate_file_transfer_key_cb(imee, L_GET_C_BACK_PTR(chatRoom), L_GET_C_BACK_PTR(message));
	}
}

int LimeLegacyEncryptionEngine::downloadingFile (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decryptedBuffer) {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	LinphoneImEncryptionEngine *imee = linphone_core_get_im_encryption_engine(chatRoom->getCore()->getCCore());
	LinphoneImEncryptionEngineCbs *imee_cbs = linphone_im_encryption_engine_get_callbacks(imee);
	LinphoneImEncryptionEngineCbsDownloadingFileCb cb_process_downloading_file = linphone_im_encryption_engine_cbs_get_process_downloading_file(imee_cbs);
	if (cb_process_downloading_file) {
		return cb_process_downloading_file(imee, L_GET_C_BACK_PTR(message), offset, buffer, size, decryptedBuffer);
	}
	return -1;
}

int LimeLegacyEncryptionEngine::uploadingFile (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encryptedBuffer) {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	LinphoneImEncryptionEngine *imee = linphone_core_get_im_encryption_engine(chatRoom->getCore()->getCCore());
	LinphoneImEncryptionEngineCbs *imee_cbs = linphone_im_encryption_engine_get_callbacks(imee);
	LinphoneImEncryptionEngineCbsUploadingFileCb cb_process_uploading_file = linphone_im_encryption_engine_cbs_get_process_uploading_file(imee_cbs);
	if (cb_process_uploading_file) {
		return cb_process_uploading_file(imee, L_GET_C_BACK_PTR(message), offset, buffer, size, encryptedBuffer);
	}
	return -1;
}

LINPHONE_END_NAMESPACE
