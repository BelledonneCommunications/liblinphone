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

// TODO: Remove me later.
#include "private.h"

#include "chat/chat-message/chat-message.h"
#include "content/content-type.h"
#include "content/header/header.h"
#include "content/content-manager.h"
#include "content/file-transfer-content.h"

#include "multipart-chat-message-modifier.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatMessageModifier::Result MultipartChatMessageModifier::encode (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
	if (message->getContents().size() <= 1)
		return ChatMessageModifier::Result::Skipped;

	Content content = ContentManager::contentListToMultipart(message->getContents());
	message->setInternalContent(content);

	return ChatMessageModifier::Result::Done;
}

ChatMessageModifier::Result MultipartChatMessageModifier::decode (const shared_ptr<ChatMessage> &message, int &errorCode) {
	if (message->getInternalContent().getContentType().isMultipart()) {
		for (Content &c : ContentManager::multipartToContentList(message->getInternalContent())) {
			Content *content;
			if (c.getContentType() == ContentType::FileTransfer) {
				content = new FileTransferContent();
				content->setContentType(c.getContentType());
				content->setContentDisposition(c.getContentDisposition());
				content->setContentEncoding(c.getContentEncoding());
				for (const Header &header : c.getHeaders()) {
					content->addHeader(header);
				}
				content->setBodyFromUtf8(c.getBodyAsUtf8String());
			} else {
				content = new Content(c);
			}
			message->addContent(content);
		}
		return ChatMessageModifier::Result::Done;
	}
	return ChatMessageModifier::Result::Skipped;
}

LINPHONE_END_NAMESPACE
