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

#ifndef _L_REAL_TIME_TEXT_CHAT_ROOM_P_H_
#define _L_REAL_TIME_TEXT_CHAT_ROOM_P_H_

#include "chat/chat-room/basic-chat-room-p.h"
#include "chat/chat-room/real-time-text-chat-room.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class RealTimeTextChatRoomPrivate : public BasicChatRoomPrivate {
public:
	struct Character {
		uint32_t value;
		bool hasBeenRead;
	};

	void realtimeTextReceived (uint32_t character, const std::shared_ptr<Call> &call);
	void sendChatMessage (const std::shared_ptr<ChatMessage> &chatMessage) override;
	void setCall (const std::shared_ptr<Call> &value) { call = value; }

	std::weak_ptr<Call> call;
	std::list<Character> receivedRttCharacters;
	std::shared_ptr<ChatMessage> pendingMessage = nullptr;

private:
	L_DECLARE_PUBLIC(RealTimeTextChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REAL_TIME_TEXT_CHAT_ROOM_P_H_
