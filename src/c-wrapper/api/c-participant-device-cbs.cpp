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

#include "linphone/api/c-participant-device-cbs.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/participant-device.h"

using namespace LinphonePrivate;
// =============================================================================

LinphoneParticipantDeviceCbs *linphone_participant_device_cbs_new(void) {
	return ParticipantDeviceCbs::createCObject();
}

LinphoneParticipantDeviceCbs *linphone_participant_device_cbs_ref(LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDeviceCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_participant_device_cbs_unref(LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDeviceCbs::toCpp(cbs)->unref();
}

void *linphone_participant_device_cbs_get_user_data(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getUserData();
}

void linphone_participant_device_cbs_set_user_data(LinphoneParticipantDeviceCbs *cbs, void *ud) {
	ParticipantDeviceCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneParticipantDeviceCbsIsSpeakingChangedCb
linphone_participant_device_cbs_get_is_speaking_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getIsSpeakingChanged();
}

void linphone_participant_device_cbs_set_is_speaking_changed(LinphoneParticipantDeviceCbs *cbs,
                                                             LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setIsSpeakingChanged(cb);
}

LinphoneParticipantDeviceCbsIsMutedCb
linphone_participant_device_cbs_get_is_muted(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getIsMuted();
}

void linphone_participant_device_cbs_set_is_muted(LinphoneParticipantDeviceCbs *cbs,
                                                  LinphoneParticipantDeviceCbsIsMutedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setIsMuted(cb);
}

LinphoneParticipantDeviceCbsScreenSharingChangedCb
linphone_participant_device_cbs_get_screen_sharing_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getScreenSharingChanged();
}

void linphone_participant_device_cbs_set_screen_sharing_changed(LinphoneParticipantDeviceCbs *cbs,
                                                           LinphoneParticipantDeviceCbsScreenSharingChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setScreenSharingChanged(cb);
}

LinphoneParticipantDeviceCbsStateChangedCb
linphone_participant_device_cbs_get_state_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getStateChanged();
}

void linphone_participant_device_cbs_set_state_changed(LinphoneParticipantDeviceCbs *cbs,
                                                       LinphoneParticipantDeviceCbsStateChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setStateChanged(cb);
}

void linphone_participant_device_cbs_set_stream_availability_changed(
    LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setStreamAvailabilityChanged(cb);
}

LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb
linphone_participant_device_cbs_get_stream_availability_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getStreamAvailabilityChanged();
}

void linphone_participant_device_cbs_set_thumbnail_stream_availability_changed(
    LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsThumbnailStreamAvailabilityChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setThumbnailStreamAvailabilityChanged(cb);
}

LinphoneParticipantDeviceCbsThumbnailStreamAvailabilityChangedCb
linphone_participant_device_cbs_get_thumbnail_stream_availability_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getThumbnailStreamAvailabilityChanged();
}

void linphone_participant_device_cbs_set_stream_capability_changed(
    LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsStreamCapabilityChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setStreamCapabilityChanged(cb);
}

LinphoneParticipantDeviceCbsStreamCapabilityChangedCb
linphone_participant_device_cbs_get_stream_capability_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getStreamCapabilityChanged();
}

void linphone_participant_device_cbs_set_thumbnail_stream_capability_changed(
    LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsThumbnailStreamCapabilityChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setThumbnailStreamCapabilityChanged(cb);
}

LinphoneParticipantDeviceCbsThumbnailStreamCapabilityChangedCb
linphone_participant_device_cbs_get_thumbnail_stream_capability_changed(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getThumbnailStreamCapabilityChanged();
}

LinphoneParticipantDeviceCbsVideoDisplayErrorOccurredCb
linphone_participant_device_cbs_get_video_display_error_occurred(const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getVideoDisplayErrorOccurred();
}

void linphone_participant_device_cbs_set_video_display_error_occurred(
    LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsVideoDisplayErrorOccurredCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setVideoDisplayErrorOccurred(cb);
}
