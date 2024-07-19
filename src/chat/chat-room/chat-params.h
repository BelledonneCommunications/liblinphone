/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_CHAT_PARAMS_H_
#define _L_CHAT_PARAMS_H_

#include "belle-sip/object++.hh"

#include "chat/chat-room/abstract-chat-room.h"

LINPHONE_BEGIN_NAMESPACE

class Core;

class LINPHONE_PUBLIC ChatParams : public bellesip::HybridObject<LinphoneChatParams, ChatParams> {

public:
	enum class _Backend { Basic = LinphoneChatRoomBackendBasic, FlexisipChat };
	typedef enum _Backend Backend;

	enum class _EncryptionBackend { None = LinphoneChatRoomEncryptionBackendNone, Lime };

	typedef enum _EncryptionBackend EncryptionBackend;

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert((int)Backend::FlexisipChat == (int)LinphoneChatRoomBackendFlexisipChat,
	              "LinphoneChatRoomBackend and ChatParams::Backend are not synchronized, fix this !");

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert(
	    (int)EncryptionBackend::Lime == (int)LinphoneChatRoomEncryptionBackendLime,
	    "LinphoneChatRoomEncryptionBackend and ChatParams::EncryptionBackend are not synchronized, fix this !");

	ChatParams() = default;
	ChatParams(const ChatParams &other) = default;

	void setChatDefaults(const std::shared_ptr<Core> &core);

	ChatParams *clone() const override {
		return new ChatParams(*this);
	}

	virtual ~ChatParams() = default;

	bool isEncrypted() const;
	void updateSecurityParams(bool encrypted);

	Backend getBackend() const;
	EncryptionBackend getEncryptionBackend() const;
	bool isRealTimeText() const;
	long getEphemeralLifetime() const;
	AbstractChatRoom::EphemeralMode getEphemeralMode() const;
	bool ephemeralEnabled() const;
	bool ephemeralAllowed() const;

	void setBackend(Backend backend);
	void setEncryptionBackend(EncryptionBackend backend);
	void setRealTimeText(bool rtt);
	void setEphemeralMode(AbstractChatRoom::EphemeralMode mode);
	void setEphemeralLifetime(long lifetime);
	void enableEphemeral(bool ephem);
	void allowEphemeral(bool ephem);

	bool isValid() const;

private:
	Backend mBackend = Backend::FlexisipChat;
	EncryptionBackend mEncryptionBackend = EncryptionBackend::None;
	bool mRtt = false; // Real Time Text
	bool mEnableEphemeral = false;
	bool mAllowEphemeral = false;
	AbstractChatRoom::EphemeralMode mEphemeralMode = AbstractChatRoom::EphemeralMode::DeviceManaged;
	long mEphemeralLifetime = 0;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_PARAMS_H_
