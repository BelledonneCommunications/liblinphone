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

#include "linphone/api/c-participant-device.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "private.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

LinphoneParticipantDevice *linphone_participant_device_ref(LinphoneParticipantDevice *participant_device) {
	belle_sip_object_ref(participant_device);
	return participant_device;
}

void linphone_participant_device_unref(LinphoneParticipantDevice *participant_device) {
	belle_sip_object_unref(participant_device);
}

void *linphone_participant_device_get_user_data(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getUserData();
}

void linphone_participant_device_set_user_data(LinphoneParticipantDevice *participant_device, void *ud) {
	ParticipantDevice::toCpp(participant_device)->setUserData(ud);
}

const LinphoneAddress *linphone_participant_device_get_address(const LinphoneParticipantDevice *participant_device) {
	const auto &addr = ParticipantDevice::toCpp(participant_device)->getAddress();
	return addr->toC();
}

void linphone_participant_device_set_state(LinphoneParticipantDevice *participant_device,
                                           LinphoneParticipantDeviceState state) {
	ParticipantDevice::toCpp(participant_device)->setState(static_cast<ParticipantDevice::State>(state));
}

LinphoneParticipantDeviceState
linphone_participant_device_get_state(const LinphoneParticipantDevice *participant_device) {
	return (LinphoneParticipantDeviceState)(ParticipantDevice::toCpp(participant_device)->getState());
}

LinphoneChatRoomSecurityLevel
linphone_participant_device_get_security_level(const LinphoneParticipantDevice *participant_device) {
	return (LinphoneChatRoomSecurityLevel)(ParticipantDevice::toCpp(participant_device)->getSecurityLevel());
}

const char *linphone_participant_device_get_name(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getName().c_str();
}

bool_t linphone_participant_device_is_in_conference(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->isInConference();
}

LinphoneParticipantDeviceJoiningMethod
linphone_participant_device_get_joining_method(const LinphoneParticipantDevice *participant_device) {
	return (LinphoneParticipantDeviceJoiningMethod)ParticipantDevice::toCpp(participant_device)->getJoiningMethod();
}

LinphoneParticipantDeviceDisconnectionMethod
linphone_participant_device_get_disconnection_method(const LinphoneParticipantDevice *participant_device) {
	return (LinphoneParticipantDeviceDisconnectionMethod)ParticipantDevice::toCpp(participant_device)
	    ->getDisconnectionMethod();
}

const char *linphone_participant_device_get_disconnection_reason(const LinphoneParticipantDevice *participant_device) {
	return L_STRING_TO_C(ParticipantDevice::toCpp(participant_device)->getDisconnectionReason());
}

time_t linphone_participant_device_get_time_of_joining(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getTimeOfJoining();
}

time_t linphone_participant_device_get_time_of_disconnection(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getTimeOfDisconnection();
}

const char *linphone_participant_device_get_stream_label(const LinphoneParticipantDevice *participant_device,
                                                         const LinphoneStreamType stream_type) {
	return L_STRING_TO_C(ParticipantDevice::toCpp(participant_device)->getStreamLabel(stream_type));
}

const char *
linphone_participant_device_get_thumbnail_stream_label(const LinphoneParticipantDevice *participant_device) {
	return L_STRING_TO_C(ParticipantDevice::toCpp(participant_device)->getThumbnailStreamLabel());
}
LinphoneMediaDirection
linphone_participant_device_get_stream_capability(const LinphoneParticipantDevice *participant_device,
                                                  const LinphoneStreamType stream_type) {
	return ParticipantDevice::toCpp(participant_device)->getStreamCapability(stream_type);
}

LinphoneMediaDirection
linphone_participant_device_get_thumbnail_stream_capability(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getThumbnailStreamCapability();
}

bool_t linphone_participant_device_get_stream_availability(const LinphoneParticipantDevice *participant_device,
                                                           const LinphoneStreamType stream_type) {
	return ParticipantDevice::toCpp(participant_device)->getStreamAvailability(stream_type);
}

bool_t
linphone_participant_device_get_thumbnail_stream_availability(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getThumbnailStreamAvailability();
}

uint32_t linphone_participant_device_get_ssrc(const LinphoneParticipantDevice *participant_device,
                                              const LinphoneStreamType stream_type) {
	return ParticipantDevice::toCpp(participant_device)->getSsrc(stream_type);
}

uint32_t linphone_participant_device_get_thumbnail_ssrc(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getThumbnailStreamSsrc();
}

bool_t linphone_participant_device_get_is_speaking(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getIsSpeaking();
}

bool_t linphone_participant_device_get_is_muted(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getIsMuted();
}

bool_t linphone_participant_device_screen_sharing_enabled(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->screenSharingEnabled();
}

LinphoneParticipant *linphone_participant_device_get_participant(const LinphoneParticipantDevice *participant_device) {
	return bellesip::toC(ParticipantDevice::toCpp(participant_device)->getParticipant());
}

LinphoneCore *linphone_participant_device_get_core(const LinphoneParticipantDevice *participant_device) {
	const auto &core = ParticipantDevice::toCpp(participant_device)->getCore();
	return core ? core->getCCore() : nullptr;
}

void linphone_participant_device_add_callbacks(LinphoneParticipantDevice *participant_device,
                                               LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDevice::toCpp(participant_device)->addCallbacks(ParticipantDeviceCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_participant_device_remove_callbacks(LinphoneParticipantDevice *participant_device,
                                                  LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDevice::toCpp(participant_device)
	    ->removeCallbacks(ParticipantDeviceCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneParticipantDeviceCbs *
linphone_participant_device_get_current_callbacks(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getCurrentCallbacks()->toC();
}

void linphone_participant_device_set_current_callbacks(LinphoneParticipantDevice *participant_device,
                                                       LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDevice::toCpp(participant_device)
	    ->setCurrentCallbacks(ParticipantDeviceCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *
linphone_participant_device_get_callbacks_list(const LinphoneParticipantDevice *participant_device) {
	return ParticipantDevice::toCpp(participant_device)->getCCallbacksList();
}

void _linphone_participant_device_notify_is_speaking_changed(LinphoneParticipantDevice *participant_device,
                                                             bool_t is_speaking) {
	ParticipantDevice::toCpp(participant_device)->setIsSpeaking(is_speaking);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_is_speaking_changed, is_speaking);
}

void _linphone_participant_device_notify_is_muted(LinphoneParticipantDevice *participant_device, bool_t is_muted) {
	ParticipantDevice::toCpp(participant_device)->setIsMuted(is_muted);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_is_muted, is_muted);
}

void linphone_participant_device_set_native_video_window_id(LinphoneParticipantDevice *participant_device,
                                                            void *window_id) {
#ifdef __ANDROID__
	shared_ptr<const LinphonePrivate::ParticipantDevice> device =
	    LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getSharedFromThis();
	shared_ptr<LinphonePrivate::Core> core = device->getCore();
	if (core) {
		LinphoneCore *lc = core->getCCore();
		getPlatformHelpers(lc)->setParticipantDeviceVideoWindow(participant_device, window_id);
	}
#else
	LinphonePrivate::ParticipantDevice::toCpp(participant_device)->setWindowId(window_id);
#endif
}

void *linphone_participant_device_get_native_video_window_id(const LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->getWindowId();
}

void *linphone_participant_device_create_native_video_window_id(LinphoneParticipantDevice *participant_device) {
	return LinphonePrivate::ParticipantDevice::toCpp(participant_device)->createWindowId();
}

void _linphone_participant_device_notify_state_changed(LinphoneParticipantDevice *participant_device,
                                                       const LinphoneParticipantDeviceState state) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_state_changed, state);
}

void _linphone_participant_device_notify_screen_sharing_enabled(LinphoneParticipantDevice *participant_device,
                                                                bool_t enabled) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_screen_sharing_changed, enabled);
}

void _linphone_participant_device_notify_stream_capability_changed(LinphoneParticipantDevice *participant_device,
                                                                   LinphoneMediaDirection direction,
                                                                   const LinphoneStreamType stream_type) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_stream_capability_changed, direction,
	                                  stream_type);
}

void _linphone_participant_device_notify_thumbnail_stream_capability_changed(
    LinphoneParticipantDevice *participant_device, LinphoneMediaDirection direction) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_thumbnail_stream_capability_changed,
	                                  direction);
}

void _linphone_participant_device_notify_stream_availability_changed(LinphoneParticipantDevice *participant_device,
                                                                     bool_t available,
                                                                     const LinphoneStreamType stream_type) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_stream_availability_changed, available,
	                                  stream_type);
}

void _linphone_participant_device_notify_thumbnail_stream_availability_changed(
    LinphoneParticipantDevice *participant_device, bool_t available) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_thumbnail_stream_availability_changed,
	                                  available);
}

void _linphone_participant_device_notify_video_display_error_occurred(LinphoneParticipantDevice *participant_device,
                                                                      int error_code) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ParticipantDevice, ParticipantDevice::toCpp(participant_device),
	                                  linphone_participant_device_cbs_get_video_display_error_occurred, error_code);
}
