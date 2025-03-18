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

#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__QNXNTO__)
#include <iconv.h>
#include <langinfo.h>
#include <string.h>
#endif // if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__QNXNTO__)

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "friend/friend-device.h"
#include "friend/friend-list.h"
#include "friend/friend-phone-number.h"
#include "friend/friend.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-friend-phone-number.h"
#include "presence/presence-model.h"
#include "vcard/vcard.h"

#define MAX_PATH_SIZE 1024

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
	return Friend::toCpp(lf)->getAddressesCList();
}

bctbx_list_t *linphone_friend_get_devices(const LinphoneFriend *lf) {
	return FriendDevice::getCListFromCppList(Friend::toCpp(lf)->getDevices());
}

bctbx_list_t *linphone_friend_get_devices_for_address(const LinphoneFriend *lf, const LinphoneAddress *addr) {
	return FriendDevice::getCListFromCppList(Friend::toCpp(lf)->getDevicesForAddress(*Address::toCpp(addr)));
}

LinphoneSecurityLevel linphone_friend_get_security_level(const LinphoneFriend *lf) {
	if (!lf) return LinphoneSecurityLevelNone;
	return Friend::toCpp(lf)->getSecurityLevel();
}

LinphoneSecurityLevel linphone_friend_get_security_level_for_address(const LinphoneFriend *lf,
                                                                     const LinphoneAddress *addr) {
	if (!lf) return LinphoneSecurityLevelNone;
	return Friend::toCpp(lf)->getSecurityLevelForAddress(*Address::toCpp(addr));
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

const char *linphone_friend_get_last_name(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	const std::shared_ptr<Vcard> vcard = Friend::toCpp(lf)->getVcard();
	return vcard ? L_STRING_TO_C(vcard->getFamilyName()) : NULL;
}

const char *linphone_friend_get_first_name(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	const std::shared_ptr<Vcard> vcard = Friend::toCpp(lf)->getVcard();
	return vcard ? L_STRING_TO_C(vcard->getGivenName()) : NULL;
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

const char *linphone_friend_dump_vcard(const LinphoneFriend *lf) {
	if (!lf) return NULL;
	const std::shared_ptr<Vcard> vcard = Friend::toCpp(lf)->getVcard();
	return vcard ? L_STRING_TO_C(vcard->asVcard4String()) : NULL;
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

LinphoneFriendList *linphone_friend_get_friend_list(const LinphoneFriend *linphone_friend) {
	FriendList *list = Friend::toCpp(linphone_friend)->getFriendList();
	if (list) {
		return list->toC();
	}
	return nullptr;
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
	Friend::toCpp(lf)->saveInDb();
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

LinphoneStatus linphone_friend_set_last_name(LinphoneFriend *lf, const char *last_name) {
	const std::shared_ptr<Vcard> vcard = Friend::toCpp(lf)->getVcard();
	if (vcard) {
		vcard->setFamilyName(L_C_TO_STRING(last_name));
		return 0;
	}
	return -1;
}

LinphoneStatus linphone_friend_set_first_name(LinphoneFriend *lf, const char *first_name) {
	const std::shared_ptr<Vcard> vcard = Friend::toCpp(lf)->getVcard();
	if (vcard) {
		vcard->setGivenName(L_C_TO_STRING(first_name));
		return 0;
	}
	return -1;
}

void linphone_friend_set_native_uri(LinphoneFriend *lf, const char *native_uri) {
	if (!lf) return;
	Friend::toCpp(lf)->setNativeUri(L_C_TO_STRING(native_uri));
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
		ms_warning("No default friend list yet, creating it now");
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
	return Friend::createCObject(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr);
}

LinphoneFriend *linphone_core_create_friend_with_address(LinphoneCore *lc, const char *address) {
	return Friend::createCObject(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr, address);
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
	return Friend::createCObject(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr, Vcard::getSharedFromThis(vcard));
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
		} else if (linphone_core_get_default_account(lc) != NULL) {
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

const char *linphone_core_get_friends_database_path(BCTBX_UNUSED(LinphoneCore *lc)) {
	lError() << "Do not use `linphone_core_get_friends_database_path`. Not necessary.";
	return "";
}

void linphone_core_set_friends_database_path(LinphoneCore *lc, const char *path) {
	if (!linphone_core_conference_server_enabled(lc)) {
		if (lc->friends_db_file) {
			ms_free(lc->friends_db_file);
			lc->friends_db_file = NULL;
		}
		if (path) {
			ms_message("Using [%s] file for friends database", path);
			lc->friends_db_file = ms_strdup(path);
		}

		auto &mainDb = L_GET_PRIVATE(lc->cppPtr)->mainDb;
		if (mainDb && mainDb->isInitialized()) {
			if (mainDb->import(LinphonePrivate::MainDb::Sqlite3, path))
				linphone_core_friends_storage_resync_friends_lists(lc);
		} else {
			ms_warning(
			    "Database has not been initialized, therefore it is not possible to import friends database at path %s",
			    path);
		}
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

int linphone_core_friends_storage_resync_friends_lists(LinphoneCore *lc) {
	auto &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;

	// First remove all the orphan friends from the DB (friends that are not in a friend list)
	mainDb->deleteOrphanFriends();

	std::list<std::shared_ptr<FriendList>> friendLists = mainDb->getFriendLists();
	if (!friendLists.empty()) {
		lc->friends_lists =
		    bctbx_list_free_with_data(lc->friends_lists, (bctbx_list_free_func)linphone_friend_list_unref);
		lc->cppPtr->clearFriendLists();

		for (auto &friendList : friendLists) {
			linphone_core_add_friend_list(lc, friendList->toC());
		}
	}

	int fetchedFromDb = (int)friendLists.size();
	lInfo() << "Fetched [" << fetchedFromDb << "] friend lists from DB";
	return fetchedFromDb;
}

bctbx_list_t *linphone_core_fetch_friends_from_db(LinphoneCore *lc, LinphoneFriendList *list) {
	auto &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	std::list<std::shared_ptr<Friend>> friends = mainDb->getFriends(FriendList::getSharedFromThis(list));
	bctbx_list_t *result = nullptr;
	for (const auto &f : friends) {
		result = bctbx_list_append(result, linphone_friend_ref(f->toC()));
	}
	return result;
}

bctbx_list_t *linphone_core_fetch_friends_lists_from_db(LinphoneCore *lc) {
	auto &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	std::list<std::shared_ptr<FriendList>> friendLists = mainDb->getFriendLists();
	bctbx_list_t *result = nullptr;
	for (const auto &l : friendLists) {
		result = bctbx_list_append(result, linphone_friend_list_ref(l->toC()));
	}
	return result;
}
