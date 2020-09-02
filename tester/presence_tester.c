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


#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"


static LinphoneCoreManager* presence_linphone_core_manager_new_with_rc_name(char* username, char * rc_name) {
	LinphoneCoreManager* mgr= linphone_core_manager_new2( rc_name, FALSE);
	LinphoneSipTransports tr = {0};
	tr.udp_port = -1; /*random port*/
	linphone_core_set_sip_transports(mgr->lc, &tr);
	linphone_core_get_sip_transports_used(mgr->lc, &tr);
	char* identity_char;
	mgr->identity= linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,username);
	linphone_address_set_port(mgr->identity, tr.udp_port);
	identity_char=linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);
	ms_free(identity_char);
	return mgr;
}

static LinphoneCoreManager* presence_linphone_core_manager_new(char* username) {
	return presence_linphone_core_manager_new_with_rc_name(username, "empty_rc");
}

static bool_t subscribe_to_callee_presence(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr) {
	stats initial_caller=caller_mgr->stat;
	stats initial_callee=callee_mgr->stat;
	bool_t result=FALSE;
	char* identity=linphone_address_as_string(callee_mgr->identity);


	LinphoneFriend* friend=linphone_core_create_friend_with_address(caller_mgr->lc,identity);
	linphone_friend_edit(friend);
	linphone_friend_enable_subscribes(friend,TRUE);
	linphone_friend_done(friend);

	linphone_core_add_friend(caller_mgr->lc,friend);
	linphone_friend_unref(friend);

	result=wait_for(caller_mgr->lc,callee_mgr->lc,&caller_mgr->stat.number_of_LinphonePresenceActivityOnline,initial_caller.number_of_LinphonePresenceActivityOnline+1);
	/*without proxy, callee cannot subscribe to caller
	result&=wait_for(caller_mgr->lc,callee_mgr->lc,&callee_mgr->stat.number_of_LinphonePresenceActivityOnline,initial_callee.number_of_LinphonePresenceActivityOnline+1);
	*/

	BC_ASSERT_EQUAL(callee_mgr->stat.number_of_NewSubscriptionRequest,initial_callee.number_of_NewSubscriptionRequest+1, int, "%d");
	/*without proxy, callee cannot subscribe to caller
	BC_ASSERT_EQUAL(callee_mgr->stat.number_of_NotifyPresenceReceived,initial_callee.number_of_NotifyPresenceReceived+1, int, "%d");
	*/
	BC_ASSERT_EQUAL(caller_mgr->stat.number_of_NotifyPresenceReceived,initial_caller.number_of_NotifyPresenceReceived+1, int, "%d");

	ms_free(identity);
	return result;

}

/* BEWARE this test will fail if the machine it is run on is behind an active firewall not sending ICMP errors on incoming connections! */
/* It will create a leak of NOTIFY because during marie error after network switch off, Pauline will never received the acknowledgement of the notify ...*/
static void subscribe_failure_handle_by_app(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneProxyConfig* config;
	LinphoneFriend* lf;
	char* lf_identity=linphone_address_as_string_uri_only(pauline->identity);

	config = linphone_core_get_default_proxy_config(marie->lc);

	BC_ASSERT_TRUE(subscribe_to_callee_presence(marie,pauline));
	wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_NewSubscriptionRequest,1); /*just to wait for unsubscription even if not notified*/

	sal_set_recv_error(linphone_core_get_sal(marie->lc), 0); /*simulate an error*/

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneRegistrationProgress,2));
	BC_ASSERT_EQUAL(linphone_proxy_config_get_error(config),LinphoneReasonIOError, int, "%d");
	sal_set_recv_error(linphone_core_get_sal(marie->lc), 1);

	lf = linphone_core_get_friend_by_address(marie->lc,lf_identity);
	ms_free(lf_identity);
	BC_ASSERT_PTR_NOT_NULL(lf);
	linphone_friend_edit(lf);
	linphone_friend_enable_subscribes(lf,FALSE); /*disable subscription*/
	linphone_friend_done(lf);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneRegistrationOk,2)); /*wait for register ok*/
	linphone_friend_edit(lf);
	linphone_friend_enable_subscribes(lf,TRUE);
	linphone_friend_done(lf);
	BC_ASSERT_FALSE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_NewSubscriptionRequest,2)); /*just to wait for unsubscription even if not notified*/

	linphone_core_manager_destroy(marie);
	BC_ASSERT_FALSE(wait_for(NULL,pauline->lc,&pauline->stat.number_of_NewSubscriptionRequest,3)); /*just to wait for unsubscription even if not notified*/

	linphone_core_manager_destroy(pauline);
}

static void simple_subscribe(void) {
	LinphoneCoreManager* marie = presence_linphone_core_manager_new("marie");
	LinphoneCoreManager* pauline = presence_linphone_core_manager_new("pauline");

	BC_ASSERT_TRUE(subscribe_to_callee_presence(marie,pauline));


	linphone_core_manager_destroy(marie);
	/*unsubscribe is not reported ?*/
	BC_ASSERT_FALSE(wait_for(NULL,pauline->lc,&pauline->stat.number_of_NewSubscriptionRequest,2)); /*just to wait for unsubscription even if not notified*/

	linphone_core_manager_destroy(pauline);
}
static void simple_subscribe_with_early_notify(void) {

	LinphoneCoreManager* marie = presence_linphone_core_manager_new("marie");
	LinphoneCoreManager* pauline = presence_linphone_core_manager_new("pauline");
	LinphoneAddress *marie_identity_addr = linphone_address_clone(marie->identity);
	LpConfig *pauline_lp;

	char* pauline_identity=linphone_address_as_string_uri_only(pauline->identity);
	char* marie_identity;

	LinphoneFriend* pauline_s_friend;
	LinphoneFriend* marie_s_friend=linphone_core_create_friend_with_address(marie->lc,pauline_identity);

	pauline_lp = linphone_core_get_config(pauline->lc);
	linphone_config_set_int(pauline_lp,"sip","notify_pending_state",1);

	linphone_friend_edit(marie_s_friend);
	linphone_friend_enable_subscribes(marie_s_friend,TRUE);
	linphone_friend_done(marie_s_friend);
	linphone_core_add_friend(marie->lc,marie_s_friend);
	ms_free(pauline_identity);


	/*to simulate pending state.*/

	linphone_address_set_port(marie_identity_addr, 0); /*we remove the port because friends aren't supposed to have port numbers*/
	marie_identity=linphone_address_as_string_uri_only(marie_identity_addr);
	pauline_s_friend=linphone_core_create_friend_with_address(pauline->lc,marie_identity);
	linphone_friend_enable_subscribes(pauline_s_friend, FALSE);
	linphone_core_add_friend(pauline->lc,pauline_s_friend);

	ms_free(marie_identity);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_NotifyPresenceReceived,1));
	BC_ASSERT_EQUAL(linphone_friend_get_subscription_state(marie_s_friend), LinphoneSubscriptionPending,int, "%d");

	wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityOnline,marie->stat.number_of_LinphonePresenceActivityOnline+1);

	BC_ASSERT_EQUAL(marie->stat.number_of_NotifyPresenceReceived,2, int, "%d");

	linphone_friend_unref(marie_s_friend);
	linphone_friend_unref(pauline_s_friend);
	linphone_address_unref(marie_identity_addr);
	linphone_core_manager_destroy(marie);

	linphone_core_manager_destroy(pauline);
}


static void unsubscribe_while_subscribing(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneFriend* friend = linphone_core_create_friend_with_address(marie->lc, "sip:toto@git.linphone.org"); /*any unexisting address*/
	linphone_friend_edit(friend);
	linphone_friend_enable_subscribes(friend,TRUE);
	linphone_friend_done(friend);
	linphone_core_add_friend(marie->lc,friend);
	linphone_friend_unref(friend);
	linphone_core_iterate(marie->lc);
	linphone_core_manager_destroy(marie);
}

#if 0
/* the core no longer changes the presence status when a call is ongoing, this is left to the application*/
static void call_with_presence(void) {
	LinphoneCoreManager* marie = presence_linphone_core_manager_new("marie");
	LinphoneCoreManager* pauline = presence_linphone_core_manager_new("pauline");
	LinphoneVideoPolicy pol={0};
	linphone_core_set_video_policy(marie->lc,&pol);
	BC_ASSERT_TRUE(subscribe_to_callee_presence(marie,pauline));
	BC_ASSERT_TRUE(subscribe_to_callee_presence(pauline,marie));

	BC_ASSERT_TRUE(call(marie,pauline));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityOnThePhone,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphonePresenceActivityOnThePhone,1));

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphonePresenceActivityOnline,1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityOnline,1));
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#endif

static void presence_information(void) {
	const char *bike_description = "Riding my bike";
	const char *vacation_note = "I'm on vacation until July 4th";
	const char *vacation_lang = "en";
	const char *contact = "sip:toto@example.com";
	LinphoneCoreManager *marie = presence_linphone_core_manager_new("marie");
	LinphoneCoreManager *pauline = presence_linphone_core_manager_new("pauline");
	LinphonePresenceModel *presence;
	LinphonePresenceActivity *activity = NULL;
	LinphonePresenceNote *note = NULL;
	const char *description = NULL;
	const char *note_content = NULL;
	char *contact2;
	time_t current_timestamp, presence_timestamp;

	BC_ASSERT_TRUE(subscribe_to_callee_presence(marie, pauline));

	/* Presence activity without description. */
	presence = linphone_presence_model_new_with_activity(LinphonePresenceActivityDinner, NULL);
	linphone_core_set_presence_model(pauline->lc, presence);
	linphone_presence_model_unref(presence);
	wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityDinner,1);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphonePresenceActivityDinner, 1, int, "%d");
	activity = linphone_presence_model_get_activity(marie->stat.last_received_presence);
	BC_ASSERT_PTR_NOT_NULL(activity);
	BC_ASSERT_EQUAL(linphone_presence_activity_get_type(activity), LinphonePresenceActivityDinner, int, "%d");
	description = linphone_presence_activity_get_description(activity);
	BC_ASSERT_PTR_NULL(description);

	/* Presence activity with description. */
	presence = linphone_presence_model_new_with_activity(LinphonePresenceActivitySteering, bike_description);
	linphone_core_set_presence_model(pauline->lc, presence);
	linphone_presence_model_unref(presence);
	wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivitySteering,1);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphonePresenceActivitySteering, 1, int, "%d");
	activity = linphone_presence_model_get_activity(marie->stat.last_received_presence);
	BC_ASSERT_PTR_NOT_NULL(activity);
	BC_ASSERT_EQUAL(linphone_presence_activity_get_type(activity), LinphonePresenceActivitySteering, int, "%d");
	description = linphone_presence_activity_get_description(activity);
	BC_ASSERT_PTR_NOT_NULL(description);
	if (description != NULL) BC_ASSERT_STRING_EQUAL(description, bike_description);

	/* Presence activity with description and note. */
	presence = linphone_presence_model_new_with_activity_and_note(LinphonePresenceActivityVacation, NULL, vacation_note, vacation_lang);
	linphone_core_set_presence_model(pauline->lc, presence);
	linphone_presence_model_unref(presence);
	wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityVacation,1);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphonePresenceActivityVacation, 1, int, "%d");
	activity = linphone_presence_model_get_activity(marie->stat.last_received_presence);
	BC_ASSERT_PTR_NOT_NULL(activity);
	BC_ASSERT_EQUAL(linphone_presence_activity_get_type(activity), LinphonePresenceActivityVacation, int, "%d");
	description = linphone_presence_activity_get_description(activity);
	BC_ASSERT_PTR_NULL(description);
	note = linphone_presence_model_get_note(marie->stat.last_received_presence, NULL);
	BC_ASSERT_PTR_NOT_NULL(note);
	if (note != NULL) {
		note_content = linphone_presence_note_get_content(note);
		BC_ASSERT_PTR_NOT_NULL(note_content);
		if (note_content != NULL) {
			BC_ASSERT_STRING_EQUAL(note_content, vacation_note);
		}
	}

	/* Presence contact. */
	presence = linphone_presence_model_new_with_activity(LinphonePresenceActivityOnThePhone, NULL);
	linphone_presence_model_set_contact(presence, contact);
	linphone_core_set_presence_model(pauline->lc, presence);
	linphone_presence_model_unref(presence);
	wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityOnThePhone,1);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphonePresenceActivityOnThePhone, 1, int, "%d");
	contact2 = linphone_presence_model_get_contact(presence);
	BC_ASSERT_PTR_NOT_NULL(contact2);
	if (contact2 != NULL) {
		BC_ASSERT_STRING_EQUAL(contact, contact2);
		ms_free(contact2);
	}

	/* Presence timestamp. */
	current_timestamp = ms_time(NULL);
	presence = linphone_presence_model_new_with_activity(LinphonePresenceActivityShopping, NULL);
	linphone_core_set_presence_model(pauline->lc, presence);
	wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityShopping,1);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphonePresenceActivityShopping, 1, int, "%d");
	presence_timestamp = linphone_presence_model_get_timestamp(presence);
	BC_ASSERT_GREATER((unsigned)presence_timestamp , (unsigned)current_timestamp, unsigned, "%u");
	linphone_presence_model_unref(presence);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static void subscribe_presence_forked(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline1 = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_tcp_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* pauline2 = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_tcp_rc" : "pauline_tcp_rc");
	LinphoneFriend *lf;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline1->lc);
	lcs = bctbx_list_append(lcs, pauline2->lc);

	lf = linphone_core_create_friend(marie->lc);
	linphone_friend_set_address(lf, pauline1->identity);
	linphone_friend_enable_subscribes(lf, TRUE);

	linphone_core_add_friend(marie->lc, lf);
	linphone_friend_unref(lf);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_NewSubscriptionRequest,1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline2->stat.number_of_NewSubscriptionRequest,1, 2000));

	/*we should get only one notify*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePresenceActivityOnline,1, 10000));
	BC_ASSERT_FALSE(wait_for_list(lcs,&marie->stat.number_of_LinphonePresenceActivityOnline,2, 2000));

	/*marie also shall receive two SUBSCRIBEs from the two paulines, but won't be notified to the app since
	 Marie set Pauline as a friend.*/
	BC_ASSERT_EQUAL(marie->stat.number_of_NewSubscriptionRequest, 0, int, "%d");
	/*and the two paulines shall be notified of marie's presence*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_LinphonePresenceActivityOnline,1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline2->stat.number_of_LinphonePresenceActivityOnline,1, 2000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);

	bctbx_list_free(lcs);
}

static void subscribe_presence_expired(void){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline1 = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneFriend *lf;
	bctbx_list_t *lcs = NULL;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline1->lc);

	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "subscribe_expires", 10);

	lf = linphone_core_create_friend(marie->lc);
	linphone_friend_set_address(lf, pauline1->identity);
	linphone_friend_enable_subscribes(lf, TRUE);

	linphone_core_add_friend(marie->lc, lf);
	linphone_friend_unref(lf);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_NewSubscriptionRequest,1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphonePresenceActivityOnline,1, 2000));

	lf = linphone_core_find_friend(pauline1->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(lf);
	if (lf) {
		BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_insubs(lf));

		/*marie comes offline suddenly*/
		linphone_core_set_network_reachable(marie->lc, FALSE);
		/*after a certain time, pauline shall see the incoming SUBSCRIBE expired*/
		wait_for_list(lcs,NULL, 0, 11000);
		BC_ASSERT_PTR_NULL(linphone_friend_get_insubs(lf));

		/*just make network reachable so that marie can unregister properly*/
		linphone_core_set_network_reachable(marie->lc, TRUE);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneRegistrationOk,2, 10000));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline1);

	bctbx_list_free(lcs);
}

static void simple_subscribe_with_friend_from_rc(void) {
	LinphoneCoreManager* pauline = presence_linphone_core_manager_new("pauline");
	LinphoneCoreManager *marie = presence_linphone_core_manager_new_with_rc_name("marie", "pauline_as_friend_rc");
	LinphoneFriend *pauline_as_friend;

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_friend_list(marie->lc)), 1, unsigned int , "%u");

	if (bctbx_list_size(linphone_core_get_friend_list(marie->lc))>0) {
		pauline_as_friend = (LinphoneFriend*)linphone_core_get_friend_list(marie->lc)->data;
		linphone_friend_edit(pauline_as_friend);
		linphone_friend_set_address(pauline_as_friend, pauline->identity); /*hack to update addr with port number*/
		linphone_friend_done(pauline_as_friend);
	}

	BC_ASSERT_TRUE (wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphonePresenceActivityOnline,1));

	BC_ASSERT_EQUAL(pauline->stat.number_of_NewSubscriptionRequest,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_NotifyPresenceReceived,1, int, "%d");

	linphone_core_manager_destroy(marie);
	/*unsubscribe is not reported ?*/
	BC_ASSERT_FALSE(wait_for(NULL,pauline->lc,&pauline->stat.number_of_NewSubscriptionRequest,2)); /*just to wait for unsubscription even if not notified*/

	linphone_core_manager_destroy(pauline);
}

test_t presence_tests[] = {
	TEST_ONE_TAG("Simple Subscribe", simple_subscribe,"presence"),
	TEST_ONE_TAG("Simple Subscribe with early NOTIFY", simple_subscribe_with_early_notify,"presence"),
	TEST_NO_TAG("Simple Subscribe with friend from rc", simple_subscribe_with_friend_from_rc),
	/*TEST_ONE_TAG("Call with presence", call_with_presence, "LeaksMemory"),*/
	TEST_NO_TAG("Unsubscribe while subscribing", unsubscribe_while_subscribing),
	TEST_NO_TAG("Presence information", presence_information),
	TEST_ONE_TAG("App managed presence failure", subscribe_failure_handle_by_app,"presence"),
	TEST_NO_TAG("Presence SUBSCRIBE forked", subscribe_presence_forked),
	TEST_NO_TAG("Presence SUBSCRIBE expired", subscribe_presence_expired),
};

test_suite_t presence_test_suite = {"Presence", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
									sizeof(presence_tests) / sizeof(presence_tests[0]), presence_tests};
