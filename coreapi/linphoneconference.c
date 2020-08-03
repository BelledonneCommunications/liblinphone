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
#include "mediastreamer2/msogl.h"

#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/conference.h"

#include "conference/participant.h"
#include "conference/session/streams.h"
#include "conference/session/ms2-streams.h"
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

const char *linphone_conference_state_to_string (LinphoneConferenceState state) {
	return ms_strdup(Utils::toString((LinphonePrivate::ConferenceInterface::State)state).c_str());
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

LinphoneConference *linphone_remote_conference_new_with_params (LinphoneCore *core, LinphoneAddress * focus, LinphoneAddress * addr, const LinphoneConferenceParams *params) {
	LinphonePrivate::MediaConference::RemoteConference * conf = new LinphonePrivate::MediaConference::RemoteConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(focus)), ConferenceId(IdentityAddress(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr))), nullptr, ConferenceParams::toCpp(const_cast<LinphoneConferenceParams *>(params))->getSharedFromThis());

	return conf->toC();
}

LinphoneConferenceState linphone_conference_get_state (const LinphoneConference *conference) {
	return (LinphoneConferenceState)MediaConference::Conference::toCpp(conference)->getState();
}

const LinphoneConferenceParams * linphone_conference_get_current_params(const LinphoneConference *conference){
	return MediaConference::Conference::toCpp(conference)->getCurrentParams().toC();
}

LinphoneStatus linphone_conference_add_participant (LinphoneConference *conference, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(conference)->addParticipant(Call::toCpp(call)->getSharedFromThis());
}

LinphoneStatus linphone_conference_add_participant_2 (LinphoneConference *conference, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(conference)->addParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri));
}

LinphoneStatus linphone_conference_remove_participant (LinphoneConference *conference, const LinphoneAddress * uri) {
	LinphoneParticipant * participant = linphone_conference_find_participant (conference, uri);
	return linphone_conference_remove_participant_2 (conference, participant);
}

LinphoneStatus linphone_conference_remove_participant_2 (LinphoneConference *conference, LinphoneParticipant *participant) {
	return MediaConference::Conference::toCpp(conference)->removeParticipant(Participant::toCpp(participant)->getSharedFromThis());
}

LinphoneStatus linphone_conference_remove_participant_3 (LinphoneConference *conference, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(conference)->removeParticipant(Call::toCpp(call)->getSharedFromThis());
}

LinphoneParticipant * linphone_conference_find_participant (LinphoneConference *conference, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(conference)->findParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri))->toC();
}

int linphone_conference_update_params(LinphoneConference *conference, const LinphoneConferenceParams *params){
	return MediaConference::Conference::toCpp(conference)->update(*ConferenceParams::toCpp(params));
}

int linphone_conference_terminate (LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->terminate();
}

int linphone_conference_enter (LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->enter();
}

int linphone_conference_leave (LinphoneConference *conference) {
	MediaConference::Conference::toCpp(conference)->leave();
	return 0;
}

bool_t linphone_conference_is_in (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->isIn();
}

int linphone_conference_mute_microphone (LinphoneConference *conference, bool_t val) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(conference)->getAudioControlInterface();
	if (!aci) return -1;
	aci->enableMic(!val);
	return 0;
}

bool_t linphone_conference_microphone_is_muted (const LinphoneConference *conference) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(conference)->getAudioControlInterface();
	if (!aci) return FALSE;
	return aci->micEnabled() ? FALSE : TRUE;
}

float linphone_conference_get_input_volume (const LinphoneConference *conference) {
	AudioControlInterface *aci = MediaConference::Conference::toCpp(conference)->getAudioControlInterface();
	if (!aci) return 0.0;
	return aci->getRecordVolume();
}

/*ConferenceId linphone_conference_get_conference_id(const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getConferenceId();
}
*/

int linphone_conference_get_participant_count (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getParticipantCount();
}

int linphone_conference_get_size (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getSize();
}

bctbx_list_t *linphone_conference_get_participants (const LinphoneConference *conference) {
	bctbx_list_t * participants = linphone_conference_get_participant_list (conference);
	bctbx_list_t * participant_addresses = NULL;
	for (bctbx_list_t * iterator = participants; iterator; iterator = bctbx_list_next(iterator)) {
		LinphoneParticipant * p = (LinphoneParticipant *)bctbx_list_get_data(iterator);
		LinphoneAddress * a = linphone_address_clone(linphone_participant_get_address(p));

		participant_addresses = bctbx_list_append(participant_addresses, a);
	}
	bctbx_free(participants);
	return participant_addresses;
}

bctbx_list_t *linphone_conference_get_participant_list (const LinphoneConference *conference) {
	const list<std::shared_ptr<LinphonePrivate::Participant>> &participants = MediaConference::Conference::toCpp(conference)->getParticipants();
	bctbx_list_t *participants_list = nullptr;
	for (auto it = participants.begin(); it != participants.end(); it++) {
		const std::shared_ptr<LinphonePrivate::Participant> participant((*it));
		participant->ref();
		LinphoneParticipant *c_participant(participant->toC());
		participants_list = bctbx_list_append(participants_list, c_participant);
	}
	return participants_list;
}

int linphone_conference_start_recording (LinphoneConference *conference, const char *path) {
	return MediaConference::Conference::toCpp(conference)->startRecording(path);
}

int linphone_conference_stop_recording (LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->stopRecording();
}

bool_t linphone_conference_check_class (LinphoneConference *conference, LinphoneConferenceClass _class) {
	switch(_class) {
		case LinphoneConferenceClassLocal:
		return typeid(MediaConference::Conference::toCpp(conference)).hash_code() ==
			   typeid(LinphonePrivate::MediaConference::LocalConference).hash_code();
		case LinphoneConferenceClassRemote:
			return typeid(MediaConference::Conference::toCpp(conference)).hash_code() ==
				   typeid(LinphonePrivate::MediaConference::RemoteConference).hash_code();
		default:
			return FALSE;
	}
}

LinphoneStatus linphone_conference_invite_participants (LinphoneConference *conference, const bctbx_list_t *addresses, const LinphoneCallParams *params){
	return MediaConference::Conference::toCpp(conference)->inviteAddresses(toStd<const LinphoneAddress*>(addresses), params);
}

LinphoneParticipant *linphone_conference_get_me (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getMe()->toC();
}

const char *linphone_conference_get_subject (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getSubject().c_str();
}

void linphone_conference_set_subject(LinphoneConference *conference, const char *subject) {
	MediaConference::Conference::toCpp(conference)->setSubject(subject);
}

AudioStream *linphone_conference_get_audio_stream(LinphoneConference *conference){
	return MediaConference::Conference::toCpp(conference)->getAudioStream();
}

void linphone_conference_set_state_changed_callback (LinphoneConference *conference, LinphoneConferenceStateChangedCb cb, void *user_data) {
	MediaConference::Conference::toCpp(conference)->setStateChangedCallback(cb, user_data);
}

void linphone_conference_preview_ogl_render(LinphoneConference *obj) {
#ifdef VIDEO_ENABLED
	MediaConference::Conference *conf = MediaConference::Conference::toCpp(obj);
	if (conf->isIn()) {// Ensure to be in conference
		MS2VideoControl *control = dynamic_cast<MS2VideoControl*>(conf->getVideoControlInterface());
		if(control) {
			VideoStream *stream = control->getVideoStream();
			if(stream && stream->output2 && ms_filter_get_id(stream->output2) == MS_OGL_ID) {
				ms_filter_call_method(stream->output2, MS_OGL_RENDER, NULL);
			}
		}
	}
#endif
}

void linphone_conference_ogl_render(LinphoneConference *obj) {
#ifdef VIDEO_ENABLED
	MediaConference::Conference *conf = MediaConference::Conference::toCpp(obj);
	if (conf->isIn()) {// Ensure to be in conference
		MS2VideoControl *control = dynamic_cast<MS2VideoControl*>(conf->getVideoControlInterface());
		if(control) {
			VideoStream *stream = control->getVideoStream();
			if(stream && stream->output && ms_filter_get_id(stream->output) == MS_OGL_ID) {
				ms_filter_call_method(stream->output, MS_OGL_RENDER, NULL);
			}
		}
	}
#endif
}

LinphoneConferenceParams *linphone_conference_params_new (const LinphoneCore *core) {
	LinphoneConferenceParams *conference = ConferenceParams::createCObject(core);
	return conference;
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

