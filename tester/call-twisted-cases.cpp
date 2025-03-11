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

#include "belle_sip_tester_utils.h"

#include "linphone/api/c-types.h"
#include "mediastreamer2/msutils.h"
#include "mediastreamer2/msvolume.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

static void _call_early_media_followed_by_ringing(bool_t notify_all_ringings) {
	LinphoneCoreManager *marie = linphone_core_manager_new("empty_rc");
	bellesip::QuickSipAgent agent("sip.example.org", "tcp");
	LinphoneCall *call;
	linphone_core_enable_video_capture(marie->lc, FALSE);
	linphone_core_enable_video_display(marie->lc, FALSE);

	if (notify_all_ringings)
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "notify_all_ringings", 1);

	agent.setRequestHandler([](bellesip::QuickSipAgent &ag, const belle_sip_request_event_t *ev) -> bool {
		belle_sip_response_t *resp =
		    belle_sip_response_create_from_request(belle_sip_request_event_get_request(ev), 183);
		belle_sip_server_transaction_t *tr =
		    belle_sip_provider_create_server_transaction(ag.getProv(), belle_sip_request_event_get_request(ev));
		const std::string sdp = "v=0\r\n"
		                        "o=jehan-mac 2463217870 2463217870 IN IP4 192.168.0.18\r\n"
		                        "s=Talk\r\n"
		                        "c=IN IP4 192.168.0.18\r\n"
		                        "t=0 0\r\n"
		                        "m=audio 1023 RTP/AVP 0\r\n"
		                        "a=recvonly\r\n";
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(resp),
		                             BELLE_SIP_HEADER(belle_sip_header_content_type_create("application", "sdp")));
		belle_sip_message_set_body(BELLE_SIP_MESSAGE(resp), sdp.c_str(), sdp.size());
		belle_sip_server_transaction_send_response(tr, resp);
		ag.doLater(
		    "send 180",
		    [tr, sdp]() {
			    belle_sip_response_t *resp = belle_sip_response_create_from_request(
			        belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(tr)), 180);
			    belle_sip_server_transaction_send_response(tr, resp);
		    },
		    1000);

		ag.doLater(
		    "accept call",
		    [tr, sdp]() {
			    belle_sip_response_t *resp = belle_sip_response_create_from_request(
			        belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(tr)), 200);
			    belle_sip_header_contact_t *ct = belle_sip_header_contact_new();
			    belle_sip_header_contact_set_automatic(ct, TRUE);
			    belle_sip_message_add_header(BELLE_SIP_MESSAGE(resp), BELLE_SIP_HEADER(ct));
			    belle_sip_message_add_header(
			        BELLE_SIP_MESSAGE(resp),
			        BELLE_SIP_HEADER(belle_sip_header_content_type_create("application", "sdp")));
			    belle_sip_message_set_body(BELLE_SIP_MESSAGE(resp), sdp.c_str(), sdp.size());
			    belle_sip_server_transaction_send_response(tr, resp);
		    },
		    2000);
		return false;
	});

	call = linphone_core_invite(marie->lc, agent.getListeningUriAsString().c_str());

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingRinging, 0, int, "%i");
	/* check the audio stream is started */
	MediaStream *ms = linphone_call_get_stream(call, LinphoneStreamTypeAudio);
	if (BC_ASSERT_PTR_NOT_NULL(ms)) {
		BC_ASSERT_TRUE(media_stream_get_state(ms) == MSStreamStarted);
	}

	if (notify_all_ringings) {
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
		MediaStream *ms = linphone_call_get_stream(call, LinphoneStreamTypeAudio);
		if (BC_ASSERT_PTR_NOT_NULL(ms)) {
			BC_ASSERT_TRUE(media_stream_get_state(ms) == MSStreamInitialized);
		}
	}

	agent.setRequestHandler([](bellesip::BasicSipAgent &ag, const belle_sip_request_event_t *ev) -> bool {
		(void)ag;
		belle_sip_request_t *req = belle_sip_request_event_get_request(ev);
		if (strcmp(belle_sip_request_get_method(req), "BYE") != 0) return false;
		belle_sip_response_t *resp =
		    belle_sip_response_create_from_request(belle_sip_request_event_get_request(ev), 200);
		belle_sip_provider_send_response(ag.getProv(), resp);
		return false;
	});
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	if (!notify_all_ringings) {
		/* Make sure that we never get back to ringing state */
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingRinging, 0, int, "%i");
	}

	linphone_call_terminate(call);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(marie);
}

static void call_early_media_followed_by_ringing(void) {
	_call_early_media_followed_by_ringing(FALSE);
}

static void call_early_media_followed_by_ringing_alternate_behavior(void) {
	_call_early_media_followed_by_ringing(TRUE);
}

static void call_challenged_after_180_base(bool_t with_account) {
	LinphoneCoreManager *marie = linphone_core_manager_new("empty_rc");
	bellesip::QuickSipAgent agent("sip.example.org", "tcp");
	LinphoneCall *call;

	LinphoneAuthInfo *ai =
	    linphone_factory_create_auth_info(linphone_factory_get(), "bob", NULL, "secret", NULL, NULL, "sip.example.org");
	linphone_core_enable_video_capture(marie->lc, FALSE);
	linphone_core_enable_video_display(marie->lc, FALSE);
	linphone_core_add_auth_info(marie->lc, ai);

	LinphoneAddress *server_address = NULL;
	LinphoneAddress *identity_address = NULL;

	if (!!with_account) {
		const char *auth_username = linphone_auth_info_get_username(ai);
		const char *auth_domain = linphone_auth_info_get_domain(ai);
		char identity[256];
		snprintf(identity, sizeof(identity), "sip:%s@%s", auth_username, auth_domain);
		identity_address = linphone_address_new(identity);
		const char *realm = linphone_auth_info_get_realm(ai);

		LinphoneAccountParams *account_params = linphone_account_params_new(marie->lc, TRUE);
		char *agent_listening_uri = ms_strdup(agent.getListeningUriAsString().c_str());
		server_address = linphone_address_new(agent_listening_uri);
		linphone_account_params_set_server_address(account_params, server_address);
		bctbx_list_t *routes = bctbx_list_new(server_address);
		ms_message("Adding route %s to the default account", agent_listening_uri);
		linphone_account_params_set_routes_addresses(account_params, routes);
		bctbx_list_free(routes);

		ms_free(agent_listening_uri);
		linphone_account_params_set_register_enabled(account_params, FALSE);
		linphone_account_params_set_identity_address(account_params, identity_address);
		linphone_account_params_set_realm(account_params, realm);
		LinphoneAccount *account = linphone_account_new(marie->lc, account_params);
		linphone_account_params_unref(account_params);
		linphone_core_add_account(marie->lc, account);
		linphone_core_set_default_account(marie->lc, account);
		linphone_account_unref(account);
	}

	linphone_auth_info_unref(ai);

	linphone_core_set_label(marie->lc, "liblinphone");
	linphone_core_set_primary_contact(marie->lc, "sip:bob@localhost");

	agent.setRequestHandler([](bellesip::QuickSipAgent &ag, const belle_sip_request_event_t *ev) -> bool {
		belle_sip_response_t *resp =
		    belle_sip_response_create_from_request(belle_sip_request_event_get_request(ev), 180);
		belle_sip_server_transaction_t *tr =
		    belle_sip_provider_create_server_transaction(ag.getProv(), belle_sip_request_event_get_request(ev));
		belle_sip_server_transaction_send_response(tr, resp);
		ag.doLater(
		    "send 401",
		    [tr]() {
			    belle_sip_response_t *resp = belle_sip_response_create_from_request(
			        belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(tr)), 401);
			    belle_sip_header_www_authenticate_t *auth = belle_sip_header_www_authenticate_new();
			    belle_sip_header_www_authenticate_set_scheme(BELLE_SIP_HEADER_WWW_AUTHENTICATE(auth), "Digest");
			    belle_sip_header_www_authenticate_set_realm(BELLE_SIP_HEADER_WWW_AUTHENTICATE(auth), "sip.example.com");
			    belle_sip_header_www_authenticate_set_algorithm(BELLE_SIP_HEADER_WWW_AUTHENTICATE(auth), "MD5");
			    belle_sip_header_www_authenticate_set_nonce(BELLE_SIP_HEADER_WWW_AUTHENTICATE(auth),
			                                                "abcdefghijklmnopqrstuvwxyz");
			    belle_sip_message_add_header(BELLE_SIP_MESSAGE(resp), BELLE_SIP_HEADER(auth));
			    belle_sip_server_transaction_send_response(tr, resp);
		    },
		    1000);
		return false;
	});

	LinphoneCallParams *call_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneAccount *default_account = linphone_core_get_default_account(marie->lc);
	linphone_call_params_set_account(call_params, default_account);
	call = linphone_core_invite_with_params(marie->lc, agent.getListeningUriAsString().c_str(), call_params);
	linphone_call_params_unref(call_params);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	/* we shall receive a new INVITE with Authorization, this time answer it with 200 OK */

	agent.setRequestHandler([](bellesip::QuickSipAgent &ag, const belle_sip_request_event_t *ev) -> bool {
		belle_sip_request_t *req = belle_sip_request_event_get_request(ev);

		if (strcmp(belle_sip_request_get_method(req), "INVITE") == 0) {
			belle_sip_object_ref(req);
			ag.doLater(
			    "answer call",
			    [&ag, req]() {
				    auto tr = belle_sip_provider_create_server_transaction(ag.getProv(), req);
				    const std::string sdp = "v=0\r\n"
				                            "o=jehan-mac 2463217870 2463217870 IN IP4 192.168.0.18\r\n"
				                            "s=Talk\r\n"
				                            "c=IN IP4 192.168.0.18\r\n"
				                            "t=0 0\r\n"
				                            "m=audio 7078 RTP/AVP 0\r\n"
				                            "a=inactive\r\n";
				    belle_sip_response_t *resp = belle_sip_response_create_from_request(
				        belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(tr)), 200);
				    belle_sip_header_contact_t *ct = belle_sip_header_contact_new();
				    belle_sip_header_contact_set_automatic(ct, TRUE);
				    belle_sip_message_add_header(BELLE_SIP_MESSAGE(resp), BELLE_SIP_HEADER(ct));
				    belle_sip_message_add_header(
				        BELLE_SIP_MESSAGE(resp),
				        BELLE_SIP_HEADER(belle_sip_header_content_type_create("application", "sdp")));
				    belle_sip_message_set_body(BELLE_SIP_MESSAGE(resp), sdp.c_str(), sdp.size());
				    belle_sip_server_transaction_send_response(tr, resp);
				    belle_sip_object_unref(req);
			    },
			    1000);
		}
		return false;
	});

	/* it goes to PausedByRemote because no streams are active */
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));

	agent.setRequestHandler([](bellesip::BasicSipAgent &ag, const belle_sip_request_event_t *ev) -> bool {
		(void)ag;
		belle_sip_request_t *req = belle_sip_request_event_get_request(ev);
		if (strcmp(belle_sip_request_get_method(req), "BYE") != 0) return false;
		belle_sip_response_t *resp =
		    belle_sip_response_create_from_request(belle_sip_request_event_get_request(ev), 200);
		belle_sip_provider_send_response(ag.getProv(), resp);
		return false;
	});

	linphone_call_terminate(call);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneCallReleased, 1));
	linphone_core_manager_destroy(marie);
	if (identity_address) linphone_address_unref(identity_address);
	if (server_address) linphone_address_unref(server_address);
}

static void call_challenged_after_180(void) {
	call_challenged_after_180_base(FALSE);
}

static void call_challenged_after_180_with_account(void) {
	call_challenged_after_180_base(TRUE);
}

static test_t call_twisted_cases_tests[] = {
    {"Call being answered with 183 then 180", call_early_media_followed_by_ringing},
    {"Call being answered with 183 then 180 - alternate behavior",
     call_early_media_followed_by_ringing_alternate_behavior},
    {"Call challenged after 180", call_challenged_after_180},
    {"Call challenged after 180 with account", call_challenged_after_180_with_account}};

test_suite_t call_twisted_cases_suite = {"Call twisted cases",
                                         NULL,
                                         NULL,
                                         liblinphone_tester_before_each,
                                         liblinphone_tester_after_each,
                                         sizeof(call_twisted_cases_tests) / sizeof(call_twisted_cases_tests[0]),
                                         call_twisted_cases_tests,
                                         10 /*average time*/};
