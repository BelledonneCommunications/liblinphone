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

#include <memory>
#include <type_traits>
#include <string>
#include <unordered_map>

#include <belle-sip/object++.hh>
#include "bctoolbox/list.h"
#include "mediastreamer2/msogl.h"

#include "core/core.h"
#include "call/call.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/conference.h"
#include "linphone/api/c-conference.h"

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

char *linphone_conference_state_to_string (LinphoneConferenceState state) {
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
	LinphonePrivate::MediaConference::RemoteConference * conference = new LinphonePrivate::MediaConference::RemoteConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(focus)), ConferenceId(IdentityAddress(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr))), nullptr, ConferenceParams::toCpp(const_cast<LinphoneConferenceParams *>(params))->getSharedFromThis());

	return conference->toC();
}

LinphoneConferenceState linphone_conference_get_state (const LinphoneConference *conference) {
	return (LinphoneConferenceState)MediaConference::Conference::toCpp(conference)->getState();
}

LinphoneParticipantDevice* linphone_conference_get_active_speaker_participant_device(const LinphoneConference *conference) {
	shared_ptr<LinphonePrivate::ParticipantDevice> p = MediaConference::Conference::toCpp(conference)->getActiveSpeakerParticipantDevice();
	if (p) {
		return p->toC();
	}
	return NULL;
}

const LinphoneConferenceParams * linphone_conference_get_current_params(const LinphoneConference *conference){
	return MediaConference::Conference::toCpp(conference)->getCurrentParams().toC();
}

LinphoneStatus linphone_conference_add_participant (LinphoneConference *conference, LinphoneCall *call) {
	return MediaConference::Conference::toCpp(conference)->addParticipant(Call::toCpp(call)->getSharedFromThis()) ? 0 : -1;
}

LinphoneStatus linphone_conference_add_participant_2 (LinphoneConference *conference, const LinphoneAddress *uri) {
	return MediaConference::Conference::toCpp(conference)->addParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri)) ? 0 : -1;
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
	shared_ptr<LinphonePrivate::Participant> p = MediaConference::Conference::toCpp(conference)->findParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(uri));
	if (p) {
		return p->toC();
	}
	return NULL;
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

bool_t linphone_conference_is_me (const LinphoneConference *conference, const LinphoneAddress * uri) {
	return MediaConference::Conference::toCpp(conference)->isMe(*L_GET_CPP_PTR_FROM_C_OBJECT(uri));
}

bool_t linphone_conference_is_in (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->isIn();
}

void linphone_conference_set_input_audio_device(LinphoneConference *conference, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		MediaConference::Conference::toCpp(conference)->setInputAudioDevice(LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

void linphone_conference_set_output_audio_device(LinphoneConference *conference, LinphoneAudioDevice *audio_device) {
	if (audio_device) {
		MediaConference::Conference::toCpp(conference)->setOutputAudioDevice(LinphonePrivate::AudioDevice::getSharedFromThis(audio_device));
	}
}

const LinphoneAudioDevice* linphone_conference_get_input_audio_device(const LinphoneConference *conference) {
	auto audioDevice = MediaConference::Conference::toCpp(conference)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}
const LinphoneAudioDevice* linphone_conference_get_output_audio_device(const LinphoneConference *conference) {
	auto audioDevice = MediaConference::Conference::toCpp(conference)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return NULL;
}

int linphone_conference_get_participant_device_volume(LinphoneConference *conference, LinphoneParticipantDevice *device) {
	return MediaConference::Conference::toCpp(conference)->getParticipantDeviceVolume(ParticipantDevice::toCpp(device)->getSharedFromThis());
}

int linphone_conference_mute_microphone (LinphoneConference *conference, bool_t val) {
	MediaConference::Conference::toCpp(conference)->notifyLocalMutedDevices(val);
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

int linphone_conference_get_participant_count (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getParticipantCount();
}

bctbx_list_t *linphone_conference_get_participants (const LinphoneConference *conference) {
	bctbx_list_t * participants = linphone_conference_get_participant_list (conference);
	bctbx_list_t * participant_addresses = NULL;
	for (bctbx_list_t * iterator = participants; iterator; iterator = bctbx_list_next(iterator)) {
		LinphoneParticipant * p = (LinphoneParticipant *)bctbx_list_get_data(iterator);
		LinphoneAddress * a = linphone_address_clone(linphone_participant_get_address(p));

		participant_addresses = bctbx_list_append(participant_addresses, a);
	}
	bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);
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

bctbx_list_t *linphone_conference_get_participant_device_list (const LinphoneConference *conference) {
	const list<std::shared_ptr<LinphonePrivate::ParticipantDevice>> devices = MediaConference::Conference::toCpp(conference)->getParticipantDevices();
	bctbx_list_t *devices_list = nullptr;
	for (auto it = devices.begin(); it != devices.end(); it++) {
		const std::shared_ptr<LinphonePrivate::ParticipantDevice> device((*it));
		device->ref();
		LinphoneParticipantDevice *c_device(device->toC());
		devices_list = bctbx_list_append(devices_list, c_device);
	}
	return devices_list;
}

int linphone_conference_start_recording (LinphoneConference *conference, const char *path) {
	return MediaConference::Conference::toCpp(conference)->startRecording(path);
}

int linphone_conference_stop_recording (LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->stopRecording();
}

bool_t linphone_conference_is_recording(const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->isRecording();
}

bool_t linphone_conference_check_class (LinphoneConference *conference, LinphoneConferenceClass _class) {
	const auto & cpp_conference = *LinphonePrivate::MediaConference::Conference::toCpp(conference);
	switch(_class) {
		case LinphoneConferenceClassLocal:
			return typeid(cpp_conference).hash_code() ==
				   typeid(LinphonePrivate::MediaConference::LocalConference).hash_code();
		case LinphoneConferenceClassRemote:
			return typeid(cpp_conference).hash_code() ==
				   typeid(LinphonePrivate::MediaConference::RemoteConference).hash_code();
		default:
			return FALSE;
	}
}

LinphoneStatus linphone_conference_invite_participants (LinphoneConference *conference, const bctbx_list_t *addresses, const LinphoneCallParams *params){
	return MediaConference::Conference::toCpp(conference)->inviteAddresses(toStd<const LinphoneAddress*>(addresses), params);
}

LinphoneStatus linphone_conference_add_participants (LinphoneConference *conference, const bctbx_list_t *calls){
	list<shared_ptr<LinphonePrivate::Call>> callList = L_GET_CPP_LIST_FROM_C_LIST_2(calls, LinphoneCall *, shared_ptr<LinphonePrivate::Call>, [] (LinphoneCall *call) {
			return LinphonePrivate::Call::toCpp(call)->getSharedFromThis();
		});
	return MediaConference::Conference::toCpp(conference)->addParticipants(callList);
}

LinphoneStatus linphone_conference_add_participants_2 (LinphoneConference *conference, const bctbx_list_t *addresses){
	list<LinphonePrivate::IdentityAddress> addressList = L_GET_CPP_LIST_FROM_C_LIST_2(addresses, LinphoneAddress *, LinphonePrivate::IdentityAddress, [] (LinphoneAddress *address) {
		return address ? LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(address)) : LinphonePrivate::IdentityAddress();
	});
	return MediaConference::Conference::toCpp(conference)->addParticipants(addressList);
}

LinphoneParticipant *linphone_conference_get_me (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getMe()->toC();
}

const char *linphone_conference_get_subject (const LinphoneConference *conference) {
	return L_STRING_TO_C(MediaConference::Conference::toCpp(conference)->getSubject());
}

void linphone_conference_set_subject(LinphoneConference *conference, const char *subject) {
	MediaConference::Conference::toCpp(conference)->setSubject(L_C_TO_STRING(subject));
}

const char *linphone_conference_get_username (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getUsername().c_str();
}

void linphone_conference_set_username(LinphoneConference *conference, const char *subject) {
	MediaConference::Conference::toCpp(conference)->setUsername(L_C_TO_STRING(subject));
}

int linphone_conference_get_duration (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getDuration();
}

time_t linphone_conference_get_start_time (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getStartTime();
}

AudioStream *linphone_conference_get_audio_stream(LinphoneConference *conference){
	return MediaConference::Conference::toCpp(conference)->getAudioStream();
}

LinphoneCall* linphone_conference_get_call(const LinphoneConference *conference) {
	const std::shared_ptr<Call> call = MediaConference::Conference::toCpp(conference)->getCall();
	if (call) {
		return call->toC();
	}
	return NULL;
}

void linphone_conference_set_local_participant_stream_capability(LinphoneConference *conference, const LinphoneMediaDirection direction, const LinphoneStreamType type) {
	MediaConference::Conference::toCpp(conference)->setLocalParticipantStreamCapability(direction, type);
}

void linphone_conference_set_state_changed_callback (LinphoneConference *conference, LinphoneConferenceStateChangedCb cb, void *user_data) {
	MediaConference::Conference::toCpp(conference)->setStateChangedCallback(cb, user_data);
}

void linphone_conference_set_participant_admin_status (LinphoneConference *conference, LinphoneParticipant *participant, bool_t isAdmin) {
	shared_ptr<LinphonePrivate::Participant> p = LinphonePrivate::Participant::toCpp(participant)->getSharedFromThis();
	MediaConference::Conference::toCpp(conference)->setParticipantAdminStatus(p, !!isAdmin);
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

void linphone_conference_params_set_audio_enabled (LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_audio (params, enable);
}

void linphone_conference_params_enable_audio (LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableAudio(enable ? true : false);
}

bool_t linphone_conference_params_is_audio_enabled (const LinphoneConferenceParams *params) {
	return linphone_conference_params_audio_enabled (params);
}

bool_t linphone_conference_params_audio_enabled (const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->audioEnabled() ? TRUE : FALSE;
}

void linphone_conference_params_set_video_enabled (LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_video (params, enable);
}

void linphone_conference_params_enable_video (LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableVideo(enable ? true : false);
}

bool_t linphone_conference_params_is_video_enabled (const LinphoneConferenceParams *params) {
	return linphone_conference_params_video_enabled (params);
}

bool_t linphone_conference_params_video_enabled (const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->videoEnabled() ? TRUE : FALSE;
}

void linphone_conference_params_set_chat_enabled (LinphoneConferenceParams *params, bool_t enable) {
	linphone_conference_params_enable_chat (params, enable);
}

void linphone_conference_params_enable_chat (LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableChat(enable ? true : false);
}

bool_t linphone_conference_params_is_chat_enabled (const LinphoneConferenceParams *params) {
	return linphone_conference_params_chat_enabled (params);
}

bool_t linphone_conference_params_chat_enabled (const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->chatEnabled() ? TRUE : FALSE;
}

LinphoneProxyConfig * linphone_conference_params_get_proxy_cfg(const LinphoneConferenceParams *params){
	auto account = ConferenceParams::toCpp(params)->getAccount();
	return linphone_core_lookup_proxy_by_identity(linphone_account_get_core(account), linphone_account_params_get_identity_address(linphone_account_get_params(account)));
}

LinphoneAccount * linphone_conference_params_get_account(const LinphoneConferenceParams *params){
	return ConferenceParams::toCpp(params)->getAccount();
}

void linphone_conference_params_set_local_participant_enabled(LinphoneConferenceParams *params, bool_t enable){
	linphone_conference_params_enable_local_participant(params, enable);
}

void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableLocalParticipant(!!enable);
}

bool_t linphone_conference_params_is_local_participant_enabled(const LinphoneConferenceParams *params){
	return linphone_conference_params_local_participant_enabled(params);
}

bool_t linphone_conference_params_local_participant_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->localParticipantEnabled();
}

void linphone_conference_params_set_one_participant_conference_enabled(LinphoneConferenceParams *params, bool_t enable){
	linphone_conference_params_enable_one_participant_conference(params, enable);
}

void linphone_conference_params_enable_one_participant_conference(LinphoneConferenceParams *params, bool_t enable) {
	ConferenceParams::toCpp(params)->enableOneParticipantConference(!!enable);
}

bool_t linphone_conference_params_is_one_participant_conference_enabled(const LinphoneConferenceParams *params){
	return linphone_conference_params_one_participant_conference_enabled(params);
}

bool_t linphone_conference_params_one_participant_conference_enabled(const LinphoneConferenceParams *params) {
	return ConferenceParams::toCpp(params)->oneParticipantConferenceEnabled();
}

void linphone_conference_params_set_subject(LinphoneConferenceParams *params, const char * subject){
	ConferenceParams::toCpp(params)->setSubject(L_C_TO_STRING(subject));
}

void linphone_conference_params_set_utf8_subject(LinphoneConferenceParams *params, const char * subject){
	ConferenceParams::toCpp(params)->setUtf8Subject(L_C_TO_STRING(subject));
}

const char * linphone_conference_params_get_subject(const LinphoneConferenceParams *params){
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getSubject());
}

const char * linphone_conference_params_get_utf8_subject(const LinphoneConferenceParams *params){
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getUtf8Subject());
}

const char *linphone_conference_params_get_description (const LinphoneConferenceParams *params) {
	return L_STRING_TO_C(ConferenceParams::toCpp(params)->getDescription());
}

void linphone_conference_params_set_description(LinphoneConferenceParams *params, const char *description) {
	ConferenceParams::toCpp(params)->setDescription(L_C_TO_STRING(description));
}

void linphone_conference_params_set_start_time(LinphoneConferenceParams *params, time_t start){
	ConferenceParams::toCpp(params)->setStartTime(start);
}

time_t linphone_conference_params_get_start_time(const LinphoneConferenceParams *params){
	return ConferenceParams::toCpp(params)->getStartTime();
}

void linphone_conference_params_set_end_time(LinphoneConferenceParams *params, time_t start){
	ConferenceParams::toCpp(params)->setEndTime(start);
}

time_t linphone_conference_params_get_end_time(const LinphoneConferenceParams *params){
	return ConferenceParams::toCpp(params)->getEndTime();
}

void linphone_conference_params_set_conference_factory_address(LinphoneConferenceParams *params, const LinphoneAddress *address){
	ConferenceParams::toCpp(params)->setConferenceFactoryAddress(address ? *L_GET_CPP_PTR_FROM_C_OBJECT(address) : Address());
}

const LinphoneAddress *linphone_conference_params_get_conference_factory_address(const LinphoneConferenceParams *params){
	auto & conferenceAddress = ConferenceParams::toCpp(params)->getConferenceFactoryAddress();
	if( conferenceAddress.isValid() && conferenceAddress != Address())	// isValid() is not enough and empty address should return NULL.
		return L_GET_C_BACK_PTR(&conferenceAddress);
	else
		return NULL;
}

void linphone_conference_params_set_participant_list_type(LinphoneConferenceParams *params, LinphoneConferenceParticipantListType type){
	ConferenceParams::toCpp(params)->setParticipantListType(static_cast<ConferenceParamsInterface::ParticipantListType>(type));
}

LinphoneConferenceParticipantListType linphone_conference_params_get_participant_list_type(const LinphoneConferenceParams *params){
	return static_cast<LinphoneConferenceParticipantListType>(ConferenceParams::toCpp(params)->getParticipantListType());
}

bool_t linphone_conference_params_is_static(const LinphoneConferenceParams *params) {
	return (ConferenceParams::toCpp(params)->isStatic() ? TRUE : FALSE);
}

const char *linphone_conference_get_ID (const LinphoneConference *conference) {
	return MediaConference::Conference::toCpp(conference)->getID().c_str();
}

void linphone_conference_set_ID(LinphoneConference *conference, const char *conferenceID) {
	MediaConference::Conference::toCpp(conference)->setID(conferenceID);
}

void linphone_conference_notify_audio_device_changed(LinphoneConference *conference, LinphoneAudioDevice *audio_device) {
	LinphoneCore * core = MediaConference::Conference::toCpp(conference)->getCore()->getCCore();
	linphone_core_notify_audio_device_changed(core, audio_device);
}
