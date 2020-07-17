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

#include "chat/chat-message/notification-message-p.h"
#include "chat/chat-message/is-composing-message.h"
#include "sip-tools/sip-headers.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

IsComposingMessage::IsComposingMessage (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	IsComposing &isComposingHandler,
	bool isComposing
) : NotificationMessage(*new NotificationMessagePrivate(chatRoom, ChatMessage::Direction::Outgoing)) {
	L_D();
	Content *content = new Content();
	content->setContentType(ContentType::ImIsComposing);
	content->setBodyFromLocale(isComposingHandler.createXml(isComposing));
	addContent(content);
	d->addSalCustomHeader(PriorityHeader::HeaderName, PriorityHeader::NonUrgent);
	d->addSalCustomHeader("Expires", "0");
}

LINPHONE_END_NAMESPACE
