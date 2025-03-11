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

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/client-conference.h"
#include "conference/conference.h"
#include "conference/participant.h"
#include "conference/server-conference.h"
#include "core/core.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-conference-cbs.h"
#include "linphone/api/c-conference.h"
#include "linphone/api/c-participant.h"
#include "linphone/wrapper_utils.h"
#include "player/call-player.h"

using namespace std;
using namespace LinphonePrivate;

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneConference *linphone_conference_ref(LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->ref();
	return conference;
}

void linphone_conference_unref(LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->unref();
}

void *linphone_conference_get_user_data(const LinphoneConference *conference) {
	return Conference::toCpp(conference)->getUserData();
}

void linphone_conference_set_user_data(LinphoneConference *conference, void *ud) {
	Conference::toCpp(conference)->setUserData(ud);
}

char *linphone_conference_state_to_string(LinphoneConferenceState state) {
	return ms_strdup(Utils::toString((ConferenceInterface::State)state).c_str());
}

LinphoneConference *linphone_local_conference_new(LinphoneCore *core, LinphoneAddress *addr) {
	return linphone_local_conference_new_with_params(core, addr, nullptr);
}

LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core,
                                                              LinphoneAddress *addr,
                                                              const LinphoneConferenceParams *params) {

	LinphoneConferenceParams *cloned_params = NULL;
	if (params) {
		cloned_params = linphone_conference_params_clone(params);
	} else {
		cloned_params = linphone_core_create_conference_params_2(core, NULL);
	}
	if (!linphone_conference_params_get_account(cloned_params) && addr) {
		linphone_conference_params_set_account(cloned_params, linphone_core_lookup_account_by_identity(core, addr));
	}
	std::shared_ptr<ConferenceParams> conf_params = ConferenceParams::toCpp(cloned_params)->getSharedFromThis();
	std::shared_ptr<Conference> localConf =
	    (new ServerConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), nullptr, conf_params))->toSharedPtr();
	if (cloned_params) {
		linphone_conference_params_unref(cloned_params);
	}
	localConf->init();
	localConf->ref();
	return localConf->toC();
}

LinphoneConference *linphone_remote_conference_new(LinphoneCore *core, LinphoneAddress *addr) {
	return linphone_remote_conference_new_with_params(core, addr, addr, nullptr);
}

LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core,
                                                               LinphoneAddress *focus,
                                                               LinphoneAddress *addr,
                                                               const LinphoneConferenceParams *params) {
	LinphoneConferenceParams *cloned_params = NULL;
	if (params) {
		cloned_params = linphone_conference_params_clone(params);
	} else {
		cloned_params = linphone_core_create_conference_params_2(core, NULL);
	}
	if (!linphone_conference_params_get_account(cloned_params) && addr) {
		linphone_conference_params_set_account(cloned_params, linphone_core_lookup_account_by_identity(core, addr));
	}
	std::shared_ptr<const ConferenceParams> conf_params = ConferenceParams::toCpp(cloned_params)->getSharedFromThis();
	std::shared_ptr<Conference> conference =
	    (new ClientConference(L_GET_CPP_PTR_FROM_C_OBJECT(core), nullptr, conf_params))->toSharedPtr();
	if (cloned_params) {
		linphone_conference_params_unref(cloned_params);
	}
	static_pointer_cast<ClientConference>(conference)
	    ->initWithFocus(Address::toCpp(focus)->getSharedFromThis(), nullptr, nullptr, conference.get());
	conference->ref();
	return conference->toC();
}

const char *linphone_conference_get_identifier(const LinphoneConference *conference) {
	const auto &identifier = Conference::toCpp(conference)->getIdentifier();
	if (identifier) {
		return L_STRING_TO_C(identifier.value());
	}
	return NULL;
}

LinphoneConferenceState linphone_conference_get_state(const LinphoneConference *conference) {
	return (LinphoneConferenceState)Conference::toCpp(conference)->getState();
}

LinphoneParticipantDevice *
linphone_conference_get_active_speaker_participant_device(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	shared_ptr<ParticipantDevice> p = Conference::toCpp(conference)->getActiveSpeakerParticipantDevice();
	if (p) {
		return p->toC();
	}
	return nullptr;
}

const LinphoneConferenceParams *linphone_conference_get_current_params(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getCurrentParams()->toC();
}

LinphoneStatus linphone_conference_add_participant(LinphoneConference *conference, LinphoneCall *call) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->addParticipant(Call::toCpp(call)->getSharedFromThis()) ? 0 : -1;
}

LinphoneStatus linphone_conference_add_participant_2(LinphoneConference *conference, const LinphoneAddress *uri) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)
	               ->addParticipant(Address::toCpp(const_cast<LinphoneAddress *>(uri))->getSharedFromThis())
	           ? 0
	           : -1;
}

LinphoneStatus linphone_conference_remove_participant(LinphoneConference *conference, const LinphoneAddress *uri) {
	ConferenceLogContextualizer logContextualizer(conference);
	LinphoneParticipant *participant =
	    linphone_conference_find_participant(conference, const_cast<LinphoneAddress *>(uri));
	return linphone_conference_remove_participant_2(conference, participant);
}

LinphoneStatus linphone_conference_remove_participant_2(LinphoneConference *conference,
                                                        LinphoneParticipant *participant) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->removeParticipant(Participant::toCpp(participant)->getSharedFromThis());
}

LinphoneStatus linphone_conference_remove_participant_3(LinphoneConference *conference, LinphoneCall *call) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->removeParticipant(Call::toCpp(call)->getSharedFromThis());
}

LinphoneParticipant *linphone_conference_find_participant(LinphoneConference *conference, const LinphoneAddress *uri) {
	ConferenceLogContextualizer logContextualizer(conference);
	shared_ptr<Participant> p =
	    Conference::toCpp(conference)
	        ->findParticipant(Address::toCpp(const_cast<LinphoneAddress *>(uri))->getSharedFromThis());
	if (p) {
		return p->toC();
	}
	return nullptr;
}

int linphone_conference_update_params(LinphoneConference *conference, const LinphoneConferenceParams *params) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->update(*ConferenceParams::toCpp(params));
}

int linphone_conference_terminate(LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->terminate();
}

int linphone_conference_enter(LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->enter();
}

int linphone_conference_leave(LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->leave();
	return 0;
}

bool_t linphone_conference_is_me(const LinphoneConference *conference, const LinphoneAddress *uri) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->isMe(Address::toCpp(const_cast<LinphoneAddress *>(uri))->getSharedFromThis());
}

bool_t linphone_conference_is_in(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->isIn();
}

void linphone_conference_set_input_audio_device(LinphoneConference *conference, LinphoneAudioDevice *audio_device) {
	ConferenceLogContextualizer logContextualizer(conference);
	if (audio_device) {
		Conference::toCpp(conference)->setInputAudioDevice(AudioDevice::getSharedFromThis(audio_device));
	}
}

void linphone_conference_set_output_audio_device(LinphoneConference *conference, LinphoneAudioDevice *audio_device) {
	ConferenceLogContextualizer logContextualizer(conference);
	if (audio_device) {
		Conference::toCpp(conference)->setOutputAudioDevice(AudioDevice::getSharedFromThis(audio_device));
	}
}

const LinphoneAudioDevice *linphone_conference_get_input_audio_device(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	auto audioDevice = Conference::toCpp(conference)->getInputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return nullptr;
}
const LinphoneAudioDevice *linphone_conference_get_output_audio_device(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	auto audioDevice = Conference::toCpp(conference)->getOutputAudioDevice();
	if (audioDevice) {
		return audioDevice->toC();
	}
	return nullptr;
}

int linphone_conference_get_participant_device_volume(LinphoneConference *conference,
                                                      LinphoneParticipantDevice *device) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)
	    ->getParticipantDeviceVolume(ParticipantDevice::toCpp(device)->getSharedFromThis());
}

float linphone_conference_get_input_volume(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getRecordVolume();
}

bool_t linphone_conference_get_microphone_muted(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getMicrophoneMuted();
}

void linphone_conference_set_microphone_muted(LinphoneConference *conference, bool_t muted) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->setMicrophoneMuted(!!muted);
}

LinphoneParticipant *linphone_conference_get_screen_sharing_participant(const LinphoneConference *conference) {
	LinphonePrivate::ConferenceLogContextualizer logContextualizer(conference);
	auto participant = Conference::toCpp(conference)->getScreenSharingParticipant();
	return participant ? participant->toC() : nullptr;
}

LinphoneParticipantDevice *
linphone_conference_get_screen_sharing_participant_device(const LinphoneConference *conference) {
	LinphonePrivate::ConferenceLogContextualizer logContextualizer(conference);
	auto device = Conference::toCpp(conference)->getScreenSharingDevice();
	return device ? device->toC() : nullptr;
}

int linphone_conference_get_participant_count(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getParticipantCount();
}

bctbx_list_t *linphone_conference_get_participants(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
	bctbx_list_t *participant_addresses = nullptr;
	for (bctbx_list_t *iterator = participants; iterator; iterator = bctbx_list_next(iterator)) {
		LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(iterator);
		LinphoneAddress *a = linphone_address_clone(linphone_participant_get_address(p));

		participant_addresses = bctbx_list_append(participant_addresses, a);
	}
	bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);
	return participant_addresses;
}

bctbx_list_t *linphone_conference_get_participant_list(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	const list<std::shared_ptr<Participant>> &participants = Conference::toCpp(conference)->getParticipants();
	bctbx_list_t *participants_list = nullptr;
	for (auto it = participants.begin(); it != participants.end(); it++) {
		const std::shared_ptr<Participant> participant((*it));
		participant->ref();
		LinphoneParticipant *c_participant(participant->toC());
		participants_list = bctbx_list_append(participants_list, c_participant);
	}
	return participants_list;
}

bctbx_list_t *linphone_conference_get_participant_device_list(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	const list<std::shared_ptr<ParticipantDevice>> devices = Conference::toCpp(conference)->getParticipantDevices();
	bctbx_list_t *devices_list = nullptr;
	for (auto it = devices.begin(); it != devices.end(); it++) {
		const std::shared_ptr<ParticipantDevice> device((*it));
		device->ref();
		LinphoneParticipantDevice *c_device(device->toC());
		devices_list = bctbx_list_append(devices_list, c_device);
	}
	return devices_list;
}

int linphone_conference_start_recording(LinphoneConference *conference, const char *path) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->startRecording(path);
}

int linphone_conference_stop_recording(LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->stopRecording();
}

bool_t linphone_conference_is_recording(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->isRecording();
}

bool_t linphone_conference_check_class(LinphoneConference *conference, LinphoneConferenceClass _class) {
	ConferenceLogContextualizer logContextualizer(conference);
	const auto &cpp_conference = *Conference::toCpp(conference);
	switch (_class) {
		case LinphoneConferenceClassLocal:
			return typeid(cpp_conference).hash_code() == typeid(ServerConference).hash_code();
		case LinphoneConferenceClassRemote:
			return typeid(cpp_conference).hash_code() == typeid(ClientConference).hash_code();
		default:
			return FALSE;
	}
}

LinphoneStatus linphone_conference_invite_participants(LinphoneConference *conference,
                                                       const bctbx_list_t *addresses,
                                                       const LinphoneCallParams *params) {
	ConferenceLogContextualizer logContextualizer(conference);
	std::list<std::shared_ptr<LinphonePrivate::Address>> addressList =
	    LinphonePrivate::Utils::bctbxListToCppSharedPtrList<LinphoneAddress, LinphonePrivate::Address>(addresses);
	return Conference::toCpp(conference)->inviteAddresses(addressList, params);
}

LinphoneStatus linphone_conference_add_participants(LinphoneConference *conference, const bctbx_list_t *calls) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)
	    ->addParticipants(Utils::bctbxListToCppSharedPtrList<LinphoneCall, Call>(calls));
}

LinphoneStatus linphone_conference_add_participants_2(LinphoneConference *conference, const bctbx_list_t *addresses) {
	ConferenceLogContextualizer logContextualizer(conference);
	std::list<std::shared_ptr<LinphonePrivate::Address>> addressList =
	    LinphonePrivate::Utils::bctbxListToCppSharedPtrList<LinphoneAddress, LinphonePrivate::Address>(addresses);
	return Conference::toCpp(conference)->addParticipants(addressList);
}

LinphoneParticipant *linphone_conference_get_me(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getMe()->toC();
}

const char *linphone_conference_get_subject(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return L_STRING_TO_C(Conference::toCpp(conference)->getSubject());
}

const char *linphone_conference_get_subject_utf8(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return L_STRING_TO_C(Conference::toCpp(conference)->getUtf8Subject());
}

void linphone_conference_set_subject(LinphoneConference *conference, const char *subject) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->setSubject(L_C_TO_STRING(subject));
}

void linphone_conference_set_subject_utf8(LinphoneConference *conference, const char *subject) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->setUtf8Subject(L_C_TO_STRING(subject));
}

const char *linphone_conference_get_username(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getUsername().c_str();
}

void linphone_conference_set_username(LinphoneConference *conference, const char *subject) {
	ConferenceLogContextualizer logContextualizer(conference);
	Conference::toCpp(conference)->setUsername(L_C_TO_STRING(subject));
}

int linphone_conference_get_duration(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getDuration();
}

time_t linphone_conference_get_start_time(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	return Conference::toCpp(conference)->getStartTime();
}

AudioStream *linphone_conference_get_audio_stream(LinphoneConference *conference) {
	return Conference::toCpp(conference)->getAudioStream();
}

LinphoneCall *linphone_conference_get_call(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	const std::shared_ptr<Call> call = Conference::toCpp(conference)->getCall();
	if (call) {
		return call->toC();
	}
	return nullptr;
}

void linphone_conference_set_local_participant_stream_capability(LinphoneConference *conference,
                                                                 const LinphoneMediaDirection direction,
                                                                 const LinphoneStreamType type) {
	Conference::toCpp(conference)->setLocalParticipantStreamCapability(direction, type);
}

void linphone_conference_set_participant_admin_status(LinphoneConference *conference,
                                                      LinphoneParticipant *participant,
                                                      bool_t isAdmin) {
	shared_ptr<Participant> p = Participant::toCpp(participant)->getSharedFromThis();
	Conference::toCpp(conference)->setParticipantAdminStatus(p, !!isAdmin);
}

void linphone_conference_notify_audio_device_changed(LinphoneConference *conference,
                                                     LinphoneAudioDevice *audio_device) {
	LinphoneCore *core = Conference::toCpp(conference)->getCore()->getCCore();
	linphone_core_notify_audio_device_changed(core, audio_device);
}
// =============================================================================
// Callbacks
// =============================================================================

void linphone_conference_add_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs) {
	Conference::toCpp(conference)->addCallbacks(ConferenceCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_conference_remove_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs) {
	Conference::toCpp(conference)->removeCallbacks(ConferenceCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_conference_set_current_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs) {
	Conference::toCpp(conference)->setCurrentCallbacks(cbs ? ConferenceCbs::toCpp(cbs)->getSharedFromThis() : nullptr);
}

LinphoneConferenceCbs *linphone_conference_get_current_callbacks(const LinphoneConference *conference) {
	return toC(Conference::toCpp(conference)->getCurrentCallbacks());
}

const bctbx_list_t *linphone_conference_get_callbacks_list(const LinphoneConference *conference) {
	return Conference::toCpp(conference)->getCCallbacksList();
}

void _linphone_conference_notify_allowed_participant_list_changed(LinphoneConference *conference) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Conference, Conference::toCpp(conference),
	                                         linphone_conference_cbs_get_allowed_participant_list_changed);
}

void _linphone_conference_notify_participant_added(LinphoneConference *conference, LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_added, participant);
}

void _linphone_conference_notify_participant_removed(LinphoneConference *conference,
                                                     const LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_removed, participant);
}

void _linphone_conference_notify_participant_device_media_capability_changed(
    LinphoneConference *conference, const LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_media_capability_changed,
	                                  participant_device);
}

void _linphone_conference_notify_participant_device_media_availability_changed(
    LinphoneConference *conference, const LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_media_availability_changed,
	                                  participant_device);
}

void _linphone_conference_notify_participant_device_added(LinphoneConference *conference,
                                                          LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_added, participant_device);
}

void _linphone_conference_notify_participant_device_removed(LinphoneConference *conference,
                                                            const LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_removed, participant_device);
}

void _linphone_conference_notify_participant_device_joining_request(LinphoneConference *conference,
                                                                    LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_joining_request,
	                                  participant_device);
}

void _linphone_conference_notify_participant_device_state_changed(LinphoneConference *conference,
                                                                  const LinphoneParticipantDevice *participant_device,
                                                                  const LinphoneParticipantDeviceState state) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_state_changed, participant_device,
	                                  state);
}

void _linphone_conference_notify_participant_device_screen_sharing_changed(
    LinphoneConference *conference, const LinphoneParticipantDevice *participant_device, bool_t enabled) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_screen_sharing_changed,
	                                  participant_device, enabled);
}

void _linphone_conference_notify_participant_role_changed(LinphoneConference *conference,
                                                          const LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_role_changed, participant);
}

void _linphone_conference_notify_participant_admin_status_changed(LinphoneConference *conference,
                                                                  const LinphoneParticipant *participant) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_admin_status_changed, participant);
}

void _linphone_conference_notify_available_media_changed(LinphoneConference *conference) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Conference, Conference::toCpp(conference),
	                                         linphone_conference_cbs_get_available_media_changed);
}

void _linphone_conference_notify_subject_changed(LinphoneConference *conference, const char *subject) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_subject_changed, subject);
}

void _linphone_conference_notify_participant_device_is_speaking_changed(
    LinphoneConference *conference, const LinphoneParticipantDevice *participant_device, bool_t is_speaking) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_is_speaking_changed,
	                                  participant_device, is_speaking);
}

void _linphone_conference_notify_participant_device_is_muted(LinphoneConference *conference,
                                                             const LinphoneParticipantDevice *participant_device,
                                                             bool_t is_muted) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_participant_device_is_muted, participant_device,
	                                  is_muted);
}

void _linphone_conference_notify_state_changed(LinphoneConference *conference, LinphoneConferenceState newState) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_state_changed, newState);
}

void _linphone_conference_notify_active_speaker_participant_device(
    LinphoneConference *conference, const LinphoneParticipantDevice *participant_device) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Conference, Conference::toCpp(conference),
	                                  linphone_conference_cbs_get_active_speaker_participant_device,
	                                  participant_device);
}

void _linphone_conference_notify_full_state_received(LinphoneConference *conference) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Conference, Conference::toCpp(conference),
	                                         linphone_conference_cbs_get_full_state_received);
}

LinphoneChatRoom *linphone_conference_get_chat_room(const LinphoneConference *conference) {
	auto &chatRoom = Conference::toCpp(conference)->getChatRoom();
	if (chatRoom) {
		return chatRoom->toC();
	}
	return nullptr;
}

LinphoneCore *linphone_conference_get_core(const LinphoneConference *conference) {
	return Conference::toCpp(conference)->getCore()->getCCore();
}

void linphone_conference_set_conference_address(LinphoneConference *conference, LinphoneAddress *address) {
	ConferenceLogContextualizer logContextualizer(conference);
	std::shared_ptr<ClientConference> clientConference =
	    dynamic_pointer_cast<ClientConference>(Conference::toCpp(conference)->getSharedFromThis());
	if (clientConference) {
		Conference::toCpp(conference)->setConferenceAddress(Address::toCpp(address)->getSharedFromThis());
	}
}

const LinphoneAddress *linphone_conference_get_conference_address(const LinphoneConference *conference) {
	ConferenceLogContextualizer logContextualizer(conference);
	const auto &address = Conference::toCpp(conference)->getConferenceAddress();
	return address && address->isValid() ? address->toC() : nullptr;
}

const char *linphone_conference_layout_to_string(const LinphoneConferenceLayout layout) {
	switch (layout) {
		case LinphoneConferenceLayoutGrid:
			return "LinphoneConferenceLayoutGrid";
		case LinphoneConferenceLayoutActiveSpeaker:
			return "LinphoneConferenceLayoutActiveSpeaker";
	}
	return nullptr;
}

LinphonePlayer *linphone_conference_get_player(LinphoneConference *conference) {
	std::shared_ptr<Player> player = Conference::toCpp(conference)->getPlayer();
	return player ? player->toC() : nullptr;
}

const LinphoneConferenceInfo *linphone_conference_get_info(LinphoneConference *conference) {
	std::shared_ptr<ConferenceInfo> info = Conference::toCpp(conference)->createOrGetConferenceInfo();
	return info ? info->toC() : nullptr;
}

LinphoneAccount *linphone_conference_get_account(LinphoneConference *conference) {
	shared_ptr<Account> account = Conference::toCpp(conference)->getAccount();
	return account ? account->toC() : nullptr;
}
