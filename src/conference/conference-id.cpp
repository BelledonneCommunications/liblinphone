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

const std::string ConferenceId::IdentifierDelimiter = "##";

ConferenceId::ConferenceId() {
}

ConferenceId::ConferenceId(Address &&pAddress, Address &&lAddress, const ConferenceIdParams &params) {
	mParams = params;
	mPeerAddress = processAddress(std::move(pAddress));
	mLocalAddress = processAddress(std::move(lAddress));
}

ConferenceId::ConferenceId(const std::shared_ptr<const Address> &pAddress,
                           const std::shared_ptr<const Address> &lAddress,
                           const ConferenceIdParams &params) {
	mParams = params;
	setPeerAddress(pAddress);
	setLocalAddress(lAddress);
}

ConferenceId::ConferenceId(const ConferenceId &other) {
	mParams = other.mParams;
	setPeerAddress(other.mPeerAddress, true);
	setLocalAddress(other.mLocalAddress, true);
	mHash = other.mHash;
	mWeakHash = other.mWeakHash;
}

ConferenceId &ConferenceId::operator=(const ConferenceId &other) {
	mParams = other.mParams;
	setPeerAddress(other.mPeerAddress, true);
	setLocalAddress(other.mLocalAddress, true);
	mHash = other.mHash;
	mWeakHash = other.mWeakHash;
	return *this;
}

bool ConferenceId::operator==(const ConferenceId &other) const {
	return mPeerAddress && other.mPeerAddress && (*mPeerAddress == *(other.mPeerAddress)) && mLocalAddress &&
	       other.mLocalAddress && (*mLocalAddress == *(other.mLocalAddress));
}

bool ConferenceId::operator!=(const ConferenceId &other) const {
	return !(*this == other);
}

bool ConferenceId::operator<(const ConferenceId &other) const {
	return *mPeerAddress < *(other.mPeerAddress) ||
	       (*mPeerAddress == *(other.mPeerAddress) && *mLocalAddress < *(other.mLocalAddress));
}

std::shared_ptr<Address> ConferenceId::processAddress(const Address &addr) const {
	if (addr.isValid()) {
		if (mParams.extractUriEnabled()) {
			if (mParams.getKeepGruu()) {
				return Address::create(addr.getUri());
			} else {
				return Address::create(addr.getUriWithoutGruu());
			}
		} else {
			auto processedAddress = Address::create(addr);
			if (!mParams.getKeepGruu()) {
				processedAddress->removeUriParam("gr");
			}
			return processedAddress;
		}
	}
	return Address::create();
}

bool ConferenceId::canUpdateAddress(const std::shared_ptr<const Address> &addr, bool useLocal) const {
	// Local and peer addresses cannot be modified if the hash or weak hash have already been computed
	const auto newUri = (addr) ? addr->getUri() : Address();
	const auto currentMember = useLocal ? mLocalAddress : mPeerAddress;
	return (!currentMember || (currentMember->toStringUriOnlyOrdered() == newUri.toStringUriOnlyOrdered()) ||
	        ((mHash == 0) && (mWeakHash == 0)));
}

void ConferenceId::setPeerAddress(const std::shared_ptr<const Address> &addr, bool forceUpdate) {
	if (!addr) return;
	if (!forceUpdate && !canUpdateAddress(addr, false)) {
		lError() << *this << ": Cannot modify peer address if either its hash or its weak hash is defined";
		abort();
	}
	mPeerAddress = processAddress(*addr);
}

void ConferenceId::setLocalAddress(const std::shared_ptr<const Address> &addr, bool forceUpdate) {
	if (!addr) return;
	if (!forceUpdate && !canUpdateAddress(addr, true)) {
		lInfo() << *this << ": Cannot modify local address if either its hash or its weak hash is defined";
		abort();
	}
	mLocalAddress = processAddress(*addr);
}

const std::shared_ptr<Address> &ConferenceId::getPeerAddress() const {
	return mPeerAddress;
}

const std::shared_ptr<Address> &ConferenceId::getLocalAddress() const {
	return mLocalAddress;
}

bool ConferenceId::isValid() const {
	return mPeerAddress && mPeerAddress->isValid() && mLocalAddress && mLocalAddress->isValid();
}

size_t ConferenceId::getHash() const {
	if (mHash == 0) {
		const auto &pAddress = mPeerAddress ? mPeerAddress->toStringUriOnlyOrdered(true) : "sip:";
		const auto &lAddress = mLocalAddress ? mLocalAddress->toStringUriOnlyOrdered(true) : "sip:";
		mHash = hash<string>()(pAddress) ^ (hash<string>()(lAddress) << 1);
	}
	return mHash;
}

const std::string &ConferenceId::getIdentifier() const {
	if (mIdentifier.empty()) {
		const auto &pAddress = mPeerAddress ? reducedAddress(*mPeerAddress).toStringUriOnlyOrdered(false) : "sip:";
		const auto &lAddress = mLocalAddress ? reducedAddress(*mLocalAddress).toStringUriOnlyOrdered(false) : "sip:";
		mIdentifier += pAddress + ConferenceId::IdentifierDelimiter + lAddress;
	}
	return mIdentifier;
}

Address ConferenceId::reducedAddress(const Address &addr) {
	Address ret = addr.getUriWithoutGruu();
	ret.removeUriParam(Conference::SecurityModeParameter);
	return ret;
}

size_t ConferenceId::getWeakHash() const {
	if (mWeakHash == 0) {
		const auto &pAddress = mPeerAddress ? reducedAddress(*mPeerAddress).toStringUriOnlyOrdered(true) : "sip:";
		const auto &lAddress = mLocalAddress ? reducedAddress(*mLocalAddress).toStringUriOnlyOrdered(true) : "sip:";
		mWeakHash = hash<string>()(pAddress) ^ (hash<string>()(lAddress) << 1);
	}
	return mWeakHash;
}

bool ConferenceId::weakEqual(const ConferenceId &other) const {
	return mPeerAddress && other.mPeerAddress && reducedAddress(*mPeerAddress) == reducedAddress(*other.mPeerAddress) &&
	       mLocalAddress && other.mLocalAddress &&
	       reducedAddress(*mLocalAddress) == reducedAddress(*other.mLocalAddress);
}

std::pair<std::shared_ptr<Address>, std::shared_ptr<Address>>
ConferenceId::parseIdentifier(const std::string &identifier) {
	std::string peerAddressString = identifier.substr(0, identifier.find(ConferenceId::IdentifierDelimiter));
	std::string localAddressString =
	    identifier.substr(identifier.find(ConferenceId::IdentifierDelimiter) + ConferenceId::IdentifierDelimiter.size(),
	                      identifier.size() - 1);
	auto localAddress = Address::create(localAddressString);
	auto peerAddress = Address::create(peerAddressString);
	return std::make_pair(localAddress, peerAddress);
}

LINPHONE_END_NAMESPACE
