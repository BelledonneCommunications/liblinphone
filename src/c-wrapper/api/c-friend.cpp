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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <bctoolbox/defs.h>

#include "c-wrapper/internal/c-tools.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "presence/presence-model.h"
#include "vcard/vcard-context.h"

#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__QNXNTO__)
#include <iconv.h>
#include <langinfo.h>
#include <string.h>
#endif // if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__QNXNTO__)

#define MAX_PATH_SIZE 1024

#include "c-wrapper/c-wrapper.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "friend/friend-list.h"
#include "friend/friend.h"
#include "friend/friend_phone_number.h"
#include "vcard/vcard.h"

using namespace std;

using namespace LinphonePrivate;

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

void linphone_friend_add_address(LinphoneFriend *lf, const LinphoneAddress *addr) {
	if (!lf) return;
	Friend::toCpp(lf)->addAddress(Address::getSharedFromThis(addr));
}

void linphone_friend_add_callbacks(LinphoneFriend *lf, LinphoneFriendCbs *cbs) {
	Friend::toCpp(lf)->addCallbacks(FriendCbs::getSharedFromThis(cbs));
}

void linphone_friend_add_phone_number(LinphoneFriend *lf, const char *phone) {
	if (!lf) return;
	Friend::toCpp(lf)->addPhoneNumber(L_C_TO_STRING(phone));
}

void linphone_friend_add_phone_number_with_label(LinphoneFriend *lf, LinphoneFriendPhoneNumber *phoneNumber) {
	if (!lf) return;
	Friend::toCpp(lf)->addPhoneNumberWithLabel(FriendPhoneNumber::getSharedFromThis(phoneNumber));
}

bool_t linphone_friend_create_vcard(LinphoneFriend *lf, const char *name) {
	if (!lf) return FALSE;
	return Friend::toCpp(lf)->createVcard(L_C_TO_STRING(name));
}

void linphone_friend_done(LinphoneFriend *lf) {
	ms_return_if_fail(lf);
	return Friend::toCpp(lf)->done();
}

void linphone_friend_edit(LinphoneFriend *lf) {
	if (!lf) return;
	Friend::toCpp(lf)->edit();
}

LinphoneStatus linphone_friend_enable_subscribes(LinphoneFriend *lf, bool_t val) {
	return Friend::toCpp(lf)->enableSubscribes(val);
}

const LinphoneAddress *linphone_friend_get_address(const LinphoneFriend *lf) {
	return (Friend::toCpp(lf)->getAddress()) ? Friend::toCpp(lf)->getAddress()->toC() : NULL;
}

const bctbx_list_t *linphone_friend_get_addresses(const LinphoneFriend *lf) {
	if (!lf) return nullptr;
	Friend::toCpp(lf)->getAddresses();
	return Friend::toCpp(lf)->mBctbxAddresses;
}

int linphone_friend_get_capabilities(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getCapabilities();
}

float linphone_friend_get_capability_version(const LinphoneFriend *lf, const LinphoneFriendCapability capability) {
	return Friend::toCpp(lf)->getCapabilityVersion(capability);
}

LinphoneConsolidatedPresence linphone_friend_get_consolidated_presence(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getConsolidatedPresence();
}

LinphoneCore *linphone_friend_get_core(const LinphoneFriend *lf) {
	return L_GET_C_BACK_PTR(Friend::toCpp(lf)->getCore());
}

LinphoneFriendCbs *linphone_friend_get_current_callbacks(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getCurrentCallbacks()->toC();
}

LinphoneSubscribePolicy linphone_friend_get_inc_subscribe_policy(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getIncSubscribePolicy();
}

BuddyInfo *linphone_friend_get_info(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getInfo();
}

const char *linphone_friend_get_job_title(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	return L_STRING_TO_C(Friend::toCpp(lf)->getJobTitle());
}

const char *linphone_friend_get_name(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	return L_STRING_TO_C(Friend::toCpp(lf)->getName());
}

const char *linphone_friend_get_native_uri(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	return L_STRING_TO_C(Friend::toCpp(lf)->getNativeUri());
}

const char *linphone_friend_get_organization(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	return L_STRING_TO_C(Friend::toCpp(lf)->getOrganization());
}

bctbx_list_t *linphone_friend_get_phone_numbers(const LinphoneFriend *lf) {
	if (!lf) return nullptr;
	return L_GET_C_LIST_FROM_CPP_LIST(Friend::toCpp(lf)->getPhoneNumbers());
}

bctbx_list_t *linphone_friend_get_phone_numbers_with_label(const LinphoneFriend *lf) {
	if (!lf) return nullptr;

	bctbx_list_t *results = nullptr;
	std::list<shared_ptr<FriendPhoneNumber>> phoneNumbers = Friend::toCpp(lf)->getPhoneNumbersWithLabel();
	for (auto &phoneNumber : phoneNumbers) {
		results = bctbx_list_append(results, linphone_friend_phone_number_ref(phoneNumber->toC()));
	}
	return results;
}

const char *linphone_friend_get_photo(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	return L_STRING_TO_C(Friend::toCpp(lf)->getPhoto());
}

const LinphonePresenceModel *linphone_friend_get_presence_model(const LinphoneFriend *lf) {
	const std::shared_ptr<PresenceModel> &model = Friend::toCpp(lf)->getPresenceModel();
	return model ? model->toC() : nullptr;
}

const LinphonePresenceModel *linphone_friend_get_presence_model_for_uri_or_tel(const LinphoneFriend *lf,
                                                                               const char *uri_or_tel) {
	const std::shared_ptr<PresenceModel> &model =
	    Friend::toCpp(lf)->getPresenceModelForUriOrTel(L_C_TO_STRING(uri_or_tel));
	return model ? model->toC() : nullptr;
}

const char *linphone_friend_get_ref_key(const LinphoneFriend *lf) {
	return L_STRING_TO_C(Friend::toCpp(lf)->getRefKey());
}

bool_t linphone_friend_get_starred(const LinphoneFriend *lf) {
	if (!lf) return FALSE;
	return Friend::toCpp(lf)->getStarred();
}

bool_t linphone_friend_get_send_subscribe(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->subscribesEnabled();
}

LinphoneSubscriptionState linphone_friend_get_subscription_state(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getSubscriptionState();
}

void *linphone_friend_get_user_data(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getUserData();
}

LinphoneVcard *linphone_friend_get_vcard(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	const std::shared_ptr<Vcard> vcard = Friend::toCpp(lf)->getVcard();
	return vcard ? vcard->toC() : nullptr;
}

bool_t linphone_friend_has_capability(const LinphoneFriend *lf, const LinphoneFriendCapability capability) {
	return Friend::toCpp(lf)->hasCapability(capability);
}

bool_t linphone_friend_has_capability_with_version(const LinphoneFriend *lf,
                                                   const LinphoneFriendCapability capability,
                                                   float version) {
	return Friend::toCpp(lf)->hasCapabilityWithVersion(capability, version);
}

bool_t linphone_friend_has_capability_with_version_or_more(const LinphoneFriend *lf,
                                                           const LinphoneFriendCapability capability,
                                                           float version) {
	return Friend::toCpp(lf)->hasCapabilityWithVersionOrMore(capability, version);
}

bool_t linphone_friend_has_phone_number(const LinphoneFriend *lf, const char *phoneNumber) {
	if (!lf) return FALSE;
	return Friend::toCpp(lf)->hasPhoneNumber(L_C_TO_STRING(phoneNumber));
}

bool_t linphone_friend_in_list(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->inList();
}

bool_t linphone_friend_is_presence_received(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->isPresenceReceived();
}

LinphoneFriend *linphone_friend_ref(LinphoneFriend *lf) {
	Friend::toCpp(lf)->ref();
	return lf;
}

void linphone_friend_remove(LinphoneFriend *lf) {
	if (!lf) return;
	Friend::toCpp(lf)->remove();
}

void linphone_friend_remove_address(LinphoneFriend *lf, const LinphoneAddress *addr) {
	if (!lf) return;
	Friend::toCpp(lf)->removeAddress(Address::getSharedFromThis(addr));
}

void linphone_friend_remove_callbacks(LinphoneFriend *lf, LinphoneFriendCbs *cbs) {
	Friend::toCpp(lf)->removeCallbacks(FriendCbs::getSharedFromThis(cbs));
}

void linphone_friend_remove_phone_number(LinphoneFriend *lf, const char *phone) {
	if (!lf) return;
	Friend::toCpp(lf)->removePhoneNumber(L_C_TO_STRING(phone));
}

void linphone_friend_remove_phone_number_with_label(LinphoneFriend *lf, const LinphoneFriendPhoneNumber *phoneNumber) {
	if (!lf) return;
	Friend::toCpp(lf)->removePhoneNumberWithLabel(FriendPhoneNumber::getSharedFromThis(phoneNumber));
}

void linphone_friend_save(LinphoneFriend *lf, BCTBX_UNUSED(LinphoneCore *lc)) {
	Friend::toCpp(lf)->save();
}

LinphoneStatus linphone_friend_set_address(LinphoneFriend *lf, const LinphoneAddress *addr) {
	return Friend::toCpp(lf)->setAddress(Address::getSharedFromThis(addr));
}

LinphoneStatus linphone_friend_set_inc_subscribe_policy(LinphoneFriend *lf, LinphoneSubscribePolicy pol) {
	return Friend::toCpp(lf)->setIncSubscribePolicy(pol);
}

void linphone_friend_set_job_title(LinphoneFriend *lf, const char *job_title) {
	if (!lf) return;
	Friend::toCpp(lf)->setJobTitle(L_C_TO_STRING(job_title));
}

LinphoneStatus linphone_friend_set_name(LinphoneFriend *lf, const char *name) {
	return Friend::toCpp(lf)->setName(L_C_TO_STRING(name));
}

void linphone_friend_set_native_uri(LinphoneFriend *lf, const char *native_uri) {
	if (!lf) return;
	Friend::toCpp(lf)->setNativeUri(native_uri);
}

void linphone_friend_set_organization(LinphoneFriend *lf, const char *organization) {
	if (!lf) return;
	Friend::toCpp(lf)->setOrganization(L_C_TO_STRING(organization));
}

void linphone_friend_set_photo(LinphoneFriend *lf, const char *picture_uri) {
	if (!lf) return;
	Friend::toCpp(lf)->setPhoto(L_C_TO_STRING(picture_uri));
}

void linphone_friend_set_presence_model(LinphoneFriend *lf, LinphonePresenceModel *presence) {
	const std::shared_ptr<PresenceModel> model = presence ? PresenceModel::getSharedFromThis(presence) : nullptr;
	Friend::toCpp(lf)->setPresenceModel(model);
}

void linphone_friend_set_presence_model_for_uri_or_tel(LinphoneFriend *lf,
                                                       const char *uri_or_tel,
                                                       LinphonePresenceModel *presence) {
	const std::shared_ptr<PresenceModel> model = presence ? PresenceModel::getSharedFromThis(presence) : nullptr;
	Friend::toCpp(lf)->setPresenceModelForUriOrTel(L_C_TO_STRING(uri_or_tel), model);
}

void linphone_friend_set_ref_key(LinphoneFriend *lf, const char *key) {
	Friend::toCpp(lf)->setRefKey(L_C_TO_STRING(key));
}

void linphone_friend_set_starred(LinphoneFriend *lf, bool_t is_starred) {
	if (lf) Friend::toCpp(lf)->setStarred(is_starred);
}

void linphone_friend_set_user_data(LinphoneFriend *lf, void *data) {
	Friend::toCpp(lf)->setUserData(data);
}

void linphone_friend_set_vcard(LinphoneFriend *lf, LinphoneVcard *vcard) {
	if (lf) Friend::toCpp(lf)->setVcard(Vcard::getSharedFromThis(vcard));
}

void linphone_friend_unref(LinphoneFriend *lf) {
	Friend::toCpp(lf)->unref();
}

LinphoneFriendCbsPresenceReceivedCb linphone_friend_cbs_get_presence_received(const LinphoneFriendCbs *cbs) {
	return FriendCbs::toCpp(cbs)->getPresenceReceived();
}

void *linphone_friend_cbs_get_user_data(const LinphoneFriendCbs *cbs) {
	return FriendCbs::toCpp(cbs)->getUserData();
}

LinphoneFriendCbs *linphone_friend_cbs_ref(LinphoneFriendCbs *cbs) {
	FriendCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_friend_cbs_set_presence_received(LinphoneFriendCbs *cbs, LinphoneFriendCbsPresenceReceivedCb cb) {
	FriendCbs::toCpp(cbs)->setPresenceReceived(cb);
}

void linphone_friend_cbs_set_user_data(LinphoneFriendCbs *cbs, void *ud) {
	FriendCbs::toCpp(cbs)->setUserData(ud);
}

void linphone_friend_cbs_unref(LinphoneFriendCbs *cbs) {
	FriendCbs::toCpp(cbs)->unref();
}

const bctbx_list_t *linphone_friend_get_callbacks_list(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->getCCallbacksList();
}

// -----------------------------------------------------------------------------
// Private functions
// -----------------------------------------------------------------------------

void linphone_friend_add_incoming_subscription(LinphoneFriend *lf, SalOp *op) {
	Friend::toCpp(lf)->addIncomingSubscription(op);
}

LinphoneFriendCbs *linphone_friend_cbs_new(void) {
	return FriendCbs::createCObject();
}

LinphonePrivate::SalPresenceOp *linphone_friend_get_outsub(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->mOutSub;
}

int linphone_friend_get_rc_index(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->mRcIndex;
}

LinphoneAddress *linphone_friend_get_uri(const LinphoneFriend *lf) {
	return Friend::toCpp(lf)->mUri ? Friend::toCpp(lf)->mUri->toC() : nullptr;
}

void linphone_friend_invalidate_subscription(LinphoneFriend *lf) {
	Friend::toCpp(lf)->invalidateSubscription();
}

#define key_compare(s1, s2) strcmp(s1, s2)

static LinphoneSubscribePolicy __policy_str_to_enum(const char *pol) {
	if (key_compare("accept", pol) == 0) {
		return LinphoneSPAccept;
	}
	if (key_compare("deny", pol) == 0) {
		return LinphoneSPDeny;
	}
	if (key_compare("wait", pol) == 0) {
		return LinphoneSPWait;
	}
	ms_warning("Unrecognized subscribe policy: %s", pol);
	return LinphoneSPWait;
}

LinphoneFriend *linphone_friend_new_from_config_file(LinphoneCore *lc, int index) {
	const char *tmp;
	char item[50];
	int a;
	LinphoneFriend *lf;
	LpConfig *config = lc->config;

	sprintf(item, "friend_%i", index);

	if (!linphone_config_has_section(config, item)) {
		return NULL;
	}

	tmp = linphone_config_get_string(config, item, "url", NULL);
	if (tmp == NULL) {
		return NULL;
	}
	lf = linphone_core_create_friend_with_address(lc, tmp);
	if (lf == NULL) {
		return NULL;
	}
	tmp = linphone_config_get_string(config, item, "pol", NULL);
	if (tmp == NULL) linphone_friend_set_inc_subscribe_policy(lf, LinphoneSPWait);
	else {
		linphone_friend_set_inc_subscribe_policy(lf, __policy_str_to_enum(tmp));
	}
	a = linphone_config_get_int(config, item, "subscribe", 0);
	linphone_friend_send_subscribe(lf, !!a);
	a = linphone_config_get_int(config, item, "presence_received", 0);
	Friend::toCpp(lf)->mPresenceReceived = (bool)a;
	Friend::toCpp(lf)->mRcIndex = index;

	linphone_friend_set_ref_key(lf, linphone_config_get_string(config, item, "refkey", NULL));
	linphone_friend_set_starred(lf, linphone_config_get_bool(config, item, "starred", FALSE));
	return lf;
}

void linphone_friend_notify_presence_received(LinphoneFriend *lf) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Friend, Friend::toCpp(lf), linphone_friend_cbs_get_presence_received);
}

void linphone_friend_release(LinphoneFriend *lf) {
	Friend::toCpp(lf)->releaseOps();
	Friend::toCpp(lf)->unref();
}

void linphone_friend_remove_incoming_subscription(LinphoneFriend *lf, SalOp *op) {
	Friend::toCpp(lf)->removeIncomingSubscription(op);
}

void linphone_friend_set_inc_subscribe_pending(LinphoneFriend *lf, bool_t pending) {
	Friend::toCpp(lf)->mIncSubscribePending = pending;
}

void linphone_friend_set_info(LinphoneFriend *lf, BuddyInfo *info) {
	Friend::toCpp(lf)->mInfo = info;
}

void linphone_friend_set_outsub(LinphoneFriend *lf, LinphonePrivate::SalPresenceOp *outsub) {
	Friend::toCpp(lf)->mOutSub = outsub;
}

void linphone_friend_set_out_sub_state(LinphoneFriend *lf, LinphoneSubscriptionState state) {
	Friend::toCpp(lf)->mOutSubState = state;
}

void linphone_friend_set_presence_received(LinphoneFriend *lf, bool_t received) {
	Friend::toCpp(lf)->mPresenceReceived = received;
}

void linphone_friend_set_storage_id(LinphoneFriend *lf, unsigned int id) {
	Friend::toCpp(lf)->mStorageId = id;
}

void linphone_friend_set_subscribe(LinphoneFriend *lf, bool_t subscribe) {
	Friend::toCpp(lf)->mSubscribe = subscribe;
}

void linphone_friend_set_subscribe_active(LinphoneFriend *lf, bool_t active) {
	Friend::toCpp(lf)->mSubscribeActive = active;
}

void linphone_friend_update_subscribes(LinphoneFriend *lf, bool_t only_when_registered) {
	Friend::toCpp(lf)->updateSubscribes(only_when_registered);
}

// -----------------------------------------------------------------------------
// DEPRECATED functions
// -----------------------------------------------------------------------------

const char *linphone_online_status_to_string(LinphoneOnlineStatus ss) {
	const char *str = NULL;
	switch (ss) {
		case LinphoneStatusOnline:
			str = "Online";
			break;
		case LinphoneStatusBusy:
			str = "Busy";
			break;
		case LinphoneStatusBeRightBack:
			str = "Be right back";
			break;
		case LinphoneStatusAway:
			str = "Away";
			break;
		case LinphoneStatusOnThePhone:
			str = "On the phone";
			break;
		case LinphoneStatusOutToLunch:
			str = "Out to lunch";
			break;
		case LinphoneStatusDoNotDisturb:
			str = "Do not disturb";
			break;
		case LinphoneStatusMoved:
			str = "Moved";
			break;
		case LinphoneStatusAltService:
			str = "Using another messaging service";
			break;
		case LinphoneStatusOffline:
			str = "Offline";
			break;
		case LinphoneStatusPending:
			str = "Pending";
			break;
		case LinphoneStatusVacation:
			str = "Vacation";
			break;
		default:
			str = "Unknown status";
	}
	return str;
}

LinphoneOnlineStatus linphone_friend_get_status(const LinphoneFriend *lf) {
	const LinphonePresenceModel *presence = linphone_friend_get_presence_model(lf);
	LinphoneOnlineStatus online_status = LinphoneStatusOffline;
	LinphonePresenceBasicStatus basic_status = LinphonePresenceBasicStatusClosed;
	LinphonePresenceActivity *activity = NULL;
	const char *description = NULL;
	unsigned int nb_activities = 0;

	if (presence != NULL) {
		basic_status = linphone_presence_model_get_basic_status(presence);
		nb_activities = linphone_presence_model_get_nb_activities(presence);
		online_status =
		    (basic_status == LinphonePresenceBasicStatusOpen) ? LinphoneStatusOnline : LinphoneStatusOffline;
		if (nb_activities > 1) {
			char *tmp = NULL;
			const LinphoneAddress *addr = linphone_friend_get_address(lf);
			if (addr) tmp = linphone_address_as_string(addr);
			ms_warning("Friend %s has several activities, get status from the first one", tmp ? tmp : "unknown");
			if (tmp) {
				ms_free(tmp);
			}
			nb_activities = 1;
		}
		if (nb_activities == 1) {
			activity = linphone_presence_model_get_activity(presence);
			description = linphone_presence_activity_get_description(activity);
			switch (linphone_presence_activity_get_type(activity)) {
				case LinphonePresenceActivityBreakfast:
				case LinphonePresenceActivityDinner:
				case LinphonePresenceActivityLunch:
				case LinphonePresenceActivityMeal:
					online_status = LinphoneStatusOutToLunch;
					break;
				case LinphonePresenceActivityAppointment:
				case LinphonePresenceActivityMeeting:
				case LinphonePresenceActivityPerformance:
				case LinphonePresenceActivityPresentation:
				case LinphonePresenceActivitySpectator:
				case LinphonePresenceActivityWorking:
				case LinphonePresenceActivityWorship:
					online_status = LinphoneStatusDoNotDisturb;
					break;
				case LinphonePresenceActivityAway:
				case LinphonePresenceActivitySleeping:
					online_status = LinphoneStatusAway;
					break;
				case LinphonePresenceActivityHoliday:
				case LinphonePresenceActivityTravel:
				case LinphonePresenceActivityVacation:
					online_status = LinphoneStatusVacation;
					break;
				case LinphonePresenceActivityBusy:
					if (description && strcmp(description, "Do not disturb") ==
					                       0) { // See linphonecore.c linphone_core_set_presence_info() method
						online_status = LinphoneStatusDoNotDisturb;
					} else {
						online_status = LinphoneStatusBusy;
					}
					break;
				case LinphonePresenceActivityLookingForWork:
				case LinphonePresenceActivityPlaying:
				case LinphonePresenceActivityShopping:
				case LinphonePresenceActivityTV:
					online_status = LinphoneStatusBusy;
					break;
				case LinphonePresenceActivityInTransit:
				case LinphonePresenceActivitySteering:
					online_status = LinphoneStatusBeRightBack;
					break;
				case LinphonePresenceActivityOnThePhone:
					online_status = LinphoneStatusOnThePhone;
					break;
				case LinphonePresenceActivityOther:
				case LinphonePresenceActivityPermanentAbsence:
					online_status = LinphoneStatusMoved;
					break;
				case LinphonePresenceActivityUnknown:
					/* Rely on the basic status information. */
					break;
			}
		}
	}

	return online_status;
}

// -----------------------------------------------------------------------------
// Core functions
// -----------------------------------------------------------------------------

void linphone_core_add_friend(LinphoneCore *lc, LinphoneFriend *lf) {
	LinphoneFriendList *friendList = linphone_core_get_default_friend_list(lc);
	if (!friendList) {
		friendList = linphone_core_create_friend_list(lc);
		linphone_core_add_friend_list(lc, friendList);
		linphone_friend_list_unref(friendList);
	}
	if (linphone_friend_list_add_friend(friendList, lf) != LinphoneFriendListOK) return;
	if (bctbx_list_find(lc->subscribers, lf)) {
		/*if this friend was in the pending subscriber list, now remove it from this list*/
		lc->subscribers = bctbx_list_remove(lc->subscribers, lf);
		linphone_friend_unref(lf);
	}
}

LinphoneFriend *linphone_core_create_friend(LinphoneCore *lc) {
	return Friend::createCObject(lc);
}

LinphoneFriend *linphone_core_create_friend_with_address(LinphoneCore *lc, const char *address) {
	return Friend::createCObject(lc, address);
}

LinphoneFriend *linphone_core_create_friend_from_vcard(LinphoneCore *lc, LinphoneVcard *vcard) {
	if (!linphone_core_vcard_supported()) {
		ms_error("VCard support is not builtin");
		return nullptr;
	}
	if (!vcard) {
		ms_error("Cannot create friend from null vcard");
		return nullptr;
	}
	return Friend::createCObject(lc, Vcard::getSharedFromThis(vcard));
}

LinphoneFriend *linphone_core_find_friend(const LinphoneCore *lc, const LinphoneAddress *addr) {
	bctbx_list_t *lists = lc->friends_lists;
	LinphoneFriend *lf = NULL;
	while (lists && !lf) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		lf = linphone_friend_list_find_friend_by_address(list, addr);
		lists = bctbx_list_next(lists);
	}
	return lf;
}

LinphoneFriend *linphone_core_get_friend_by_address(const LinphoneCore *lc, const char *uri) {
	bctbx_list_t *lists = lc->friends_lists;
	LinphoneFriend *lf = NULL;
	while (lists && !lf) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		lf = linphone_friend_list_find_friend_by_uri(list, uri);
		lists = bctbx_list_next(lists);
	}
	return lf;
}

LinphoneFriend *linphone_core_find_friend_by_inc_subscribe(const LinphoneCore *lc, SalOp *op) {
	bctbx_list_t *lists = lc->friends_lists;
	std::shared_ptr<Friend> lf = nullptr;
	while (lists && !lf) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		lf = FriendList::toCpp(list)->findFriendByIncSubscribe(op);
		lists = bctbx_list_next(lists);
	}
	return lf ? lf->toC() : nullptr;
}

LinphoneFriend *linphone_core_find_friend_by_out_subscribe(const LinphoneCore *lc, SalOp *op) {
	bctbx_list_t *lists = lc->friends_lists;
	std::shared_ptr<Friend> lf = nullptr;
	while (lists && !lf) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		lf = FriendList::toCpp(list)->findFriendByOutSubscribe(op);
		lists = bctbx_list_next(lists);
	}
	return lf ? lf->toC() : nullptr;
}

LinphoneFriend *linphone_core_find_friend_by_phone_number(const LinphoneCore *lc, const char *phoneNumber) {
	LinphoneAccount *account = linphone_core_get_default_account(lc);
	// Account can be null, both linphone_account_is_phone_number and linphone_account_normalize_phone_number can handle
	// it
	if (phoneNumber == NULL || !linphone_account_is_phone_number(account, phoneNumber)) {
		ms_warning("Phone number [%s] isn't valid", phoneNumber);
		return NULL;
	}

	if (!linphone_core_vcard_supported()) {
		ms_warning("SDK built without vCard support, can't do a phone number search without it");
		return NULL;
	}

	const bctbx_list_t *elem;
	const bctbx_list_t *accounts = linphone_core_get_account_list(lc);
	for (elem = accounts; elem != NULL; elem = bctbx_list_next(elem)) {
		account = (LinphoneAccount *)bctbx_list_get_data(elem);
		char *normalized_phone_number = linphone_account_normalize_phone_number(account, phoneNumber);

		bctbx_list_t *lists = lc->friends_lists;
		LinphoneFriend *lf = NULL;
		while (lists && !lf) {
			std::shared_ptr<FriendList> list =
			    FriendList::getSharedFromThis((LinphoneFriendList *)bctbx_list_get_data(lists));
			std::shared_ptr<Friend> f =
			    list->findFriendByPhoneNumber(Account::getSharedFromThis(account), normalized_phone_number);
			if (f) lf = f->toC();
			lists = bctbx_list_next(lists);
		}

		ms_free(normalized_phone_number);
		if (lf) return lf;
	}
	return NULL;
}

LinphoneFriend *linphone_core_get_friend_by_ref_key(const LinphoneCore *lc, const char *key) {
	bctbx_list_t *lists = lc->friends_lists;
	LinphoneFriend *lf = NULL;
	while (lists && !lf) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		lf = linphone_friend_list_find_friend_by_ref_key(list, key);
		lists = bctbx_list_next(lists);
	}
	return lf;
}

bctbx_list_t *linphone_core_find_friends(const LinphoneCore *lc, const LinphoneAddress *addr) {
	bctbx_list_t *result = NULL;
	bctbx_list_t *lists = lc->friends_lists;
	while (lists) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		bctbx_list_t *list_result = linphone_friend_list_find_friends_by_address(list, addr);
		if (list_result) {
			bctbx_list_t *elem;
			for (elem = list_result; elem != NULL; elem = bctbx_list_next(elem)) {
				LinphoneFriend *lf = (LinphoneFriend *)bctbx_list_get_data(elem);
				if (lf) {
					result = bctbx_list_append(result, linphone_friend_ref(lf));
				}
			}
			bctbx_list_free_with_data(list_result, (void (*)(void *))linphone_friend_unref);
		}
		lists = bctbx_list_next(lists);
	}
	return result;
}

const char *linphone_core_get_friends_database_path(LinphoneCore *lc) {
	return lc->friends_db_file;
}

void linphone_core_interpret_friend_uri(LinphoneCore *lc, const char *uri, char **result) {
	LinphoneAddress *fr = NULL;
	*result = NULL;
	fr = linphone_address_new(uri);
	if (fr == NULL) {
		char *tmp = NULL;
		if (strchr(uri, '@') != NULL) {
			LinphoneAddress *u;
			/*try adding sip:*/
			tmp = ms_strdup_printf("sip:%s", uri);
			u = linphone_address_new(tmp);
			if (u != NULL) {
				*result = tmp;
			}
		} else if (lc->default_proxy != NULL) {
			/*try adding domain part from default current proxy*/
			LinphoneAddress *id = linphone_address_new(linphone_core_get_identity(lc));
			if ((id != NULL) && (uri[0] != '\0')) {
				linphone_address_set_display_name(id, NULL);
				linphone_address_set_username(id, uri);
				*result = linphone_address_as_string(id);
				linphone_address_unref(id);
			}
		}
		if (*result) {
			/*looks good */
			ms_message("%s interpreted as %s", uri, *result);
		} else {
			ms_warning("Fail to interpret friend uri %s", uri);
		}
	} else {
		*result = linphone_address_as_string(fr);
		linphone_address_unref(fr);
	}
}

void linphone_core_invalidate_friend_subscriptions(LinphoneCore *lc) {
	bctbx_list_t *lists = lc->friends_lists;
	while (lists) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		FriendList::toCpp(list)->invalidateSubscriptions();
		lists = bctbx_list_next(lists);
	}
	lc->initial_subscribes_sent = FALSE;
}

void linphone_core_remove_friend(BCTBX_UNUSED(LinphoneCore *lc), LinphoneFriend *lf) {
	if (lf && Friend::toCpp(lf)->mFriendList) {
		LinphoneFriendListStatus status = Friend::toCpp(lf)->mFriendList->removeFriend(Friend::getSharedFromThis(lf));
		if (status == LinphoneFriendListNonExistentFriend)
			ms_error("linphone_core_remove_friend(): friend [%p] is not part of core's list.", lf);
	}
}

void linphone_core_send_initial_subscribes(LinphoneCore *lc) {
	if (lc->initial_subscribes_sent) return;
	linphone_core_update_friends_subscriptions(lc);
	ms_message("Initial friend lists subscribes has been sent");
}

void linphone_core_set_friends_database_path(LinphoneCore *lc, const char *path) {
	if (!linphone_core_conference_server_enabled(lc) && L_GET_PRIVATE(lc->cppPtr)->mainDb)
		L_GET_PRIVATE(lc->cppPtr)->mainDb->import(LinphonePrivate::MainDb::Sqlite3, path);

	// TODO: Remove me later.
	if (lc->friends_db_file) {
		ms_free(lc->friends_db_file);
		lc->friends_db_file = NULL;
	}
	if (path) {
		ms_message("Using [%s] file for friends database", path);
		lc->friends_db_file = ms_strdup(path);
		linphone_core_friends_storage_init(lc);
	}
}

bool_t linphone_core_should_subscribe_friends_only_when_registered(const LinphoneCore *lc) {
	return !!linphone_config_get_int(lc->config, "sip", "subscribe_presence_only_when_registered", 1);
}

void linphone_core_update_friends_subscriptions(LinphoneCore *lc) {
	bctbx_list_t *lists = lc->friends_lists;
	while (lists) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(lists);
		linphone_friend_list_update_subscriptions(list);
		lists = bctbx_list_next(lists);
	}
	// done here to avoid double initial subscribtion if triggered by proxy or from the app.
	lc->initial_subscribes_sent = TRUE;
}

// -----------------------------------------------------------------------------
// SQL storage related functions
// -----------------------------------------------------------------------------

#ifdef HAVE_SQLITE

static void linphone_create_friends_table(sqlite3 *db) {
	char *errmsg = NULL;
	int ret;
	ret = sqlite3_exec(db,
	                   "CREATE TABLE IF NOT EXISTS friends ("
	                   "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
	                   "friend_list_id    INTEGER,"
	                   "sip_uri           TEXT,"
	                   "subscribe_policy  INTEGER,"
	                   "send_subscribe    INTEGER,"
	                   "ref_key           TEXT,"
	                   "vCard             TEXT,"
	                   "vCard_etag        TEXT,"
	                   "vCard_url         TEXT,"
	                   "presence_received INTEGER"
	                   ");",
	                   0, 0, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("Error in creation: %s.", errmsg);
		sqlite3_free(errmsg);
	}

	ret = sqlite3_exec(db,
	                   "CREATE TABLE IF NOT EXISTS friends_lists ("
	                   "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
	                   "display_name      TEXT,"
	                   "rls_uri           TEXT,"
	                   "uri               TEXT,"
	                   "revision          INTEGER"
	                   ");",
	                   0, 0, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("Error in creation: %s.", errmsg);
		sqlite3_free(errmsg);
	}
}

static bool_t linphone_update_friends_table(sqlite3 *db) {
	static sqlite3_stmt *stmt_version;
	int database_user_version = -1;
	char *errmsg = NULL;

	if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt_version, NULL) == SQLITE_OK) {
		while (sqlite3_step(stmt_version) == SQLITE_ROW) {
			database_user_version = sqlite3_column_int(stmt_version, 0);
			ms_debug("friends database user version = %i", database_user_version);
		}
	}
	sqlite3_finalize(stmt_version);

	if (database_user_version != 3100) { // Linphone 3.10.0
		int ret =
		    sqlite3_exec(db,
		                 "BEGIN TRANSACTION;\n"
		                 "ALTER TABLE friends RENAME TO temp_friends;\n"
		                 "CREATE TABLE IF NOT EXISTS friends ("
		                 "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
		                 "friend_list_id    INTEGER,"
		                 "sip_uri           TEXT,"
		                 "subscribe_policy  INTEGER,"
		                 "send_subscribe    INTEGER,"
		                 "ref_key           TEXT,"
		                 "vCard             TEXT,"
		                 "vCard_etag        TEXT,"
		                 "vCard_url         TEXT,"
		                 "presence_received INTEGER"
		                 ");\n"
		                 "INSERT INTO friends SELECT id, friend_list_id, sip_uri, subscribe_policy, send_subscribe, "
		                 "ref_key, vCard, vCard_etag, vCard_url, presence_received FROM temp_friends;\n"
		                 "DROP TABLE temp_friends;\n"
		                 "PRAGMA user_version = 3100;\n"
		                 "COMMIT;",
		                 0, 0, &errmsg);
		if (ret != SQLITE_OK) {
			ms_error("Error altering table friends: %s.", errmsg);
			sqlite3_free(errmsg);
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

void linphone_core_friends_storage_init(LinphoneCore *lc) {
	int ret;
	const char *errmsg;
	sqlite3 *db;

	linphone_core_friends_storage_close(lc);

	ret = _linphone_sqlite3_open(lc->friends_db_file, &db);
	if (ret != SQLITE_OK) {
		errmsg = sqlite3_errmsg(db);
		ms_error("Error in the opening: %s.", errmsg);
		sqlite3_close(db);
		return;
	}

	linphone_create_friends_table(db);
	if (linphone_update_friends_table(db)) {
		// After updating schema, database need to be closed/reopenned
		sqlite3_close(db);
		_linphone_sqlite3_open(lc->friends_db_file, &db);
	}

	lc->friends_db = db;

	linphone_core_friends_storage_resync_friends_lists(lc);
}

static int linphone_sql_request_generic(sqlite3 *db, const char *stmt) {
	char *errmsg = NULL;
	int ret;
	ret = sqlite3_exec(db, stmt, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("linphone_sql_request: statement %s -> error sqlite3_exec(): %s.", stmt, errmsg);
		sqlite3_free(errmsg);
	}
	return ret;
}

int linphone_core_friends_storage_resync_friends_lists(LinphoneCore *lc) {
	bctbx_list_t *friends_lists = NULL;
	int synced_friends_lists = 0;

	/**
	 * First lets remove all the orphan friends from the DB
	 */
	char *buf = sqlite3_mprintf("delete from friends where friend_list_id not in (select id from friends_lists)");
	linphone_sql_request_generic(lc->friends_db, buf);
	sqlite3_free(buf);

	friends_lists = linphone_core_fetch_friends_lists_from_db(lc);
	if (friends_lists) {
		const bctbx_list_t *it;
		ms_warning("Replacing current default friend list by the one(s) from the database");
		lc->friends_lists =
		    bctbx_list_free_with_data(lc->friends_lists, (bctbx_list_free_func)linphone_friend_list_unref);

		const char *url = linphone_config_get_string(lc->config, "misc", "contacts-vcard-list", NULL);

		for (it = friends_lists; it != NULL; it = bctbx_list_next(it)) {
			LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(it);

			const char *list_name = linphone_friend_list_get_display_name(list);
			if (list_name && url && strcmp(url, list_name) == 0) {
				linphone_friend_list_set_type(list, LinphoneFriendListTypeVCard4);
			}
			linphone_core_add_friend_list(lc, list);
			synced_friends_lists++;
		}
		friends_lists = bctbx_list_free_with_data(friends_lists, (bctbx_list_free_func)linphone_friend_list_unref);
	}

	return synced_friends_lists;
}

void linphone_core_friends_storage_close(LinphoneCore *lc) {
	if (lc->friends_db) {
		sqlite3_close(lc->friends_db);
		lc->friends_db = NULL;
	}
}

/* DB layout:
 * | 0  | storage_id
 * | 1  | display_name
 * | 2  | rls_uri
 * | 3  | uri
 * | 4  | revision
 */
int create_friend_list_from_db(void *data, BCTBX_UNUSED(int argc), char **argv, BCTBX_UNUSED(char **colName)) {
	bctbx_list_t **list = (bctbx_list_t **)data;
	LinphoneFriendList *lfl = FriendList::createCObject(nullptr);
	std::shared_ptr<FriendList> friendList = FriendList::getSharedFromThis(lfl);
	friendList->mStoreInDb = true; // Obviously
	friendList->mStorageId = (unsigned int)atoi(argv[0]);
	friendList->setDisplayName(L_C_TO_STRING(argv[1]));
	friendList->setRlsUri(L_C_TO_STRING(argv[2]));
	friendList->setUri(L_C_TO_STRING(argv[3]));
	friendList->mRevision = atoi(argv[4]);
	*list = bctbx_list_append(*list, lfl);
	return 0;
}

/* DB layout:
 * | 0  | storage_id
 * | 1  | friend_list_id
 * | 2  | sip_uri
 * | 3  | subscribe_policy
 * | 4  | send_subscribe
 * | 5  | ref_key
 * | 6  | vCard
 * | 7  | vCard eTag
 * | 8  | vCard URL
 * | 9  | presence_received
 */
static int create_friend(void *data, BCTBX_UNUSED(int argc), char **argv, BCTBX_UNUSED(char **colName)) {
	LinphoneCore *lc = (LinphoneCore *)data;
	bctbx_list_t **list = (bctbx_list_t **)VcardContext::toCpp(lc->vcard_context)->getUserData();
	LinphoneFriend *lf = nullptr;
	unsigned int storage_id = (unsigned int)atoi(argv[0]);

	const std::shared_ptr<Vcard> vcard =
	    VcardContext::toCpp(lc->vcard_context)->getVcardFromBuffer(L_C_TO_STRING(argv[6]));
	if (vcard) {
		vcard->setEtag(L_C_TO_STRING(argv[7]));
		vcard->setUrl(L_C_TO_STRING(argv[8]));
		lf = linphone_core_create_friend_from_vcard(lc, vcard->toC());
	}
	if (!lf) {
		lf = linphone_core_create_friend(lc);
		if (argv[2] != nullptr) {
			LinphoneAddress *addr = linphone_address_new(argv[2]);
			if (addr) {
				linphone_friend_set_address(lf, addr);
				linphone_address_unref(addr);
			}
		}
	}
	linphone_friend_set_inc_subscribe_policy(lf, static_cast<LinphoneSubscribePolicy>(atoi(argv[3])));
	linphone_friend_send_subscribe(lf, !!atoi(argv[4]));
	linphone_friend_set_ref_key(lf, ms_strdup(argv[5]));
	linphone_friend_set_presence_received(lf, !!atoi(argv[9]));
	linphone_friend_set_storage_id(lf, storage_id);

	*list = bctbx_list_append(*list, linphone_friend_ref(lf));
	linphone_friend_unref(lf);
	return 0;
}

static int linphone_sql_request_friend(sqlite3 *db, const char *stmt, LinphoneCore *lc) {
	char *errmsg = NULL;
	int ret;
	ret = sqlite3_exec(db, stmt, create_friend, lc, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("linphone_sql_request_friend: statement %s -> error sqlite3_exec(): %s.", stmt, errmsg);
		sqlite3_free(errmsg);
	}
	return ret;
}

static int linphone_sql_request_friends_list(sqlite3 *db, const char *stmt, bctbx_list_t **list) {
	char *errmsg = NULL;
	int ret;
	ret = sqlite3_exec(db, stmt, create_friend_list_from_db, list, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("linphone_sql_request_friends_list: statement %s -> error sqlite3_exec(): %s.", stmt, errmsg);
		sqlite3_free(errmsg);
	}
	return ret;
}

void linphone_core_store_friend_in_db(LinphoneCore *lc, LinphoneFriend *lf) {
	if (lc && lc->friends_db) {
		if (!lf) {
			ms_error("Can't store a NULL friend!");
			return;
		}
		std::shared_ptr<Friend> f = Friend::getSharedFromThis(lf);
		if (!f->mFriendList) {
			ms_warning("Can't store friend [%s], not added to any friend list", f->getName().c_str());
			return;
		}
		if (!f->mFriendList->databaseStorageEnabled()) return;

		/**
		 * The friends_list store logic is hidden into the friend store logic
		 */
		if (f->mFriendList->mStorageId == 0) {
			ms_warning("Trying to add a friend in db, but friend list isn't, let's do that first");
			linphone_core_store_friends_list_in_db(lc, f->mFriendList->toC());
		}

		std::shared_ptr<Vcard> vcard = nullptr;
		char *buf = nullptr;
		std::string addrStr;
		if (linphone_core_vcard_supported()) vcard = f->getVcard();
		const std::shared_ptr<Address> addr = f->getAddress();
		if (addr) addrStr = addr->asString();
		if (Friend::toCpp(lf)->mStorageId > 0) {
			buf =
			    sqlite3_mprintf("UPDATE friends SET "
			                    "friend_list_id=%u,sip_uri=%Q,subscribe_policy=%i,send_subscribe=%i,ref_key=%Q,vCard="
			                    "%Q,vCard_etag=%Q,vCard_url=%Q,presence_received=%i WHERE (id = %u);",
			                    f->mFriendList->mStorageId, L_STRING_TO_C(addrStr), f->mSubscribePolicy, f->mSubscribe,
			                    L_STRING_TO_C(f->mRefKey), vcard ? vcard->asVcard4String().c_str() : nullptr,
			                    vcard ? vcard->getEtag().c_str() : nullptr, vcard ? vcard->getUrl().c_str() : nullptr,
			                    f->isPresenceReceived(), f->mStorageId);
		} else {
			buf = sqlite3_mprintf(
			    "INSERT INTO friends VALUES(NULL,%u,%Q,%i,%i,%Q,%Q,%Q,%Q,%i);", f->mFriendList->mStorageId,
			    L_STRING_TO_C(addrStr), f->mSubscribePolicy, f->mSubscribe, L_STRING_TO_C(f->mRefKey),
			    vcard ? vcard->asVcard4String().c_str() : nullptr, vcard ? vcard->getEtag().c_str() : nullptr,
			    vcard ? vcard->getUrl().c_str() : nullptr, f->isPresenceReceived());
		}
		linphone_sql_request_generic(lc->friends_db, buf);
		sqlite3_free(buf);

		if (f->mStorageId == 0) f->mStorageId = (unsigned int)sqlite3_last_insert_rowid(lc->friends_db);
	}
}

void linphone_core_store_friends_list_in_db(LinphoneCore *lc, LinphoneFriendList *list) {
	if (lc && lc->friends_db) {
		std::shared_ptr<FriendList> friendList = FriendList::getSharedFromThis(list);
		if (!friendList->databaseStorageEnabled()) return;

		char *buf;
		if (friendList->mStorageId > 0) {
			buf = sqlite3_mprintf(
			    "UPDATE friends_lists SET display_name=%Q,rls_uri=%Q,uri=%Q,revision=%i WHERE (id = %u);",
			    friendList->getDisplayName().c_str(), friendList->getRlsUri().c_str(), friendList->getUri().c_str(),
			    friendList->mRevision, friendList->mStorageId);
		} else {
			buf = sqlite3_mprintf("INSERT INTO friends_lists VALUES(NULL,%Q,%Q,%Q,%i);",
			                      friendList->getDisplayName().c_str(), friendList->getRlsUri().c_str(),
			                      friendList->getUri().c_str(), friendList->mRevision);
		}
		linphone_sql_request_generic(lc->friends_db, buf);
		sqlite3_free(buf);

		if (friendList->mStorageId == 0)
			friendList->mStorageId = (unsigned int)sqlite3_last_insert_rowid(lc->friends_db);
	}
}

void linphone_core_remove_friend_from_db(LinphoneCore *lc, LinphoneFriend *lf) {
	if (lc && lc->friends_db) {
		if (Friend::toCpp(lf)->mStorageId == 0) {
			ms_error("Friend doesn't have a storage_id !");
			return;
		}

		char *buf = sqlite3_mprintf("DELETE FROM friends WHERE id = %u", Friend::toCpp(lf)->mStorageId);
		linphone_sql_request_generic(lc->friends_db, buf);
		sqlite3_free(buf);

		Friend::toCpp(lf)->mStorageId = 0;
	}
}

void linphone_core_remove_friends_list_from_db(LinphoneCore *lc, LinphoneFriendList *list) {
	if (lc && lc->friends_db) {
		std::shared_ptr<FriendList> friendList = FriendList::getSharedFromThis(list);
		if (friendList->mStorageId == 0) {
			ms_error("Friend list doesn't have a storage id!");
			return;
		}

		char *buf =
		    sqlite3_mprintf("DELETE FROM friends WHERE friend_list_id in (select id from friends_lists where id = %u)",
		                    friendList->mStorageId);
		linphone_sql_request_generic(lc->friends_db, buf);
		sqlite3_free(buf);

		buf = sqlite3_mprintf("DELETE FROM friends_lists WHERE id = %u", friendList->mStorageId);
		linphone_sql_request_generic(lc->friends_db, buf);
		sqlite3_free(buf);

		friendList->mStorageId = 0;
	}
}

bctbx_list_t *linphone_core_fetch_friends_from_db(LinphoneCore *lc, LinphoneFriendList *list) {
	if (!lc) {
		ms_warning("lc is NULL");
		return nullptr;
	}

	if (!lc->friends_db) {
		ms_warning("Friends database wasn't initialized with linphone_core_friends_storage_init() yet");
		return nullptr;
	}

	bctbx_list_t *result = nullptr;
	VcardContext::toCpp(lc->vcard_context)->setUserData(&result);
	std::shared_ptr<FriendList> friendList = FriendList::getSharedFromThis(list);
	char *buf = sqlite3_mprintf("SELECT * FROM friends WHERE friend_list_id = %u ORDER BY id", friendList->mStorageId);
	uint64_t begin = bctbx_get_cur_time_ms();
	linphone_sql_request_friend(lc->friends_db, buf, lc);
	uint64_t end = bctbx_get_cur_time_ms();
	ms_message("%s(): %u results fetched, completed in %i ms", __FUNCTION__, (unsigned int)bctbx_list_size(result),
	           (int)(end - begin));
	sqlite3_free(buf);

	for (bctbx_list_t *elem = result; elem != NULL; elem = bctbx_list_next(elem)) {
		std::shared_ptr<Friend> lf = Friend::getSharedFromThis((LinphoneFriend *)bctbx_list_get_data(elem));
		lf->mFriendList = friendList.get();
		lf->addAddressesAndNumbersIntoMaps(friendList);
	}
	VcardContext::toCpp(lc->vcard_context)->setUserData(nullptr);

	return result;
}

bctbx_list_t *linphone_core_fetch_friends_lists_from_db(LinphoneCore *lc) {
	if (!lc) {
		ms_warning("lc is NULL");
		return nullptr;
	}

	if (!lc->friends_db) {
		ms_warning("Friends database wasn't initialized with linphone_core_friends_storage_init() yet");
		return nullptr;
	}

	bctbx_list_t *result = nullptr;
	char *buf = sqlite3_mprintf("SELECT * FROM friends_lists ORDER BY id");
	uint64_t begin = bctbx_get_cur_time_ms();
	linphone_sql_request_friends_list(lc->friends_db, buf, &result);
	uint64_t end = bctbx_get_cur_time_ms();
	ms_message("%s(): %u results fetched, completed in %i ms", __FUNCTION__, (unsigned int)bctbx_list_size(result),
	           (int)(end - begin));
	sqlite3_free(buf);

	for (bctbx_list_t *elem = result; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *lfl = (LinphoneFriendList *)bctbx_list_get_data(elem);
		FriendList::toCpp(lfl)->setCore(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
		bctbx_list_t *friends = linphone_core_fetch_friends_from_db(lc, lfl);
		linphone_friend_list_set_friends(lfl, friends);
		bctbx_list_free_with_data(friends, (bctbx_list_free_func)linphone_friend_unref);
	}

	return result;
}

#else

void linphone_core_store_friends_list_in_db(BCTBX_UNUSED(LinphoneCore *lc), BCTBX_UNUSED(LinphoneFriendList *list)) {
	ms_warning("linphone_core_store_friends_list_in_db(): stubbed");
}

void linphone_core_friends_storage_init(BCTBX_UNUSED(LinphoneCore *lc)) {
}

void linphone_core_friends_storage_close(BCTBX_UNUSED(LinphoneCore *lc)) {
}

void linphone_core_store_friend_in_db(BCTBX_UNUSED(LinphoneCore *lc), BCTBX_UNUSED(LinphoneFriend *lf)) {
	ms_warning("linphone_core_store_friend_in_db(): stubbed");
}

void linphone_core_remove_friends_list_from_db(BCTBX_UNUSED(LinphoneCore *lc), BCTBX_UNUSED(LinphoneFriendList *list)) {
	ms_warning("linphone_core_store_friend_in_db(): stubbed");
}

int linphone_core_friends_storage_resync_friends_lists(BCTBX_UNUSED(LinphoneCore *lc)) {
	ms_warning("linphone_core_friends_storage_resync_friends_lists(): stubbed");
	return -1;
}

#endif /* HAVE_SQLITE */
