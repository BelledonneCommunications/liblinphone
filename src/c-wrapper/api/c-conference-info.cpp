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

#include "linphone/api/c-conference-info.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "conference/conference-info.h"
#include "conference/participant-info.h"
#include "linphone/api/c-address.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneConferenceInfo *linphone_conference_info_new() {
	return ConferenceInfo::createCObject();
}

LinphoneConferenceInfo *linphone_conference_info_ref(LinphoneConferenceInfo *conference_info) {
	ConferenceInfo::toCpp(conference_info)->ref();
	return conference_info;
}

LinphoneConferenceInfo *linphone_conference_info_clone(const LinphoneConferenceInfo *info) {
	return static_cast<ConferenceInfo *>(ConferenceInfo::toCpp(info)->clone())->toC();
}

void linphone_conference_info_unref(LinphoneConferenceInfo *conference_info) {
	ConferenceInfo::toCpp(conference_info)->unref();
}

const LinphoneParticipantInfo *
linphone_conference_info_get_organizer_info(const LinphoneConferenceInfo *conference_info) {
	const auto &organizer = ConferenceInfo::toCpp(conference_info)->getOrganizer();
	return organizer ? organizer->toC() : nullptr;
}

const LinphoneAddress *linphone_conference_info_get_organizer(const LinphoneConferenceInfo *conference_info) {
	const auto &address = ConferenceInfo::toCpp(conference_info)->getOrganizerAddress();
	return address && address->isValid() ? address->toC() : nullptr;
}

void linphone_conference_info_set_organizer(LinphoneConferenceInfo *conference_info, const LinphoneAddress *organizer) {
	ConferenceInfo::toCpp(conference_info)->setOrganizer(Address::toCpp(organizer)->getSharedFromThis());
}

const bctbx_list_t *linphone_conference_info_get_participants(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getParticipantAddressCList();
}

const bctbx_list_t *linphone_conference_info_get_participant_infos(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getParticipantsCList();
}

void linphone_conference_info_set_participants(LinphoneConferenceInfo *conference_info,
                                               const bctbx_list_t *participants) {
	const std::list<std::shared_ptr<LinphonePrivate::Address>> participantsList =
	    LinphonePrivate::Utils::bctbxListToCppSharedPtrList<LinphoneAddress, LinphonePrivate::Address>(participants);
	ConferenceInfo::toCpp(conference_info)->setParticipants(participantsList);
}

void linphone_conference_info_set_participant_infos(LinphoneConferenceInfo *conference_info,
                                                    const bctbx_list_t *participant_infos) {
	const std::list<std::shared_ptr<LinphonePrivate::ParticipantInfo>> participantInfos =
	    LinphonePrivate::Utils::bctbxListToCppSharedPtrList<LinphoneParticipantInfo, LinphonePrivate::ParticipantInfo>(
	        participant_infos);
	ConferenceInfo::toCpp(conference_info)->setParticipants(participantInfos);
}

void linphone_conference_info_add_participant_infos(LinphoneConferenceInfo *conference_info,
                                                    const bctbx_list_t *participant_infos) {
	const std::list<std::shared_ptr<LinphonePrivate::ParticipantInfo>> participantInfos =
	    Utils::bctbxListToCppSharedPtrList<LinphoneParticipantInfo, ParticipantInfo>(participant_infos);
	ConferenceInfo::toCpp(conference_info)->addParticipants(participantInfos);
}

void linphone_conference_info_add_participant(LinphoneConferenceInfo *conference_info,
                                              const LinphoneAddress *participant) {
	ConferenceInfo::toCpp(conference_info)->addParticipant(Address::toCpp(participant)->getSharedFromThis());
}

void linphone_conference_info_add_participant_2(LinphoneConferenceInfo *conference_info,
                                                const LinphoneParticipantInfo *participant_info) {
	ConferenceInfo::toCpp(conference_info)
	    ->addParticipant(ParticipantInfo::toCpp(participant_info)->getSharedFromThis());
}

void linphone_conference_info_update_participant(LinphoneConferenceInfo *conference_info,
                                                 const LinphoneParticipantInfo *participant_info) {
	ConferenceInfo::toCpp(conference_info)
	    ->updateParticipant(ParticipantInfo::toCpp(participant_info)->getSharedFromThis());
}

void linphone_conference_info_remove_participant(LinphoneConferenceInfo *conference_info,
                                                 const LinphoneAddress *participant) {
	ConferenceInfo::toCpp(conference_info)->removeParticipant(Address::toCpp(participant)->getSharedFromThis());
}

const LinphoneParticipantInfo *linphone_conference_info_find_participant(LinphoneConferenceInfo *conference_info,
                                                                         const LinphoneAddress *participant) {
	const auto &participant_info =
	    ConferenceInfo::toCpp(conference_info)->findParticipant(Address::toCpp(participant)->getSharedFromThis());
	return (participant_info) ? participant_info->toC() : NULL;
}

const LinphoneAddress *linphone_conference_info_get_uri(const LinphoneConferenceInfo *conference_info) {
	const auto &address = ConferenceInfo::toCpp(conference_info)->getUri();
	return address && address->isValid() ? address->toC() : nullptr;
}

void linphone_conference_info_set_uri(LinphoneConferenceInfo *conference_info, const LinphoneAddress *uri) {
	ConferenceInfo::toCpp(conference_info)->setUri(Address::toCpp(uri)->getSharedFromThis());
}

time_t linphone_conference_info_get_date_time(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getDateTime();
}

void linphone_conference_info_set_date_time(LinphoneConferenceInfo *conference_info, time_t datetime) {
	ConferenceInfo::toCpp(conference_info)->setDateTime(datetime);
}

time_t linphone_conference_info_get_earlier_joining_time(const LinphoneConferenceInfo *info) {
	return ConferenceInfo::toCpp(info)->getEarlierJoiningTime();
}

time_t linphone_conference_info_get_expiry_time(const LinphoneConferenceInfo *info) {
	return ConferenceInfo::toCpp(info)->getExpiryTime();
}

unsigned int linphone_conference_info_get_duration(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getDuration();
}

void linphone_conference_info_set_duration(LinphoneConferenceInfo *conference_info, unsigned int duration) {
	ConferenceInfo::toCpp(conference_info)->setDuration(duration);
}

const char *linphone_conference_info_get_subject(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getSubject());
}

void linphone_conference_info_set_subject(LinphoneConferenceInfo *conference_info, const char *subject) {
	ConferenceInfo::toCpp(conference_info)->setSubject(L_C_TO_STRING(subject));
}

const char *linphone_conference_info_get_subject_utf8(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getUtf8Subject());
}

void linphone_conference_info_set_subject_utf8(LinphoneConferenceInfo *conference_info, const char *subject) {
	ConferenceInfo::toCpp(conference_info)->setUtf8Subject(L_C_TO_STRING(subject));
}

void linphone_conference_info_set_ics_sequence(LinphoneConferenceInfo *conference_info, unsigned int sequence) {
	ConferenceInfo::toCpp(conference_info)->setIcsSequence(sequence);
}

unsigned int linphone_conference_info_get_ics_sequence(const LinphoneConferenceInfo *conference_info) {
	return ConferenceInfo::toCpp(conference_info)->getIcsSequence();
}

void linphone_conference_info_set_ccmp_uri(LinphoneConferenceInfo *conference_info, const char *uri) {
	ConferenceInfo::toCpp(conference_info)->setCcmpUri(L_C_TO_STRING(uri));
}

const char *linphone_conference_info_get_ccmp_uri(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getCcmpUri());
}

void linphone_conference_info_set_ics_uid(LinphoneConferenceInfo *conference_info, const char *uid) {
	return ConferenceInfo::toCpp(conference_info)->setIcsUid(L_C_TO_STRING(uid));
}

const char *linphone_conference_info_get_ics_uid(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getIcsUid());
}

const char *linphone_conference_info_get_description(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getDescription());
}

void linphone_conference_info_set_description(LinphoneConferenceInfo *conference_info, const char *description) {
	ConferenceInfo::toCpp(conference_info)->setDescription(L_C_TO_STRING(description));
}

const char *linphone_conference_info_get_description_utf8(const LinphoneConferenceInfo *conference_info) {
	return L_STRING_TO_C(ConferenceInfo::toCpp(conference_info)->getUtf8Description());
}

void linphone_conference_info_set_description_utf8(LinphoneConferenceInfo *conference_info, const char *description) {
	ConferenceInfo::toCpp(conference_info)->setUtf8Description(L_C_TO_STRING(description));
}

LinphoneConferenceSecurityLevel
linphone_conference_info_get_security_level(const LinphoneConferenceInfo *conference_info) {
	return (LinphoneConferenceSecurityLevel)ConferenceInfo::toCpp(conference_info)->getSecurityLevel();
}

void linphone_conference_info_set_security_level(LinphoneConferenceInfo *conference_info,
                                                 LinphoneConferenceSecurityLevel security_level) {
	ConferenceInfo::toCpp(conference_info)->setSecurityLevel((ConferenceParamsInterface::SecurityLevel)security_level);
}

void linphone_conference_info_set_capability(LinphoneConferenceInfo *conference_info,
                                             const LinphoneStreamType stream_type,
                                             bool_t enable) {
	return ConferenceInfo::toCpp(conference_info)->setCapability(stream_type, !!enable);
}

bool_t linphone_conference_info_get_capability(const LinphoneConferenceInfo *conference_info,
                                               const LinphoneStreamType stream_type) {
	return ConferenceInfo::toCpp(conference_info)->getCapability(stream_type) ? TRUE : FALSE;
}

char *linphone_conference_info_get_icalendar_string(const LinphoneConferenceInfo *conference_info) {
	std::string tmp = ConferenceInfo::toCpp(conference_info)->toIcsString();
	if (!tmp.empty()) {
		return bctbx_strdup(L_STRING_TO_C(tmp));
	}

	return NULL;
}

void linphone_conference_info_set_state(LinphoneConferenceInfo *conference_info, LinphoneConferenceInfoState state) {
	ConferenceInfo::toCpp(conference_info)->setState(static_cast<ConferenceInfo::State>(state));
}
LinphoneConferenceInfoState linphone_conference_info_get_state(const LinphoneConferenceInfo *conference_info) {
	return (LinphoneConferenceInfoState)ConferenceInfo::toCpp(conference_info)->getState();
}
