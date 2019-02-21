/*
 * c-chat-room-params.cpp
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

#include "linphone/api/c-chat-room-params.h"
#include "chat/chat-room/chat-room-params.h"

// =============================================================================

using namespace LinphonePrivate;

// =============================================================================

LinphoneChatRoomParams *linphone_chat_room_params_new(void) {
	return (new ChatRoomParams())->toC();
}

LinphoneChatRoomParams *linphone_chat_room_params_ref(LinphoneChatRoomParams *params) {
	ChatRoomParams::toCpp(params)->ref();
	return params;
}

void linphone_chat_room_params_unref(LinphoneChatRoomParams *params) {
	ChatRoomParams::toCpp(params)->unref();
}

LinphoneChatRoomImpl linphone_chat_room_params_get_impl(LinphoneChatRoomParams *params) {
	return static_cast<LinphoneChatRoomImpl>(ChatRoomParams::toCpp(params)->getChatRoomImpl());
}

LinphoneChatRoomEncryptionImpl linphone_chat_room_params_get_encryption_impl(LinphoneChatRoomParams *params) {
	return static_cast<LinphoneChatRoomEncryptionImpl>(ChatRoomParams::toCpp(params)->getChatRoomEncryptionImpl());
}

bool_t linphone_chat_room_params_group_enabled(LinphoneChatRoomParams *params) {
	return ChatRoomParams::toCpp(params)->isGroup();
}

bool_t linphone_chat_room_params_encryption_enabled(LinphoneChatRoomParams *params) {
	return ChatRoomParams::toCpp(params)->isEncrypted();
}

bool_t linphone_chat_room_params_rtt_enabled(LinphoneChatRoomParams *params) {
	return ChatRoomParams::toCpp(params)->isRealTimeText();
}

void linphone_chat_room_params_set_impl(LinphoneChatRoomParams *params, LinphoneChatRoomImpl impl) {
	ChatRoomParams::toCpp(params)->setChatRoomImpl(static_cast<ChatRoomParams::ChatRoomImpl>(impl));
}

void linphone_chat_room_params_set_encryption_impl(LinphoneChatRoomParams *params, LinphoneChatRoomEncryptionImpl impl) {
	ChatRoomParams::toCpp(params)->setChatRoomEncryptionImpl(static_cast<ChatRoomParams::ChatRoomEncryptionImpl>(impl));
}

void linphone_chat_room_params_enable_group(LinphoneChatRoomParams *params, bool_t group) {
	ChatRoomParams::toCpp(params)->setGroup(!!group);
}

void linphone_chat_room_params_enable_encryption(LinphoneChatRoomParams *params, bool_t encrypted) {
	ChatRoomParams::toCpp(params)->setEncrypted(!!encrypted);
}

void linphone_chat_room_params_enable_rtt(LinphoneChatRoomParams *params, bool_t rtt) {
	ChatRoomParams::toCpp(params)->setRealTimeText(!!rtt);
}
