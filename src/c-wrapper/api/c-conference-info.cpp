/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include "conference/conference-info.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-address.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneConferenceInfo* linphone_conference_info_new() {
	return ConferenceInfo::createCObject();
}

LinphoneConferenceInfo* linphone_conference_info_ref(LinphoneConferenceInfo *conference_info) {
	ConferenceInfo::toCpp(conference_info)->ref();
	return conference_info;
}

void linphone_conference_info_unref(LinphoneConferenceInfo *conference_info) {
	ConferenceInfo::toCpp(conference_info)->unref();
}

const LinphoneAddress *linphone_conference_info_get_organizer(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getOrganizer();
}

void linphone_conference_info_set_organizer(LinphoneConferenceInfo *conference_info, LinphoneAddress *organizer) {
	ConferenceInfo::toCpp(conference_info)->setOrganizer(organizer);
}

const bctbx_list_t *linphone_conference_info_get_participants(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getParticipants();
}

void linphone_conference_info_set_participants(LinphoneConferenceInfo *conference_info, bctbx_list_t *participants) {
	ConferenceInfo::toCpp(conference_info)->setParticipants(participants);
}

void linphone_conference_info_add_participant(LinphoneConferenceInfo *conference_info, LinphoneAddress *participant) {
	ConferenceInfo::toCpp(conference_info)->addParticipant(participant);
}

const LinphoneAddress *linphone_conference_info_get_uri(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getUri();
}

void linphone_conference_info_set_uri(LinphoneConferenceInfo *conference_info, LinphoneAddress *uri) {
	ConferenceInfo::toCpp(conference_info)->setUri(uri);
}

time_t linphone_conference_info_get_date_time(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getDateTime();
}

void linphone_conference_info_set_date_time(LinphoneConferenceInfo *conference_info, time_t datetime) {
	ConferenceInfo::toCpp(conference_info)->setDateTime(datetime);
}

int linphone_conference_info_get_duration(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getDuration();
}

void linphone_conference_info_set_duration(LinphoneConferenceInfo *conference_info, int duration) {
	ConferenceInfo::toCpp(conference_info)->setDuration(duration);
}

const char *linphone_conference_info_get_subject(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getSubject());
}

void linphone_conference_info_set_subject(LinphoneConferenceInfo *conference_info, const char *subject) {
	ConferenceInfo::toCpp(conference_info)->setSubject(L_C_TO_STRING(subject));
}

const char *linphone_conference_info_get_description(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getDescription());
}

void linphone_conference_info_set_description(LinphoneConferenceInfo *conference_info, const char *description) {
	ConferenceInfo::toCpp(conference_info)->setDescription(L_C_TO_STRING(description));
}

char *linphone_conference_info_get_icalendar_string(const LinphoneConferenceInfo *conference_info) {
	std::string tmp = ConferenceInfo::toCpp(conference_info)->toIcsString();
	if (!tmp.empty()) {
		return bctbx_strdup(L_STRING_TO_C(tmp));
	}

	return NULL;
}
