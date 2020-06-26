/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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
#include <type_traits>
#include <string>
#include <unordered_map>

#include <belle-sip/object++.hh>
#include "bctoolbox/list.h"

#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/conference.h"

#include "conference/participant.h"
#include "conference/session/streams.h"
#include "conference_private.h"

using namespace std;

template <typename _type>
inline list<_type> toStd(const bctbx_list_t *l){
	list<_type> ret;
	for(; l != NULL; l = l->next){
		ret.push_back(static_cast<_type>(l->data));
	}
	return ret;
}

using namespace LinphonePrivate;
using namespace LinphonePrivate::MediaConference;

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneConference *linphone_conference_ref (LinphoneConference *conference) {
	MediaConference::Conference::toCpp(conference)->ref();
	return conference;
}

void linphone_conference_unref (LinphoneConference *conference) {
	MediaConference::Conference::toCpp(conference)->unref();
}

void *linphone_conference_get_user_data (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getUserData();
}

void linphone_conference_set_user_data (LinphoneConference *conference, void *ud) {
	MediaConference::Conference::toCpp(conference)->setUserData(ud);
}

const char *linphone_conference_state_to_string (LinphoneChatRoomState state) {
	return LinphonePrivate::MediaConference::Conference::stateToString((LinphonePrivate::ConferenceInterface::State)state);
}

LinphoneConference *linphone_local_conference_new (LinphoneCore *core, LinphoneAddress * addr) {
	return (new LinphonePrivate::MediaConference::LocalConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), nullptr, ConferenceParams::create(core)))->toC();
}

LinphoneConference *linphone_local_conference_new_with_params (LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params) {
	return (new LinphonePrivate::MediaConference::LocalConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), nullptr, ConferenceParams::toCpp(const_cast<LinphoneConferenceParams *>(params))->getSharedFromThis()))->toC();
}

LinphoneConference *linphone_remote_conference_new (LinphoneCore *core, LinphoneAddress * addr) {
	return (new LinphonePrivate::MediaConference::RemoteConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), ConferenceId(IdentityAddress(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr))), nullptr, ConferenceParams::create(core)))->toC();
}

LinphoneConference *linphone_remote_conference_new_with_params (LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params) {
	return (new LinphonePrivate::MediaConference::RemoteConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr)), ConferenceId(IdentityAddress(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr))), nullptr, ConferenceParams::toCpp(const_cast<LinphoneConferenceParams *>(params))->getSharedFromThis()))->toC();
}

LinphoneChatRoomState linphone_conference_get_state (const LinphoneConference *obj) {
	return (LinphoneChatRoomState)MediaConference::Conference::toCpp(obj)->getState();
}

const LinphoneConferenceParams * linphone_conference_get_current_params(const LinphoneConference *obj){
	return MediaConference::Conference::toCpp(obj)->getCurrentParams().toC();
}

LinphoneStatus linphone_conference_add_participant_with_call (LinphoneConference *obj, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(obj)->addParticipant(Call::toCpp(call)->getSharedFromThis());
}

LinphoneStatus linphone_conference_add_participant_with_address (LinphoneConference *obj, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(obj)->addParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri));
}

LinphoneStatus linphone_conference_remove_participant (LinphoneConference *obj, LinphoneParticipant *participant) {
	return MediaConference::Conference::toCpp(obj)->removeParticipant(Participant::toCpp(participant)->getSharedFromThis());
}

LinphoneParticipant * linphone_conference_find_participant (LinphoneConference *obj, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(obj)->findParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri))->toC();
}

int linphone_conference_update_params(LinphoneConference *obj, const LinphoneConferenceParams *params){
	return MediaConference::Conference::toCpp(obj)->update(*ConferenceParams::toCpp(params));
}

int linphone_conference_terminate (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->terminate();
}

int linphone_conference_enter (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->enter();
}

int linphone_conference_leave (LinphoneConference *obj) {
	MediaConference::Conference::toCpp(obj)->leave();
	return 0;
}

bool_t linphone_conference_is_in (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->isIn();
}

int linphone_conference_mute_microphone (LinphoneConference *obj, bool_t val) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(obj)->getAudioControlInterface();
	if (!aci) return -1;
	aci->enableMic(!val);
	return 0;
}

bool_t linphone_conference_microphone_is_muted (const LinphoneConference *obj) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(obj)->getAudioControlInterface();
	if (!aci) return FALSE;
	return aci->micEnabled() ? FALSE : TRUE;
}

float linphone_conference_get_input_volume (const LinphoneConference *obj) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(obj)->getAudioControlInterface();
	if (!aci) return 0.0;
	return aci->getRecordVolume();
}

/*ConferenceId linphone_conference_get_conference_id(const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getConferenceId();
}
*/

int linphone_conference_get_participant_count (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getParticipantCount();
}

int linphone_conference_get_size (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getSize();
}

bctbx_list_t *linphone_conference_get_participants (const LinphoneConference *obj) {
	const list<std::shared_ptr<LinphonePrivate::Participant>> &participants = MediaConference::Conference::toCpp(obj)->getParticipants();
	bctbx_list_t *participants_list = nullptr;
	for (auto it = participants.begin(); it != participants.end(); it++) {
		const std::shared_ptr<LinphonePrivate::Participant> participant((*it));
		participant->ref();
		LinphoneParticipant *c_participant(participant->toC());
		participants_list = bctbx_list_append(participants_list, c_participant);
	}
	return participants_list;
}

int linphone_conference_start_recording (LinphoneConference *obj, const char *path) {
	return MediaConference::Conference::toCpp(obj)->startRecording(path);
}

int linphone_conference_stop_recording (LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->stopRecording();
}

bool_t linphone_conference_check_class (LinphoneConference *obj, LinphoneConferenceClass _class) {
	switch(_class) {
		case LinphoneConferenceClassLocal:
		return typeid(MediaConference::Conference::toCpp(obj)) ==
			   typeid(LinphonePrivate::MediaConference::LocalConference);
		case LinphoneConferenceClassRemote:
			return typeid(MediaConference::Conference::toCpp(obj)) ==
				   typeid(LinphonePrivate::MediaConference::RemoteConference);
		default:
			return FALSE;
	}
}

LinphoneStatus linphone_conference_invite_participants (LinphoneConference *obj, const bctbx_list_t *addresses, const LinphoneCallParams *params){
	return MediaConference::Conference::toCpp(obj)->inviteAddresses(toStd<const LinphoneAddress*>(addresses), params);
}

const char *linphone_conference_get_subject (const LinphoneConference *obj) {
	return MediaConference::Conference::toCpp(obj)->getSubject().c_str();
}

void linphone_conference_set_subject(LinphoneConference *obj, const char *subject) {
	MediaConference::Conference::toCpp(obj)->setSubject(subject);
}

AudioStream *linphone_conference_get_audio_stream(LinphoneConference *obj){
	return MediaConference::Conference::toCpp(obj)->getAudioStream();
}

void linphone_conference_set_state_changed_callback (LinphoneConference *obj, LinphoneConferenceStateChangedCb cb, void *user_data) {
	MediaConference::Conference::toCpp(obj)->setStateChangedCallback(cb, user_data);
}


LinphoneConferenceParams *linphone_conference_params_new (const LinphoneCore *core) {
	LinphoneConferenceParams *obj = ConferenceParams::createCObject(core);
	return obj;
}

LinphoneConferenceParams *linphone_conference_params_ref (LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->ref();
	return params;
}

void linphone_conference_params_unref (LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

void linphone_conference_params_free (LinphoneConferenceParams *params) {
	ConferenceParams::toCpp(params)->unref();
}

LinphoneConferenceParams *linphone_conference_params_clone (const LinphoneConferenceParams *params) {
	return static_cast<ConferenceParams*>(ConferenceParams::toCpp(params)->clone())->toC();
}

void linphone_conference_params_enable_video (LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableVideo(enable ? true : false);
}

bool_t linphone_conference_params_video_enabled (const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->videoEnabled() ? TRUE : FALSE;
}

void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params, bool_t enable){
	ConferenceParams::toCpp(params)->enableLocalParticipant(!!enable);
}

bool_t linphone_conference_params_local_participant_enabled(const LinphoneConferenceParams *params){
	return ConferenceParams::toCpp(params)->localParticipantEnabled();
}

