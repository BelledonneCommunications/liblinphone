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



#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include <linphone/event.h>
#include "liblinphone_tester.h"
#include "tester_utils.h"

static const char *notify_content="<somexml2>blabla</somexml2>";
static const char *subscribe_content="<somexml>blabla</somexml>";

const char *liblinphone_tester_get_subscribe_content(void){
	return subscribe_content;
}

const char *liblinphone_tester_get_notify_content(void){
	return notify_content;
}

static void subscribe_test_declined(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	const LinphoneErrorInfo *ei;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	pauline->decline_subscribe=TRUE;

	lev=linphone_core_subscribe(marie->lc,pauline->identity,"dodo",600,content);
	linphone_event_ref(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionError,1,21000));/*yes flexisip may wait 20 secs in case of forking*/
	ei=linphone_event_get_error_info(lev);
	BC_ASSERT_PTR_NOT_NULL(ei);
	if (ei){
		BC_ASSERT_EQUAL(linphone_error_info_get_protocol_code(ei),603, int, "%d");
		BC_ASSERT_PTR_NOT_NULL(linphone_error_info_get_phrase(ei));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,1000));

	bctbx_list_free(lcs);
	linphone_content_unref(content);
	linphone_event_unref(lev);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

typedef enum RefreshTestType{
	NoRefresh,
	AutoRefresh,
	ManualRefresh
}RefreshTestType;

static void subscribe_test_with_args(bool_t terminated_by_subscriber, RefreshTestType refresh_type) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	int expires= refresh_type!=NoRefresh ? 4 : 600;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);

	if (refresh_type==ManualRefresh){
		lp_config_set_int(linphone_core_get_config(marie->lc),"sip","refresh_generic_subscribe",0);
	}

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lev=linphone_core_subscribe(marie->lc,pauline->identity,"dodo",expires,content);
	linphone_event_ref(lev);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,1,1000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,1000));

	if (refresh_type==AutoRefresh){
		wait_for_list(lcs,NULL,0,6000);
		BC_ASSERT_PTR_NOT_NULL(pauline->lev);
		if (pauline->lev){
			BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
		}
	}else if (refresh_type==ManualRefresh){
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionExpiring,1,4000));
		linphone_event_update_subscribe(lev,NULL);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,2,2000));
	}

	if (terminated_by_subscriber){
		linphone_event_terminate(lev);
	}else{
		BC_ASSERT_PTR_NOT_NULL(pauline->lev);
		linphone_event_terminate(pauline->lev);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,1000));

	bctbx_list_free(lcs);
	linphone_event_unref(lev);
	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void subscribe_test_with_args2(bool_t terminated_by_subscriber, RefreshTestType refresh_type) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	int expires= refresh_type!=NoRefresh ? 4 : 600;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);

	if (refresh_type==ManualRefresh){
		lp_config_set_int(linphone_core_get_config(marie->lc),"sip","refresh_generic_subscribe",0);
	}

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lev=linphone_core_create_subscribe(marie->lc,pauline->identity,"dodo",expires);
	linphone_event_add_custom_header(lev,"My-Header","pouet");
	linphone_event_add_custom_header(lev,"My-Header2","pimpon");
	linphone_event_send_subscribe(lev,content);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));

	if (pauline->stat.number_of_LinphoneSubscriptionIncomingReceived == 1) {
		/*check good receipt of custom headers*/
		BC_ASSERT_STRING_EQUAL(linphone_event_get_custom_header(pauline->lev,"My-Header"),"pouet");
		BC_ASSERT_STRING_EQUAL(linphone_event_get_custom_header(pauline->lev,"My-Header2"),"pimpon");
	}
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,1,5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,5000));

	if (refresh_type==AutoRefresh){
		wait_for_list(lcs,NULL,0,6000);
		BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
	}else if (refresh_type==ManualRefresh){
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionExpiring,1,4000));
		linphone_event_update_subscribe(lev,NULL);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,2,5000));
	}

	if (terminated_by_subscriber){
		linphone_event_terminate(lev);
	}else{
		BC_ASSERT_PTR_NOT_NULL(pauline->lev);
		linphone_event_terminate(pauline->lev);
	}

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void subscribe_test_terminated_by_subscriber(void){
	subscribe_test_with_args(TRUE,NoRefresh);
}

static void subscribe_test_terminated_by_notifier(void){
	subscribe_test_with_args(FALSE,NoRefresh);
}

/* Caution: this test does not really check that the subscribe are refreshed, because the core is not managing the expiration of
 * unrefreshed subscribe dialogs. So it is just checking that it is not crashing.
 */
static void subscribe_test_refreshed(void){
	subscribe_test_with_args(TRUE,AutoRefresh);
}

static void subscribe_test_with_custom_header(void){
	subscribe_test_with_args2(TRUE,NoRefresh);
}

static void subscribe_test_manually_refreshed(void){
	subscribe_test_with_args(TRUE,ManualRefresh);
}

static void subscribe_loosing_dialog(void) {
#ifdef WIN32
	/*Unfortunately this test doesn't work on windows due to the way closed TCP ports behave.
	 * Unlike linux and macOS, released TCP port don't send an ICMP error (or maybe at least for a period of time.
	 * This prevents this test from working, see comments below*/
	ms_warning("subscribe_loosing_dialog() skipped on windows.");
#else
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	int expires= 4;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lev=linphone_core_create_subscribe(marie->lc,pauline->identity,"dodo",expires);
	linphone_event_add_custom_header(lev,"My-Header","pouet");
	linphone_event_add_custom_header(lev,"My-Header2","pimpon");
	linphone_event_send_subscribe(lev,content);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));


	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,1,5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,5000));

	/* now pauline looses internet connection and reboots */
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	/*let expire the incoming subscribe received by pauline */
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,5000));
	lcs = bctbx_list_remove(lcs, pauline->lc);
	linphone_core_manager_destroy(pauline);
	pauline = linphone_core_manager_new( "pauline_tcp_rc");
	lcs = bctbx_list_append(lcs, pauline->lc);

	/* Marie will retry the subscription.
	 * She will first receive a 503 Service unavailable from flexisip thanks the ICMP error returned by the no longer existing Pauline.
	 * Then she will forge a new SUBSCRIBE in order to restart a new dialog, and this one will reach the new Pauline.*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,2,8000));
	/*and get it accepted again*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,2,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,1,5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->lev);
	if (pauline->lev) BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,2,5000));
	linphone_event_terminate(lev);


	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
#endif
}

static void subscribe_with_io_error(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	int expires= 4;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lev=linphone_core_create_subscribe(marie->lc,pauline->identity,"dodo",expires);
	linphone_event_add_custom_header(lev,"My-Header","pouet");
	linphone_event_add_custom_header(lev,"My-Header2","pimpon");
	linphone_event_send_subscribe(lev,content);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));


	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,1,5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,5000));

	/* now marie gets network errors when refreshing*/
	sal_set_send_error(linphone_core_get_sal(marie->lc), -1);

	/*marie will retry the subscription*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,2,8000));
	sal_set_send_error(linphone_core_get_sal(marie->lc), 0);

	/*and get it accepted again*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,2,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,2,5000));
	BC_ASSERT_EQUAL(linphone_event_get_subscription_state(pauline->lev), LinphoneSubscriptionActive, int, "%d");
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,2,5000));
	linphone_event_terminate(lev);


	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void subscribe_not_timely_responded(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	int expires= 4;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lev=linphone_core_create_subscribe(marie->lc,pauline->identity,"dodo",expires);
	linphone_event_add_custom_header(lev,"My-Header","pouet");
	linphone_event_add_custom_header(lev,"My-Header2","pimpon");
	linphone_event_send_subscribe(lev,content);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));


	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionActive,1,5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,5000));

	/* now pauline is no longer scheduled (simulating a very big latency in the network) */
	lcs = bctbx_list_remove(lcs, pauline->lc);
	/*marie's dialog will expire while the SUBSCRIBE refresh is in progress*/
	wait_for_list(lcs, NULL, 0, 8000);

	lcs = bctbx_list_append(lcs, pauline->lc);
	wait_for_list(lcs, NULL, 0, 3000);
	linphone_event_terminate(lev);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionTerminated,1,5000));

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}


static void publish_test_with_args(bool_t refresh, int expires){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lp_config_set_int(linphone_core_get_config(marie->lc),"sip","refresh_generic_publish",refresh);

	lev=linphone_core_create_publish(marie->lc,pauline->identity,"dodo",expires);
	linphone_event_add_custom_header(lev,"CustomHeader","someValue");
	linphone_event_ref(lev);
	linphone_event_send_publish(lev,content);


	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePublishProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePublishOk,1,3000));

	if (!refresh){
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePublishExpiring,1,5000));
		linphone_event_update_publish(lev,content);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePublishProgress,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePublishOk,1,3000));
	}else{

	}

	linphone_event_terminate(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePublishCleared,1,3000));

	linphone_event_unref(lev);

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void publish_test(void){
	publish_test_with_args(TRUE,5);
}

static void publish_no_auto_test(void){
	publish_test_with_args(FALSE,5);
}

static void publish_without_expires(void){
	publish_test_with_args(TRUE,-1);
}

static void out_of_dialog_notify(void){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content,(const uint8_t *)notify_content,strlen(notify_content));

	lev = linphone_core_create_notify(marie->lc,pauline->identity,"dodo");
	linphone_event_ref(lev);
	linphone_event_add_custom_header(lev,"CustomHeader","someValue");
	linphone_event_notify(lev,content);


	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_NotifyReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,3000));

	linphone_event_unref(lev);

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t event_tests[] = {
	TEST_ONE_TAG("Subscribe declined", subscribe_test_declined, "presence"),
	TEST_ONE_TAG("Subscribe terminated by subscriber", subscribe_test_terminated_by_subscriber, "presence"),
	TEST_ONE_TAG("Subscribe with custom headers", subscribe_test_with_custom_header, "presence"),
	TEST_ONE_TAG("Subscribe refreshed", subscribe_test_refreshed, "presence"),
	TEST_ONE_TAG("Subscribe loosing dialog", subscribe_loosing_dialog, "presence"),
	TEST_ONE_TAG("Subscribe with io error", subscribe_with_io_error, "presence"),
	TEST_ONE_TAG("Subscribe manually refreshed", subscribe_test_manually_refreshed, "presence"),
	TEST_ONE_TAG("Subscribe terminated by notifier", subscribe_test_terminated_by_notifier, "presence"),
	TEST_ONE_TAG("Subscribe not timely responded", subscribe_not_timely_responded, "presence"),
	TEST_ONE_TAG("Publish", publish_test, "presence"),
	TEST_ONE_TAG("Publish without expires", publish_without_expires, "presence"),
	TEST_ONE_TAG("Publish without automatic refresh",publish_no_auto_test, "presence"),
	TEST_ONE_TAG("Out of dialog notify", out_of_dialog_notify, "presence")
};

test_suite_t event_test_suite = {"Event", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								 sizeof(event_tests) / sizeof(event_tests[0]), event_tests};
