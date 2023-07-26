/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "linphone/api/c-chat-params.h"
#include "chat/chat-room/chat-params.h"

using namespace LinphonePrivate;

LinphoneChatParams *linphone_chat_params_new() {
	LinphoneChatParams *params = ChatParams::createCObject();
	return params;
}

LinphoneChatParams *linphone_chat_params_ref(LinphoneChatParams *params) {
	ChatParams::toCpp(params)->ref();
	return params;
}

void linphone_chat_params_unref(LinphoneChatParams *params) {
	ChatParams::toCpp(params)->unref();
}

void linphone_chat_params_free(LinphoneChatParams *params) {
	ChatParams::toCpp(params)->unref();
}

LinphoneChatParams *linphone_chat_params_clone(const LinphoneChatParams *params) {
	return static_cast<ChatParams *>(ChatParams::toCpp(params)->clone())->toC();
}

LinphoneChatRoomBackend linphone_chat_params_get_backend(const LinphoneChatParams *params) {
	return static_cast<LinphoneChatRoomBackend>(ChatParams::toCpp(params)->getBackend());
}

LinphoneChatRoomEncryptionBackend linphone_chat_params_get_encryption_backend(const LinphoneChatParams *params) {
	return static_cast<LinphoneChatRoomEncryptionBackend>(ChatParams::toCpp(params)->getEncryptionBackend());
}

LinphoneChatRoomEphemeralMode linphone_chat_params_get_ephemeral_mode(const LinphoneChatParams *params) {
	return static_cast<LinphoneChatRoomEphemeralMode>(ChatParams::toCpp(params)->getEphemeralMode());
}

long linphone_chat_params_get_ephemeral_lifetime(const LinphoneChatParams *params) {
	return ChatParams::toCpp(params)->getEphemeralLifetime();
}

bool_t linphone_chat_params_encryption_enabled(const LinphoneChatParams *params) {
	return ChatParams::toCpp(params)->isEncrypted();
}

bool_t linphone_chat_params_rtt_enabled(const LinphoneChatParams *params) {
	return ChatParams::toCpp(params)->isRealTimeText();
}

void linphone_chat_params_set_backend(LinphoneChatParams *params, LinphoneChatRoomBackend backend) {
	ChatParams::toCpp(params)->setBackend(static_cast<ChatParams::Backend>(backend));
}

void linphone_chat_params_set_encryption_backend(LinphoneChatParams *params,
                                                 LinphoneChatRoomEncryptionBackend backend) {
	ChatParams::toCpp(params)->setEncryptionBackend(static_cast<ChatParams::EncryptionBackend>(backend));
}

void linphone_chat_params_set_ephemeral_mode(LinphoneChatParams *params, LinphoneChatRoomEphemeralMode mode) {
	ChatParams::toCpp(params)->setEphemeralMode(static_cast<AbstractChatRoom::EphemeralMode>(mode));
}

void linphone_chat_params_set_ephemeral_lifetime(LinphoneChatParams *params, long lifetime) {
	ChatParams::toCpp(params)->setEphemeralLifetime(lifetime);
}

void linphone_chat_params_enable_rtt(LinphoneChatParams *params, bool_t rtt) {
	ChatParams::toCpp(params)->setRealTimeText(!!rtt);
}
