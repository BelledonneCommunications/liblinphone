/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "linphone/api/c-address.h"
#include "linphone/friend.h"
#include "linphone/presence.h"

#include "search-result.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE


void SearchResult::updateCapabilities () {
	if (!mFriend) return;

	mCapabilities = LinphoneFriendCapabilityNone;
	const LinphonePresenceModel *presenceModel = nullptr;

	if (mAddress) {
		char *addressString = linphone_address_as_string_uri_only(mAddress);
		presenceModel = linphone_friend_get_presence_model_for_uri_or_tel(mFriend, addressString);
		bctbx_free(addressString);
	}

	if (!presenceModel && !mPhoneNumber.empty()) {
		presenceModel = linphone_friend_get_presence_model_for_uri_or_tel(mFriend, mPhoneNumber.c_str());
	}

	if (presenceModel)
		mCapabilities = linphone_presence_model_get_capabilities(presenceModel);
}

// ------------------------------------------------------------------------------
SearchResult::SearchResult(){
	mWeight = 0;
	mAddress = NULL;
	mFriend = NULL;
	mSourceFlags = LinphoneMagicSearchSourceNone;
}

SearchResult::SearchResult (
	const unsigned int weight,
	const LinphoneAddress *address,
	const string &phoneNumber,
	const LinphoneFriend *linphoneFriend,
	int sourceFlags
) {
	mWeight = weight;
	mAddress = address;
	if (mAddress) linphone_address_ref(const_cast<LinphoneAddress *>(mAddress));
	mPhoneNumber = phoneNumber;
	mFriend = linphoneFriend;
	if (mFriend) linphone_friend_ref(const_cast<LinphoneFriend *>(mFriend));
	mSourceFlags = sourceFlags;
	updateCapabilities();
}

SearchResult::SearchResult (const SearchResult &sr) : HybridObject(sr) {
	mWeight = sr.getWeight();
	mAddress = sr.getAddress();
	if (mAddress) linphone_address_ref(const_cast<LinphoneAddress *>(mAddress));
	mPhoneNumber = sr.getPhoneNumber();
	mFriend = sr.getFriend();
	if (mFriend) linphone_friend_ref(const_cast<LinphoneFriend *>(mFriend));
	mSourceFlags = sr.getSourceFlags();
	mCapabilities = sr.getCapabilities();
}

SearchResult::~SearchResult () {
	// FIXME: Ugly temporary workaround to solve weak. Remove me later.
	if (mAddress) linphone_address_unref(const_cast<LinphoneAddress *>(mAddress));
	if (mFriend) linphone_friend_unref(const_cast<LinphoneFriend *>(mFriend));
};

bool SearchResult::operator< (const SearchResult &other) const {
	return getWeight() < other.getWeight();
}

bool SearchResult::operator> (const SearchResult &other) const {
	return getWeight() > other.getWeight();
}

bool SearchResult::operator>= (const SearchResult &other) const {
	return getWeight() >= other.getWeight();
}

bool SearchResult::operator== (const SearchResult &other) const {
	return getWeight() == other.getWeight();
}

std::string SearchResult::toString() const {
	std::ostringstream ss;
	ss << getDisplayName();

	const LinphoneAddress* addr = getAddress();
	if (addr) {
		ss << " address [" << linphone_address_as_string(addr) << "]";
	}

	const string & phoneNumber = getPhoneNumber();
	if (!phoneNumber.empty()) {
		ss << " phone number [" << phoneNumber << "]";
	}
	
	return ss.str();
}

const char* SearchResult::getDisplayName() const {
	const char *name = NULL;
	if (getFriend()) {
		name = linphone_friend_get_name(getFriend());
	}
	if (!name && getAddress()){
		name = linphone_address_get_display_name(getAddress()) ?
			linphone_address_get_display_name(getAddress()) : linphone_address_get_username(getAddress());
	}
	if (!name) {
		return getPhoneNumber().c_str();
	}
	return name;
}

const LinphoneFriend *SearchResult::getFriend () const {
	return mFriend;
}

const LinphoneAddress *SearchResult::getAddress () const {
	return mAddress;
}

const string &SearchResult::getPhoneNumber () const {
	return mPhoneNumber;
}

int SearchResult::getCapabilities () const {
	return mCapabilities;
}

bool SearchResult::hasCapability (const LinphoneFriendCapability capability) const {
	return static_cast<bool>(mCapabilities & capability);
}

unsigned int SearchResult::getWeight () const {
	return mWeight;
}

void SearchResult::setWeight(const unsigned int& weight){
	mWeight = weight;
}

int SearchResult::getSourceFlags() const {
	return mSourceFlags;
}

void SearchResult::merge(const std::shared_ptr<SearchResult>& withResult) {
	bool doOverride = mWeight <= withResult->getWeight();
	
	if(doOverride)
		mWeight = withResult->getWeight();
	mSourceFlags |= withResult->getSourceFlags();
	
	if( withResult->getAddress()){// There is a new data
		if( doOverride && mAddress)
			linphone_address_unref(const_cast<LinphoneAddress *>(mAddress));
		if(doOverride || !mAddress) {
			mAddress = withResult->getAddress();
			linphone_address_ref(const_cast<LinphoneAddress *>(mAddress));
		}
	}
	
	if(doOverride || mPhoneNumber.empty())
		mPhoneNumber = withResult->getPhoneNumber();
		
	if( withResult->getFriend()){// There is a new data
		if( doOverride && mFriend)
			linphone_friend_unref(const_cast<LinphoneFriend *>(mFriend));
		if(doOverride || !mFriend) {
			mFriend = withResult->getFriend();
			linphone_friend_ref(const_cast<LinphoneFriend *>(mFriend));
		}
	}	
	
	updateCapabilities();
}
LINPHONE_END_NAMESPACE
