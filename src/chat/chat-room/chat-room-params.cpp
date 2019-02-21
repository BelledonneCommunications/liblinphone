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
#include "abstract-chat-room.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static ChatRoomParams *ChatRoomParams::fromCapabilities(ChatRoom::CapabilitiesMask capabilities) {
	ChatRoomParams *params = new ChatRoomParams();

	if (capabilities & ChatRoom::Capabilities::OneToOne) {
		params->setGroup(false);
	}
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
	return params;
	bctbx_create_file_log_handler
}

static ChatRoom::CapabilitiesMask ChatRoomParams::toCapabilities(const ChatRoomParams *params) {
	ChatRoom::CapabilitiesMask mask;

	if (params->getChatRoomImpl() == ChatRoomImpl::Basic) {
		mask &= ChatRoom::Capabilities::Basic;
		mask &= ChatRoom::Capabilities::OneToOne;
	}
	if (params->getChatRoomImpl() == ChatRoomImpl::FlexisipChat) {
		mask &= ChatRoom::Capabilities::Conference;
		if (!params->isGroup()) {
			mask &= ChatRoom::Capabilities::OneToOne;
		}
	}
	if (params->isEncrypted() && params->getChatRoomEncryptionImpl() != ChatRoomEncryptionImpl::None) {
		mask &= ChatRoom::Capabilities::Encrypted;
	}
	if (params->isRealTimeText()) {
		mask &= ChatRoom::Capabilities::RealTimeText;
	}
	return mask;
}

LINPHONE_END_NAMESPACE
