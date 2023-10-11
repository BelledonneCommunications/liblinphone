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

#include "chat/chat-message/is-composing-message.h"
#include "chat/chat-message/notification-message-p.h"
#include "sip-tools/sip-headers.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

IsComposingMessage::IsComposingMessage(const shared_ptr<AbstractChatRoom> &chatRoom,
                                       IsComposing &isComposingHandler,
                                       bool isComposing)
    : NotificationMessage(*new NotificationMessagePrivate(chatRoom, ChatMessage::Direction::Outgoing)) {
	L_D();
	auto content = Content::create();
	content->setContentType(ContentType::ImIsComposing);
	content->setBodyFromUtf8(isComposingHandler.createXml(isComposing));
	addContent(content);
	d->addSalCustomHeader(PriorityHeader::HeaderName, PriorityHeader::NonUrgent);
	d->addSalCustomHeader("Expires", "0");
}

LINPHONE_END_NAMESPACE
