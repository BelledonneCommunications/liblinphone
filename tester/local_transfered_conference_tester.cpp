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
#include "liblinphone_tester.h"
#include "local_conference_tester_functions.h"
#include "shared_tester_functions.h"

namespace LinphoneTest {

void create_transfer_conference_base(time_t start_time,
                                     int duration,
                                     LinphoneConferenceParticipantListType participant_list_type,
                                     const LinphoneMediaEncryption encryption,
                                     LinphoneConferenceLayout layout,
                                     LinphoneConferenceSecurityLevel security_level) {
	char *pauline_recordpath = bc_tester_file("record-local_transfered_conference_with_file_player_pauline.wav");
	char *laure_recordpath = bc_tester_file("record-local_transfered_conference_with_file_player_laure.wav");
	char *marie_recordpath = bc_tester_file("record-local_transfered_conference_with_file_player_marie.wav");
	char *pauline_soundpath = bc_tester_res("sounds/vrroom.wav");
	char *laure_soundpath = bc_tester_res("sounds/hello8000.wav");
	char *marie_soundpath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	unlink(pauline_recordpath);
	unlink(laure_recordpath);
	unlink(marie_recordpath);
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		// Record files, each participant gets a different input file so we can check manually the mix is actually
		// performed
		linphone_core_set_record_file(marie.getCMgr()->lc, marie_recordpath);
		linphone_core_set_record_file(pauline.getCMgr()->lc, pauline_recordpath);
		linphone_core_set_record_file(laure.getCMgr()->lc, laure_recordpath);
		linphone_core_set_play_file(marie.getCMgr()->lc, marie_soundpath);
		linphone_core_set_play_file(pauline.getCMgr()->lc, pauline_soundpath);
		linphone_core_set_play_file(laure.getCMgr()->lc, laure_soundpath);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
				linphone_core_set_media_encryption(mgr->lc, encryption);
			}

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		// Focus is in full packet mode: transfer packet not payload
		LinphoneConfig *focus_config = linphone_core_get_config(focus.getLc());
		linphone_config_set_bool(focus_config, "sound", "conference_full_packet", TRUE);
		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
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

		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, start_time, end_time,
		                                                        initialSubject, description, TRUE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		// Each member call the conference
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
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

		// Check everyone is connected to the conference
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated,
			                             ((mgr == marie.getCMgr()) ? 3 : 2), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
			}
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
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 3, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 3,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, false);

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
					size_t no_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
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
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

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

		// Explicitely terminate conference as those on server are static by default
		LinphoneConference *pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			linphone_conference_terminate(pconference);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		std::list<LinphoneCoreManager *> allMembers{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};
		for (auto mgr : allMembers) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), ((false && (mgr == marie.getCMgr())) ? 2 : 1),
			                unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log),
				                ((false && (mgr == marie.getCMgr())) ? 2 : 1), unsigned int, "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		// wait bit more to detect side effect if any
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
static void create_transfer_conference(void) {
	create_transfer_conference_base(ms_time(NULL), -1, LinphoneConferenceParticipantListTypeOpen,
	                                LinphoneMediaEncryptionNone, LinphoneConferenceLayoutGrid,
	                                LinphoneConferenceSecurityLevelNone);
}

} // namespace LinphoneTest

static test_t local_conference_transfered_conference_basic_tests[] = {
    TEST_NO_TAG("Create transfer conference", LinphoneTest::create_transfer_conference),
};

test_suite_t local_conference_test_suite_transfered_conference_basic = {
    "Local conference tester (Transfered Conference Basic)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_transfered_conference_basic_tests) /
        sizeof(local_conference_transfered_conference_basic_tests[0]),
    local_conference_transfered_conference_basic_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};
