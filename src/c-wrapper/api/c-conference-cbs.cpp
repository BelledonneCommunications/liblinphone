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

#include "linphone/api/c-conference-cbs.h"
#include "conference_private.h"

#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneConferenceCbs * _linphone_conference_cbs_new (void) {
	return ConferenceCbs::createCObject();
}

LinphoneConferenceCbs * linphone_conference_cbs_ref (LinphoneConferenceCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_conference_cbs_unref (LinphoneConferenceCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void * linphone_conference_cbs_get_user_data (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->getUserData();
}

void linphone_conference_cbs_set_user_data (LinphoneConferenceCbs *cbs, void *ud) {
	ConferenceCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneConferenceCbsParticipantAddedCb linphone_conference_cbs_get_participant_added (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantAddedCb;
}

void linphone_conference_cbs_set_participant_added (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAddedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantAddedCb = cb;
}

LinphoneConferenceCbsParticipantRemovedCb linphone_conference_cbs_get_participant_removed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantRemovedCb;
}

void linphone_conference_cbs_set_participant_removed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantRemovedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantRemovedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceAddedCb linphone_conference_cbs_get_participant_device_added (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceAddedCb;
}

void linphone_conference_cbs_set_participant_device_added (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceAddedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceAddedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceRemovedCb linphone_conference_cbs_get_participant_device_removed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceRemovedCb;
}

void linphone_conference_cbs_set_participant_device_removed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceRemovedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceRemovedCb = cb;
}

LinphoneConferenceCbsParticipantAdminStatusChangedCb linphone_conference_cbs_get_participant_admin_status_changed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantAdminStatusChangedCb;
}

void linphone_conference_cbs_set_participant_admin_status_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAdminStatusChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantAdminStatusChangedCb = cb;
}

LinphoneConferenceCbsParticipantDeviceMediaChangedCb linphone_conference_cbs_get_participant_device_media_changed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->participantDeviceMediaChangedCb;
}

void linphone_conference_cbs_set_participant_device_media_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantDeviceMediaChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->participantDeviceMediaChangedCb = cb;
}

LinphoneConferenceCbsStateChangedCb linphone_conference_cbs_get_state_changed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->stateChangedCb;
}

void linphone_conference_cbs_set_state_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsStateChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->stateChangedCb = cb;
}

LinphoneConferenceCbsSubjectChangedCb linphone_conference_cbs_get_subject_changed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->subjectChangedCb;
}

void linphone_conference_cbs_set_subject_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsSubjectChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->subjectChangedCb = cb;
}

LinphoneConferenceCbsAudioDeviceChangedCb linphone_conference_cbs_get_audio_device_changed (const LinphoneConferenceCbs *cbs) {
	return ConferenceCbs::toCpp(cbs)->audioDeviceChangedCb;
}

void linphone_conference_cbs_set_audio_device_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsAudioDeviceChangedCb cb) {
	ConferenceCbs::toCpp(cbs)->audioDeviceChangedCb = cb;
}
