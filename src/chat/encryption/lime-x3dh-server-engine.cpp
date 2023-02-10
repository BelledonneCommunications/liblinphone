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

#include "bctoolbox/crypto.h"
#include <bctoolbox/defs.h>

#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "chat/chat-room/client-group-chat-room.h"
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "content/content-manager.h"
#include "content/header/header-param.h"
#include "conference/participant.h"
#include "conference/participant-device.h"
#include "core/core.h"
#include "factory/factory.h"
#include "c-wrapper/c-wrapper.h"
#include "lime-x3dh-server-engine.h"
#include "private.h"
#include "bctoolbox/exception.hh"


using namespace std;

LINPHONE_BEGIN_NAMESPACE


// -----------------------------------------------  LimeX3dhEncryptionServerEngine

LimeX3dhEncryptionServerEngine::LimeX3dhEncryptionServerEngine (const shared_ptr<Core> core) : EncryptionEngine(core) {
	engineType = EncryptionEngine::EngineType::LimeX3dhServer;
	lInfo() << "[LIME][server] instanciate a LimeX3dhEncryptionServer engine "<<this;
}

LimeX3dhEncryptionServerEngine::~LimeX3dhEncryptionServerEngine () {
	lInfo()<<"[LIME][server] destroy LimeX3dhEncryptionServer engine "<<this;
}

ChatMessageModifier::Result LimeX3dhEncryptionServerEngine::processOutgoingMessage (
	const std::shared_ptr<ChatMessage> &message,
	UNUSED(int &errorCode)) {
	// We use a shared_ptr here due to non synchronism with the lambda in the encrypt method
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &toDeviceId = message->getToAddress().asString();
	const Content *internalContent;

	// Check if chatroom is encrypted or not
	if (chatRoom->getCapabilities() & ChatRoom::Capabilities::Encrypted) {
		lInfo() << "[LIME][server] this chatroom is encrypted, proceed to encrypt outgoing message";
	} else {
		lInfo() << "[LIME][server] this chatroom is not encrypted, no need to encrypt outgoing message";
		return ChatMessageModifier::Result::Skipped;
	}

	if (!message->getInternalContent().isEmpty())
		internalContent = &(message->getInternalContent());
	else
		internalContent = message->getContents().front();

	// Check if the message is encrypted
	if (!isMessageEncrypted(internalContent)) {
		return ChatMessageModifier::Result::Skipped;
	}

	list<Content> contentsList = ContentManager::multipartToContentList(*internalContent);
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
		lError() << "[LIME][server] this message doesn't contain the cipher key for participant " << toDeviceId;
		return ChatMessageModifier::Result::Error;
	}

	/* FIXME: to preserve backward compatibility with liblinphone <= 5.1, we must keep using an fixed multipart boundary.
	 * Remove this awful stuff after March 2023. */
	const char *harcodedBoundary = "---------------------------14737809831466499882746641449";
	Content finalContent = ContentManager::contentListToMultipart(contents, harcodedBoundary, true);
	/* Set the original ContentType, but we need to set the new boundary parameter for the new forged multipart. */
	string boundary = finalContent.getContentType().getParameter("boundary").getValue();
	finalContent.setContentType(internalContent->getContentType());
	finalContent.getContentType().removeParameter("boundary");
	finalContent.getContentType().addParameter("boundary", boundary);
	message->setInternalContent(finalContent);
	return ChatMessageModifier::Result::Done;
}

EncryptionEngine::EngineType LimeX3dhEncryptionServerEngine::getEngineType () {
	return engineType;
}

bool LimeX3dhUtils::isMessageEncrypted(const Content *internalContent) {
	const ContentType & incomingContentType = internalContent->getContentType();
	ContentType expectedContentType = ContentType::Encrypted;
	
	if (incomingContentType == expectedContentType){
		string protocol = incomingContentType.getParameter("protocol").getValue();
		if (protocol == "\"application/lime\""){
			return true;
		}else if (protocol.empty()){
			lWarning() << "Accepting possible legacy lime message.";
			return true;
		}
	}
	
	return false;
}

LINPHONE_END_NAMESPACE
