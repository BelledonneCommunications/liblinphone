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

#include "linphone/api/c-conference-params.h"
#include "account/account.h"
#include "c-wrapper/internal/c-tools.h"
#include "conference/conference-params.h"
#include "core/core.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "private_functions.h"

using namespace LinphonePrivate;

LinphoneConferenceParams *linphone_conference_params_new(LinphoneCore *core) {
	LinphoneConferenceParams *params =
	    ConferenceParams::createCObject(core ? L_GET_CPP_PTR_FROM_C_OBJECT(core) : nullptr);
	return params;
}

LinphoneConferenceParams *linphone_conference_params_ref(LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->ref();
	return params;
}

void linphone_conference_params_unref(LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

void linphone_conference_params_free(LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

LinphoneConferenceParams *linphone_conference_params_clone(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->clone()->toC();
}

void linphone_conference_params_set_audio_enabled(LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_audio(params, enable);
}

void linphone_conference_params_enable_audio(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableAudio(enable ? true : false);
}

bool_t linphone_conference_params_is_audio_enabled(const LinphoneConferenceParams *params) {
	return linphone_conference_params_audio_enabled(params);
}

bool_t linphone_conference_params_audio_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->audioEnabled() ? TRUE : FALSE;
}

void linphone_conference_params_set_video_enabled(LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_video(params, enable);
}

void linphone_conference_params_enable_video(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableVideo(enable ? true : false);
}

bool_t linphone_conference_params_is_video_enabled(const LinphoneConferenceParams *params) {
	return linphone_conference_params_video_enabled(params);
}

bool_t linphone_conference_params_video_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->videoEnabled() ? TRUE : FALSE;
}

void linphone_conference_params_set_chat_enabled(LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_chat(params, enable);
}

void linphone_conference_params_enable_chat(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableChat(enable ? true : false);
}

bool_t linphone_conference_params_is_chat_enabled(const LinphoneConferenceParams *params) {
	return linphone_conference_params_chat_enabled(params);
}

bool_t linphone_conference_params_chat_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->chatEnabled() ? TRUE : FALSE;
}

LinphoneProxyConfig *linphone_conference_params_get_proxy_cfg(const LinphoneConferenceParams *params) {
	auto account = ConferenceParams::toCpp(params)->getAccount()->toC();
	return linphone_core_lookup_proxy_by_identity(
	    linphone_account_get_core(account),
	    linphone_account_params_get_identity_address(linphone_account_get_params(account)));
}

void linphone_conference_params_set_account(LinphoneConferenceParams *params, LinphoneAccount *account) {
	return ConferenceParams::toCpp(params)->setAccount(account ? Account::toCpp(account)->getSharedFromThis()
	                                                           : nullptr);
}

LinphoneAccount *linphone_conference_params_get_account(const LinphoneConferenceParams *params) {
	const auto &account = ConferenceParams::toCpp(params)->getAccount();
	return account ? account->toC() : nullptr;
}

void linphone_conference_params_set_local_participant_enabled(LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_local_participant(params, enable);
}

void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableLocalParticipant(!!enable);
}

bool_t linphone_conference_params_is_local_participant_enabled(const LinphoneConferenceParams *params) {
	return linphone_conference_params_local_participant_enabled(params);
}

bool_t linphone_conference_params_local_participant_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->localParticipantEnabled();
}

void linphone_conference_params_set_one_participant_conference_enabled(LinphoneConferenceParams *params,
                                                                       bool_t enable) {
	linphone_conference_params_enable_one_participant_conference(params, enable);
}

void linphone_conference_params_enable_one_participant_conference(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableOneParticipantConference(!!enable);
}

bool_t linphone_conference_params_is_one_participant_conference_enabled(const LinphoneConferenceParams *params) {
	return linphone_conference_params_one_participant_conference_enabled(params);
}

bool_t linphone_conference_params_one_participant_conference_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->oneParticipantConferenceEnabled();
}

void linphone_conference_params_set_subject(LinphoneConferenceParams *params, const char *subject) {
	ConferenceParams::toCpp(params)->setSubject(L_C_TO_STRING(subject));
}

void linphone_conference_params_set_subject_utf8(LinphoneConferenceParams *params, const char *subject) {
	ConferenceParams::toCpp(params)->setUtf8Subject(L_C_TO_STRING(subject));
}

const char *linphone_conference_params_get_subject(const LinphoneConferenceParams *params) {
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getSubject());
}

const char *linphone_conference_params_get_subject_utf8(const LinphoneConferenceParams *params) {
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getUtf8Subject());
}

const char *linphone_conference_params_get_description_utf8(const LinphoneConferenceParams *params) {
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getUtf8Description());
}

void linphone_conference_params_set_description_utf8(LinphoneConferenceParams *params, const char *description) {
	ConferenceParams::toCpp(params)->setUtf8Description(L_C_TO_STRING(description));
}

void linphone_conference_params_set_start_time(LinphoneConferenceParams *params, time_t start) {
	ConferenceParams::toCpp(params)->setStartTime(start);
}

time_t linphone_conference_params_get_start_time(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->getStartTime();
}

time_t linphone_conference_params_get_earlier_joining_time(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->getEarlierJoiningTime();
}

void linphone_conference_params_set_end_time(LinphoneConferenceParams *params, time_t start) {
	ConferenceParams::toCpp(params)->setEndTime(start);
}

time_t linphone_conference_params_get_end_time(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->getEndTime();
}

time_t linphone_conference_params_get_expiry_time(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->getExpiryTime();
}

LinphoneConferenceSecurityLevel linphone_conference_params_get_security_level(const LinphoneConferenceParams *params) {
	return (LinphoneConferenceSecurityLevel)ConferenceParams::toCpp(params)->getSecurityLevel();
}

void linphone_conference_params_set_security_level(LinphoneConferenceParams *params,
                                                   LinphoneConferenceSecurityLevel security_level) {
	ConferenceParams::toCpp(params)->setSecurityLevel((ConferenceParamsInterface::SecurityLevel)security_level);
}

void linphone_conference_params_set_conference_factory_address(LinphoneConferenceParams *params,
                                                               const LinphoneAddress *address) {
	ConferenceParams::toCpp(params)->setConferenceFactoryAddress(
	    address ? Address::toCpp(const_cast<LinphoneAddress *>(address))->getSharedFromThis() : nullptr);
}

const LinphoneAddress *
linphone_conference_params_get_conference_factory_address(const LinphoneConferenceParams *params) {
	auto conferenceAddress = ConferenceParams::toCpp(params)->getConferenceFactoryAddress();
	if (conferenceAddress) return conferenceAddress->toC();
	else return NULL;
}

void linphone_conference_params_set_participant_list_type(LinphoneConferenceParams *params,
                                                          LinphoneConferenceParticipantListType type) {
	ConferenceParams::toCpp(params)->setParticipantListType(
	    static_cast<ConferenceParamsInterface::ParticipantListType>(type));
}

LinphoneConferenceParticipantListType
linphone_conference_params_get_participant_list_type(const LinphoneConferenceParams *params) {
	return static_cast<LinphoneConferenceParticipantListType>(
	    ConferenceParams::toCpp(params)->getParticipantListType());
}

void linphone_conference_params_set_hidden(LinphoneConferenceParams *params, bool_t hidden) {
	return ConferenceParams::toCpp(params)->setHidden(!!hidden);
}

bool_t linphone_conference_params_is_hidden(const LinphoneConferenceParams *params) {
	return (ConferenceParams::toCpp(params)->isHidden() ? TRUE : FALSE);
}

bool_t linphone_conference_params_is_valid(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->isValid();
}

bool_t linphone_conference_params_group_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->isGroup();
}

void linphone_conference_params_enable_group(LinphoneConferenceParams *params, bool_t group) {
	ConferenceParams::toCpp(params)->setGroup(!!group);
}

LinphoneChatParams *linphone_conference_params_get_chat_params(const LinphoneConferenceParams *params) {
	auto chatParams = ConferenceParams::toCpp(params)->getChatParams();
	return chatParams ? chatParams->toC() : nullptr;
}
