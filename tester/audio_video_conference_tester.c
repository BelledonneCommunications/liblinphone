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

#include <sys/stat.h>
#include <sys/types.h>

#include "belle-sip/sipstack.h"

#include "mediastreamer2/msanalysedisplay.h"
#include "mediastreamer2/msmire.h"
#include "mediastreamer2/msutils.h"
#include "mediastreamer2/msvolume.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

#ifdef _WIN32
#define unlink _unlink
#endif

static void set_video_in_call(LinphoneCoreManager *m1,
                              LinphoneCoreManager *m2,
                              bool_t enable_video,
                              bctbx_list_t *participants,
                              const LinphoneAddress *conference_address) {

	ms_message("%s %s video in call with %s", linphone_core_get_identity(m1->lc),
	           ((enable_video) ? "enables" : "disables"), linphone_core_get_identity(m2->lc));

	LinphoneConference *m1_conference = linphone_core_search_conference(m1->lc, NULL, NULL, conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(m1_conference);
	bool_t expected_m1_video_capability = FALSE;
	if (m1_conference) {
		const LinphoneConferenceParams *params = linphone_conference_get_current_params(m1_conference);
		expected_m1_video_capability = linphone_conference_params_video_enabled(params);
		if (linphone_core_conference_server_enabled(m1->lc)) {
			BC_ASSERT_TRUE(linphone_conference_is_in(m1_conference) ==
			               linphone_conference_params_local_participant_enabled(params));
		} else {
			BC_ASSERT_TRUE(linphone_conference_is_in(m1_conference));
		}
	}

	LinphoneConference *m2_conference = linphone_core_search_conference(m2->lc, NULL, NULL, conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(m2_conference);
	bool_t expected_m2_video_capability = FALSE;
	if (m2_conference) {
		const LinphoneConferenceParams *params = linphone_conference_get_current_params(m2_conference);
		expected_m2_video_capability = linphone_conference_params_video_enabled(params);
		if (linphone_core_conference_server_enabled(m2->lc)) {
			BC_ASSERT_TRUE(linphone_conference_is_in(m2_conference) ==
			               linphone_conference_params_local_participant_enabled(params));
		} else {
			BC_ASSERT_TRUE(linphone_conference_is_in(m2_conference));
		}
	}

	LinphoneCall *m1_calls_m2 = linphone_core_get_call_by_remote_address2(m1->lc, m2->identity);
	BC_ASSERT_PTR_NOT_NULL(m1_calls_m2);

	LinphoneCall *m2_calls_m1 = linphone_core_get_call_by_remote_address2(m2->lc, m1->identity);
	BC_ASSERT_PTR_NOT_NULL(m2_calls_m1);

	bctbx_list_t *lcs = bctbx_list_append(NULL, m2->lc);
	stats *initial_participants_stats = NULL;
	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		lcs = bctbx_list_append(lcs, m->lc);

		// Participant stats
		initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));
		initial_participants_stats[counter - 1] = m->stat;
		counter++;

		if (m != m2) {
			LinphoneConference *m_conference =
			    linphone_core_search_conference(m->lc, NULL, NULL, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(m_conference);
			LinphoneCall *m_calls_m2 = linphone_core_get_call_by_remote_address2(m->lc, m2->identity);
			BC_ASSERT_PTR_NOT_NULL(m_calls_m2);
		}
	}

	if (m1_calls_m2) {
		stats initial_m2_stat = m2->stat;
		stats initial_m1_stat = m1->stat;
		LinphoneCallParams *new_params = linphone_core_create_call_params(m1->lc, m1_calls_m2);
		linphone_call_params_enable_video(new_params, enable_video);
		linphone_call_update(m1_calls_m2, new_params);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallUpdatedByRemote,
		                        initial_m2_stat.number_of_LinphoneCallUpdatedByRemote + 1));

		int m2_defer_update =
		    !!linphone_config_get_int(linphone_core_get_config(m2->lc), "sip", "defer_update_default", FALSE);
		if (m2_defer_update == TRUE) {
			LinphoneCallParams *m2_params = linphone_core_create_call_params(m2->lc, m2_calls_m1);
			linphone_call_params_enable_video(m2_params, enable_video);
			linphone_call_accept_update(m2_calls_m1, m2_params);
			linphone_call_params_unref(m2_params);
		}
		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallUpdating,
		                        initial_m1_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_m2_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallStreamsRunning,
		                        initial_m1_stat.number_of_LinphoneCallStreamsRunning + 1));

		counter = 0;
		int idx = 1;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_participant_devices_media_capability_changed,
			    initial_participants_stats[counter].number_of_participant_devices_media_capability_changed + 1, 3000));
			if ((m != m1) && (m != m2)) {
				LinphoneConference *m_conference =
				    linphone_core_search_conference(m->lc, NULL, NULL, conference_address, NULL);
				BC_ASSERT_PTR_NOT_NULL(m_conference);
				LinphoneCall *m_calls_m2 = linphone_core_get_call_by_remote_address2(m->lc, m2->identity);
				BC_ASSERT_PTR_NOT_NULL(m_calls_m2);
				if (m_calls_m2 && m_conference) {
					const LinphoneCallParams *call_params = linphone_call_get_current_params(m_calls_m2);
					if (!!linphone_call_params_video_enabled(call_params)) {
						idx++;
						BC_ASSERT_TRUE(wait_for_list(lcs, &m2->stat.number_of_LinphoneCallUpdatedByRemote,
						                             initial_m2_stat.number_of_LinphoneCallUpdatedByRemote + idx,
						                             3000));
					}
				}
			}
			counter++;
		}

		counter = 0;
		idx = 0;

		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneConference *m_conference =
			    linphone_core_search_conference(m->lc, NULL, NULL, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(m_conference);
			if (m != m2) {
				LinphoneCall *m_calls_m2 = linphone_core_get_call_by_remote_address2(m->lc, m2->identity);
				BC_ASSERT_PTR_NOT_NULL(m_calls_m2);
				if (m_calls_m2 && m_conference) {
					const LinphoneCallParams *call_params = linphone_call_get_current_params(m_calls_m2);
					if (!!linphone_call_params_video_enabled(call_params)) {
						if (m != m1) {
							int defer_update = !!linphone_config_get_int(linphone_core_get_config(m2->lc), "sip",
							                                             "defer_update_default", FALSE);
							if (defer_update == TRUE) {
								LinphoneCall *m2_calls_m =
								    linphone_core_get_call_by_remote_address2(m2->lc, m->identity);
								LinphoneCallParams *m2_params = linphone_core_create_call_params(m2->lc, m2_calls_m);
								linphone_call_accept_update(m2_calls_m, m2_params);
								linphone_call_params_unref(m2_params);
							}
						}
						idx++;

						BC_ASSERT_TRUE(wait_for_list(
						    lcs, &m->stat.number_of_LinphoneCallUpdating,
						    initial_participants_stats[counter].number_of_LinphoneCallUpdating + 1, 5000));
						BC_ASSERT_TRUE(wait_for_list(
						    lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
						    initial_participants_stats[counter].number_of_LinphoneCallStreamsRunning + 1, 5000));
						BC_ASSERT_TRUE(wait_for_list(lcs, &m2->stat.number_of_LinphoneCallStreamsRunning,
						                             initial_m2_stat.number_of_LinphoneCallStreamsRunning + idx, 5000));
					}
				}
			}

			if (m_conference) {
				BC_ASSERT_TRUE(linphone_conference_is_in(m_conference));
			}
			counter++;
		}

		// Check video parameters
		if (m1_calls_m2) {
			const LinphoneCallParams *m1_call_params = linphone_call_get_current_params(m1_calls_m2);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(m1_call_params) == enable_video);
		}
		if (m2_calls_m1) {
			const LinphoneCallParams *m2_call_params = linphone_call_get_current_params(m2_calls_m1);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(m2_call_params) == enable_video);
		}
	}

	if (initial_participants_stats) {
		ms_free(initial_participants_stats);
	}

	if (m1_conference) {
		// Verify that video capabilities are still enabled
		const LinphoneConferenceParams *params = linphone_conference_get_current_params(m1_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == expected_m1_video_capability);
		if (linphone_core_conference_server_enabled(m1->lc)) {
			BC_ASSERT_TRUE(linphone_conference_is_in(m1_conference) ==
			               linphone_conference_params_local_participant_enabled(params));
		} else {
			BC_ASSERT_TRUE(linphone_conference_is_in(m1_conference));
		}
	}

	if (m2_conference) {
		// Verify that video capabilities are still enabled
		const LinphoneConferenceParams *params = linphone_conference_get_current_params(m2_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == expected_m2_video_capability);
		if (linphone_core_conference_server_enabled(m2->lc)) {
			BC_ASSERT_TRUE(linphone_conference_is_in(m2_conference) ==
			               linphone_conference_params_local_participant_enabled(params));
		} else {
			BC_ASSERT_TRUE(linphone_conference_is_in(m2_conference));
		}
	}

	if (lcs) {
		bctbx_list_free(lcs);
	}
}

static void
set_video_in_conference(bctbx_list_t *lcs, LinphoneCoreManager *conf, bctbx_list_t *participants, bool_t enable_video) {
	LinphoneConference *conference = linphone_core_get_conference(conf->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	stats *initial_stats = NULL;
	bool_t *initial_video_call = NULL;
	size_t *initial_video_streams = NULL;
	int idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_stats = (stats *)realloc(initial_stats, (idx + 1) * sizeof(stats));
		initial_video_call = (bool_t *)realloc(initial_video_call, (idx + 1) * sizeof(bool_t));
		initial_video_streams = (size_t *)realloc(initial_video_streams, (idx + 1) * sizeof(int));
		// Append element
		initial_stats[idx] = m->stat;
		LinphoneCall *call = linphone_core_get_call_by_remote_address2(conf->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		const LinphoneCallParams *params = linphone_call_get_params(call);
		initial_video_call[idx] = linphone_call_params_video_enabled(params);
		initial_video_streams[idx] = _linphone_call_get_nb_video_steams(call);

		idx++;
	}

	wait_for_list(lcs, NULL, 0, 2000);

	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	stats initial_conf_stat = conf->stat;
	if (conference) {
		const LinphoneConferenceParams *old_params = linphone_conference_get_current_params(conference);
		LinphoneConferenceParams *new_params = linphone_conference_params_clone(old_params);
		linphone_conference_params_enable_video(new_params, enable_video);

		BC_ASSERT_TRUE(linphone_conference_update_params(conference, new_params));
		linphone_conference_params_unref(new_params);
	}

	LinphoneConference *l_conference = linphone_core_get_conference(conf->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	unsigned int local_conf_participants = linphone_conference_get_participant_count(l_conference);
	BC_ASSERT_EQUAL(local_conf_participants, no_participants, int, "%d");

	idx = 0;
	int update_cnt = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;

		LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(c, conf->identity);
		BC_ASSERT_PTR_NOT_NULL(participant_call);

		if (initial_video_call[idx] != enable_video) {

			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdatedByRemote,
			                             initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1, 5000));

			int defer_update =
			    !!linphone_config_get_int(linphone_core_get_config(c), "sip", "defer_update_default", FALSE);
			if (defer_update == TRUE) {
				LinphoneCallParams *m_params = linphone_core_create_call_params(m->lc, participant_call);
				linphone_call_params_enable_video(m_params, enable_video);
				linphone_call_accept_update(participant_call, m_params);
				linphone_call_params_unref(m_params);
			}

			BC_ASSERT_TRUE(wait_for_list(lcs, &conf->stat.number_of_LinphoneCallUpdating,
			                             initial_conf_stat.number_of_LinphoneCallUpdating + update_cnt, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
			                             initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &conf->stat.number_of_LinphoneCallStreamsRunning,
			                             initial_conf_stat.number_of_LinphoneCallStreamsRunning + update_cnt, 5000));

			update_cnt++;
		}

		// Focus removed and added
		// Conference media capability changed
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_added,
		                             initial_stats[idx].number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_added,
		                             initial_stats[idx].number_of_participant_devices_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed,
		                             initial_stats[idx].number_of_participants_removed + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_removed,
		                             initial_stats[idx].number_of_participant_devices_removed + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_media_capability_changed,
		                             initial_stats[idx].number_of_participant_devices_media_capability_changed + 1,
		                             3000));

		// Wait for first frame if video is enabled
		if (enable_video) {
			// Make sure video is received for participants. For conference we can't because of missing APIs.*/
			liblinphone_tester_set_next_video_frame_decoded_cb(participant_call);
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_IframeDecoded,
			                             initial_stats[idx].number_of_IframeDecoded + 1, 5000));
		}

		// Remote  conference
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));

		const LinphoneCallParams *participant_call_params = linphone_call_get_current_params(participant_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(participant_call_params) == enable_video);

		LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		const LinphoneCallParams *conf_call_params = linphone_call_get_current_params(conf_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(conf_call_params) == enable_video);

		LinphoneConference *pconference = linphone_call_get_conference(participant_call);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			bctbx_list_t *pconf_participants = linphone_conference_get_participant_list(pconference);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(pconf_participants), no_participants, unsigned int, "%u");
			bctbx_list_free_with_data(pconf_participants, (void (*)(void *))linphone_participant_unref);
		}

		idx++;
	}

	wait_for_list(lcs, NULL, 0, 1000);

	const LinphoneCall *lcall = linphone_conference_get_call(l_conference);
	const LinphoneCallParams *lcall_local_params = linphone_call_get_params(lcall);
	const LinphoneConferenceLayout layout = linphone_call_params_get_conference_video_layout(lcall_local_params);
	const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(l_conference);
	const bool_t video_enabled = !!linphone_conference_params_video_enabled(conf_params);
	BC_ASSERT_TRUE(video_enabled == enable_video);
	const LinphoneAddress *local_conference_address = linphone_conference_get_conference_address(conference);
	const int nb_audio_streams = 1;
	int nb_video_streams = 0;

	idx = 0;
	bool_t conf_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(conf->lc), "misc", "conference_event_log_enabled", TRUE);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		bool_t p_event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
		if (p_event_log_enabled && conf_event_log_enabled) {
			LinphoneAddress *m_uri = linphone_address_new(linphone_core_get_identity(m->lc));
			LinphoneConference *pconference =
			    linphone_core_search_conference(m->lc, NULL, m_uri, local_conference_address, NULL);
			linphone_address_unref(m_uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				check_conference_medias(conference, pconference);
			}
		}

		if (video_enabled) {
			// One stream per participant
			// One stream for the active speaker layout
			// One stream for the local participant
			nb_video_streams = local_conf_participants + ((layout == LinphoneConferenceLayoutActiveSpeaker) ? 1 : 0) +
			                   (linphone_conference_is_in(conference) ? 1 : 0);
		} else {
			nb_video_streams = (int)initial_video_streams[idx];
		}

		check_nb_streams(conf, m, nb_audio_streams, nb_video_streams, 0);
		idx++;
	}

	ms_free(initial_stats);
	ms_free(initial_video_call);
	ms_free(initial_video_streams);

	const LinphoneConferenceParams *params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == enable_video);
}

static void check_conference_volumes(LinphoneCall *call) {
	LinphoneConference *conference = linphone_call_get_conference(call);

	if (conference) {
		bctbx_list_t *participants = linphone_conference_get_participant_list(conference);

		for (bctbx_list_t *it = participants; it != NULL; it = it->next) {
			LinphoneParticipant *p = (LinphoneParticipant *)it->data;
			bctbx_list_t *devices = linphone_participant_get_devices(p);

			for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
				uint32_t audioSsrc = linphone_participant_device_get_ssrc(d, LinphoneStreamTypeAudio);
				if (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeAudio) ==
				    LinphoneMediaDirectionInactive) {
					BC_ASSERT_EQUAL((unsigned long)audioSsrc, 0, unsigned long, "%0lu");
				} else {
					BC_ASSERT_NOT_EQUAL((unsigned long)audioSsrc, 0, unsigned long, "%0lu");
					if (linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeAudio)) {
						const int volume = linphone_conference_get_participant_device_volume(conference, d);
						BC_ASSERT_NOT_EQUAL(volume, AUDIOSTREAMVOLUMES_NOT_FOUND, int, "%d");
						BC_ASSERT_GREATER(volume, MS_VOLUME_DB_LOWEST, int, "%d");
					}
				}
				uint32_t videoSsrc = linphone_participant_device_get_ssrc(d, LinphoneStreamTypeVideo);
				if (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				    LinphoneMediaDirectionInactive) {
					BC_ASSERT_EQUAL((unsigned long)videoSsrc, 0, unsigned long, "%0lu");
				} else {
					BC_ASSERT_NOT_EQUAL((unsigned long)videoSsrc, 0, unsigned long, "%0lu");
				}
			}
			bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
		}
		bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);
	}

	AudioStream *audiostream = (AudioStream *)linphone_call_get_stream(call, LinphoneStreamTypeAudio);
	BC_ASSERT_EQUAL(audiostream->mixer_to_client_extension_id, 3, int, "%d");
}

static void simple_conference_base(LinphoneCoreManager *marie,
                                   LinphoneCoreManager *pauline,
                                   LinphoneCoreManager *laure,
                                   LinphoneCoreManager *focus,
                                   bool_t pause_and_hangup) {
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;

	char *filepath = bc_tester_res("sounds/vrroom.wav");

	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneConference *conference = NULL;
	LinphoneAddress *marie_conference_address = NULL;
	const bctbx_list_t *calls;
	bool_t is_remote_conf;
	bool_t focus_is_up = (focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	if (focus) lcs = bctbx_list_append(lcs, focus->lc);

	is_remote_conf =
	    (strcmp(linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "conference_type", "local"),
	            "remote") == 0);
	if (is_remote_conf) BC_ASSERT_PTR_NOT_NULL(focus);

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;
	initial_laure_stat = laure->stat;

	marie_call_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *new_participants = bctbx_list_append(NULL, laure);
	// TODO: Find a way to extract participants managers from conference
	bctbx_list_t *lcs2 = bctbx_list_append(NULL, marie->lc);
	lcs2 = bctbx_list_append(lcs2, laure->lc);

	if (!is_remote_conf) {
		add_calls_to_local_conference(lcs2, marie, NULL, new_participants, FALSE);

		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(marie_call_laure));
		BC_ASSERT_TRUE(linphone_call_is_in_conference(marie_call_laure));
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(marie_call_pauline));
	} else {
		if (focus_is_up) {
			lcs2 = bctbx_list_append(lcs2, focus->lc);
			add_calls_to_remote_conference(lcs2, focus, marie, new_participants, NULL, FALSE);

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		} else {
			linphone_core_add_to_conference(marie->lc, marie_call_laure);
			linphone_call_terminate(marie_call_pauline);
			linphone_call_terminate(marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
			                             initial_marie_stat.number_of_LinphoneCallEnd + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
			                             initial_pauline_stat.number_of_LinphoneCallEnd + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd,
			                             initial_laure_stat.number_of_LinphoneCallEnd + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
			                             initial_marie_stat.number_of_LinphoneCallReleased + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
			                             initial_pauline_stat.number_of_LinphoneCallReleased + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
			                             initial_laure_stat.number_of_LinphoneCallReleased + 1, 5000));

			BC_ASSERT_EQUAL(linphone_core_terminate_conference(marie->lc), 0, int, "%d");
			BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
			bctbx_list_free(lcs2);
			bctbx_list_free(new_participants);
			goto end;
		}
	}
	bctbx_list_free(lcs2);
	bctbx_list_free(new_participants);

	new_participants = bctbx_list_append(NULL, pauline);
	if (!is_remote_conf) {
		add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(marie_call_laure));
		BC_ASSERT_TRUE(linphone_call_is_in_conference(marie_call_laure));
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(marie_call_pauline));
		BC_ASSERT_TRUE(linphone_call_is_in_conference(marie_call_pauline));
	} else {
		add_calls_to_remote_conference(lcs, focus, marie, new_participants, NULL, TRUE);

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	}
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_marie_stat.number_of_LinphoneCallStreamsRunning + 2, 3000));

	bool_t marie_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_log_enabled", TRUE);
	if (marie_event_log_enabled) {
		bool_t marie_conference_server = linphone_core_conference_server_enabled(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive,
		                             (initial_marie_stat.number_of_LinphoneSubscriptionActive + 1), 5000));
		if (!marie_conference_server) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyFullStateReceived,
			                             (initial_marie_stat.number_of_NotifyFullStateReceived + 1),
			                             liblinphone_tester_sip_timeout));
		}
	}
	bool_t laure_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(laure->lc), "misc", "conference_event_log_enabled", TRUE);
	if (laure_event_log_enabled && marie_event_log_enabled) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionActive,
		                             (initial_laure_stat.number_of_LinphoneSubscriptionActive + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_NotifyFullStateReceived,
		                             (initial_laure_stat.number_of_NotifyFullStateReceived + 1),
		                             liblinphone_tester_sip_timeout));
	}
	bool_t pauline_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_log_enabled", TRUE);
	if (pauline_event_log_enabled && marie_event_log_enabled) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive,
		                             (initial_pauline_stat.number_of_LinphoneSubscriptionActive + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyFullStateReceived,
		                             (initial_pauline_stat.number_of_NotifyFullStateReceived + 1),
		                             liblinphone_tester_sip_timeout));
	}

	linphone_core_enable_mic(pauline->lc, FALSE);
	linphone_core_enable_mic(laure->lc, FALSE);
	linphone_core_enable_mic(marie->lc, FALSE);
	// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
	wait_for_list(lcs, NULL, 0, liblinphone_tester_sip_timeout);

	// Let all participant to speak simultaneously to send their volumes
	int talking_time = 4000;
	linphone_core_enable_mic(pauline->lc, TRUE);
	linphone_core_enable_mic(laure->lc, TRUE);
	linphone_core_enable_mic(marie->lc, TRUE);
	linphone_core_set_play_file(pauline->lc, filepath);
	linphone_core_set_play_file(marie->lc, filepath);
	linphone_core_set_play_file(laure->lc, filepath);
	wait_for_list(lcs, NULL, 0, talking_time);

	// Check that laure received volumes from other participant's devices
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	LinphoneConference *laure_conference = (laure_call) ? linphone_call_get_conference(laure_call) : NULL;
	BC_ASSERT_PTR_NOT_NULL(laure_conference);
	if (laure_conference) {
		BC_ASSERT_PTR_NOT_NULL(linphone_conference_get_call(laure_conference));
		LinphoneParticipant *marie_participant =
		    linphone_conference_find_participant(laure_conference, marie->identity);
		if (marie_event_log_enabled) {
			BC_ASSERT_PTR_NOT_NULL(marie_participant);
		} else {
			BC_ASSERT_PTR_NULL(marie_participant);
		}
		if (marie_participant) {
			BC_ASSERT_TRUE(linphone_participant_is_admin(marie_participant));
		}
	}
	if (laure_call) {
		liblinphone_tester_check_rtcp_2(((is_remote_conf) ? focus : marie), laure);
		check_conference_volumes(laure_call);
	}

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	LinphoneConference *pauline_conference = (pauline_call) ? linphone_call_get_conference(pauline_call) : NULL;
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	if (pauline_conference) {
		BC_ASSERT_PTR_NOT_NULL(linphone_conference_get_call(pauline_conference));
		LinphoneParticipant *marie_participant =
		    linphone_conference_find_participant(pauline_conference, marie->identity);
		if (pauline_event_log_enabled && marie_event_log_enabled) {
			BC_ASSERT_PTR_NOT_NULL(marie_participant);
		} else {
			BC_ASSERT_PTR_NULL(marie_participant);
		}
		if (marie_participant) {
			BC_ASSERT_TRUE(linphone_participant_is_admin(marie_participant));
		}
	}
	if (pauline_call) {
		liblinphone_tester_check_rtcp_2(((is_remote_conf) ? focus : marie), pauline);
		check_conference_volumes(pauline_call);
	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	if (l_conference) {
		BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");
		LinphoneParticipant *marie_participant = linphone_conference_get_me(l_conference);
		BC_ASSERT_PTR_NOT_NULL(marie_participant);
		if (marie_participant) {
			BC_ASSERT_TRUE(linphone_participant_is_admin(marie_participant));
		}
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));

	/*
	 * FIXME: check_ice() cannot work as it is today because there is no current call for the party that hosts the
	conference if (linphone_core_get_firewall_policy(marie->lc) == LinphonePolicyUseIce) { if
	(linphone_core_get_firewall_policy(pauline->lc) == LinphonePolicyUseIce) {
	        check_ice(marie,pauline,LinphoneIceStateHostConnection);
	    }
	    if (linphone_core_get_firewall_policy(laure->lc) == LinphonePolicyUseIce) {
	        check_ice(marie,laure,LinphoneIceStateHostConnection);
	    }
	}
	*/
	for (calls = linphone_core_get_calls(marie->lc); calls != NULL; calls = calls->next) {
		LinphoneCall *call = (LinphoneCall *)calls->data;
		BC_ASSERT_EQUAL(linphone_core_get_media_encryption(marie->lc),
		                linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)), int, "%d");
	}

	BC_ASSERT_PTR_NOT_NULL(conference = linphone_core_get_conference(marie->lc));
	if (conference) {
		marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(conference));
		bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
		bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);
	}

	if (pause_and_hangup) {
		if (!is_remote_conf) {
			// Marie pauses the call with Laure, therefore she leaves the conference
			// As only Marie and Pauline are left in the conference, it becomes a simple call, therefore Pauline's
			// session is updated
			stats marie_stat = marie->stat;
			stats laure_stat = laure->stat;
			stats pauline_stat = pauline->stat;
			BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(marie_call_laure));
			BC_ASSERT_TRUE(linphone_call_is_in_conference(marie_call_laure));
			linphone_core_pause_call(marie->lc, marie_call_laure);

			bool_t event_log_enabled = linphone_config_get_bool(linphone_core_get_config(marie->lc), "misc",
			                                                    "conference_event_log_enabled", TRUE);
			if (event_log_enabled) {
				// Call between Marie and Laure
				BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
				                              marie_stat.number_of_LinphoneCallPausing + 1, 1000));
				BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
				                              marie_stat.number_of_LinphoneCallPaused + 1, 1000));
				BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote,
				                              laure_stat.number_of_LinphoneCallPausedByRemote + 1, 1000));

				remove_participant_from_local_conference(lcs, marie, laure, NULL);

				BC_ASSERT_PTR_NOT_NULL(marie_conference_address);
				if (marie_conference_address) {
					LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
					LinphoneConference *pauline_conference =
					    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
					BC_ASSERT_PTR_NULL(pauline_conference);
					linphone_address_unref(pauline_uri);
					BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
					                             pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,
					                             liblinphone_tester_sip_timeout));

					LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
					LinphoneConference *laure_conference =
					    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
					BC_ASSERT_PTR_NULL(laure_conference);
					linphone_address_unref(laure_uri);
					BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
					                             laure_stat.number_of_LinphoneSubscriptionTerminated + 1,
					                             liblinphone_tester_sip_timeout));

					LinphoneAddress *marie_uri = linphone_address_new(linphone_core_get_identity(marie->lc));
					LinphoneConference *marie_conference =
					    linphone_core_search_conference(marie->lc, NULL, marie_uri, marie_conference_address, NULL);
					BC_ASSERT_PTR_NULL(marie_conference);
					linphone_address_unref(marie_uri);
				}

			} else {
				// Call between Marie and Laure
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
				                             marie_stat.number_of_LinphoneCallPausing + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
				                             marie_stat.number_of_LinphoneCallPaused + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote,
				                             laure_stat.number_of_LinphoneCallPausedByRemote + 1,
				                             liblinphone_tester_sip_timeout));

				// Conference on Laure's side
				BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
				                             laure_stat.number_of_LinphoneConferenceStateTerminated + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
				                             laure_stat.number_of_LinphoneConferenceStateDeleted + 1, 5000));

				// Conference on Pauline's side
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
				                             5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
				                             pauline_stat.number_of_LinphoneConferenceStateTerminated + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
				                             pauline_stat.number_of_LinphoneConferenceStateDeleted + 1, 5000));

				// Conference on Marie's side
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
				                             marie_stat.number_of_LinphoneConferenceStateTerminated + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
				                             marie_stat.number_of_LinphoneConferenceStateDeleted + 1, 5000));

				// Call between Marie and Pauline
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating,
				                             marie_stat.number_of_LinphoneCallUpdating + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,
				                             pauline_stat.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
				                             marie_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
				                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));

				BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

				// Marie has 2 ongoing calls: one with Pauline and one with Laure
				const bctbx_list_t *marie_calls = linphone_core_get_calls(marie->lc);
				BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 2, int, "%i");

				LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
				LinphoneConference *pauline_conference =
				    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
				BC_ASSERT_PTR_NULL(pauline_conference);
				linphone_address_unref(pauline_uri);

				LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
				LinphoneConference *laure_conference =
				    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
				BC_ASSERT_PTR_NULL(laure_conference);
				linphone_address_unref(laure_uri);

				LinphoneAddress *marie_uri = linphone_address_new(linphone_core_get_identity(marie->lc));
				LinphoneConference *marie_conference =
				    linphone_core_search_conference(marie->lc, NULL, marie_uri, marie_conference_address, NULL);
				BC_ASSERT_PTR_NULL(marie_conference);
				linphone_address_unref(marie_uri);
			}

			linphone_core_terminate_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
			linphone_call_terminate(marie_call_pauline);
		} else {
			linphone_core_pause_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote, 1,
			                             liblinphone_tester_sip_timeout));
			linphone_core_terminate_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, is_remote_conf ? 2 : 1,
			                             liblinphone_tester_sip_timeout));
		}
	} else {
		bctbx_list_t *participants = bctbx_list_append(NULL, laure);
		participants = bctbx_list_append(participants, pauline);
		if (is_remote_conf) {
			terminate_conference(participants, marie, NULL, focus, FALSE);
		} else {
			terminate_conference(participants, marie, NULL, NULL, FALSE);
		}
		bctbx_list_free(participants);
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, is_remote_conf ? 2 : 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, is_remote_conf ? 3 : 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, is_remote_conf ? 2 : 1,
	                             liblinphone_tester_sip_timeout));
	if (is_remote_conf)
		BC_ASSERT_TRUE(wait_for_list(lcs, &focus->stat.number_of_LinphoneCallEnd, 3, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, is_remote_conf ? 2 : 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, is_remote_conf ? 3 : 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, is_remote_conf ? 2 : 1,
	                             liblinphone_tester_sip_timeout));
	if (is_remote_conf)
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &focus->stat.number_of_LinphoneCallReleased, 3, liblinphone_tester_sip_timeout));

end:
	if (filepath) bctbx_free(filepath);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	// Wait for all conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));

	// If focus is not registered, the conference is not attached to any core, hence it cannot be destroyed
	if (!(is_remote_conf && !focus_is_up)) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
		                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
		                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	}

	if (focus_is_up) {
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			if (c != focus->lc) {
				LinphoneCoreManager *mgr = get_manager(c);
				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), ((c == marie->lc) ? 3 : 2), unsigned int,
				                "%u");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), 1, unsigned int, "%u");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}
	}

	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
}

static void on_speaking_notified(LinphoneParticipantDevice *participant_device, bool_t is_speaking) {
	stats *stat = get_stats((LinphoneCore *)linphone_participant_device_get_user_data(participant_device));
	if (is_speaking) {
		stat->number_of_LinphoneParticipantDeviceStartSpeaking++;
	} else {
		stat->number_of_LinphoneParticipantDeviceStopSpeaking++;
	}
}

static void simple_conference_notify_speaking_device(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneAddress *marie_conference_address = NULL;
	/* Simulate speaker changed */
	char *filepath = bc_tester_res("sounds/vrroom.wav");
	linphone_core_set_play_file(pauline->lc, filepath);
	linphone_core_set_play_file(marie->lc, filepath);
	// linphone_core_set_play_file(laure->lc, filepath);

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));
	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *new_participants = bctbx_list_append(NULL, laure);
	new_participants = bctbx_list_append(new_participants, pauline);

	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;
	initial_laure_stat = laure->stat;
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_marie_stat.number_of_LinphoneCallStreamsRunning + 2, 3000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote,
	                              initial_laure_stat.number_of_LinphoneCallPausedByRemote + 1, 2000));
	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCallPausedByRemote, 0, int, "%d");

	// Set Laure is_speaking callback for all participants to check that pauline is speaking
	LinphoneConference *laure_conf = linphone_call_get_conference(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(laure_conf);
	if (!laure_conf) goto end;
	bctbx_list_t *participants = linphone_conference_get_participant_list(laure_conf);

	for (bctbx_list_t *it = participants; it != NULL; it = it->next) {
		LinphoneParticipant *p = (LinphoneParticipant *)it->data;
		bctbx_list_t *devices = linphone_participant_get_devices(p);
		for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
			LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
			linphone_participant_device_set_user_data(d, laure->lc);
			LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
			linphone_participant_device_cbs_set_is_speaking_changed(cbs, on_speaking_notified);
			linphone_participant_device_add_callbacks(d, cbs);
			linphone_participant_device_cbs_unref(cbs);
		}
		bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
	}
	bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

	// Set Pauline is_speaking callback for herself to check that she is speaking
	LinphoneConference *pauline_conf = linphone_call_get_conference(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(pauline_conf);
	if (!pauline_conf) goto end;

	LinphoneParticipant *p = linphone_conference_get_me(pauline_conf);
	bctbx_list_t *devices = linphone_participant_get_devices(p);
	for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
		LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
		linphone_participant_device_set_user_data(d, pauline->lc);
		LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
		linphone_participant_device_cbs_set_is_speaking_changed(cbs, on_speaking_notified);
		linphone_participant_device_add_callbacks(d, cbs);
		linphone_participant_device_cbs_unref(cbs);
	}
	bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);

	// Need time to be notified
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneParticipantDeviceStartSpeaking, 2, 20000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneParticipantDeviceStopSpeaking, 2, 20000));

	// No need to wait as much this time as pauline should also be notified at the same time
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneParticipantDeviceStartSpeaking, 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneParticipantDeviceStopSpeaking, 1,
	                             liblinphone_tester_sip_timeout));

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));

	bctbx_list_free(new_participants);

end:
	// Wait for all conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));

	if (filepath) bctbx_free(filepath);
	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

void on_muted_notified(LinphoneParticipantDevice *participant_device, bool_t is_muted) {
	stats *stat = get_stats((LinphoneCore *)linphone_participant_device_get_user_data(participant_device));
	if (is_muted) {
		stat->number_of_LinphoneParticipantDeviceMuted++;
	} else {
		stat->number_of_LinphoneParticipantDeviceUnmuted++;
	}
}

static void simple_conference_notify_muted_device(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneAddress *marie_conference_address = NULL;
	/* Simulate speaker changed */
	char *filepath = bc_tester_res("sounds/vrroom.wav");
	linphone_core_set_play_file(pauline->lc, filepath);
	linphone_core_set_play_file(marie->lc, filepath);
	linphone_core_set_play_file(laure->lc, filepath);

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));
	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *new_participants = bctbx_list_append(NULL, laure);
	new_participants = bctbx_list_append(new_participants, pauline);

	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;
	initial_laure_stat = laure->stat;
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_marie_stat.number_of_LinphoneCallStreamsRunning + 2, 3000));

	LinphoneConference *laure_conf = linphone_call_get_conference(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(laure_conf);
	if (!laure_conf) goto end;
	bctbx_list_t *participant_devices = linphone_conference_get_participant_device_list(laure_conf);

	for (bctbx_list_t *it = participant_devices; it != NULL; it = it->next) {
		LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it->data;
		linphone_participant_device_set_user_data(d, laure->lc);
		LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
		linphone_participant_device_cbs_set_is_muted(cbs, on_muted_notified);
		linphone_participant_device_add_callbacks(d, cbs);
		linphone_participant_device_cbs_unref(cbs);
	}
	bctbx_list_free_with_data(participant_devices, (void (*)(void *))linphone_participant_device_unref);

	linphone_core_enable_mic(pauline->lc, FALSE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneParticipantDeviceMuted, 1, 5000));

	linphone_core_enable_mic(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneParticipantDeviceUnmuted, 1, 5000));

	linphone_core_enable_mic(laure->lc, FALSE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneParticipantDeviceMuted, 2, 5000));

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));

	bctbx_list_free(new_participants);

end:
	// Wait for all conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             laure->stat.number_of_LinphoneConferenceStateCreated, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));

	if (filepath) bctbx_free(filepath);
	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void simple_conference_with_admin_changed(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	linphone_core_enable_conference_server(((LinphoneCoreManager *)focus)->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	LinphoneCall *marie_call_chloe;
	LinphoneCall *chloe_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, chloe->lc);
	lcs = bctbx_list_append(lcs, ((LinphoneCoreManager *)focus)->lc);

	BC_ASSERT_TRUE(focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, chloe));
	marie_call_chloe = linphone_core_get_current_call(marie->lc);
	chloe_called_by_marie = linphone_core_get_current_call(chloe->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_chloe, chloe, chloe_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, chloe);
	add_calls_to_remote_conference(lcs, (LinphoneCoreManager *)focus, marie, participants, NULL, TRUE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);

	if (!marie_conference) goto end;

	stats *initial_participants_stats = NULL;
	int idx = 0;
	int counter = 0;

	// Marie change Pauline's status to admin
	LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *pauline_participant = linphone_conference_find_participant(marie_conference, pauline_uri);
	BC_ASSERT_PTR_NOT_NULL(pauline_participant);
	if (pauline_participant) {
		BC_ASSERT_FALSE(linphone_participant_is_admin(pauline_participant));

		counter = 1;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			// Allocate memory
			initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));

			// Append element
			initial_participants_stats[counter - 1] = m->stat;

			// Increment counter
			counter++;
		}
		stats initial_marie = marie->stat;

		linphone_conference_set_participant_admin_status(marie_conference, pauline_participant, TRUE);
		// Participants should receive the admin changed notification:
		idx = 0;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_participant_admin_statuses_changed,
			    (initial_participants_stats[idx].number_of_participant_admin_statuses_changed + 1), 3000));
			idx++;
		}
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_admin_statuses_changed,
		                             initial_marie.number_of_participant_admin_statuses_changed + 1, 3000));

		ms_free(initial_participants_stats);
		initial_participants_stats = NULL;

		BC_ASSERT_TRUE(linphone_participant_is_admin(pauline_participant));
	}

	// Pauline removes Marie as admin
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
	linphone_address_unref(pauline_uri);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	if (pauline_conference) {
		bctbx_list_t *participants_before_removal = linphone_conference_get_participant_list(marie_conference);
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants_before_removal),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		bctbx_list_free_with_data(participants_before_removal, (void (*)(void *))linphone_participant_unref);

		const char *marie_uri = linphone_core_get_identity(marie->lc);
		LinphoneAddress *marie_address = linphone_address_new(marie_uri);
		LinphoneParticipant *marie_participant =
		    linphone_conference_find_participant(pauline_conference, marie_address);
		BC_ASSERT_PTR_NOT_NULL(marie_participant);

		counter = 1;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			// Allocate memory
			initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));

			// Append element
			initial_participants_stats[counter - 1] = m->stat;

			// Increment counter
			counter++;
		}

		linphone_conference_set_participant_admin_status(pauline_conference, marie_participant, FALSE);
		// Participants should receive the admin changed notification:
		idx = 0;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_participant_admin_statuses_changed,
			    (initial_participants_stats[idx].number_of_participant_admin_statuses_changed + 1), 3000));
			idx++;
		}

		ms_free(initial_participants_stats);
		initial_participants_stats = NULL;

		BC_ASSERT_FALSE(linphone_participant_is_admin(marie_participant));
		linphone_address_unref(marie_address);
	}

	participants = bctbx_list_remove(participants, pauline);

	counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));

		// Append element
		initial_participants_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	// Pauline ends all calls therefore Pauline exits the conference
	// A new admin must be designated
	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &((LinphoneCoreManager *)focus)->stat.number_of_LinphoneCallEnd, 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &((LinphoneCoreManager *)focus)->stat.number_of_LinphoneCallReleased, 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 1000));
	// Participants should have received 3 notifications:
	// - participant removed
	// - participant device removed
	// - admin rights of participant exiting from the conference are removed
	idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed,
		                             (initial_participants_stats[idx].number_of_participants_removed + 1), 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_removed,
		                             (initial_participants_stats[idx].number_of_participant_devices_removed + 1),
		                             3000));
		idx++;
	}

	ms_free(initial_participants_stats);

	const LinphoneAddress *conference_address = linphone_conference_get_conference_address(marie_conference);

	// Check that there only one admin in each conference and it has the same address for all of them
	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	LinphoneAddress *admin_address = NULL;
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference = linphone_core_search_conference(m->lc, NULL, uri, conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);

		if (conference) {
			bctbx_list_t *participants_after_removal = linphone_conference_get_participant_list(conference);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants_after_removal),
			                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");

			LinphoneParticipant *me = linphone_conference_get_me(conference);
			bool_t admin_found = linphone_participant_is_admin(me);

			// Check that participant address that is admin is the same in all chat rooms
			for (bctbx_list_t *it = participants_after_removal; it; it = bctbx_list_next(it)) {
				LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(it);
				bool_t isAdmin = linphone_participant_is_admin(p);
				admin_found |= isAdmin;
				if (isAdmin) {
					if (!admin_address) {
						admin_address = (LinphoneAddress *)(linphone_participant_get_address(p));
					} else {
						const LinphoneAddress *other_admin_address = linphone_participant_get_address(p);
						char *other_admin_address_str = linphone_address_as_string(other_admin_address);
						char *admin_address_str = linphone_address_as_string(admin_address);
						BC_ASSERT_TRUE(strcmp(admin_address_str, other_admin_address_str) == 0);
						ms_free(admin_address_str);
						ms_free(other_admin_address_str);
					}
				}
			}
			BC_ASSERT_FALSE(admin_found);
			bctbx_list_free_with_data(participants_after_removal, (void (*)(void *))linphone_participant_unref);
		}
	}
	bctbx_list_free(all_manangers_in_conf);

	terminate_conference(participants, marie, marie_conference, (LinphoneCoreManager *)focus, FALSE);

end:
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_participant_removal_from_non_admin(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	linphone_core_enable_conference_server(((LinphoneCoreManager *)focus)->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	LinphoneCall *marie_call_chloe;
	LinphoneCall *chloe_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, chloe->lc);
	lcs = bctbx_list_append(lcs, ((LinphoneCoreManager *)focus)->lc);

	BC_ASSERT_TRUE(focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, chloe));
	marie_call_chloe = linphone_core_get_current_call(marie->lc);
	chloe_called_by_marie = linphone_core_get_current_call(chloe->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_chloe, chloe, chloe_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, chloe);
	add_calls_to_remote_conference(lcs, (LinphoneCoreManager *)focus, marie, participants, NULL, FALSE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	// Pauline tries to remove Marie from conference
	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
	linphone_address_unref(pauline_uri);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	BC_ASSERT_FALSE(linphone_participant_is_admin(linphone_conference_get_me(pauline_conference)));

	LinphoneAddress *marie_uri = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marie_participant = linphone_conference_find_participant(pauline_conference, marie_uri);
	BC_ASSERT_PTR_NOT_NULL(marie_participant);
	linphone_address_unref(marie_uri);

	BC_ASSERT_TRUE(linphone_participant_is_admin(marie_participant));

	// Pauline tries to remove Marie. As it is not admin, this operation should not be possible hence the number of
	// participants should be the same
	linphone_conference_remove_participant_2(pauline_conference, marie_participant);
	// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
	wait_for_list(lcs, NULL, 0, 5000);

	// Number of participants should not have changed
	bctbx_list_t *participants_after_admin_attempted_removal =
	    linphone_conference_get_participant_list(pauline_conference);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants_after_admin_attempted_removal),
	                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
	bctbx_list_free_with_data(participants_after_admin_attempted_removal, (void (*)(void *))linphone_participant_unref);

	LinphoneAddress *michelle_uri = linphone_address_new(linphone_core_get_identity(michelle->lc));
	LinphoneParticipant *michelle_participant = linphone_conference_find_participant(pauline_conference, michelle_uri);
	BC_ASSERT_PTR_NOT_NULL(michelle_participant);
	linphone_address_unref(michelle_uri);

	BC_ASSERT_FALSE(linphone_participant_is_admin(michelle_participant));

	// Pauline tries to remove Marie. As it is not admin, this operation should not be possible hence the number of
	// participants should be the same
	linphone_conference_remove_participant_2(pauline_conference, michelle_participant);
	// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
	wait_for_list(lcs, NULL, 0, 5000);

	// Number of participants should not have changed
	bctbx_list_t *participants_after_non_admin_attempted_removal =
	    linphone_conference_get_participant_list(pauline_conference);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants_after_non_admin_attempted_removal),
	                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
	bctbx_list_free_with_data(participants_after_non_admin_attempted_removal,
	                          (void (*)(void *))linphone_participant_unref);

	terminate_conference(participants, marie, NULL, (LinphoneCoreManager *)focus, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_participant_addition_from_non_admin(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	LinphoneCall *michelle_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, ((LinphoneCoreManager *)focus)->lc);

	BC_ASSERT_TRUE(focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	linphone_call_set_microphone_muted(michelle_called_by_marie, TRUE);
	BC_ASSERT_TRUE(linphone_call_get_microphone_muted(michelle_called_by_marie));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);
	add_calls_to_remote_conference(lcs, (LinphoneCoreManager *)focus, marie, participants, NULL, TRUE);

	// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(marie_conference);

	// wait a bit to ensure that all devices became aware that Michelle joined the conference in mute
	wait_for_list(lcs, NULL, 0, 2000);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(c))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(c));
		LinphoneConference *m_conference =
		    linphone_core_search_conference(c, NULL, m_uri, marie_conference_address, NULL);
		linphone_address_unref(m_uri);
		BC_ASSERT_PTR_NOT_NULL(m_conference);
		if (m_conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(m_conference),
			                (unsigned int)bctbx_list_size(participants) +
			                    ((c == ((LinphoneCoreManager *)focus)->lc) ? 1 : 0),
			                unsigned int, "%u");
			BC_ASSERT_TRUE(linphone_conference_is_in(m_conference) == (c != ((LinphoneCoreManager *)focus)->lc));
			bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(m_conference);
			for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
				BC_ASSERT_PTR_NOT_NULL(d);
				if (d) {
					bool_t is_muted =
					    linphone_address_weak_equal(linphone_participant_device_get_address(d), michelle->identity);
					int part_counter = 0;
					do {
						part_counter++;
						wait_for_list(lcs, NULL, 0, 100);
					} while ((part_counter < 100) && ((!!linphone_participant_device_get_is_muted(d)) != is_muted));
					BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) == (!!is_muted));
				}
			}
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
		}
	}

	stats focus_stats = focus_mgr->stat;
	stats pauline_stats = pauline->stat;
	stats marie_stats = marie->stat;
	stats laure_stats = laure->stat;
	stats michelle_stats = michelle->stat;
	stats chloe_stats = chloe->stat;

	lcs = bctbx_list_append(lcs, chloe->lc);
	ms_message("%s calls %s", linphone_core_get_identity(pauline->lc), linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(call(pauline, chloe));
	LinphoneCall *chloe_called_by_pauline = linphone_core_get_call_by_remote_address2(pauline->lc, chloe->identity);
	BC_ASSERT_PTR_NOT_NULL(chloe_called_by_pauline);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
	                             pauline_stats.number_of_LinphoneCallPaused + 1, 5000));
	// Laure received the notification about Pauline's media capability changed
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_media_capability_changed,
	                             laure_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
	LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	linphone_address_unref(laure_uri);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);
	if (laure_conference) {
		// Check that number pf participants on Laure's side is unchanged
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(laure_conference), 1, int, "%d");
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 5000));
	LinphoneAddress *michelle_uri = linphone_address_new(linphone_core_get_identity(michelle->lc));
	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, michelle_uri, marie_conference_address, NULL);
	linphone_address_unref(michelle_uri);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);
	if (michelle_conference) {
		// Check that number pf participants on Pauline's side is unchanged
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(michelle_conference), 1, int, "%d");
	}

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, (marie_stats.number_of_NotifyReceived + 1), 5000));
	if (marie_conference) {
		// Check that number pf participants on Marie's side is unchanged
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(marie_conference), 1, int, "%d");
	}

	LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	if (pauline_conference && chloe_called_by_pauline) {
		// Pauline tries to add Chloe to the conference
		linphone_conference_add_participant(pauline_conference, chloe_called_by_pauline);

		// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
		wait_for_list(lcs, NULL, 0, 1000);

		BC_ASSERT_EQUAL(linphone_conference_is_in(pauline_conference), 0, int, "%d");
	}

	focus_stats = focus_mgr->stat;
	pauline_stats = pauline->stat;
	marie_stats = marie->stat;
	laure_stats = laure->stat;
	michelle_stats = michelle->stat;
	chloe_stats = chloe->stat;

	if (pauline_conference) {
		// Pauline rejoins conference
		linphone_conference_enter(pauline_conference);
	}

	// Call between Pauline and Marie is resumed as Pauline rejoins the conference
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallResuming,
	                             (pauline_stats.number_of_LinphoneCallResuming + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             (focus_stats.number_of_LinphoneCallStreamsRunning + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             (pauline_stats.number_of_LinphoneCallStreamsRunning + 1), 5000));

	// Call with Chloe is paused
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
	                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
	                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallPausedByRemote,
	                             (chloe_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionOutgoingProgress,
	                              (pauline_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1), 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive,
	                              (pauline_stats.number_of_LinphoneSubscriptionActive + 1), 1000));

	// Notify about pauline's media capability changes
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_NotifyReceived, (laure_stats.number_of_NotifyReceived + 1), 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 1000));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, (marie_stats.number_of_NotifyReceived + 1), 1000));

	if (marie_conference) {
		// Check that Marie was notified that Pauline rejoined the conference
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(marie_conference), 1, int, "%d");
	}
	if (michelle_conference) {
		// Check that Michelle was notified that Pauline rejoined the conference
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(michelle_conference), 1, int, "%d");
	}
	if (laure_conference) {
		// Check that Laure was notified that Pauline rejoined the conference
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(laure_conference), 1, int, "%d");
	}

	pauline_stats = pauline->stat;
	chloe_stats = chloe->stat;
	ms_message("%s terminates direct call to %s", linphone_core_get_identity(pauline->lc),
	           linphone_core_get_identity(chloe->lc));
	linphone_call_terminate(chloe_called_by_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc, chloe->lc, &pauline->stat.number_of_LinphoneCallEnd,
	                        pauline_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, chloe->lc, &chloe->stat.number_of_LinphoneCallEnd,
	                        chloe_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, chloe->lc, &pauline->stat.number_of_LinphoneCallReleased,
	                        pauline_stats.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, chloe->lc, &chloe->stat.number_of_LinphoneCallReleased,
	                        chloe_stats.number_of_LinphoneCallReleased + 1));

	pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	if (pauline_conference) {
		// Check that participant number has not changed
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference),
		                (unsigned int)bctbx_list_size(participants), unsigned int, "%u");
		BC_ASSERT_EQUAL(linphone_conference_is_in(pauline_conference), 1, int, "%d");
	}

	terminate_conference(participants, marie, NULL, (LinphoneCoreManager *)focus, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
	linphone_address_unref(pauline_uri);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_subject_change_from_non_admin(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(focus_mgr->lc);
	LinphoneProxyConfig *laure_proxy_config = linphone_core_get_default_proxy_config(laure->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, ((LinphoneCoreManager *)focus)->lc);

	BC_ASSERT_TRUE(focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);
	add_calls_to_remote_conference(lcs, (LinphoneCoreManager *)focus, marie, participants, NULL, TRUE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);

	int counter = 1;
	stats *initial_participants_stats = NULL;
	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));

		// Append element
		initial_participants_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	stats marie_stats = marie->stat;
	stats focus_stats = focus_mgr->stat;
	const char *original_subject = "SIGABRT (signal 6)";
	ms_message("%s is changing conference subject to %s", linphone_core_get_identity(marie->lc), original_subject);
	linphone_conference_set_subject(marie_conference, original_subject);
	// Participants should have received the subject change notification
	int idx = 0;

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             marie_stats.number_of_LinphoneCallStreamsRunning + 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             focus_stats.number_of_LinphoneCallStreamsRunning + 1, 3000));
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_subject_changed,
		                             (initial_participants_stats[idx].number_of_subject_changed + 1),
		                             liblinphone_tester_sip_timeout));

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);

		if (conference) {
			BC_ASSERT_STRING_EQUAL(original_subject, linphone_conference_get_subject(conference));
		}
		idx++;
	}
	ms_free(initial_participants_stats);

	LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
	linphone_address_unref(pauline_uri);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	const char *new_subject = "SEGFAULT (signal 11)";
	ms_message("%s is attempting to change conference subject to %s", linphone_core_get_identity(pauline->lc),
	           new_subject);
	linphone_conference_set_subject(pauline_conference, new_subject);
	// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
	wait_for_list(lcs, NULL, 0, 5000);

	const char *pauline_subject = linphone_conference_get_subject(pauline_conference);
	BC_ASSERT_PTR_NOT_NULL(pauline_subject);
	if (pauline_subject) {
		// Subject should not have changed
		BC_ASSERT_STRING_EQUAL(original_subject, pauline_subject);
	}

	terminate_conference(participants, marie, NULL, (LinphoneCoreManager *)focus, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(all_manangers_in_conf);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_one_participant_base(bool_t local) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	// marie creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_one_participant_conference(conf_params, TRUE);
	linphone_conference_params_enable_local_participant(conf_params, local);
	LinphoneConference *conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	const LinphoneConferenceParams *actual_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(actual_conf_params) == local);
	BC_ASSERT_TRUE(linphone_conference_params_one_participant_conference_enabled(actual_conf_params));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 0, int, "%0d");

	add_calls_to_local_conference(lcs, marie, conf, participants, TRUE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	actual_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(actual_conf_params) == local);
	BC_ASSERT_TRUE(linphone_conference_params_one_participant_conference_enabled(actual_conf_params));
	BC_ASSERT_TRUE(linphone_conference_is_in(conf) == local);

	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(conf);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(m->lc))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, m_uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(m_uri);
		if (conference) {
			int no_parts = (local || (m == marie)) ? 3 : 2;
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
		}
	}

	bctbx_list_t *lcs2 = bctbx_list_copy(lcs);
	remove_participant_from_local_conference(lcs2, marie, pauline, conf);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	lcs2 = bctbx_list_remove(lcs2, pauline->lc);
	all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, pauline);

	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		int no_parts = (local || (m == marie)) ? 2 : 1;
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed, 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_removed, 1, 3000));

		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(m->lc))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, m_uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(m_uri);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
		}
	}

	remove_participant_from_local_conference(lcs2, marie, michelle, conf);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	lcs2 = bctbx_list_remove(lcs2, michelle->lc);
	all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, michelle);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed, 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_removed, 2, 3000));
		int no_parts = (local || (m == marie)) ? 1 : 0;
		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(m->lc))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, m_uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(m_uri);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
		}
	}

	remove_participant_from_local_conference(lcs2, marie, laure, conf);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	// wait a bit to have a stable situation
	wait_for_list(lcs, NULL, 0, 1000);

	linphone_conference_unref(conf);

	int marie_call_no = (int)bctbx_list_size(linphone_core_get_calls(marie->lc));
	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, marie_call_no, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, marie_call_no, liblinphone_tester_sip_timeout));

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
	bctbx_list_free(lcs2);
	bctbx_list_free(all_manangers_in_conf);
}

static void simple_conference_with_one_participant_local(void) {
	simple_conference_with_one_participant_base(FALSE);
}

static void simple_conference_with_one_participant_no_local(void) {
	simple_conference_with_one_participant_base(FALSE);
}

static void simple_conference_with_user_defined_layout(const LinphoneConferenceLayout layout,
                                                       bool_t local_change_layout,
                                                       bool_t remote_change_layout,
                                                       bool_t add_participant,
                                                       const LinphoneMediaEncryption encryption) {
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = ((LinphoneCoreManager *)focus);
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);

	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, ((LinphoneCoreManager *)focus)->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));

		linphone_core_set_media_encryption(c, encryption);

		linphone_core_set_default_conference_layout(c, layout);
	}
	linphone_video_activation_policy_unref(pol);

	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	BC_ASSERT_TRUE(focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);

	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	// Marie creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_one_participant_conference(conf_params, TRUE);
	linphone_conference_params_enable_video(conf_params, TRUE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateInstantiated, 1, 5000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 0, int, "%0d");

	add_calls_to_remote_conference(lcs, focus_mgr, marie, participants, conf, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	const LinphoneConferenceParams *actual_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_FALSE(linphone_conference_params_local_participant_enabled(actual_conf_params));
	BC_ASSERT_TRUE(linphone_conference_params_one_participant_conference_enabled(actual_conf_params));
	BC_ASSERT_TRUE(linphone_conference_is_in(conf));

	// Generate traffic to compute volumes correctly
	wait_for_list(lcs, NULL, 0, 1000);

	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(conf);

	LinphoneConference *focus_conference =
	    linphone_core_search_conference(focus_mgr->lc, NULL, marie_conference_address, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(focus_conference);

	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);
		if (conference) {
			bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(conference);
			BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 4, size_t, "%0zu");
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 3, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
			LinphoneCall *conf_call = linphone_conference_get_call(conference);
			BC_ASSERT_PTR_NOT_NULL(conf_call);
			if (conf_call) {
				const LinphoneCallParams *call_local_params = linphone_call_get_params(conf_call);
				const LinphoneConferenceLayout conf_layout =
				    linphone_call_params_get_conference_video_layout(call_local_params);
				BC_ASSERT_EQUAL(layout, conf_layout, int, "%d");
			}
		}

		for (const bctbx_list_t *call_it = linphone_core_get_calls(m->lc); call_it;
		     call_it = bctbx_list_next(call_it)) {
			LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(call_it);
			check_conference_volumes(call);
		}

		bool_t enabled = (layout == LinphoneConferenceLayoutGrid) || (layout == LinphoneConferenceLayoutActiveSpeaker);
		LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(m->lc, focus_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(pcall);
		if (pcall) {
			const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
			const LinphoneCallParams *call_params = linphone_call_get_current_params(pcall);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_params), enabled, int, "%0d");
		}
		LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus_mgr->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(ccall);
		if (ccall) {
			const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
		}
	}

	LinphoneParticipant *pauline_participant =
	    linphone_conference_find_participant(focus_conference, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_participant);
	bool_t pauline_preserve_session = TRUE;
	if (pauline_participant) {
		pauline_preserve_session = linphone_participant_preserve_session(pauline_participant);
	}
	bctbx_list_t *lcs2 = bctbx_list_copy(lcs);
	remove_participant_from_local_conference(lcs2, focus_mgr, pauline, focus_conference);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	lcs2 = bctbx_list_remove(lcs2, pauline->lc);
	all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, pauline);

	stats michelle_stats = michelle->stat;
	stats focus_stats = focus_mgr->stat;
	if (local_change_layout) {
		LinphoneConferenceLayout new_layout = LinphoneConferenceLayoutGrid;
		LinphoneCall *conf_call = linphone_conference_get_call(conf);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		if (conf_call) {
			LinphoneCallParams *call_params = linphone_core_create_call_params(marie->lc, conf_call);
			new_layout = ((layout == LinphoneConferenceLayoutGrid) ? LinphoneConferenceLayoutActiveSpeaker
			                                                       : LinphoneConferenceLayoutGrid);
			linphone_call_params_set_conference_video_layout(call_params, new_layout);
			linphone_call_update(conf_call, call_params);
			linphone_call_params_unref(call_params);

			const LinphoneCallParams *call_local_params = linphone_call_get_params(conf_call);
			const LinphoneConferenceLayout local_conf_layout =
			    linphone_call_params_get_conference_video_layout(call_local_params);
			BC_ASSERT_EQUAL(new_layout, local_conf_layout, int, "%d");
		}
	}

	if (remote_change_layout) {
		const LinphoneAddress *local_conference_address = linphone_conference_get_conference_address(conf);
		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(michelle->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(michelle->lc, NULL, uri, local_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);
		if (conference) {
			LinphoneConferenceLayout new_layout = LinphoneConferenceLayoutGrid;
			LinphoneCall *rcall = linphone_conference_get_call(conference);
			BC_ASSERT_PTR_NOT_NULL(rcall);
			if (rcall) {
				const LinphoneCallParams *rcall_local_params = linphone_call_get_params(rcall);
				const LinphoneConferenceLayout remote_conf_layout =
				    linphone_call_params_get_conference_video_layout(rcall_local_params);

				new_layout =
				    ((remote_conf_layout == LinphoneConferenceLayoutGrid) ? LinphoneConferenceLayoutActiveSpeaker
				                                                          : LinphoneConferenceLayoutGrid);
				LinphoneCallParams *call_params = linphone_core_create_call_params(michelle->lc, rcall);
				linphone_call_params_set_conference_video_layout(call_params, new_layout);
				linphone_call_update(rcall, call_params);
				linphone_call_params_unref(call_params);
			}

			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallUpdating,
			                             michelle_stats.number_of_LinphoneCallUpdating + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallUpdatedByRemote,
			                             focus_stats.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning,
			                             michelle_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                             focus_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));

			if (rcall) {
				const LinphoneCallParams *rcall_local_params = linphone_call_get_params(rcall);
				const LinphoneConferenceLayout remote_conf_layout =
				    linphone_call_params_get_conference_video_layout(rcall_local_params);
				BC_ASSERT_EQUAL(new_layout, remote_conf_layout, int, "%d");
			}

			LinphoneConference *fconf = linphone_core_search_conference(focus_mgr->lc, NULL, local_conference_address,
			                                                            local_conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(fconf);
			if (fconf) {
				LinphoneParticipant *participant = linphone_conference_find_participant(fconf, michelle->identity);
				BC_ASSERT_PTR_NOT_NULL(participant);
				bctbx_list_t *devices = linphone_participant_get_devices(participant);

				for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
					BC_ASSERT_PTR_NOT_NULL(d);
					LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(
					    focus_mgr->lc, linphone_participant_device_get_address(d));
					BC_ASSERT_PTR_NOT_NULL(participant_call);
					if (participant_call) {
						const LinphoneCallParams *call_remote_params =
						    linphone_call_get_remote_params(participant_call);
						const LinphoneConferenceLayout device_layout =
						    linphone_call_params_get_conference_video_layout(call_remote_params);
						BC_ASSERT_EQUAL(device_layout, new_layout, int, "%d");
					}
				}
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}
	}

	marie_conference_address = linphone_conference_get_conference_address(conf);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		int no_parts = 2;
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed, 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_removed, 1, 3000));

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);
		if (conference) {
			bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(conference);
			BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 3, size_t, "%0zu");
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
		}
	}

	LinphoneCall *focus_called_by_michelle =
	    linphone_core_get_call_by_remote_address2(focus_mgr->lc, michelle->identity);
	LinphoneCall *michelle_call_focus = linphone_core_get_call_by_remote_address2(michelle->lc, focus_mgr->identity);
	LinphoneCall *focus_called_by_laure = linphone_core_get_call_by_remote_address2(focus_mgr->lc, laure->identity);
	LinphoneCall *laure_call_focus = linphone_core_get_call_by_remote_address2(laure->lc, focus_mgr->identity);
	LinphoneCall *focus_called_by_marie = linphone_core_get_call_by_remote_address2(focus_mgr->lc, marie->identity);
	LinphoneCall *marie_call_focus = linphone_core_get_call_by_remote_address2(marie->lc, focus_mgr->identity);
	LinphoneCall *focus_called_by_pauline = linphone_core_get_call_by_remote_address2(focus_mgr->lc, pauline->identity);
	LinphoneCall *pauline_call_focus = linphone_core_get_call_by_remote_address2(pauline->lc, focus_mgr->identity);

	const LinphoneCallParams *call_params = linphone_call_get_current_params(focus_called_by_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(michelle_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(focus_called_by_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(laure_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	if (pauline_preserve_session) {
		call_params = linphone_call_get_current_params(focus_called_by_pauline);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_call_focus);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	}
	call_params = linphone_call_get_current_params(focus_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(marie_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 1000);

	set_video_in_call(laure, focus_mgr, FALSE, all_manangers_in_conf, marie_conference_address);
	call_params = linphone_call_get_current_params(focus_called_by_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(michelle_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(focus_called_by_laure);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(laure_call_focus);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
	if (pauline_preserve_session) {
		call_params = linphone_call_get_current_params(focus_called_by_pauline);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_call_focus);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	}
	call_params = linphone_call_get_current_params(focus_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(marie_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 1000);

	set_video_in_call(laure, focus_mgr, TRUE, all_manangers_in_conf, marie_conference_address);
	call_params = linphone_call_get_current_params(focus_called_by_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(michelle_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(focus_called_by_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(laure_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	if (pauline_preserve_session) {
		call_params = linphone_call_get_current_params(focus_called_by_pauline);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_call_focus);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	}
	call_params = linphone_call_get_current_params(focus_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(marie_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 1000);

	set_video_in_call(laure, focus_mgr, FALSE, all_manangers_in_conf, marie_conference_address);
	call_params = linphone_call_get_current_params(focus_called_by_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(michelle_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(focus_called_by_laure);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(laure_call_focus);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
	if (pauline_preserve_session) {
		call_params = linphone_call_get_current_params(focus_called_by_pauline);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_call_focus);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	}
	call_params = linphone_call_get_current_params(focus_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(marie_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));

	set_video_in_call(laure, focus_mgr, TRUE, all_manangers_in_conf, marie_conference_address);
	call_params = linphone_call_get_current_params(focus_called_by_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(michelle_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(focus_called_by_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(laure_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	if (pauline_preserve_session) {
		call_params = linphone_call_get_current_params(focus_called_by_pauline);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_call_focus);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	}
	call_params = linphone_call_get_current_params(focus_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
	call_params = linphone_call_get_current_params(marie_call_focus);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));

	remove_participant_from_local_conference(lcs2, focus_mgr, michelle, focus_conference);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	lcs2 = bctbx_list_remove(lcs2, michelle->lc);
	all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, michelle);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed, 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_removed, 2, 3000));
		int no_parts = 1;
		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);
		if (conference) {
			bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(conference);
			BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 2, size_t, "%0zu");
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	if (add_participant) {
		lcs = bctbx_list_append(lcs, chloe->lc);

		BC_ASSERT_TRUE(call(marie, chloe));

		lcs2 = bctbx_list_append(lcs2, chloe->lc);
		bctbx_list_t *participants2 = NULL;
		participants2 = bctbx_list_append(participants2, chloe);
		all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, chloe);
		add_calls_to_remote_conference(lcs2, focus_mgr, marie, participants2, conf, TRUE);
		bctbx_list_free(participants2);
		stats marie_stats = marie->stat;
		linphone_conference_enter(conf);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             marie_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));

		for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
			LinphoneConference *conference =
			    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(conference);
			linphone_address_unref(uri);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 2, int, "%d");
				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(conference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 3, size_t, "%0zu");
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		// wait a bit for the conference audio processing to run, despite we do not test it for the moment
		wait_for_list(lcs, NULL, 0, 1000);

		remove_participant_from_local_conference(lcs2, focus_mgr, chloe, focus_conference);
		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

		BC_ASSERT_FALSE(
		    wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

		lcs2 = bctbx_list_remove(lcs2, chloe->lc);
		all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, chloe);
		for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
			LinphoneConference *conference =
			    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(conference);
			linphone_address_unref(uri);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 1, int, "%d");
				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(conference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 2, size_t, "%0zu");
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
			}
		}
	}

	remove_participant_from_local_conference(lcs2, focus_mgr, laure, focus_conference);
	lcs2 = bctbx_list_remove(lcs2, laure->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	remove_participant_from_local_conference(lcs2, focus_mgr, marie, focus_conference);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	int focus_call_no = (int)bctbx_list_size(linphone_core_get_calls(focus_mgr->lc));
	linphone_core_terminate_all_calls(focus_mgr->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, (add_participant ? 5 : 4),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, (add_participant ? 5 : 4),
	                             liblinphone_tester_sip_timeout));
	if (add_participant) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallEnd, focus_call_no, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallReleased, focus_call_no,
	                             liblinphone_tester_sip_timeout));

	focus_call_no = (int)bctbx_list_size(linphone_core_get_calls(focus_mgr->lc));
	BC_ASSERT_EQUAL(focus_call_no, 0, int, "%0d");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
	bctbx_list_free(lcs2);
	bctbx_list_free(all_manangers_in_conf);
}

static void simple_conference_with_grid_layout(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE,
	                                           LinphoneMediaEncryptionNone);
}

static void simple_srtp_conference_with_grid_layout(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE,
	                                           LinphoneMediaEncryptionSRTP);
}

static void simple_conference_with_active_speaker_layout(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE,
	                                           LinphoneMediaEncryptionNone);
}

static void simple_srtp_conference_with_active_speaker_layout(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE,
	                                           LinphoneMediaEncryptionSRTP);
}

static void simple_conference_with_layout_change_local_participant(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE,
	                                           LinphoneMediaEncryptionNone);
}

static void simple_conference_with_layout_change_remote_participant(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutGrid, FALSE, TRUE, FALSE,
	                                           LinphoneMediaEncryptionNone);
}

static void simple_conference_with_layout_change_remote_participant_without_conference_params_update(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutGrid, FALSE, TRUE, FALSE,
	                                           LinphoneMediaEncryptionNone);
}

static void add_participant_after_layout_change(void) {
	simple_conference_with_user_defined_layout(LinphoneConferenceLayoutGrid, FALSE, TRUE, TRUE,
	                                           LinphoneMediaEncryptionNone);
}

static void simple_conference_with_one_participant(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	// marie creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_one_participant_conference(conf_params, TRUE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 0, int, "%0d");

	add_calls_to_local_conference(lcs, marie, NULL, participants, TRUE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(conf);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(m->lc))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, m_uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(m_uri);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 3, int, "%d");
		}
	}

	bctbx_list_t *lcs2 = bctbx_list_copy(lcs);
	remove_participant_from_local_conference(lcs2, marie, pauline, NULL);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	lcs2 = bctbx_list_remove(lcs2, pauline->lc);
	all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, pauline);

	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(m->lc))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, m_uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(m_uri);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 2, int, "%d");
		}
	}

	remove_participant_from_local_conference(lcs2, marie, michelle, NULL);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	lcs2 = bctbx_list_remove(lcs2, michelle->lc);
	all_manangers_in_conf = bctbx_list_remove(all_manangers_in_conf, michelle);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneAddress *m_uri = (linphone_core_conference_server_enabled(m->lc))
		                             ? linphone_address_clone(marie_conference_address)
		                             : linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, m_uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(m_uri);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 1, int, "%d");
		}
	}

	remove_participant_from_local_conference(lcs2, marie, laure, NULL);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, 1, 1000));

	int marie_call_no = (int)bctbx_list_size(linphone_core_get_calls(marie->lc));
	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, marie_call_no, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, marie_call_no, liblinphone_tester_sip_timeout));

	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
	bctbx_list_free(lcs2);
	bctbx_list_free(all_manangers_in_conf);
}

static void simple_conference_with_subject_change_from_admin_base(bool_t enable_video,
                                                                  bool_t remove_conference_version) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_initiate(pol, enable_video);
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_core_set_video_activation_policy(michelle->lc, pol);
	linphone_core_set_video_activation_policy(focus_mgr->lc, pol);
	linphone_video_activation_policy_unref(pol);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(laure->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(michelle->lc, liblinphone_tester_mire_id);
	if (enable_video) {
		linphone_core_set_video_device(focus_mgr->lc, liblinphone_tester_mire_id);
	}

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(laure->lc, TRUE);
	linphone_core_enable_video_display(laure->lc, TRUE);
	linphone_core_enable_video_capture(michelle->lc, TRUE);
	linphone_core_enable_video_display(michelle->lc, TRUE);
	if (enable_video) {
		linphone_core_enable_video_capture(focus_mgr->lc, TRUE);
		linphone_core_enable_video_display(focus_mgr->lc, TRUE);
	}

	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(focus_mgr->lc);
	LinphoneProxyConfig *laure_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	// Remove conference spec after setting conference factory uri as the callback onConferenceFactoryAddressChanged
	// sets ephemeral, groupchat and conference versions
	if (remove_conference_version) {
		linphone_core_remove_linphone_spec(marie->lc, "conference");
	}

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, focus_mgr->lc);

	BC_ASSERT_TRUE(focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	// marie creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, enable_video);
	LinphoneConference *conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE((!!linphone_conference_params_video_enabled(current_conf_params)) == enable_video);

	const char *original_subject = "SIGABRT (signal 6)";
	ms_message("%s is changing conference subject to %s", linphone_core_get_identity(marie->lc), original_subject);
	linphone_conference_set_subject(conf, original_subject);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 0, int, "%0d");

	add_calls_to_remote_conference(lcs, (LinphoneCoreManager *)focus, marie, participants, conf, FALSE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	// Subject should not have changed
	BC_ASSERT_STRING_EQUAL(original_subject, linphone_conference_get_subject(conf));

	current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE((!!linphone_conference_params_video_enabled(current_conf_params)) == enable_video);

	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(conf);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);
		if (conference) {
			BC_ASSERT_STRING_EQUAL(original_subject, linphone_conference_get_subject(conference));
		}

		// Wait for all participants to have the expected video availability
		int ret = FALSE;
		int part_counter = 0;
		do {
			part_counter++;
			bctbx_list_t *participant_devices = linphone_conference_get_participant_device_list(conference);
			ret = (bctbx_list_size(participant_devices) == bctbx_list_size(all_manangers_in_conf));
			for (bctbx_list_t *d_it = participant_devices; d_it; d_it = bctbx_list_next(d_it)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
				BC_ASSERT_PTR_NOT_NULL(d);
				if (d) {
					ret &= (linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
					        enable_video);
				}
			}
			bctbx_list_free_with_data(participant_devices, (void (*)(void *))linphone_participant_device_unref);
			wait_for_list(lcs, NULL, 0, 100);
		} while ((part_counter < 100) && (ret == FALSE));
		BC_ASSERT_TRUE(ret);
	}

	stats marie_stats = marie->stat;
	stats focus_stats = focus_mgr->stat;
	stats *initial_participants_stats = NULL;
	int counter = 1;
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));

		// Append element
		initial_participants_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(michelle->lc, focus_mgr->identity);
	LinphoneCallParams *new_params = linphone_core_create_call_params(michelle->lc, conf_call);
	bool_t michelle_enable_video =
	    !!linphone_call_params_video_enabled(linphone_call_get_current_params(conf_call)) ? FALSE : TRUE;
	linphone_call_params_enable_video(new_params, michelle_enable_video);
	ms_message("%s toggles video capabilities (video is now %s)", linphone_core_get_identity(michelle->lc),
	           (michelle_enable_video ? "enabled" : "disabled"));
	linphone_call_update(conf_call, new_params);
	linphone_call_params_unref(new_params);

	const char *new_subject = "SEGFAULT (signal 11)";
	ms_message("%s is changing conference subject to %s", linphone_core_get_identity(marie->lc), new_subject);
	linphone_conference_set_subject(conf, new_subject);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             marie_stats.number_of_LinphoneCallStreamsRunning + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             focus_stats.number_of_LinphoneCallStreamsRunning + (enable_video)
	                                 ? ((int)bctbx_list_size(participants) + 1)
	                                 : 2,
	                             3000));

	// Participants should have received the subject change notification
	int idx = 0;
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_subject_changed,
		                             (initial_participants_stats[idx].number_of_subject_changed + 1),
		                             liblinphone_tester_sip_timeout));
		if (enable_video) {
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_participant_devices_media_capability_changed,
			    (initial_participants_stats[idx].number_of_participant_devices_media_capability_changed + 1),
			    liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_LinphoneCallUpdating,
			    initial_participants_stats[idx].number_of_LinphoneCallUpdating + ((m == marie) ? 2 : 1), 3000));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
			    initial_participants_stats[idx].number_of_LinphoneCallStreamsRunning + ((m == marie) ? 2 : 1), 3000));
		} else {
			BC_ASSERT_FALSE(wait_for_list(
			    lcs, &m->stat.number_of_participant_devices_media_capability_changed,
			    (initial_participants_stats[idx].number_of_participant_devices_media_capability_changed + 1), 1000));
		}

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);

		if (conference) {
			BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(conference), new_subject);
		}
		idx++;
	}

	ms_free(initial_participants_stats);
	initial_participants_stats = NULL;

	counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));

		// Append element
		initial_participants_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	marie_stats = marie->stat;
	focus_stats = focus_mgr->stat;

	const char *new_subject2 = "Media change";
	ms_message("%s is attempting to change conference subject to %s", linphone_core_get_identity(marie->lc),
	           new_subject2);
	linphone_conference_set_subject(conf, new_subject2);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             marie_stats.number_of_LinphoneCallStreamsRunning + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             focus_stats.number_of_LinphoneCallStreamsRunning + 1, 3000));

	// Participants should have received the subject change notification
	idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		if (remove_conference_version) {
			BC_ASSERT_FALSE(wait_for_list(lcs, &m->stat.number_of_subject_changed,
			                              (initial_participants_stats[idx].number_of_subject_changed + 1), 1000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_subject_changed,
			                             (initial_participants_stats[idx].number_of_subject_changed + 1), 1000));
		}
		idx++;
	}
	ms_free(initial_participants_stats);
	initial_participants_stats = NULL;

	// need time to finish all communications
	wait_for_list(lcs, NULL, 1, 5000);

	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);

		if (conference) {
			if (remove_conference_version) {
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(conference), new_subject);
			} else {
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(conference), new_subject2);
			}
		}
	}
	bctbx_list_free(all_manangers_in_conf);

	terminate_conference(participants, marie, NULL, (LinphoneCoreManager *)focus, FALSE);

	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_subject_change_from_admin(void) {
	simple_conference_with_subject_change_from_admin_base(FALSE, FALSE);
}

static void simple_video_conference_with_subject_change_from_admin(void) {
	simple_conference_with_subject_change_from_admin_base(TRUE, FALSE);
}

static void video_conference_with_no_conference_version(void) {
	simple_conference_with_subject_change_from_admin_base(TRUE, TRUE);
}

static void simple_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	simple_conference_base(marie, pauline, laure, NULL, FALSE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void simple_ccmp_conference_base(bool_t update_conference, bool_t cancel_conference) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	coresList = bctbx_list_append(coresList, michelle->lc);

	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_account);
	const LinphoneAccountParams *marie_account_params =
	    marie_account ? linphone_account_get_params(marie_account) : NULL;
	BC_ASSERT_PTR_NOT_NULL(marie_account_params);
	LinphoneAddress *marie_identity = NULL;
	if (marie_account_params) {
		marie_identity = linphone_address_clone(marie_account_params
		                                            ? linphone_account_params_get_identity_address(marie_account_params)
		                                            : marie->identity);
		LinphoneAccountParams *account_params = linphone_account_params_clone(marie_account_params);
		linphone_account_params_set_ccmp_server_url(account_params, ccmp_server_url);
		linphone_account_set_params(marie_account, account_params);
		linphone_account_params_unref(account_params);
	}

	// The organizer creates a conference scheduler
	LinphoneConferenceScheduler *conference_scheduler =
	    linphone_core_create_ccmp_conference_scheduler(marie->lc, linphone_core_get_default_account(marie->lc));
	LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
	linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
	linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
	linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
	linphone_conference_scheduler_cbs_unref(cbs);

	LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
	linphone_conference_info_set_organizer(conf_info, marie_identity);
	bctbx_list_t *participants_info = NULL;
	add_participant_info_to_list(&participants_info, pauline->identity, LinphoneParticipantRoleSpeaker, -1);
	add_participant_info_to_list(&participants_info, laure->identity, LinphoneParticipantRoleListener, -1);
	linphone_conference_info_set_participant_infos(conf_info, participants_info);
	const int duration = 600;
	linphone_conference_info_set_duration(conf_info, duration);
	const time_t start_time = ms_time(NULL) - 60;
	linphone_conference_info_set_date_time(conf_info, start_time);
	const char *subject = "CCMP conference";
	linphone_conference_info_set_subject(conf_info, subject);
	const char *description = "my first CCMP conference";
	linphone_conference_info_set_description(conf_info, description);
	LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
	linphone_conference_info_set_security_level(conf_info, security_level);

	linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
	linphone_conference_info_unref(conf_info);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerStateReady, 1,
	                             liblinphone_tester_sip_timeout));

	const LinphoneConferenceInfo *updated_conf_info = linphone_conference_scheduler_get_info(conference_scheduler);
	BC_ASSERT_PTR_NOT_NULL(updated_conf_info);
	char *uid = NULL;
	LinphoneAddress *conference_address = NULL;
	char *conference_address_str = NULL;
	const LinphoneAddress *conference_uri = linphone_conference_info_get_uri(updated_conf_info);
	BC_ASSERT_PTR_NOT_NULL(conference_uri);
	if (!conference_uri) {
		goto end;
	}
	conference_address = linphone_address_clone(conference_uri);
	BC_ASSERT_PTR_NOT_NULL(conference_address);

	check_conference_info_in_db(marie, NULL, conference_address, marie->identity, participants_info, start_time,
	                            duration, subject, description, 0, LinphoneConferenceInfoStateNew, security_level, TRUE,
	                            TRUE, TRUE, FALSE);

	LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(marie->lc);
	linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
	linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
	linphone_chat_room_params_unref(chat_room_params);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerInvitationsSent, 1,
	                             liblinphone_tester_sip_timeout));

	LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie->lc, conference_address);
	if (BC_ASSERT_PTR_NOT_NULL(info)) {
		uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
		BC_ASSERT_PTR_NOT_NULL(uid);
		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			if (mgr != marie) {
				linphone_conference_info_check_participant(info, mgr->identity, 0);
			}
		}
		const bctbx_list_t *participant_infos = linphone_conference_info_get_participant_infos(info);
		for (const bctbx_list_t *it = participant_infos; it; it = bctbx_list_next(it)) {
			LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
			BC_ASSERT_PTR_NOT_NULL(linphone_participant_info_get_ccmp_uri(participant_info));
		}

		const LinphoneParticipantInfo *organizer_info = linphone_conference_info_get_organizer_info(info);
		BC_ASSERT_PTR_NOT_NULL(organizer_info);
		if (organizer_info) {
			const char *organizer_ccmp_uri = linphone_participant_info_get_ccmp_uri(organizer_info);
			BC_ASSERT_PTR_NOT_NULL(organizer_ccmp_uri);
			if (organizer_ccmp_uri) {
				marie_account_params = marie_account ? linphone_account_get_params(marie_account) : NULL;
				BC_ASSERT_STRING_EQUAL(organizer_ccmp_uri,
				                       linphone_account_params_get_ccmp_user_id(marie_account_params));
			}
		}
		linphone_conference_info_unref(info);
	}

	for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
		bool is_marie = (mgr == marie);

		// chat room in created state
		BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated, (is_marie) ? 2 : 1,
		                             liblinphone_tester_sip_timeout));
		if (is_marie) {
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageSent, 2, liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (!linphone_core_conference_ics_in_message_body_enabled(marie->lc)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile, 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
			if (mgr->stat.last_received_chat_message != NULL) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
				                       "text/calendar;conference-event=yes");
			}

			bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
			LinphoneChatRoom *cr =
			    linphone_core_search_chat_room(marie->lc, NULL, marie->identity, NULL, chat_room_participants);
			bctbx_list_free(chat_room_participants);
			BC_ASSERT_PTR_NOT_NULL(cr);
			if (cr) {
				LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
				BC_ASSERT_PTR_NOT_NULL(msg);

				if (msg) {
					const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL(bctbx_list_size(original_contents), 1, size_t, "%zu");
					LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
					BC_ASSERT_PTR_NOT_NULL(original_content);

					LinphoneConferenceInfo *conf_info_in_db =
					    linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
					if (BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
						check_conference_info_members(conf_info_in_db, uid, conference_address, marie_identity,
						                              participants_info, start_time, duration, subject, description, 0,
						                              LinphoneConferenceInfoStateNew,
						                              LinphoneConferenceSecurityLevelNone, TRUE, TRUE, TRUE, FALSE);

						LinphoneConferenceInfo *conf_info_from_original_content =
						    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
						                                                                   original_content);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
							compare_conference_infos(conf_info_from_original_content, conf_info_in_db, FALSE);
							linphone_conference_info_unref(conf_info_from_original_content);
						}
						linphone_conference_info_unref(conf_info_in_db);
					}
					linphone_chat_message_unref(msg);
				}
			}
		}
	}

	if (update_conference) {
		bctbx_list_t *coresManagerList2 = NULL;
		coresManagerList2 = bctbx_list_append(coresManagerList2, michelle);

		bctbx_list_t *unused_list = init_core_for_conference(coresManagerList2);
		bctbx_list_free(unused_list);
		bctbx_list_free(coresManagerList2);

		coresManagerList = bctbx_list_append(coresManagerList, michelle);

		// The organizer creates a conference scheduler and updates the conference
		LinphoneConferenceScheduler *update_conference_scheduler =
		    linphone_core_create_ccmp_conference_scheduler(marie->lc, linphone_core_get_default_account(marie->lc));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(update_conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneConferenceInfo *updated_conf_info =
		    linphone_core_find_conference_information_from_uri(marie->lc, conference_address);
		LinphoneParticipantInfo *participant_info = linphone_participant_info_new(michelle->identity);
		linphone_participant_info_set_role(participant_info, LinphoneParticipantRoleSpeaker);
		linphone_conference_info_add_participant_2(updated_conf_info, participant_info);
		subject = "Updated CCMP conference";
		linphone_conference_info_set_subject(updated_conf_info, subject);
		linphone_conference_scheduler_set_info(update_conference_scheduler, updated_conf_info);
		linphone_conference_info_unref(updated_conf_info);
		linphone_participant_info_unref(participant_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerStateUpdating, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerStateReady, 2,
		                             liblinphone_tester_sip_timeout));

		add_participant_info_to_list(&participants_info, michelle->identity, LinphoneParticipantRoleSpeaker, -1);
		check_conference_info_in_db(marie, NULL, conference_address, marie->identity, participants_info, start_time,
		                            duration, subject, description, 1, LinphoneConferenceInfoStateUpdated,
		                            security_level, TRUE, TRUE, TRUE, FALSE);

		LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(marie->lc);
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
		linphone_conference_scheduler_send_invitations(update_conference_scheduler, chat_room_params);
		linphone_chat_room_params_unref(chat_room_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerInvitationsSent, 2,
		                             liblinphone_tester_sip_timeout));
		linphone_conference_scheduler_unref(update_conference_scheduler);

		char *uid2 = NULL;
		LinphoneConferenceInfo *info =
		    linphone_core_find_conference_information_from_uri(marie->lc, conference_address);
		if (BC_ASSERT_PTR_NOT_NULL(info)) {
			uid2 = ms_strdup(linphone_conference_info_get_ics_uid(info));
			BC_ASSERT_PTR_NOT_NULL(uid2);
			for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
				LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
				if (mgr != marie) {
					linphone_conference_info_check_participant(info, mgr->identity, (mgr == michelle) ? 0 : 1);
				}
			}
			linphone_conference_info_unref(info);
		}

		BC_ASSERT_STRING_EQUAL(uid, uid2);

		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			bool is_marie = (mgr == marie);

			// chat room in created state
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             (is_marie) ? 3 : 1, liblinphone_tester_sip_timeout));
			if (is_marie) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageSent, 5,
				                             liblinphone_tester_sip_timeout));
			} else {
				const int msg_received = (mgr == michelle) ? 1 : 2;
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived, msg_received,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(marie->lc)) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             msg_received, liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       "text/calendar;conference-event=yes");
				}

				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
				LinphoneChatRoom *cr =
				    linphone_core_search_chat_room(marie->lc, NULL, marie->identity, NULL, chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);
				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					if (msg) {
						const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
						BC_ASSERT_EQUAL(bctbx_list_size(original_contents), 1, size_t, "%zu");
						LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
						BC_ASSERT_PTR_NOT_NULL(original_content);

						LinphoneConferenceInfo *conf_info_in_db =
						    linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
							LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
							if (mgr == michelle) {
								exp_state = LinphoneConferenceInfoStateNew;
							} else {
								exp_state = LinphoneConferenceInfoStateUpdated;
							}

							check_conference_info_members(conf_info_in_db, uid2, conference_address, marie_identity,
							                              participants_info, start_time, duration, subject, description,
							                              (mgr == michelle) ? 0 : 1, exp_state,
							                              LinphoneConferenceSecurityLevelNone, TRUE, TRUE, TRUE, FALSE);

							LinphoneConferenceInfo *conf_info_from_original_content =
							    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
							                                                                   original_content);
							if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
								compare_conference_infos(conf_info_from_original_content, conf_info_in_db, FALSE);
								linphone_conference_info_unref(conf_info_from_original_content);
							}
							linphone_conference_info_unref(conf_info_in_db);
						}
						linphone_chat_message_unref(msg);
					}
				}
			}
		}
		if (uid2) {
			ms_free(uid2);
		}
	}

	if (cancel_conference) {
		stats marie_stats = marie->stat;
		// The organizer creates a conference scheduler and cancels the conference
		LinphoneConferenceScheduler *cancel_conference_scheduler =
		    linphone_core_create_ccmp_conference_scheduler(marie->lc, linphone_core_get_default_account(marie->lc));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(cancel_conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneConferenceInfo *cancel_conf_info =
		    linphone_core_find_conference_information_from_uri(marie->lc, conference_address);
		linphone_conference_scheduler_cancel_conference(cancel_conference_scheduler, cancel_conf_info);
		linphone_conference_info_unref(cancel_conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerStateUpdating,
		                             marie_stats.number_of_ConferenceSchedulerStateUpdating + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerStateReady,
		                             marie_stats.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(marie->lc);
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
		linphone_conference_scheduler_send_invitations(cancel_conference_scheduler, chat_room_params);
		linphone_chat_room_params_unref(chat_room_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerInvitationsSent,
		                             marie_stats.number_of_ConferenceSchedulerInvitationsSent + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_conference_scheduler_unref(cancel_conference_scheduler);

		char *uid2 = NULL;
		LinphoneConferenceInfo *info =
		    linphone_core_find_conference_information_from_uri(marie->lc, conference_address);
		if (BC_ASSERT_PTR_NOT_NULL(info)) {
			uid2 = ms_strdup(linphone_conference_info_get_ics_uid(info));
			linphone_conference_info_unref(info);
		}

		BC_ASSERT_STRING_EQUAL(uid, uid2);

		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			bool is_marie = (mgr == marie);

			// chat room in created state
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             (is_marie) ? 3 : 1, liblinphone_tester_sip_timeout));
			if (is_marie) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageSent,
				                             marie_stats.number_of_LinphoneMessageSent + ((update_conference) ? 3 : 2),
				                             liblinphone_tester_sip_timeout));
			} else {
				const int msg_received = ((mgr == michelle) ? 1 : 2) + ((update_conference) ? 1 : 0);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived, msg_received,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(marie->lc)) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             msg_received, liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       "text/calendar;conference-event=yes");
				}

				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
				LinphoneChatRoom *cr =
				    linphone_core_search_chat_room(marie->lc, NULL, marie->identity, NULL, chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);
				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					if (msg) {
						const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
						BC_ASSERT_EQUAL(bctbx_list_size(original_contents), 1, size_t, "%zu");
						LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
						BC_ASSERT_PTR_NOT_NULL(original_content);

						LinphoneConferenceInfo *conf_info_in_db =
						    linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
							LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateCancelled;
							unsigned int ics_sequence = 0;
							if ((mgr == michelle) || !update_conference) {
								ics_sequence = 1;
							} else {
								ics_sequence = 2;
							}
							check_conference_info_members(conf_info_in_db, uid2, conference_address, marie_identity,
							                              NULL, start_time, duration, subject, description,
							                              ics_sequence, exp_state, LinphoneConferenceSecurityLevelNone,
							                              TRUE, TRUE, TRUE, FALSE);

							LinphoneConferenceInfo *conf_info_from_original_content =
							    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
							                                                                   original_content);
							if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
								compare_conference_infos(conf_info_from_original_content, conf_info_in_db, FALSE);
								linphone_conference_info_unref(conf_info_from_original_content);
							}
							linphone_conference_info_unref(conf_info_in_db);
						}
						linphone_chat_message_unref(msg);
					}
				}
			}
		}
		if (uid2) {
			ms_free(uid2);
		}
	} else {
		conference_address_str =
		    (conference_address) ? linphone_address_as_string(conference_address) : ms_strdup("sip:");
		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, NULL);
			linphone_core_invite_address_with_params_2(mgr->lc, conference_address, new_params, NULL, NULL);
			linphone_call_params_unref(new_params);
		}

		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneCall *currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));
		}

		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			ms_message("%s is ending its call to conference %s", linphone_core_get_identity(mgr->lc),
			           conference_address_str);
			LinphoneConference *conference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				linphone_conference_terminate(conference);
			}
		}

		for (bctbx_list_t *it = coresManagerList; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}
	}

end:
	if (conference_scheduler) linphone_conference_scheduler_unref(conference_scheduler);
	if (marie_identity) linphone_address_unref(marie_identity);
	if (conference_address) linphone_address_unref(conference_address);
	if (conference_address_str) bctbx_free(conference_address_str);
	bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	if (uid) {
		ms_free(uid);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(michelle);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
}

static void simple_ccmp_conference(void) {
	simple_ccmp_conference_base(FALSE, FALSE);
}

static void simple_ccmp_conference_with_conference_update(void) {
	simple_ccmp_conference_base(TRUE, FALSE);
}

static void simple_ccmp_conference_with_conference_update_cancel(void) {
	simple_ccmp_conference_base(TRUE, TRUE);
}

void conference_information_updated_on_account(LinphoneAccount *account, BCTBX_UNUSED(const bctbx_list_t *info)) {
	LinphoneCore *lc = linphone_account_get_core(account);
	ms_message("Conference information updated for user id [%s] at account [%s]",
	           linphone_account_params_get_identity(linphone_account_get_params(account)),
	           linphone_account_params_get_server_addr(linphone_account_get_params(account)));
	stats *counters = get_stats(lc);
	counters->number_of_ConferenceInformationUpdated++;
}

static void simple_ccmp_conference_retrieval(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, michelle);

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	for (bctbx_list_t *core_it = coresList; core_it; core_it = bctbx_list_next(core_it)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(core_it);
		LinphoneAccount *account = linphone_core_get_default_account(core);
		LinphoneAccountParams *account_params =
		    account ? linphone_account_params_clone(linphone_account_get_params(account)) : NULL;
		if (account_params) {
			linphone_account_params_set_ccmp_server_url(account_params, ccmp_server_url);
			linphone_account_set_params(account, account_params);
			linphone_account_params_unref(account_params);
		}
		LinphoneAccountCbs *cbs = linphone_factory_create_account_cbs(linphone_factory_get());
		linphone_account_cbs_set_conference_information_updated(cbs, conference_information_updated_on_account);
		linphone_account_add_callbacks(account, cbs);
		linphone_account_cbs_unref(cbs);
	}

	int number_conferences = 10;
	int base_duration = 10;
	const char *base_subject = "Conference number";
	const char *base_description = "The CCMP conference description";
	time_t base_start_time = ms_time(NULL);
	int base_start_time_offset = 60;
	bctbx_list_t *participants_info = NULL;
	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	LinphoneAddress *organizer_address =
	    marie_account ? linphone_address_clone(
	                        linphone_account_params_get_identity_address(linphone_account_get_params(marie_account)))
	                  : linphone_address_clone(marie->identity);
	add_participant_info_to_list(&participants_info, pauline->identity, LinphoneParticipantRoleSpeaker, -1);
	add_participant_info_to_list(&participants_info, laure->identity, LinphoneParticipantRoleListener, -1);
	add_participant_info_to_list(&participants_info, michelle->identity, LinphoneParticipantRoleSpeaker, -1);
	bctbx_list_t *reference_conference_infos = NULL;

	for (int idx = 1; idx <= number_conferences; idx++) {
		LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
		linphone_conference_info_set_organizer(conf_info, organizer_address);
		linphone_conference_info_set_participant_infos(conf_info, participants_info);
		const int duration = base_duration * idx;
		linphone_conference_info_set_duration(conf_info, duration);
		const time_t start_time = base_start_time + base_start_time_offset * idx;
		linphone_conference_info_set_date_time(conf_info, start_time);
		char subject[100];
		sprintf(subject, "%s %0d", base_subject, idx);
		linphone_conference_info_set_subject(conf_info, subject);
		char description[300];
		sprintf(description, "%s %0d", base_description, idx);
		linphone_conference_info_set_description(conf_info, description);
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		linphone_conference_info_set_security_level(conf_info, security_level);

		// The organizer creates a conference scheduler
		LinphoneConferenceScheduler *conference_scheduler =
		    linphone_core_create_ccmp_conference_scheduler(marie->lc, linphone_core_get_default_account(marie->lc));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
		linphone_conference_info_unref(conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ConferenceSchedulerStateReady, idx,
		                             liblinphone_tester_sip_timeout));

		const LinphoneConferenceInfo *updated_conf_info = linphone_conference_scheduler_get_info(conference_scheduler);
		BC_ASSERT_PTR_NOT_NULL(updated_conf_info);
		LinphoneAddress *conference_address = NULL;
		const LinphoneAddress *conference_uri = linphone_conference_info_get_uri(updated_conf_info);
		BC_ASSERT_PTR_NOT_NULL(conference_uri);
		if (!conference_uri) {
			goto end;
		}
		conference_address = linphone_address_clone(conference_uri);
		BC_ASSERT_PTR_NOT_NULL(conference_address);
		LinphoneConferenceInfo *updated_conf_info_cloned = linphone_conference_info_clone(updated_conf_info);
		linphone_conference_info_add_participant_2(
		    updated_conf_info_cloned, linphone_conference_info_get_organizer_info(updated_conf_info_cloned));
		reference_conference_infos = bctbx_list_append(reference_conference_infos, updated_conf_info_cloned);

		check_conference_info_in_db(marie, NULL, conference_address, marie->identity, participants_info, start_time,
		                            duration, subject, description, 0, LinphoneConferenceInfoStateNew, security_level,
		                            TRUE, TRUE, TRUE, FALSE);
		linphone_conference_scheduler_unref(conference_scheduler);
		linphone_address_unref(conference_address);
	}

	for (bctbx_list_t *core_it = coresList; core_it; core_it = bctbx_list_next(core_it)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(core_it);
		bctbx_list_t *conference_infos =
		    linphone_account_get_conference_information_list_2(linphone_core_get_default_account(core), NULL);
		if (core == marie->lc) {
			BC_ASSERT_PTR_NOT_NULL(conference_infos);
			BC_ASSERT_EQUAL(bctbx_list_size(conference_infos), bctbx_list_size(reference_conference_infos), size_t,
			                "%zu");
			bctbx_list_free_with_data(conference_infos, (bctbx_list_free_func)linphone_conference_info_unref);
		} else {
			BC_ASSERT_PTR_NULL(conference_infos);
		}
		LinphoneCoreManager *mgr = get_manager(core);
		BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_ConferenceInformationUpdated, 1,
		                             liblinphone_tester_sip_timeout));
		for (const bctbx_list_t *ref_info_it = reference_conference_infos; ref_info_it;
		     ref_info_it = bctbx_list_next(ref_info_it)) {
			LinphoneConferenceInfo *ref_info = (LinphoneConferenceInfo *)bctbx_list_get_data(ref_info_it);
			LinphoneAddress *conference_address = linphone_address_clone(linphone_conference_info_get_uri(ref_info));
			check_conference_info_against_db(mgr, conference_address, ref_info, TRUE);
			linphone_address_unref(conference_address);
		}
	}

end:
	if (organizer_address) linphone_address_unref(organizer_address);
	bctbx_list_free_with_data(reference_conference_infos, (bctbx_list_free_func)linphone_conference_info_unref);
	bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(michelle);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
}

static void simple_conference_two_devices_same_address(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_remove_supported_tag(pauline->lc, "gruu");
	linphone_core_remove_supported_tag(laure->lc, "gruu");

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 2));
	linphone_core_set_network_reachable(laure->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneRegistrationOk, 2));
	simple_conference_base(marie, pauline, laure, NULL, FALSE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void simple_conference_through_inviting_participants(bool_t check_for_proxies) {
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", check_for_proxies, NULL);
	LinphoneCoreManager *laure = create_mgr_for_conference(
	    liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", check_for_proxies, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", check_for_proxies, NULL);
	LinphoneConference *conf = NULL;
	bctbx_list_t *participants = NULL;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call = NULL, *laure_call = NULL, *michelle_call = NULL;
	LinphoneCall *conf_call = NULL;
	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	bctbx_list_t *lcs = NULL;
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;
	stats initial_michelle_stat;

	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", check_for_proxies, NULL);
	// marie creates the conference
	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	lcs = bctbx_list_append(lcs, marie->lc);

	if (check_for_proxies == FALSE) {
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneCoreManager *m = get_manager(c);
			int proxy_count = (int)bctbx_list_size(linphone_core_get_proxy_config_list(m->lc));

			if (proxy_count > 0) {
#define REGISTER_TIMEOUT 20 /* seconds per proxy */
				int success = wait_for_until(m->lc, NULL, &m->stat.number_of_LinphoneRegistrationOk, proxy_count,
				                             (REGISTER_TIMEOUT * 1000 * proxy_count));
				if (!success) {
					ms_error("Did not register after %d seconds for %d proxies", REGISTER_TIMEOUT, proxy_count);
				}
			}
		}
	}

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, michelle);

	add_participant_to_local_conference_through_invite(lcs, marie, participants, NULL);

	pauline_call = linphone_core_get_current_call(pauline->lc);
	laure_call = linphone_core_get_current_call(laure->lc);
	michelle_call = linphone_core_get_current_call(michelle->lc);

	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	BC_ASSERT_PTR_NOT_NULL(michelle_call);

	if (pauline_call && laure_call && michelle_call) {
		const bctbx_list_t *marie_calls, *it;
		linphone_call_accept(pauline_call);
		linphone_call_accept(laure_call);

		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 3, 3000));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));

		// Conference may have already been created of call was paused before hence initial stats can lead to false
		// errors
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

		// Check subscriptions
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionIncomingReceived, 3, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->subscription_received, 3, 5000));

		// make sure that the two calls from Marie's standpoint are in conference
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 3, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next) {
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(
			                   linphone_call_get_current_params((LinphoneCall *)it->data)) == TRUE);
		}
		// wait a bit for the conference audio processing to run, despite we do not test it for the moment
		wait_for_list(lcs, NULL, 0, 5000);

		initial_marie_stat = marie->stat;
		initial_pauline_stat = pauline->stat;
		initial_laure_stat = laure->stat;
		initial_michelle_stat = michelle->stat;

		// Remove Pauline
		conf_call = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		linphone_core_remove_from_conference(marie->lc, conf_call);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
		                             initial_marie_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
		                             initial_marie_stat.number_of_LinphoneCallReleased + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
		                             initial_pauline_stat.number_of_LinphoneCallEnd + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
		                             initial_pauline_stat.number_of_LinphoneCallReleased + 1,
		                             liblinphone_tester_sip_timeout));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (initial_pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
		                             (initial_pauline_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
		                             (initial_pauline_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_pauline_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));

		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conf), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_conference_is_in(conf), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_NotifyReceived,
		                             (initial_laure_stat.number_of_NotifyReceived + 2), 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
		                             (initial_michelle_stat.number_of_NotifyReceived + 2), 3000));

		initial_marie_stat = marie->stat;
		initial_laure_stat = laure->stat;
		initial_michelle_stat = michelle->stat;

		// Remove Michelle
		conf_call = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		linphone_core_remove_from_conference(marie->lc, conf_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
		                             (initial_marie_stat.number_of_LinphoneCallPausing + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
		                             (initial_marie_stat.number_of_LinphoneCallPaused + 1), 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallPausedByRemote,
		                             (initial_michelle_stat.number_of_LinphoneCallPausedByRemote + 1), 5000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (initial_michelle_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated,
		                             (initial_michelle_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted,
		                             (initial_michelle_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_michelle_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_NotifyReceived,
		                             (initial_laure_stat.number_of_NotifyReceived + 2),
		                             liblinphone_tester_sip_timeout));

		// CHeck that conference is not destroyed
		BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
		                              (initial_laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                              1000));
		BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
		                              (initial_laure_stat.number_of_LinphoneConferenceStateTerminated + 1), 1000));
		BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
		                              (initial_laure_stat.number_of_LinphoneConferenceStateDeleted + 1), 1000));

		BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
		                              (initial_marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                              1000));
		BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
		                              (initial_marie_stat.number_of_LinphoneConferenceStateTerminated + 1), 1000));
		BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
		                              (initial_marie_stat.number_of_LinphoneConferenceStateDeleted + 1), 1000));

		// Conference stays active with one participant as Laure's call was created for the conference
		conf = linphone_core_get_conference(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(conf);
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conf), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_conference_is_in(conf), 1, int, "%d");

		// Remove laure
		initial_marie_stat = marie->stat;
		initial_laure_stat = laure->stat;

		conf_call = linphone_core_get_call_by_remote_address2(marie->lc, laure->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		linphone_core_remove_from_conference(marie->lc, conf_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
		                             initial_marie_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
		                             initial_marie_stat.number_of_LinphoneCallReleased + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd,
		                             initial_laure_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
		                             initial_laure_stat.number_of_LinphoneCallReleased + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (initial_laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
		                             (initial_laure_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
		                             (initial_laure_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (initial_marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
		                             (initial_marie_stat.number_of_LinphoneConferenceStateTerminated + 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
		                             (initial_marie_stat.number_of_LinphoneConferenceStateDeleted + 1),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_laure_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));

		end_call(marie, michelle);
	}

end:
	if (conf) linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_not_converted_to_call(void) {
	simple_conference_through_inviting_participants(TRUE);
}

static void on_eof(LinphonePlayer *player) {
	LinphonePlayerCbs *cbs = linphone_player_get_current_callbacks(player);
	LinphoneCoreManager *mgr = (LinphoneCoreManager *)linphone_player_cbs_get_user_data(cbs);
	mgr->stat.number_of_player_eof++;
}

int liblinphone_tester_check_recorded_audio(const char *hellopath, const char *recordpath) {
	double similar = 1;
	const double threshold = 0.9;
	BC_ASSERT_EQUAL(ms_audio_diff(hellopath, recordpath, &similar, &audio_cmp_params, NULL, NULL), 0, int, "%d");
	BC_ASSERT_GREATER(similar, threshold, double, "%g");
	BC_ASSERT_LOWER(similar, 1.0, double, "%g");
	if (similar >= threshold && similar <= 1.0) {
		remove(recordpath);
		return 1;
	}
	return 0;
}

static void simple_conference_with_file_player(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *pauline_dummy_recordpath = bc_tester_file("record-local_conference_with_file_player_dummy_pauline.wav");
	char *laure_dummy_recordpath = bc_tester_file("record-local_conference_with_file_player_dummy_laure.wav");
	char *michelle_dummy_recordpath = bc_tester_file("record-local_conference_with_file_player_dummy_michelle.wav");
	char *pauline_recordpath = bc_tester_file("record-local_conference_with_file_player_pauline.wav");
	char *laure_recordpath = bc_tester_file("record-local_conference_with_file_player_laure.wav");
	char *michelle_recordpath = bc_tester_file("record-local_conference_with_file_player_michelle.wav");

	// Make sure the record fileis don't already exists, otherwise this test will append new samples to it
	unlink(pauline_recordpath);
	unlink(laure_recordpath);
	unlink(michelle_recordpath);

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);
	reset_counters(&michelle->stat);
	linphone_core_reset_tone_manager_stats(marie->lc);
	linphone_core_reset_tone_manager_stats(pauline->lc);
	linphone_core_reset_tone_manager_stats(laure->lc);
	linphone_core_reset_tone_manager_stats(michelle->lc);
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, NULL);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_play_file(pauline->lc, NULL);
	linphone_core_set_record_file(pauline->lc, pauline_dummy_recordpath);
	linphone_core_set_use_files(laure->lc, TRUE);
	linphone_core_set_play_file(laure->lc, NULL);
	linphone_core_set_record_file(laure->lc, laure_dummy_recordpath);
	linphone_core_set_use_files(michelle->lc, TRUE);
	linphone_core_set_play_file(michelle->lc, NULL);
	linphone_core_set_record_file(michelle->lc, michelle_dummy_recordpath);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	bctbx_list_t *unique_participant_identity = NULL;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		if (!BC_ASSERT_TRUE(call(marie, m))) return;
		lcs = bctbx_list_append(lcs, c);
		if (bctbx_list_find(unique_participant_identity, m->identity) == NULL) {
			unique_participant_identity = bctbx_list_append(unique_participant_identity, m->identity);
		}
		LinphoneCall *conf_call_participant = linphone_core_get_current_call(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(conf_call_participant);
		LinphoneCall *participant_called_by_conf = linphone_core_get_current_call(c);
		BC_ASSERT_PTR_NOT_NULL(participant_called_by_conf);
		// Last call is not put on hold
		if (bctbx_list_next(it)) {
			BC_ASSERT_TRUE(pause_call_1(marie, conf_call_participant, m, participant_called_by_conf));
		}
	}
	unsigned int no_unique_participants = (unsigned int)bctbx_list_size(unique_participant_identity);
	bctbx_list_free(unique_participant_identity);

	add_calls_to_local_conference(lcs, marie, NULL, participants, TRUE);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		// Wait that all participants have joined the local conference, by checking the StreamsRunning states
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2 * no_participants,
	                             liblinphone_tester_sip_timeout));

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	if (l_conference) {
		BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), no_unique_participants, int, "%d");

		BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

		LinphonePlayer *player = linphone_conference_get_player(l_conference);
		LinphonePlayerCbs *player_cbs = NULL;
		BC_ASSERT_PTR_NOT_NULL(player);
		if (player) {
			player_cbs = linphone_factory_create_player_cbs(linphone_factory_get());
			linphone_player_cbs_set_eof_reached(player_cbs, on_eof);
			linphone_player_cbs_set_user_data(player_cbs, marie);
			linphone_player_add_callbacks(player, player_cbs);
			linphone_core_set_record_file(pauline->lc, pauline_recordpath);
			linphone_core_set_record_file(laure->lc, laure_recordpath);
			linphone_core_set_record_file(michelle->lc, michelle_recordpath);
			BC_ASSERT_EQUAL(linphone_player_open(player, hellopath), 0, int, "%d");
			BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
		}
		if (player_cbs) linphone_player_cbs_unref(player_cbs);
	}

	// The waiting duration must be at least as long as the wav file
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_player_eof, 1, 10000));
	// Wait one second more for transmission to be fully ended (transmission time + jitter buffer)
	wait_for_list(lcs, NULL, 0, 1000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	bctbx_list_free(lcs);

	bctbx_list_free(participants);

	BC_ASSERT_TRUE(liblinphone_tester_check_recorded_audio(hellopath, pauline_recordpath));
	BC_ASSERT_TRUE(liblinphone_tester_check_recorded_audio(hellopath, laure_recordpath));
	BC_ASSERT_TRUE(liblinphone_tester_check_recorded_audio(hellopath, michelle_recordpath));

	unlink(pauline_dummy_recordpath);
	unlink(laure_dummy_recordpath);
	unlink(michelle_dummy_recordpath);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);

	bctbx_free(hellopath);
	bctbx_free(pauline_recordpath);
	bctbx_free(laure_recordpath);
	bctbx_free(michelle_recordpath);
	bctbx_free(pauline_dummy_recordpath);
	bctbx_free(laure_dummy_recordpath);
	bctbx_free(michelle_dummy_recordpath);
}

static void simple_conference_with_file_player_participants_leave(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	LinphonePlayerCbs *player_cbs = NULL;

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);

	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, NULL);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_play_file(pauline->lc, NULL);
	linphone_core_set_record_file(pauline->lc, NULL);
	linphone_core_set_use_files(laure->lc, TRUE);
	linphone_core_set_play_file(laure->lc, NULL);
	linphone_core_set_record_file(laure->lc, NULL);

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	if (!BC_ASSERT_TRUE(call(pauline, marie))) goto end;
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	// linphone_conference_params_enable_one_participant_conference(conf_params, TRUE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	linphone_conference_add_participant(conf, linphone_core_get_current_call(marie->lc));
	linphone_core_invite_address(marie->lc, laure->identity);

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, liblinphone_tester_sip_timeout));

	LinphonePlayer *player = linphone_conference_get_player(conf);

	BC_ASSERT_PTR_NOT_NULL(player);
	if (player) {
		player_cbs = linphone_factory_create_player_cbs(linphone_factory_get());
		linphone_player_cbs_set_eof_reached(player_cbs, on_eof);
		linphone_player_cbs_set_user_data(player_cbs, marie);
		linphone_player_add_callbacks(player, player_cbs);
		BC_ASSERT_EQUAL(linphone_player_open(player, hellopath), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
		ms_message("Conference player is started.");
	}

	wait_for_list(lcs, NULL, 0, 1000);
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	if (BC_ASSERT_PTR_NOT_NULL(laure_call)) {
		linphone_call_accept(laure_call);
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
		wait_for_list(lcs, NULL, 0, 1000);
	}

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	wait_for_list(lcs, NULL, 0, 1000);

	linphone_core_terminate_all_calls(laure->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, liblinphone_tester_sip_timeout));

	linphone_conference_unref(conf);

end:

	bctbx_list_free(lcs);

	if (player_cbs) linphone_player_cbs_unref(player_cbs);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_free(hellopath);
}

static void simple_conference_established_before_proxy_config_creation(void) {
	simple_conference_through_inviting_participants(FALSE);
}

static void _simple_conference_from_scratch(bool_t with_video) {
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);

	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneConference *conf;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call;
	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	bctbx_list_t *lcs = NULL;
	char record_file_name[255];
	char random_part[10];
	belle_sip_random_token(random_part, sizeof(random_part) - 1);
	snprintf(record_file_name, sizeof(record_file_name), "conference-record-%s.mkv", random_part);
	char *recordfile = bc_tester_file(record_file_name);

	unlink(recordfile);
	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, focus_mgr->lc);

	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	// marie creates the conference
	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, with_video);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);
	linphone_conference_params_unref(conf_params);

	if (with_video) {
		LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
		linphone_core_set_video_activation_policy(pauline->lc, pol);
		linphone_core_set_video_activation_policy(marie->lc, pol);
		linphone_core_set_video_activation_policy(laure->lc, pol);
		linphone_video_activation_policy_unref(pol);

		linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
		linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
		linphone_core_set_video_device(laure->lc, liblinphone_tester_mire_id);

		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_capture(laure->lc, TRUE);
		linphone_core_enable_video_display(laure->lc, TRUE);

		linphone_core_set_default_conference_layout(laure->lc, LinphoneConferenceLayoutActiveSpeaker);
		linphone_core_set_default_conference_layout(pauline->lc, LinphoneConferenceLayoutActiveSpeaker);
	}

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, pauline);

	add_participant_to_local_conference_through_invite(lcs, marie, participants, NULL);

	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params) == with_video);

	pauline_call = linphone_core_get_current_call(pauline->lc);
	laure_call = linphone_core_get_current_call(laure->lc);

	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(laure_call);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_conference_get_conference_address(conf));

	if (pauline_call && laure_call) {
		const bctbx_list_t *marie_calls, *it;
		linphone_call_accept(pauline_call);
		linphone_call_accept(laure_call);

		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2, 3000));

		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));

		// Conference may have already been created of call was paused before hence initial stats can lead to false
		// errors
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

		// Check subscriptions
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionIncomingReceived, 2, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->subscription_received, 2, 5000));

		// make sure that the two calls from Marie's standpoint are in conference
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 2, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next) {
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(
			                   linphone_call_get_current_params((LinphoneCall *)it->data)) == TRUE);
		}
		linphone_conference_start_recording(conf, recordfile);
		wait_for_list(lcs, NULL, 0, 5500);
		linphone_conference_stop_recording(conf);

		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) ==
		               with_video);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == with_video);

		terminate_conference(participants, marie, NULL, NULL, FALSE);
	}
	/* make sure that the recorded file has correct length */
	{
		LinphonePlayer *player = linphone_core_create_local_player(marie->lc, NULL, NULL, NULL);
		if (BC_ASSERT_PTR_NOT_NULL(player)) {
			BC_ASSERT_TRUE(linphone_player_open(player, recordfile) == 0);
			BC_ASSERT_GREATER(linphone_player_get_duration(player), 5000, int, "%i");
		}
		linphone_player_close(player);
		linphone_player_unref(player);
	}

	bctbx_list_t *participants_info = NULL;
	add_participant_info_to_list(&participants_info, pauline->identity, LinphoneParticipantRoleSpeaker, -1);
	add_participant_info_to_list(&participants_info, laure->identity, LinphoneParticipantRoleSpeaker, -1);
	add_participant_info_to_list(&participants_info, marie->identity, LinphoneParticipantRoleSpeaker, -1);
	LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie->lc, confAddr);
	BC_ASSERT_PTR_NULL(info);
	check_conference_info_in_db(pauline, NULL, confAddr, marie->identity, participants_info, 0, 0, NULL, NULL, 0,
	                            LinphoneConferenceInfoStateNew, LinphoneConferenceSecurityLevelNone, FALSE, TRUE,
	                            with_video, FALSE);
	check_conference_info_in_db(laure, NULL, confAddr, marie->identity, participants_info, 0, 0, NULL, NULL, 0,
	                            LinphoneConferenceInfoStateNew, LinphoneConferenceSecurityLevelNone, FALSE, TRUE,
	                            with_video, FALSE);

	bc_free(recordfile);
	bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	linphone_address_unref(confAddr);
	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_from_scratch(void) {
	_simple_conference_from_scratch(FALSE);
}

static void simple_conference_from_scratch_with_video(void) {
	_simple_conference_from_scratch(TRUE);
}

static void video_conference_by_merging_calls(void) {
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(focus_mgr->lc), "sip", "reject_duplicated_calls", 0);

	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneConference *conf = NULL;
	LinphoneCallParams *params;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call, *marie_call;
	bctbx_list_t *new_participants = NULL;
	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, focus_mgr->lc);

	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	{
		LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
		linphone_core_set_video_activation_policy(pauline->lc, pol);
		linphone_core_set_video_activation_policy(marie->lc, pol);
		linphone_core_set_video_activation_policy(laure->lc, pol);
		linphone_core_set_video_activation_policy(focus_mgr->lc, pol);
		linphone_video_activation_policy_unref(pol);

		linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
		linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
		linphone_core_set_video_device(laure->lc, liblinphone_tester_mire_id);
		linphone_core_set_video_device(focus_mgr->lc, liblinphone_tester_mire_id);

		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_capture(laure->lc, TRUE);
		linphone_core_enable_video_display(laure->lc, TRUE);
		linphone_core_enable_video_capture(focus_mgr->lc, TRUE);
		linphone_core_enable_video_display(focus_mgr->lc, TRUE);
	}

	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	// Marie first estabishes a call with Pauline, with video.
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(params, TRUE);
	pauline_call = linphone_core_invite_address_with_params(marie->lc, pauline->identity, params);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
	if (BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1,
	                                 liblinphone_tester_sip_timeout))) {
		linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	} else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

	// Then she calls Laure with video
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(params, TRUE);
	laure_call = linphone_core_invite_address_with_params(marie->lc, laure->identity, params);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingProgress, 2, liblinphone_tester_sip_timeout));
	if (BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1,
	                                 liblinphone_tester_sip_timeout))) {
		linphone_call_accept(linphone_core_get_current_call(laure->lc));
	} else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

	// This of course puts on hold Pauline.
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1, 5000));

	// Marie now creates a conference - join with video.
	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	// She adds Pauline and Laure to the conference.
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_remote_conference(lcs, focus_mgr, marie, new_participants, conf, TRUE);

	// Now check that both Pauline and Laure have video.
	pauline_call = linphone_core_get_current_call(pauline->lc);
	laure_call = linphone_core_get_current_call(laure->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (pauline_call && laure_call && marie_call) {
		const bctbx_list_t *focus_calls, *it;

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));

		// make sure that the two calls from Marie's standpoint are in conference
		focus_calls = linphone_core_get_calls(focus_mgr->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(focus_calls), 3, int, "%i");
		for (it = focus_calls; it != NULL; it = it->next) {
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(
			                   linphone_call_get_current_params((LinphoneCall *)it->data)) == TRUE);
		}
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)));

		LinphoneAddress *conference_address = linphone_address_clone(linphone_conference_get_conference_address(conf));
		BC_ASSERT_PTR_NOT_NULL(conference_address);

		// Change camera, unfortunately there is no way to test its effectiveness for the moment.
		ms_message("Changing Marie's video device...");
		linphone_core_set_video_device(marie->lc, liblinphone_tester_static_image_id);
		wait_for_list(lcs, NULL, 0, 2000);

		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		stats initial_laure_stat = laure->stat;

		ms_message("Removing video to the conference...");
		// Marie upgrades the conference with video.
		LinphoneConference *focus_conference =
		    linphone_core_search_conference(focus_mgr->lc, NULL, NULL, conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(focus_conference);
		if (focus_conference) {
			conf_params = linphone_core_create_conference_params_2(focus_mgr->lc, focus_conference);
			linphone_conference_params_enable_video(conf_params, FALSE);
			linphone_conference_update_params(focus_conference, conf_params);
			linphone_conference_params_unref(conf_params);
		}

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_available_media_changed,
		                             initial_marie_stat.number_of_available_media_changed + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_available_media_changed,
		                             initial_pauline_stat.number_of_available_media_changed + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_available_media_changed,
		                             initial_laure_stat.number_of_available_media_changed + 1, 3000));

		if (conference_address) {
			LinphoneConference *pauline_conference =
			    linphone_core_search_conference(pauline->lc, NULL, NULL, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(pauline_conference);
			if (pauline_conference) {
				const LinphoneConferenceParams *current_pauline_conf_params =
				    linphone_conference_get_current_params(pauline_conference);
				BC_ASSERT_PTR_NOT_NULL(current_pauline_conf_params);
				BC_ASSERT_FALSE(linphone_conference_params_video_enabled(current_pauline_conf_params));
			}

			LinphoneConference *marie_conference =
			    linphone_core_search_conference(marie->lc, NULL, NULL, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(marie_conference);
			if (marie_conference) {
				const LinphoneConferenceParams *current_marie_conf_params =
				    linphone_conference_get_current_params(marie_conference);
				BC_ASSERT_PTR_NOT_NULL(current_marie_conf_params);
				BC_ASSERT_FALSE(linphone_conference_params_video_enabled(current_marie_conf_params));
			}

			LinphoneConference *laure_conference =
			    linphone_core_search_conference(laure->lc, NULL, NULL, conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(laure_conference);
			if (laure_conference) {
				const LinphoneConferenceParams *current_laure_conf_params =
				    linphone_conference_get_current_params(laure_conference);
				BC_ASSERT_PTR_NOT_NULL(current_laure_conf_params);
				BC_ASSERT_FALSE(linphone_conference_params_video_enabled(current_laure_conf_params));
			}
		}
		linphone_address_unref(conference_address);

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallUpdating,
		                             initial_laure_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating,
		                             initial_marie_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating,
		                             initial_pauline_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));

		terminate_conference(new_participants, marie, conf, focus_mgr, FALSE);
	}

end:
	if (conf) linphone_conference_unref(conf);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(new_participants);

	bctbx_list_free(lcs);
}

static void simple_conference_from_scratch_no_answer(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneConference *conf;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	// marie creates the conference
	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, FALSE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	linphone_conference_unref(conf);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, pauline);

	add_participant_to_local_conference_through_invite(lcs, marie, participants, NULL);

	lcs = bctbx_list_remove(lcs, laure->lc);

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		pauline_call = linphone_core_get_current_call(pauline->lc);
		// Pauline immediately declines the call.
		linphone_call_decline(pauline_call, LinphoneReasonDeclined);
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 1, 5000));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 1, 5000));

	wait_for_list(lcs, NULL, 0, 1000);

	lcs = bctbx_list_append(lcs, laure->lc);
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, liblinphone_tester_sip_timeout));
	laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);

	if (laure_call) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 2, 2000));

		wait_for_list(lcs, NULL, 0, 1000);

		// Laure accepts.
		linphone_call_accept(laure_call);
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		wait_for_list(lcs, NULL, 0, 1000);

		// the conference no longer exists, as there was finally only one participant.
		// Terminate the call, simply.
		linphone_call_terminate(laure_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 1000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
		                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
		                             marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	}

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_encrypted_conference_with_ice(LinphoneMediaEncryption mode) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	if (linphone_core_media_encryption_supported(marie->lc, mode)) {
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(laure->lc, LinphonePolicyUseIce);

		linphone_core_set_media_encryption(marie->lc, mode);
		linphone_core_set_media_encryption(pauline->lc, mode);
		linphone_core_set_media_encryption(laure->lc, mode);

		simple_conference_base(marie, pauline, laure, NULL, FALSE);
	} else {
		ms_warning("No [%s] support available", linphone_media_encryption_to_string(mode));
		BC_PASS("Passed");
	}

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void simple_conference_with_ice(void) {
	simple_encrypted_conference_with_ice(LinphoneMediaEncryptionNone);
}

static void simple_zrtp_conference_with_ice(void) {
	simple_encrypted_conference_with_ice(LinphoneMediaEncryptionZRTP);
}

static void conference_without_event_pkg_hang_up_call_on_hold(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_config_set_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_log_enabled", FALSE);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	simple_conference_base(marie, pauline, laure, NULL, TRUE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void conference_with_event_pkg_hang_up_call_on_hold(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	simple_conference_base(marie, pauline, laure, NULL, TRUE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void eject_from_3_participants_conference(LinphoneCoreManager *marie,
                                                 LinphoneCoreManager *pauline,
                                                 LinphoneCoreManager *laure,
                                                 LinphoneCoreManager *focus,
                                                 const LinphoneConferenceLayout layout) {
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;

	linphone_core_set_default_conference_layout(pauline->lc, layout);
	linphone_core_set_default_conference_layout(laure->lc, layout);
	linphone_core_set_default_conference_layout(marie->lc, layout);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	bool_t is_remote_conf;
	LinphoneConference *conf = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	if (focus) lcs = bctbx_list_append(lcs, focus->lc);

	is_remote_conf =
	    (strcmp(linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "conference_type", "local"),
	            "remote") == 0);
	if (is_remote_conf) BC_ASSERT_PTR_NOT_NULL(focus);

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;
	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;
	initial_laure_stat = laure->stat;

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	// Marie creates the conference
	const char *subject = "Coffee break";
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_set_subject(conf_params, subject);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateInstantiated, 1, 5000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 0, int, "%0d");

	if (!is_remote_conf) {
		bctbx_list_t *new_participants = bctbx_list_append(NULL, laure);
		// TODO: Find a way to extract participants managers from conference
		bctbx_list_t *lcs2 = bctbx_list_append(NULL, marie->lc);
		lcs2 = bctbx_list_append(lcs2, laure->lc);
		add_calls_to_local_conference(lcs2, marie, conf, new_participants, TRUE);
		bctbx_list_free(lcs2);
		bctbx_list_free(new_participants);
	} else {

		linphone_conference_add_participant(conf, marie_call_laure);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending,
		                             initial_marie_stat.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated,
		                             initial_marie_stat.number_of_LinphoneConferenceStateCreated + 1, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallConnected,
		                             initial_marie_stat.number_of_LinphoneTransferCallConnected + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
		                             initial_marie_stat.number_of_LinphoneCallEnd + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd,
		                             initial_laure_stat.number_of_LinphoneCallEnd + 1, 5000));

		// Try to change conference address after creation.
		const char *new_conference_address_str = "sip:toto@sip.example.org";
		LinphoneAddress *new_conference_address = linphone_address_new(new_conference_address_str);
		BC_ASSERT_PTR_NOT_NULL(new_conference_address);
		if (new_conference_address) {
			linphone_conference_set_conference_address(conf, new_conference_address);
			linphone_address_unref(new_conference_address);
		}

		const LinphoneAddress *current_conference_address = linphone_conference_get_conference_address(conf);
		char *current_conference_address_str = linphone_address_as_string(current_conference_address);
		// Change of conference address after creation is not allowed.
		BC_ASSERT_STRING_NOT_EQUAL(current_conference_address_str, new_conference_address_str);
		ms_free(current_conference_address_str);
	}

	if (!is_remote_conf) {
		bctbx_list_t *new_participants = bctbx_list_append(NULL, pauline);
		add_calls_to_local_conference(lcs, marie, conf, new_participants, FALSE);
		bctbx_list_free(new_participants);
	} else {
		linphone_conference_add_participant(conf, marie_call_pauline);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallConnected,
		                             initial_marie_stat.number_of_LinphoneTransferCallConnected + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
		                             initial_marie_stat.number_of_LinphoneCallEnd + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
		                             initial_pauline_stat.number_of_LinphoneCallEnd + 1, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_marie_stat.number_of_LinphoneCallStreamsRunning + 2, 3000));
	}

	wait_for_list(lcs, NULL, 0, 2000);

	BC_ASSERT_TRUE(linphone_conference_is_in(conf));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conf), 2, int, "%d");

	const LinphoneAddress *conference_address = linphone_conference_get_conference_address(conf);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *lc = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *pconference = linphone_core_search_conference(lc, NULL, NULL, conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject);
		}
	}

	if (!is_remote_conf) BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	ms_message("Removing pauline from conference.");
	if (!is_remote_conf) {
		remove_participant_from_local_conference(lcs, marie, pauline, conf);
	} else {
		LinphoneConference *conference = linphone_core_get_conference(marie->lc);
		const LinphoneAddress *uri = linphone_call_get_remote_address(marie_call_pauline);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			LinphoneParticipant *pauline_participant = linphone_conference_find_participant(conference, uri);
			BC_ASSERT_PTR_NOT_NULL(pauline_participant);
			if (pauline_participant) {
				linphone_conference_remove_participant_2(conference, pauline_participant);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
		                             initial_pauline_stat.number_of_LinphoneCallEnd + 2, 5000));
	}

	if (!is_remote_conf) {
		end_call(laure, marie);
		end_call(pauline, marie);

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
		                             initial_laure_stat.number_of_LinphoneCallReleased + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
		                             initial_marie_stat.number_of_LinphoneCallReleased + 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
		                             initial_pauline_stat.number_of_LinphoneCallReleased + 1, 3000));
	} else {
		linphone_core_terminate_conference(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd,
		                             initial_laure_stat.number_of_LinphoneCallEnd + 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
		                             initial_marie_stat.number_of_LinphoneCallEnd + 3, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
		                             initial_laure_stat.number_of_LinphoneCallReleased + 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
		                             initial_marie_stat.number_of_LinphoneCallReleased + 3, 3000));

		int marie_call_no = (int)bctbx_list_size(linphone_core_get_calls(marie->lc));
		BC_ASSERT_EQUAL(marie_call_no, 0, int, "%0d");
	}
end:
	bctbx_list_free(lcs);
	if (conf) {
		linphone_conference_unref(conf);
	}
}

static void eject_from_3_participants_local_conference_grid_layout(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_video_activation_policy_unref(pol);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(laure->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(laure->lc, TRUE);
	linphone_core_enable_video_display(laure->lc, TRUE);

	eject_from_3_participants_conference(marie, pauline, laure, NULL, LinphoneConferenceLayoutGrid);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void eject_from_3_participants_local_conference_active_speaker_layout(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_video_activation_policy_unref(pol);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(laure->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(laure->lc, TRUE);
	linphone_core_enable_video_display(laure->lc, TRUE);

	eject_from_3_participants_conference(marie, pauline, laure, NULL, LinphoneConferenceLayoutActiveSpeaker);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void eject_from_4_participants_conference_call_terminated_one_by_one(LinphoneCoreManager *marie,
                                                                            LinphoneCoreManager *pauline,
                                                                            LinphoneCoreManager *laure,
                                                                            LinphoneCoreManager *michelle) {
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;
	stats initial_michelle_stat;

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *marie_call_laure;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;
	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;
	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;
	initial_laure_stat = laure->stat;
	initial_michelle_stat = michelle->stat;

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(laure->lc, marie->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, laure->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(michelle->lc, marie->identity)) ==
	               LinphoneCallPausedByRemote);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity)) ==
	               LinphoneCallPaused);

	bctbx_list_t *new_participants = bctbx_list_append(NULL, michelle);
	bctbx_list_t *lcs2 = bctbx_list_append(NULL, marie->lc);
	lcs2 = bctbx_list_append(lcs2, michelle->lc);
	add_calls_to_local_conference(lcs2, marie, NULL, new_participants, TRUE);
	bctbx_list_free(new_participants);

	// As the call between Marie and Michelle is added to the conference while in the paused state, the local
	// participant is not added in and the call between Marie and Laure keeps being in state StreamsRunning
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(michelle->lc, marie->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(laure->lc, marie->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, laure->identity)) ==
	               LinphoneCallStreamsRunning);

	if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_conference(marie->lc))) {
		goto end;
	}

	new_participants = bctbx_list_append(NULL, pauline);
	lcs2 = bctbx_list_append(lcs2, pauline->lc);
	add_calls_to_local_conference(lcs2, marie, NULL, new_participants, FALSE);
	bctbx_list_free(lcs2);
	bctbx_list_free(new_participants);

	wait_for_list(lcs, NULL, 0, 2000);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));

	new_participants = bctbx_list_append(NULL, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);
	bctbx_list_free(new_participants);

	wait_for_list(lcs, NULL, 0, 2000);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	initial_marie_stat = marie->stat;
	initial_pauline_stat = pauline->stat;
	initial_laure_stat = laure->stat;
	initial_michelle_stat = michelle->stat;

	linphone_call_terminate(marie_call_pauline);
	linphone_call_terminate(marie_call_michelle);
	linphone_call_terminate(marie_call_laure);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
	                             initial_pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd,
	                             initial_michelle_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd,
	                             initial_laure_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
	                             initial_marie_stat.number_of_LinphoneCallEnd + 3, liblinphone_tester_sip_timeout));

	// Wait for conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (initial_michelle_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
	                             5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated,
	                             (initial_michelle_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted,
	                             (initial_michelle_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (initial_laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             (initial_laure_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             (initial_laure_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (initial_pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             (initial_pauline_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             (initial_pauline_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (initial_marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             (initial_marie_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             (initial_marie_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionTerminated,
	                             initial_michelle_stat.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
	                             initial_pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
	                             initial_laure_stat.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 3,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
	                             initial_pauline_stat.number_of_LinphoneCallReleased + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased,
	                             initial_michelle_stat.number_of_LinphoneCallReleased + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
	                             initial_laure_stat.number_of_LinphoneCallReleased + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
	                             initial_marie_stat.number_of_LinphoneCallReleased + 3, 3000));
end:
	bctbx_list_free(lcs);
}

static void eject_from_4_participants_local_conference_call_terminated_one_by_one(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	eject_from_4_participants_conference_call_terminated_one_by_one(marie, pauline, laure, michelle);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void eject_from_4_participants_conference_base(const LinphoneConferenceLayout layout) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneConference *conf = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	linphone_core_set_default_conference_layout(michelle->lc, layout);
	linphone_core_set_default_conference_layout(laure->lc, layout);
	linphone_core_set_default_conference_layout(pauline->lc, layout);
	linphone_core_set_default_conference_layout(marie->lc, layout);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call(marie, laure));

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	// Marie creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 0, int, "%0d");

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, laure);
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	/* Wait that the three participants are joined to the local conference, by checking the StreamsRunning states*/
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	remove_participant_from_local_conference(lcs, marie, pauline, NULL);
	new_participants = bctbx_list_remove(new_participants, pauline);

	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");
	int marie_call_no = (int)bctbx_list_size(linphone_core_get_calls(marie->lc));
	BC_ASSERT_EQUAL(marie_call_no, 3, int, "%0d");
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	wait_for_list(lcs, NULL, 0, 2000);

	LinphoneCall *marie_calls_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_calls_pauline);
	if (marie_calls_pauline) {
		linphone_call_terminate(marie_calls_pauline);
	}

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, marie_call_no, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, marie_call_no, liblinphone_tester_sip_timeout));

end:

	if (conf) {
		linphone_conference_unref(conf);
	}
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void eject_from_4_participants_conference_grid_layout(void) {
	eject_from_4_participants_conference_base(LinphoneConferenceLayoutGrid);
}

static void eject_from_4_participants_conference_active_speaker_layout(void) {
	eject_from_4_participants_conference_base(LinphoneConferenceLayoutActiveSpeaker);
}

static void participants_exit_conference_after_pausing(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneAddress *marie_conference_address = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call(marie, laure));

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, laure);
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);
	bctbx_list_free(new_participants);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	if (!marie_conference) goto end;

	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	unsigned int marie_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), marie_call_no, int, "%d");

	bctbx_list_t *participants = linphone_conference_get_participant_list(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), marie_call_no, unsigned int, "%u");
	bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	stats marie_stats = marie->stat;
	stats pauline_stats = pauline->stat;
	stats laure_stats = laure->stat;
	stats michelle_stats = michelle->stat;

	int no_parts = marie_call_no;

	marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));

	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(laure_conference));

	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));

	LinphoneCall *laure_call_marie = linphone_core_get_current_call(laure->lc);
	linphone_core_pause_call(laure->lc, laure_call_marie);

	// Pausing participant calls causes a NOTIFY to be sent to the participants to notify of the change in media
	// capabilities
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausing,
	                             laure_stats.number_of_LinphoneCallPausing + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused,
	                             laure_stats.number_of_LinphoneCallPaused + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             marie_stats.number_of_LinphoneCallPausedByRemote + 1, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived,
	                             (pauline_stats.number_of_NotifyReceived + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 5000));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference), no_parts, int, "%d");
	BC_ASSERT_FALSE(linphone_conference_is_in(laure_conference));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));

	LinphoneCall *pauline_call_marie = linphone_core_get_current_call(pauline->lc);
	linphone_core_pause_call(pauline->lc, pauline_call_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
	                             pauline_stats.number_of_LinphoneCallPausing + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
	                             pauline_stats.number_of_LinphoneCallPaused + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             marie_stats.number_of_LinphoneCallPausedByRemote + 2, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_NotifyReceived, (laure_stats.number_of_NotifyReceived + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 2), 5000));

	// Check that conferences are not terminated
	BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                              (laure_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                              (laure_stats.number_of_LinphoneConferenceStateTerminated + 1), 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                              (laure_stats.number_of_LinphoneConferenceStateDeleted + 1), 1000));

	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                              (pauline_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                              (pauline_stats.number_of_LinphoneConferenceStateTerminated + 1), 1000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                              (pauline_stats.number_of_LinphoneConferenceStateDeleted + 1), 1000));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), no_parts, int, "%d");
	BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference), no_parts, int, "%d");
	BC_ASSERT_FALSE(linphone_conference_is_in(laure_conference));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));

	// Remove Michelle from conference.
	remove_participant_from_local_conference(lcs, marie, michelle, NULL);

	// Wait for conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (michelle_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated,
	                             (michelle_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted,
	                             (michelle_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionTerminated,
	                             michelle_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             marie_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	linphone_core_terminate_all_calls(marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
	                             pauline_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
	                             laure_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (laure_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             (laure_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             (laure_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (pauline_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             (pauline_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             (pauline_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (marie_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             (marie_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             (marie_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             marie_stats.number_of_LinphoneSubscriptionTerminated + 3,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
	                             marie_stats.number_of_LinphoneCallEnd + marie_call_no,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, laure_stats.number_of_LinphoneCallEnd + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
	                             pauline_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd,
	                             michelle_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
	                             marie_stats.number_of_LinphoneCallReleased + marie_call_no,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
	                             laure_stats.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
	                             pauline_stats.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased,
	                             michelle_stats.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	pauline_conference = linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NULL(pauline_conference);

	laure_conference = linphone_core_search_conference(laure->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NULL(laure_conference);

	michelle_conference = linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NULL(michelle_conference);

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
}

static void add_participant_after_conference_started_base(bool_t pause_all_calls) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	if (pause_all_calls) {
		BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));
	}

	bctbx_list_t *participants = NULL;
	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	// If calls were paused before creating the conference, local participant is not added
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference) == pause_all_calls);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, laure));

	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	if (pause_all_calls) {
		BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));
	}

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *additional_participants = NULL;
	additional_participants = bctbx_list_append(additional_participants, laure);
	participants = bctbx_list_append(participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, additional_participants, TRUE);
	bctbx_list_free(additional_participants);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	// If Laure's call was paused before creating the conference, local participant is not added
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference) == pause_all_calls);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	if (pause_all_calls) {
		// Marie enters conference
		stats pauline_stats = pauline->stat;
		stats laure_stats = laure->stat;
		stats michelle_stats = michelle->stat;

		linphone_conference_enter(l_conference);

		// Participants are notified that Marie has entered the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participants_added,
		                             michelle_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participants_added,
		                             laure_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_added,
		                             pauline_stats.number_of_participants_added + 1, 3000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_added,
		                             michelle_stats.number_of_participant_devices_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_added,
		                             laure_stats.number_of_participant_devices_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_added,
		                             pauline_stats.number_of_participant_devices_added + 1, 3000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_present,
		                             michelle_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_conference_participant_devices_present,
		                             laure_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_conference_participant_devices_present,
		                             pauline_stats.number_of_conference_participant_devices_present + 1, 3000));
	}

	// Generate traffic to compute volumes correctly
	wait_for_list(lcs, NULL, 0, 1000);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *lc = (LinphoneCore *)bctbx_list_get_data(it);
		for (const bctbx_list_t *call_it = linphone_core_get_calls(lc); call_it; call_it = bctbx_list_next(call_it)) {
			LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(call_it);
			check_conference_volumes(call);
		}
	}

	terminate_conference(participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void add_participant_after_conference_started(void) {
	add_participant_after_conference_started_base(FALSE);
}

static void add_paused_calls_to_conference(void) {
	add_participant_after_conference_started_base(TRUE);
}

static void conference_with_last_call_paused(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	bctbx_list_t *participants = NULL;
	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void add_all_calls_to_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call(marie, laure));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);
	linphone_core_add_all_to_conference(marie->lc);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	// SUBSCRIBEs
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 3, 3000));

	// NOTIFYs
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_NotifyReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived, 1, 3000));

	// Conferences
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	// Even though calls were paused before creating the conference, local participant is added
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void add_call_not_accepted_to_conference_remote(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_called_by_pauline = NULL;
	LinphoneCall *pauline_call_marie = NULL;
	LinphoneCall *marie_called_by_michelle = NULL;
	LinphoneCall *michelle_call_marie = NULL;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	// Michelle calls Marie
	michelle_call_marie = linphone_core_invite_address(michelle->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(michelle_call_marie);
	if (!michelle_call_marie) goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallOutgoingInit, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 5000));

	marie_called_by_michelle = linphone_core_get_current_call(marie->lc);
	marie_called_by_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_called_by_michelle);
	linphone_core_add_to_conference(marie->lc, marie_called_by_michelle);

	wait_for_list(lcs, NULL, 0, 2000);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 1, int, "%d");

	participants = bctbx_list_append(participants, michelle);

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallOutgoingRinging, 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(marie_called_by_michelle);

	linphone_call_accept(marie_called_by_michelle);

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 1, int, "%d");

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived, 1, 3000));

	const LinphoneAddress *l_conference_address = linphone_conference_get_conference_address(l_conference);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference = linphone_core_search_conference(m->lc, NULL, uri, l_conference_address, NULL);
		linphone_address_unref(uri);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (!conference) goto end;
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 1, int, "%d");
	}

	lcs = bctbx_list_append(lcs, pauline->lc);

	// Pauline calls Marie
	pauline_call_marie = linphone_core_invite_address(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_call_marie);
	if (!pauline_call_marie) goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1, 5000));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived, 2, liblinphone_tester_sip_timeout));

	marie_called_by_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_called_by_pauline);
	linphone_core_add_to_conference(marie->lc, marie_called_by_pauline);

	wait_for_list(lcs, NULL, 0, 2000);

	participants = bctbx_list_append(participants, pauline);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(marie_called_by_pauline);

	linphone_call_accept(marie_called_by_pauline);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));

	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived, 1, 3000));

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference = linphone_core_search_conference(m->lc, NULL, uri, l_conference_address, NULL);
		linphone_address_unref(uri);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (!conference) goto end;

		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 2, int, "%d");
	}

	BC_ASSERT_TRUE(call(laure, marie));

	wait_for_list(lcs, NULL, 0, 2000);

	stats marie_initial_stats = marie->stat;
	stats michelle_initial_stats = michelle->stat;
	stats pauline_initial_stats = pauline->stat;

	end_call(michelle, marie);

	// Wait for conferences to be terminated
	// As there is an active call between Marie and Laure, then pause call between Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (michelle_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
	                             5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated,
	                             (michelle_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted,
	                             (michelle_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (pauline_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
	                             5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             (pauline_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             (pauline_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote,
	                             (pauline_initial_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
	                             (marie_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
	                             (marie_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             marie_initial_stats.number_of_LinphoneSubscriptionTerminated + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionTerminated,
	                             michelle_initial_stats.number_of_LinphoneSubscriptionTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
	                             pauline_initial_stats.number_of_LinphoneSubscriptionTerminated + 1, 3000));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 2, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 1, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(michelle->lc)), 0, unsigned int, "%u");

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity));

	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(laure->lc, marie->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, laure->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity)) ==
	               LinphoneCallPausedByRemote);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity)) ==
	               LinphoneCallPaused);

	wait_for_list(lcs, NULL, 0, 2000);

	end_call(laure, marie);
	end_call(marie, pauline);

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(michelle->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	bctbx_list_free(participants);
}

static void add_call_not_accepted_to_conference_local(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	// Marie calls Michelle
	marie_call_michelle = linphone_core_invite_address(marie->lc, michelle->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call_michelle);
	if (!marie_call_michelle) goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingInit, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallIncomingReceived, 1, 5000));

	linphone_core_add_to_conference(marie->lc, marie_call_michelle);

	wait_for_list(lcs, NULL, 0, 2000);

	participants = bctbx_list_append(participants, michelle);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 5000));
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_PTR_NOT_NULL(michelle_called_by_marie);

	linphone_call_accept(michelle_called_by_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 1, int, "%d");

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived, 1, 3000));

	const LinphoneAddress *l_conference_address = linphone_conference_get_conference_address(l_conference);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference = linphone_core_search_conference(m->lc, NULL, uri, l_conference_address, NULL);
		linphone_address_unref(uri);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (!conference) goto end;

		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 1, int, "%d");
	}

	lcs = bctbx_list_append(lcs, pauline->lc);

	// Marie calls Pauline
	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	bctbx_list_t *new_participants = NULL;
	participants = bctbx_list_append(participants, pauline);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);
	bctbx_list_free(new_participants);

	lcs = bctbx_list_append(lcs, laure->lc);

	// As calls were paused before adding to the conference, local participant is not added
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference = linphone_core_search_conference(m->lc, NULL, uri, l_conference_address, NULL);
		linphone_address_unref(uri);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (!conference) goto end;

		// Marie left conference to call Pauline and she is not brought back into the conference because call
		// between Pauline and her was paused
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 1, int, "%d");
	}

	// Marie calls Laure
	marie_call_laure = linphone_core_invite_address(marie->lc, laure->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call_laure);
	if (!marie_call_laure) goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingInit, 3, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 3, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 5000));
	linphone_core_add_to_conference(marie->lc, marie_call_laure);

	wait_for_list(lcs, NULL, 0, 2000);

	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_called_by_marie);

	linphone_call_accept(laure_called_by_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 3, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 3, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_NotifyReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived, 3, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived, 5, 3000));

	participants = bctbx_list_append(participants, laure);

	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference = linphone_core_search_conference(m->lc, NULL, uri, l_conference_address, NULL);
		linphone_address_unref(uri);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (!conference) goto end;

		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), 3, int, "%d");
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	bctbx_list_free(participants);
}

static void remove_participant_from_video_conference_base(const LinphoneConferenceLayout layout) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneConference *conf = NULL;
	LinphoneConferenceParams *conf_params = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == TRUE);

		linphone_core_set_default_conference_layout(c, layout);
	}

	linphone_video_activation_policy_unref(pol);

	const LinphoneCallParams *negotiated_call_params = NULL;
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCallParams *michelle_call_params = linphone_core_create_call_params(michelle->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_set_conference_video_layout(marie_call_params, layout);
	linphone_call_params_enable_video(pauline_call_params, TRUE);
	linphone_call_params_enable_video(laure_call_params, TRUE);
	linphone_call_params_enable_video(michelle_call_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call_with_params(marie, michelle, marie_call_params, michelle_call_params))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	linphone_call_params_unref(laure_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(michelle_call_params);

	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	// Temporary wirkaround to avoid having multiple invites on the same dialog at the same time
	add_calls_to_local_conference(lcs, marie, conf, new_participants, TRUE);
	// add_calls_to_local_conference(lcs, marie, conf, new_participants, FALSE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");
	BC_ASSERT_PTR_EQUAL(conf, l_conference);

	// Check that video capabilities are enabled in the conference
	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(l_conference);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	// Check that video is still on
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));

			const LinphoneConferenceParams *current_remote_conf_params =
			    linphone_conference_get_current_params(conference);
			BC_ASSERT_PTR_NOT_NULL(current_remote_conf_params);
			BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_remote_conf_params));
		}
	}

	wait_for_list(lcs, NULL, 0, 2000);

	remove_participant_from_local_conference(lcs, marie, laure, NULL);
	end_call(laure, marie);

	participants = bctbx_list_remove(participants, laure);

	no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		if (c == laure->lc) {
			BC_ASSERT_PTR_NULL(conference);
		} else {
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(conference));

				const LinphoneConferenceParams *current_remote_conf_params =
				    linphone_conference_get_current_params(conference);
				BC_ASSERT_PTR_NOT_NULL(current_remote_conf_params);
				BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_remote_conf_params));
			}
		}
	}

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:
	if (conf) {
		linphone_conference_unref(conf);
	}
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void remove_participant_from_video_conference_active_speaker_layout(void) {
	remove_participant_from_video_conference_base(LinphoneConferenceLayoutActiveSpeaker);
}

static void remove_participant_from_video_conference_grid_layout(void) {
	remove_participant_from_video_conference_base(LinphoneConferenceLayoutGrid);
}

static void conference_created_by_merging_video_calls_base(bool_t event_package_enabled,
                                                           bool_t enable_video,
                                                           const LinphoneConferenceLayout layout,
                                                           bool_t enable_ice,
                                                           bool_t enable_one_participant_conference,
                                                           bool_t participants_exit_conference) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	linphone_config_set_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_log_enabled",
	                         event_package_enabled);

	LinphoneConference *conf = NULL;
	LinphoneConferenceParams *conf_params = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	const LinphoneCallParams *params = NULL;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == TRUE);

		linphone_config_set_int(linphone_core_get_config(c), "rtp", "rtcp_mux", 1);

		if (enable_ice) {
			linphone_core_set_firewall_policy(c, LinphonePolicyUseIce);
		}

		linphone_core_set_default_conference_layout(c, layout);
	}

	linphone_video_activation_policy_unref(pol);

	const LinphoneCallParams *negotiated_call_params = NULL;
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCallParams *michelle_call_params = linphone_core_create_call_params(michelle->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_enable_video(pauline_call_params, TRUE);
	linphone_call_params_enable_video(laure_call_params, TRUE);
	linphone_call_params_enable_video(michelle_call_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	params = linphone_call_get_remote_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);

	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	params = linphone_call_get_remote_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);

	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call_with_params(marie, michelle, marie_call_params, michelle_call_params))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	params = linphone_call_get_remote_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);

	linphone_call_params_unref(laure_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(michelle_call_params);

	if (enable_ice) {
		check_ice(marie, michelle, LinphoneIceStateHostConnection);
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
		check_ice(marie, laure, LinphoneIceStateHostConnection);
	}

	const char *subject = "Weekly team meeting";
	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_set_subject(conf_params, subject);
	linphone_conference_params_enable_video(conf_params, enable_video);
	linphone_conference_params_enable_one_participant_conference(conf_params, enable_one_participant_conference);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, conf, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");
	BC_ASSERT_PTR_EQUAL(conf, l_conference);

	// Check that video capabilities are enabled in the conference
	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(l_conference);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params) == enable_video);

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2 + ((enable_ice) ? 1 : 0),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2 + ((enable_ice) ? 1 : 0),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2 + ((enable_ice) ? 1 : 0),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 4 + ((enable_ice) ? 6 : 0),
	                             liblinphone_tester_sip_timeout));

	// Check that conference capabilities hasn't changed
	current_conf_params = linphone_conference_get_current_params(l_conference);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params) == enable_video);
	BC_ASSERT_STRING_EQUAL(subject, linphone_conference_get_subject(l_conference));

	// Check that video is still on
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_laure), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_pauline), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_michelle), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);

	wait_for_list(lcs, NULL, 0, 2000);
	bool_t event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_log_enabled", TRUE);
	if (linphone_conference_params_video_enabled(current_conf_params) && event_log_enabled) {
		check_video_conference_with_local_participant(participants, layout, TRUE);
	}

	if (enable_video) {
		stats pauline_stats = pauline->stat;
		stats michelle_stats = michelle->stat;
		stats laure_stats = laure->stat;
		stats marie_stats = marie->stat;

		LinphoneCallParams *new_params = linphone_core_create_call_params(marie->lc, michelle_called_by_marie);
		linphone_call_params_enable_video(new_params, !enable_video);
		linphone_call_update(michelle_called_by_marie, new_params);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallUpdating,
		                             michelle_stats.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning,
		                             michelle_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
		                             marie_stats.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             marie_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_media_capability_changed,
		                             marie_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
		if (event_package_enabled) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
			                             michelle_stats.number_of_participant_devices_media_capability_changed + 1,
			                             3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_media_capability_changed,
			                             laure_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_media_capability_changed,
			                             pauline_stats.number_of_participant_devices_media_capability_changed + 1,
			                             3000));
		}
	}

	// Check that video is still on
	BC_ASSERT_STRING_EQUAL(subject, linphone_conference_get_subject(l_conference));
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_laure), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_pauline), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_video);
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_michelle), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	int no_parts = (int)bctbx_list_size(participants);
	if (event_package_enabled) {
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneConference *conference =
			    linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(conference));

				const LinphoneConferenceParams *current_remote_conf_params =
				    linphone_conference_get_current_params(conference);
				BC_ASSERT_PTR_NOT_NULL(current_remote_conf_params);
				BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_remote_conf_params) == enable_video);
			}
		}
	} else {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), no_parts, int, "%d");
	}

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, NULL, participants_exit_conference);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	if (conf) {
		linphone_conference_unref(conf);
	}
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void one_participant_video_conference_created_by_merging_video_calls_without_conference_event_package(void) {
	conference_created_by_merging_video_calls_base(FALSE, TRUE, LinphoneConferenceLayoutGrid, FALSE, TRUE, TRUE);
}

static void
video_conference_created_by_merging_video_calls_without_conference_event_package_terminated_by_server(void) {
	conference_created_by_merging_video_calls_base(FALSE, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE);
}

static void
video_conference_created_by_merging_video_calls_without_conference_event_package_terminated_by_participants(void) {
	conference_created_by_merging_video_calls_base(FALSE, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE);
}

static void video_conference_created_by_merging_video_calls_with_active_speaker_layout(void) {
	conference_created_by_merging_video_calls_base(TRUE, TRUE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                                               FALSE);
}

static void video_conference_created_by_merging_video_calls_with_grid_layout(void) {
	conference_created_by_merging_video_calls_base(TRUE, TRUE, LinphoneConferenceLayoutGrid, FALSE, TRUE, TRUE);
}

static void ice_video_conference_one_participant(const LinphoneConferenceLayout layout, bool_t local_participant) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	linphone_config_set_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_log_enabled", TRUE);

	LinphoneConference *conf = NULL;
	LinphoneConferenceParams *conf_params = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	bctbx_list_t *participants = NULL;
	const LinphoneCallParams *params = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == TRUE);

		linphone_core_set_firewall_policy(c, LinphonePolicyUseIce);

		linphone_config_set_int(linphone_core_get_config(c), "rtp", "rtcp_mux", 1);

		linphone_core_set_default_conference_layout(c, layout);
	}

	linphone_video_activation_policy_unref(pol);

	const LinphoneCallParams *negotiated_call_params = NULL;
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *michelle_call_params = linphone_core_create_call_params(michelle->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_set_conference_video_layout(marie_call_params, layout);
	linphone_call_params_enable_video(michelle_call_params, TRUE);

	if (!BC_ASSERT_TRUE(call_with_params(marie, michelle, marie_call_params, michelle_call_params))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	check_ice(marie, michelle, LinphoneIceStateHostConnection);

	params = linphone_call_get_remote_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeVideo, "rtcp-mux") !=
	               NULL);

	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(michelle_call_params);

	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	linphone_conference_params_enable_local_participant(conf_params, local_participant);
	linphone_conference_params_enable_one_participant_conference(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	add_calls_to_local_conference(lcs, marie, conf, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	if (local_participant) {
		LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(l_conference);
		BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 1, int, "%d");
		BC_ASSERT_PTR_EQUAL(conf, l_conference);
	} else {
		BC_ASSERT_FALSE(linphone_conference_is_in(conf));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conf), 1, int, "%d");
	}

	// Check that video capabilities are enabled in the conference
	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params));

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 3, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 3, liblinphone_tester_sip_timeout));

	// Check that conference capabilities hasn't changed
	current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params));

	// Check that video is still on
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_michelle), conf);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	if (local_participant) {
		LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(marie_conference);
		const LinphoneAddress *marie_conference_address = NULL;
		if (marie_conference) {
			marie_conference_address = linphone_conference_get_conference_address(marie_conference);
		}
		BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

		int no_parts = (int)bctbx_list_size(participants);
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneConference *conference =
			    linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(conference));

				const LinphoneConferenceParams *current_remote_conf_params =
				    linphone_conference_get_current_params(conference);
				BC_ASSERT_PTR_NOT_NULL(current_remote_conf_params);
				BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_remote_conf_params));
			}
		}
	}

	wait_for_list(lcs, NULL, 0, 2000);
	check_video_conference_with_local_participant(participants, layout, local_participant);
	terminate_conference(participants, marie, conf, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	if (conf) {
		linphone_conference_unref(conf);
	}
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void ice_video_conference_one_participant_grid_layout(void) {
	ice_video_conference_one_participant(LinphoneConferenceLayoutGrid, TRUE);
}

static void ice_video_conference_one_participant_active_speaker_layout(void) {
	ice_video_conference_one_participant(LinphoneConferenceLayoutActiveSpeaker, TRUE);
}

static void ice_video_conference_created_by_merging_video_calls_with_grid_layout(void) {
	conference_created_by_merging_video_calls_base(TRUE, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE);
}

static void audio_conference_created_by_merging_video_calls(void) {
	conference_created_by_merging_video_calls_base(TRUE, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE);
}

static void audio_calls_added_to_video_conference_base(const LinphoneConferenceLayout layout,
                                                       bool_t enable_ice,
                                                       bool_t enable_participant_video,
                                                       bool_t participant_calls_conference) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneConference *conf = NULL;
	LinphoneConferenceParams *conf_params = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;
	LinphoneCall *marie_call_michelle = NULL;
	LinphoneCall *michelle_called_by_marie = NULL;
	const LinphoneCallParams *params = NULL;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	LinphoneVideoActivationPolicy *participant_pol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(participant_pol, enable_participant_video);
	linphone_video_activation_policy_set_automatically_initiate(participant_pol, enable_participant_video);

	LinphoneVideoActivationPolicy *conference_pol =
	    linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(conference_pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(conference_pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_device(c, liblinphone_tester_mire_id);

		if (c == marie->lc) {
			linphone_core_set_video_activation_policy(c, conference_pol);
			linphone_core_enable_video_capture(c, TRUE);
			linphone_core_enable_video_display(c, TRUE);

			const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
			BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));
			BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_initiate(cpol));

		} else {
			linphone_core_set_video_activation_policy(c, participant_pol);
			linphone_core_enable_video_capture(c, enable_participant_video);
			linphone_core_enable_video_display(c, enable_participant_video);

			const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
			BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == enable_participant_video);
			BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_initiate(cpol) ==
			               enable_participant_video);
		}

		linphone_config_set_int(linphone_core_get_config(c), "rtp", "rtcp_mux", 1);

		linphone_core_set_default_conference_layout(c, layout);

		if (enable_ice) {
			linphone_core_set_firewall_policy(c, LinphonePolicyUseIce);
		}
	}

	linphone_video_activation_policy_unref(participant_pol);
	linphone_video_activation_policy_unref(conference_pol);

	const LinphoneCallParams *negotiated_call_params = NULL;
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCallParams *michelle_call_params = linphone_core_create_call_params(michelle->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, FALSE);
	linphone_call_params_set_conference_video_layout(marie_call_params, layout);
	linphone_call_params_enable_video(pauline_call_params, enable_participant_video);
	linphone_call_params_enable_video(laure_call_params, enable_participant_video);
	linphone_call_params_enable_video(michelle_call_params, enable_participant_video);

	if (participant_calls_conference) {
		BC_ASSERT_TRUE(call_with_params(laure, marie, laure_call_params, marie_call_params));
	} else {
		BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	}
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));

	params = linphone_call_get_remote_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);

	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	if (participant_calls_conference) {
		BC_ASSERT_TRUE(call_with_params(pauline, marie, pauline_call_params, marie_call_params));
	} else {
		BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	}
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));

	params = linphone_call_get_remote_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);

	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	bool_t success = FALSE;
	if (participant_calls_conference) {
		success = call_with_params(michelle, marie, michelle_call_params, marie_call_params);
	} else {
		success = call_with_params(marie, michelle, marie_call_params, michelle_call_params);
	}

	BC_ASSERT_TRUE(success);
	if (!success) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(negotiated_call_params));

	params = linphone_call_get_remote_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);
	params = linphone_call_get_remote_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_get_custom_sdp_media_attribute(params, LinphoneStreamTypeAudio, "rtcp-mux") !=
	               NULL);

	linphone_call_params_unref(laure_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(michelle_call_params);

	if (enable_ice) {
		check_ice(marie, michelle, LinphoneIceStateHostConnection);
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
		check_ice(marie, laure, LinphoneIceStateHostConnection);
	}

	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, conf, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");
	BC_ASSERT_PTR_EQUAL(conf, l_conference);

	// Check that video capabilities are enabled in the conference
	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(l_conference);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params));

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2 + ((enable_ice) ? 2 : 0),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2 + ((enable_ice) ? 2 : 0),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2 + ((enable_ice) ? 2 : 0),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6 + ((enable_ice) ? 6 : 0),
	                             liblinphone_tester_sip_timeout));

	// Check that conference capabilities hasn't changed
	current_conf_params = linphone_conference_get_current_params(l_conference);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params));

	// Check that video is still on
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_laure), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_participant_video);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_participant_video);
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_pauline), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_participant_video);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_participant_video);
	BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(marie_call_michelle), l_conference);
	negotiated_call_params = linphone_call_get_current_params(marie_call_michelle);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_participant_video);
	negotiated_call_params = linphone_call_get_current_params(michelle_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params) == enable_participant_video);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));

			const LinphoneConferenceParams *current_remote_conf_params =
			    linphone_conference_get_current_params(conference);
			BC_ASSERT_PTR_NOT_NULL(current_remote_conf_params);
			BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_remote_conf_params));
		}
	}

	wait_for_list(lcs, NULL, 0, 2000);
	terminate_conference(participants, marie, NULL, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	if (conf) {
		linphone_conference_unref(conf);
	}
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void audio_calls_added_to_video_conference(void) {
	audio_calls_added_to_video_conference_base(LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE);
}

static void audio_calls_initiated_by_host_added_to_video_conference(void) {
	audio_calls_added_to_video_conference_base(LinphoneConferenceLayoutGrid, FALSE, TRUE, FALSE);
}

static void audio_calls_with_video_rejected_added_to_video_conference(void) {
	audio_calls_added_to_video_conference_base(LinphoneConferenceLayoutGrid, FALSE, TRUE, TRUE);
}

static void simple_participant_leaves_conference_base(bool_t remote_participant_leaves) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 4, liblinphone_tester_sip_timeout));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	stats pauline_stats = pauline->stat;
	stats michelle_stats = michelle->stat;
	stats marie_stats = marie->stat;
	if (remote_participant_leaves) {
		// Pauline leaves the conference temporarely
		if (pauline_conference) {
			linphone_conference_leave(pauline_conference);
			participants = bctbx_list_remove(participants, pauline);

			// Call betwenn Pauline and Marie is paused as Pauline leaves the conference
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
			                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
			                             (marie_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
			                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));

			BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_on_hold,
			                             marie_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_on_hold,
			                             marie_stats.number_of_participant_devices_on_hold + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_on_hold,
			                             michelle_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_on_hold,
			                             michelle_stats.number_of_participant_devices_on_hold + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
			                             michelle_stats.number_of_participant_devices_media_capability_changed + 1,
			                             3000));

			if (marie_conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
			}
		}
	} else {
		// Marie (the local participant) leaves the conference temporarely
		if (marie_conference) {
			linphone_conference_leave(marie_conference);
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_removed,
			                             pauline_stats.number_of_participants_removed + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_removed,
			                             pauline_stats.number_of_participant_devices_removed + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participants_removed,
			                             michelle_stats.number_of_participants_removed + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_removed,
			                             michelle_stats.number_of_participant_devices_removed + 1, 3000));
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
			BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));
			if (pauline_conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), 1, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
			}
		}
	}

	// Michelle receives 2 NOTIFYs: one for the participant and one for the device removed
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference),
		                ((remote_participant_leaves) ? 2 : 1), int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 2000);

	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	marie_stats = marie->stat;
	if (remote_participant_leaves) {
		// Pauline rejoins conference
		if (pauline_conference) {
			linphone_conference_enter(pauline_conference);
			participants = bctbx_list_append(participants, pauline);

			// Call between Pauline and Marie is resumed as Pauline rejoins the conference
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallResuming,
			                             (pauline_stats.number_of_LinphoneCallResuming + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
			                             (marie_stats.number_of_LinphoneCallStreamsRunning + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
			                             (pauline_stats.number_of_LinphoneCallStreamsRunning + 1), 5000));

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_present,
			                             marie_stats.number_of_conference_participant_devices_present + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_present,
			                             marie_stats.number_of_participant_devices_present + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_present,
			                             michelle_stats.number_of_conference_participant_devices_present + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_present,
			                             michelle_stats.number_of_participant_devices_present + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
			                             michelle_stats.number_of_participant_devices_media_capability_changed + 1,
			                             3000));

			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
		}
	} else {
		// If the local participant left earlier on, then she should join the conference again
		// Marie (the local participant) left the conference temporarely
		if (marie_conference) {
			linphone_conference_enter(marie_conference);
		}
		// Participants are notified that Marie has entered the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_added,
		                             pauline_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_added,
		                             pauline_stats.number_of_participant_devices_added + 1, 3000));
		// Michelle receives 2 NOTIFYs: one for the participant and one for the device added
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participants_added,
		                             michelle_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_added,
		                             michelle_stats.number_of_participant_devices_added + 1, 3000));
	}

	int conf_parts_no = 2;
	if (marie_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	}

	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}
	if (pauline_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void simple_remote_participant_leaves_conference(void) {
	simple_participant_leaves_conference_base(TRUE);
}

static void simple_local_participant_leaves_conference(void) {
	simple_participant_leaves_conference_base(FALSE);
}

static void
participant_leaves_conference_base(bool_t remote_participant_leaves, bool_t add_participant, bool_t wait_for_updates) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 4, liblinphone_tester_sip_timeout));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	stats pauline_stats = pauline->stat;
	stats michelle_stats = michelle->stat;
	stats marie_stats = marie->stat;
	if (remote_participant_leaves) {
		// Pauline leaves the conference temporarely
		if (pauline_conference) {
			linphone_conference_leave(pauline_conference);

			if (wait_for_updates) {
				// Call between Pauline and Marie is paused as Pauline leaves the conference
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
				                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
				                             (marie_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
				                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));
				BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
				                              marie_stats.number_of_LinphoneSubscriptionTerminated + 1, 1000));
				BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
				                              pauline_stats.number_of_LinphoneSubscriptionTerminated + 1, 1000));
				BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_on_hold,
				                             marie_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_on_hold,
				                             marie_stats.number_of_participant_devices_on_hold + 1, 3000));
			}
		}

		if (marie_conference) {
			if (wait_for_updates) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
			}
			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		}
	} else {
		// Marie (the local participant) leaves the conference temporarely
		if (marie_conference) {
			linphone_conference_leave(marie_conference);
			if (wait_for_updates) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
				BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));
			}
		}
		if (wait_for_updates) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_removed,
			                             pauline_stats.number_of_participants_removed + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_removed,
			                             pauline_stats.number_of_participant_devices_removed + 1, 3000));
			if (pauline_conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), 1, int, "%d");
			}
		}
		if (pauline_conference) {
			BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
		}
	}

	if (wait_for_updates) {
		if (remote_participant_leaves) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
			                             michelle_stats.number_of_participant_devices_media_capability_changed + 1,
			                             3000));
		} else {
			// 2 NOTIFYs: one for the participant and one for the device
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participants_removed,
			                             michelle_stats.number_of_participants_removed + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_removed,
			                             michelle_stats.number_of_participant_devices_removed + 1, 3000));
		}
		if (michelle_conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference),
			                ((remote_participant_leaves) ? 2 : 1), int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
		}
	}

	// Marie calls Laure
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, laure));

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	wait_for_list(lcs, NULL, 0, 2000);

	if (!wait_for_updates) {
		if (remote_participant_leaves) {
			if (pauline_conference) {
				// Call between Pauline and Marie is paused as Pauline leaves the conference
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
				                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
				                             (marie_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
				                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));

				BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
				                              marie_stats.number_of_LinphoneSubscriptionTerminated + 1, 1000));
				BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
				                              pauline_stats.number_of_LinphoneSubscriptionTerminated + 1, 1000));

				BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));
			}

			if (marie_conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
			}

		} else {
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived,
			                             (pauline_stats.number_of_NotifyReceived + 1), 5000));
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
			BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));
			if (pauline_conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), 1, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
			}
		}
	}

	// Marie left the conference because she called Laure
	BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));

	// Michelle is notifed that both Pauline and Marie left the conference
	int expectedNotify = ((remote_participant_leaves) ? 2 : 1);
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + expectedNotify), 5000));
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), 1, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	marie_stats = marie->stat;
	stats laure_stats = laure->stat;
	int conf_parts_no = 2;

	// If the local participant left earlier on, then she should join the conference again
	if (marie_conference) {
		if (!linphone_conference_is_in(marie_conference)) {
			// Marie (the local participant) enters the conference
			linphone_conference_enter(marie_conference);
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), conf_parts_no, int, "%d");

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
			                             marie_stats.number_of_LinphoneCallPausing + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
			                             marie_stats.number_of_LinphoneCallPaused + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote,
			                             laure_stats.number_of_LinphoneCallPausedByRemote + 1, 5000));

			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		}
	}

	wait_for_list(lcs, NULL, 0, 2000);

	if (!remote_participant_leaves) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived,
		                             (pauline_stats.number_of_NotifyReceived + 1), 5000));
	}
	if (pauline_conference) {
		if (remote_participant_leaves) {
			BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));
		} else {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), 2, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
		}
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 5000));
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), 2, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	if (add_participant) {
		bctbx_list_t *additional_participants = NULL;
		additional_participants = bctbx_list_append(additional_participants, laure);
		bctbx_list_t *lcs2 = NULL;
		lcs2 = bctbx_list_append(lcs2, laure->lc);
		lcs2 = bctbx_list_append(lcs2, marie->lc);
		lcs2 = bctbx_list_append(lcs2, michelle->lc);
		lcs2 = bctbx_list_append(lcs2, pauline->lc);
		add_calls_to_local_conference(lcs2, marie, NULL, additional_participants, FALSE);
		bctbx_list_free(additional_participants);
		bctbx_list_free(lcs2);
		participants = bctbx_list_append(participants, laure);

		// Wait that Laure joins the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
		                             laure_stats.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             marie_stats.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
	}

	conf_parts_no = 2 + add_participant;
	// Pauline is still out of the conference
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), conf_parts_no, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));

	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	marie_stats = marie->stat;
	laure_stats = laure->stat;

	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, NULL, marie_conference_address, NULL);
	if (add_participant) {
		BC_ASSERT_PTR_NOT_NULL(laure_conference);
	} else {
		BC_ASSERT_PTR_NULL(laure_conference);
	}
	if (laure_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference), ((add_participant) ? 3 : 2), int,
		                "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(laure_conference));
	}

	if (remote_participant_leaves) {
		// Pauline rejoins conference
		if (pauline_conference) {
			linphone_conference_enter(pauline_conference);
		}

		// Call between Pauline and Marie is resumed as Pauline rejoins the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallResuming,
		                             (pauline_stats.number_of_LinphoneCallResuming + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             (marie_stats.number_of_LinphoneCallStreamsRunning + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                             (pauline_stats.number_of_LinphoneCallStreamsRunning + 1), 5000));

		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), conf_parts_no, int, "%d");

		// NOTIFY to resend list of conference participants
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_conference_participant_devices_present,
		                             pauline_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_present,
		                             pauline_stats.number_of_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_present,
		                             marie_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_present,
		                             marie_stats.number_of_participant_devices_present + 1, 3000));

		// Notify that Pauline media capabilities changed
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_present,
		                             michelle_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_present,
		                             michelle_stats.number_of_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
		                             michelle_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
		if (add_participant) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_conference_participant_devices_present,
			                             laure_stats.number_of_conference_participant_devices_present + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_present,
			                             laure_stats.number_of_participant_devices_present + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_media_capability_changed,
			                             laure_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
		}
	}

	if (marie_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	}
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}
	if (laure_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(laure_conference));
	}
	if (pauline_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	if (!add_participant) {
		end_call(marie, laure);
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void remote_participant_leaves_conference_and_add_participant(void) {
	participant_leaves_conference_base(TRUE, TRUE, TRUE);
}

static void remote_participant_leaves_conference_and_call_to_focus(void) {
	participant_leaves_conference_base(TRUE, FALSE, TRUE);
}

static void local_participant_leaves_conference_and_add_participant(void) {
	participant_leaves_conference_base(FALSE, TRUE, TRUE);
}

static void local_participant_leaves_conference_and_call_to_focus(void) {
	participant_leaves_conference_base(FALSE, FALSE, TRUE);
}

static void quick_remote_participant_leaves_conference_and_add_participant(void) {
	participant_leaves_conference_base(TRUE, TRUE, FALSE);
}

static void quick_remote_participant_leaves_conference_and_call_to_focus(void) {
	participant_leaves_conference_base(TRUE, FALSE, FALSE);
}

static void quick_local_participant_leaves_conference_and_add_participant(void) {
	participant_leaves_conference_base(FALSE, TRUE, FALSE);
}

static void quick_local_participant_leaves_conference_and_call_to_focus(void) {
	participant_leaves_conference_base(FALSE, FALSE, FALSE);
}

static void all_temporarely_leave_conference_base(bool_t local_enters_first) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	int no_parts = (int)bctbx_list_size(participants);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);

	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);

	// wait a bit to ensure that should NOTIFYs be sent, they reach their destination
	wait_for_list(lcs, NULL, 0, 5000);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	stats pauline_stats = pauline->stat;
	stats michelle_stats = michelle->stat;
	stats laure_stats = laure->stat;
	stats marie_stats = marie->stat;

	// All participants (remote and local) temporarely leave the conference at the same time
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		// Participant leaves the conference temporarely
		if (conference) {
			linphone_conference_leave(conference);
			BC_ASSERT_FALSE(linphone_conference_is_in(conference));
		}
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
	                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
	                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausing,
	                             (laure_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused,
	                             (laure_stats.number_of_LinphoneCallPaused + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallPausing,
	                             (michelle_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallPaused,
	                             (michelle_stats.number_of_LinphoneCallPaused + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             (marie_stats.number_of_LinphoneCallPausedByRemote + 3), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_on_hold,
	                             marie_stats.number_of_conference_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_on_hold,
	                             marie_stats.number_of_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_on_hold,
	                             michelle_stats.number_of_conference_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_on_hold,
	                             michelle_stats.number_of_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_conference_participant_devices_on_hold,
	                             pauline_stats.number_of_conference_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_on_hold,
	                             pauline_stats.number_of_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_conference_participant_devices_on_hold,
	                             laure_stats.number_of_conference_participant_devices_on_hold + 4, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_on_hold,
	                             laure_stats.number_of_participant_devices_on_hold + 4, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_removed,
	                             pauline_stats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_removed,
	                             pauline_stats.number_of_participant_devices_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participants_removed,
	                             laure_stats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_removed,
	                             laure_stats.number_of_participant_devices_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participants_removed,
	                             michelle_stats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_removed,
	                             michelle_stats.number_of_participant_devices_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participants_removed,
	                             marie_stats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_removed,
	                             marie_stats.number_of_participant_devices_removed + 1, 3000));

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");
	BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_FALSE(linphone_conference_is_in(conference));
			bctbx_list_t *conference_participants = linphone_conference_get_participant_list(conference);
			for (bctbx_list_t *itp = conference_participants; itp; itp = bctbx_list_next(itp)) {
				LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
				bctbx_list_t *devices = linphone_participant_get_devices(p);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					BC_ASSERT_FALSE(linphone_participant_device_is_in_conference(d));
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
			bctbx_list_free_with_data(conference_participants, (void (*)(void *))linphone_participant_unref);
		}
	}

	if (marie_conference && local_enters_first) {
		// Marie (the local participant) rejoins the conference
		if (!linphone_conference_is_in(marie_conference)) {
			linphone_conference_enter(marie_conference);
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		}
	}

	marie_stats = marie->stat;
	stats *participants_initial_stats = NULL;
	int counter = 1;
	// All remote participants reenter the conference at the same time
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
		// Append element
		participants_initial_stats[counter - 1] = m->stat;
		// Increment counter
		counter++;
	}

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		// Participant rejoins the conference
		if (conference) {
			linphone_conference_enter(conference);
			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference) == local_enters_first);
		}
	}

	counter = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallResuming,
		                             (participants_initial_stats[counter].number_of_LinphoneCallResuming + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
		                             (participants_initial_stats[counter].number_of_LinphoneCallStreamsRunning + 1),
		                             5000));

		int participants_joined = (local_enters_first) ? 4 : 3;
		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &m->stat.number_of_participant_devices_media_capability_changed,
		    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &m->stat.number_of_conference_participant_devices_present,
		    participants_initial_stats[counter].number_of_conference_participant_devices_present + participants_joined,
		    3000));
		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &m->stat.number_of_participant_devices_present,
		    participants_initial_stats[counter].number_of_participant_devices_present + participants_joined, 3000));

		if (local_enters_first) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_added,
			                             participants_initial_stats[counter].number_of_participants_added + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participant_devices_added,
			                             participants_initial_stats[counter].number_of_participant_devices_added + 1,
			                             3000));
		}

		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		// Participant leaves the conference temporarely
		if (conference) {
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference) == local_enters_first);
		}
		// Increment counter
		counter++;
	}

	if (participants_initial_stats) {
		ms_free(participants_initial_stats);
		participants_initial_stats = NULL;
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             (marie_stats.number_of_LinphoneCallStreamsRunning + no_parts), 5000));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference) == local_enters_first);

	if (!local_enters_first) {
		pauline_stats = pauline->stat;
		michelle_stats = michelle->stat;
		laure_stats = laure->stat;
		marie_stats = marie->stat;

		if (marie_conference) {
			// Marie (the local participant) rejoins the conference
			if (!linphone_conference_is_in(marie_conference)) {
				linphone_conference_enter(marie_conference);
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
			}
		}

		// NOTIFY that marie rejoined the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_added,
		                             pauline_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_added,
		                             pauline_stats.number_of_participant_devices_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participants_added,
		                             laure_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_added,
		                             laure_stats.number_of_participant_devices_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participants_added,
		                             michelle_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_added,
		                             michelle_stats.number_of_participant_devices_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participants_added,
		                             marie_stats.number_of_participants_added + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_added,
		                             marie_stats.number_of_participant_devices_added + 1, 3000));

		// wait a bit to ensure that NOTIFYs reach their destination even if they are challenged
		wait_for_list(lcs, NULL, 0, 1000);
	}

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference),
			                linphone_conference_get_participant_count(marie_conference), int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	terminate_conference(participants, marie, NULL, NULL, FALSE);

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void everybody_temporarely_leave_conference_and_local_enters_first(void) {
	all_temporarely_leave_conference_base(TRUE);
}

static void everybody_temporarely_leave_conference_and_local_enters_last(void) {
	all_temporarely_leave_conference_base(FALSE);
}

static void focus_takes_call_after_conference_started_and_participants_leave(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_called_by_laure;
	LinphoneCall *laure_call_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(conference)) goto end;
	const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(conf_params));

	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(laure, marie));

	marie_called_by_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_called_by_laure)) goto end;
	laure_call_marie = linphone_core_get_current_call(laure->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_call_marie)) goto end;

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	// Local participant is expected to have left as it joined another call
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));

	bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
	bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

	conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(conf_params));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	new_participants = terminate_participant_call(new_participants, marie, michelle);
	bctbx_list_free(new_participants);

	unsigned int marie_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	// Marie has 2 active calls:
	// - one to Laure that is ongoing
	// - one to Pauline that is paused
	BC_ASSERT_EQUAL(marie_call_no, 2, unsigned int, "%u");
	// Call to Marie is still active
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));

	stats marie_stat = marie->stat;
	stats laure_stat = laure->stat;
	stats pauline_stat = pauline->stat;
	unsigned int laure_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc));
	BC_ASSERT_EQUAL(laure_call_no, 1, unsigned int, "%u");
	unsigned int pauline_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc));
	BC_ASSERT_EQUAL(pauline_call_no, 1, unsigned int, "%u");
	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
	                             marie_stat.number_of_LinphoneCallEnd + marie_call_no, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, laure_stat.number_of_LinphoneCallEnd + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
	                             pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
	                             marie_stat.number_of_LinphoneCallReleased + marie_call_no,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
	                             laure_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
	                             pauline_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void remote_participant_leaves_and_conference_ends_base(bool_t local_ends_conference) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	bctbx_list_t *participants = NULL;
	LinphoneAddress *marie_conference_address = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 4, liblinphone_tester_sip_timeout));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	if (marie_conference) {
		marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(marie_conference));
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	stats pauline_stats = pauline->stat;
	stats michelle_stats = michelle->stat;
	stats marie_stats = marie->stat;
	// Pauline leaves the conference temporarely
	if (pauline_conference) {
		linphone_conference_leave(pauline_conference);

		// Call betwenn Pauline and Marie is paused as Pauline leaves the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
		                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
		                             (marie_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
		                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));

		BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_on_hold,
		                             marie_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_on_hold,
		                             marie_stats.number_of_participant_devices_on_hold + 1, 3000));

		if (marie_conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		}
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_on_hold,
	                             michelle_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_on_hold,
	                             michelle_stats.number_of_participant_devices_on_hold + 1, 3000));
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), 2, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 2000);

	if (local_ends_conference) {
		terminate_conference(participants, marie, NULL, NULL, FALSE);
	} else {
		// Michelle terminates call with Marie which trigger the conference end procedure
		LinphoneCall *michelle_called_by_marie =
		    linphone_core_get_call_by_remote_address2(michelle->lc, marie->identity);
		BC_ASSERT_PTR_NOT_NULL(michelle_called_by_marie);
		if (michelle_called_by_marie) {
			marie_stats = marie->stat;
			michelle_stats = michelle->stat;
			pauline_stats = pauline->stat;
			linphone_core_terminate_call(michelle->lc, michelle_called_by_marie);
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
			                             marie_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd,
			                             michelle_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
			                             marie_stats.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased,
			                             michelle_stats.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));

			// Wait for conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             (michelle_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
			                             5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminated,
			                             (michelle_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateDeleted,
			                             (michelle_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             (pauline_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
			                             5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
			                             (pauline_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
			                             (pauline_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             (marie_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
			                             (marie_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
			                             (marie_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
			                             marie_stats.number_of_LinphoneSubscriptionTerminated + 2, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionTerminated,
			                             michelle_stats.number_of_LinphoneSubscriptionTerminated + 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
			                             pauline_stats.number_of_LinphoneSubscriptionTerminated + 1, 3000));

			pauline_conference =
			    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
			BC_ASSERT_PTR_NULL(pauline_conference);
		}

		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 1, unsigned int, "%u");
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 1, unsigned int, "%u");

		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity));
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity));

		end_call(marie, pauline);
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
}

static void remote_participant_leaves_and_other_remote_terminate_call(void) {
	remote_participant_leaves_and_conference_ends_base(FALSE);
}

static void remote_participant_leaves_and_local_ends_conference(void) {
	remote_participant_leaves_and_conference_ends_base(TRUE);
}

static void participant_call_terminated_after_leaving_conference_base(bool_t local_terminates) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_chloe;
	LinphoneCall *chloe_called_by_marie;
	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, chloe->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, chloe));
	marie_call_chloe = linphone_core_get_current_call(marie->lc);
	chloe_called_by_marie = linphone_core_get_current_call(chloe->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_chloe, chloe, chloe_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, chloe);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (marie_conference) {
		marie_conference_address = linphone_conference_get_conference_address(marie_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);

	LinphoneConference *chloe_conference =
	    linphone_core_search_conference(chloe->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(chloe_conference);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	stats pauline_stats = pauline->stat;
	stats michelle_stats = michelle->stat;
	stats chloe_stats = chloe->stat;
	stats marie_stats = marie->stat;
	// Pauline leaves the conference temporarely
	if (pauline_conference) {
		linphone_conference_leave(pauline_conference);

		// Call betwenn Pauline and Marie is paused as Pauline leaves the conference
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,
		                             (pauline_stats.number_of_LinphoneCallPausing + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
		                             (marie_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,
		                             (pauline_stats.number_of_LinphoneCallPaused + 1), 5000));

		BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));

		if (marie_conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		}
	}

	// Notify Michelle that Pauline's media capabilities have changed
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), 3, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	// Notify Chloe that Pauline's media capabilities have changed
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_participant_devices_media_capability_changed,
	                             chloe_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
	if (chloe_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(chloe_conference), 3, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(chloe_conference));
	}

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 2000);

	LinphoneCoreManager *mgr_requested = NULL;
	LinphoneCoreManager *mgr_requesting = NULL;
	if (local_terminates) {
		mgr_requested = pauline;
		mgr_requesting = marie;
	} else {
		mgr_requested = marie;
		mgr_requesting = pauline;
	}

	pauline_conference = linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneCall *call_to_terminate =
	    linphone_core_get_call_by_remote_address2(mgr_requesting->lc, mgr_requested->identity);
	BC_ASSERT_PTR_NOT_NULL(call_to_terminate);
	if (call_to_terminate) {
		stats previous_stats_requesting = mgr_requesting->stat;
		stats previous_stats_requested = mgr_requested->stat;
		linphone_call_terminate(call_to_terminate);
		BC_ASSERT_TRUE(wait_for(mgr_requesting->lc, mgr_requested->lc, &mgr_requesting->stat.number_of_LinphoneCallEnd,
		                        previous_stats_requesting.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(mgr_requesting->lc, mgr_requested->lc, &mgr_requested->stat.number_of_LinphoneCallEnd,
		                        previous_stats_requested.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(mgr_requesting->lc, mgr_requested->lc,
		                        &mgr_requesting->stat.number_of_LinphoneCallReleased,
		                        previous_stats_requesting.number_of_LinphoneCallReleased + 1));
		BC_ASSERT_TRUE(wait_for(mgr_requesting->lc, mgr_requested->lc,
		                        &mgr_requested->stat.number_of_LinphoneCallReleased,
		                        previous_stats_requested.number_of_LinphoneCallReleased + 1));
	}

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 1000);

	pauline_conference = linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NULL(pauline_conference);

	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	chloe_stats = chloe->stat;
	marie_stats = marie->stat;

	// Marie calls Pauline again
	// Marie leaves conference to call Pauline
	BC_ASSERT_TRUE(call(marie, pauline));

	marie_call_pauline = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_pauline)) goto end;

	BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &chloe->stat.number_of_NotifyReceived, (chloe_stats.number_of_NotifyReceived + 1), 5000));
	if (chloe_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(chloe_conference), 1, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(chloe_conference));
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 5000));
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), 1, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	chloe_stats = chloe->stat;
	marie_stats = marie->stat;
	int conf_parts_no = 2;
	// If the local participant left earlier on, then she should join the conference again
	if (marie_conference) {
		if (!linphone_conference_is_in(marie_conference)) {
			// Marie (the local participant) enters the conference
			linphone_conference_enter(marie_conference);
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), conf_parts_no, int, "%d");

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
			                             marie_stats.number_of_LinphoneCallPausing + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
			                             marie_stats.number_of_LinphoneCallPaused + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote,
			                             pauline_stats.number_of_LinphoneCallPausedByRemote + 1, 5000));

			BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		}
	}

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &chloe->stat.number_of_NotifyReceived, (chloe_stats.number_of_NotifyReceived + 1), 5000));
	if (chloe_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(chloe_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(chloe_conference));
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 5000));
	if (michelle_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(michelle_conference), conf_parts_no, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(michelle_conference));
	}

	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	chloe_stats = chloe->stat;
	marie_stats = marie->stat;
	bctbx_list_t *additional_participants = NULL;
	additional_participants = bctbx_list_append(additional_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, additional_participants, FALSE);
	bctbx_list_free(additional_participants);

	pauline_conference = linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	// Wait that Pauline joins the conference
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             pauline_stats.number_of_LinphoneCallStreamsRunning + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             marie_stats.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
	}

	terminate_conference(participants, marie, NULL, NULL, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(michelle->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(chloe->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(chloe->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void local_participant_call_terminated_after_leaving_conference(void) {
	participant_call_terminated_after_leaving_conference_base(TRUE);
}

static void remote_participant_call_terminated_after_leaving_conference(void) {
	participant_call_terminated_after_leaving_conference_base(FALSE);
}

static void local_participant_takes_call_after_conference_started_and_conference_ends(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);
	bctbx_list_free(new_participants);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	const LinphoneAddress *marie_conference_address = NULL;
	if (l_conference) {
		marie_conference_address = linphone_conference_get_conference_address(l_conference);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;
	stats pauline_initial_stats = pauline->stat;

	// Chloe calls Marie and she accepts the call
	// Laure and Pauline are notified that Marie temporarely leaves the conference
	BC_ASSERT_TRUE(call(chloe, marie));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participants_removed,
	                             pauline_initial_stats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_removed,
	                             pauline_initial_stats.number_of_participant_devices_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participants_removed,
	                             laure_initial_stats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_removed,
	                             laure_initial_stats.number_of_participant_devices_removed + 1, 3000));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 3, unsigned int, "%u");
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 2, int, "%d");
	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));

	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);
	if (laure_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(laure_conference), 1, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(laure_conference));
	}

	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, NULL, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	if (pauline_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pauline_conference), 1, int, "%d");
		BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
	}

	wait_for_list(lcs, NULL, 0, 2000);

	marie_initial_stats = marie->stat;
	laure_initial_stats = laure->stat;
	pauline_initial_stats = pauline->stat;

	end_call(laure, marie);

	// Wait for conferences to be terminated
	// As there is an active call between Marie and Chloe, then pause call between Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (pauline_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
	                             5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
	                             (pauline_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
	                             (pauline_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote,
	                             (pauline_initial_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
	                             (marie_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
	                             (marie_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             marie_initial_stats.number_of_LinphoneSubscriptionTerminated + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
	                             laure_initial_stats.number_of_LinphoneSubscriptionTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
	                             pauline_initial_stats.number_of_LinphoneSubscriptionTerminated + 1, 3000));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 2, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(chloe->lc)), 1, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 1, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity));

	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(chloe->lc, marie->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, chloe->identity)) ==
	               LinphoneCallStreamsRunning);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity)) ==
	               LinphoneCallPausedByRemote);
	BC_ASSERT_TRUE(linphone_call_get_state(linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity)) ==
	               LinphoneCallPaused);

	wait_for_list(lcs, NULL, 0, 2000);

	end_call(chloe, marie);
	end_call(marie, pauline);

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(pauline->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(chloe->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");
end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);
}

static void remote_participant_adds_video_during_conference(void) {
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);

	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(focus_mgr->lc);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_core_set_video_activation_policy(michelle->lc, pol);
	linphone_video_activation_policy_unref(pol);

	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(laure->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(michelle->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(laure->lc, TRUE);
	linphone_core_enable_video_display(laure->lc, TRUE);
	linphone_core_enable_video_capture(michelle->lc, TRUE);
	linphone_core_enable_video_display(michelle->lc, TRUE);
	linphone_core_enable_video_capture(focus_mgr->lc, TRUE);
	linphone_core_enable_video_display(focus_mgr->lc, TRUE);

	linphone_core_set_default_conference_layout(pauline->lc, LinphoneConferenceLayoutGrid);
	linphone_core_set_default_conference_layout(marie->lc, LinphoneConferenceLayoutGrid);
	linphone_core_set_default_conference_layout(laure->lc, LinphoneConferenceLayoutGrid);
	linphone_core_set_default_conference_layout(michelle->lc, LinphoneConferenceLayoutGrid);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, focus_mgr->lc);

	BC_ASSERT_TRUE(call(marie, laure));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	BC_ASSERT_TRUE(call(marie, michelle));

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		if (conf_call) {
			const LinphoneCallParams *conf_call_params = linphone_call_get_current_params(conf_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(conf_call_params));
		}

		LinphoneCall *p_call = linphone_core_get_call_by_remote_address2(m->lc, marie->identity);
		BC_ASSERT_PTR_NOT_NULL(p_call);
		if (p_call) {
			const LinphoneCallParams *p_call_params = linphone_call_get_current_params(p_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(p_call_params));
		}
	}

	pol = linphone_video_activation_policy_clone(linphone_core_get_video_activation_policy(marie->lc));
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_video_activation_policy_unref(pol);

	// marie creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conf);

	add_calls_to_remote_conference(lcs, focus_mgr, marie, participants, conf, FALSE);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params));

	bctbx_list_t *all_manangers_in_conf = bctbx_list_copy(participants);
	all_manangers_in_conf = bctbx_list_append(all_manangers_in_conf, marie);
	const LinphoneAddress *marie_conference_address = linphone_conference_get_conference_address(conf);
	for (bctbx_list_t *it = all_manangers_in_conf; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *conference =
		    linphone_core_search_conference(m->lc, NULL, uri, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		linphone_address_unref(uri);

		if (conference) {
			const LinphoneConferenceParams *params = linphone_conference_get_current_params(conf);
			BC_ASSERT_PTR_NOT_NULL(params);
			BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params));
		}

		LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(focus_mgr->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		if (conf_call) {
			const LinphoneCallParams *conf_call_params = linphone_call_get_current_params(conf_call);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(conf_call_params) == (m == marie));
		}

		LinphoneCall *p_call = linphone_core_get_call_by_remote_address2(m->lc, focus_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(p_call);
		if (p_call) {
			const LinphoneCallParams *p_call_params = linphone_call_get_current_params(p_call);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(p_call_params) == (m == marie));
		}
	}

	bctbx_list_free(all_manangers_in_conf);

	wait_for_list(lcs, NULL, 0, 2000);

	pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_core_set_video_activation_policy(michelle->lc, pol);
	linphone_video_activation_policy_unref(pol);

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;
	stats pauline_initial_stats = pauline->stat;
	stats michelle_initial_stats = michelle->stat;

	LinphoneCall *pauline_conf_call = linphone_core_get_call_by_remote_address2(pauline->lc, focus_mgr->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_conf_call);
	if (pauline_conf_call) {
		LinphoneCallParams *pauline_new_params = linphone_core_create_call_params(pauline->lc, pauline_conf_call);
		linphone_call_params_enable_video(pauline_new_params, TRUE);
		linphone_call_update(pauline_conf_call, pauline_new_params);
		linphone_call_params_unref(pauline_new_params);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating,
	                             pauline_initial_stats.number_of_LinphoneCallUpdating + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             pauline_initial_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
	// Laure and Michelle do not send reINVITEs because their video is off
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_participant_devices_media_capability_changed,
	                             laure_initial_stats.number_of_participant_devices_media_capability_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_initial_stats.number_of_participant_devices_media_capability_changed + 1,
	                             3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating,
	                             marie_initial_stats.number_of_LinphoneCallUpdating + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             marie_initial_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_media_capability_changed,
	                             marie_initial_stats.number_of_participant_devices_media_capability_changed + 1, 3000));

	marie_initial_stats = marie->stat;
	laure_initial_stats = laure->stat;
	pauline_initial_stats = pauline->stat;
	michelle_initial_stats = michelle->stat;

	LinphoneCall *laure_conf_call = linphone_core_get_call_by_remote_address2(laure->lc, focus_mgr->identity);
	BC_ASSERT_PTR_NOT_NULL(laure_conf_call);
	if (laure_conf_call) {
		LinphoneCallParams *laure_new_params = linphone_core_create_call_params(laure->lc, laure_conf_call);
		linphone_call_params_enable_video(laure_new_params, TRUE);
		linphone_call_update(laure_conf_call, laure_new_params);
		linphone_call_params_unref(laure_new_params);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallUpdating,
	                             laure_initial_stats.number_of_LinphoneCallUpdating + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning,
	                             laure_initial_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating,
	                             pauline_initial_stats.number_of_LinphoneCallUpdating + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
	                             pauline_initial_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_media_capability_changed,
	                             pauline_initial_stats.number_of_participant_devices_media_capability_changed + 1,
	                             3000));
	// Michelle does not send reINVITEs because its video is off
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_initial_stats.number_of_participant_devices_media_capability_changed + 1,
	                             3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating,
	                             marie_initial_stats.number_of_LinphoneCallUpdating + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
	                             marie_initial_stats.number_of_LinphoneCallStreamsRunning + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_media_capability_changed,
	                             marie_initial_stats.number_of_participant_devices_media_capability_changed + 1, 3000));

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(participants, marie, NULL, focus_mgr, FALSE);

	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(michelle);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void remote_participant_takes_call_after_conference_started_and_conference_ends(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *chloe_call_laure;
	LinphoneCall *laure_called_by_chloe;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;

	const LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe, laure));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausing,
	                             (laure_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             (marie_initial_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused,
	                             (laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int,
	                "%d");

	chloe_call_laure = linphone_core_get_current_call(chloe->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;

	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), 3, int, "%d");

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	laure_initial_stats = laure->stat;
	stats chloe_initial_stats = chloe->stat;
	linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
	BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallEnd,
	                        laure_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallEnd,
	                        chloe_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallReleased,
	                        laure_initial_stats.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallReleased,
	                        chloe_initial_stats.number_of_LinphoneCallReleased + 1));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);
}

static void participant_quits_conference_and_is_called_by_focus(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	bctbx_list_t *new_participants = NULL;
	LinphoneAddress *marie_conference_address = NULL;

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(marie_conference));

	LinphoneAddress *pauline_uri = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneConference *pauline_conference =
	    linphone_core_search_conference(pauline->lc, NULL, pauline_uri, marie_conference_address, NULL);
	linphone_address_unref(pauline_uri);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);

	LinphoneAddress *michelle_uri = linphone_address_new(linphone_core_get_identity(michelle->lc));
	LinphoneConference *michelle_conference =
	    linphone_core_search_conference(michelle->lc, NULL, michelle_uri, marie_conference_address, NULL);
	linphone_address_unref(michelle_uri);
	BC_ASSERT_PTR_NOT_NULL(michelle_conference);

	LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	linphone_address_unref(laure_uri);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);

	int no_parts = 3;

	LinphoneConference *l_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), no_parts, int, "%d");

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		}
		if (c != marie->lc) {
			LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(marie->lc, get_manager(c)->identity);
			BC_ASSERT_PTR_NOT_NULL(conf_call);
			if (conf_call) {
				BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(conf_call));
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(conf_call), marie_conference);
			}
			LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(c, marie->identity);
			BC_ASSERT_PTR_NOT_NULL(participant_call);
			if (participant_call) {
				BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
			}
		}
	}

	new_participants = terminate_participant_call(new_participants, marie, laure);

	wait_for_list(lcs, NULL, 0, 1000);

	no_parts = 2;

	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), no_parts, int, "%d");

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		if (c != laure->lc) {
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(conference));
			}
			if (c != marie->lc) {
				LinphoneCall *conf_call =
				    linphone_core_get_call_by_remote_address2(marie->lc, get_manager(c)->identity);
				BC_ASSERT_PTR_NOT_NULL(conf_call);
				if (conf_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(conf_call));
					BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(conf_call), marie_conference);
				}
				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(c, marie->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
				}
			}
		} else {
			BC_ASSERT_PTR_NULL(conference);
		}
	}

	stats laure_stats = laure->stat;
	stats pauline_stats = pauline->stat;
	stats michelle_stats = michelle->stat;
	stats marie_stats = marie->stat;

	// Marie calls Laure therefore she temporarely leaves conference
	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	wait_for_list(lcs, NULL, 0, 1000);

	// Wait for notification of marie's exit fo conference
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived,
	                             (pauline_stats.number_of_NotifyReceived + 1), 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 3000));

	LinphoneCall *marie_laure_call = linphone_core_get_call_by_remote_address2(marie->lc, laure->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_laure_call);
	if (marie_laure_call) {
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(marie_laure_call));
	}
	LinphoneCall *laure_marie_call = linphone_core_get_call_by_remote_address2(laure->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(laure_marie_call);
	if (laure_marie_call) {
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_marie_call));
	}

	BC_ASSERT_FALSE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), no_parts, int, "%d");

	BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                              (marie_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 2000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
	                              (pauline_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 2000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateTerminationPending,
	                              (michelle_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 2000));

	no_parts = 2;

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		if (c != laure->lc) {
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				int current_no_parts =
				    (c == marie->lc) ? no_parts : (no_parts - (linphone_conference_is_in(l_conference) ? 0 : 1));
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), current_no_parts, int, "%d");
				if (c == marie->lc) {
					BC_ASSERT_FALSE(linphone_conference_is_in(conference));
				} else {
					BC_ASSERT_TRUE(linphone_conference_is_in(conference));
				}
			}
			if (c != marie->lc) {
				LinphoneCall *conf_call =
				    linphone_core_get_call_by_remote_address2(marie->lc, get_manager(c)->identity);
				BC_ASSERT_PTR_NOT_NULL(conf_call);
				if (conf_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(conf_call));
				}
				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(c, marie->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
				}
			}
		} else {
			BC_ASSERT_PTR_NULL(conference);
		}
	}

	laure_stats = laure->stat;
	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	marie_stats = marie->stat;

	no_parts = 2;

	// Marie (the local participant) enters the conference
	linphone_conference_enter(marie_conference);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), no_parts, int, "%d");

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
	                             marie_stats.number_of_LinphoneCallPausing + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
	                             marie_stats.number_of_LinphoneCallPaused + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote,
	                             laure_stats.number_of_LinphoneCallPausedByRemote + 1, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived,
	                             (pauline_stats.number_of_NotifyReceived + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_stats.number_of_NotifyReceived + 1), 5000));

	wait_for_list(lcs, NULL, 0, 2000);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		if (c != laure->lc) {
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");
				BC_ASSERT_TRUE(linphone_conference_is_in(conference));
			}
			if (c != marie->lc) {
				LinphoneCall *conf_call =
				    linphone_core_get_call_by_remote_address2(marie->lc, get_manager(c)->identity);
				BC_ASSERT_PTR_NOT_NULL(conf_call);
				if (conf_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(conf_call));
					BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(conf_call), marie_conference);
				}
				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(c, marie->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
				}
			}
		} else {
			BC_ASSERT_PTR_NULL(conference);
			if (c != marie->lc) {
				LinphoneCall *conf_call =
				    linphone_core_get_call_by_remote_address2(marie->lc, get_manager(c)->identity);
				BC_ASSERT_PTR_NOT_NULL(conf_call);
				if (conf_call) {
					BC_ASSERT_PTR_NULL(linphone_call_get_conference(conf_call));
				}
				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(c, marie->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					BC_ASSERT_PTR_NULL(linphone_call_get_conference(participant_call));
				}
			}
		}
	}

	// Pauline exits from conference hence it terminates because only Michelle and Marie are left
	new_participants = terminate_participant_call(new_participants, marie, pauline);

	no_parts = 0;

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NULL(conference);
	}

	LinphoneCall *marie_michelle_call = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_michelle_call);
	if (marie_michelle_call) {
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(marie_michelle_call));
	}
	LinphoneCall *michelle_marie_call = linphone_core_get_call_by_remote_address2(michelle->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(michelle_marie_call);
	if (michelle_marie_call) {
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(michelle_marie_call));
	}

	michelle_stats = michelle->stat;
	marie_stats = marie->stat;

	// Marie calls Pauline again
	if (!BC_ASSERT_TRUE(call(marie, pauline))) goto end;

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
	                             marie_stats.number_of_LinphoneCallPausing + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
	                             marie_stats.number_of_LinphoneCallPaused + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallPausedByRemote,
	                             michelle_stats.number_of_LinphoneCallPausedByRemote + 1, 5000));

	LinphoneCall *marie_pauline_call = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_pauline_call);
	if (marie_pauline_call) {
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(marie_pauline_call));
	}
	LinphoneCall *pauline_marie_call = linphone_core_get_call_by_remote_address2(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_marie_call);
	if (pauline_marie_call) {
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(pauline_marie_call));
	}

	int marie_calls_no = (int)bctbx_list_size(linphone_core_get_calls(marie->lc));

	laure_stats = laure->stat;
	pauline_stats = pauline->stat;
	michelle_stats = michelle->stat;
	marie_stats = marie->stat;

	linphone_core_terminate_all_calls(marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
	                             marie_stats.number_of_LinphoneCallEnd + marie_calls_no, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,
	                             pauline_stats.number_of_LinphoneCallEnd + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd,
	                             michelle_stats.number_of_LinphoneCallEnd + 1, 5000));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, laure_stats.number_of_LinphoneCallEnd + 1, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
	                             marie_stats.number_of_LinphoneCallReleased + marie_calls_no, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased,
	                             pauline_stats.number_of_LinphoneCallReleased + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased,
	                             michelle_stats.number_of_LinphoneCallReleased + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
	                             laure_stats.number_of_LinphoneCallReleased + 1, 5000));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
	if (new_participants) {
		bctbx_list_free(new_participants);
	}
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
}

static void participant_takes_call_after_conference_started_and_rejoins_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *chloe_call_laure;
	LinphoneCall *laure_called_by_chloe;
	LinphoneConference *marie_conference = NULL;
	LinphoneAddress *marie_conference_address = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	stats marie_initial_stats = marie->stat;
	stats pauline_initial_stats = pauline->stat;
	stats michelle_initial_stats = michelle->stat;
	stats laure_initial_stats = laure->stat;
	stats chloe_initial_stats = chloe->stat;

	const LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe, laure));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausing,
	                             (laure_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             (marie_initial_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused,
	                             (laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// Pauline and Michelle are notified of Laure media changes and marie temporarely left the conference
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_initial_stats.number_of_participant_devices_media_capability_changed + 1,
	                             3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_media_capability_changed,
	                             pauline_initial_stats.number_of_participant_devices_media_capability_changed + 1,
	                             3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_on_hold,
	                             michelle_initial_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_on_hold,
	                             michelle_initial_stats.number_of_participant_devices_on_hold + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_conference_participant_devices_on_hold,
	                             pauline_initial_stats.number_of_conference_participant_devices_on_hold + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_participant_devices_on_hold,
	                             pauline_initial_stats.number_of_participant_devices_on_hold + 1, 3000));

	marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");
	marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(marie_conference));

	LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	linphone_address_unref(laure_uri);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);
	if (laure_conference) {
		BC_ASSERT_FALSE(linphone_conference_is_in(laure_conference));
	}

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int,
	                "%d");

	chloe_call_laure = linphone_core_get_current_call(chloe->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;

	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");

	wait_for_list(lcs, NULL, 0, 2000);

	LinphoneCall *laure_calls_marie = linphone_core_get_call_by_remote_address2(laure->lc, marie->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_calls_marie)) goto end;
	// Remote  conference
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));

	marie_initial_stats = marie->stat;
	pauline_initial_stats = pauline->stat;
	michelle_initial_stats = michelle->stat;
	laure_initial_stats = laure->stat;
	chloe_initial_stats = chloe->stat;

	// Call between Chloe and Laure ends and Laure rejoins conference
	if (laure_called_by_chloe) {
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
		linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallEnd,
		                        laure_initial_stats.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallEnd,
		                        chloe_initial_stats.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallReleased,
		                        laure_initial_stats.number_of_LinphoneCallReleased + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallReleased,
		                        chloe_initial_stats.number_of_LinphoneCallReleased + 1));

		// Remote  conference
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");

		marie_initial_stats = marie->stat;
		pauline_initial_stats = pauline->stat;
		michelle_initial_stats = michelle->stat;
		laure_initial_stats = laure->stat;

		linphone_call_resume(laure_calls_marie);

		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &laure->stat.number_of_LinphoneCallResuming,
		                        laure_initial_stats.number_of_LinphoneCallResuming + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &laure->stat.number_of_LinphoneCallStreamsRunning,
		                        laure_initial_stats.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        marie_initial_stats.number_of_LinphoneCallStreamsRunning + 1));

		bool_t event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(laure->lc), "misc", "conference_event_log_enabled", TRUE);

		if (event_log_enabled) {
			BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                              (laure_initial_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),
			                              1000));
			BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionIncomingReceived,
			                              (marie_initial_stats.number_of_LinphoneSubscriptionIncomingReceived + 1),
			                              1000));

			BC_ASSERT_FALSE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionActive,
			                              (laure_initial_stats.number_of_LinphoneSubscriptionActive + 1), 1000));
			BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive,
			                              (marie_initial_stats.number_of_LinphoneSubscriptionActive + 1), 1000));
		}
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived,
	                             (pauline_initial_stats.number_of_NotifyReceived + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
	                             (michelle_initial_stats.number_of_NotifyReceived + 1), 5000));

	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
	BC_ASSERT_TRUE(linphone_conference_is_in(laure_conference));

	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");

	new_participants = terminate_participant_call(new_participants, marie, laure);

	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
}

static void participants_take_call_after_conference_started_and_rejoins_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *marie_call_chloe;
	LinphoneCall *chloe_called_by_marie;
	LinphoneCall *chloe_call_laure;
	LinphoneCall *laure_called_by_chloe;
	LinphoneAddress *marie_conference_address = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, chloe->lc);

	if (!BC_ASSERT_TRUE(call(marie, chloe))) goto end;

	chloe_called_by_marie = linphone_core_get_current_call(chloe->lc);
	marie_call_chloe = linphone_core_get_call_by_remote_address2(marie->lc, chloe->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_chloe, chloe, chloe_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, laure);
	new_participants = bctbx_list_append(new_participants, chloe);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	if (marie_conference) {
		BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");
		marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(marie_conference));
	} else {
		goto end;
	}

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;
	stats chloe_initial_stats = chloe->stat;
	stats michelle_initial_stats = michelle->stat;

	const LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe, laure));

	// Wait a little bit
	wait_for_list(lcs, NULL, 0, 2000);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausing,
	                             (laure_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallPausing,
	                             (chloe_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             (marie_initial_stats.number_of_LinphoneCallPausedByRemote + 2), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused,
	                             (laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallPaused,
	                             (chloe_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// Michelle is notified that Laure and Chloe media capabilities changed and they temporarely left the conference
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_initial_stats.number_of_participant_devices_media_capability_changed + 2,
	                             3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_conference_participant_devices_on_hold,
	                             michelle_initial_stats.number_of_conference_participant_devices_on_hold + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_on_hold,
	                             michelle_initial_stats.number_of_participant_devices_on_hold + 2, 3000));

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int,
	                "%d");

	chloe_call_laure = linphone_core_get_call_by_remote_address2(chloe->lc, laure->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;

	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");

	// Taking michelle out of the conference causes the conference to be destroyed
	new_participants = terminate_participant_call(new_participants, marie, michelle);
	bctbx_list_free(new_participants);

	wait_for_list(lcs, NULL, 0, 2000);

	LinphoneCall *laure_calls_marie = linphone_core_get_call_by_remote_address2(laure->lc, marie->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_calls_marie)) goto end;

	LinphoneCall *chloe_calls_marie = linphone_core_get_call_by_remote_address2(chloe->lc, marie->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_calls_marie)) goto end;

	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	if (marie_conference) {
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
	}

	// Remote  conference
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(chloe_calls_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_calls_marie));

	LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);

	LinphoneAddress *chloe_uri = linphone_address_new(linphone_core_get_identity(chloe->lc));
	LinphoneConference *chloe_conference =
	    linphone_core_search_conference(chloe->lc, NULL, chloe_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(chloe_conference);

	if (laure_called_by_chloe) {
		marie_initial_stats = marie->stat;
		laure_initial_stats = laure->stat;
		chloe_initial_stats = chloe->stat;
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
		linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallEnd,
		                        laure_initial_stats.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallEnd,
		                        chloe_initial_stats.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallReleased,
		                        laure_initial_stats.number_of_LinphoneCallReleased + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallReleased,
		                        chloe_initial_stats.number_of_LinphoneCallReleased + 1));

		// Remote  conference
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");

		linphone_call_resume(laure_calls_marie);

		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &laure->stat.number_of_LinphoneCallResuming,
		                        laure_initial_stats.number_of_LinphoneCallResuming + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &laure->stat.number_of_LinphoneCallStreamsRunning,
		                        laure_initial_stats.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        marie_initial_stats.number_of_LinphoneCallStreamsRunning + 1));

		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_participant_devices_media_capability_changed,
		                             chloe_initial_stats.number_of_participant_devices_media_capability_changed + 1,
		                             3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_present,
		                             marie_initial_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_present,
		                             marie_initial_stats.number_of_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_conference_participant_devices_present,
		                             chloe_initial_stats.number_of_conference_participant_devices_present + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_participant_devices_present,
		                             chloe_initial_stats.number_of_participant_devices_present + 1, 3000));

		// Remote  conference
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
	}

	laure_conference = linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(laure_conference);
	if (laure_conference) {
		BC_ASSERT_TRUE(linphone_conference_is_in(laure_conference));
	}
	linphone_address_unref(laure_uri);

	chloe_conference = linphone_core_search_conference(chloe->lc, NULL, chloe_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NOT_NULL(chloe_conference);
	if (chloe_conference) {
		BC_ASSERT_FALSE(linphone_conference_is_in(chloe_conference));
	}
	linphone_address_unref(chloe_uri);

	marie_initial_stats = marie->stat;
	laure_initial_stats = laure->stat;
	chloe_initial_stats = chloe->stat;
	linphone_core_terminate_call(laure->lc, laure_calls_marie);
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &laure->stat.number_of_LinphoneCallEnd,
	                        laure_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallEnd,
	                        marie_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &laure->stat.number_of_LinphoneCallReleased,
	                        laure_initial_stats.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallReleased,
	                        marie_initial_stats.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
	                             laure_initial_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneSubscriptionTerminated,
	                             chloe_initial_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             marie_initial_stats.number_of_LinphoneSubscriptionTerminated + 2,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (chloe_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneConferenceStateTerminated,
	                             (chloe_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneConferenceStateDeleted,
	                             (chloe_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	marie_initial_stats = marie->stat;
	chloe_initial_stats = chloe->stat;
	linphone_core_terminate_call(chloe->lc, chloe_calls_marie);
	BC_ASSERT_TRUE(wait_for(marie->lc, chloe->lc, &chloe->stat.number_of_LinphoneCallEnd,
	                        chloe_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, chloe->lc, &marie->stat.number_of_LinphoneCallEnd,
	                        marie_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, chloe->lc, &chloe->stat.number_of_LinphoneCallReleased,
	                        chloe_initial_stats.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, chloe->lc, &marie->stat.number_of_LinphoneCallReleased,
	                        marie_initial_stats.number_of_LinphoneCallReleased + 1));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(chloe->lc));

end:

	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
}

static void participant_takes_call_after_conference_started_and_rejoins_conference_after_conference_ended(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);

	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *chloe_call_laure;
	LinphoneCall *laure_called_by_chloe;
	LinphoneAddress *marie_conference_address = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, laure))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
	if (marie_conference) {
		marie_conference_address = linphone_address_clone(linphone_conference_get_conference_address(marie_conference));
	} else {
		goto end;
	}

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;
	stats michelle_initial_stats = michelle->stat;
	stats chloe_initial_stats = chloe->stat;

	const LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe, laure));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausing,
	                             (laure_initial_stats.number_of_LinphoneCallPausing + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote,
	                             (marie_initial_stats.number_of_LinphoneCallPausedByRemote + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused,
	                             (laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// Michelle is notified that Laure media capability changed
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_participant_devices_media_capability_changed,
	                             michelle_initial_stats.number_of_participant_devices_media_capability_changed + 1,
	                             3000));

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int,
	                "%d");

	chloe_call_laure = linphone_core_get_current_call(chloe->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;

	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");

	// Taking michelle out of the conference causes the conference to be destroyed
	new_participants = terminate_participant_call(new_participants, marie, michelle);
	bctbx_list_free(new_participants);

	wait_for_list(lcs, NULL, 0, 2000);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated,
	                             laure_initial_stats.number_of_LinphoneSubscriptionTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
	                             marie_initial_stats.number_of_LinphoneSubscriptionTerminated + 2,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted,
	                             (laure_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateTerminated + 1), 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted,
	                             (marie_initial_stats.number_of_LinphoneConferenceStateDeleted + 1), 5000));

	LinphoneCall *laure_calls_marie = linphone_core_get_call_by_remote_address2(laure->lc, marie->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_calls_marie)) goto end;

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	// Remote  conference
	BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_calls_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));

	LinphoneAddress *laure_uri = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneConference *laure_conference =
	    linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NULL(laure_conference);

	// Call between Chloe and Laure ends and Laure resumes call with Marie. Conferece has terminated because
	// Michelle left in the meantime
	if (laure_called_by_chloe) {
		marie_initial_stats = marie->stat;
		laure_initial_stats = laure->stat;
		chloe_initial_stats = chloe->stat;
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
		linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallEnd,
		                        laure_initial_stats.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallEnd,
		                        chloe_initial_stats.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &laure->stat.number_of_LinphoneCallReleased,
		                        laure_initial_stats.number_of_LinphoneCallReleased + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc, laure->lc, &chloe->stat.number_of_LinphoneCallReleased,
		                        chloe_initial_stats.number_of_LinphoneCallReleased + 1));

		// Remote  conference
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");

		linphone_call_resume(laure_calls_marie);

		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &laure->stat.number_of_LinphoneCallResuming,
		                        laure_initial_stats.number_of_LinphoneCallResuming + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &laure->stat.number_of_LinphoneCallStreamsRunning,
		                        laure_initial_stats.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                        marie_initial_stats.number_of_LinphoneCallStreamsRunning + 1));

		// Remote  conference
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
	}

	laure_conference = linphone_core_search_conference(laure->lc, NULL, laure_uri, marie_conference_address, NULL);
	BC_ASSERT_PTR_NULL(laure_conference);
	linphone_address_unref(laure_uri);

	marie_initial_stats = marie->stat;
	laure_initial_stats = laure->stat;
	chloe_initial_stats = chloe->stat;
	linphone_core_terminate_call(laure->lc, laure_calls_marie);
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &laure->stat.number_of_LinphoneCallEnd,
	                        laure_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallEnd,
	                        marie_initial_stats.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &laure->stat.number_of_LinphoneCallReleased,
	                        laure_initial_stats.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallReleased,
	                        marie_initial_stats.number_of_LinphoneCallReleased + 1));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(chloe->lc));

end:

	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);
	if (marie_conference_address) {
		linphone_address_unref(marie_conference_address);
	}
}

static void toggle_video_settings_during_conference_base(bool_t automatically_video_accept, bool_t defer_update) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	bctbx_list_t *new_participants = NULL;
	LinphoneConference *conference = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, automatically_video_accept);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == automatically_video_accept);
	}

	linphone_video_activation_policy_unref(pol);

	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *michelle_call_params = linphone_core_create_call_params(michelle->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_enable_video(pauline_call_params, TRUE);
	linphone_call_params_enable_video(laure_call_params, TRUE);
	linphone_call_params_enable_video(michelle_call_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call_with_params(marie, michelle, marie_call_params, michelle_call_params))) goto end;
	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);

	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(michelle_call_params);
	linphone_call_params_unref(laure_call_params);

	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);

	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conference = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conference);
	if (conference) {
		add_calls_to_local_conference(lcs, marie, conference, new_participants, TRUE);

		BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		unsigned int no_participants = (unsigned int)bctbx_list_size(new_participants);
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_participants, int, "%d");

		// Wait that the three participants have joined the local conference, by checking the StreamsRunning states
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2 * no_participants,
		                             liblinphone_tester_sip_timeout));

		const LinphoneAddress *conference_address = linphone_conference_get_conference_address(conference);
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			if (defer_update == TRUE) {
				linphone_config_set_int(linphone_core_get_config(c), "sip", "defer_update_default", TRUE);
			}
		}

		// Disable video in calls of the remote participants
		bool_t video_enabled = FALSE;

		for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			set_video_in_call(m, marie, video_enabled, new_participants, conference_address);
		}

		// Verify that video capabilities are still enabled
		const LinphoneConferenceParams *params = linphone_conference_get_current_params(conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == TRUE);

		wait_for_list(lcs, NULL, 0, 2000);

		BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_participants, int, "%d");
		const LinphoneCallParams *call_params = linphone_call_get_current_params(marie_call_michelle);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(michelle_called_by_marie);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(marie_call_laure);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(laure_called_by_marie);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(marie_call_pauline);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_called_by_marie);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));

		video_enabled = TRUE;
		// Video is enabled in the call between Marie and Laure
		set_video_in_call(laure, marie, video_enabled, new_participants, conference_address);

		wait_for_list(lcs, NULL, 0, 2000);

		BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_participants, int, "%d");
		call_params = linphone_call_get_current_params(marie_call_michelle);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(michelle_called_by_marie);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(marie_call_laure);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(laure_called_by_marie);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(marie_call_pauline);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_called_by_marie);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));

		// Pauline tries to enable video in the current call
		set_video_in_call(pauline, marie, video_enabled, new_participants, conference_address);

		wait_for_list(lcs, NULL, 0, 2000);

		call_params = linphone_call_get_current_params(marie_call_michelle);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(michelle_called_by_marie);
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(marie_call_laure);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(laure_called_by_marie);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(marie_call_pauline);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));
		call_params = linphone_call_get_current_params(pauline_called_by_marie);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_params));

		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			if (defer_update == TRUE) {
				linphone_config_set_int(linphone_core_get_config(c), "sip", "defer_update_default", FALSE);
			}
		}

		terminate_conference(new_participants, marie, conference, NULL, FALSE);
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	if (conference) linphone_conference_unref(conference);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	if (new_participants) bctbx_list_free(new_participants);
	bctbx_list_free(lcs);
}

static void toggle_video_settings_during_conference_with_update_deferred(void) {
	toggle_video_settings_during_conference_base(FALSE, TRUE);
}

static void toggle_video_settings_during_conference_with_automatically_accept_video_policy(void) {
	toggle_video_settings_during_conference_base(TRUE, FALSE);
}

static void toggle_video_settings_during_conference_without_automatically_accept_video_policy(void) {
	toggle_video_settings_during_conference_base(FALSE, FALSE);
}

static void simultaneous_toggle_video_settings_during_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *marie_call_laure;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	bctbx_list_t *new_participants = NULL;
	LinphoneConference *conference = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));
	}

	linphone_video_activation_policy_unref(pol);

	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *michelle_call_params = linphone_core_create_call_params(michelle->lc, NULL);
	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_enable_video(pauline_call_params, TRUE);
	linphone_call_params_enable_video(laure_call_params, TRUE);
	linphone_call_params_enable_video(michelle_call_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call_with_params(marie, michelle, marie_call_params, michelle_call_params))) goto end;
	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(michelle_call_params);
	linphone_call_params_unref(laure_call_params);

	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);

	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conference = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	BC_ASSERT_PTR_NOT_NULL(conference);
	if (conference) {
		add_calls_to_local_conference(lcs, marie, conference, new_participants, TRUE);

		BC_ASSERT_TRUE(linphone_conference_is_in(conference));
		unsigned int no_participants = (unsigned int)bctbx_list_size(new_participants);
		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_participants, int, "%d");

		// Wait that the three participants have joined the local conference, by checking the StreamsRunning states
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 2 * no_participants,
		                             liblinphone_tester_sip_timeout));

		bool_t enable_video = FALSE;

		for (int i = 0; i < 4; i++) {
			stats *participants_initial_stats = NULL;
			int counter = 1;
			stats marie_stats = marie->stat;
			for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
				LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
				LinphoneCore *c = m->lc;

				participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
				// Append element
				participants_initial_stats[counter - 1] = m->stat;

				LinphoneCall *call = linphone_core_get_call_by_remote_address2(c, marie->identity);
				BC_ASSERT_PTR_NOT_NULL(call);

				if (call) {
					const LinphoneCallParams *participant_call_params = linphone_call_get_current_params(call);
					BC_ASSERT_FALSE(linphone_call_params_video_enabled(participant_call_params) == enable_video);
					ms_message("%s %s video", linphone_core_get_identity(m->lc),
					           (enable_video ? "enables" : "disables"));
					LinphoneCallParams *new_params = linphone_core_create_call_params(c, call);
					linphone_call_params_enable_video(new_params, enable_video);
					linphone_call_update(call, new_params);
					linphone_call_params_unref(new_params);
				}

				counter++;
			}

			counter = 0;
			for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
				LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdating,
				                             participants_initial_stats[counter].number_of_LinphoneCallUpdating + 1,
				                             5000));
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
				                  participants_initial_stats[counter].number_of_LinphoneCallStreamsRunning + 1, 5000));

				BC_ASSERT_TRUE(wait_for_list(
				    lcs, &m->stat.number_of_participant_devices_media_capability_changed,
				    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 2,
				    3000));
				BC_ASSERT_FALSE(
				    wait_for_list(lcs, &m->stat.number_of_participant_devices_added,
				                  participants_initial_stats[counter].number_of_participant_devices_added + 1, 3000));
				BC_ASSERT_FALSE(wait_for_list(
				    lcs, &m->stat.number_of_conference_participant_devices_present,
				    participants_initial_stats[counter].number_of_conference_participant_devices_present + 1, 3000));
				BC_ASSERT_FALSE(
				    wait_for_list(lcs, &m->stat.number_of_participant_devices_present,
				                  participants_initial_stats[counter].number_of_participant_devices_present + 1, 3000));

				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, marie->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					const LinphoneCallParams *participant_call_params =
					    linphone_call_get_current_params(participant_call);
					BC_ASSERT_TRUE(linphone_call_params_video_enabled(participant_call_params) == enable_video);
				}

				LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
				BC_ASSERT_PTR_NOT_NULL(focus_call);
				if (focus_call) {
					const LinphoneCallParams *focus_call_params = linphone_call_get_current_params(focus_call);
					BC_ASSERT_TRUE(linphone_call_params_video_enabled(focus_call_params) == enable_video);
				}

				counter++;
			}

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
			                             marie_stats.number_of_LinphoneCallUpdatedByRemote + 3, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
			                             marie_stats.number_of_LinphoneCallStreamsRunning + 3, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_media_capability_changed,
			                             marie_stats.number_of_participant_devices_media_capability_changed + 3, 3000));
			BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_added,
			                              marie_stats.number_of_participant_devices_added + 1, 3000));
			BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_conference_participant_devices_present,
			                              marie_stats.number_of_conference_participant_devices_present + 1, 3000));
			BC_ASSERT_FALSE(wait_for_list(lcs, &marie->stat.number_of_participant_devices_present,
			                              marie_stats.number_of_participant_devices_present + 1, 3000));

			ms_free(participants_initial_stats);
			enable_video = !enable_video;
		}

		terminate_conference(new_participants, marie, conference, NULL, FALSE);
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	if (conference) linphone_conference_unref(conference);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	if (new_participants) bctbx_list_free(new_participants);
	bctbx_list_free(lcs);
}

/*
static void update_conf_params_during_conference(void) {
    LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE, NULL);
    linphone_core_enable_conference_server(marie->lc,TRUE);
    LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE, NULL);
    LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" :
"laure_rc_udp", TRUE, NULL); LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc", TRUE, NULL);

    LinphoneCall* marie_call_pauline;
    LinphoneCall* pauline_called_by_marie;
    LinphoneCall* marie_call_michelle;
    LinphoneCall* michelle_called_by_marie;
    LinphoneCall* marie_call_laure;
    bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
    bctbx_list_t* new_participants=NULL;
    lcs=bctbx_list_append(lcs,pauline->lc);
    lcs=bctbx_list_append(lcs,michelle->lc);
    lcs=bctbx_list_append(lcs,laure->lc);

    char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
    linphone_core_set_play_file(pauline->lc, play_file_pauline);
    bc_free(play_file_pauline);

    LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
    linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

    for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
        LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
        linphone_core_set_video_activation_policy(c, pol);
        linphone_core_set_video_device(c, liblinphone_tester_mire_id);
        linphone_core_enable_video_capture(c, TRUE);
        linphone_core_enable_video_display(c, TRUE);

        const LinphoneVideoActivationPolicy * cpol = linphone_core_get_video_activation_policy(c);
        BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));
    }

    linphone_video_activation_policy_unref(pol);

    BC_ASSERT_TRUE(call(marie,pauline));
    marie_call_pauline=linphone_core_get_current_call(marie->lc);
    pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
    BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

    if (!BC_ASSERT_TRUE(call(marie,michelle)))
        goto end;
    marie_call_michelle=linphone_core_get_current_call(marie->lc);
    michelle_called_by_marie=linphone_core_get_current_call(michelle->lc);
    BC_ASSERT_TRUE(pause_call_1(marie,marie_call_michelle,michelle,michelle_called_by_marie));

    BC_ASSERT_TRUE(call(marie,laure));

    marie_call_laure=linphone_core_get_current_call(marie->lc);

    if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure))
        goto end;

    new_participants=bctbx_list_append(new_participants,michelle);
    new_participants=bctbx_list_append(new_participants,pauline);
    new_participants=bctbx_list_append(new_participants,laure);
    add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);

    LinphoneConference * marie_conference = linphone_core_get_conference(marie->lc);
    BC_ASSERT_PTR_NOT_NULL(marie_conference);
    BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
    BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference),3, int, "%d");

    // Wait that the three participants have joined the local conference, by checking the StreamsRunning states
    BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2,
liblinphone_tester_sip_timeout));
BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2,
liblinphone_tester_sip_timeout));
    BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2,
liblinphone_tester_sip_timeout));
BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6,
liblinphone_tester_sip_timeout));

    // Enable video
    bool_t video_enabled = TRUE;
    set_video_in_conference(lcs, marie, new_participants, video_enabled);

    wait_for_list(lcs ,NULL, 0, 2000);

    // Disable video
    video_enabled = FALSE;
    set_video_in_conference(lcs, marie, new_participants, video_enabled);

    // Remote  conference
    BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(pauline_called_by_marie));
    BC_ASSERT_FALSE(linphone_call_is_in_conference(pauline_called_by_marie));
    BC_ASSERT_TRUE(linphone_call_is_in_conference(marie_call_pauline));
    new_participants = terminate_participant_call(new_participants, marie, pauline);

    wait_for_list(lcs ,NULL, 0, 2000);

    BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
    unsigned int no_participants = (unsigned int)bctbx_list_size(new_participants);
    BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference),no_participants, int, "%d");

    for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
        LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
        LinphoneCore * c = m->lc;

        LinphoneCall * participant_call = linphone_core_get_current_call(c);
        BC_ASSERT_PTR_NOT_NULL(participant_call);

        // Remote  conference
        BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
        BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
        const LinphoneCallParams * participant_call_params = linphone_call_get_current_params(participant_call);
        BC_ASSERT_TRUE(linphone_call_params_video_enabled(participant_call_params) == video_enabled);

        const LinphoneAddress *participant_uri = m->identity;
        LinphoneCall * conf_call = linphone_core_get_call_by_remote_address2(marie->lc, participant_uri);
        BC_ASSERT_PTR_NOT_NULL(conf_call);
        const LinphoneCallParams * conf_call_params = linphone_call_get_current_params(conf_call);
        BC_ASSERT_TRUE(linphone_call_params_video_enabled(conf_call_params) == video_enabled);

        LinphoneConference *pconference = linphone_call_get_conference(participant_call);
        BC_ASSERT_PTR_NOT_NULL(pconference);
        if (pconference) {
            bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
            BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), no_participants, unsigned int, "%u");
            bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);
        }
    }

    wait_for_list(lcs ,NULL, 0, 2000);

    terminate_conference(new_participants, marie, NULL, NULL, FALSE);

    BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
    BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
    BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
    BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
    BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

    destroy_mgr_in_conference(pauline);
    destroy_mgr_in_conference(laure);
    destroy_mgr_in_conference(michelle);
    destroy_mgr_in_conference(marie);
    if (new_participants) bctbx_list_free(new_participants);
    bctbx_list_free(lcs);
}
*/

static void focus_takes_quick_call_after_conference_started_base(bool_t toggle_video) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_called_by_laure;
	LinphoneCall *laure_call_marie;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);

	bctbx_list_t *lcs2 = bctbx_list_copy(lcs);
	lcs2 = bctbx_list_append(lcs2, laure->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs2; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);
		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));
	}

	linphone_video_activation_policy_unref(pol);
	bctbx_list_free(lcs2);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(conference)) goto end;
	const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(conf_params));

	bool_t video_enabled = !linphone_conference_params_video_enabled(conf_params);
	if (toggle_video == TRUE) {
		// Toggle video
		set_video_in_conference(lcs, marie, new_participants, video_enabled);
	}

	lcs = bctbx_list_append(lcs, laure->lc);
	BC_ASSERT_TRUE(call(laure, marie));

	marie_called_by_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_called_by_laure)) goto end;
	laure_call_marie = linphone_core_get_current_call(laure->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_call_marie)) goto end;

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call_marie)) == FALSE);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_called_by_laure)) ==
	               FALSE);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	// Local participant is expected to have left as it joined another call
	BC_ASSERT_FALSE(linphone_conference_is_in(marie_conference));

	bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
	bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

	conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(conf_params));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	stats marie_stat = marie->stat;
	stats laure_stat = laure->stat;
	linphone_conference_enter(conference);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,
	                             marie_stat.number_of_LinphoneCallPausing + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,
	                             marie_stat.number_of_LinphoneCallPaused + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote,
	                             laure_stat.number_of_LinphoneCallPausedByRemote + 1, 5000));

	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	if (toggle_video == TRUE) {
		conf_params = linphone_conference_get_current_params(conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(conf_params) == video_enabled);
	}

	wait_for_list(lcs, NULL, 0, 2000);

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(new_participants);

	unsigned int marie_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(marie_call_no, 1, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	// Current call is not set because the call between Marie and Laure is paused as Marie re-entered the conference
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));

	marie_stat = marie->stat;
	laure_stat = laure->stat;
	unsigned int laure_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc));
	BC_ASSERT_EQUAL(laure_call_no, 1, unsigned int, "%u");
	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd,
	                             marie_stat.number_of_LinphoneCallEnd + marie_call_no, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, laure_stat.number_of_LinphoneCallEnd + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased,
	                             marie_stat.number_of_LinphoneCallReleased + marie_call_no,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased,
	                             laure_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

#if 0
static void enable_video_during_conference_and_take_another_call(void) {
	focus_takes_quick_call_after_conference_started_base(TRUE);
}
#endif

static void focus_takes_quick_call_after_conference_started(void) {
	focus_takes_quick_call_after_conference_started_base(FALSE);
}

static void try_to_update_call_params_during_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *marie_call_laure;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));
	}
	linphone_video_activation_policy_unref(pol);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;
	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call(marie, laure));

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, TRUE);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");
	const LinphoneConferenceParams *marie_conf_params = linphone_conference_get_current_params(marie_conference);
	bool_t orig_marie_video_enabled = linphone_conference_params_video_enabled(marie_conf_params);

	// Remote  conference
	LinphoneConference *pauline_conference = linphone_call_get_conference(pauline_called_by_marie);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	BC_ASSERT_FALSE(linphone_call_is_in_conference(pauline_called_by_marie));
	const LinphoneConferenceParams *pauline_conf_params = linphone_conference_get_current_params(pauline_conference);
	bool_t orig_pauline_video_enabled = linphone_conference_params_video_enabled(pauline_conf_params);

	if (pauline_called_by_marie) {
		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		LinphoneCallParams *new_params = linphone_core_create_call_params(marie->lc, pauline_called_by_marie);
		linphone_call_params_enable_video(new_params, TRUE);
		linphone_call_update(pauline_called_by_marie, new_params);
		linphone_call_params_unref(new_params);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
		                             initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating,
		                             initial_pauline_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,
		                             initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));

		// Test that update to call params of a participant didn't affect conference params
		const LinphoneConferenceParams *pauline_conf_params =
		    linphone_conference_get_current_params(pauline_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(pauline_conf_params) == orig_pauline_video_enabled);

		const LinphoneConferenceParams *marie_conf_params = linphone_conference_get_current_params(marie_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(marie_conf_params) == orig_marie_video_enabled);

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_called_by_marie)));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call_pauline)));
	}

	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");

	// Ensure that the core has not been kicked out of the conference
	// Remote  conference
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(pauline_called_by_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(pauline_called_by_marie));
	if (pauline_called_by_marie) {
		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		stats initial_laure_stat = laure->stat;
		stats initial_michelle_stat = michelle->stat;
		linphone_core_terminate_call(marie->lc, pauline_called_by_marie);
		new_participants = bctbx_list_remove(new_participants, pauline);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

		// Wait for conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (initial_pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated,
		                             (initial_pauline_stat.number_of_LinphoneConferenceStateTerminated + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted,
		                             (initial_pauline_stat.number_of_LinphoneConferenceStateDeleted + 1), 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_pauline_stat.number_of_LinphoneSubscriptionTerminated + 1, 3000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_NotifyReceived,
		                             (initial_laure_stat.number_of_NotifyReceived + 2), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_NotifyReceived,
		                             (initial_michelle_stat.number_of_NotifyReceived + 2), 5000));
	}

	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 2, int, "%d");

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void register_again_during_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	stats initial_marie_stats = marie->stat;
	stats initial_pauline_stats = pauline->stat;
	stats initial_laure_stats = laure->stat;

	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_michelle;
	LinphoneCall *michelle_called_by_marie;
	LinphoneCall *marie_call_laure;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, michelle->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie, michelle))) goto end;
	marie_call_michelle = linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie = linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_michelle, michelle, michelle_called_by_marie));

	BC_ASSERT_TRUE(call(marie, laure));

	marie_call_laure = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants, FALSE);

	LinphoneConference *marie_conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 6, liblinphone_tester_sip_timeout));

	LinphoneProxyConfig *proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, FALSE);
	linphone_proxy_config_done(proxyConfig);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneRegistrationCleared,
	                             initial_laure_stats.number_of_LinphoneRegistrationCleared + 1, 5000));

	wait_for_list(lcs, NULL, 1, 3000);

	// to force re-re-connection to restarted flexisip
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, FALSE);
	linphone_core_set_network_reachable(marie->lc, FALSE);
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));

	proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, TRUE);
	linphone_proxy_config_done(proxyConfig);

	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneRegistrationOk,
	                             initial_marie_stats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneRegistrationOk,
	                             initial_pauline_stats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));
	linphone_core_set_network_reachable(laure->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneRegistrationOk,
	                             initial_laure_stats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));

	// Wait for subscriptins to be resent
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 5, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionActive, 2, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(linphone_conference_is_in(marie_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(marie_conference), 3, int, "%d");

	terminate_conference(new_participants, marie, NULL, NULL, FALSE);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void
simple_conference_base2(LinphoneCoreManager *local_conf, bctbx_list_t *participants, bool_t use_conference_terminate) {

	bctbx_list_t *lcs = bctbx_list_append(NULL, local_conf->lc);
	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	bctbx_list_t *unique_participant_identity = NULL;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		if (!BC_ASSERT_TRUE(call(local_conf, m))) return;
		lcs = bctbx_list_append(lcs, c);
		if (bctbx_list_find(unique_participant_identity, m->identity) == NULL) {
			unique_participant_identity = bctbx_list_append(unique_participant_identity, m->identity);
		}
		LinphoneCall *conf_call_participant = linphone_core_get_current_call(local_conf->lc);
		BC_ASSERT_PTR_NOT_NULL(conf_call_participant);
		LinphoneCall *participant_called_by_conf = linphone_core_get_current_call(c);
		BC_ASSERT_PTR_NOT_NULL(participant_called_by_conf);
		// Last call is not put on hold
		if (bctbx_list_next(it)) {
			BC_ASSERT_TRUE(pause_call_1(local_conf, conf_call_participant, m, participant_called_by_conf));
		}
	}
	unsigned int no_unique_participants = (unsigned int)bctbx_list_size(unique_participant_identity);
	bctbx_list_free(unique_participant_identity);

	add_calls_to_local_conference(lcs, local_conf, NULL, participants, TRUE);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		/* Wait that all participants have joined the local conference, by checking the StreamsRunning states*/
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &local_conf->stat.number_of_LinphoneCallStreamsRunning, 2 * no_participants,
	                             liblinphone_tester_sip_timeout));

	LinphoneConference *l_conference = linphone_core_get_conference(local_conf->lc);
	BC_ASSERT_PTR_NOT_NULL(l_conference);
	BC_ASSERT_TRUE(linphone_conference_is_in(l_conference));
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(l_conference), no_unique_participants, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(local_conf->lc));

	if (use_conference_terminate == TRUE) {
		terminate_conference(participants, local_conf, NULL, NULL, FALSE);
	} else {
		stats initial_conf_stats = local_conf->stat;
		int counter = 1;
		stats *initial_participants_stats = NULL;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			// Participant stats
			initial_participants_stats = (stats *)realloc(initial_participants_stats, counter * sizeof(stats));
			initial_participants_stats[counter - 1] = m->stat;

			LinphoneCall *m_call = linphone_core_get_current_call(m->lc);
			BC_ASSERT_PTR_NOT_NULL(m_call);
			linphone_core_terminate_call(m->lc, m_call);

			counter++;
		}

		int idx = 0;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for(local_conf->lc, m->lc, &m->stat.number_of_LinphoneCallEnd,
			                        (initial_participants_stats[idx].number_of_LinphoneCallEnd + 1)));
			BC_ASSERT_TRUE(wait_for(local_conf->lc, m->lc, &local_conf->stat.number_of_LinphoneCallEnd,
			                        (initial_conf_stats.number_of_LinphoneCallEnd + idx + 1)));

			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending,
			    (initial_participants_stats[idx].number_of_LinphoneConferenceStateTerminationPending + 1), 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated,
			                  (initial_participants_stats[idx].number_of_LinphoneConferenceStateTerminated + 1), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted,
			                             (initial_participants_stats[idx].number_of_LinphoneConferenceStateDeleted + 1),
			                             5000));

			BC_ASSERT_TRUE(wait_for_list(lcs, &local_conf->stat.number_of_LinphoneSubscriptionTerminated,
			                             initial_conf_stats.number_of_LinphoneSubscriptionTerminated + idx + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionTerminated,
			                             initial_participants_stats[idx].number_of_LinphoneSubscriptionTerminated + 1,
			                             3000));

			BC_ASSERT_TRUE(wait_for(local_conf->lc, m->lc, &m->stat.number_of_LinphoneCallReleased,
			                        (initial_participants_stats[idx].number_of_LinphoneCallReleased + 1)));
			BC_ASSERT_TRUE(wait_for(local_conf->lc, m->lc, &local_conf->stat.number_of_LinphoneCallReleased,
			                        (initial_conf_stats.number_of_LinphoneCallReleased + idx + 1)));

			idx++;
		}

		if (initial_participants_stats) {
			ms_free(initial_participants_stats);
		}

		BC_ASSERT_TRUE(wait_for_list(lcs, &local_conf->stat.number_of_LinphoneSubscriptionTerminated,
		                             initial_conf_stats.number_of_LinphoneSubscriptionActive,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &local_conf->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (initial_conf_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &local_conf->stat.number_of_LinphoneConferenceStateTerminated,
		                             (initial_conf_stats.number_of_LinphoneConferenceStateTerminated + 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &local_conf->stat.number_of_LinphoneConferenceStateDeleted,
		                             (initial_conf_stats.number_of_LinphoneConferenceStateDeleted + 1),
		                             liblinphone_tester_sip_timeout));
	}

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(local_conf->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(local_conf->lc)), 0, unsigned int, "%u");
	bctbx_list_free(lcs);
}

static void simple_4_participants_conference_ended_by_terminating_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	simple_conference_base2(marie, participants, TRUE);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void simple_4_participants_conference_ended_by_terminating_calls(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	simple_conference_base2(marie, participants, FALSE);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

#if 0
static void simple_conference_with_multi_device(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager* pauline2 = create_mgr_for_conference( "pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,laure);
	participants=bctbx_list_append(participants,pauline2);
	participants=bctbx_list_append(participants,pauline);
	simple_conference_base2(marie,participants,TRUE);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(pauline2);
	destroy_mgr_in_conference(laure);
}
#endif

static void simple_conference_with_local_participant_with_no_event_log(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_config_set_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_log_enabled", FALSE);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	simple_conference_base(marie, pauline, laure, NULL, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
}

static void simple_conference_with_remote_participant_with_no_event_log(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_log_enabled", FALSE);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	simple_conference_base(marie, pauline, laure, NULL, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
}

void simple_remote_conference_base(bool_t enable_ice) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(focus_mgr->lc);
	LinphoneProxyConfig *laure_proxy_config = linphone_core_get_default_proxy_config(laure->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	if (enable_ice) {
		linphone_core_set_firewall_policy(focus_mgr->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(laure->lc, LinphonePolicyUseIce);
	}

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	simple_conference_base(marie, pauline, laure, focus_mgr, FALSE);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	linphone_conference_server_destroy(focus);
}

void simple_remote_conference(void) {
	simple_remote_conference_base(FALSE);
}

void simple_ice_remote_conference(void) {
	simple_remote_conference_base(TRUE);
}

void simple_remote_conference_shut_down_focus(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", FALSE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	simple_conference_base(marie, pauline, laure, (LinphoneCoreManager *)focus, FALSE);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	linphone_conference_server_destroy(focus);
}

void eject_from_3_participants_remote_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	eject_from_3_participants_conference(marie, pauline, laure, (LinphoneCoreManager *)focus,
	                                     LinphoneConferenceLayoutActiveSpeaker);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	linphone_conference_server_destroy(focus);
}

void initiate_calls(bctbx_list_t *caller, LinphoneCoreManager *callee) {
	stats *initial_callers_stats = NULL;
	stats initial_callee_stat = callee->stat;

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, callee->lc);

	int counter = 1;
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		lcs = bctbx_list_append(lcs, m->lc);
		// Allocate memory
		initial_callers_stats = (stats *)realloc(initial_callers_stats, counter * sizeof(stats));
		// Append element
		initial_callers_stats[counter - 1] = m->stat;
		// Increment counter
		counter++;

		LinphoneCall *caller_call = linphone_core_invite_address(m->lc, callee->identity);
		BC_ASSERT_PTR_NOT_NULL(caller_call);
	}

	unsigned int no_callers = (unsigned int)bctbx_list_size(caller);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallIncomingReceived,
	                             initial_callee_stat.number_of_LinphoneCallIncomingReceived + no_callers, 5000));

	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCall *callee_call = linphone_core_get_call_by_remote_address2(callee->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(callee_call);
	}

	counter = 0;
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallOutgoingRinging,
		                             initial_callers_stats[counter].number_of_LinphoneCallOutgoingRinging + 1, 5000));
		counter++;
	}

	bctbx_list_free(lcs);
	if (initial_callers_stats) {
		ms_free(initial_callers_stats);
	}
}

static void initiate_call(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	bctbx_list_t *caller_list = NULL;
	caller_list = bctbx_list_append(caller_list, caller);
	initiate_calls(caller_list, callee);
	bctbx_list_free(caller_list);
}

static void take_calls_to_callee(bctbx_list_t *lcs, bctbx_list_t *caller, LinphoneCoreManager *callee) {

	LinphoneCall *current_call = linphone_core_get_current_call(callee->lc);
	LinphoneCall *current_call_caller = NULL;
	bool_t pausing_current_call = FALSE;
	if (current_call) {
		pausing_current_call = ((linphone_call_get_state(current_call) == LinphoneCallStreamsRunning) ||
		                        (linphone_call_get_state(current_call) == LinphoneCallPaused));
		char *remote_address_string = linphone_call_get_remote_address_as_string(current_call);
		// Search core that matches the remote address

		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneCoreManager *m = get_manager(c);
			char *identity_string = linphone_address_as_string(m->identity);
			bool_t manager_found = (strcmp(remote_address_string, identity_string) == 0);
			ms_free(identity_string);
			if (manager_found == TRUE) {
				current_call_caller = linphone_core_get_current_call(c);
				break;
			}
		}

		ms_free(remote_address_string);
	}
	stats initial_callee_stat = callee->stat;

	int no_paused_by_remote = 0;

	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *caller_mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
		const LinphoneAddress *caller_uri = caller_mgr->identity;

		// Decremement here no_paused_by_remote because past history has to be discarded for checks. Here, it is
		// imprtant only the delta between before and after taking the call. As below no_paused_by_remote is
		// incremented by the stats, a corrective factor (i.e. value before taking the call) is applied here
		no_paused_by_remote -= caller_mgr->stat.number_of_LinphoneCallPausedByRemote;
		LinphoneCall *callee_call = linphone_core_get_call_by_remote_address2(callee->lc, caller_uri);
		BC_ASSERT_PTR_NOT_NULL(callee_call);

		if (callee_call) {
			// Take call - ringing ends
			linphone_call_accept(callee_call);
		}
	}

	unsigned int no_callers = (unsigned int)bctbx_list_size(caller);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallStreamsRunning,
	                             initial_callee_stat.number_of_LinphoneCallStreamsRunning + no_callers, 5000));
	// Last call is not paused
	// If core had a running call, it will be paused
	unsigned int no_call_paused = no_callers - 1 + ((pausing_current_call) ? 1 : 0);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPausing,
	                             initial_callee_stat.number_of_LinphoneCallPausing + no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPaused,
	                             initial_callee_stat.number_of_LinphoneCallPaused + no_call_paused, 5000));

	int updated_by_remote_count = 0;
	int call_checked_cnt = 0;
	bool_t callee_uses_ice = (linphone_core_get_firewall_policy(callee->lc) == LinphonePolicyUseIce);
	// Wait that all calls but the last one are paused
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
		if (callee_uses_ice && linphone_core_get_firewall_policy(m->lc) == LinphonePolicyUseIce) {
			updated_by_remote_count++;
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdating, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &callee->stat.number_of_LinphoneCallUpdatedByRemote,
			    initial_callee_stat.number_of_LinphoneCallUpdatedByRemote + updated_by_remote_count, 5000));

			LinphoneCall *callee_call = linphone_core_get_call_by_remote_address2(callee->lc, m->identity);
			LinphoneCall *current_callee_call = linphone_core_get_current_call(callee->lc);
			if (callee_call == current_callee_call) {
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallStreamsRunning,
				                             initial_callee_stat.number_of_LinphoneCallStreamsRunning + no_callers + 1,
				                             5000));
			} else {
				call_checked_cnt++;
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallPausedByRemote, 2, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPaused,
				                             initial_callee_stat.number_of_LinphoneCallPaused + call_checked_cnt,
				                             5000));
				// If ICE is enabled, calls are paused twice:
				// - after accepting another call
				// - after ICE negotiation ends
				no_call_paused++;
			}
			BC_ASSERT_TRUE(check_ice(m, callee, LinphoneIceStateHostConnection));
			BC_ASSERT_TRUE(check_ice(callee, m, LinphoneIceStateHostConnection));
		}
		// Calls can be paused in an order different from the one they are accepted For example if ICE is enabled,
		// it may take longer to reach this state
		no_paused_by_remote += m->stat.number_of_LinphoneCallPausedByRemote;
	}
	if (pausing_current_call && current_call_caller) {
		no_paused_by_remote += (linphone_call_get_state(current_call_caller) == LinphoneCallPausedByRemote) ? 1 : 0;
	}

	BC_ASSERT_EQUAL(no_paused_by_remote, no_call_paused, int, "%d");
}

static void take_call_to_callee(bctbx_list_t *lcs, LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	bctbx_list_t *caller_list = NULL;
	caller_list = bctbx_list_append(caller_list, caller);
	take_calls_to_callee(lcs, caller_list, callee);
	bctbx_list_free(caller_list);
}

static void conference_with_calls_queued(LinphoneCoreManager *local_conf,
                                         LinphoneConference *conference,
                                         bctbx_list_t *participants,
                                         bool_t back_to_back_invite,
                                         bool_t back_to_back_accept) {
	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, local_conf->lc);

	stats initial_local_conf_stat = local_conf->stat;

	if (back_to_back_invite == TRUE) {
		initiate_calls(participants, local_conf);
	} else {
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			initiate_call(m, local_conf);
		}
	}

	// Let ring calls for a little while
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallStreamsRunning,
	                initial_local_conf_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallPausing,
	                initial_local_conf_stat.number_of_LinphoneCallPausing, int, "%d");
	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallPaused, initial_local_conf_stat.number_of_LinphoneCallPaused,
	                int, "%d");
	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallPausedByRemote,
	                initial_local_conf_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	if (back_to_back_accept == TRUE) {
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneCore *c = m->lc;
			lcs = bctbx_list_append(lcs, c);
		}
		take_calls_to_callee(lcs, participants, local_conf);
	} else {
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneCore *c = m->lc;
			lcs = bctbx_list_append(lcs, c);
			take_call_to_callee(lcs, m, local_conf);
		}
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(local_conf->lc));

	add_calls_to_local_conference(lcs, local_conf, conference, participants, FALSE);

	terminate_conference(participants, local_conf, conference, NULL, FALSE);

	bctbx_list_free(lcs);
}

static void conference_with_calls_queued_without_ice(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	conference_with_calls_queued(marie, NULL, participants, FALSE, FALSE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_calls_queued_with_ice(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// ICE is enabled
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(marie->lc, mode)) {
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc, mode);
	}

	// ICE is enabled
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(pauline->lc, mode)) {
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc, mode);
	}

	// ICE is enabled
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(laure->lc, mode)) {
		linphone_core_set_firewall_policy(laure->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(laure->lc, mode);
	}

	// ICE is enabled
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(chloe->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(chloe->lc, mode)) {
		linphone_core_set_firewall_policy(chloe->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc, mode);
	}

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, chloe);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	conference_with_calls_queued(marie, NULL, participants, FALSE, FALSE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}

static void conference_with_back_to_back_call_accept_without_ice(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	conference_with_calls_queued(marie, NULL, participants, FALSE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_back_to_back_call_accept_with_ice(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// ICE is enabled
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(marie->lc, mode)) {
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc, mode);
	}

	// ICE is enabled
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(pauline->lc, mode)) {
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc, mode);
	}

	// ICE is enabled
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(laure->lc, mode)) {
		linphone_core_set_firewall_policy(laure->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(laure->lc, mode);
	}

	// ICE is enabled
	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(chloe->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(chloe->lc, mode)) {
		linphone_core_set_firewall_policy(chloe->lc, LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc, mode);
	}

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, chloe);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	conference_with_calls_queued(marie, NULL, participants, FALSE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}

static void conference_with_back_to_back_call_invite_accept_without_ice(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	conference_with_calls_queued(marie, NULL, participants, TRUE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

#if 0
static void conference_with_back_to_back_call_invite_accept_with_ice(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// ICE is enabled
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(laure->lc,mode)) {
		linphone_core_set_firewall_policy(laure->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(laure->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(chloe->lc, liblinphone_tester_sip_timeout);
	if (linphone_core_media_encryption_supported(chloe->lc,mode)) {
		linphone_core_set_firewall_policy(chloe->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc,mode);
	}

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,chloe);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, NULL, participants, TRUE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}
#endif

static void back_to_back_conferences_same_core(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	// Marie hosts the conference
	conference_with_calls_queued(marie, NULL, participants, FALSE, FALSE);
	bctbx_list_free(participants);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, laure);
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);

	// Laure hosts the conference
	conference_with_calls_queued(marie, NULL, new_participants, FALSE, FALSE);
	bctbx_list_free(new_participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void back_to_back_conferences(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, laure);

	// Marie hosts the conference
	conference_with_calls_queued(marie, NULL, participants, FALSE, FALSE);
	bctbx_list_free(participants);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, michelle);
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, marie);

	// Laure hosts the conference
	conference_with_calls_queued(laure, NULL, new_participants, FALSE, FALSE);
	bctbx_list_free(new_participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void try_to_create_second_conference_with_local_participant(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(chloe->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, chloe);
	participants = bctbx_list_append(participants, pauline);

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, marie->lc);

	stats initial_marie_stat = marie->stat;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		lcs = bctbx_list_append(lcs, c);
		initiate_call(m, marie);
	}

	// Let ring calls for a little while
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                initial_marie_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausing, initial_marie_stat.number_of_LinphoneCallPausing, int,
	                "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, initial_marie_stat.number_of_LinphoneCallPaused, int,
	                "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote,
	                initial_marie_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(marie->lc));

	add_calls_to_local_conference(lcs, marie, NULL, participants, TRUE);

	stats *lcm_stats = NULL;

	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));

		// Append element
		lcm_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));
	lcm_stats[counter - 1] = marie->stat;

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	LinphoneConferenceParams *new_maries_conference_params = linphone_conference_params_new(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference_params);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(new_maries_conference_params));
	LinphoneConference *new_maries_conference =
	    linphone_core_create_conference_with_params(marie->lc, new_maries_conference_params);
	// As a side effect of changes made to method linphone_core_create_conference_with_params, it is now possible to
	// have multiple conference created by the core
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference);
	linphone_conference_params_unref(new_maries_conference_params);
	linphone_conference_terminate(new_maries_conference);
	linphone_conference_unref(new_maries_conference);

	linphone_conference_terminate(conference);

	int idx = 0;
	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	participants = bctbx_list_append(participants, marie);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;

		unsigned int no_calls = 0;
		unsigned int no_conference = 0;
		if (m == marie) {
			no_calls = no_participants;
			no_conference = 1;
		} else {
			no_calls = 1;
			no_conference = 1;
		}

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
		                             lcm_stats[idx].number_of_LinphoneCallEnd + no_calls,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
		                             lcm_stats[idx].number_of_LinphoneCallReleased + no_calls,
		                             liblinphone_tester_sip_timeout));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateTerminationPending + no_conference,
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateTerminated + no_conference, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateDeleted + no_conference, 5000));

		bool_t event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
		if ((m != marie) && event_log_enabled) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionTerminated,
			                             lcm_stats[idx].number_of_LinphoneSubscriptionTerminated + no_conference,
			                             liblinphone_tester_sip_timeout));
		}

		LinphoneConference *conference = linphone_core_get_conference(c);

		BC_ASSERT_PTR_NULL(conference);

		if (m != marie) {
			LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, marie->identity);
			BC_ASSERT_PTR_NULL(participant_call);
			LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
			BC_ASSERT_PTR_NULL(conference_call);
		}

		idx++;
	}

	// Verify that a second conference is created
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreationPending, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminationPending, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminated, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateDeleted, 2, int, "%d");

	ms_free(lcm_stats);

	bctbx_list_free(lcs);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}

static void interleaved_conferences_base(bool_t add_participants_immediately_after_creation) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(chloe->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, marie->lc);

	stats initial_marie_stat = marie->stat;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		lcs = bctbx_list_append(lcs, c);
		initiate_call(m, marie);
	}

	// Let ring calls for a little while
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                initial_marie_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausing, initial_marie_stat.number_of_LinphoneCallPausing, int,
	                "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, initial_marie_stat.number_of_LinphoneCallPaused, int,
	                "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote,
	                initial_marie_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(marie->lc));

	add_calls_to_local_conference(lcs, marie, NULL, participants, FALSE);

	stats *lcm_stats = NULL;

	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));

		// Append element
		lcm_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));
	lcm_stats[counter - 1] = marie->stat;

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	linphone_core_terminate_conference(marie->lc);

	LinphoneConferenceParams *new_maries_conference_params = linphone_conference_params_new(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference_params);
	linphone_conference_params_enable_local_participant(new_maries_conference_params, FALSE);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(new_maries_conference_params) == FALSE);
	LinphoneConference *new_maries_conference =
	    linphone_core_create_conference_with_params(marie->lc, new_maries_conference_params);
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference);
	linphone_conference_params_unref(new_maries_conference_params);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 2, 5000));

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, laure);
	new_participants = bctbx_list_append(new_participants, chloe);

	if (add_participants_immediately_after_creation == TRUE) {
		conference_with_calls_queued(marie, new_maries_conference, new_participants, FALSE, FALSE);
	}

	int idx = 0;

	participants = bctbx_list_append(participants, marie);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;

		unsigned int no_calls = 0;
		unsigned int no_conference = 0;
		if (m == marie) {
			if (add_participants_immediately_after_creation == TRUE) {
				no_calls = 4;
				no_conference = 2;
			} else {
				no_calls = 2;
				no_conference = 1;
			}
		} else {
			no_calls = 1;
			no_conference = 1;
		}

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
		                             lcm_stats[idx].number_of_LinphoneCallEnd + no_calls,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
		                             lcm_stats[idx].number_of_LinphoneCallReleased + no_calls,
		                             liblinphone_tester_sip_timeout));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateTerminationPending + no_conference,
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateTerminated + no_conference, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateDeleted + no_conference, 5000));

		bool_t event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
		if ((m != marie) && event_log_enabled) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionTerminated,
			                             lcm_stats[idx].number_of_LinphoneSubscriptionTerminated + no_conference,
			                             liblinphone_tester_sip_timeout));
		}

		LinphoneConference *conference = linphone_core_get_conference(c);

		BC_ASSERT_PTR_NULL(conference);

		if (m != marie) {
			LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, marie->identity);
			BC_ASSERT_PTR_NULL(participant_call);
			LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
			BC_ASSERT_PTR_NULL(conference_call);
		}

		idx++;
	}

	if (add_participants_immediately_after_creation == FALSE) {
		conference_with_calls_queued(marie, new_maries_conference, new_participants, FALSE, FALSE);
	}

	// Verify that a third conference is not created when adding calls
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreationPending, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminationPending, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminated, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateDeleted, 2, int, "%d");

	ms_free(lcm_stats);

	linphone_conference_unref(new_maries_conference);

	bctbx_list_free(lcs);
	bctbx_list_free(participants);
	bctbx_list_free(new_participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
}

static void interleaved_conference_creation(void) {
	interleaved_conferences_base(FALSE);
}

static void interleaved_conference_creation_with_quick_participant_addition(void) {
	interleaved_conferences_base(TRUE);
}

static void multiple_conferences_in_server_mode(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	linphone_core_enable_conference_server(marie->lc, TRUE);
	linphone_core_set_inc_timeout(marie->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(pauline->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);
	linphone_core_set_inc_timeout(laure->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *michelle = create_mgr_for_conference("michelle_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(michelle->lc, liblinphone_tester_sip_timeout);

	LinphoneCoreManager *chloe = create_mgr_for_conference("chloe_rc", TRUE, NULL);
	linphone_core_set_inc_timeout(chloe->lc, liblinphone_tester_sip_timeout);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, michelle);
	participants = bctbx_list_append(participants, pauline);

	bctbx_list_t *participants2 = NULL;
	participants2 = bctbx_list_append(participants2, laure);
	participants2 = bctbx_list_append(participants2, chloe);

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, marie->lc);

	bctbx_list_t *lcs1 = NULL;
	lcs1 = bctbx_list_append(lcs1, marie->lc);

	stats initial_marie_stat = marie->stat;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		lcs1 = bctbx_list_append(lcs1, c);
		lcs = bctbx_list_append(lcs, c);
		initiate_call(m, marie);
	}

	bctbx_list_t *lcs2 = NULL;
	lcs2 = bctbx_list_append(lcs2, marie->lc);

	for (bctbx_list_t *it = participants2; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		lcs2 = bctbx_list_append(lcs2, c);
		lcs = bctbx_list_append(lcs, c);
		initiate_call(m, marie);
	}

	// Let ring calls for a little while
	wait_for_list(lcs, NULL, 0, 1000);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,
	                initial_marie_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausing, initial_marie_stat.number_of_LinphoneCallPausing, int,
	                "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, initial_marie_stat.number_of_LinphoneCallPaused, int,
	                "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote,
	                initial_marie_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	for (bctbx_list_t *it = participants2; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(marie->lc));

	LinphoneConferenceParams *maries_conference_params = linphone_conference_params_new(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(maries_conference_params);
	linphone_conference_params_enable_local_participant(maries_conference_params, FALSE);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(maries_conference_params) == FALSE);
	LinphoneConference *maries_conference =
	    linphone_core_create_conference_with_params(marie->lc, maries_conference_params);
	BC_ASSERT_PTR_NOT_NULL(maries_conference);
	linphone_conference_params_unref(maries_conference_params);

	add_calls_to_local_conference(lcs1, marie, maries_conference, participants, TRUE);

	LinphoneConferenceParams *maries_conference_params2 = linphone_conference_params_new(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(maries_conference_params2);
	linphone_conference_params_enable_local_participant(maries_conference_params2, FALSE);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(maries_conference_params2) == FALSE);
	LinphoneConference *maries_conference2 =
	    linphone_core_create_conference_with_params(marie->lc, maries_conference_params2);
	BC_ASSERT_PTR_NOT_NULL(maries_conference2);
	linphone_conference_params_unref(maries_conference_params2);

	add_calls_to_local_conference(lcs2, marie, maries_conference2, participants2, FALSE);

	terminate_conference(participants2, marie, maries_conference2, NULL, FALSE);
	terminate_conference(participants, marie, maries_conference, NULL, FALSE);

	// Verify that a third conference is not created when adidng calls
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreationPending, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminationPending, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminated, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateDeleted, 2, int, "%d");

	linphone_conference_unref(maries_conference);
	linphone_conference_unref(maries_conference2);

	bctbx_list_free(lcs);
	bctbx_list_free(lcs1);
	bctbx_list_free(lcs2);
	bctbx_list_free(participants);
	bctbx_list_free(participants2);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);
}

static void conference_mix_created_by_merging_video_calls_base(LinphoneConferenceLayout layout, bool_t enable_ice) {
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LinphoneCoreManager *focus_mgr = (LinphoneCoreManager *)focus;
	linphone_core_enable_conference_server(focus_mgr->lc, TRUE);
	LinphoneProxyConfig *focus_proxy_config =
	    linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE, NULL);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *laure =
	    create_mgr_for_conference(liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE, NULL);

	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie->lc);
	linphone_proxy_config_edit(marie_proxy);
	linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
	linphone_proxy_config_done(marie_proxy);

	LinphoneConference *conf = NULL;
	LinphoneConferenceParams *conf_params = NULL;
	LinphoneCall *marie_call_laure = NULL;
	LinphoneCall *laure_called_by_marie = NULL;
	LinphoneCall *marie_call_pauline = NULL;
	LinphoneCall *pauline_called_by_marie = NULL;

	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	lcs = bctbx_list_append(lcs, focus_mgr->lc);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);
		linphone_core_set_video_display_filter(c, "MSAnalyseDisplay");

		linphone_core_enable_rtp_bundle(c, TRUE);

		linphone_core_set_default_conference_layout(c, layout);
		const LinphoneVideoActivationPolicy *cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == TRUE);

		if (enable_ice) {
			linphone_core_set_firewall_policy(c, LinphonePolicyUseIce);
		}
	}

	linphone_video_activation_policy_unref(pol);

	const LinphoneCallParams *negotiated_call_params = NULL;
	LinphoneCallParams *marie_call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *laure_call_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *pauline_call_params = linphone_core_create_call_params(pauline->lc, NULL);

	linphone_call_params_enable_video(marie_call_params, TRUE);
	linphone_call_params_set_conference_video_layout(marie_call_params, layout);
	linphone_call_params_enable_video(pauline_call_params, TRUE);
	linphone_call_params_enable_video(laure_call_params, TRUE);

	BC_ASSERT_TRUE(call_with_params(marie, laure, marie_call_params, laure_call_params));
	marie_call_laure = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_laure);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	laure_called_by_marie = linphone_core_get_current_call(laure->lc);
	negotiated_call_params = linphone_call_get_current_params(laure_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_call_params, pauline_call_params));
	marie_call_pauline = linphone_core_get_current_call(marie->lc);
	negotiated_call_params = linphone_call_get_current_params(marie_call_pauline);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	negotiated_call_params = linphone_call_get_current_params(pauline_called_by_marie);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));

	linphone_call_params_unref(laure_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(marie_call_params);

	conf_params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_video(conf_params, TRUE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *new_participants = NULL;
	new_participants = bctbx_list_append(new_participants, pauline);
	new_participants = bctbx_list_append(new_participants, laure);
	add_calls_to_remote_conference(lcs, focus_mgr, marie, new_participants, conf, FALSE);
	participants = bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NOT_NULL(conf);
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conf), 2, int, "%d");

	// Check that video capabilities are enabled in the conference
	const LinphoneConferenceParams *current_conf_params = linphone_conference_get_current_params(conf);
	BC_ASSERT_PTR_NOT_NULL(current_conf_params);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_conf_params) == TRUE);

	const LinphoneAddress *marie_conference_address = NULL;
	if (conf) {
		marie_conference_address = linphone_conference_get_conference_address(conf);
	}
	BC_ASSERT_PTR_NOT_NULL(marie_conference_address);

	int no_parts = (int)bctbx_list_size(participants);
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneConference *conference = linphone_core_search_conference(c, NULL, NULL, marie_conference_address, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			int part_counter = 0;
			bctbx_list_t *participant_device_list = NULL;
			do {
				if (participant_device_list) {
					bctbx_list_free_with_data(participant_device_list,
					                          (void (*)(void *))linphone_participant_device_unref);
				}
				participant_device_list = linphone_conference_get_participant_device_list(conference);
				part_counter++;
				wait_for_list(lcs, NULL, 0, 100);
			} while ((part_counter < 100) && (bctbx_list_size(participant_device_list) != (size_t)(no_parts + 1)));
			BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), (no_parts + 1), size_t, "%0zu");
			if (focus_mgr->lc == c) {
				BC_ASSERT_FALSE(linphone_conference_is_in(conference));
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), (no_parts + 1), int, "%d");

				for (bctbx_list_t *p_it = participant_device_list; p_it; p_it = bctbx_list_next(p_it)) {
					LinphoneParticipantDevice *p = (LinphoneParticipantDevice *)bctbx_list_get_data(p_it);
					BC_ASSERT_PTR_NOT_NULL(p);
					if (p) {
						BC_ASSERT_TRUE(_linphone_participant_device_get_layout(p) == layout);
					}
				}

				for (const bctbx_list_t *call_it = linphone_core_get_calls(c); call_it;
				     call_it = bctbx_list_next(call_it)) {
					LinphoneCall *fcall = (LinphoneCall *)bctbx_list_get_data(call_it);
					BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(fcall), conference);
					negotiated_call_params = linphone_call_get_current_params(fcall);
					BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
				}
			} else {
				BC_ASSERT_TRUE(linphone_conference_is_in(conference));
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), no_parts, int, "%d");

				LinphoneCall *conf_call = linphone_conference_get_call(conference);
				BC_ASSERT_PTR_NOT_NULL(conf_call);
				if (conf_call) {
					const LinphoneCallParams *call_local_params = linphone_call_get_params(conf_call);
					const LinphoneConferenceLayout m_layout =
					    linphone_call_params_get_conference_video_layout(call_local_params);
					BC_ASSERT_TRUE(m_layout == layout);
					negotiated_call_params = linphone_call_get_current_params(conf_call);
					BC_ASSERT_TRUE(linphone_call_params_video_enabled(negotiated_call_params));
				}
			}
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);

			const LinphoneConferenceParams *current_remote_conf_params =
			    linphone_conference_get_current_params(conference);
			BC_ASSERT_PTR_NOT_NULL(current_remote_conf_params);
			BC_ASSERT_TRUE(linphone_conference_params_video_enabled(current_remote_conf_params) == TRUE);
		}
	}

	wait_for_list(lcs, NULL, 0, 3000);
	/* In case of active speaker, mute other participants to ensure that the output video is from pauline. */
	linphone_core_enable_mic(marie->lc, FALSE);
	linphone_core_enable_mic(laure->lc, FALSE);
	check_video_conference(lcs, pauline, laure, layout);
	terminate_conference(participants, marie, conf, focus_mgr, FALSE);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));

	int marie_calls = (int)bctbx_list_size(linphone_core_get_calls(marie->lc));
	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, marie_calls, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, marie_calls, liblinphone_tester_sip_timeout));

	if (conf) {
		linphone_conference_unref(conf);
	}
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(marie);
	linphone_conference_server_destroy(focus);
	bctbx_list_free(lcs);
	if (participants) {
		bctbx_list_free(participants);
	}
}

static void video_conference_created_by_merging_video_calls_with_grid_layout_2(void) {
	conference_mix_created_by_merging_video_calls_base(LinphoneConferenceLayoutGrid, TRUE);
}

static void video_conference_created_by_merging_video_calls_with_active_speaker_layout_2(void) {
	conference_mix_created_by_merging_video_calls_base(LinphoneConferenceLayoutActiveSpeaker, FALSE);
}

static test_t audio_video_conference_basic_tests[] = {
    TEST_NO_TAG("Simple conference", simple_conference), TEST_NO_TAG("Simple CCMP conference", simple_ccmp_conference),
    TEST_NO_TAG("Simple CCMP conference retrieval", simple_ccmp_conference_retrieval),
    TEST_NO_TAG("Simple conference notify speaking device", simple_conference_notify_speaking_device),
    TEST_NO_TAG("Simple conference notify muted device", simple_conference_notify_muted_device),
    TEST_NO_TAG("Simple conference established before proxy config is created",
                simple_conference_established_before_proxy_config_creation),
    TEST_NO_TAG("Simple conference with local participant with no event log",
                simple_conference_with_local_participant_with_no_event_log),
    TEST_NO_TAG("Simple conference with remote participant with no event log",
                simple_conference_with_remote_participant_with_no_event_log),
    TEST_NO_TAG("Simple conference with admin changed", simple_conference_with_admin_changed),
    TEST_NO_TAG("Simple conference with participant removal from non admin",
                simple_conference_with_participant_removal_from_non_admin),
    TEST_NO_TAG("Simple conference with participant addition from non admin",
                simple_conference_with_participant_addition_from_non_admin),
    TEST_NO_TAG("Simple conference with subject change from non admin",
                simple_conference_with_subject_change_from_non_admin),
    TEST_NO_TAG("Simple conference with subject change from admin", simple_conference_with_subject_change_from_admin),
    TEST_NO_TAG("Simple conference with one participant with local", simple_conference_with_one_participant_local),
    TEST_NO_TAG("Simple conference with one participant without local",
                simple_conference_with_one_participant_no_local),
    TEST_NO_TAG("Simple video conference with subject change from admin",
                simple_video_conference_with_subject_change_from_admin),
    TEST_NO_TAG("Simple conference with one participant", simple_conference_with_one_participant),
    TEST_NO_TAG("Simple conference with active speaker layout", simple_conference_with_active_speaker_layout),
    TEST_NO_TAG("Simple conference with grid layout", simple_conference_with_grid_layout),
    TEST_NO_TAG("Simple conference established from scratch", simple_conference_from_scratch),
    TEST_NO_TAG("Simple conference with 2 devices with same address", simple_conference_two_devices_same_address),
    TEST_NO_TAG("Simple 4 participant conference ended by terminating conference",
                simple_4_participants_conference_ended_by_terminating_conference),
    TEST_NO_TAG("Simple 4 participant conference ended by terminating all calls",
                simple_4_participants_conference_ended_by_terminating_calls),
    //	TEST_NO_TAG("Simple conference with multi device", simple_conference_with_multi_device),
    TEST_NO_TAG("Simple conference established from scratch, but attendees do not answer",
                simple_conference_from_scratch_no_answer),
    TEST_NO_TAG("Simple conference with no conversion to call", simple_conference_not_converted_to_call),
    TEST_NO_TAG("Simple conference with file player", simple_conference_with_file_player),
    TEST_NO_TAG("Simple conference with file player where participants leave",
                simple_conference_with_file_player_participants_leave)};

static test_t audio_video_conference_basic2_tests[] = {
    TEST_NO_TAG("Simple CCMP conference with conference update", simple_ccmp_conference_with_conference_update),
    TEST_NO_TAG("Simple CCMP conference with conference update and cancel",
                simple_ccmp_conference_with_conference_update_cancel),
    TEST_NO_TAG("Eject from 4 participants conference (call terminated one by one)",
                eject_from_4_participants_local_conference_call_terminated_one_by_one),
    TEST_NO_TAG("Conference without conference event package pauses and terminate call",
                conference_without_event_pkg_hang_up_call_on_hold),
    TEST_NO_TAG("Conference with conference event package pauses and terminate call",
                conference_with_event_pkg_hang_up_call_on_hold),
    TEST_NO_TAG("Register again during conference", register_again_during_conference),
    TEST_NO_TAG("Back to back conferences", back_to_back_conferences),
    TEST_NO_TAG("Back to back conferences with same core", back_to_back_conferences_same_core),
    TEST_NO_TAG("Try to create second conference with local participant",
                try_to_create_second_conference_with_local_participant),
    TEST_NO_TAG("Interleaved conference creation", interleaved_conference_creation),
    TEST_NO_TAG("Interleaved conference creation with participant added before the first one ends",
                interleaved_conference_creation_with_quick_participant_addition),
    TEST_NO_TAG("Multiple conferences in server mode", multiple_conferences_in_server_mode),
    TEST_NO_TAG("Conference with calls queued without ICE", conference_with_calls_queued_without_ice),
    TEST_NO_TAG("Conference with back to back call accept without ICE",
                conference_with_back_to_back_call_accept_without_ice),
    TEST_NO_TAG("Conference with back to back call invite and accept without ICE",
                conference_with_back_to_back_call_invite_accept_without_ice),
    TEST_NO_TAG("Simple client conference", simple_remote_conference),
    TEST_NO_TAG("Simple client conference with shut down focus", simple_remote_conference_shut_down_focus),
    TEST_NO_TAG("Eject from 3 participants in client conference", eject_from_3_participants_remote_conference)};

static test_t audio_conference_tests[] = {
    TEST_NO_TAG("Audio conference by merging video calls", audio_conference_created_by_merging_video_calls),
    TEST_NO_TAG("Audio calls added to video conference", audio_calls_added_to_video_conference),
    TEST_NO_TAG("Participants exit conference after pausing", participants_exit_conference_after_pausing),
    TEST_NO_TAG("Add participant after conference started", add_participant_after_conference_started),
    TEST_NO_TAG("Add paused calls to conference", add_paused_calls_to_conference),
    TEST_NO_TAG("Conference with last call paused", conference_with_last_call_paused),
    TEST_NO_TAG("Add all calls to conference", add_all_calls_to_conference),
    TEST_NO_TAG("Add not accepted calls made by local participant to conference",
                add_call_not_accepted_to_conference_local),
    TEST_NO_TAG("Add not accepted calls to local participant to conference",
                add_call_not_accepted_to_conference_remote),
    TEST_NO_TAG("Remove participant from video conference with active speaker layout",
                remove_participant_from_video_conference_active_speaker_layout),
    TEST_NO_TAG("Remove participant from video conference with grid layout",
                remove_participant_from_video_conference_grid_layout),
    TEST_NO_TAG("Focus takes quick call after conference started and then resumes it",
                focus_takes_quick_call_after_conference_started),
    TEST_NO_TAG("Focus takes call after conference started and participants leave",
                focus_takes_call_after_conference_started_and_participants_leave),
    TEST_NO_TAG("Participant quits conference and is called by focus",
                participant_quits_conference_and_is_called_by_focus),
    TEST_NO_TAG("Participant takes call after conference started and conference ends",
                remote_participant_takes_call_after_conference_started_and_conference_ends),
    TEST_NO_TAG("Participant takes call after conference started and rejoins conference",
                participant_takes_call_after_conference_started_and_rejoins_conference),
    TEST_NO_TAG("Participants take call after conference started and rejoins conference",
                participants_take_call_after_conference_started_and_rejoins_conference),
    TEST_NO_TAG("Participant takes call after conference started and rejoins conference after conference ended",
                participant_takes_call_after_conference_started_and_rejoins_conference_after_conference_ended),
    TEST_NO_TAG("Everybody leave conference and local enters first",
                everybody_temporarely_leave_conference_and_local_enters_first),
    TEST_NO_TAG("Everybody leave conference and local enters last",
                everybody_temporarely_leave_conference_and_local_enters_last)};

static test_t audio_conference_local_participant_tests[] = {
    TEST_NO_TAG("Simple local participant leaves conference", simple_local_participant_leaves_conference),
    TEST_NO_TAG("Local participant takes call after conference started and conference ends",
                local_participant_takes_call_after_conference_started_and_conference_ends),
    TEST_NO_TAG("Local participant leaves conference and add participant",
                local_participant_leaves_conference_and_add_participant),
    TEST_NO_TAG("Local participant leaves conference and call to focus",
                local_participant_leaves_conference_and_call_to_focus),
    TEST_NO_TAG("Local participant call terminating after leaving conference",
                local_participant_call_terminated_after_leaving_conference),
    TEST_NO_TAG("Quick local participant leaves conference and add participant",
                quick_local_participant_leaves_conference_and_add_participant),
    TEST_NO_TAG("Quick local participant leaves conference and call to focus",
                quick_local_participant_leaves_conference_and_call_to_focus)};

static test_t audio_conference_remote_participant_tests[] = {
    TEST_NO_TAG("Simple remote participant leaves conference", simple_remote_participant_leaves_conference),
    TEST_NO_TAG("Remote participant leaves conference and add participant",
                remote_participant_leaves_conference_and_add_participant),
    TEST_NO_TAG("Remote participant leaves conference and call to focus",
                remote_participant_leaves_conference_and_call_to_focus),
    TEST_NO_TAG("Remote participant leaves conference and other remote terminates call",
                remote_participant_leaves_and_other_remote_terminate_call),
    TEST_NO_TAG("Remote participant leaves conference and local ends conference",
                remote_participant_leaves_and_local_ends_conference),
    TEST_NO_TAG("Remote participant call terminating after leaving conference",
                remote_participant_call_terminated_after_leaving_conference),
    TEST_NO_TAG("Remote participant adds video during conference", remote_participant_adds_video_during_conference),
    TEST_NO_TAG("Quick remote participant leaves conference and add participant",
                quick_remote_participant_leaves_conference_and_add_participant),
    TEST_NO_TAG("Quick remote participant leaves conference and call to focus",
                quick_remote_participant_leaves_conference_and_call_to_focus)};

static test_t video_conference_tests[] = {
    TEST_NO_TAG("Simple conference established from scratch with video", simple_conference_from_scratch_with_video),
    TEST_NO_TAG("Audio calls initiated by host added to video conference",
                audio_calls_initiated_by_host_added_to_video_conference),
    TEST_NO_TAG("Audio calls with video rejected added to video conference",
                audio_calls_with_video_rejected_added_to_video_conference),
    TEST_NO_TAG("Video conference by merging calls", video_conference_by_merging_calls),
    TEST_NO_TAG(
        "Video conference by merging video calls without conference event package terminated by participants",
        video_conference_created_by_merging_video_calls_without_conference_event_package_terminated_by_participants),
    TEST_NO_TAG("Video conference by merging video calls without conference event package terminated by server",
                video_conference_created_by_merging_video_calls_without_conference_event_package_terminated_by_server),
    TEST_NO_TAG("One participant video conference by merging video calls without conference event package",
                one_participant_video_conference_created_by_merging_video_calls_without_conference_event_package),
    TEST_NO_TAG("Try to update call parameter during conference", try_to_update_call_params_during_conference),
    //	TEST_NO_TAG("Update conference parameter during conference", update_conf_params_during_conference),
    TEST_NO_TAG("Toggle video settings during conference without automatically accept video policy",
                toggle_video_settings_during_conference_without_automatically_accept_video_policy),
    TEST_NO_TAG("Video conference with no conference version", video_conference_with_no_conference_version),
    TEST_NO_TAG("Toggle video settings during conference with automatically accept video policy",
                toggle_video_settings_during_conference_with_automatically_accept_video_policy),
    TEST_NO_TAG("Toggle video settings during conference with update deferred",
                toggle_video_settings_during_conference_with_update_deferred),
    //	TEST_NO_TAG("Enable video during conference and take another call",
    // enable_video_during_conference_and_take_another_call),
    TEST_NO_TAG("Simultaneous toggle of video settings during conference",
                simultaneous_toggle_video_settings_during_conference)};

static test_t video_conference_layout_tests[] = {
    TEST_NO_TAG("Simple SRTP conference with active speaker layout", simple_srtp_conference_with_active_speaker_layout),
    TEST_NO_TAG("Simple SRTP conference with grid layout", simple_srtp_conference_with_grid_layout),
    TEST_NO_TAG("Simple conference with layout change of local participant",
                simple_conference_with_layout_change_local_participant),
    TEST_NO_TAG("Simple conference with layout change of remote participant",
                simple_conference_with_layout_change_remote_participant),
    TEST_NO_TAG("Simple conference with layout change of remote participant without conference params update",
                simple_conference_with_layout_change_remote_participant_without_conference_params_update),
    TEST_NO_TAG("Add participant after layout change", add_participant_after_layout_change),
    TEST_NO_TAG("Video conference by merging video calls with grid layout",
                video_conference_created_by_merging_video_calls_with_grid_layout),
    TEST_NO_TAG("Video conference by merging video calls with active speaker layout",
                video_conference_created_by_merging_video_calls_with_active_speaker_layout),
    TEST_NO_TAG("Video conference by merging video calls with active speaker layout 2",
                video_conference_created_by_merging_video_calls_with_active_speaker_layout_2),
    TEST_NO_TAG("Eject from 3 participants conference with grid layout",
                eject_from_3_participants_local_conference_grid_layout),
    TEST_NO_TAG("Eject from 4 participants conference with grid layout",
                eject_from_4_participants_conference_grid_layout),
    TEST_NO_TAG("Eject from 3 participants conference with active speaker layout",
                eject_from_3_participants_local_conference_active_speaker_layout),
    TEST_NO_TAG("Eject from 4 participants conference with active speaker layout",
                eject_from_4_participants_conference_active_speaker_layout)};

static test_t ice_conference_tests[] = {
    TEST_ONE_TAG("Simple conference with ICE", simple_conference_with_ice, "ICE"),
    TEST_ONE_TAG("Simple ZRTP conference with ICE", simple_zrtp_conference_with_ice, "ICE"),
    TEST_ONE_TAG("Simple ICE client conference", simple_ice_remote_conference, "ICE"),
    TEST_ONE_TAG("Conference with calls queued with ICE", conference_with_calls_queued_with_ice, "ICE"),
    TEST_ONE_TAG(
        "Conference with back to back call accept with ICE", conference_with_back_to_back_call_accept_with_ice, "ICE"),
    //	TEST_ONE_TAG("Conference with back to back call invite and accept with ICE",
    // conference_with_back_to_back_call_invite_accept_with_ice, "ICE"),

    TEST_ONE_TAG("Video conference by merging video calls with grid layout 2",
                 video_conference_created_by_merging_video_calls_with_grid_layout_2,
                 "ICE"),
    TEST_ONE_TAG("ICE video conference by merging video calls with grid layout",
                 ice_video_conference_created_by_merging_video_calls_with_grid_layout,
                 "ICE"),
    TEST_ONE_TAG("One participant ICE video conference with grid layout",
                 ice_video_conference_one_participant_grid_layout,
                 "ICE"),
    TEST_ONE_TAG("One participant ICE video conference with active speaker layout",
                 ice_video_conference_one_participant_active_speaker_layout,
                 "ICE")};

test_suite_t audio_video_conference_basic_test_suite = {"Audio video conference (Basic)",
                                                        NULL,
                                                        NULL,
                                                        liblinphone_tester_before_each,
                                                        liblinphone_tester_after_each,
                                                        sizeof(audio_video_conference_basic_tests) /
                                                            sizeof(audio_video_conference_basic_tests[0]),
                                                        audio_video_conference_basic_tests,
                                                        0,
                                                        4};

test_suite_t audio_video_conference_basic2_test_suite = {"Audio video conference 2 (Basic)",
                                                         NULL,
                                                         NULL,
                                                         liblinphone_tester_before_each,
                                                         liblinphone_tester_after_each,
                                                         sizeof(audio_video_conference_basic2_tests) /
                                                             sizeof(audio_video_conference_basic2_tests[0]),
                                                         audio_video_conference_basic2_tests,
                                                         0,
                                                         4};

test_suite_t audio_conference_test_suite = {"Audio conference",
                                            NULL,
                                            NULL,
                                            liblinphone_tester_before_each,
                                            liblinphone_tester_after_each,
                                            sizeof(audio_conference_tests) / sizeof(audio_conference_tests[0]),
                                            audio_conference_tests,
                                            0,
                                            2};

test_suite_t audio_conference_local_participant_test_suite = {"Audio conference (Local participant)",
                                                              NULL,
                                                              NULL,
                                                              liblinphone_tester_before_each,
                                                              liblinphone_tester_after_each,
                                                              sizeof(audio_conference_local_participant_tests) /
                                                                  sizeof(audio_conference_local_participant_tests[0]),
                                                              audio_conference_local_participant_tests,
                                                              0,
                                                              0};

test_suite_t audio_conference_remote_participant_test_suite = {"Audio conference (Remote participant)",
                                                               NULL,
                                                               NULL,
                                                               liblinphone_tester_before_each,
                                                               liblinphone_tester_after_each,
                                                               sizeof(audio_conference_remote_participant_tests) /
                                                                   sizeof(audio_conference_remote_participant_tests[0]),
                                                               audio_conference_remote_participant_tests,
                                                               0,
                                                               0};

test_suite_t video_conference_test_suite = {"Video conference",
                                            NULL,
                                            NULL,
                                            liblinphone_tester_before_each,
                                            liblinphone_tester_after_each,
                                            sizeof(video_conference_tests) / sizeof(video_conference_tests[0]),
                                            video_conference_tests,
                                            0,
                                            4};

test_suite_t video_conference_layout_test_suite = {"Video conference (Layout)",
                                                   NULL,
                                                   NULL,
                                                   liblinphone_tester_before_each,
                                                   liblinphone_tester_after_each,
                                                   sizeof(video_conference_layout_tests) /
                                                       sizeof(video_conference_layout_tests[0]),
                                                   video_conference_layout_tests,
                                                   0,
                                                   4};

test_suite_t ice_conference_test_suite = {"ICE conference",
                                          NULL,
                                          NULL,
                                          liblinphone_tester_before_each,
                                          liblinphone_tester_after_each,
                                          sizeof(ice_conference_tests) / sizeof(ice_conference_tests[0]),
                                          ice_conference_tests,
                                          0,
                                          4};
