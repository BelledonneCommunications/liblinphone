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

#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/chat-room.h"
#include "conference-chat-message-event.h"
#include "conference-event-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static inline ConferenceId getSafeConferenceId (const shared_ptr<const AbstractChatRoom> chatRoom) {
	return chatRoom ? chatRoom->getConferenceId() : ConferenceId();
}

// -----------------------------------------------------------------------------

class ConferenceChatMessageEventPrivate : public ConferenceEventPrivate {
public:
	shared_ptr<ChatMessage> chatMessage;
};

// -----------------------------------------------------------------------------

ConferenceChatMessageEvent::ConferenceChatMessageEvent (
	time_t creationTime,
	const shared_ptr<ChatMessage> &chatMessage
) : ConferenceEvent(
	*new ConferenceChatMessageEventPrivate,
	EventLog::Type::ConferenceChatMessage,
	creationTime,
	getSafeConferenceId(chatMessage->getChatRoom())
) {
	L_D();
	L_ASSERT(chatMessage);
	d->chatMessage = chatMessage;
}

shared_ptr<ChatMessage> ConferenceChatMessageEvent::getChatMessage () const {
	L_D();
	return d->chatMessage;
}

LINPHONE_END_NAMESPACE
