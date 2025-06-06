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

#ifndef _L_CARDDAV_CONTEXT_H_
#define _L_CARDDAV_CONTEXT_H_

#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "core/core.h"
#include "http/http-client.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CardDAVQuery;
class CardDAVResponse;
class CardDavMagicSearchPlugin;
class CardDavPropFilter;
class Friend;
class FriendList;

class LINPHONE_PUBLIC CardDAVContext : public CoreAccessor, public UserDataAccessor {
public:
	CardDAVContext(const std::shared_ptr<Core> &core);
	CardDAVContext(const CardDAVContext &other) = delete;
	virtual ~CardDAVContext() = default;

	// Friends
	friend CardDAVQuery;
	friend FriendList;

	// Setters
	void setFriendList(const std::shared_ptr<FriendList> &lf) {
		mFriendList = lf;
	}

	void deleteVcard(const std::shared_ptr<Friend> &f);
	void putVcard(const std::shared_ptr<Friend> &f);
	void synchronize();

	void setMagicSearchPlugin(const std::shared_ptr<CardDavMagicSearchPlugin> &plugin);
	void queryVcards(const std::string serverUrl, const std::list<CardDavPropFilter> &propFilters, unsigned int limit);

private:
	void clientToServerSyncDone(bool success, const std::string &msg);

	void userPrincipalUrlRetrieved(std::string principalUrl);
	void userAddressBookHomeUrlRetrieved(std::string addressBookHomeUrl);
	void addressBookUrlAndCtagRetrieved(const std::list<CardDAVResponse> &list);
	void addressBookCtagRetrieved(std::string ctag);

	void fetchVcards();
	void pullVcards(const std::list<CardDAVResponse> &list);

	void queryWellKnown(std::shared_ptr<CardDAVQuery> query);
	void retrieveUserPrincipalUrl();
	void retrieveUserAddressBooksHomeUrl();
	void retrieveAddressBookUrlAndCtag();
	void retrieveAddressBookCtag();

	void sendQuery(const std::shared_ptr<CardDAVQuery> &query, bool cancelCurrentIfAny = false);
	void setSchemeAndHostIfNotDoneYet(std::shared_ptr<CardDAVQuery> query);
	void processRedirect(std::shared_ptr<CardDAVQuery> query, const std::string &location);
	void processQueryResponse(std::shared_ptr<CardDAVQuery> query, const HttpResponse &response);
	void serverToClientSyncDone(bool success, const std::string &msg);
	void vcardsFetched(const std::list<CardDAVResponse> &vCards);
	void magicSearchResultsVcardsPulled(const std::list<CardDAVResponse> &vCards);
	void vcardsPulled(const std::list<CardDAVResponse> &vCards);

	std::string generateUrlFromServerAddressAndUid(const std::string &serverUrl);
	std::string parseUserPrincipalUrlValueFromXmlResponse(const std::string &body);
	std::string parseUserAddressBookUrlValueFromXmlResponse(const std::string &body);
	std::list<CardDAVResponse> parseAddressBookUrlAndCtagValueFromXmlResponse(const std::string &body);
	std::string parseAddressBookCtagValueFromXmlResponse(const std::string &body);
	std::list<CardDAVResponse> parseVcardsEtagsFromXmlResponse(const std::string &body);
	std::list<CardDAVResponse> parseVcardsFromXmlResponse(const std::string &body);

	std::string getUrlSchemeHostAndPort() const;

	std::string mCtag = "";
	std::string mSyncUri = "";
	std::string mScheme = "http";
	std::string mHost = "";
	int mPort = 0;
	bool mWellKnownQueried = true;
	HttpRequest *mHttpRequest = nullptr;
	std::shared_ptr<CardDAVQuery> mQuery = nullptr;

	std::weak_ptr<FriendList> mFriendList;
	std::weak_ptr<CardDavMagicSearchPlugin> mCardDavMagicSearchPlugin;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CARDDAV_CONTEXT_H_
