/*
 * copyright (c) 2010-2024 belledonne communications sarl.
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
#include "conference/conference-info.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "local-conference-tester-functions.h"

namespace LinphoneTest {

void on_muted_notified(LinphoneParticipantDevice *participant_device, bool_t is_muted) {
	stats *stat = get_stats((LinphoneCore *)linphone_participant_device_get_user_data(participant_device));
	if (is_muted) {
		stat->number_of_LinphoneParticipantDeviceMuted++;
	} else {
		stat->number_of_LinphoneParticipantDeviceUnmuted++;
	}
}

void create_transfer_conference_base(time_t start_time,
                                     int duration,
                                     LinphoneConferenceParticipantListType participant_list_type,
                                     const std::vector<LinphoneMediaEncryption> encryption,
                                     LinphoneConferenceLayout layout,
                                     LinphoneConferenceSecurityLevel security_level,
                                     bool audio_transfer,
                                     bool video_transfer) {
	char *pauline_recordpath = bc_tester_file("record-local_transferred_conference_with_file_player_pauline.wav");
	char *laure_recordpath = bc_tester_file("record-local_transferred_conference_with_file_player_laure.wav");
	char *marie_recordpath = bc_tester_file("record-local_transferred_conference_with_file_player_marie.wav");
	char *pauline_soundpath = bc_tester_res("sounds/vrroom.wav");
	char *laure_soundpath = bc_tester_res("sounds/hello8000.wav");
	char *marie_soundpath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	unlink(pauline_recordpath);
	unlink(laure_recordpath);
	unlink(marie_recordpath);

	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), enable_lime);

		// Record files, each participant gets a different input file so we can check manually the mix is actually
		// performed
		linphone_core_set_record_file(marie.getLc(), marie_recordpath);
		linphone_core_set_record_file(pauline.getLc(), pauline_recordpath);
		linphone_core_set_record_file(laure.getLc(), laure_recordpath);
		linphone_core_set_play_file(marie.getLc(), marie_soundpath);
		linphone_core_set_play_file(pauline.getLc(), pauline_soundpath);
		linphone_core_set_play_file(laure.getLc(), laure_soundpath);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		auto encryptionIndex = 0;

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		for (auto mgr : conferenceMgrs) {
			if (video_transfer) {
				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(mgr->lc, pol);
				linphone_video_activation_policy_unref(pol);
				disable_all_video_codecs_except_one(mgr->lc, "vp8");

				linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
				linphone_core_enable_video_capture(mgr->lc, TRUE);
				linphone_core_enable_video_display(mgr->lc, TRUE);

				if (layout == LinphoneConferenceLayoutGrid) {
					linphone_core_set_preferred_video_definition_by_name(mgr->lc, "720p");
					linphone_config_set_string(linphone_core_get_config(mgr->lc), "video", "max_conference_size",
					                           "vga");
				}

				linphone_core_set_conference_max_thumbnails(mgr->lc, 2);
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
				linphone_core_set_media_encryption(mgr->lc, encryption[encryptionIndex]);
				encryptionIndex++;
			}

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		// Focus is in full packet mode: transfer packet not payload
		LinphoneConfig *focus_config = linphone_core_get_config(focus.getLc());

		if (audio_transfer) {
			linphone_config_set_int(focus_config, "sound", "conference_mode",
			                        static_cast<int>(MSConferenceModeRouterFullPacket));
		}

		if (video_transfer) {
			linphone_config_set_int(focus_config, "video", "conference_mode",
			                        static_cast<int>(MSConferenceModeRouterFullPacket));
		}

		int nortp_timeout = 15;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(
		    std::make_pair(marie.getCMgr(), add_participant_info_to_list(&participants_info, marie.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, video_transfer, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		// Each member call the conference
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
		}

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

		// Check everyone is connected to the conference
		encryptionIndex = 0;
		for (auto mgr : members) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running,
			                             liblinphone_tester_sip_timeout));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             ((mgr == marie.getCMgr()) ? 2 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption[encryptionIndex], int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption[encryptionIndex], int, "%d");
			}
			encryptionIndex++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 6;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + (3 * nb_subscriptions),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + (3 * nb_subscriptions), 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, video_transfer);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait to know if the no RTP timeout is triggered
		CoreManagerAssert({focus, marie, pauline, laure})
		    .waitUntil(chrono::seconds(nortp_timeout + 1), [&marie, confAddr, nortp_timeout] {
			    LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
			    return marie_call && (linphone_call_get_duration(marie_call) > nortp_timeout);
		    });

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time, long long, "%lld");

				BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
				                (int)security_level, int, "%0d");

				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), members.size(), size_t, "%zu");
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);

				if (mgr == focus.getCMgr()) {
					no_participants = static_cast<int>(members.size());
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = static_cast<int>(members.size() - 1);
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					size_t no_streams_audio = 0;
					size_t no_streams_video = video_transfer ? 3 : 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						no_streams_video = compute_no_video_streams(video_transfer, pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
							auto *callStats = linphone_call_get_stats(pcall, LinphoneStreamTypeAudio);
							auto *srtpInfo = linphone_call_stats_get_srtp_info(
							    callStats, TRUE); // TRUE to get inner encryption stats
							BC_ASSERT_TRUE(srtpInfo->send_source == MSSrtpKeySourceEKT);
							BC_ASSERT_TRUE(srtpInfo->recv_source == MSSrtpKeySourceEKT);
							linphone_call_stats_unref(callStats);
						}
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		// Restart marie audiostream
		LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		linphone_call_restart_main_audio_stream(marie_call);
		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [] { return false; });

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 3 : 2), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		// Check to verify that muting is correctly notified
		LinphoneConference *laure_conf = linphone_call_get_conference(linphone_core_get_current_call(laure.getLc()));
		if (BC_ASSERT_PTR_NOT_NULL(laure_conf)) {
			bctbx_list_t *participant_devices = linphone_conference_get_participant_device_list(laure_conf);

			for (bctbx_list_t *it = participant_devices; it != NULL; it = it->next) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it->data;
				linphone_participant_device_set_user_data(d, laure.getLc());
				LinphoneParticipantDeviceCbs *cbs =
				    linphone_factory_create_participant_device_cbs(linphone_factory_get());
				linphone_participant_device_cbs_set_is_muted(cbs, on_muted_notified);
				linphone_participant_device_add_callbacks(d, cbs);
				linphone_participant_device_cbs_unref(cbs);
			}
			bctbx_list_free_with_data(participant_devices, (bctbx_list_free_func)linphone_participant_device_unref);

			linphone_core_enable_mic(pauline.getLc(), FALSE);
			CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [&laure] {
				return laure.getCMgr()->stat.number_of_LinphoneParticipantDeviceMuted == 1;
			});

			linphone_core_enable_mic(pauline.getLc(), TRUE);
			CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [&laure] {
				return laure.getCMgr()->stat.number_of_LinphoneParticipantDeviceUnmuted == 1;
			});

			linphone_core_enable_mic(laure.getLc(), FALSE);
			CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [&laure] {
				return laure.getCMgr()->stat.number_of_LinphoneParticipantDeviceMuted == 2;
			});
		}

		// terminate all calls
		for (auto mgr : members) {
			const bctbx_list_t *calls = linphone_core_get_calls(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		check_delete_focus_conference_info({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), confAddr,
		                                   end_time);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		std::list<LinphoneCoreManager *> allMembers{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};
		for (auto mgr : allMembers) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		bctbx_list_free(coresList);
	}

	bc_free(pauline_recordpath);
	bc_free(laure_recordpath);
	bc_free(marie_recordpath);
	bc_free(pauline_soundpath);
	bc_free(laure_soundpath);
	bc_free(marie_soundpath);
}

void create_transfer_conference_base(time_t start_time,
                                     int duration,
                                     LinphoneConferenceParticipantListType participant_list_type,
                                     const LinphoneMediaEncryption encryption,
                                     LinphoneConferenceLayout layout,
                                     LinphoneConferenceSecurityLevel security_level,
                                     bool audio_transfer,
                                     bool video_transfer) {
	create_transfer_conference_base(start_time, duration, participant_list_type, {encryption, encryption, encryption},
	                                layout, security_level, audio_transfer, video_transfer);
}

static void create_audio_transfer_conference(void) {
	create_transfer_conference_base(ms_time(NULL), -1, LinphoneConferenceParticipantListTypeOpen,
	                                LinphoneMediaEncryptionNone, LinphoneConferenceLayoutGrid,
	                                LinphoneConferenceSecurityLevelNone, true, false);
}

static void create_video_transfer_conference(void) {
	create_transfer_conference_base(ms_time(NULL), -1, LinphoneConferenceParticipantListTypeOpen,
	                                LinphoneMediaEncryptionNone, LinphoneConferenceLayoutActiveSpeaker,
	                                LinphoneConferenceSecurityLevelNone, true, true);
}

#ifdef HAVE_EKT_SERVER_PLUGIN
static void create_audio_video_encrypted_conference_multi_encrypt(void) {
	create_transfer_conference_base(
	    ms_time(NULL), -1, LinphoneConferenceParticipantListTypeOpen,
	    {LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionDTLS},
	    LinphoneConferenceLayoutActiveSpeaker, LinphoneConferenceSecurityLevelEndToEnd, true, true);
}

static void create_audio_video_encrypted_conference(void) {
	create_transfer_conference_base(ms_time(NULL), -1, LinphoneConferenceParticipantListTypeOpen,
	                                LinphoneMediaEncryptionSRTP, LinphoneConferenceLayoutActiveSpeaker,
	                                LinphoneConferenceSecurityLevelEndToEnd, true, true);
}
#endif // HAVE_EKT_SERVER_PLUGIN

static void create_video_transfer_conference_active_speaker_changed(void) {
	change_active_speaker_base(true);
}

static void conference_joined_in_early_media(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), FALSE);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), FALSE);

		// A B2BUA doesn't have a factory address
		LinphoneAccount *account = linphone_core_get_default_account(focus.getLc());
		LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(account));
		linphone_account_params_set_conference_factory_address(account_params, NULL);
		linphone_account_set_params(account, account_params);
		linphone_account_params_unref(account_params);

		char *marie_filename = liblinphone_tester_make_unique_file_path("marie_record", "wav");
		char *marie_recordpath = bc_tester_file(marie_filename);
		bc_free(marie_filename);
		linphone_core_set_record_file(marie.getLc(), marie_recordpath);
		linphone_core_set_play_file(marie.getLc(), NULL);
		linphone_core_set_use_files(marie.getLc(), TRUE);

		char *pauline_soundpath = bc_tester_res("sounds/hello8000_mkv_ref.wav");
		char *pauline_filename = liblinphone_tester_make_unique_file_path("pauline_record", "wav");
		char *pauline_recordpath = bc_tester_file(pauline_filename);
		bc_free(pauline_filename);
		linphone_core_set_record_file(pauline.getLc(), pauline_recordpath);
		linphone_core_set_play_file(pauline.getLc(), NULL);
		linphone_core_set_use_files(pauline.getLc(), TRUE);

		linphone_core_set_record_file(focus.getLc(), NULL);
		linphone_core_set_play_file(focus.getLc(), NULL);
		linphone_core_set_use_files(focus.getLc(), TRUE);

		linphone_core_set_media_resource_mode(focus.getLc(), LinphoneSharedMediaResources);
		LinphoneConfig *focus_config = linphone_core_get_config(focus.getLc());
		linphone_config_set_int(focus_config, "sound", "conference_mode",
		                        static_cast<int>(MSConferenceModeRouterFullPacket));

		bctbx_list_t *coresList = NULL;
		coresList = bctbx_list_append(coresList, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie calls the focus
		LinphoneCallParams *marie_params = linphone_core_create_call_params(marie.getLc(), nullptr);
		linphone_call_params_enable_video(marie_params, FALSE);
		linphone_call_params_enable_early_media_sending(marie_params, TRUE);
		ms_message("%s is calling %s", linphone_core_get_identity(marie.getLc()),
		           linphone_core_get_identity(focus.getLc()));
		linphone_core_invite_address_with_params_2(marie.getLc(), focus.getIdentity().toC(), marie_params, NULL,
		                                           nullptr);
		linphone_call_params_unref(marie_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
		                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));

		// The focus relays the call to Pauline
		LinphoneCallParams *focus_params = linphone_core_create_call_params(focus.getLc(), nullptr);
		linphone_call_params_enable_early_media_sending(focus_params, TRUE);
		linphone_call_params_enable_video(focus_params, FALSE);
		ms_message("%s is calling %s", linphone_core_get_identity(focus.getLc()),
		           linphone_core_get_identity(pauline.getLc()));
		linphone_core_invite_address_with_params_2(focus.getLc(), pauline.getIdentity().toC(), focus_params, NULL,
		                                           nullptr);
		linphone_call_params_unref(focus_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallIncomingReceived,
		                             pauline_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));

		// The focus creates a conference that is hidden to the participants and adds the calls
		LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(focus.getLc(), NULL);
		const char *initialSubject = "B2BUA test conference";
		linphone_conference_params_enable_local_participant(conf_params, FALSE);
		linphone_conference_params_enable_one_participant_conference(conf_params, TRUE);
		linphone_conference_params_set_subject(conf_params, initialSubject);
		linphone_conference_params_set_hidden(conf_params, TRUE);
		LinphoneConference *conference = linphone_core_create_conference_with_params(focus.getLc(), conf_params);
		linphone_conference_params_unref(conf_params);

		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 2, size_t, "%zu");
		for (const bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
			LinphoneCall *call = (LinphoneCall *)it->data;
			linphone_conference_add_participant(conference, call);
		}

		// Pauline accepts the early media
		LinphoneCall *pauline_call =
		    linphone_core_get_call_by_remote_address2(pauline.getLc(), focus.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(pauline_call);
		if (pauline_call) {
			LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline.getLc(), nullptr);
			linphone_call_params_enable_video(pauline_params, FALSE);
			linphone_call_params_enable_early_media_sending(pauline_params, TRUE);
			ms_message("%s accepts early media in call to %s", linphone_core_get_identity(pauline.getLc()),
			           linphone_core_get_identity(focus.getLc()));
			linphone_call_accept_early_media_with_params(pauline_call, pauline_params);
			linphone_call_params_unref(pauline_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallIncomingEarlyMedia,
		                             pauline_stat.number_of_LinphoneCallIncomingEarlyMedia + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingEarlyMedia,
		                             focus_stat.number_of_LinphoneCallOutgoingEarlyMedia + 1,
		                             liblinphone_tester_sip_timeout));

		// The focus relays the early media to Marie
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), marie.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		std::string oldFocusMarieOpCallId;
		if (focus_call) {
			SalOp *op = linphone_call_get_op(focus_call);
			oldFocusMarieOpCallId = op->getCallId();
			BC_ASSERT_FALSE(oldFocusMarieOpCallId.empty());
			const char *logCallId = linphone_call_log_get_call_id(linphone_call_get_call_log(focus_call));
			BC_ASSERT_PTR_NOT_NULL(logCallId);
			BC_ASSERT_STRING_EQUAL(oldFocusMarieOpCallId.c_str(), logCallId);
			LinphoneCallParams *focus_early_media_params = linphone_core_create_call_params(focus.getLc(), nullptr);
			linphone_call_params_enable_video(focus_early_media_params, FALSE);
			linphone_call_params_enable_early_media_sending(focus_early_media_params, TRUE);
			ms_message("%s accepts early media in call to %s", linphone_core_get_identity(focus.getLc()),
			           linphone_core_get_identity(marie.getLc()));
			linphone_call_accept_early_media_with_params(focus_call, focus_early_media_params);
			linphone_call_params_unref(focus_early_media_params);
		}

		LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(marie_call);
		std::string oldMarieFocusOpCallId;
		if (marie_call) {
			SalOp *op = linphone_call_get_op(marie_call);
			oldMarieFocusOpCallId = op->getCallId();
			BC_ASSERT_FALSE(oldMarieFocusOpCallId.empty());
			const char *logCallId = linphone_call_log_get_call_id(linphone_call_get_call_log(marie_call));
			BC_ASSERT_PTR_NOT_NULL(logCallId);
			BC_ASSERT_STRING_EQUAL(oldMarieFocusOpCallId.c_str(), logCallId);
		}
		BC_ASSERT_STRING_EQUAL(oldMarieFocusOpCallId.c_str(), oldFocusMarieOpCallId.c_str());

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingEarlyMedia,
		                             focus_stat.number_of_LinphoneCallIncomingEarlyMedia + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingEarlyMedia,
		                             marie_stat.number_of_LinphoneCallOutgoingEarlyMedia + 1,
		                             liblinphone_tester_sip_timeout));

		// Search again the call as it may have been terminated
		pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), focus.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(pauline_call);
		// Pauline plays a file
		if (pauline_call) {
			LinphonePlayer *player = linphone_call_get_player(pauline_call);
			BC_ASSERT_PTR_NOT_NULL(player);
			if (player) {
				LinphonePlayerCbs *cbs = linphone_factory_create_player_cbs(linphone_factory_get());
				linphone_player_cbs_set_eof_reached(cbs, on_player_eof);
				linphone_player_cbs_set_user_data(cbs, pauline.getCMgr());
				linphone_player_add_callbacks(player, cbs);
				linphone_player_cbs_unref(cbs);
				BC_ASSERT_EQUAL(linphone_player_open(player, pauline_soundpath), 0, int, "%d");
				BC_ASSERT_EQUAL(linphone_player_start(player), 0, int, "%d");
			}
		}

		// Marie should register the call into a file and the latter will be compared to the file played by Pauline
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_player_eof,
		                             pauline_stat.number_of_player_eof + 1, 25000));
		/*wait for one second longer so that last RTP packets can arrive*/
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(1), [] { return false; });

		ms_message("%s's network becomes instable. The network suddenly drops and comes back up immediately",
		           linphone_core_get_identity(marie.getLc()));
		linphone_core_set_network_reachable(marie.getLc(), FALSE);
		linphone_core_set_network_reachable(marie.getLc(), TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingEarlyMedia,
		                             marie_stat.number_of_LinphoneCallOutgoingEarlyMedia + 2,
		                             liblinphone_tester_sip_timeout));

		// Pauline accepts the call
		pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), focus.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(pauline_call);
		if (pauline_call) {
			LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline.getLc(), pauline_call);
			linphone_call_params_enable_video(pauline_params, TRUE);
			linphone_call_params_enable_early_media_sending(pauline_params, FALSE);
			ms_message("%s accepts the call from %s", linphone_core_get_identity(pauline.getLc()),
			           linphone_core_get_identity(focus.getLc()));
			linphone_call_accept_with_params(pauline_call, pauline_params);
			linphone_call_params_unref(pauline_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
		                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));

		// The focus relays the acceptance of the call to Marie
		focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), marie.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		std::string newFocusMarieOpCallId;
		if (focus_call) {
			SalOp *op = linphone_call_get_op(focus_call);
			newFocusMarieOpCallId = op->getCallId();
			BC_ASSERT_FALSE(newFocusMarieOpCallId.empty());
			const char *logCallId = linphone_call_log_get_call_id(linphone_call_get_call_log(focus_call));
			BC_ASSERT_PTR_NOT_NULL(logCallId);
			BC_ASSERT_STRING_EQUAL(newFocusMarieOpCallId.c_str(), logCallId);
			BC_ASSERT_STRING_NOT_EQUAL(newFocusMarieOpCallId.c_str(), oldFocusMarieOpCallId.c_str());
			LinphoneCallParams *focus_accept_params = linphone_core_create_call_params(focus.getLc(), focus_call);
			linphone_call_params_enable_video(focus_accept_params, TRUE);
			linphone_call_params_enable_early_media_sending(focus_accept_params, FALSE);
			ms_message("%s accepts the call from %s", linphone_core_get_identity(focus.getLc()),
			           linphone_core_get_identity(marie.getLc()));
			linphone_call_accept_with_params(focus_call, focus_accept_params);
			linphone_call_params_unref(focus_accept_params);
		}

		marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getIdentity().toC());
		BC_ASSERT_PTR_NOT_NULL(marie_call);
		std::string newMarieFocusOpCallId;
		if (marie_call) {
			SalOp *op = linphone_call_get_op(marie_call);
			newMarieFocusOpCallId = op->getCallId();
			BC_ASSERT_FALSE(newMarieFocusOpCallId.empty());
			const char *logCallId = linphone_call_log_get_call_id(linphone_call_get_call_log(marie_call));
			BC_ASSERT_PTR_NOT_NULL(logCallId);
			BC_ASSERT_STRING_EQUAL(newMarieFocusOpCallId.c_str(), logCallId);
			BC_ASSERT_STRING_NOT_EQUAL(newMarieFocusOpCallId.c_str(), oldMarieFocusOpCallId.c_str());
		}
		// Verify that the call ID has changed as Marie recovered the call after the network dropped
		// Earlier in the test, there are more verification to ensure the call logs have been updated
		BC_ASSERT_STRING_EQUAL(newMarieFocusOpCallId.c_str(), newFocusMarieOpCallId.c_str());
		BC_ASSERT_STRING_NOT_EQUAL(newMarieFocusOpCallId.c_str(), oldFocusMarieOpCallId.c_str());

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));

		end_call(marie.getCMgr(), focus.getCMgr());
		end_call(pauline.getCMgr(), focus.getCMgr());

		double similar = 1;
		const double threshold = 0.85;
		BC_ASSERT_EQUAL(ms_audio_diff(pauline_soundpath, marie_recordpath, &similar, &audio_cmp_params, NULL, NULL), 0,
		                int, "%d");
		BC_ASSERT_GREATER(similar, threshold, double, "%g");
		BC_ASSERT_LOWER(similar, 1.0, double, "%g");

		linphone_conference_unref(conference);
		bc_free(pauline_recordpath);
		bc_free(pauline_soundpath);
		bc_free(marie_recordpath);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_transferred_conference_basic_tests[] = {
    TEST_NO_TAG("Create audio transfer conference", LinphoneTest::create_audio_transfer_conference),
    TEST_NO_TAG("Create video transfer conference", LinphoneTest::create_video_transfer_conference),
    TEST_NO_TAG("Create video transfer conference with active speaker changed",
                LinphoneTest::create_video_transfer_conference_active_speaker_changed),
    TEST_NO_TAG("Conference joined in early media", LinphoneTest::conference_joined_in_early_media),
#ifdef HAVE_EKT_SERVER_PLUGIN
    TEST_ONE_TAG(
        "Create encrypted video conference", LinphoneTest::create_audio_video_encrypted_conference, "End2EndConf"),
    TEST_ONE_TAG("Create encrypted audio conference multiple external SRTP key negociation scheme",
                 LinphoneTest::create_audio_video_encrypted_conference_multi_encrypt,
                 "End2EndConf"),
#endif // HAVE_EKT_SERVER_PLUGIN
};

test_suite_t local_conference_test_suite_transferred_conference_basic = {
    "Local conference tester (Transferred Conference Basic)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_transferred_conference_basic_tests) /
        sizeof(local_conference_transferred_conference_basic_tests[0]),
    local_conference_transferred_conference_basic_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};
