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
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceId::ConferenceId() {
}

ConferenceId::ConferenceId(Address &&peerAddress, Address &&localAddress) {
	this->peerAddress = Address::create(std::move(peerAddress));
	this->localAddress = Address::create(std::move(localAddress));
}

ConferenceId::ConferenceId(const std::shared_ptr<Address> &peerAddress,
                           const std::shared_ptr<const Address> &localAddress) {
	this->peerAddress = (peerAddress) ? Address::create(peerAddress->getUri()) : nullptr;
	this->localAddress = (localAddress) ? Address::create(localAddress->getUri()) : nullptr;
}

ConferenceId::ConferenceId(const std::shared_ptr<Address> &peerAddress, const std::shared_ptr<Address> &localAddress) {
	this->peerAddress = (peerAddress) ? Address::create(peerAddress->getUri()) : nullptr;
	this->localAddress = (localAddress) ? Address::create(localAddress->getUri()) : nullptr;
}

ConferenceId::ConferenceId(const ConferenceId &other)
    : peerAddress(other.peerAddress), localAddress(other.localAddress) {
}

ConferenceId &ConferenceId::operator=(const ConferenceId &other) {
	this->peerAddress = other.peerAddress;
	this->localAddress = other.localAddress;
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

void ConferenceId::setPeerAddress(const std::shared_ptr<Address> &addr) {
	peerAddress = addr;
}

void ConferenceId::setLocalAddress(const std::shared_ptr<Address> &addr) {
	localAddress = addr;
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

LINPHONE_END_NAMESPACE
