/*
 * chat-room-params.cpp
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

#include "chat-room-params.h"
#include "chat-room.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ChatRoomParams::ChatRoomParams() {
	mChatRoomImpl =	ChatRoomImpl::Basic;
	mChatRoomEncryptionImpl = ChatRoomEncryptionImpl::None;
	mEncrypted = false;
	mGroup = false;
	mRtt = false;
}

ChatRoomParams::ChatRoomParams(const ChatRoomParams &params) {
	mChatRoomImpl = params.mChatRoomImpl;
	mChatRoomEncryptionImpl = params.mChatRoomEncryptionImpl;
	mEncrypted = params.mEncrypted;
	mGroup = params.mGroup;
	mRtt = params.mRtt;
}

ChatRoomParams::ChatRoomParams(bool encrypted, bool group, ChatRoomImpl impl)
	: mChatRoomImpl(impl), mEncrypted(encrypted), mGroup(group) {
	if (encrypted) {
		mChatRoomEncryptionImpl = ChatRoomEncryptionImpl::Lime;
	}
}

ChatRoomParams::ChatRoomImpl ChatRoomParams::getChatRoomImpl() const { return mChatRoomImpl; }

ChatRoomParams::ChatRoomEncryptionImpl ChatRoomParams::getChatRoomEncryptionImpl() const { return mChatRoomEncryptionImpl; }

bool ChatRoomParams::isEncrypted() const { return mEncrypted; }

bool ChatRoomParams::isGroup() const { return mGroup; }

bool ChatRoomParams::isRealTimeText() const { return mRtt; }


void ChatRoomParams::setChatRoomImpl(ChatRoomParams::ChatRoomImpl impl) { mChatRoomImpl = impl; }

void ChatRoomParams::setChatRoomEncryptionImpl(ChatRoomParams::ChatRoomEncryptionImpl impl) { mChatRoomEncryptionImpl = impl; }

void ChatRoomParams::setEncrypted(bool encrypted) {
	mEncrypted = encrypted;
	if (encrypted) {
		mChatRoomEncryptionImpl = ChatRoomEncryptionImpl::Lime;
	}
}

void ChatRoomParams::setGroup(bool group) { mGroup = group; }

void ChatRoomParams::setRealTimeText(bool rtt) { mRtt = rtt; }

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
		params->setChatRoomImpl(ChatRoomImpl::Basic);
	}
	if (capabilities & ChatRoom::Capabilities::Conference) {
		params->setGroup(true);
		params->setChatRoomImpl(ChatRoomImpl::FlexisipChat);
	}
	if (capabilities & ChatRoom::Capabilities::RealTimeText) {
		params->setRealTimeText(true);
	}
	if (capabilities & ChatRoom::Capabilities::Encrypted) {
		params->setEncrypted(true);
		params->setChatRoomEncryptionImpl(ChatRoomEncryptionImpl::Lime);
	} else {
		params->setEncrypted(false);
		params->setChatRoomEncryptionImpl(ChatRoomEncryptionImpl::None);
	}
	params->setGroup(~capabilities & ChatRoom::Capabilities::OneToOne);
	return params;
}

ChatRoom::CapabilitiesMask ChatRoomParams::toCapabilities(const std::shared_ptr<ChatRoomParams> &params) {
	ChatRoom::CapabilitiesMask mask;

	if (params->getChatRoomImpl() == ChatRoomImpl::Basic) {
		mask |= ChatRoom::Capabilities::Basic;
		mask |= ChatRoom::Capabilities::OneToOne;
	} else if (params->getChatRoomImpl() == ChatRoomImpl::FlexisipChat) {
		mask |= ChatRoom::Capabilities::Conference;
		if (!params->isGroup()) {
			mask |= ChatRoom::Capabilities::OneToOne;
		}
	}
	if (params->isEncrypted() && params->getChatRoomEncryptionImpl() != ChatRoomEncryptionImpl::None) {
		mask |= ChatRoom::Capabilities::Encrypted;
	}
	if (params->isRealTimeText()) {
		mask |= ChatRoom::Capabilities::RealTimeText;
	}
	return mask;
}

//Returns false	if there are any inconsistencies between parameters
bool ChatRoomParams::isValid() const {
	if (mEncrypted && mChatRoomEncryptionImpl != ChatRoomEncryptionImpl::Lime) {
		return false;
	}
	if (mEncrypted && mChatRoomImpl == ChatRoomImpl::Basic) {
		return false;
	}
	if (mGroup && mChatRoomImpl != ChatRoomImpl::FlexisipChat) {
		return false;
	}
	if (mRtt && mChatRoomImpl == ChatRoomImpl::FlexisipChat) {
		return false;
	}
	return true;
}

std::string ChatRoomParams::toString() const {
	std::ostringstream ss;

	ss << "Encrypted[" << mEncrypted << "];";
	ss << "Group[" << mGroup << "];";
	ss << "Rtt[" << mRtt << "];";
	ss << "Impl[";
	if (mChatRoomImpl == ChatRoomImpl::Basic)
		ss << "Basic];";
	else
		ss << "FlexisipChat];";
	ss << "EncryptionImpl[" << ((mChatRoomEncryptionImpl == ChatRoomEncryptionImpl::None) ? "None" : "Lime X3DH")  << "];";
	ss << std::endl;

	return ss.str();
}

LINPHONE_END_NAMESPACE
