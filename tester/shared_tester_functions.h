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

#ifndef _SHARED_TESTER_FUNCTIONS_H_
#define _SHARED_TESTER_FUNCTIONS_H_

#include "liblinphone_tester.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_t check_ice(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneIceState state);
bool_t check_ice_sdp (LinphoneCall *call);
bool_t is_srtp_secured (LinphoneCall *call, LinphoneStreamType ctype);
void check_media_stream(LinphoneCall *call, bool_t is_null);
void check_local_desc_stream (LinphoneCall *call);
void check_result_desc_rtp_rtcp_ports (LinphoneCall *call, int rtp_port, int rtcp_port);
void _linphone_call_check_nb_streams(const LinphoneCall *call, const int nb_audio_streams, const int nb_video_streams, const int nb_text_streams);
int _linphone_call_get_nb_audio_steams(const LinphoneCall * call);
int _linphone_call_get_nb_video_steams(const LinphoneCall * call);
int _linphone_call_get_nb_text_steams(const LinphoneCall * call);
bool_t _linphone_participant_device_get_audio_enabled(const LinphoneParticipantDevice * participant_device);
bool_t _linphone_participant_device_get_video_enabled(const LinphoneParticipantDevice * participant_device);
bool_t _linphone_participant_device_get_real_time_text_enabled(const LinphoneParticipantDevice * participant_device);
void check_video_conference(LinphoneCoreManager* lc1, LinphoneCoreManager *lc2, LinphoneConferenceLayout layout);
void check_video_conference_with_local_participant(bctbx_list_t *participants, LinphoneConferenceLayout layout, bool_t local_partifipant);
const char * _linphone_call_get_subject(LinphoneCall * call);


int liblinphone_tester_send_data(const void *buffer, size_t length, const char *dest_ip, int dest_port, int sock_type);

#ifdef __cplusplus
}
#endif


#endif // _SHARED_TESTER_FUNCTIONS_H_
