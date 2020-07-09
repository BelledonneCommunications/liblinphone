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

static void linphone_subscribe_received_internal(LinphoneCore *lc, LinphoneEvent *lev, const char *eventname, const LinphoneContent *content) {
	int *subscription_received = (int*)(((LinphoneCoreManager *)linphone_core_get_user_data(lc))->user_info);
	*subscription_received += 1;
}

static void linphone_notify_received_internal(LinphoneCore *lc, LinphoneEvent *lev, const char *eventname, const LinphoneContent *content){
	LinphoneCoreManager *mgr = get_manager(lc);
	mgr->stat.number_of_NotifyReceived++;
}

static void configure_core_for_conference_callbacks(LinphoneCoreManager *lcm, LinphoneCoreCbs *cbs) {
	_linphone_core_add_callbacks(lcm->lc, cbs, TRUE);
	linphone_core_set_user_data(lcm->lc, lcm);
}

LinphoneCoreManager *create_mgr_for_conference(const char * rc_file) {
	LinphoneCoreManager *mgr = linphone_core_manager_new(rc_file);

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	// Add subscribe and notify received here as when a participant is added, we must wait for its notify is the call goes to StreamRunning
	linphone_core_cbs_set_subscription_state_changed(cbs, linphone_subscription_state_change);
	linphone_core_cbs_set_subscribe_received(cbs, linphone_subscribe_received_internal);
	linphone_core_cbs_set_notify_received(cbs, linphone_notify_received_internal);
	configure_core_for_conference_callbacks(mgr, cbs);
	linphone_core_cbs_unref(cbs);

	linphone_core_set_user_data(mgr->lc, mgr);

	int* subscription_received = (int *)ms_new0(int, 1);
	*subscription_received = 0;
	mgr->user_info = subscription_received;

	return mgr;
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

		if(focus_is_up) {
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning+1,5000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallConnected,initial_marie_stat.number_of_LinphoneTransferCallConnected+1,5000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd+1,5000));
		} else {
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
			goto end;
		}
	}


	if(!is_remote_conf) {
		bctbx_list_t* new_participants=bctbx_list_append(NULL,pauline);
		add_calls_to_local_conference(lcs, marie, new_participants);
		bctbx_list_free(new_participants);

	} else {
		linphone_core_add_to_conference(marie->lc,marie_call_pauline);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallConnected,initial_marie_stat.number_of_LinphoneTransferCallConnected+2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd+2,5000));
	}

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,initial_pauline_stat.number_of_LinphoneCallStreamsRunning+1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,initial_laure_stat.number_of_LinphoneCallStreamsRunning+1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning+2,3000));

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	if(!is_remote_conf) {
		BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));
	} else {
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
		bctbx_list_t *participants = linphone_conference_get_participants(conference);
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
		bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);
	}

	if (pause_and_hangup) {
		linphone_core_pause_call(marie->lc, marie_call_laure);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,2,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausedByRemote,1,10000));
		if (!is_remote_conf){
			/* Since Laure has been removed, the conference will automatically disapear to let
			 * Pauline and Marie communicate directly through a normal Call.
			 */
			remove_participant_from_local_conference(lcs, marie, laure);
			linphone_core_terminate_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,is_remote_conf?2:1,10000));
			BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning+3,10000));
			BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
			linphone_call_terminate(marie_call_pauline);
		} else {
			linphone_core_remove_from_conference(marie->lc, marie_call_laure);
			linphone_core_terminate_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,is_remote_conf?2:1,10000));
		}
	}else{
		terminate_local_conference(lcs, marie);
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

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));

	bctbx_list_free(lcs);
}

static void simple_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	simple_conference_base(marie,pauline,laure, NULL, FALSE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}


static void _simple_conference_from_scratch(bool_t with_video){
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

	/*marie creates the conference*/
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

		/*make sure that the two calls from Marie's standpoint are in conference*/
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 2, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next){
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
		}
		/*wait a bit for the conference audio processing to run, despite we do not test it for the moment*/
		wait_for_list(lcs,NULL,0,5000);
		
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) == with_video);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == with_video);

		terminate_local_conference(lcs, marie);
	}
	linphone_conference_unref(conf);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(marie);
}

static void simple_conference_from_scratch(void){
	_simple_conference_from_scratch(FALSE);
}

static void simple_conference_from_scratch_with_video(void){
	_simple_conference_from_scratch(TRUE);
}

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
	

	/* Marie first estabishes a call with Pauline, with video.*/
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_video(params, TRUE);
	pauline_call = linphone_core_invite_address_with_params(marie->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,1,10000));
	if (BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,10000))){
		linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	}else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	
	/* Then she calls Laure, audio-only. */
	
	laure_call = linphone_core_invite_address(marie->lc, laure->identity);
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,2,10000));
	if (BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,10000))){
		linphone_call_accept(linphone_core_get_current_call(laure->lc));
	}else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,2,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	
	/* This of course puts on hold Pauline. */
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausedByRemote,1,5000));
	
	/* Marie now creates a conference, audio-only.*/
	conf_params = linphone_core_create_conference_params(marie->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	
	/* She adds Pauline and Laure to the conference. */
	linphone_conference_add_participant_with_call(conf, pauline_call);
	linphone_conference_add_participant_with_call(conf, laure_call);
	
	/* Now check that both Pauline and Laure have video. */
	pauline_call = linphone_core_get_current_call(pauline->lc);
	laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	if (pauline_call && laure_call){
		const bctbx_list_t *marie_calls, *it;
		
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,4,5000));

		/*make sure that the two calls from Marie's standpoint are in conference*/
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 2, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next){
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
		}
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) == FALSE);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == FALSE);
		
		ms_message("Adding video to the conference...");
		/* Marie upgrades the conference with video. */
		conf_params = linphone_core_create_conference_params(marie->lc);
		linphone_conference_params_enable_video(conf_params, TRUE);
		linphone_conference_update_params(conf, conf_params);
		linphone_conference_params_unref(conf_params);
		
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,3,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,3,3000));
		
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)) == TRUE);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call)) == TRUE);
		
		/* Make sure video is received for Pauline and Laure. For Marie we can't because of missing APIs.*/
		liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
		liblinphone_tester_set_next_video_frame_decoded_cb(laure_call);

		BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_IframeDecoded ,1, 5000));
		BC_ASSERT_TRUE( wait_for_list(lcs ,&laure->stat.number_of_IframeDecoded, 1, 5000));
		
		/* Change camera, unfortunately there is no way to test its effectiveness for the moment. */
		ms_message("Changing Marie's video device...");
		linphone_core_set_video_device(marie->lc, liblinphone_tester_static_image_id);
		wait_for_list(lcs ,NULL, 0, 2000);

		terminate_local_conference(lcs, marie);
	}
	
	
end:	
	if (conf) linphone_conference_unref(conf);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_from_scratch_no_answer(void){
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

	/*marie creates the conference*/
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
		/* Pauline immediately declines the call.*/
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
		
		/* Laure accepts. */
		linphone_call_accept(laure_call);
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,5000));

		wait_for_list(lcs,NULL,0,1000);
		
		/* the conference no longer exists, as there was finally only one participant.
		 * Terminate the call, simply.*/
		linphone_call_terminate(laure_call);

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,2,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,2,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,1,1000));

	}
	
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_encrypted_conference_with_ice(LinphoneMediaEncryption mode) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());

	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(laure->lc,LinphonePolicyUseIce);

		/**/

		linphone_core_set_media_encryption(marie->lc,mode);
		linphone_core_set_media_encryption(pauline->lc,mode);
		linphone_core_set_media_encryption(laure->lc,mode);

		simple_conference_base(marie,pauline,laure,NULL,FALSE);
	} else {
		ms_warning("No [%s] support available",linphone_media_encryption_to_string(mode));
		BC_PASS("Passed");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void simple_conference_with_ice(void) {
	simple_encrypted_conference_with_ice(LinphoneMediaEncryptionNone);
}

static void simple_zrtp_conference_with_ice(void) {
	simple_encrypted_conference_with_ice(LinphoneMediaEncryptionZRTP);
}

static void conference_hang_up_call_on_hold(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc");
	LinphoneCoreManager* pauline = create_mgr_for_conference("pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference(get_laure_rc());
	simple_conference_base(marie, pauline, laure, NULL, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
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

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);


	linphone_call_transfer(pauline_called_by_marie,laure_identity);
	bctbx_free(laure_identity);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallRefered,1,2000));
	/*marie pausing pauline*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausing,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausedByRemote,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,1,2000));
	/*marie calling laure*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,1,2000));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_transfer_target_call(marie_calling_pauline));

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneTransferCallOutgoingInit,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneTransferCallOutgoingProgress,1,2000));
	linphone_call_accept(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,2000));

	marie_calling_laure=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_calling_laure)) goto end;
	BC_ASSERT_PTR_EQUAL(linphone_call_get_transferer_call(marie_calling_laure),marie_calling_pauline);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneTransferCallConnected,1,2000));

	/*terminate marie to pauline call*/
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

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);

	linphone_call_transfer(pauline_called_by_marie,laure_identity);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,3000));

	/*marie ends the call  */
	linphone_call_terminate(pauline_called_by_marie);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,3000));

	/*Pauline starts the transfer*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingInit,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	linphone_call_accept(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,3000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,3000));

	end_call(laure, pauline);
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

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);

		linphone_call_transfer(pauline_called_by_marie,"unknown_user");
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,2000));

		/*Pauline starts the transfer*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingInit,1,2000));
		/* and immediately get an error*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallError,1,2000));

		/*the error must be reported back to marie*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallError,1,2000));

		/*and pauline should resume the call automatically*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallResuming,1,2000));

		/*and call should be resumed*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,2000));

		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
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

	/*marie call pauline*/
	BC_ASSERT_TRUE((call_ok=call(marie,pauline)));
	if (call_ok){
		marie_call_pauline=linphone_core_get_current_call(marie->lc);
		pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
		/*marie pause pauline*/
		if (!BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie))) {
			goto end;
		}

		if (outgoing_call) {
			/*marie call laure*/
			if (!BC_ASSERT_TRUE(call(marie,laure))) {
				end_call(marie, pauline);
				goto end;
			}
		} else {
			/*laure call pauline*/
			if (!BC_ASSERT_TRUE(call(laure,marie))) {
				end_call(marie,pauline);
				goto end;
			}
		}

		marie_call_laure=linphone_core_get_current_call(marie->lc);
		laure_called_by_marie=linphone_core_get_current_call(laure->lc);

		/*marie pause laure*/
		BC_ASSERT_TRUE(pause_call_1(marie,marie_call_laure,laure,laure_called_by_marie));

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);
		reset_counters(&laure->stat);


		linphone_call_transfer_to_another(marie_call_pauline,marie_call_laure);
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,2000));

		/*pauline pausing marie*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausing,1,4000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPaused,1,4000));
		/*pauline calling laure*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallOutgoingInit,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneTransferCallOutgoingProgress,1,2000));

		/*laure accept call*/
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

		/*terminate marie to pauline/laure call*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,2,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,2,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,2000));

		end_call(pauline, laure);
	}
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
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

	/* marie calls pauline */
	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (call_ok) {
		marie_call_pauline = linphone_core_get_current_call(marie->lc);
		pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
		/* marie pauses pauline */
		if (!BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie))) goto end;

		initial_marie_stats = marie->stat;
		BC_ASSERT_PTR_NOT_NULL((marie_call_laure = linphone_core_invite_address(marie->lc, laure->identity)));
		if (!marie_call_laure) goto end;
		BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, initial_marie_stats.number_of_LinphoneCallOutgoingRinging + 1));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
		laure_call = linphone_core_get_current_call(laure->lc);
		
		linphone_call_transfer_to_another(marie_call_pauline, marie_call_laure);
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallRefered, 1, 2000));

		/* pauline pausing marie */
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing, 1, 4000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused, 1, 4000));
		/* pauline calling laure */
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingInit, 1, 2000));
		
		
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingProgress, 1, 2000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 2, 2000));
		/* laure accepts the new (replacing) call */
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

		/* terminate marie to pauline/laure call */
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 2000));

		end_call(pauline, laure);
	}

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
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
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	if(!is_remote_conf) BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	if(!is_remote_conf) {
		ms_message("Removing pauline from conference.");
		remove_participant_from_local_conference(lcs, marie, pauline);
	} else {
		LinphoneConference *conference = linphone_core_get_conference(marie->lc);
		const LinphoneAddress *uri = linphone_call_get_remote_address(marie_call_pauline);
		LinphoneParticipant * pauline_participant = linphone_conference_find_participant(conference, uri);
		BC_ASSERT_PTR_NOT_NULL(pauline_participant);
		linphone_conference_remove_participant(conference, pauline_participant);

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
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc");
	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());

	eject_from_3_participants_conference(marie, pauline, laure, NULL);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void eject_from_4_participants_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc");
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

	remove_participant_from_local_conference(lcs, marie, pauline);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 3, unsigned int, "%u");
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	linphone_core_terminate_all_calls(laure->lc);
	linphone_core_terminate_all_calls(pauline->lc);
	linphone_core_terminate_all_calls(michelle->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 3, 10000));

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 3, 10000));

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(michelle);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
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
	linphone_config_set_string(marie_config, "misc", "conference_focus_addr", focus_uri);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	simple_conference_base(marie, pauline, laure, (LinphoneCoreManager *)focus, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
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
	linphone_config_set_string(marie_config, "misc", "conference_focus_addr", focus_uri);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	simple_conference_base(marie, pauline, laure, (LinphoneCoreManager *)focus, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
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
	linphone_config_set_string(marie_config, "misc", "conference_focus_addr", focus_uri);

	linphone_proxy_config_edit(laure_proxy_config);
	linphone_proxy_config_set_route(laure_proxy_config, laure_proxy_uri);
	linphone_proxy_config_done(laure_proxy_config);

	eject_from_3_participants_conference(marie, pauline, laure, (LinphoneCoreManager *)focus);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
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
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 1, int, "%d");

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc,pauline->identity,marie_params));
	linphone_call_params_unref(marie_params);

	BC_ASSERT_TRUE(wait_for(marie->lc
							,pauline->lc
							,&pauline->stat.number_of_LinphoneCallIncomingReceived
							,2));
	pauline_called_by_marie=linphone_core_get_current_call(marie->lc);
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

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
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

static void simple_conference_with_audio_device_change_base(bool_t during_setup, bool_t before_all_join, bool_t after_all_join) {
	bctbx_list_t *lcs = NULL;

	// Marie is the caller
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc");

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(dev0);
	linphone_audio_device_ref(dev0);

	// 2nd device in the list
	LinphoneAudioDevice *dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(dev1);
	linphone_audio_device_ref(dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *current_dev = dev0;
	BC_ASSERT_PTR_NOT_NULL(current_dev);
	linphone_audio_device_ref(current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,marie->lc);

	// Pauline is offline
	LinphoneCoreManager* pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_network_reachable(pauline->lc,FALSE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	lcs=bctbx_list_append(lcs,pauline->lc);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_network_reachable(laure->lc,TRUE);
	lcs=bctbx_list_append(lcs,laure->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, current_dev);

	/*Laure creates the conference*/
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params(laure->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(laure->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, marie);

	int noParticipants = (int)bctbx_list_size(participants);

	add_participant_to_local_conference_through_invite(lcs, laure, participants, NULL);

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	linphone_audio_device_unref(current_dev);
	current_dev = linphone_audio_device_ref(dev1);
	BC_ASSERT_PTR_NOT_NULL(current_dev);

	// Set audio device to start with a known situation - do not use default device
	linphone_core_set_input_audio_device(marie->lc, current_dev);
	linphone_core_set_output_audio_device(marie->lc, current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	current_dev = change_device(during_setup, marie, current_dev, dev0, dev1);

	// wait a bit before Marie accepts the call
	wait_for_list(lcs,NULL,0,2000);
	linphone_call_accept(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,3000));

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	current_dev = change_device(before_all_join, marie, current_dev, dev0, dev1);

	// wait a bit before Pauline is reachable
	wait_for_list(lcs,NULL,0,2000);
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallOutgoingProgress,noParticipants,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,10000));
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,noParticipants,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,noParticipants,3000));

	/*make sure that the two calls from Marie's standpoint are in conference*/
	const bctbx_list_t *laure_calls = linphone_core_get_calls(laure->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(laure_calls), noParticipants, int, "%i");
	const bctbx_list_t *it;
	for (it = laure_calls; it != NULL; it = it->next){
		BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
	}

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	current_dev = change_device(after_all_join, marie, current_dev, dev0, dev1);

	// wait a bit before endign the conference
	wait_for_list(lcs,NULL,0,5000);

	terminate_local_conference(lcs, laure);

	linphone_conference_unref(conf);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);

	linphone_audio_device_unref(dev0);
	linphone_audio_device_unref(dev1);
	linphone_audio_device_unref(current_dev);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_audio_device_change_during_setup(void) {
	simple_conference_with_audio_device_change_base(TRUE, FALSE, FALSE);
}

static void simple_conference_with_audio_device_change_before_all_join(void) {
	simple_conference_with_audio_device_change_base(FALSE, TRUE, FALSE);
}

static void simple_conference_with_audio_device_change_after_all_join(void) {
	simple_conference_with_audio_device_change_base(FALSE, FALSE, TRUE);
}

static void simple_conference_with_audio_device_change_pingpong(void) {
	simple_conference_with_audio_device_change_base(TRUE, TRUE, TRUE);
}

static void simple_conference_with_audio_device_change_during_pause_base(bool_t callee, bool_t caller) {
	bctbx_list_t *lcs = NULL;

	// Marie is the caller
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc");

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *marie_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(marie_dev0);
	linphone_audio_device_ref(marie_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *marie_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(marie_dev1);
	linphone_audio_device_ref(marie_dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *marie_current_dev = marie_dev0;
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);
	linphone_audio_device_ref(marie_current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,marie->lc);

	// Pauline is onlineoffline
	LinphoneCoreManager* pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(pauline->lc);
	audio_devices = linphone_core_get_extended_audio_devices(pauline->lc);
	native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	factory = linphone_core_get_ms_factory(pauline->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(pauline->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(pauline->lc);
	audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *pauline_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(pauline_dev0);
	linphone_audio_device_ref(pauline_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *pauline_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(pauline_dev1);
	linphone_audio_device_ref(pauline_dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *pauline_current_dev = pauline_dev0;
	BC_ASSERT_PTR_NOT_NULL(pauline_current_dev);
	linphone_audio_device_ref(pauline_current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,pauline->lc);

	LinphoneCoreManager* laure = create_mgr_for_conference( get_laure_rc());
	linphone_core_set_network_reachable(laure->lc,TRUE);
	lcs=bctbx_list_append(lcs,laure->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_input_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_default_output_audio_device(pauline->lc, pauline_current_dev);

	/*Laure creates the conference*/
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params(pauline->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(pauline->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, marie);

	int noParticipants = (int)bctbx_list_size(participants);

	add_participant_to_local_conference_through_invite(lcs, pauline, participants, NULL);

	linphone_audio_device_unref(marie_current_dev);
	marie_current_dev = linphone_audio_device_ref(marie_dev1);
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);

	linphone_audio_device_unref(pauline_current_dev);
	pauline_current_dev = linphone_audio_device_ref(pauline_dev1);
	BC_ASSERT_PTR_NOT_NULL(pauline_current_dev);

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	// Set audio device to start with a known situation - do not use default device
	linphone_core_set_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_output_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_input_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_output_audio_device(pauline->lc, pauline_current_dev);

	// wait a bit before Marie accepts the call
	wait_for_list(lcs,NULL,0,2000);
	linphone_call_accept(marie_call);


	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));

	// wait a bit before Pauline is reachable
	wait_for_list(lcs,NULL,0,2000);
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress,noParticipants,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,10000));
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	linphone_call_accept(laure_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,noParticipants,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,noParticipants,3000));

	LinphoneCall *pauline_call = NULL;

	/*make sure that the two calls from Marie's standpoint are in conference*/
	const bctbx_list_t *pauline_calls = linphone_core_get_calls(pauline->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_calls), noParticipants, int, "%i");
	const bctbx_list_t *it;
	for (it = pauline_calls; it != NULL; it = it->next){
		pauline_call = (LinphoneCall*)it->data;
		BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
	}

	BC_ASSERT_PTR_NOT_NULL(pauline_call);

	// wait a bit before changing device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);

	// Callee pauses call and changes device
	pauline_current_dev = pause_call_changing_device(caller, lcs, pauline_call, pauline, marie, pauline, pauline_current_dev, pauline_dev0, pauline_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);

	// Callee pauses call and caller changes device
	marie_current_dev = pause_call_changing_device(caller, lcs, pauline_call, pauline, marie, marie, marie_current_dev, marie_dev0, marie_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);

	// Caller pauses call and callee changes device
	pauline_current_dev = pause_call_changing_device(callee, lcs, marie_call, marie, pauline, pauline, pauline_current_dev, pauline_dev0, pauline_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);

	// Caller pauses call and changes device
	marie_current_dev = pause_call_changing_device(callee, lcs, marie_call, marie, pauline, marie, marie_current_dev, marie_dev0, marie_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);

	// wait a bit before ending the conference
	wait_for_list(lcs,NULL,0,5000);

	terminate_local_conference(lcs, pauline);
	linphone_conference_unref(conf);

	// After call, unref the sound card
	linphone_audio_device_unref(marie_dev0);
	linphone_audio_device_unref(marie_dev1);
	linphone_audio_device_unref(marie_current_dev);
	linphone_core_manager_destroy(marie);
	linphone_audio_device_unref(pauline_dev0);
	linphone_audio_device_unref(pauline_dev1);
	linphone_audio_device_unref(pauline_current_dev);
	linphone_core_manager_destroy(pauline);

	linphone_core_manager_destroy(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_audio_device_change_during_pause_callee(void) {
	simple_conference_with_audio_device_change_during_pause_base(TRUE, FALSE);
}

static void simple_conference_with_audio_device_change_during_pause_caller(void) {
	simple_conference_with_audio_device_change_during_pause_base(FALSE, TRUE);
}

static void simple_conference_with_audio_device_change_during_pause_caller_callee(void) {
	simple_conference_with_audio_device_change_during_pause_base(TRUE, TRUE);
}

test_t multi_call_tests[] = {
	TEST_NO_TAG("Call waiting indication", call_waiting_indication),
	TEST_NO_TAG("Call waiting indication with privacy", call_waiting_indication_with_privacy),
	TEST_NO_TAG("Second call rejected if first one in progress", second_call_rejected_if_first_one_in_progress),
	TEST_NO_TAG("Second call allowed if not using audio", second_call_allowed_if_not_using_audio),
	TEST_NO_TAG("Incoming call accepted when outgoing call in progress", incoming_call_accepted_when_outgoing_call_in_progress),
	TEST_NO_TAG("Incoming call accepted when outgoing call in outgoing ringing", incoming_call_accepted_when_outgoing_call_in_outgoing_ringing),
	TEST_NO_TAG("Incoming call accepted when outgoing call in outgoing ringing early media", incoming_call_accepted_when_outgoing_call_in_outgoing_ringing_early_media),
	TEST_NO_TAG("Simple conference", simple_conference),
	TEST_NO_TAG("Simple conference established from scratch", simple_conference_from_scratch),
	TEST_NO_TAG("Simple conference established from scratch with video", simple_conference_from_scratch_with_video),
	TEST_NO_TAG("Video conference by merging calls", video_conference_by_merging_calls),
	TEST_NO_TAG("Simple conference established from scratch, but attendees do not answer", simple_conference_from_scratch_no_answer),
	TEST_ONE_TAG("Simple conference with ICE", simple_conference_with_ice, "ICE"),
	TEST_ONE_TAG("Simple ZRTP conference with ICE", simple_zrtp_conference_with_ice, "ICE"),
	TEST_NO_TAG("Simple conference with audio device change during setup", simple_conference_with_audio_device_change_during_setup),
	TEST_NO_TAG("Simple conference with audio device change before all join", simple_conference_with_audio_device_change_before_all_join),
	TEST_NO_TAG("Simple conference with audio device change after all join", simple_conference_with_audio_device_change_after_all_join),
	TEST_NO_TAG("Simple conference with audio device change ping pong", simple_conference_with_audio_device_change_pingpong),
	TEST_NO_TAG("Simple conference with audio device change during pause callee", simple_conference_with_audio_device_change_during_pause_callee),
	TEST_NO_TAG("Simple conference with audio device change during pause caller", simple_conference_with_audio_device_change_during_pause_caller),
	TEST_NO_TAG("Simple conference with audio device change during pause both parties", simple_conference_with_audio_device_change_during_pause_caller_callee),
	TEST_NO_TAG("Eject from 3 participants conference", eject_from_3_participants_local_conference),
	TEST_NO_TAG("Eject from 4 participants conference", eject_from_4_participants_conference),
	TEST_NO_TAG("Conference pause and terminate call", conference_hang_up_call_on_hold),
	TEST_NO_TAG("Simple call transfer", simple_call_transfer),
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
