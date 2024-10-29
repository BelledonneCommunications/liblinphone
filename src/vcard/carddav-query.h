/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_CARDDAV_QUERY_H_
#define _L_CARDDAV_QUERY_H_

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CardDAVContext;
class CardDAVResponse;
class Vcard;

class CardDavPropFilter {
public:
	CardDavPropFilter(const std::string &filter, const std::string &field, bool exactMatch) {
		mFilter = filter;
		mField = field;
		mExactMatch = exactMatch;
	}

	std::string toXmlString() const;

	std::string mFilter;
	std::string mField;
	std::string mCollation = "i;unicode-casemap";
	bool mExactMatch = false; // if false then contains will be used instead
};

class CardDAVQuery : public UserDataAccessor {
public:
	enum class Type { Propfind, AddressbookQuery, AddressbookQueryWithFilter, AddressbookMultiget, Put, Delete };
	enum class PropfindType { UserPrincipal, UserAddressBooksHome, AddressBookUrlAndCTAG, AddressBookCTAG };

	CardDAVQuery(CardDAVContext *context);
	CardDAVQuery(const CardDAVQuery &other) = delete;
	virtual ~CardDAVQuery();

	// Friends
	friend CardDAVContext;

	bool isClientToServerSync() const;

	static std::shared_ptr<CardDAVQuery> createUserPrincipalPropfindQuery(CardDAVContext *context);
	static std::shared_ptr<CardDAVQuery> createUserAddressBookPropfindQuery(CardDAVContext *context);
	static std::shared_ptr<CardDAVQuery> createAddressBookUrlAndCtagPropfindQuery(CardDAVContext *context);
	static std::shared_ptr<CardDAVQuery> createAddressBookCtagPropfindQuery(CardDAVContext *context);
	static std::shared_ptr<CardDAVQuery> createAddressbookQuery(CardDAVContext *context);
	static std::shared_ptr<CardDAVQuery> createAddressbookQueryWithFilter(
	    CardDAVContext *context, const std::list<CardDavPropFilter> &propFilters, unsigned int limit);
	static std::shared_ptr<CardDAVQuery> createAddressbookMultigetQuery(CardDAVContext *context,
	                                                                    const std::list<CardDAVResponse> &list);
	static std::shared_ptr<CardDAVQuery> createDeleteQuery(CardDAVContext *context,
	                                                       const std::shared_ptr<Vcard> &vcard);
	static std::shared_ptr<CardDAVQuery> createPutQuery(CardDAVContext *context, const std::shared_ptr<Vcard> &vcard);

private:
	CardDAVContext *mContext = nullptr;
	Type mType;
	std::string mUrl;
	std::string mMethod;
	std::string mBody;
	std::string mDepth;
	std::string mIfmatch;
	PropfindType mPropfindType;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CARDDAV_QUERY_H_
