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

#include "ekt-info.h"

#include "buffer/buffer.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

shared_ptr<Address> EktInfo::getFrom() const {
	return mFrom;
}

void EktInfo::setFrom(const Address &from) {
	mFrom = from.clone()->toSharedPtr();
}

uint16_t EktInfo::getSSpi() const {
	return mSSpi;
}

void EktInfo::setSSpi(uint16_t sSpi) {
	mSSpi = sSpi;
}

const vector<uint8_t> &EktInfo::getCSpi() const {
	return mCSpi;
}

void EktInfo::setCSpi(const vector<uint8_t> &cSpi) {
	mCSpi = cSpi;
}

shared_ptr<Dictionary> EktInfo::getCiphers() const {
	return mCiphers;
}

void EktInfo::setCiphers(const shared_ptr<Dictionary> &ciphers) {
	mCiphers = ciphers;
}

void EktInfo::addCipher(const string &to, const vector<uint8_t> &cipher) {
	if (mCiphers == nullptr) mCiphers = Dictionary::create();
	auto buffer = Buffer::create(cipher);
	mCiphers->setProperty(to, buffer);
}

LINPHONE_END_NAMESPACE