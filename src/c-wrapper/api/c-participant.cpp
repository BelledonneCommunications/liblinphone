/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "linphone/api/c-participant.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/participant-device.h"
#include "conference/participant.h"

// =============================================================================

using namespace std;

LinphoneParticipant *linphone_participant_ref(LinphoneParticipant *participant) {
	belle_sip_object_ref(participant);
	return participant;
}

void linphone_participant_unref(LinphoneParticipant *participant) {
	belle_sip_object_unref(participant);
}

void *linphone_participant_get_user_data(const LinphoneParticipant *participant) {
	return LinphonePrivate::Participant::toCpp(participant)->getUserData();
}

void linphone_participant_set_user_data(LinphoneParticipant *participant, void *ud) {
	LinphonePrivate::Participant::toCpp(participant)->setUserData(ud);
}

const LinphoneAddress *linphone_participant_get_address(const LinphoneParticipant *participant) {
	const auto &addr = LinphonePrivate::Participant::toCpp(participant)->getAddress();
	return addr->toC();
}

bool_t linphone_participant_is_admin(const LinphoneParticipant *participant) {
	return LinphonePrivate::Participant::toCpp(participant)->isAdmin();
}

bool_t linphone_participant_is_focus(const LinphoneParticipant *participant) {
	return LinphonePrivate::Participant::toCpp(participant)->isFocus();
}

LinphoneChatRoomSecurityLevel linphone_participant_get_security_level(const LinphoneParticipant *participant) {
	return (LinphoneChatRoomSecurityLevel)LinphonePrivate::Participant::toCpp(participant)->getSecurityLevel();
}

bctbx_list_t *linphone_participant_get_devices(const LinphoneParticipant *participant) {
	return LinphonePrivate::ParticipantDevice::getCListFromCppList(
	    LinphonePrivate::Participant::toCpp(participant)->getDevices());
}

LinphoneParticipantDevice *linphone_participant_find_device_2(const LinphoneParticipant *participant,
                                                              const LinphoneCall *call) {
	std::shared_ptr<LinphonePrivate::ParticipantDevice> device =
	    LinphonePrivate::Participant::toCpp(participant)
	        ->findDevice(LinphonePrivate::Call::toCpp(call)->getActiveSession());
	if (device) {
		return device->toC();
	}
	return NULL;
}

LinphoneParticipantDevice *linphone_participant_find_device(const LinphoneParticipant *participant,
                                                            const LinphoneAddress *address) {
	std::shared_ptr<LinphonePrivate::ParticipantDevice> device =
	    LinphonePrivate::Participant::toCpp(participant)
	        ->findDevice(LinphonePrivate::Address::toCpp(address)->getSharedFromThis());
	if (device) {
		return device->toC();
	}
	return NULL;
}

time_t linphone_participant_get_creation_time(const LinphoneParticipant *participant) {
	return LinphonePrivate::Participant::toCpp(participant)->getCreationTime();
}

bool_t linphone_participant_preserve_session(const LinphoneParticipant *participant) {
	return LinphonePrivate::Participant::toCpp(participant)->getPreserveSession();
}

LinphoneParticipantRole linphone_participant_get_role(const LinphoneParticipant *participant) {
	return (LinphoneParticipantRole)LinphonePrivate::Participant::toCpp(participant)->getRole();
}

const char *linphone_participant_role_to_string(LinphoneParticipantRole role) {
	switch (role) {
		case LinphoneParticipantRoleSpeaker:
			return "LinphoneParticipantRoleSpeaker";
		case LinphoneParticipantRoleListener:
			return "LinphoneParticipantRoleListener";
		case LinphoneParticipantRoleUnknown:
			return "LinphoneParticipantRoleUnknown";
	}
	return NULL;
}
