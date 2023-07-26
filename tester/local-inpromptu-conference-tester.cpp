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
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_and_ics(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(TRUE, LinphoneConferenceLayoutGrid, pol, TRUE, TRUE,
	                                LinphoneConferenceParticipantListTypeOpen, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_calls_declined(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutGrid, pol, TRUE, TRUE,
	                                LinphoneConferenceParticipantListTypeOpen, FALSE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_point_to_point_encrypted_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelPointToPoint);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_participant_codec_mismatch(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_video_not_accepted(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_video_not_initiated(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutGrid, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
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
		                                                        description, TRUE, security_level);
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
		                                                        description, FALSE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		LinphoneConference *fconference =
		    (confAddr) ? linphone_core_search_conference_2(focus.getLc(), confAddr) : NULL;

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

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 0, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		// Explicitely terminate conference as those on server are static by default
		if (fconference) {
			linphone_conference_terminate(fconference);
		}
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

} // namespace LinphoneTest

static test_t local_conference_encrypted_inpromptu_conference_tests[] = {
    TEST_NO_TAG("Create simple point-to-point encrypted dial out conference",
                LinphoneTest::create_simple_point_to_point_encrypted_conference_dial_out),
};

static test_t local_conference_inpromptu_conference_tests[] = {
    TEST_NO_TAG("Create simple dial out conference", LinphoneTest::create_simple_conference_dial_out),
    TEST_NO_TAG("Create simple dial out ICE conference and ICS sent",
                LinphoneTest::create_simple_conference_dial_out_and_ics),
    TEST_NO_TAG("Create simple dial out conference with late participant addition",
                LinphoneTest::create_simple_conference_dial_out_with_late_participant_addition),
    TEST_NO_TAG("Create simple dial out conference with many late participant additions",
                LinphoneTest::create_simple_conference_dial_out_with_many_late_participant_additions),
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

static test_t local_conference_inpromptu_mismatch_conference_tests[] = {
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

test_suite_t local_conference_test_suite_encrypted_inpromptu_conference = {
    "Local conference tester (Inpromptu Encrypted Conference)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_encrypted_inpromptu_conference_tests) /
        sizeof(local_conference_encrypted_inpromptu_conference_tests[0]),
    local_conference_encrypted_inpromptu_conference_tests,
    0,
    8 /*cpu_weight : video encnrypted conference uses more resources */
};

test_suite_t local_conference_test_suite_inpromptu_conference = {
    "Local conference tester (Inpromptu Conference)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_inpromptu_conference_tests) / sizeof(local_conference_inpromptu_conference_tests[0]),
    local_conference_inpromptu_conference_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};

test_suite_t local_conference_test_suite_inpromptu_mismatch_conference = {
    "Local conference tester (Inpromptu Conference with mismatch)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_inpromptu_mismatch_conference_tests) /
        sizeof(local_conference_inpromptu_mismatch_conference_tests[0]),
    local_conference_inpromptu_mismatch_conference_tests,
    0,
    4};
