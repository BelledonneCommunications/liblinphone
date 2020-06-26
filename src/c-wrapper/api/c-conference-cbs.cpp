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

#include "c-wrapper/c-wrapper.h"

// =============================================================================

struct _LinphoneConferenceCbs {
	belle_sip_object_t base;
	void *userData;
	LinphoneConferenceCbsParticipantAddedCb participantAddedCb;
	LinphoneConferenceCbsParticipantRemovedCb participantRemovedCb;
	LinphoneConferenceCbsParticipantDeviceAddedCb participantDeviceAddedCb;
	LinphoneConferenceCbsParticipantDeviceRemovedCb participantDeviceRemovedCb;
	LinphoneConferenceCbsParticipantAdminStatusChangedCb participantAdminStatusChangedCb;
	LinphoneConferenceCbsStateChangedCb stateChangedCb;
	LinphoneConferenceCbsSubjectChangedCb subjectChangedCb;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneConferenceCbs);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneConferenceCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneConferenceCbs, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

// =============================================================================

LinphoneConferenceCbs * _linphone_conference_cbs_new (void) {
	return belle_sip_object_new(LinphoneConferenceCbs);
}

LinphoneConferenceCbs * linphone_conference_cbs_ref (LinphoneConferenceCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_conference_cbs_unref (LinphoneConferenceCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void * linphone_conference_cbs_get_user_data (const LinphoneConferenceCbs *cbs) {
	return cbs->userData;
}

void linphone_conference_cbs_set_user_data (LinphoneConferenceCbs *cbs, void *ud) {
	cbs->userData = ud;
}

LinphoneConferenceCbsParticipantAddedCb linphone_conference_cbs_get_participant_added (const LinphoneConferenceCbs *cbs) {
	return cbs->participantAddedCb;
}

void linphone_conference_cbs_set_participant_added (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAddedCb cb) {
	cbs->participantAddedCb = cb;
}

LinphoneConferenceCbsParticipantRemovedCb linphone_conference_cbs_get_participant_removed (const LinphoneConferenceCbs *cbs) {
	return cbs->participantRemovedCb;
}

void linphone_conference_cbs_set_participant_removed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantRemovedCb cb) {
	cbs->participantRemovedCb = cb;
}

LinphoneConferenceCbsParticipantAdminStatusChangedCb linphone_conference_cbs_get_participant_admin_status_changed (const LinphoneConferenceCbs *cbs) {
	return cbs->participantAdminStatusChangedCb;
}

void linphone_conference_cbs_set_participant_admin_status_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsParticipantAdminStatusChangedCb cb) {
	cbs->participantAdminStatusChangedCb = cb;
}

LinphoneConferenceCbsStateChangedCb linphone_conference_cbs_get_state_changed (const LinphoneConferenceCbs *cbs) {
	return cbs->stateChangedCb;
}

void linphone_conference_cbs_set_state_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsStateChangedCb cb) {
	cbs->stateChangedCb = cb;
}

LinphoneConferenceCbsSubjectChangedCb linphone_conference_cbs_get_subject_changed (const LinphoneConferenceCbs *cbs) {
	return cbs->subjectChangedCb;
}

void linphone_conference_cbs_set_subject_changed (LinphoneConferenceCbs *cbs, LinphoneConferenceCbsSubjectChangedCb cb) {
	cbs->subjectChangedCb = cb;
}
