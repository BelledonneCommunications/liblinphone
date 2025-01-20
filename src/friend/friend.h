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

#ifndef _L_FRIEND_H_
#define _L_FRIEND_H_

#include <memory>

#include "c-wrapper/c-wrapper.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "linphone/api/c-friend.h"
#include "linphone/api/c-types.h"
#include "private_functions.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Account;
class CardDAVContext;
class FriendCbs;
class FriendList;
class FriendDevice;
class FriendPhoneNumber;
class MainDb;
class MainDbPrivate;
class PresenceModel;
class PresenceService;
class SalPresenceOp;
class SalOp;
class Vcard;

class LINPHONE_PUBLIC Friend : public bellesip::HybridObject<LinphoneFriend, Friend>,
                               public UserDataAccessor,
                               public CallbacksHolder<FriendCbs>,
                               public CoreAccessor {
public:
	Friend(std::shared_ptr<Core> core);
	Friend(std::shared_ptr<Core> core, const std::string &address);
	Friend(std::shared_ptr<Core> core, const std::shared_ptr<Vcard> &vcard);
	Friend(const Friend &other) = delete;
	virtual ~Friend();

	Friend *clone() const override;
	virtual std::string toString() const override;

	// Friends
	friend CardDAVContext;
	friend FriendList;
	friend MainDb;
	friend MainDbPrivate;
	friend PresenceModel;
	friend PresenceService;

	// TODO: Remove these friend declarations if possible
	friend void ::linphone_core_remove_friend(LinphoneCore *lc, LinphoneFriend *lf);
	friend void ::linphone_friend_add_incoming_subscription(LinphoneFriend *lf, LinphonePrivate::SalOp *op);
	friend const bctbx_list_t * ::linphone_friend_get_addresses(const LinphoneFriend *lf);
	friend bctbx_list_t * ::linphone_friend_get_insubs(const LinphoneFriend *lf);
	friend SalPresenceOp * ::linphone_friend_get_outsub(const LinphoneFriend *lf);
	friend int ::linphone_friend_get_rc_index(const LinphoneFriend *lf);
	friend long long ::linphone_friend_get_storage_id(const LinphoneFriend *lf);
	friend LinphoneAddress * ::linphone_friend_get_uri(const LinphoneFriend *lf);
	friend void ::linphone_friend_invalidate_subscription(LinphoneFriend *lf);
	friend LinphoneFriend * ::linphone_friend_new_from_config_file(LinphoneCore *lc, int index);
	friend void ::linphone_friend_release(LinphoneFriend *lf);
	friend void ::linphone_friend_remove_incoming_subscription(LinphoneFriend *lf, LinphonePrivate::SalOp *op);
	friend void ::linphone_friend_save(LinphoneFriend *lf, LinphoneCore *lc);
	friend void ::linphone_friend_set_inc_subscribe_pending(LinphoneFriend *lf, bool_t pending);
	friend void ::linphone_friend_set_info(LinphoneFriend *lf, BuddyInfo *info);
	friend void ::linphone_friend_set_outsub(LinphoneFriend *lf, LinphonePrivate::SalPresenceOp *outsub);
	friend void ::linphone_friend_set_out_sub_state(LinphoneFriend *lf, LinphoneSubscriptionState state);
	friend void ::linphone_friend_set_presence_received(LinphoneFriend *lf, bool_t received);
	friend void ::linphone_friend_set_storage_id(LinphoneFriend *lf, unsigned int id);
	friend void ::linphone_friend_set_subscribe(LinphoneFriend *lf, bool_t subscribe);
	friend void ::linphone_friend_set_subscribe_active(LinphoneFriend *lf, bool_t active);
	friend void ::linphone_friend_update_subscribes(LinphoneFriend *lf, bool_t only_when_registered);

	// Setters
	LinphoneStatus setAddress(const std::shared_ptr<const Address> &address);
	LinphoneStatus setIncSubscribePolicy(LinphoneSubscribePolicy policy);
	void setJobTitle(const std::string &title);
	LinphoneStatus setName(const std::string &name);
	void setNativeUri(const std::string &nativeUri);
	void setOrganization(const std::string &organization);
	void setPhoto(const std::string &pictureUri);
	void setPresenceModel(const std::shared_ptr<PresenceModel> &model);
	void setPresenceModelForUriOrTel(const std::string &uriOrTel, const std::shared_ptr<PresenceModel> &model);
	void setRefKey(const std::string &key);
	void setStarred(bool starred);
	void setVcard(const std::shared_ptr<Vcard> &vcard);

	// Getters
	const std::shared_ptr<Address> getAddress() const;
	const std::list<std::shared_ptr<Address>> &getAddresses() const;
	const bctbx_list_t *getAddressesCList() const;
	int getCapabilities() const;
	float getCapabilityVersion(LinphoneFriendCapability capability) const;
	LinphoneConsolidatedPresence getConsolidatedPresence() const;
	LinphoneSubscribePolicy getIncSubscribePolicy() const;
	BuddyInfo *getInfo() const;
	const std::string &getJobTitle() const;
	const std::string &getOrganization() const;
	const std::string &getName() const;
	const std::string &getNativeUri() const;
	std::list<std::string> getPhoneNumbers() const;
	std::list<std::shared_ptr<FriendPhoneNumber>> getPhoneNumbersWithLabel() const;
	const std::string &getPhoto() const;
	const std::shared_ptr<PresenceModel> getPresenceModel() const;
	const std::shared_ptr<PresenceModel> &getPresenceModelForUriOrTel(const std::string &uriOrTel) const;
	const std::shared_ptr<PresenceModel> &
	getPresenceModelForAddress(const std::shared_ptr<const Address> &address) const;
	const std::string &getRefKey() const;
	bool getStarred() const;
	LinphoneSubscriptionState getSubscriptionState() const;
	std::shared_ptr<Vcard> getVcard() const;

	const std::list<std::shared_ptr<FriendDevice>> getDevices() const;
	const std::list<std::shared_ptr<FriendDevice>> getDevicesForAddress(const Address &address) const;
	LinphoneSecurityLevel getSecurityLevel() const;
	LinphoneSecurityLevel getSecurityLevelForAddress(const Address &address) const;

	// Other
	void addAddress(const std::shared_ptr<const Address> &address);
	void addPhoneNumber(const std::string &phoneNumber);
	void addPhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber);
	bool createVcard(const std::string &name);
	void done();
	void edit();
	LinphoneStatus enableSubscribes(bool enabled);
	bool hasCapability(const LinphoneFriendCapability capability) const;
	bool hasCapabilityWithVersion(const LinphoneFriendCapability capability, float version) const;
	bool hasCapabilityWithVersionOrMore(const LinphoneFriendCapability capability, float version) const;
	bool hasPhoneNumber(const std::string &searchedPhoneNumber) const;
	bool inList() const;
	FriendList *getFriendList() const;
	bool isPresenceReceived() const;
	void remove();
	void removeAddress(const std::shared_ptr<const Address> &address);
	void removePhoneNumber(const std::string &phoneNumber);
	void removePhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber);
	bool subscribesEnabled() const;

private:
	void addAddressesAndNumbersIntoMaps(const std::shared_ptr<FriendList> &list);
	void addFriendToListMapIfNotInItYet(const std::string &uri);
	void addIncomingSubscription(SalOp *op);
	void addPresenceModelForUriOrTel(const std::string &uriOrTel, const std::shared_ptr<PresenceModel> &model);
	void apply();
	void clearPresenceModels();
	void closeIncomingSubscriptions();
	void closeSubscriptions();
	void doSubscribe();
	bool hasPhoneNumber(const std::shared_ptr<Account> &account, const std::string &searchedPhoneNumber) const;
	void invalidateSubscription();
	void notify(const std::shared_ptr<PresenceModel> &presence);
	const std::string &phoneNumberToSipUri(const std::string &phoneNumber) const;
	void presenceReceived(const std::shared_ptr<FriendList> list,
	                      const std::string &uri,
	                      const std::shared_ptr<PresenceModel> &model);
	void releaseOps();
	void removeFriendFromListMapIfAlreadyInIt(const std::string &uri);
	void removeFromDb();
	void removeIncomingSubscription(SalOp *op);
	void saveInDb();
	const std::string &sipUriToPhoneNumber(const std::string &uri) const;
	void unsubscribe();
	void updateSubscribes(bool onlyWhenRegistered);

	static std::string capabilityToName(const LinphoneFriendCapability capability);
	static LinphoneFriendCapability nameToCapability(const std::string &name);

	static LinphoneSecurityLevel getSecurityLevelFromChatRoomSecurityLevel(AbstractChatRoom::SecurityLevel level);
	static LinphoneSecurityLevel getSecurityLevelForDevices(const std::list<std::shared_ptr<FriendDevice>> &devices);

	LinphoneSubscribePolicy mSubscribePolicy = LinphoneSPDeny;
	LinphoneSubscriptionState mOutSubState;
	bool mSubscribe = false;
	bool mSubscribeActive = false;
	bool mIsStarred = false;
	bool mCommit = false;
	bool mIncSubscribePending = false;
	bool mPresenceReceived = false;
	bool mInitialSubscribesSent = false; /* Used to know if initial subscribe message was sent or not. */
	std::shared_ptr<Address> mUri;
	std::string mNativeUri;
	std::string mRefKey;
	long long mStorageId = -1;
	int mRcIndex = -1;

	SalPresenceOp *mOutSub = nullptr;
	std::list<SalOp *> mInSubs; /* There can be multiple instances of a same Friend that subscribe to our presence. */
	std::map<std::shared_ptr<Address>, std::shared_ptr<PresenceModel>>
	    mPresenceModels; /* It associates SIP URIs and phone numbers with their respective presence models. */
	mutable std::map<std::string, std::string> mPhoneNumberToSipUriMap;
	mutable std::map<std::string, std::string> mSipUriToPhoneNumberMap;

	BuddyInfo *mInfo = nullptr;
	std::shared_ptr<Vcard> mVcard;
	FriendList *mFriendList = nullptr;

	mutable ListHolder<Address> mAddresses;
	mutable std::string mName;
	mutable std::list<std::shared_ptr<FriendDevice>> mDevices;
};

class FriendCbs : public bellesip::HybridObject<LinphoneFriendCbs, FriendCbs>, public Callbacks {
public:
	LinphoneFriendCbsPresenceReceivedCb getPresenceReceived() const;
	void setPresenceReceived(LinphoneFriendCbsPresenceReceivedCb cb);

private:
	LinphoneFriendCbsPresenceReceivedCb mPresenceReceivedCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_FRIEND_H_
