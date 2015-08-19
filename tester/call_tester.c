/*
	liblinphone_tester - liblinphone test suite
	Copyright (C) 2013  Belledonne Communications SARL

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <sys/types.h>
#include <sys/stat.h>
#include "linphonecore.h"
#include "lpconfig.h"
#include "private.h"
#include "liblinphone_tester.h"
#include "mediastreamer2/msutils.h"
#include "belle-sip/sipstack.h"

#ifdef _WIN32
#define unlink _unlink
#ifndef F_OK
#define F_OK 00 /*visual studio does not define F_OK*/
#endif
#endif

static void srtp_call(void);
static char *create_filepath(const char *dir, const char *filename, const char *ext);

// prototype definition for call_recording()
#ifdef ANDROID
#ifdef HAVE_OPENH264
extern void libmsopenh264_init(void);
#endif
#endif

void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
	char* to=linphone_address_as_string(linphone_call_get_call_log(call)->to);
	char* from=linphone_address_as_string(linphone_call_get_call_log(call)->from);
	stats* counters;
	ms_message(" %s call from [%s] to [%s], new state is [%s]"	,linphone_call_get_call_log(call)->dir==LinphoneCallIncoming?"Incoming":"Outgoing"
																,from
																,to
																,linphone_call_state_to_string(cstate));
	ms_free(to);
	ms_free(from);
	counters = get_stats(lc);
	switch (cstate) {
	case LinphoneCallIncomingReceived:counters->number_of_LinphoneCallIncomingReceived++;break;
	case LinphoneCallOutgoingInit :counters->number_of_LinphoneCallOutgoingInit++;break;
	case LinphoneCallOutgoingProgress :counters->number_of_LinphoneCallOutgoingProgress++;break;
	case LinphoneCallOutgoingRinging :counters->number_of_LinphoneCallOutgoingRinging++;break;
	case LinphoneCallOutgoingEarlyMedia :counters->number_of_LinphoneCallOutgoingEarlyMedia++;break;
	case LinphoneCallConnected :counters->number_of_LinphoneCallConnected++;break;
	case LinphoneCallStreamsRunning :counters->number_of_LinphoneCallStreamsRunning++;break;
	case LinphoneCallPausing :counters->number_of_LinphoneCallPausing++;break;
	case LinphoneCallPaused :counters->number_of_LinphoneCallPaused++;break;
	case LinphoneCallResuming :counters->number_of_LinphoneCallResuming++;break;
	case LinphoneCallRefered :counters->number_of_LinphoneCallRefered++;break;
	case LinphoneCallError :counters->number_of_LinphoneCallError++;break;
	case LinphoneCallEnd :counters->number_of_LinphoneCallEnd++;break;
	case LinphoneCallPausedByRemote :counters->number_of_LinphoneCallPausedByRemote++;break;
	case LinphoneCallUpdatedByRemote :counters->number_of_LinphoneCallUpdatedByRemote++;break;
	case LinphoneCallIncomingEarlyMedia :counters->number_of_LinphoneCallIncomingEarlyMedia++;break;
	case LinphoneCallUpdating :counters->number_of_LinphoneCallUpdating++;break;
	case LinphoneCallReleased :counters->number_of_LinphoneCallReleased++;break;
	case LinphoneCallEarlyUpdating: counters->number_of_LinphoneCallEarlyUpdating++;break;
	case LinphoneCallEarlyUpdatedByRemote: counters->number_of_LinphoneCallEarlyUpdatedByRemote++;break;
	default:
		BC_FAIL("unexpected event");break;
	}
}

void call_stats_updated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *lstats) {
	stats* counters = get_stats(lc);
	counters->number_of_LinphoneCallStatsUpdated++;
	if (lstats->updated & LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE) {
		counters->number_of_rtcp_received++;
	}
	if (lstats->updated & LINPHONE_CALL_STATS_SENT_RTCP_UPDATE ) {
		counters->number_of_rtcp_sent++;
	}
	if (lstats->updated & LINPHONE_CALL_STATS_PERIODICAL_UPDATE ) {
		int tab_size = sizeof (counters->audio_download_bandwidth)/sizeof(int);
		int index =  (counters->current_bandwidth_index++) % tab_size;

		counters->current_audio_download_bandwidth = counters->audio_download_bandwidth + index;
		counters->current_audio_upload_bandwidth = counters->audio_upload_bandwidth +index;

		counters->audio_download_bandwidth[index] = linphone_call_get_audio_stats(call)->download_bandwidth;
		counters->audio_upload_bandwidth[index] = linphone_call_get_audio_stats(call)->upload_bandwidth;
		counters->video_download_bandwidth[index] = linphone_call_get_video_stats(call)->download_bandwidth;
		counters->video_upload_bandwidth[index] = linphone_call_get_video_stats(call)->upload_bandwidth;
	}

}

void linphone_call_encryption_changed(LinphoneCore *lc, LinphoneCall *call, bool_t on, const char *authentication_token) {
	char* to=linphone_address_as_string(linphone_call_get_call_log(call)->to);
	char* from=linphone_address_as_string(linphone_call_get_call_log(call)->from);
	stats* counters;
	ms_message(" %s call from [%s] to [%s], is now [%s]",linphone_call_get_call_log(call)->dir==LinphoneCallIncoming?"Incoming":"Outgoing"
														,from
														,to
														,(on?"encrypted":"unencrypted"));
	ms_free(to);
	ms_free(from);
	counters = get_stats(lc);
	if (on)
		counters->number_of_LinphoneCallEncryptedOn++;
	else
		counters->number_of_LinphoneCallEncryptedOff++;
}

void linphone_transfer_state_changed(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	char* to=linphone_address_as_string(linphone_call_get_call_log(transfered)->to);
	char* from=linphone_address_as_string(linphone_call_get_call_log(transfered)->from);
	stats* counters;
	ms_message("Transferred call from [%s] to [%s], new state is [%s]",from,to,linphone_call_state_to_string(new_call_state));
	ms_free(to);
	ms_free(from);

	counters = get_stats(lc);
	switch (new_call_state) {
	case LinphoneCallOutgoingInit :counters->number_of_LinphoneTransferCallOutgoingInit++;break;
	case LinphoneCallOutgoingProgress :counters->number_of_LinphoneTransferCallOutgoingProgress++;break;
	case LinphoneCallOutgoingRinging :counters->number_of_LinphoneTransferCallOutgoingRinging++;break;
	case LinphoneCallOutgoingEarlyMedia :counters->number_of_LinphoneTransferCallOutgoingEarlyMedia++;break;
	case LinphoneCallConnected :counters->number_of_LinphoneTransferCallConnected++;break;
	case LinphoneCallStreamsRunning :counters->number_of_LinphoneTransferCallStreamsRunning++;break;
	case LinphoneCallError :counters->number_of_LinphoneTransferCallError++;break;
	default:
		BC_FAIL("unexpected event");break;
	}
}


void linphone_call_cb(LinphoneCall *call,void * user_data) {
	char* to=linphone_address_as_string(linphone_call_get_call_log(call)->to);
	char* from=linphone_address_as_string(linphone_call_get_call_log(call)->from);
	stats* counters;
	LinphoneCore* lc=(LinphoneCore*)user_data;
	ms_message("call from [%s] to [%s] receive iFrame",from,to);
	ms_free(to);
	ms_free(from);
	counters = (stats*)get_stats(lc);
	counters->number_of_IframeDecoded++;
}

void liblinphone_tester_check_rtcp(LinphoneCoreManager* caller, LinphoneCoreManager* callee) {
	LinphoneCall *c1,*c2;
	MSTimeSpec ts;

	c1=linphone_core_get_current_call(caller->lc);
	c2=linphone_core_get_current_call(callee->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	if (!c1 || !c2) return;
	linphone_call_ref(c1);
	linphone_call_ref(c2);

	liblinphone_tester_clock_start(&ts);
	do {
		if (linphone_call_get_audio_stats(c1)->round_trip_delay > 0.0
				&& linphone_call_get_audio_stats(c2)->round_trip_delay > 0.0
				&& (!linphone_call_log_video_enabled(linphone_call_get_call_log(c1)) || linphone_call_get_video_stats(c1)->round_trip_delay>0.0)
				&& (!linphone_call_log_video_enabled(linphone_call_get_call_log(c2))  || linphone_call_get_video_stats(c2)->round_trip_delay>0.0)) {
			break;

		}
		wait_for_until(caller->lc,callee->lc,NULL,0,20); /*just to sleep while iterating*/
	}while (!liblinphone_tester_clock_elapsed(&ts,15000));
	BC_ASSERT_GREATER(linphone_call_get_audio_stats(c1)->round_trip_delay,0.0,float,"%f");
	BC_ASSERT_GREATER(linphone_call_get_audio_stats(c2)->round_trip_delay,0.0,float,"%f");
	if (linphone_call_log_video_enabled(linphone_call_get_call_log(c1))) {
		BC_ASSERT_GREATER(linphone_call_get_video_stats(c1)->round_trip_delay,0.0,float,"%f");
	}
	if (linphone_call_log_video_enabled(linphone_call_get_call_log(c2))) {
		BC_ASSERT_GREATER(linphone_call_get_video_stats(c2)->round_trip_delay,0.0,float,"%f");
	}
	linphone_call_unref(c1);
	linphone_call_unref(c2);
}

static void setup_sdp_handling(const LinphoneCallTestParams* params, LinphoneCoreManager* mgr ){
	if( params->sdp_removal ){
		sal_default_set_sdp_handling(mgr->lc->sal, SalOpSDPSimulateRemove);
	} else if( params->sdp_simulate_error ){
		sal_default_set_sdp_handling(mgr->lc->sal, SalOpSDPSimulateError);
	}
}

bool_t call_with_params2(LinphoneCoreManager* caller_mgr
						,LinphoneCoreManager* callee_mgr
						, const LinphoneCallTestParams *caller_test_params
						, const LinphoneCallTestParams *callee_test_params
						, bool_t build_callee_params) {
	int retry=0;
	stats initial_caller=caller_mgr->stat;
	stats initial_callee=callee_mgr->stat;
	bool_t result=FALSE;
	LinphoneCallParams *caller_params = caller_test_params->base;
	LinphoneCallParams *callee_params = callee_test_params->base;
	bool_t did_receive_call;
	LinphoneCall *callee_call=NULL;

	setup_sdp_handling(caller_test_params, caller_mgr);
	setup_sdp_handling(callee_test_params, callee_mgr);

	if (!caller_params){
		BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(caller_mgr->lc,callee_mgr->identity));
	}else{
		BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(caller_mgr->lc,callee_mgr->identity,caller_params));
	}

	did_receive_call = wait_for(callee_mgr->lc
				,caller_mgr->lc
				,&callee_mgr->stat.number_of_LinphoneCallIncomingReceived
				,initial_callee.number_of_LinphoneCallIncomingReceived+1);
	BC_ASSERT_EQUAL(did_receive_call, !callee_test_params->sdp_simulate_error, int, "%d");

	sal_default_set_sdp_handling(caller_mgr->lc->sal, SalOpSDPNormal);
	sal_default_set_sdp_handling(callee_mgr->lc->sal, SalOpSDPNormal);

	if (!did_receive_call) return 0;


	if (linphone_core_get_calls_nb(callee_mgr->lc)<=1)
		BC_ASSERT_TRUE(linphone_core_inc_invite_pending(callee_mgr->lc));
	BC_ASSERT_EQUAL(caller_mgr->stat.number_of_LinphoneCallOutgoingProgress,initial_caller.number_of_LinphoneCallOutgoingProgress+1, int, "%d");


	while (caller_mgr->stat.number_of_LinphoneCallOutgoingRinging!=(initial_caller.number_of_LinphoneCallOutgoingRinging + 1)
			&& caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia!=(initial_caller.number_of_LinphoneCallOutgoingEarlyMedia +1)
			&& retry++ < 100) {
			linphone_core_iterate(caller_mgr->lc);
			linphone_core_iterate(callee_mgr->lc);
			ms_usleep(20000);
	}


	BC_ASSERT_TRUE((caller_mgr->stat.number_of_LinphoneCallOutgoingRinging==initial_caller.number_of_LinphoneCallOutgoingRinging+1)
							||(caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia==initial_caller.number_of_LinphoneCallOutgoingEarlyMedia+1));


	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call_remote_address(callee_mgr->lc));
	callee_call=linphone_core_get_call_by_remote_address2(callee_mgr->lc,caller_mgr->identity);

	if(!linphone_core_get_current_call(caller_mgr->lc) || !linphone_core_get_current_call(callee_mgr->lc) || !linphone_core_get_current_call_remote_address(callee_mgr->lc)) {
		return 0;
	} else if (caller_mgr->identity){
		LinphoneAddress* callee_from=linphone_address_clone(caller_mgr->identity);
		linphone_address_set_port(callee_from,0); /*remove port because port is never present in from header*/

		if (linphone_call_params_get_privacy(linphone_call_get_current_params(linphone_core_get_current_call(caller_mgr->lc))) == LinphonePrivacyNone) {
			/*don't check in case of p asserted id*/
			if (!lp_config_get_int(callee_mgr->lc->config,"sip","call_logs_use_asserted_id_instead_of_from",0))
				BC_ASSERT_TRUE(linphone_address_weak_equal(callee_from,linphone_call_get_remote_address(callee_call)));
		} else {
			BC_ASSERT_FALSE(linphone_address_weak_equal(callee_from,linphone_call_get_remote_address(linphone_core_get_current_call(callee_mgr->lc))));
		}
		linphone_address_destroy(callee_from);
	}


	if (callee_params){
		linphone_core_accept_call_with_params(callee_mgr->lc,callee_call,callee_params);
	}else if (build_callee_params){
		LinphoneCallParams *default_params=linphone_core_create_call_params(callee_mgr->lc,callee_call);
		ms_message("Created default call params with video=%i", linphone_call_params_video_enabled(default_params));
		linphone_core_accept_call_with_params(callee_mgr->lc,callee_call,default_params);
		linphone_call_params_destroy(default_params);
	}else{
		linphone_core_accept_call(callee_mgr->lc,callee_call);
	}

	BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallConnected,initial_callee.number_of_LinphoneCallConnected+1));
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallConnected,initial_callee.number_of_LinphoneCallConnected+1));

	result = wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_caller.number_of_LinphoneCallStreamsRunning+1)
			&&
			wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_callee.number_of_LinphoneCallStreamsRunning+1);

	if (linphone_core_get_media_encryption(caller_mgr->lc) != LinphoneMediaEncryptionNone
		|| linphone_core_get_media_encryption(callee_mgr->lc) != LinphoneMediaEncryptionNone) {
		/*wait for encryption to be on, in case of zrtp or dtls, it can take a few seconds*/
		if (	(linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionZRTP)
				|| (linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionDTLS))
			wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallEncryptedOn,initial_caller.number_of_LinphoneCallEncryptedOn+1);
		if ((linphone_core_get_media_encryption(callee_mgr->lc) == LinphoneMediaEncryptionZRTP)
			|| (linphone_core_get_media_encryption(callee_mgr->lc) == LinphoneMediaEncryptionDTLS)
			|| (linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionDTLS) /*also take care of caller policy*/ )
			wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallEncryptedOn,initial_callee.number_of_LinphoneCallEncryptedOn+1);
		{
			const LinphoneCallParams* call_param = linphone_call_get_current_params(callee_call);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller_mgr->lc), int, "%d");
			call_param = linphone_call_get_current_params(linphone_core_get_current_call(caller_mgr->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller_mgr->lc), int, "%d");

		}
	}
	/*wait ice re-invite*/
	if (linphone_core_get_firewall_policy(caller_mgr->lc) == LinphonePolicyUseIce
			&& linphone_core_get_firewall_policy(callee_mgr->lc) == LinphonePolicyUseIce
			&& !linphone_core_sdp_200_ack_enabled(caller_mgr->lc)) { /*ice does not work with sdp less invite*/
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_caller.number_of_LinphoneCallStreamsRunning+2));
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_callee.number_of_LinphoneCallStreamsRunning+2));

	}
	return result;
}

bool_t call_with_params(LinphoneCoreManager* caller_mgr
						,LinphoneCoreManager* callee_mgr
						,const LinphoneCallParams *caller_params
						,const LinphoneCallParams *callee_params){
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params =  {0};
	caller_test_params.base = (LinphoneCallParams*)caller_params;
	callee_test_params.base = (LinphoneCallParams*)caller_params;
	return call_with_params2(caller_mgr,callee_mgr,&caller_test_params,&callee_test_params,FALSE);
}

bool_t call_with_test_params(LinphoneCoreManager* caller_mgr
				,LinphoneCoreManager* callee_mgr
				,const LinphoneCallTestParams *caller_test_params
				,const LinphoneCallTestParams *callee_test_params){
	return call_with_params2(caller_mgr,callee_mgr,caller_test_params,callee_test_params,FALSE);
}

bool_t call_with_caller_params(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr, const LinphoneCallParams *params) {
	return call_with_params(caller_mgr,callee_mgr,params,NULL);
}

bool_t call(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr){
	return call_with_params(caller_mgr,callee_mgr,NULL,NULL);
}

void end_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2){
	linphone_core_terminate_all_calls(m1->lc);
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m1->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m2->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m1->stat.number_of_LinphoneCallReleased,1));
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m2->stat.number_of_LinphoneCallReleased,1));
}

void simple_call_base(bool_t enable_multicast_recv_side) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	const LinphoneAddress *from;
	LinphoneCall *pauline_call;
	LinphoneProxyConfig* marie_cfg;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/* with the account manager, we might lose the identity */
	marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	{
		LinphoneAddress* marie_addr = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_cfg));
		char* marie_tmp_id = NULL;
		linphone_address_set_display_name(marie_addr, "Super Marie");
		marie_tmp_id = linphone_address_as_string(marie_addr);

		linphone_proxy_config_edit(marie_cfg);
		linphone_proxy_config_set_identity(marie_cfg,marie_tmp_id);
		linphone_proxy_config_done(marie_cfg);

		ms_free(marie_tmp_id);
		linphone_address_destroy(marie_addr);
	}

	linphone_core_enable_audio_multicast(pauline->lc,enable_multicast_recv_side);

	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_call=linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	/*check that display name is correctly propagated in From */
	if (pauline_call){
		from=linphone_call_get_remote_address(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_PTR_NOT_NULL(from);
		if (from){
			const char *dname=linphone_address_get_display_name(from);
			BC_ASSERT_PTR_NOT_NULL(dname);
			if (dname){
				BC_ASSERT_STRING_EQUAL(dname, "Super Marie");
			}
		}
	}


	liblinphone_tester_check_rtcp(marie,pauline);
	end_call(marie,pauline);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void simple_call() {
	simple_call_base(FALSE);
}

static void call_with_timeouted_bye(void) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	belle_sip_timer_config_t timer_config;
	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	BC_ASSERT_TRUE(call(marie,pauline));

	sal_set_send_error(pauline->lc->sal,1500); /*to trash the message without generating error*/
	timer_config.T1=50; /*to have timer F = 3s*/
	timer_config.T2=4000;
	timer_config.T3=0;
	timer_config.T4=5000;

	belle_sip_stack_set_timer_config(sal_get_belle_sip_stack(pauline->lc->sal),&timer_config);
	linphone_core_terminate_all_calls(pauline->lc);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1,timer_config.T1*84));

	sal_set_send_error(pauline->lc->sal,0);

	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1,5000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallReleased,1,5000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void direct_call_over_ipv6(){
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	if (liblinphone_tester_ipv6_available()){
		LCSipTransports pauline_transports;
		LinphoneAddress* pauline_dest = linphone_address_new("sip:[::1];transport=tcp");
		marie = linphone_core_manager_new( "marie_rc");
		pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

		linphone_core_enable_ipv6(marie->lc,TRUE);
		linphone_core_enable_ipv6(pauline->lc,TRUE);
		linphone_core_set_default_proxy_config(marie->lc,NULL);
		/*wait for register in v6 mode, however sip2.linphone.org has an ipv6 address but doesn't listen to it*/
#if 0
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 2, 2000));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2, 2000));
#endif

		linphone_core_get_sip_transports_used(pauline->lc,&pauline_transports);
		linphone_address_set_port(pauline_dest,pauline_transports.tcp_port);
		linphone_core_invite_address(marie->lc,pauline_dest);

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallOutgoingRinging,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallIncomingReceived,1));
		linphone_core_accept_call(pauline->lc,linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,1));

		liblinphone_tester_check_rtcp(marie,pauline);
		end_call(marie,pauline);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		linphone_address_destroy(pauline_dest);
	}else ms_warning("Test skipped, no ipv6 available");
}

static void call_outbound_with_multiple_proxy() {
	LinphoneCoreManager* marie   = linphone_core_manager_new2( "marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2( "pauline_tcp_rc", FALSE);

	LinphoneProxyConfig* lpc = NULL;
	LinphoneProxyConfig* registered_lpc = linphone_proxy_config_new();

	linphone_core_get_default_proxy(marie->lc, &lpc);
	linphone_core_set_default_proxy(marie->lc,NULL);

	BC_ASSERT_FATAL(lpc != NULL);
	BC_ASSERT_FATAL(registered_lpc != NULL);

	// create new LPC that will successfully register
	linphone_proxy_config_set_identity(registered_lpc, linphone_proxy_config_get_identity(lpc));
	linphone_proxy_config_set_server_addr(registered_lpc, linphone_proxy_config_get_addr(lpc));
	linphone_proxy_config_set_route(registered_lpc, linphone_proxy_config_get_route(lpc));
	linphone_proxy_config_enable_register(registered_lpc, TRUE);

	linphone_core_add_proxy_config(marie->lc, registered_lpc);

	// set first LPC to unreacheable proxy addr
	linphone_proxy_config_edit(lpc);
	linphone_proxy_config_set_server_addr(lpc,"12.13.14.15:5223;transport=udp");
	linphone_proxy_config_set_route(lpc, "12.13.14.15:5223;transport=udp;lr");
	linphone_proxy_config_done(lpc);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 2000));

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationProgress, 2, 200));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1, 2000));

	// calling marie should go through the second proxy config
	BC_ASSERT_TRUE(call(marie, pauline));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#if 0 /* TODO: activate test when the implementation is ready */
static void multiple_answers_call() {
	/* Scenario is this: pauline calls marie, which is registered 2 times.
	   Both linphones answer at the same time, and only one should get the
	   call running, the other should be terminated */
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_rc" );
	LinphoneCoreManager* marie1  = linphone_core_manager_new( "marie_rc" );
	LinphoneCoreManager* marie2  = linphone_core_manager_new( "marie_rc" );

	LinphoneCall* call1, *call2;

	MSList* lcs = ms_list_append(NULL,pauline->lc);
	lcs = ms_list_append(lcs,marie1->lc);
	lcs = ms_list_append(lcs,marie2->lc);


	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 2000));

	BC_ASSERT_PTR_NOT_NULL( linphone_core_invite_address(pauline->lc, marie1->identity ) );

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie1->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));

	// marie 1 and 2 answer at the same time
	call1 = linphone_core_get_current_call(marie1->lc);
	call2 = linphone_core_get_current_call(marie2->lc);

	BC_ASSERT_PTR_NOT_NULL_FATAL(call1);
	BC_ASSERT_PTR_NOT_NULL_FATAL(call2);

	BC_ASSERT_EQUAL( linphone_core_accept_call(marie1->lc, call1), 0, int, "%d");
	BC_ASSERT_EQUAL( linphone_core_accept_call(marie2->lc, call2), 0, int, "%d");

	BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 2000) );
	BC_ASSERT_TRUE( wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallStreamsRunning, 1, 2000) );
	BC_ASSERT_TRUE( wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallEnd, 1, 2000) );


	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
}
#endif

static void multiple_answers_call_with_media_relay(void) {

	/* Scenario is this: pauline calls marie, which is registered 2 times.
	 *   Both linphones answer at the same time, and only one should get the
	 *   call running, the other should be terminated */
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc" );
	LinphoneCoreManager* marie1  = linphone_core_manager_new( "marie_rc" );
	LinphoneCoreManager* marie2  = linphone_core_manager_new( "marie_rc" );

	LinphoneCall* call1, *call2;

	MSList* lcs = ms_list_append(NULL,pauline->lc);
	lcs = ms_list_append(lcs,marie1->lc);
	lcs = ms_list_append(lcs,marie2->lc);

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie1->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie2->lc, "Natted Linphone", NULL);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 2000));

	BC_ASSERT_PTR_NOT_NULL( linphone_core_invite_address(pauline->lc, marie1->identity ) );

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie1->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));

	// marie 1 and 2 answer at the same time
	call1 = linphone_core_get_current_call(marie1->lc);
	call2 = linphone_core_get_current_call(marie2->lc);

	BC_ASSERT_PTR_NOT_NULL_FATAL(call1);
	BC_ASSERT_PTR_NOT_NULL_FATAL(call2);

	BC_ASSERT_EQUAL( linphone_core_accept_call(marie1->lc, call1), 0, int, "%d");
	BC_ASSERT_EQUAL( linphone_core_accept_call(marie2->lc, call2), 0, int, "%d");

	BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 2000) );
	BC_ASSERT_TRUE( wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallStreamsRunning, 1, 2000) );
	BC_ASSERT_TRUE( wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallEnd, 1, 2000) );


	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
}

static void call_with_specified_codec_bitrate(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	const char * codec = "opus";
	int rate = 48000;
	int min_bw=24;
	int max_bw=50;

#ifdef __arm__
	if (ms_get_cpu_count() <2) { /*2 opus codec channel + resampler is too much for a single core*/
#ifndef ANDROID
		codec = "speex";
		rate = 8000;
		min_bw=20;
		max_bw=35;
#else
		BC_PASS("Test requires at least a dual core");
		goto end;
#endif
	}
#endif
	/*Force marie to play from file: if soundcard is used and it is silient, then vbr mode will drop down the bitrate
	 Note that a play file is already set by linphone_core_manager_new() (but not used)*/
	linphone_core_set_use_files(marie->lc, TRUE);

	if (linphone_core_find_payload_type(marie->lc,codec,rate,-1)==NULL){
		BC_PASS("opus codec not supported, test skipped.");
		goto end;
	}

	disable_all_audio_codecs_except_one(marie->lc,codec,rate);
	disable_all_audio_codecs_except_one(pauline->lc,codec,rate);

	linphone_core_set_payload_type_bitrate(marie->lc,
		linphone_core_find_payload_type(marie->lc,codec,rate,-1),
		max_bw);
	linphone_core_set_payload_type_bitrate(pauline->lc,
		linphone_core_find_payload_type(pauline->lc,codec,rate,-1),
		min_bw);

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(marie,pauline);
	/*wait a bit that bitstreams are stabilized*/
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(marie), min_bw+5+min_bw*.1, int, "%i");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie), 10, int, "%i"); /*check that at least something is received */
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(pauline), (max_bw-5-max_bw*.1), int, "%i");
	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void simple_call_compatibility_mode(void) {
	char route[256];
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCore* lc_marie=marie->lc;
	LinphoneCore* lc_pauline=pauline->lc;
	stats* stat_marie=&marie->stat;
	stats* stat_pauline=&pauline->stat;
	LinphoneProxyConfig* proxy;
	const LinphoneAddress* identity;
	LinphoneAddress* proxy_address;
	char*tmp;
	LCSipTransports transport;

	linphone_core_get_default_proxy(lc_marie,&proxy);
	BC_ASSERT_PTR_NOT_NULL (proxy);
	identity = linphone_proxy_config_get_identity_address(proxy);


	proxy_address=linphone_address_new(linphone_proxy_config_get_addr(proxy));
	linphone_address_clean(proxy_address);
	tmp=linphone_address_as_string_uri_only(proxy_address);
	linphone_proxy_config_set_server_addr(proxy,tmp);
	sprintf(route,"sip:%s",test_route);
	linphone_proxy_config_set_route(proxy,route);
	ms_free(tmp);
	linphone_address_destroy(proxy_address);
	linphone_core_get_sip_transports(lc_marie,&transport);
	transport.udp_port=0;
	transport.tls_port=0;
	transport.dtls_port=0;
	/*only keep tcp*/
	linphone_core_set_sip_transports(lc_marie,&transport);
	stat_marie->number_of_LinphoneRegistrationOk=0;

	BC_ASSERT_TRUE (wait_for(lc_marie,lc_marie,&stat_marie->number_of_LinphoneRegistrationOk,1));

	linphone_core_invite_address(lc_marie,pauline->identity);

	BC_ASSERT_TRUE (wait_for(lc_pauline,lc_marie,&stat_pauline->number_of_LinphoneCallIncomingReceived,1));
	BC_ASSERT_TRUE(linphone_core_inc_invite_pending(lc_pauline));
	BC_ASSERT_EQUAL(stat_marie->number_of_LinphoneCallOutgoingProgress,1, int, "%d");
	BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_marie->number_of_LinphoneCallOutgoingRinging,1));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call_remote_address(lc_pauline));
	if (linphone_core_get_current_call_remote_address(lc_pauline)) {
		BC_ASSERT_TRUE(linphone_address_weak_equal(identity,linphone_core_get_current_call_remote_address(lc_pauline)));

		linphone_core_accept_call(lc_pauline,linphone_core_get_current_call(lc_pauline));

		BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_pauline->number_of_LinphoneCallConnected,1));
		BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_marie->number_of_LinphoneCallConnected,1));
		BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_pauline->number_of_LinphoneCallStreamsRunning,1));
		BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_marie->number_of_LinphoneCallStreamsRunning,1));

		wait_for(lc_pauline,lc_marie,&stat_marie->number_of_LinphoneCallStreamsRunning,3);
		linphone_core_terminate_all_calls(lc_pauline);
		BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_pauline->number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(lc_pauline,lc_marie,&stat_marie->number_of_LinphoneCallEnd,1));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static void cancelled_call(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall* out_call = linphone_core_invite_address(pauline->lc,marie->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallOutgoingInit,1));

	linphone_core_terminate_call(pauline->lc,out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	//BC_ASSERT_EQUAL(linphone_call_get_reason(out_call),LinphoneReasonCanceled, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingReceived,0, int, "%d");
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1));

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void disable_all_audio_codecs_except_one(LinphoneCore *lc, const char *mime, int rate){
	const MSList *elem=linphone_core_get_audio_codecs(lc);
	PayloadType *pt;

	for(;elem!=NULL;elem=elem->next){
		pt=(PayloadType*)elem->data;
		linphone_core_enable_payload_type(lc,pt,FALSE);
	}
	pt=linphone_core_find_payload_type(lc,mime,rate,-1);
	BC_ASSERT_PTR_NOT_NULL_FATAL(pt);
	linphone_core_enable_payload_type(lc,pt,TRUE);
}

#ifdef VIDEO_ENABLED
void disable_all_video_codecs_except_one(LinphoneCore *lc, const char *mime) {
	const MSList *codecs = linphone_core_get_video_codecs(lc);
	const MSList *it = NULL;
	PayloadType *pt = NULL;

	for(it = codecs; it != NULL; it = it->next) {
		linphone_core_enable_payload_type(lc, (PayloadType *)it->data, FALSE);
	}
	pt = linphone_core_find_payload_type(lc, mime, -1, -1);
	BC_ASSERT_PTR_NOT_NULL_FATAL(pt);
	linphone_core_enable_payload_type(lc, pt, TRUE);
}
#endif

static void call_with_dns_time_out(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new2( "empty_rc", FALSE);
	LCSipTransports transport = {9773,0,0,0};
	int i;

	linphone_core_set_sip_transports(marie->lc,&transport);
	linphone_core_iterate(marie->lc);
	sal_set_dns_timeout(marie->lc->sal,0);
	linphone_core_invite(marie->lc,"\"t\x8et\x8e\" <sip:toto@toto.com>"); /*just to use non ascii values*/
	for(i=0;i<10;i++){
		ms_usleep(200000);
		linphone_core_iterate(marie->lc);
	}
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingInit,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingProgress,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallError,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallReleased,1, int, "%d");
	linphone_core_manager_destroy(marie);
}

static void early_cancelled_call(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new2( "empty_rc",FALSE);

	LinphoneCall* out_call = linphone_core_invite_address(pauline->lc,marie->identity);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallOutgoingInit,1));
	linphone_core_terminate_call(pauline->lc,out_call);

	/*since everything is executed in a row, no response can be received from the server, thus the CANCEL cannot be sent.
	 It will ring at Marie's side.*/

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd,1, int, "%d");

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallIncomingReceived,1));
	/* now the CANCEL should have been sent and the the call at marie's side should terminate*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void cancelled_ringing_call(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall* out_call = linphone_core_invite_address(pauline->lc,marie->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallIncomingReceived,1));

	linphone_core_terminate_call(pauline->lc,out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallReleased,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd,1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd,1, int, "%d");

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_declined_call(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallLog* out_call_log;
	LinphoneCall* out_call;

	linphone_core_set_max_calls(marie->lc,0);
	out_call = linphone_core_invite_address(pauline->lc,marie->identity);
	linphone_call_ref(out_call);

	/*wait until flexisip transfers the busy...*/
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallError,1,33000));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallError,1, int, "%d");
	/* FIXME http://git.linphone.org/mantis/view.php?id=757

	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call),LinphoneReasonBusy, int, "%d");
	 */
	if (ms_list_size(linphone_core_get_call_logs(pauline->lc))>0) {
		BC_ASSERT_PTR_NOT_NULL(out_call_log=(LinphoneCallLog*)(linphone_core_get_call_logs(pauline->lc)->data));
		BC_ASSERT_EQUAL(linphone_call_log_get_status(out_call_log),LinphoneCallAborted, int, "%d");
	}
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_busy_when_calling_self(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCall *out_call=linphone_core_invite_address(marie->lc,marie->identity);
	linphone_call_ref(out_call);

	/*wait until flexisip transfers the busy...*/
	BC_ASSERT_TRUE(wait_for_until(marie->lc,marie->lc,&marie->stat.number_of_LinphoneCallError,1,33000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallError,1, int, "%d");

	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call),LinphoneReasonBusy, int, "%d");
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
}


static void call_declined(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	LinphoneCall* in_call;
	LinphoneCall* out_call = linphone_core_invite_address(pauline->lc,marie->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallIncomingReceived,1));
	BC_ASSERT_PTR_NOT_NULL(in_call=linphone_core_get_current_call(marie->lc));
	if (in_call) {
		linphone_call_ref(in_call);
		linphone_core_terminate_call(marie->lc,in_call);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallReleased,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1));
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd,1, int, "%d");
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallEnd,1, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(in_call),LinphoneReasonDeclined, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(out_call),LinphoneReasonDeclined, int, "%d");
		linphone_call_unref(in_call);
	}
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_terminated_by_caller(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	BC_ASSERT_TRUE(call(pauline,marie));

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_sdp_200_ack(marie->lc,TRUE);

	BC_ASSERT_TRUE(call(marie,pauline));

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_no_sdp_ack_without_sdp(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call;

	linphone_core_enable_sdp_200_ack(marie->lc,TRUE);

	linphone_core_invite_address(marie->lc,pauline->identity);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallIncomingReceived,1));
	call=linphone_core_get_current_call(pauline->lc);
	if (call){
		sal_call_set_sdp_handling(call->op, SalOpSDPSimulateError); /*this will have the effect that the SDP received in the ACK will be ignored*/
		linphone_core_accept_call(pauline->lc, call);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallError,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static bool_t check_ice(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneIceState state) {
	LinphoneCall *c1,*c2;
	bool_t audio_success=FALSE;
	bool_t video_success=FALSE;
	bool_t video_enabled;
	MSTimeSpec ts;

	c1=linphone_core_get_current_call(caller->lc);
	c2=linphone_core_get_current_call(callee->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (!c1 || !c2) return FALSE;
	linphone_call_ref(c1);
	linphone_call_ref(c2);

	BC_ASSERT_EQUAL(linphone_call_params_video_enabled(linphone_call_get_current_params(c1)),linphone_call_params_video_enabled(linphone_call_get_current_params(c2)), int, "%d");
	video_enabled=linphone_call_params_video_enabled(linphone_call_get_current_params(c1));
	liblinphone_tester_clock_start(&ts);
	do{
		if ((c1 != NULL) && (c2 != NULL)) {
			if (linphone_call_get_audio_stats(c1)->ice_state==state &&
				linphone_call_get_audio_stats(c2)->ice_state==state ){
				audio_success=TRUE;
				break;
			}
			linphone_core_iterate(caller->lc);
			linphone_core_iterate(callee->lc);
		}
		ms_usleep(20000);
	}while(!liblinphone_tester_clock_elapsed(&ts,10000));

	if (video_enabled){
		liblinphone_tester_clock_start(&ts);
		do{
			if ((c1 != NULL) && (c2 != NULL)) {
				if (linphone_call_get_video_stats(c1)->ice_state==state &&
					linphone_call_get_video_stats(c2)->ice_state==state ){
					video_success=TRUE;
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
			}
			ms_usleep(20000);
		}while(!liblinphone_tester_clock_elapsed(&ts,10000));
	}

	 /*make sure encryption mode are preserved*/
	if (c1) {
		const LinphoneCallParams* call_param = linphone_call_get_current_params(c1);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller->lc), int, "%d");
	}
	if (c2) {
		const LinphoneCallParams* call_param = linphone_call_get_current_params(c2);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(callee->lc), int, "%d");
	}
	linphone_call_unref(c1);
	linphone_call_unref(c2);
	return video_enabled ? audio_success && video_success : audio_success;
}

static void _call_with_ice_base(LinphoneCoreManager* pauline,LinphoneCoreManager* marie, bool_t caller_with_ice, bool_t callee_with_ice, bool_t random_ports) {
	if (callee_with_ice){
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
	}
	if (caller_with_ice){
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
	}

	if (random_ports){
		linphone_core_set_audio_port(marie->lc,-1);
		linphone_core_set_video_port(marie->lc,-1);
		linphone_core_set_audio_port(pauline->lc,-1);
		linphone_core_set_video_port(pauline->lc,-1);
	}


	if (!BC_ASSERT_TRUE(call(pauline,marie)))
		goto end;

	if (callee_with_ice && caller_with_ice) {
		/*wait for the ICE reINVITE to complete*/
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));

		BC_ASSERT_TRUE(check_ice(pauline,marie,LinphoneIceStateHostConnection));
	}

	liblinphone_tester_check_rtcp(marie,pauline);
	/*then close the call*/
	linphone_core_terminate_all_calls(pauline->lc);
end:
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

}
static void _call_with_ice(bool_t caller_with_ice, bool_t callee_with_ice, bool_t random_ports) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	_call_with_ice_base(pauline,marie,caller_with_ice,callee_with_ice,random_ports);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_with_ice(void){
	_call_with_ice(TRUE,TRUE,FALSE);
}

/*ICE is not expected to work in this case, however this should not crash*/
static void call_with_ice_no_sdp(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_sdp_200_ack(pauline->lc,TRUE);

	linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);

	linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);

	call(pauline,marie);

	liblinphone_tester_check_rtcp(marie,pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_random_ports(void){
	_call_with_ice(TRUE,TRUE,TRUE);
}

static void ice_to_not_ice(void){
	_call_with_ice(TRUE,FALSE,FALSE);
}

static void not_ice_to_ice(void){
	_call_with_ice(FALSE,TRUE,FALSE);
}

static void call_with_custom_headers(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_marie,*call_pauline;
	LinphoneCallParams *params;
	const LinphoneCallParams *marie_remote_params;
	const char *hvalue;
	char	*pauline_remote_contact_header,
				*pauline_remote_contact,
				*marie_remote_contact,
				*marie_remote_contact_header;
	LinphoneAddress* marie_identity;
	char* tmp=linphone_address_as_string_uri_only(marie->identity);
	char tmp2[256];
	snprintf(tmp2,sizeof(tmp2),"%s?uriHeader=myUriHeader",tmp);
	marie_identity=linphone_address_new(tmp2);
	ms_free(tmp);
	linphone_address_destroy(marie->identity);
	marie->identity=marie_identity;

	params=linphone_core_create_default_call_parameters(marie->lc);
	linphone_call_params_add_custom_header(params,"Weather","bad");
	linphone_call_params_add_custom_header(params,"Working","yes");

	BC_ASSERT_TRUE(call_with_caller_params(pauline,marie,params));
	linphone_call_params_destroy(params);

	call_marie=linphone_core_get_current_call(marie->lc);
	call_pauline=linphone_core_get_current_call(pauline->lc);

	BC_ASSERT_PTR_NOT_NULL(call_marie);
	BC_ASSERT_PTR_NOT_NULL(call_pauline);

	marie_remote_params=linphone_call_get_remote_params(call_marie);
	hvalue=linphone_call_params_get_custom_header(marie_remote_params,"Weather");
	BC_ASSERT_PTR_NOT_NULL(hvalue);
	BC_ASSERT_STRING_EQUAL(hvalue,"bad");
	hvalue=linphone_call_params_get_custom_header(marie_remote_params,"uriHeader");
	BC_ASSERT_PTR_NOT_NULL(hvalue);
	BC_ASSERT_STRING_EQUAL(hvalue,"myUriHeader");


	// FIXME: we have to strdup because successive calls to get_remote_params erase the returned const char*!!
	pauline_remote_contact        = ms_strdup(linphone_call_get_remote_contact(call_pauline));
	pauline_remote_contact_header = ms_strdup(linphone_call_params_get_custom_header(linphone_call_get_remote_params(call_pauline), "Contact"));

	marie_remote_contact        = ms_strdup(linphone_call_get_remote_contact(call_marie));
	marie_remote_contact_header = ms_strdup(linphone_call_params_get_custom_header(linphone_call_get_remote_params(call_marie), "Contact"));

	BC_ASSERT_PTR_NOT_NULL(pauline_remote_contact);
	BC_ASSERT_PTR_NOT_NULL(pauline_remote_contact_header);
	BC_ASSERT_PTR_NOT_NULL(marie_remote_contact);
	BC_ASSERT_PTR_NOT_NULL(marie_remote_contact_header);
	BC_ASSERT_STRING_EQUAL(pauline_remote_contact,pauline_remote_contact_header);
	BC_ASSERT_STRING_EQUAL(marie_remote_contact,marie_remote_contact_header);

	ms_free(pauline_remote_contact);
	ms_free(pauline_remote_contact_header);
	ms_free(marie_remote_contact);
	ms_free(marie_remote_contact_header);


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_paused_resumed_base(bool_t multicast) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall* call_pauline;
	const rtp_stats_t * stats;
	bool_t call_ok;

	linphone_core_enable_audio_multicast(pauline->lc,multicast);

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));

	if (!call_ok) goto end;

	call_pauline = linphone_core_get_current_call(pauline->lc);

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

	linphone_core_pause_call(pauline->lc,call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPausing,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausedByRemote,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPaused,1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_core_resume_call(pauline->lc,call_pauline);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	stats = rtp_session_get_stats(call_pauline->sessions->rtp_session);
	BC_ASSERT_EQUAL(stats->cum_packet_loss, 0, int, "%d");

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_paused_resumed(void) {
	call_paused_resumed_base(FALSE);
}
#define CHECK_CURRENT_LOSS_RATE() \
	rtcp_count_current = pauline->stat.number_of_rtcp_sent; \
	/*wait for an RTCP packet to have an accurate cumulative lost value*/ \
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_rtcp_sent, rtcp_count_current+1, 10000)); \
	stats = rtp_session_get_stats(call_pauline->audiostream->ms.sessions.rtp_session); \
	loss_percentage = stats->cum_packet_loss * 100.f / (stats->packet_recv + stats->cum_packet_loss); \
	BC_ASSERT_LOWER(.75 * params.loss_rate , loss_percentage, float, "%f"); \
	BC_ASSERT_LOWER(loss_percentage , 1.25 * params.loss_rate, float, "%f")

static void call_paused_resumed_with_loss(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall* call_pauline;
	const rtp_stats_t * stats;
	float loss_percentage;
	int rtcp_count_current;

	OrtpNetworkSimulatorParams params={0};
	params.enabled=TRUE;
	params.loss_rate=20;

	BC_ASSERT_TRUE(call(pauline,marie));
	call_pauline = linphone_core_get_current_call(pauline->lc);
	if (call_pauline){
		rtp_session_enable_network_simulation(call_pauline->audiostream->ms.sessions.rtp_session,&params);

		/*generate some traffic*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 6000);
		CHECK_CURRENT_LOSS_RATE();

		/*pause call*/
		linphone_core_pause_call(pauline->lc,call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPausing,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausedByRemote,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPaused,1));
		/*stay in pause a little while in order to generate traffic*/
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);
		CHECK_CURRENT_LOSS_RATE();

		/*resume*/
		linphone_core_resume_call(pauline->lc,call_pauline);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
		wait_for_until(pauline->lc, marie->lc, NULL, 5, 6000);

		/*since stats are NOT totally reset during pause, the stats->packet_recv is computed from
		the start of call. This test ensures that the loss rate is consistent during the entire call.*/
		CHECK_CURRENT_LOSS_RATE();
		linphone_core_terminate_all_calls(marie->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

bool_t pause_call_1(LinphoneCoreManager* mgr_1,LinphoneCall* call_1,LinphoneCoreManager* mgr_2,LinphoneCall* call_2) {
	stats initial_call_stat_1=mgr_1->stat;
	stats initial_call_stat_2=mgr_2->stat;
	linphone_core_pause_call(mgr_1->lc,call_1);
	BC_ASSERT_TRUE(wait_for(mgr_1->lc,mgr_2->lc,&mgr_1->stat.number_of_LinphoneCallPausing,initial_call_stat_1.number_of_LinphoneCallPausing+1));
	BC_ASSERT_TRUE(wait_for(mgr_1->lc,mgr_2->lc,&mgr_1->stat.number_of_LinphoneCallPaused,initial_call_stat_1.number_of_LinphoneCallPaused+1));
	BC_ASSERT_TRUE(wait_for(mgr_1->lc,mgr_2->lc,&mgr_2->stat.number_of_LinphoneCallPausedByRemote,initial_call_stat_2.number_of_LinphoneCallPausedByRemote+1));
	BC_ASSERT_EQUAL(linphone_call_get_state(call_1),LinphoneCallPaused, int, "%d");
	BC_ASSERT_EQUAL(linphone_call_get_state(call_2),LinphoneCallPausedByRemote, int, "%d");
	return linphone_call_get_state(call_1) == LinphoneCallPaused && linphone_call_get_state(call_2)==LinphoneCallPausedByRemote;
}
#if 0
void concurrent_paused_resumed_base() {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall* call_pauline,call_marie;
	const rtp_stats_t * stats;


	BC_ASSERT_TRUE(call(pauline,marie));

	call_pauline = linphone_core_get_current_call(pauline->lc);
	call_marie = linphone_core_get_current_call(marie->lc);

	linphone_core_pause_call(pauline->lc,call_pauline);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPausing,1));

	linphone_core_pause_call(marie->lc,call_marie);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausedByRemote,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPaused,1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_core_resume_call(pauline->lc,call_pauline);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	stats = rtp_session_get_stats(call_pauline->sessions->rtp_session);
	BC_ASSERT_EQUAL(stats->cum_packet_loss, 0, int, "%d");


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));


	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
#endif
static void call_paused_resumed_from_callee(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall* call_marie;
	const rtp_stats_t * stats;
	bool_t call_ok;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (!call_ok) goto end;
	call_marie = linphone_core_get_current_call(marie->lc);

	linphone_core_pause_call(marie->lc,call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausing,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPausedByRemote,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPaused,1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_core_resume_call(marie->lc,call_marie);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
	/*same here: wait a while for a bit of a traffic, we need to receive a RTCP packet*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 5000);

	/*since RTCP streams are reset when call is paused/resumed, there should be no loss at all*/
	stats = rtp_session_get_stats(call_marie->sessions->rtp_session);
	BC_ASSERT_EQUAL(stats->cum_packet_loss, 0, int, "%d");

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void audio_call_with_ice_no_matching_audio_codecs(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call;

	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1), FALSE); /* Disable PCMU */
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1), TRUE); /* Enable PCMA */
	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);

	out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));

	/* flexisip will retain the 488 until the "urgent reply" timeout arrives. */
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#ifdef VIDEO_ENABLED
static LinphoneCall* setup_video(LinphoneCoreManager* caller,LinphoneCoreManager* callee, bool_t change_video_policy) {
	LinphoneVideoPolicy  caller_policy;
	LinphoneCallParams* callee_params;
	LinphoneCall* call_obj;

	if (!linphone_core_get_current_call(callee->lc) || linphone_call_get_state(linphone_core_get_current_call(callee->lc)) != LinphoneCallStreamsRunning
			|| !linphone_core_get_current_call(caller->lc) || linphone_call_get_state(linphone_core_get_current_call(caller->lc)) != LinphoneCallStreamsRunning ) {
		ms_warning("bad state for adding video");
		return NULL;
	}

	if (change_video_policy) {
		caller_policy.automatically_accept=TRUE;
		caller_policy.automatically_initiate=TRUE;
		linphone_core_set_video_policy(caller->lc,&caller_policy);
	}
	linphone_core_enable_video_capture(callee->lc, TRUE);
	linphone_core_enable_video_display(callee->lc, TRUE);
	linphone_core_enable_video_capture(caller->lc, TRUE);
	linphone_core_enable_video_display(caller->lc, FALSE);

	if ((call_obj = linphone_core_get_current_call(callee->lc))) {
		callee_params = linphone_call_params_copy(linphone_call_get_current_params(call_obj));
		/*add video*/
		linphone_call_params_enable_video(callee_params,TRUE);
		linphone_core_update_call(callee->lc,call_obj,callee_params);
	}
	return call_obj;
}

bool_t add_video(LinphoneCoreManager* caller,LinphoneCoreManager* callee, bool_t change_video_policy) {
	stats initial_caller_stat=caller->stat;
	stats initial_callee_stat=callee->stat;
	const LinphoneVideoPolicy *video_policy;
	LinphoneCall *call_obj;
	if ((call_obj=setup_video(caller, callee, change_video_policy))){
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&caller->stat.number_of_LinphoneCallUpdatedByRemote,initial_caller_stat.number_of_LinphoneCallUpdatedByRemote+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallUpdating,initial_callee_stat.number_of_LinphoneCallUpdating+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,initial_callee_stat.number_of_LinphoneCallStreamsRunning+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,initial_caller_stat.number_of_LinphoneCallStreamsRunning+1));

		video_policy = linphone_core_get_video_policy(caller->lc);
		if (video_policy->automatically_accept) {
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));
		} else {
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));
		}
		if (linphone_core_get_media_encryption(caller->lc) != LinphoneMediaEncryptionNone
				&& linphone_core_get_media_encryption(callee->lc) != LinphoneMediaEncryptionNone) {
			const LinphoneCallParams* call_param;

			switch (linphone_core_get_media_encryption(caller->lc)) {
			case LinphoneMediaEncryptionZRTP:
			case LinphoneMediaEncryptionDTLS:
				/*wait for encryption to be on, in case of zrtp/dtls, it can take a few seconds*/
				wait_for(callee->lc,caller->lc,&caller->stat.number_of_LinphoneCallEncryptedOn,initial_caller_stat.number_of_LinphoneCallEncryptedOn+1);
			break;
			case LinphoneMediaEncryptionNone:
			case LinphoneMediaEncryptionSRTP:
				break;
			}
			switch (linphone_core_get_media_encryption(callee->lc)) {
			case LinphoneMediaEncryptionZRTP:
			case LinphoneMediaEncryptionDTLS:
				wait_for(callee->lc,caller->lc,&callee->stat.number_of_LinphoneCallEncryptedOn,initial_callee_stat.number_of_LinphoneCallEncryptedOn+1);
			break;
			case LinphoneMediaEncryptionNone:
			case LinphoneMediaEncryptionSRTP:
				break;
			}

			call_param = linphone_call_get_current_params(linphone_core_get_current_call(callee->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller->lc), int, "%d");
			call_param = linphone_call_get_current_params(linphone_core_get_current_call(caller->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller->lc), int, "%d");

		}

		if (video_policy->automatically_accept) {
			linphone_call_set_next_video_frame_decoded_callback(call_obj,linphone_call_cb,callee->lc);
			/*send vfu*/
			linphone_call_send_vfu_request(call_obj);
			return wait_for(caller->lc,callee->lc,&callee->stat.number_of_IframeDecoded,initial_callee_stat.number_of_IframeDecoded+1);
		} else {
			return TRUE;
		}
	}
	return FALSE;
}

static bool_t remove_video(LinphoneCoreManager *caller, LinphoneCoreManager *callee) {
	LinphoneCallParams *callee_params;
	LinphoneCall *call_obj;
	stats initial_caller_stat = caller->stat;
	stats initial_callee_stat = callee->stat;

	if (!linphone_core_get_current_call(callee->lc)
		|| (linphone_call_get_state(linphone_core_get_current_call(callee->lc)) != LinphoneCallStreamsRunning)
		|| !linphone_core_get_current_call(caller->lc)
		|| (linphone_call_get_state(linphone_core_get_current_call(caller->lc)) != LinphoneCallStreamsRunning)) {
		ms_warning("bad state for removing video");
		return FALSE;
	}

	if ((call_obj = linphone_core_get_current_call(callee->lc))) {
		callee_params = linphone_call_params_copy(linphone_call_get_current_params(call_obj));

		/* Remove video. */
		linphone_call_params_enable_video(callee_params, FALSE);
		linphone_core_update_call(callee->lc, call_obj, callee_params);

		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallUpdatedByRemote, initial_caller_stat.number_of_LinphoneCallUpdatedByRemote + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallUpdating, initial_callee_stat.number_of_LinphoneCallUpdating + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &callee->stat.number_of_LinphoneCallStreamsRunning, initial_callee_stat.number_of_LinphoneCallStreamsRunning + 1));
		BC_ASSERT_TRUE(wait_for(caller->lc, callee->lc, &caller->stat.number_of_LinphoneCallStreamsRunning, initial_caller_stat.number_of_LinphoneCallStreamsRunning + 1));

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));

		return TRUE;
	}
	return FALSE;
}

static void call_with_video_added(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_video_added_2(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	/*in this variant marie is already in automatically accept*/
	LinphoneVideoPolicy  marie_policy;
	marie_policy.automatically_accept=TRUE;


	linphone_core_set_video_policy(marie->lc,&marie_policy);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, FALSE);

	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(add_video(marie,pauline, TRUE));

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_video_added_random_ports(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	linphone_core_set_audio_port(marie->lc,-1);
	linphone_core_set_video_port(marie->lc,-1);
	linphone_core_set_audio_port(pauline->lc,-1);
	linphone_core_set_video_port(pauline->lc,-1);

	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_several_video_switches(void) {
	int dummy = 0;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;
	BC_ASSERT_TRUE(call_ok=call(pauline,marie));

	if (!call_ok) goto end;

	BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));
	wait_for_until(pauline->lc,marie->lc,&dummy,1,1000); /* Wait for VFU request exchanges to be finished. */
	BC_ASSERT_TRUE(remove_video(pauline,marie));
	BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));
	wait_for_until(pauline->lc,marie->lc,&dummy,1,1000); /* Wait for VFU request exchanges to be finished. */
	BC_ASSERT_TRUE(remove_video(pauline,marie));
	/**/
	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_several_video_switches(void) {
	int dummy = 0;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	if (linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionSRTP)) {
		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

		BC_ASSERT_TRUE(call_ok=call(pauline,marie));
		if (!call_ok) goto end;

		BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));
		wait_for_until(pauline->lc,marie->lc,&dummy,1,1000); /* Wait for VFU request exchanges to be finished. */
		BC_ASSERT_TRUE(remove_video(pauline,marie));
		BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));
		wait_for_until(pauline->lc,marie->lc,&dummy,1,1000); /* Wait for VFU request exchanges to be finished. */
		BC_ASSERT_TRUE(remove_video(pauline,marie));
		/**/
		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	} else {
		ms_warning("Not tested because SRTP is not available.");
	}
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_declined_video_base(bool_t using_policy) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall* marie_call;
	LinphoneCall* pauline_call;
	LinphoneVideoPolicy marie_policy, pauline_policy;
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	bool_t call_ok;

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);

	if (using_policy) {
		pauline_policy.automatically_initiate=TRUE;
		pauline_policy.automatically_accept=FALSE;
		marie_policy.automatically_initiate=FALSE;
		marie_policy.automatically_accept=FALSE;

		linphone_core_set_video_policy(marie->lc,&marie_policy);
		linphone_core_set_video_policy(pauline->lc,&pauline_policy);
	}

	caller_test_params.base=linphone_core_create_default_call_parameters(pauline->lc);
	if (!using_policy)
		linphone_call_params_enable_video(caller_test_params.base,TRUE);

	if (!using_policy){
		callee_test_params.base=linphone_core_create_default_call_parameters(marie->lc);
		linphone_call_params_enable_video(callee_test_params.base,FALSE);
	}

	BC_ASSERT_TRUE((call_ok=call_with_params2(pauline,marie,&caller_test_params,&callee_test_params,using_policy)));
	if (!call_ok) goto end;

	linphone_call_params_destroy(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_destroy(callee_test_params.base);
	marie_call=linphone_core_get_current_call(marie->lc);
	pauline_call=linphone_core_get_current_call(pauline->lc);

	BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
	BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
static void call_with_declined_video(void) {
	call_with_declined_video_base(FALSE);
}
static void call_with_declined_video_using_policy(void) {
	call_with_declined_video_base(TRUE);
}

void video_call_base_2(LinphoneCoreManager* pauline,LinphoneCoreManager* marie, bool_t using_policy,LinphoneMediaEncryption mode, bool_t callee_video_enabled, bool_t caller_video_enabled) {
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	LinphoneCall* marie_call;
	LinphoneCall* pauline_call;
	LinphoneVideoPolicy marie_policy, pauline_policy;

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);

	if (using_policy) {
		marie_policy.automatically_initiate=FALSE;
		marie_policy.automatically_accept=TRUE;
		pauline_policy.automatically_initiate=TRUE;
		pauline_policy.automatically_accept=FALSE;

		linphone_core_set_video_policy(marie->lc,&marie_policy);
		linphone_core_set_video_policy(pauline->lc,&pauline_policy);
	}
	if (callee_video_enabled) {
		linphone_core_enable_video_display(marie->lc, TRUE);
		linphone_core_enable_video_capture(marie->lc, TRUE);
	} else {
		linphone_core_enable_video_display(marie->lc, FALSE);
		linphone_core_enable_video_capture(marie->lc, FALSE);
	}
	if (caller_video_enabled) {
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_enable_video_capture(pauline->lc, TRUE);
	} else {
		linphone_core_enable_video_display(pauline->lc, FALSE);
		linphone_core_enable_video_capture(pauline->lc, FALSE);
	}

	if (mode==LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a directory to store them */
		marie->lc->user_certificates_path = bc_tester_file("certificates-marie");
		pauline->lc->user_certificates_path = bc_tester_file("certificates-pauline");
		belle_sip_mkdir(marie->lc->user_certificates_path);
		belle_sip_mkdir(pauline->lc->user_certificates_path);
	}

	linphone_core_set_media_encryption(marie->lc,mode);
	linphone_core_set_media_encryption(pauline->lc,mode);

	caller_test_params.base=linphone_core_create_default_call_parameters(pauline->lc);
	if (!using_policy)
		linphone_call_params_enable_video(caller_test_params.base,TRUE);

	if (!using_policy){
		callee_test_params.base=linphone_core_create_default_call_parameters(marie->lc);
		linphone_call_params_enable_video(callee_test_params.base,TRUE);
	}

	BC_ASSERT_TRUE(call_with_params2(pauline,marie,&caller_test_params,&callee_test_params,using_policy));
	marie_call=linphone_core_get_current_call(marie->lc);
	pauline_call=linphone_core_get_current_call(pauline->lc);

	linphone_call_params_destroy(caller_test_params.base);
	if (callee_test_params.base) linphone_call_params_destroy(callee_test_params.base);

	if (marie_call && pauline_call ) {
		if (callee_video_enabled && caller_video_enabled) {
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));

			/*check video path*/
			linphone_call_set_next_video_frame_decoded_callback(marie_call,linphone_call_cb,marie->lc);
			linphone_call_send_vfu_request(marie_call);
			BC_ASSERT_TRUE( wait_for(marie->lc,pauline->lc,&marie->stat.number_of_IframeDecoded,1));
		} else {
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(marie_call)));
			BC_ASSERT_FALSE(linphone_call_log_video_enabled(linphone_call_get_call_log(pauline_call)));
		}

		liblinphone_tester_check_rtcp(marie,pauline);

	}

}
static void video_call_base(LinphoneCoreManager* pauline,LinphoneCoreManager* marie, bool_t using_policy,LinphoneMediaEncryption mode, bool_t callee_video_enabled, bool_t caller_video_enabled) {
	video_call_base_2(pauline,marie,using_policy,mode,callee_video_enabled,caller_video_enabled);
	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
}
static void video_call(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie,pauline,FALSE,LinphoneMediaEncryptionNone,TRUE,TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_zrtp(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionZRTP)) {
		video_call_base(marie,pauline,FALSE,LinphoneMediaEncryptionZRTP,TRUE,TRUE);
	} else
		ms_message("Skipping video_call_zrtp");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_dtls(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(pauline->lc,LinphoneMediaEncryptionDTLS)) {
		video_call_base(marie,pauline,FALSE,LinphoneMediaEncryptionDTLS,TRUE,TRUE);
	} else
		ms_message("Skipping video_call_dtls");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void video_call_using_policy(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie,pauline,TRUE,LinphoneMediaEncryptionNone,TRUE,TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_using_policy_with_callee_video_disabled(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie,pauline,TRUE,LinphoneMediaEncryptionNone,FALSE,TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_using_policy_with_caller_video_disabled(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	video_call_base(marie,pauline,TRUE,LinphoneMediaEncryptionNone,TRUE,FALSE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_no_sdp(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_sdp_200_ack(pauline->lc,TRUE);
	video_call_base(pauline,marie,FALSE,LinphoneMediaEncryptionNone,TRUE,TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_video_to_novideo(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneVideoPolicy vpol={0};
	vpol.automatically_initiate=TRUE;
	linphone_core_set_video_policy(pauline->lc,&vpol);
	vpol.automatically_initiate=FALSE;
	linphone_core_set_video_policy(marie->lc,&vpol);
	_call_with_ice_base(pauline,marie,TRUE,TRUE,TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void _call_with_ice_video(LinphoneVideoPolicy caller_policy, LinphoneVideoPolicy callee_policy,
	bool_t video_added_by_caller, bool_t video_added_by_callee, bool_t video_removed_by_caller, bool_t video_removed_by_callee) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	linphone_core_set_video_policy(pauline->lc, &caller_policy);
	linphone_core_set_video_policy(marie->lc, &callee_policy);
	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);

	linphone_core_set_audio_port(marie->lc, -1);
	linphone_core_set_video_port(marie->lc, -1);
	linphone_core_set_audio_port(pauline->lc, -1);
	linphone_core_set_video_port(pauline->lc, -1);

	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;
	BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));

	/* Wait for ICE reINVITEs to complete. */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2)
		&& wait_for(pauline->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	if (video_added_by_caller) {
		BC_ASSERT_TRUE(add_video(marie, pauline, FALSE));
	} else if (video_added_by_callee) {
		BC_ASSERT_TRUE(add_video(pauline, marie, FALSE));
	}
	if (video_added_by_caller || video_added_by_callee) {
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

	if (video_removed_by_caller) {
		BC_ASSERT_TRUE(remove_video(marie, pauline));
	} else if (video_removed_by_callee) {
		BC_ASSERT_TRUE(remove_video(pauline, marie));
	}
	if (video_removed_by_caller || video_removed_by_callee) {
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	}

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_video_added(void) {
	LinphoneVideoPolicy vpol = { TRUE, TRUE };
	_call_with_ice_video(vpol, vpol, TRUE, FALSE, TRUE, FALSE);
}

static void call_with_ice_video_added_2(void) {
	LinphoneVideoPolicy vpol = { TRUE, TRUE };
	_call_with_ice_video(vpol, vpol, TRUE, FALSE, FALSE, TRUE);
}

static void call_with_ice_video_added_3(void) {
	LinphoneVideoPolicy vpol = { TRUE, TRUE };
	_call_with_ice_video(vpol, vpol, FALSE, TRUE, TRUE, FALSE);
}

static void call_with_ice_video_added_and_refused(void) {
	LinphoneVideoPolicy caller_policy = { TRUE, TRUE };
	LinphoneVideoPolicy callee_policy = { FALSE, FALSE };
	_call_with_ice_video(caller_policy, callee_policy, TRUE, FALSE, FALSE, FALSE);
}


static void video_call_with_early_media_no_matching_audio_codecs(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call, *pauline_call;
	LinphoneVideoPolicy vpol={0};

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);

	vpol.automatically_initiate=TRUE;
	vpol.automatically_accept=TRUE;
	linphone_core_set_video_policy(pauline->lc,&vpol);
	linphone_core_set_video_policy(marie->lc,&vpol);

	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1), FALSE); /* Disable PCMU */
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1), TRUE); /* Enable PCMA */

	out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!pauline_call) goto end;

	linphone_core_accept_early_media(pauline->lc, pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));
	/*audio stream shall not have been requested to start*/
	BC_ASSERT_PTR_NULL(pauline_call->audiostream->soundread);

	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(out_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));

	linphone_core_accept_call(pauline->lc, pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_core_terminate_call(marie->lc, out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

end:
	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_call_limited_bandwidth(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_download_bandwidth(pauline->lc, 100);
	video_call_base(marie,pauline,FALSE,LinphoneMediaEncryptionNone,TRUE,TRUE);


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#endif /*VIDEO_ENABLED*/

static void _call_with_media_relay(bool_t random_ports) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	if (random_ports){
		linphone_core_set_audio_port(marie->lc,-1);
		linphone_core_set_video_port(marie->lc,-1);
		linphone_core_set_audio_port(pauline->lc,-1);
		linphone_core_set_video_port(pauline->lc,-1);
	}

	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(pauline,marie);

#ifdef VIDEO_ENABLED
	BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));
	liblinphone_tester_check_rtcp(pauline,marie);
#endif

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_media_relay(void) {
	_call_with_media_relay(FALSE);
}

static void call_with_media_relay_random_ports(void) {
	_call_with_media_relay(TRUE);
}

static void call_with_privacy(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *c1,*c2;
	LinphoneCallParams *params;
	LinphoneProxyConfig* pauline_proxy;
	params=linphone_core_create_default_call_parameters(pauline->lc);
	linphone_call_params_set_privacy(params,LinphonePrivacyId);

	BC_ASSERT_TRUE(call_with_caller_params(pauline,marie,params));
	linphone_call_params_destroy(params);

	c1=linphone_core_get_current_call(pauline->lc);
	c2=linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (c1 && c2){
		/*make sure local identity is unchanged*/
		BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from(linphone_call_get_call_log(c1)),pauline->identity));

		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2),pauline->identity));

		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)),LinphonePrivacyId, int, "%d");
	}


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	/*test proxy config privacy*/
	linphone_core_get_default_proxy(pauline->lc,&pauline_proxy);
	linphone_proxy_config_set_privacy(pauline_proxy,LinphonePrivacyId);

	BC_ASSERT_TRUE(call(pauline,marie));

	c1=linphone_core_get_current_call(pauline->lc);
	c2=linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (c1 && c2){

		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2),pauline->identity));

		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)),LinphonePrivacyId, int, "%d");
	}

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,2));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,2));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*this ones makes call with privacy without previous registration*/
static void call_with_privacy2(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new2(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	LinphoneCall *c1,*c2;
	LinphoneCallParams *params;
	LinphoneProxyConfig* pauline_proxy;
	params=linphone_core_create_default_call_parameters(pauline->lc);
	linphone_call_params_set_privacy(params,LinphonePrivacyId);

	linphone_core_get_default_proxy(pauline->lc,&pauline_proxy);
	linphone_proxy_config_edit(pauline_proxy);
	linphone_proxy_config_enable_register(pauline_proxy,FALSE);
	linphone_proxy_config_done(pauline_proxy);

	BC_ASSERT_TRUE(call_with_caller_params(pauline,marie,params));
	linphone_call_params_destroy(params);

	c1=linphone_core_get_current_call(pauline->lc);
	c2=linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	if (c1 && c2){
		/*make sure local identity is unchanged*/
		BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from(linphone_call_get_call_log(c1)),pauline->identity));
		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2),pauline->identity));

		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)),LinphonePrivacyId, int, "%d");
	}


	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));

	/*test proxy config privacy*/
	linphone_proxy_config_set_privacy(pauline_proxy,LinphonePrivacyId);

	BC_ASSERT_TRUE(call(pauline,marie));
	c1=linphone_core_get_current_call(pauline->lc);
	c2=linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	if (c1 && c2){
		/*make sure remote identity is hidden*/
		BC_ASSERT_FALSE(linphone_address_weak_equal(linphone_call_get_remote_address(c2),pauline->identity));
		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(c2)),LinphonePrivacyId, int, "%d");
	}
	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,2));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,2));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call() {
	call_base(LinphoneMediaEncryptionSRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void zrtp_call() {
	call_base(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void zrtp_sas_call() {
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_b256_rc", "pauline_zrtp_b256_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_b256_rc", "pauline_tcp_rc");
}

static void zrtp_cipher_call() {
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_srtpsuite_aes256_rc", "pauline_zrtp_srtpsuite_aes256_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_aes256_rc", "pauline_zrtp_aes256_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_aes256_rc", "pauline_tcp_rc");
}

static void zrtp_video_call() {
	call_base(LinphoneMediaEncryptionZRTP,TRUE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void dtls_srtp_call() {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void dtls_srtp_call_with_media_realy() {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,TRUE,LinphonePolicyNoFirewall,FALSE);
}

static void dtls_srtp_ice_call() {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,FALSE,LinphonePolicyUseIce,FALSE);
}
#ifdef VIDEO_ENABLED
static void dtls_srtp_video_call() {
	call_base(LinphoneMediaEncryptionDTLS,TRUE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void dtls_srtp_ice_video_call() {
	call_base(LinphoneMediaEncryptionDTLS,TRUE,FALSE,LinphonePolicyUseIce,FALSE);
}
static void dtls_srtp_ice_video_call_with_relay() {
	call_base(LinphoneMediaEncryptionDTLS,TRUE,TRUE,LinphonePolicyUseIce,FALSE);
}
#endif
static void call_with_declined_srtp(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionSRTP)) {
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionSRTP);

		BC_ASSERT_TRUE(call(pauline,marie));

		linphone_core_terminate_all_calls(marie->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	} else {
		ms_warning ("not tested because srtp not available");
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_srtp_paused_and_resumed(void) {
	/*
	 * This test was made to evidence a bug due to internal usage of current_params while not yet filled by linphone_call_get_current_params().
	 * As a result it must not use the call() function because it calls linphone_call_get_current_params().
	 */
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const LinphoneCallParams *params;
	LinphoneCall *pauline_call;

	if (!linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionSRTP)) goto end;
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionSRTP);

	linphone_core_invite_address(pauline->lc, marie->identity);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallIncomingReceived,1))) goto end;
	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_core_accept_call(marie->lc, linphone_core_get_current_call(marie->lc));

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,1))) goto end;

	linphone_core_pause_call(pauline->lc, pauline_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPaused,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausedByRemote,1));

	linphone_core_resume_call(pauline->lc, pauline_call);
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2))) goto end;

	/*assert that after pause and resume, SRTP is still being used*/
	params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params) , LinphoneMediaEncryptionSRTP, int, "%d");
	params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params) , LinphoneMediaEncryptionSRTP, int, "%d");

	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void on_eof(LinphonePlayer *player, void *user_data){
	LinphoneCoreManager *marie=(LinphoneCoreManager*)user_data;
	marie->stat.number_of_player_eof++;
}

static void call_with_file_player(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recordpath = create_filepath(bc_tester_get_writable_dir_prefix(), "record-call_with_file_player", "wav");
	bool_t call_ok;
	int attempts;
	double similar=1;
	const double threshold = 0.9;

	/*this test is actually attempted three times in case of failure, because the audio comparison at the end is very sensitive to
	 * jitter buffer drifts, which sometimes happen if the machine is unable to run the test in good realtime conditions */
	for (attempts=0; attempts<3; attempts++){
		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);
		/*make sure the record file doesn't already exists, otherwise this test will append new samples to it*/
		unlink(recordpath);
		/*caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using call's player*/
		linphone_core_use_files(marie->lc,TRUE);
		linphone_core_set_play_file(marie->lc,NULL);

		/*callee is recording and plays file*/
		linphone_core_use_files(pauline->lc,TRUE);
		linphone_core_set_play_file(pauline->lc,NULL);
		linphone_core_set_record_file(pauline->lc,recordpath);

		BC_ASSERT_TRUE((call_ok=call(marie,pauline)));
		if (!call_ok) goto end;
		player=linphone_call_get_player(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_PTR_NOT_NULL(player);
		if (player){
			BC_ASSERT_EQUAL(linphone_player_open(player,hellopath,on_eof,marie),0, int, "%d");
			BC_ASSERT_EQUAL(linphone_player_start(player),0, int, "%d");
		}
		/* This assert should be modified to be at least as long as the WAV file */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_player_eof,1,10000));
		/*wait one second more for transmission to be fully ended (transmission time + jitter buffer)*/
		wait_for_until(pauline->lc,marie->lc,NULL,0,1000);

		end_call(marie, pauline);
		/*cannot run on iphone simulator because locks main loop beyond permitted time (should run
		on another thread) */
		BC_ASSERT_EQUAL(ms_audio_diff(hellopath,recordpath,&similar,audio_cmp_max_shift,NULL,NULL), 0, int, "%d");
		if (similar>=threshold)
			break;
	}
	BC_ASSERT_GREATER(similar, threshold, double, "%g");
	BC_ASSERT_LOWER(similar, 1.0, double, "%g");
	if (similar >= threshold && similar <= 1.0) {
		remove(recordpath);
	}

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(recordpath);
	ms_free(hellopath);
}

static bool_t is_format_supported(LinphoneCore *lc, const char *fmt){
	const char **formats=linphone_core_get_supported_file_formats(lc);
	for(;*formats!=NULL;++formats){
		if (strcasecmp(*formats,fmt)==0) return TRUE;
	}
	return FALSE;
}

static void call_with_mkv_file_player(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	char *hellomkv;
	char *hellowav;
	char *recordpath;
	bool_t call_ok;
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(ANDROID)
	double similar;
	const double threshold = 0.9;
#define DO_AUDIO_CMP
#endif
	hellowav = bc_tester_res("sounds/hello8000_mkv_ref.wav");
	hellomkv = bc_tester_res("sounds/hello8000.mkv");

	if (!is_format_supported(marie->lc,"mkv")){
		ms_warning("Test skipped, no mkv support.");
		goto end;
	}
	recordpath = create_filepath(bc_tester_get_writable_dir_prefix(), "record-call_with_mkv_file_player", "wav");
	/*make sure the record file doesn't already exists, otherwise this test will append new samples to it*/
	unlink(recordpath);


	/*caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using call's player*/
	linphone_core_use_files(marie->lc,TRUE);
	linphone_core_set_play_file(marie->lc,NULL);
	/*callee is recording and plays file*/
	linphone_core_use_files(pauline->lc,TRUE);
	linphone_core_set_play_file(pauline->lc,hellowav); /*just to send something but we are not testing what is sent by pauline*/
	linphone_core_set_record_file(pauline->lc,recordpath);

	BC_ASSERT_TRUE((call_ok=call(marie,pauline)));
	if (!call_ok) goto end;
	player=linphone_call_get_player(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(player);
	if (player){
		int res = linphone_player_open(player,hellomkv,on_eof,marie);
		if(!ms_filter_codec_supported("opus")) {
			BC_ASSERT_EQUAL(res, -1, int, "%d");
			goto end;
		}
		BC_ASSERT_EQUAL(res, 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_player_start(player),0,int,"%d");
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_player_eof,1,12000));
		linphone_player_close(player);
		/*wait for one second more so that last RTP packets can arrive*/
		wait_for_until(pauline->lc,marie->lc,NULL,0,1000);
	}
	end_call(marie, pauline);
#ifdef DO_AUDIO_CMP
	BC_ASSERT_EQUAL(ms_audio_diff(hellowav,recordpath,&similar,audio_cmp_max_shift,NULL,NULL),0,int,"%d");
	BC_ASSERT_GREATER(similar,threshold,double,"%f");
	BC_ASSERT_LOWER(similar,1.0,double,"%f");
	if(similar>threshold && similar<=1.0) {
		remove(recordpath);
	}
#else
	/*inter-correlation process is too much CPU consuming ending in a 20 minutes test on arm...*/
	remove(recordpath);
#endif
	ms_free(recordpath);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(hellomkv);
	ms_free(hellowav);
}

void call_base_with_configfile(LinphoneMediaEncryption mode, bool_t enable_video,bool_t enable_relay,LinphoneFirewallPolicy policy,bool_t enable_tunnel, const char *marie_rc, const char *pauline_rc) {
	LinphoneCoreManager* marie = linphone_core_manager_new(marie_rc);
	LinphoneCoreManager* pauline = linphone_core_manager_new(pauline_rc);
	bool_t call_ok;

	if (enable_relay) {
		linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
		linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);
	}
	if (enable_tunnel) {
		int i;
		LinphoneTunnelConfig * tunnel_config = linphone_tunnel_config_new();
		linphone_tunnel_config_set_host(tunnel_config,"tunnel.linphone.org");
		linphone_tunnel_config_set_port(tunnel_config,443);
		linphone_tunnel_add_server(linphone_core_get_tunnel(marie->lc),tunnel_config);
		linphone_tunnel_enable_sip(linphone_core_get_tunnel(marie->lc),FALSE);
		linphone_tunnel_set_mode(linphone_core_get_tunnel(marie->lc),LinphoneTunnelModeEnable);
		for (i=0;i<100;i++) {
			if (linphone_tunnel_connected(linphone_core_get_tunnel(marie->lc))) {
				break;
			}
			linphone_core_iterate(marie->lc);
			ms_usleep(20000);
		}
		BC_ASSERT_TRUE(linphone_tunnel_connected(linphone_core_get_tunnel(marie->lc)));

	}
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_media_encryption(marie->lc,mode);
		linphone_core_set_media_encryption(pauline->lc,mode);
		if (mode==LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a directory to store them */
			marie->lc->user_certificates_path = bc_tester_file("certificates-marie");
			pauline->lc->user_certificates_path = bc_tester_file("certificates-pauline");
			belle_sip_mkdir(marie->lc->user_certificates_path);
			belle_sip_mkdir(pauline->lc->user_certificates_path);
		}

		linphone_core_set_firewall_policy(marie->lc,policy);
		linphone_core_set_firewall_policy(pauline->lc,policy);

		BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
		if (!call_ok) goto end;
		if (linphone_core_get_media_encryption(pauline->lc) == LinphoneMediaEncryptionZRTP
			&& linphone_core_get_media_encryption(pauline->lc) == LinphoneMediaEncryptionZRTP) {
			/*wait for SAS*/
			int i;
			for (i=0;i<100;i++) {
				if (linphone_call_get_authentication_token(linphone_core_get_current_call(pauline->lc))
					&&
					linphone_call_get_authentication_token(linphone_core_get_current_call(marie->lc))) {
					/*check SAS*/
					BC_ASSERT_STRING_EQUAL(linphone_call_get_authentication_token(linphone_core_get_current_call(pauline->lc))
								,linphone_call_get_authentication_token(linphone_core_get_current_call(marie->lc)));
					liblinphone_tester_check_rtcp(pauline,marie);
					break;
				}
				linphone_core_iterate(marie->lc);
				linphone_core_iterate(pauline->lc);
				ms_usleep(20000);
			}

		}

		if (policy == LinphonePolicyUseIce){
			BC_ASSERT_TRUE(check_ice(pauline,marie,enable_tunnel?LinphoneIceStateReflexiveConnection:LinphoneIceStateHostConnection));
			wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);/*fixme to workaround a crash*/
		}
#ifdef VIDEO_ENABLED
		if (enable_video) {
			if (linphone_core_video_supported(marie->lc)) {
				add_video(pauline,marie, TRUE);
				if (policy == LinphonePolicyUseIce)
					BC_ASSERT_TRUE(check_ice(pauline,marie,enable_tunnel?LinphoneIceStateReflexiveConnection:LinphoneIceStateHostConnection));

				liblinphone_tester_check_rtcp(marie,pauline);
				/*wait for ice to found the direct path*/
				BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_IframeDecoded,1));
			} else {
				ms_warning ("not tested because video not available");
			}
		}
#endif



		linphone_core_terminate_all_calls(marie->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	} else {
		ms_warning ("not tested because %s not available", linphone_media_encryption_to_string(mode));
	}
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void call_base(LinphoneMediaEncryption mode, bool_t enable_video,bool_t enable_relay,LinphoneFirewallPolicy policy,bool_t enable_tunnel) {
	call_base_with_configfile(mode, enable_video, enable_relay, policy, enable_tunnel, "marie_rc", "pauline_tcp_rc");
}

#ifdef VIDEO_ENABLED
static void srtp_video_ice_call(void) {
	call_base(LinphoneMediaEncryptionSRTP,TRUE,FALSE,LinphonePolicyUseIce,FALSE);
}
static void zrtp_video_ice_call(void) {
	call_base(LinphoneMediaEncryptionZRTP,TRUE,FALSE,LinphonePolicyUseIce,FALSE);
}
#endif

static void srtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionSRTP,FALSE,FALSE,LinphonePolicyUseIce,FALSE);
}

static void zrtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyUseIce,FALSE);
}
static void zrtp_ice_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionZRTP,FALSE,TRUE,LinphonePolicyUseIce,FALSE);
}

static void dtls_ice_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,TRUE,LinphonePolicyUseIce,FALSE);
}

static void early_media_call(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	BC_ASSERT_TRUE(call(pauline,marie));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingEarlyMedia,1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia,1, int, "%d");

	wait_for_until(pauline->lc,marie->lc,NULL,0,1000);

	/*added because a bug related to early-media caused the Connected state to be reached two times*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallConnected,1, int, "%d");


	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));



	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_ringing(void){
	LinphoneCoreManager* marie   = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");
	MSList* lcs = NULL;
	LinphoneCall* marie_call;
	LinphoneCallLog *marie_call_log;
	uint64_t connected_time=0;
	uint64_t ended_time=0;
	int dummy=0;

	lcs = ms_list_append(lcs,marie->lc);
	lcs = ms_list_append(lcs,pauline->lc);
	/*
		Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	marie_call_log = linphone_call_get_call_log(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging,1,1000));


	if (linphone_core_inc_invite_pending(pauline->lc)) {
		/* send a 183 to initiate the early media */

		linphone_core_accept_early_media(pauline->lc, linphone_core_get_current_call(pauline->lc));

		BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia,1,2000) );
		BC_ASSERT_TRUE( wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,2000) );

		liblinphone_tester_check_rtcp(marie, pauline);

		linphone_core_accept_call(pauline->lc, linphone_core_get_current_call(pauline->lc));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1,1000));
		connected_time=ms_get_cur_time_ms();
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1,1000));

		BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));

		liblinphone_tester_check_rtcp(marie, pauline);
		/*just to have a call duration !=0*/
		wait_for_list(lcs,&dummy,1,2000);

		linphone_core_terminate_all_calls(pauline->lc);

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
		ended_time=ms_get_cur_time_ms();
		BC_ASSERT_LOWER( labs((long)((linphone_call_log_get_duration(marie_call_log)*1000) - (int64_t)(ended_time - connected_time))), 1000, long, "%ld");
		ms_list_free(lcs);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_update_base(bool_t media_change){
	LinphoneCoreManager* marie   = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	MSList* lcs = NULL;
	LinphoneCall *marie_call, *pauline_call;
	LinphoneCallParams *pauline_params;

	lcs = ms_list_append(lcs,marie->lc);
	lcs = ms_list_append(lcs,pauline->lc);
	if (media_change) {
		disable_all_audio_codecs_except_one(marie->lc,"pcmu",-1);
		disable_all_audio_codecs_except_one(pauline->lc,"pcmu",-1);
	}
	/*
		Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging,1,5000));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!pauline_call) goto end;
	/* send a 183 to initiate the early media */
	linphone_core_accept_early_media(pauline->lc, pauline_call);
	BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia,1,1000) );
	BC_ASSERT_TRUE( wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,5000) );


	pauline_params = linphone_call_params_copy(linphone_call_get_current_params(pauline_call));

	if (media_change) {
		disable_all_audio_codecs_except_one(marie->lc,"pcma",-1);
		disable_all_audio_codecs_except_one(pauline->lc,"pcma",-1);
	}
	#define UPDATED_SESSION_NAME "nouveau nom de session"

	linphone_call_params_set_session_name(pauline_params,UPDATED_SESSION_NAME);
	linphone_core_update_call(pauline->lc, pauline_call, pauline_params);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEarlyUpdating,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEarlyUpdatedByRemote,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia,1,2000));

	/*just to wait 2s*/
	liblinphone_tester_check_rtcp(marie, pauline);

	BC_ASSERT_STRING_EQUAL(	  linphone_call_params_get_session_name(linphone_call_get_remote_params(marie_call))
							, UPDATED_SESSION_NAME);

	linphone_core_accept_call(pauline->lc, linphone_core_get_current_call(pauline->lc));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1,1000));

	liblinphone_tester_check_rtcp(marie, pauline);

	linphone_core_terminate_all_calls(pauline->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));

end:

	ms_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_session_update(void){
	early_media_call_with_update_base(FALSE);
}

static void early_media_call_with_codec_update(void){
	early_media_call_with_update_base(TRUE);
}


static void check_call_state(LinphoneCoreManager* mgr,LinphoneCallState state) {
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(mgr->lc));
	if (linphone_core_get_current_call(mgr->lc))
		BC_ASSERT_EQUAL(linphone_call_get_state(linphone_core_get_current_call(mgr->lc)),state, int, "%d");
}

static void call_established_with_rejected_info(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	int dummy=0;
	bool_t call_ok=FALSE;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (call_ok){

		sal_enable_unconditional_answer(marie->lc->sal,TRUE);
		linphone_call_send_info_message(linphone_core_get_current_call(pauline->lc),linphone_core_create_info_message(pauline->lc));

		wait_for_until(marie->lc,pauline->lc,&dummy,1,1000); /*just to sleep while iterating 1s*/

		sal_enable_unconditional_answer(marie->lc->sal,FALSE);

		linphone_call_send_info_message(linphone_core_get_current_call(pauline->lc),linphone_core_create_info_message(pauline->lc));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_inforeceived,1));
		BC_ASSERT_EQUAL(marie->stat.number_of_inforeceived,1, int, "%d");

		check_call_state(pauline,LinphoneCallStreamsRunning);
		check_call_state(marie,LinphoneCallStreamsRunning);


		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_complex_rejected_operation(void) {

	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok=FALSE;
	LinphoneCallParams *params;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (call_ok){

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,1));

		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/
		linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/

		/*just to authenticate marie*/
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc),linphone_core_create_info_message(marie->lc));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_inforeceived,1));
		BC_ASSERT_EQUAL(pauline->stat.number_of_inforeceived,1, int, "%d");
		/*to give time for 200ok to arrive*/
		wait_for_until(marie->lc,pauline->lc,NULL,0,1000);



		linphone_core_update_call(	pauline->lc
									,linphone_core_get_current_call(pauline->lc)
									,linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)));


		linphone_core_update_call(	marie->lc
											,linphone_core_get_current_call(marie->lc)
											,linphone_call_get_current_params(linphone_core_get_current_call(marie->lc)));

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),LinphoneReasonTemporarilyUnavailable, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),LinphoneReasonTemporarilyUnavailable, int, "%d");

		check_call_state(pauline,LinphoneCallStreamsRunning);
		check_call_state(marie,LinphoneCallStreamsRunning);

		linphone_core_update_call(	pauline->lc
										,linphone_core_get_current_call(pauline->lc)
										,linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)));


		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc),linphone_core_create_info_message(marie->lc));


		params=linphone_core_create_call_params(marie->lc,linphone_core_get_current_call(marie->lc));
		sal_enable_pending_trans_checking(marie->lc->sal,FALSE); /*to allow // transactions*/
		linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMU",8000,1),TRUE);
		linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMA",8000,1),FALSE);

		linphone_core_update_call(	marie->lc
											,linphone_core_get_current_call(marie->lc)
											,params);

		linphone_call_params_destroy(params);

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,3));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,3));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),LinphoneReasonTemporarilyUnavailable, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),LinphoneReasonTemporarilyUnavailable, int, "%d");



		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_info_during_reinvite(void) {

	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok=FALSE;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (call_ok){

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,1));

		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/
		linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/

		/*just to authenticate marie*/
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc),linphone_core_create_info_message(marie->lc));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_inforeceived,1));
		BC_ASSERT_EQUAL(pauline->stat.number_of_inforeceived,1, int, "%d");
		/*to give time for 200ok to arrive*/
		wait_for_until(marie->lc,pauline->lc,NULL,0,1000);


		//sal_enable_pending_trans_checking(marie->lc->sal,FALSE); /*to allow // transactions*/

		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc),linphone_core_create_info_message(marie->lc));

		//sal_set_send_error(marie->lc->sal, -1); /*to avoid 491 pending to be sent*/

		linphone_core_update_call(	pauline->lc
									,linphone_core_get_current_call(pauline->lc)
									,linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)));



		wait_for_until(pauline->lc,pauline->lc,NULL,0,2000); /*to avoid 491 pending to be sent to early*/


		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));


		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static void call_established_with_rejected_reinvite(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok=FALSE;

	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (call_ok){
		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/


		linphone_core_update_call(	pauline->lc
									,linphone_core_get_current_call(pauline->lc)
									,linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)));


		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),LinphoneReasonNotAcceptable, int, "%d");

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,1, int, "%d");
		check_call_state(pauline,LinphoneCallStreamsRunning);
		check_call_state(marie,LinphoneCallStreamsRunning);


		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_incoming_reinvite(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok=FALSE;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));

	if (call_ok){

		/*wait for ACK to be transmitted before going to reINVITE*/
		wait_for_until(marie->lc,pauline->lc,NULL,0,1000);

		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/

		linphone_core_update_call(marie->lc
					,linphone_core_get_current_call(marie->lc)
					,linphone_call_get_current_params(linphone_core_get_current_call(marie->lc)));


		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallUpdating,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(marie->lc)),LinphoneReasonNotAcceptable, int, "%d");

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning,1, int, "%d");
		check_call_state(pauline,LinphoneCallStreamsRunning);
		check_call_state(marie,LinphoneCallStreamsRunning);


		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_redirect(void){
	LinphoneCoreManager* marie   = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* laure   = linphone_core_manager_new("laure_rc");
	MSList* lcs = NULL;
	char *margaux_url = NULL;
	LinphoneCall* marie_call;

	lcs = ms_list_append(lcs,marie->lc);
	lcs = ms_list_append(lcs,pauline->lc);
	lcs = ms_list_append(lcs,laure->lc);
	/*
		Marie calls Pauline, which will redirect the call to Laure via a 302
	*/

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived,1,6000));

	if (linphone_core_get_current_call(pauline->lc)){
		margaux_url = linphone_address_as_string(laure->identity);
		linphone_core_redirect_call(pauline->lc, linphone_core_get_current_call(pauline->lc), margaux_url);
		ms_free(margaux_url);

		/* laure should be ringing now */
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived,1,6000));
		/* pauline should have ended the call */
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd,1,1000));
		/* the call should still be ringing on marie's side */
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1,1000));

		linphone_core_accept_call(laure->lc, linphone_core_get_current_call(laure->lc));

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1,1000));

		BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));

		liblinphone_tester_check_rtcp(marie, laure);

		linphone_core_terminate_all_calls(laure->lc);

		BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallEnd,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,5000));
	}

	ms_list_free(lcs);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);

}

static void call_established_with_rejected_reinvite_with_error_base(bool_t trans_pending) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok=TRUE;
	int result;

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));

	if (call_ok){
		linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMA",8000,1),TRUE); /*add PCMA*/

		if (trans_pending) {
			LinphoneInfoMessage * info = linphone_core_create_info_message(pauline->lc);
			linphone_call_send_info_message(linphone_core_get_current_call(pauline->lc),info);

		} else
			sal_enable_unconditional_answer(marie->lc->sal,TRUE);

		result = linphone_core_update_call(	pauline->lc
						,linphone_core_get_current_call(pauline->lc)
						,linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)));

		if (trans_pending)
			BC_ASSERT_NOT_EQUAL(result,0, int, "%d");
		else
			BC_ASSERT_EQUAL(result,0,int, "%d");

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));

		BC_ASSERT_EQUAL(linphone_call_get_reason(linphone_core_get_current_call(pauline->lc)),LinphoneReasonTemporarilyUnavailable, int, "%d"); /*might be change later*/

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning,1, int, "%d");
		check_call_state(pauline,LinphoneCallStreamsRunning);
		check_call_state(marie,LinphoneCallStreamsRunning);

		if (!trans_pending)
			sal_enable_unconditional_answer(marie->lc->sal,FALSE);


		linphone_core_terminate_all_calls(pauline->lc);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_established_with_rejected_reinvite_with_error(void) {
	call_established_with_rejected_reinvite_with_error_base(FALSE);
}

static void call_established_with_rejected_reinvite_with_trans_pending_error(void) {
	call_established_with_rejected_reinvite_with_error_base(TRUE);
}

static void call_rejected_because_wrong_credentials_with_params(const char* user_agent,bool_t enable_auth_req_cb) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneAuthInfo* good_auth_info=linphone_auth_info_clone(linphone_core_find_auth_info(marie->lc,NULL,linphone_address_get_username(marie->identity),NULL));
	LinphoneAuthInfo* wrong_auth_info=linphone_auth_info_clone(good_auth_info);
	bool_t result=FALSE;
	linphone_auth_info_set_passwd(wrong_auth_info,"passecretdutout");
	linphone_core_clear_all_auth_info(marie->lc);

	if (user_agent) {
		linphone_core_set_user_agent(marie->lc,user_agent,NULL);
	}
	if (!enable_auth_req_cb) {
		((VTableReference*)(marie->lc->vtable_refs->data))->vtable->auth_info_requested=NULL;
		linphone_core_add_auth_info(marie->lc,wrong_auth_info);
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(marie->lc,marie->identity));

	result=wait_for(marie->lc,marie->lc,&marie->stat.number_of_auth_info_requested,1);

	if (enable_auth_req_cb) {
		BC_ASSERT_TRUE(result);
		/*automatically re-inititae the call*/
		linphone_core_add_auth_info(marie->lc,wrong_auth_info);
	}

	BC_ASSERT_TRUE(wait_for(marie->lc,marie->lc,&marie->stat.number_of_LinphoneCallError,1));
	if (enable_auth_req_cb) {
		BC_ASSERT_EQUAL(marie->stat.number_of_auth_info_requested,2, int, "%d");
	}
	/*to make sure unregister will work*/
	linphone_core_clear_all_auth_info(marie->lc);
	linphone_core_add_auth_info(marie->lc,good_auth_info);
	linphone_auth_info_destroy(good_auth_info);
	linphone_core_manager_destroy(marie);
}

static void call_rejected_because_wrong_credentials() {
	call_rejected_because_wrong_credentials_with_params(NULL,TRUE);
}

static void call_rejected_without_403_because_wrong_credentials() {
	call_rejected_because_wrong_credentials_with_params("tester-no-403",TRUE);
}

static void call_rejected_without_403_because_wrong_credentials_no_auth_req_cb() {
	call_rejected_because_wrong_credentials_with_params("tester-no-403",FALSE);
}

#ifdef VIDEO_ENABLED
/*this is call forking with early media managed at client side (not by flexisip server)*/
static void multiple_early_media(void) {
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager* marie1 = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new("marie_early_rc");
	MSList *lcs=NULL;
	LinphoneCallParams *params=linphone_core_create_default_call_parameters(pauline->lc);
	LinphoneVideoPolicy pol;
	LinphoneCall *marie1_call;
	LinphoneCall *marie2_call;
	LinphoneCall *pauline_call;
	LinphoneInfoMessage *info;
	int dummy=0;
	pol.automatically_accept=1;
	pol.automatically_initiate=1;

	linphone_core_enable_video(pauline->lc,TRUE,TRUE);

	linphone_core_enable_video(marie1->lc,TRUE,TRUE);
	linphone_core_set_video_policy(marie1->lc,&pol);

	linphone_core_enable_video(marie2->lc,TRUE,TRUE);
	linphone_core_set_video_policy(marie2->lc,&pol);
	linphone_core_set_audio_port_range(marie2->lc,40200,40300);
	linphone_core_set_video_port_range(marie2->lc,40400,40500);

	lcs=ms_list_append(lcs,marie1->lc);
	lcs=ms_list_append(lcs,marie2->lc);
	lcs=ms_list_append(lcs,pauline->lc);

	linphone_call_params_enable_early_media_sending(params,TRUE);
	linphone_call_params_enable_video(params,TRUE);

	linphone_core_invite_address_with_params(pauline->lc,marie1->identity,params);
	linphone_call_params_destroy(params);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallIncomingEarlyMedia,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallIncomingEarlyMedia,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,3000));

	pauline_call=linphone_core_get_current_call(pauline->lc);
	marie1_call=linphone_core_get_current_call(marie1->lc);
	marie2_call=linphone_core_get_current_call(marie2->lc);

	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(marie1_call);
	BC_ASSERT_PTR_NOT_NULL(marie2_call);

	if (pauline_call && marie1_call && marie2_call){

		/*wait a bit that streams are established*/
		wait_for_list(lcs,&dummy,1,6000);
		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(pauline),70,int,"%i");
		BC_ASSERT_TRUE(linphone_call_get_audio_stats(marie1_call)->download_bandwidth>70);
		BC_ASSERT_TRUE(linphone_call_get_audio_stats(marie2_call)->download_bandwidth>70);

		linphone_core_accept_call(marie1->lc,linphone_core_get_current_call(marie1->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie1->stat.number_of_LinphoneCallStreamsRunning,1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));

		/*marie2 should get her call terminated*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));

		/*wait a bit that streams are established*/
		wait_for_list(lcs,&dummy,1,3000);
		BC_ASSERT_TRUE(linphone_call_get_audio_stats(pauline_call)->download_bandwidth>71);
		BC_ASSERT_TRUE(linphone_call_get_audio_stats(marie1_call)->download_bandwidth>71);

		/*send an INFO in reverse side to check that dialogs are properly established*/
		info=linphone_core_create_info_message(marie1->lc);
		linphone_call_send_info_message(marie1_call,info);
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_inforeceived,1,3000));
	}

	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie1->stat.number_of_LinphoneCallEnd,1,3000));

	ms_list_free(lcs);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
}
#endif

void check_media_direction(LinphoneCoreManager* mgr, LinphoneCall *call, MSList* lcs,LinphoneMediaDirection audio_dir, LinphoneMediaDirection video_dir) {
	BC_ASSERT_PTR_NOT_NULL(call);
	if  (call) {
		const LinphoneCallParams *params = linphone_call_get_current_params(call);
#ifdef VIDEO_ENABLED
		int current_recv_iframe = mgr->stat.number_of_IframeDecoded;
		int expected_recv_iframe=0;
		int dummy = 0;

		BC_ASSERT_EQUAL(video_dir,linphone_call_params_get_video_direction(params), int, "%d");
		linphone_call_set_next_video_frame_decoded_callback(call,linphone_call_cb,mgr->lc);
		linphone_call_send_vfu_request(call);


		wait_for_list(lcs,&dummy,1,2000); /*on some device, it may take 3 to 4s to get audio from mic*/

		switch (video_dir) {
		case LinphoneMediaDirectionInactive:
			BC_ASSERT_TRUE(linphone_call_get_video_stats(call)->upload_bandwidth<5);
		case LinphoneMediaDirectionSendOnly:
			expected_recv_iframe = 0;
			BC_ASSERT_TRUE(linphone_call_get_video_stats(call)->download_bandwidth<5);
			break;
		case LinphoneMediaDirectionRecvOnly:
			BC_ASSERT_TRUE(linphone_call_get_video_stats(call)->upload_bandwidth<5);
		case LinphoneMediaDirectionSendRecv:
			expected_recv_iframe = 1;
			break;
		}

		BC_ASSERT_TRUE(wait_for_list(lcs, &mgr->stat.number_of_IframeDecoded,current_recv_iframe + expected_recv_iframe,3000));
#endif
		BC_ASSERT_EQUAL(audio_dir,linphone_call_params_get_audio_direction(params), int, "%d");
		switch (audio_dir) {
			case LinphoneMediaDirectionInactive:
				BC_ASSERT_TRUE(linphone_call_get_audio_stats(call)->upload_bandwidth<5);
			case LinphoneMediaDirectionSendOnly:
				BC_ASSERT_TRUE(linphone_call_get_video_stats(call)->download_bandwidth<5);
				if (audio_dir == LinphoneMediaDirectionSendOnly) BC_ASSERT_TRUE(wait_for_list(lcs,mgr->stat.current_audio_upload_bandwidth,70,4000));
				break;
			case LinphoneMediaDirectionRecvOnly:
				BC_ASSERT_TRUE(linphone_call_get_audio_stats(call)->upload_bandwidth<5);
			case LinphoneMediaDirectionSendRecv:
				BC_ASSERT_TRUE(wait_for_list(lcs,mgr->stat.current_audio_download_bandwidth,70,4000));
				if (audio_dir == LinphoneMediaDirectionSendRecv) BC_ASSERT_TRUE(wait_for_list(lcs,mgr->stat.current_audio_upload_bandwidth,70,4000));
				break;
			}
	}

}
#ifdef VIDEO_ENABLED
static void accept_call_in_send_only_base(LinphoneCoreManager* pauline, LinphoneCoreManager *marie, MSList *lcs) {
#define DEFAULT_WAIT_FOR 10000
	LinphoneCallParams *params;
	LinphoneVideoPolicy pol;
	LinphoneCall *call;
	pol.automatically_accept=1;
	pol.automatically_initiate=1;

	linphone_core_enable_video(pauline->lc,TRUE,TRUE);
	linphone_core_set_video_policy(pauline->lc,&pol);
	linphone_core_set_video_device(pauline->lc,"Mire: Mire (synthetic moving picture)");

	linphone_core_enable_video(marie->lc,TRUE,TRUE);
	linphone_core_set_video_policy(marie->lc,&pol);
	linphone_core_set_video_device(marie->lc,"Mire: Mire (synthetic moving picture)");

	linphone_call_set_next_video_frame_decoded_callback(linphone_core_invite_address(pauline->lc,marie->identity)
														,linphone_call_cb
														,pauline->lc);


	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived,1,DEFAULT_WAIT_FOR));

	{
		char* remote_uri = linphone_address_as_string_uri_only(pauline->identity);
		call = linphone_core_find_call_from_uri(marie->lc,remote_uri);
		ms_free(remote_uri);
	}

	if  (call) {
		params=linphone_core_create_default_call_parameters(marie->lc);
		linphone_call_params_set_audio_direction(params,LinphoneMediaDirectionSendOnly);
		linphone_call_params_set_video_direction(params,LinphoneMediaDirectionSendOnly);
		linphone_core_accept_call_with_params(marie->lc,call,params);
		linphone_call_params_destroy(params);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,1,DEFAULT_WAIT_FOR));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote,1,DEFAULT_WAIT_FOR));

		check_media_direction(marie,call,lcs,LinphoneMediaDirectionSendOnly,LinphoneMediaDirectionSendOnly);
	}


	call=linphone_core_get_current_call(pauline->lc);
	if  (call) {
		check_media_direction(pauline,call,lcs,LinphoneMediaDirectionRecvOnly,LinphoneMediaDirectionRecvOnly);
	}

}
static void accept_call_in_send_base(bool_t caller_has_ice) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager *pauline, *marie;
	MSList *lcs=NULL;;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (caller_has_ice) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
	}

	lcs=ms_list_append(lcs,pauline->lc);
	lcs=ms_list_append(lcs,marie->lc);

	accept_call_in_send_only_base(pauline,marie,lcs);


	end_call(marie,pauline);
	ms_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;

	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void accept_call_in_send_only(void)  {
	accept_call_in_send_base(FALSE);
}

static void accept_call_in_send_only_with_ice(void)  {
	accept_call_in_send_base(TRUE);
}

void two_accepted_call_in_send_only() {
	int begin;
	int leaked_objects;
	LinphoneCoreManager *pauline, *marie, *laure;
	MSList *lcs=NULL;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	laure = linphone_core_manager_new("laure_rc");

	lcs=ms_list_append(lcs,pauline->lc);
	lcs=ms_list_append(lcs,marie->lc);
	lcs=ms_list_append(lcs,laure->lc);

	accept_call_in_send_only_base(pauline,marie,lcs);


	reset_counters(&marie->stat);
	accept_call_in_send_only_base(laure,marie,lcs);

	end_call(marie,pauline);
	end_call(laure,marie);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);

	leaked_objects=belle_sip_object_get_object_count()-begin;

	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}

}
#endif

static char *create_filepath(const char *dir, const char *filename, const char *ext) {
	return ms_strdup_printf("%s/%s.%s",dir,filename,ext);
}

static void record_call(const char *filename, bool_t enableVideo) {
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *pauline = NULL;
	LinphoneCallParams *marieParams = NULL;
	LinphoneCallParams *paulineParams = NULL;
	LinphoneCall *callInst = NULL;
	const char **formats, *format;
	char *filepath;
	int dummy=0, i;
	bool_t call_succeeded = FALSE;

#if defined(HAVE_OPENH264) && defined(ANDROID)
	ms_init();
	libmsopenh264_init();
#endif


	marie = linphone_core_manager_new("marie_h264_rc");
	pauline = linphone_core_manager_new("pauline_h264_rc");
	marieParams = linphone_core_create_default_call_parameters(marie->lc);
	paulineParams = linphone_core_create_default_call_parameters(pauline->lc);

#ifdef VIDEO_ENABLED
	if(enableVideo) {
		if((linphone_core_find_payload_type(marie->lc, "H264", -1, -1) != NULL)
				&& (linphone_core_find_payload_type(pauline->lc, "H264", -1, -1) != NULL)) {
			linphone_call_params_enable_video(marieParams, TRUE);
			linphone_call_params_enable_video(paulineParams, TRUE);
			disable_all_video_codecs_except_one(marie->lc, "H264");
			disable_all_video_codecs_except_one(pauline->lc, "H264");
		} else {
			ms_warning("call_recording(): the H264 payload has not been found. Only sound will be recorded");
		}
	}
#endif

	formats = linphone_core_get_supported_file_formats(marie->lc);

	for(i=0, format = formats[0]; format != NULL; i++, format = formats[i]) {
		filepath = create_filepath(bc_tester_get_writable_dir_prefix(), filename, format);
		remove(filepath);
		linphone_call_params_set_record_file(marieParams, filepath);
		BC_ASSERT_TRUE(call_succeeded = call_with_params(marie, pauline, marieParams, paulineParams));
		BC_ASSERT_PTR_NOT_NULL(callInst = linphone_core_get_current_call(marie->lc));
		if ((call_succeeded == TRUE) && (callInst != NULL)) {
			ms_message("call_recording(): start recording into %s", filepath);
			linphone_call_start_recording(callInst);
			wait_for_until(marie->lc,pauline->lc,&dummy,1,5000);
			linphone_call_stop_recording(callInst);
			end_call(marie, pauline);
			BC_ASSERT_EQUAL(access(filepath, F_OK), 0, int, "%d");
		}
		remove(filepath);
		ms_free(filepath);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
#if defined(HAVE_OPENH264) && defined(ANDROID)
	ms_exit();
#endif
}

static void audio_call_recording_test(void) {
	record_call("recording", FALSE);
}

#ifdef VIDEO_ENABLED
static void video_call_recording_test(void) {
	record_call("recording", TRUE);
}

static void video_call_snapshot(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *marieParams = linphone_core_create_default_call_parameters(marie->lc);
	LinphoneCallParams *paulineParams = linphone_core_create_default_call_parameters(pauline->lc);
	LinphoneCall *callInst = NULL;
	char *filename = create_filepath(bc_tester_get_writable_dir_prefix(), "snapshot", "jpeg");
	int dummy = 0;
	bool_t call_succeeded = FALSE;

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, FALSE);
	linphone_call_params_enable_video(marieParams, TRUE);
	linphone_call_params_enable_video(paulineParams, TRUE);

	BC_ASSERT_TRUE(call_succeeded = call_with_params(marie, pauline, marieParams, paulineParams));
	BC_ASSERT_PTR_NOT_NULL(callInst = linphone_core_get_current_call(marie->lc));
	if((call_succeeded == TRUE) && (callInst != NULL)) {
		linphone_call_take_video_snapshot(callInst, filename);
		wait_for_until(marie->lc, pauline->lc, &dummy, 1, 5000);
		BC_ASSERT_EQUAL(access(filename, F_OK), 0, int, "%d");
		remove(filename);
	}
	ms_free(filename);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#endif

static void call_with_in_dialog_update(void) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCallParams *params;
	bool_t call_ok;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;

	liblinphone_tester_check_rtcp(marie,pauline);
	params=linphone_core_create_call_params(marie->lc,linphone_core_get_current_call(marie->lc));
	params->no_user_consent=TRUE;
	linphone_core_update_call(marie->lc,linphone_core_get_current_call(marie->lc),params);
	linphone_call_params_destroy(params);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallUpdating,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallUpdatedByRemote,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
	end_call(marie,pauline);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}
static void call_with_in_dialog_codec_change_base(bool_t no_sdp) {
	int begin;
	int leaked_objects;
	int dummy=0;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCallParams *params;
	bool_t call_ok;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;

	liblinphone_tester_check_rtcp(marie,pauline);
	params=linphone_core_create_call_params(marie->lc,linphone_core_get_current_call(marie->lc));

	linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
	linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMU",8000,1),FALSE); /*disable PCMU*/
	linphone_core_enable_payload_type(pauline->lc,linphone_core_find_payload_type(pauline->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/
	linphone_core_enable_payload_type(marie->lc,linphone_core_find_payload_type(marie->lc,"PCMA",8000,1),TRUE); /*enable PCMA*/
	if (no_sdp) {
		linphone_core_enable_sdp_200_ack(marie->lc,TRUE);
	}
	linphone_core_update_call(marie->lc,linphone_core_get_current_call(marie->lc),params);
	linphone_call_params_destroy(params);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallUpdating,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallUpdatedByRemote,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_STRING_EQUAL("PCMA",linphone_payload_type_get_mime_type(linphone_call_params_get_used_audio_codec(linphone_call_get_current_params(linphone_core_get_current_call(marie->lc)))));
	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 5000);
	BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(marie),70,int,"%i");
	BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(pauline),70,int,"%i");

	end_call(marie,pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}
static void call_with_in_dialog_codec_change(void) {
	call_with_in_dialog_codec_change_base(FALSE);
}
static void call_with_in_dialog_codec_change_no_sdp(void) {
	call_with_in_dialog_codec_change_base(TRUE);
}
static void call_with_custom_supported_tags(void) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	const LinphoneCallParams *remote_params;
	const char *recv_supported;
	bool_t call_ok;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_add_supported_tag(marie->lc,"pouet-tag");
	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(marie,pauline);
	remote_params=linphone_call_get_remote_params(linphone_core_get_current_call(pauline->lc));
	recv_supported=linphone_call_params_get_custom_header(remote_params,"supported");
	BC_ASSERT_PTR_NOT_NULL(recv_supported);
	if (recv_supported){
		BC_ASSERT_PTR_NOT_NULL(strstr(recv_supported,"pouet-tag"));
	}
	end_call(marie,pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void call_log_from_taken_from_p_asserted_id(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *c1,*c2;
	LinphoneCallParams *params;
	const char* paulie_asserted_id ="\"Paupauche\" <sip:pauline@super.net>";
	LinphoneAddress *paulie_asserted_id_addr = linphone_address_new(paulie_asserted_id);
	LpConfig *marie_lp;
	bool_t call_ok;

	params=linphone_core_create_default_call_parameters(pauline->lc);

	linphone_call_params_add_custom_header(params,"P-Asserted-Identity",paulie_asserted_id);
	/*fixme, should be able to add several time the same header linphone_call_params_add_custom_header(params,"P-Asserted-Identity","\"Paupauche\" <tel:+12345>");*/

	marie_lp = linphone_core_get_config(marie->lc);
	lp_config_set_int(marie_lp,"sip","call_logs_use_asserted_id_instead_of_from",1);


	BC_ASSERT_TRUE(call_ok=call_with_caller_params(pauline,marie,params));

	if (!call_ok) goto end;

	c1=linphone_core_get_current_call(pauline->lc);
	c2=linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);

	/*make sure remote identity is hidden*/
	BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_get_remote_address(c2),paulie_asserted_id_addr));



	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
end:
	linphone_call_params_destroy(params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void incoming_invite_with_invalid_sdp() {
	LinphoneCoreManager* caller = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* callee = linphone_core_manager_new( "marie_rc");
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};

	callee_test_params.sdp_simulate_error = TRUE;
	BC_ASSERT_FALSE(call_with_params2(caller,callee,&caller_test_params, &callee_test_params, FALSE));

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(callee->lc));
	BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError,1, int, "%d");
	/*call will be drop before presented to the application, because it is invalid*/
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived,0, int, "%d");

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void outgoing_invite_with_invalid_sdp() {
	LinphoneCoreManager* caller = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* callee = linphone_core_manager_new( "marie_rc");
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};

	caller_test_params.sdp_simulate_error = TRUE;
	BC_ASSERT_FALSE(call_with_params2(caller,callee,&caller_test_params, &callee_test_params, FALSE));

	BC_ASSERT_PTR_NULL(linphone_core_get_current_call(callee->lc));
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallIncomingReceived,1, int, "%d");
	BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallError,1, int, "%d");
	// actually callee does not receive error, because it just get a BYE from the other part
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallError,0, int, "%d");
	BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallEnd,1, int, "%d");

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
}

static void incoming_reinvite_with_invalid_ack_sdp(){
#ifdef VIDEO_ENABLED
	LinphoneCoreManager* caller = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* callee = linphone_core_manager_new( "marie_rc");
	LinphoneCall * inc_call;
	BC_ASSERT_TRUE(call(caller,callee));
	inc_call = linphone_core_get_current_call(callee->lc);

	BC_ASSERT_PTR_NOT_NULL(inc_call);
	if (inc_call) {
		const LinphoneCallParams *caller_params;
		stats initial_caller_stat=caller->stat;
		stats initial_callee_stat=callee->stat;
		sal_call_set_sdp_handling(inc_call->op, SalOpSDPSimulateError); /* will force a parse error for the ACK SDP*/
		BC_ASSERT_PTR_NOT_NULL(setup_video(caller, callee, TRUE));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallUpdating,initial_callee_stat.number_of_LinphoneCallUpdating+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,initial_callee_stat.number_of_LinphoneCallStreamsRunning+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,initial_caller_stat.number_of_LinphoneCallStreamsRunning));
		/*Basically the negotiation failed but since the call was already running, we expect it to restore to
		the previous state so error stats should not be changed*/
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallError,initial_callee_stat.number_of_LinphoneCallError, int, "%d");
		/*and remote should have received an update notification*/
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallUpdatedByRemote,initial_caller_stat.number_of_LinphoneCallUpdatedByRemote+1, int, "%d");


		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
		caller_params = linphone_call_get_current_params(linphone_core_get_current_call(caller->lc));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,(int*)&caller_params->has_video,FALSE));

		sal_call_set_sdp_handling(inc_call->op, SalOpSDPNormal);
	}
	linphone_core_terminate_all_calls(caller->lc);
	BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&caller->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallEnd,1));

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
#else
	ms_warning("not tested because video not available");
#endif
}

static void outgoing_reinvite_with_invalid_ack_sdp()  {
#ifdef VIDEO_ENABLED
	LinphoneCoreManager* caller = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* callee = linphone_core_manager_new( "marie_rc");
	LinphoneCall * out_call;
	BC_ASSERT_TRUE(call(caller,callee));
	out_call = linphone_core_get_current_call(caller->lc);

	BC_ASSERT_PTR_NOT_NULL(out_call);
	if (out_call) {
		stats initial_caller_stat=caller->stat;
		stats initial_callee_stat=callee->stat;
		sal_call_set_sdp_handling(out_call->op, SalOpSDPSimulateError); /* will force a parse error for the ACK SDP*/
		BC_ASSERT_PTR_NOT_NULL(setup_video(caller, callee, TRUE));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallUpdating,initial_callee_stat.number_of_LinphoneCallUpdating+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallStreamsRunning,initial_callee_stat.number_of_LinphoneCallStreamsRunning+1));
		BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&caller->stat.number_of_LinphoneCallStreamsRunning,initial_caller_stat.number_of_LinphoneCallStreamsRunning));
		/*Basically the negotiation failed but since the call was already running, we expect it to restore to
		the previous state so error stats should not be changed*/
		BC_ASSERT_EQUAL(callee->stat.number_of_LinphoneCallError,initial_callee_stat.number_of_LinphoneCallError, int, "%d");
		/*and remote should not have received any update notification*/
		BC_ASSERT_EQUAL(caller->stat.number_of_LinphoneCallUpdatedByRemote,initial_caller_stat.number_of_LinphoneCallUpdatedByRemote, int, "%d");

		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(callee->lc))));
		BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(linphone_core_get_current_call(caller->lc))));

		sal_call_set_sdp_handling(out_call->op, SalOpSDPNormal);
	}
	linphone_core_terminate_all_calls(caller->lc);
	BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&caller->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(caller->lc,callee->lc,&callee->stat.number_of_LinphoneCallEnd,1));

	linphone_core_manager_destroy(callee);
	linphone_core_manager_destroy(caller);
#else
	ms_warning("not tested because video not available");
#endif
}


static void call_with_paused_no_sdp_on_resume() {
	int begin;
	int leaked_objects;
	int dummy=0;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall* call_marie = NULL;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	BC_ASSERT_TRUE(call(pauline,marie));
	liblinphone_tester_check_rtcp(marie,pauline);

	call_marie = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(call_marie);
	if (!call_marie) goto end;

	ms_message("== Call is OK ==");

	/* the called party pause the call */
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

	linphone_core_pause_call(marie->lc,call_marie);
	ms_message("== Call pausing ==");
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausing,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPausedByRemote,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPaused,1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	ms_message("== Call paused, marie call: %p ==", call_marie);

	linphone_core_enable_sdp_200_ack(marie->lc,TRUE);

	linphone_core_resume_call(marie->lc,call_marie);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));

	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 3000);
	BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(marie),70,int,"%i");
	BC_ASSERT_TRUE(linphone_call_get_audio_stats(linphone_core_get_current_call(pauline->lc))->download_bandwidth>70);
end:
	end_call(marie,pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void early_media_without_sdp_in_200_base( bool_t use_video, bool_t use_ice ){
	LinphoneCoreManager* marie   = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	MSList* lcs = NULL;
	LinphoneCall* marie_call;
	LinphoneCallParams* params = NULL;
	LinphoneCallLog *marie_call_log;
	uint64_t connected_time=0;
	uint64_t ended_time=0;
	int dummy=0;

	lcs = ms_list_append(lcs,marie->lc);
	lcs = ms_list_append(lcs,pauline->lc);
	if (use_ice){
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	}
	/*
		Marie calls Pauline, and after the call has rung, transitions to an early_media session
	*/
	params = linphone_core_create_default_call_parameters(marie->lc);

	if( use_video){

		linphone_call_params_enable_video(params, TRUE);

		linphone_core_enable_video_capture(pauline->lc, TRUE);
		linphone_core_enable_video_display(pauline->lc, TRUE);
		linphone_core_enable_video_capture(marie->lc, TRUE);
		linphone_core_enable_video_display(marie->lc, FALSE);
	}

	marie_call = linphone_core_invite_address_with_params(marie->lc, pauline->identity, params);
	marie_call_log = linphone_call_get_call_log(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging,1,1000));

	if (linphone_core_inc_invite_pending(pauline->lc)) {
		LinphoneCall* pauline_call = linphone_core_get_current_call(pauline->lc);

		/* send a 183 to initiate the early media */
		linphone_core_accept_early_media(pauline->lc, pauline_call);

		BC_ASSERT_TRUE( wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia,1,2000) );
		BC_ASSERT_TRUE( wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,2000) );

		liblinphone_tester_check_rtcp(marie, pauline);

		/* will send the 200OK _without_ SDP. We expect the early-media SDP to be used instead */
		sal_call_set_sdp_handling(pauline_call->op, SalOpSDPSimulateRemove);
		linphone_core_accept_call(pauline->lc, pauline_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1,1000));
		connected_time=ms_get_cur_time_ms();
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1,3000));

		BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));

		liblinphone_tester_check_rtcp(marie, pauline);
		/*just to have a call duration !=0*/
		wait_for_list(lcs,&dummy,1,2000);

		linphone_core_terminate_all_calls(pauline->lc);

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
		ended_time=ms_get_cur_time_ms();
		BC_ASSERT_LOWER(labs((long)((linphone_call_log_get_duration(marie_call_log)*1000) - (int64_t)(ended_time - connected_time))), 1000, long, "%ld");
		ms_list_free(lcs);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_early_media_and_no_sdp_in_200_with_video(){
	early_media_without_sdp_in_200_base(TRUE, FALSE);
}

static void call_with_early_media_and_no_sdp_in_200(){
	early_media_without_sdp_in_200_base(FALSE, FALSE);
}

static void call_with_early_media_ice_and_no_sdp_in_200(){
	early_media_without_sdp_in_200_base(FALSE, TRUE);
}

static void call_with_generic_cn(void) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall *pauline_call;
	char *audio_file_with_silence=bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recorded_file=bc_tester_file("result.wav");

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	remove(recorded_file);

	linphone_core_use_files(marie->lc,TRUE);
	linphone_core_use_files(pauline->lc,TRUE);
	linphone_core_set_play_file(marie->lc, audio_file_with_silence);
	/*linphone_core_set_play_file(pauline->lc, NULL);*/
	linphone_core_set_record_file(pauline->lc, recorded_file);
	linphone_core_enable_generic_confort_noise(marie->lc, TRUE);
	linphone_core_enable_generic_confort_noise(pauline->lc, TRUE);
	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_call=linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call){
		const rtp_stats_t *rtps;

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 8000);
		rtps=rtp_session_get_stats(pauline_call->audiostream->ms.sessions.rtp_session);
		BC_ASSERT_TRUE(rtps->packet_recv<=300 && rtps->packet_recv>=200);
	}
	end_call(marie,pauline);

	if (pauline_call){
		struct stat stbuf;
		int err;

		err=stat(recorded_file,&stbuf);
		BC_ASSERT_EQUAL(err, 0, int, "%d");
		if (err==0){
			BC_ASSERT_GREATER(stbuf.st_size,120000,int, "%d");
		}
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
	ms_free(audio_file_with_silence);
	ms_free(recorded_file);
}
void static call_state_changed_2(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
	LCSipTransports sip_tr;
	if (cstate==LinphoneCallReleased) {
		/*to make sure transport is changed*/
		sip_tr.udp_port = 0;
		sip_tr.tcp_port = 45876;
		sip_tr.tls_port = 0;

		linphone_core_set_sip_transports(lc,&sip_tr);
	}
}
void static call_state_changed_3(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
/*just to check multi listener in such situation*/
	char* to=linphone_address_as_string(linphone_call_get_call_log(call)->to);
	char* from=linphone_address_as_string(linphone_call_get_call_log(call)->from);
	ms_message("Third call listener reports: %s call from [%s] to [%s], new state is [%s]"	,linphone_call_get_call_log(call)->dir==LinphoneCallIncoming?"Incoming":"Outgoing"
																,from
																,to
																,linphone_call_state_to_string(cstate));
	ms_free(to);
	ms_free(from);
}


static void call_with_transport_change_base(bool_t succesfull_call) {
	int begin;
	int leaked_objects;
	LCSipTransports sip_tr;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCoreVTable * v_table;
	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();
	v_table = linphone_core_v_table_new();
	v_table->call_state_changed=call_state_changed_2;
	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_add_listener(marie->lc,v_table);
	v_table = linphone_core_v_table_new();
	v_table->call_state_changed=call_state_changed_3;
	linphone_core_add_listener(marie->lc,v_table);

	sip_tr.udp_port = 0;
	sip_tr.tcp_port = 45875;
	sip_tr.tls_port = 0;
	linphone_core_set_sip_transports(marie->lc,&sip_tr);
	if (succesfull_call) {
		BC_ASSERT_TRUE(call(marie,pauline));
		linphone_core_terminate_all_calls(marie->lc);
	}
	else
		linphone_core_invite(marie->lc,"nexiste_pas");

	if (succesfull_call)
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallEnd,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallReleased,1));
	if (succesfull_call) {
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallReleased,1));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects,0,int,"%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}

}
static void call_with_transport_change_after_released(void) {
	call_with_transport_change_base(TRUE);
}
static void unsucessfull_call_with_transport_change_after_released(void) {
	call_with_transport_change_base(FALSE);
}
#ifdef VIDEO_ENABLED

static void video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryption mode, bool_t no_sdp) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCallParams *params;
	const LinphoneCallParams *current_params;
	MSList *lcs=NULL;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_avpf_mode(pauline->lc,TRUE);
	linphone_core_set_video_device(pauline->lc,"Mire: Mire (synthetic moving picture)");
	linphone_core_set_video_device(marie->lc,"Mire: Mire (synthetic moving picture)");
	linphone_core_set_avpf_mode(marie->lc,TRUE);
	lcs=ms_list_append(lcs,pauline->lc);
	lcs=ms_list_append(lcs,marie->lc);

	video_call_base_2(marie,pauline,TRUE,mode,TRUE,TRUE);

	if (linphone_core_get_current_call(marie->lc)) {
		params=linphone_core_create_call_params(marie->lc,linphone_core_get_current_call(marie->lc));
		linphone_call_params_set_audio_direction(params,LinphoneMediaDirectionInactive);
		linphone_call_params_set_video_direction(params,LinphoneMediaDirectionInactive);

		linphone_core_update_call(marie->lc, linphone_core_get_current_call(marie->lc),params);
		linphone_call_params_destroy(params);

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallUpdating,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallPaused,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallPausedByRemote,1));

		check_media_direction(marie,linphone_core_get_current_call(marie->lc),lcs,LinphoneMediaDirectionInactive,LinphoneMediaDirectionInactive);
		check_media_direction(pauline,linphone_core_get_current_call(pauline->lc), lcs, LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive);

		if (no_sdp) {
			linphone_core_enable_sdp_200_ack(marie->lc,TRUE);
		}

		/*
		currently update call cannot be used in paused state, might not be good.
		params=linphone_core_create_call_params(marie->lc,linphone_core_get_current_call(marie->lc));
		linphone_call_params_set_audio_direction(params,LinphoneMediaDirectionSendRecv);
		linphone_call_params_set_video_direction(params,LinphoneMediaDirectionSendRecv);
		linphone_core_update_call(marie->lc,linphone_core_get_current_call(marie->lc),params);
		linphone_call_params_destroy(params);
		*/

		linphone_core_resume_call(marie->lc,linphone_core_get_current_call(marie->lc));

		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallResuming,1));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2));
		BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2));

		check_media_direction(marie,linphone_core_get_current_call(marie->lc),lcs,LinphoneMediaDirectionSendRecv,LinphoneMediaDirectionSendRecv);
		check_media_direction(pauline,linphone_core_get_current_call(pauline->lc),lcs,LinphoneMediaDirectionSendRecv,LinphoneMediaDirectionSendRecv);

		/*assert that after pause and resume, SRTP is still being used*/
		current_params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(current_params) , mode, int, "%d");
		current_params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(current_params) , mode, int, "%d");

	}
	end_call(marie,pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void video_call_with_re_invite_inactive_followed_by_re_invite() {
	video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionNone,FALSE);
}

static void video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp() {
	video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionNone, TRUE);
}
static void srtp_video_call_with_re_invite_inactive_followed_by_re_invite() {
	if (ms_srtp_supported())
		video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionSRTP,FALSE);
	else
		ms_message("srtp_video_call_with_re_invite_inactive_followed_by_re_invite skipped, missing srtp support");
}
static void srtp_video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp() {
	if (ms_srtp_supported())
		video_call_with_re_invite_inactive_followed_by_re_invite_base(LinphoneMediaEncryptionSRTP, TRUE);
	else
		ms_message("srtp_video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp skipped, missing srtp support");
}
static void video_call_ice_params() {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
	video_call_base(marie,pauline,FALSE,LinphoneMediaEncryptionNone,TRUE,TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
#endif

static void completion_cb(void *user_data, int percentage){
	fprintf(stdout,"%i %% completed\r",percentage);
	fflush(stdout);
}

static void simple_stereo_call(const char *codec_name, int clock_rate, int bitrate_override, bool_t stereo) {
	int begin;
	int leaked_objects;
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	PayloadType *pt;
	char *stereo_file = bc_tester_res("sounds/vrroom.wav");
	char *recordpath = bc_tester_file("stereo-record.wav");
	bool_t audio_cmp_failed = FALSE;

	belle_sip_object_enable_leak_detector(TRUE);
	begin=belle_sip_object_get_object_count();

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/*make sure we have opus*/
	pt = linphone_core_find_payload_type(marie->lc, codec_name, clock_rate, 2);
	if (!pt) {
		ms_warning("%s not available, stereo with %s not tested.",codec_name, codec_name);
		goto end;
	}
	if (stereo) payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	if (bitrate_override) linphone_core_set_payload_type_bitrate(marie->lc, pt, bitrate_override);
	pt = linphone_core_find_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (stereo) payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	if (bitrate_override) linphone_core_set_payload_type_bitrate(pauline->lc, pt, bitrate_override);

	disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);

	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, stereo_file);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_record_file(pauline->lc, recordpath);

	/*stereo is supported only without volume control, echo canceller...*/
	lp_config_set_string(marie->lc->config,"sound","features","NONE");
	lp_config_set_string(pauline->lc->config,"sound","features","NONE");

	if (!BC_ASSERT_TRUE(call(pauline,marie))) goto end;
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 6000);
	end_call(pauline, marie);


	if (clock_rate!=48000) {
		ms_warning("Similarity checking not implemented for files not having the same sampling rate");
	}else{
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(ANDROID)
		double similar;
		double min_threshold = .7f;
		double max_threshold = 1.f;
		if (!stereo){
			/*when opus doesn't transmit stereo, the cross correlation is around 0.54 : as expected, it is not as good as in full stereo mode*/
			min_threshold = .4f;
			max_threshold = .6f;
		}
		BC_ASSERT_EQUAL(ms_audio_diff(recordpath, stereo_file,&similar,audio_cmp_max_shift,completion_cb,NULL), 0, int, "%d");
		BC_ASSERT_GREATER(similar, min_threshold, double, "%g");
		BC_ASSERT_LOWER(similar, max_threshold, double, "%g");
		if (similar<min_threshold || similar>max_threshold){
			audio_cmp_failed = TRUE;
		}
#endif
	}
	if (!audio_cmp_failed) unlink(recordpath);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(stereo_file);
	ms_free(recordpath);

	leaked_objects=belle_sip_object_get_object_count()-begin;
	BC_ASSERT_EQUAL(leaked_objects, 0, int, "%d");
	if (leaked_objects>0){
		belle_sip_object_dump_active_objects();
	}
}

static void simple_stereo_call_l16(void){
	simple_stereo_call("L16", 44100, 0, TRUE);
}

static void simple_stereo_call_opus(void){
	simple_stereo_call("opus", 48000, 150, TRUE);
}

static void simple_mono_call_opus(void){
	/*actually a call where input/output is made with stereo but opus transmits everything as mono*/
	simple_stereo_call("opus", 48000, 150, FALSE);
}

/* because SIP ALG (like in android phones) crash when seing a domain name in SDP, we prefer using SIP/TLS for both participants*/
static void call_with_fqdn_in_sdp(void) {
	bool_t tls_supported = transport_supported(LinphoneTransportTls);
	LinphoneCoreManager* marie = linphone_core_manager_new(tls_supported ? "marie_sips_rc" : "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(tls_supported ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *lp;
	bool_t call_ok;

	lp = linphone_core_get_config(marie->lc);
	lp_config_set_string(lp,"rtp","bind_address","localhost");
	lp = linphone_core_get_config(pauline->lc);
	lp_config_set_string(lp,"rtp","bind_address","localhost");


	BC_ASSERT_TRUE(call_ok=call(marie,pauline));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(pauline,marie);

#ifdef VIDEO_ENABLED
	BC_ASSERT_TRUE(add_video(pauline,marie, TRUE));
	liblinphone_tester_check_rtcp(pauline,marie);
#endif
	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_rtp_io_mode(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphonePlayer *player;
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recordpath = create_filepath(bc_tester_get_writable_dir_prefix(), "record-call_with_rtp_io_mode", "wav");
	bool_t call_ok;
	int attempts;
	double similar=1;
	const double threshold = 0.9;

	/*this test is actually attempted three times in case of failure, because the audio comparison at the end is very sensitive to
	 * jitter buffer drifts, which sometimes happen if the machine is unable to run the test in good realtime conditions */
	for (attempts=0; attempts<3; attempts++){
		/* Make sure that the record file doesn't already exists, otherwise this test will append new samples to it. */
		unlink(recordpath);
		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);

		/* The caller uses files instead of soundcard in order to avoid mixing soundcard input with file played using call's player. */
		linphone_core_use_files(marie->lc, TRUE);
		linphone_core_set_play_file(marie->lc, NULL);
		linphone_core_set_record_file(marie->lc, recordpath);
		linphone_core_use_files(pauline->lc, FALSE);

		/* The callee uses the RTP IO mode with the PCMU codec to send back audio to the caller. */
		disable_all_audio_codecs_except_one(pauline->lc, "pcmu", -1);
		lp_config_set_int(pauline->lc->config, "sound", "rtp_io", 1);
		lp_config_set_string(pauline->lc->config, "sound", "rtp_local_addr", "127.0.0.1");
		lp_config_set_string(pauline->lc->config, "sound", "rtp_remote_addr", "127.0.0.1");
		lp_config_set_int(pauline->lc->config, "sound", "rtp_local_port", 17076);
		lp_config_set_int(pauline->lc->config, "sound", "rtp_remote_port", 17076);
		lp_config_set_string(pauline->lc->config, "sound", "rtp_map", "pcmu/8000/1");

		BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
		if (!call_ok) goto end;
		player = linphone_call_get_player(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_PTR_NOT_NULL(player);
		if (player) {
			BC_ASSERT_EQUAL(linphone_player_open(player, hellopath, on_eof, marie) , 0, int, "%d");
			BC_ASSERT_EQUAL(linphone_player_start(player) , 0, int, "%d");
		}

		/* This assert should be modified to be at least as long as the WAV file */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_player_eof, 1, 10000));
		/*wait for one second more so that last RTP packets can arrive*/
		wait_for_until(pauline->lc,marie->lc,NULL,0,1000);
		end_call(pauline,marie);

		BC_ASSERT_EQUAL(ms_audio_diff(hellopath, recordpath, &similar, audio_cmp_max_shift, NULL, NULL), 0, int, "%d");
		if (similar>=threshold) break;
	}
	BC_ASSERT_GREATER(similar, threshold, double, "%g");
	BC_ASSERT_LOWER(similar, 1.0, double, "%g");
	if (similar >= threshold && similar <= 1.0) {
		remove(recordpath);
	}

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(recordpath);
	ms_free(hellopath);
}

static void generic_nack_received(const OrtpEventData *evd, stats *st) {
	if (rtcp_is_RTPFB(evd->packet)) {
		switch (rtcp_RTPFB_get_type(evd->packet)) {
			case RTCP_RTPFB_NACK:
				st->number_of_rtcp_generic_nack++;
				break;
			default:
				break;
		}
	}
}

static void call_with_generic_nack_rtcp_feedback(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LpConfig *lp;
	LinphoneCall *call_marie;
	bool_t call_ok;
	OrtpNetworkSimulatorParams params = { 0 };

	params.enabled = TRUE;
	params.loss_rate = 10;
	params.consecutive_loss_probability = 0.75;
	params.mode = OrtpNetworkSimulatorOutbound;
	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);
	lp = linphone_core_get_config(pauline->lc);
	lp_config_set_int(lp, "rtp", "rtcp_fb_generic_nack_enabled", 1);


	BC_ASSERT_TRUE(call_ok = call(pauline, marie));
	if (!call_ok) goto end;
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	call_marie = linphone_core_get_current_call(marie->lc);
	if (call_marie) {
		rtp_session_enable_network_simulation(call_marie->audiostream->ms.sessions.rtp_session, &params);
		ortp_ev_dispatcher_connect(media_stream_get_event_dispatcher(&call_marie->audiostream->ms),
			ORTP_EVENT_RTCP_PACKET_RECEIVED, RTCP_RTPFB, (OrtpEvDispatcherCb)generic_nack_received, &marie->stat);
	}

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_rtcp_generic_nack, 5, 5000));
	linphone_core_terminate_all_calls(pauline->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t call_tests[] = {
	{ "Early declined call", early_declined_call },
	{ "Call declined", call_declined },
	{ "Cancelled call", cancelled_call },
	{ "Early cancelled call", early_cancelled_call},
	{ "Call with DNS timeout", call_with_dns_time_out },
	{ "Cancelled ringing call", cancelled_ringing_call },
	{ "Call busy when calling self", call_busy_when_calling_self},
	{ "Simple call", simple_call },
	{ "Call with timeouted bye", call_with_timeouted_bye },
	{ "Direct call over IPv6", direct_call_over_ipv6},
	{ "Outbound call with multiple proxy possible", call_outbound_with_multiple_proxy },
	{ "Audio call recording", audio_call_recording_test },
#if 0 /* not yet activated because not implemented */
	{ "Multiple answers to a call", multiple_answers_call },
#endif
	{ "Multiple answers to a call with media relay", multiple_answers_call_with_media_relay },
	{ "Call with media relay", call_with_media_relay},
	{ "Call with media relay (random ports)", call_with_media_relay_random_ports},
	{ "Simple call compatibility mode", simple_call_compatibility_mode },
	{ "Early-media call", early_media_call },
	{ "Early-media call with ringing", early_media_call_with_ringing },
	{ "Early-media call with updated media session", early_media_call_with_session_update},
	{ "Early-media call with updated codec", early_media_call_with_codec_update},
	{ "Call terminated by caller", call_terminated_by_caller },
	{ "Call without SDP", call_with_no_sdp},
	{ "Call without SDP and ACK without SDP", call_with_no_sdp_ack_without_sdp},
	{ "Call paused resumed", call_paused_resumed },
	{ "Call paused resumed with loss", call_paused_resumed_with_loss },
	{ "Call paused resumed from callee", call_paused_resumed_from_callee },
	{ "SRTP call", srtp_call },
	{ "ZRTP call",zrtp_call},
	{ "ZRTP SAS call",zrtp_sas_call},
	{ "ZRTP Cipher call",zrtp_cipher_call},
	{ "DTLS SRTP call",dtls_srtp_call},
	{ "DTLS SRTP call with media relay", dtls_srtp_call_with_media_realy},
	{ "ZRTP video call",zrtp_video_call},
	{ "SRTP call with declined srtp", call_with_declined_srtp },
	{ "SRTP call paused and resumed", call_srtp_paused_and_resumed },
	{ "Call with file player", call_with_file_player},
	{ "Call with mkv file player", call_with_mkv_file_player},
	{ "Audio call with ICE no matching audio codecs", audio_call_with_ice_no_matching_audio_codecs },
#ifdef VIDEO_ENABLED
	{ "Simple video call",video_call},
	{ "Simple ZRTP video call",video_call_zrtp},
	{ "Simple DTLS video call",video_call_dtls},
	{ "Simple video call using policy",video_call_using_policy},
	{ "Video call using policy with callee video disabled", video_call_using_policy_with_callee_video_disabled },
	{ "Video call using policy with caller video disabled", video_call_using_policy_with_caller_video_disabled },
	{ "Video call without SDP",video_call_no_sdp},
	{ "SRTP ice video call", srtp_video_ice_call },
	{ "ZRTP ice video call", zrtp_video_ice_call },
	{ "Call with video added", call_with_video_added },
	{ "Call with video added 2", call_with_video_added_2 },
	{ "Call with video added (random ports)", call_with_video_added_random_ports },
	{ "Call with several video switches", call_with_several_video_switches },
	{ "SRTP call with several video switches", srtp_call_with_several_video_switches },
	{ "Call with video declined", call_with_declined_video},
	{ "Call with video declined using policy", call_with_declined_video_using_policy},
	{ "Call with multiple early media", multiple_early_media },
	{ "Call with ICE from video to non-video", call_with_ice_video_to_novideo},
	{ "Call with ICE and video added", call_with_ice_video_added },
	{ "Call with ICE and video added 2", call_with_ice_video_added_2 },
	{ "Call with ICE and video added 3", call_with_ice_video_added_3 },
	{ "Call with ICE and video added and refused", call_with_ice_video_added_and_refused },
	{ "Video call with ICE accepted using call params",video_call_ice_params},
	{ "Video call recording", video_call_recording_test },
	{ "Snapshot", video_call_snapshot },
	{ "Video call with early media and no matching audio codecs", video_call_with_early_media_no_matching_audio_codecs },
	{ "DTLS SRTP video call",dtls_srtp_video_call},
	{ "DTLS SRTP ice video call",dtls_srtp_ice_video_call},
	{ "DTLS SRTP ice video call with relay",dtls_srtp_ice_video_call_with_relay},
	{ "Video call with limited bandwidth", video_call_limited_bandwidth},
	{ "Video call accepted in send only", accept_call_in_send_only},
	{ "Video call accepted in send only with ice", accept_call_in_send_only_with_ice},
	{ "2 Video call accepted in send only", two_accepted_call_in_send_only},
	{ "Video call with re-invite(inactive) followed by re-invite", video_call_with_re_invite_inactive_followed_by_re_invite},
	{ "Video call with re-invite(inactive) followed by re-invite(no sdp)", video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp},
	{ "SRTP Video call with re-invite(inactive) followed by re-invite", srtp_video_call_with_re_invite_inactive_followed_by_re_invite},
	{ "SRTP Video call with re-invite(inactive) followed by re-invite(no sdp)", srtp_video_call_with_re_invite_inactive_followed_by_re_invite_no_sdp},
	#endif
	{ "SRTP ice call", srtp_ice_call },
	{ "ZRTP ice call", zrtp_ice_call },
	{ "ZRTP ice call with relay", zrtp_ice_call_with_relay},
	{ "DTLS SRTP ice call",dtls_srtp_ice_call},
	{ "DTLS ice call with relay", dtls_ice_call_with_relay},
	{ "Call with privacy", call_with_privacy },
	{ "Call with privacy 2", call_with_privacy2 },
	{ "Call rejected because of wrong credential", call_rejected_because_wrong_credentials},
	{ "Call rejected without 403 because of wrong credential", call_rejected_without_403_because_wrong_credentials},
	{ "Call rejected without 403 because of wrong credential and no auth req cb", call_rejected_without_403_because_wrong_credentials_no_auth_req_cb},
	{ "Call with ICE", call_with_ice },
	{ "Call with ICE without SDP", call_with_ice_no_sdp },
	{ "Call with ICE (random ports)", call_with_ice_random_ports },
	{ "Call from ICE to not ICE",ice_to_not_ice},
	{ "Call from not ICE to ICE",not_ice_to_ice},
	{ "Call with custom headers",call_with_custom_headers},
	{ "Call established with rejected INFO",call_established_with_rejected_info},
	{ "Call established with rejected RE-INVITE",call_established_with_rejected_reinvite},
	{ "Call established with rejected incoming RE-INVITE", call_established_with_rejected_incoming_reinvite },
	{ "Call established with rejected RE-INVITE in error", call_established_with_rejected_reinvite_with_error},
	{ "Call established with rejected RE-INVITE with trans pending error", call_established_with_rejected_reinvite_with_trans_pending_error},
	{ "Call established with complex rejected operation",call_established_with_complex_rejected_operation},
	{ "Call established with rejected info during re-invite",call_established_with_rejected_info_during_reinvite},
	{ "Call redirected by callee", call_redirect},
	{ "Call with specified codec bitrate", call_with_specified_codec_bitrate},
	{ "Call with in-dialog UPDATE request", call_with_in_dialog_update },
	{ "Call with in-dialog codec change", call_with_in_dialog_codec_change },
	{ "Call with in-dialog codec change no sdp", call_with_in_dialog_codec_change_no_sdp },
	{ "Call with pause no SDP on resume", call_with_paused_no_sdp_on_resume },
	{ "Call with early media and no SDP in 200 Ok", call_with_early_media_and_no_sdp_in_200 },
	{ "Call with early media and no SDP in 200 Ok with video", call_with_early_media_and_no_sdp_in_200_with_video },
	{ "Call with ICE and no SDP in 200 OK", call_with_early_media_ice_and_no_sdp_in_200},
	{ "Call with custom supported tags", call_with_custom_supported_tags },
	{ "Call log from taken from asserted id",call_log_from_taken_from_p_asserted_id},
	{ "Incoming INVITE with invalid SDP",incoming_invite_with_invalid_sdp},
	{ "Outgoing INVITE with invalid ACK SDP",outgoing_invite_with_invalid_sdp},
	{ "Incoming REINVITE with invalid SDP in ACK",incoming_reinvite_with_invalid_ack_sdp},
	{ "Outgoing REINVITE with invalid SDP in ACK",outgoing_reinvite_with_invalid_ack_sdp},
	{ "Call with generic CN", call_with_generic_cn },
	{ "Call with transport change after released", call_with_transport_change_after_released },
	{ "Unsuccessful call with transport change after released",unsucessfull_call_with_transport_change_after_released},
	{ "Simple stereo call with L16", simple_stereo_call_l16 },
	{ "Simple stereo call with opus", simple_stereo_call_opus },
	{ "Simple mono call with opus", simple_mono_call_opus },
	{ "Call with FQDN in SDP", call_with_fqdn_in_sdp},
	{ "Call with RTP IO mode", call_with_rtp_io_mode },
	{ "Call with generic NACK RTCP feedback", call_with_generic_nack_rtcp_feedback }
};

test_suite_t call_test_suite = {
	"Single Call",
	liblinphone_tester_setup,
	NULL,
	sizeof(call_tests) / sizeof(call_tests[0]),
	call_tests
};
