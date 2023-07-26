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

#include "conference-id.h"
#include "conference.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceId::ConferenceId() {
}

ConferenceId::ConferenceId(Address &&pAddress, Address &&lAddress) {
	peerAddress = Address::create(std::move(pAddress));
	localAddress = Address::create(std::move(lAddress));
}

ConferenceId::ConferenceId(const std::shared_ptr<const Address> &pAddress,
                           const std::shared_ptr<const Address> &lAddress) {
	setPeerAddress(pAddress);
	setLocalAddress(lAddress);
}

ConferenceId::ConferenceId(const std::shared_ptr<Address> &pAddress, const std::shared_ptr<Address> &lAddress) {
	setPeerAddress(pAddress);
	setLocalAddress(lAddress);
}

ConferenceId::ConferenceId(const ConferenceId &other) {
	setLocalAddress(other.localAddress);
	setPeerAddress(other.peerAddress);
	mHash = other.mHash;
}

ConferenceId &ConferenceId::operator=(const ConferenceId &other) {
	setLocalAddress(other.localAddress);
	setPeerAddress(other.peerAddress);
	mHash = other.mHash;
	return *this;
}

bool ConferenceId::operator==(const ConferenceId &other) const {
	return peerAddress && other.peerAddress && (*peerAddress == *(other.peerAddress)) && localAddress &&
	       other.localAddress && (*localAddress == *(other.localAddress));
}

bool ConferenceId::operator!=(const ConferenceId &other) const {
	return !(*this == other);
}

bool ConferenceId::operator<(const ConferenceId &other) const {
	return *peerAddress < *(other.peerAddress) ||
	       (*peerAddress == *(other.peerAddress) && *localAddress < *(other.localAddress));
}

void ConferenceId::setPeerAddress(const std::shared_ptr<const Address> &addr) {
	peerAddress = (addr) ? Address::create(addr->getUri()) : Address::create();
	mHash = 0;
}

void ConferenceId::setLocalAddress(const std::shared_ptr<const Address> &addr) {
	localAddress = (addr) ? Address::create(addr->getUri()) : Address::create();
	mHash = 0;
}

const std::shared_ptr<Address> &ConferenceId::getPeerAddress() const {
	return peerAddress;
}

const std::shared_ptr<Address> &ConferenceId::getLocalAddress() const {
	return localAddress;
}

bool ConferenceId::isValid() const {
	return peerAddress && peerAddress->isValid() && localAddress && localAddress->isValid();
}

size_t ConferenceId::getHash() const {
	if (mHash == 0) {
		const auto &pAddress = peerAddress ? peerAddress->toStringOrdered(true) : "sip:";
		const auto &lAddress = localAddress ? localAddress->toStringOrdered(true) : "sip:";
		mHash = hash<string>()(pAddress) ^ (hash<string>()(lAddress) << 1);
	}
	return mHash;
}

Address ConferenceId::reducedAddress(const Address &addr) {
	Address ret = addr.getUriWithoutGruu();
	ret.removeUriParam(Conference::SecurityModeParameter);
	return ret;
}

size_t ConferenceId::getWeakHash() const {
	if (mWeakHash == 0) {
		const auto &pAddress = peerAddress ? reducedAddress(*peerAddress).toStringUriOnlyOrdered(true) : "sip:";
		const auto &lAddress = localAddress ? reducedAddress(*localAddress).toStringUriOnlyOrdered(true) : "sip:";
		mWeakHash = hash<string>()(pAddress) ^ (hash<string>()(lAddress) << 1);
	}
	return mWeakHash;
}

bool ConferenceId::weakEqual(const ConferenceId &other) const {
	return peerAddress && other.peerAddress && reducedAddress(*peerAddress) == reducedAddress(*other.peerAddress) &&
	       localAddress && other.localAddress && reducedAddress(*localAddress) == reducedAddress(*other.localAddress);
}

LINPHONE_END_NAMESPACE
