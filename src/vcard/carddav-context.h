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

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CardDAVQuery;
class CardDAVResponse;
class Friend;
class FriendList;

class LINPHONE_PUBLIC CardDAVContext : public UserDataAccessor {
public:
	CardDAVContext(const std::shared_ptr<FriendList> &friendList);
	CardDAVContext(const CardDAVContext &other) = delete;
	virtual ~CardDAVContext() = default;

	// Friends
	friend CardDAVQuery;
	friend FriendList;

	// Setters
	void
	setContactCreatedCallback(std::function<void(const CardDAVContext *context, const std::shared_ptr<Friend> &f)> cb) {
		mContactCreatedCb = cb;
	}
	void
	setContactRemovedCallback(std::function<void(const CardDAVContext *context, const std::shared_ptr<Friend> &f)> cb) {
		mContactRemovedCb = cb;
	}
	void setContactUpdatedCallback(std::function<void(const CardDAVContext *context,
	                                                  const std::shared_ptr<Friend> &newFriend,
	                                                  const std::shared_ptr<Friend> &oldFriend)> cb) {
		mContactUpdatedCb = cb;
	}
	void setSynchronizationDoneCallback(
	    std::function<void(const CardDAVContext *context, bool success, const std::string &message)> cb) {
		mSynchronizationDoneCb = cb;
	}

	// Other
	bool isValid() const {
		return mFriendList != nullptr;
	}

	void deleteVcard(const std::shared_ptr<Friend> &f);
	void putVcard(const std::shared_ptr<Friend> &f);
	void synchronize();

private:
	void clientToServerSyncDone(bool success, const std::string &msg);

	void userPrincipalUrlRetrieved(std::string principalUrl);
	void userAddressBookHomeUrlRetrieved(std::string addressBookHomeUrl);
	void addressBookUrlAndCtagRetrieved(const std::list<CardDAVResponse> &list);
	void addressBookCtagRetrieved(std::string ctag);

	void setSchemeAndHostIfNotDoneYet(CardDAVQuery *query);
	void processRedirect(CardDAVQuery *query, belle_sip_message_t *message);

	void fetchVcards();
	void pullVcards(const std::list<CardDAVResponse> &list);

	void queryWellKnown(CardDAVQuery *query);
	void retrieveUserPrincipalUrl();
	void retrieveUserAddressBooksHomeUrl();
	void retrieveAddressBookUrlAndCtag();
	void retrieveAddressBookCtag();

	void sendQuery(CardDAVQuery *query);
	void serverToClientSyncDone(bool success, const std::string &msg);
	void vcardsFetched(const std::list<CardDAVResponse> &vCards);
	void vcardsPulled(const std::list<CardDAVResponse> &vCards);

	static std::string generateUrlFromServerAddressAndUid(const std::string &serverUrl);
	static std::string parseUserPrincipalUrlValueFromXmlResponse(const std::string &body);
	static std::string parseUserAddressBookUrlValueFromXmlResponse(const std::string &body);
	static std::list<CardDAVResponse> parseAddressBookUrlAndCtagValueFromXmlResponse(const std::string &body);
	static std::string parseAddressBookCtagValueFromXmlResponse(const std::string &body);
	static std::list<CardDAVResponse> parseVcardsEtagsFromXmlResponse(const std::string &body);
	static std::list<CardDAVResponse> parseVcardsFromXmlResponse(const std::string &body);
	static void processAuthRequestedFromCarddavRequest(void *data, belle_sip_auth_event_t *event);
	static void processIoErrorFromCarddavRequest(void *data, const belle_sip_io_error_event_t *event);
	static void processResponseFromCarddavRequest(void *data, const belle_http_response_event_t *event);

	std::shared_ptr<FriendList> mFriendList = nullptr;
	std::string mCtag = "";
	std::string mSyncUri = "";
	std::string mScheme = "http";
	std::string mHost = "";
	bool mWellKnownQueried = true;

	std::function<void(const CardDAVContext *context, const std::shared_ptr<Friend> &f)> mContactCreatedCb = nullptr;
	std::function<void(const CardDAVContext *context, const std::shared_ptr<Friend> &f)> mContactRemovedCb = nullptr;
	std::function<void(const CardDAVContext *context,
	                   const std::shared_ptr<Friend> &newFriend,
	                   const std::shared_ptr<Friend> &oldFriend)>
	    mContactUpdatedCb = nullptr;
	std::function<void(const CardDAVContext *context, bool success, const std::string &message)>
	    mSynchronizationDoneCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CARDDAV_CONTEXT_H_
