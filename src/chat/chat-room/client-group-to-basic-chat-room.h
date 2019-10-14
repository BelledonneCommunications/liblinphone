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

#ifndef _L_CLIENT_GROUP_TO_BASIC_CHAT_ROOM_H_
#define _L_CLIENT_GROUP_TO_BASIC_CHAT_ROOM_H_

#include "proxy-chat-room.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientGroupToBasicChatRoomPrivate;

class LINPHONE_PUBLIC ClientGroupToBasicChatRoom : public ProxyChatRoom {
public:
	ClientGroupToBasicChatRoom (const std::shared_ptr<ChatRoom> &chatRoom);

	bool addParticipant (
		const IdentityAddress &participantAddress,
		const CallSessionParams *params,
		bool hasMedia
	) override;
	bool addParticipants (
		const std::list<IdentityAddress> &addresses,
		const CallSessionParams *params,
		bool hasMedia
	) override;

private:
	L_DECLARE_PRIVATE(ClientGroupToBasicChatRoom);
	L_DISABLE_COPY(ClientGroupToBasicChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CLIENT_GROUP_TO_BASIC_CHAT_ROOM_H_
