/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "linphone/api/c-participant.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/participant.h"
#include "conference/participant-device.h"

// =============================================================================

using namespace std;

LinphoneParticipant *linphone_participant_ref (LinphoneParticipant *participant) {
	belle_sip_object_ref(participant);
	return participant;
}

void linphone_participant_unref (LinphoneParticipant *participant) {
	belle_sip_object_unref(participant);
}

void *linphone_participant_get_user_data(const LinphoneParticipant *participant) {
//	return L_GET_USER_DATA_FROM_C_OBJECT(participant);
	return nullptr;
}

void linphone_participant_set_user_data(LinphoneParticipant *participant, void *ud) {
//	L_SET_USER_DATA_FROM_C_OBJECT(participant, ud);
}

const LinphoneAddress *linphone_participant_get_address (const LinphoneParticipant *participant) {
	LinphonePrivate::Address addr(LinphonePrivate::Participant::toCpp(participant)->getAddress());
	return linphone_address_new(addr.asString().c_str());
}

bool_t linphone_participant_is_admin (const LinphoneParticipant *participant) {
	return LinphonePrivate::Participant::toCpp(participant)->isAdmin();
}

LinphoneChatRoomSecurityLevel linphone_participant_get_security_level (const LinphoneParticipant *participant) {
	return (LinphoneChatRoomSecurityLevel)LinphonePrivate::Participant::toCpp(participant)->getSecurityLevel();
}

bctbx_list_t *linphone_participant_get_devices (const LinphoneParticipant *participant) {
	return LinphonePrivate::ParticipantDevice::getCListFromCppList(LinphonePrivate::Participant::toCpp(participant)->getDevices());
}

LinphoneParticipantDevice *linphone_participant_find_device (const LinphoneParticipant *participant, const LinphoneAddress *address) {
	char *addrStr = linphone_address_as_string(address);
	LinphonePrivate::Address deviceAddress(addrStr);
	bctbx_free(addrStr);
	return linphone_participant_device_ref (LinphonePrivate::Participant::toCpp(participant)->findDevice(deviceAddress)->toC());
}
