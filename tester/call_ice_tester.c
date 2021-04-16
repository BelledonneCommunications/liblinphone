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




static void call_with_ice_in_ipv4_with_v6_enabled(void) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()){
		marie = linphone_core_manager_new("marie_v4proxy_rc");
		pauline = linphone_core_manager_new("pauline_v4proxy_rc");

		_call_with_ice_base(marie,pauline,TRUE,TRUE,TRUE,TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void call_with_ice_ipv4_to_ipv6(void) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()){
		marie = linphone_core_manager_new("marie_v4proxy_rc");
		pauline = linphone_core_manager_new("pauline_tcp_rc");

		_call_with_ice_base(marie,pauline,TRUE,TRUE,TRUE,TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void call_with_ice_ipv6_to_ipv4(void) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()){
		marie = linphone_core_manager_new("marie_rc");
		pauline = linphone_core_manager_new("pauline_v4proxy_rc");

		_call_with_ice_base(marie, pauline,TRUE,TRUE,TRUE,TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void call_with_ice_ipv6_to_ipv6(void) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()){
		marie = linphone_core_manager_new("marie_rc");
		pauline = linphone_core_manager_new("pauline_tcp_rc");

		_call_with_ice_base(marie, pauline,TRUE,TRUE,TRUE,TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void _early_media_call_with_ice(bool_t callee_has_ice) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *marie_call, *pauline_call;
	LinphoneCallStats *stats1, *stats2;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	if (callee_has_ice) linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);

	pauline_call = linphone_core_invite_address(pauline->lc, marie->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingEarlyMedia,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,3000));
	BC_ASSERT_TRUE(linphone_call_get_all_muted(pauline_call));

	marie_call = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call)) goto end;
	check_media_direction(marie, marie_call, lcs, LinphoneMediaDirectionSendRecv,LinphoneMediaDirectionInvalid);

	wait_for_until(pauline->lc,marie->lc,NULL,0,3000);
	if (callee_has_ice){
		stats1 = linphone_call_get_audio_stats(marie_call);
		stats2 = linphone_call_get_audio_stats(pauline_call);

		BC_ASSERT_TRUE(linphone_call_stats_get_ice_state(stats1) == LinphoneIceStateHostConnection);
		BC_ASSERT_TRUE(linphone_call_stats_get_ice_state(stats2) == LinphoneIceStateHostConnection);

		linphone_call_stats_unref(stats1);
		linphone_call_stats_unref(stats2);
	}

	linphone_call_accept(marie_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_FALSE(linphone_call_get_all_muted(pauline_call));

	end_call(marie, pauline);
end:
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_ice(void){
	/*in this test, pauline has ICE activated, marie not, but marie proposes early media.
	 * We want to check that ICE processing is not disturbing early media,
	 * ICE shall not deactivate itself automatically.*/
	_early_media_call_with_ice(FALSE);
}


static void early_media_call_with_ice_2(void){
	/*in this test, both pauline and marie do ICE, and marie requests early media.
	 Ice shall complete during the early media phase. No reINVITE can be sent.*/
	_early_media_call_with_ice(TRUE);
}

static void audio_call_with_ice_no_matching_audio_codecs(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call;
	const bctbx_list_t *logs;
	LinphoneCallLog *cl;

	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1), FALSE); /* Disable PCMU */
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1), TRUE); /* Enable PCMA */
	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);

	linphone_core_manager_wait_for_stun_resolution(marie);
	linphone_core_manager_wait_for_stun_resolution(pauline);

	out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));

	/* flexisip will retain the 488 until the "urgent reply" timeout arrives. */
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	logs = linphone_core_get_call_logs(pauline->lc);

	BC_ASSERT_EQUAL((int)bctbx_list_size(logs), 1, int, "%d");
	if (logs){
		const LinphoneErrorInfo *ei;
		cl = (LinphoneCallLog*)logs->data;
		BC_ASSERT_EQUAL(linphone_call_log_get_status(cl), LinphoneCallEarlyAborted, int, "%d");
		BC_ASSERT_TRUE(linphone_call_log_get_start_date(cl) != 0);
		ei = linphone_call_log_get_error_info(cl);
		BC_ASSERT_PTR_NOT_NULL(ei);
		if (ei){
			BC_ASSERT_EQUAL(linphone_error_info_get_reason(ei), LinphoneReasonNotAcceptable, int, "%d");
		}
	}

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void dtls_srtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,FALSE,LinphonePolicyUseIce,FALSE);
}

static void srtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionSRTP,FALSE,FALSE,LinphonePolicyUseIce,FALSE);
}

static void zrtp_ice_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionZRTP,FALSE,TRUE,LinphonePolicyUseIce,FALSE);
}

static void dtls_ice_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,TRUE,LinphonePolicyUseIce,FALSE);
}

static void zrtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyUseIce,FALSE);
}

static void call_with_ice_and_rtcp_mux(void){
	_call_with_rtcp_mux(TRUE, TRUE, TRUE,TRUE);
}

static void call_with_ice_and_rtcp_mux_without_reinvite(void){
	_call_with_rtcp_mux(TRUE, TRUE, TRUE,FALSE);
}

static bool_t is_matching_a_local_address(const char *ip, const bctbx_list_t *addresses){
	if (strlen(ip)==0) return FALSE;
	for (; addresses != NULL; addresses = addresses->next){
		if (strcmp(ip, (const char*)addresses->data) == 0) return TRUE;
	}
	return FALSE;
}

/*
 * this test checks the 'dont_default_to_stun_candidates' mode, where the c= line is left to host
 * ip instead of stun candidate when ice is enabled*/
static void _call_with_ice_with_default_candidate_not_stun(bool_t with_ipv6_prefered){
	LinphoneCoreManager * marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char localip[LINPHONE_IPADDR_SIZE]={0};
	char localip6[LINPHONE_IPADDR_SIZE]={0};
	bctbx_list_t *local_addresses = linphone_fetch_local_addresses();
	LinphoneCall *pauline_call, *marie_call;

	linphone_config_set_int(linphone_core_get_config(marie->lc), "net", "dont_default_to_stun_candidates", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "prefer_ipv6", (int)with_ipv6_prefered);
	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	linphone_core_get_local_ip(marie->lc, AF_INET, NULL, localip);
	linphone_core_get_local_ip(marie->lc, AF_INET6, NULL, localip6);
	
	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	
	if (marie_call && pauline_call){
		linphone_call_accept(pauline_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		/*check immmediately that the offer has a c line with the expected family.*/
		if (with_ipv6_prefered){
			/* the address in the c line shall be an ipv6 one.*/
			BC_ASSERT_TRUE(strchr(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))->addr, ':') != NULL);
		}else{
			BC_ASSERT_TRUE(strchr(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))->addr, ':') == NULL);
		}
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
		BC_ASSERT_TRUE(is_matching_a_local_address(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))->addr, local_addresses));
		BC_ASSERT_TRUE(is_matching_a_local_address(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))->streams[0].rtp_addr, local_addresses));
		BC_ASSERT_TRUE(is_matching_a_local_address(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))->streams[0].rtp_addr, local_addresses));
		BC_ASSERT_TRUE(is_matching_a_local_address(_linphone_call_get_result_desc(linphone_core_get_current_call(pauline->lc))->streams[0].rtp_addr, local_addresses)
				|| is_matching_a_local_address(_linphone_call_get_result_desc(linphone_core_get_current_call(pauline->lc))->addr, local_addresses)
		);
	}
	end_call(marie, pauline);
	bctbx_list_free_with_data(local_addresses, bctbx_free);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_with_default_candidate_not_stun(void){
	_call_with_ice_with_default_candidate_not_stun(TRUE);
}

static void call_with_ice_with_default_candidate_not_stun_ipv4_prefered(void){
	_call_with_ice_with_default_candidate_not_stun(FALSE);
}

static void call_with_ice_without_stun(void){
	LinphoneCoreManager * marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_stun_server(marie->lc, NULL);
	linphone_core_set_stun_server(pauline->lc, NULL);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_without_stun2(void){
	LinphoneCoreManager * marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	//linphone_core_set_stun_server(marie->lc, NULL);
	linphone_core_set_stun_server(pauline->lc, NULL);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_stun_not_responding(void){
	LinphoneCoreManager * marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/*set dummy stun servers*/
	linphone_core_set_stun_server(marie->lc, "belledonne-communications.com:443");
	linphone_core_set_stun_server(pauline->lc, "belledonne-communications.com:443");
	/*we expect ICE to continue without stun candidates*/
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE);

	/*retry but with nat policy instead of core */
	linphone_core_set_stun_server(marie->lc, NULL);
	linphone_core_set_stun_server(pauline->lc, NULL);
	linphone_nat_policy_set_stun_server(linphone_core_get_nat_policy(marie->lc), "belledonne-communications.com:443");
	linphone_nat_policy_set_stun_server(linphone_core_get_nat_policy(pauline->lc), "belledonne-communications.com:443");
	/*we expect ICE to continue without stun candidates*/
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void _call_with_ice(bool_t caller_with_ice, bool_t callee_with_ice, bool_t random_ports, bool_t forced_relay, bool_t ipv6) {
	LinphoneCoreManager* marie = linphone_core_manager_new2("marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (ipv6) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	_call_with_ice_base(pauline,marie,caller_with_ice,callee_with_ice,random_ports,forced_relay);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice(void){
	_call_with_ice(TRUE,TRUE,FALSE,FALSE,FALSE);
}

static void call_with_ice_ipv6(void) {
	if (liblinphone_tester_ipv6_available()) {
		_call_with_ice(TRUE, TRUE, FALSE, FALSE, TRUE);
	} else {
		ms_warning("Test skipped, no ipv6 available");
	}
}

static void call_with_ice_ufrag_and_password_set_in_sdp_m_line(void){
	LinphoneCoreManager* marie = linphone_core_manager_new2("marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (liblinphone_tester_ipv6_available()) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "ice_password_ufrag_in_media_description", 1);
	
	_call_with_ice_base(pauline,marie,TRUE,TRUE,TRUE,FALSE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_ufrag_and_password_set_in_sdp_m_line_2(void){
	LinphoneCoreManager* marie = linphone_core_manager_new2("marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (liblinphone_tester_ipv6_available()) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "ice_password_ufrag_in_media_description", 1);
	
	_call_with_ice_base(pauline,marie,TRUE,TRUE,TRUE,FALSE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_ufrag_and_password_set_in_sdp_m_line_3(void){
	LinphoneCoreManager* marie = linphone_core_manager_new2("marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (liblinphone_tester_ipv6_available()) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "ice_password_ufrag_in_media_description", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "ice_password_ufrag_in_media_description", 1);
	
	_call_with_ice_base(pauline,marie,TRUE,TRUE,TRUE,FALSE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*ICE is not expected to work in this case, however this should not crash.
 Updated 08/01/2020: now ICE works also in the case of an INVITE without SDP.
 */
static void call_with_ice_no_sdp(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_sdp_200_ack(pauline->lc,TRUE);

	linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);

	linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);

	BC_ASSERT_TRUE(call(pauline,marie));

	liblinphone_tester_check_rtcp(marie,pauline);

	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_random_ports(void){
	_call_with_ice(TRUE,TRUE,TRUE,FALSE,FALSE);
}

static void call_with_ice_forced_relay(void) {
	_call_with_ice(TRUE, TRUE, TRUE, TRUE, FALSE);
}

static void ice_to_not_ice(void){
	_call_with_ice(TRUE,FALSE,FALSE,FALSE,FALSE);
}

static void not_ice_to_ice(void){
	_call_with_ice(FALSE,TRUE,FALSE,FALSE,FALSE);
}

static void ice_added_by_reinvite(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneNatPolicy *pol;
	LinphoneCallParams *params;
	LinphoneCall *c;
	LinphoneProxyConfig *cfg;
	bool_t call_ok;

	linphone_config_set_int(linphone_core_get_config(marie->lc), "net", "allow_late_ice", 1);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "net", "allow_late_ice", 1);

	BC_ASSERT_TRUE((call_ok=call(pauline,marie)));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(marie,pauline);

	/*enable ICE on both ends*/
	pol = linphone_core_get_nat_policy(marie->lc);
	linphone_nat_policy_enable_ice(pol, TRUE);
	linphone_nat_policy_enable_stun(pol, TRUE);
	linphone_core_set_nat_policy(marie->lc, pol);

	pol = linphone_core_get_nat_policy(pauline->lc);
	linphone_nat_policy_enable_ice(pol, TRUE);
	linphone_nat_policy_enable_stun(pol, TRUE);
	linphone_core_set_nat_policy(pauline->lc, pol);

	linphone_core_manager_wait_for_stun_resolution(marie);
	linphone_core_manager_wait_for_stun_resolution(pauline);

	c = linphone_core_get_current_call(marie->lc);
	params = linphone_core_create_call_params(marie->lc, c);
	linphone_call_update(c, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallUpdatedByRemote,1));

	/*wait for the ICE reINVITE*/
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,3));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,3));
	BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));

	end_call(pauline, marie);

	/* Opportunistic test: perform a set_network_reachable FALSE/TRUE and call stop() immediately after.
	 * It was crashing because the DNS resolution of stun server wasn't cancelled before the main loop is destroyed.
	 */
	cfg = linphone_core_get_default_proxy_config(pauline->lc);
	linphone_proxy_config_edit(cfg);
	linphone_proxy_config_enable_register(cfg, FALSE);
	linphone_proxy_config_done(cfg);
	
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneRegistrationCleared,1));
	
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	linphone_core_stop(pauline->lc);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_early_media_ice_and_no_sdp_in_200(void){
	early_media_without_sdp_in_200_base(FALSE, TRUE);
}


static test_t call_with_ice_tests[] = {
	TEST_ONE_TAG("Call with ICE in IPv4 with IPv6 enabled", call_with_ice_in_ipv4_with_v6_enabled, "ICE"),
	TEST_ONE_TAG("Call with ICE IPv4 to IPv6", call_with_ice_ipv4_to_ipv6, "ICE"),
	TEST_ONE_TAG("Call with ICE IPv6 to IPv4", call_with_ice_ipv6_to_ipv4, "ICE"),
	TEST_ONE_TAG("Call with ICE IPv6 to IPv6", call_with_ice_ipv6_to_ipv6, "ICE"),
	TEST_ONE_TAG("Early-media call with ICE", early_media_call_with_ice, "ICE"),
	TEST_ONE_TAG("Early-media call with ICE 2", early_media_call_with_ice_2, "ICE"),
	TEST_ONE_TAG("Audio call with ICE no matching audio codecs", audio_call_with_ice_no_matching_audio_codecs, "ICE"),
	TEST_ONE_TAG("SRTP ice call", srtp_ice_call, "ICE"),
	TEST_ONE_TAG("ZRTP ice call", zrtp_ice_call, "ICE"),
	TEST_ONE_TAG("ZRTP ice call with relay", zrtp_ice_call_with_relay, "ICE"),
	TEST_TWO_TAGS("DTLS SRTP ice call", dtls_srtp_ice_call, "ICE", "DTLS"),
	TEST_TWO_TAGS("DTLS ice call with relay", dtls_ice_call_with_relay, "ICE", "DTLS"),
	TEST_ONE_TAG("Call with ICE", call_with_ice, "ICE"),
	TEST_ONE_TAG("Call with ICE IPv6", call_with_ice_ipv6, "ICE"),
	TEST_ONE_TAG("Call with ICE without SDP", call_with_ice_no_sdp, "ICE"),
	TEST_ONE_TAG("Call with ICE (random ports)", call_with_ice_random_ports, "ICE"),
	TEST_ONE_TAG("Call with ICE (forced relay)", call_with_ice_forced_relay, "ICE"),
	TEST_ONE_TAG("Call from ICE to not ICE", ice_to_not_ice, "ICE"),
	TEST_ONE_TAG("Call from not ICE to ICE", not_ice_to_ice, "ICE"),
	TEST_ONE_TAG("Call with ICE added by reINVITE", ice_added_by_reinvite, "ICE"),
	TEST_ONE_TAG("Call with ICE and no SDP in 200 OK", call_with_early_media_ice_and_no_sdp_in_200, "ICE"),
	TEST_ONE_TAG("Call with ICE and rtcp-mux", call_with_ice_and_rtcp_mux, "ICE"),
	TEST_ONE_TAG("Call with ICE and rtcp-mux without ICE re-invite", call_with_ice_and_rtcp_mux_without_reinvite, "ICE"),
	TEST_ONE_TAG("Call with ICE with default candidate not stun", call_with_ice_with_default_candidate_not_stun, "ICE"),
	TEST_ONE_TAG("Call with ICE with default candidate not stun and ipv4 prefered", call_with_ice_with_default_candidate_not_stun_ipv4_prefered, "ICE"),
	TEST_ONE_TAG("Call with ICE without stun server", call_with_ice_without_stun, "ICE"),
	TEST_ONE_TAG("Call with ICE without stun server one side", call_with_ice_without_stun2, "ICE"),
	TEST_ONE_TAG("Call with ICE and stun server not responding", call_with_ice_stun_not_responding, "ICE"),
	TEST_ONE_TAG("Call with ICE ufrag and password set in SDP m line", call_with_ice_ufrag_and_password_set_in_sdp_m_line, "ICE"),
	TEST_ONE_TAG("Call with ICE ufrag and password set in SDP m line 2", call_with_ice_ufrag_and_password_set_in_sdp_m_line_2, "ICE"),
	TEST_ONE_TAG("Call with ICE ufrag and password set in SDP m line 3", call_with_ice_ufrag_and_password_set_in_sdp_m_line_3, "ICE")
};

test_suite_t call_with_ice_test_suite = {"Call with ICE", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_with_ice_tests) / sizeof(call_with_ice_tests[0]), call_with_ice_tests};
