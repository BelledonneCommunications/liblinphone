/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include "linphone/types.h"
#include "linphone/utils/general.h"

#include "object/clonable-object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class SearchResultPrivate;

class LINPHONE_PUBLIC SearchResult : public ClonableObject {
public:
	// TODO: Use C++ Address! Not LinphoneAddress.
	SearchResult (const unsigned int weight, const LinphoneAddress *a, const std::string &pn, const LinphoneFriend *f = nullptr);
	SearchResult (const SearchResult &other);
	~SearchResult ();

	SearchResult* clone () const override {
		return new SearchResult(*this);
	}

	bool operator< (const SearchResult &other) const;
	bool operator> (const SearchResult &other) const;
	bool operator>= (const SearchResult &other) const;
	bool operator= (const SearchResult &other) const;

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

private:
	L_DECLARE_PRIVATE(SearchResult);
};

LINPHONE_END_NAMESPACE

#endif //_L_SEARCH_RESULT_H_
