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

#ifndef _L_BASIC_TO_CLIENT_GROUP_CHAT_ROOM_H_
#define _L_BASIC_TO_CLIENT_GROUP_CHAT_ROOM_H_

#include "proxy-chat-room.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class BasicToClientGroupChatRoomPrivate;
class ClientGroupChatRoom;

class LINPHONE_PUBLIC BasicToClientGroupChatRoom : public ProxyChatRoom {
public:
	BasicToClientGroupChatRoom (const std::shared_ptr<ChatRoom> &chatRoom);

	CapabilitiesMask getCapabilities () const override;

	std::shared_ptr<ChatMessage> createChatMessage () override;
	std::shared_ptr<ChatMessage> createChatMessage (const std::string &text) override;

	static void migrate (
		const std::shared_ptr<ClientGroupChatRoom> &clientGroupChatRoom,
		const std::shared_ptr<AbstractChatRoom> &chatRoom
	);

private:
	L_DECLARE_PRIVATE(BasicToClientGroupChatRoom);
	L_DISABLE_COPY(BasicToClientGroupChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_BASIC_TO_CLIENT_GROUP_CHAT_ROOM_H_
