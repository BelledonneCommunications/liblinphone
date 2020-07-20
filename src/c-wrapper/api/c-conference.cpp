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
#include "linphone/api/c-conference.h"
#include "linphone/wrapper_utils.h"
#include "c-wrapper/c-wrapper.h"
#include "conference_private.h"

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
	return MediaConference::Conference::toCpp(conference)->getCurrentCbs();
}

const bctbx_list_t *linphone_conference_get_callbacks_list(const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getCallbacksList();
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...) \
for (bctbx_list_t *it = MediaConference::Conference::toCpp(conference)->getCallbacksList(); it; it = bctbx_list_next(it)) { \
	MediaConference::Conference::toCpp(conference)->setCurrentCbs(reinterpret_cast<LinphoneConferenceCbs *>(bctbx_list_get_data(it))); \
	LinphoneConferenceCbs ## cbName ## Cb cb = linphone_conference_cbs_get_ ## functionName (MediaConference::Conference::toCpp(conference)->getCurrentCbs()); \
	if (cb) \
		cb(__VA_ARGS__); \
}

void _linphone_conference_notify_participant_added(LinphoneConference *conference, const LinphoneEventLog *event_log) {
	NOTIFY_IF_EXIST(ParticipantAdded, participant_added, conference, event_log)
}

void _linphone_conference_notify_participant_removed(LinphoneConference *conference, const LinphoneEventLog *event_log) {
	NOTIFY_IF_EXIST(ParticipantRemoved, participant_removed, conference, event_log)
}

void _linphone_conference_notify_participant_device_added(LinphoneConference *conference, const LinphoneEventLog *event_log) {
	NOTIFY_IF_EXIST(ParticipantDeviceAdded, participant_device_added, conference, event_log)
}

void _linphone_conference_notify_participant_device_removed(LinphoneConference *conference, const LinphoneEventLog *event_log) {
	NOTIFY_IF_EXIST(ParticipantDeviceRemoved, participant_device_added, conference, event_log)
}

void _linphone_conference_notify_participant_admin_status_changed(LinphoneConference *conference, const LinphoneEventLog *event_log) {
	NOTIFY_IF_EXIST(ParticipantAdminStatusChanged, participant_admin_status_changed, conference, event_log)
}

void _linphone_conference_notify_subject_changed(LinphoneConference *conference, const LinphoneEventLog *event_log) {
	NOTIFY_IF_EXIST(SubjectChanged, subject_changed, conference, event_log)
}

void _linphone_conference_notify_state_changed(LinphoneConference *conference, LinphoneConferenceState newState) {
	NOTIFY_IF_EXIST(StateChanged, state_changed, conference, newState)
}

LinphoneCore *linphone_conference_get_core (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getCore()->getCCore();
}

const LinphoneAddress *linphone_conference_get_conference_address (const LinphoneConference *conference) {
	const LinphonePrivate::Address address = MediaConference::Conference::toCpp(conference)->getConferenceAddress();

	printf("%s - address %s\n", __func__, address.asString().c_str());
	return address.isValid() ? L_GET_C_BACK_PTR(&address) : nullptr;

}

char *linphone_conference_get_conference_address_as_string (const LinphoneConference *conference) {
	return ms_strdup(MediaConference::Conference::toCpp(conference)->getConferenceAddress().asString().c_str());
}
