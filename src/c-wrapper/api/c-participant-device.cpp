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
#include "private.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

LinphoneParticipantDevice *linphone_participant_device_ref (LinphoneParticipantDevice *participant_device) {
	belle_sip_object_ref(participant_device);
	return participant_device;
}

void linphone_participant_device_unref (LinphoneParticipantDevice *participant_device) {
	belle_sip_object_unref(participant_device);
}

void *linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getUserData();
}

void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *ud) {
	ParticipantDevice::toCpp(participant_device)->setUserData(ud);
}

const LinphoneAddress *linphone_participant_device_get_address(const LinphoneParticipantDevice *participant_device) {
	const LinphonePrivate::Address & addr = ParticipantDevice::toCpp(participant_device)->getAddress().asAddress();
	return L_GET_C_BACK_PTR(&addr);
}

LinphoneChatRoomSecurityLevel linphone_participant_device_get_security_level (const LinphoneParticipantDevice *participant_device) {
	return (LinphoneChatRoomSecurityLevel)(ParticipantDevice::toCpp(participant_device)->getSecurityLevel());
}

LinphoneConferenceLayout linphone_participant_device_get_layout (const LinphoneParticipantDevice *participant_device) {
	return (LinphoneConferenceLayout)(LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getLayout());
}

const char *linphone_participant_device_get_name (const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getName().c_str();
}

bool_t linphone_participant_device_is_in_conference (const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->isInConference();
}

time_t linphone_participant_device_get_time_of_joining (const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getTimeOfJoining();
}

LinphoneMediaDirection linphone_participant_device_get_stream_capability (const LinphoneParticipantDevice *participant_device, const LinphoneStreamType stream_type) {
	return ParticipantDevice::toCpp(participant_device)->getStreamCapability(stream_type);
}

bool_t linphone_participant_device_get_stream_availability (const LinphoneParticipantDevice *participant_device, const LinphoneStreamType stream_type) {
	return ParticipantDevice::toCpp(participant_device)->getStreamAvailability(stream_type);
}

uint32_t linphone_participant_device_get_ssrc(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getSsrc();
}

bool_t linphone_participant_device_get_is_muted (const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getIsMuted();
}

void linphone_participant_device_add_callbacks (LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDevice::toCpp(participant_device)->addCallbacks(ParticipantDeviceCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_participant_device_remove_callbacks (LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDevice::toCpp(participant_device)->removeCallbacks(ParticipantDeviceCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneParticipantDeviceCbs *linphone_participant_device_get_current_callbacks (const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getCurrentCallbacks()->toC();
}

void linphone_participant_device_set_current_callbacks(LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDevice::toCpp(participant_device)->setCurrentCallbacks(ParticipantDeviceCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *linphone_participant_device_get_callbacks_list(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getCCallbacksList();
}

void _linphone_participant_device_notify_is_speaking_changed(LinphoneParticipantDevice *participant_device, bool_t is_speaking) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device), linphone_participant_device_cbs_get_is_speaking_changed, is_speaking);
}

void _linphone_participant_device_notify_is_muted(LinphoneParticipantDevice *participant_device, bool_t is_muted) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device), linphone_participant_device_cbs_get_is_muted, is_muted);
	ParticipantDevice::toCpp(participant_device)->setIsMuted(is_muted);
}

void linphone_participant_device_set_native_video_window_id(const LinphoneParticipantDevice *participant_device, void* window_id){
#ifdef __ANDROID__
	shared_ptr<LinphonePrivate::Core> core = LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getCore();
	if (core) {
		LinphoneCore *lc = core->getCCore();
		getPlatformHelpers(lc)->setParticipantDeviceVideoWindow(participant_device, window_id);
	}
#else
	LinphonePrivate::ParticipantDevice::toCpp(participant_device)->setWindowId(window_id);
#endif
}

void * linphone_participant_device_get_native_video_window_id(const LinphoneParticipantDevice *participant_device){
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getWindowId();
}

void * linphone_participant_device_create_native_video_window_id(const LinphoneParticipantDevice *participant_device){
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->createWindowId();
}

void _linphone_participant_device_notify_conference_left(LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(ParticipantDevice, ParticipantDevice::toCpp(participant_device), linphone_participant_device_cbs_get_conference_left);
}

void _linphone_participant_device_notify_conference_joined(LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(ParticipantDevice, ParticipantDevice::toCpp(participant_device), linphone_participant_device_cbs_get_conference_joined);
}

void _linphone_participant_device_notify_stream_capability_changed(LinphoneParticipantDevice *participant_device, LinphoneMediaDirection direction, const LinphoneStreamType stream_type) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device), linphone_participant_device_cbs_get_stream_capability_changed, direction, stream_type);
}

void _linphone_participant_device_notify_stream_availability_changed(LinphoneParticipantDevice *participant_device, bool_t available, const LinphoneStreamType stream_type) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device), linphone_participant_device_cbs_get_stream_availability_changed, available, stream_type);
}
