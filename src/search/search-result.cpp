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
#include "linphone/friend.h"
#include "linphone/presence.h"

#include "search-result.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void SearchResult::updateCapabilities() {
	if (!mFriend) return;

	mCapabilities = LinphoneFriendCapabilityNone;
	const LinphonePresenceModel *presenceModel = nullptr;

	if (mAddress) {
		std::string addressString = mAddress->asStringUriOnly();
		presenceModel = linphone_friend_get_presence_model_for_uri_or_tel(mFriend, L_STRING_TO_C(addressString));
	}

	if (!presenceModel && !mPhoneNumber.empty()) {
		presenceModel = linphone_friend_get_presence_model_for_uri_or_tel(mFriend, mPhoneNumber.c_str());
	}

	if (presenceModel) mCapabilities = linphone_presence_model_get_capabilities(presenceModel);
}

// ------------------------------------------------------------------------------
SearchResult::SearchResult() {
	mWeight = 0;
	mAddress = nullptr;
	mFriend = nullptr;
	mSourceFlags = LinphoneMagicSearchSourceNone;
}

SearchResult::SearchResult(const unsigned int weight,
                           std::shared_ptr<const Address> address,
                           const string &phoneNumber,
                           LinphoneFriend *linphoneFriend,
                           int sourceFlags) {
	mWeight = weight;
	mAddress = (address) ? address->clone()->toSharedPtr() : nullptr;
	mPhoneNumber = phoneNumber;
	mFriend = linphoneFriend;
	if (mFriend) linphone_friend_ref(mFriend);
	mSourceFlags = sourceFlags;
	updateCapabilities();
}

SearchResult::SearchResult(const SearchResult &sr) : HybridObject(sr) {
	mWeight = sr.getWeight();
	mAddress = sr.getAddress()->clone()->toSharedPtr();
	mPhoneNumber = sr.getPhoneNumber();
	mFriend = sr.getFriend();
	if (mFriend) linphone_friend_ref(mFriend);
	mSourceFlags = sr.getSourceFlags();
	mCapabilities = sr.getCapabilities();
}

SearchResult::~SearchResult() {
	if (mFriend) linphone_friend_unref(mFriend);
};

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

std::string SearchResult::toString() const {
	std::ostringstream ss;
	ss << getDisplayName();

	const auto &addr = getAddress();
	if (addr) {
		ss << " address [" << *addr << "]";
	}

	const string &phoneNumber = getPhoneNumber();
	if (!phoneNumber.empty()) {
		ss << " phone number [" << phoneNumber << "]";
	}

	return ss.str();
}

const char *SearchResult::getDisplayName() const {
	const char *name = NULL;
	if (getFriend()) {
		name = linphone_friend_get_name(getFriend());
	}
	const auto &addr = getAddress();
	if (!name && addr) {
		const char *displayName = addr->getDisplayNameCstr();
		name = displayName ? displayName : addr->getUsernameCstr();
	}
	if (!name) {
		return getPhoneNumber().c_str();
	}
	return name;
}

LinphoneFriend *SearchResult::getFriend() const {
	return mFriend;
}

const std::shared_ptr<Address> SearchResult::getAddress() const {
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

void SearchResult::merge(const std::shared_ptr<SearchResult> &withResult) {
	bool doOverride = mWeight <= withResult->getWeight();

	if (doOverride) mWeight = withResult->getWeight();
	mSourceFlags |= withResult->getSourceFlags();

	if (withResult->getAddress()) { // There is a new data
		if (doOverride || !mAddress) {
			mAddress = withResult->getAddress()->clone()->toSharedPtr();
		}
	}

	if (doOverride || mPhoneNumber.empty()) mPhoneNumber = withResult->getPhoneNumber();

	LinphoneFriend *other = withResult->getFriend();
	if (other && other != mFriend) { // There is a new data
		if (doOverride && mFriend) linphone_friend_unref(mFriend);
		if (doOverride || !mFriend) {
			mFriend = other;
			linphone_friend_ref(mFriend);
		}
	}

	updateCapabilities();
}
LINPHONE_END_NAMESPACE
