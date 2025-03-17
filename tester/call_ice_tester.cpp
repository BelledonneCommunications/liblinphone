/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include <sys/stat.h>
#include <sys/types.h>

#include "bctoolbox/defs.h"

#include "belle-sip/sipstack.h"

#include "mediastreamer2/msutils.h"

#include "c-wrapper/c-wrapper.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-nat-policy.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "sal/sal_media_description.h"
#include "sal/sal_stream_description.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

static void call_with_ice_in_ipv4_with_v6_enabled(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()) {
		marie = linphone_core_manager_new("marie_v4proxy_rc");
		pauline = linphone_core_manager_new("pauline_v4proxy_rc");

		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, FALSE);
		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void call_with_ice_ipv4_to_ipv6(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()) {
		marie = linphone_core_manager_new("marie_v4proxy_rc");
		pauline = linphone_core_manager_new("pauline_tcp_rc");

		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, FALSE);
		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void call_with_ice_ipv6_to_ipv4(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()) {
		marie = linphone_core_manager_new("marie_rc");
		pauline = linphone_core_manager_new("pauline_v4proxy_rc");

		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, FALSE);
		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void call_with_ice_ipv6_to_ipv6(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	if (liblinphone_tester_ipv4_available() && liblinphone_tester_ipv6_available()) {
		marie = linphone_core_manager_new("marie_rc");
		pauline = linphone_core_manager_new("pauline_tcp_rc");

		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, FALSE);
		_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, TRUE, TRUE);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
	} else ms_warning("Test skipped, need both ipv6 and v4 available");
}

static void _early_media_call_with_ice(bool_t callee_has_ice) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *marie_call, *pauline_call;
	LinphoneCallStats *stats1, *stats2;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);

	if (callee_has_ice) {
		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, FALSE);
	}

	pauline_call = linphone_core_invite_address(pauline->lc, marie->identity);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 10000));
	BC_ASSERT_TRUE(linphone_call_get_all_muted(pauline_call));

	marie_call = linphone_core_get_current_call(marie->lc);

	if (!BC_ASSERT_PTR_NOT_NULL(marie_call)) goto end;
	check_media_direction(marie, marie_call, lcs, LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionInvalid);

	wait_for_until(pauline->lc, marie->lc, NULL, 0, 3000);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_call))
		goto end; // Second test to ensire that marie is not lost in previous iterations
	if (callee_has_ice) {
		stats1 = linphone_call_get_audio_stats(marie_call);
		stats2 = linphone_call_get_audio_stats(pauline_call);

		BC_ASSERT_TRUE(linphone_call_stats_get_ice_state(stats1) == LinphoneIceStateHostConnection);
		BC_ASSERT_TRUE(linphone_call_stats_get_ice_state(stats2) == LinphoneIceStateHostConnection);

		linphone_call_stats_unref(stats1);
		linphone_call_stats_unref(stats2);
	}

	linphone_call_accept(marie_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
	BC_ASSERT_FALSE(linphone_call_get_all_muted(pauline_call));

	end_call(marie, pauline);
end:
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void early_media_call_with_ice(void) {
	/*in this test, pauline has ICE activated, marie not, but marie proposes early media.
	 * We want to check that ICE processing is not disturbing early media,
	 * ICE shall not deactivate itself automatically.*/
	_early_media_call_with_ice(FALSE);
}

static void early_media_call_with_ice_2(void) {
	/*in this test, both pauline and marie do ICE, and marie requests early media.
	 Ice shall complete during the early media phase. No reINVITE can be sent.*/
	_early_media_call_with_ice(TRUE);
}

static void audio_call_with_ice_no_matching_audio_codecs(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *out_call;
	const bctbx_list_t *logs;
	LinphoneCallLog *cl;

	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1),
	                                  FALSE); /* Disable PCMU */
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1),
	                                  TRUE); /* Enable PCMA */

	enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, FALSE);
	enable_stun_in_mgr(pauline, TRUE, TRUE, FALSE, FALSE);

	out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));

	/* flexisip will retain the 488 until the "urgent reply" timeout arrives. */
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	logs = linphone_core_get_call_logs(pauline->lc);

	BC_ASSERT_EQUAL((int)bctbx_list_size(logs), 1, int, "%d");
	if (logs) {
		const LinphoneErrorInfo *ei;
		cl = (LinphoneCallLog *)logs->data;
		BC_ASSERT_EQUAL(linphone_call_log_get_status(cl), LinphoneCallEarlyAborted, int, "%d");
		BC_ASSERT_TRUE(linphone_call_log_get_start_date(cl) != 0);
		ei = linphone_call_log_get_error_info(cl);
		BC_ASSERT_PTR_NOT_NULL(ei);
		if (ei) {
			BC_ASSERT_EQUAL(linphone_error_info_get_reason(ei), LinphoneReasonNotAcceptable, int, "%d");
		}
	}

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void dtls_srtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE, LinphonePolicyUseIce, FALSE);
}

static void srtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE, LinphonePolicyUseIce, FALSE);
}

static void zrtp_ice_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionZRTP, FALSE, TRUE, LinphonePolicyUseIce, FALSE);
}

static void dtls_ice_call_with_relay(void) {
	call_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE, LinphonePolicyUseIce, FALSE);
}

static void zrtp_ice_call(void) {
	call_base(LinphoneMediaEncryptionZRTP, FALSE, FALSE, LinphonePolicyUseIce, FALSE);
}

static void call_with_ice_and_rtcp_mux(void) {
	_call_with_rtcp_mux(TRUE, TRUE, TRUE, TRUE);
}

static void call_with_ice_and_rtcp_mux_without_reinvite(void) {
	_call_with_rtcp_mux(TRUE, TRUE, TRUE, FALSE);
}

static void call_with_ice_and_rtcp_mux_not_accepted(void) {
	_call_with_rtcp_mux(TRUE, FALSE, TRUE, TRUE);
}

static bool_t is_matching_a_local_address(const std::string &ip, const bctbx_list_t *addresses) {
	if (ip.empty()) return FALSE;
	for (; addresses != NULL; addresses = addresses->next) {
		if (ip.compare((const char *)addresses->data) == 0) return TRUE;
	}
	return FALSE;
}

static bool assert_ice_candidate_presence(const LinphonePrivate::SalMediaDescription *md,
                                          const std::string &type,
                                          const std::string &addr) {
	const LinphonePrivate::SalStreamDescription &st = md->getStreamAtIdx(0);
	for (const auto &candidate : st.getIceCandidates()) {
		if (candidate.addr == addr && candidate.type == type) return true;
	}
	return false;
}

/*
 * this test setups hardcoded non working server-reflexive address, to simulate what would happen in a server that is
 * behind a nat.
 */
static void call_with_configured_sflrx_addresses(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *pauline_call, *marie_call;
	const char *mariev4 = "5.135.31.160";
	const char *mariev6 = "2001:41d0:303:3aee::1";
	const char *paulinev4 = "5.135.31.161";
	const char *paulinev6 = "2001:41d0:303:3aee::2";

	LinphoneNatPolicy *pol = linphone_core_create_nat_policy(marie->lc);
	linphone_nat_policy_enable_ice(pol, TRUE);
	linphone_nat_policy_set_nat_v4_address(pol, mariev4);
	linphone_nat_policy_set_nat_v6_address(pol, mariev6);
	linphone_core_set_nat_policy(marie->lc, pol);
	linphone_nat_policy_unref(pol);

	pol = linphone_core_create_nat_policy(pauline->lc);
	linphone_nat_policy_enable_ice(pol, TRUE);
	linphone_nat_policy_set_nat_v4_address(pol, paulinev4);
	linphone_nat_policy_set_nat_v6_address(pol, paulinev6);
	linphone_core_set_nat_policy(pauline->lc, pol);
	linphone_nat_policy_unref(pol);

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);

	if (marie_call && pauline_call) {
		linphone_call_accept(pauline_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		/*check immmediately that the offer and answer have expected sflrx candidates.*/
		BC_ASSERT_TRUE(assert_ice_candidate_presence(_linphone_call_get_remote_desc(pauline_call), "srflx", mariev4));
		BC_ASSERT_TRUE(assert_ice_candidate_presence(_linphone_call_get_remote_desc(marie_call), "srflx", paulinev4));
		if (liblinphone_tester_ipv6_available()) {
			BC_ASSERT_TRUE(
			    assert_ice_candidate_presence(_linphone_call_get_remote_desc(pauline_call), "srflx", mariev6));
			BC_ASSERT_TRUE(
			    assert_ice_candidate_presence(_linphone_call_get_remote_desc(marie_call), "srflx", paulinev6));
		}

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
	}
	end_call(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * this test checks that default candidates are correct (c= line shall be set to the ICE default candidate).
 */
static void _call_with_ice_with_default_candidate(bool_t dont_default_to_stun_candidates, bool_t with_ipv6_prefered) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *pauline_call, *marie_call;

	linphone_config_set_int(linphone_core_get_config(marie->lc), "net", "dont_default_to_stun_candidates",
	                        (int)dont_default_to_stun_candidates);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "prefer_ipv6", (int)with_ipv6_prefered);

	enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, FALSE);
	enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, FALSE);

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);

	if (marie_call && pauline_call) {
		linphone_call_accept(pauline_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		/*check immmediately that the offer has a c line with the expected family.*/
		if (dont_default_to_stun_candidates) {
			/* the address in the c line shall be of requested type */
			if (with_ipv6_prefered && liblinphone_tester_ipv6_available()) {
				BC_ASSERT_TRUE(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))
				                   ->getConnectionAddress()
				                   .find(':') != std::string::npos);
			} else {
				BC_ASSERT_TRUE(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))
				                   ->getConnectionAddress()
				                   .find(':') == std::string::npos);
			}
		} else {
			BC_ASSERT_TRUE(_linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))
			                   ->getStreamAtIdx(0)
			                   .getRtpAddress()
			                   .find(':') == std::string::npos);
		}
		liblinphone_tester_check_ice_default_candidates(
		    marie, dont_default_to_stun_candidates ? TesterIceCandidateHost : TesterIceCandidateSflrx, pauline,
		    TesterIceCandidateSflrx);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
	}
	end_call(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_with_default_candidate_not_stun(void) {
	_call_with_ice_with_default_candidate(TRUE, TRUE);
}

static void call_with_ice_with_default_candidate_not_stun_ipv4_prefered(void) {
	_call_with_ice_with_default_candidate(TRUE, FALSE);
}

static void call_with_ice_with_default_candidate_relay_or_stun(void) {
	_call_with_ice_with_default_candidate(FALSE, FALSE);
}

static void call_with_ice_without_stun(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_set_stun_server(marie->lc, NULL);
	linphone_core_set_stun_server(pauline->lc, NULL);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_without_stun2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	// linphone_core_set_stun_server(marie->lc, NULL);
	linphone_core_set_stun_server(pauline->lc, NULL);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_stun_not_responding(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	bctbx_list_t *mgrList = NULL;
	mgrList = bctbx_list_append(mgrList, marie);
	mgrList = bctbx_list_append(mgrList, pauline);

	for (const bctbx_list_t *mgr_it = mgrList; mgr_it != NULL; mgr_it = mgr_it->next) {
		LinphoneCoreManager *mgr = (LinphoneCoreManager *)(bctbx_list_get_data(mgr_it));
		linphone_core_set_stun_server(mgr->lc, NULL);
		const bctbx_list_t *accounts = linphone_core_get_account_list(mgr->lc);
		for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
			LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
			const LinphoneAccountParams *account_params = linphone_account_get_params(account);
			LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
			linphone_account_params_set_nat_policy(new_account_params, NULL); // Force to use core policy
			linphone_account_set_params(account, new_account_params);
			linphone_account_params_unref(new_account_params);
		}
	}

	const char *stun_server = "belledonne-communications.com:443";

	/*set dummy stun servers*/
	linphone_core_set_stun_server(marie->lc, stun_server);
	linphone_core_set_stun_server(pauline->lc, stun_server);

	/*we expect ICE to continue without stun candidates*/
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, FALSE);

	/*retry but with nat policy instead of core */
	for (const bctbx_list_t *mgr_it = mgrList; mgr_it != NULL; mgr_it = mgr_it->next) {
		LinphoneCoreManager *mgr = (LinphoneCoreManager *)(bctbx_list_get_data(mgr_it));
		linphone_core_set_stun_server(mgr->lc, NULL);
		linphone_nat_policy_set_stun_server(linphone_core_get_nat_policy(mgr->lc), "belledonne-communications.com:443");
	}

	/*we expect ICE to continue without stun candidates*/
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(marie, pauline, TRUE, TRUE, TRUE, FALSE, TRUE);

	bctbx_list_free(mgrList);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void
_call_with_ice(bool_t caller_with_ice, bool_t callee_with_ice, bool_t random_ports, bool_t forced_relay, bool_t ipv6) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check(
	    transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (ipv6) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	_call_with_ice_base(pauline, marie, caller_with_ice, callee_with_ice, random_ports, forced_relay, FALSE);
	_call_with_ice_base(pauline, marie, caller_with_ice, callee_with_ice, random_ports, forced_relay, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice(void) {
	_call_with_ice(TRUE, TRUE, FALSE, FALSE, FALSE);
}

static void call_with_ice_ipv6(void) {
	if (liblinphone_tester_ipv6_available()) {
		_call_with_ice(TRUE, TRUE, FALSE, FALSE, TRUE);
	} else {
		ms_warning("Test skipped, no ipv6 available");
	}
}

static void call_with_ice_ufrag_and_password_set_in_sdp_m_line(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check(
	    transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (liblinphone_tester_ipv6_available()) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "ice_password_ufrag_in_media_description", 1);

	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_ufrag_and_password_set_in_sdp_m_line_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check(
	    transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (liblinphone_tester_ipv6_available()) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "ice_password_ufrag_in_media_description", 1);

	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_ufrag_and_password_set_in_sdp_m_line_3(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new_with_proxies_check(
	    transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	if (liblinphone_tester_ipv6_available()) {
		linphone_core_enable_ipv6(marie->lc, TRUE);
		linphone_core_enable_ipv6(pauline->lc, TRUE);
	}
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "ice_password_ufrag_in_media_description", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "ice_password_ufrag_in_media_description", 1);

	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, FALSE);
	_call_with_ice_base(pauline, marie, TRUE, TRUE, TRUE, FALSE, TRUE);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*ICE is not expected to work in this case, however this should not crash.
 Updated 08/01/2020: now ICE works also in the case of an INVITE without SDP.
 */
static void call_with_ice_no_sdp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_core_enable_sdp_200_ack(pauline->lc, TRUE);

	enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, FALSE);
	enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, FALSE);

	BC_ASSERT_TRUE(call(pauline, marie));

	liblinphone_tester_check_rtcp(marie, pauline);

	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_random_ports(void) {
	_call_with_ice(TRUE, TRUE, TRUE, FALSE, FALSE);
}

static void call_with_ice_forced_relay(void) {
	_call_with_ice(TRUE, TRUE, TRUE, TRUE, FALSE);
}

static void ice_to_not_ice(void) {
	_call_with_ice(TRUE, FALSE, FALSE, FALSE, FALSE);
}

static void not_ice_to_ice(void) {
	_call_with_ice(FALSE, TRUE, FALSE, FALSE, FALSE);
}

static void ice_added_by_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *params;
	LinphoneCall *c;
	LinphoneProxyConfig *cfg;
	bool_t call_ok;

	linphone_config_set_int(linphone_core_get_config(marie->lc), "net", "allow_late_ice", 1);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "net", "allow_late_ice", 1);

	BC_ASSERT_TRUE((call_ok = call(pauline, marie)));
	if (!call_ok) goto end;
	liblinphone_tester_check_rtcp(marie, pauline);

	/*enable ICE on both ends*/
	enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
	enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);

	c = linphone_core_get_current_call(marie->lc);
	params = linphone_core_create_call_params(marie->lc, c);
	linphone_call_update(c, params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));

	/*wait for the ICE reINVITE*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));
	BC_ASSERT_TRUE(check_ice(marie, pauline, LinphoneIceStateHostConnection));

	end_call(pauline, marie);

	/* Opportunistic test: perform a set_network_reachable FALSE/TRUE and call stop() immediately after.
	 * It was crashing because the DNS resolution of stun server wasn't cancelled before the main loop is destroyed.
	 */
	cfg = linphone_core_get_default_proxy_config(pauline->lc);
	linphone_proxy_config_edit(cfg);
	linphone_proxy_config_enable_register(cfg, FALSE);
	linphone_proxy_config_done(cfg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneRegistrationCleared, 1));

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	linphone_core_stop(pauline->lc);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_early_media_ice_and_no_sdp_in_200(void) {
	early_media_without_sdp_in_200_base(FALSE, TRUE);
}

static void call_paused_resumed_with_ice(LinphoneMediaEncryption encryption,
                                         bool_t caller_ice,
                                         bool_t callee_ice,
                                         bool_t enable_rtcp_mux) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *call_pauline = NULL;
	LinphoneCall *call_marie = NULL;

	bool_t call_ok;

	if (caller_ice) {
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	}
	if (callee_ice) {
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	}

	if (linphone_core_media_encryption_supported(marie->lc, encryption)) {
		linphone_core_set_media_encryption(marie->lc, encryption);
		if (encryption == LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a
			                                                directory to store them */
			char *path = bc_tester_file("certificates-marie");
			linphone_core_set_user_certificates_path(marie->lc, path);
			bc_free(path);
			bctbx_mkdir(linphone_core_get_user_certificates_path(marie->lc));
		}
	}

	if (linphone_core_media_encryption_supported(pauline->lc, encryption)) {
		linphone_core_set_media_encryption(pauline->lc, encryption);
		if (encryption == LinphoneMediaEncryptionDTLS) { /* for DTLS we must access certificates or at least have a
			                                                directory to store them */
			char *path = bc_tester_file("certificates-pauline");
			linphone_core_set_user_certificates_path(pauline->lc, path);
			bc_free(path);
			bctbx_mkdir(linphone_core_get_user_certificates_path(pauline->lc));
		}
	}

	if (enable_rtcp_mux) {
		linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_mux", 1);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_mux", 1);
	}

	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));

	if (!call_ok) goto end;

	call_marie = linphone_core_get_current_call(marie->lc);
	linphone_call_ref(call_marie);
	if (caller_ice) {
		BC_ASSERT_TRUE(check_ice_sdp(call_marie));
	} else {
		BC_ASSERT_FALSE(check_ice_sdp(call_marie));
	}

	call_pauline = linphone_core_get_current_call(pauline->lc);
	linphone_call_ref(call_pauline);

	wait_for_until(pauline->lc, marie->lc, NULL, 5, 3000);

	linphone_call_pause(call_pauline);
	if (caller_ice && callee_ice) {
		BC_ASSERT_TRUE(check_ice_sdp(call_pauline));
	} else {
		BC_ASSERT_FALSE(check_ice_sdp(call_pauline));
	}
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

	/*stay in pause a little while in order to generate traffic*/
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	linphone_call_resume(call_pauline);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;

	end_call(pauline, marie);
end:
	if (call_pauline) linphone_call_unref(call_pauline);
	if (call_marie) linphone_call_unref(call_marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_paused_resumed_with_caller_ice() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionNone, TRUE, FALSE, FALSE);
}

static void call_paused_resumed_with_callee_ice() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionNone, FALSE, TRUE, FALSE);
}

static void call_paused_resumed_with_both_ice() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionNone, TRUE, TRUE, FALSE);
}

static void call_paused_resumed_with_caller_ice_and_rtcp_mux() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionNone, TRUE, FALSE, TRUE);
}

static void call_paused_resumed_with_callee_ice_and_rtcp_mux() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionNone, FALSE, TRUE, TRUE);
}

static void call_paused_resumed_with_both_ice_and_rtcp_mux() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionNone, TRUE, TRUE, TRUE);
}

static void dtls_srtp_call_paused_resumed_with_caller_ice_and_rtcp_mux() {
#if 0
	call_paused_resumed_with_ice(LinphoneMediaEncryptionDTLS, TRUE, FALSE, TRUE);
#else
	BC_PASS("Test disabled until https://bugs.linphone.org/view.php?id=9288 is fixed");
#endif
}

static void dtls_srtp_call_paused_resumed_with_callee_ice_and_rtcp_mux() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionDTLS, FALSE, TRUE, TRUE);
}

static void dtls_srtp_call_paused_resumed_with_both_ice_and_rtcp_mux() {
	call_paused_resumed_with_ice(LinphoneMediaEncryptionDTLS, TRUE, TRUE, TRUE);
}

static void call_terminated_during_ice_reinvite(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_sips_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *pauline_call, *marie_call;

	enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
	enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);

	if (marie_call && pauline_call) {
		linphone_call_accept(pauline_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		/*wait for ICE reINVITE to be received */
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1));
		/* let the callee answer the reINVITE */
		BC_ASSERT_TRUE(wait_for(NULL, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		/* callee now sends a reINVITE now */
		linphone_call_pause(pauline_call);
		/* Wait for the INVITE to be challenged by proxy and restarted by pauline */
		wait_for_until(NULL, pauline->lc, NULL, 0, 5000);
		/* Caller sends BYE but the 200OK is not received yet. */
		linphone_call_terminate(marie_call);
		/* The 200 ok will be received and processed by marie, followed by the INVITE(pause) */

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

		BC_ASSERT_TRUE(marie->stat.number_of_LinphoneCallStreamsRunning == 1);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_ice_and_dual_stack_stun_server(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *local_addresses = linphone_fetch_local_addresses();
	LinphoneCall *pauline_call, *marie_call;

	LinphoneNatPolicy *pol = linphone_core_get_nat_policy(marie->lc);
	pol = linphone_nat_policy_clone(pol);
	linphone_nat_policy_set_stun_server(pol, "sip.example.org"); /* this host has ipv4 and ipv6 address.*/
	linphone_nat_policy_enable_stun(pol, TRUE);
	linphone_nat_policy_enable_ice(pol, TRUE);
	linphone_core_set_nat_policy(marie->lc, pol);
	linphone_nat_policy_unref(pol);

	const bctbx_list_t *accounts = linphone_core_get_account_list(marie->lc);
	for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
		LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
		linphone_account_params_set_nat_policy(new_account_params, NULL);
		linphone_account_set_params(account, new_account_params);
		linphone_account_params_unref(new_account_params);
	}
	linphone_core_manager_wait_for_stun_resolution(marie);

	enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);

	marie_call = linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	pauline_call = linphone_core_get_current_call(pauline->lc);

	if (marie_call && pauline_call) {
		std::string paulineConnectionAddress =
		    _linphone_call_get_local_desc(linphone_core_get_current_call(pauline->lc))
		        ->getStreamAtIdx(0)
		        .getRtpAddress();
		std::string marieConnectionAddress =
		    _linphone_call_get_local_desc(linphone_core_get_current_call(marie->lc))->getStreamAtIdx(0).getRtpAddress();

		/* The stun server shall provide marie with an IPv4 address, that we should find in c= of SDP.*/
		BC_ASSERT_TRUE(marieConnectionAddress.find(':') == std::string::npos);

		/* Pauline has an IPv4-only stun server, that is expected to work. */
		bool isBehindNat = !is_matching_a_local_address(paulineConnectionAddress, local_addresses);

		linphone_call_accept(pauline_call);

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

		if (isBehindNat) {
			/*check immmediately that the offer has a c line with a v4 IP address that is not the local one, which means
			that STUN discovery was successful.*/
			BC_ASSERT_FALSE(is_matching_a_local_address(marieConnectionAddress, local_addresses));
		} else {
			ms_warning("Test skipped, the tester is not behind a NAT.");
		}

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		check_ice(marie, pauline, LinphoneIceStateHostConnection);
	}
	end_call(marie, pauline);
	bctbx_list_free_with_data(local_addresses, bctbx_free);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_ice_call_to_no_encryption(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	// important: VP8 has really poor performances with the mire camera, at least
	// on iOS - so when ever h264 is available, let's use it instead
	if (linphone_core_find_payload_type(pauline->lc, "h264", -1, -1) != NULL) {
		disable_all_video_codecs_except_one(pauline->lc, "h264");
		disable_all_video_codecs_except_one(marie->lc, "h264");
	}
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);

	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);

	// Marie and Pauline use the Nat Policy stored in the core
	const bctbx_list_t *accounts = linphone_core_get_account_list(marie->lc);
	for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
		LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
		linphone_account_params_set_nat_policy(new_account_params, NULL);
		linphone_account_set_params(account, new_account_params);
		linphone_account_params_unref(new_account_params);
	}
	enable_stun_in_core(marie, TRUE, TRUE);
	linphone_core_manager_wait_for_stun_resolution(marie);

	accounts = linphone_core_get_account_list(pauline->lc);
	for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
		LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
		linphone_account_params_set_nat_policy(new_account_params, NULL);
		linphone_account_set_params(account, new_account_params);
		linphone_account_params_unref(new_account_params);
	}
	enable_stun_in_core(pauline, TRUE, TRUE);
	linphone_core_manager_wait_for_stun_resolution(pauline);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionNone);

	bctbx_list_t *call_enc = NULL;
	call_enc = bctbx_list_append(call_enc, LINPHONE_INT_TO_PTR(LinphoneMediaEncryptionNone));
	linphone_core_set_supported_media_encryptions(pauline->lc, call_enc);
	bctbx_list_free(call_enc);

	BC_ASSERT_FALSE(linphone_core_is_media_encryption_supported(pauline->lc, LinphoneMediaEncryptionSRTP));

	linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
	linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFDisabled);

	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "no_avpf_for_audio", TRUE);

	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (!call_ok) goto end;

	BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 2000); /*fixme to workaround a crash*/

#ifdef VIDEO_ENABLED
	if (linphone_core_video_supported(marie->lc)) {
		BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
		BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
		liblinphone_tester_check_rtcp(marie, pauline);
	} else {
		ms_warning("not tested because video not available");
	}
#endif
	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
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
    TEST_ONE_TAG(
        "Call with ICE and rtcp-mux without ICE re-invite", call_with_ice_and_rtcp_mux_without_reinvite, "ICE"),
    TEST_ONE_TAG("Call with ICE and rtcp-mux not accepted", call_with_ice_and_rtcp_mux_not_accepted, "ICE"),
    TEST_ONE_TAG("Call with ICE with default candidate not stun", call_with_ice_with_default_candidate_not_stun, "ICE"),
    TEST_ONE_TAG("Call with ICE with default candidate not stun and ipv4 prefered",
                 call_with_ice_with_default_candidate_not_stun_ipv4_prefered,
                 "ICE"),
    TEST_ONE_TAG(
        "Call with ICE with default default candidate", call_with_ice_with_default_candidate_relay_or_stun, "ICE"),
    TEST_ONE_TAG("Call with ICE without stun server", call_with_ice_without_stun, "ICE"),
    TEST_ONE_TAG("Call with ICE without stun server one side", call_with_ice_without_stun2, "ICE"),
    TEST_ONE_TAG(
        "Call with ICE and configured server-reflexive addresses", call_with_configured_sflrx_addresses, "ICE"),
    TEST_ONE_TAG("Call with ICE and stun server not responding", call_with_ice_stun_not_responding, "ICE"),
    TEST_ONE_TAG("Call with ICE ufrag and password set in SDP m line",
                 call_with_ice_ufrag_and_password_set_in_sdp_m_line,
                 "ICE"),
    TEST_ONE_TAG("Call with ICE ufrag and password set in SDP m line 2",
                 call_with_ice_ufrag_and_password_set_in_sdp_m_line_2,
                 "ICE"),
    TEST_ONE_TAG("Call with ICE ufrag and password set in SDP m line 3",
                 call_with_ice_ufrag_and_password_set_in_sdp_m_line_3,
                 "ICE"),
    TEST_ONE_TAG("Call with ICE pause and resume with caller ice", call_paused_resumed_with_caller_ice, "ICE"),
    TEST_ONE_TAG("Call with ICE pause and resume with callee ice", call_paused_resumed_with_callee_ice, "ICE"),
    TEST_ONE_TAG("Call with ICE pause and resume with both ice", call_paused_resumed_with_both_ice, "ICE"),
    TEST_ONE_TAG("Call with ICE pause and resume with caller ice and rtcp mux",
                 call_paused_resumed_with_caller_ice_and_rtcp_mux,
                 "ICE"),
    TEST_ONE_TAG("Call with ICE pause and resume with callee ice and rtcp mux",
                 call_paused_resumed_with_callee_ice_and_rtcp_mux,
                 "ICE"),
    TEST_ONE_TAG("Call with ICE pause and resume with both ice and rtcp mux",
                 call_paused_resumed_with_both_ice_and_rtcp_mux,
                 "ICE"),
    TEST_TWO_TAGS("DTLS SRTP Call with ICE pause and resume with caller ice and rtcp mux",
                  dtls_srtp_call_paused_resumed_with_caller_ice_and_rtcp_mux,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP Call with ICE pause and resume with callee ice and rtcp mux",
                  dtls_srtp_call_paused_resumed_with_callee_ice_and_rtcp_mux,
                  "ICE",
                  "DTLS"),
    TEST_TWO_TAGS("DTLS SRTP Call with ICE pause and resume with both ice and rtcp mux",
                  dtls_srtp_call_paused_resumed_with_both_ice_and_rtcp_mux,
                  "ICE",
                  "DTLS"),
    TEST_ONE_TAG("Call terminated during ICE re-INVITE", call_terminated_during_ice_reinvite, "ICE"),
    TEST_ONE_TAG("Call with ICE using dual-stack stun server", call_with_ice_and_dual_stack_stun_server, "ICE"),
    TEST_ONE_TAG("SRTP ice call to no encryption", srtp_ice_call_to_no_encryption, "ICE")};

test_suite_t call_with_ice_test_suite = {"Call with ICE",
                                         NULL,
                                         NULL,
                                         liblinphone_tester_before_each,
                                         liblinphone_tester_after_each,
                                         sizeof(call_with_ice_tests) / sizeof(call_with_ice_tests[0]),
                                         call_with_ice_tests,
                                         0};
