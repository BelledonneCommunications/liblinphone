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

#include "linphone/api/c-participant-device.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/participant-device.h"

// =============================================================================

using namespace std;

LinphoneParticipantDevice *linphone_participant_device_ref (LinphoneParticipantDevice *participant_device) {
	belle_sip_object_ref(participant_device);
	return participant_device;
}

void linphone_participant_device_unref (LinphoneParticipantDevice *participant_device) {
	belle_sip_object_unref(participant_device);
}

void *linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getUserData();
}

void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *ud) {
	LinphonePrivate::ParticipantDevice::toCpp(participant_device)->setUserData(ud);
}

const LinphoneAddress *linphone_participant_device_get_address(const LinphoneParticipantDevice *participant_device) {
	const LinphonePrivate::Address & addr = LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getAddress().asAddress();
	return L_GET_C_BACK_PTR(&addr);
}

LinphoneChatRoomSecurityLevel linphone_participant_device_get_security_level (const LinphoneParticipantDevice *participant_device) {
	return (LinphoneChatRoomSecurityLevel)(LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSecurityLevel());
}

const char *linphone_participant_device_get_name (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getName().c_str();
}

bool_t linphone_participant_device_is_in_conference (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->isInConference();
}

time_t linphone_participant_device_get_time_of_joining (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getTimeOfJoining();
}

LinphoneMediaDirection linphone_participant_device_get_audio_direction (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getAudioDirection();
}

LinphoneMediaDirection linphone_participant_device_get_video_direction (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getVideoDirection();
}

LinphoneMediaDirection linphone_participant_device_get_text_direction (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getTextDirection();
}
