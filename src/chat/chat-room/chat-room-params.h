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
//#include "c-wrapper/c-wrapper.h"

LINPHONE_BEGIN_NAMESPACE

class ChatRoom;

class LINPHONE_PUBLIC ChatRoomParams : public bellesip::HybridObject<LinphoneChatRoomParams, ChatRoomParams>, public std::enable_shared_from_this<ChatRoomParams> {
public:
	friend class ChatRoom;

	L_DECLARE_ENUM(ChatRoomImpl, L_ENUM_VALUES_CHAT_ROOM_IMPL);
	L_DECLARE_ENUM(ChatRoomEncryptionImpl, L_ENUM_VALUES_CHAT_ROOM_ENCRYPTION_IMPL);

	ChatRoomParams() {
		mChatRoomImpl =	ChatRoomImpl::Basic;
		mChatRoomEncryptionImpl = ChatRoomEncryptionImpl::None;
		mEncrypted = false;
		mGroup = false;
		mRtt = false;
	}

	ChatRoomParams(const ChatRoomParams &params) {
		mChatRoomImpl = params.mChatRoomImpl;
		mChatRoomEncryptionImpl = params.mChatRoomEncryptionImpl;
		mEncrypted = params.mEncrypted;
		mGroup = params.mGroup;
		mRtt = params.mRtt;
	}

	~ChatRoomParams() = default;

	ChatRoomParams *clone() const override {
		return new ChatRoomParams(*this);
	}

	ChatRoomImpl getChatRoomImpl() { return mChatRoomImpl; }
	ChatRoomEncryptionImpl getChatRoomEncryptionImpl() { return mChatRoomEncryptionImpl; }
	bool isEncrypted() { return mEncrypted; }
	bool isGroup() { return mGroup; }
	bool isRealTimeText() { return mRtt; }

	void setChatRoomImpl(ChatRoomImpl impl) { mChatRoomImpl = impl; }
	void setChatRoomEncryptionImpl(ChatRoomEncryptionImpl impl) { mChatRoomEncryptionImpl = impl; }
	void setEncrypted(bool encrypted) { mEncrypted = encrypted; }
	void setGroup(bool group) { mGroup = group; }
	void setRealTimeText(bool rtt) { mRtt = rtt; }

private:
	ChatRoomImpl mChatRoomImpl;
	ChatRoomEncryptionImpl mChatRoomEncryptionImpl;
	bool mEncrypted;
	bool mGroup;
	bool mRtt;//Real Time Text
};

LINPHONE_END_NAMESPACE

//L_DECLARE_C_HYBRID_OBJECT(ChatRoomParams);

#endif // ifndef _L_CHAT_ROOM_PARAMS_H_
