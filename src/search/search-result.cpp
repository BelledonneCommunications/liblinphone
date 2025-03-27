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

#include "address/address.h"
#include "c-wrapper/internal/c-tools.h"
#include "friend/friend.h"
#include "presence/presence-model.h"

#include "search-result.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void SearchResult::updateCapabilities() {
	if (!mFriend) return;

	mCapabilities = LinphoneFriendCapabilityNone;
	shared_ptr<PresenceModel> presenceModel = nullptr;

	if (mAddress) {
		presenceModel = mFriend->getPresenceModelForAddress(mAddress);
	}

	if (!presenceModel && !mPhoneNumber.empty()) {
		presenceModel = mFriend->getPresenceModelForUriOrTel(mPhoneNumber);
	}

	if (presenceModel) mCapabilities = presenceModel->getCapabilities();
}

// ------------------------------------------------------------------------------
SearchResult::SearchResult() {
	mWeight = 0;
	mAddress = nullptr;
	mFriend = nullptr;
	mSourceFlags = LinphoneMagicSearchSourceNone;
}

SearchResult::SearchResult(const unsigned int weight,
                           shared_ptr<const Address> address,
                           const string &phoneNumber,
                           shared_ptr<Friend> linphoneFriend,
                           int sourceFlags) {
	mWeight = weight;
	mAddress = (address) ? address->clone()->toSharedPtr() : nullptr;
	mPhoneNumber = phoneNumber;
	mFriend = linphoneFriend;
	mSourceFlags = sourceFlags;
	updateCapabilities();
}

SearchResult::SearchResult(const SearchResult &sr) : HybridObject(sr) {
	mWeight = sr.getWeight();
	mAddress = sr.getAddress()->clone()->toSharedPtr();
	mPhoneNumber = sr.getPhoneNumber();
	mFriend = sr.getFriend();
	mSourceFlags = sr.getSourceFlags();
	mCapabilities = sr.getCapabilities();
}

SearchResult::~SearchResult() {};

bool SearchResult::operator<(const SearchResult &other) const {
	return getWeight() < other.getWeight();
}

bool SearchResult::operator>(const SearchResult &other) const {
	return getWeight() > other.getWeight();
}

bool SearchResult::operator>=(const SearchResult &other) const {
	return getWeight() >= other.getWeight();
}

bool SearchResult::operator==(const SearchResult &other) const {
	return getWeight() == other.getWeight();
}

string SearchResult::toString() const {
	ostringstream ss;
	ss << getDisplayName();

	const auto &addr = getAddress();
	if (addr) {
		ss << " address [" << *addr << "]";
	}

	const string &phoneNumber = getPhoneNumber();
	if (!phoneNumber.empty()) {
		ss << " phone number [" << phoneNumber << "]";
	}

	if (mFriend) {
		ss << " friend [" << mFriend << "]";
	}

	ss << " source(s) [" << mSourceFlags << "]";
	ss << " capabilities [" << mCapabilities << "]";

	return ss.str();
}

string SearchResult::getDisplayName() const {
	if (mFriend && !mFriend->getName().empty()) {
		return mFriend->getName();
	}

	const auto &addr = getAddress();
	if (addr) {
		if (!addr->getDisplayName().empty()) {
			return addr->getDisplayName();
		}
		if (!addr->getUsername().empty()) {
			return addr->getUsername();
		}
	}
	return getPhoneNumber();
}

shared_ptr<Friend> SearchResult::getFriend() const {
	return mFriend;
}

shared_ptr<Address> SearchResult::getAddress() const {
	return mAddress;
}

const string &SearchResult::getPhoneNumber() const {
	return mPhoneNumber;
}

int SearchResult::getCapabilities() const {
	return mCapabilities;
}

bool SearchResult::hasCapability(const LinphoneFriendCapability capability) const {
	return static_cast<bool>(mCapabilities & capability);
}

unsigned int SearchResult::getWeight() const {
	return mWeight;
}

void SearchResult::setWeight(const unsigned int &weight) {
	mWeight = weight;
}

int SearchResult::getSourceFlags() const {
	return mSourceFlags;
}

bool SearchResult::hasSourceFlag(const LinphoneMagicSearchSource source) const {
	return static_cast<bool>(mSourceFlags & source);
}

void SearchResult::merge(const shared_ptr<SearchResult> &other) {
	bool updateSourceFlags = false;

	// No need to merge addresses, they are equal otherwise we wouldn't be here

	if (mPhoneNumber.empty()) {
		mPhoneNumber = other->getPhoneNumber();
		updateSourceFlags = true;
	}

	if (!mFriend && other->mFriend) {
		mFriend = other->mFriend;
		updateSourceFlags = true;
	} else if (other->mFriend && !mFriend->inList() && other->mFriend->inList()) {
		// Prefer friends storred locally
		mFriend = other->mFriend;
		updateSourceFlags = true;
	}

	if (updateSourceFlags) {
		mSourceFlags |= other->getSourceFlags();
	}

	updateCapabilities();
}
LINPHONE_END_NAMESPACE
