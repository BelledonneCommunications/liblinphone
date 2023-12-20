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

#ifndef _SHARED_TESTER_FUNCTIONS_H_
#define _SHARED_TESTER_FUNCTIONS_H_

#include "liblinphone_tester.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_t check_ice(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneIceState state);
bool_t check_ice_sdp (LinphoneCall *call);

typedef enum _TesterIceCandidateType{
	TesterIceCandidateHost,
	TesterIceCandidateSflrx,
	TesterIceCandidateRelay
} TesterIceCandidateType;
void liblinphone_tester_check_ice_default_candidates(LinphoneCoreManager *marie, TesterIceCandidateType marie_expected_type, LinphoneCoreManager *pauline, TesterIceCandidateType pauline_expected_type);
bool_t is_srtp_secured (LinphoneCall *call, LinphoneStreamType ctype);
void check_media_stream(LinphoneCall *call, bool_t is_null);
void check_local_desc_stream (LinphoneCall *call);
void check_result_desc_rtp_rtcp_ports (LinphoneCall *call, int rtp_port, int rtcp_port);
void _check_call_media_ip_consistency(LinphoneCall *call);
void _linphone_call_check_nb_active_streams(const LinphoneCall *call, const int nb_audio_streams, const int nb_video_streams, const int nb_text_streams);
void _linphone_call_check_nb_streams(const LinphoneCall *call, const int nb_audio_streams, const int nb_video_streams, const int nb_text_streams);
void _linphone_call_check_max_nb_streams(const LinphoneCall *call,
                                         const size_t nb_audio_streams,
                                         const size_t nb_video_streams,
                                         const size_t nb_text_streams);
int _linphone_call_get_nb_audio_steams(const LinphoneCall * call);
int _linphone_call_get_nb_video_steams(const LinphoneCall * call);
int _linphone_call_get_nb_text_steams(const LinphoneCall * call);
LinphoneConferenceLayout _linphone_participant_device_get_layout(const LinphoneParticipantDevice * participant_device);
bool_t _linphone_participant_device_get_audio_enabled(const LinphoneParticipantDevice * participant_device);
bool_t _linphone_participant_device_get_video_enabled(const LinphoneParticipantDevice * participant_device);
bool_t _linphone_participant_device_get_real_time_text_enabled(const LinphoneParticipantDevice * participant_device);

bool_t linphone_conference_type_is_full_state(const char * text);
void check_video_conference(bctbx_list_t *lcs, LinphoneCoreManager* lc1, LinphoneCoreManager *lc2, LinphoneConferenceLayout layout);
void check_video_conference_with_local_participant(bctbx_list_t *participants, LinphoneConferenceLayout layout, bool_t local_partifipant);
const char * _linphone_call_get_subject(LinphoneCall * call);
void _linphone_conference_video_change(bctbx_list_t *lcs, LinphoneCoreManager *mgr1, LinphoneCoreManager *mgr2, LinphoneCoreManager *mgr3);

int liblinphone_tester_send_data(const void *buffer, size_t length, const char *dest_ip, int dest_port, int sock_type);

void linphone_conference_info_check_participant(const LinphoneConferenceInfo * conference_info, LinphoneAddress * address, int sequence_number);
void linphone_conference_info_check_organizer(const LinphoneConferenceInfo * conference_info, int sequence_number);

void check_chat_message_properties(LinphoneChatMessage *msg);

#ifdef __cplusplus
}
#endif


#endif // _SHARED_TESTER_FUNCTIONS_H_
