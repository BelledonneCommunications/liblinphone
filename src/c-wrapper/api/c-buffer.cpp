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

#include "linphone/api/c-buffer.h"

#include "buffer/buffer.h"
#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneBuffer *linphone_buffer_new(void) {
	return Buffer::createCObject<Buffer>();
}

LinphoneBuffer *linphone_buffer_new_from_data(const uint8_t *data, size_t size) {
	return Buffer::createCObject<Buffer>(std::vector<uint8_t>(data, data + size));
}

LinphoneBuffer *linphone_buffer_new_from_string(const char *data) {
	return Buffer::createCObject<Buffer>(std::string(data));
}

LinphoneBuffer *linphone_buffer_ref(LinphoneBuffer *buffer) {
	Buffer::toCpp(buffer)->ref();
	return buffer;
}

void linphone_buffer_unref(LinphoneBuffer *buffer) {
	Buffer::toCpp(buffer)->unref();
}

void *linphone_buffer_get_user_data(const LinphoneBuffer *buffer) {
	return Buffer::toCpp(buffer)->getUserData();
}

void linphone_buffer_set_user_data(LinphoneBuffer *buffer, void *user_data) {
	Buffer::toCpp(buffer)->setUserData(user_data);
}

const uint8_t *linphone_buffer_get_content(const LinphoneBuffer *buffer) {
	return Buffer::toCpp(buffer)->getContent().data();
}

void linphone_buffer_set_content(LinphoneBuffer *buffer, const uint8_t *content, size_t size) {
	Buffer::toCpp(buffer)->setContent(std::vector<uint8_t>(content, content + size));
}

const char *linphone_buffer_get_string_content(const LinphoneBuffer *buffer) {
	return L_STRING_TO_C(Buffer::toCpp(buffer)->getStringContent());
}

void linphone_buffer_set_string_content(LinphoneBuffer *buffer, const char *content) {
	Buffer::toCpp(buffer)->setStringContent(std::string(content));
}

size_t linphone_buffer_get_size(const LinphoneBuffer *buffer) {
	return Buffer::toCpp(buffer)->getSize();
}

void linphone_buffer_set_size(LinphoneBuffer *buffer, size_t size) {
	Buffer::toCpp(buffer)->setSize(size);
}

bool_t linphone_buffer_is_empty(const LinphoneBuffer *buffer) {
	return Buffer::toCpp(buffer)->isEmpty();
}