/*
 * c-participant.cpp
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

#include "linphone/api/c-participant.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/participant-p.h"
#include "conference/participant-device.h"

// =============================================================================

using namespace std;

L_DECLARE_C_OBJECT_IMPL(Participant,
	mutable LinphoneAddress *addressCache;
);

LinphoneParticipant *linphone_participant_ref (LinphoneParticipant *participant) {
	belle_sip_object_ref(participant);
	return participant;
}

void linphone_participant_unref (LinphoneParticipant *participant) {
	belle_sip_object_unref(participant);
}

void *linphone_participant_get_user_data(const LinphoneParticipant *participant) {
	return L_GET_USER_DATA_FROM_C_OBJECT(participant);
}

void linphone_participant_set_user_data(LinphoneParticipant *participant, void *ud) {
	L_SET_USER_DATA_FROM_C_OBJECT(participant, ud);
}

const LinphoneAddress *linphone_participant_get_address (const LinphoneParticipant *participant) {
	LinphonePrivate::Address addr(L_GET_CPP_PTR_FROM_C_OBJECT(participant)->getAddress());
	if (participant->addressCache)
		linphone_address_unref(participant->addressCache);
	participant->addressCache = linphone_address_new(addr.asString().c_str());
	return participant->addressCache;
}

bool_t linphone_participant_is_admin (const LinphoneParticipant *participant) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(participant)->isAdmin();
}

LinphoneChatRoomSecurityLevel linphone_participant_get_security_level (const LinphoneParticipant *participant) {
	return (LinphoneChatRoomSecurityLevel)L_GET_CPP_PTR_FROM_C_OBJECT(participant)->getSecurityLevel();
}

bctbx_list_t *linphone_participant_get_devices (const LinphoneParticipant *participant) {
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(L_GET_PRIVATE_FROM_C_OBJECT(participant)->getDevices());
}

LinphoneParticipantDevice *linphone_participant_find_device (const LinphoneParticipant *participant, const LinphoneAddress *address) {
	char *addrStr = linphone_address_as_string(address);
	LinphonePrivate::Address deviceAddress(addrStr);
	bctbx_free(addrStr);
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(participant)->findDevice(deviceAddress));
}
