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

#include <bctoolbox/defs.h>

#include "carddav-context.h"
#include "carddav-query.h"
#include "carddav-response.h"
#include "friend/friend-list.h"
#include "vcard.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

string CardDavPropFilter::toXmlString() const {
	ostringstream ss;
	ss << "<card:prop-filter name=\"" << mField << "\"><card:text-match";
	if (!mCollation.empty()) {
		ss << " collation=\"" << mCollation << "\"";
	}
	if (mExactMatch) {
		ss << " match-type=\"equals\"";
	} else {
		ss << " match-type=\"contains\"";
	}
	ss << ">" << mFilter << "</card:text-match></card:prop-filter>";
	return ss.str();
}

// =============================================================================

CardDAVQuery::CardDAVQuery(CardDAVContext *context) {
	mContext = context;
}

CardDAVQuery::~CardDAVQuery() {
}

// -----------------------------------------------------------------------------

bool CardDAVQuery::isClientToServerSync() const {
	switch (mType) {
		case Type::Propfind:
		case Type::AddressbookQuery:
		case Type::AddressbookQueryWithFilter:
		case Type::AddressbookMultiget:
			return false;
		case Type::Put:
		case Type::Delete:
			return true;
		default:
			lError() << "[CardDAV] Unknown request: " << static_cast<int>(mType);
			return false;
	}
}

// -----------------------------------------------------------------------------

shared_ptr<CardDAVQuery> CardDAVQuery::createAddressbookMultigetQuery(CardDAVContext *context,
                                                                      const list<CardDAVResponse> &list) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "1";
	query->mMethod = "REPORT";
	query->mUrl = context->mSyncUri;
	query->mType = Type::AddressbookMultiget;
	stringstream ssBody;
	ssBody << "<card:addressbook-multiget xmlns:d=\"DAV:\" "
	          "xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:prop><d:getetag "
	          "/><card:address-data content-type='text/vcard' version='4.0'/></d:prop>";
	for (const auto &response : list)
		ssBody << "<d:href>" << response.mUrl << "</d:href>";
	ssBody << "</card:addressbook-multiget>";
	query->mBody = ssBody.str();
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createAddressbookQuery(CardDAVContext *context) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "1";
	query->mBody =
	    "<card:addressbook-query xmlns:d=\"DAV:\" xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:prop><d:getetag "
	    "/></d:prop><card:filter></card:filter></card:addressbook-query>";
	query->mMethod = "REPORT";
	query->mUrl = context->mSyncUri;
	query->mType = Type::AddressbookQuery;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createAddressbookQueryWithFilter(CardDAVContext *context,
                                                                        const list<CardDavPropFilter> &propFilters,
                                                                        unsigned int limit) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "1";

	ostringstream carddavQuery;
	carddavQuery
	    << "<card:addressbook-query xmlns:d=\"DAV:\" xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:prop><d:getetag "
	       "/></d:prop>";
	if (!propFilters.empty()) {
		carddavQuery << "<card:filter>";
		for (auto propFilter : propFilters) {
			carddavQuery << propFilter.toXmlString();
		}
		carddavQuery << "</card:filter>";
	}
	if (limit > 0) {
		carddavQuery << "<card:limit><card:nresults>" << limit << "</card:nresults></card:limit>";
	}
	carddavQuery << "</card:addressbook-query>";
	query->mBody = carddavQuery.str();

	query->mMethod = "REPORT";
	query->mUrl = context->mSyncUri;
	query->mType = Type::AddressbookQueryWithFilter;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createDeleteQuery(CardDAVContext *context, const shared_ptr<Vcard> &vcard) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mIfmatch = vcard->getEtag();
	query->mMethod = "DELETE";
	query->mUrl = vcard->getUrl();
	query->mType = Type::Delete;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createUserPrincipalPropfindQuery(CardDAVContext *context) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "0";
	query->mBody = "<d:propfind xmlns:d=\"DAV:\"><d:prop><d:current-user-principal /></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::UserPrincipal;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createUserAddressBookPropfindQuery(CardDAVContext *context) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "0";
	query->mBody =
	    "<d:propfind xmlns:d=\"DAV:\" xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:prop><card:addressbook-home-set "
	    "/></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::UserAddressBooksHome;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createAddressBookUrlAndCtagPropfindQuery(CardDAVContext *context) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "1"; // This PROPFIND must have Depth 1!
	query->mBody = "<d:propfind xmlns:d=\"DAV:\" xmlns:cs=\"http://calendarserver.org/ns/\"><d:prop><d:resourcetype "
	               "/><d:displayname /><cs:getctag /></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::AddressBookUrlAndCTAG;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createAddressBookCtagPropfindQuery(CardDAVContext *context) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mDepth = "1"; // This PROPFIND must have Depth 1!
	query->mBody = "<d:propfind xmlns:d=\"DAV:\" xmlns:cs=\"http://calendarserver.org/ns/\"><d:prop><cs:getctag "
	               "/></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::AddressBookCTAG;
	return query;
}

shared_ptr<CardDAVQuery> CardDAVQuery::createPutQuery(CardDAVContext *context, const shared_ptr<Vcard> &vcard) {
	shared_ptr<CardDAVQuery> query = make_shared<CardDAVQuery>(context);
	query->mIfmatch = vcard->getEtag();
	query->mBody = vcard->asVcard4StringWithBase64Picture();
	query->mMethod = "PUT";
	query->mUrl = vcard->getUrl();
	query->mType = Type::Put;
	return query;
}

LINPHONE_END_NAMESPACE
