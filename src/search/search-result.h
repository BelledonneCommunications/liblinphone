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

#ifndef _L_SEARCH_RESULT_H_
#define _L_SEARCH_RESULT_H_

#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Address;
class Friend;

/** Class that represents a single result for a request made using the MagicSearch object. */
class LINPHONE_PUBLIC SearchResult : public bellesip::HybridObject<LinphoneSearchResult, SearchResult> {
public:
	SearchResult();
	SearchResult(const unsigned int weight,
	             std::shared_ptr<const Address> a,
	             const std::string &pn,
	             std::shared_ptr<Friend> f,
	             int sourceFlags);
	SearchResult(const SearchResult &other);
	~SearchResult();

	SearchResult *clone() const override {
		return new SearchResult(*this);
	}

	bool operator<(const SearchResult &other) const;
	bool operator>(const SearchResult &other) const;
	bool operator>=(const SearchResult &other) const;
	bool operator==(const SearchResult &other) const;
	std::ostream &operator<<(std::ostream &str) const {
		str << this->toString();
		return str;
	}

	std::string toString() const override;

	/**
	 * @return a display name for this results, using the friend's name if possible, otherwise the username from the
	 *address if any, and finally the phone number.
	 **/
	std::string getDisplayName() const;

	/**
	 * @return LinphoneFriend associed
	 **/
	std::shared_ptr<Friend> getFriend() const;

	/**
	 * @return LinphoneAddress associed
	 **/
	std::shared_ptr<Address> getAddress() const;

	/**
	 * @return Phone Number associed
	 **/
	const std::string &getPhoneNumber() const;

	/**
	 * @return a capability mask associated to the search result
	 **/
	int getCapabilities() const;

	/**
	 * @return whether or not the search results has a capability
	 **/
	bool hasCapability(const LinphoneFriendCapability capability) const;

	/**
	 * @return the result weight
	 **/
	unsigned int getWeight() const;

	/**
	 * @param the result weight to set. Default is 0.
	 **/
	void setWeight(const unsigned int &weight);

	/**
	 * @return the source flags associated to the result
	 **/
	int getSourceFlags() const;

	/**
	 * @return whether or not the search results has a source flag
	 **/
	bool hasSourceFlag(const LinphoneMagicSearchSource source) const;

	/**
	 * @brief Merge the results with withResult : add sourceFlags, complete missing field (no override if weight is
	 *lesser than current weight)
	 **/
	void merge(const std::shared_ptr<SearchResult> &withResult);

private:
	void updateCapabilities();

	int mSourceFlags;
	std::shared_ptr<Friend> mFriend;
	std::shared_ptr<Address> mAddress;
	std::string mPhoneNumber;
	int mCapabilities = LinphoneFriendCapabilityGroupChat | LinphoneFriendCapabilityLimeX3dh |
	                    LinphoneFriendCapabilityEphemeralMessages;
	unsigned int mWeight;
};

LINPHONE_END_NAMESPACE

#endif //_L_SEARCH_RESULT_H_
