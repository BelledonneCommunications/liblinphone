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

#include <fstream>
#include <set>

#include "bctoolbox/list.h"
#include <bctoolbox/defs.h>

#include "friend-list.h"

#include "c-wrapper/internal/c-tools.h"
#include "content/content.h"
#include "core/core-p.h"
#include "core/core.h"
#include "db/main-db.h"
#include "event/event.h"
#include "friend.h"
#include "http/http-client.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-content.h"
#include "linphone/types.h"
#include "presence/presence-model.h"
#include "private.h" // TODO: To remove if possible
#include "private_functions.h"
#include "vcard/carddav-context.h"
#include "vcard/vcard-context.h"
#include "vcard/vcard.h"
#ifdef HAVE_XML2
#include "xml/xml-parsing-context.h"
#endif // HAVE_XML2

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

FriendList::FriendList(std::shared_ptr<Core> core) : CoreAccessor(core) {
	if (core) { // Will be nullptr if created from database
		mSubscriptionsEnabled = linphone_core_is_friend_list_subscription_enabled(core->getCCore());
	}
}

FriendList::~FriendList() {
	if (mEvent) mEvent->terminate();
	if (mContentDigest) delete mContentDigest;
	if (mBctbxDirtyFriendsToUpdate) bctbx_list_free(mBctbxDirtyFriendsToUpdate);
	release();
}

FriendList *FriendList::clone() const {
	return nullptr;
}

void FriendList::release() {
#if VCARD_ENABLED
	if (mCardDavContext) {
		mCardDavContext = nullptr;
	}
#endif
	for (auto &f : mFriendsList.mList) {
		f->releaseOps();
	}
}

// -----------------------------------------------------------------------------

void FriendList::setDisplayName(const std::string &displayName) {
	mDisplayName = displayName;
	if (!mDisplayName.empty()) saveInDb();
}

void FriendList::setRlsAddress(const std::shared_ptr<const Address> &rlsAddr) {
	mRlsAddr = rlsAddr ? rlsAddr->clone()->toSharedPtr() : nullptr;
	if (mRlsAddr) {
		mRlsUri = mRlsAddr->asString();
	} else {
		mRlsUri = "";
	}
	saveInDb();
}

void FriendList::setRlsUri(const std::string &rlsUri) {
	std::shared_ptr<Address> addr = rlsUri.empty() ? nullptr : Address::create(rlsUri);
	setRlsAddress(addr);
}

void FriendList::setSubscriptionBodyless(bool bodyless) {
	mBodylessSubscription = bodyless;
}

void FriendList::setType(LinphoneFriendListType type) {
	mType = type;
	saveInDb();
}

void FriendList::setUri(const std::string &uri) {
	mUri = uri;
	saveInDb();
}

// -----------------------------------------------------------------------------

const std::string &FriendList::getDisplayName() const {
	return mDisplayName;
}

const std::list<std::shared_ptr<Friend>> &FriendList::getFriends() const {
	return mFriendsList.mList;
}

const bctbx_list_t *FriendList::getFriendsCList() const {
	return mFriendsList.getCList();
}

const std::shared_ptr<Address> &FriendList::getRlsAddress() const {
	return mRlsAddr;
}

const std::string &FriendList::getRlsUri() const {
	return mRlsUri;
}

const std::list<std::shared_ptr<Friend>> &FriendList::getDirtyFriendsToUpdate() const {
	return mDirtyFriendsToUpdate;
}

LinphoneFriendListType FriendList::getType() const {
	return mType;
}

const std::string &FriendList::getUri() const {
	return mUri;
}

bool FriendList::isSubscriptionBodyless() const {
	return mBodylessSubscription;
}

// -----------------------------------------------------------------------------

LinphoneFriendListStatus FriendList::addFriend(const std::shared_ptr<Friend> &lf) {
	return addFriend(lf, true);
}

LinphoneFriendListStatus FriendList::addLocalFriend(const std::shared_ptr<Friend> &lf) {
	return addFriend(lf, false);
}

bool FriendList::databaseStorageEnabled() const {
	if (isSubscriptionBodyless()) return false; // Do not store list if bodyless subscription is enabled
	int storeFriends =
	    linphone_config_get_int(getCore()->getCCore()->config, "misc", "store_friends", 1); // Legacy setting
	return storeFriends || mStoreInDb;
}

void FriendList::enableDatabaseStorage(bool enable) {
	if (enable && isSubscriptionBodyless()) {
		lWarning() << "Can't store in DB a friend list [" << mDisplayName << "] with bodyless subscription enabled";
		return;
	}

	if (mStoreInDb && !enable) {
		lWarning() << "We are asked to remove database storage for friend list [" << mDisplayName << "]";
		mStoreInDb = enable;
		removeFromDb();
	} else if (!mStoreInDb && enable) {
		mStoreInDb = enable;
		saveInDb();
		for (const auto &f : mFriendsList.mList) {
			lWarning() << "Found existing friend [" << f->getName() << "] in list [" << mDisplayName
			           << "] that was added before the list was configured to be saved in DB, doing it now";
			f->saveInDb();
		}
	}
}

void FriendList::enableSubscriptions(bool enabled) {
	if (mSubscriptionsEnabled != enabled) {
		mSubscriptionsEnabled = enabled;
		if (enabled) {
			lInfo() << "Updating friend list [" << toC() << "] subscriptions";
			updateSubscriptions();
		} else {
			lInfo() << "Closing friend list [" << toC() << "] subscriptions";
			closeSubscriptions();
		}
	}
}

void FriendList::exportFriendsAsVcard4File(const std::string &vcardFile) const {
	if (!linphone_core_vcard_supported()) {
		lError() << "vCard support wasn't enabled at compilation time";
		return;
	}
	std::ofstream ostrm(vcardFile, std::ios::binary | std::ios::trunc);
	if (!ostrm.is_open()) {
		lWarning() << "Could not write " << vcardFile << "! Maybe it is read-only. Contacts will not be saved.";
		return;
	}
	const std::list<std::shared_ptr<Friend>> friends = getFriends();
	for (const auto &f : friends) {
		std::shared_ptr<Vcard> vcard = f->getVcard();
		if (vcard) {
			ostrm << vcard->asVcard4StringWithBase64Picture();
		}
	}
	ostrm.close();
}

std::shared_ptr<Friend> FriendList::findFriendByAddress(const std::shared_ptr<const Address> &address) const {
	string cleanUri;
	if (address->hasUriParam("gr")) {
		std::shared_ptr<Address> cleanAddress = address->clone()->toSharedPtr();
		cleanAddress->removeUriParam("gr");
		cleanUri = cleanAddress->asStringUriOnly();
	} else {
		cleanUri = address->asStringUriOnly();
	}
	std::shared_ptr<Friend> lf = findFriendByUri(cleanUri);
	return lf;
}

std::shared_ptr<Friend> FriendList::findFriendByPhoneNumber(const std::string &phoneNumber) const {
	if (phoneNumber.empty()) {
		lWarning() << "Phone number [" << phoneNumber << "] isn't valid";
		return nullptr;
	}
	if (!linphone_core_vcard_supported()) {
		lWarning() << "SDK built without vCard support, can't do a phone number search without it";
		return nullptr;
	}

	const auto &accounts = getCore()->getAccounts();
	for (const auto &accountInList : accounts) {
		char *normalizedPhoneNumber =
		    linphone_account_normalize_phone_number(accountInList->toC(), L_STRING_TO_C(phoneNumber));
		if (normalizedPhoneNumber && strlen(normalizedPhoneNumber) > 0) {
			std::shared_ptr<Friend> result = findFriendByPhoneNumber(accountInList, normalizedPhoneNumber);
			bctbx_free(normalizedPhoneNumber);
			if (result) return result;
		}
	}
	return nullptr;
}

std::shared_ptr<Friend> FriendList::findFriendByRefKey(const std::string &refKey) const {
	try {
		return mFriendsMapByRefKey.at(refKey);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

std::shared_ptr<Friend> FriendList::findFriendByUri(const std::string &uri) const {
	const auto it = mFriendsMapByUri.find(uri);
	return (it == mFriendsMapByUri.cend()) ? nullptr : it->second;
}

std::list<std::shared_ptr<Friend>>
FriendList::findFriendsByAddress(const std::shared_ptr<const Address> &address) const {
	if (address->hasUriParam("gr")) {
		std::shared_ptr<Address> cleanAddress = address->clone()->toSharedPtr();
		cleanAddress->removeUriParam("gr");
		std::list<std::shared_ptr<Friend>> result = findFriendsByUri(cleanAddress->asStringUriOnly());
		return result;
	}

	std::list<std::shared_ptr<Friend>> result = findFriendsByUri(address->asStringUriOnly());
	return result;
}

std::list<std::shared_ptr<Friend>> FriendList::findFriendsByUri(const std::string &uri) const {
	std::list<std::shared_ptr<Friend>> result;
	for (auto [it, rangeEnd] = mFriendsMapByUri.equal_range(uri); it != rangeEnd; it++) {
		result.push_back(it->second);
	}
	return result;
}

LinphoneStatus FriendList::importFriendsFromVcard4Buffer(const std::string &vcardBuffer) {
	std::list<std::shared_ptr<Vcard>> vcards =
	    VcardContext::toCpp(getCore()->getCCore()->vcard_context)->getVcardListFromBuffer(vcardBuffer);
	if (vcards.empty()) {
		lError() << "Failed to parse the buffer";
		return -1;
	}
	return importFriendsFromVcard4(vcards);
}

LinphoneStatus FriendList::importFriendsFromVcard4File(const std::string &vcardFile) {
	std::list<std::shared_ptr<Vcard>> vcards =
	    VcardContext::toCpp(getCore()->getCCore()->vcard_context)->getVcardListFromFile(vcardFile);
	if (vcards.empty()) {
		lError() << "Failed to parse the file " << vcardFile;
		return -1;
	}
	return importFriendsFromVcard4(vcards);
}

void FriendList::notifyPresence(const std::shared_ptr<PresenceModel> &model) const {
	for (const auto &f : mFriendsList.mList)
		f->notify(model);
}

LinphoneFriendListStatus FriendList::removeFriend(const std::shared_ptr<Friend> &lf) {
	return removeFriend(lf, true);
}

void FriendList::removeFriends() {
	removeFriends(true);
}

bool FriendList::subscriptionsEnabled() const {
	return mSubscriptionsEnabled;
}

#ifdef VCARD_ENABLED

void FriendList::createCardDavContextIfNotDoneYet() {
	if (mCardDavContext == nullptr) {
		mCardDavContext = make_shared<CardDAVContext>(getCore());
		mCardDavContext->setFriendList(getSharedFromThis());
	}
}

void FriendList::synchronizeFriendsFromServer() {
	LinphoneCore *lc = getCore()->getCCore();

	// Vcard4.0 list synchronisation
	if (mType == LinphoneFriendListTypeVCard4) {
		if (mUri.empty()) {
			lError() << "Can't synchronize vCard4 list [" << toC() << "](" << getDisplayName() << ") without an URI";
			return;
		}

		ms_message("Starting downloading vCard 4 list using URI [%s], any existing friends will be removed first",
		           mUri.c_str());
		// Clear any previously existing friends
		removeFriends(false);
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_sync_status_changed,
		                                  LinphoneFriendListSyncStarted, nullptr);

		belle_http_request_listener_callbacks_t belle_request_listener = {0};
		belle_generic_uri_t *uri = belle_generic_uri_parse(mUri.c_str());
		belle_request_listener.process_auth_requested = [](void *ctx, belle_sip_auth_event_t *event) {
			LinphoneFriendList *list = (LinphoneFriendList *)ctx;
			linphone_core_fill_belle_sip_auth_event(FriendList::toCpp(list)->getCore()->getCCore(), event, NULL, NULL);
		};
		belle_request_listener.process_response = [](void *ctx, const belle_http_response_event_t *event) {
			LinphoneFriendList *list = (LinphoneFriendList *)ctx;
			FriendList *friendList = FriendList::toCpp(list);
			const char *body = belle_sip_message_get_body(BELLE_SIP_MESSAGE(event->response));
			if (body) {
				int importedFriends = friendList->importFriendsFromVcard4Buffer(body);
				if (importedFriends > 0) {
					ms_message("Successfully downloaded and imported [%i] friends", importedFriends);
					LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, friendList,
					                                  linphone_friend_list_cbs_get_sync_status_changed,
					                                  LinphoneFriendListSyncSuccessful, nullptr);
				} else {
					ms_error("Failed to import downloaded vCards!");
					LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, friendList,
					                                  linphone_friend_list_cbs_get_sync_status_changed,
					                                  LinphoneFriendListSyncFailure, nullptr);
				}
			} else {
				ms_warning("No body was received...");
				LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, friendList,
				                                  linphone_friend_list_cbs_get_sync_status_changed,
				                                  LinphoneFriendListSyncSuccessful, nullptr);
			}
		};
		belle_request_listener.process_io_error = [](void *ctx, BCTBX_UNUSED(const belle_sip_io_error_event_t *event)) {
			FriendList *list = FriendList::toCpp((LinphoneFriendList *)ctx);
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, list, linphone_friend_list_cbs_get_sync_status_changed,
			                                  LinphoneFriendListSyncFailure, "IO error");
		};
		belle_request_listener.process_timeout = [](void *ctx, BCTBX_UNUSED(const belle_sip_timeout_event_t *event)) {
			FriendList *list = FriendList::toCpp((LinphoneFriendList *)ctx);
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, list, linphone_friend_list_cbs_get_sync_status_changed,
			                                  LinphoneFriendListSyncFailure, "Timeout reached");
		};

		// We free-up the existing listeners if the previous request was not cancelled properly
		if (lc->base_contacts_list_http_listener) {
			belle_sip_object_unref(lc->base_contacts_list_http_listener);
			lc->base_contacts_list_http_listener = nullptr;
		}

		lc->base_contacts_list_http_listener =
		    belle_http_request_listener_create_from_callbacks(&belle_request_listener, toC());
		belle_http_request_t *request = belle_http_request_create(
		    "GET", uri, belle_sip_header_create("User-Agent", linphone_core_get_user_agent(lc)), nullptr);
		const auto &account = getCore()->getDefaultAccount();

		// https://datatracker.ietf.org/doc/id/draft-ietf-vcarddav-vcardrev-02.html#mime_type_registration
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_http_header_create("Accept", "text/vcard"));

		if (account) {
			std::string uri = account->getAccountParams()->getIdentityAddress()->asStringUriOnly();
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_http_header_create("From", uri.c_str()));
		}

		belle_http_provider_send_request(getCore()->getHttpClient().getProvider(), request,
		                                 lc->base_contacts_list_http_listener);
	} else if (mType == LinphoneFriendListTypeCardDAV) {
		if (mUri.empty()) {
			lError() << "Can't synchronize CardDAV list [" << toC() << "](" << getDisplayName() << ") without an URI";
			return;
		}

		// CardDav synchronisation
		createCardDavContextIfNotDoneYet();
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_sync_status_changed,
		                                  LinphoneFriendListSyncStarted, nullptr);
		mCardDavContext->synchronize();
	} else {
		lError() << "Failed to create a CardDAV context for friend list [" << toC() << "] with URI [" << mUri << "]";
	}
}

void FriendList::updateDirtyFriends() {
	createCardDavContextIfNotDoneYet();
	for (const auto &lf : mDirtyFriendsToUpdate) {
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_sync_status_changed,
		                                  LinphoneFriendListSyncStarted, nullptr);
		mCardDavContext->putVcard(lf);
	}
	mDirtyFriendsToUpdate.clear();
	mBctbxDirtyFriendsToUpdate = bctbx_list_free(mBctbxDirtyFriendsToUpdate);
}

#else

void FriendList::synchronizeFriendsFromServer() {
	lWarning() << "FriendList::synchronizeFriendsFromServer(): stubbed.";
}

void FriendList::updateDirtyFriends() {
	lWarning() << "FriendList::updateDirtyFriends(): stubbed.";
}

#endif /* VCARD_ENABLED */

void FriendList::updateRevision(const string &revision) {
	mRevision = revision;
	saveInDb();
}

// -----------------------------------------------------------------------------

LinphoneFriendListStatus FriendList::addFriend(const std::shared_ptr<Friend> &lf, bool synchronize) {
	if (lf->mFriendList) {
		lError() << "FriendList::addFriend(): invalid friend, already in list";
		return LinphoneFriendListInvalidFriend;
	}

	const std::shared_ptr<Address> addr = lf->getAddress();
	std::list<std::string> phoneNumbers = lf->getPhoneNumbers();
	if (!addr && lf->getAddresses().empty() && !lf->getVcard() && phoneNumbers.empty()) {
		lError() << "FriendList::addFriend(): invalid friend, no vCard, SIP URI or phone number";
		return LinphoneFriendListInvalidFriend;
	}

	LinphoneFriendListStatus status = LinphoneFriendListInvalidFriend;
	bool present = false;
	const std::string refKey = lf->getRefKey();
	if (refKey.empty()) {
		const auto it = std::find_if(mFriendsList.mList.cbegin(), mFriendsList.mList.cend(),
		                             [&](const auto &f) { return f == lf; });
		present = it != mFriendsList.mList.cend();
	} else {
		present = (findFriendByRefKey(refKey) != nullptr);
	}
	if (present) {
		std::string tmp = lf->getName();
		lWarning() << "Friend " << (tmp.empty() ? "unknown" : tmp.c_str()) << " already in list [" << mDisplayName
		           << "], ignored.";
	} else {
		status = importFriend(lf, synchronize);
		lf->saveInDb();
	}

	if (mRlsUri.empty()) // Mimic the behaviour of linphone_core_add_friend() when a resource list server is not in use
		lf->apply();

	return status;
}

void FriendList::closeSubscriptions() {
	/* FIXME we should wait until subscription is complete. */
	if (mEvent) {
		mEvent->terminate();
		mEvent = nullptr;
	}
	for (const auto &f : mFriendsList.mList)
		f->closeSubscriptions();
}

#ifdef HAVE_XML2

std::string FriendList::createResourceListXml() const {
	std::string xmlContent;
	if (mFriendsMapByUri.empty()) {
		lWarning() << __FUNCTION__ << ": Empty list in subscription, ignored.";
		return std::string();
	}
	xmlBufferPtr buf = xmlBufferCreate();
	if (!buf) {
		lError() << __FUNCTION__ << ": Error creating the XML buffer";
		return std::string();
	}
	xmlTextWriterPtr writer = xmlNewTextWriterMemory(buf, 0);
	if (!writer) {
		lError() << __FUNCTION__ << ": Error creating the XML writer";
		xmlBufferFree(buf);
		return std::string();
	}
	xmlTextWriterSetIndent(writer, 1);
	int err = xmlTextWriterStartDocument(writer, "1.0", "UTF-8", nullptr);
	if (err >= 0)
		err = xmlTextWriterStartElementNS(writer, nullptr, (const xmlChar *)"resource-lists",
		                                  (const xmlChar *)"urn:ietf:params:xml:ns:resource-lists");
	if (err >= 0)
		err = xmlTextWriterWriteAttributeNS(writer, (const xmlChar *)"xmlns", (const xmlChar *)"xsi", nullptr,
		                                    (const xmlChar *)"http://www.w3.org/2001/XMLSchema-instance");
	if (err >= 0) {
		err = xmlTextWriterStartElement(writer, (const xmlChar *)"list");
	}

	std::string previousEntry;
	for (const auto &entry : mFriendsMapByUri) {
		// Map is sorted, prevent duplicates
		if (previousEntry.empty() || (previousEntry != entry.first)) {
			if (err >= 0) err = xmlTextWriterStartElement(writer, (const xmlChar *)"entry");
			if (err >= 0)
				err = xmlTextWriterWriteAttribute(writer, (const xmlChar *)"uri", (const xmlChar *)entry.first.c_str());
			if (err >= 0) err = xmlTextWriterEndElement(writer); // Close the "entry" element.
		}
		previousEntry = entry.first;
	}

	if (err >= 0) err = xmlTextWriterEndElement(writer); // Close the "list" element.
	if (err >= 0) err = xmlTextWriterEndElement(writer); // Close the "resource-lists" element.
	if (err >= 0) err = xmlTextWriterEndDocument(writer);
	if (err > 0) {
		// xmlTextWriterEndDocument returns the size of the content.
		xmlContent = (char *)buf->content;
	}
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
	return xmlContent;
}

#else

std::string FriendList::createResourceListXml() const {
	lWarning() << "FriendList::createResourceListXml() is stubbed.";
	return std::string();
}

#endif

std::shared_ptr<Friend> FriendList::findFriendByIncSubscribe(SalOp *op) const {
	const auto it = std::find_if(mFriendsList.mList.cbegin(), mFriendsList.mList.cend(), [&](const auto &f) {
		const auto subIt =
		    std::find_if(f->mInSubs.cbegin(), f->mInSubs.cend(), [&](const SalOp *inSubOp) { return inSubOp == op; });
		return (subIt != f->mInSubs.cend());
	});
	return (it == mFriendsList.mList.cend()) ? nullptr : *it;
}

std::shared_ptr<Friend> FriendList::findFriendByOutSubscribe(SalOp *op) const {
	const auto it = std::find_if(mFriendsList.mList.cbegin(), mFriendsList.mList.cend(), [&](const auto &f) {
		return (f->mOutSub && ((f->mOutSub == op) || f->mOutSub->isForkedOf(op)));
	});
	return (it == mFriendsList.mList.cend()) ? nullptr : *it;
}

std::shared_ptr<Friend> FriendList::findFriendByPhoneNumber(const std::shared_ptr<Account> &account,
                                                            const std::string &normalizedPhoneNumber) const {
	const auto it = std::find_if(mFriendsList.mList.cbegin(), mFriendsList.mList.cend(),
	                             [&](const auto &f) { return f->hasPhoneNumber(account, normalizedPhoneNumber); });
	return (it == mFriendsList.mList.cend()) ? nullptr : *it;
}

std::shared_ptr<Address> FriendList::getRlsAddressWithCoreFallback() const {
	std::shared_ptr<Address> addr = getRlsAddress();
	if (addr) return addr;
	LinphoneCore *lc = getCore()->getCCore();
	const char *rlsUri = linphone_config_get_string(lc->config, "sip", "rls_uri", nullptr);
	if (lc->default_rls_addr) linphone_address_unref(lc->default_rls_addr);
	lc->default_rls_addr = nullptr;
	if (rlsUri) {
		// To make sure changes in config are used if any
		lc->default_rls_addr = linphone_address_new(rlsUri);
	}
	return lc->default_rls_addr ? Address::getSharedFromThis(lc->default_rls_addr) : nullptr;
}

bool FriendList::hasSubscribeInactive() const {
	if (mBodylessSubscription) return true;
	for (const auto &lf : mFriendsList.mList) {
		if (!lf->mSubscribeActive) return true;
	}
	return false;
}

LinphoneFriendListStatus FriendList::importFriend(const std::shared_ptr<Friend> &lf, bool synchronize) {
	if (lf->mFriendList) {
		lError() << "FriendList::importFriend(): invalid friend, already in list";
		return LinphoneFriendListInvalidFriend;
	}
	lf->mFriendList = this;
	mFriendsList.mList.push_front(lf);
	lf->addAddressesAndNumbersIntoMaps(getSharedFromThis());
	if (synchronize) {
		mDirtyFriendsToUpdate.push_front(lf);
		mBctbxDirtyFriendsToUpdate = bctbx_list_prepend(mBctbxDirtyFriendsToUpdate, lf->toC());
	}
	return LinphoneFriendListOK;
}

LinphoneStatus FriendList::importFriendsFromVcard4(const std::list<std::shared_ptr<Vcard>> &vcards) {
	if (!linphone_core_vcard_supported()) {
		lError() << "vCard support wasn't enabled at compilation time";
		return -1;
	}
	int count = 0;
	for (const auto &vcard : vcards) {
		if (vcard->getUid().empty()) vcard->generateUniqueId();
		std::shared_ptr<Friend> f = Friend::create(getCore(), vcard);
		if (importFriend(f, true) == LinphoneFriendListOK) {
			f->saveInDb(), count++;
		}
	}
	saveInDb();
	return count;
}

void FriendList::invalidateFriendsMaps() {
	mFriendsMapByRefKey.clear();
	mFriendsMapByUri.clear();
	for (const auto &f : mFriendsList.mList)
		f->addAddressesAndNumbersIntoMaps(getSharedFromThis());
}

void FriendList::invalidateSubscriptions() {
	lInfo() << "Invalidating friend list's [" << toC() << "] subscriptions";
	// Terminate subscription event
	if (mEvent) {
		mEvent->terminate();
		mEvent = nullptr;
	}
	for (const auto &f : mFriendsList.mList)
		f->invalidateSubscription();
}

void FriendList::notifyPresenceReceived(const std::shared_ptr<const Content> &content) {
	if (!content) return;
	const ContentType &contentType = content->getContentType();
	if (contentType != ContentType::MultipartRelated) {
		lWarning() << "multipart presence notified but it is not 'multipart/related', instead is '"
		           << contentType.getType() << "/" << contentType.getSubType() << "'";
		return;
	}
	LinphoneContent *firstPart = linphone_content_get_part(content->toC(), 0);
	if (!firstPart) {
		lWarning() << "'multipart/related' presence notified but it doesn't contain any part";
		return;
	}
	if (Content::toCpp(firstPart)->getContentType() != ContentType::Rlmi) {
		lWarning() << "multipart presence notified but first part is not 'application/rlmi+xml'";
		linphone_content_unref(firstPart);
		return;
	}
	parseMultipartRelatedBody(content, Content::toCpp(firstPart)->getBodyAsUtf8String());
	linphone_content_unref(firstPart);
}

#ifdef HAVE_XML2

class FriendListXmlException : public std::exception {
public:
	FriendListXmlException(const char *msg) : mMessage(msg) {
	}
	const char *what() const throw() override {
		return mMessage;
	}

private:
	const char *mMessage;
};

void FriendList::parseMultipartRelatedBody(const std::shared_ptr<const Content> &content,
                                           const std::string &firstPartBody) {
	try {
		XmlParsingContext xmlCtx(firstPartBody);
		if (!xmlCtx.isValid()) {
			stringstream ss;
			ss << "Wrongly formatted rlmi+xml body: " << xmlCtx.getError();
			throw FriendListXmlException(ss.str().c_str());
		}

		xmlXPathRegisterNs(xmlCtx.getXpathContext(), reinterpret_cast<const xmlChar *>("rlmi"),
		                   reinterpret_cast<const xmlChar *>("urn:ietf:params:xml:ns:rlmi"));
		std::string versionStr = xmlCtx.getAttributeTextContent("/rlmi:list", "version");
		if (versionStr.empty()) throw FriendListXmlException("rlmi+xml: No version attribute in list");
		int version = atoi(versionStr.c_str());
		if (version < mExpectedNotificationVersion) {
			// No longer an error as dialog may be silently restarting by the refresher
			lWarning() << "rlmi+xml: Received notification with version " << version << " expected was "
			           << mExpectedNotificationVersion << ", dialog may have been reseted";
		}
		std::string fullStateStr = xmlCtx.getAttributeTextContent("/rlmi:list", "fullState");
		if (fullStateStr.empty()) throw FriendListXmlException("rlmi+xml: No fullState attribute in list");
		bool fullState = false;
		std::string fullStateString(fullStateStr);
		if ((fullStateString == "true") || (fullStateString == "1")) {
			fullState = true;
			for (const auto &lf : mFriendsList.mList)
				lf->clearPresenceModels();
		}
		if ((mExpectedNotificationVersion == 0) && !fullState)
			throw FriendListXmlException("rlmi+xml: Notification with version 0 is not full state, this is not valid");
		mExpectedNotificationVersion = version + 1;

		xmlXPathObjectPtr nameObject = xmlCtx.getXpathObjectForNodeList("/rlmi:list/rlmi:resource/rlmi:name/..");
		if (nameObject && nameObject->nodesetval) {
			for (int i = 1; i <= nameObject->nodesetval->nodeNr; i++) {
				xmlCtx.setXpathContextNode(xmlXPathNodeSetItem(nameObject->nodesetval, i - 1));
				std::string name = xmlCtx.getTextContent("./rlmi:name");
				std::string uri = xmlCtx.getTextContent("./@uri");
				if (uri.empty()) continue;
				std::shared_ptr<Address> addr = Address::create(uri);
				if (!addr) continue;
				std::shared_ptr<Friend> lf = findFriendByAddress(addr);
				if (!lf && mBodylessSubscription) {
					lf = Friend::create(getCore(), uri);
					addFriend(lf);
				}
				if (!name.empty()) lf->setName(name);
			}
		}
		if (nameObject) xmlXPathFreeObject(nameObject);

		std::set<std::shared_ptr<Friend>> listFriendsPresenceReceived;
		bctbx_list_t *parts = linphone_content_get_parts(content->toC());
		xmlXPathObjectPtr resourceObject =
		    xmlCtx.getXpathObjectForNodeList("/rlmi:list/rlmi:resource/rlmi:instance[@state=\"active\"]/..");
		if (resourceObject && resourceObject->nodesetval) {
			for (int i = 1; i <= resourceObject->nodesetval->nodeNr; i++) {
				xmlCtx.setXpathContextNode(xmlXPathNodeSetItem(resourceObject->nodesetval, i - 1));
				std::string cid = xmlCtx.getTextContent("./rlmi:instance/@cid");
				if (!cid.empty()) {
					std::shared_ptr<Content> presencePart = nullptr;
					bctbx_list_t *it = parts;
					while (it != nullptr) {
						LinphoneContent *content = (LinphoneContent *)it->data;
						const char *header = linphone_content_get_custom_header(content, "Content-Id");
						if (header && (std::string(header) == cid)) {
							presencePart = Content::toCpp(content)->getSharedFromThis();
							break;
						}
						it = bctbx_list_next(it);
					}
					if (!presencePart) {
						lWarning() << "rlmi+xml: Cannot find part with Content-Id: " << cid;
					} else {
						SalPresenceModel *presence = nullptr;
						const ContentType &presencePartContentType = presencePart->getContentType();
						PresenceModel::parsePresence(presencePartContentType.getType(),
						                             presencePartContentType.getSubType(),
						                             presencePart->getBodyAsUtf8String(), &presence);
						if (presence) {
							// Try to reduce CPU cost of linphone_address_new and find_friend_by_address by only doing
							// it when we know for sure we have a presence to notify
							std::string uri = xmlCtx.getTextContent("./@uri");
							if (uri.empty()) continue;
							std::shared_ptr<Address> addr = Address::create(uri);
							if (!addr) continue;

							// Clean the URI
							if (addr->hasUriParam("gr")) addr->removeUriParam("gr");
							uri = addr->asStringUriOnly();

							const auto [first, last] = mFriendsMapByUri.equal_range(uri);
							if (first == last) {
								if (mBodylessSubscription) {
									std::shared_ptr<Friend> lf = Friend::create(getCore(), uri);
									addFriend(lf);
									lf->presenceReceived(
									    getSharedFromThis(), uri,
									    PresenceModel::toCpp((LinphonePresenceModel *)presence)->getSharedFromThis());
									listFriendsPresenceReceived.insert(lf);
								}
							} else {
								// Save the equal_range iterators for looping because mFriendsMapByUri might
								// change during the loop, leading to wrong presence notifications
								std::list<std::multimap<std::string, std::shared_ptr<Friend>>::iterator> its;
								for (auto it = first; it != last; it++)
									its.push_back(it);
								for (const auto &it : its) {
									it->second->presenceReceived(
									    getSharedFromThis(), uri,
									    PresenceModel::toCpp((LinphonePresenceModel *)presence)->getSharedFromThis());
									listFriendsPresenceReceived.insert(it->second);
								}
							}

							PresenceModel::toCpp((LinphonePresenceModel *)presence)->unref();
						}
					}
				}
			}

			// Notify list with all friends for which we received presence information
			if (!listFriendsPresenceReceived.empty()) {
				bctbx_list_t *l = nullptr;
				for (const auto &lf : listFriendsPresenceReceived)
					l = bctbx_list_append(l, lf->toC());
				LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_presence_received, l);
				bctbx_list_free(l);
			}
		}

		bctbx_list_free_with_data(parts, (void (*)(void *))linphone_content_unref);
		if (resourceObject) xmlXPathFreeObject(resourceObject);
	} catch (FriendListXmlException &e) {
		lWarning() << e.what();
	}
}

#else

void FriendList::parseMultipartRelatedBody(BCTBX_UNUSED(const std::shared_ptr<const Content> &content),
                                           BCTBX_UNUSED(const std::string &firstPartBody)) {
	lWarning() << "FriendList::parseMultipartRelatedBody() is stubbed.";
}

#endif /* HAVE_XML2 */

void FriendList::deleteFriend(const std::shared_ptr<Friend> &lf, bool removeFromServer) {
#ifdef VCARD_ENABLED
	if (lf && databaseStorageEnabled()) lf->removeFromDb();
	if (removeFromServer && getType() == LinphoneFriendListTypeCardDAV) {
		std::shared_ptr<Vcard> vcard = lf->getVcard();
		if (vcard && !vcard->getUid().empty()) {
			createCardDavContextIfNotDoneYet();
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_sync_status_changed,
			                                  LinphoneFriendListSyncStarted, nullptr);
			mCardDavContext->deleteVcard(lf);
		}
	}
#else
	lDebug() << "FriendList::removeFriend(" << lf->toC() << ", " << removeFromServer << ")";
#endif

	const std::string &refKey = lf->getRefKey();
	if (!refKey.empty()) {
		const auto mapIt = mFriendsMapByRefKey.find(refKey);
		if (mapIt != mFriendsMapByRefKey.cend()) mFriendsMapByRefKey.erase(mapIt);
	}

	std::list<std::string> phoneNumbers = lf->getPhoneNumbers();
	for (const auto &phoneNumber : phoneNumbers) {
		const std::string uri = lf->phoneNumberToSipUri(phoneNumber);
		if (!uri.empty()) {
			const auto mapIt = mFriendsMapByUri.find(uri);
			if (mapIt != mFriendsMapByUri.cend()) mFriendsMapByUri.erase(mapIt);
		}
	}

	std::list<std::shared_ptr<Address>> addresses = lf->getAddresses();
	for (const auto &address : addresses) {
		const std::string uri = address->asStringUriOnly();
		if (!uri.empty()) {
			const auto mapIt = mFriendsMapByUri.find(uri);
			if (mapIt != mFriendsMapByUri.cend()) mFriendsMapByUri.erase(mapIt);
		}
	}

	lf->mFriendList = nullptr;
}

void FriendList::removeFriends(bool removeFromServer) {
	for (auto &lf : mFriendsList.mList) {
		deleteFriend(lf, removeFromServer);
	}
	mFriendsList.mList.clear();
}

LinphoneFriendListStatus FriendList::removeFriend(const std::shared_ptr<Friend> &lf, bool removeFromServer) {
	const auto it =
	    std::find_if(mFriendsList.mList.cbegin(), mFriendsList.mList.cend(), [&](const auto &f) { return f == lf; });
	if (it == mFriendsList.mList.cend()) return LinphoneFriendListNonExistentFriend;

	deleteFriend(lf, removeFromServer);
	mFriendsList.mList.erase(it);
	return LinphoneFriendListOK;
}

void FriendList::removeFromDb() {
#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->mainDb;
	if (mainDb) mainDb->deleteFriendList(getSharedFromThis());
#endif
	mStorageId = -1;
}

void FriendList::saveInDb() {
#ifdef HAVE_DB_STORAGE
	try {
		if (getCore() && databaseStorageEnabled()) {
			std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->mainDb;
			if (mainDb) {
				mStorageId = mainDb->insertFriendList(getSharedFromThis());
			}
		} else {
			lWarning() << "Can't save friend list [" << getDisplayName()
			           << "] in DB, either Core is not available or database storage is disabled";
		}
	} catch (std::bad_weak_ptr &) {
	}
#endif
}

void FriendList::sendListSubscription() {
	std::shared_ptr<Address> address = getRlsAddressWithCoreFallback();
	if (!address) {
		lWarning() << "Friend list's [" << toC() << "] has no RLS address, can't send subscription";
		return;
	}
	if (!hasSubscribeInactive()) {
		lWarning() << "Friend list's [" << toC() << "] subscribe is inactive, can't send subscription";
		return;
	}

	if (mBodylessSubscription) sendListSubscriptionWithoutBody(address);
	else sendListSubscriptionWithBody(address);
}

void FriendList::sendListSubscriptionWithBody(const std::shared_ptr<Address> &address) {
	std::string xmlContent = createResourceListXml();
	if (xmlContent.empty()) return;

	std::array<unsigned char, 16> digest;
	bctbx_md5((unsigned char *)xmlContent.c_str(), xmlContent.length(), digest.data());
	if (mEvent && mContentDigest && (*mContentDigest == digest)) {
		// The content has not changed, only refresh the event.
		linphone_event_refresh_subscribe(mEvent->toC());
	} else {
		int expires = linphone_config_get_int(getCore()->getCCore()->config, "sip", "rls_presence_expires", 3600);
		mExpectedNotificationVersion = 0;
		if (mContentDigest) delete mContentDigest;
		mContentDigest = new std::array<unsigned char, 16>(digest);
		if (mEvent) mEvent->terminate();
		mEvent = (new EventSubscribe(getCore(), address, "presence", expires))->toSharedPtr();
		mEvent->setInternal(true);
		mEvent->addCustomHeader("Require", "recipient-list-subscribe");
		mEvent->addCustomHeader("Supported", "eventlist");
		mEvent->addCustomHeader("Accept", "multipart/related, application/pidf+xml, application/rlmi+xml");
		mEvent->addCustomHeader("Content-Disposition", "recipient-list");
		std::shared_ptr<Content> content = Content::create();
		ContentType contentType("application", "resource-lists+xml");
		content->setContentType(contentType);
		content->setBodyFromUtf8(xmlContent);
		if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
			content->setContentEncoding("deflate");
			mEvent->addCustomHeader("Accept-Encoding", "deflate");
		}
		for (auto &lf : mFriendsList.mList)
			lf->mSubscribeActive = true;
		mEvent->send(content);
		mEvent->setUserData(this);
	}
}

void FriendList::sendListSubscriptionWithoutBody(const std::shared_ptr<Address> &address) {
	int expires = linphone_config_get_int(getCore()->getCCore()->config, "sip", "rls_presence_expires", 3600);
	mExpectedNotificationVersion = 0;
	if (mContentDigest) bctbx_free(mContentDigest);

	if (mEvent) mEvent->terminate();
	mEvent = (new EventSubscribe(getCore(), address, "presence", expires))->toSharedPtr();
	mEvent->setInternal(true);
	mEvent->addCustomHeader("Supported", "eventlist");
	mEvent->addCustomHeader("Accept", "multipart/related, application/pidf+xml, application/rlmi+xml");
	if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate"))
		mEvent->addCustomHeader("Accept-Encoding", "deflate");
	for (auto &lf : mFriendsList.mList)
		lf->mSubscribeActive = true;
	linphone_event_send_subscribe(mEvent->toC(), nullptr);
	mEvent->setUserData(this);
}

void FriendList::setFriends(const std::list<std::shared_ptr<Friend>> &friends) {
	mFriendsList.mList = friends;
}

void FriendList::updateSubscriptions() {
	std::shared_ptr<Account> account = nullptr;
	bool onlyWhenRegistered = false;
	bool shouldSendListSubscribe = false;

	lInfo() << "Updating friend list [" << toC() << "](" << getDisplayName() << ") subscriptions";
	std::shared_ptr<Address> address = getRlsAddressWithCoreFallback();
	if (address) account = getCore()->lookupKnownAccount(address, true);
	onlyWhenRegistered = linphone_core_should_subscribe_friends_only_when_registered(getCore()->getCCore());
	// In case of onlyWhenRegistered, proxy config is mandatory to send subscribes. Otherwise, unexpected
	// subscribtion can be issued using default contact address even if no account is configured yet.
	shouldSendListSubscribe = (!onlyWhenRegistered || (account && (account->getState() == LinphoneRegistrationOk)));

	if (address) {
		if (mSubscriptionsEnabled) {
			if (shouldSendListSubscribe) {
				sendListSubscription();
			} else {
				if (mEvent) {
					mEvent->terminate();
					mEvent = nullptr;
					lInfo() << "Friend list [" << toC()
					        << "] subscription terminated because proxy config lost connection";
				} else {
					lInfo() << "Friend list [" << toC()
					        << "] subscription update skipped since dependant proxy config is not yet registered";
				}
			}
		} else {
			lInfo() << "Friend list [" << toC() << "] subscription update skipped since subscriptions not enabled yet";
		}
	} else if (mSubscriptionsEnabled) {
		lInfo() << "Updating friend list's [" << toC() << "] friends subscribes";
		for (auto &lf : mFriendsList.mList)
			lf->updateSubscribes(onlyWhenRegistered);
	}
}

// -----------------------------------------------------------------------------

void FriendList::subscriptionStateChanged(LinphoneCore *lc,
                                          const std::shared_ptr<Event> event,
                                          LinphoneSubscriptionState state) {
	FriendList *list = reinterpret_cast<FriendList *>(event->getUserData());
	if (!list) {
		lWarning() << "core [" << lc << "] Receiving unexpected state [" << linphone_subscription_state_to_string(state)
		           << "] for event [" << event->toC() << "], no associated friend list";
	} else {
		lInfo() << "Receiving new state [" << linphone_subscription_state_to_string(state) << "] for event ["
		        << event->toC() << "] for friend list [" << list << "]";
		if ((state == LinphoneSubscriptionOutgoingProgress) && (event->getReason() == LinphoneReasonNoMatch)) {
			lInfo() << "Reseting version count for friend list [" << list->toC() << "]";
			list->mExpectedNotificationVersion = 0;
		}
	}
}

#ifdef VCARD_ENABLED

void FriendList::carddavCreated(const std::shared_ptr<Friend> &f) {
	addLocalFriend(f); // Add as local because we do not want to synchronize it right now
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_contact_created, f->toC());
}

void FriendList::carddavDone(bool success, const std::string &msg) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_sync_status_changed,
	                                  success ? LinphoneFriendListSyncSuccessful : LinphoneFriendListSyncFailure,
	                                  msg.c_str());
}

void FriendList::carddavRemoved(const std::shared_ptr<Friend> &f) {
	removeFriend(f, false);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_contact_deleted, f->toC());
}

void FriendList::carddavUpdated(const std::shared_ptr<Friend> &newFriend, const std::shared_ptr<Friend> &oldFriend) {
	auto it = std::find_if(mFriendsList.mList.begin(), mFriendsList.mList.end(),
	                       [&](const auto &elem) { return elem == oldFriend; });
	if (it != mFriendsList.mList.end()) *it = newFriend;
	newFriend->saveInDb();
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(FriendList, this, linphone_friend_list_cbs_get_contact_updated, newFriend->toC(),
	                                  oldFriend->toC());
}

#endif /* VCARD_ENABLED */

// -----------------------------------------------------------------------------

LinphoneFriendListCbsContactCreatedCb FriendListCbs::getContactCreated() const {
	return mContactCreatedCb;
}

LinphoneFriendListCbsContactDeletedCb FriendListCbs::getContactDeleted() const {
	return mContactDeletedCb;
}

LinphoneFriendListCbsContactUpdatedCb FriendListCbs::getContactUpdated() const {
	return mContactUpdatedCb;
}

LinphoneFriendListCbsPresenceReceivedCb FriendListCbs::getPresenceReceived() const {
	return mPresenceReceivedCb;
}

LinphoneFriendListCbsNewSipAddressDiscoveredCb FriendListCbs::getNewlyDiscoveredSipAddress() const {
	return mNewSipAddressDiscoveredCb;
}

LinphoneFriendListCbsSyncStateChangedCb FriendListCbs::getSyncStatusChanged() const {
	return mSyncStatusChangedCb;
}

void FriendListCbs::setContactCreated(LinphoneFriendListCbsContactCreatedCb cb) {
	mContactCreatedCb = cb;
}

void FriendListCbs::setContactDeleted(LinphoneFriendListCbsContactDeletedCb cb) {
	mContactDeletedCb = cb;
}

void FriendListCbs::setContactUpdated(LinphoneFriendListCbsContactUpdatedCb cb) {
	mContactUpdatedCb = cb;
}

void FriendListCbs::setPresenceReceived(LinphoneFriendListCbsPresenceReceivedCb cb) {
	mPresenceReceivedCb = cb;
}

void FriendListCbs::setNewlyDiscoveredSipAddress(LinphoneFriendListCbsNewSipAddressDiscoveredCb cb) {
	mNewSipAddressDiscoveredCb = cb;
}

void FriendListCbs::setSyncStatusChanged(LinphoneFriendListCbsSyncStateChangedCb cb) {
	mSyncStatusChangedCb = cb;
}

LINPHONE_END_NAMESPACE
