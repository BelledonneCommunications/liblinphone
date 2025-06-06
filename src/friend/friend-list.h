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

#ifndef _L_FRIEND_LIST_H_
#define _L_FRIEND_LIST_H_

#include "belle-sip/object++.hh"

#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "private_functions.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Account;
#if VCARD_ENABLED
class CardDAVContext;
#endif
class Event;
class Friend;
class FriendListCbs;
class MainDb;
class MainDbPrivate;
class PresenceModel;
class SalOp;
class Vcard;

class LINPHONE_PUBLIC FriendList : public bellesip::HybridObject<LinphoneFriendList, FriendList>,
                                   public UserDataAccessor,
                                   public CallbacksHolder<FriendListCbs>,
                                   public CoreAccessor {
public:
	FriendList(std::shared_ptr<Core> core);
	FriendList(const FriendList &other) = delete;
	virtual ~FriendList();
	void release();

	FriendList *clone() const override;

	// Friends
#if VCARD_ENABLED
	friend CardDAVContext;
#endif
	friend Friend;
	friend MainDb;
	friend MainDbPrivate;
	// TODO: To remove when possible
	friend void ::linphone_core_add_friend_list(LinphoneCore *lc, LinphoneFriendList *list);
	friend LinphoneFriend * ::linphone_core_find_friend_by_inc_subscribe(const LinphoneCore *lc,
	                                                                     LinphonePrivate::SalOp *op);
	friend LinphoneFriend * ::linphone_core_find_friend_by_out_subscribe(const LinphoneCore *lc,
	                                                                     LinphonePrivate::SalOp *op);
	friend LinphoneFriend * ::linphone_core_find_friend_by_phone_number(const LinphoneCore *lc,
	                                                                    const char *phoneNumber);
	friend void ::linphone_core_invalidate_friend_subscriptions(LinphoneCore *lc);
	friend void ::linphone_core_iterate(LinphoneCore *lc);
	friend void ::linphone_core_remove_friend_list(LinphoneCore *lc, LinphoneFriendList *list);
	friend const bctbx_list_t * ::linphone_friend_list_get_dirty_friends_to_update(const LinphoneFriendList *lfl);
	friend LinphoneEvent * ::linphone_friend_list_get_event(const LinphoneFriendList *list);
	friend int ::linphone_friend_list_get_expected_notification_version(const LinphoneFriendList *list);
	friend const bctbx_list_t * ::linphone_friend_list_get_friends(const LinphoneFriendList *list);
	friend const char * ::linphone_friend_list_get_revision(const LinphoneFriendList *lfl);
	friend long long ::linphone_friend_list_get_storage_id(const LinphoneFriendList *list);
	friend void ::linphone_friend_list_invalidate_friends_maps(LinphoneFriendList *list);
	friend void ::linphone_friend_list_notify_presence_received(LinphoneFriendList *list,
	                                                            LinphoneEvent *lev,
	                                                            const LinphoneContent *body);
	friend void ::linphone_friend_list_subscription_state_changed(LinphoneCore *lc,
	                                                              LinphoneEvent *lev,
	                                                              LinphoneSubscriptionState state);
	friend void ::linphone_friend_list_update_subscriptions(LinphoneFriendList *list);
	friend int ::create_friend_list_from_db(void *data, int argc, char **argv, char **colName);

	// Setters
	void setDisplayName(const std::string &displayName);
	void setRlsAddress(const std::shared_ptr<const Address> &rlsAddr);
	void setRlsUri(const std::string &rlsUri);
	void setSubscriptionBodyless(bool bodyless);
	void setType(LinphoneFriendListType type);
	void setUri(const std::string &uri);

	// Getters
	const std::string &getDisplayName() const;
	const std::list<std::shared_ptr<Friend>> &getFriends() const;
	const bctbx_list_t *getFriendsCList() const;
	const std::shared_ptr<Address> &getRlsAddress() const;
	const std::string &getRlsUri() const;
	LinphoneFriendListType getType() const;
	const std::string &getUri() const;
	const std::list<std::shared_ptr<Friend>> &getDirtyFriendsToUpdate() const;
	bool isSubscriptionBodyless() const;

	// Other
	LinphoneFriendListStatus addFriend(const std::shared_ptr<Friend> &lf);
	LinphoneFriendListStatus addLocalFriend(const std::shared_ptr<Friend> &lf);
	bool databaseStorageEnabled() const;
	void enableDatabaseStorage(bool enable);
	void enableSubscriptions(bool enabled);
	void exportFriendsAsVcard4File(const std::string &vcardFile) const;
	std::shared_ptr<Friend> findFriendByAddress(const std::shared_ptr<const Address> &address) const;
	std::shared_ptr<Friend> findFriendByPhoneNumber(const std::string &phoneNumber) const;
	std::shared_ptr<Friend> findFriendByRefKey(const std::string &refKey) const;
	std::shared_ptr<Friend> findFriendByUri(const std::string &uri) const;
	std::list<std::shared_ptr<Friend>> findFriendsByAddress(const std::shared_ptr<const Address> &address) const;
	std::list<std::shared_ptr<Friend>> findFriendsByUri(const std::string &uri) const;
	LinphoneStatus importFriendsFromVcard4Buffer(const std::string &vcardBuffer);
	LinphoneStatus importFriendsFromVcard4File(const std::string &vcardFile);
	void notifyPresence(const std::shared_ptr<PresenceModel> &model) const;
	LinphoneFriendListStatus removeFriend(const std::shared_ptr<Friend> &lf);
	void removeFriends();
	bool subscriptionsEnabled() const;
	void synchronizeFriendsFromServer();
	void updateDirtyFriends();
	void updateRevision(const std::string &revision);

	const std::string &getRevision() const {
		return mRevision;
	}

private:
	LinphoneFriendListStatus addFriend(const std::shared_ptr<Friend> &lf, bool synchronize);
	void closeSubscriptions();
	std::string createResourceListXml() const;
	std::shared_ptr<Friend> findFriendByIncSubscribe(SalOp *op) const;
	std::shared_ptr<Friend> findFriendByOutSubscribe(SalOp *op) const;
	std::shared_ptr<Friend> findFriendByPhoneNumber(const std::shared_ptr<Account> &account,
	                                                const std::string &normalizedPhoneNumber) const;
	std::shared_ptr<Address> getRlsAddressWithCoreFallback() const;
	bool hasSubscribeInactive() const;
	LinphoneFriendListStatus importFriend(const std::shared_ptr<Friend> &lf, bool synchronize);
	LinphoneStatus importFriendsFromVcard4(const std::list<std::shared_ptr<Vcard>> &vcards);
	void invalidateFriendsMaps();
	void invalidateSubscriptions();
	void notifyPresenceReceived(const std::shared_ptr<const Content> &content);
	void parseMultipartRelatedBody(const std::shared_ptr<const Content> &content, const std::string &firstPartBody);
	void deleteFriend(const std::shared_ptr<Friend> &lf, bool removeFromServer);
	LinphoneFriendListStatus removeFriend(const std::shared_ptr<Friend> &lf, bool removeFromServer);
	void removeFriends(bool removeFromServer);
	void removeFromDb();
	void saveInDb();
	void sendListSubscription();
	void sendListSubscriptionWithBody(const std::shared_ptr<Address> &address);
	void sendListSubscriptionWithoutBody(const std::shared_ptr<Address> &address);
	void setFriends(const std::list<std::shared_ptr<Friend>> &friends);
	void syncBctbxFriends() const;
	void updateSubscriptions();

	static void
	subscriptionStateChanged(LinphoneCore *lc, const std::shared_ptr<Event> event, LinphoneSubscriptionState state);
#ifdef VCARD_ENABLED
	void createCardDavContextIfNotDoneYet();
	void carddavCreated(const std::shared_ptr<Friend> &f);
	void carddavDone(bool success, const std::string &msg);
	void carddavRemoved(const std::shared_ptr<Friend> &f);
	void carddavUpdated(const std::shared_ptr<Friend> &newFriend, const std::shared_ptr<Friend> &oldFriend);
#endif

	std::shared_ptr<Event> mEvent;
	std::string mDisplayName;
	std::string mRlsUri; // This field must be kept in sync with mRlsAddr
	std::shared_ptr<Address> mRlsAddr;
	mutable ListHolder<Friend> mFriendsList;
	std::map<std::string, std::shared_ptr<Friend>> mFriendsMapByRefKey;
	std::multimap<std::string, std::shared_ptr<Friend>> mFriendsMapByUri;
	std::array<unsigned char, 16> *mContentDigest = nullptr;
	int mExpectedNotificationVersion;
	long long mStorageId = -1;
	std::string mUri;
	std::list<std::shared_ptr<Friend>> mDirtyFriendsToUpdate;
	bctbx_list_t *mBctbxDirtyFriendsToUpdate = nullptr; // This field must be kept in sync with mDirtyFriendsToUpdate
	std::string mRevision = "";
	bool mSubscriptionsEnabled = false;
	bool mBodylessSubscription = false;
	LinphoneFriendListType mType = LinphoneFriendListTypeDefault;
	bool mStoreInDb = false;
#if VCARD_ENABLED
	std::shared_ptr<CardDAVContext> mCardDavContext;
#endif
};

class FriendListCbs : public bellesip::HybridObject<LinphoneFriendListCbs, FriendListCbs>, public Callbacks {
public:
	// Getters
	LinphoneFriendListCbsContactCreatedCb getContactCreated() const;
	LinphoneFriendListCbsContactDeletedCb getContactDeleted() const;
	LinphoneFriendListCbsContactUpdatedCb getContactUpdated() const;
	LinphoneFriendListCbsPresenceReceivedCb getPresenceReceived() const;
	LinphoneFriendListCbsNewSipAddressDiscoveredCb getNewlyDiscoveredSipAddress() const;
	LinphoneFriendListCbsSyncStateChangedCb getSyncStatusChanged() const;

	// Setters
	void setContactCreated(LinphoneFriendListCbsContactCreatedCb cb);
	void setContactDeleted(LinphoneFriendListCbsContactDeletedCb cb);
	void setContactUpdated(LinphoneFriendListCbsContactUpdatedCb cb);
	void setPresenceReceived(LinphoneFriendListCbsPresenceReceivedCb cb);
	void setNewlyDiscoveredSipAddress(LinphoneFriendListCbsNewSipAddressDiscoveredCb cb);
	void setSyncStatusChanged(LinphoneFriendListCbsSyncStateChangedCb cb);

private:
	LinphoneFriendListCbsContactCreatedCb mContactCreatedCb = nullptr;
	LinphoneFriendListCbsContactDeletedCb mContactDeletedCb = nullptr;
	LinphoneFriendListCbsContactUpdatedCb mContactUpdatedCb = nullptr;
	LinphoneFriendListCbsPresenceReceivedCb mPresenceReceivedCb = nullptr;
	LinphoneFriendListCbsNewSipAddressDiscoveredCb mNewSipAddressDiscoveredCb = nullptr;
	LinphoneFriendListCbsSyncStateChangedCb mSyncStatusChangedCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_FRIEND_LIST_H_
