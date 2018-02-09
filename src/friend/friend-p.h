/*
 * friend-p.h
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

#ifndef _L_FRIEND_P_H_
#define _L_FRIEND_P_H_

#include "friend.h"
#include "object/object-p.h"

#include <bctoolbox/list.h>
#include <bctoolbox/map.h>

#include "address/address.h"
#include "sal/presence-op.h"
#include "linphone/types.h"
#include "linphone/sipsetup.h"


LINPHONE_BEGIN_NAMESPACE

class FriendPrivate : public ObjectPrivate {
public:
	FriendPrivate();
	~FriendPrivate();

private:
	/*Address uri;
	LinphoneVcard *vcard;
	unsigned int storage_id;

	bctbx_list_t *insubs; //list of SalOp. There can be multiple instances of a same Friend that subscribe to our presence
	LinphonePrivate::SalPresenceOp *outsub;
	LinphoneSubscribePolicy pol;
	bctbx_list_t *presence_models; // list of LinphoneFriendPresence. It associates SIP URIs and phone numbers with their respective presence models.
	bctbx_list_t *phone_number_sip_uri_map; // list of LinphoneFriendPhoneNumberSipUri. It associates phone numbers with their corresponding SIP URIs.
	BuddyInfo *info;
	char *refkey;
	bool_t subscribe;
	bool_t subscribe_active;
	bool_t inc_subscribe_pending;
	bool_t commit;
	bool_t initial_subscribes_sent; //used to know if initial subscribe message was sent or not
	bool_t presence_received;
	LinphoneSubscriptionState out_sub_state;

	//REMOVE ? REWORK FriendList
	LinphoneFriendList *friend_list;*/

	L_DECLARE_PUBLIC(Friend);
};

LINPHONE_END_NAMESPACE

#endif //_L_FRIEND_P_H_
