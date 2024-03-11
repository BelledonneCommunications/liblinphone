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

CardDAVQuery::CardDAVQuery(CardDAVContext *context) {
	mContext = context;
}

CardDAVQuery::~CardDAVQuery() {
	if (mHttpRequestListener) belle_sip_object_unref(mHttpRequestListener);
}

// -----------------------------------------------------------------------------

bool CardDAVQuery::isClientToServerSync() const {
	switch (mType) {
		case Type::Propfind:
		case Type::AddressbookQuery:
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

CardDAVQuery *CardDAVQuery::createAddressbookMultigetQuery(CardDAVContext *context,
                                                           const std::list<CardDAVResponse> &list) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mDepth = "1";
	query->mMethod = "REPORT";
	query->mUrl = context->mSyncUri;
	query->mType = Type::AddressbookMultiget;
	std::stringstream ssBody;
	ssBody << "<card:addressbook-multiget xmlns:d=\"DAV:\" "
	          "xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:prop><d:getetag "
	          "/><card:address-data content-type='text/vcard' version='4.0'/></d:prop>";
	for (const auto &response : list)
		ssBody << "<d:href>" << response.mUrl << "</d:href>";
	ssBody << "</card:addressbook-multiget>";
	query->mBody = ssBody.str();
	return query;
}

CardDAVQuery *CardDAVQuery::createAddressbookQuery(CardDAVContext *context) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mDepth = "1";
	query->mBody =
	    "<card:addressbook-query xmlns:d=\"DAV:\" xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:prop><d:getetag "
	    "/></d:prop><card:filter></card:filter></card:addressbook-query>";
	query->mMethod = "REPORT";
	query->mUrl = context->mSyncUri;
	query->mType = Type::AddressbookQuery;
	return query;
}

CardDAVQuery *CardDAVQuery::createDeleteQuery(CardDAVContext *context, const std::shared_ptr<Vcard> &vcard) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mIfmatch = vcard->getEtag();
	query->mMethod = "DELETE";
	query->mUrl = vcard->getUrl();
	query->mType = Type::Delete;
	return query;
}

CardDAVQuery *CardDAVQuery::createUserPrincipalPropfindQuery(CardDAVContext *context) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mDepth = "0";
	query->mBody = "<d:propfind xmlns:d=\"DAV:\"><d:prop><d:current-user-principal /></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::UserPrincipal;
	return query;
}

CardDAVQuery *CardDAVQuery::createUserAddressBookPropfindQuery(CardDAVContext *context) {
	CardDAVQuery *query = new CardDAVQuery(context);
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

CardDAVQuery *CardDAVQuery::createAddressBookUrlAndCtagPropfindQuery(CardDAVContext *context) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mDepth = "1"; // This PROPFIND must have Depth 1!
	query->mBody = "<d:propfind xmlns:d=\"DAV:\" xmlns:cs=\"http://calendarserver.org/ns/\"><d:prop><d:resourcetype "
	               "/><d:displayname /><cs:getctag /></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::AddressBookUrlAndCTAG;
	return query;
}

CardDAVQuery *CardDAVQuery::createAddressBookCtagPropfindQuery(CardDAVContext *context) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mDepth = "1"; // This PROPFIND must have Depth 1!
	query->mBody = "<d:propfind xmlns:d=\"DAV:\" xmlns:cs=\"http://calendarserver.org/ns/\"><d:prop><cs:getctag "
	               "/></d:prop></d:propfind>";
	query->mMethod = "PROPFIND";
	query->mUrl = context->mSyncUri;
	query->mType = Type::Propfind;
	query->mPropfindType = PropfindType::AddressBookCTAG;
	return query;
}

CardDAVQuery *CardDAVQuery::createPutQuery(CardDAVContext *context, const std::shared_ptr<Vcard> &vcard) {
	CardDAVQuery *query = new CardDAVQuery(context);
	query->mIfmatch = vcard->getEtag();
	query->mBody = vcard->asVcard4String();
	query->mMethod = "PUT";
	query->mUrl = vcard->getUrl();
	query->mType = Type::Put;
	return query;
}

LINPHONE_END_NAMESPACE
