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

#include "chat-room-params.h"
#include "chat-room.h"
#include "logger/logger.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatRoomParams::ChatRoomParams() {
	mChatRoomBackend =	ChatRoomBackend::Basic;
	mChatRoomEncryptionBackend = ChatRoomEncryptionBackend::None;
	mEncrypted = false;
	mGroup = false;
	mRtt = false;
	mSubject = "";
}

ChatRoomParams::ChatRoomParams(bool encrypted, bool group, ChatRoomBackend backend)
	: ChatRoomParams("", encrypted, group, backend) {}

ChatRoomParams::ChatRoomParams(string subject, bool encrypted, bool group, ChatRoomBackend backend)
	: mChatRoomBackend(backend), mEncrypted(encrypted), mGroup(group), mSubject(subject) {
	if (encrypted) {
		mChatRoomEncryptionBackend = ChatRoomEncryptionBackend::Lime;
	}
}

ChatRoomParams::ChatRoomParams(const ChatRoomParams &other) : HybridObject(other) {
	mChatRoomBackend = other.mChatRoomBackend;
	mChatRoomEncryptionBackend = other.mChatRoomEncryptionBackend;
	mEncrypted = other.mEncrypted;
	mGroup = other.mGroup;
	mRtt = other.mRtt;
	mSubject = other.mSubject;
}

ChatRoomParams::ChatRoomBackend ChatRoomParams::getChatRoomBackend() const { return mChatRoomBackend; }

ChatRoomParams::ChatRoomEncryptionBackend ChatRoomParams::getChatRoomEncryptionBackend() const { return mChatRoomEncryptionBackend; }

bool ChatRoomParams::isEncrypted() const { return mEncrypted; }

bool ChatRoomParams::isGroup() const { return mGroup; }

bool ChatRoomParams::isRealTimeText() const { return mRtt; }

const string& ChatRoomParams::getSubject() const { return mSubject; }

void ChatRoomParams::setChatRoomBackend(ChatRoomParams::ChatRoomBackend backend) { mChatRoomBackend = backend; }

void ChatRoomParams::setChatRoomEncryptionBackend(ChatRoomParams::ChatRoomEncryptionBackend backend) { mChatRoomEncryptionBackend = backend; }

void ChatRoomParams::setEncrypted(bool encrypted) {
	mEncrypted = encrypted;
	if (encrypted) {
		mChatRoomEncryptionBackend = ChatRoomEncryptionBackend::Lime;
		mChatRoomBackend = ChatRoomBackend::FlexisipChat;
	}
}

void ChatRoomParams::setGroup(bool group) {
	mGroup = group;
	if (group) {
		mChatRoomBackend = ChatRoomBackend::FlexisipChat;
	}
}

void ChatRoomParams::setRealTimeText(bool rtt) { mRtt = rtt; }

void ChatRoomParams::setSubject(string subject) { mSubject = subject; }

shared_ptr<ChatRoomParams> ChatRoomParams::getDefaults() {
	return ChatRoomParams::create();
}

//Later define default params from core specs (group chat enabled, lime enabled, ...)
shared_ptr<ChatRoomParams> ChatRoomParams::getDefaults(const std::shared_ptr<Core> &core) {
	return getDefaults();
}

shared_ptr<ChatRoomParams> ChatRoomParams::fromCapabilities(ChatRoom::CapabilitiesMask capabilities) {
	auto params = ChatRoomParams::create();

	if (capabilities & ChatRoom::Capabilities::Basic) {
		params->setGroup(false);
		params->setChatRoomBackend(ChatRoomBackend::Basic);
	}
	if (capabilities & ChatRoom::Capabilities::Conference) {
		params->setGroup(true);
		params->setChatRoomBackend(ChatRoomBackend::FlexisipChat);
	}
	if (capabilities & ChatRoom::Capabilities::RealTimeText) {
		params->setRealTimeText(true);
	}
	if (capabilities & ChatRoom::Capabilities::Encrypted) {
		params->setEncrypted(true);
		params->setChatRoomEncryptionBackend(ChatRoomEncryptionBackend::Lime);
	} else {
		params->setEncrypted(false);
		params->setChatRoomEncryptionBackend(ChatRoomEncryptionBackend::None);
	}
	params->setGroup(~capabilities & ChatRoom::Capabilities::OneToOne);
	return params;
}

ChatRoom::CapabilitiesMask ChatRoomParams::toCapabilities(const std::shared_ptr<ChatRoomParams> &params) {
	ChatRoom::CapabilitiesMask mask;

	if (params->getChatRoomBackend() == ChatRoomBackend::Basic) {
		mask |= ChatRoom::Capabilities::Basic;
		mask |= ChatRoom::Capabilities::OneToOne;
	} else if (params->getChatRoomBackend() == ChatRoomBackend::FlexisipChat) {
		mask |= ChatRoom::Capabilities::Conference;
		if (!params->isGroup()) {
			mask |= ChatRoom::Capabilities::OneToOne;
		}
	}
	if (params->isEncrypted() && params->getChatRoomEncryptionBackend() != ChatRoomEncryptionBackend::None) {
		mask |= ChatRoom::Capabilities::Encrypted;
	}
	if (params->isRealTimeText()) {
		mask |= ChatRoom::Capabilities::RealTimeText;
	}
	return mask;
}

//Returns false	if there are any inconsistencies between parameters
bool ChatRoomParams::isValid() const {
	if (mEncrypted && mChatRoomEncryptionBackend != ChatRoomEncryptionBackend::Lime) {
		lError() << "Currently only Lime encryption backend is supported";
		return false;
	}
	if (mEncrypted && mChatRoomBackend == ChatRoomBackend::Basic) {
		lError() << "Encryption isn't supported with Basic backend";
		return false;
	}
	if (mGroup && mChatRoomBackend != ChatRoomBackend::FlexisipChat) {
		lError() << "FlexisipChat backend must be used when group is enabled";
		return false;
	}
	if (mRtt && mChatRoomBackend == ChatRoomBackend::FlexisipChat) {
		lError() << "Real time text chat room isn't compatible with FlexisipChat backend";
		return false;
	}
	if (mSubject.empty() && mChatRoomBackend == ChatRoomBackend::FlexisipChat) {
		lError() << "You must set a non empty subject when using the FlexisipChat backend";
		return false;
	}
	return true;
}

std::string ChatRoomParams::toString() const {
	std::ostringstream ss;

	ss << "Subject[" << mSubject << "];";
	ss << "Encrypted[" << mEncrypted << "];";
	ss << "Group[" << mGroup << "];";
	ss << "Rtt[" << mRtt << "];";
	ss << "Backend[";
	if (mChatRoomBackend == ChatRoomBackend::Basic)
		ss << "Basic];";
	else
		ss << "FlexisipChat];";
	ss << "EncryptionBackend[" << ((mChatRoomEncryptionBackend == ChatRoomEncryptionBackend::None) ? "None" : "Lime X3DH")  << "];";
	return ss.str();
}

LINPHONE_END_NAMESPACE
