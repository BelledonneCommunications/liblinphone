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

#include "lime-x3dh-server-engine.h"
#include "bctoolbox/crypto.h"
#include "bctoolbox/exception.hh"
#include <bctoolbox/defs.h>

#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room.h"
#include "chat/chat-room/client-chat-room.h"
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "content/content-manager.h"
#include "content/header/header-param.h"
#include "core/core.h"
#include "factory/factory.h"
#include "lime-x3dh-server-engine.h"
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------  LimeX3dhEncryptionServerEngine

LimeX3dhEncryptionServerEngine::LimeX3dhEncryptionServerEngine(const shared_ptr<Core> core) : EncryptionEngine(core) {
	engineType = EncryptionEngine::EngineType::LimeX3dhServer;
	lInfo() << "[LIME][server] instanciate a LimeX3dhEncryptionServer engine " << this;
}

LimeX3dhEncryptionServerEngine::~LimeX3dhEncryptionServerEngine() {
	lInfo() << "[LIME][server] destroy LimeX3dhEncryptionServer engine " << this;
}

ChatMessageModifier::Result
LimeX3dhEncryptionServerEngine::processOutgoingMessage(const std::shared_ptr<ChatMessage> &message,
                                                       BCTBX_UNUSED(int &errorCode)) {
	// We use a shared_ptr here due to non synchronism with the lambda in the encrypt method
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &toDeviceId = message->getToAddress()->asStringUriOnly();
	const Content *internalContent;

	// Check if chatroom is encrypted or not
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	if (chatRoomParams->getChatParams()->isEncrypted()) {
		lInfo() << "[LIME][server] " << *chatRoom << " is encrypted, proceed to encrypt outgoing message";
	} else {
		lInfo() << "[LIME][server] " << *chatRoom << " is not encrypted, no need to encrypt outgoing message";
		return ChatMessageModifier::Result::Skipped;
	}

	if (!message->getInternalContent().isEmpty()) internalContent = &(message->getInternalContent());
	else internalContent = message->getContents().front().get();

	// Check if the message is encrypted
	if (!internalContent || !isMessageEncrypted(*internalContent)) {
		return ChatMessageModifier::Result::Skipped;
	}

	auto contentList = message->getProperty("content-list");
	list<Content> contentsList = contentList.isValid() ? contentList.getValue<list<Content>>()
	                                                   : ContentManager::multipartToContentList(*internalContent);
	list<Content *> contents;
	bool hasKey = FALSE;
	for (auto &content : contentsList) {
		if (content.getContentType() != ContentType::LimeKey) {
			contents.push_back(&content);
		} else if (content.getHeader("Content-Id").getValueWithParams() == toDeviceId) {
			contents.push_back(&content);
			hasKey = TRUE;
		}
	}

	if (!hasKey) {
		lError() << "[LIME][server] " << *chatRoom << ": message [" << message
		         << "] doesn't contain the cipher key for participant " << toDeviceId;
		return ChatMessageModifier::Result::Error;
	}

	Content finalContent = ContentManager::contentListToMultipart(contents, true);
	/* Set the original ContentType, but we need to set the new boundary parameter for the new forged multipart. */
	string boundary = finalContent.getContentType().getParameter("boundary").getValue();
	finalContent.setContentType(internalContent->getContentType());
	finalContent.getContentType().removeParameter("boundary");
	finalContent.getContentType().addParameter("boundary", boundary);
	if (linphone_core_content_encoding_supported(message->getChatRoom()->getCore()->getCCore(), "deflate")) {
		finalContent.setContentEncoding("deflate");
	} else {
		lWarning()
		    << "[LIME][server] " << *chatRoom
		    << ": Cannot use 'deflate' Content-Encoding to compress body - consider rebuilding with libz support.";
	}
	message->setInternalContent(finalContent);
	return ChatMessageModifier::Result::Done;
}

EncryptionEngine::EngineType LimeX3dhEncryptionServerEngine::getEngineType() {
	return engineType;
}

LINPHONE_END_NAMESPACE
