/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-event-cbs.h"
#include "linphone/api/c-event.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "tester_utils.h"

static const char *notify_content = "<somexml2>blabla</somexml2>";
static const char *subscribe_content = "<somexml>blabla</somexml>";

const char *liblinphone_tester_get_subscribe_content(void) {
	return subscribe_content;
}

const char *liblinphone_tester_get_notify_content(void) {
	return notify_content;
}

static void subscribe_test_declined(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	const LinphoneErrorInfo *ei;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	pauline->subscribe_policy = DenySubscription;

	lev = linphone_core_subscribe(marie->lc, pauline->identity, "dodo", 600, content);
	linphone_event_ref(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionError, 1,
	                             21000)); /*yes flexisip may wait 20 secs in case of forking*/
	ei = linphone_event_get_error_info(lev);
	BC_ASSERT_PTR_NOT_NULL(ei);
	if (ei) {
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(ei), 603, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(ei));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 1000));

	bctbx_list_free(lcs);
	linphone_content_unref(content);
	linphone_event_unref(lev);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

typedef enum RefreshTestType { NoRefresh = 0, AutoRefresh = 1, ManualRefresh = 2 } RefreshTestType;

static void subscribe_test_with_args(bool_t terminated_by_subscriber,
                                     RefreshTestType refresh_type,
                                     bool_t without_notify,
                                     bool_t ua_restarts) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = refresh_type != NoRefresh ? 4 : 600;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	if (refresh_type == ManualRefresh) {
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "refresh_generic_subscribe", 0);
	}

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	if (without_notify) {
		lev = linphone_core_subscribe(marie->lc, pauline->identity, "doingnothing", expires, content);
	} else {
		lev = linphone_core_subscribe(marie->lc, pauline->identity, "dodo", expires, content);
	}
	linphone_event_ref(lev);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 1000));

	/* make sure transport used for the subscribe is the same as the one used for REGISTER*/
	belle_sip_header_via_t *via =
	    BELLE_SIP_HEADER_VIA(belle_sip_header_create("Via", linphone_event_get_custom_header(lev, "Via")));
	BC_ASSERT_STRING_EQUAL(belle_sip_header_via_get_transport_lowercase(via),
	                       linphone_proxy_config_get_transport(linphone_core_get_default_proxy_config(marie->lc)));

	if (without_notify == FALSE) {
		/*make sure marie receives first notification before terminating*/
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));
	}

	if (refresh_type == AutoRefresh) {
		wait_for_list(lcs, NULL, 0, 6000);
		BC_ASSERT_PTR_NOT_NULL(pauline->lev);
		if (pauline->lev) {
			BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
		}
	} else if (refresh_type == ManualRefresh) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionExpiring, 1, 4000));
		linphone_event_update_subscribe(lev, NULL);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 2, 5000));
	}

	if (ua_restarts) {
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		linphone_core_set_network_reachable(pauline->lc, TRUE);
	}

	if (terminated_by_subscriber) {
		linphone_event_terminate(lev);
	} else {
		BC_ASSERT_PTR_NOT_NULL(pauline->lev);
		linphone_event_terminate(pauline->lev);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));

	bctbx_list_free(lcs);
	linphone_event_unref(lev);
	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void subscribe_test_destroy_core_before_event_terminate(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 600;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_subscribe(marie->lc, pauline->identity, "dodo", expires, content);
	linphone_event_ref(lev);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 1000));

	/* make sure transport used for the subscribe is the same as the one used for REGISTER*/
	belle_sip_header_via_t *via =
	    BELLE_SIP_HEADER_VIA(belle_sip_header_create("Via", linphone_event_get_custom_header(lev, "Via")));
	BC_ASSERT_STRING_EQUAL(belle_sip_header_via_get_transport_lowercase(via),
	                       linphone_proxy_config_get_transport(linphone_core_get_default_proxy_config(marie->lc)));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	wait_for_list(lcs, NULL, 0, 6000);
	BC_ASSERT_PTR_NOT_NULL(pauline->lev);
	if (pauline->lev) {
		BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
	}

	/*linphone_event_terminate(lev);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,5000));*/

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	int dummy = 0;
	wait_for_until(NULL, NULL, &dummy, 1, 5000);
	linphone_event_terminate(lev);
	linphone_event_unref(lev);
	linphone_content_unref(content);
}

static void subscribe_test_with_args2(bool_t terminated_by_subscriber, RefreshTestType refresh_type) {
	// Purpose of this settigs is to make sure subscribe is sent using the same connection as the register only based on
	// the domain name of both marie identity and pauline identity regardless of the sip proxy address
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneAccount *account = linphone_core_get_default_account(marie->lc);
	const LinphoneAccountParams *params = linphone_account_get_params(account);
	LinphoneAddress *addr = linphone_address_clone(linphone_account_params_get_server_address(params));
	linphone_address_set_domain(addr, "sipv4.example.org");
	LinphoneAccountParams *new_params = linphone_account_params_clone(linphone_account_get_params(account));
	linphone_account_params_set_server_address(new_params, addr);
	// remove route to force fonction linphone_configure_op_with_account to make a choice
	linphone_account_params_set_routes_addresses(new_params, NULL);
	linphone_account_set_params(account, new_params);
	linphone_account_params_unref(new_params);
	linphone_address_unref(addr);
	linphone_core_manager_start(marie, TRUE);

	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = refresh_type != NoRefresh ? 4 : 600;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	if (refresh_type == ManualRefresh) {
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "refresh_generic_subscribe", 0);
	}

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_create_subscribe(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_add_custom_header(lev, "My-Header", "pouet");
	linphone_event_add_custom_header(lev, "My-Header2", "pimpon");
	linphone_event_send_subscribe(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));
	/* wait for receiving the 200 Ok in order to check the Via */
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 1000));
	/* make sure transport used for the subscribe is the same as the one used for REGISTER*/
	BC_ASSERT_STRING_NOT_EQUAL(linphone_proxy_config_get_transport(linphone_core_get_default_proxy_config(marie->lc)),
	                           "udp");
	belle_sip_header_via_t *via =
	    BELLE_SIP_HEADER_VIA(belle_sip_header_create("Via", linphone_event_get_custom_header(lev, "Via")));
	if (BC_ASSERT_PTR_NOT_NULL(via)) {
		BC_ASSERT_STRING_EQUAL(belle_sip_header_via_get_transport_lowercase(via),
		                       linphone_proxy_config_get_transport(linphone_core_get_default_proxy_config(marie->lc)));
	}

	if (pauline->stat.number_of_LinphoneSubscriptionIncomingReceived == 1) {
		/*check good receipt of custom headers*/
		BC_ASSERT_STRING_EQUAL(linphone_event_get_custom_header(pauline->lev, "My-Header"), "pouet");
		BC_ASSERT_STRING_EQUAL(linphone_event_get_custom_header(pauline->lev, "My-Header2"), "pimpon");
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	if (refresh_type == AutoRefresh) {
		wait_for_list(lcs, NULL, 0, 6000);
		BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
	} else if (refresh_type == ManualRefresh) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionExpiring, 1, 4000));
		linphone_event_update_subscribe(lev, NULL);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 2, 5000));
	}

	if (terminated_by_subscriber) {
		linphone_event_terminate(lev);
	} else {
		BC_ASSERT_PTR_NOT_NULL(pauline->lev);
		linphone_event_terminate(pauline->lev);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	linphone_event_unref(lev);
	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void subscribe_test_terminated_by_subscriber(void) {
	subscribe_test_with_args(TRUE, NoRefresh, FALSE, FALSE);
}

static void subscribe_test_terminated_by_notifier(void) {
	subscribe_test_with_args(FALSE, NoRefresh, FALSE, FALSE);
}

static void subscribe_test_terminated_by_notifier_without_notify_restarts(void) {
	// FIXME : If the server doesn't have the realm parameter, the NOTIFY isn't authenticated and the test doesn't pass.
	subscribe_test_with_args(FALSE, NoRefresh, TRUE, TRUE);
}

/* Caution: this test does not really check that the subscribe are refreshed, because the core is not managing the
 * expiration of unrefreshed subscribe dialogs. So it is just checking that it is not crashing.
 */
static void subscribe_test_refreshed(void) {
	subscribe_test_with_args(TRUE, AutoRefresh, FALSE, FALSE);
}

static void subscribe_test_with_custom_header(void) {
	subscribe_test_with_args2(TRUE, NoRefresh);
}

static void subscribe_test_manually_refreshed(void) {
	subscribe_test_with_args(TRUE, ManualRefresh, FALSE, FALSE);
}

/* This test has LeaksMemory attribute due to the brutal disconnection of pauline, followed by core destruction.
 * TODO: fix it.
 */
static void subscribe_loosing_dialog(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 4;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline1->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_create_subscribe(marie->lc, pauline1->identity, "dodo", expires);
	linphone_event_add_custom_header(lev, "My-Header", "pouet");
	linphone_event_add_custom_header(lev, "My-Header2", "pimpon");
	linphone_event_send_subscribe(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline1->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline1->stat.number_of_LinphoneSubscriptionActive, 1, 5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	/* now pauline looses internet connection and reboots, as ICMP error sent when flexisip tries to re-open the
	 * connection can be blocked by firewall, we directly send 503 from pauline1 */
	sal_set_unconditional_answer(linphone_core_get_sal(pauline1->lc), 503);
	sal_enable_unconditional_answer(linphone_core_get_sal(pauline1->lc), 1);

	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	lcs = bctbx_list_append(lcs, pauline->lc);
	/*let expire the incoming subscribe received by pauline */
	BC_ASSERT_TRUE(wait_for_list(lcs, NULL, 0, 5000));

	/* Marie will retry the subscription.
	 * She will first receive a 503 Service unavailable from flexisip thanks the "no longer existing" Pauline1.
	 * Then she will forge a new SUBSCRIBE in order to restart a new dialog, and this one will reach the new Pauline.*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 2, 8000));
	/*and get it accepted again*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->lev);
	if (pauline->lev)
		BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 2, 5000));
	linphone_event_terminate(lev);
	linphone_event_unref(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	sal_enable_unconditional_answer(linphone_core_get_sal(pauline1->lc), 0);
	linphone_core_manager_destroy(pauline1);
	bctbx_list_free(lcs);
}

/* This test has LeaksMemory attribute due to the brutal disconnection of pauline, followed by core destruction.
 */
static void subscribe_loosing_dialog_2(void) {
#ifdef WIN32
	/*Unfortunately this test doesn't work on windows due to the way closed TCP ports behave.
	 * Unlike linux and macOS, released TCP port don't send an ICMP error (or maybe at least for a period of time.
	 * This prevents this test from working, see comments below*/
	ms_warning("subscribe_loosing_dialog() skipped on windows.");
#else
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 4;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_create_subscribe(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_add_custom_header(lev, "My-Header", "pouet");
	linphone_event_add_custom_header(lev, "My-Header2", "pimpon");
	linphone_event_send_subscribe(lev, content);
	linphone_content_unref(content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	/* now marie looses internet connection and reboots */
	linphone_core_set_network_reachable(marie->lc, FALSE);
	lcs = bctbx_list_remove(lcs, marie->lc);
	linphone_event_unref(lev);
	linphone_core_manager_reinit(marie);
	linphone_core_manager_start(marie, TRUE);
	lcs = bctbx_list_append(lcs, marie->lc);
	// now try to notify through the "broken" dialog
	if (pauline->lev) {
		LinphoneContent *ct = linphone_core_create_content(pauline->lc);
		linphone_content_set_type(ct, "application");
		linphone_content_set_subtype(ct, "somexml2");
		linphone_content_set_buffer(ct, (const uint8_t *)notify_content, strlen(notify_content));
		linphone_event_notify(pauline->lev, ct);
		linphone_content_unref(ct);

		/* the notify should fail, causing termination of the LinphoneEvent at notifier side */
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	} else {
		BC_FAIL("Unexpect null event for pauline");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
#endif
}

static void subscribe_with_io_error(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 4;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_create_subscribe(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_add_custom_header(lev, "My-Header", "pouet");
	linphone_event_add_custom_header(lev, "My-Header2", "pimpon");
	linphone_event_send_subscribe(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	/* now marie gets network errors when refreshing*/
	sal_set_send_error(linphone_core_get_sal(marie->lc), -1);

	/*marie will retry the subscription*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 2, 8000));
	sal_set_send_error(linphone_core_get_sal(marie->lc), 0);

	/*and get it accepted again*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 2, 5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->lev);
	if (pauline->lev) {
		BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 2, 5000));
	linphone_event_terminate(lev);
	linphone_event_unref(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void subscribe_not_timely_responded(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 4;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_create_subscribe(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_add_custom_header(lev, "My-Header", "pouet");
	linphone_event_add_custom_header(lev, "My-Header2", "pimpon");
	linphone_event_send_subscribe(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	/* now pauline is no longer scheduled (simulating a very big latency in the network) */
	lcs = bctbx_list_remove(lcs, pauline->lc);
	/*marie's dialog will expire while the SUBSCRIBE refresh is in progress*/
	wait_for_list(lcs, NULL, 0, 8000);

	lcs = bctbx_list_append(lcs, pauline->lc);
	wait_for_list(lcs, NULL, 0, 3000);
	linphone_event_terminate(lev);
	linphone_event_unref(lev);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void publish_test_with_args(bool_t refresh, int expires) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "xml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "refresh_generic_publish", refresh);

	lev = linphone_core_create_publish(marie->lc, pauline->identity, "dodo", expires);

	linphone_event_add_custom_header(lev, "CustomHeader", "someValue");
	linphone_event_send_publish(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishOk, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishOk, 1, 3000));

	if (!refresh) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishExpiring, 1, 5000));
		linphone_event_update_publish(lev, content);
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishOutgoingProgress, 2, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishRefreshing, 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishOk, 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishOk, 2, 3000));
	} else {
	}

	linphone_event_terminate(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishCleared, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishCleared, 1, 3000));

	linphone_event_unref(lev);

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void publish_test(void) {
	publish_test_with_args(TRUE, 5);
}

static void publish_without_expires(void) {
	publish_test_with_args(TRUE, -1);
}

static void publish_no_auto_test(void) {
	publish_test_with_args(FALSE, 5);
}

static void publish_expired(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 4;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "xml");
	linphone_content_set_buffer(content, (const uint8_t *)subscribe_content, strlen(subscribe_content));

	lev = linphone_core_create_publish(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_add_custom_header(lev, "My-Header", "Marie loses connection");
	linphone_event_send_publish(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishIncomingReceived, 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphonePublishOk, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishOk, 1, 3000));

	linphone_core_set_network_reachable(marie->lc, FALSE);
	lcs = bctbx_list_remove(lcs, marie->lc);
	linphone_event_unref(lev);
	linphone_core_manager_stop(marie);
	linphone_core_manager_uninit(marie);
	bctbx_free(marie);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphonePublishCleared, 1, 5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void on_notify_response(LinphoneEvent *lev) {
	int *flag = (int *)linphone_event_get_user_data(lev);
	*flag = 1;
}

static void out_of_dialog_notify(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	LinphoneEventCbs *lev_cbs;
	int notify_response_done = 0;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)notify_content, strlen(notify_content));

	lev = linphone_core_create_notify(marie->lc, pauline->identity, "dodo");
	lev_cbs = linphone_factory_create_event_cbs(linphone_factory_get());
	linphone_event_cbs_set_notify_response(lev_cbs, on_notify_response);
	linphone_event_set_user_data(lev, &notify_response_done);
	linphone_event_add_callbacks(lev, lev_cbs);
	linphone_event_add_custom_header(lev, "CustomHeader", "someValue");
	linphone_event_notify(lev, content);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_NotifyReceived, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &notify_response_done, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));

	linphone_event_unref(lev);
	linphone_event_cbs_unref(lev_cbs);

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void subscribe_notify_with_missing_200ok(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneContent *content;
	LinphoneEvent *lev;
	int expires = 10;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);
	pauline->subscribe_policy = RetainSubscription;

	lev = linphone_core_create_subscribe(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_send_subscribe(lev, NULL);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 10000));

	LinphoneEvent *pauline_event = pauline->lev;
	if (BC_ASSERT_PTR_NOT_NULL(pauline_event)) {
		sal_set_send_error(linphone_core_get_sal(pauline->lc), 1); /*to trash the message without generating error*/
		linphone_event_accept_subscription(pauline->lev);
		sal_set_send_error(linphone_core_get_sal(pauline->lc), 0); /*normal behavior*/
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml");
	linphone_content_set_buffer(content, (const uint8_t *)notify_content, strlen(notify_content));
	linphone_event_notify(pauline->lev, content);
	linphone_content_unref(content);

	/*make sure marie receives the notify and transitions the subscribption to active state */
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_NotifyReceived, 1, 5000));

	linphone_event_terminate(pauline->lev);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	linphone_event_unref(lev);
	linphone_event_unref(pauline_event);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void subscribe_notify_not_handled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneEvent *lev;
	int expires = 10;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);
	pauline->subscribe_policy = DoNothingWithSubscription;

	lev = linphone_core_create_subscribe(marie->lc, pauline->identity, "dodo", expires);
	linphone_event_send_subscribe(lev, NULL);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 1000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneSubscriptionTerminated, 1, 5000));
	BC_ASSERT_PTR_NULL(pauline->lev);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneSubscriptionError, 1, 5000));

	linphone_event_unref(lev);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

test_t event_tests[] = {
    TEST_ONE_TAG("Subscribe declined", subscribe_test_declined, "presence"),
    TEST_ONE_TAG("Subscribe terminated by subscriber", subscribe_test_terminated_by_subscriber, "presence"),
    TEST_ONE_TAG("Subscribe with custom headers", subscribe_test_with_custom_header, "presence"),
    TEST_ONE_TAG("Subscribe refreshed", subscribe_test_refreshed, "presence"),
    TEST_TWO_TAGS("Subscribe loosing dialog", subscribe_loosing_dialog, "presence", "LeaksMemory"),
    TEST_ONE_TAG("Server try to terminate a lost dialog", subscribe_loosing_dialog_2, "presence"),
    TEST_ONE_TAG("Subscribe with io error", subscribe_with_io_error, "presence"),
    TEST_ONE_TAG("Subscribe manually refreshed", subscribe_test_manually_refreshed, "presence"),
    TEST_ONE_TAG("Subscribe terminated by notifier", subscribe_test_terminated_by_notifier, "presence"),
    TEST_ONE_TAG("Subscribe terminated by notifier without notify and Pauline restarts",
                 subscribe_test_terminated_by_notifier_without_notify_restarts,
                 "presence"),
    TEST_ONE_TAG("Subscribe not timely responded", subscribe_not_timely_responded, "presence"),
    TEST_TWO_TAGS("Subscribe terminated after Core stopped",
                  subscribe_test_destroy_core_before_event_terminate,
                  "presence",
                  "LeaksMemory"),
    TEST_NO_TAG("Subscribe with missing 200 ok", subscribe_notify_with_missing_200ok),
    TEST_NO_TAG("Subscribe not handled", subscribe_notify_not_handled),
    TEST_ONE_TAG("Publish", publish_test, "presence"),
    TEST_ONE_TAG("Publish without expires", publish_without_expires, "presence"),
    TEST_ONE_TAG("Publish without automatic refresh", publish_no_auto_test, "presence"),
    TEST_ONE_TAG("Publish expired", publish_expired, "presence"),
    TEST_ONE_TAG("Out of dialog notify", out_of_dialog_notify, "presence")};

test_suite_t event_test_suite = {"Event",
                                 NULL,
                                 NULL,
                                 liblinphone_tester_before_each,
                                 liblinphone_tester_after_each,
                                 sizeof(event_tests) / sizeof(event_tests[0]),
                                 event_tests,
                                 0};
