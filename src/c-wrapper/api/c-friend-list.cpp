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

#include <bctoolbox/crypto.h>

#include "linphone/api/c-content.h"
#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"
#include "friend/friend-list.h"
#include "friend/friend.h"
#include "presence/presence-model.h"

// TODO: From coreapi. Remove me later.
#include "private.h"
#include "private_functions.h"
#include "tester_utils.h"

using namespace LinphonePrivate;

LinphoneFriendListCbs *linphone_friend_list_cbs_new(void) {
	return FriendListCbs::createCObject();
}

void linphone_friend_list_release(LinphoneFriendList *friend_list) {
	FriendList::toCpp(friend_list)->release();
	linphone_friend_list_unref(friend_list);
}

void linphone_friend_list_add_callbacks(LinphoneFriendList *friend_list, LinphoneFriendListCbs *cbs) {
	FriendList::toCpp(friend_list)->addCallbacks(FriendListCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_friend_list_remove_callbacks(LinphoneFriendList *friend_list, LinphoneFriendListCbs *cbs) {
	FriendList::toCpp(friend_list)->removeCallbacks(FriendListCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneFriendListCbs *linphone_friend_list_get_current_callbacks(const LinphoneFriendList *friend_list) {
	return FriendList::toCpp(friend_list)->getCurrentCallbacks()->toC();
}

void linphone_friend_list_set_current_callbacks(LinphoneFriendList *friend_list, LinphoneFriendListCbs *cbs) {
	FriendList::toCpp(friend_list)->setCurrentCallbacks(FriendListCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *linphone_friend_list_get_callbacks_list(const LinphoneFriendList *friend_list) {
	return FriendList::toCpp(friend_list)->getCCallbacksList();
}

LinphoneFriendListCbs *linphone_friend_list_cbs_ref(LinphoneFriendListCbs *cbs) {
	FriendListCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_friend_list_cbs_unref(LinphoneFriendListCbs *cbs) {
	FriendListCbs::toCpp(cbs)->unref();
}

void *linphone_friend_list_cbs_get_user_data(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getUserData();
}

void linphone_friend_list_cbs_set_user_data(LinphoneFriendListCbs *cbs, void *ud) {
	FriendListCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneFriendListCbsContactCreatedCb linphone_friend_list_cbs_get_contact_created(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getContactCreated();
}

void linphone_friend_list_cbs_set_contact_created(LinphoneFriendListCbs *cbs,
                                                  LinphoneFriendListCbsContactCreatedCb cb) {
	FriendListCbs::toCpp(cbs)->setContactCreated(cb);
}

LinphoneFriendListCbsContactDeletedCb linphone_friend_list_cbs_get_contact_deleted(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getContactDeleted();
}

void linphone_friend_list_cbs_set_contact_deleted(LinphoneFriendListCbs *cbs,
                                                  LinphoneFriendListCbsContactDeletedCb cb) {
	FriendListCbs::toCpp(cbs)->setContactDeleted(cb);
}

LinphoneFriendListCbsContactUpdatedCb linphone_friend_list_cbs_get_contact_updated(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getContactUpdated();
}

void linphone_friend_list_cbs_set_contact_updated(LinphoneFriendListCbs *cbs,
                                                  LinphoneFriendListCbsContactUpdatedCb cb) {
	FriendListCbs::toCpp(cbs)->setContactUpdated(cb);
}

LinphoneFriendListCbsSyncStateChangedCb
linphone_friend_list_cbs_get_sync_status_changed(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getSyncStatusChanged();
}

void linphone_friend_list_cbs_set_sync_status_changed(LinphoneFriendListCbs *cbs,
                                                      LinphoneFriendListCbsSyncStateChangedCb cb) {
	FriendListCbs::toCpp(cbs)->setSyncStatusChanged(cb);
}

LinphoneFriendListCbsPresenceReceivedCb
linphone_friend_list_cbs_get_presence_received(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getPresenceReceived();
}

void linphone_friend_list_cbs_set_presence_received(LinphoneFriendListCbs *cbs,
                                                    LinphoneFriendListCbsPresenceReceivedCb cb) {
	FriendListCbs::toCpp(cbs)->setPresenceReceived(cb);
}

LinphoneFriendListCbsNewSipAddressDiscoveredCb
linphone_friend_list_cbs_get_new_sip_address_discovered(const LinphoneFriendListCbs *cbs) {
	return FriendListCbs::toCpp(cbs)->getNewlyDiscoveredSipAddress();
}

void linphone_friend_list_cbs_set_new_sip_address_discovered(LinphoneFriendListCbs *cbs,
                                                             LinphoneFriendListCbsNewSipAddressDiscoveredCb cb) {
	FriendListCbs::toCpp(cbs)->setNewlyDiscoveredSipAddress(cb);
}

LinphoneFriendList *linphone_core_create_friend_list(LinphoneCore *lc) {
	return FriendList::createCObject(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr);
}

LinphoneFriendList *linphone_friend_list_ref(LinphoneFriendList *list) {
	FriendList::toCpp(list)->ref();
	return list;
}

void linphone_friend_list_unref(LinphoneFriendList *list) {
	FriendList::toCpp(list)->unref();
}

void *linphone_friend_list_get_user_data(const LinphoneFriendList *list) {
	return FriendList::toCpp(list)->getUserData();
}

void linphone_friend_list_set_user_data(LinphoneFriendList *list, void *ud) {
	FriendList::toCpp(list)->setUserData(ud);
}

void linphone_friend_list_set_type(LinphoneFriendList *list, LinphoneFriendListType type) {
	FriendList::toCpp(list)->setType(type);
}

LinphoneFriendListType linphone_friend_list_get_type(LinphoneFriendList *list) {
	return FriendList::toCpp(list)->getType();
}

const char *linphone_friend_list_get_display_name(const LinphoneFriendList *list) {
	return L_STRING_TO_C(FriendList::toCpp(list)->getDisplayName());
}

void linphone_friend_list_set_display_name(LinphoneFriendList *list, const char *display_name) {
	FriendList::toCpp(list)->setDisplayName(display_name);
}

LinphoneAddress *linphone_friend_list_get_rls_address(const LinphoneFriendList *list) {
	std::shared_ptr<Address> addr = FriendList::toCpp(list)->getRlsAddress();
	return addr ? addr->toC() : nullptr;
}

void linphone_friend_list_set_rls_address(LinphoneFriendList *list, const LinphoneAddress *rls_addr) {
	const std::shared_ptr<const Address> addr = rls_addr ? Address::getSharedFromThis(rls_addr) : nullptr;
	FriendList::toCpp(list)->setRlsAddress(addr);
}

const char *linphone_friend_list_get_rls_uri(const LinphoneFriendList *list) {
	return L_STRING_TO_C(FriendList::toCpp(list)->getRlsUri());
}

void linphone_friend_list_set_rls_uri(LinphoneFriendList *list, const char *rls_uri) {
	FriendList::toCpp(list)->setRlsUri(L_C_TO_STRING(rls_uri));
}

LinphoneFriendListStatus linphone_friend_list_add_friend(LinphoneFriendList *list, LinphoneFriend *lf) {
	return FriendList::toCpp(list)->addFriend(Friend::getSharedFromThis(lf));
}

LinphoneFriendListStatus linphone_friend_list_add_local_friend(LinphoneFriendList *list, LinphoneFriend *lf) {
	return FriendList::toCpp(list)->addLocalFriend(Friend::getSharedFromThis(lf));
}

void linphone_friend_list_invalidate_friends_maps(LinphoneFriendList *list) {
	FriendList::toCpp(list)->invalidateFriendsMaps();
}

LinphoneFriendListStatus linphone_friend_list_remove_friend(LinphoneFriendList *list, LinphoneFriend *lf) {
	if (!lf) return LinphoneFriendListNonExistentFriend;
	return FriendList::toCpp(list)->removeFriend(Friend::getSharedFromThis(lf));
}

const bctbx_list_t *linphone_friend_list_get_friends(const LinphoneFriendList *list) {
	return FriendList::toCpp(list)->getFriendsCList();
}

void linphone_friend_list_update_dirty_friends(LinphoneFriendList *list) {
	FriendList::toCpp(list)->updateDirtyFriends();
}

void linphone_friend_list_synchronize_friends_from_server(LinphoneFriendList *list) {
	if (!list) {
		ms_error("List is null, this is not expected!");
		return;
	}
	FriendList::toCpp(list)->synchronizeFriendsFromServer();
}

LinphoneFriend *linphone_friend_list_find_friend_by_address(const LinphoneFriendList *list,
                                                            const LinphoneAddress *address) {
	if (!address) return nullptr;
	std::shared_ptr<Friend> lf = FriendList::toCpp(list)->findFriendByAddress(Address::getSharedFromThis(address));
	return lf ? lf->toC() : nullptr;
}

LinphoneFriend *linphone_friend_list_find_friend_by_phone_number(const LinphoneFriendList *list,
                                                                 const char *phoneNumber) {
	std::shared_ptr<Friend> lf = FriendList::toCpp(list)->findFriendByPhoneNumber(L_C_TO_STRING(phoneNumber));
	return lf ? lf->toC() : nullptr;
}

bctbx_list_t *linphone_friend_list_find_friends_by_address(const LinphoneFriendList *list,
                                                           const LinphoneAddress *address) {
	std::list<std::shared_ptr<Friend>> cppResult =
	    FriendList::toCpp(list)->findFriendsByAddress(Address::getSharedFromThis(address));
	bctbx_list_t *result = nullptr;
	for (const auto &elem : cppResult) {
		result = bctbx_list_append(result, linphone_friend_ref(elem->toC()));
	}
	return result;
}

LinphoneFriend *linphone_friend_list_find_friend_by_uri(const LinphoneFriendList *list, const char *uri) {
	std::shared_ptr<Friend> lf = FriendList::toCpp(list)->findFriendByUri(L_C_TO_STRING(uri));
	return lf ? lf->toC() : nullptr;
}

bctbx_list_t *linphone_friend_list_find_friends_by_uri(const LinphoneFriendList *list, const char *uri) {
	std::list<std::shared_ptr<Friend>> cppResult = FriendList::toCpp(list)->findFriendsByUri(L_C_TO_STRING(uri));
	bctbx_list_t *result = nullptr;
	for (const auto &elem : cppResult) {
		result = bctbx_list_append(result, linphone_friend_ref(elem->toC()));
	}
	return result;
}

LinphoneFriend *linphone_friend_list_find_friend_by_ref_key(const LinphoneFriendList *list, const char *ref_key) {
	std::shared_ptr<Friend> lf = FriendList::toCpp(list)->findFriendByRefKey(L_C_TO_STRING(ref_key));
	return lf ? lf->toC() : nullptr;
}

void linphone_friend_list_update_subscriptions(LinphoneFriendList *list) {
	FriendList::toCpp(list)->updateSubscriptions();
}

void linphone_friend_list_notify_presence(LinphoneFriendList *list, LinphonePresenceModel *presence) {
	FriendList::toCpp(list)->notifyPresence(PresenceModel::getSharedFromThis(presence));
}

void linphone_friend_list_notify_presence_received(LinphoneFriendList *list,
                                                   BCTBX_UNUSED(LinphoneEvent *lev),
                                                   const LinphoneContent *body) {
	if (!body) return;
	FriendList::toCpp(list)->notifyPresenceReceived(Content::getSharedFromThis(body));
}

const char *linphone_friend_list_get_uri(const LinphoneFriendList *list) {
	return L_STRING_TO_C(FriendList::toCpp(list)->getUri());
}

void linphone_friend_list_set_uri(LinphoneFriendList *list, const char *uri) {
	FriendList::toCpp(list)->setUri(L_C_TO_STRING(uri));
}

bool_t linphone_friend_list_is_subscription_bodyless(const LinphoneFriendList *list) {
	return FriendList::toCpp(list)->isSubscriptionBodyless();
}

void linphone_friend_list_set_subscription_bodyless(LinphoneFriendList *list, bool_t bodyless) {
	FriendList::toCpp(list)->setSubscriptionBodyless(bodyless);
}

void linphone_friend_list_update_revision(LinphoneFriendList *list, const char *rev) {
	FriendList::toCpp(list)->updateRevision(L_C_TO_STRING(rev));
}

void linphone_friend_list_subscription_state_changed(LinphoneCore *lc,
                                                     LinphoneEvent *lev,
                                                     LinphoneSubscriptionState state) {
	FriendList::subscriptionStateChanged(lc, Event::getSharedFromThis(lev), state);
}

LinphoneCore *linphone_friend_list_get_core(const LinphoneFriendList *list) {
	try {
		auto core = FriendList::toCpp(list)->getCore();
		return core->getCCore();
	} catch (std::exception &) {
	}
	return nullptr;
}

LinphoneStatus linphone_friend_list_import_friends_from_vcard4_file(LinphoneFriendList *list, const char *vcard_file) {
	if (!list) {
		ms_error("Can't import into a NULL list");
		return -1;
	}
	return FriendList::toCpp(list)->importFriendsFromVcard4File(L_C_TO_STRING(vcard_file));
}

LinphoneStatus linphone_friend_list_import_friends_from_vcard4_buffer(LinphoneFriendList *list,
                                                                      const char *vcard_buffer) {
	if (!list) {
		ms_error("Can't import into a NULL list");
		return -1;
	}
	return FriendList::toCpp(list)->importFriendsFromVcard4Buffer(L_C_TO_STRING(vcard_buffer));
}

void linphone_friend_list_export_friends_as_vcard4_file(LinphoneFriendList *list, const char *vcard_file) {
	FriendList::toCpp(list)->exportFriendsAsVcard4File(L_C_TO_STRING(vcard_file));
}

void linphone_friend_list_enable_subscriptions(LinphoneFriendList *list, bool_t enabled) {
	FriendList::toCpp(list)->enableSubscriptions(enabled);
}

bool_t linphone_friend_list_subscriptions_enabled(LinphoneFriendList *list) {
	return FriendList::toCpp(list)->subscriptionsEnabled();
}

bool_t linphone_friend_list_database_storage_enabled(const LinphoneFriendList *list) {
	return FriendList::toCpp(list)->databaseStorageEnabled();
}

void linphone_friend_list_enable_database_storage(LinphoneFriendList *list, bool_t enable) {
	FriendList::toCpp(list)->enableDatabaseStorage(enable);
}

LinphoneEvent *linphone_friend_list_get_event(const LinphoneFriendList *list) {
	std::shared_ptr<Event> event = FriendList::toCpp(list)->mEvent;
	return event ? event->toC() : nullptr;
}
