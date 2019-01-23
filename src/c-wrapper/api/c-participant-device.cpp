/*
 * c-participant-device.cpp
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

#include "linphone/api/c-participant-device.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/participant-device.h"

// =============================================================================

using namespace std;

L_DECLARE_C_OBJECT_IMPL(ParticipantDevice,
	mutable LinphoneAddress *addressCache;
);

LinphoneParticipantDevice *linphone_participant_device_ref (LinphoneParticipantDevice *participant_device) {
	belle_sip_object_ref(participant_device);
	return participant_device;
}

void linphone_participant_device_unref (LinphoneParticipantDevice *participant_device) {
	belle_sip_object_unref(participant_device);
}

void *linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device) {
	return L_GET_USER_DATA_FROM_C_OBJECT(participant_device);
}

void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *ud) {
	L_SET_USER_DATA_FROM_C_OBJECT(participant_device, ud);
}

const LinphoneAddress *linphone_participant_device_get_address(const LinphoneParticipantDevice *participant_device) {
	LinphonePrivate::Address addr(L_GET_CPP_PTR_FROM_C_OBJECT(participant_device)->getAddress());
	if (participant_device->addressCache)
		linphone_address_unref(participant_device->addressCache);
	participant_device->addressCache = linphone_address_new(addr.asString().c_str());
	return participant_device->addressCache;
}

LinphoneChatRoomSecurityLevel linphone_participant_device_get_security_level (const LinphoneParticipantDevice *participant_device) {
	return (LinphoneChatRoomSecurityLevel)L_GET_CPP_PTR_FROM_C_OBJECT(participant_device)->getSecurityLevel();
}

const char *linphone_participant_device_get_name (const LinphoneParticipantDevice *participant_device) {
	return L_STRING_TO_C(L_GET_CPP_PTR_FROM_C_OBJECT(participant_device)->getName());
}
