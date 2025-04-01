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

#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-conference-scheduler.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "linphone/chat.h"
#include "local-conference-tester-functions.h"

namespace LinphoneTest {

static void edit_and_cancel_simple_conference_db_conference_scheduler(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = FALSE;

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference lise("lise_rc", focus.getConferenceFactoryAddress(), enable_lime);

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
		time_t start_time = ms_time(NULL) + 60;
		int duration = 30;
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
		conf_info = NULL;

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

		conf_info = linphone_core_find_conference_information_from_uri(focus.getLc(), conference_address);
		BC_ASSERT_PTR_NOT_NULL(conf_info);
		if (conf_info) {
			ms_message("%s is trying to update conference %s - adding %s", linphone_core_get_identity(focus.getLc()),
			           conference_address_str, linphone_core_get_identity(lise.getLc()));

			LinphoneParticipantRole role = LinphoneParticipantRoleListener;
			focus_stat = focus.getStats();
			linphone_conference_info_set_subject(conf_info, subject);
			linphone_conference_info_set_description(conf_info, description);
			LinphoneParticipantInfo *lise_participant_info =
			    add_participant_info_to_list(&participants_info, lise.getCMgr()->identity, role, -1);
			linphone_conference_info_add_participant_2(conf_info, lise_participant_info);
			participantList.insert(std::make_pair(lise.getCMgr(), lise_participant_info));

			LinphoneParticipantRole current_pauline_role =
			    linphone_participant_info_get_role(participantList[pauline.getCMgr()]);
			LinphoneParticipantRole new_pauline_role = (current_pauline_role == LinphoneParticipantRoleListener)
			                                               ? LinphoneParticipantRoleSpeaker
			                                               : LinphoneParticipantRoleListener;
			ms_message("%s is trying to update conference %s - changing role of %s from %s to %s",
			           linphone_core_get_identity(focus.getLc()), conference_address_str,
			           linphone_core_get_identity(pauline.getLc()),
			           linphone_participant_role_to_string(current_pauline_role),
			           linphone_participant_role_to_string(new_pauline_role));
			const LinphoneParticipantInfo *old_pauline_partipant_info =
			    linphone_conference_info_find_participant(conf_info, pauline.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(old_pauline_partipant_info);
			if (old_pauline_partipant_info) {
				LinphoneParticipantInfo *pauline_participant_info =
				    linphone_participant_info_clone(old_pauline_partipant_info);
				linphone_participant_info_set_role(pauline_participant_info, new_pauline_role);
				linphone_conference_info_update_participant(conf_info, pauline_participant_info);

				linphone_participant_info_set_role(participantList[pauline.getCMgr()], new_pauline_role);
				const bctbx_list_t *participant_info_it = bctbx_list_find_custom(
				    participants_info, (int (*)(const void *, const void *))find_matching_participant_info,
				    pauline_participant_info);
				BC_ASSERT_PTR_NOT_NULL(participant_info_it);
				if (participant_info_it) {
					LinphoneParticipantInfo *participant_info_found =
					    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_it);
					linphone_participant_info_set_role(participant_info_found, new_pauline_role);
				}
				linphone_participant_info_unref(pauline_participant_info);
			}

			conference_scheduler = linphone_core_create_db_conference_scheduler(
			    focus.getLc(), linphone_core_get_default_account(focus.getLc()));
			LinphoneConferenceSchedulerCbs *cbs =
			    linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
			linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
			linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
			linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
			linphone_conference_scheduler_cbs_unref(cbs);
			linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
			linphone_conference_info_unref(conf_info);
			conf_info = NULL;

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateUpdating,
			                             focus_stat.number_of_ConferenceSchedulerStateUpdating + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateReady,
			                             focus_stat.number_of_ConferenceSchedulerStateReady + 1,
			                             liblinphone_tester_sip_timeout));

			linphone_conference_scheduler_unref(conference_scheduler);
			conference_scheduler = NULL;
		}

		check_conference_info_in_db(focus.getCMgr(), NULL, conference_address, organizer_address, participants_info,
		                            start_time, duration, subject, description, 1, LinphoneConferenceInfoStateUpdated,
		                            security_level, FALSE, TRUE, TRUE, FALSE);

		conf_info = linphone_core_find_conference_information_from_uri(focus.getLc(), conference_address);
		BC_ASSERT_PTR_NOT_NULL(conf_info);
		if (conf_info) {
			ms_message("%s is trying to cancel conference %s", linphone_core_get_identity(focus.getLc()),
			           conference_address_str);

			focus_stat = focus.getStats();
			conference_scheduler = linphone_core_create_db_conference_scheduler(
			    focus.getLc(), linphone_core_get_default_account(focus.getLc()));
			LinphoneConferenceSchedulerCbs *cbs =
			    linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
			linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
			linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
			linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
			linphone_conference_scheduler_cbs_unref(cbs);
			linphone_conference_scheduler_cancel_conference(conference_scheduler, conf_info);
			linphone_conference_info_unref(conf_info);
			conf_info = NULL;

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateUpdating,
			                             focus_stat.number_of_ConferenceSchedulerStateUpdating + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_ConferenceSchedulerStateReady,
			                             focus_stat.number_of_ConferenceSchedulerStateReady + 1,
			                             liblinphone_tester_sip_timeout));

			if (participants_info) {
				bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
				participants_info = NULL;
			}
			linphone_conference_scheduler_unref(conference_scheduler);
			conference_scheduler = NULL;
		}

		check_conference_info_in_db(focus.getCMgr(), NULL, conference_address, organizer_address, participants_info,
		                            start_time, duration, subject, description, 2, LinphoneConferenceInfoStateCancelled,
		                            security_level, FALSE, TRUE, TRUE, FALSE);

	end:
		if (conference_address) linphone_address_unref(conference_address);
		if (conference_address_str) ms_free(conference_address_str);
		if (conference_scheduler) linphone_conference_scheduler_unref(conference_scheduler);
		bctbx_list_free(coresList);
		if (participants_info)
			bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	}
}

static void edit_simple_conference_base(bool_t from_organizer,
                                        bool_t use_default_account,
                                        bool_t enable_bundle_mode,
                                        bool_t join,
                                        bool_t enable_encryption,
                                        bool_t server_restart,
                                        LinphoneConferenceSecurityLevel security_level,
                                        bool_t role_changed,
                                        bool_t enable_chat) {

	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference lise("lise_rc", focus.getConferenceFactoryAddress());

		LinphoneCoreManager *manager_editing = (from_organizer) ? marie.getCMgr() : laure.getCMgr();
		linphone_core_enable_rtp_bundle(manager_editing->lc, enable_bundle_mode);
		int default_index =
		    linphone_config_get_int(linphone_core_get_config(manager_editing->lc), "sip", "default_proxy", 0);
		LinphoneAccountParams *params = linphone_account_params_new_with_config(manager_editing->lc, default_index);
		LinphoneAddress *alternative_address = linphone_address_new("sip:toto@sip.linphone.org");
		linphone_account_params_set_identity_address(params, alternative_address);
		LinphoneAccount *new_account = linphone_account_new(manager_editing->lc, params);
		linphone_core_add_account(manager_editing->lc, new_account);
		linphone_account_params_unref(params);
		linphone_account_unref(new_account);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(lise);

		linphone_core_enable_gruu_in_conference_address(focus.getLc(), TRUE);

		setup_conference_info_cbs(marie.getCMgr());
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

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

			if ((mgr != focus.getCMgr()) && enable_encryption) {
				linphone_config_set_int(linphone_core_get_config(mgr->lc), "rtp", "accept_any_encryption", 1);
				linphone_core_set_media_encryption_mandatory(mgr->lc, TRUE);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionZRTP);
			}
		}

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, lise.getLc());

		std::list<LinphoneCoreManager *> invitedParticipants{pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = time(NULL) + 600; // Start in 10 minutes
		int duration = 60;                    // minutes
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: <S-F12><S-F11><S-F6> £$%§";
		const char *description = "Testing characters";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : invitedParticipants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, enable_chat, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		char *uid = NULL;
		unsigned int sequence = 0;
		LinphoneConferenceInfo *conf_info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(conf_info);
		if (conf_info) {
			uid = ms_strdup(linphone_conference_info_get_ics_uid(conf_info));
			BC_ASSERT_PTR_NOT_NULL(uid);
			sequence = linphone_conference_info_get_ics_sequence(conf_info);

			ms_message("%s is trying to update conference %s - adding %s", linphone_core_get_identity(marie.getLc()),
			           conference_address_str, linphone_core_get_identity(lise.getLc()));

			participants.push_back(lise.getCMgr());

			LinphoneParticipantRole role = LinphoneParticipantRoleListener;
			stats focus_stat = focus.getStats();
			stats marie_stat = marie.getStats();
			start_time = time(NULL) + 5; // Start in 5 seconds
			duration = 1;                // minutes
			end_time = (start_time + duration * 60);
			linphone_conference_info_set_date_time(conf_info, start_time);
			linphone_conference_info_set_duration(conf_info, duration);
			linphone_conference_info_set_subject(conf_info, initialSubject);
			linphone_conference_info_set_description(conf_info, description);
			LinphoneParticipantInfo *lise_participant_info =
			    add_participant_info_to_list(&participants_info, lise.getCMgr()->identity, role, -1);
			linphone_conference_info_add_participant_2(conf_info, lise_participant_info);
			participantList.insert(std::make_pair(lise.getCMgr(), lise_participant_info));

			if (role_changed) {
				LinphoneParticipantRole current_pauline_role =
				    linphone_participant_info_get_role(participantList[pauline.getCMgr()]);
				LinphoneParticipantRole new_pauline_role = (current_pauline_role == LinphoneParticipantRoleListener)
				                                               ? LinphoneParticipantRoleSpeaker
				                                               : LinphoneParticipantRoleListener;
				ms_message("%s is trying to update conference %s - changing role of %s from %s to %s",
				           linphone_core_get_identity(marie.getLc()), conference_address_str,
				           linphone_core_get_identity(pauline.getLc()),
				           linphone_participant_role_to_string(current_pauline_role),
				           linphone_participant_role_to_string(new_pauline_role));
				const LinphoneParticipantInfo *old_pauline_partipant_info =
				    linphone_conference_info_find_participant(conf_info, pauline.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(old_pauline_partipant_info);
				if (old_pauline_partipant_info) {
					LinphoneParticipantInfo *pauline_participant_info =
					    linphone_participant_info_clone(old_pauline_partipant_info);
					linphone_participant_info_set_role(pauline_participant_info, new_pauline_role);
					linphone_conference_info_update_participant(conf_info, pauline_participant_info);

					linphone_participant_info_set_role(participantList[pauline.getCMgr()], new_pauline_role);
					const bctbx_list_t *participant_info_it = bctbx_list_find_custom(
					    participants_info, (int (*)(const void *, const void *))find_matching_participant_info,
					    pauline_participant_info);
					BC_ASSERT_PTR_NOT_NULL(participant_info_it);
					if (participant_info_it) {
						LinphoneParticipantInfo *participant_info_found =
						    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_it);
						linphone_participant_info_set_role(participant_info_found, new_pauline_role);
					}
					linphone_participant_info_unref(pauline_participant_info);
				}
			}

			const auto ics_participant_number = 3;
			const bctbx_list_t *ics_participants = linphone_conference_info_get_participant_infos(conf_info);
			BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), ics_participant_number, size_t, "%zu");

			BC_ASSERT_TRUE(linphone_conference_info_get_capability(conf_info, LinphoneStreamTypeAudio));
			BC_ASSERT_TRUE(linphone_conference_info_get_capability(conf_info, LinphoneStreamTypeVideo));
			BC_ASSERT_EQUAL(linphone_conference_info_get_capability(conf_info, LinphoneStreamTypeText), enable_chat,
			                int, "%0d");

			std::list<stats> participant_stats;
			for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), lise.getCMgr()}) {
				participant_stats.push_back(mgr->stat);
			}

			LinphoneConferenceScheduler *conference_scheduler =
			    linphone_core_create_sip_conference_scheduler(marie.getLc(), nullptr);
			LinphoneConferenceSchedulerCbs *cbs =
			    linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
			linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
			linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
			linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
			linphone_conference_scheduler_cbs_unref(cbs);
			linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
			                             marie_stat.number_of_ConferenceSchedulerStateUpdating + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateReady,
			                             marie_stat.number_of_ConferenceSchedulerStateReady + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneConferenceParams *chat_room_params = linphone_core_create_conference_params_2(marie.getLc(), NULL);
			linphone_conference_params_enable_chat(chat_room_params, TRUE);
			linphone_conference_params_enable_group(chat_room_params, FALSE);
			LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(chat_room_params);
			linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendBasic);
			linphone_conference_scheduler_send_invitations_2(conference_scheduler, chat_room_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerInvitationsSent,
			                             marie_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), lise.getCMgr()}) {
				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				if (mgr == focus.getCMgr()) {
					BC_ASSERT_PTR_NOT_NULL(call_logs);
					if (call_logs) {
						BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");
					}
				} else {
					BC_ASSERT_PTR_NULL(call_logs);
				}
				int exp_sequence = 0;
				if ((mgr == lise.getCMgr()) || (mgr == focus.getCMgr())) {
					exp_sequence = 0;
				} else {
					exp_sequence = (sequence + 1);
				}

				LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
				if (mgr == lise.getCMgr()) {
					exp_state = LinphoneConferenceInfoStateNew;
				} else {
					exp_state = LinphoneConferenceInfoStateUpdated;
				}
				auto old_stats = participant_stats.front();
				if ((mgr != focus.getCMgr()) && (mgr != marie.getCMgr())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
					                             old_stats.number_of_LinphoneMessageReceived + 1,
					                             liblinphone_tester_sip_timeout));
					if (!linphone_core_conference_ics_in_message_body_enabled(marie.getLc())) {
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
						                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
						                             liblinphone_tester_sip_timeout));
					}

					BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
					if (mgr->stat.last_received_chat_message != NULL) {
						const string expected = ContentType::Icalendar.getMediaType();
						BC_ASSERT_STRING_EQUAL(
						    linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
						    expected.c_str());
					}
					BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(mgr->lc)), 1, int, "%d");

					update_sequence_number(&participants_info, {lise.getCMgr()->identity},
					                       (mgr == focus.getCMgr()) ? 0 : 1, (mgr == focus.getCMgr()) ? -1 : 0);
					check_conference_info_in_db(mgr, uid, confAddr, marie.getCMgr()->identity, participants_info,
					                            start_time, duration, initialSubject, description, exp_sequence,
					                            exp_state, security_level, FALSE, TRUE, TRUE, FALSE);

					bctbx_list_t *participant_chat_room_participants =
					    bctbx_list_append(NULL, marie.getCMgr()->identity);
					LinphoneChatRoom *pcr = linphone_core_search_chat_room(mgr->lc, chat_room_params, mgr->identity,
					                                                       NULL, participant_chat_room_participants);
					bctbx_list_free(participant_chat_room_participants);
					BC_ASSERT_PTR_NOT_NULL(pcr);
					if (pcr) {
						LinphoneChatMessage *participant_msg = linphone_chat_room_get_last_message_in_history(pcr);
						BC_ASSERT_PTR_NOT_NULL(participant_msg);
						if (participant_msg) {
							const bctbx_list_t *original_contents = linphone_chat_message_get_contents(participant_msg);
							BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
							LinphoneContent *original_content =
							    (LinphoneContent *)bctbx_list_get_data(original_contents);
							if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
								LinphoneConferenceInfo *conf_info_from_original_content =
								    linphone_factory_create_conference_info_from_icalendar_content(
								        linphone_factory_get(), original_content);
								if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
									check_conference_info_against_db(mgr, confAddr, conf_info_from_original_content,
									                                 FALSE);
									linphone_conference_info_unref(conf_info_from_original_content);
								}
							}
							linphone_chat_message_unref(participant_msg);
						}
					}

					bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
					LinphoneChatRoom *cr = linphone_core_search_chat_room(
					    marie.getLc(), chat_room_params, marie.getCMgr()->identity, NULL, chat_room_participants);
					bctbx_list_free(chat_room_participants);
					BC_ASSERT_PTR_NOT_NULL(cr);
					if (cr) {
						LinphoneChatMessage *editing_participant_msg =
						    linphone_chat_room_get_last_message_in_history(cr);
						BC_ASSERT_PTR_NOT_NULL(editing_participant_msg);
						if (editing_participant_msg) {
							const bctbx_list_t *original_contents =
							    linphone_chat_message_get_contents(editing_participant_msg);
							BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
							LinphoneContent *original_content =
							    (LinphoneContent *)bctbx_list_get_data(original_contents);
							if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
								LinphoneConferenceInfo *conf_info_from_original_content =
								    linphone_factory_create_conference_info_from_icalendar_content(
								        linphone_factory_get(), original_content);
								if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
									check_conference_info_against_db(mgr, confAddr, conf_info_from_original_content,
									                                 FALSE);
									linphone_conference_info_unref(conf_info_from_original_content);
								}
							}
							linphone_chat_message_unref(editing_participant_msg);
						}
					}
				} else {
					bctbx_list_t *participants_info2 = bctbx_list_copy_with_data(
					    participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
					if (mgr == focus.getCMgr()) {
						add_participant_info_to_list(&participants_info2, marie.getCMgr()->identity,
						                             LinphoneParticipantRoleSpeaker, 0);
					}
					check_conference_info_in_db(mgr, uid, confAddr, marie.getCMgr()->identity, participants_info2,
					                            start_time, duration, initialSubject, description, exp_sequence,
					                            exp_state, security_level, TRUE, TRUE, TRUE, enable_chat);
					bctbx_list_free_with_data(participants_info2,
					                          (bctbx_list_free_func)linphone_participant_info_unref);
				}
				participant_stats.pop_front();
			}
			linphone_chat_room_params_unref(chat_room_params);
			linphone_conference_info_unref(conf_info);
			linphone_conference_scheduler_unref(conference_scheduler);
		}
		ms_free(uid);
		uid = NULL;

		if (join) {
			CoreManagerAssert({focus, marie, pauline, laure, michelle, lise}).waitUntil(chrono::seconds(5), [] {
				return false;
			});

			std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()};
			std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr()};

			stats focus_stat = focus.getStats();
			for (auto mgr : members) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
			}

			for (auto mgr : members) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
				int no_streams_running = 2;
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating,
				                             (no_streams_running - 1), liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
				                             no_streams_running, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
				                             ((mgr == marie.getCMgr()) ? 2 : 1), liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 2,
			                             liblinphone_tester_sip_timeout));
			int focus_no_streams_running = 4;
			// Update to end ICE negotiations
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                  focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 2),
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
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 2, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + 2, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat.number_of_conference_participant_devices_present + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + 2,
			                             liblinphone_tester_sip_timeout));

			bctbx_list_t *participants_info_tmp =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
			    fill_member_list(members, participantList, marie.getCMgr(), participants_info_tmp);
			wait_for_conference_streams({focus, marie, pauline, laure, michelle, lise}, conferenceMgrs, focus.getCMgr(),
			                            memberList, confAddr, TRUE);
			bctbx_list_free_with_data(participants_info_tmp, (bctbx_list_free_func)linphone_participant_info_unref);

			LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(fconference);

			// wait a bit longer to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, lise}).waitUntil(chrono::seconds(2), [] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
				LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					const LinphoneConferenceParams *conference_params =
					    linphone_conference_get_current_params(pconference);
					int no_participants = 0;
					if (start_time >= 0) {
						BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
						                (long long)start_time, long long, "%lld");
					}
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
					                (long long)end_time, long long, "%lld");

					bctbx_list_t *participant_device_list =
					    linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 2, size_t, "%zu");
					for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
						BC_ASSERT_PTR_NOT_NULL(d);
						if (d) {
							check_muted({focus, marie, pauline, laure, michelle, lise}, d, {laure.getCMgr()});
							linphone_participant_device_set_user_data(d, mgr->lc);
							LinphoneParticipantDeviceCbs *cbs =
							    linphone_factory_create_participant_device_cbs(linphone_factory_get());
							linphone_participant_device_cbs_set_is_muted(cbs, on_muted_notified);
							linphone_participant_device_add_callbacks(d, cbs);
							linphone_participant_device_cbs_unref(cbs);
						}
					}
					bctbx_list_free_with_data(participant_device_list,
					                          (void (*)(void *))linphone_participant_device_unref);

					if (mgr == focus.getCMgr()) {
						// Marie and Pauline joined the conference
						no_participants = 2;
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {
						no_participants = 1;
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
						size_t no_streams_video = 3;
						size_t no_active_streams_video = 0;
						size_t no_streams_text = 0;

						LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);
						if (pcall) {
							no_streams_audio = compute_no_audio_streams(pcall, pconference);
							no_active_streams_video = compute_no_video_streams(enabled, pcall, pconference);
							_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
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
							_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
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
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int,
					                "%0d");
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
					bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
					for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
						LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
						BC_ASSERT_TRUE(linphone_participant_is_admin(p) ==
						               linphone_address_weak_equal(linphone_participant_get_address(p),
						                                           marie.getCMgr()->identity));
					}
					bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

					if (mgr != focus.getCMgr()) {
						BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, lise})
						                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
							                   return check_conference_ssrc(fconference, pconference);
						                   }));
					}
				}
			}
		}

		if (server_restart) {
			ms_message("%s is restarting", linphone_core_get_identity(focus.getLc()));
			coresList = bctbx_list_remove(coresList, focus.getLc());
			// Restart flexisip
			focus.reStart();

			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(focus.getLc(), pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_enable_video_capture(focus.getLc(), TRUE);
			linphone_core_enable_video_display(focus.getLc(), TRUE);
			coresList = bctbx_list_append(coresList, focus.getLc());
		}

		const char *subject = "Test characters: <S-F12><S-F11><S-F6> £$%§ (+edits)";
		const char *description2 = "Testing characters (+edits)";

		stats manager_editing_stat = manager_editing->stat;
		LinphoneAccount *editing_account = NULL;
		if (use_default_account) {
			editing_account = linphone_core_get_default_account(manager_editing->lc);
		} else {
			editing_account = linphone_core_lookup_known_account(manager_editing->lc, alternative_address);
		}
		BC_ASSERT_PTR_NOT_NULL(editing_account);
		if (editing_account) {
			LinphoneAccountParams *account_params =
			    linphone_account_params_clone(linphone_account_get_params(editing_account));
			linphone_account_params_enable_rtp_bundle(account_params, enable_bundle_mode);
			linphone_account_set_params(editing_account, account_params);
			linphone_account_params_unref(account_params);
		}

		LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(info)) {
			uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
			BC_ASSERT_PTR_NOT_NULL(uid);
			sequence = linphone_conference_info_get_ics_sequence(info);
			linphone_conference_info_unref(info);
		}

		bool_t add = TRUE;
		std::list<LinphoneCoreManager *> initialParticipants{pauline.getCMgr(), laure.getCMgr(), lise.getCMgr()};
		bctbx_list_t *initialInfos = NULL;
		for (const auto &participant : initialParticipants) {
			LinphoneParticipantInfo *info = linphone_participant_info_new(participant->identity);
			const bctbx_list_t *participant_info_it = bctbx_list_find_custom(
			    participants_info, (int (*)(const void *, const void *))find_matching_participant_info, info);
			BC_ASSERT_PTR_NOT_NULL(participant_info_it);
			if (participant_info_it) {
				LinphoneParticipantInfo *participant_info_found =
				    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_it);
				initialInfos = bctbx_list_append(initialInfos, participant_info_found);
			}
			linphone_participant_info_unref(info);
		}

		for (int attempt = 0; attempt < 3; attempt++) {
			ms_message("%s is trying to update conference %s - attempt %0d - %s %s",
			           linphone_core_get_identity(manager_editing->lc), conference_address_str, attempt,
			           (add) ? "adding" : "removing", linphone_core_get_identity(michelle.getLc()));

			stats focus_stat = focus.getStats();
			bool edit_successful = (use_default_account && !join);
			std::list<LinphoneCoreManager *> participants = initialParticipants;
			LinphoneConferenceInfo *conf_info =
			    linphone_core_find_conference_information_from_uri(manager_editing->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(conf_info);
			if (conf_info) {
				linphone_conference_info_set_subject(conf_info, subject);
				linphone_conference_info_set_description(conf_info, description2);
				// If the edition is not successfull, the conference information is not updated
				// It bring about that if Marie joined the conference, the old conference information stays in the
				// database and it has Marie as a participant
				size_t ics_participant_number = 0;
				if (add) {
					linphone_conference_info_add_participant(conf_info, michelle.getCMgr()->identity);
					ics_participant_number = 4 + ((((attempt == 0) || !edit_successful) && join) ? 1 : 0);
				} else {
					// If marie was in the list of participants, she will be removed as well but she will be kept as
					// organizer
					linphone_conference_info_set_participant_infos(conf_info, initialInfos);
					ics_participant_number = bctbx_list_size(initialInfos);
				}

				const bctbx_list_t *ics_participants = linphone_conference_info_get_participant_infos(conf_info);
				BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), ics_participant_number, size_t, "%zu");

				std::list<stats> participant_stats;
				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(),
				                 michelle.getCMgr(), lise.getCMgr()}) {
					participant_stats.push_back(mgr->stat);
				}

				LinphoneConferenceScheduler *conference_scheduler =
				    linphone_core_create_sip_conference_scheduler(manager_editing->lc, editing_account);
				LinphoneConferenceSchedulerCbs *cbs =
				    linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
				linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
				linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
				linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
				linphone_conference_scheduler_cbs_unref(cbs);
				linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

				if (edit_successful) {
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &manager_editing->stat.number_of_ConferenceSchedulerStateUpdating,
					                             manager_editing_stat.number_of_ConferenceSchedulerStateUpdating + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &manager_editing->stat.number_of_ConferenceSchedulerStateReady,
					                             manager_editing_stat.number_of_ConferenceSchedulerStateReady + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
					                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
					                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
					                             focus_stat.number_of_LinphoneCallEnd + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
					                             focus_stat.number_of_LinphoneCallReleased + 1,
					                             liblinphone_tester_sip_timeout));
					if (add) {
						LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
						add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity, role, 0);
					} else {
						for (const auto &mgr : {michelle.getCMgr(), marie.getCMgr()}) {
							LinphoneParticipantInfo *participant_info = linphone_participant_info_new(mgr->identity);
							bctbx_list_t *participant_info_found_it = bctbx_list_find_custom(
							    participants_info, (int (*)(const void *, const void *))find_matching_participant_info,
							    participant_info);
							if (participant_info_found_it) {
								LinphoneParticipantInfo *participant_info_found =
								    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_found_it);
								participants_info = bctbx_list_remove(participants_info, participant_info_found);
								linphone_participant_info_unref(participant_info_found);
							}
							linphone_participant_info_unref(participant_info);
						}
					}
					LinphoneConferenceParams *chat_room_params =
					    linphone_core_create_conference_params_2(manager_editing->lc, NULL);
					linphone_conference_params_enable_chat(chat_room_params, TRUE);
					linphone_conference_params_enable_group(chat_room_params, FALSE);
					LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(chat_room_params);
					linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendBasic);
					linphone_conference_scheduler_send_invitations_2(conference_scheduler, chat_room_params);

					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &manager_editing->stat.number_of_ConferenceSchedulerInvitationsSent,
					                             manager_editing_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
					                             liblinphone_tester_sip_timeout));

					for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(),
					                 michelle.getCMgr(), lise.getCMgr()}) {
						int exp_sequence = 0;
						if (mgr == focus.getCMgr()) {
							exp_sequence = 0;
						} else if (mgr == michelle.getCMgr()) {
							if (add) {
								exp_sequence = 0;
							} else {
								exp_sequence = 1;
							}
						} else if (mgr == lise.getCMgr()) {
							exp_sequence = (attempt + 1);
						} else {
							exp_sequence = (sequence + attempt + 1);
						}

						LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
						if (mgr == michelle.getCMgr()) {
							if (add) {
								exp_state = LinphoneConferenceInfoStateNew;
							} else {
								exp_state = LinphoneConferenceInfoStateCancelled;
							}
						} else {
							exp_state = LinphoneConferenceInfoStateUpdated;
						}
						auto old_stats = participant_stats.front();
						if ((mgr != focus.getCMgr()) && (mgr != manager_editing)) {
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
							                             old_stats.number_of_LinphoneMessageReceived + 1,
							                             liblinphone_tester_sip_timeout));
							if (!linphone_core_conference_ics_in_message_body_enabled(manager_editing->lc)) {
								BC_ASSERT_TRUE(wait_for_list(coresList,
								                             &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
								                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
								                             liblinphone_tester_sip_timeout));
							}

							BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
							if (mgr->stat.last_received_chat_message != NULL) {
								const string expected = ContentType::Icalendar.getMediaType();
								BC_ASSERT_STRING_EQUAL(
								    linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
								    expected.c_str());
							}

							int number_chat_rooms = 0;
							if (mgr == marie.getCMgr()) {
								number_chat_rooms = 3;
							} else if (from_organizer || (mgr == michelle.getCMgr())) {
								number_chat_rooms = 1;
							} else {
								number_chat_rooms = 2;
							}

							BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(mgr->lc)),
							                number_chat_rooms, int, "%d");

							update_sequence_number(&participants_info, {michelle.getCMgr()->identity},
							                       (mgr == focus.getCMgr()) ? 0 : (sequence + attempt + 1),
							                       (mgr == focus.getCMgr()) ? -1 : 0);
							LinphoneParticipantInfo *participant_info =
							    linphone_participant_info_new(lise.getCMgr()->identity);
							const bctbx_list_t *participant_info_it = bctbx_list_find_custom(
							    participants_info, (int (*)(const void *, const void *))find_matching_participant_info,
							    participant_info);
							BC_ASSERT_PTR_NOT_NULL(participant_info_it);
							if (participant_info_it) {
								LinphoneParticipantInfo *participant_info_found =
								    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_it);
								linphone_participant_info_set_sequence_number(participant_info_found, (attempt + 1));
							}
							linphone_participant_info_unref(participant_info);
							check_conference_info_in_db(mgr, uid, confAddr, marie.getCMgr()->identity,
							                            participants_info, start_time, duration, subject, description2,
							                            exp_sequence, exp_state, security_level, FALSE, TRUE, TRUE,
							                            (mgr == marie.getCMgr()) ? enable_chat : FALSE);

							bctbx_list_t *participant_chat_room_participants =
							    bctbx_list_append(NULL, manager_editing->identity);
							LinphoneChatRoom *pcr = linphone_core_search_chat_room(
							    mgr->lc, chat_room_params, mgr->identity, NULL, participant_chat_room_participants);
							bctbx_list_free(participant_chat_room_participants);
							BC_ASSERT_PTR_NOT_NULL(pcr);
							if (pcr) {
								LinphoneChatMessage *participant_msg =
								    linphone_chat_room_get_last_message_in_history(pcr);
								BC_ASSERT_PTR_NOT_NULL(participant_msg);
								if (participant_msg) {
									const bctbx_list_t *original_contents =
									    linphone_chat_message_get_contents(participant_msg);
									BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
									LinphoneContent *original_content =
									    (LinphoneContent *)bctbx_list_get_data(original_contents);
									if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
										LinphoneConferenceInfo *conf_info_from_original_content =
										    linphone_factory_create_conference_info_from_icalendar_content(
										        linphone_factory_get(), original_content);
										if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
											check_conference_info_against_db(mgr, confAddr,
											                                 conf_info_from_original_content, FALSE);
											linphone_conference_info_unref(conf_info_from_original_content);
										}
									}
									linphone_chat_message_unref(participant_msg);
								}
							}

							bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
							LinphoneChatRoom *cr =
							    linphone_core_search_chat_room(manager_editing->lc, chat_room_params,
							                                   manager_editing->identity, NULL, chat_room_participants);
							bctbx_list_free(chat_room_participants);
							BC_ASSERT_PTR_NOT_NULL(cr);
							if (cr) {
								LinphoneChatMessage *editing_participant_msg =
								    linphone_chat_room_get_last_message_in_history(cr);
								BC_ASSERT_PTR_NOT_NULL(editing_participant_msg);
								if (editing_participant_msg) {
									const bctbx_list_t *original_contents =
									    linphone_chat_message_get_contents(editing_participant_msg);
									BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
									LinphoneContent *original_content =
									    (LinphoneContent *)bctbx_list_get_data(original_contents);
									if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
										LinphoneConferenceInfo *conf_info_from_original_content =
										    linphone_factory_create_conference_info_from_icalendar_content(
										        linphone_factory_get(), original_content);
										if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
											check_conference_info_against_db(mgr, confAddr,
											                                 conf_info_from_original_content, FALSE);
											linphone_conference_info_unref(conf_info_from_original_content);
										}
									}
									linphone_chat_message_unref(editing_participant_msg);
								}
							}
						} else {
							bctbx_list_t *participants_info2 = bctbx_list_copy_with_data(
							    participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
							if (mgr == focus.getCMgr()) {
								add_participant_info_to_list(&participants_info2, marie.getCMgr()->identity,
								                             LinphoneParticipantRoleSpeaker, 0);
							}
							check_conference_info_in_db(
							    mgr, uid, confAddr, marie.getCMgr()->identity, participants_info2, start_time, duration,
							    subject, description2, exp_sequence, exp_state, security_level, TRUE, TRUE, TRUE,
							    ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())) ? enable_chat : FALSE);
							bctbx_list_free_with_data(participants_info2,
							                          (bctbx_list_free_func)linphone_participant_info_unref);
						}
						participant_stats.pop_front();
					}
					linphone_chat_room_params_unref(chat_room_params);
				} else {
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &manager_editing->stat.number_of_ConferenceSchedulerStateError,
					                             manager_editing_stat.number_of_ConferenceSchedulerStateError + 1,
					                             liblinphone_tester_sip_timeout));
				}
				linphone_conference_scheduler_unref(conference_scheduler);

				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(),
				                 michelle.getCMgr(), lise.getCMgr()}) {
					LinphoneConferenceInfo *info =
					    linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
					if (!edit_successful && (mgr == michelle.getCMgr())) {
						BC_ASSERT_PTR_NULL(info);
					} else if (BC_ASSERT_PTR_NOT_NULL(info)) {

						const char *exp_subject = NULL;
						if (edit_successful) {
							exp_subject = subject;
						} else {
							exp_subject = initialSubject;
						}

						const char *exp_description = NULL;
						if (mgr != focus.getCMgr()) {
							if (edit_successful) {
								exp_description = description2;
							} else {
								exp_description = description;
							}
						}

						unsigned int exp_sequence = 0;
						LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
						if (mgr == focus.getCMgr()) {
							exp_sequence = 0;
							exp_state = LinphoneConferenceInfoStateUpdated;
						} else {
							if (mgr == michelle.getCMgr()) {
								if (add) {
									exp_state = LinphoneConferenceInfoStateNew;
								} else {
									exp_state = LinphoneConferenceInfoStateCancelled;
								}
							} else if (mgr == lise.getCMgr()) {
								exp_state = edit_successful ? LinphoneConferenceInfoStateUpdated
								                            : LinphoneConferenceInfoStateNew;
							} else {
								exp_state = LinphoneConferenceInfoStateUpdated;
							}
							if (mgr == michelle.getCMgr()) {
								if (add) {
									exp_sequence = 0;
								} else {
									exp_sequence = 1;
								}
							} else if (mgr == lise.getCMgr()) {
								exp_sequence = edit_successful ? (attempt + 1) : 0;
							} else {
								exp_sequence = edit_successful ? (attempt + 2) : 1;
							}
						}

						bool is_focus = (mgr == focus.getCMgr());
						bool has_joined = ((mgr == marie.getCMgr()) || (mgr == pauline.getCMgr()));
						bctbx_list_t *participants_info2 = bctbx_list_copy_with_data(
						    participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
						if (is_focus || (!!join && has_joined)) {
							add_participant_info_to_list(&participants_info2, marie.getCMgr()->identity,
							                             LinphoneParticipantRoleSpeaker, 0);
						}

						// If not using the default account (which was used to create the conference), the
						// conference scheduler errors out and Michelle is not added
						int exp_sequence_number = 0;
						if (edit_successful) {
							if (is_focus) {
								exp_sequence_number = (attempt + 1);
							} else {
								exp_sequence_number = (attempt + 2);
							}
						} else {
							if (is_focus) {
								exp_sequence_number = 0;
							} else {
								exp_sequence_number = 1;
							}
						}
						std::list<LinphoneAddress *> new_participants{michelle.getCMgr()->identity};
						if (!join || (is_focus && (!edit_successful || (attempt > 0)))) {
							new_participants.push_back(marie.getCMgr()->identity);
						}
						update_sequence_number(&participants_info2, new_participants, exp_sequence_number,
						                       is_focus ? -1 : 0);

						LinphoneParticipantInfo *lise_participant_info =
						    linphone_participant_info_new(lise.getCMgr()->identity);
						const bctbx_list_t *participant_info_it = bctbx_list_find_custom(
						    participants_info2, (int (*)(const void *, const void *))find_matching_participant_info,
						    lise_participant_info);
						BC_ASSERT_PTR_NOT_NULL(participant_info_it);
						if (participant_info_it) {
							int exp_lise_sequence_number = 0;
							if (edit_successful) {
								if (is_focus) {
									exp_lise_sequence_number = attempt;
								} else {
									exp_lise_sequence_number = (attempt + 1);
								}
							} else {
								if (is_focus) {
									exp_lise_sequence_number = -1;
								} else {
									exp_lise_sequence_number = 0;
								}
							}
							LinphoneParticipantInfo *participant_info_found =
							    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_it);
							linphone_participant_info_set_sequence_number(participant_info_found,
							                                              exp_lise_sequence_number);
						}
						linphone_participant_info_unref(lise_participant_info);

						if (!is_focus) {
							int exp_organizer_sequence = 0;
							if (edit_successful) {
								exp_organizer_sequence = attempt + 2;
							} else {
								exp_organizer_sequence = 1;
							}
							linphone_conference_info_check_organizer(info, exp_organizer_sequence);
						}

						check_conference_info_in_db(
						    mgr, uid, confAddr, marie.getCMgr()->identity, participants_info2, start_time, duration,
						    exp_subject, exp_description, exp_sequence, exp_state, security_level, FALSE, TRUE, TRUE,
						    (is_focus || ((!!join && has_joined) || (mgr == marie.getCMgr()))) ? enable_chat : FALSE);

						bctbx_list_free_with_data(participants_info2,
						                          (bctbx_list_free_func)linphone_participant_info_unref);
					}
					if (info) {
						linphone_conference_info_unref(info);
					}
				}
				linphone_conference_info_unref(conf_info);
			}
			add = !add;
		}
		bctbx_list_free(initialInfos);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(uid);
		ms_free(conference_address_str);
		linphone_address_unref(alternative_address);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void organizer_edits_simple_conference(void) {
	edit_simple_conference_base(TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, LinphoneConferenceSecurityLevelNone, FALSE,
	                            FALSE);
}
static void organizer_edits_simple_conference_using_different_account(void) {
	edit_simple_conference_base(TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, LinphoneConferenceSecurityLevelNone, FALSE,
	                            FALSE);
}

static void organizer_edits_simple_conference_while_active(void) {
	edit_simple_conference_base(TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, LinphoneConferenceSecurityLevelNone, FALSE,
	                            FALSE);
}

static void participant_edits_simple_conference(void) {
	edit_simple_conference_base(FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, LinphoneConferenceSecurityLevelNone, FALSE,
	                            FALSE);
}

static void participant_edits_simple_conference_using_different_account(void) {
	edit_simple_conference_base(FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, LinphoneConferenceSecurityLevelNone, FALSE,
	                            FALSE);
}

static void organizer_edits_simple_conference_with_server_restart(void) {
	edit_simple_conference_base(TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, LinphoneConferenceSecurityLevelNone, FALSE,
	                            FALSE);
}

static void conference_edition_with_participant_role_changed(void) {
	edit_simple_conference_base(TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, LinphoneConferenceSecurityLevelNone, TRUE, TRUE);
}

static void conference_edition_with_simultaneous_participant_add_remove_base(bool_t codec_mismatch) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), true);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);

		setup_conference_info_cbs(marie.getCMgr());
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_X3dhUserCreationSuccess, 1,
		                             x3dhServer_creationTimeout));

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()}) {
			if (codec_mismatch) {
				if (mgr == marie.getCMgr()) {
					disable_all_audio_codecs_except_one(mgr->lc, "pcmu", -1);
				} else {
					disable_all_audio_codecs_except_one(mgr->lc, "pcma", -1);
				}
			}
		}

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), marie.getCMgr(), laure.getCMgr()};

		time_t start_time = time(NULL) + 20; // Start in 20 seconds
		int duration = 1;                    // minutes
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: <S-F12><S-F11><S-F6> £$%§";
		const char *description = "Testing characters";
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
		                                description, TRUE, security_level, FALSE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		const char *subject = "Test characters: <S-F12><S-F11><S-F6> £$%§ (+edits)";
		const char *description2 = "Testing characters (+edits)";

		stats marie_stat = marie.getStats();
		LinphoneAccount *editing_account = linphone_core_get_default_account(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(editing_account);

		char *uid = NULL;
		unsigned int sequence = 0;
		LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(info)) {
			uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
			BC_ASSERT_PTR_NOT_NULL(uid);
			sequence = linphone_conference_info_get_ics_sequence(info);
			linphone_conference_info_unref(info);
		}

		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");
		ms_message("%s is trying to update conference %s - adding %s and removing %s",
		           linphone_core_get_identity(marie.getLc()), conference_address_str,
		           linphone_core_get_identity(michelle.getLc()), linphone_core_get_identity(laure.getLc()));
		ms_free(conference_address_str);

		// Remove Laure from list of expected participants
		LinphoneParticipantInfo *laure_participant_info = linphone_participant_info_new(laure.getCMgr()->identity);
		bctbx_list_t *laure_participant_info_found_it = bctbx_list_find_custom(
		    participants_info, (int (*)(const void *, const void *))find_matching_participant_info,
		    laure_participant_info);
		BC_ASSERT_PTR_NOT_NULL(laure_participant_info_found_it);
		if (laure_participant_info_found_it) {
			LinphoneParticipantInfo *laure_participant_info_found =
			    (LinphoneParticipantInfo *)bctbx_list_get_data(laure_participant_info_found_it);
			participants_info = bctbx_list_remove(participants_info, laure_participant_info_found);
			linphone_participant_info_unref(laure_participant_info_found);
		}
		linphone_participant_info_unref(laure_participant_info);
		participantList.erase(laure.getCMgr());

		// Add michelle to list of expected participants
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleUnknown, 0)));

		stats focus_stat = focus.getStats();

		// Add Michelle and remove Laure
		LinphoneConferenceInfo *conf_info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description2);
		linphone_conference_info_add_participant(conf_info, michelle.getCMgr()->identity);
		linphone_conference_info_remove_participant(conf_info, laure.getCMgr()->identity);

		const bctbx_list_t *ics_participants = linphone_conference_info_get_participant_infos(conf_info);
		BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), participants.size(), size_t, "%zu");

		std::list<stats> participant_stats;
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			participant_stats.push_back(mgr->stat);
		}

		LinphoneConferenceScheduler *conference_scheduler =
		    linphone_core_create_sip_conference_scheduler(marie.getLc(), editing_account);
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);
		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
		                             marie_stat.number_of_ConferenceSchedulerStateUpdating + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateReady,
		                             marie_stat.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_set_subject(chat_room_params, "Conference");
		linphone_chat_room_params_enable_encryption(chat_room_params, TRUE);
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendFlexisipChat);
		linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerInvitationsSent,
		                             marie_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			auto old_stats = participant_stats.front();
			if ((mgr != focus.getCMgr()) && (mgr != marie.getCMgr())) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
				                             old_stats.number_of_LinphoneMessageReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(marie.getLc())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					const string expected = ContentType::Icalendar.getMediaType();
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       expected.c_str());
				}

				bctbx_list_t *participant_chat_room_participants = bctbx_list_append(NULL, marie.getCMgr()->identity);
				LinphoneChatRoom *pcr = linphone_core_search_chat_room(mgr->lc, chat_room_params, mgr->identity, NULL,
				                                                       participant_chat_room_participants);
				bctbx_list_free(participant_chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(pcr);
				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);

				LinphoneChatRoom *cr = linphone_core_search_chat_room(
				    marie.getLc(), chat_room_params, marie.getCMgr()->identity, NULL, chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);

				BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(mgr->lc)),
				                (mgr == michelle.getCMgr()) ? 1 : 2, int, "%d");

				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
					LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
					if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
						LinphoneConferenceInfo *conf_info_from_original_content =
						    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
						                                                                   original_content);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
							LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
							if (mgr == laure.getCMgr()) {
								exp_state = LinphoneConferenceInfoStateCancelled;
							} else if (mgr == michelle.getCMgr()) {
								exp_state = LinphoneConferenceInfoStateNew;
							} else {
								exp_state = LinphoneConferenceInfoStateUpdated;
							}

							update_sequence_number(&participants_info, {michelle.getCMgr()->identity},
							                       (mgr == focus.getCMgr()) ? 0 : 1, (mgr == focus.getCMgr()) ? -1 : 0);

							const unsigned int ics_sequence = (mgr == michelle.getCMgr()) ? 0 : (sequence + 1);
							check_conference_info_members(conf_info_from_original_content, uid, confAddr,
							                              marie.getCMgr()->identity, participants_info, start_time,
							                              duration, subject, description2, ics_sequence, exp_state,
							                              security_level, FALSE, TRUE, TRUE, FALSE);
							linphone_conference_info_unref(conf_info_from_original_content);
						}
					}
					linphone_chat_message_unref(msg);
				}
			}
			participant_stats.pop_front();
		}
		participant_stats.clear();
		linphone_chat_room_params_unref(chat_room_params);
		linphone_conference_info_unref(conf_info);
		linphone_conference_scheduler_unref(conference_scheduler);

		// Update subject
		const char *newSubject = "Test characters: <S-F12><S-F11><S-F6> £$%§ (+moreEdits)";
		conf_info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		linphone_conference_info_set_subject(conf_info, newSubject);

		ics_participants = linphone_conference_info_get_participant_infos(conf_info);
		BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), participants.size(), size_t, "%zu");

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			participant_stats.push_back(mgr->stat);
		}

		conference_scheduler = linphone_core_create_sip_conference_scheduler(marie.getLc(), editing_account);
		cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);
		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
		                             marie_stat.number_of_ConferenceSchedulerStateUpdating + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateReady,
		                             marie_stat.number_of_ConferenceSchedulerStateReady + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 2, liblinphone_tester_sip_timeout));

		chat_room_params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_set_subject(chat_room_params, "Conference");
		linphone_chat_room_params_enable_encryption(chat_room_params, TRUE);
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendFlexisipChat);
		linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerInvitationsSent,
		                             marie_stat.number_of_ConferenceSchedulerInvitationsSent + 2,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			auto old_stats = participant_stats.front();
			if ((mgr != focus.getCMgr()) && (mgr != marie.getCMgr())) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
				                             old_stats.number_of_LinphoneMessageReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(marie.getLc())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					const string expected = ContentType::Icalendar.getMediaType();
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       expected.c_str());
				}

				bctbx_list_t *participant_chat_room_participants = bctbx_list_append(NULL, marie.getCMgr()->identity);
				LinphoneChatRoom *pcr = linphone_core_search_chat_room(mgr->lc, chat_room_params, mgr->identity, NULL,
				                                                       participant_chat_room_participants);
				bctbx_list_free(participant_chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(pcr);
				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);

				LinphoneChatRoom *cr = linphone_core_search_chat_room(
				    marie.getLc(), chat_room_params, marie.getCMgr()->identity, NULL, chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);

				BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(mgr->lc)),
				                (mgr == michelle.getCMgr()) ? 1 : 2, int, "%d");

				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
					LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
					if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
						LinphoneConferenceInfo *conf_info_from_original_content =
						    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
						                                                                   original_content);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
							LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateUpdated;

							update_sequence_number(&participants_info, {michelle.getCMgr()->identity},
							                       (mgr == focus.getCMgr()) ? 0 : 2, (mgr == focus.getCMgr()) ? -1 : 1);

							const unsigned int ics_sequence = (mgr == michelle.getCMgr()) ? 1 : (sequence + 2);
							check_conference_info_members(conf_info_from_original_content, uid, confAddr,
							                              marie.getCMgr()->identity, participants_info, start_time,
							                              duration, newSubject, description2, ics_sequence, exp_state,
							                              security_level, FALSE, TRUE, TRUE, FALSE);
							linphone_conference_info_unref(conf_info_from_original_content);
						}
					}
					linphone_chat_message_unref(msg);
				}
			}
			participant_stats.pop_front();
		}
		linphone_chat_room_params_unref(chat_room_params);
		linphone_conference_scheduler_unref(conference_scheduler);
		linphone_conference_info_unref(conf_info);

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(info)) {

				const char *exp_subject = (mgr == laure.getCMgr()) ? subject : newSubject;

				const char *exp_description = NULL;
				if (mgr != focus.getCMgr()) {
					exp_description = description2;
				}

				unsigned int exp_sequence = 0;
				LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
				if (mgr == focus.getCMgr()) {
					exp_sequence = 0;
					exp_state = LinphoneConferenceInfoStateUpdated;
				} else {
					if (mgr == laure.getCMgr()) {
						exp_sequence = (sequence + 1);
						exp_state = LinphoneConferenceInfoStateCancelled;
					} else {
						exp_sequence = (mgr == michelle.getCMgr()) ? 1 : (sequence + 2);
						exp_state = LinphoneConferenceInfoStateUpdated;
					}
				}

				int old_participant_sequence_number = 0;
				int new_participant_sequence_number = -1;
				if ((mgr == focus.getCMgr()) || (mgr == laure.getCMgr())) {
					old_participant_sequence_number = 1;
					new_participant_sequence_number = 0;
				} else {
					old_participant_sequence_number = 2;
					new_participant_sequence_number = 1;
				}
				update_sequence_number(&participants_info, {michelle.getCMgr()->identity},
				                       old_participant_sequence_number, new_participant_sequence_number);

				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info,
				                            start_time, duration, exp_subject, exp_description, exp_sequence, exp_state,
				                            security_level, FALSE, TRUE,
				                            (mgr != focus.getCMgr()) && (mgr != marie.getCMgr()), FALSE);
			}
			if (info) {
				linphone_conference_info_unref(info);
			}
		}
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(uid);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void conference_edition_with_simultaneous_participant_add_remove(void) {
	conference_edition_with_simultaneous_participant_add_remove_base(FALSE);
}

static void conference_edition_with_organizer_codec_mismatch(void) {
	conference_edition_with_simultaneous_participant_add_remove_base(TRUE);
}

static void conference_cancelled_through_edit_base(bool_t server_restart, bool_t enable_encryption, bool_t join) {
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
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		if (!!enable_encryption) {
			for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()}) {
				linphone_core_set_media_encryption_mandatory(mgr->lc, TRUE);
				linphone_core_set_media_encryption(mgr->lc, LinphoneMediaEncryptionZRTP);
			}
			security_level = LinphoneConferenceSecurityLevelPointToPoint;
		}

		std::list<LinphoneCoreManager *> participants{michelle.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = time(NULL) - 60; // Started 1 minute ago
		int duration = 60;                   // minutes
		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: <S-F12><S-F11><S-F6> £$%§";
		const char *description = "Testing characters";

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
		                                description, TRUE, security_level, FALSE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		char *uid = NULL;
		unsigned int sequence = 0;
		LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(info)) {
			uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
			BC_ASSERT_PTR_NOT_NULL(uid);
			sequence = linphone_conference_info_get_ics_sequence(info);
			linphone_conference_info_unref(info);
		}

		stats focus_stat = focus.getStats();
		stats manager_editing_stat = marie.getStats();

		std::list<stats> participant_stats;
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			participant_stats.push_back(mgr->stat);
		}

		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");
		ms_message("%s is trying to change duration of conference %s", linphone_core_get_identity(marie.getLc()),
		           conference_address_str);
		LinphoneConferenceInfo *conf_info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		unsigned int new_duration = 2000;
		linphone_conference_info_set_duration(conf_info, new_duration);

		LinphoneAccount *editing_account = linphone_core_get_default_account(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(editing_account);
		LinphoneConferenceScheduler *conference_scheduler =
		    linphone_core_create_sip_conference_scheduler(marie.getLc(), editing_account);
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);
		cbs = nullptr;
		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
		                             manager_editing_stat.number_of_ConferenceSchedulerStateUpdating + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateReady,
		                             manager_editing_stat.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
		linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
		linphone_chat_room_params_unref(chat_room_params);
		chat_room_params = nullptr;

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerInvitationsSent,
		                             manager_editing_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			auto old_stats = participant_stats.front();
			if ((mgr != focus.getCMgr()) && (mgr != marie.getCMgr())) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
				                             old_stats.number_of_LinphoneMessageReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(marie.getLc())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					const string expected = ContentType::Icalendar.getMediaType();
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       expected.c_str());
				}

				bctbx_list_t *participant_chat_room_participants = bctbx_list_append(NULL, marie.getCMgr()->identity);
				LinphoneChatRoom *pcr = linphone_core_search_chat_room(mgr->lc, NULL, mgr->identity, NULL,
				                                                       participant_chat_room_participants);
				bctbx_list_free(participant_chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(pcr);
				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);

				LinphoneChatRoom *cr = linphone_core_search_chat_room(marie.getLc(), NULL, marie.getCMgr()->identity,
				                                                      NULL, chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);

				BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(mgr->lc)), 1, int, "%d");

				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
					LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
					if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
						LinphoneConferenceInfo *conf_info_from_original_content =
						    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
						                                                                   original_content);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
							const unsigned int ics_sequence = (sequence + 1);
							LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateUpdated;
							update_sequence_number(&participants_info, {}, (mgr == focus.getCMgr()) ? 0 : 1,
							                       (mgr == focus.getCMgr()) ? -1 : 0);

							check_conference_info_members(
							    conf_info_from_original_content, uid, confAddr, marie.getCMgr()->identity,
							    participants_info, start_time, new_duration, initialSubject, description, ics_sequence,
							    exp_state, LinphoneConferenceSecurityLevelNone, FALSE, TRUE, TRUE, FALSE);
							linphone_conference_info_unref(conf_info_from_original_content);
						}
					}
					linphone_chat_message_unref(msg);
				}
			}
			participant_stats.pop_front();
		}
		linphone_conference_scheduler_unref(conference_scheduler);
		conference_scheduler = nullptr;
		linphone_conference_info_unref(conf_info);
		conf_info = nullptr;

		if (server_restart) {
			ms_message("%s is restarting", linphone_core_get_identity(focus.getLc()));
			coresList = bctbx_list_remove(coresList, focus.getLc());
			// Restart flexisip
			focus.reStart();

			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(focus.getLc(), pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_enable_video_capture(focus.getLc(), TRUE);
			linphone_core_enable_video_display(focus.getLc(), TRUE);
			coresList = bctbx_list_append(coresList, focus.getLc());
		}

		if (!!join) {
			focus_stat = focus.getStats();
			stats pauline_stat = pauline.getStats();
			ms_message("%s is entering conference %s", linphone_core_get_identity(pauline.getLc()),
			           conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(pauline.getLc(), nullptr);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			linphone_core_invite_address_with_params_2(pauline.getLc(), confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallOutgoingProgress,
			                             pauline_stat.number_of_LinphoneCallOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating,
			                             pauline_stat.number_of_LinphoneCallUpdating + (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
			                             pauline_stat.number_of_LinphoneCallStreamsRunning + no_streams_running,
			                             liblinphone_tester_sip_timeout));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated,
			                             pauline_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             pauline_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionActive,
			                             pauline_stat.number_of_LinphoneSubscriptionActive + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyFullStateReceived,
			                             pauline_stat.number_of_NotifyFullStateReceived + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
			int focus_no_streams_running = 2;
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                  focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 1),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
			                             liblinphone_tester_sip_timeout));
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
			                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + 1, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));

			std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
			    fill_member_list({pauline.getCMgr()}, participantList, marie.getCMgr(), participants_info);
			wait_for_conference_streams({focus, marie, pauline, laure, michelle}, {focus.getCMgr(), pauline.getCMgr()},
			                            focus.getCMgr(), memberList, confAddr, TRUE);

			ms_message("%s exits conference %s", linphone_core_get_identity(pauline.getLc()), conference_address_str);
			if (pcall) {
				linphone_call_terminate(pcall);
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd,
				                             pauline_stat.number_of_LinphoneCallEnd + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased,
				                             pauline_stat.number_of_LinphoneCallReleased + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated,
				                             pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
				                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
				                             focus_stat.number_of_LinphoneCallReleased + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
				                             focus_stat.number_of_LinphoneSubscriptionTerminated + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
				                             pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated,
				                             pauline_stat.number_of_LinphoneConferenceStateTerminated + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted,
				                             pauline_stat.number_of_LinphoneConferenceStateDeleted + 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		const char *subject = "Test characters: <S-F12><S-F11><S-F6> £$%§ (+cancelled)";
		const char *description2 = "Testing characters (+cancelled)";

		info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(info)) {
			sequence = linphone_conference_info_get_ics_sequence(info);
			linphone_conference_info_unref(info);
		}

		ms_message("%s is trying to cancel conference %s", linphone_core_get_identity(marie.getLc()),
		           conference_address_str);
		ms_free(conference_address_str);

		focus_stat = focus.getStats();
		manager_editing_stat = marie.getStats();

		participant_stats.clear();
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			participant_stats.push_back(mgr->stat);
		}

		conf_info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description2);

		const bctbx_list_t *ics_participants = linphone_conference_info_get_participant_infos(conf_info);
		BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), 3, size_t, "%zu");

		conference_scheduler = linphone_core_create_sip_conference_scheduler(marie.getLc(), editing_account);
		cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);
		linphone_conference_scheduler_cancel_conference(conference_scheduler, conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
		                             manager_editing_stat.number_of_ConferenceSchedulerStateUpdating + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateReady,
		                             manager_editing_stat.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                             focus_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                             focus_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		chat_room_params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
		linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
		linphone_chat_room_params_unref(chat_room_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerInvitationsSent,
		                             manager_editing_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			auto old_stats = participant_stats.front();
			if ((mgr != focus.getCMgr()) && (mgr != marie.getCMgr())) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
				                             old_stats.number_of_LinphoneMessageReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(marie.getLc())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					const string expected = ContentType::Icalendar.getMediaType();
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       expected.c_str());
				}

				bctbx_list_t *participant_chat_room_participants = bctbx_list_append(NULL, marie.getCMgr()->identity);
				LinphoneChatRoom *pcr = linphone_core_search_chat_room(mgr->lc, NULL, mgr->identity, NULL,
				                                                       participant_chat_room_participants);
				bctbx_list_free(participant_chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(pcr);
				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);

				LinphoneChatRoom *cr = linphone_core_search_chat_room(marie.getLc(), NULL, marie.getCMgr()->identity,
				                                                      NULL, chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);

				BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(mgr->lc)), 1, int, "%d");

				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
					LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
					if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
						LinphoneConferenceInfo *conf_info_from_original_content =
						    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
						                                                                   original_content);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
							const unsigned int ics_sequence = (sequence + 1);
							LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateNew;
							if (mgr == focus.getCMgr()) {
								exp_state = LinphoneConferenceInfoStateUpdated;
							} else {
								exp_state = LinphoneConferenceInfoStateCancelled;
							}
							check_conference_info_members(conf_info_from_original_content, uid, confAddr,
							                              marie.getCMgr()->identity, NULL, start_time, new_duration,
							                              subject, description2, ics_sequence, exp_state,
							                              LinphoneConferenceSecurityLevelNone, FALSE, TRUE,
							                              (mgr != focus.getCMgr()) && (mgr != marie.getCMgr()), FALSE);
							linphone_conference_info_unref(conf_info_from_original_content);
						}
					}
					linphone_chat_message_unref(msg);
				}
			}
			participant_stats.pop_front();
		}
		linphone_conference_scheduler_unref(conference_scheduler);

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(info)) {

				const char *exp_subject = subject;

				const char *exp_description = NULL;
				if (mgr != focus.getCMgr()) {
					exp_description = description2;
				}

				unsigned int exp_sequence = 0;
				LinphoneConferenceInfoState exp_state = LinphoneConferenceInfoStateCancelled;
				if (mgr == focus.getCMgr()) {
					exp_sequence = 0;
				} else {
					exp_sequence = (sequence + 1);
				}
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, NULL, start_time,
				                            new_duration, exp_subject, exp_description, exp_sequence, exp_state,
				                            ((mgr == focus.getCMgr()) || (mgr == marie.getCMgr()))
				                                ? security_level
				                                : LinphoneConferenceSecurityLevelNone,
				                            FALSE, TRUE, (mgr != focus.getCMgr()) && (mgr != marie.getCMgr()), FALSE);
			}
			if (info) {
				linphone_conference_info_unref(info);
			}
		}
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_conference_info_unref(conf_info);
		ms_free(uid);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void conference_cancelled_through_edit(void) {
	conference_cancelled_through_edit_base(FALSE, FALSE, FALSE);
}

static void zrtp_conference_cancelled_through_edit(void) {
	conference_cancelled_through_edit_base(FALSE, TRUE, FALSE);
}

static void create_conference_with_server_restart_conference_cancelled(void) {
	conference_cancelled_through_edit_base(TRUE, FALSE, FALSE);
}

static void conference_cancelled_through_edit_after_joining(void) {
	conference_cancelled_through_edit_base(FALSE, FALSE, TRUE);
}

static void conference_cancelled_through_edit_while_active(void) {
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

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr()};
		auto conferenceMgrs = members;
		conferenceMgrs.push_back(focus.getCMgr());

		time_t start_time = ms_time(NULL) - 10;
		int duration = 5;
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
		                                description, TRUE, security_level, FALSE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 3,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             ((mgr == marie.getCMgr()) ? 3 : 1), liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, TRUE);

		ms_message("%s is trying to cancel conference %s while it is running",
		           linphone_core_get_identity(marie.getLc()), conference_address_str);
		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_conference_scheduler(marie.getLc());
		linphone_conference_scheduler_set_account(conference_scheduler,
		                                          linphone_core_get_default_account(marie.getLc()));
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);
		LinphoneConferenceInfo *conf_info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(conf_info);
		linphone_conference_scheduler_cancel_conference(conference_scheduler, conf_info);
		linphone_conference_info_unref(conf_info);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateUpdating,
		                             marie_stat.number_of_ConferenceSchedulerStateUpdating + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateError,
		                             marie_stat.number_of_ConferenceSchedulerStateError + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                              focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                              focus_stat.number_of_LinphoneConferenceStateTerminated + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                              focus_stat.number_of_LinphoneConferenceStateDeleted + 1, 1000));
		linphone_conference_scheduler_unref(conference_scheduler);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

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

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()}) {
			size_t nb_call_logs = 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), nb_call_logs, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), nb_call_logs, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		ms_free(conference_address_str);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_conference_edition_tests[] = {
    TEST_NO_TAG("Edit and cancel simple conference with database conference scheduler",
                LinphoneTest::edit_and_cancel_simple_conference_db_conference_scheduler),
    TEST_NO_TAG("Organizer edits simple conference", LinphoneTest::organizer_edits_simple_conference),
    TEST_NO_TAG("Organizer edits simple conference using different account",
                LinphoneTest::organizer_edits_simple_conference_using_different_account),
    TEST_NO_TAG("Organizer edits simple conference while it is active",
                LinphoneTest::organizer_edits_simple_conference_while_active),
    TEST_NO_TAG("Organizer edits simple conference with server restart",
                LinphoneTest::organizer_edits_simple_conference_with_server_restart),
    TEST_NO_TAG("Participant edits simple conference", LinphoneTest::participant_edits_simple_conference),
    TEST_NO_TAG("Participant edits simple conference using different account",
                LinphoneTest::participant_edits_simple_conference_using_different_account),
    TEST_NO_TAG("Conference cancelled through edit", LinphoneTest::conference_cancelled_through_edit),
    TEST_NO_TAG("ZRTP conference cancelled through edit", LinphoneTest::zrtp_conference_cancelled_through_edit),
    TEST_NO_TAG("Conference cancelled through edit after joining",
                LinphoneTest::conference_cancelled_through_edit_after_joining),
    TEST_NO_TAG("Conference cancelled through edit while active",
                LinphoneTest::conference_cancelled_through_edit_while_active),
    TEST_NO_TAG("Conference edition with simultanoues participant added removed",
                LinphoneTest::conference_edition_with_simultaneous_participant_add_remove),
    TEST_NO_TAG("Conference edition with participant role changed",
                LinphoneTest::conference_edition_with_participant_role_changed),
    TEST_NO_TAG("Conference edition with organizer codec mismatch",
                LinphoneTest::conference_edition_with_organizer_codec_mismatch),
    TEST_NO_TAG("Create conference with server restart (conference cancelled)",
                LinphoneTest::create_conference_with_server_restart_conference_cancelled)};

test_suite_t local_conference_test_suite_conference_edition = {"Local conference tester (Conference edition)",
                                                               NULL,
                                                               NULL,
                                                               liblinphone_tester_before_each,
                                                               liblinphone_tester_after_each,
                                                               sizeof(local_conference_conference_edition_tests) /
                                                                   sizeof(local_conference_conference_edition_tests[0]),
                                                               local_conference_conference_edition_tests,
                                                               0};
