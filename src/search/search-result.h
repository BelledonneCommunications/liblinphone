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

#ifndef _L_SEARCH_RESULT_H_
#define _L_SEARCH_RESULT_H_

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "linphone/types.h"

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC SearchResult : public bellesip::HybridObject<LinphoneSearchResult, SearchResult> {
public:
	// TODO: Use C++ Address! Not LinphoneAddress.
	SearchResult();
	SearchResult (const unsigned int weight, const LinphoneAddress *a, const std::string &pn, const LinphoneFriend *f, int sourceFlags);
	SearchResult (const SearchResult &other);
	~SearchResult ();

	SearchResult* clone () const override {
		return new SearchResult(*this);
	}

	bool operator< (const SearchResult &other) const;
	bool operator> (const SearchResult &other) const;
	bool operator>= (const SearchResult &other) const;
	bool operator== (const SearchResult &other) const;
	std::ostream & operator << (std::ostream & str) const {
		str << this->toString();
		return str;
	}

	std::string toString() const override;

	const char* getDisplayName() const;

	/**
	 * @return LinphoneFriend associed
	 **/
	const LinphoneFriend *getFriend ()const;

	/**
	 * @return LinphoneAddress associed
	 **/
	const LinphoneAddress *getAddress () const;

	/**
	 * @return Phone Number associed
	 **/
	const std::string &getPhoneNumber () const;

	/**
	 * @return a capability mask associated to the search result
	 **/
	int getCapabilities () const;

	/**
	 * @return whether or not the search results has a capability
	 **/
	bool hasCapability (const LinphoneFriendCapability capability) const;

	/**
	 * @return the result weight
	 **/
	unsigned int getWeight () const;
	
	/**
	 * @param the result weight to set. Default is 0.
	 **/
	void setWeight(const unsigned int& weight);
	
	
	/**
	 * @return the source flags associated to the result
	 **/
	int getSourceFlags() const;
	
	/**
	 * @brief Merge the results with withResult : add sourceFlags, complete missing field (no override if weight is lesser than current weight)
	 **/
	void merge(const std::shared_ptr<SearchResult>& withResult);

private:
	void updateCapabilities ();

	int mSourceFlags;
	const LinphoneFriend *mFriend;
	const LinphoneAddress *mAddress;
	std::string mPhoneNumber;
	int mCapabilities = LinphoneFriendCapabilityGroupChat | LinphoneFriendCapabilityLimeX3dh | LinphoneFriendCapabilityEphemeralMessages;
	unsigned int mWeight;
};

LINPHONE_END_NAMESPACE

#endif //_L_SEARCH_RESULT_H_
