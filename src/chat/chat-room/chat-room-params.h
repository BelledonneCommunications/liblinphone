/*
 * chat-room-params.h
 * Copyright (C) 2010-2019 Belledonne Communications SARL
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

#ifndef _L_CHAT_ROOM_PARAMS_H_
#define _L_CHAT_ROOM_PARAMS_H_

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "abstract-chat-room.h"

LINPHONE_BEGIN_NAMESPACE

class ChatRoom;

class LINPHONE_PUBLIC ChatRoomParams : public bellesip::HybridObject<LinphoneChatRoomParams, ChatRoomParams> {
public:

	L_DECLARE_ENUM(ChatRoomImpl, L_ENUM_VALUES_CHAT_ROOM_IMPL);
	L_DECLARE_ENUM(ChatRoomEncryptionImpl, L_ENUM_VALUES_CHAT_ROOM_ENCRYPTION_IMPL);

	static AbstractChatRoom::CapabilitiesMask toCapabilities(const std::shared_ptr<ChatRoomParams> &params);
	static std::shared_ptr<ChatRoomParams> fromCapabilities(AbstractChatRoom::CapabilitiesMask capabilities);
	static std::shared_ptr<ChatRoomParams> getDefaults();
	static std::shared_ptr<ChatRoomParams> getDefaults(const std::shared_ptr<Core> &core);

	//Derived HybridObject constructors have to be public to allow construction from factory-like `bellesip::HybridObject::create` method
	//Base constructor is protected	anyways to prevent unmanaged creation.
	ChatRoomParams();
	ChatRoomParams(const ChatRoomParams &params);
	//Convenience constructor
	ChatRoomParams(bool encrypted, bool group, ChatRoomImpl impl);

	bool isValid() const;
	std::string toString() const;

	ChatRoomImpl getChatRoomImpl() const;
	ChatRoomEncryptionImpl getChatRoomEncryptionImpl() const;
	bool isEncrypted() const;
	bool isGroup() const;
	bool isRealTimeText() const;

	void setChatRoomImpl(ChatRoomImpl impl);
	void setChatRoomEncryptionImpl(ChatRoomEncryptionImpl impl);
	void setEncrypted(bool encrypted);
	void setGroup(bool group);
	void setRealTimeText(bool rtt);

protected:
	~ChatRoomParams() = default;

private:
	ChatRoomImpl mChatRoomImpl;
	ChatRoomEncryptionImpl mChatRoomEncryptionImpl;
	bool mEncrypted = false;
	bool mGroup = false; //one to one
	bool mRtt = false; //Real Time Text
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_ROOM_PARAMS_H_
