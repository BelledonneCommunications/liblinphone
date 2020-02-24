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

#ifndef _L_CHAT_ROOM_PARAMS_H_
#define _L_CHAT_ROOM_PARAMS_H_

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "abstract-chat-room.h"

LINPHONE_BEGIN_NAMESPACE

class ChatRoom;

class ChatRoomParams : public bellesip::HybridObject<LinphoneChatRoomParams, ChatRoomParams> {
public:

	enum class Backend{
		Basic = LinphoneChatRoomBackendBasic,
		FlexisipChat
	};
	typedef enum Backend ChatRoomBackend;

	enum class EncryptionBackend{
		None = LinphoneChatRoomEncryptionBackendNone,
		Lime
	};

	typedef enum EncryptionBackend ChatRoomEncryptionBackend;

	//casting to int to get rid of the enum compare warning.
	//Here we are comparing two enums serving the same purpose
	static_assert((int)Backend::FlexisipChat == (int)LinphoneChatRoomBackendFlexisipChat, "LinphoneChatRoomBackend and ChatRoomParams::Backend are not synchronized, fix this !");

	//casting to int to get rid of the enum compare warning.
	//Here we are comparing two enums serving the same purpose
	static_assert((int)EncryptionBackend::Lime == (int)LinphoneChatRoomEncryptionBackendLime, "LinphoneChatRoomEncryptionBackend and ChatRoomParams::EncryptionBackend are not synchronized, fix this !");

	static AbstractChatRoom::CapabilitiesMask toCapabilities(const std::shared_ptr<ChatRoomParams> &params);
	static std::shared_ptr<ChatRoomParams> fromCapabilities(AbstractChatRoom::CapabilitiesMask capabilities);
	static std::shared_ptr<ChatRoomParams> getDefaults();
	static std::shared_ptr<ChatRoomParams> getDefaults(const std::shared_ptr<Core> &core);

	//Derived HybridObject constructors have to be public to allow construction from factory-like `bellesip::HybridObject::create` method
	//Base constructor is protected	anyways to prevent unmanaged creation.
	ChatRoomParams();
	ChatRoomParams(const ChatRoomParams &other);
	//Convenience constructor
	ChatRoomParams(bool encrypted, bool group, ChatRoomBackend backend);

	ChatRoomParams *clone() const override { return new ChatRoomParams(*this); }

	bool isValid() const;
	std::string toString() const override;

	ChatRoomBackend getChatRoomBackend() const;
	ChatRoomEncryptionBackend getChatRoomEncryptionBackend() const;
	bool isEncrypted() const;
	bool isGroup() const;
	bool isRealTimeText() const;

	void setChatRoomBackend(ChatRoomBackend backend);
	void setChatRoomEncryptionBackend(ChatRoomEncryptionBackend backend);
	void setEncrypted(bool encrypted);
	void setGroup(bool group);
	void setRealTimeText(bool rtt);

protected:
	~ChatRoomParams() = default;

private:
	ChatRoomBackend mChatRoomBackend;
	ChatRoomEncryptionBackend mChatRoomEncryptionBackend;
	bool mEncrypted = false;
	bool mGroup = false; //one to one
	bool mRtt = false; //Real Time Text
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_ROOM_PARAMS_H_
