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

#include "linphone/api/c-chat-room-params.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-room/chat-params.h"
#include "conference/conference-params.h"
#include "core/core-p.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/wrapper_utils.h"

// =============================================================================

using namespace LinphonePrivate;

// =============================================================================

LinphoneChatRoomParams *linphone_chat_room_params_new(void) {
	return linphone_chat_room_params_new_with_core(nullptr);
}

LinphoneChatRoomParams *linphone_chat_room_params_new_with_core(LinphoneCore *core) {
	LinphoneChatRoomParams *params = linphone_conference_params_new(core);
	ConferenceParams::toCpp(params)->enableChat(true);
	ConferenceParams::toCpp(params)->setGroup(false);
	ConferenceParams::toCpp(params)->getChatParams()->setBackend(ChatParams::Backend::Basic);
	ConferenceParams::toCpp(params)->getChatParams()->setEncryptionBackend(ChatParams::EncryptionBackend::None);
	ConferenceParams::toCpp(params)->getChatParams()->setRealTimeText(false);
	ConferenceParams::toCpp(params)->getChatParams()->setEphemeralMode(AbstractChatRoom::EphemeralMode::DeviceManaged);
	ConferenceParams::toCpp(params)->getChatParams()->setEphemeralLifetime(0);
	return params;
}

LinphoneChatRoomParams *linphone_chat_room_params_new_and_init(LinphoneCore *core) {
	LinphoneChatRoomParams *params = linphone_chat_room_params_new_with_core(core);
	ConferenceParams::toCpp(params)->setChatDefaults();
	return params;
}

LinphoneChatRoomParams *linphone_chat_room_params_clone(const LinphoneChatRoomParams *params) {
	return ConferenceParams::toCpp(params)->clone()->toC();
}

LinphoneChatRoomParams *linphone_chat_room_params_ref(LinphoneChatRoomParams *params) {
	ConferenceParams::toCpp(params)->ref();
	return params;
}

void linphone_chat_room_params_unref(LinphoneChatRoomParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

bool_t linphone_chat_room_params_is_valid(const LinphoneChatRoomParams *params) {
	return ConferenceParams::toCpp(params)->isValid();
}

LinphoneChatRoomBackend linphone_chat_room_params_get_backend(const LinphoneChatRoomParams *params) {
	return static_cast<LinphoneChatRoomBackend>(
	    const_cast<ConferenceParams *>(ConferenceParams::toCpp(params))->getChatParams()->getBackend());
}

LinphoneChatRoomEncryptionBackend
linphone_chat_room_params_get_encryption_backend(const LinphoneChatRoomParams *params) {
	return static_cast<LinphoneChatRoomEncryptionBackend>(
	    const_cast<ConferenceParams *>(ConferenceParams::toCpp(params))->getChatParams()->getEncryptionBackend());
}

bool_t linphone_chat_room_params_group_enabled(const LinphoneChatRoomParams *params) {
	return ConferenceParams::toCpp(params)->isGroup();
}

LinphoneChatRoomEphemeralMode linphone_chat_room_params_get_ephemeral_mode(const LinphoneChatRoomParams *params) {
	return static_cast<LinphoneChatRoomEphemeralMode>(
	    const_cast<ConferenceParams *>(ConferenceParams::toCpp(params))->getChatParams()->getEphemeralMode());
}

long linphone_chat_room_params_get_ephemeral_lifetime(const LinphoneChatRoomParams *params) {
	return const_cast<ConferenceParams *>(ConferenceParams::toCpp(params))->getChatParams()->getEphemeralLifetime();
}

bool_t linphone_chat_room_params_encryption_enabled(const LinphoneChatRoomParams *params) {
	return const_cast<ConferenceParams *>(ConferenceParams::toCpp(params))->getChatParams()->isEncrypted();
}

bool_t linphone_chat_room_params_rtt_enabled(const LinphoneChatRoomParams *params) {
	return const_cast<ConferenceParams *>(ConferenceParams::toCpp(params))->getChatParams()->isRealTimeText();
}

void linphone_chat_room_params_set_backend(LinphoneChatRoomParams *params, LinphoneChatRoomBackend backend) {
	ConferenceParams::toCpp(params)->getChatParams()->setBackend(static_cast<ChatParams::Backend>(backend));
}

void linphone_chat_room_params_set_encryption_backend(LinphoneChatRoomParams *params,
                                                      LinphoneChatRoomEncryptionBackend backend) {
	ConferenceParams::toCpp(params)->getChatParams()->setEncryptionBackend(
	    static_cast<ChatParams::EncryptionBackend>(backend));
}

void linphone_chat_room_params_enable_group(LinphoneChatRoomParams *params, bool_t group) {
	ConferenceParams::toCpp(params)->setGroup(!!group);
}

void linphone_chat_room_params_set_ephemeral_mode(LinphoneChatRoomParams *params, LinphoneChatRoomEphemeralMode mode) {
	ConferenceParams::toCpp(params)->getChatParams()->setEphemeralMode(
	    static_cast<AbstractChatRoom::EphemeralMode>(mode));
}

void linphone_chat_room_params_set_ephemeral_lifetime(LinphoneChatRoomParams *params, long lifetime) {
	ConferenceParams::toCpp(params)->getChatParams()->setEphemeralLifetime(lifetime);
}

void linphone_chat_room_params_enable_encryption(LinphoneChatRoomParams *params, bool_t encrypted) {
	ConferenceParams::toCpp(params)->setSecurityLevel(encrypted ? ConferenceParams::SecurityLevel::EndToEnd
	                                                            : ConferenceParams::SecurityLevel::None);
}

void linphone_chat_room_params_enable_rtt(LinphoneChatRoomParams *params, bool_t rtt) {
	ConferenceParams::toCpp(params)->getChatParams()->setRealTimeText(!!rtt);
}

const char *linphone_chat_room_params_get_subject(const LinphoneChatRoomParams *params) {
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getSubject());
}

void linphone_chat_room_params_set_subject(LinphoneChatRoomParams *params, const char *subject) {
	ConferenceParams::toCpp(params)->setSubject(L_C_TO_STRING(subject));
}
