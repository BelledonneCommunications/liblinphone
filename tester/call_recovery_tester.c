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
#include <bctoolbox/defs.h>

/*Case where the caller disconnects just after initiating the call*/
static void recovered_call_on_network_switch_in_early_state(LinphoneCoreManager* callerMgr) {
	const LinphoneCallParams *remote_params;
	LinphoneCall *incoming_call;

	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_invite_address(callerMgr->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &callerMgr->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	linphone_core_set_network_reachable(callerMgr->lc, FALSE);
	wait_for(callerMgr->lc, pauline->lc, &callerMgr->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(callerMgr->lc, TRUE);
	wait_for(callerMgr->lc, pauline->lc, &callerMgr->stat.number_of_NetworkReachableTrue, 2);

	BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &callerMgr->stat.number_of_LinphoneCallOutgoingRinging, 2));
	incoming_call = linphone_core_get_current_call(pauline->lc);
	remote_params = linphone_call_get_remote_params(incoming_call);
	BC_ASSERT_PTR_NOT_NULL(remote_params);
	if (remote_params != NULL) {
		const char *replaces_header = linphone_call_params_get_custom_header(remote_params, "Replaces");
		BC_ASSERT_PTR_NOT_NULL(replaces_header);
	}
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &callerMgr->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &callerMgr->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(callerMgr->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:

	linphone_core_manager_destroy(pauline);
}
static void recovered_call_on_network_switch_in_early_state_1(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	recovered_call_on_network_switch_in_early_state(marie);
	linphone_core_manager_destroy(marie);
}
static void recovered_call_on_network_switch_in_early_state_1_udp(void) {
	LinphoneCoreManager* laure = linphone_core_manager_new("laure_rc_udp");
	recovered_call_on_network_switch_in_early_state(laure);
	linphone_core_manager_destroy(laure);
}

/*case where the caller disconnects just after the call is accepted*/
static void recovered_call_on_network_switch_in_early_state_2(void) {
	LinphoneCall *incoming_call;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(marie->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_NetworkReachableTrue, 2);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


/*case where the callee disconnects between the moment it receives the call and the moment it accepts the call*/
static void recovered_call_on_network_switch_in_early_state_3(void) {
	LinphoneCall *incoming_call;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableTrue, 2);

	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*case where the callee disconnects just after accepting the call*/
static void recovered_call_on_network_switch_in_early_state_4(void) {
	LinphoneCall *incoming_call;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableTrue, 2);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	BC_ASSERT_TRUE(sal_call_dialog_request_pending(linphone_call_get_op_as_sal_op(incoming_call)));
	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	BC_ASSERT_FALSE(sal_call_dialog_request_pending(linphone_call_get_op_as_sal_op(incoming_call)));
	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/* This test simulates a socket disconnection (like broken pipe, connection reset by peer etc...) during an incoming call in ringing state,
 * but WITHOUT the network being down/up.
 * This case is unhandled in the library and results in the call being totally lost.
 * Uncomment this test when this is implemented in the library.
 * The issue is tracked by ticket #5802 in bug tracker.
 */
#if 0

static void recovered_call_on_socket_disconnection_in_early_state(void) {
	LinphoneCall *incoming_call;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	
	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;
	
	/*simulate a general socket error*/
	sal_set_recv_error(linphone_core_get_sal(pauline->lc), 0);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationProgress, pauline->stat.number_of_LinphoneRegistrationProgress +1));
	/*restart normal behavior*/
	sal_set_recv_error(linphone_core_get_sal(pauline->lc), 1);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, pauline->stat.number_of_LinphoneRegistrationOk +1));
	
	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	liblinphone_tester_check_rtcp(marie,pauline);
	end_call(marie,pauline);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#endif

static void recovered_call_on_network_switch_during_reinvite_1(void) {
	LinphoneCall *incoming_call;
	LinphoneCall *outgoing_call;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	outgoing_call = linphone_core_get_current_call(marie->lc);
	linphone_call_pause(outgoing_call);
	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(marie->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_NetworkReachableTrue, 2);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 1));
	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void configure_video_policies_for_network_switch(LinphoneCore *marie, LinphoneCore *pauline) {
	LinphoneVideoPolicy policy;
	policy.automatically_accept = FALSE;
	policy.automatically_initiate = FALSE;

	linphone_core_enable_video_capture(marie, TRUE);
	linphone_core_enable_video_display(marie, TRUE);
	linphone_core_enable_video_capture(pauline, TRUE);
	linphone_core_enable_video_display(pauline, TRUE);
	linphone_core_set_video_policy(marie, &policy);
	linphone_core_set_video_policy(pauline, &policy);
	linphone_config_set_int(linphone_core_get_config(pauline), "sip", "defer_update_default", TRUE);
}

static void recovered_call_on_network_switch_during_reinvite_2(void) {
	LinphoneCall *incoming_call;
	LinphoneCall *outgoing_call;
	LinphoneCallParams *params;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	configure_video_policies_for_network_switch(marie->lc, pauline->lc);
	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	outgoing_call = linphone_core_get_current_call(marie->lc);
	params = linphone_core_create_call_params(marie->lc, outgoing_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_update(outgoing_call, params);
	linphone_call_params_unref(params);
	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(marie->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_NetworkReachableTrue, 2);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));
	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	params = linphone_core_create_call_params(pauline->lc, incoming_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_accept_update(incoming_call, params);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void recovered_call_on_network_switch_during_reinvite_3(void) {
	LinphoneCall *incoming_call;
	LinphoneCall *outgoing_call;
	LinphoneCallParams *params;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	configure_video_policies_for_network_switch(marie->lc, pauline->lc);
	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	outgoing_call = linphone_core_get_current_call(marie->lc);
	params = linphone_core_create_call_params(marie->lc, outgoing_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_update(outgoing_call, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableTrue, 2);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	params = linphone_core_create_call_params(marie->lc, outgoing_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_update(outgoing_call, params);
	linphone_call_params_unref(params);
	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	params = linphone_core_create_call_params(pauline->lc, incoming_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_accept_update(incoming_call, params);
	linphone_call_params_unref(params);

	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void recovered_call_on_network_switch_during_reinvite_4(void) {
	LinphoneCall *incoming_call;
	LinphoneCall *outgoing_call;
	LinphoneCallParams *params;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	configure_video_policies_for_network_switch(marie->lc, pauline->lc);
	linphone_core_invite_address(marie->lc, pauline->identity);
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1))) goto end;

	incoming_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	outgoing_call = linphone_core_get_current_call(marie->lc);
	params = linphone_core_create_call_params(marie->lc, outgoing_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_update(outgoing_call, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));

	params = linphone_core_create_call_params(pauline->lc, incoming_call);
	linphone_call_params_enable_video(params, TRUE);
	linphone_call_accept_update(incoming_call, params);
	linphone_call_params_unref(params);
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableTrue, 2);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
	linphone_call_terminate(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void recovered_call_on_network_switch_in_early_media_base (bool_t callerLosesNetwork) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	LinphoneCall *marie_call;
	LinphoneCallLog *marie_call_log;
	uint64_t connected_time = 0;
	uint64_t ended_time = 0;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	/* Marie calls Pauline, and after the call has rung, transitions to an early-media session */
	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	marie_call_log = linphone_call_get_call_log(marie_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 1000));

	if (linphone_core_is_incoming_invite_pending(pauline->lc)) {
		/* Send a 183 to initiate the early media */
		linphone_call_accept_early_media(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 2000) );
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 2000) );
		BC_ASSERT_TRUE(linphone_call_get_all_muted(marie_call));

		liblinphone_tester_check_rtcp(marie, pauline);

		if (callerLosesNetwork) {
			/* Disconnect Marie's network and then reconnect it */
			linphone_core_set_network_reachable(marie->lc, FALSE);
			BC_ASSERT_EQUAL(marie->stat.number_of_NetworkReachableFalse, 1, int, "%d");
			linphone_core_set_network_reachable(marie->lc, TRUE);
			BC_ASSERT_EQUAL(marie->stat.number_of_NetworkReachableTrue, 2, int, "%d");
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));
		} else {
			/* Disconnect Pauline's network and then reconnect it */
			linphone_core_set_network_reachable(pauline->lc, FALSE);
			BC_ASSERT_EQUAL(pauline->stat.number_of_NetworkReachableFalse, 1, int, "%d");
			linphone_core_set_network_reachable(pauline->lc, TRUE);
			BC_ASSERT_EQUAL(pauline->stat.number_of_NetworkReachableTrue, 2, int, "%d");
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 2));
		}

		wait_for_until(marie->lc, pauline->lc, NULL, 1, 2000);
		liblinphone_tester_check_rtcp(marie, pauline);

		if (linphone_core_get_current_call(pauline->lc)
			&& (linphone_call_get_state(linphone_core_get_current_call(pauline->lc)) ==  LinphoneCallIncomingEarlyMedia)
		) {
			linphone_call_accept(linphone_core_get_current_call(pauline->lc));
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 1000));
			connected_time = ms_get_cur_time_ms();
			BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
			BC_ASSERT_PTR_EQUAL(marie_call, linphone_core_get_current_call(marie->lc));
			BC_ASSERT_FALSE(linphone_call_get_all_muted(marie_call));

			liblinphone_tester_check_rtcp(marie, pauline);
			wait_for_list(lcs, 0, 1, 2000); /* Just to have a call duration != 0 */

			end_call(pauline, marie);
			ended_time = ms_get_cur_time_ms();
			BC_ASSERT_LOWER(labs((long)((linphone_call_log_get_duration(marie_call_log) * 1000) - (int64_t)(ended_time - connected_time))), 1500, long, "%ld");
		}
	}

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void recovered_call_on_network_switch_in_early_media_1 (void) {
	recovered_call_on_network_switch_in_early_media_base(TRUE);
}

static void recovered_call_on_network_switch_in_early_media_2 (void) {
	recovered_call_on_network_switch_in_early_media_base(FALSE);
}

static void _call_with_network_switch(bool_t use_ice, bool_t with_socket_refresh, bool_t enable_rtt, bool_t caller_pause, bool_t callee_pause) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_params = NULL;
	LinphoneCall *marie_call = NULL;
	LinphoneCall *pauline_call = NULL;
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	if (use_ice){
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_manager_wait_for_stun_resolution(marie);
		linphone_core_manager_wait_for_stun_resolution(pauline);
	}
	if (with_socket_refresh){
		linphone_config_set_int(linphone_core_get_config(marie->lc), "net", "recreate_sockets_when_network_is_up", 1);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "net", "recreate_sockets_when_network_is_up", 1);
	}
	if (enable_rtt) {
		pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
		linphone_call_params_enable_realtime_text(pauline_params, TRUE);
	}

	BC_ASSERT_TRUE((call_ok=call_with_params(pauline, marie, pauline_params, NULL)));
	if (!call_ok) goto end;

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);
	if (use_ice) {
		/*wait for ICE reINVITE to complete*/
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(check_ice(pauline,marie,LinphoneIceStateHostConnection));
	}

	if (caller_pause) {
		pauline_call = linphone_core_get_current_call(pauline->lc);
		linphone_call_pause(pauline_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);
	} else if (callee_pause) {
		marie_call = linphone_core_get_current_call(marie->lc);
		linphone_call_pause(marie_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 1));
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);
	}

	/*marie looses the network and reconnects*/
	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	/*marie will reconnect and register*/
	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	if (use_ice){
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));
		/*now comes the ICE reINVITE*/
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 4));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 4));
	}else{
		if (caller_pause) {
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 2));
			linphone_call_resume(pauline_call);
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		} else if (callee_pause) {
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 2));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 2));
			linphone_call_resume(marie_call);
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		} else {
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
			BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		}
	}

	/*check that media is back*/
	check_media_direction(marie, linphone_core_get_current_call(marie->lc), lcs, LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionInvalid);
	liblinphone_tester_check_rtcp(pauline, marie);
	if (use_ice) BC_ASSERT_TRUE(check_ice(pauline,marie,LinphoneIceStateHostConnection));

	/*pauline shall be able to end the call without problem now*/
	end_call(pauline, marie);
end:
	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
	}
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_network_switch(void){
	_call_with_network_switch(FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void call_with_network_switch_in_paused_state(void) {
	_call_with_network_switch(FALSE, FALSE, FALSE, FALSE, TRUE);
}

static void call_with_network_switch_in_paused_by_remote_state(void) {
	_call_with_network_switch(FALSE, FALSE, FALSE, TRUE, FALSE);
}

static void call_with_network_switch_and_ice(void){
	_call_with_network_switch(TRUE, FALSE, FALSE, FALSE, FALSE);
}

static void call_with_network_switch_ice_and_rtt(void) {
	_call_with_network_switch(TRUE, FALSE, TRUE, FALSE, FALSE);
}

static void call_with_network_switch_and_socket_refresh(void){
	_call_with_network_switch(TRUE, TRUE, FALSE, FALSE, FALSE);
}

static void call_with_network_switch_no_recovery(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_params = NULL;
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	linphone_core_set_nortp_timeout(marie->lc, 50000);

	BC_ASSERT_TRUE((call_ok=call_with_params(pauline, marie, pauline_params, NULL)));
	if (!call_ok) goto end;

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);

	/*marie looses the network and reconnects*/
	linphone_core_set_network_reachable(marie->lc, FALSE);
	/*but meanwhile pauline terminates the call.*/
	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	/*
	 * We have to wait 32 seconds so that the BYE transaction is terminated, and dialog removed.
	 * This is the condition to receive a 481 when marie sends the reINVITE.*/
	wait_for_list(lcs, NULL, 0, 32500);

	/*marie will reconnect, register, and send an automatic reINVITE to try to repair the call*/
	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
	/*This reINVITE should of course fail, so marie's call should be terminated.*/
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

end:
	if (pauline_params) {
		linphone_call_params_unref(pauline_params);
	}
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_sip_and_rtp_independant_switches(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;
	bool_t use_ice = TRUE;
	bool_t with_socket_refresh = TRUE;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	if (use_ice){
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
	}
	if (with_socket_refresh){
		linphone_config_set_int(linphone_core_get_config(marie->lc), "net", "recreate_sockets_when_network_is_up", 1);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "net", "recreate_sockets_when_network_is_up", 1);
	}

	linphone_core_set_media_network_reachable(marie->lc, TRUE);

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (!call_ok) goto end;

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000);
	if (use_ice) {
		/*wait for ICE reINVITE to complete*/
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(check_ice(pauline,marie,LinphoneIceStateHostConnection));
	}
	/*marie looses the SIP network and reconnects*/
	linphone_core_set_sip_network_reachable(marie->lc, FALSE);
	linphone_core_set_media_network_reachable(marie->lc, FALSE);
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	/*marie will reconnect and register*/
	linphone_core_set_sip_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 5000);
	/*at this stage, no reINVITE is expected to be send*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallUpdating, 0, int, "%i");

	/*now we notify the a reconnection of media network*/
	linphone_core_set_media_network_reachable(marie->lc, TRUE);

	if (use_ice){
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));
		/*now comes the ICE reINVITE*/
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 4));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 4));
	}else{
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	}

	/*check that media is back*/
	check_media_direction(marie, linphone_core_get_current_call(marie->lc), lcs, LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionInvalid);
	liblinphone_tester_check_rtcp(pauline, marie);
	if (use_ice) BC_ASSERT_TRUE(check_ice(pauline,marie,LinphoneIceStateHostConnection));

	/*pauline shall be able to end the call without problem now*/
	end_call(pauline, marie);
end:
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static test_t call_recovery_tests[] = {
	TEST_NO_TAG("Call with network switch", call_with_network_switch),
	TEST_NO_TAG("Call with network switch and no recovery possible", call_with_network_switch_no_recovery),
	TEST_ONE_TAG("Recovered call on network switch in early state 1", recovered_call_on_network_switch_in_early_state_1, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch in early state 1 (udp caller)", recovered_call_on_network_switch_in_early_state_1_udp, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch in early state 2", recovered_call_on_network_switch_in_early_state_2, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch in early state 3", recovered_call_on_network_switch_in_early_state_3, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch in early state 4", recovered_call_on_network_switch_in_early_state_4, "CallRecovery"),
#if 0 /*enable this test when library has support for it*/	
	TEST_ONE_TAG("Recovered call on socket disconnection in early state", recovered_call_on_socket_disconnection_in_early_state, "CallRecovery"),
#endif
	TEST_ONE_TAG("Recovered call on network switch during re-invite 1", recovered_call_on_network_switch_during_reinvite_1, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch during re-invite 2", recovered_call_on_network_switch_during_reinvite_2, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch during re-invite 3", recovered_call_on_network_switch_during_reinvite_3, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch during re-invite 4", recovered_call_on_network_switch_during_reinvite_4, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch in early media 1", recovered_call_on_network_switch_in_early_media_1, "CallRecovery"),
	TEST_ONE_TAG("Recovered call on network switch in early media 2", recovered_call_on_network_switch_in_early_media_2, "CallRecovery"),
	TEST_ONE_TAG("Call with network switch in paused state", call_with_network_switch_in_paused_state, "CallRecovery"),
	TEST_ONE_TAG("Call with network switch in paused by remote state", call_with_network_switch_in_paused_by_remote_state, "CallRecovery"),
	TEST_ONE_TAG("Call with network switch and ICE", call_with_network_switch_and_ice, "ICE"),
	TEST_ONE_TAG("Call with network switch, ICE and RTT", call_with_network_switch_ice_and_rtt, "ICE"),
	TEST_NO_TAG("Call with network switch with socket refresh", call_with_network_switch_and_socket_refresh),
	TEST_NO_TAG("Call with SIP and RTP independant switches", call_with_sip_and_rtp_independant_switches)
};

test_suite_t call_recovery_test_suite = {"Call recovery", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_recovery_tests) / sizeof(call_recovery_tests[0]), call_recovery_tests};
								
