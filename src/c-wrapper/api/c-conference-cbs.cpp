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

#include "linphone/api/c-conference-cbs.h"
#include "conference/conference.h"

#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneConferenceCbs *_linphone_conference_cbs_new(void) {
	return ConferenceCbs::createCObject();
}

LinphoneConferenceCbs *linphone_conference_cbs_ref(LinphoneConferenceCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_conference_cbs_unref(LinphoneConferenceCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void *linphone_conference_cbs_get_user_data(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->getUserData();
}

void linphone_conference_cbs_set_user_data(LinphoneConferenceCbs *cbs, void *ud) {
	ConferenceCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneConferenceCbsAllowedParticipantListChangedCb
linphone_conference_cbs_get_allowed_participant_list_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->allowedParticipantListChangedCb;
}

void linphone_conference_cbs_set_allowed_participant_list_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsAllowedParticipantListChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->allowedParticipantListChangedCb = cb;
}

LinphoneConferenceCbsParticipantAddedCb
linphone_conference_cbs_get_participant_added(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantAddedCb;
}

void linphone_conference_cbs_set_participant_added(LinphoneConferenceCbs *cbs,
                                                   LinphoneConferenceCbsParticipantAddedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantAddedCb = cb;
}

LinphoneConferenceCbsParticipantRemovedCb
linphone_conference_cbs_get_participant_removed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantRemovedCb;
}

void linphone_conference_cbs_set_participant_removed(LinphoneConferenceCbs *cbs,
                                                     LinphoneConferenceCbsParticipantRemovedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantRemovedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceAddedCb
linphone_conference_cbs_get_participant_device_added(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceAddedCb;
}

void linphone_conference_cbs_set_participant_device_added(LinphoneConferenceCbs *cbs,
                                                          LinphoneConferenceCbsParticipantDeviceAddedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceAddedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceRemovedCb
linphone_conference_cbs_get_participant_device_removed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceRemovedCb;
}

void linphone_conference_cbs_set_participant_device_removed(LinphoneConferenceCbs *cbs,
                                                            LinphoneConferenceCbsParticipantDeviceRemovedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceRemovedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceJoiningRequestCb
linphone_conference_cbs_get_participant_device_joining_request(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceJoiningRequestCb;
}

void linphone_conference_cbs_set_participant_device_joining_request(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceJoiningRequestCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceJoiningRequestCb = cb;
}

LinphoneConferenceCbsParticipantRoleChangedCb
linphone_conference_cbs_get_participant_role_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantRoleChangedCb;
}

void linphone_conference_cbs_set_participant_role_changed(LinphoneConferenceCbs *cbs,
                                                          LinphoneConferenceCbsParticipantRoleChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantRoleChangedCb = cb;
}

LinphoneConferenceCbsParticipantAdminStatusChangedCb
linphone_conference_cbs_get_participant_admin_status_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantAdminStatusChangedCb;
}

void linphone_conference_cbs_set_participant_admin_status_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAdminStatusChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantAdminStatusChangedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceStateChangedCb
linphone_conference_cbs_get_participant_device_state_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceStateChangedCb;
}

void linphone_conference_cbs_set_participant_device_state_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceStateChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceStateChangedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceMediaAvailabilityChangedCb
linphone_conference_cbs_get_participant_device_media_availability_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceMediaAvailabilityChangedCb;
}

void linphone_conference_cbs_set_participant_device_media_availability_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceMediaAvailabilityChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceMediaAvailabilityChangedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceMediaCapabilityChangedCb
linphone_conference_cbs_get_participant_device_media_capability_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceMediaCapabilityChangedCb;
}

void linphone_conference_cbs_set_participant_device_media_capability_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceMediaCapabilityChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceMediaCapabilityChangedCb = cb;
}

LinphoneConferenceCbsStateChangedCb linphone_conference_cbs_get_state_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->stateChangedCb;
}

void linphone_conference_cbs_set_state_changed(LinphoneConferenceCbs *cbs, LinphoneConferenceCbsStateChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->stateChangedCb = cb;
}

LinphoneConferenceCbsAvailableMediaChangedCb
linphone_conference_cbs_get_available_media_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->availableMediaChangedCb;
}

void linphone_conference_cbs_set_available_media_changed(LinphoneConferenceCbs *cbs,
                                                         LinphoneConferenceCbsAvailableMediaChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->availableMediaChangedCb = cb;
}

LinphoneConferenceCbsSubjectChangedCb linphone_conference_cbs_get_subject_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->subjectChangedCb;
}

void linphone_conference_cbs_set_subject_changed(LinphoneConferenceCbs *cbs, LinphoneConferenceCbsSubjectChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->subjectChangedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceIsSpeakingChangedCb
linphone_conference_cbs_get_participant_device_is_speaking_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceIsSpeakingChangedCb;
}

void linphone_conference_cbs_set_participant_device_is_speaking_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceIsSpeakingChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceIsSpeakingChangedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceIsMutedCb
linphone_conference_cbs_get_participant_device_is_muted(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceIsMutedCb;
}

void linphone_conference_cbs_set_participant_device_is_muted(LinphoneConferenceCbs *cbs,
                                                             LinphoneConferenceCbsParticipantDeviceIsMutedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceIsMutedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceScreenSharingChangedCb
linphone_conference_cbs_get_participant_device_screen_sharing_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceScreenSharingChangedCb;
}

void linphone_conference_cbs_set_participant_device_screen_sharing_changed(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceScreenSharingChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceScreenSharingChangedCb = cb;
}

LinphoneConferenceCbsAudioDeviceChangedCb
linphone_conference_cbs_get_audio_device_changed(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->audioDeviceChangedCb;
}

void linphone_conference_cbs_set_audio_device_changed(LinphoneConferenceCbs *cbs,
                                                      LinphoneConferenceCbsAudioDeviceChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->audioDeviceChangedCb = cb;
}

LinphoneConferenceCbsActiveSpeakerParticipantDeviceCb
linphone_conference_cbs_get_active_speaker_participant_device(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->activeSpeakerParticipantDeviceCb;
}

void linphone_conference_cbs_set_active_speaker_participant_device(
    LinphoneConferenceCbs *cbs, LinphoneConferenceCbsActiveSpeakerParticipantDeviceCb cb) {
	ConferenceCbs::toCpp(cbs)->activeSpeakerParticipantDeviceCb = cb;
}

LinphoneConferenceCbsFullStateReceivedCb
linphone_conference_cbs_get_full_state_received(const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->fullStateReceivedCb;
}

void linphone_conference_cbs_set_full_state_received(LinphoneConferenceCbs *cbs,
                                                     LinphoneConferenceCbsFullStateReceivedCb cb) {
	ConferenceCbs::toCpp(cbs)->fullStateReceivedCb = cb;
}
