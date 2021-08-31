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

uint32_t linphone_participant_device_get_ssrc(const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSsrc();
}

void linphone_participant_device_set_native_video_window_id(LinphoneParticipantDevice *participant_device, void* window_id){
#ifdef __ANDROID__
	LinphoneCore *lc = LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getCore()->getCCore();
	getPlatformHelpers(lc)->setParticipantDeviceVideoWindow(participant_device, window_id);
#else
	LinphonePrivate::ParticipantDevice::toCpp(participant_device)->setWindowId(window_id);
#endif
}

void * linphone_participant_device_get_native_video_window_id(const LinphoneParticipantDevice *participant_device){
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getWindowId();
}

LinphoneVideoSize *linphone_participant_device_get_received_video_size(const LinphoneParticipantDevice *participant_device) {
	LinphoneVideoSize *result = linphone_video_size_new();
	MSVideoSize size = LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getReceivedVideoSize();
	result->width = size.width;
	result->height = size.height;
	return result;
}

void linphone_participant_device_add_callbacks (LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs) {
	LinphonePrivate::ParticipantDevice::toCpp(participant_device)->addCallbacks(cbs);
}

void linphone_participant_device_remove_callbacks (LinphoneParticipantDevice *participant_device, LinphoneParticipantDeviceCbs *cbs) {
	LinphonePrivate::ParticipantDevice::toCpp(participant_device)->removeCallbacks(cbs);
}

LinphoneParticipantDeviceCbs *linphone_participant_device_get_current_callbacks (const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getCurrentCbs();
}

const bctbx_list_t *linphone_participant_device_get_callbacks_list(const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getCallbacksList();
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...) \
for (bctbx_list_t *it = LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getCallbacksList(); it; it = bctbx_list_next(it)) { \
LinphonePrivate::ParticipantDevice::toCpp(participant_device)->setCurrentCbs(reinterpret_cast<LinphoneParticipantDeviceCbs *>(bctbx_list_get_data(it))); \
	LinphoneParticipantDeviceCbs ## cbName ## Cb cb = linphone_participant_device_cbs_get_ ## functionName (LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getCurrentCbs()); \
	if (cb) \
		cb(__VA_ARGS__); \
}

void _linphone_participant_device_notify_capture_video_size_changed(LinphoneParticipantDevice *participant_device, LinphoneVideoSize *size) {
	NOTIFY_IF_EXIST(CaptureVideoSizeChanged, capture_video_size_changed, participant_device, size)
}
