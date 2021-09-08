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

#include <memory>

#include "linphone/api/c-conference-cbs.h"
#include "linphone/api/c-conference.h"
#include "linphone/wrapper_utils.h"
#include "c-wrapper/c-wrapper.h"
#include "conference_private.h"
#include "core/core.h"

using namespace std;
using namespace LinphonePrivate;

// =============================================================================
// Callbacks
// =============================================================================

void linphone_conference_add_callbacks (LinphoneConference *conference, LinphoneConferenceCbs *cbs) {
	MediaConference::Conference::toCpp(conference)->addCallbacks(cbs);
}

void linphone_conference_remove_callbacks (LinphoneConference *conference, LinphoneConferenceCbs *cbs) {
	MediaConference::Conference::toCpp(conference)->removeCallbacks(cbs);
}

LinphoneConferenceCbs *linphone_conference_get_current_callbacks (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getCurrentCallbacks();
}

const bctbx_list_t *linphone_conference_get_callbacks_list(const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getCallbacksList();
}

void _linphone_conference_notify_participant_added(LinphoneConference *conference, const LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_participant_added, participant);
}

void _linphone_conference_notify_participant_removed(LinphoneConference *conference, const LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_participant_removed, participant);
}

void _linphone_conference_notify_participant_device_added(LinphoneConference *conference, const LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_participant_device_added, participant_device);
}

void _linphone_conference_notify_participant_device_removed(LinphoneConference *conference, const LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_participant_device_removed, participant_device);
}

void _linphone_conference_notify_participant_admin_status_changed(LinphoneConference *conference, const LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_participant_admin_status_changed, participant);
}

void _linphone_conference_notify_subject_changed(LinphoneConference *conference, const char *subject) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_subject_changed, subject);
}

void _linphone_conference_notify_state_changed(LinphoneConference *conference, LinphoneConferenceState newState) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(Conference, MediaConference::Conference::toCpp(conference), linphone_conference_cbs_get_state_changed, newState);
}

LinphoneCore *linphone_conference_get_core (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getCore()->getCCore();
}

void linphone_conference_set_conference_address(LinphoneConference *conference, LinphoneAddress *address) {
	std::shared_ptr<LinphonePrivate::MediaConference::RemoteConference> remoteConference = dynamic_pointer_cast<LinphonePrivate::MediaConference::RemoteConference>(MediaConference::Conference::toCpp(conference)->getSharedFromThis());
	if (remoteConference) {
		MediaConference::Conference::toCpp(conference)->setConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(address));
	}
}

const LinphoneAddress *linphone_conference_get_conference_address (const LinphoneConference *conference) {
	const LinphonePrivate::Address & address = MediaConference::Conference::toCpp(conference)->getConferenceAddress().asAddress();
	return address.isValid() ? L_GET_C_BACK_PTR(&address) : nullptr;
}
