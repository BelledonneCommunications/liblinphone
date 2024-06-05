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

#include "conference/conference-info.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-conference-scheduler.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "local-conference-tester-functions.h"
#include "mediastreamer2/mspacketrouter.h"
#include "shared_tester_functions.h"

namespace LinphoneTest {

static void call_to_inexisting_conference_address(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());

		bctbx_list_t *coresList = NULL;
		coresList = bctbx_list_append(coresList, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());

		focus.registerAsParticipantDevice(marie);

		LinphoneAddress *confAddr = linphone_address_new(L_STRING_TO_C(focus.getIdentity().toString()));
		linphone_address_set_uri_param(confAddr, "conf-id", "xxxxx");

		stats marie_stat = marie.getStats();

		LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), nullptr);
		linphone_core_invite_address_with_params_2(marie.getLc(), confAddr, new_params, NULL, nullptr);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
		                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallError,
		                             marie_stat.number_of_LinphoneCallError + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased,
		                             marie_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_conference_on_unresponsive_server(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);

		setup_conference_info_cbs(marie.getCMgr());

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());

		// Wait for 25s to make sure the request times out
		int request_timeout_wait = 25000;

		time_t start_time = ms_time(NULL) - 60;
		time_t end_time = start_time + 600;
		const char *subject = "Colleagues";
		const char *description = "Tom Black";

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelEndToEnd;

		bctbx_list_t *participants_info = NULL;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		const char *domain = linphone_address_get_domain(marie.getCMgr()->identity);
		char user_address[100];
		for (int idx = 0; idx < 180; idx++) {
			snprintf(user_address, sizeof(user_address), "sip:happyuser%0d@%s", idx, domain);
			LinphoneAddress *user = linphone_factory_create_address(linphone_factory_get(), user_address);
			add_participant_info_to_list(&participants_info, user, role, -1);
			if (role == LinphoneParticipantRoleSpeaker) {
				role = LinphoneParticipantRoleListener;
			} else if (role == LinphoneParticipantRoleListener) {
				role = LinphoneParticipantRoleUnknown;
			} else if (role == LinphoneParticipantRoleUnknown) {
				role = LinphoneParticipantRoleSpeaker;
			}
			linphone_address_unref(user);
		}

		// Turn off Chloe's network
		linphone_core_set_network_reachable(focus.getLc(), FALSE);

		stats marie_stat = marie.getStats();

		// Marie creates a conference scheduler
		LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_sip_conference_scheduler(
		    marie.getLc(), linphone_core_get_default_account(marie.getLc()));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneConferenceInfo *conf_info = linphone_conference_info_new();

		LinphoneAccount *default_account = linphone_core_get_default_account(marie.getLc());
		LinphoneAddress *organizer_address = default_account
		                                         ? linphone_address_clone(linphone_account_params_get_identity_address(
		                                               linphone_account_get_params(default_account)))
		                                         : linphone_address_new(linphone_core_get_identity(marie.getLc()));
		linphone_conference_info_set_organizer(conf_info, organizer_address);
		linphone_conference_info_set_participant_infos(conf_info, participants_info);
		linphone_conference_info_set_duration(
		    conf_info, (int)((end_time - start_time) / 60)); // duration is expected to be set in minutes
		linphone_conference_info_set_date_time(conf_info, start_time);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description);
		linphone_conference_info_set_security_level(conf_info, security_level);

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateAllocationPending,
		                             marie_stat.number_of_ConferenceSchedulerStateAllocationPending + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateError,
		                             marie_stat.number_of_ConferenceSchedulerStateError + 1, request_timeout_wait));

		// Marie retries to create a conference and she takes the opportunity to change the duration
		linphone_conference_info_set_duration(
		    conf_info, (int)((3 * end_time - start_time) / 60)); // duration is expected to be set in minutes

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
		                             marie_stat.number_of_ConferenceSchedulerStateUpdating + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateError,
		                             marie_stat.number_of_ConferenceSchedulerStateError + 2, request_timeout_wait));

		linphone_conference_info_unref(conf_info);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		if (organizer_address) linphone_address_unref(organizer_address);
		linphone_conference_scheduler_unref(conference_scheduler);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_db_conference_scheduler_base(bool_t server_restart) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = FALSE;

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), enable_lime);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		const LinphoneConferenceInfo *scheduler_conference_info = NULL;
		LinphoneAddress *conference_address = NULL;
		LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		int nb_subscriptions = 1;
		int focus_no_streams_running = 6;

		setup_conference_info_cbs(marie.getCMgr());
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		bctbx_list_t *coresList = NULL;
		bctbx_list_t *participants_info = NULL;
		std::list<LinphoneCoreManager *> members;
		std::list<LinphoneCoreManager *> conferenceMgrs;

		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			conferenceMgrs.push_back(mgr);
			if (mgr != focus.getCMgr()) {
				members.push_back(mgr);
				participantList.insert(
				    std::make_pair(mgr, add_participant_info_to_list(&participants_info, mgr->identity, role, -1)));
				switch (role) {
					case LinphoneParticipantRoleSpeaker:
						role = LinphoneParticipantRoleUnknown;
						break;
					case LinphoneParticipantRoleListener:
						role = LinphoneParticipantRoleSpeaker;
						break;
					case LinphoneParticipantRoleUnknown:
						role = LinphoneParticipantRoleListener;
						break;
				}
			}
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		stats focus_stat = focus.getStats();

		LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_db_conference_scheduler(
		    focus.getLc(), linphone_core_get_default_account(focus.getLc()));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneAddress *organizer_address = marie.getCMgr()->identity;
		time_t start_time = ms_time(NULL) + 5;
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		const char *subject = "DB test";
		const char *description = "Testing database conference scheduler";
		char *conference_address_str = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList;

		LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
		linphone_conference_info_set_organizer(conf_info, organizer_address);
		linphone_conference_info_set_participant_infos(conf_info, participants_info);
		linphone_conference_info_set_duration(conf_info, duration);
		linphone_conference_info_set_date_time(conf_info, start_time);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description);
		linphone_conference_info_set_security_level(conf_info, security_level);

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
		linphone_conference_info_unref(conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateAllocationPending,
		                             focus_stat.number_of_ConferenceSchedulerStateAllocationPending + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateReady,
		                             focus_stat.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		scheduler_conference_info = linphone_conference_scheduler_get_info(conference_scheduler);
		BC_ASSERT_PTR_NOT_NULL(scheduler_conference_info);
		if (!scheduler_conference_info) {
			goto end;
		}
		conference_address = linphone_conference_info_get_uri(scheduler_conference_info)
		                         ? linphone_address_clone(linphone_conference_info_get_uri(scheduler_conference_info))
		                         : NULL;
		linphone_conference_scheduler_unref(conference_scheduler);
		conference_scheduler = NULL;
		BC_ASSERT_PTR_NOT_NULL(conference_address);
		if (!conference_address) {
			goto end;
		}
		conference_address_str = linphone_address_as_string(conference_address);

		check_conference_info_in_db(focus.getCMgr(), NULL, conference_address, organizer_address, participants_info,
		                            start_time, duration, subject, description, 0, LinphoneConferenceInfoStateNew,
		                            security_level, FALSE, TRUE, TRUE, FALSE);

		if (!!server_restart) {
			coresList = bctbx_list_remove(coresList, focus.getLc());
			ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
			// Restart flexisip
			focus.reStart();
			coresList = bctbx_list_append(coresList, focus.getLc());
			check_conference_info_in_db(focus.getCMgr(), NULL, conference_address, organizer_address, participants_info,
			                            start_time, duration, subject, description, 0, LinphoneConferenceInfoStateNew,
			                            security_level, FALSE, TRUE, TRUE, FALSE);
		}

		// wait a bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, conference_address, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, conference_address);
			BC_ASSERT_PTR_NOT_NULL(pcall);
		}

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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, conference_address);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");

				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 3,
		                             liblinphone_tester_sip_timeout));

		memberList = fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, conference_address, FALSE);

		// wait a bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		for (auto mgr : members) {
			ms_message("%s is exiting conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, conference_address);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				linphone_call_terminate(pcall);
			}
		}

		for (auto mgr : members) {
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             nb_subscriptions, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
				    .waitUntil(chrono::seconds((time_left + 1)), [] { return false; });
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
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

		// wait a bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

	end:
		if (conference_address) linphone_address_unref(conference_address);
		if (conference_address_str) ms_free(conference_address_str);
		if (conference_scheduler) linphone_conference_scheduler_unref(conference_scheduler);
		bctbx_list_free(coresList);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	}
}

static void create_simple_conference_db_conference_scheduler(void) {
	create_simple_conference_db_conference_scheduler_base(FALSE);
}

static void create_simple_conference_db_conference_scheduler_server_restart(void) {
	create_simple_conference_db_conference_scheduler_base(TRUE);
}

static void schedule_simple_conference_db_conference_scheduler(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = FALSE;

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), enable_lime);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		const LinphoneConferenceInfo *scheduler_conference_info = NULL;
		LinphoneAddress *conference_address = NULL;

		setup_conference_info_cbs(marie.getCMgr());
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		bctbx_list_t *coresList = NULL;
		bctbx_list_t *participants_info = NULL;
		LinphoneConferenceInfo *info = NULL;
		std::list<LinphoneCoreManager *> members;
		std::list<LinphoneCoreManager *> conferenceMgrs;

		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			conferenceMgrs.push_back(mgr);
			if (mgr != focus.getCMgr()) {
				members.push_back(mgr);
				participantList.insert(
				    std::make_pair(mgr, add_participant_info_to_list(&participants_info, mgr->identity, role, -1)));
				switch (role) {
					case LinphoneParticipantRoleSpeaker:
						role = LinphoneParticipantRoleUnknown;
						break;
					case LinphoneParticipantRoleListener:
						role = LinphoneParticipantRoleSpeaker;
						break;
					case LinphoneParticipantRoleUnknown:
						role = LinphoneParticipantRoleListener;
						break;
				}
			}
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_conference_cleanup_period(focus.getLc(), 5);

		stats focus_stat = focus.getStats();

		LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_db_conference_scheduler(
		    focus.getLc(), linphone_core_get_default_account(focus.getLc()));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneAddress *organizer_address = marie.getCMgr()->identity;
		time_t start_time = ms_time(NULL) - 45;
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		const char *subject = "DB test";
		const char *description = "Testing database conference scheduler";
		char *conference_address_str = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList;

		LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
		linphone_conference_info_set_organizer(conf_info, organizer_address);
		linphone_conference_info_set_participant_infos(conf_info, participants_info);
		linphone_conference_info_set_duration(conf_info, duration);
		linphone_conference_info_set_date_time(conf_info, start_time);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description);
		linphone_conference_info_set_security_level(conf_info, security_level);

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
		linphone_conference_info_unref(conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateAllocationPending,
		                             focus_stat.number_of_ConferenceSchedulerStateAllocationPending + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateReady,
		                             focus_stat.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		scheduler_conference_info = linphone_conference_scheduler_get_info(conference_scheduler);
		BC_ASSERT_PTR_NOT_NULL(scheduler_conference_info);
		if (!scheduler_conference_info) {
			goto end;
		}
		conference_address = linphone_conference_info_get_uri(scheduler_conference_info)
		                         ? linphone_address_clone(linphone_conference_info_get_uri(scheduler_conference_info))
		                         : NULL;
		linphone_conference_scheduler_unref(conference_scheduler);
		conference_scheduler = NULL;
		BC_ASSERT_PTR_NOT_NULL(conference_address);
		if (!conference_address) {
			goto end;
		}
		conference_address_str = linphone_address_as_string(conference_address);

		check_conference_info_in_db(focus.getCMgr(), NULL, conference_address, organizer_address, participants_info,
		                            start_time, duration, subject, description, 0, LinphoneConferenceInfoStateNew,
		                            security_level, FALSE, TRUE, TRUE, FALSE);

		coresList = bctbx_list_remove(coresList, focus.getLc());
		ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
		// Restart flexisip
		focus.reStart();
		coresList = bctbx_list_append(coresList, focus.getLc());
		check_conference_info_in_db(focus.getCMgr(), NULL, conference_address, organizer_address, participants_info,
		                            start_time, duration, subject, description, 0, LinphoneConferenceInfoStateNew,
		                            security_level, FALSE, TRUE, TRUE, FALSE);

		ms_message("Waiting for conference %s to expire", conference_address_str);

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		info = linphone_core_find_conference_information_from_uri(focus.getLc(), conference_address);
		BC_ASSERT_PTR_NULL(info);

	end:
		if (conference_address) linphone_address_unref(conference_address);
		if (conference_address_str) ms_free(conference_address_str);
		bctbx_list_free(coresList);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	}
}

static void create_simple_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_point_to_point_encrypted_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE,
	                       LinphoneConferenceSecurityLevelPointToPoint,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_conference_with_server_restart(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, TRUE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_conference_with_client_restart(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       TRUE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_zrtp_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionZRTP, TRUE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE,
	                       LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_dtls_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE,
	                       LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_srtp_conference(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_conference_with_only_speakers_and_uninvited_participant(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE,
	                       LinphoneConferenceSecurityLevelNone, {LinphoneParticipantRoleSpeaker}, FALSE);
}

/*
static void create_conference_with_only_listeners_and_uninvited_participant(void) {
    create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
                           LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
                           FALSE, FALSE, LinphoneMediaDirectionSendRecv, TRUE, LinphoneConferenceSecurityLevelNone,
                           {LinphoneParticipantRoleListener}, FALSE);
}
*/

static void create_conference_with_uninvited_participant(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_conference_with_uninvited_participant_added_after_end(void) {
	create_conference_base(ms_time(NULL), 1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, TRUE);
}

static void create_conference_with_uninvited_participant_not_allowed(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeClosed, FALSE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE,
	                       LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_conference_starting_immediately(void) {
	create_conference_base(ms_time(NULL), 0, FALSE, LinphoneConferenceParticipantListTypeClosed, FALSE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_conference_starting_in_the_past(void) {
	create_conference_base(ms_time(NULL) - 640, 11, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE,
	                       LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE,
	                       LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_conference_with_audio_only_participant(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_conference_with_audio_only_and_uninvited_participant(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_conference_with_audio_only_participant_enabling_video(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelNone,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_conference_with_late_participant_addition(void) {
	create_conference_with_late_participant_addition_base(ms_time(NULL), -1, LinphoneConferenceLayoutGrid,
	                                                      LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE,
	                                                      LinphoneConferenceSecurityLevelNone);
}

static void create_conference_with_late_participant_addition_declined(void) {
	create_conference_with_late_participant_addition_base(ms_time(NULL), -1, LinphoneConferenceLayoutActiveSpeaker,
	                                                      LinphoneConferenceParticipantListTypeClosed, FALSE, TRUE,
	                                                      LinphoneConferenceSecurityLevelNone);
}

static void create_simple_conference_with_screen_sharing(void) {
	create_conference_with_screen_sharing_base(ms_time(NULL), -1, LinphoneMediaEncryptionNone,
	                                           LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, FALSE,
	                                           LinphoneMediaDirectionSendRecv, LinphoneConferenceSecurityLevelNone,
	                                           {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener});
}

static void create_simple_conference_with_screen_sharing_override(void) {
	create_conference_with_screen_sharing_base(
	    ms_time(NULL), -1, LinphoneMediaEncryptionZRTP, LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, FALSE,
	    LinphoneMediaDirectionSendOnly, LinphoneConferenceSecurityLevelNone, {LinphoneParticipantRoleSpeaker});
}

static void create_simple_conference_with_screen_sharing_turn_off(void) {
	create_conference_with_screen_sharing_base(
	    ms_time(NULL), -1, LinphoneMediaEncryptionZRTP, LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, TRUE,
	    LinphoneMediaDirectionSendOnly, LinphoneConferenceSecurityLevelNone, {LinphoneParticipantRoleSpeaker});
}

static void create_simple_conference_with_screen_sharing_override_no_camera(void) {
	create_conference_with_screen_sharing_base(
	    ms_time(NULL), -1, LinphoneMediaEncryptionZRTP, LinphoneConferenceLayoutActiveSpeaker, TRUE, FALSE, FALSE,
	    LinphoneMediaDirectionSendOnly, LinphoneConferenceSecurityLevelNone, {LinphoneParticipantRoleSpeaker});
}

static void create_simple_conference_with_screen_sharing_grid_layout(void) {
	create_conference_with_screen_sharing_base(ms_time(NULL), -1, LinphoneMediaEncryptionNone,
	                                           LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE,
	                                           LinphoneMediaDirectionSendRecv, LinphoneConferenceSecurityLevelNone,
	                                           {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener});
}

static void create_simple_conference_with_screen_sharing_no_video(void) {
	create_conference_with_screen_sharing_base(ms_time(NULL), -1, LinphoneMediaEncryptionNone,
	                                           LinphoneConferenceLayoutActiveSpeaker, FALSE, TRUE, FALSE,
	                                           LinphoneMediaDirectionSendRecv, LinphoneConferenceSecurityLevelNone,
	                                           {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener});
}

static void create_simple_conference_with_screen_sharing_no_video_send_component(void) {
	create_conference_with_screen_sharing_base(
	    ms_time(NULL), -1, LinphoneMediaEncryptionSRTP, LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, FALSE,
	    LinphoneMediaDirectionRecvOnly, LinphoneConferenceSecurityLevelPointToPoint,
	    {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener});
}

void participant_joins_simple_conference_with_screen_sharing(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_static_image_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionSRTP);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr()};

		time_t start_time = ms_time(NULL);
		time_t end_time = -1;
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		int idx = 1;

		std::list<LinphoneCoreManager *> addedMembers;
		std::list<LinphoneCoreManager *> addedConferenceMgrs{focus.getCMgr()};
		// The head of the members list enables screensharing whereas the others join the conference without vidoe
		// capabilities and then they add it later on
		for (auto mgr : members) {
			addedConferenceMgrs.push_back(mgr);
			addedMembers.push_back(mgr);
			bool is_first_member = (mgr == members.front());
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_params_enable_video(new_params, is_first_member);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}

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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

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

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + idx,
			                             liblinphone_tester_sip_timeout));
			int focus_no_streams_running = 2 * idx;
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                  focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - idx),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
			                             liblinphone_tester_sip_timeout));
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
			                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + idx, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + idx, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + idx,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + idx,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat.number_of_participant_devices_joined + idx,
			                             liblinphone_tester_sip_timeout));

			idx++;

			std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
			    fill_memmber_list(addedMembers, participantList, marie.getCMgr(), participants_info);
			wait_for_conference_streams({focus, marie, pauline}, addedConferenceMgrs, focus.getCMgr(), memberList,
			                            confAddr, TRUE);

			int nb_reinvites = 0;
			stats mgr_stat = mgr->stat;
			stats focus_stat2 = focus.getStats();
			if (is_first_member) {
				ms_message("%s enables screen sharing", linphone_core_get_identity(mgr->lc));
			} else {
				ms_message("%s enables video capabilities", linphone_core_get_identity(mgr->lc));
			}
			LinphoneCall *mgr_call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(mgr_call);
			if (mgr_call) {
				nb_reinvites++;
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, mgr_call);
				linphone_call_params_enable_video(new_params, TRUE);
				linphone_call_params_enable_screen_sharing(new_params, is_first_member);
				linphone_call_update(mgr_call, new_params);
				linphone_call_params_unref(new_params);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating,
			                             mgr_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
			                             mgr_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			if (!is_first_member) {
				ms_message("%s disables video capabilities", linphone_core_get_identity(mgr->lc));
				nb_reinvites++;
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, mgr_call);
				linphone_call_params_enable_video(new_params, FALSE);
				linphone_call_params_enable_screen_sharing(new_params, FALSE);
				linphone_call_update(mgr_call, new_params);
				linphone_call_params_unref(new_params);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating,
			                             mgr_stat.number_of_LinphoneCallUpdating + nb_reinvites,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
			                             mgr_stat.number_of_LinphoneCallStreamsRunning + nb_reinvites,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + nb_reinvites,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + nb_reinvites,
			                             liblinphone_tester_sip_timeout));

			if (is_first_member) {
				BC_ASSERT_TRUE(check_screen_sharing_sdp(focus.getCMgr(), mgr, true));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &focus.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             focus_stat2.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_screen_sharing_enabled,
				                             mgr_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
			} else {
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_participant_devices_media_capability_changed,
				                  mgr_stat.number_of_participant_devices_media_capability_changed + nb_reinvites,
				                  liblinphone_tester_sip_timeout));
			}

			if (mgr_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(mgr_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), is_first_member, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), is_first_member, int, "%0d");
			}

			LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(focus_call);
			if (focus_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), is_first_member, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), is_first_member, int, "%0d");

				const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), is_first_member, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_rparams), is_first_member, int, "%0d");
			}

			wait_for_conference_streams({focus, marie, pauline}, addedConferenceMgrs, focus.getCMgr(), memberList,
			                            confAddr, TRUE);
		}

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline})
			        .waitUntil(chrono::seconds(50), [mgr, &focus, &members, confAddr] {
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        bool_t screen_sharing_ok = TRUE;
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        bool_t video_available =
					            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					        bool_t is_screensharing =
					            linphone_address_weak_equal(device_address, members.front()->identity);
					        screen_sharing_ok &=
					            (((mgr == members.front()) || (mgr == focus.getCMgr())) && is_screensharing)
					                ? video_available
					                : !video_available;
					        LinphoneParticipant *participant =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        screen_sharing_ok &=
					            (linphone_participant_device_screen_sharing_enabled(d) == is_screensharing);
					        if (is_screensharing) {
						        screen_sharing_ok &= (d == screen_sharing_device);
						        screen_sharing_ok &= (participant == screen_sharing_participant);
					        } else {
						        screen_sharing_ok &= (d != screen_sharing_device);
						        screen_sharing_ok &= (participant != screen_sharing_participant);
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return screen_sharing_ok;
			        }));
		}

		focus_stat = focus.getStats();

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 2, int, "%d");

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(10), [] { return false; });

		int participant_call_ended = 1;
		auto remaining_members = members;
		for (auto mgr : members) {
			stats focus_stat2 = focus.getStats();
			remaining_members.pop_front();
			std::list<stats> remaining_members_stats;
			int nb_members = 0;
			for (auto remaining_mgr : remaining_members) {
				remaining_members_stats.push_back(remaining_mgr->stat);
				LinphoneCall *pcall =
				    linphone_core_get_call_by_remote_address2(remaining_mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
					if (linphone_call_params_video_enabled(call_cparams)) {
						nb_members++;
					}
				}
			}
			const bctbx_list_t *participant_calls = linphone_core_get_calls(mgr->lc);
			int participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 1, int, "%d");

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
			                             participant_call_ended, liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

			participant_calls = linphone_core_get_calls(mgr->lc);
			participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

			for (auto remaining_mgr : remaining_members) {
				stats stat = remaining_members_stats.front();
				remaining_members_stats.pop_front();
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participants_removed,
				                             stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participant_devices_removed,
				                             stat.number_of_participant_devices_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat2.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat2.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			unsigned int expected_call_logs = 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), expected_call_logs, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), expected_call_logs, unsigned int,
				                "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info2 =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
				LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, FALSE);
			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void conference_with_screen_sharing_enabled_since_the_start(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionSRTP);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		time_t start_time = ms_time(NULL);
		time_t end_time = -1;
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleListener, -1)));

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 4,
		                             liblinphone_tester_sip_timeout));

		const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			if (mgr == laure.getCMgr()) {
				linphone_call_params_enable_screen_sharing(new_params, TRUE);
			} else if (mgr == pauline.getCMgr()) {
				linphone_call_params_enable_mic(new_params, FALSE);
			}
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

		int idx = 1;
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
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

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

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 5,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 10;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 5),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 5, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 5,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, &laure, &marie, confAddr] {
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        bool_t screen_sharing_ok = TRUE;
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        bool_t video_available =
					            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					        bool_t is_laure = linphone_address_weak_equal(device_address, laure.getCMgr()->identity);
					        bool_t is_marie = linphone_address_weak_equal(device_address, marie.getCMgr()->identity);
					        if (is_laure || is_marie) {
						        screen_sharing_ok &= video_available;
					        } else {
						        screen_sharing_ok &= !video_available;
					        }
					        LinphoneParticipant *participant =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        screen_sharing_ok &= (linphone_participant_device_screen_sharing_enabled(d) == is_laure);
					        if (is_laure) {
						        screen_sharing_ok &= (d == screen_sharing_device);
						        screen_sharing_ok &= (participant == screen_sharing_participant);
					        } else {
						        screen_sharing_ok &= (d != screen_sharing_device);
						        screen_sharing_ok &= (participant != screen_sharing_participant);
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return screen_sharing_ok;
			        }));
		}

		focus_stat = focus.getStats();

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 5, int, "%d");

		int participant_call_ended = 1;

		auto remaining_members = members;

		for (auto mgr : members) {
			stats focus_stat2 = focus.getStats();
			remaining_members.pop_front();
			std::list<stats> remaining_members_stats;
			int nb_members = 0;
			for (auto remaining_mgr : remaining_members) {
				remaining_members_stats.push_back(remaining_mgr->stat);
				LinphoneCall *pcall =
				    linphone_core_get_call_by_remote_address2(remaining_mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
					if (linphone_call_params_video_enabled(call_cparams)) {
						nb_members++;
					}
				}
			}
			const bctbx_list_t *participant_calls = linphone_core_get_calls(mgr->lc);
			int participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 1, int, "%d");

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
			                             participant_call_ended, liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

			participant_calls = linphone_core_get_calls(mgr->lc);
			participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

			for (auto remaining_mgr : remaining_members) {
				stats stat = remaining_members_stats.front();
				remaining_members_stats.pop_front();
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participants_removed,
				                             stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participant_devices_removed,
				                             stat.number_of_participant_devices_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat2.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat2.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline, michelle, berthe})
				    .waitUntil(chrono::seconds((time_left + 1)), [] { return false; });
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			unsigned int expected_call_logs = 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), expected_call_logs, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), expected_call_logs, unsigned int,
				                "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info2 =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
				LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, FALSE);
			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void conference_with_two_participant_having_screen_sharing_enabled_since_the_start(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionSRTP);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		time_t start_time = ms_time(NULL);
		time_t end_time = -1;
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
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleListener, -1)));

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			if (mgr == laure.getCMgr()) {
				linphone_call_params_enable_screen_sharing(new_params, TRUE);
			} else if (mgr == pauline.getCMgr()) {
				linphone_call_params_enable_screen_sharing(new_params, TRUE);
				linphone_call_params_enable_mic(new_params, FALSE);
			}
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

		int idx = 1;
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
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

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

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 5,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 10;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 5),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 5, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 5,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, &laure, &pauline, confAddr] {
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        bool_t screen_sharing_ok = TRUE;
				        int screen_sharing_counter = 0;
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        bool_t is_laure = linphone_address_weak_equal(device_address, laure.getCMgr()->identity);
					        bool_t is_pauline =
					            linphone_address_weak_equal(device_address, pauline.getCMgr()->identity);
					        LinphoneParticipant *participant =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
					        if (!is_laure && !is_pauline) {
						        // Either Laure or Pauline is screen sharing
						        screen_sharing_ok &= !is_screen_sharing;
					        }
					        if (is_screen_sharing) {
						        screen_sharing_counter++;
						        screen_sharing_ok &= (d == screen_sharing_device);
						        screen_sharing_ok &= (participant == screen_sharing_participant);
					        } else {
						        screen_sharing_ok &= (d != screen_sharing_device);
						        screen_sharing_ok &= (participant != screen_sharing_participant);
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        // Verify that there is only one active screen sharing participant in the conference
				        screen_sharing_ok &= (screen_sharing_counter == 1);
				        return screen_sharing_ok;
			        }));
		}

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		if (fconference) {
			LinphoneParticipantDevice *screen_sharing_device =
			    linphone_conference_get_screen_sharing_participant_device(fconference);
			const LinphoneAddress *screen_sharing_device_address =
			    linphone_participant_device_get_address(screen_sharing_device);
			for (const auto &mgr : conferenceMgrs) {
				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				LinphoneParticipantDevice *client_screen_sharing_device =
				    linphone_conference_get_screen_sharing_participant_device(conference);
				BC_ASSERT_TRUE(
				    linphone_address_equal(linphone_participant_device_get_address(client_screen_sharing_device),
				                           screen_sharing_device_address));
				LinphoneParticipant *client_screen_sharing_participant =
				    linphone_conference_get_screen_sharing_participant(conference);
				BC_ASSERT_TRUE(
				    linphone_address_weak_equal(linphone_participant_get_address(client_screen_sharing_participant),
				                                screen_sharing_device_address));
			}
		}

		focus_stat = focus.getStats();

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 5, int, "%d");

		int participant_call_ended = 1;

		auto remaining_members = members;

		for (auto mgr : members) {
			stats focus_stat2 = focus.getStats();
			remaining_members.pop_front();
			std::list<stats> remaining_members_stats;
			int nb_members = 0;
			for (auto remaining_mgr : remaining_members) {
				remaining_members_stats.push_back(remaining_mgr->stat);
				LinphoneCall *pcall =
				    linphone_core_get_call_by_remote_address2(remaining_mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
					if (linphone_call_params_video_enabled(call_cparams)) {
						nb_members++;
					}
				}
			}
			const bctbx_list_t *participant_calls = linphone_core_get_calls(mgr->lc);
			int participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 1, int, "%d");

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
			                             participant_call_ended, liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

			participant_calls = linphone_core_get_calls(mgr->lc);
			participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

			for (auto remaining_mgr : remaining_members) {
				stats stat = remaining_members_stats.front();
				remaining_members_stats.pop_front();
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participants_removed,
				                             stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participant_devices_removed,
				                             stat.number_of_participant_devices_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat2.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat2.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			unsigned int expected_call_logs = 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), expected_call_logs, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), expected_call_logs, unsigned int,
				                "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info2 =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
				LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, FALSE);
			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void conference_with_screen_sharing_participant_only(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionSRTP);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		int nortp_timeout = 5;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		stats focus_stat = focus.getStats();
		stats laure_stat = laure.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), laure.getCMgr(), marie.getCMgr()};
		std::list<LinphoneCoreManager *> members{laure.getCMgr(), marie.getCMgr()};

		time_t start_time = ms_time(NULL);
		time_t end_time = -1;
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleListener, -1)));

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			if (mgr == marie.getCMgr()) {
				linphone_call_params_enable_screen_sharing(new_params, TRUE);
			}
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

		int idx = 1;
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
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

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

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 2,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 4;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 2),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 2, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 2,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, &marie, confAddr] {
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        bool_t screen_sharing_ok = TRUE;
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        bool_t video_available =
					            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					        screen_sharing_ok &= video_available;
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        bool_t is_marie = linphone_address_weak_equal(device_address, marie.getCMgr()->identity);
					        LinphoneParticipant *participant =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        screen_sharing_ok &= (linphone_participant_device_screen_sharing_enabled(d) == is_marie);
					        if (is_marie) {
						        screen_sharing_ok &= (d == screen_sharing_device);
						        screen_sharing_ok &= (participant == screen_sharing_participant);
					        } else {
						        screen_sharing_ok &= (d != screen_sharing_device);
						        screen_sharing_ok &= (participant != screen_sharing_participant);
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return screen_sharing_ok;
			        }));
		}

		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd,
		                              marie_stat.number_of_LinphoneCallEnd + 1, (1000 * nortp_timeout)));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased,
		                              marie_stat.number_of_LinphoneCallReleased + 1, (1000 * nortp_timeout)));

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		laure_stat = laure.getStats();

		auto itMembers = std::find(members.begin(), members.end(), laure.getCMgr());
		if (itMembers != members.end()) {
			members.erase(itMembers);
		}

		ms_message("%s is terminating call with %s and leaves %s alone while she is screen sharing",
		           linphone_core_get_identity(laure.getLc()), linphone_core_get_identity(focus.getLc()),
		           linphone_core_get_identity(marie.getLc()));
		LinphoneCall *laure_call = linphone_core_get_call_by_remote_address2(laure.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(laure_call);
		if (laure_call) {
			linphone_call_terminate(laure_call);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd,
		                             laure_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased,
		                             laure_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 1, int, "%d");

		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd,
		                              marie_stat.number_of_LinphoneCallEnd + 1, (1000 * nortp_timeout)));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased,
		                              marie_stat.number_of_LinphoneCallReleased + 1, (1000 * nortp_timeout)));

		focus_stat = focus.getStats();

		int participant_call_ended = 1;

		auto remaining_members = members;

		for (auto mgr : members) {
			stats focus_stat2 = focus.getStats();
			remaining_members.pop_front();
			std::list<stats> remaining_members_stats;
			int nb_members = 0;
			for (auto remaining_mgr : remaining_members) {
				remaining_members_stats.push_back(remaining_mgr->stat);
				LinphoneCall *pcall =
				    linphone_core_get_call_by_remote_address2(remaining_mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
					if (linphone_call_params_video_enabled(call_cparams)) {
						nb_members++;
					}
				}
			}
			const bctbx_list_t *participant_calls = linphone_core_get_calls(mgr->lc);
			int participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 1, int, "%d");

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
			                             participant_call_ended, liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

			participant_calls = linphone_core_get_calls(mgr->lc);
			participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

			for (auto remaining_mgr : remaining_members) {
				stats stat = remaining_members_stats.front();
				remaining_members_stats.pop_front();
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participants_removed,
				                             stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participant_devices_removed,
				                             stat.number_of_participant_devices_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat2.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat2.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			unsigned int expected_call_logs = 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), expected_call_logs, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), expected_call_logs, unsigned int,
				                "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info2 =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
				LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, FALSE);
			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void organizer_schedule_two_conferences(void) {
	two_overlapping_conferences_base(TRUE, FALSE);
}

static void two_overlapping_scheduled_conferences_from_different_organizers(void) {
	two_overlapping_conferences_base(FALSE, FALSE);
}

static void create_conference_with_audio_only_participants(void) {
	create_conference_with_audio_only_participants_base(LinphoneConferenceSecurityLevelNone);
}

static void create_conference_with_codec_mismatch_base(bool_t organizer_codec_mismatch) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		LinphoneConferenceLayout layout = LinphoneConferenceLayoutGrid;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionSRTP);
			}

			enable_stun_in_mgr(mgr, TRUE, TRUE, TRUE, FALSE);

			if ((organizer_codec_mismatch && (mgr == marie.getCMgr())) ||
			    (!organizer_codec_mismatch && (mgr == michelle.getCMgr()))) {
				disable_all_audio_codecs_except_one(mgr->lc, "pcmu", -1);
			} else {
				disable_all_audio_codecs_except_one(mgr->lc, "pcma", -1);
			}

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		time_t start_time = ms_time(NULL);
		time_t end_time = -1;
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		participantList.insert(
		    std::make_pair(marie.getCMgr(), add_participant_info_to_list(&participants_info, marie.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleListener, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		std::list<LinphoneCoreManager *> codec_mismatch_members;
		if (organizer_codec_mismatch) {
			codec_mismatch_members.push_back(marie.getCMgr());
		} else {
			codec_mismatch_members.push_back(michelle.getCMgr());
		}

		for (const auto &m : codec_mismatch_members) {
			auto itConferenceMgrs = std::find(conferenceMgrs.begin(), conferenceMgrs.end(), m);
			if (itConferenceMgrs != conferenceMgrs.end()) {
				conferenceMgrs.erase(itConferenceMgrs);
			}

			auto itParticipants = std::find(participants.begin(), participants.end(), m);
			if (itParticipants != participants.end()) {
				participants.erase(itParticipants);
			}

			auto itMembers = std::find(members.begin(), members.end(), m);
			if (itMembers != members.end()) {
				members.erase(itMembers);
			}

			participantList.erase(m);
		}

		for (auto [mgr, info] : participantList) {
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
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));

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
		int focus_no_streams_running = 9;
		// Update to end ICE negotiations
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// Update to add to conference.
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
		if (organizer_codec_mismatch) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
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

				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), members.size(), size_t, "%zu");
				for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
					BC_ASSERT_PTR_NOT_NULL(d);
					if (d) {
						BC_ASSERT_TRUE(!!!linphone_participant_device_get_is_muted(d));
					}
				}
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
					LinphoneParticipantRole role =
					    linphone_participant_get_role(linphone_conference_get_me(pconference));
					if ((role != LinphoneParticipantRoleListener) && (layout != LinphoneConferenceLayoutGrid)) {
						BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
					}

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);

					size_t no_streams_audio = 1;
					size_t no_active_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_active_streams_video =
						    static_cast<int>(compute_no_video_streams(enabled, pcall, pconference));
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams),
						                enabled && (no_active_streams_video > 0), int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
						                enabled && (no_active_streams_video > 0), int, "%0d");
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
						                enabled && (no_active_streams_video > 0), int, "%0d");
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
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		for (auto mgr : members) {
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
		                             focus_stat.number_of_participants_removed + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                  focus_stat.number_of_participant_devices_removed + static_cast<int>(members.size()),
		                  liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
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

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_conference_with_organizer_codec_mismatch(void) {
	create_conference_with_codec_mismatch_base(TRUE);
}

static void create_conference_with_participant_codec_mismatch(void) {
	create_conference_with_codec_mismatch_base(FALSE);
}

static void create_conference_with_server_restart_base(bool_t organizer_first) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = ms_time(NULL) + 5;
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "London Pub";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		participantList.insert(
		    std::make_pair(marie.getCMgr(), add_participant_info_to_list(&participants_info, marie.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);

		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		coresList = bctbx_list_remove(coresList, focus.getLc());
		// Restart flexisip
		focus.reStart();

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [] { return false; });

		LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
		linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
		linphone_core_set_video_activation_policy(focus.getLc(), pol);
		linphone_video_activation_policy_unref(pol);

		linphone_core_enable_video_capture(focus.getLc(), TRUE);
		linphone_core_enable_video_display(focus.getLc(), TRUE);

		coresList = bctbx_list_append(coresList, focus.getLc());

		LinphoneCoreManager *first_to_join = NULL;
		std::list<LinphoneCoreManager *> other_members{pauline.getCMgr()};
		if (organizer_first) {
			first_to_join = marie.getCMgr();
			other_members.push_back(laure.getCMgr());
		} else {
			first_to_join = laure.getCMgr();
			other_members.push_back(marie.getCMgr());
		}

		ms_message("First participant %s is calling conference %s", linphone_core_get_identity(first_to_join->lc),
		           conference_address_str);
		LinphoneCallParams *first_part_new_params = linphone_core_create_call_params(first_to_join->lc, nullptr);
		linphone_core_invite_address_with_params_2(first_to_join->lc, confAddr, first_part_new_params, NULL, nullptr);
		linphone_call_params_unref(first_part_new_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &first_to_join->stat.number_of_LinphoneCallOutgoingProgress, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &first_to_join->stat.number_of_LinphoneCallStreamsRunning, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : other_members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			ms_message("%s is calling conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

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
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 6;
		// Update to end ICE negotiations
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// Update to add to conference.
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
		                            confAddr, TRUE);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

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
				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 3, size_t, "%zu");
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);

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

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);

					size_t no_streams_audio = 0;
					size_t no_streams_video = (enabled) ? 4 : 0;
					size_t no_active_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						no_active_streams_video = compute_no_video_streams(enabled, pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
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
						_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
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

		const int total_marie_calls =
		    marie.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(marie.getLc()));
		const int total_focus_calls =
		    focus.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(focus.getLc()));
		const int total_pauline_calls = pauline.getStats().number_of_LinphoneCallEnd +
		                                (int)bctbx_list_size(linphone_core_get_calls(pauline.getLc()));
		const int total_laure_calls =
		    laure.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(laure.getLc()));

		linphone_core_terminate_all_calls(pauline.getLc());
		linphone_core_terminate_all_calls(laure.getLc());
		linphone_core_terminate_all_calls(marie.getLc());

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, total_marie_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, total_laure_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, total_focus_calls, 40000));

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, total_marie_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, total_laure_calls, 30000));

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             (mgr == focus.getCMgr()) ? 3 : 1, liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
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

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_conference_with_server_restart_organizer_first(void) {
	create_conference_with_server_restart_base(TRUE);
}

static void create_conference_with_server_restart_participant_first(void) {
	create_conference_with_server_restart_base(FALSE);
}

static void create_simple_conference_with_update_deferred(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);

		setup_conference_info_cbs(marie.getCMgr());

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutGrid);
			}
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		stats marie_stat = marie.getStats();
		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = ms_time(NULL) + 5;
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(5), [] { return false; });

		for (auto mgr : members) {
			ms_message("%s is calling conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 3,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
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
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		// Wait a little bit
		wait_for_list(coresList, NULL, 0, 3000);

		LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);

		if (pauline_call) {
			const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}
		Address paulineIdentity = pauline.getIdentity();
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineIdentity.toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		if (focus_call) {
			const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}

		LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
		LinphoneConference *paulineConference =
		    linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		linphone_address_unref(paulineUri);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		linphone_config_set_int(linphone_core_get_config(focus.getLc()), "sip", "defer_update_default", TRUE);

		for (int i = 0; i < 4; i++) {
			set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), members, confAddr, enable,
			                                 LinphoneMediaDirectionSendRecv, !enable, LinphoneMediaDirectionSendRecv);

			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			enable = !enable;
		}

		linphone_config_set_int(linphone_core_get_config(focus.getLc()), "sip", "defer_update_default", FALSE);

		if (paulineConference) {

			stats focus_stat2 = focus.getStats();
			stats marie_stat2 = marie.getStats();
			stats pauline_stat2 = pauline.getStats();
			stats laure_stat2 = laure.getStats();

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			bool_t video_enabled = FALSE;
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				video_enabled = linphone_call_params_video_enabled(call_cparams);
			}

			linphone_conference_leave(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing,
			                             pauline_stat2.number_of_LinphoneCallPausing + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused,
			                             pauline_stat2.number_of_LinphoneCallPaused + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote,
			                             focus_stat2.number_of_LinphoneCallPausedByRemote + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
			                             focus_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold,
			                             laure_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &laure.getStats().number_of_participant_devices_media_capability_changed,
			                             laure_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold,
			                             marie_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));

			linphone_conference_enter(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming,
			                             pauline_stat2.number_of_LinphoneCallResuming + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
			                             pauline_stat2.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat2.number_of_participant_devices_joined + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined,
			                             laure_stat2.number_of_participant_devices_joined + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &laure.getStats().number_of_participant_devices_media_capability_changed,
			                             laure_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
			                             marie_stat2.number_of_participant_devices_joined + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));

			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
			}
		}

		std::list<LinphoneCoreManager *> mgrsToRemove{pauline.getCMgr()};
		mgrsToRemove.push_back(laure.getCMgr());

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

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted,
		                marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

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

			if (end_time > 0) {
				time_t now = ms_time(NULL);
				time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
				if (time_left > 0) {
					// wait for the conference to end
					CoreManagerAssert({focus, marie, pauline, laure, michelle})
					    .waitUntil(chrono::seconds((time_left + 1)), [] { return false; });
				}
			}

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
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
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

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void change_active_speaker(void) {
	change_active_speaker_base(false);
}

static void one_participant_conference_toggles_video_grid(void) {
	create_one_participant_conference_toggle_video_base(LinphoneConferenceLayoutGrid, FALSE, FALSE);
}

static void one_participant_conference_toggles_video_active_speaker(void) {
	create_one_participant_conference_toggle_video_base(LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE);
}

static void create_scheduled_conference_with_active_call(void) {
	create_conference_with_active_call_base(FALSE);
}

static void conference_with_participant_added_outside_valid_time_slot(bool_t before_start) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		time_t start_time = (time_t)-1;
		time_t end_time = (time_t)-1;

		if (before_start) {
			start_time = ms_time(NULL) + 600;
		} else {
			start_time = ms_time(NULL) - 60;
		}
		end_time = start_time + 62;
		const char *initialSubject = "Colleagues";
		const char *description = "Tom Black";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, FALSE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(3), [] { return false; });

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		LinphoneReason reason = (before_start) ? LinphoneReasonForbidden : LinphoneReasonNotFound;
		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallError, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			const bctbx_list_t *logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(logs), 1, size_t, "%zu");
			for (const bctbx_list_t *itlog = logs; itlog; itlog = bctbx_list_next(itlog)) {
				auto log = (LinphoneCallLog *)bctbx_list_get_data(itlog);
				BC_ASSERT_EQUAL(linphone_error_info_get_reason(linphone_call_log_get_error_info(log)), reason, int,
				                "%d");
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void conference_with_participants_added_after_end(void) {
	conference_with_participant_added_outside_valid_time_slot(FALSE);
}

static void conference_with_participants_added_before_start(void) {
	conference_with_participant_added_outside_valid_time_slot(TRUE);
}

void uninvited_participant_rejoins(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()};

		time_t start_time = ms_time(NULL);
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Weekly recap";
		const char *description = "What happened in the past week";
		LinphoneMediaDirection video_direction = LinphoneMediaDirectionInactive;
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		auto members = participants;
		members.push_back(marie.getCMgr());
		auto conferenceMgrs = members;
		conferenceMgrs.push_back(focus.getCMgr());

		update_sequence_number(&participants_info, {}, 0, -1);

		if (confAddr) {
			for (auto mgr : members) {
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info,
				                            start_time, duration, initialSubject, description, 0,
				                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, TRUE, FALSE);

				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				linphone_call_params_set_video_direction(new_params, video_direction);
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
			}

			for (auto mgr : members) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));

		LinphoneConference *oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference),
			                static_cast<int>(participants.size()), int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), (participants.size() + 1), size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
			}
		}

		add_participant_info_to_list(&participants_info, marie.getCMgr()->identity, LinphoneParticipantRoleSpeaker, 0);
		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			// Try search in the databse removing the gr parameter
			LinphoneAddress *confAddr2 = linphone_address_clone(confAddr);
			linphone_address_remove_uri_param(confAddr2, "gr");
			check_conference_info_in_db(mgr, NULL, confAddr2, marie.getCMgr()->identity, participants_info, start_time,
			                            duration, initialSubject, description, 0, LinphoneConferenceInfoStateNew,
			                            security_level, FALSE, TRUE, TRUE, FALSE);
			linphone_address_unref(confAddr2);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info);
				}
			}
		}

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                  focus_stat.number_of_LinphoneCallStreamsRunning + static_cast<int>(participants.size() + 1),
		                  liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             marie_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                  focus_stat.number_of_LinphoneSubscriptionIncomingReceived + static_cast<int>(members.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, FALSE);

		ms_message("%s is dialing conference %s", linphone_core_get_identity(berthe.getLc()),
		           Address::toCpp(confAddr)->toString().c_str());
		stats berthe_stat = berthe.getStats();
		conferenceMgrs.push_back(berthe.getCMgr());
		members.push_back(berthe.getCMgr());
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleListener, 0)));

		LinphoneCallParams *berthe_new_params = linphone_core_create_call_params(berthe.getLc(), nullptr);
		linphone_call_params_set_video_direction(berthe_new_params, video_direction);
		linphone_core_invite_address_with_params_2(berthe.getLc(), confAddr, berthe_new_params, NULL, nullptr);
		linphone_call_params_unref(berthe_new_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
		                             berthe_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive,
		                             berthe_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyFullStateReceived,
		                             berthe_stat.number_of_NotifyFullStateReceived + 1,
		                             liblinphone_tester_sip_timeout));

		memberList = fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, FALSE);

		focus_stat = focus.getStats();
		berthe_stat = berthe.getStats();
		LinphoneCall *berthe_call = linphone_core_get_call_by_remote_address2(berthe.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(berthe_call);
		if (berthe_call) {
			ms_message("%s is terminating call with %s", linphone_core_get_identity(berthe.getLc()),
			           linphone_core_get_identity(focus.getLc()));
			linphone_call_terminate(berthe_call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &berthe.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(berthe.getLc(), NULL, berthe.getIdentity().toC(), confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

			for (auto mgr : members) {
				if (mgr != berthe.getCMgr()) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participants_removed, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_removed, 1,
					                             liblinphone_tester_sip_timeout));
				}
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));

		ms_message("%s rejoins conference %s", linphone_core_get_identity(berthe.getLc()),
		           Address::toCpp(confAddr)->toString().c_str());
		berthe_stat = berthe.getStats();
		berthe_new_params = linphone_core_create_call_params(berthe.getLc(), nullptr);
		linphone_call_params_set_video_direction(berthe_new_params, video_direction);
		linphone_core_invite_address_with_params_2(berthe.getLc(), confAddr, berthe_new_params, NULL, nullptr);
		linphone_call_params_unref(berthe_new_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
		                             berthe_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive,
		                             berthe_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyFullStateReceived,
		                             berthe_stat.number_of_NotifyFullStateReceived + 1,
		                             liblinphone_tester_sip_timeout));

		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, FALSE);

		focus_stat = focus.getStats();
		for (auto mgr : members) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
				int call_ended = (mgr == berthe.getCMgr()) ? 2 : 1;
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, call_ended,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, call_ended,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, call_ended,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             call_ended, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
				                             call_ended, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, call_ended,
				                             liblinphone_tester_sip_timeout));

				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 5,
		                             liblinphone_tester_sip_timeout));

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_in_sfu_payload_mode(void) {
	char *pauline_recordpath = bc_tester_file("record-local_scheduled_conference_sfu_pauline.wav");
	char *marie_recordpath = bc_tester_file("record-local_scheduled_conference_sfu_marie.wav");
	char *soundpath = bc_tester_res("sounds/ahbahouaismaisbon.wav");

	time_t start_time = ms_time(nullptr);

	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());

		linphone_core_set_record_file(marie.getCMgr()->lc, marie_recordpath);
		linphone_core_set_record_file(pauline.getCMgr()->lc, pauline_recordpath);
		linphone_core_set_use_files(marie.getCMgr()->lc, TRUE);
		linphone_core_set_use_files(pauline.getCMgr()->lc, TRUE);
		linphone_core_set_play_file(marie.getCMgr()->lc, soundpath);
		linphone_core_set_play_file(pauline.getCMgr()->lc, soundpath);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = nullptr;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()}) {

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutGrid);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionNone);
			}

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		// Focus is in payload mode: transfer payload, no mixer
		LinphoneConfig *focus_config = linphone_core_get_config(focus.getLc());
		linphone_config_set_int(focus_config, "sound", "conference_mode",
		                        static_cast<int>(MSConferenceModeRouterPayload));
		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr()};

		time_t end_time = -1;
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = nullptr;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;

		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(
		    std::make_pair(marie.getCMgr(), add_participant_info_to_list(&participants_info, marie.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, LinphoneConferenceSecurityLevelNone, TRUE, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:unknown");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		// Each member call the conference
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_enable_video(new_params, FALSE);
			linphone_call_params_set_media_encryption(new_params, LinphoneMediaEncryptionNone);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, nullptr, nullptr);
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
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
				BC_ASSERT_EQUAL(pcall_enc, LinphoneMediaEncryptionNone, int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, LinphoneMediaEncryptionNone, int, "%d");
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 2,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 4;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 2),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 2, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 2,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline}, conferenceMgrs, focus.getCMgr(), memberList, confAddr,
		                            false);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait to know if the no RTP timeout is triggered
		CoreManagerAssert({focus, marie, pauline})
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
				                (int)LinphoneConferenceSecurityLevelNone, int, "%0d");

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

						// Check that the we are correctly linked with the PacketRouter and that it is in payload
						// mode
						auto *audioStream = linphone_call_get_stream(ccall, LinphoneStreamTypeAudio);
						BC_ASSERT_PTR_NOT_NULL(audioStream);
						if (auto *q = audioStream->rtprecv->outputs[0]; BC_ASSERT_PTR_NOT_NULL(q)) {
							if (auto *next = q->next.filter; BC_ASSERT_PTR_NOT_NULL(next)) {
								if (BC_ASSERT_TRUE(!strcmp(next->desc->name, "MSPacketRouter"))) {
									bool_t enabled;
									ms_filter_call_method(next, MS_PACKET_ROUTER_GET_FULL_PACKET_MODE_ENABLED,
									                      &enabled);
									BC_ASSERT_FALSE(enabled);
								}
							}
						}
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
					auto p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 2 : 1), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 2, size_t, "%zu");
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

		if (end_time > 0) {
			time_t now = ms_time(NULL);
			time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
			if (time_left > 0) {
				// wait for the conference to end
				CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds((time_left + 1)), [] {
					return false;
				});
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		std::list<LinphoneCoreManager *> allMembers{marie.getCMgr(), pauline.getCMgr()};
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
					auto call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		bctbx_list_free(coresList);

		BC_ASSERT_PTR_NOT_NULL(marie_recordpath);
		BC_ASSERT_PTR_NOT_NULL(pauline_recordpath);

		liblinphone_tester_check_recorded_audio(soundpath, pauline_recordpath);
		liblinphone_tester_check_recorded_audio(soundpath, marie_recordpath);

		unlink(marie_recordpath);
		unlink(pauline_recordpath);
	}

	bc_free(pauline_recordpath);
	bc_free(marie_recordpath);
	bc_free(soundpath);
}

} // namespace LinphoneTest

static test_t local_conference_scheduled_conference_basic_tests[] = {
    TEST_NO_TAG("Call to inexisting conference address", LinphoneTest::call_to_inexisting_conference_address),
    TEST_NO_TAG("Create conference on unresponsive server", LinphoneTest::create_conference_on_unresponsive_server),
    TEST_NO_TAG("Create simple conference", LinphoneTest::create_simple_conference),
    TEST_NO_TAG("Create simple conference using DB conference scheduler",
                LinphoneTest::create_simple_conference_db_conference_scheduler),
    TEST_NO_TAG("Create simple conference using DB conference scheduler with server restart",
                LinphoneTest::create_simple_conference_db_conference_scheduler_server_restart),
    TEST_NO_TAG("Schedule simple conference using DB conference scheduler",
                LinphoneTest::schedule_simple_conference_db_conference_scheduler),
    //    TEST_NO_TAG("Create conference with only listeners and uninvited participant",
    //                LinphoneTest::create_conference_with_only_listeners_and_uninvited_participant),
    TEST_NO_TAG("Create conference with only speakers and uninvited participant",
                LinphoneTest::create_conference_with_only_speakers_and_uninvited_participant),
    TEST_NO_TAG("Create conference with uninvited participant",
                LinphoneTest::create_conference_with_uninvited_participant),
    TEST_NO_TAG("Create simple conference with server restart",
                LinphoneTest::create_simple_conference_with_server_restart),
    TEST_NO_TAG("Create simple conference with client restart",
                LinphoneTest::create_simple_conference_with_client_restart),
    TEST_NO_TAG("Create conference with late participant addition",
                LinphoneTest::create_conference_with_late_participant_addition),
    TEST_NO_TAG("Organizer schedules 2 conferences", LinphoneTest::organizer_schedule_two_conferences),
    TEST_NO_TAG("Create simple point-to-point encrypted conference",
                LinphoneTest::create_simple_point_to_point_encrypted_conference),
    TEST_NO_TAG("Create conference starting immediately", LinphoneTest::create_conference_starting_immediately),
    TEST_NO_TAG("Create conference starting in the past", LinphoneTest::create_conference_starting_in_the_past),
    TEST_NO_TAG("Create conference with participant codec mismatch",
                LinphoneTest::create_conference_with_participant_codec_mismatch),
    TEST_NO_TAG("Create conference with organizer codec mismatch",
                LinphoneTest::create_conference_with_organizer_codec_mismatch)};

static test_t local_conference_scheduled_conference_advanced_tests[] = {
    TEST_NO_TAG("Create simple SRTP conference", LinphoneTest::create_simple_srtp_conference),
    TEST_NO_TAG("Create simple ZRTP conference", LinphoneTest::create_simple_zrtp_conference),
    TEST_NO_TAG("Create simple DTLS conference", LinphoneTest::create_simple_dtls_conference),
    TEST_NO_TAG("Create conference with server restart (participant first)",
                LinphoneTest::create_conference_with_server_restart_participant_first),
    TEST_NO_TAG("Create conference with server restart (organizer first)",
                LinphoneTest::create_conference_with_server_restart_organizer_first),
    TEST_NO_TAG("Create simple conference with update deferred",
                LinphoneTest::create_simple_conference_with_update_deferred),
    TEST_NO_TAG("Create conference with uninvited participant added after end",
                LinphoneTest::create_conference_with_uninvited_participant_added_after_end),
    TEST_NO_TAG("Create conference with uninvited participant not allowed",
                LinphoneTest::create_conference_with_uninvited_participant_not_allowed),
    TEST_NO_TAG("Create conference with late participant addition declined",
                LinphoneTest::create_conference_with_late_participant_addition_declined),
    TEST_NO_TAG("Conference with participants added after conference ends",
                LinphoneTest::conference_with_participants_added_after_end),
    TEST_NO_TAG("Conference with participants added before conference starts",
                LinphoneTest::conference_with_participants_added_before_start),
    TEST_NO_TAG("Create one participant conference toggles video in grid layout",
                LinphoneTest::one_participant_conference_toggles_video_grid),
    TEST_NO_TAG("Create one participant conference toggles video in active speaker layout",
                LinphoneTest::one_participant_conference_toggles_video_active_speaker),
    TEST_NO_TAG("2 overlapping conferences from different organizers",
                LinphoneTest::two_overlapping_scheduled_conferences_from_different_organizers),
    TEST_NO_TAG("Create scheduled conference with active call",
                LinphoneTest::create_scheduled_conference_with_active_call),
    TEST_NO_TAG("Change active speaker", LinphoneTest::change_active_speaker)};

static test_t local_conference_scheduled_conference_audio_only_participant_tests[] = {
    TEST_NO_TAG("Create conference with audio only and uninvited participant",
                LinphoneTest::create_conference_with_audio_only_and_uninvited_participant),
    TEST_NO_TAG("Create simple conference with audio only participant enabling video",
                LinphoneTest::create_simple_conference_with_audio_only_participant_enabling_video),
    TEST_NO_TAG("Create simple conference with audio only participant",
                LinphoneTest::create_simple_conference_with_audio_only_participant),
    TEST_NO_TAG("Uninvited participant rejoins", LinphoneTest::uninvited_participant_rejoins),
    TEST_NO_TAG("Create conference with audio only participants",
                LinphoneTest::create_conference_with_audio_only_participants),
    TEST_NO_TAG("Create simple audio conference in SFU payload mode",
                LinphoneTest::create_simple_conference_in_sfu_payload_mode)};

static test_t local_conference_scheduled_conference_with_screen_sharing_tests[] = {
    TEST_NO_TAG("Create simple conference with screen sharing",
                LinphoneTest::create_simple_conference_with_screen_sharing),
    TEST_NO_TAG("Participant joins simple conference with screen sharing",
                LinphoneTest::participant_joins_simple_conference_with_screen_sharing),
    TEST_NO_TAG("Create simple conference with screen sharing override",
                LinphoneTest::create_simple_conference_with_screen_sharing_override),
    TEST_NO_TAG("Create simple conference with screen sharing turned off",
                LinphoneTest::create_simple_conference_with_screen_sharing_turn_off),
    TEST_NO_TAG("Create simple conference with screen sharing override and no camera",
                LinphoneTest::create_simple_conference_with_screen_sharing_override_no_camera),
    TEST_NO_TAG("Create simple conference with screen sharing and grid layout",
                LinphoneTest::create_simple_conference_with_screen_sharing_grid_layout),
    TEST_NO_TAG("Create simple conference with screen sharing and no video",
                LinphoneTest::create_simple_conference_with_screen_sharing_no_video),
    TEST_NO_TAG("Conference with screen sharing enabled since the start",
                LinphoneTest::conference_with_screen_sharing_enabled_since_the_start),
    TEST_NO_TAG("Conference with two participants having screen sharing enabled since the start",
                LinphoneTest::conference_with_two_participant_having_screen_sharing_enabled_since_the_start),
    TEST_NO_TAG("Conference with screen sharing participant only",
                LinphoneTest::conference_with_screen_sharing_participant_only),
    TEST_NO_TAG("Create simple conference with participant ask to screenshare without video send component",
                LinphoneTest::create_simple_conference_with_screen_sharing_no_video_send_component)};

test_suite_t local_conference_test_suite_scheduled_conference_basic = {
    "Local conference tester (Scheduled Conference Basic)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_scheduled_conference_basic_tests) /
        sizeof(local_conference_scheduled_conference_basic_tests[0]),
    local_conference_scheduled_conference_basic_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};

test_suite_t local_conference_test_suite_scheduled_conference_advanced = {
    "Local conference tester (Scheduled Conference Advanced)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_scheduled_conference_advanced_tests) /
        sizeof(local_conference_scheduled_conference_advanced_tests[0]),
    local_conference_scheduled_conference_advanced_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};

test_suite_t local_conference_test_suite_scheduled_conference_audio_only_participant = {
    "Local conference tester (Audio only participants)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_scheduled_conference_audio_only_participant_tests) /
        sizeof(local_conference_scheduled_conference_audio_only_participant_tests[0]),
    local_conference_scheduled_conference_audio_only_participant_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};

test_suite_t local_conference_test_suite_scheduled_conference_with_screen_sharing = {
    "Local conference tester (Screen sharing)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_scheduled_conference_with_screen_sharing_tests) /
        sizeof(local_conference_scheduled_conference_with_screen_sharing_tests[0]),
    local_conference_scheduled_conference_with_screen_sharing_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};
