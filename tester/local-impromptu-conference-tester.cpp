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
#include "linphone/api/c-participant-info.h"
#include "local-conference-tester-functions.h"
#include "shared_tester_functions.h"

namespace LinphoneTest {

static void create_simple_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_chat(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, TRUE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_ice_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutGrid, pol, TRUE, TRUE,
	                                LinphoneConferenceParticipantListTypeOpen, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_calls_declined(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutGrid, pol, TRUE, TRUE,
	                                LinphoneConferenceParticipantListTypeOpen, FALSE, FALSE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_point_to_point_encrypted_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelPointToPoint, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_participant_codec_mismatch(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_video_not_accepted(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_video_not_initiated(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(LinphoneConferenceLayoutGrid, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_organizer_codec_mismatch(void) {
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
			if (mgr == marie.getCMgr()) {
				disable_all_audio_codecs_except_one(mgr->lc, "pcmu", -1);
			} else {
				disable_all_audio_codecs_except_one(mgr->lc, "pcma", -1);
			}

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
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the Goutier mountain hut!!!! :-)";
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
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreationFailed,
		                             focus_stat.number_of_LinphoneConferenceStateCreationFailed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                              marie_stat.number_of_LinphoneConferenceStateCreated + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                              focus_stat.number_of_LinphoneCallIncomingReceived + 1, 1000));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NULL(fconference);

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		if (confAddr) linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_dial_out_with_some_calls_declined(void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonDeclined,
	                                                                LinphoneConferenceSecurityLevelNone);
}

static void create_simple_conference_dial_out_with_some_calls_busy(void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonBusy,
	                                                                LinphoneConferenceSecurityLevelNone);
}

static void create_simple_conference_dial_out_with_late_participant_addition(void) {
	create_conference_with_late_participant_addition_base(-1, -1, LinphoneConferenceLayoutActiveSpeaker,
	                                                      LinphoneConferenceParticipantListTypeOpen, TRUE, TRUE,
	                                                      LinphoneConferenceSecurityLevelNone);
}

static void create_simple_conference_dial_out_with_many_late_participant_additions(void) {
	create_conference_with_late_participant_addition_base(-1, -1, LinphoneConferenceLayoutGrid,
	                                                      LinphoneConferenceParticipantListTypeOpen, TRUE, FALSE,
	                                                      LinphoneConferenceSecurityLevelNone);
}

static void organizer_creates_two_dialout_conferences(void) {
	two_overlapping_conferences_base(TRUE, TRUE);
}

static void two_overlapping_dialout_conferences_from_different_organizers(void) {
	two_overlapping_conferences_base(FALSE, TRUE);
}

static void create_simple_conference_merging_calls(void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE,
	                                            LinphoneConferenceSecurityLevelNone, FALSE);
}

static void create_simple_conference_merging_calls_with_screen_sharing(void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, TRUE, FALSE, FALSE,
	                                            LinphoneConferenceSecurityLevelNone, TRUE);
}

static void create_simple_conference_merging_calls_with_video_toggling(void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE,
	                                            LinphoneConferenceSecurityLevelNone, FALSE);
}

static void create_simple_conference_merging_calls_with_video_toggling_after_screen_sharing(void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE,
	                                            LinphoneConferenceSecurityLevelNone, FALSE);
}

static void create_dial_out_conference_with_active_call(void) {
	create_conference_with_active_call_base(TRUE);
}

static void simple_dial_out_conference_with_no_payloads(void) {
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

		const bctbx_list_t *elem = linphone_core_get_audio_codecs(pauline.getLc());
		disable_all_codecs(elem, pauline.getCMgr());

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		linphone_core_set_avpf_mode(focus.getCMgr()->lc, LinphoneAVPFEnabled);
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";

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
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, FALSE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		std::list<LinphoneCoreManager *> mgr_in_conference{marie.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                                   berthe.getCMgr()};

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 5,
		                             liblinphone_tester_sip_timeout));
		for (auto mgr : participants) {
			if (mgr == pauline.getCMgr()) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallError,
				                             focus_stat.number_of_LinphoneCallError + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
				                             focus_stat.number_of_LinphoneCallReleased + 1,
				                             liblinphone_tester_sip_timeout));
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				if (mgr == pauline.getCMgr()) {
					BC_ASSERT_PTR_NULL(pcall);
				} else {
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						linphone_call_accept(pcall);
					}
				}
			}
		}

		for (auto mgr : participants) {
			if (mgr != pauline.getCMgr()) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		focus_stat = focus.getStats();
		for (auto mgr : {marie.getCMgr(), laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()}) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + 4,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 4,
		                             liblinphone_tester_sip_timeout));

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

static void create_conference_dial_out_with_video_activation_and_layout_change(void) {
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
			linphone_video_activation_policy_set_automatically_accept(pol, (mgr != marie.getCMgr()));
			linphone_video_activation_policy_set_automatically_initiate(pol, (mgr != marie.getCMgr()));
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
			}

			enable_stun_in_mgr(mgr, TRUE, TRUE, TRUE, TRUE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
		}
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, FALSE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
		                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		LinphoneConference *oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(oconference);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, 20000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));

		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 1, int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), members.size(), size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		bctbx_list_t *participants_info2 = NULL;
		for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
			const LinphoneParticipantInfo *participant_info_el =
			    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
			const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
			const int sequence = -1;
			LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
			add_participant_info_to_list(&participants_info2, address, role, sequence);
		}

		add_participant_info_to_list(&participants_info2, marie.getCMgr()->identity, LinphoneParticipantRoleUnknown,
		                             -1);

		if (confAddr) {
			for (auto mgr : participants) {
				// Encryption is None because we haven't received yet the NOTIFY full state yet
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
				                            initialSubject, NULL, 0, LinphoneConferenceInfoStateNew,
				                            LinphoneConferenceSecurityLevelNone, FALSE, TRUE, TRUE, FALSE);

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					linphone_call_accept(pcall);
				}
			}
		}

		bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);

		add_participant_info_to_list(&participants_info, marie.getCMgr()->identity, LinphoneParticipantRoleSpeaker, -1);

		update_sequence_number(&participants_info, {}, -1, -1);

		int participant_no = static_cast<int>(participants.size());
		for (auto mgr : participants) {

			bctbx_list_t *participants_info3 = NULL;
			for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
				const LinphoneParticipantInfo *participant_info_el =
				    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
				const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
				const int sequence = linphone_participant_info_get_sequence_number(participant_info_el);
				bool found = false;
				LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
				LinphoneParticipantRole current_role = linphone_participant_info_get_role(participant_info_el);
				if (!found && (current_role == LinphoneParticipantRoleUnknown)) {
					role = LinphoneParticipantRoleSpeaker;
				} else {
					role = current_role;
				}
				add_participant_info_to_list(&participants_info3, address, role, sequence);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info3, 0, 0,
			                            initialSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level, FALSE,
			                            TRUE, TRUE, FALSE);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
				}
			}
			bctbx_list_free_with_data(participants_info3, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdating,
		                             focus_stat.number_of_LinphoneCallUpdating + participant_no - 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning +
		                                 2 * static_cast<int>(participants.size() + 1),
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + participant_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + participant_no,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + participant_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + participant_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + participant_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + participant_no,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline}, conferenceMgrs, focus.getCMgr(), memberList, confAddr,
		                            TRUE);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(15), [] { return false; });

		for (auto mgr : conferenceMgrs) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);

			const LinphoneVideoActivationPolicy *mgr_pol = linphone_core_get_video_activation_policy(mgr->lc);
			bool_t video_enabled =
			    !!((mgr == marie.getCMgr()) ? linphone_video_activation_policy_get_automatically_initiate(mgr_pol)
			                                : linphone_video_activation_policy_get_automatically_accept(mgr_pol));

			if (pconference) {
				int no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = static_cast<int>(members.size());
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					bctbx_list_t *participants_info3 = NULL;
					for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
						const LinphoneParticipantInfo *participant_info_el =
						    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
						const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
						const int sequence = linphone_participant_info_get_sequence_number(participant_info_el);
						LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
						LinphoneParticipantRole current_role = linphone_participant_info_get_role(participant_info_el);
						if ((mgr == marie.getCMgr()) && (current_role == LinphoneParticipantRoleUnknown)) {
							role = LinphoneParticipantRoleSpeaker;
						} else {
							role = current_role;
						}
						add_participant_info_to_list(&participants_info3, address, role, sequence);
					}

					check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info3, 0,
					                            0, initialSubject, (mgr == marie.getCMgr()) ? description : NULL, 0,
					                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, TRUE,
					                            FALSE);
					bctbx_list_free_with_data(participants_info3,
					                          (bctbx_list_free_func)linphone_participant_info_unref);

					no_participants = participant_no;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled =
					    !!((mgr == marie.getCMgr()) ? linphone_video_activation_policy_get_automatically_initiate(pol)
					                                : linphone_video_activation_policy_get_automatically_accept(pol));

					size_t no_streams_audio = 0;
					size_t no_max_streams_audio = 1;
					size_t no_max_streams_video = (enabled || (mgr == marie.getCMgr())) ? (participants.size() + 2) : 1;
					size_t no_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						// Even if video is not enabled, the server will offer it and clients reject the video
						// stream if they do not want to send or receive it.
						no_streams_video = compute_no_video_streams(TRUE, pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_max_streams_audio, no_max_streams_video,
						                                    no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), video_enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), video_enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");

						LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
						LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
						if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
							check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
						}
					}

					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_max_nb_streams(ccall, no_max_streams_audio, no_max_streams_video,
						                                    no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), video_enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), video_enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), members.size(), size_t, "%zu");
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
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}

				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						// If we are currently carrying out checks on the conference server side, the must set the
						// value of video enabled flag for each participant. In fact each call session may have the
						// video enabled or not and this is taken into account to compute the video availablity
						// flag. Nonetheless, this is not required for the participants as they only have one call
						// session towards the conference server therefore we can reuse the value computed earlier
						// on.
						if (mgr == focus.getCMgr()) {
							if (linphone_address_weak_equal(marie.getIdentity().toC(),
							                                linphone_participant_device_get_address(d))) {
								// The organizer will not offer video streams in its INVITE to join a conference if
								// the policy doesn't allow it
								video_enabled = FALSE;
							} else {
								// The participants will not accept video streams to answer the conference server
								// INVITE to join a conference if the policy doesn't allow it
								video_enabled = TRUE;
							}
						}

						bool video_available =
						    !!linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						LinphoneMediaDirection video_direction =
						    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
						BC_ASSERT_TRUE(video_available == (((video_direction == LinphoneMediaDirectionSendOnly) ||
						                                    (video_direction == LinphoneMediaDirectionSendRecv)) &&
						                                   video_enabled));
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}
		}

		LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(marie_call);

		Address marieAddr = marie.getIdentity();
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), marieAddr.toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);

		const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(marie.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_accept(pol);

		LinphoneAddress *marieUri = linphone_address_new(linphone_core_get_identity(marie.getLc()));
		LinphoneConference *marieConference =
		    linphone_core_search_conference(marie.getLc(), NULL, marieUri, confAddr, NULL);
		linphone_address_unref(marieUri);
		BC_ASSERT_PTR_NOT_NULL(marieConference);

		set_video_settings_in_conference(focus.getCMgr(), marie.getCMgr(), members, confAddr, enable,
		                                 LinphoneMediaDirectionSendRecv, enable, LinphoneMediaDirectionSendRecv);

		if (marieConference) {
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(marieConference);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				if (enable) {
					if (linphone_conference_is_me(marieConference, linphone_participant_device_get_address(d))) {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
					} else {
						BC_ASSERT_TRUE(
						    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
						    (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
						     LinphoneMediaDirectionSendRecv));
					}
				} else {
					BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
					               enable);
				}
			}

			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}
		// Wait a little bit
		wait_for_list(coresList, NULL, 0, 1000);

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		LinphoneConferenceLayout new_layout = LinphoneConferenceLayoutGrid;
		ms_message("%s changes the layout to Grid", linphone_core_get_identity(marie.getLc()));

		LinphoneCallParams *call_params = linphone_core_create_call_params(marie.getLc(), marie_call);
		linphone_call_params_set_conference_video_layout(call_params, new_layout);
		linphone_call_update(marie_call, call_params);
		linphone_call_params_unref(call_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));

		if (marie_call) {
			const LinphoneCallParams *marie_call_local_params = linphone_call_get_params(marie_call);
			const LinphoneConferenceLayout marie_conf_layout =
			    linphone_call_params_get_conference_video_layout(marie_call_local_params);
			BC_ASSERT_EQUAL(new_layout, marie_conf_layout, int, "%d");
		}

		if (focus_call) {
			const LinphoneCallParams *focus_call_local_params = linphone_call_get_remote_params(focus_call);
			const LinphoneConferenceLayout focus_conf_layout =
			    linphone_call_params_get_conference_video_layout(focus_call_local_params);
			BC_ASSERT_EQUAL(new_layout, focus_conf_layout, int, "%d");
		}

		focus_stat = focus.getStats();
		for (auto mgr : members) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
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

		int members_no = static_cast<int>(members.size());
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + members_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + members_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + members_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + members_no,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + members_no,
		                             liblinphone_tester_sip_timeout));

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

		for (auto mgr : members) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
					}
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info3 = NULL;
			for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
				const LinphoneParticipantInfo *participant_info_el =
				    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
				const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
				const int sequence = linphone_participant_info_get_sequence_number(participant_info_el);
				LinphoneParticipantRole role = linphone_participant_info_get_role(participant_info_el);
				add_participant_info_to_list(&participants_info3, address, role, sequence);
			}
			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info3, 0, 0,
			                            initialSubject, (mgr == marie.getCMgr()) ? description : NULL, 0,
			                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, TRUE, FALSE);
			bctbx_list_free_with_data(participants_info3, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_encrypted_impromptu_conference_tests[] = {
    TEST_NO_TAG("Create simple point-to-point encrypted dial out conference",
                LinphoneTest::create_simple_point_to_point_encrypted_conference_dial_out),
};

static test_t local_conference_impromptu_conference_tests[] = {
    TEST_NO_TAG("Create simple dial out conference", LinphoneTest::create_simple_conference_dial_out),
    TEST_NO_TAG("Create simple dial out conference with chat",
                LinphoneTest::create_simple_conference_dial_out_with_chat),
    TEST_NO_TAG("Create simple dial out ICE conference", LinphoneTest::create_simple_ice_conference_dial_out),
    TEST_NO_TAG("Create simple dial out conference with late participant addition",
                LinphoneTest::create_simple_conference_dial_out_with_late_participant_addition),
    TEST_NO_TAG("Create simple dial out conference with many late participant additions",
                LinphoneTest::create_simple_conference_dial_out_with_many_late_participant_additions),
    TEST_NO_TAG("Create simple dial out conference with video activation and layout change",
                LinphoneTest::create_conference_dial_out_with_video_activation_and_layout_change),
    TEST_NO_TAG("Create simple conference by merging calls", LinphoneTest::create_simple_conference_merging_calls),
    TEST_NO_TAG("Create simple conference by merging calls with video toggling",
                LinphoneTest::create_simple_conference_merging_calls_with_video_toggling),
    TEST_NO_TAG("Create simple conference by merging calls with screen sharing",
                LinphoneTest::create_simple_conference_merging_calls_with_screen_sharing),
    TEST_NO_TAG("Create simple conference by merging calls with video toggling after screen sharing",
                LinphoneTest::create_simple_conference_merging_calls_with_video_toggling_after_screen_sharing),
    TEST_NO_TAG("Create dial out conference with active call",
                LinphoneTest::create_dial_out_conference_with_active_call),
    TEST_NO_TAG("Organizer creates 2 dialout conferences", LinphoneTest::organizer_creates_two_dialout_conferences),
    TEST_NO_TAG("2 overlapping dialout conferences from different organizers",
                LinphoneTest::two_overlapping_dialout_conferences_from_different_organizers)};

static test_t local_conference_impromptu_mismatch_conference_tests[] = {
    TEST_NO_TAG("Create simple dial out conference with calls declined",
                LinphoneTest::create_simple_conference_dial_out_with_calls_declined),
    TEST_NO_TAG("Create simple dial out conference with some calls declined",
                LinphoneTest::create_simple_conference_dial_out_with_some_calls_declined),
    TEST_NO_TAG("Create simple dial out conference with some calls busy",
                LinphoneTest::create_simple_conference_dial_out_with_some_calls_busy),
    TEST_NO_TAG("Create simple dial out conference with participant codec mismatch",
                LinphoneTest::create_simple_conference_dial_out_participant_codec_mismatch),
    TEST_NO_TAG("Create simple dial out conference with organizer codec mismatch",
                LinphoneTest::create_simple_conference_dial_out_organizer_codec_mismatch),
    TEST_NO_TAG("Create simple dial out conference with video not initiated",
                LinphoneTest::create_simple_conference_dial_out_with_video_not_initiated),
    TEST_NO_TAG("Create simple dial out conference with video not accepted",
                LinphoneTest::create_simple_conference_dial_out_with_video_not_accepted),
    TEST_NO_TAG("Simple dial out conference with no payloads",
                LinphoneTest::simple_dial_out_conference_with_no_payloads)};

test_suite_t local_conference_test_suite_encrypted_impromptu_conference = {
    "Local conference tester (Impromptu Encrypted Conference Point to Point Encryption)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_encrypted_impromptu_conference_tests) /
        sizeof(local_conference_encrypted_impromptu_conference_tests[0]),
    local_conference_encrypted_impromptu_conference_tests,
    0,
    8 /*cpu_weight : video encnrypted conference uses more resources */
};

test_suite_t local_conference_test_suite_impromptu_conference = {
    "Local conference tester (Impromptu Conference)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_impromptu_conference_tests) / sizeof(local_conference_impromptu_conference_tests[0]),
    local_conference_impromptu_conference_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};

test_suite_t local_conference_test_suite_impromptu_mismatch_conference = {
    "Local conference tester (Impromptu Conference with mismatch)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_impromptu_mismatch_conference_tests) /
        sizeof(local_conference_impromptu_mismatch_conference_tests[0]),
    local_conference_impromptu_mismatch_conference_tests,
    0,
    4};
