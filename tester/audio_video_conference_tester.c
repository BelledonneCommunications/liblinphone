/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

void destroy_mgr_in_conference(LinphoneCoreManager *mgr) {
	if (mgr->user_info) {
		ms_free(mgr->user_info);
	}

	linphone_core_manager_destroy(mgr);
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

	bctbx_list_t* new_participants=bctbx_list_append(NULL,laure);
	// TODO: Find a way to extract participants managers from conference
	bctbx_list_t* lcs2=bctbx_list_append(NULL,marie->lc);
	lcs2=bctbx_list_append(lcs2,laure->lc);

	if(!is_remote_conf) {
		add_calls_to_local_conference(lcs2, marie, NULL, new_participants);
	} else {

		if(focus_is_up) {
			lcs2=bctbx_list_append(lcs2,focus->lc);
			add_calls_to_remote_conference(lcs2, focus, marie, new_participants);

			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

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
		add_calls_to_local_conference(lcs, marie, NULL, new_participants);
	} else {
		add_calls_to_remote_conference(lcs, focus, marie, new_participants);
	}
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,initial_pauline_stat.number_of_LinphoneCallStreamsRunning+1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,initial_laure_stat.number_of_LinphoneCallStreamsRunning+1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning+2,3000));

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));

	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

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
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
		bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);
	}

	if (pause_and_hangup) {
		if (!is_remote_conf){
			/* Since Laure has been removed, the conference will automatically disapear to let
			 * Pauline and Marie communicate directly through a normal Call.
			 */
			stats marie_stat=marie->stat;
			stats laure_stat=laure->stat;
			linphone_core_pause_call(marie->lc, marie_call_laure);
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausing, marie_stat.number_of_LinphoneCallPausing+1,10000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused, marie_stat.number_of_LinphoneCallPaused+1,10000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausedByRemote, laure_stat.number_of_LinphoneCallPausedByRemote+1,10000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneConferenceStateTerminationPending,laure_stat.number_of_LinphoneConferenceStateTerminationPending,5000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneConferenceStateTerminated,laure_stat.number_of_LinphoneConferenceStateTerminated,5000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneConferenceStateDeleted,laure_stat.number_of_LinphoneConferenceStateDeleted,5000));

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
		bctbx_list_t* participants=bctbx_list_append(NULL,laure);
		participants=bctbx_list_append(participants,pauline);
		if (is_remote_conf){
			terminate_conference(participants, marie, NULL, focus);
		} else {
			terminate_conference(participants, marie, NULL, NULL);
		}
		bctbx_list_free(participants);
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

	// If focus is not registered, the conference is not attached to any core, hence it cannot be destroyed
	if (!(is_remote_conf && !focus_is_up)) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
	}

	bctbx_list_free(lcs);
}

static void simple_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	simple_conference_base(marie,pauline,laure, NULL, FALSE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void simple_conference_through_inviting_participants(bool_t check_for_proxies) {
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", check_for_proxies);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", check_for_proxies);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", check_for_proxies);
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

	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", check_for_proxies);
	//marie creates the conference
	conf_params = linphone_core_create_conference_params(marie->lc);
	conf = linphone_core_create_conference_with_params(marie->lc, conf_params);
	linphone_conference_params_unref(conf_params);
	lcs = bctbx_list_append(lcs, marie->lc);

	if (check_for_proxies == FALSE) {
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneCoreManager * m = get_manager(c);
			int proxy_count=(int)bctbx_list_size(linphone_core_get_proxy_config_list(m->lc));

			if (proxy_count > 0){
	#define REGISTER_TIMEOUT 20 /* seconds per proxy */
				int success = wait_for_until(m->lc,NULL,&m->stat.number_of_LinphoneRegistrationOk,
											proxy_count,(REGISTER_TIMEOUT * 1000 * proxy_count));
				if( !success ){
					ms_error("Did not register after %d seconds for %d proxies", REGISTER_TIMEOUT, proxy_count);
				}
			}
		}
	}

	if (!BC_ASSERT_TRUE(call(marie,michelle))) goto end;

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

	if (pauline_call && laure_call && michelle_call){
		const bctbx_list_t *marie_calls, *it;
		linphone_call_accept(pauline_call);
		linphone_call_accept(laure_call);

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,3,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,3,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,3000));

		// Conference may have already been created of call was paused before hence initial stats can lead to false errors
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

		// Check subscriptions
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionIncomingReceived,3,5000));

		int* subscription_count = ((int *)(marie->user_info));
		BC_ASSERT_TRUE(wait_for_list(lcs,subscription_count,3,5000));

		//make sure that the two calls from Marie's standpoint are in conference
		marie_calls = linphone_core_get_calls(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_calls), 3, int, "%i");
		for (it = marie_calls; it != NULL; it = it->next){
			BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
		}
		//wait a bit for the conference audio processing to run, despite we do not test it for the moment
		wait_for_list(lcs,NULL,0,5000);

		initial_marie_stat = marie->stat;
		initial_pauline_stat = pauline->stat;
		initial_laure_stat = laure->stat;
		initial_michelle_stat = michelle->stat;

		// Remove Pauline
		conf_call = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		linphone_core_remove_from_conference(marie->lc, conf_call);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, initial_marie_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, initial_marie_stat.number_of_LinphoneCallReleased + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, initial_pauline_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, initial_pauline_stat.number_of_LinphoneCallReleased + 1, 10000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateTerminated,(initial_pauline_stat.number_of_LinphoneConferenceStateTerminated + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneConferenceStateDeleted,(initial_pauline_stat.number_of_LinphoneConferenceStateDeleted + 1),5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated,initial_pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));

		BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conf), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_conference_is_in(conf), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_NotifyReceived,(initial_laure_stat.number_of_NotifyReceived + 2),3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_NotifyReceived,(initial_michelle_stat.number_of_NotifyReceived + 2),3000));

		initial_marie_stat = marie->stat;
		initial_laure_stat = laure->stat;
		initial_michelle_stat = michelle->stat;

		// Remove Michelle
		conf_call = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		linphone_core_remove_from_conference(marie->lc, conf_call);

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausing,(initial_marie_stat.number_of_LinphoneCallPausing + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPaused,(initial_marie_stat.number_of_LinphoneCallPaused + 1),5000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallPausedByRemote,(initial_michelle_stat.number_of_LinphoneCallPausedByRemote + 1),5000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_michelle_stat.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneConferenceStateTerminated,(initial_michelle_stat.number_of_LinphoneConferenceStateTerminated + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneConferenceStateDeleted,(initial_michelle_stat.number_of_LinphoneConferenceStateDeleted + 1),5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &michelle->stat.number_of_LinphoneSubscriptionTerminated,initial_michelle_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated,initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_NotifyReceived,(initial_laure_stat.number_of_NotifyReceived + 2),10000));

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

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, initial_marie_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, initial_marie_stat.number_of_LinphoneCallReleased + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, initial_laure_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, initial_laure_stat.number_of_LinphoneCallReleased + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,initial_marie_stat.number_of_LinphoneSubscriptionTerminated + 1,10000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneConferenceStateTerminated,(initial_laure_stat.number_of_LinphoneConferenceStateTerminated + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneConferenceStateDeleted,(initial_laure_stat.number_of_LinphoneConferenceStateDeleted + 1),5000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneConferenceStateTerminated,(initial_marie_stat.number_of_LinphoneConferenceStateTerminated + 1),10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneConferenceStateDeleted,(initial_marie_stat.number_of_LinphoneConferenceStateDeleted + 1),10000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneSubscriptionTerminated,initial_laure_stat.number_of_LinphoneSubscriptionTerminated + 1,3000));

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

static void simple_conference_established_before_proxy_config_creation(void) {
	simple_conference_through_inviting_participants(FALSE);
}

static void _simple_conference_from_scratch(bool_t with_video){
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
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

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,3000));

		// Conference may have already been created of call was paused before hence initial stats can lead to false errors
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

		// Check subscriptions
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionIncomingReceived,2,5000));

		int* subscription_count = ((int *)(marie->user_info));
		BC_ASSERT_TRUE(wait_for_list(lcs,subscription_count,2,5000));

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

		terminate_conference(participants, marie, NULL, NULL);
	}
	linphone_conference_unref(conf);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
}

static void simple_conference_from_scratch(void){
	_simple_conference_from_scratch(FALSE);
}

static void simple_conference_from_scratch_with_video(void){
	_simple_conference_from_scratch(TRUE);
}

static void video_conference_by_merging_calls(void){
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneConference *conf = NULL;
	LinphoneCallParams *params;
	LinphoneConferenceParams *conf_params;
	LinphoneCall *pauline_call, *laure_call;
	bctbx_list_t* new_participants = NULL;
	char *play_file_pauline = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);
	
	linphone_core_set_play_file(pauline->lc, play_file_pauline);
	bc_free(play_file_pauline);

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
		linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	}else goto end;
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,5000));
	
	// Then she calls Laure, audio-only. */
	
	laure_call = linphone_core_invite_address(marie->lc, laure->identity);
	
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingProgress,2,10000));
	if (BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,10000))){
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
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

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

		terminate_conference(new_participants, marie, NULL, NULL);

	}
	
	
end:	
	if (conf) linphone_conference_unref(conf);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	bctbx_list_free(new_participants);

	bctbx_list_free(lcs);
}

static void simple_conference_from_scratch_no_answer(void){
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
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

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminationPending, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateTerminated, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateDeleted, marie->stat.number_of_LinphoneConferenceStateCreated, 5000));

	}
	
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_encrypted_conference_with_ice(LinphoneMediaEncryption mode) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);

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
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	simple_conference_base(marie, pauline, laure, NULL, TRUE);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
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
		add_calls_to_local_conference(lcs2, marie, NULL, new_participants);
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
		add_calls_to_local_conference(lcs, marie, NULL, new_participants);
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

	ms_message("Removing pauline from conference.");
	if(!is_remote_conf) {
		remove_participant_from_local_conference(lcs, marie, pauline);
	} else {
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
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,2,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,10000));

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased,initial_laure_stat.number_of_LinphoneCallReleased+1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased,initial_marie_stat.number_of_LinphoneCallReleased+2,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased,initial_pauline_stat.number_of_LinphoneCallReleased+1,3000));
	} else {

		linphone_core_terminate_conference(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd+2,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd+3,3000));
	}
end:
	bctbx_list_free(lcs);
}

static void eject_from_3_participants_local_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);

	eject_from_3_participants_conference(marie, pauline, laure, NULL);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
}

static void eject_from_4_participants_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

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
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	/* Wait that the three participants are joined to the local conference, by checking the StreamsRunning states*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	remove_participant_from_local_conference(lcs, marie, pauline);
	new_participants=bctbx_list_remove(new_participants,pauline);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 3, unsigned int, "%u");
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	wait_for_list(lcs ,NULL, 0, 2000);

	terminate_conference(new_participants, marie, NULL, NULL);
	bctbx_list_free(new_participants);

	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void participants_exit_conference_after_pausing(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

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
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);
	bctbx_list_free(new_participants);

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	unsigned int marie_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),(marie_call_no + 1), int, "%d");

	bctbx_list_t *participants = linphone_conference_get_participant_list(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), marie_call_no, unsigned int, "%u");
	bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(marie->lc));

	stats marie_stats = marie->stat;
	stats pauline_stats = pauline->stat;
	stats laure_stats = laure->stat;
	stats michelle_stats = michelle->stat;

	int conf_size = linphone_core_get_conference_size(marie->lc);

	LinphoneCall * laure_call_marie = linphone_core_get_current_call(laure->lc);
	linphone_core_pause_call(laure->lc, laure_call_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausing, laure_stats.number_of_LinphoneCallPausing + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPaused, laure_stats.number_of_LinphoneCallPaused + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote, marie_stats.number_of_LinphoneCallPausedByRemote + 1, 10000));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),conf_size, int, "%d");

	LinphoneCall * pauline_call_marie = linphone_core_get_current_call(pauline->lc);
	linphone_core_pause_call(laure->lc, pauline_call_marie);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPausing, pauline_stats.number_of_LinphoneCallPausing + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallPaused, pauline_stats.number_of_LinphoneCallPaused + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote, marie_stats.number_of_LinphoneCallPausedByRemote + 2, 10000));
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),conf_size, int, "%d");

	// Remove Laure, whose call is paused, from the conference.
	remove_participant_from_local_conference(lcs, marie, laure);

	bctbx_list_t* lcs2=bctbx_list_copy(lcs);
	lcs2=bctbx_list_remove(lcs2,laure->lc);

	// Remove Michelle from conference.
	// This should also take out Pauline from the conference whose call is paused.
	remove_participant_from_local_conference(lcs2, marie, michelle);
	bctbx_list_free(lcs2);

	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd, marie_stats.number_of_LinphoneCallEnd+marie_call_no,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd, laure_stats.number_of_LinphoneCallEnd+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd, pauline_stats.number_of_LinphoneCallEnd+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallEnd, michelle_stats.number_of_LinphoneCallEnd+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased, marie_stats.number_of_LinphoneCallReleased+marie_call_no,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased, laure_stats.number_of_LinphoneCallReleased+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased, pauline_stats.number_of_LinphoneCallReleased+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallReleased, michelle_stats.number_of_LinphoneCallReleased+1,10000));

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

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

	bctbx_list_t* participants=NULL;
	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);
	participants=bctbx_list_copy(new_participants);
	bctbx_list_free(new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	lcs=bctbx_list_append(lcs,laure->lc);

	BC_ASSERT_TRUE(call(marie,laure));

	marie_call_laure=linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;

	bctbx_list_t* additional_participants=NULL;
	additional_participants=bctbx_list_append(additional_participants,laure);
	participants=bctbx_list_append(participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, additional_participants);
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

	terminate_conference(participants, marie, NULL, NULL);
	bctbx_list_free(participants);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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

static void focus_takes_call_after_conference_started_and_participants_leave(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_called_by_laure;
	LinphoneCall* laure_call_marie;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);

	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(conference)) goto end;
	const LinphoneConferenceParams * conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(conf_params));

	lcs=bctbx_list_append(lcs,laure->lc);

	BC_ASSERT_TRUE(call(laure,marie));

	marie_called_by_laure=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_called_by_laure)) goto end;
	laure_call_marie=linphone_core_get_current_call(laure->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_call_marie)) goto end;

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	// Local participant is expected to have left as it joined another call
	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));

	bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
	bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

	conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_FALSE(linphone_conference_params_local_participant_enabled(conf_params));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	new_participants = terminate_participant_call(new_participants, marie, michelle);
	bctbx_list_free(new_participants);

	unsigned int marie_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc));

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd, marie_stat.number_of_LinphoneCallEnd+marie_call_no,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd, laure_stat.number_of_LinphoneCallEnd+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd, pauline_stat.number_of_LinphoneCallEnd+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased, marie_stat.number_of_LinphoneCallReleased+marie_call_no,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased, laure_stat.number_of_LinphoneCallReleased+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallReleased, pauline_stat.number_of_LinphoneCallReleased+1,10000));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);

}

static void participant_takes_call_after_conference_started_and_conference_ends(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	LinphoneCall* chloe_call_laure;
	LinphoneCall* laure_called_by_chloe;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	michelle_called_by_marie=linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_michelle,michelle,michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,laure)))
		goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;

	LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe,laure));

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausing,(laure_initial_stats.number_of_LinphoneCallPausing + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote,(marie_initial_stats.number_of_LinphoneCallPausedByRemote + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPaused,(laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int, "%d");
	
	chloe_call_laure=linphone_core_get_current_call(chloe->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;

	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	wait_for_list(lcs ,NULL, 0, 2000);

	terminate_conference(new_participants, marie, NULL, NULL);
	bctbx_list_free(new_participants);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

	stats initial_laure_stat = laure->stat;
	stats initial_chloe_stat = chloe->stat;
	linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
	BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&chloe->stat.number_of_LinphoneCallEnd,initial_chloe_stat.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&laure->stat.number_of_LinphoneCallReleased,initial_laure_stat.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&chloe->stat.number_of_LinphoneCallReleased,initial_chloe_stat.number_of_LinphoneCallReleased + 1));


end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);

}

static void participant_takes_call_after_conference_started_and_rejoins_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	LinphoneCall* chloe_call_laure;
	LinphoneCall* laure_called_by_chloe;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	marie_call_pauline = linphone_core_get_call_by_remote_address2(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	michelle_called_by_marie=linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_michelle,michelle,michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,laure)))
		goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;

	LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe,laure));

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausing,(laure_initial_stats.number_of_LinphoneCallPausing + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote,(marie_initial_stats.number_of_LinphoneCallPausedByRemote + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPaused,(laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int, "%d");

	chloe_call_laure=linphone_core_get_current_call(chloe->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;

	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	wait_for_list(lcs ,NULL, 0, 2000);

	LinphoneCall * laure_calls_marie = linphone_core_get_call_by_remote_address2(laure->lc, marie->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_calls_marie)) goto end;
	// Remote  conference
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));

	// Call between Chloe and Laure ends and Laure rejoins conference
	if (laure_called_by_chloe) {
		stats initial_marie_stat = marie->stat;
		stats initial_laure_stat = laure->stat;
		stats initial_chloe_stat = chloe->stat;
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
		linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&chloe->stat.number_of_LinphoneCallEnd,initial_chloe_stat.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&laure->stat.number_of_LinphoneCallReleased,initial_laure_stat.number_of_LinphoneCallReleased + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&chloe->stat.number_of_LinphoneCallReleased,initial_chloe_stat.number_of_LinphoneCallReleased + 1));

		// Remote  conference
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");

		linphone_call_resume(laure_calls_marie);

		BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&laure->stat.number_of_LinphoneCallResuming,initial_laure_stat.number_of_LinphoneCallResuming + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&laure->stat.number_of_LinphoneCallStreamsRunning,initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	new_participants = terminate_participant_call(new_participants, marie, laure);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	terminate_conference(new_participants, marie, NULL, NULL);
	bctbx_list_free(new_participants);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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

}

static void participant_takes_call_after_conference_started_and_rejoins_conference_after_conference_ended(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);

	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	LinphoneCall* chloe_call_laure;
	LinphoneCall* laure_called_by_chloe;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	michelle_called_by_marie=linphone_core_get_current_call(michelle->lc);
	marie_call_michelle = linphone_core_get_call_by_remote_address2(marie->lc, michelle->identity);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_michelle,michelle,michelle_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,laure)))
		goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	stats marie_initial_stats = marie->stat;
	stats laure_initial_stats = laure->stat;

	LinphoneCoreToneManagerStats *marie_tone_mgr_stats = linphone_core_get_tone_manager_stats(marie->lc);
	int initial_named_tone = marie_tone_mgr_stats->number_of_startNamedTone;

	BC_ASSERT_TRUE(call(chloe,laure));

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPausing,(laure_initial_stats.number_of_LinphoneCallPausing + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallPausedByRemote,(marie_initial_stats.number_of_LinphoneCallPausedByRemote + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallPaused,(laure_initial_stats.number_of_LinphoneCallPaused + 1), 5000));

	// As Marie is in a conference, the named tone should not be played even though the call is paused
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startNamedTone, initial_named_tone, int, "%d");

	chloe_call_laure=linphone_core_get_current_call(chloe->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe_call_laure)) goto end;
	
	laure_called_by_chloe = linphone_core_get_call_by_remote_address2(laure->lc, chloe->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_chloe)) goto end;

	BC_ASSERT_FALSE(linphone_call_is_in_conference(chloe_call_laure));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_called_by_chloe));

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	// Taking michelle out of the conference causes the conference to be destroyed
	new_participants = terminate_participant_call(new_participants, marie, michelle);
	bctbx_list_free(new_participants);

	wait_for_list(lcs ,NULL, 0, 2000);

	LinphoneCall * laure_calls_marie = linphone_core_get_call_by_remote_address2(laure->lc, marie->identity);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_calls_marie)) goto end;
	// Remote  conference
	BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_calls_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));

	// Call between Chloe and Laure ends and Laure resumes call with Marie. Conferece has terminated because Michelle left in the meantime
	if (laure_called_by_chloe) {
		stats initial_marie_stat = marie->stat;
		stats initial_laure_stat = laure->stat;
		stats initial_chloe_stat = chloe->stat;
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 2, unsigned int, "%u");
		linphone_core_terminate_call(laure->lc, laure_called_by_chloe);
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&chloe->stat.number_of_LinphoneCallEnd,initial_chloe_stat.number_of_LinphoneCallEnd + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&laure->stat.number_of_LinphoneCallReleased,initial_laure_stat.number_of_LinphoneCallReleased + 1));
		BC_ASSERT_TRUE(wait_for(chloe->lc,laure->lc,&chloe->stat.number_of_LinphoneCallReleased,initial_chloe_stat.number_of_LinphoneCallReleased + 1));

		// Remote  conference
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 1, unsigned int, "%u");

		linphone_call_resume(laure_calls_marie);

		// Remote  conference
		BC_ASSERT_PTR_NULL(linphone_call_get_conference(laure_calls_marie));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(laure_calls_marie));

		BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&laure->stat.number_of_LinphoneCallResuming,initial_laure_stat.number_of_LinphoneCallResuming + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&laure->stat.number_of_LinphoneCallStreamsRunning,initial_laure_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1));
	}

	stats initial_marie_stat = marie->stat;
	stats initial_laure_stat = laure->stat;
	linphone_core_terminate_call(laure->lc, laure_calls_marie);
	BC_ASSERT_TRUE(wait_for(marie->lc,laure->lc,&laure->stat.number_of_LinphoneCallEnd,initial_laure_stat.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc,laure->lc,&marie->stat.number_of_LinphoneCallEnd,initial_marie_stat.number_of_LinphoneCallEnd + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc,laure->lc,&laure->stat.number_of_LinphoneCallReleased,initial_laure_stat.number_of_LinphoneCallReleased + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc,laure->lc,&marie->stat.number_of_LinphoneCallReleased,initial_marie_stat.number_of_LinphoneCallReleased + 1));

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(laure->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);

}

static void set_video_in_conference(bctbx_list_t* lcs, LinphoneCoreManager* conf, bctbx_list_t* participants, bool_t enable_video) {
	LinphoneConference *conference = linphone_core_get_conference(conf->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	stats* initial_stats = NULL;
	bool_t* initial_video_call = NULL;
	int idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		initial_stats = (stats*)realloc(initial_stats, (idx+1) * sizeof(stats));
		initial_video_call = (bool_t*)realloc(initial_video_call, (idx+1) * sizeof(bool_t));
		// Append element
		initial_stats[idx] = m->stat;
		LinphoneCall * call = linphone_core_get_call_by_remote_address2(conf->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		const LinphoneCallParams * params = linphone_call_get_params(call);
		initial_video_call[idx] = linphone_call_params_video_enabled(params);

		idx++;
	}

	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	stats initial_conf_stat = conf->stat;
	if (conference) {
		const LinphoneConferenceParams * old_params = linphone_conference_get_current_params(conference);
		LinphoneConferenceParams * new_params = linphone_conference_params_clone(old_params);
		linphone_conference_params_enable_video (new_params, enable_video);

		BC_ASSERT_TRUE(linphone_conference_update_params(conference, new_params));
		linphone_conference_params_unref (new_params);
	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(conf->lc));
	// number of participants is incremented by 1 because Marie is in
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(conf->lc),no_participants+1, int, "%d");

	idx = 0;
	int update_cnt = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;

		LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(c, conf->identity);
		BC_ASSERT_PTR_NOT_NULL(participant_call);

		if (initial_video_call[idx] != enable_video) {

			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdatedByRemote, initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1, 5000));

			int defer_update = !!linphone_config_get_int(linphone_core_get_config(c), "sip", "defer_update_default", FALSE);
			if (defer_update == TRUE) {
				LinphoneCallParams * m_params = linphone_core_create_call_params(m->lc, participant_call);
				linphone_call_params_enable_video(m_params, enable_video);
				linphone_call_accept_update(participant_call, m_params);
				linphone_call_params_unref(m_params);
			}

			BC_ASSERT_TRUE(wait_for_list(lcs, &conf->stat.number_of_LinphoneCallUpdating, initial_conf_stat.number_of_LinphoneCallUpdating + update_cnt, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &conf->stat.number_of_LinphoneCallStreamsRunning, initial_conf_stat.number_of_LinphoneCallStreamsRunning + update_cnt, 5000));

			update_cnt++;

		}

		// Wait for first frame if video is enabled
		if (enable_video) {
			// Make sure video is received for participants. For conference we can't because of missing APIs.*/
			liblinphone_tester_set_next_video_frame_decoded_cb(participant_call);
			BC_ASSERT_TRUE( wait_for_list(lcs, &m->stat.number_of_IframeDecoded, initial_stats[idx].number_of_IframeDecoded + 1, 5000));
		}

		// Remote  conference
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));

		const LinphoneCallParams * participant_call_params = linphone_call_get_current_params(participant_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(participant_call_params) == enable_video);

		LinphoneCall * conf_call = linphone_core_get_call_by_remote_address2(conf->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		const LinphoneCallParams * conf_call_params = linphone_call_get_current_params(conf_call);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(conf_call_params) == enable_video);

		LinphoneConference *pconference = linphone_call_get_conference(participant_call);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			bctbx_list_t *pconf_participants = linphone_conference_get_participant_list(pconference);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(pconf_participants), no_participants, unsigned int, "%u");
			bctbx_list_free_with_data(pconf_participants, (void(*)(void *))linphone_participant_unref);
		}


		idx++;

	}
	ms_free(initial_stats);
	ms_free(initial_video_call);

	const LinphoneConferenceParams * params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == enable_video);
}

static void set_video_in_call(LinphoneCoreManager* m1, LinphoneCoreManager* m2, bool_t enable_video, bool_t exp_video_enabled) {

	LinphoneConference *m1_conference = linphone_core_get_conference(m1->lc);
	bool_t expected_m1_video_capability = FALSE;
	if (m1_conference) {
		const LinphoneConferenceParams * params = linphone_conference_get_current_params(m1_conference);
		expected_m1_video_capability = linphone_conference_params_video_enabled(params);
	}

	LinphoneConference *m2_conference = linphone_core_get_conference(m2->lc);
	bool_t expected_m2_video_capability = FALSE;
	if (m2_conference) {
		const LinphoneConferenceParams * params = linphone_conference_get_current_params(m2_conference);
		expected_m2_video_capability = linphone_conference_params_video_enabled(params);
	}

	LinphoneCall * m1_calls_m2 = linphone_core_get_call_by_remote_address2(m1->lc, m2->identity);
	BC_ASSERT_PTR_NOT_NULL(m1_calls_m2);

	LinphoneCall * m2_calls_m1 = linphone_core_get_call_by_remote_address2(m2->lc, m1->identity);
	BC_ASSERT_PTR_NOT_NULL(m2_calls_m1);

	if (m1_calls_m2) {
		stats initial_m2_stat = m2->stat;
		stats initial_m1_stat = m1->stat;
		const LinphoneCallParams *old_params = linphone_call_get_params(m1_calls_m2);
		LinphoneCallParams * new_params = linphone_call_params_copy(old_params);
		linphone_call_params_enable_video (new_params, enable_video);
		linphone_call_update(m1_calls_m2, new_params);
		linphone_call_params_unref (new_params);

		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallUpdatedByRemote, initial_m2_stat.number_of_LinphoneCallUpdatedByRemote + 1));

		int defer_update = !!linphone_config_get_int(linphone_core_get_config(m2->lc), "sip", "defer_update_default", FALSE);
		if (defer_update == TRUE) {
			LinphoneCallParams * m2_params = linphone_core_create_call_params(m2->lc, m2_calls_m1);
			linphone_call_params_enable_video(m2_params, exp_video_enabled);
			linphone_call_accept_update(m2_calls_m1, m2_params);
			linphone_call_params_unref(m2_params);
		}
		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallUpdating, initial_m1_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallStreamsRunning, initial_m2_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallStreamsRunning, initial_m1_stat.number_of_LinphoneCallStreamsRunning + 1));

		// Wait for first frame if video is enabled
		if (exp_video_enabled) {
			// Make sure video is received for participants. For conference we can't because of missing APIs.*/
			liblinphone_tester_set_next_video_frame_decoded_cb(m1_calls_m2);
			BC_ASSERT_TRUE( wait_for(m1->lc, m2->lc, &m1->stat.number_of_IframeDecoded, initial_m1_stat.number_of_IframeDecoded + 1));
		}

		// Check video parameters
		const LinphoneCallParams *m1_call_params = linphone_call_get_params(m1_calls_m2);
		BC_ASSERT_TRUE(linphone_call_params_video_enabled(m1_call_params) == exp_video_enabled);
		if (!m2_conference) {
			const LinphoneCallParams *m2_call_params = linphone_call_get_params(m2_calls_m1);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(m2_call_params) == exp_video_enabled);
		}
	}

	if (m1_conference) {
		// Verify that video capabilities are still enabled
		const LinphoneConferenceParams * params = linphone_conference_get_current_params(m1_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == expected_m1_video_capability);
	}

	if (m2_conference) {
		// Verify that video capabilities are still enabled
		const LinphoneConferenceParams * params = linphone_conference_get_current_params(m2_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == expected_m2_video_capability);
	}
}

static void toggle_video_settings_during_conference_base(bool_t automatically_video_accept, bool_t defer_update) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

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
	linphone_video_activation_policy_set_automatically_accept(pol, automatically_video_accept);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		linphone_core_set_video_activation_policy(c, pol);

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);

		LinphoneVideoActivationPolicy * cpol = linphone_core_get_video_activation_policy(c);
		BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(cpol) == automatically_video_accept);
		linphone_video_activation_policy_unref(cpol);
	}

	linphone_video_activation_policy_unref(pol);

	LinphoneCallParams * marie_call_params=linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams * pauline_call_params=linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneCallParams * laure_call_params=linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams * michelle_call_params=linphone_core_create_call_params(michelle->lc, NULL);
	if ((automatically_video_accept == FALSE) && (defer_update == FALSE)) {
		linphone_call_params_enable_video(marie_call_params,TRUE);
		linphone_call_params_enable_video(pauline_call_params,TRUE);
		linphone_call_params_enable_video(laure_call_params,TRUE);
		linphone_call_params_enable_video(michelle_call_params,TRUE);
	}

	BC_ASSERT_TRUE(call_with_params(marie,pauline,marie_call_params,pauline_call_params));
	marie_call_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call_with_params(marie,michelle,marie_call_params,michelle_call_params)))
		goto end;
	marie_call_michelle=linphone_core_get_current_call(marie->lc);
	michelle_called_by_marie=linphone_core_get_current_call(michelle->lc);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_michelle,michelle,michelle_called_by_marie));

	BC_ASSERT_TRUE(call_with_params(marie,laure,marie_call_params,laure_call_params));
	marie_call_laure=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure))
		goto end;

	linphone_call_params_unref(marie_call_params);
	linphone_call_params_unref(pauline_call_params);
	linphone_call_params_unref(michelle_call_params);
	linphone_call_params_unref(laure_call_params);

	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	unsigned int no_participants = (unsigned int)bctbx_list_size(new_participants);
	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),(no_participants+1), int, "%d");

	// Wait that the three participants have joined the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 2*no_participants, 10000));

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		if (defer_update == TRUE) {
			linphone_config_set_int(linphone_core_get_config(c), "sip", "defer_update_default", TRUE);
		}
	}

	// Enable video
	bool_t video_enabled = TRUE;
	set_video_in_conference(lcs, marie, new_participants, video_enabled);

	wait_for_list(lcs ,NULL, 0, 2000);

	// Disable video in calls of the remote participants
	video_enabled = FALSE;

	for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		set_video_in_call(m, marie, video_enabled, video_enabled);
	}

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);
	// Verify that video capabilities are still enabled
	const LinphoneConferenceParams * params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_video_enabled(params) == TRUE);

	wait_for_list(lcs ,NULL, 0, 2000);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),(no_participants+1), int, "%d");

	video_enabled = TRUE;
	// Video is enabled in the call between Marie and Laure
	set_video_in_call(laure, marie, video_enabled, video_enabled);

	wait_for_list(lcs ,NULL, 0, 2000);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),(no_participants+1), int, "%d");

	// Disable video capability in conference
	video_enabled = FALSE;
	set_video_in_conference(lcs, marie, new_participants, video_enabled);

	wait_for_list(lcs ,NULL, 0, 2000);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),(no_participants+1), int, "%d");

	// Pauline tries to enable video in the current call
	set_video_in_call(pauline, marie, TRUE, video_enabled);

	wait_for_list(lcs ,NULL, 0, 2000);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		if (defer_update == TRUE) {
			linphone_config_set_int(linphone_core_get_config(c), "sip", "defer_update_default", FALSE);
		}
	}

	terminate_conference(new_participants, marie, NULL, NULL);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(michelle->lc));

end:

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

static void update_conf_params_during_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

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
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

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

	// Remote  conference
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(pauline_called_by_marie));
	BC_ASSERT_FALSE(linphone_call_is_in_conference(pauline_called_by_marie));
	BC_ASSERT_TRUE(linphone_call_is_in_conference(marie_call_pauline));
	new_participants = terminate_participant_call(new_participants, marie, pauline);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	unsigned int no_participants = (unsigned int)bctbx_list_size(new_participants);
	// number of participants is incremented by 1 because Marie is in
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),no_participants+1, int, "%d");

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

	terminate_conference(new_participants, marie, NULL, NULL);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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

static void focus_takes_quick_call_after_conference_started_base(bool_t toggle_video) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_called_by_laure;
	LinphoneCall* laure_call_marie;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);

	bctbx_list_t* lcs2=bctbx_list_copy(lcs);
	lcs2=bctbx_list_append(lcs2,laure->lc);

	LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);

	for (bctbx_list_t *it = lcs2; it; it = bctbx_list_next(it)) {
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
	bctbx_list_free(lcs2);

	BC_ASSERT_TRUE(call(marie,pauline));
	marie_call_pauline=linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie=linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_TRUE(pause_call_1(marie,marie_call_pauline,pauline,pauline_called_by_marie));

	if (!BC_ASSERT_TRUE(call(marie,michelle)))
		goto end;

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(conference)) goto end;
	const LinphoneConferenceParams * conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(conf_params));

	bool_t video_enabled = !linphone_conference_params_video_enabled(conf_params);
	if (toggle_video == TRUE) {
		// Toggle video
		set_video_in_conference(lcs, marie, new_participants, video_enabled);
	}

	lcs=bctbx_list_append(lcs,laure->lc);
	BC_ASSERT_TRUE(call(laure,marie));

	marie_called_by_laure=linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_called_by_laure)) goto end;
	laure_call_marie=linphone_core_get_current_call(laure->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_call_marie)) goto end;

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(laure_call_marie)) == FALSE);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_called_by_laure)) == FALSE);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	// Local participant is expected to have left as it joined another call
	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));

	bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(participants), 2, unsigned int, "%u");
	bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

	conf_params = linphone_conference_get_current_params(conference);
	BC_ASSERT_FALSE(linphone_conference_params_local_participant_enabled(conf_params));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(laure->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	stats marie_stat = marie->stat;
	stats laure_stat = laure->stat;
	linphone_conference_enter(conference);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, marie_stat.number_of_LinphoneCallPausing + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, marie_stat.number_of_LinphoneCallPaused + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPausedByRemote, laure_stat.number_of_LinphoneCallPaused + 1, 5000));

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),3, int, "%d");
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(michelle->lc));

	if (toggle_video == TRUE) {
		conf_params = linphone_conference_get_current_params(conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(conf_params) == video_enabled);
	}

	wait_for_list(lcs ,NULL, 0, 2000);

	terminate_conference(new_participants, marie, NULL, NULL);
	bctbx_list_free(new_participants);

	unsigned int marie_call_no = (unsigned int)bctbx_list_size(linphone_core_get_calls(marie->lc));

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd, marie_stat.number_of_LinphoneCallEnd+marie_call_no,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd, laure_stat.number_of_LinphoneCallEnd+1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallReleased, marie_stat.number_of_LinphoneCallReleased+marie_call_no,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallReleased, laure_stat.number_of_LinphoneCallReleased+1,10000));

end:

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
	destroy_mgr_in_conference(marie);
	bctbx_list_free(lcs);
}

static void enable_video_during_conference_and_take_another_call(void) {
	focus_takes_quick_call_after_conference_started_base(TRUE);
}

static void focus_takes_quick_call_after_conference_started(void) {
	focus_takes_quick_call_after_conference_started_base(FALSE);
}

static void try_to_update_call_params_during_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

	LinphoneCall* marie_call_pauline;
	LinphoneCall* pauline_called_by_marie;
	LinphoneCall* marie_call_michelle;
	LinphoneCall* michelle_called_by_marie;
	LinphoneCall* marie_call_laure;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,michelle->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

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

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,laure);
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	// Wait that the three participants are joined to the local conference, by checking the StreamsRunning states
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&michelle->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning, 6, 10000));
	

	LinphoneConference * marie_conference = linphone_core_get_conference(marie->lc);
	const LinphoneConferenceParams * marie_conf_params = linphone_conference_get_current_params(marie_conference);
	bool_t orig_marie_video_enabled = linphone_conference_params_video_enabled(marie_conf_params);

	// Remote  conference
	LinphoneConference * pauline_conference = linphone_call_get_conference(pauline_called_by_marie);
	BC_ASSERT_PTR_NOT_NULL(pauline_conference);
	BC_ASSERT_FALSE(linphone_call_is_in_conference(pauline_called_by_marie));
	const LinphoneConferenceParams * pauline_conf_params = linphone_conference_get_current_params(pauline_conference);
	bool_t orig_pauline_video_enabled = linphone_conference_params_video_enabled(pauline_conf_params);

	if (pauline_called_by_marie) {
		stats initial_marie_stat = marie->stat;
		stats initial_pauline_stat = pauline->stat;
		const LinphoneCallParams *old_params = linphone_call_get_params(pauline_called_by_marie);
		LinphoneCallParams * new_params = linphone_call_params_copy(old_params);
		linphone_call_params_enable_video (new_params, TRUE);
		linphone_call_update(pauline_called_by_marie, new_params);
		linphone_call_params_unref (new_params);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote, initial_marie_stat.number_of_LinphoneCallUpdatedByRemote + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating, initial_pauline_stat.number_of_LinphoneCallUpdating + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, initial_pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 5000));

		// Test that update to call params of a participant didn't affect conference params
		const LinphoneConferenceParams * pauline_conf_params = linphone_conference_get_current_params(pauline_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(pauline_conf_params) == orig_pauline_video_enabled);

		const LinphoneConferenceParams * marie_conf_params = linphone_conference_get_current_params(marie_conference);
		BC_ASSERT_TRUE(linphone_conference_params_video_enabled(marie_conf_params) == orig_marie_video_enabled);

	}

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

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
		new_participants=bctbx_list_remove(new_participants,pauline);
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

	terminate_conference(new_participants, marie, NULL, NULL);
	bctbx_list_free(new_participants);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

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
	add_calls_to_local_conference(lcs, marie, NULL, new_participants);

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

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneRegistrationCleared, initial_laure_stats.number_of_LinphoneRegistrationCleared + 1, 5000));

	wait_for_list(lcs,NULL,1,3000);

	//to force re-re-connection to restarted flexisip
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, FALSE);
	linphone_core_set_network_reachable(marie->lc, FALSE);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionTerminated, 1, 10000));

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

	// Wait for subscriptins to be resent
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 5, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneSubscriptionActive, 2, 10000));

	BC_ASSERT_TRUE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),4, int, "%d");

	terminate_conference(new_participants, marie, NULL, NULL);
	bctbx_list_free(new_participants);

	BC_ASSERT_FALSE(linphone_core_is_in_conference(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(marie->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(marie->lc),0, int, "%d");
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

static void simple_conference_base2(LinphoneCoreManager* local_conf, bctbx_list_t * participants, bool_t use_conference_terminate) {

	bctbx_list_t* lcs=bctbx_list_append(NULL,local_conf->lc);
	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	bctbx_list_t* unique_participant_identity=NULL;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		if (!BC_ASSERT_TRUE(call(local_conf,m)))
			return;
		lcs=bctbx_list_append(lcs,c);
		if (bctbx_list_find(unique_participant_identity, m->identity) == NULL) {
			unique_participant_identity=bctbx_list_append(unique_participant_identity,m->identity);
		}
		LinphoneCall * conf_call_participant=linphone_core_get_current_call(local_conf->lc);
		BC_ASSERT_PTR_NOT_NULL(conf_call_participant);
		LinphoneCall * participant_called_by_conf=linphone_core_get_current_call(c);
		BC_ASSERT_PTR_NOT_NULL(participant_called_by_conf);
		// Last call is not put on hold
		if (bctbx_list_next(it)) {
			BC_ASSERT_TRUE(pause_call_1(local_conf,conf_call_participant,m,participant_called_by_conf));
		}
	}
	unsigned int no_unique_participants = (unsigned int)bctbx_list_size(unique_participant_identity);
	bctbx_list_free(unique_participant_identity);

	add_calls_to_local_conference(lcs, local_conf, NULL, participants);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		/* Wait that all participants have joined the local conference, by checking the StreamsRunning states*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs,&local_conf->stat.number_of_LinphoneCallStreamsRunning, 2*no_participants, 10000));
	
	BC_ASSERT_TRUE(linphone_core_is_in_conference(local_conf->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(local_conf->lc),no_unique_participants+1, int, "%d");

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(local_conf->lc));

	if (use_conference_terminate == TRUE) {
		terminate_conference(participants, local_conf, NULL, NULL);
	} else {
		stats initial_conf_stats = local_conf->stat;
		int counter = 1;
		stats* initial_participants_stats = NULL;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			// Particiopant stats
			initial_participants_stats = (stats*)realloc(initial_participants_stats, counter * sizeof(stats));
			initial_participants_stats[counter - 1] = m->stat;

			LinphoneCall * m_call = linphone_core_get_current_call(m->lc);
			BC_ASSERT_PTR_NOT_NULL(m_call);
			linphone_core_terminate_call(m->lc, m_call);

			counter++;
		}

		int idx = 0;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			BC_ASSERT_TRUE(wait_for(local_conf->lc,m->lc,&m->stat.number_of_LinphoneCallEnd,(initial_participants_stats[idx].number_of_LinphoneCallEnd + 1)));
			BC_ASSERT_TRUE(wait_for(local_conf->lc,m->lc,&local_conf->stat.number_of_LinphoneCallEnd,(initial_conf_stats.number_of_LinphoneCallEnd + idx + 1)));

			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_participants_stats[idx].number_of_LinphoneConferenceStateTerminationPending + 1),5000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneConferenceStateTerminated,(initial_participants_stats[idx].number_of_LinphoneConferenceStateTerminated + 1),5000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneConferenceStateDeleted,(initial_participants_stats[idx].number_of_LinphoneConferenceStateDeleted + 1),5000));

			BC_ASSERT_TRUE(wait_for_list(lcs,&local_conf->stat.number_of_LinphoneSubscriptionTerminated,initial_conf_stats.number_of_LinphoneSubscriptionTerminated + idx + 1,10000));
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneSubscriptionTerminated,initial_participants_stats[idx].number_of_LinphoneSubscriptionTerminated + 1,3000));

			BC_ASSERT_TRUE(wait_for(local_conf->lc,m->lc,&m->stat.number_of_LinphoneCallReleased,(initial_participants_stats[idx].number_of_LinphoneCallReleased + 1)));
			BC_ASSERT_TRUE(wait_for(local_conf->lc,m->lc,&local_conf->stat.number_of_LinphoneCallReleased,(initial_conf_stats.number_of_LinphoneCallReleased + idx + 1)));

			idx++;
		}

		if (initial_participants_stats) {
			ms_free(initial_participants_stats);
		}

		BC_ASSERT_TRUE(wait_for_list(lcs,&local_conf->stat.number_of_LinphoneSubscriptionTerminated,initial_conf_stats.number_of_LinphoneSubscriptionActive,10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&local_conf->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_conf_stats.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&local_conf->stat.number_of_LinphoneConferenceStateTerminated,(initial_conf_stats.number_of_LinphoneConferenceStateTerminated + 1),10000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&local_conf->stat.number_of_LinphoneConferenceStateDeleted,(initial_conf_stats.number_of_LinphoneConferenceStateDeleted + 1),10000));

	}

	BC_ASSERT_FALSE(linphone_core_is_in_conference(local_conf->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_conference(local_conf->lc));
	BC_ASSERT_EQUAL(linphone_core_get_conference_size(local_conf->lc),0, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_calls(local_conf->lc)), 0, unsigned int, "%u");
}

static void simple_4_participants_conference_ended_by_terminating_conference(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,laure);
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	simple_conference_base2(marie,participants,TRUE);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void simple_4_participants_conference_ended_by_terminating_calls(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,laure);
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	simple_conference_base2(marie,participants,FALSE);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

#if 0
static void simple_conference_with_multi_device(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* pauline2 = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);

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

static void simple_conference_with_participant_with_no_event_log(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_log_enabled",FALSE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);

	simple_conference_base(marie,pauline,laure, NULL, FALSE);

	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(marie);
}

void simple_remote_conference(void) {
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
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
	LinphoneCoreManager *marie = create_mgr_for_conference("marie_rc", TRUE);
	LinphoneCoreManager *pauline = create_mgr_for_conference("pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
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

void initiate_calls(bctbx_list_t* caller, LinphoneCoreManager* callee) {
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

	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCall * callee_call = linphone_core_get_call_by_remote_address2(callee->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(callee_call);
	}

	counter = 0;
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallOutgoingRinging,initial_callers_stats[counter].number_of_LinphoneCallOutgoingRinging + 1,5000));
		counter++;
	}

	bctbx_list_free(lcs);
	if (initial_callers_stats) {
		ms_free(initial_callers_stats);
	}
}

static void initiate_call(LinphoneCoreManager* caller, LinphoneCoreManager* callee) {
	bctbx_list_t *caller_list = NULL;
	caller_list=bctbx_list_append(caller_list,caller);
	initiate_calls(caller_list, callee);
	bctbx_list_free(caller_list);
}

static void take_calls_to_callee(bctbx_list_t* lcs, bctbx_list_t* caller, LinphoneCoreManager* callee) {

	LinphoneCall * current_call = linphone_core_get_current_call(callee->lc);
	LinphoneCall * current_call_caller = NULL;
	bool_t pausing_current_call = FALSE;
	if (current_call) {
		pausing_current_call = ((linphone_call_get_state(current_call) == LinphoneCallStreamsRunning) || (linphone_call_get_state(current_call) == LinphoneCallPaused));
		char* remote_address_string = linphone_call_get_remote_address_as_string(current_call);
		// Search core that matches the remote address 

		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneCoreManager * m = get_manager(c);
			char * identity_string = linphone_address_as_string(m->identity);
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
		LinphoneCoreManager * caller_mgr = (LinphoneCoreManager *)bctbx_list_get_data(it);
		const LinphoneAddress *caller_uri = caller_mgr->identity;

		// Decremement here no_paused_by_remote because past history has to be discarded for checks. Here, it is imprtant only the delta between before and after taking the call. As below no_paused_by_remote is incremented by the stats, a corrective factor (i.e. value before taking the call) is applied here
		no_paused_by_remote -= caller_mgr->stat.number_of_LinphoneCallPausedByRemote;
		LinphoneCall * callee_call = linphone_core_get_call_by_remote_address2(callee->lc, caller_uri);
		BC_ASSERT_PTR_NOT_NULL(callee_call);

		if (callee_call) {
			// Take call - ringing ends
			linphone_call_accept(callee_call);
		}
	}

	unsigned int no_callers = (unsigned int)bctbx_list_size(caller);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallStreamsRunning, initial_callee_stat.number_of_LinphoneCallStreamsRunning + no_callers, 5000));
	// Last call is not paused
	// If core had a running call, it will be paused
	unsigned int no_call_paused = no_callers - 1 + ((pausing_current_call) ? 1 : 0);
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPausing, initial_callee_stat.number_of_LinphoneCallPausing + no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPaused, initial_callee_stat.number_of_LinphoneCallPaused + no_call_paused, 5000));

	int updated_by_remote_count = 0;
	int call_checked_cnt = 0;
	bool_t callee_uses_ice = (linphone_core_get_firewall_policy(callee->lc) == LinphonePolicyUseIce);
	// Wait that all calls but the last one are paused
	for (bctbx_list_t *it = caller; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
		if (callee_uses_ice && linphone_core_get_firewall_policy(m->lc) == LinphonePolicyUseIce) {
			updated_by_remote_count++;
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallUpdating,1,5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallUpdatedByRemote, initial_callee_stat.number_of_LinphoneCallUpdatedByRemote + updated_by_remote_count, 5000));

			LinphoneCall * callee_call = linphone_core_get_call_by_remote_address2(callee->lc, m->identity);
			LinphoneCall * current_callee_call = linphone_core_get_current_call(callee->lc);
			if (callee_call == current_callee_call) {
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallStreamsRunning, initial_callee_stat.number_of_LinphoneCallStreamsRunning + no_callers + 1, 5000));
			} else {
				call_checked_cnt++;
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallPausedByRemote, 2, 5000));
				BC_ASSERT_TRUE(wait_for_list(lcs, &callee->stat.number_of_LinphoneCallPaused, initial_callee_stat.number_of_LinphoneCallPaused + call_checked_cnt, 5000));
				// If ICE is enabled, calls are paused twice:
				// - after accepting another call
				// - after ICE negotiation ends
				no_call_paused++;
			}
			BC_ASSERT_TRUE(check_ice(m,callee,LinphoneIceStateHostConnection));
			BC_ASSERT_TRUE(check_ice(callee,m,LinphoneIceStateHostConnection));
		}
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

static void conference_with_calls_queued(LinphoneCoreManager* local_conf, LinphoneConference * conference, bctbx_list_t* participants, bool_t back_to_back_invite, bool_t back_to_back_accept) {
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

	add_calls_to_local_conference(lcs, local_conf, conference, participants);

	terminate_conference(participants, local_conf, conference, NULL);

	bctbx_list_free(lcs);
}

static void conference_with_calls_queued_without_ice(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);
	if (linphone_core_media_encryption_supported(laure->lc,mode)) {
		linphone_core_set_firewall_policy(laure->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(laure->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);
	linphone_core_set_inc_timeout(chloe->lc, 10000);
	if (linphone_core_media_encryption_supported(chloe->lc,mode)) {
		linphone_core_set_firewall_policy(chloe->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc,mode);
	}

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,chloe);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, NULL, participants, FALSE, FALSE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}

static void conference_with_back_to_back_call_accept_without_ice(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);
	if (linphone_core_media_encryption_supported(laure->lc,mode)) {
		linphone_core_set_firewall_policy(laure->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(laure->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);
	linphone_core_set_inc_timeout(chloe->lc, 10000);
	if (linphone_core_media_encryption_supported(chloe->lc,mode)) {
		linphone_core_set_firewall_policy(chloe->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc,mode);
	}

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,chloe);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	conference_with_calls_queued(marie, NULL, participants, FALSE, TRUE);

	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}

static void conference_with_back_to_back_call_invite_accept_without_ice(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);
	if (linphone_core_media_encryption_supported(laure->lc,mode)) {
		linphone_core_set_firewall_policy(laure->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(laure->lc,mode);
	}

	// ICE is enabled
	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);
	linphone_core_set_inc_timeout(chloe->lc, 10000);
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

static void back_to_back_conferences(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,laure);

	// Marie hosts the conference
	conference_with_calls_queued(marie, NULL, participants, FALSE, FALSE);
	bctbx_list_free(participants);

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,michelle);
	new_participants=bctbx_list_append(new_participants,pauline);
	new_participants=bctbx_list_append(new_participants,marie);

	// Laure hosts the conference
	conference_with_calls_queued(laure, NULL, new_participants, FALSE, FALSE);
	bctbx_list_free(new_participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(michelle);
}

static void try_to_create_second_conference_with_local_participant(void) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);
	linphone_core_set_inc_timeout(chloe->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,laure);
	participants=bctbx_list_append(participants,chloe);
	participants=bctbx_list_append(participants,pauline);

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,marie->lc);

	stats initial_marie_stat = marie->stat;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);
		initiate_call(m, marie);
	}

	//Let ring calls for a little while
	wait_for_list(lcs,NULL,0,1000);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausing, initial_marie_stat.number_of_LinphoneCallPausing, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, initial_marie_stat.number_of_LinphoneCallPaused, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote, initial_marie_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(marie->lc));

	add_calls_to_local_conference(lcs, marie, NULL, participants);

	stats* lcm_stats = NULL;

	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		lcm_stats = (stats*)realloc(lcm_stats, counter * sizeof(stats));

		// Append element
		lcm_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	lcm_stats = (stats*)realloc(lcm_stats, counter * sizeof(stats));
	lcm_stats[counter - 1] = marie->stat;

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	linphone_core_terminate_conference(marie->lc);

	LinphoneConferenceParams * new_maries_conference_params = linphone_conference_params_new (marie->lc);
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference_params);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(new_maries_conference_params));
	LinphoneConference * new_maries_conference = linphone_core_create_conference_with_params(marie->lc, new_maries_conference_params);
	BC_ASSERT_PTR_NULL(new_maries_conference);
	linphone_conference_params_unref(new_maries_conference_params);

	int idx = 0;
	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	participants=bctbx_list_append(participants,marie);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;

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
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd, lcm_stats[idx].number_of_LinphoneCallEnd + no_calls, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased, lcm_stats[idx].number_of_LinphoneCallReleased + no_calls, 10000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending, lcm_stats[idx].number_of_LinphoneConferenceStateTerminationPending + no_conference, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated, lcm_stats[idx].number_of_LinphoneConferenceStateTerminated + no_conference, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted, lcm_stats[idx].number_of_LinphoneConferenceStateDeleted + no_conference, 5000));

		bool_t event_log_enabled = linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE );
		if ((m != marie) && event_log_enabled) {
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneSubscriptionTerminated,lcm_stats[idx].number_of_LinphoneSubscriptionTerminated + no_conference,10000));
		}

		LinphoneConference *conference = linphone_core_get_conference(c);

		BC_ASSERT_PTR_NULL(conference);
		BC_ASSERT_FALSE(linphone_core_is_in_conference(c));

		if (m != marie) {
			LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(m->lc, marie->identity);
			BC_ASSERT_PTR_NULL(participant_call);
			LinphoneCall * conference_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
			BC_ASSERT_PTR_NULL(conference_call);
		}

		idx++;
	}

	// Verify that a second conference is not created
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateCreated, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminationPending, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateTerminated, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceStateDeleted, 1, int, "%d");

	ms_free(lcm_stats);

	bctbx_list_free(lcs);
	bctbx_list_free(participants);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);
	destroy_mgr_in_conference(chloe);
}

static void interleaved_conferences_base(bool_t add_participants_immediately_after_creation) {
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);
	linphone_core_set_inc_timeout(chloe->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,marie->lc);

	stats initial_marie_stat = marie->stat;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);
		initiate_call(m, marie);
	}

	//Let ring calls for a little while
	wait_for_list(lcs,NULL,0,1000);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausing, initial_marie_stat.number_of_LinphoneCallPausing, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, initial_marie_stat.number_of_LinphoneCallPaused, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote, initial_marie_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(marie->lc));

	add_calls_to_local_conference(lcs, marie, NULL, participants);

	stats* lcm_stats = NULL;

	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		lcm_stats = (stats*)realloc(lcm_stats, counter * sizeof(stats));

		// Append element
		lcm_stats[counter - 1] = m->stat;

		// Increment counter
		counter++;
	}

	lcm_stats = (stats*)realloc(lcm_stats, counter * sizeof(stats));
	lcm_stats[counter - 1] = marie->stat;

	LinphoneConference *conference = linphone_core_get_conference(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(conference);

	linphone_core_terminate_conference(marie->lc);

	LinphoneConferenceParams * new_maries_conference_params = linphone_conference_params_new (marie->lc);
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference_params);
	linphone_conference_params_enable_local_participant(new_maries_conference_params, FALSE);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(new_maries_conference_params) == FALSE);
	LinphoneConference * new_maries_conference = linphone_core_create_conference_with_params(marie->lc, new_maries_conference_params);
	BC_ASSERT_PTR_NOT_NULL(new_maries_conference);
	linphone_conference_params_unref(new_maries_conference_params);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 2, 5000));

	bctbx_list_t* new_participants=NULL;
	new_participants=bctbx_list_append(new_participants,laure);
	new_participants=bctbx_list_append(new_participants,chloe);

	if (add_participants_immediately_after_creation == TRUE) {
		conference_with_calls_queued(marie, new_maries_conference, new_participants, FALSE, FALSE);
	}

	int idx = 0;

	participants=bctbx_list_append(participants,marie);
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;

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
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd, lcm_stats[idx].number_of_LinphoneCallEnd + no_calls, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased, lcm_stats[idx].number_of_LinphoneCallReleased + no_calls, 10000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending, lcm_stats[idx].number_of_LinphoneConferenceStateTerminationPending + no_conference, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated, lcm_stats[idx].number_of_LinphoneConferenceStateTerminated + no_conference, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted, lcm_stats[idx].number_of_LinphoneConferenceStateDeleted + no_conference, 5000));

		bool_t event_log_enabled = linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE );
		if ((m != marie) && event_log_enabled) {
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneSubscriptionTerminated,lcm_stats[idx].number_of_LinphoneSubscriptionTerminated + no_conference,10000));
		}

		LinphoneConference *conference = linphone_core_get_conference(c);

		BC_ASSERT_PTR_NULL(conference);
		BC_ASSERT_FALSE(linphone_core_is_in_conference(c));

		if (m != marie) {
			LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(m->lc, marie->identity);
			BC_ASSERT_PTR_NULL(participant_call);
			LinphoneCall * conference_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
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
	LinphoneCoreManager* marie = create_mgr_for_conference( "marie_rc", TRUE);
	linphone_core_enable_conference_server(marie->lc,TRUE);
	linphone_core_set_inc_timeout(marie->lc, 10000);

	LinphoneCoreManager* pauline = create_mgr_for_conference( "pauline_tcp_rc", TRUE);
	linphone_core_set_inc_timeout(pauline->lc, 10000);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_inc_timeout(laure->lc, 10000);

	LinphoneCoreManager* michelle = create_mgr_for_conference( "michelle_rc_udp", TRUE);
	linphone_core_set_inc_timeout(michelle->lc, 10000);

	LinphoneCoreManager* chloe = create_mgr_for_conference( "chloe_rc", TRUE);
	linphone_core_set_inc_timeout(chloe->lc, 10000);

	bctbx_list_t* participants=NULL;
	participants=bctbx_list_append(participants,michelle);
	participants=bctbx_list_append(participants,pauline);

	bctbx_list_t* participants2=NULL;
	participants2=bctbx_list_append(participants2,laure);
	participants2=bctbx_list_append(participants2,chloe);

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,marie->lc);

	bctbx_list_t* lcs1 = NULL;
	lcs1=bctbx_list_append(lcs1,marie->lc);

	stats initial_marie_stat = marie->stat;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs1=bctbx_list_append(lcs1,c);
		lcs=bctbx_list_append(lcs,c);
		initiate_call(m, marie);
	}

	bctbx_list_t* lcs2 = NULL;
	lcs2=bctbx_list_append(lcs2,marie->lc);

	for (bctbx_list_t *it = participants2; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs2=bctbx_list_append(lcs2,c);
		lcs=bctbx_list_append(lcs,c);
		initiate_call(m, marie);
	}

	//Let ring calls for a little while
	wait_for_list(lcs,NULL,0,1000);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, initial_marie_stat.number_of_LinphoneCallStreamsRunning, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausing, initial_marie_stat.number_of_LinphoneCallPausing, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPaused, initial_marie_stat.number_of_LinphoneCallPaused, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote, initial_marie_stat.number_of_LinphoneCallPausedByRemote, int, "%d");

	for (bctbx_list_t *it = participants2; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		take_call_to_callee(lcs, m, marie);
	}

	BC_ASSERT_FALSE(linphone_core_sound_resources_locked(marie->lc));

	LinphoneConferenceParams * maries_conference_params = linphone_conference_params_new (marie->lc);
	BC_ASSERT_PTR_NOT_NULL(maries_conference_params);
	linphone_conference_params_enable_local_participant(maries_conference_params, FALSE);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(maries_conference_params) == FALSE);
	LinphoneConference * maries_conference = linphone_core_create_conference_with_params(marie->lc, maries_conference_params);
	BC_ASSERT_PTR_NOT_NULL(maries_conference);
	linphone_conference_params_unref(maries_conference_params);

	add_calls_to_local_conference(lcs1, marie, maries_conference, participants);

	LinphoneConferenceParams * maries_conference_params2 = linphone_conference_params_new (marie->lc);
	BC_ASSERT_PTR_NOT_NULL(maries_conference_params2);
	linphone_conference_params_enable_local_participant(maries_conference_params2, FALSE);
	BC_ASSERT_TRUE(linphone_conference_params_local_participant_enabled(maries_conference_params2) == FALSE);
	LinphoneConference * maries_conference2 = linphone_core_create_conference_with_params(marie->lc, maries_conference_params2);
	BC_ASSERT_PTR_NOT_NULL(maries_conference2);
	linphone_conference_params_unref(maries_conference_params2);

	add_calls_to_local_conference(lcs2, marie, maries_conference2, participants2);

	terminate_conference(participants2, marie, maries_conference2, NULL);
	terminate_conference(participants, marie, maries_conference, NULL);

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

test_t audio_video_conference_tests[] = {
	TEST_NO_TAG("Simple conference", simple_conference),
	TEST_NO_TAG("Simple conference estblished before proxy config is created", simple_conference_established_before_proxy_config_creation),
	TEST_NO_TAG("Simple conference with participant with no event log", simple_conference_with_participant_with_no_event_log),
	TEST_NO_TAG("Simple conference established from scratch", simple_conference_from_scratch),
	TEST_NO_TAG("Simple conference established from scratch with video", simple_conference_from_scratch_with_video),
	TEST_NO_TAG("Simple 4 participant conference ended by terminating conference", simple_4_participants_conference_ended_by_terminating_conference),
	TEST_NO_TAG("Simple 4 participant conference ended by terminating all calls", simple_4_participants_conference_ended_by_terminating_calls),
//	TEST_NO_TAG("Simple conference with multi device", simple_conference_with_multi_device),
	TEST_NO_TAG("Video conference by merging calls", video_conference_by_merging_calls),
	TEST_NO_TAG("Simple conference established from scratch, but attendees do not answer", simple_conference_from_scratch_no_answer),
	TEST_ONE_TAG("Simple conference with ICE", simple_conference_with_ice, "ICE"),
	TEST_ONE_TAG("Simple ZRTP conference with ICE", simple_zrtp_conference_with_ice, "ICE"),
	TEST_NO_TAG("Simple conference with no conversion to call", simple_conference_not_converted_to_call),
	TEST_NO_TAG("Eject from 3 participants conference", eject_from_3_participants_local_conference),
	TEST_NO_TAG("Eject from 4 participants conference", eject_from_4_participants_conference),
	TEST_NO_TAG("Conference pause and terminate call", conference_hang_up_call_on_hold),
	TEST_NO_TAG("Participants exit conference after pausing", participants_exit_conference_after_pausing),
	TEST_NO_TAG("Add participant after conference started", add_participant_after_conference_started),
	TEST_NO_TAG("Focus takes quick call after conference started and then resumes it", focus_takes_quick_call_after_conference_started),
	TEST_NO_TAG("Focus takes call after conference started and participants leave", focus_takes_call_after_conference_started_and_participants_leave),
	TEST_NO_TAG("Participant takes call after conference started and conference ends", participant_takes_call_after_conference_started_and_conference_ends),
	TEST_NO_TAG("Participant takes call after conference started and rejoins conference", participant_takes_call_after_conference_started_and_rejoins_conference),
	TEST_NO_TAG("Participant takes call after conference started and rejoins conference after conference ended", participant_takes_call_after_conference_started_and_rejoins_conference_after_conference_ended),
	TEST_ONE_TAG("Register again during conference", register_again_during_conference, "LeaksMemory"), /* due to re-registration of cores */
	TEST_NO_TAG("Try to update call parameter during conference", try_to_update_call_params_during_conference),
	TEST_NO_TAG("Update conference parameter during conference", update_conf_params_during_conference),
	TEST_NO_TAG("Toggle video settings during conference without automatically accept video policy", toggle_video_settings_during_conference_without_automatically_accept_video_policy),
	TEST_NO_TAG("Toggle video settings during conference with automatically accept video policy", toggle_video_settings_during_conference_with_automatically_accept_video_policy),
	TEST_NO_TAG("Toggle video settings during conference with update deferred", toggle_video_settings_during_conference_with_update_deferred),
	TEST_NO_TAG("Enable video during conference and take another call", enable_video_during_conference_and_take_another_call),
	TEST_NO_TAG("Back to back conferences", back_to_back_conferences),
	TEST_NO_TAG("Try to create second conference with local participant", try_to_create_second_conference_with_local_participant),
	TEST_NO_TAG("Interleaved conference creation", interleaved_conference_creation),
	TEST_NO_TAG("Interleaved conference creation with participant added before the first one ends", interleaved_conference_creation_with_quick_participant_addition),
	TEST_NO_TAG("Multiple conferences in server mode", multiple_conferences_in_server_mode),
	TEST_NO_TAG("Conference with calls queued without ICE", conference_with_calls_queued_without_ice),
	TEST_ONE_TAG("Conference with calls queued with ICE", conference_with_calls_queued_with_ice, "ICE"),
	TEST_NO_TAG("Conference with back to back call accept without ICE", conference_with_back_to_back_call_accept_without_ice),
	TEST_ONE_TAG("Conference with back to back call accept with ICE", conference_with_back_to_back_call_accept_with_ice, "ICE"),
	TEST_NO_TAG("Conference with back to back call invite and accept without ICE", conference_with_back_to_back_call_invite_accept_without_ice),
//	TEST_ONE_TAG("Conference with back to back call invite and accept with ICE", conference_with_back_to_back_call_invite_accept_with_ice, "ICE"),
	TEST_NO_TAG("Simple remote conference", simple_remote_conference),
	TEST_NO_TAG("Simple remote conference with shut down focus", simple_remote_conference_shut_down_focus),
	TEST_NO_TAG("Eject from 3 participants in remote conference", eject_from_3_participants_remote_conference),
};

test_suite_t audio_video_conference_test_suite = {"Audio video conference", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
									  sizeof(audio_video_conference_tests) / sizeof(audio_video_conference_tests[0]), audio_video_conference_tests};
