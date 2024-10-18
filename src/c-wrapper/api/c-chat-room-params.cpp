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
	linphone_conference_params_enable_chat(params, TRUE);
	linphone_conference_params_enable_group(params, FALSE);
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendBasic);
	linphone_chat_params_set_encryption_backend(chat_params, LinphoneChatRoomEncryptionBackendNone);
	linphone_chat_params_enable_rtt(chat_params, FALSE);
	linphone_chat_params_set_ephemeral_mode(chat_params, LinphoneChatRoomEphemeralModeDeviceManaged);
	linphone_chat_params_set_ephemeral_lifetime(chat_params, 0);
	return params;
}

LinphoneChatRoomParams *linphone_chat_room_params_new_and_init(LinphoneCore *core) {
	LinphoneChatRoomParams *params = linphone_chat_room_params_new_with_core(core);
	ConferenceParams::toCpp(params)->setChatDefaults();
	return params;
}

LinphoneChatRoomParams *linphone_chat_room_params_clone(const LinphoneChatRoomParams *params) {
	return linphone_conference_params_clone(params);
}

LinphoneChatRoomParams *linphone_chat_room_params_ref(LinphoneChatRoomParams *params) {
	linphone_conference_params_ref(params);
	return params;
}

void linphone_chat_room_params_unref(LinphoneChatRoomParams *params) {
	linphone_conference_params_unref(params);
}

bool_t linphone_chat_room_params_is_valid(const LinphoneChatRoomParams *params) {
	return linphone_conference_params_is_valid(params);
}

LinphoneChatRoomBackend linphone_chat_room_params_get_backend(const LinphoneChatRoomParams *params) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	return linphone_chat_params_get_backend(chat_params);
}

LinphoneChatRoomEncryptionBackend
linphone_chat_room_params_get_encryption_backend(const LinphoneChatRoomParams *params) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	return linphone_chat_params_get_encryption_backend(chat_params);
}

bool_t linphone_chat_room_params_group_enabled(const LinphoneChatRoomParams *params) {
	return linphone_conference_params_group_enabled(params);
}

LinphoneChatRoomEphemeralMode linphone_chat_room_params_get_ephemeral_mode(const LinphoneChatRoomParams *params) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	return linphone_chat_params_get_ephemeral_mode(chat_params);
}

long linphone_chat_room_params_get_ephemeral_lifetime(const LinphoneChatRoomParams *params) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	return linphone_chat_params_get_ephemeral_lifetime(chat_params);
}

bool_t linphone_chat_room_params_encryption_enabled(const LinphoneChatRoomParams *params) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	return linphone_chat_params_encryption_enabled(chat_params);
}

bool_t linphone_chat_room_params_rtt_enabled(const LinphoneChatRoomParams *params) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	return linphone_chat_params_rtt_enabled(chat_params);
}

void linphone_chat_room_params_set_backend(LinphoneChatRoomParams *params, LinphoneChatRoomBackend backend) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_set_backend(chat_params, backend);
}

void linphone_chat_room_params_set_encryption_backend(LinphoneChatRoomParams *params,
                                                      LinphoneChatRoomEncryptionBackend backend) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_set_encryption_backend(chat_params, backend);
}

void linphone_chat_room_params_enable_group(LinphoneChatRoomParams *params, bool_t group) {
	linphone_conference_params_enable_group(params, group);
}

void linphone_chat_room_params_set_ephemeral_mode(LinphoneChatRoomParams *params, LinphoneChatRoomEphemeralMode mode) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_set_ephemeral_mode(chat_params, mode);
}

void linphone_chat_room_params_set_ephemeral_lifetime(LinphoneChatRoomParams *params, long lifetime) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_set_ephemeral_lifetime(chat_params, lifetime);
}

void linphone_chat_room_params_enable_encryption(LinphoneChatRoomParams *params, bool_t encrypted) {
	linphone_conference_params_set_security_level(params, encrypted ? LinphoneConferenceSecurityLevelEndToEnd
	                                                                : LinphoneConferenceSecurityLevelNone);
}

void linphone_chat_room_params_enable_rtt(LinphoneChatRoomParams *params, bool_t rtt) {
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_enable_rtt(chat_params, rtt);
}

const char *linphone_chat_room_params_get_subject(const LinphoneChatRoomParams *params) {
	return linphone_conference_params_get_subject(params);
}

void linphone_chat_room_params_set_subject(LinphoneChatRoomParams *params, const char *subject) {
	linphone_conference_params_set_subject(params, subject);
}
