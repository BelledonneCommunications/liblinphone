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

#include "buffer.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

Buffer::Buffer(const vector<uint8_t> &data) : Buffer() {
	setContent(data);
}

Buffer::Buffer(const string &data) : Buffer() {
	setStringContent(data);
}

Buffer::Buffer(std::vector<uint8_t> &&data) {
	setContent(std::move(data));
}
Buffer::Buffer(std::string &&data) {
	setStringContent(std::move(data));
}

void *Buffer::getUserData() const {
	return mUserData;
}

void Buffer::setUserData(void *ud) {
	mUserData = ud;
}

const vector<uint8_t> &Buffer::getContent() const {
	return mContent;
}

void Buffer::setContent(const vector<uint8_t> &content) {
	if (!mContent.empty()) mContent.clear();
	mContent = content;
}

const string &Buffer::getStringContent() const {
	mStringContent = string(mContent.begin(), mContent.end());
	return mStringContent;
}

void Buffer::setStringContent(const string &content) {
	if (!mContent.empty()) mContent.clear();
	mContent = vector<uint8_t>(content.begin(), content.end());
}

size_t Buffer::getSize() const {
	return mContent.size();
}

void Buffer::setSize(size_t size) {
	mContent.resize(size);
}

bool_t Buffer::isEmpty() const {
	return mContent.empty();
}

LINPHONE_END_NAMESPACE