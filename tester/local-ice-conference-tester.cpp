/*
 * copyright (c) 2010-2023 belledonne communications sarl.
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

#include "liblinphone_tester.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "local-conference-tester-functions.h"
#include "shared_tester_functions.h"

namespace LinphoneTest {

static void create_simple_ice_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_stun_ice_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, TRUE,
	                       FALSE);
}

static void create_simple_ice_srtp_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_ice_dtls_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_stun_ice_srtp_conference(void) {
	create_conference_base(
	    ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE,
	    LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv,
	    FALSE, LinphoneConferenceSecurityLevelNone, {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener},
	    FALSE, FALSE, FALSE, FALSE);
}

static void create_simple_ice_conference_with_audio_only_participant(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_stun_ice_conference_with_audio_only_participant(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_stun_ice_srtp_conference_with_audio_only_participant(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_point_to_point_encrypted_ice_conference(void) {
	create_conference_base(
	    ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE,
	    LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE,
	    LinphoneConferenceSecurityLevelPointToPoint, {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener},
	    FALSE, FALSE, TRUE, FALSE);
}

static void create_simple_ice_conference_merging_calls(void) {
	create_simple_conference_merging_calls_base(TRUE, LinphoneConferenceLayoutActiveSpeaker, TRUE, FALSE, TRUE,
	                                            LinphoneConferenceSecurityLevelNone, FALSE);
}

static void abort_call_to_ice_conference(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		const LinphoneConferenceLayout layout = LinphoneConferenceLayoutGrid;

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			const bctbx_list_t *accounts = linphone_core_get_account_list(mgr->lc);
			for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
				LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
				enable_stun_in_account(mgr, account, TRUE, TRUE);
			}
			enable_stun_in_mgr(mgr, TRUE, TRUE, TRUE, TRUE);
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		auto members = participants;
		members.push_back(marie.getCMgr());

		time_t start_time = ms_time(NULL);
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test aborted ICE call";
		const char *description = "Grenoble";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		stats focus_stat = focus.getStats();

		bctbx_list_t *participant_infos = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			LinphoneParticipantInfo *participant_info = linphone_participant_info_new(p->identity);
			linphone_participant_info_set_role(participant_info, role);
			participant_infos = bctbx_list_append(participant_infos, participant_info);
			participantList.insert(std::make_pair(p, participant_info));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, FALSE, FALSE, NULL);

		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *confAddrStr = (confAddr) ? linphone_address_as_string(confAddr) : NULL;

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			ms_message("%s calls conference %s", linphone_core_get_identity(mgr->lc), confAddrStr);
			LinphoneCall *call =
			    linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			BC_ASSERT_PTR_NOT_NULL(call);
			linphone_call_params_unref(new_params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			if (call) {
				linphone_call_terminate(call);
			}
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, 3,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 3,
		                             liblinphone_tester_sip_timeout));

		// The conference goes three times because participants enter it one after the other and there isn't ever two
		// calls running at the same time
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 3, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, 3, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, 3, int, "%d");

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			reset_counters(&mgr->stat);
			ms_message("%s calls again conference %s", linphone_core_get_identity(mgr->lc), confAddrStr);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 3,
		                             liblinphone_tester_sip_timeout));

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
		    fill_member_list(members, participantList, marie.getCMgr(), participant_infos);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, FALSE);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			linphone_address_unref(uri);
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
				if (mgr == focus.getCMgr()) {
					no_participants = 3;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 2;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
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
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
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

		// Wait a little bit
		wait_for_list(coresList, NULL, 0, 3000);

		std::list<LinphoneCoreManager *> mgrsToRemove{pauline.getCMgr()};
		mgrsToRemove.push_back(laure.getCMgr());

		stats marie_stat = marie.getStats();

		for (auto mgr : mgrsToRemove) {
			LinphoneCall *call = linphone_core_get_current_call(mgr->lc);
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

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
		                             marie_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
		                             marie_stat.number_of_participant_devices_removed + 2,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted,
		                marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr(), marie.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t *calls = linphone_core_get_calls(marie.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall *call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 2, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 2, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(participant_infos, (bctbx_list_free_func)linphone_participant_info_unref);
		if (confAddrStr) ms_free(confAddrStr);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_scheduled_ice_conference_tests[] = {
    TEST_ONE_TAG("Create simple ICE conference", LinphoneTest::create_simple_ice_conference, "ICE"),
    TEST_ONE_TAG("Create simple STUN+ICE conference", LinphoneTest::create_simple_stun_ice_conference, "ICE"),
    TEST_ONE_TAG("Create simple ICE SRTP conference", LinphoneTest::create_simple_ice_srtp_conference, "ICE"),
    TEST_ONE_TAG("Create simple ICE DTLS conference", LinphoneTest::create_simple_ice_dtls_conference, "ICE"),
    TEST_ONE_TAG("Create simple STUN+ICE SRTP conference", LinphoneTest::create_simple_stun_ice_srtp_conference, "ICE"),
    TEST_ONE_TAG("Create simple ICE conference with audio only participant",
                 LinphoneTest::create_simple_ice_conference_with_audio_only_participant,
                 "ICE"),
    TEST_ONE_TAG("Create simple STUN+ICE conference with audio only participant",
                 LinphoneTest::create_simple_stun_ice_conference_with_audio_only_participant,
                 "ICE"),
    TEST_ONE_TAG("Create simple STUN+ICE SRTP conference with audio only participant",
                 LinphoneTest::create_simple_stun_ice_srtp_conference_with_audio_only_participant,
                 "ICE"),
    TEST_ONE_TAG("Create simple point-to-point encrypted ICE conference",
                 LinphoneTest::create_simple_point_to_point_encrypted_ice_conference,
                 "ICE"),
    TEST_ONE_TAG("Create simple ICE conference by merging calls",
                 LinphoneTest::create_simple_ice_conference_merging_calls,
                 "ICE"), /* because of aborted calls*/
    TEST_TWO_TAGS("Abort call to ICE conference",
                  LinphoneTest::abort_call_to_ice_conference,
                  "LeaksMemory",
                  "ICE") /* because of aborted calls*/
};

test_suite_t local_conference_test_suite_scheduled_ice_conference = {
    "Local conference tester (Scheduled ICE Conference)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_scheduled_ice_conference_tests) /
        sizeof(local_conference_scheduled_ice_conference_tests[0]),
    local_conference_scheduled_ice_conference_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};
