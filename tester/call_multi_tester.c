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


#include <sys/types.h>
#include <sys/stat.h>
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "mediastreamer2/msutils.h"
#include "belle-sip/sipstack.h"

#ifdef _WIN32
#define unlink _unlink
#endif

/*
 * With IPV6, Flexisip automatically switches to TCP, so it's no more possible to really have Laure configured with UDP
 * Anyway for IPV4, it's still a good opportunity to test UDP.
 */
static const char* get_laure_rc(void) {
	if (liblinphone_tester_ipv6_available()) {
		return "laure_tcp_rc";
	} else {
		return "laure_rc_udp";
	}
}

void destroy_mgr_in_conference(LinphoneCoreManager *mgr) {
	if (mgr->user_info) {
		ms_free(mgr->user_info);
	}

	linphone_core_manager_destroy(mgr);
}

static void call_waiting_indication_with_param(bool_t enable_caller_privacy) {
	bctbx_list_t *iterator;
	bctbx_list_t* lcs;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* pauline_called_by_laure=NULL;
	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_remove_supported_tag(marie->lc,"gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline, "pauline_tcp_rc", NULL);
	linphone_core_remove_supported_tag(pauline->lc,"gruu");
	linphone_core_manager_start(pauline,TRUE);
	LinphoneCoreManager *laure = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(laure, get_laure_rc(), NULL);
	linphone_core_remove_supported_tag(laure->lc,"gruu");
	linphone_core_manager_start(laure, TRUE);
	LinphoneCallParams *laure_params=linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params=linphone_core_create_call_params(marie->lc, NULL);

	if (enable_caller_privacy)
		linphone_call_params_set_privacy(marie_params,LinphonePrivacyId);

	lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

	BC_ASSERT_TRUE(call_with_caller_params(marie,pauline,marie_params));
	linphone_call_params_unref(marie_params);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);

	if (enable_caller_privacy)
		linphone_call_params_set_privacy(laure_params,LinphonePrivacyId);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(laure->lc,pauline->identity,laure_params));
	linphone_call_params_unref(laure_params);

	BC_ASSERT_TRUE(wait_for(laure->lc
							,pauline->lc
							,&pauline->stat.number_of_LinphoneCallIncomingReceived
							,2));

	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCallOutgoingProgress,1, int, "%d");


	BC_ASSERT_TRUE(wait_for(laure->lc
							,pauline->lc
							,&laure->stat.number_of_LinphoneCallOutgoingRinging
							,1));

	bctbx_list_t *calls = bctbx_list_copy(linphone_core_get_calls(pauline->lc));
	for (iterator = calls; iterator; iterator = bctbx_list_next(iterator)) {
		LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(iterator);
		if (call != pauline_called_by_marie) {
			/*fine, this is the call waiting*/
			pauline_called_by_laure=call;
			linphone_call_accept(pauline_called_by_laure);
		}
	}
	bctbx_list_free(calls);

	BC_ASSERT_TRUE(wait_for(laure->lc
							,pauline->lc
							,&laure->stat.number_of_LinphoneCallConnected
							,1));

	BC_ASSERT_TRUE(wait_for(pauline->lc
								,marie->lc
								,&marie->stat.number_of_LinphoneCallPausedByRemote
								,1));

	if (pauline_called_by_laure && enable_caller_privacy )
		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(pauline_called_by_laure)),LinphonePrivacyId, int, "%d");
	/*wait a bit for ACK to be sent*/
	wait_for_list(lcs,NULL,0,1000);
	linphone_core_terminate_all_calls(pauline->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,1,10000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}
static void call_waiting_indication(void) {
	call_waiting_indication_with_param(FALSE);
}

static void call_waiting_indication_with_privacy(void) {
	call_waiting_indication_with_param(TRUE);
}

static void second_call_rejection(bool_t second_without_audio){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCall *pauline_call;
	LinphoneCallParams *params;
	LinphoneCall *marie_call;

	/*start a call to pauline*/
	linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc
			,pauline->lc
			,&marie->stat.number_of_LinphoneCallOutgoingRinging
			,1));

	/*attempt to send a second call while the first one is not answered.
	 * It must be rejected by the core, since the audio resources are already engaged for the first call*/
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_audio(params, !second_without_audio);
	marie_call = linphone_core_invite_with_params(marie->lc, "sip:laure_non_exstent@test.linphone.org", params);

	linphone_call_params_unref(params);

	if (second_without_audio){
		BC_ASSERT_PTR_NOT_NULL(marie_call);
		BC_ASSERT_TRUE(wait_for(marie->lc
			,pauline->lc
			,&marie->stat.number_of_LinphoneCallError
			,1));

	}else{
		BC_ASSERT_PTR_NULL(marie_call);
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call){
		linphone_call_accept(pauline_call);
	}
	BC_ASSERT_TRUE(wait_for(marie->lc
			,pauline->lc
			,&marie->stat.number_of_LinphoneCallStreamsRunning
			,1));
	BC_ASSERT_TRUE(wait_for(marie->lc
			,pauline->lc
			,&pauline->stat.number_of_LinphoneCallStreamsRunning
			,1));
	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void second_call_rejected_if_first_one_in_progress(void){
	second_call_rejection(FALSE);
}

static void second_call_allowed_if_not_using_audio(void){
	second_call_rejection(TRUE);
}

static void incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallState state) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( get_laure_rc());
	bctbx_list_t* lcs;
	LinphoneCallParams *laure_params=linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params=linphone_core_create_call_params(marie->lc, NULL);

	lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);


	if (state==LinphoneCallOutgoingRinging || state==LinphoneCallOutgoingEarlyMedia) {
		BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc,pauline->identity,marie_params));

		BC_ASSERT_TRUE(wait_for(marie->lc
								,pauline->lc
								,&pauline->stat.number_of_LinphoneCallIncomingReceived
								,1));

		if (state==LinphoneCallOutgoingEarlyMedia)
			linphone_call_accept_early_media(linphone_core_get_current_call(pauline->lc));

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingProgress,1, int, "%d");
		BC_ASSERT_TRUE(wait_for(marie->lc
									,pauline->lc
									,state==LinphoneCallOutgoingEarlyMedia?&marie->stat.number_of_LinphoneCallOutgoingEarlyMedia:&marie->stat.number_of_LinphoneCallOutgoingRinging
									,1));
       BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingbackTone, 1));
	} else if (state==LinphoneCallOutgoingProgress) {
		BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(marie->lc,pauline->identity));
	} else {
		ms_error("Unsupported state");
		return;
	}

	BC_ASSERT_TRUE(call_with_caller_params(laure,marie,laure_params));

	if (state==LinphoneCallOutgoingRinging) {
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingtone, 0, int, "%d");
	} else if (state==LinphoneCallOutgoingProgress || state==LinphoneCallOutgoingEarlyMedia) {
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingtone, 1, int, "%d");
	}

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,10000));


	linphone_core_terminate_all_calls(marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,2,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,2,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,1,10000));

	linphone_call_params_unref(laure_params);
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}
static void incoming_call_accepted_when_outgoing_call_in_progress(void) {
	incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallOutgoingProgress);
}
static void incoming_call_accepted_when_outgoing_call_in_outgoing_ringing(void) {
	incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallOutgoingRinging);
}
static void incoming_call_accepted_when_outgoing_call_in_outgoing_ringing_early_media(void) {
	incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallOutgoingEarlyMedia);
}

static void simple_conference_base(LinphoneCoreManager* marie, LinphoneCoreManager* pauline, LinphoneCoreManager* laure, LinphoneCoreManager *focus, bool_t pause_and_hangup) {
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;
	stats initial_focus_stat;

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	LinphoneConference *conference;
	const bctbx_list_t* calls;
	bool_t is_remote_conf;
	bool_t focus_is_up = (focus && ((LinphoneConferenceServer *)focus)->reg_state == LinphoneRegistrationOk);
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);
	if (focus) lcs=bctbx_list_append(lcs,focus->lc);

	is_remote_conf = (strcmp(linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "conference_type", "local"), "remote") == 0);
	if(is_remote_conf) BC_ASSERT_PTR_NOT_NULL(focus);

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	
	if (!BC_ASSERT_TRUE(call(marie,pauline))) goto end;

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	marie_call_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;

	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,laure))) goto end;

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	initial_marie_stat=marie->stat;
	initial_pauline_stat=pauline->stat;
	initial_laure_stat=laure->stat;
	if (focus_is_up) initial_focus_stat=focus->stat;

	marie_call_laure=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t* new_participants=bctbx_list_append(NULL,laure);
	// TODO: Find a way to extract participants managers from conference
	bctbx_list_t* lcs2=bctbx_list_append(NULL,marie->lc);
	lcs2=bctbx_list_append(lcs2,laure->lc);

	if(!is_remote_conf) {
		add_calls_to_local_conference(lcs2, marie, new_participants);
	} else {

		if(focus_is_up) {
			lcs2=bctbx_list_append(lcs2,focus->lc);
			add_calls_to_remote_conference(lcs2, focus, marie, new_participants);

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

			// Check subscriptions
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,(initial_marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1),1000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&focus->stat.number_of_LinphoneSubscriptionIncomingReceived,(initial_focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 2),1000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,(initial_marie_stat.number_of_LinphoneSubscriptionActive + 1),1000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&focus->stat.number_of_LinphoneSubscriptionActive,(initial_focus_stat.number_of_LinphoneSubscriptionActive + 2),1000));

		} else {
			linphone_core_add_to_conference(marie->lc,marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallError, initial_marie_stat.number_of_LinphoneCallError+1, 5000));
			BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
			BC_ASSERT_EQUAL(linphone_core_terminate_conference(marie->lc), -1, int, "%d");
			linphone_call_terminate(marie_call_pauline);
			linphone_call_terminate(marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, initial_marie_stat.number_of_LinphoneCallEnd+2, 10000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, initial_pauline_stat.number_of_LinphoneCallEnd+1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, initial_laure_stat.number_of_LinphoneCallEnd+1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, initial_marie_stat.number_of_LinphoneCallReleased+3, 10000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, initial_pauline_stat.number_of_LinphoneCallReleased+1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, initial_laure_stat.number_of_LinphoneCallReleased+1, 5000));

			bctbx_list_free(lcs2);
			bctbx_list_free(new_participants);
			goto end;
		}
	}
	bctbx_list_free(lcs2);
	bctbx_list_free(new_participants);


	new_participants=bctbx_list_append(NULL,pauline);
	if(!is_remote_conf) {
		add_calls_to_local_conference(lcs, marie, new_participants);
	} else {
		add_calls_to_remote_conference(lcs, focus, marie, new_participants);
	}
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,initial_pauline_stat.number_of_LinphoneCallStreamsRunning+1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,initial_laure_stat.number_of_LinphoneCallStreamsRunning+1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning+2,3000));

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));

	if(!is_remote_conf) {
		BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");
		BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));
	} else {
		// Participants are Focus (local participant of the local conference), Pauline and Laure
		// Marie is also in
		BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
	}
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));


	/*
	 * FIXME: check_ice() cannot work as it is today because there is no current call for the party that hosts the conference
	if (linphone_core_get_firewall_policy(marie->lc) == LinphonePolicyUseIce) {
		if (linphone_core_get_firewall_policy(pauline->lc) == LinphonePolicyUseIce) {
			check_ice(marie,pauline,LinphoneIceStateHostConnection);
		}
		if (linphone_core_get_firewall_policy(laure->lc) == LinphonePolicyUseIce) {
			check_ice(marie,laure,LinphoneIceStateHostConnection);
		}
	}
	*/
	for (calls=linphone_core_get_calls(marie->lc);calls!=NULL;calls=calls->next) {
		LinphoneCall *call=(LinphoneCall *)calls->data;
		BC_ASSERT_EQUAL(linphone_core_get_media_encryption(marie->lc),linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),int,"%d");
	}

	BC_ASSERT_PTR_NOT_NULL(conference = linphone_core_get_conference(marie->lc));
	if(conference) {
		bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
		if(!is_remote_conf) {
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
		} else {
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 3, unsigned int, "%u");
		}
		bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);
	}

	if (pause_and_hangup) {
		if (!is_remote_conf){
			/* Since Laure has been removed, the conference will automatically disapear to let
			 * Pauline and Marie communicate directly through a normal Call.
			 */
			linphone_core_pause_call(marie->lc, marie_call_pauline);
			remove_participant_from_local_conference(lcs, marie, laure);
			linphone_core_terminate_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,is_remote_conf?2:1,10000));
			BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning+3,10000));
			BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
			linphone_call_terminate(marie_call_pauline);
		} else {
			linphone_core_pause_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,2,10000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausedByRemote,1,10000));
			linphone_core_terminate_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,is_remote_conf?2:1,10000));
		}
	}else{
		if (is_remote_conf){
			linphone_core_terminate_conference(marie->lc);
		} else {
			terminate_local_conference(lcs, marie);
		}
	}
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,is_remote_conf?2:1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,is_remote_conf?3:2,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,is_remote_conf?2:1,10000));
	if(is_remote_conf) BC_ASSERT_TRUE(wait_for_list(lcs,&focus->stat.number_of_LinphoneCallEnd,3,10000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,is_remote_conf?2:1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,is_remote_conf?3:2,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,is_remote_conf?2:1,10000));
	if(is_remote_conf) BC_ASSERT_TRUE(wait_for_list(lcs,&focus->stat.number_of_LinphoneCallReleased,3,10000));

end:
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	// Wait for all conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminationPending, pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateTerminated, pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateDeleted, pauline->stat.number_of_LinphoneConferenceStateCreated, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending, laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated, laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted, laure->stat.number_of_LinphoneConferenceStateCreated, 5000));

	// If focus is ot registered, the conference is not attached to any core, hence it cannot be destroyed
	if (!(is_remote_conf && !focus_is_up)) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	}

	bctbx_list_free(lcs);
}

static void simple_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	simple_conference_base(marie,pauline,laure, NULL, FALSE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

/*
static void _simple_conference_from_scratch(bool_t with_video){
#if 0
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneConference *conf;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call;
	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	
	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	//marie creates the conference
	conf_params = linphone_core_create_conference_params(marie->lc);
	linphone_conference_params_enable_video(conf_params, with_video);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	if (with_video){
		LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
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
	}

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, pauline);

	add_participant_to_local_conference_through_invite(lcs, marie, participants, NULL);

	pauline_call = linphone_core_get_current_call(pauline->lc);
	laure_call = linphone_core_get_current_call(laure->lc);

	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(laure_call);

	if (pauline_call && laure_call){
		const bctbx_list_t *marie_calls, *it;
		linphone_call_accept(pauline_call);
		linphone_call_accept(laure_call);

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,2,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,2,3000));

		//make sure that the two calls from Marie's standpoint are in conference
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 2, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next){
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
		}
		//wait a bit for the conference audio processing to run, despite we do not test it for the moment
		wait_for_list(lcs,NULL,0,5000);
		
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) == with_video);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == with_video);

		terminate_local_conference(lcs, marie);
	}
	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
#else
	BC_FAIL("Test temporally disabled because of crash");
#endif
}

static void simple_conference_from_scratch(void){
	_simple_conference_from_scratch(FALSE);
}

static void simple_conference_from_scratch_with_video(void){
	_simple_conference_from_scratch(TRUE);
}
*/

static void video_conference_by_merging_calls(void){
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneConference *conf = NULL;
	LinphoneCallParams *params;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call;
	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	bctbx_list_t *participants = NULL;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	
	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

	participants = bctbx_list_append(participants, pauline->identity);
	participants = bctbx_list_append(participants, laure->identity);
	
	{
		LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
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
	}
	

	// Marie first estabishes a call with Pauline, with video.*/
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(params, TRUE);
	pauline_call = linphone_core_invite_address_with_params(marie->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,1,10000));
	if (BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,10000))){
		if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc))) goto end;
		linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	}else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	
	// Then she calls Laure, audio-only. */
	
	laure_call = linphone_core_invite_address(marie->lc, laure->identity);
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,2,10000));
	if (BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,10000))){
		if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc))) goto end;
		linphone_call_accept(linphone_core_get_current_call(laure->lc));
	}else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,2,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	
	// This of course puts on hold Pauline. */
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausedByRemote,1,5000));
	
	// Marie now creates a conference, audio-only.*/
	conf_params = linphone_core_create_conference_params(marie->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	
	// She adds Pauline and Laure to the conference. */
	bctbx_list_t* new_participants=bctbx_list_append(NULL,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	// Now check that both Pauline and Laure have video. */
	pauline_call = linphone_core_get_current_call(pauline->lc);
	laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	if (pauline_call && laure_call){
		const bctbx_list_t *marie_calls, *it;
		
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,4,5000));

		//make sure that the two calls from Marie's standpoint are in conference*/
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 2, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next){
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
		}
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) == FALSE);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == FALSE);
		
		ms_message("Adding video to the conference...");
		// Marie upgrades the conference with video. */
		conf_params = linphone_core_create_conference_params(marie->lc);
		linphone_conference_params_enable_video(conf_params, TRUE);
		linphone_conference_update_params(conf, conf_params);
		linphone_conference_params_unref(conf_params);
		
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,3,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,3,3000));
		
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) == TRUE);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == TRUE);
		
		// Make sure video is received for Pauline and Laure. For Marie we can't because of missing APIs.*/
		liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
		liblinphone_tester_set_next_video_frame_decoded_cb(laure_call);

		BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_IframeDecoded ,1, 5000));
		BC_ASSERT_TRUE( wait_for_list(lcs ,&laure->stat.number_of_IframeDecoded, 1, 5000));
		
		// Change camera, unfortunately there is no way to test its effectiveness for the moment. */
		ms_message("Changing Marie's video device...");
		linphone_core_set_video_device(marie->lc, liblinphone_tester_static_image_id);
		wait_for_list(lcs ,NULL, 0, 2000);

		terminate_local_conference(lcs, marie);
	}
	
	
end:	
	if (conf) linphone_conference_unref(conf);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

/*
static void simple_conference_from_scratch_no_answer(void){
#if 0
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneConference *conf;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	//marie creates the conference
	conf_params = linphone_core_create_conference_params(marie->lc);
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
	if (pauline_call){
		pauline_call = linphone_core_get_current_call(pauline->lc);
		// Pauline immediately declines the call.
		linphone_call_decline(pauline_call, LinphoneReasonDeclined);
	}
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,5000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,1,5000));

	// Wait for all conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));

	wait_for_list(lcs,NULL,0,1000);

	lcs = bctbx_list_append(lcs, laure->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,10000));
	laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);

	if ( laure_call){
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,2,2000));
		
		
		wait_for_list(lcs,NULL,0,1000);
		
		// Laure accepts.
		linphone_call_accept(laure_call);
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,5000));

		wait_for_list(lcs,NULL,0,1000);
		
		// the conference no longer exists, as there was finally only one participant.
		// Terminate the call, simply.
		linphone_call_terminate(laure_call);

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,2,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,1,1000));

	}
	
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
#else
	BC_FAIL("Test temporally disabled as, call.update is performed in stage outgoinginit");
#endif
}
*/

static void simple_encrypted_conference_with_ice(LinphoneMediaEncryption mode) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());

	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(laure->lc,LinphonePolicyUseIce);

		linphone_core_set_media_encryption(marie->lc,mode);
		linphone_core_set_media_encryption(pauline->lc,mode);
		linphone_core_set_media_encryption(laure->lc,mode);

		simple_conference_base(marie,pauline,laure,NULL,FALSE);
	} else {
		ms_warning("No [%s] support available",linphone_media_encryption_to_string(mode));
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

static void conference_hang_up_call_on_hold(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference("pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference(get_laure_rc());
	simple_conference_base(marie, pauline, laure, NULL, TRUE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void simple_call_transfer(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( get_laure_rc());
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall *marie_calling_pauline;
	LinphoneCall *marie_calling_laure;

	char* laure_identity=linphone_address_as_string(laure->identity);
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);


	BC_ASSERT_TRUE(call(marie,pauline));
	marie_calling_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_calling_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;


	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);


	linphone_call_transfer(pauline_called_by_marie,laure_identity);
	bctbx_free(laure_identity);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallRefered,1,2000));
	//marie pausing pauline
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausing,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausedByRemote,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,1,2000));
	//marie calling laure
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,1,2000));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_transfer_target_call(marie_calling_pauline));

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneTransferCallOutgoingInit,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneTransferCallOutgoingProgress,1,2000));
	if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc))) goto end;
	linphone_call_accept(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,2000));

	marie_calling_laure=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_calling_laure)) goto end;
	BC_ASSERT_PTR_EQUAL(linphone_call_get_transferer_call(marie_calling_laure),marie_calling_pauline);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneTransferCallConnected,1,2000));

	//terminate marie to pauline call
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,1,2000));

	end_call(marie, laure);
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,1,2000));

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}

static void unattended_call_transfer(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( get_laure_rc());
	LinphoneCall* pauline_called_by_marie;

	char* laure_identity=linphone_address_as_string(laure->identity);
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);


	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_called_by_marie=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);

	linphone_call_transfer(pauline_called_by_marie,laure_identity);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,3000));

	//marie ends the call
	linphone_call_terminate(pauline_called_by_marie);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,3000));

	//Pauline starts the transfer
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingInit,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc))) goto end;
	linphone_call_accept(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,3000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,3000));

	end_call(laure, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}

static void unattended_call_transfer_with_error(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCall* pauline_called_by_marie;
	bool_t call_ok=TRUE;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);

	BC_ASSERT_TRUE((call_ok=call(marie,pauline)));
	if (call_ok){
		pauline_called_by_marie=linphone_core_get_current_call(marie->lc);

		if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);

		linphone_call_transfer(pauline_called_by_marie,"unknown_user");
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,2000));

		//Pauline starts the transfer
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingInit,1,2000));
		// and immediately get an error
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallError,1,2000));

		//the error must be reported back to marie
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallError,1,2000));

		//and pauline should resume the call automatically
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallResuming,1,2000));

		//and call should be resumed
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,2000));

		end_call(marie, pauline);
	}
end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}


static void call_transfer_existing_call(bool_t outgoing_call, bool_t auto_answer_replacing_calls) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( get_laure_rc());
	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	LinphoneCall* laure_called_by_marie;
	LinphoneCall* lcall;
	bool_t call_ok=TRUE;
	const bctbx_list_t* calls;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);
	
	if (!auto_answer_replacing_calls)
		linphone_config_set_int(linphone_core_get_config(laure->lc), "sip", "auto_answer_replacing_calls", 0);

	//marie call pauline
	BC_ASSERT_TRUE((call_ok=call(marie,pauline)));
	if (call_ok){
		marie_call_pauline=linphone_core_get_current_call(marie->lc);
		pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
		//marie pause pauline
		if (!BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie))) {
			goto end;
		}

		if (outgoing_call) {
			//marie call laure
			if (!BC_ASSERT_TRUE(call(marie,laure))) {
				end_call(marie, pauline);
				goto end;
			}
		} else {
			//laure call pauline
			if (!BC_ASSERT_TRUE(call(laure,marie))) {
				end_call(marie,pauline);
				goto end;
			}
		}

		marie_call_laure=linphone_core_get_current_call(marie->lc);
		laure_called_by_marie=linphone_core_get_current_call(laure->lc);
		if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;
		if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_marie)) goto end;

		//marie pause laure
		BC_ASSERT_TRUE(pause_call_1(marie,marie_call_laure,laure,laure_called_by_marie));

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);
		reset_counters(&laure->stat);


		linphone_call_transfer_to_another(marie_call_pauline,marie_call_laure);
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,2000));

		//pauline pausing marie
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausing,1,4000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPaused,1,4000));
		//pauline calling laure
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallOutgoingInit,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallOutgoingProgress,1,2000));

		//laure accept call
		if (!auto_answer_replacing_calls){
			for(calls=linphone_core_get_calls(laure->lc);calls!=NULL;calls=calls->next) {
				lcall = (LinphoneCall*)calls->data;
				if (linphone_call_get_state(lcall) == LinphoneCallIncomingReceived) {
					BC_ASSERT_PTR_EQUAL(linphone_call_get_replaced_call(lcall),laure_called_by_marie);
					linphone_call_accept(lcall);
					break;
				}
			}
		}
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallConnected,1,2000));

		//terminate marie to pauline/laure call
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,2,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,2,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,2000));

		end_call(pauline, laure);
	}
end:
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

static void call_transfer_existing_call_outgoing_call(void) {
	call_transfer_existing_call(TRUE, TRUE);
}

static void call_transfer_existing_call_outgoing_call_no_auto_answer(void) {
	call_transfer_existing_call(TRUE, FALSE);
}

static void call_transfer_existing_call_incoming_call(void) {
	call_transfer_existing_call(FALSE, TRUE);
}

static void call_transfer_existing_ringing_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *lcall;
	LinphoneCall *laure_call;
	bool_t call_ok = TRUE;
	const bctbx_list_t *calls;
	stats initial_marie_stats;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	// marie calls pauline 
	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (call_ok) {
		marie_call_pauline = linphone_core_get_current_call(marie->lc);
		pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
		// marie pauses pauline 
		if (!BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie))) goto end;

		initial_marie_stats = marie->stat;
		BC_ASSERT_PTR_NOT_NULL((marie_call_laure = linphone_core_invite_address(marie->lc, laure->identity)));
		if (!marie_call_laure) goto end;
		BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, initial_marie_stats.number_of_LinphoneCallOutgoingRinging + 1));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
		laure_call = linphone_core_get_current_call(laure->lc);
		if (!BC_ASSERT_PTR_NOT_NULL(laure_call)) goto end;
		
		linphone_call_transfer_to_another(marie_call_pauline, marie_call_laure);
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallRefered, 1, 2000));

		// pauline pausing marie 
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing, 1, 4000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused, 1, 4000));
		// pauline calling laure 
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingInit, 1, 2000));
		
		
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingProgress, 1, 2000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 2, 2000));
		// laure accepts the new (replacing) call 
		for (calls = linphone_core_get_calls(laure->lc); calls != NULL; calls = calls->next) {
			lcall = (LinphoneCall*)calls->data;
			if (lcall == laure_call) continue;
			if (linphone_call_get_state(lcall) == LinphoneCallIncomingReceived) {
				linphone_call_accept(lcall);
				break;
			}
		}
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallConnected, 1, 2000));

		// terminate marie to pauline/laure call 
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 2000));

		end_call(pauline, laure);
	}

end:
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

static void eject_from_3_participants_conference(LinphoneCoreManager *marie, LinphoneCoreManager *pauline, LinphoneCoreManager *laure, LinphoneCoreManager *focus) {
	stats initial_marie_stat;
	stats initial_pauline_stat;
	stats initial_laure_stat;

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	bool_t is_remote_conf;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);
	if(focus) lcs=bctbx_list_append(lcs,focus->lc);

	is_remote_conf = (strcmp(linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "conference_type", "local"), "remote") == 0);
	if(is_remote_conf) BC_ASSERT_PTR_NOT_NULL(focus);

	if (!BC_ASSERT_TRUE(call(marie,pauline))) goto end;
	marie_call_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,laure))) goto end;
	initial_marie_stat=marie->stat;
	initial_pauline_stat=pauline->stat;
	initial_laure_stat=laure->stat;

	marie_call_laure=linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	if(!is_remote_conf) {
		bctbx_list_t* new_participants=bctbx_list_append(NULL,laure);
		// TODO: Find a way to extract participants managers from conference
		bctbx_list_t* lcs2=bctbx_list_append(NULL,marie->lc);
		lcs2=bctbx_list_append(lcs2,laure->lc);
		add_calls_to_local_conference(lcs2, marie, new_participants);
		bctbx_list_free(lcs2);
		bctbx_list_free(new_participants);
	} else {

		linphone_core_add_to_conference(marie->lc,marie_call_laure);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, initial_marie_stat.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, initial_marie_stat.number_of_LinphoneConferenceStateCreated + 1, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallConnected,initial_marie_stat.number_of_LinphoneTransferCallConnected+1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd+1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd+1,5000));
	}

	if (!BC_ASSERT_PTR_NOT_NULL(linphone_core_get_conference(marie->lc))) {
		goto end;
	}


	if(!is_remote_conf) {
		bctbx_list_t* new_participants=bctbx_list_append(NULL,pauline);
		add_calls_to_local_conference(lcs, marie, new_participants);
		bctbx_list_free(new_participants);
	} else {
		linphone_core_add_to_conference(marie->lc,marie_call_pauline);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallConnected,initial_marie_stat.number_of_LinphoneTransferCallConnected+2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd+2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,initial_pauline_stat.number_of_LinphoneCallEnd+1,5000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,initial_pauline_stat.number_of_LinphoneCallStreamsRunning+1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,initial_laure_stat.number_of_LinphoneCallStreamsRunning+1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning+2,3000));

	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));

	if(!is_remote_conf) BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	ms_message("Removing pauline from conference.");
	if(!is_remote_conf) {
		BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");
		remove_participant_from_local_conference(lcs, marie, pauline);
	} else {
		BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");
		LinphoneConference *conference = linphone_core_get_conference(marie->lc);
		const LinphoneAddress *uri = linphone_call_get_remote_address(marie_call_pauline);
		LinphoneParticipant * pauline_participant = linphone_conference_find_participant(conference, uri);
		BC_ASSERT_PTR_NOT_NULL(pauline_participant);
		linphone_conference_remove_participant_2(conference, pauline_participant);

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,initial_pauline_stat.number_of_LinphoneCallEnd+2,5000));

	}

	if(!is_remote_conf) {
		end_call(laure, marie);
		end_call(pauline, marie);

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,initial_laure_stat.number_of_LinphoneCallReleased+1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,initial_marie_stat.number_of_LinphoneCallReleased+1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,initial_pauline_stat.number_of_LinphoneCallReleased+1,3000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminationPending, laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateTerminated, laure->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateDeleted, laure->stat.number_of_LinphoneConferenceStateCreated, 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));

	} else {

		linphone_core_terminate_conference(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd+2,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd+3,3000));
	}
end:
	bctbx_list_free(lcs);
}

static void eject_from_3_participants_local_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());

	eject_from_3_participants_conference(marie, pauline, laure, NULL);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void eject_from_4_participants_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);

	BC_ASSERT_TRUE(call(marie,pauline));
	marie_call_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	marie_call_michelle=linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie=linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_michelle,michelle,michelle_called_by_marie));

	BC_ASSERT_TRUE(call(marie,laure));

	marie_call_laure=linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,laure);
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	/* Wait that the three participants are joined to the local conference, by checking the StreamsRunning states*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	remove_participant_from_local_conference(lcs, marie, pauline);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 3, unsigned int, "%u");
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	wait_for_list(lcs ,NULL, 0, 2000);

	terminate_local_conference(lcs, marie);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void participants_exit_conference_after_pausing(void) {
#if 0
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);

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

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,laure);
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	stats marie_stats = marie->stat;
	stats pauline_stats = pauline->stat;
	stats laure_stats = laure->stat;

	int conf_size = linphone_core_get_conference_size(marie->lc);

	LinphoneCall * laure_call_marie = linphone_core_get_current_call(laure->lc);
	linphone_core_pause_call(laure->lc, laure_call_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausing, laure_stats.number_of_LinphoneCallPausing + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPaused, laure_stats.number_of_LinphoneCallPaused + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote, marie_stats.number_of_LinphoneCallPausedByRemote + 1, 10000));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(laure->lc));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),conf_size, int, "%d");

	LinphoneCall * pauline_call_marie = linphone_core_get_current_call(pauline->lc);
	linphone_core_pause_call(laure->lc, pauline_call_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausing, pauline_stats.number_of_LinphoneCallPausing + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPaused, pauline_stats.number_of_LinphoneCallPaused + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote, marie_stats.number_of_LinphoneCallPausedByRemote + 2, 10000));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),conf_size, int, "%d");

	// Remove Laure, whose call is paused, from the conference.
	remove_participant_from_local_conference(lcs, marie, laure);
	lcs=bctbx_list_remove(lcs,laure->lc);

	// Remove Michelle from conference.
	// This should also take out Pauline from the conference whose call is paused.
	remove_participant_from_local_conference(lcs, marie, michelle);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
#else
	BC_FAIL("Test temporally disabled because of crash");
#endif
}

static void add_participant_after_conference_started(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);

	BC_ASSERT_TRUE(call(marie,pauline));
	marie_call_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	lcs=bctbx_list_append(lcs,laure->lc);

	BC_ASSERT_TRUE(call(marie,laure));

	marie_call_laure=linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t* additional_participants=NULL;
	additional_participants=bctbx_list_append(additional_participants,laure);
	add_calls_to_local_conference(lcs, marie, additional_participants);
	bctbx_list_free(additional_participants);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	wait_for_list(lcs ,NULL, 0, 2000);

	terminate_local_conference(lcs, marie);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void set_video_in_conference(bctbx_list_t* lcs, LinphoneCoreManager* conf, bctbx_list_t* participants, bool_t enable_video) {
	LinphoneConference *conference = linphone_core_get_conference(conf->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	stats* initial_stats = NULL;
	int idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_stats = (stats*)realloc(initial_stats, (idx+1) * sizeof(stats));
		// Append element
		initial_stats[idx] = m->stat;

		idx++;
	}

	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	if (conference) {
		stats initial_conf_stat = conf->stat;
		const LinphoneConferenceParams * old_params = linphone_conference_get_current_params(conference);
		LinphoneConferenceParams * new_params = linphone_conference_params_clone(old_params);
		linphone_conference_params_enable_video (new_params, enable_video);
		BC_ASSERT_TRUE(linphone_conference_update_params(conference, new_params));
		linphone_conference_params_unref (new_params);
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf->stat.number_of_LinphoneCallUpdating, initial_conf_stat.number_of_LinphoneCallUpdating + no_participants, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf->stat.number_of_LinphoneCallStreamsRunning, initial_conf_stat.number_of_LinphoneCallStreamsRunning + no_participants, 5000));
	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(conf->lc));
	// number of participants is incremented by 1 because Marie is in
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(conf->lc),no_participants+1, int, "%d");

	idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;

		LinphoneCall * participant_call = linphone_core_get_current_call(c);
		BC_ASSERT_PTR_NOT_NULL(participant_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdatedByRemote, initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1, 5000));

		// Ensure that the core has not been kicked out of the conference
		BC_ASSERT_TRUE(linphone_call_is_in_conference(participant_call));

		const LinphoneCallParams * participant_call_params = linphone_call_get_current_params(participant_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(participant_call_params) == enable_video);

		const LinphoneAddress *participant_uri = m->identity;
		LinphoneCall * conf_call = linphone_core_get_call_by_remote_address(conf->lc, linphone_address_as_string(participant_uri));
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		const LinphoneCallParams * conf_call_params = linphone_call_get_current_params(conf_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(conf_call_params) == enable_video);

		LinphoneConference *pconference = linphone_call_get_conference(participant_call);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			// number of participants is incremented by 1 because Marie is in
			BC_ASSERT_EQUAL(linphone_conference_get_size(pconference),no_participants+1, int, "%d");
		}

		if (enable_video) {
			// Make sure video is received for Pauline and Laure. For Marie we can't because of missing APIs.*/
			liblinphone_tester_set_next_video_frame_decoded_cb(participant_call);
			BC_ASSERT_TRUE( wait_for_list(lcs, &m->stat.number_of_IframeDecoded, initial_stats[idx].number_of_IframeDecoded + 1, 5000));
		}

		idx++;
	}
	ms_free(initial_stats);

	const LinphoneConferenceParams * params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == enable_video);
}

static void update_conf_params_during_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

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

		LinphoneVideoActivationPolicy * cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol));
		linphone_video_activation_policy_unref(cpol);
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
	add_calls_to_local_conference(lcs, marie, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	// Wait that the three participants have joined the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));

	// Enable video
	bool_t video_enabled = TRUE;
	set_video_in_conference(lcs, marie, new_participants, video_enabled);

	wait_for_list(lcs ,NULL, 0, 2000);

	// Disable video
	video_enabled = FALSE;
	set_video_in_conference(lcs, marie, new_participants, video_enabled);

	BC_ASSERT_TRUE(linphone_call_is_in_conference(pauline_called_by_marie));
	if (pauline_called_by_marie) {
		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		stats initial_laure_stat = laure->stat;
		stats initial_michelle_stat = michelle->stat;
		linphone_core_terminate_call(marie->lc, pauline_called_by_marie);
		new_participants = bctbx_list_remove(new_participants, pauline);
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallReleased,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallReleased,1));

		// Wait for conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateTerminated,(initial_pauline_stat.number_of_LinphoneConferenceStateTerminated + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateDeleted,(initial_pauline_stat.number_of_LinphoneConferenceStateDeleted + 1),5000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,initial_pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_NotifyReceived,(initial_laure_stat.number_of_NotifyReceived + 2),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_NotifyReceived,(initial_michelle_stat.number_of_NotifyReceived + 2),5000));
	}

	unsigned int no_participants = (unsigned int)bctbx_list_size(new_participants);
	// number of participants is incremented by 1 because Marie is in
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),no_participants+1, int, "%d");

	for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;

		LinphoneCall * participant_call = linphone_core_get_current_call(c);
		BC_ASSERT_PTR_NOT_NULL(participant_call);

		// Ensure that the core has not been kicked out of the conference
		BC_ASSERT_TRUE(linphone_call_is_in_conference(participant_call));
		const LinphoneCallParams * participant_call_params = linphone_call_get_current_params(participant_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(participant_call_params) == video_enabled);

		const LinphoneAddress *participant_uri = m->identity;
		LinphoneCall * conf_call = linphone_core_get_call_by_remote_address(marie->lc, linphone_address_as_string(participant_uri));
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		const LinphoneCallParams * conf_call_params = linphone_call_get_current_params(conf_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(conf_call_params) == video_enabled);

		LinphoneConference *pconference = linphone_call_get_conference(participant_call);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			// number of participants is incremented by 1 because Marie is in
			BC_ASSERT_EQUAL(linphone_conference_get_size(pconference),no_participants+1, int, "%d");
		}
	}

	wait_for_list(lcs ,NULL, 0, 2000);

	terminate_local_conference(lcs, marie);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	if (new_participants) bctbx_list_free(new_participants);
	bctbx_list_free(lcs);
}

static void try_to_update_call_params_during_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

	LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_core_set_video_activation_policy(michelle->lc, pol);
	linphone_video_activation_policy_unref(pol);

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	LinphoneCall* marie_call_laure;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

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

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_TRUE(linphone_call_is_in_conference(pauline_called_by_marie));

	if (pauline_called_by_marie) {
		linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, TRUE);

		linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);

		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		const LinphoneCallParams *old_params = linphone_call_get_params(pauline_called_by_marie);
		LinphoneCallParams * new_params = linphone_call_params_copy(old_params);
		linphone_call_params_enable_video (new_params, TRUE);
		BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(old_params));
		BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(new_params));
		// Update call params should return false as Pauline is not allowed to do so
		BC_ASSERT_FALSE(linphone_call_update(pauline_called_by_marie, new_params));
		linphone_call_params_unref (new_params);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote, initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating, initial_pauline_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	// Ensure that the core has not been kicked out of the conference
	BC_ASSERT_TRUE(linphone_call_is_in_conference(pauline_called_by_marie));
	if (pauline_called_by_marie) {
		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		stats initial_laure_stat = laure->stat;
		stats initial_michelle_stat = michelle->stat;
		linphone_core_terminate_call(marie->lc, pauline_called_by_marie);
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallReleased,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallReleased,1));

		// Wait for conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateTerminated,(initial_pauline_stat.number_of_LinphoneConferenceStateTerminated + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateDeleted,(initial_pauline_stat.number_of_LinphoneConferenceStateDeleted + 1),5000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,initial_pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_NotifyReceived,(initial_laure_stat.number_of_NotifyReceived + 2),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_NotifyReceived,(initial_michelle_stat.number_of_NotifyReceived + 2),5000));
	}

	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	terminate_local_conference(lcs, marie);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void register_again_during_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

	stats initial_marie_stats = marie->stat;
	stats initial_pauline_stats = pauline->stat;
	stats initial_laure_stats = laure->stat;

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	LinphoneCall* marie_call_laure;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

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

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));

	LinphoneProxyConfig *proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, FALSE);
	linphone_proxy_config_done(proxyConfig);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneRegistrationCleared, initial_marie_stats.number_of_LinphoneRegistrationCleared + 1, 5000));

	wait_for_list(lcs,NULL,1,3000);

	//to force re-re-connection to restarted flexisip
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, FALSE);

	proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, TRUE);
	linphone_proxy_config_done(proxyConfig);

	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneRegistrationOk, initial_marie_stats.number_of_LinphoneRegistrationOk + 1, 10000));
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneRegistrationOk, initial_pauline_stats.number_of_LinphoneRegistrationOk + 1, 10000));
	linphone_core_set_network_reachable(laure->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneRegistrationOk, initial_laure_stats.number_of_LinphoneRegistrationOk + 1, 10000));

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	terminate_local_conference(lcs, marie);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void simple_4_participants_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_laure;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);

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

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,laure);
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, new_participants);
	bctbx_list_free(new_participants);

	/* Wait that the three participants are joined to the local conference, by checking the StreamsRunning states*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	terminate_local_conference(lcs, marie);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
printf("%s - manager %p (rc %s) - notify %0d - subscribe outgoing %0d incoming %0d active %0d terminated %0d\n", __func__, m, m->rc_path, m->stat.number_of_NotifyReceived, m->stat.number_of_LinphoneSubscriptionOutgoingProgress, m->stat.number_of_LinphoneSubscriptionIncomingReceived, m->stat.number_of_LinphoneSubscriptionActive, m->stat.number_of_LinphoneSubscriptionTerminated);
	}

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void simple_conference_with_participant_with_no_event_log(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_log_enabled",FALSE);
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());

	simple_conference_base(marie,pauline,laure, NULL, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
}

void simple_remote_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc");
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc");
	LinphoneCoreManager *laure = create_mgr_for_conference(get_laure_rc());
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config = linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	linphone_proxy_config_set_conference_factory_uri(linphone_core_get_default_proxy_config(marie->lc), focus_uri);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	simple_conference_base(marie, pauline, laure, (LinphoneCoreManager *)focus, FALSE);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	linphone_conference_server_destroy(focus);
}

void simple_remote_conference_shut_down_focus(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc");
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc");
	LinphoneCoreManager *laure = create_mgr_for_conference(get_laure_rc());
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", FALSE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config = linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	linphone_proxy_config_set_conference_factory_uri(linphone_core_get_default_proxy_config(marie->lc), focus_uri);

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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	LinphoneConferenceServer *focus = linphone_conference_server_new("conference_focus_rc", TRUE);
	LpConfig *marie_config = linphone_core_get_config(marie->lc);
	LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(((LinphoneCoreManager *)focus)->lc);
	LinphoneProxyConfig *laure_proxy_config = linphone_core_get_default_proxy_config(((LinphoneCoreManager *)laure)->lc);
	const char *laure_proxy_uri = linphone_proxy_config_get_server_addr(laure_proxy_config);
	const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

	linphone_config_set_string(marie_config, "misc", "conference_type", "remote");
	linphone_proxy_config_set_conference_factory_uri(linphone_core_get_default_proxy_config(marie->lc), focus_uri);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	eject_from_3_participants_conference(marie, pauline, laure, (LinphoneCoreManager *)focus);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	linphone_conference_server_destroy(focus);
}

void do_not_stop_ringing_when_declining_one_of_two_incoming_calls(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( get_laure_rc());
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* pauline_called_by_laure;
	LinphoneCallParams *laure_params=linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params=linphone_core_create_call_params(marie->lc, NULL);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(laure->lc,pauline->identity,laure_params));
	linphone_call_params_unref(laure_params);

	BC_ASSERT_TRUE(wait_for(laure->lc
							,pauline->lc
							,&pauline->stat.number_of_LinphoneCallIncomingReceived
							,1));
	pauline_called_by_laure=linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_laure)) goto end;
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 1, int, "%d");

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc,pauline->identity,marie_params));
	linphone_call_params_unref(marie_params);

	BC_ASSERT_TRUE(wait_for(marie->lc
							,pauline->lc
							,&pauline->stat.number_of_LinphoneCallIncomingReceived
							,2));
	pauline_called_by_marie=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 1, int, "%d");

	linphone_call_decline(pauline_called_by_laure, LinphoneReasonDeclined);
	BC_ASSERT_TRUE(wait_for(laure->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(laure->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallReleased,1));

	// check that rigntone player restart
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopRingtone, 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 2, int, "%d");

	BC_ASSERT_TRUE(linphone_ringtoneplayer_is_started(linphone_core_get_ringtoneplayer(pauline->lc)));
	linphone_call_terminate(pauline_called_by_marie);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallReleased,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallEnd,2));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallReleased,2));
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_stopRingtone, 2, int, "%d");
end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(marie);
}

void no_auto_answer_on_fake_call_with_replaces_header (void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie1->lc);
	lcs = bctbx_list_append(lcs, marie2->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	LinphoneCallParams *params = linphone_core_create_call_params(marie1->lc, NULL);
	linphone_call_params_set_privacy(params, LinphonePrivacyId);
	LinphoneCall *call1 = linphone_core_invite_address_with_params(marie1->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(call1);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallOutgoingRinging, 1, 3000));

	reset_counters(&marie1->stat);
	reset_counters(&marie2->stat);
	reset_counters(&pauline->stat);

	const char *callId = linphone_call_log_get_call_id(linphone_call_get_call_log(call1));
	BC_ASSERT_PTR_NOT_NULL(callId);
	SalOp *op = linphone_call_get_op_as_sal_op(call1);
	const char *fromTag = sal_call_get_local_tag(op);
	const char *toTag = sal_call_get_remote_tag(op);
	BC_ASSERT_PTR_NOT_NULL(fromTag);
	BC_ASSERT_PTR_NOT_NULL(toTag);

	params = linphone_core_create_call_params(marie2->lc, NULL);
	char *headerValue = bctbx_strdup_printf("%s;from-tag=%s;to-tag=%s", callId, fromTag, toTag);
	linphone_call_params_add_custom_header(params, "Replaces", headerValue);
	bctbx_free(headerValue);
	LinphoneCall *call2 = linphone_core_invite_address_with_params(marie2->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(call2);
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneCallOutgoingProgress, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallOutgoingRinging, 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 2000));
	
	linphone_call_terminate(call1);
	linphone_call_terminate(call2);
	
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallReleased, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallReleased, 1, 3000));

	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void initiate_calls(bctbx_list_t* caller, LinphoneCoreManager* callee) {
	stats* initial_callers_stats = NULL;
	stats initial_callee_stat = callee->stat;

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,callee->lc);

	int counter = 1;
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		lcs=bctbx_list_append(lcs,m->lc);
		// Allocate memory
		initial_callers_stats = (stats*)realloc(initial_callers_stats, counter * sizeof(stats));
		// Append element
		initial_callers_stats[counter - 1] = m->stat;
		// Increment counter
		counter++;

		LinphoneCall * caller_call = linphone_core_invite_address(m->lc,callee->identity);
		BC_ASSERT_PTR_NOT_NULL(caller_call);
	}

	unsigned int no_callers = (unsigned int)bctbx_list_size(caller);
	BC_ASSERT_TRUE(wait_for_list(lcs,&callee->stat.number_of_LinphoneCallIncomingReceived,initial_callee_stat.number_of_LinphoneCallIncomingReceived + no_callers,5000));

	LinphoneCall * callee_call = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(callee_call);

	counter = 0;
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallOutgoingRinging,initial_callers_stats[counter].number_of_LinphoneCallOutgoingRinging + 1,5000));
		counter++;
	}

	bctbx_list_free(lcs);
}

static void initiate_call(LinphoneCoreManager* caller, LinphoneCoreManager* callee) {
	bctbx_list_t *caller_list = NULL;
	caller_list=bctbx_list_append(caller_list,caller);
	initiate_calls(caller_list, callee);
	bctbx_list_free(caller_list);

/*
	stats initial_caller_stat = caller->stat;
	stats initial_callee_stat = callee->stat;

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,caller->lc);
	lcs=bctbx_list_append(lcs,callee->lc);

	LinphoneCall * caller_call = linphone_core_invite_address(caller->lc,callee->identity);
	BC_ASSERT_PTR_NOT_NULL(caller_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&callee->stat.number_of_LinphoneCallIncomingReceived,initial_callee_stat.number_of_LinphoneCallIncomingReceived + 1,5000));

	LinphoneCall * callee_call = linphone_core_get_current_call(callee->lc);
	BC_ASSERT_PTR_NOT_NULL(callee_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&caller->stat.number_of_LinphoneCallOutgoingRinging,initial_caller_stat.number_of_LinphoneCallOutgoingRinging + 1,5000));

	bctbx_list_free(lcs);
*/
}

static void take_calls_to_callee(bctbx_list_t* lcs, bctbx_list_t* caller, LinphoneCoreManager* callee) {

	LinphoneCall * current_call = linphone_core_get_current_call(callee->lc);
	LinphoneCall * current_call_caller = NULL;
	bool_t pausing_current_call = FALSE;
	if (current_call) {
		pausing_current_call = ((linphone_call_get_state(current_call) == LinphoneCallStreamsRunning) || (linphone_call_get_state(current_call) == LinphoneCallPaused));
		char* remote_address_string = linphone_call_get_remote_address_as_string(current_call);
printf("%s - remote address of current call %s\n", __func__, linphone_call_get_remote_address_as_string(current_call));
		// Search core that matches the remote address 

		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneCoreManager * m = get_manager(c);
			char * identity_string = linphone_address_as_string(m->identity);
			bool_t manager_found = (strcmp(remote_address_string, identity_string) == 0);
printf("%s - remote address of current call %s identity %s compare %0d\n", __func__, remote_address_string, identity_string, manager_found);
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
		LinphoneCoreManager * caller_mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
		const LinphoneAddress *caller_uri = caller_mgr->identity;

		// Decremement here no_paused_by_remote because past history has to be discarded for checks. Here, it is imprtant only the delta between before and after taking the call. As below no_paused_by_remote is incremented by the stats, a corrective factor (i.e. value before taking the call) is applied here
		no_paused_by_remote -= caller_mgr->stat.number_of_LinphoneCallPausedByRemote;
		LinphoneCall * callee_call = linphone_core_get_call_by_remote_address(callee->lc, linphone_address_as_string(caller_uri));
printf("%s - manager caller %s callee %s - call %p\n", __func__, caller_mgr->rc_path, callee->rc_path, callee_call);
		BC_ASSERT_PTR_NOT_NULL(callee_call);

		// Take call - ringing ends
		linphone_call_accept(callee_call);
	}

	unsigned int no_callers = (unsigned int)bctbx_list_size(caller);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallStreamsRunning, initial_callee_stat.number_of_LinphoneCallStreamsRunning + no_callers, 5000));
	// Last call is not paused
	// If core had a running call, it will be paused
	unsigned int no_call_paused = no_callers - 1 + ((pausing_current_call) ? 1 : 0);
printf("%s - manager %s call paused %0d caller size %0d callee first call %0d\n", __func__, callee->rc_path, no_call_paused, (unsigned int)bctbx_list_size(caller), pausing_current_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPausing, initial_callee_stat.number_of_LinphoneCallPausing + no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPaused, initial_callee_stat.number_of_LinphoneCallPaused + no_call_paused, 5000));

	int updated_by_remote_count = 0;
	bool_t callee_uses_ice = (linphone_core_get_firewall_policy(callee->lc) == LinphonePolicyUseIce);
	// Wait that all calls but the last one are paused
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
		if (callee_uses_ice && linphone_core_get_firewall_policy(m->lc) == LinphonePolicyUseIce) {
			updated_by_remote_count++;
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallUpdating,1,5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallUpdatedByRemote, initial_callee_stat.number_of_LinphoneCallUpdatedByRemote + updated_by_remote_count, 5000));
			BC_ASSERT_TRUE(check_ice(m,callee,LinphoneIceStateHostConnection));
			BC_ASSERT_TRUE(check_ice(callee,m,LinphoneIceStateHostConnection));
		}
printf("%s - manager %s call paused by remote %0d caller size %0d callee first call %0d\n", __func__, m->rc_path, m->stat.number_of_LinphoneCallPausedByRemote, (unsigned int)bctbx_list_size(caller), pausing_current_call);
		// Calls can be paused in an order different from the one they are accepted For example if ICE is enabled, it may take longer to reach this state
		no_paused_by_remote += m->stat.number_of_LinphoneCallPausedByRemote;
	}
	if (pausing_current_call && current_call_caller) {
		no_paused_by_remote += (linphone_call_get_state(current_call_caller) == LinphoneCallPausedByRemote) ? 1 : 0;
	}

	BC_ASSERT_EQUAL(no_paused_by_remote,no_call_paused, int, "%d");
}

static void take_call_to_callee(bctbx_list_t* lcs, LinphoneCoreManager* caller, LinphoneCoreManager* callee) {
	bctbx_list_t *caller_list = NULL;
	caller_list=bctbx_list_append(caller_list,caller);
	take_calls_to_callee(lcs, caller_list, callee);
	bctbx_list_free(caller_list);
}

static void conference_with_calls_queued(LinphoneCoreManager* local_conf, bctbx_list_t* participants, bool_t back_to_back_invite, bool_t back_to_back_accept) {
	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,local_conf->lc);

	stats initial_local_conf_stat = local_conf->stat;

	if (back_to_back_invite == TRUE) {
		initiate_calls(participants, local_conf);
	} else {
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			initiate_call(m, local_conf);
		}
	}

	//Let ring calls for a little while
	wait_for_list(lcs,NULL,0,1000);

	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallStreamsRunning, initial_local_conf_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallPausing, initial_local_conf_stat.number_of_LinphoneCallPausing, int, "%d");
	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallPaused, initial_local_conf_stat.number_of_LinphoneCallPaused, int, "%d");
	BC_ASSERT_EQUAL(local_conf->stat.number_of_LinphoneCallPausedByRemote, initial_local_conf_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	if (back_to_back_accept == TRUE) {
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneCore * c = m->lc;
			lcs=bctbx_list_append(lcs,c);
		}
		take_calls_to_callee(lcs, participants, local_conf);
	} else {
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneCore * c = m->lc;
			lcs=bctbx_list_append(lcs,c);
			take_call_to_callee(lcs, m, local_conf);
		}
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(local_conf->lc));

	add_calls_to_local_conference(lcs, local_conf, participants);

	terminate_local_conference(lcs, local_conf);

	bctbx_list_free(lcs);
}

static void conference_with_calls_queued_without_ice(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, participants, FALSE, FALSE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_calls_queued_with_ice(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// ICE is enabled
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is disabled
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	// ICE is disabled
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, participants, FALSE, FALSE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_back_to_back_call_accept_without_ice(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, participants, FALSE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_back_to_back_call_accept_with_ice(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// ICE is enabled
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is disabled
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	// ICE is disabled
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, participants, FALSE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_back_to_back_call_invite_accept_without_ice(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, participants, TRUE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void conference_with_back_to_back_call_invite_accept_with_ice(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// ICE is enabled
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is disabled
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	// ICE is disabled
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, participants, TRUE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	bctbx_list_free(lcs);
#else
}

static void back_to_back_conferences(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	// Marie hosts the conference
	conference_with_calls_queued(marie, participants, FALSE, FALSE);
	bctbx_list_free(participants);

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,marie);

	// Laure hosts the conference
	conference_with_calls_queued(laure, new_participants, FALSE, FALSE);
	bctbx_list_free(new_participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void call_accepted_while_another_one_is_updating(bool_t update_from_callee) {
	int call_ring_timeout = 10000000;

	// Local conference
	// ICE is enabled
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_set_inc_timeout(marie->lc, call_ring_timeout);

	bctbx_list_t* participants=NULL;
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, call_ring_timeout);
	participants=bctbx_list_append(participants,pauline);

	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc");
	linphone_core_set_inc_timeout(chloe->lc, call_ring_timeout);
	participants=bctbx_list_append(participants,chloe);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, call_ring_timeout);
	participants=bctbx_list_append(participants,laure);

	LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_core_set_video_activation_policy(chloe->lc, pol);
	linphone_video_activation_policy_unref(pol);

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,marie->lc);

	initiate_calls(participants, marie);

	unsigned int no_callers = (unsigned int)bctbx_list_size(participants);

	unsigned int idx = 0;

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		const LinphoneAddress *caller_uri = m->identity;
		LinphoneCall * marie_call = linphone_core_get_call_by_remote_address(marie->lc, linphone_address_as_string(caller_uri));
		BC_ASSERT_PTR_NOT_NULL(marie_call);

		// Take call - ringing ends
		linphone_call_accept(marie_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, (idx+1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, (idx+1), 5000));


		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, idx, 5000));

		// Send update only on the first call taken
		if (it == participants) {
			LinphoneCall * call_to_update = NULL;
			if (update_from_callee) {
				const LinphoneAddress *caller_uri = m->identity;
				call_to_update = linphone_core_get_call_by_remote_address(marie->lc, linphone_address_as_string(caller_uri));
			} else {
				call_to_update = linphone_core_get_current_call(c);
			}
			BC_ASSERT_PTR_NOT_NULL(call_to_update);
			if (call_to_update) {
				const LinphoneCallParams *old_params = linphone_call_get_params(call_to_update);
				LinphoneCallParams * new_params = linphone_call_params_copy(old_params);
				linphone_call_params_enable_video (new_params, TRUE);
				BC_ASSERT_TRUE(linphone_call_update(call_to_update, new_params));
				linphone_call_params_unref (new_params);
			}
		}

		idx++;
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, no_callers, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, no_callers, 5000));

	LinphoneCoreManager * phead = (LinphoneCoreManager *)bctbx_list_get_data(participants);

	if (update_from_callee) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallUpdatedByRemote, 1, 5000));
	} else {
		BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallUpdating, 1, 5000));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallPausedByRemote, 1, 5000));

	if (update_from_callee) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating, no_callers, 5000));
	} else {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote, no_callers, 5000));
	}

	// Only one call is not paused
	unsigned int no_call_paused = no_callers - 1;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, no_call_paused, 5000));

	LinphoneCall * pcall = NULL;
	unsigned int no_paused_by_remote = 0;
	LinphoneCoreManager * pm = NULL;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCall * call = linphone_core_get_current_call(m->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
printf("%s - call state %s\n", __func__, linphone_call_state_to_string(linphone_call_get_state(call)));
			no_paused_by_remote += (linphone_call_get_state(call) == LinphoneCallPausedByRemote) ? 1 : 0;
			if (linphone_call_get_state(call) == LinphoneCallStreamsRunning) { 
				pcall = call;
				pm = m;
			}
		}
	}

	char* p_remote_address = pcall ? linphone_call_get_remote_address_as_string(pcall) : "Unknown Call";
	BC_ASSERT_EQUAL(strcmp(p_remote_address,linphone_address_as_string(marie->identity)), 0, int, "%d");
	BC_ASSERT_EQUAL(no_paused_by_remote,no_call_paused, int, "%d");

	LinphoneCall * hcall = NULL;
	int no_active_calls_stream_running = 0;
	bctbx_list_t *calls = bctbx_list_copy(linphone_core_get_calls(marie->lc));
	for (bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
		LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(it);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			if (linphone_call_get_state(call) == LinphoneCallStreamsRunning) { 
				no_active_calls_stream_running += 1;
				hcall = call;
			}
printf("%s - call state %s\n", __func__, linphone_call_state_to_string(linphone_call_get_state(call)));
		}
	}
	bctbx_list_free(calls);

	char* h_remote_address = hcall ? linphone_call_get_remote_address_as_string(hcall) : "Unknown remote address";
	char* pm_address = pm ? linphone_address_as_string(pm->identity) : "Unknown participant address";
	BC_ASSERT_EQUAL(strcmp(h_remote_address, pm_address), 0, int, "%d");
	BC_ASSERT_EQUAL(no_active_calls_stream_running,1, int, "%d");

	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&chloe->stat.number_of_LinphoneCallEnd,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,no_callers,10000));
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&chloe->stat.number_of_LinphoneCallReleased,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,no_callers,10000));

	bctbx_list_free(participants);
	bctbx_list_free(lcs);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);

}

static void call_accepted_while_callee_is_updating_another_one(void) {
	call_accepted_while_another_one_is_updating(TRUE);
}

static void call_accepted_while_caller_is_updating_to_same_callee(void) {
	call_accepted_while_another_one_is_updating(FALSE);
}

static void conference_with_ice_negotiations_ending_while_accepting_call(void) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// Local conference
	// ICE is enabled
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	bctbx_list_t* ice_participants=NULL;
	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}
	ice_participants=bctbx_list_append(ice_participants,pauline);

	// ICE is enabled
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc");
	linphone_core_set_inc_timeout(chloe->lc, 10000);
	if (linphone_core_media_encryption_supported(chloe->lc,mode)) {
		linphone_core_set_firewall_policy(chloe->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc,mode);
	}
	ice_participants=bctbx_list_append(ice_participants,chloe);

	bctbx_list_t* non_ice_participants=NULL;
	// ICE is disabled
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, 10000);
	non_ice_participants=bctbx_list_append(non_ice_participants,laure);

	// ICE is disabled
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);
	non_ice_participants=bctbx_list_append(non_ice_participants,michelle);

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,marie->lc);

	initiate_calls(non_ice_participants, marie);
	initiate_calls(ice_participants, marie);

	unsigned int no_ice_callers = (unsigned int)bctbx_list_size(ice_participants);
	unsigned int no_non_ice_callers = (unsigned int)bctbx_list_size(non_ice_participants);
	unsigned int no_calls = no_ice_callers + no_non_ice_callers;

	bctbx_list_t* participants = NULL;
	participants=bctbx_list_append(participants,chloe);
	participants=bctbx_list_append(participants,laure);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,michelle);
	bool_t callee_uses_ice = (linphone_core_get_firewall_policy(marie->lc) == LinphonePolicyUseIce);

	unsigned int no_calls_paused_by_remote = 0;

	LinphoneCoreManager * pm = NULL;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);
		const LinphoneAddress *caller_uri = m->identity;
		LinphoneCall * marie_call = linphone_core_get_call_by_remote_address(marie->lc, linphone_address_as_string(caller_uri));
		BC_ASSERT_PTR_NOT_NULL(marie_call);

		// Take call - ringing ends
		linphone_call_accept(marie_call);

		if (pm && callee_uses_ice && linphone_core_get_firewall_policy(pm->lc) == LinphonePolicyUseIce) {
printf("%s - %s uses ICE\n", __func__, pm->rc_path);
			BC_ASSERT_TRUE(wait_for_until(pm->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, no_calls_paused_by_remote, 5000));
		}


		BC_ASSERT_TRUE(wait_for_until(m->lc,marie->lc, &m->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_until(m->lc,marie->lc, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		if (m && callee_uses_ice && linphone_core_get_firewall_policy(m->lc) == LinphonePolicyUseIce) {
			no_calls_paused_by_remote++;
			BC_ASSERT_TRUE(wait_for_until(m->lc,marie->lc, &m->stat.number_of_LinphoneCallUpdating,1,5000));
		}
		pm = m;
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, no_calls, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, no_calls, 5000));

	for (bctbx_list_t *it = ice_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
		BC_ASSERT_TRUE(check_ice(m,marie,LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(marie,m,LinphoneIceStateHostConnection));
	}
	// Calls whose core enables ICE send a reINVITE hence they go twice to stream running
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, (2*no_ice_callers) +  no_non_ice_callers, 5000));

	// Only one call is not paused
	unsigned int no_call_paused = no_ice_callers + no_non_ice_callers - 1;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, no_call_paused, 5000));

	unsigned int no_paused_by_remote = 0;
	for (bctbx_list_t *it = ice_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		no_paused_by_remote += m->stat.number_of_LinphoneCallPausedByRemote;
	}

	for (bctbx_list_t *it = non_ice_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		no_paused_by_remote += m->stat.number_of_LinphoneCallPausedByRemote;
	}

	BC_ASSERT_EQUAL(no_paused_by_remote,no_call_paused, int, "%d");

	int no_active_calls_stream_running = 0;
	bctbx_list_t *calls = bctbx_list_copy(linphone_core_get_calls(marie->lc));
	for (bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
		LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(it);
		no_active_calls_stream_running += (linphone_call_get_state(call) == LinphoneCallStreamsRunning) ? 1 : 0;
	}
	bctbx_list_free(calls);

	BC_ASSERT_EQUAL(no_active_calls_stream_running,1, int, "%d");

	bctbx_list_free(non_ice_participants);
	bctbx_list_free(ice_participants);
	bctbx_list_free(lcs);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(chloe);

}

static void conference_with_simple_audio_device_change(void) {
	bctbx_list_t *lcs = NULL;

	// Marie is the caller
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc");

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *marie_audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_marie_audio_devices_count = bctbx_list_size(marie_audio_devices);
	bctbx_list_free_with_data(marie_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *marie_factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *marie_sndcard_manager = ms_factory_get_snd_card_manager(marie_factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(marie_sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(marie_sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	marie_audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int marie_audio_devices_count = bctbx_list_size(marie_audio_devices);
	BC_ASSERT_EQUAL(marie_audio_devices_count, (native_marie_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *marie_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(marie_audio_devices);
	BC_ASSERT_PTR_NOT_NULL(marie_dev0);
	linphone_audio_device_ref(marie_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *marie_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(marie_audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(marie_dev1);
	linphone_audio_device_ref(marie_dev1);

	// At the start, choose default device i.e. marie_dev0
	LinphoneAudioDevice *marie_current_dev = marie_dev0;
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);
	linphone_audio_device_ref(marie_current_dev);

	// Unref cards
	bctbx_list_free_with_data(marie_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	// Pauline is offline
	LinphoneCoreManager* pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(laure->lc);
	bctbx_list_t *laure_audio_devices = linphone_core_get_extended_audio_devices(laure->lc);
	int native_laure_audio_devices_count = bctbx_list_size(laure_audio_devices);
	bctbx_list_free_with_data(laure_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *laure_factory = linphone_core_get_ms_factory(laure->lc);
	// Adding 2 devices to Laure' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *laure_sndcard_manager = ms_factory_get_snd_card_manager(laure_factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(laure_sndcard_manager, &dummy2_test_snd_card_desc);
	ms_snd_card_manager_register_desc(laure_sndcard_manager, &dummy_test_snd_card_desc);
	linphone_core_reload_sound_devices(laure->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	laure_audio_devices = linphone_core_get_extended_audio_devices(laure->lc);
	int laure_audio_devices_count = bctbx_list_size(laure_audio_devices);
	BC_ASSERT_EQUAL(laure_audio_devices_count, (native_laure_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *laure_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(laure_audio_devices);
	BC_ASSERT_PTR_NOT_NULL(laure_dev0);
	linphone_audio_device_ref(laure_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *laure_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(laure_audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(laure_dev1);
	linphone_audio_device_ref(laure_dev1);

	// At the start, choose default device i.e. laure_dev0
	LinphoneAudioDevice *laure_current_dev = laure_dev0;
	BC_ASSERT_PTR_NOT_NULL(laure_current_dev);
	linphone_audio_device_ref(laure_current_dev);

	// Unref cards
	bctbx_list_free_with_data(laure_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,laure->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, marie_current_dev);

	linphone_core_set_default_input_audio_device(laure->lc, laure_current_dev);
	linphone_core_set_default_output_audio_device(laure->lc, laure_current_dev);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, marie);

	initiate_calls(participants, laure);

	unsigned int idx = 0;

	LinphoneCoreManager* prev_mgr = NULL;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);

		if (prev_mgr != NULL) {
			LinphoneCall *m_called_by_laure=linphone_core_get_current_call(prev_mgr->lc);
			LinphoneCall *laure_call_m = linphone_core_get_call_by_remote_address(laure->lc, linphone_address_as_string(prev_mgr->identity));
			BC_ASSERT_TRUE(pause_call_1(laure,laure_call_m,prev_mgr,m_called_by_laure));
		}

		const LinphoneAddress *caller_uri = m->identity;
		LinphoneCall * laure_call = linphone_core_get_call_by_remote_address(laure->lc, linphone_address_as_string(caller_uri));
		BC_ASSERT_PTR_NOT_NULL(laure_call);

		// Take call - ringing ends
		linphone_call_accept(laure_call);

		idx++;

		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &m->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &laure->stat.number_of_LinphoneCallConnected, idx, 5000));
		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &laure->stat.number_of_LinphoneCallStreamsRunning, idx, 5000));

		prev_mgr = m;
	}

	add_calls_to_local_conference(lcs, laure, participants);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	marie_current_dev = change_device(TRUE, marie, marie_current_dev, marie_dev0, marie_dev1);

	// wait a bit before Laure changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(laure->lc), laure_current_dev);
	laure_current_dev = change_device(TRUE, laure, laure_current_dev, laure_dev0, laure_dev1);

	terminate_local_conference(lcs, laure);

	bctbx_list_free(lcs);
	bctbx_list_free(participants);

	linphone_audio_device_unref(marie_dev0);
	linphone_audio_device_unref(marie_dev1);
	linphone_audio_device_unref(marie_current_dev);

	linphone_audio_device_unref(laure_dev0);
	linphone_audio_device_unref(laure_dev1);
	linphone_audio_device_unref(laure_current_dev);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

}

test_t multi_call_tests[] = {
	TEST_NO_TAG("Call waiting indication", call_waiting_indication),
	TEST_NO_TAG("Call waiting indication with privacy", call_waiting_indication_with_privacy),
	TEST_NO_TAG("Second call rejected if first one in progress", second_call_rejected_if_first_one_in_progress),
	TEST_NO_TAG("Second call allowed if not using audio", second_call_allowed_if_not_using_audio),
	TEST_NO_TAG("Incoming call accepted when outgoing call in progress", incoming_call_accepted_when_outgoing_call_in_progress),
	TEST_NO_TAG("Incoming call accepted when outgoing call in outgoing ringing", incoming_call_accepted_when_outgoing_call_in_outgoing_ringing),
	TEST_NO_TAG("Incoming call accepted when outgoing call in outgoing ringing early media", incoming_call_accepted_when_outgoing_call_in_outgoing_ringing_early_media),
	TEST_NO_TAG("Call accepted while callee is updating another one", call_accepted_while_callee_is_updating_another_one),
	TEST_NO_TAG("Call accepted while caller is updating to same callee", call_accepted_while_caller_is_updating_to_same_callee),
	TEST_NO_TAG("Simple conference", simple_conference),
	TEST_NO_TAG("Participant exits conference by pausing", participant_exits_conference_by_pausing),
	TEST_NO_TAG("Simple conference with participant with no event log", simple_conference_with_participant_with_no_event_log),
	TEST_NO_TAG("Simple conference established from scratch", simple_conference_from_scratch),
	TEST_NO_TAG("Simple conference established from scratch with video", simple_conference_from_scratch_with_video),
	TEST_NO_TAG("Simple 4 participant conference", simple_4_participants_conference),
	TEST_NO_TAG("Video conference by merging calls", video_conference_by_merging_calls),
	TEST_NO_TAG("Simple conference established from scratch, but attendees do not answer", simple_conference_from_scratch_no_answer),
	TEST_ONE_TAG("Simple conference with ICE", simple_conference_with_ice, "ICE"),
	TEST_ONE_TAG("Simple ZRTP conference with ICE", simple_zrtp_conference_with_ice, "ICE"),
	TEST_NO_TAG("Conference with simple audio device change", conference_with_simple_audio_device_change),
	TEST_NO_TAG("Eject from 3 participants conference", eject_from_3_participants_local_conference),
	TEST_NO_TAG("Eject from 4 participants conference", eject_from_4_participants_conference),
	TEST_NO_TAG("Conference pause and terminate call", conference_hang_up_call_on_hold),
	TEST_NO_TAG("Participants exit conference after pausing", participants_exit_conference_after_pausing),
	TEST_NO_TAG("Add participant after conference started", add_participant_after_conference_started),
	TEST_NO_TAG("Register again during conference", register_again_during_conference),
	TEST_NO_TAG("Try to update call parameter during conference", try_to_update_call_params_during_conference),
	TEST_NO_TAG("Update conference parameter during conference", update_conf_params_during_conference),
	TEST_NO_TAG("Back to back conferences", back_to_back_conferences),
	TEST_NO_TAG("Simple call transfer", simple_call_transfer),
	TEST_NO_TAG("Conference with calls queued without ICE", conference_with_calls_queued_without_ice),
	TEST_NO_TAG("Conference with calls queued with ICE", conference_with_calls_queued_with_ice),
	TEST_NO_TAG("Conference with back to back call accept without ICE", conference_with_back_to_back_call_accept_without_ice),
	TEST_NO_TAG("Conference with back to back call accept with ICE", conference_with_back_to_back_call_accept_with_ice),
	TEST_NO_TAG("Conference with back to back call invite and accept without ICE", conference_with_back_to_back_call_invite_accept_without_ice),
	TEST_NO_TAG("Conference with back to back call invite and accept with ICE", conference_with_back_to_back_call_invite_accept_with_ice),
	TEST_NO_TAG("Conference with ICE negotiations ending while accepting call", conference_with_ice_negotiations_ending_while_accepting_call),
	TEST_NO_TAG("Unattended call transfer", unattended_call_transfer),
	TEST_NO_TAG("Unattended call transfer with error", unattended_call_transfer_with_error),
	TEST_NO_TAG("Call transfer existing outgoing call", call_transfer_existing_call_outgoing_call),
	TEST_NO_TAG("Call transfer existing outgoing call without auto answer of replacing call", call_transfer_existing_call_outgoing_call_no_auto_answer),
	TEST_NO_TAG("Call transfer existing incoming call", call_transfer_existing_call_incoming_call),
	TEST_NO_TAG("Call transfer existing ringing call", call_transfer_existing_ringing_call),
	TEST_NO_TAG("Simple remote conference", simple_remote_conference),
	TEST_NO_TAG("Simple remote conference with shut down focus", simple_remote_conference_shut_down_focus),
	TEST_NO_TAG("Eject from 3 participants in remote conference", eject_from_3_participants_remote_conference),
	TEST_NO_TAG("Do not stop ringing when declining one of two incoming calls", do_not_stop_ringing_when_declining_one_of_two_incoming_calls),
	TEST_NO_TAG("No auto answer on fake call with Replaces header", no_auto_answer_on_fake_call_with_replaces_header)
};

test_suite_t multi_call_test_suite = {"Multi call", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
									  sizeof(multi_call_tests) / sizeof(multi_call_tests[0]), multi_call_tests};
