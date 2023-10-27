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

#include "address/address.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "friend-list.h"
#include "friend.h"
#include "friend_phone_number.h"
#include "vcard/vcard.h"

#include "c-wrapper/internal/c-tools.h"
#include "core/core.h"

#include "linphone/sipsetup.h"
#include "presence/presence-model.h"
#include "private.h" // TODO: To remove if possible
#include "private_functions.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

struct FriendCapabilityNameMap {
	std::string_view name;
	LinphoneFriendCapability capability;
};

static constexpr std::array<FriendCapabilityNameMap, 3> friendCapabilityNameMap{
    {{"groupchat", LinphoneFriendCapabilityGroupChat},
     {"lime", LinphoneFriendCapabilityLimeX3dh},
     {"ephemeral", LinphoneFriendCapabilityEphemeralMessages}}};

static const std::string emptyString;

// -----------------------------------------------------------------------------

Friend::Friend(LinphoneCore *lc) : CoreAccessor(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr) {
}

Friend::Friend(LinphoneCore *lc, const std::string &address) : Friend(lc) {
	setAddress(Address::create(address));
}

Friend::Friend(LinphoneCore *lc, const std::shared_ptr<Vcard> &vcard) : Friend(lc) {
	// Currently presence takes too much time when dealing with hundreds of friends, so I disable it for now
	mSubscribePolicy = LinphoneSPDeny;
	mSubscribe = false;
	setVcard(vcard);
}

Friend::~Friend() {
	releaseOps();
	clearPresenceModels();
	if (mInfo) buddy_info_free(mInfo);
	if (mBctbxAddresses) bctbx_list_free(mBctbxAddresses);
}

Friend *Friend::clone() const {
	return nullptr;
}

std::string Friend::toString() const {
	if (mUri) return mUri->asString();
	else return std::string();
}

// -----------------------------------------------------------------------------

LinphoneStatus Friend::setAddress(const std::shared_ptr<const Address> &address) {
	if (!address) return -1;
	Address *newAddress = address->clone();
	newAddress->clean();

	const std::shared_ptr<Address> previousAddress = getAddress();
	if (previousAddress && mFriendList) {
		std::string strAddress = previousAddress->asStringUriOnly();
		removeFriendFromListMapIfAlreadyInIt(strAddress);
	}

	std::string strAddress = newAddress->asStringUriOnly();
	if (mFriendList) addFriendToListMapIfNotInItYet(strAddress);

	if (linphone_core_vcard_supported()) {
		if (!mVcard) {
			const std::string name =
			    newAddress->getDisplayName().empty() ? newAddress->getUsername() : newAddress->getDisplayName();
			createVcard(name);
		}
		if (mVcard) mVcard->editMainSipAddress(strAddress);
		newAddress->unref();
	} else {
		mUri = newAddress->getSharedFromThis();
	}

	return 0;
}

LinphoneStatus Friend::setIncSubscribePolicy(LinphoneSubscribePolicy policy) {
	mSubscribePolicy = policy;
	return 0;
}

void Friend::setJobTitle(const std::string &title) {
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->setJobTitle(title);
	}
}

LinphoneStatus Friend::setName(const std::string &name) {
	if (linphone_core_vcard_supported()) {
		if (!mVcard) createVcard(name);
		mVcard->setFullName(name);
	} else {
		if (!mUri) {
			lWarning()
			    << "Friend::setAddress() must be called before Friend::setName() to be able to set display name.";
			return -1;
		}
		mUri->setDisplayName(name);
	}
	return 0;
}

void Friend::setNativeUri(const std::string &nativeUri) {
	mNativeUri = nativeUri;
}

void Friend::setOrganization(const std::string &organization) {
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->setOrganization(organization);
	}
}

void Friend::setPhoto(const std::string &pictureUri) {
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->setPhoto(pictureUri);
	}
}

void Friend::setPresenceModel(const std::shared_ptr<PresenceModel> &model) {
	const std::shared_ptr<Address> addr = getAddress();
	if (addr) {
		std::string uri = addr->asStringUriOnly();
		setPresenceModelForUriOrTel(uri, model);
	}
}

void Friend::setPresenceModelForUriOrTel(const std::string &uriOrTel, const std::shared_ptr<PresenceModel> &model) {
	addPresenceModelForUriOrTel(uriOrTel, model);
}

void Friend::setRefKey(const std::string &key) {
	mRefKey = key;
	if (mFriendList) saveInDb();
}

void Friend::setStarred(bool starred) {
	mIsStarred = starred;
}

void Friend::setVcard(const std::shared_ptr<Vcard> &vcard) {
	if (!linphone_core_vcard_supported()) return;

	const std::string fullname = vcard->getFullName();
	if (fullname.empty()) {
		lWarning() << "Trying to set an invalid vCard (no fullname) to friend, aborting";
		return;
	}

	mVcard = vcard;
	if (mFriendList) saveInDb();
}

// -----------------------------------------------------------------------------

const std::shared_ptr<Address> Friend::getAddress() const {
	if (linphone_core_vcard_supported()) {
		if (mVcard) {
			const std::list<std::shared_ptr<Address>> sipAddresses = mVcard->getSipAddresses();
			if (!sipAddresses.empty()) return sipAddresses.front();
		}
		return nullptr;
	}

	if (mUri) return mUri;
	return nullptr;
}

const std::list<std::shared_ptr<Address>> &Friend::getAddresses() const {
	if (linphone_core_vcard_supported()) {
		mAddresses = mVcard->getSipAddresses();
	} else {
		mAddresses.clear();
		mAddresses.push_back(mUri);
	}
	syncBctbxAddresses();
	return mAddresses;
}

int Friend::getCapabilities() const {
	int capabilities = 0;

	const std::list<std::shared_ptr<Address>> addrs = getAddresses();
	for (auto addr : addrs) {
		std::string uri = addr->asStringUriOnly();
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(uri);
		if (model) capabilities |= model->getCapabilities();
	}

	const std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(phoneNumber);
		if (model) capabilities |= model->getCapabilities();
	}

	return capabilities;
}

float Friend::getCapabilityVersion(LinphoneFriendCapability capability) const {
	float version = -1.0;

	const std::list<std::shared_ptr<Address>> addrs = getAddresses();
	for (auto addr : addrs) {
		std::string uri = addr->asStringUriOnly();
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(uri);
		if (model) {
			float presence_version = model->getCapabilityVersion(capability);
			if (presence_version > version) version = presence_version;
		}
	}

	const std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(phoneNumber);
		if (model) {
			float presence_version = model->getCapabilityVersion(capability);
			if (presence_version > version) version = presence_version;
		}
	}

	return version;
}

LinphoneConsolidatedPresence Friend::getConsolidatedPresence() const {
	LinphoneConsolidatedPresence result = LinphoneConsolidatedPresenceOffline;

	const std::list<std::shared_ptr<Address>> addrs = getAddresses();
	for (auto addr : addrs) {
		std::string uri = addr->asStringUriOnly();
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(uri);
		if (model) {
			LinphoneConsolidatedPresence consolidated = model->getConsolidatedPresence();
			if (consolidated != LinphoneConsolidatedPresenceOffline) {
				result = consolidated;
				if (result == LinphoneConsolidatedPresenceOnline) break;
			}
		}
	}
	if (result == LinphoneConsolidatedPresenceOnline) return result;

	const std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(phoneNumber);
		if (model) {
			LinphoneConsolidatedPresence consolidated = model->getConsolidatedPresence();
			if (consolidated != LinphoneConsolidatedPresenceOffline) {
				result = consolidated;
				if (result == LinphoneConsolidatedPresenceOnline) break;
			}
		}
	}

	return result;
}

LinphoneSubscribePolicy Friend::getIncSubscribePolicy() const {
	return mSubscribePolicy;
}

BuddyInfo *Friend::getInfo() const {
	return mInfo;
}

const std::string &Friend::getJobTitle() const {
	if (linphone_core_vcard_supported() && mVcard) {
		return mVcard->getJobTitle();
	}
	return emptyString;
}

const std::string &Friend::getOrganization() const {
	if (linphone_core_vcard_supported() && mVcard) {
		return mVcard->getOrganization();
	}
	return emptyString;
}

const std::string &Friend::getName() const {
	if (linphone_core_vcard_supported() && mVcard) {
		mName = mVcard->getFullName();
	}
	if (mName.empty() && mUri) {
		mName = mUri->getDisplayName();
	}
	return mName;
}

const std::string &Friend::getNativeUri() const {
	return mNativeUri;
}

std::list<std::string> Friend::getPhoneNumbers() const {
	if (linphone_core_vcard_supported() && mVcard) {
		return mVcard->getPhoneNumbers();
	}
	return std::list<std::string>();
}

std::list<std::shared_ptr<FriendPhoneNumber>> Friend::getPhoneNumbersWithLabel() const {
	if (linphone_core_vcard_supported() && mVcard) {
		return mVcard->getPhoneNumbersWithLabel();
	}
	return std::list<std::shared_ptr<FriendPhoneNumber>>();
}

const std::string &Friend::getPhoto() const {
	if (linphone_core_vcard_supported() && mVcard) {
		return mVcard->getPhoto();
	}
	return emptyString;
}

const std::shared_ptr<PresenceModel> Friend::getPresenceModel() const {
	std::shared_ptr<PresenceModel> result = nullptr;
	time_t presenceModelLatestTimestamp = 0;

	std::list<shared_ptr<Address>> addrs = getAddresses();
	for (auto addr : addrs) {
		std::string uri = addr->asStringUriOnly();
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(uri);
		if (model) {
			time_t timestamp = model->getTimestamp();
			if (!result || (timestamp > presenceModelLatestTimestamp)) {
				presenceModelLatestTimestamp = timestamp;
				result = model;
			}
		}
	}

	std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(phoneNumber);
		if (model) {
			time_t timestamp = model->getTimestamp();
			if (!result || (timestamp > presenceModelLatestTimestamp)) {
				presenceModelLatestTimestamp = timestamp;
				result = model;
			}
		}
	}

	return result;
}

const std::shared_ptr<PresenceModel> Friend::getPresenceModelForUriOrTel(const std::string &uriOrTel) const {
	if (mPresenceModels.empty()) return nullptr;
	const std::shared_ptr<Address> uriOrTelAddr = getCore()->interpretUrl(uriOrTel, false);
	if (!uriOrTelAddr) return nullptr;
	const auto it = std::find_if(mPresenceModels.cbegin(), mPresenceModels.cend(),
	                             [&](const auto &elem) { return elem.first->weakEqual(*uriOrTelAddr); });
	return (it == mPresenceModels.cend()) ? nullptr : it->second;
}

const std::string &Friend::getRefKey() const {
	return mRefKey;
}

bool Friend::getStarred() const {
	return mIsStarred;
}

LinphoneSubscriptionState Friend::getSubscriptionState() const {
	return mOutSubState;
}

std::shared_ptr<Vcard> Friend::getVcard() const {
	return linphone_core_vcard_supported() ? mVcard : nullptr;
}

// -----------------------------------------------------------------------------

void Friend::addAddress(const std::shared_ptr<const Address> &address) {
	if (!address) return;

	std::shared_ptr<Address> newAddr = address->clone()->getSharedFromThis();
	newAddr->clean();
	std::string uri = newAddr->asStringUriOnly();
	if (mFriendList) addFriendToListMapIfNotInItYet(uri);

	if (linphone_core_vcard_supported()) {
		if (mVcard) mVcard->addSipAddress(uri);
	} else if (!mUri) mUri = newAddr;
	newAddr->unref();
}

void Friend::addPhoneNumber(const std::string &phoneNumber) {
	if (phoneNumber.empty()) return;
	if (mFriendList) {
		const std::string uri = phoneNumberToSipUri(phoneNumber);
		addFriendToListMapIfNotInItYet(uri);
	}
	if (linphone_core_vcard_supported()) {
		if (!mVcard) createVcard(phoneNumber);
		mVcard->addPhoneNumber(phoneNumber);
	}
}

void Friend::addPhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber) {
	if (!phoneNumber) return;
	const std::string &phone = phoneNumber->getPhoneNumber();
	if (phone.empty()) return;
	if (mFriendList) addFriendToListMapIfNotInItYet(phoneNumberToSipUri(phone));
	if (linphone_core_vcard_supported()) {
		if (!mVcard) createVcard(phone);
		mVcard->addPhoneNumberWithLabel(phoneNumber);
	}
}

bool Friend::createVcard(const std::string &name) {
	if (name.empty()) {
		lError() << "Can't create vCard for friend [" << toC() << "] with name [" << L_STRING_TO_C(name) << "]";
		return false;
	}
	if (!linphone_core_vcard_supported()) {
		lWarning() << "VCard support is not builtin";
		return false;
	}
	if (mVcard) {
		lError() << "Friend already has a VCard";
		return false;
	}

	std::shared_ptr<Vcard> vcard = Vcard::create();
	vcard->setFullName(name);
	setVcard(vcard);
	lDebug() << "VCard created for friend [" << toC() << "]";
	return true;
}

void Friend::done() {
	if (linphone_core_vcard_supported() && mVcard) {
		if (mVcard->compareMd5Hash()) {
			lDebug() << "vCard's md5 has changed, mark friend as dirty and clear sip addresses list cache";
			mVcard->cleanCache();
			if (mFriendList) {
				mFriendList->mDirtyFriendsToUpdate.push_back(getSharedFromThis());
				mFriendList->mBctbxDirtyFriendsToUpdate =
				    bctbx_list_append(mFriendList->mBctbxDirtyFriendsToUpdate, toC());
			}
		}
	}
	apply();
	if (mFriendList) saveInDb();
}

void Friend::edit() {
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->computeMd5Hash();
	}
}

LinphoneStatus Friend::enableSubscribes(bool enabled) {
	mSubscribe = enabled;
	return 0;
}

bool Friend::hasCapability(const LinphoneFriendCapability capability) const {
	return !!(getCapabilities() & capability);
}

bool Friend::hasCapabilityWithVersion(const LinphoneFriendCapability capability, float version) const {
	std::list<std::shared_ptr<Address>> addrs = getAddresses();
	for (auto addr : addrs) {
		std::string uri = addr->asStringUriOnly();
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(addr->asStringUriOnly());
		if (model && model->hasCapabilityWithVersion(capability, version)) return true;
	}

	std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(phoneNumber);
		if (model && model->hasCapabilityWithVersion(capability, version)) return true;
	}

	return false;
}

bool Friend::hasCapabilityWithVersionOrMore(const LinphoneFriendCapability capability, float version) const {
	std::list<std::shared_ptr<Address>> addrs = getAddresses();
	for (auto addr : addrs) {
		std::string uri = addr->asStringUriOnly();
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(addr->asStringUriOnly());
		if (model && model->hasCapabilityWithVersionOrMore(capability, version)) return true;
	}

	std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		const std::shared_ptr<PresenceModel> &model = getPresenceModelForUriOrTel(phoneNumber);
		if (model && model->hasCapabilityWithVersionOrMore(capability, version)) return true;
	}

	return false;
}

bool Friend::hasPhoneNumber(const std::string &searchedPhoneNumber) const {
	if (searchedPhoneNumber.empty()) return false;

	LinphoneAccount *account = linphone_core_get_default_account(getCore()->getCCore());
	/* account can be null, both linphone_account_is_phone_number and linphone_account_normalize_phone_number
	can handle it */
	if (!linphone_account_is_phone_number(account, L_STRING_TO_C(searchedPhoneNumber))) {
		lWarning() << "Phone number [" << L_STRING_TO_C(searchedPhoneNumber) << "] isn't valid";
		return false;
	}

	if (!linphone_core_vcard_supported()) {
		lWarning() << "SDK built without vCard support, can't do a phone number search without it";
		return false;
	}

	bool found = false;
	const bctbx_list_t *accounts = linphone_core_get_account_list(getCore()->getCCore());
	for (const bctbx_list_t *elem = accounts; elem != nullptr; elem = bctbx_list_next(elem)) {
		account = (LinphoneAccount *)bctbx_list_get_data(elem);
		char *normalizedPhoneNumber =
		    linphone_account_normalize_phone_number(account, L_STRING_TO_C(searchedPhoneNumber));
		found = hasPhoneNumber(Account::getSharedFromThis(account), normalizedPhoneNumber);
		if (normalizedPhoneNumber) bctbx_free(normalizedPhoneNumber);
		if (found) break;
	}

	return found;
}

bool Friend::inList() const {
	return mFriendList != nullptr;
}

bool Friend::isPresenceReceived() const {
	return mPresenceReceived;
}

void Friend::remove() {
	if (mFriendList) mFriendList->removeFriend(getSharedFromThis());

	if (mRcIndex >= 0) {
		LinphoneCore *lc = getCore()->getCCore();
		LinphoneConfig *config = linphone_core_get_config(lc);
		if (config) {
			char section[50];
			sprintf(section, "friend_%i", mRcIndex);
			linphone_config_clean_section(config, section);
			linphone_core_config_sync(lc);
			mRcIndex = -1;
		}
	}
}

void Friend::removeAddress(const std::shared_ptr<const Address> &address) {
	if (!address) return;

	std::string uri = address->asStringUriOnly();
	if (mFriendList) removeFriendFromListMapIfAlreadyInIt(uri);
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->removeSipAddress(uri);
	}
}

void Friend::removePhoneNumber(const std::string &phoneNumber) {
	if (phoneNumber.empty()) return;

	if (mFriendList) removeFriendFromListMapIfAlreadyInIt(phoneNumberToSipUri(phoneNumber));
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->removePhoneNumber(phoneNumber);
	}
}

void Friend::removePhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber) {
	if (!phoneNumber) return;

	const std::string &phone = phoneNumber->getPhoneNumber();
	if (phone.empty()) return;

	if (mFriendList) removeFriendFromListMapIfAlreadyInIt(phoneNumberToSipUri(phone));
	if (linphone_core_vcard_supported() && mVcard) {
		mVcard->removePhoneNumberWithLabel(phoneNumber);
	}
}

bool Friend::subscribesEnabled() const {
	return mSubscribe;
}

// -----------------------------------------------------------------------------

void Friend::addAddressesAndNumbersIntoMaps(const std::shared_ptr<FriendList> &list) {
	if (!mRefKey.empty()) list->mFriendsMapByRefKey.insert({mRefKey, getSharedFromThis()});

	std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		addFriendToListMapIfNotInItYet(phoneNumberToSipUri(phoneNumber));
	}

	std::list<shared_ptr<Address>> addresses = getAddresses();
	for (auto address : addresses) {
		addFriendToListMapIfNotInItYet(address->asStringUriOnly());
	}
}

void Friend::addFriendToListMapIfNotInItYet(const std::string &uri) {
	if (!mFriendList || uri.empty()) return;
	bool found = false;
	for (auto [it, rangeEnd] = mFriendList->mFriendsMapByUri.equal_range(uri); it != rangeEnd; it++) {
		if (it->second == getSharedFromThis()) found = true;
	}
	if (!found) {
		mFriendList->mFriendsMapByUri.insert({uri, getSharedFromThis()});
	}
}

void Friend::addIncomingSubscription(SalOp *op) {
	/* Ownership of the op is transfered from sal to the Friend */
	mInSubs.push_back(op);
}

void Friend::addPresenceModelForUriOrTel(const std::string &uriOrTel, const std::shared_ptr<PresenceModel> &model) {
	const std::shared_ptr<Address> uriOrTelAddr = getCore()->interpretUrl(uriOrTel, false);
	if (!uriOrTelAddr) return;
	auto it = std::find_if(mPresenceModels.begin(), mPresenceModels.end(),
	                       [&](const auto &elem) { return elem.first->weakEqual(*uriOrTelAddr); });
	if (it == mPresenceModels.end()) mPresenceModels.insert({uriOrTelAddr, model});
	else it->second = model;
}

void Friend::apply() {
	const std::shared_ptr<Address> addr = getAddress();
	if (!addr) {
		lDebug() << "No sip url defined in friend " << getName();
		return;
	}

	if (!linphone_core_ready(getCore()->getCCore())) {
		/* core is not ready, deffering subscription */
		mCommit = true;
		return;
	}

	std::shared_ptr<PresenceModel> model = nullptr;
	if (mIncSubscribePending) {
		switch (mSubscribePolicy) {
			case LinphoneSPWait:
				model = PresenceModel::create(LinphonePresenceActivityOther, "Waiting for user acceptance");
				notify(model);
				break;
			case LinphoneSPAccept:
				if (getCore()->getCCore())
					notify(PresenceModel::getSharedFromThis(getCore()->getCCore()->presence_model));
				break;
			case LinphoneSPDeny:
				notify(nullptr);
				break;
		}
		mIncSubscribePending = false;
	}

	if ((mSubscribePolicy == LinphoneSPDeny) && !mInSubs.empty()) {
		closeIncomingSubscriptions();
	}

	updateSubscribes(linphone_core_should_subscribe_friends_only_when_registered(getCore()->getCCore()));

	lDebug() << "Friend::apply() done.";
	getCore()->getCCore()->bl_refresh = true;
	mCommit = false;
}

void Friend::clearPresenceModels() {
	mPresenceModels.clear();
}

void Friend::closeIncomingSubscriptions() {
	for (auto op : mInSubs) {
		static_cast<SalPresenceOp *>(op)->notifyPresenceClose();
	}
	for (auto op : mInSubs) {
		op->release();
	}
	mInSubs.clear();
}

void Friend::closeSubscriptions() {
	unsubscribe();
	closeIncomingSubscriptions();
}

void Friend::doSubscribe() {
	const std::shared_ptr<Address> addr = getAddress();
	if (!addr) {
		lError() << "Can't send a SUBSCRIBE for friend [" << toC() << "] without an address!";
		return;
	}

	if (mOutSub) {
		mOutSub->release();
		mOutSub = nullptr;
	} else {
		/* People for which we don't have yet an answer should appear as offline */
		clearPresenceModels();
	}

	LinphoneCore *lc = getCore()->getCCore();
	mOutSub = new SalPresenceOp(lc->sal.get());
	linphone_configure_op(lc, mOutSub, addr->toC(), nullptr, true);
	mOutSub->subscribe(linphone_config_get_int(lc->config, "sip", "subscribe_expires", 600));
	mSubscribeActive = true;
}

bool Friend::hasPhoneNumber(const std::shared_ptr<Account> &account, const std::string &searchedPhoneNumber) const {
	if (searchedPhoneNumber.empty()) return false;

	bool found = false;
	std::list<std::string> phoneNumbers = getPhoneNumbers();
	for (auto phoneNumber : phoneNumbers) {
		char *normalizedPhoneNumber =
		    linphone_account_normalize_phone_number(account->toC(), L_STRING_TO_C(phoneNumber));
		if (normalizedPhoneNumber) {
			if (strcmp(normalizedPhoneNumber, L_STRING_TO_C(searchedPhoneNumber)) == 0) {
				found = true;
				bctbx_free(normalizedPhoneNumber);
				break;
			}
		}
		bctbx_free(normalizedPhoneNumber);
	}

	return found;
}

void Friend::invalidateSubscription() {
	if (mOutSub) {
		mOutSub->release();
		mOutSub = nullptr;
	}

	// To resend a subscribe on the next network_reachable(TRUE)
	mSubscribeActive = false;

	/* Notify application that we no longer know the presence activity */
	for (auto &elem : mPresenceModels) {
		elem.second = PresenceModel::create();
		elem.second->setBasicStatus(LinphonePresenceBasicStatusClosed);
		std::string uri = elem.first->asStringUriOnly();
		linphone_core_notify_notify_presence_received_for_uri_or_tel(getCore()->getCCore(), toC(), L_STRING_TO_C(uri),
		                                                             elem.second->toC());
	}
	if (mPresenceModels.size() > 0) {
		// Deprecated
		linphone_core_notify_notify_presence_received(getCore()->getCCore(), toC());
	}
	mInitialSubscribesSent = false;
}

void Friend::notify(const std::shared_ptr<PresenceModel> &model) {
	if (!mInSubs.empty()) {
		const std::shared_ptr<Address> addr = getAddress();
		if (addr) {
			std::string addrStr = addr->asString();
			lInfo() << "Want to notify " << addrStr;
		}
	}

	for (auto op : mInSubs) {
		auto presenceOp = static_cast<SalPresenceOp *>(op);
		presenceOp->notifyPresence((SalPresenceModel *)model->toC());
	}
}

const std::string &Friend::phoneNumberToSipUri(const std::string &phoneNumber) const {
	auto it = mPhoneNumberToSipUriMap.find(phoneNumber);
	if (it != mPhoneNumberToSipUriMap.cend()) {
		// Force sip uri computation because proxy config may have changed, specially, ccc could have been added
		// since last computation
		std::string uri = it->second;
		mPhoneNumberToSipUriMap.erase(it);
		mSipUriToPhoneNumberMap.erase(uri);
	}

	LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(getCore()->getCCore());
	if (!cfg) return emptyString;
	std::string cleanedPhoneNumber(phoneNumber);
	if (cleanedPhoneNumber.find("tel:") == 0) cleanedPhoneNumber.replace(0, 4, "");
	char *normalizedNumber = linphone_proxy_config_normalize_phone_number(cfg, cleanedPhoneNumber.c_str());
	if (!normalizedNumber) return emptyString;
	std::stringstream ss;
	ss << "sip:" << normalizedNumber << "@" << linphone_proxy_config_get_domain(cfg) << ";user=phone";
	bctbx_free(normalizedNumber);
	std::string uri = ss.str();
	const auto pair = mPhoneNumberToSipUriMap.insert({phoneNumber, uri});
	mSipUriToPhoneNumberMap.insert({uri, phoneNumber});
	return pair.first->second;
}

void Friend::presenceReceived(const std::shared_ptr<FriendList> list,
                              const std::string &uri,
                              const std::shared_ptr<PresenceModel> &model) {
	mPresenceReceived = true;
	const std::string phoneNumber = sipUriToPhoneNumber(uri);
	if (phoneNumber.empty()) {
		setPresenceModelForUriOrTel(uri, model);
		linphone_core_notify_notify_presence_received_for_uri_or_tel(getCore()->getCCore(), toC(), uri.c_str(),
		                                                             model->toC());
	} else {
		std::string presenceUri = model->getContact();
		bool foundFriendWithPhone = false;
		for (auto [it, rangeEnd] = list->mFriendsMapByUri.equal_range(presenceUri); it != rangeEnd; it++) {
			if (it->second == getSharedFromThis()) foundFriendWithPhone = true;
		}
		if (!foundFriendWithPhone) {
			list->mFriendsMapByUri.insert({presenceUri, getSharedFromThis()});
		}
		setPresenceModelForUriOrTel(phoneNumber, model);
		linphone_core_notify_notify_presence_received_for_uri_or_tel(getCore()->getCCore(), toC(), phoneNumber.c_str(),
		                                                             model->toC());
	}

	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Friend, this, linphone_friend_cbs_get_presence_received);
	linphone_core_notify_notify_presence_received(getCore()->getCCore(), toC()); // Deprecated
}

void Friend::releaseOps() {
	for (auto op : mInSubs) {
		op->release();
	}
	mInSubs.clear();
	if (mOutSub) {
		mOutSub->release();
		mOutSub = nullptr;
	}
}

void Friend::removeFromDb() {
#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->mainDb;
	if (mainDb) mainDb->deleteFriend(getSharedFromThis());
#endif
	mStorageId = -1;
}

void Friend::removeFriendFromListMapIfAlreadyInIt(const std::string &uri) {
	if (!mFriendList || uri.empty()) return;
	for (auto [it, rangeEnd] = mFriendList->mFriendsMapByUri.equal_range(uri); it != rangeEnd;) {
		if (it->second == getSharedFromThis()) {
			it = mFriendList->mFriendsMapByUri.erase(it);
		} else it++;
	}
}

void Friend::removeIncomingSubscription(SalOp *op) {
	auto it = std::find(mInSubs.cbegin(), mInSubs.cend(), op);
	if (it != mInSubs.cend()) {
		op->release();
		mInSubs.erase(it);
	}
}

void Friend::saveInDb() {
	if (!mFriendList->databaseStorageEnabled()) return;
	// The friend list store logic is hidden into the friend store logic
	if (mFriendList->mStorageId < 0) {
		lWarning() << "Trying to add a friend in db, but friend list isn't, let's do that first";
		mFriendList->saveInDb();
	}
#ifdef HAVE_DB_STORAGE
	try {
		std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->mainDb;
		if (mainDb) mStorageId = mainDb->insertFriend(getSharedFromThis());
	} catch (std::bad_weak_ptr &) {
	}
#endif
}

const std::string &Friend::sipUriToPhoneNumber(const std::string &uri) const {
	try {
		return mSipUriToPhoneNumberMap.at(uri);
	} catch (std::out_of_range &) {
		return emptyString;
	}
}

void Friend::syncBctbxAddresses() const {
	if (mBctbxAddresses) {
		bctbx_list_free(mBctbxAddresses), mBctbxAddresses = nullptr;
	}
	for (const auto &addr : mAddresses) {
		mBctbxAddresses = bctbx_list_append(mBctbxAddresses, addr->toC());
	}
}

void Friend::unsubscribe() {
	if (mOutSub) mOutSub->unsubscribe();
	/* For friend list there is no necessary outsub */
	mSubscribeActive = false;
}

/**
 * Updates the p2p subscriptions.
 * If onlyWhenRegistered is true, subscribe will be sent only if the friend's corresponding proxy config is in
 * registered. Otherwise if the proxy config goes to unregistered state, the subscription refresh will be suspended. An
 * optional proxy whose state has changed can be passed to optimize the processing.
 **/
void Friend::updateSubscribes(bool onlyWhenRegistered) {
	bool canSubscribe = true;

	if (onlyWhenRegistered && (mSubscribe || mSubscribeActive)) {
		const std::shared_ptr<Address> addr = getAddress();
		if (addr) {
			LinphoneProxyConfig *cfg = linphone_core_lookup_known_proxy(getCore()->getCCore(), addr->toC());
			if (cfg && linphone_proxy_config_get_state(cfg) != LinphoneRegistrationOk) {
				lDebug() << "Friend [" << addr->asString() << "] belongs to account with contact address ["
				         << Address::toCpp(linphone_proxy_config_get_identity_address(cfg))->asString()
				         << "], but this one isn't registered. Subscription is suspended.";
				canSubscribe = false;
			}
		}
	}
	if (canSubscribe && mSubscribe && !mSubscribeActive) {
		lInfo() << "Sending a new SUBSCRIBE for friend [" << toC() << "]";
		doSubscribe();
	} else if (canSubscribe && mSubscribeActive && !mSubscribe) {
		unsubscribe();
	} else if (!canSubscribe && mOutSub) {
		mSubscribeActive = false;
		mOutSub->stopRefreshing();
	}
}

std::string Friend::capabilityToName(const LinphoneFriendCapability capability) {
	const auto found = std::find_if(begin(friendCapabilityNameMap), end(friendCapabilityNameMap),
	                                [&](const auto &elem) { return elem.capability == capability; });
	if (found == end(friendCapabilityNameMap)) return "none";
	return std::string(found->name);
}

LinphoneFriendCapability Friend::nameToCapability(const std::string &name) {
	const auto found = std::find_if(begin(friendCapabilityNameMap), end(friendCapabilityNameMap),
	                                [&](const auto &elem) { return elem.name == name; });
	if (found == end(friendCapabilityNameMap)) return LinphoneFriendCapabilityNone;
	return found->capability;
}

// -----------------------------------------------------------------------------

LinphoneFriendCbsPresenceReceivedCb FriendCbs::getPresenceReceived() const {
	return mPresenceReceivedCb;
}

void FriendCbs::setPresenceReceived(LinphoneFriendCbsPresenceReceivedCb cb) {
	mPresenceReceivedCb = cb;
}

LINPHONE_END_NAMESPACE
