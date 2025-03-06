/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "linphone/api/c-participant-info.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "conference/participant-info.h"
#include "linphone/api/c-address.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneParticipantInfo *linphone_participant_info_new(const LinphoneAddress *address) {
	return ParticipantInfo::createCObject(Address::toCpp(address)->getSharedFromThis());
}

LinphoneParticipantInfo *linphone_participant_info_ref(LinphoneParticipantInfo *participant_info) {
	ParticipantInfo::toCpp(participant_info)->ref();
	return participant_info;
}

LinphoneParticipantInfo *linphone_participant_info_clone(const LinphoneParticipantInfo *info) {
	return static_cast<ParticipantInfo *>(ParticipantInfo::toCpp(info)->clone())->toC();
}

void linphone_participant_info_unref(LinphoneParticipantInfo *participant_info) {
	ParticipantInfo::toCpp(participant_info)->unref();
}

const LinphoneAddress *linphone_participant_info_get_address(const LinphoneParticipantInfo *participant_info) {
	const auto &addr = ParticipantInfo::toCpp(participant_info)->getAddress();
	return addr->toC();
}

void linphone_participant_info_set_role(LinphoneParticipantInfo *participant_info, LinphoneParticipantRole role) {
	return ParticipantInfo::toCpp(participant_info)->setRole((Participant::Role)role);
}

LinphoneParticipantRole linphone_participant_info_get_role(const LinphoneParticipantInfo *participant_info) {
	return (LinphoneParticipantRole)ParticipantInfo::toCpp(participant_info)->getRole();
}

int linphone_participant_info_get_sequence_number(const LinphoneParticipantInfo *participant_info) {
	return ParticipantInfo::toCpp(participant_info)->getSequenceNumber();
}

void linphone_participant_info_set_sequence_number(LinphoneParticipantInfo *participant_info, int sequence) {
	ParticipantInfo::toCpp(participant_info)->setSequenceNumber(sequence);
}

void linphone_participant_info_add_parameter(LinphoneParticipantInfo *participant_info,
                                             const char *name,
                                             const char *value) {
	ParticipantInfo::toCpp(participant_info)->addParameter(L_C_TO_STRING(name), L_C_TO_STRING(value));
}

const char *linphone_participant_info_get_parameter_value(const LinphoneParticipantInfo *participant_info,
                                                          const char *name) {
	return L_STRING_TO_C(ParticipantInfo::toCpp(participant_info)->getParameterValue(L_C_TO_STRING(name)));
}

bool_t linphone_participant_info_has_parameter(const LinphoneParticipantInfo *participant_info, const char *name) {
	return (bool_t)ParticipantInfo::toCpp(participant_info)->hasParameter(L_C_TO_STRING(name));
}

void linphone_participant_info_remove_parameter(LinphoneParticipantInfo *participant_info, const char *name) {
	ParticipantInfo::toCpp(participant_info)->removeParameter(L_C_TO_STRING(name));
}

const char *linphone_participant_info_get_ccmp_uri(const LinphoneParticipantInfo *participant_info) {
	return ParticipantInfo::toCpp(participant_info)->getCcmpUriCstr();
}
