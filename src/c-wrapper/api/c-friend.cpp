/*
 * c-friend.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "friend/friend.h"
#include "c-wrapper/c-wrapper.h"

#include "core/core.h"
#include "private_structs.h"
#include "belle-sip/object.h"
#include "sal/op.h"
#include "sal/presence-op.h"
#include "mediastreamer2/mscommon.h"
#include "linphone/linphonefriend.h"
#include "bctoolbox/list.h"
#include "bctoolbox/map.h"

using namespace std;

static void _linphone_friend_constructor(LinphoneFriend *f);
static void _linphone_friend_destructor(LinphoneFriend *f);

L_DECLARE_C_OBJECT_IMPL_WITH_XTORS(Friend,
	_linphone_friend_constructor, _linphone_friend_destructor,
	void *user_data;
	LinphoneAddress *uri;
	MSList *insubs; /*list of SalOp. There can be multiple instances of a same Friend that subscribe to our presence*/
	LinphonePrivate::SalPresenceOp *outsub;
	LinphoneSubscribePolicy pol;
	MSList *presence_models; /* list of LinphoneFriendPresence. It associates SIP URIs and phone numbers with their respective presence models. */
	MSList *phone_number_sip_uri_map; /* list of LinphoneFriendPhoneNumberSipUri. It associates phone numbers with their corresponding SIP URIs. */
	BuddyInfo *info;
	char *refkey;
	bool_t subscribe;
	bool_t subscribe_active;
	bool_t inc_subscribe_pending;
	bool_t commit;
	bool_t initial_subscribes_sent; /*used to know if initial subscribe message was sent or not*/
	bool_t presence_received;
	LinphoneVcard *vcard;
	unsigned int storage_id;
	LinphoneFriendList *friend_list;
	LinphoneSubscriptionState out_sub_state;
)

void linphone_friend_init(LinphoneFriend *lf) {
	lf->pol = LinphoneSPAccept;
	lf->subscribe = TRUE;
	lf->vcard = nullptr;
	lf->storage_id = 0;
}

static void release_sal_op(LinphonePrivate::SalOp *op) {
	op->release();
}

static void _linphone_friend_release_ops(LinphoneFriend *lf){
	lf->insubs = bctbx_list_free_with_data(lf->insubs, (bctbx_list_free_func) release_sal_op);
	if (lf->outsub){
		lf->outsub->release();
		lf->outsub=NULL;
	}
}

static void _linphone_friend_destroy(LinphoneFriend *lf){
	_linphone_friend_release_ops(lf);
	//if (lf->presence_models) bctbx_list_free_with_data(lf->presence_models, (bctbx_list_free_func)free_friend_presence);
	//if (lf->phone_number_sip_uri_map) bctbx_list_free_with_data(lf->phone_number_sip_uri_map, (bctbx_list_free_func)free_phone_number_sip_uri);
	if (lf->uri!=NULL) linphone_address_unref(lf->uri);
	if (lf->info!=NULL) buddy_info_free(lf->info);
	if (lf->vcard != NULL) linphone_vcard_unref(lf->vcard);
	if (lf->refkey != NULL) ms_free(lf->refkey);
}

static void _linphone_friend_constructor(LinphoneFriend *f) {}
static void _linphone_friend_destructor(LinphoneFriend *f) {
	_linphone_friend_destroy(f);
}

/* DEPRECATED */
void linphone_friend_destroy(LinphoneFriend *lf) {
	linphone_friend_unref(lf);
}

LinphoneStatus linphone_friend_set_address(LinphoneFriend *lf, const LinphoneAddress *addr) {
	LinphoneAddress *fr = linphone_address_clone(addr);
	char *address;
	const LinphoneAddress *mAddr = linphone_friend_get_address(lf);
	if(mAddr && lf->friend_list) {
		char *mainAddress = linphone_address_as_string_uri_only(mAddr);
		bctbx_iterator_t *it = bctbx_map_cchar_find_key(lf->friend_list->friends_map_uri, mainAddress);
		if (!bctbx_iterator_cchar_equals(it, bctbx_map_cchar_end(lf->friend_list->friends_map_uri))){
			linphone_friend_unref((LinphoneFriend*)bctbx_pair_cchar_get_second(bctbx_iterator_cchar_get_pair(it)));
			bctbx_map_cchar_erase(lf->friend_list->friends_map_uri, it);
		}
		bctbx_iterator_cchar_delete(it);
	}
	linphone_address_clean(fr);
	address = linphone_address_as_string_uri_only(fr);
	if(lf->friend_list) {
		bctbx_pair_t *pair = (bctbx_pair_t*) bctbx_pair_cchar_new(address, linphone_friend_ref(lf));
		bctbx_map_cchar_insert_and_delete(lf->friend_list->friends_map_uri, pair);
	}

	if (linphone_core_vcard_supported()) {
		if (!lf->vcard) {
			const char *dpname = linphone_address_get_display_name(fr) ? linphone_address_get_display_name(fr) : linphone_address_get_username(fr);
			linphone_friend_create_vcard(lf, dpname);
		}
		linphone_vcard_edit_main_sip_address(lf->vcard, address);
		linphone_address_unref(fr);
	} else {
		if (lf->uri != NULL) linphone_address_unref(lf->uri);
		lf->uri = fr;
	}

	ms_free(address);
	return 0;
}

const bctbx_list_t* linphone_friend_get_addresses(const LinphoneFriend *lf) {
	if (!lf) return NULL;

	if (linphone_core_vcard_supported()) {
		const bctbx_list_t * addresses = linphone_vcard_get_sip_addresses(lf->vcard);
		return addresses;
	} else {
		bctbx_list_t *addresses = NULL;
		return lf->uri ? bctbx_list_append(addresses, lf->uri) : NULL;
	}
}

void linphone_friend_add_address(LinphoneFriend *lf, const LinphoneAddress *addr) {
	LinphoneAddress *fr;
	char *uri;
	if (!lf || !addr) return;

	fr = linphone_address_clone(addr);
	linphone_address_clean(fr);
	uri = linphone_address_as_string_uri_only(fr);
	if(lf->friend_list) {
		bctbx_pair_t *pair = (bctbx_pair_t*) bctbx_pair_cchar_new(uri, linphone_friend_ref(lf));
		bctbx_map_cchar_insert_and_delete(lf->friend_list->friends_map_uri, pair);
	}

	if (linphone_core_vcard_supported()) {
		if (lf->vcard) {
			linphone_vcard_add_sip_address(lf->vcard, uri);
			linphone_address_unref(fr);
		}
	} else {
		if (lf->uri == NULL) lf->uri = fr;
		else linphone_address_unref(fr);
	}
	ms_free(uri);
}

void linphone_friend_remove_address(LinphoneFriend *lf, const LinphoneAddress *addr) {
	char *address ;
	if (!lf || !addr || !lf->vcard) return;

	address = linphone_address_as_string_uri_only(addr);
	if(lf->friend_list) {
		bctbx_iterator_t *it = bctbx_map_cchar_find_key(lf->friend_list->friends_map_uri, address);
		if (!bctbx_iterator_cchar_equals(it, bctbx_map_cchar_end(lf->friend_list->friends_map_uri))){
			linphone_friend_unref((LinphoneFriend*)bctbx_pair_cchar_get_second(bctbx_iterator_cchar_get_pair(it)));
			bctbx_map_cchar_erase(lf->friend_list->friends_map_uri, it);
		}
		bctbx_iterator_cchar_delete(it);
	}

	if (linphone_core_vcard_supported()) {
		linphone_vcard_remove_sip_address(lf->vcard, address);
	}
	ms_free(address);
}

void linphone_friend_add_phone_number(LinphoneFriend *lf, const char *phone) {
	if (!lf || !phone) return;

	if(lf->friend_list) {
		const char *uri = linphone_friend_phone_number_to_sip_uri(lf, phone);
		if(uri) {
			bctbx_pair_t *pair = (bctbx_pair_t*) bctbx_pair_cchar_new(uri, linphone_friend_ref(lf));
			bctbx_map_cchar_insert_and_delete(lf->friend_list->friends_map_uri, pair);
		}
	}

	if (linphone_core_vcard_supported()) {
		if (!lf->vcard) {
			linphone_friend_create_vcard(lf, phone);
		}
		linphone_vcard_add_phone_number(lf->vcard, phone);
	}
}

bctbx_list_t* linphone_friend_get_phone_numbers(LinphoneFriend *lf) {
	if (!lf || !lf->vcard) return NULL;

	if (linphone_core_vcard_supported()) {
		return linphone_vcard_get_phone_numbers(lf->vcard);
	}
	return NULL;
}

void linphone_friend_remove_phone_number(LinphoneFriend *lf, const char *phone) {
	if (!lf || !phone || !lf->vcard) return;

	if(lf->friend_list) {
		const char *uri = linphone_friend_phone_number_to_sip_uri(lf, phone);
		if(uri) {
			bctbx_iterator_t *it = bctbx_map_cchar_find_key(lf->friend_list->friends_map_uri, uri);
			if (!bctbx_iterator_cchar_equals(it, bctbx_map_cchar_end(lf->friend_list->friends_map_uri))){
				linphone_friend_unref((LinphoneFriend*)bctbx_pair_cchar_get_second(bctbx_iterator_cchar_get_pair(it)));
				bctbx_map_cchar_erase(lf->friend_list->friends_map_uri, it);
			}
			bctbx_iterator_cchar_delete(it);
		}
	}

	if (linphone_core_vcard_supported()) {
		linphone_vcard_remove_phone_number(lf->vcard, phone);
	}
}

LinphoneStatus linphone_friend_set_name(LinphoneFriend *lf, const char *name){
	if (linphone_core_vcard_supported()) {
		if (!lf->vcard) linphone_friend_create_vcard(lf, name);
		linphone_vcard_set_full_name(lf->vcard, name);
	} else {
		if (!lf->uri) {
			ms_warning("linphone_friend_set_address() must be called before linphone_friend_set_name() to be able to set display name.");
			return -1;
		}
		linphone_address_set_display_name(lf->uri, name);
	}
	return 0;
}

const char * linphone_friend_get_name(const LinphoneFriend *lf) {
	if (!lf) return NULL;

	if (linphone_core_vcard_supported()) {
		if (lf->vcard) return linphone_vcard_get_full_name(lf->vcard);
	} else if (lf->uri) {
		return linphone_address_get_display_name(lf->uri);
	}
	return NULL;
}

LinphoneStatus linphone_friend_enable_subscribes(LinphoneFriend *fr, bool_t val){
	fr->subscribe=val;
	return 0;
}

LinphoneStatus linphone_friend_set_inc_subscribe_policy(LinphoneFriend *fr, LinphoneSubscribePolicy pol) {
	fr->pol=pol;
	return 0;
}

LinphoneSubscribePolicy linphone_friend_get_inc_subscribe_policy(const LinphoneFriend *lf){
	return lf->pol;
}

void linphone_friend_edit(LinphoneFriend *fr) {
	if (fr && linphone_core_vcard_supported() && fr->vcard) {
		linphone_vcard_compute_md5_hash(fr->vcard);
	}
}

void linphone_friend_done(LinphoneFriend *fr) {
	ms_return_if_fail(fr);
	if (!fr->lc) return;

	if (fr && linphone_core_vcard_supported() && fr->vcard) {
		if (linphone_vcard_compare_md5_hash(fr->vcard) != 0) {
			ms_debug("vCard's md5 has changed, mark friend as dirty and clear sip addresses list cache");
			linphone_vcard_clean_cache(fr->vcard);
			if (fr->friend_list) {
				fr->friend_list->dirty_friends_to_update = bctbx_list_append(fr->friend_list->dirty_friends_to_update, linphone_friend_ref(fr));
			}
		}
	}
	linphone_friend_apply(fr, fr->lc);
	linphone_friend_save(fr, fr->lc);
}

LinphoneOnlineStatus linphone_friend_get_status(const LinphoneFriend *lf){
	const LinphonePresenceModel *presence = linphone_friend_get_presence_model(lf);
	LinphoneOnlineStatus online_status = LinphoneStatusOffline;
	LinphonePresenceBasicStatus basic_status = LinphonePresenceBasicStatusClosed;
	LinphonePresenceActivity *activity = NULL;
	const char *description = NULL;
	unsigned int nb_activities = 0;

	if (presence != NULL) {
		basic_status = linphone_presence_model_get_basic_status(presence);
		nb_activities = linphone_presence_model_get_nb_activities(presence);
		online_status = (basic_status == LinphonePresenceBasicStatusOpen) ? LinphoneStatusOnline : LinphoneStatusOffline;
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
					if (description && strcmp(description, "Do not disturb") == 0) { // See linphonecore.c linphone_core_set_presence_info() method
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

LinphoneSubscriptionState linphone_friend_get_subscription_state(const LinphoneFriend *lf) {
	return lf->out_sub_state;
}

const LinphonePresenceModel * linphone_friend_get_presence_model(const LinphoneFriend *lf) {
	const LinphonePresenceModel *presence = NULL;
	LinphoneFriend* const_lf = (LinphoneFriend*)lf;
	const bctbx_list_t* addrs = linphone_friend_get_addresses(const_lf);
	bctbx_list_t* phones = NULL;
	bctbx_list_t *it;

	for (it = (bctbx_list_t *)addrs; it!= NULL; it = it->next) {
		LinphoneAddress *addr = (LinphoneAddress*)it->data;
		char *uri = linphone_address_as_string_uri_only(addr);
		presence = linphone_friend_get_presence_model_for_uri_or_tel(const_lf, uri);
		ms_free(uri);
		if (presence) break;
	}
	if (presence) return presence;

	phones = linphone_friend_get_phone_numbers(const_lf);
	for (it = phones; it!= NULL; it = it->next) {
		presence = linphone_friend_get_presence_model_for_uri_or_tel(const_lf, reinterpret_cast<const char *>(it->data));
		if (presence) break;
	}
	bctbx_list_free(phones);
	return presence;
}

LinphoneConsolidatedPresence linphone_friend_get_consolidated_presence(const LinphoneFriend *lf) {
	const LinphonePresenceModel *model = linphone_friend_get_presence_model(lf);
	if (!model) return LinphoneConsolidatedPresenceOffline;
	return linphone_presence_model_get_consolidated_presence(model);
}

const LinphonePresenceModel * linphone_friend_get_presence_model_for_uri_or_tel(const LinphoneFriend *lf, const char *uri_or_tel) {
	//TODO
	//LinphoneFriendPresence *lfp = find_presence_model_for_uri_or_tel(lf, uri_or_tel);
	//if (lfp) return lfp->presence;
	return NULL;
}

void linphone_friend_set_presence_model(LinphoneFriend *lf, LinphonePresenceModel *presence) {
	const LinphoneAddress *addr = linphone_friend_get_address(lf);
	if (addr) {
		char *uri = linphone_address_as_string_uri_only(addr);
		linphone_friend_set_presence_model_for_uri_or_tel(lf, uri, presence);
		ms_free(uri);
	}
}

void linphone_friend_set_presence_model_for_uri_or_tel(LinphoneFriend *lf, const char *uri_or_tel, LinphonePresenceModel *presence) {
	//TODO
	/*LinphoneFriendPresence *lfp = find_presence_model_for_uri_or_tel(lf, uri_or_tel);
	if (lfp) {
		if (lfp->presence) linphone_presence_model_unref(lfp->presence);
		lfp->presence = presence;
	} else {
		add_presence_model_for_uri_or_tel(lf, uri_or_tel, presence);
	}*/
}

bool_t linphone_friend_is_presence_received(const LinphoneFriend *lf) {
	return lf->presence_received;
}

void linphone_friend_set_user_data(LinphoneFriend *lf, void *data){
	lf->user_data=data;
}

void* linphone_friend_get_user_data(const LinphoneFriend *lf){
	return lf->user_data;
}

BuddyInfo * linphone_friend_get_info(const LinphoneFriend *lf){
	return lf->info;
}

void linphone_friend_set_ref_key(LinphoneFriend *lf, const char *key){
	if (lf->refkey != NULL) {
		ms_free(lf->refkey);
		lf->refkey = NULL;
	}
	if (key) {
		lf->refkey = ms_strdup(key);
	}
	if (lf->lc) {
		linphone_friend_save(lf, lf->lc);
	}
}

const char *linphone_friend_get_ref_key(const LinphoneFriend *lf){
	return lf->refkey;
}

bool_t linphone_friend_in_list(const LinphoneFriend *lf) {
	return lf->friend_list != NULL;
}

LinphoneFriend *linphone_friend_ref(LinphoneFriend *lf) {
	belle_sip_object_ref(lf);
	return lf;
}

void linphone_friend_unref(LinphoneFriend *lf) {
	belle_sip_object_unref(lf);
}

LinphoneCore *linphone_friend_get_core(const LinphoneFriend *fr){
	return fr->lc;
}

LinphoneVcard* linphone_friend_get_vcard(LinphoneFriend *fr) {
	if (fr && linphone_core_vcard_supported()) return fr->vcard;
	return NULL;
}

void linphone_friend_set_vcard(LinphoneFriend *fr, LinphoneVcard *vcard) {
	if (!fr || !linphone_core_vcard_supported()) return;

	if (vcard) linphone_vcard_ref(vcard);

	if (fr->vcard) linphone_vcard_unref(fr->vcard);
	fr->vcard = vcard;
	linphone_friend_save(fr, fr->lc);
}

bool_t linphone_friend_create_vcard(LinphoneFriend *fr, const char *name) {
	LinphoneVcard *vcard = NULL;
	LinphoneCore *lc = NULL;
	bool_t skip = FALSE;

	if (!fr || !name) {
		ms_error("Friend or name is null");
		return FALSE;
	}
	if (!linphone_core_vcard_supported()) {
		ms_warning("VCard support is not builtin");
		return FALSE;
	}
	if (fr->vcard) {
		ms_error("Friend already has a VCard");
		return FALSE;
	}

	vcard = linphone_factory_create_vcard(linphone_factory_get());

	lc = fr->lc;
	if (!lc && fr->friend_list) {
		lc = fr->friend_list->lc;
	}
	if (lc) {
		skip = !lp_config_get_int(linphone_core_get_config(L_GET_CPP_PTR_FROM_C_OBJECT(fr)->getCore()->getCCore()), "misc", "store_friends", 1);
		linphone_vcard_set_skip_validation(vcard, skip);
	}
	linphone_vcard_set_full_name(vcard, name);
	linphone_friend_set_vcard(fr, vcard);
	linphone_vcard_unref(vcard);
	return TRUE;
}

LinphoneFriend *linphone_friend_new_from_vcard(LinphoneVcard *vcard) {
	LinphoneFriend *fr;

	if (!linphone_core_vcard_supported()) {
		ms_error("VCard support is not builtin");
		return NULL;
	}
	if (vcard == NULL) {
		ms_error("Cannot create friend from null vcard");
		return NULL;
	}

	fr = L_INIT(Friend);
	// Currently presence takes too much time when dealing with hundreds of friends, so I disabled it for now
	fr->pol = LinphoneSPDeny;
	fr->subscribe = FALSE;
	linphone_friend_set_vcard(fr, vcard);
	return fr;
}

void linphone_friend_save(LinphoneFriend *fr, LinphoneCore *lc) {
	if (!lc) return;
	#ifdef SQLITE_STORAGE_ENABLED
	if (linphone_core_get_friends_database_path(lc)) {
		linphone_core_store_friend_in_db(lc, fr);
	} else {
		linphone_core_write_friends_config(lc);
	}
	#else
	linphone_core_write_friends_config(lc);
	#endif
}

