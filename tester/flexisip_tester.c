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
#include "liblinphone_tester.h"
#include "tester_utils.h"

static void setPublish(LinphoneProxyConfig * proxy_config, bool_t enable) {
	linphone_proxy_config_edit(proxy_config);
	linphone_proxy_config_enable_publish(proxy_config, enable);
	linphone_proxy_config_done(proxy_config);
}

static void subscribe_forking(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* pauline2 = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneContent* content;
	LinphoneEvent *lev;
	int expires=  600;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,pauline2->lc);

	content = linphone_core_create_content(marie->lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml");
	linphone_content_set_buffer(content, (const uint8_t *)liblinphone_tester_get_subscribe_content(), strlen(liblinphone_tester_get_subscribe_content()));

	lev=linphone_core_subscribe(marie->lc,pauline->identity,"dodo",expires,content);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneSubscriptionIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline2->stat.number_of_LinphoneSubscriptionIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,1000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,1000));

	linphone_event_terminate(lev);

	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(pauline2);
	bctbx_list_free(lcs);
}

static void message_forking(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage* message = linphone_chat_room_create_message(chat_room,"Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(message);

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);

	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneMessageReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneMessageReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneMessageDelivered,1,1000));
	linphone_chat_message_unref(message);

	/*wait a bit that 200Ok for MESSAGE are sent to server before shuting down the cores, because otherwise Flexisip will consider the messages
	 * as not delivered and will expedite them in the next test, after receiving the REGISTER from marie's instances*/
	wait_for_list(lcs, NULL, 0, 2000);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,1, int, "%d");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void message_forking_with_unreachable_recipients(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage* message = linphone_chat_room_create_message(chat_room,"Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(message);

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	/*the following lines are to workaround a problem with messages sent by a previous test (Message forking) that arrive together with REGISTER responses,
	 * because the ForkMessageContext is not terminated at flexisip side if Message forking test is passing fast*/
	wait_for_list(lcs,NULL,0,1000);
	marie->stat.number_of_LinphoneMessageReceived = 0;
	marie2->stat.number_of_LinphoneMessageReceived = 0;
	marie3->stat.number_of_LinphoneMessageReceived = 0;

	/*marie2 and marie3 go offline*/
	linphone_core_set_network_reachable(marie2->lc,FALSE);
	linphone_core_set_network_reachable(marie3->lc,FALSE);

	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneMessageReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneMessageDelivered,1,1000));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,1, int, "%d");
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	BC_ASSERT_EQUAL(marie3->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	linphone_chat_message_unref(message);

	/*marie 2 goes online */
	linphone_core_set_network_reachable(marie2->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneMessageReceived,1,3000));

	/*wait a long time so that all transactions are expired*/
	wait_for_list(lcs,NULL,0,32000);

	/*marie 3 goes online now*/
	linphone_core_set_network_reachable(marie3->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneMessageReceived,1,3000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void message_forking_with_all_recipients_unreachable(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage* message = linphone_chat_room_create_message(chat_room,"Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(message);

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	/*the following lines are to workaround a problem with messages sent by a previous test (Message forking) that arrive together with REGISTER responses,
	 * because the ForkMessageContext is not terminated at flexisip side if Message forking test is passing fast*/
	wait_for_list(lcs,NULL,0,1000);
	marie->stat.number_of_LinphoneMessageReceived = 0;
	marie2->stat.number_of_LinphoneMessageReceived = 0;
	marie3->stat.number_of_LinphoneMessageReceived = 0;


	/*All marie's device go offline*/
	linphone_core_set_network_reachable(marie->lc,FALSE);
	linphone_core_set_network_reachable(marie2->lc,FALSE);
	linphone_core_set_network_reachable(marie3->lc,FALSE);

	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(message);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneMessageInProgress,1,5000));
	/*flexisip will accept the message with 202 after 16 seconds*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneMessageDelivered,1,18000));
	BC_ASSERT_EQUAL( marie->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	BC_ASSERT_EQUAL( marie2->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	BC_ASSERT_EQUAL( marie3->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	linphone_chat_message_unref(message);

	/*marie 1 goes online */
	linphone_core_set_network_reachable(marie->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneMessageReceived,1,3000));

	/*marie 2 goes online */
	linphone_core_set_network_reachable(marie2->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneMessageReceived,1,3000));

	/*wait a long time so that all transactions are expired*/
	wait_for_list(lcs,NULL,0,32000);

	/*marie 3 goes online now*/
	linphone_core_set_network_reachable(marie3->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneMessageReceived,1,3000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void message_forking_with_unreachable_recipients_with_gruu(void) {
	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	LinphoneCoreManager *marie2 = ms_new0(LinphoneCoreManager, 1);

	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_manager_init(pauline, transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", NULL);
	linphone_core_manager_init(marie2, "marie_rc", NULL);

	linphone_core_add_supported_tag(marie->lc,"gruu");
	linphone_core_add_supported_tag(pauline->lc,"gruu");
	linphone_core_add_supported_tag(marie2->lc,"gruu");

	linphone_core_manager_start(marie,TRUE);
	linphone_core_manager_start(pauline,TRUE);
	linphone_core_manager_start(marie2,TRUE);

	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

	LinphoneProxyConfig *marie_proxy_config = linphone_core_get_default_proxy_config(marie->lc);
	const LinphoneAddress *marie_address = linphone_proxy_config_get_contact(marie_proxy_config);
	LinphoneChatRoom* chat_room_1 = linphone_core_get_chat_room(pauline->lc, marie_address);
	LinphoneChatMessage* message_1 = linphone_chat_room_create_message(chat_room_1,"Bli bli bli \n blu");

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);

	/*the following lines are to workaround a problem with messages sent by a previous test (Message forking) that arrive together with REGISTER responses,
	 * because the ForkMessageContext is not terminated at flexisip side if Message forking test is passing fast*/
	wait_for_list(lcs,NULL,0,1000);
	marie->stat.number_of_LinphoneMessageReceived = 0;
	marie2->stat.number_of_LinphoneMessageReceived = 0;

	/*marie and marie2 go offline*/
	linphone_core_set_network_reachable(marie->lc,FALSE);
	linphone_core_set_network_reachable(marie2->lc,FALSE);

	linphone_chat_room_send_chat_message(chat_room_1, message_1);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneMessageReceived, 0, int, "%d");

	/*marie 2 goes online */
	linphone_core_set_network_reachable(marie2->lc,TRUE);
	BC_ASSERT_FALSE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneMessageReceived,1,3000));

	/*wait a long time so that all transactions are expired*/
	wait_for_list(lcs,NULL,0,32000);

	/*marie goes online now*/
	linphone_core_set_network_reachable(marie->lc,TRUE);
	if (BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneMessageReceived,1,3000))) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), linphone_chat_message_get_text(message_1));
	}
	linphone_chat_message_unref(message_1);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void text_message_expires(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new4("marie_rc",TRUE,NULL,"message-expires=60",3);
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	linphone_core_set_network_reachable(marie->lc, FALSE);
	/* Wait for 5 seconds for surely cut marie of network */
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 5000);

	linphone_chat_room_send_message(linphone_core_get_chat_room(pauline->lc,marie->identity), "hello");
	linphone_core_set_network_reachable(marie->lc, TRUE);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_call_expires(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new4("marie_rc",TRUE,NULL,"message-expires=60",3);
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);
	lcs=bctbx_list_append(lcs,marie->lc);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	/* Wait for 5 seconds for surely cut marie of network */
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 5000);

	linphone_core_invite_address(pauline->lc,marie->identity);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	/*pauline shouldn't hear ringback*/
	BC_ASSERT_FALSE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,5000));
	/*all devices from Marie shouldn't  be ringing*/
	BC_ASSERT_FALSE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,5000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void call_forking(void){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_invite_address(pauline->lc,marie->identity);
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*all devices from Marie should be ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallIncomingReceived,1,3000));

	/*marie accepts the call on its first device*/
	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));

	/*other devices should stop ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,1000));

	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

static void call_forking_with_urgent_reply(void){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);
	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	if (linphone_core_media_encryption_supported(pauline->lc,LinphoneMediaEncryptionSRTP)) {
		linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
		linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
		linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);
		linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);


		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionSRTP);
		linphone_core_set_network_reachable(marie2->lc,FALSE);
		linphone_core_set_network_reachable(marie3->lc,FALSE);

		linphone_core_invite_address(pauline->lc,marie->identity);
		/*pauline should hear ringback, after 5 seconds, when it will retry without SRTP*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,9000));
		/*Marie should be ringing*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,1000));

		/*marie accepts the call on its first device*/
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));

		linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
	}
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

static void call_forking_cancelled(void){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_invite_address(pauline->lc,marie->identity);
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*all devices from Marie should be ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallIncomingReceived,1,1000));

	/*pauline finally cancels the call*/
	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));

	/*all devices should stop ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,1000));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

static void call_forking_declined(bool_t declined_globaly){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_invite_address(pauline->lc,marie->identity);
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*all devices from Marie should be ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallIncomingReceived,1,1000));

	/*marie finally declines the call*/
	linphone_call_decline(linphone_core_get_current_call(marie->lc),
		declined_globaly ? LinphoneReasonDeclined : LinphoneReasonBusy
	);

	if (declined_globaly){
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
		/*all devices should stop ringing*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,1000));
	}else{
		/*pauline should continue ringing and be able to hear a call taken by marie2 */
		linphone_call_accept(linphone_core_get_current_call(marie2->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,2000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallStreamsRunning,1,2000));
		liblinphone_tester_check_rtcp(pauline,marie2);
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,3000));
		linphone_call_terminate(linphone_core_get_current_call(marie2->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,3000));
	}

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

static void call_forking_declined_globaly(void){
	call_forking_declined(TRUE);
}

static void call_forking_declined_localy(void){
	call_forking_declined(FALSE);
}

static void call_forking_with_push_notification_single(void){
	bctbx_list_t* lcs;
	LinphoneCoreManager* marie = linphone_core_manager_new2( "marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc",FALSE);
	int dummy=0;

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);
	linphone_proxy_config_set_contact_uri_parameters(
		linphone_core_get_default_proxy_config(marie->lc),
		"app-id=org.linphonetester;pn-tok=aaabbb;pn-type=apple;pn-msg-str=33;pn-call-str=34;");

	lcs=bctbx_list_append(NULL,pauline->lc);
	lcs=bctbx_list_append(lcs,marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneRegistrationOk,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneRegistrationOk,1,5000));

	/*unfortunately marie gets unreachable due to crappy 3G operator or iOS bug...*/
	linphone_core_set_network_reachable(marie->lc,FALSE);

	linphone_core_invite_address(pauline->lc,marie->identity);

	/*After 5 seconds the server is expected to send a push notification to marie, this will wake up linphone, that will reconnect:*/
	wait_for_list(lcs,&dummy,1,6000);
	linphone_core_set_network_reachable(marie->lc,TRUE);

	/*Marie shall receive the call immediately*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,5000));
	/*pauline should hear ringback as well*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,1000));

	/*marie accepts the call*/
	if (BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));

		liblinphone_tester_check_rtcp(pauline,marie);

		linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,5000));
	}
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

/*
 * This test is a variant of push notification (single) where the client do send ambigous REGISTER with two contacts,
 * one of them being the previous contact address with "expires=0" to tell the server to remove it.
**/
static void call_forking_with_push_notification_double_contact(void){
	bctbx_list_t* lcs;
	LinphoneCoreManager* marie = linphone_core_manager_new2( "marie_rc", FALSE);
	LinphoneCoreManager* pauline = linphone_core_manager_new2( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc",FALSE);
	int dummy=0;


	lp_config_set_int(linphone_core_get_config(marie->lc), "sip", "unregister_previous_contact", 1);
	lp_config_set_int(linphone_core_get_config(pauline->lc), "sip", "unregister_previous_contact", 1);
	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);
	linphone_proxy_config_set_contact_uri_parameters(
		linphone_core_get_default_proxy_config(marie->lc),
		"app-id=org.linphonetester;pn-tok=aaabbb;pn-type=apple;pn-msg-str=33;pn-call-str=34;");

	lcs=bctbx_list_append(NULL,pauline->lc);
	lcs=bctbx_list_append(lcs,marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneRegistrationOk,1,5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneRegistrationOk,1,5000));

	/*unfortunately marie gets unreachable due to crappy 3G operator or iOS bug...*/
	linphone_core_set_network_reachable(marie->lc,FALSE);

	linphone_core_invite_address(pauline->lc,marie->identity);

	/*After 5 seconds the server is expected to send a push notification to marie, this will wake up linphone, that will reconnect:*/
	wait_for_list(lcs,&dummy,1,6000);
	linphone_core_set_network_reachable(marie->lc,TRUE);

	/*Marie shall receive the call immediately*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,5000));
	/*pauline should hear ringback as well*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,1000));

	/*marie accepts the call*/
	if (BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie->lc))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));

		liblinphone_tester_check_rtcp(pauline,marie);

		linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,5000));
	}
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

static void call_forking_with_push_notification_multiple(void){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");

	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	/*unfortunately marie gets unreachable due to crappy 3G operator or iOS bug...*/
	linphone_core_set_network_reachable(marie2->lc,FALSE);

	linphone_core_invite_address(pauline->lc,marie->identity);

	/*marie will ring*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,5000));
	/*pauline should hear ringback as well*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,1000));

	/*the server is expected to send a push notification to marie2, this will wake up linphone, that will reconnect:*/
	linphone_core_set_network_reachable(marie2->lc,TRUE);

	/*Marie shall receive the call immediately*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,5000));

	/*marie2 accepts the call*/
	if (BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(marie2->lc))) {
		linphone_call_accept(linphone_core_get_current_call(marie2->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallStreamsRunning,1,1000));

		/*call to marie should be cancelled*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));

		liblinphone_tester_check_rtcp(pauline,marie2);

		linphone_call_terminate(linphone_core_get_current_call(pauline->lc));

		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));
	}
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
}

static void call_forking_not_responded(void){
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_new( "marie_rc");
	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_invite_address(pauline->lc,marie->identity);
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*all devices from Marie should be ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallIncomingReceived,1,1000));

	/*nobody answers, flexisip should close the call after XX seconds*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallError,1,22000));
	/*all devices should stop ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,1000));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

static void early_media_call_forking(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_new("marie_early_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bctbx_list_t *lcs=NULL;
	LinphoneCallParams *params=linphone_core_create_call_params(pauline->lc, NULL);
	LinphoneVideoPolicy pol;
	int dummy=0;

	pol.automatically_accept=1;
	pol.automatically_initiate=1;

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_enable_video_capture(pauline->lc,TRUE);
	linphone_core_enable_video_display(pauline->lc,TRUE);

	linphone_core_enable_video_capture(marie->lc,TRUE);
	linphone_core_enable_video_display(marie->lc,TRUE);
	linphone_core_set_video_policy(marie->lc,&pol);

	linphone_core_enable_video_capture(marie2->lc,TRUE);
	linphone_core_enable_video_display(marie2->lc,TRUE);

	linphone_core_set_video_policy(marie2->lc,&pol);
	linphone_core_set_audio_port_range(marie2->lc,40200,40300);
	linphone_core_set_video_port_range(marie2->lc,40400,40500);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);

	linphone_call_params_enable_early_media_sending(params,TRUE);
	linphone_call_params_enable_video(params,TRUE);

	linphone_core_invite_address_with_params(pauline->lc,marie->identity,params);
	linphone_call_params_unref(params);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingEarlyMedia,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallIncomingEarlyMedia,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia,1,3000));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia,1, int, "%d");

	/*wait a bit that streams are established*/
	wait_for_list(lcs,&dummy,1,5000);
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(pauline), 60, int, "%d");
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(pauline), 99, int, "%d");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie), 60, int, "%d");
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(marie), 99, int, "%d");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie2), 60, int, "%d");
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(marie2), 99, int, "%d");

	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));

	/*marie2 should get her call terminated*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));

	/*wait a bit that streams are established*/
	wait_for_list(lcs,&dummy,1,3000);
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(pauline), 60, int, "%d");
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(pauline), 99, int, "%d");
	BC_ASSERT_GREATER(linphone_core_manager_get_mean_audio_down_bw(marie), 60, int, "%d");
	BC_ASSERT_LOWER(linphone_core_manager_get_mean_audio_down_bw(marie), 99, int, "%d");

	end_call(pauline, marie);

	bctbx_list_free(lcs);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie);
}

static void call_with_sips(void){
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager* marie = linphone_core_manager_new( "marie_sips_rc");
		LinphoneCoreManager* pauline1 = linphone_core_manager_new( "pauline_sips_rc");
		LinphoneCoreManager* pauline2 = linphone_core_manager_new( "pauline_tcp_rc");
		bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);

		lcs=bctbx_list_append(lcs,pauline1->lc);
		lcs=bctbx_list_append(lcs,pauline2->lc);

		linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
		linphone_core_set_user_agent(pauline1->lc,"Natted Linphone",NULL);
		linphone_core_set_user_agent(pauline2->lc,"Natted Linphone",NULL);

		linphone_core_invite_address(marie->lc,pauline1->identity);

		/*marie should hear ringback*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
		/*Only the sips registered device from pauline should ring*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_LinphoneCallIncomingReceived,1,1000));

		/*pauline accepts the call */
		linphone_call_accept(linphone_core_get_current_call(pauline1->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_LinphoneCallStreamsRunning,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));

		/*pauline2 should not have ring*/
		BC_ASSERT_EQUAL(pauline2->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

		linphone_call_terminate(linphone_core_get_current_call(pauline1->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs,&pauline1->stat.number_of_LinphoneCallEnd,1,3000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,3000));

		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline1);
		linphone_core_manager_destroy(pauline2);
		bctbx_list_free(lcs);
	}
}

static void call_with_sips_not_achievable(void){
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager* pauline2 = linphone_core_manager_new( "pauline_tcp_rc");
		LinphoneCoreManager* marie = linphone_core_manager_new( "marie_sips_rc");
		LinphoneCoreManager* pauline1 = linphone_core_manager_new( "pauline_rc");
		bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
		LinphoneAddress *dest;
		LinphoneCall *call;
		const LinphoneErrorInfo *ei;

		lcs=bctbx_list_append(lcs,pauline1->lc);
		lcs=bctbx_list_append(lcs,pauline2->lc);


		dest=linphone_address_clone(pauline1->identity);
		linphone_address_set_secure(dest,TRUE);
		call=linphone_core_invite_address(marie->lc,dest);
		linphone_call_ref(call);
		linphone_address_unref(dest);

		/*Call should be rejected by server with 480*/
		BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallError,1,6000));
		ei=linphone_call_get_error_info(call);
		BC_ASSERT_PTR_NOT_NULL(ei);
		if (ei){
			BC_ASSERT_EQUAL(linphone_error_info_get_reason(ei), LinphoneReasonTemporarilyUnavailable, int, "%d");
		}
		linphone_call_unref(call);

		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline1);
		linphone_core_manager_destroy(pauline2);
		bctbx_list_free(lcs);
	}
}


static bool_t is_sending_ipv6(RtpSession *session, bool_t rtcp){
	const struct sockaddr *dest = rtcp ? (struct sockaddr*)&session->rtcp.gs.rem_addr : (struct sockaddr*)&session->rtp.gs.rem_addr;
	struct sockaddr_in6 *in6=(struct sockaddr_in6*)dest;
	return dest->sa_family == AF_INET6 && !IN6_IS_ADDR_V4MAPPED(&in6->sin6_addr);
}

static bool_t is_remote_contact_ipv6(LinphoneCall *call){
	const char *contact=linphone_call_get_remote_contact(call);
	LinphoneAddress *ct_addr;
	bool_t ret = FALSE;

	BC_ASSERT_PTR_NOT_NULL(contact);
	if (contact){
		ct_addr=linphone_address_new(contact);
		BC_ASSERT_PTR_NOT_NULL(ct_addr);
		if (ct_addr){
			ret = strchr(linphone_address_get_domain(ct_addr),':') != NULL;
		}
		linphone_address_unref(ct_addr);
	}
	return ret;
}

static void _call_with_ipv6(bool_t caller_with_ipv6, bool_t callee_with_ipv6) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	LinphoneCall *pauline_call, *marie_call;

	/*calling ortp_init() here is done to have WSAStartup() done, otherwise liblinphone_tester_ipv6_available() will not work.*/
	ortp_init();

	if (!liblinphone_tester_ipv6_available()){
		ms_warning("Call with ipv6 not tested, no ipv6 connectivity");
		return;
	}

	marie = linphone_core_manager_new2( "marie_rc", FALSE);
	linphone_core_remove_supported_tag(marie->lc,"gruu"); // With gruu, we have no access to the "public IP from contact
	linphone_core_enable_ipv6(marie->lc, caller_with_ipv6);
	linphone_core_manager_start(marie, TRUE);

	pauline = linphone_core_manager_new2( transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", FALSE);
	linphone_core_remove_supported_tag(pauline->lc,"gruu"); // With gruu, we have no access to the "public IP from contact
	linphone_core_enable_ipv6(pauline->lc, callee_with_ipv6);
	linphone_core_manager_start(pauline, TRUE);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);
	BC_ASSERT_TRUE(call(marie,pauline));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	if (pauline_call && marie_call){
		/*check that the remote contact is IPv6*/
		BC_ASSERT_EQUAL(is_remote_contact_ipv6(pauline_call), caller_with_ipv6, int, "%i");
		BC_ASSERT_EQUAL(is_remote_contact_ipv6(marie_call), callee_with_ipv6, int, "%i");

		/*check that the RTP destinations are IPv6 (flexisip should propose an IPv6 relay for parties with IPv6)*/
		BC_ASSERT_EQUAL(is_sending_ipv6(linphone_call_get_stream(marie_call, LinphoneStreamTypeAudio)->sessions.rtp_session, FALSE), caller_with_ipv6, int, "%i");
		BC_ASSERT_EQUAL(is_sending_ipv6(linphone_call_get_stream(marie_call, LinphoneStreamTypeAudio)->sessions.rtp_session, TRUE), caller_with_ipv6, int, "%i");
		BC_ASSERT_EQUAL(is_sending_ipv6(linphone_call_get_stream(pauline_call, LinphoneStreamTypeAudio)->sessions.rtp_session, FALSE), callee_with_ipv6, int, "%i");
		BC_ASSERT_EQUAL(is_sending_ipv6(linphone_call_get_stream(pauline_call, LinphoneStreamTypeAudio)->sessions.rtp_session, TRUE), callee_with_ipv6, int, "%i");
	}

	liblinphone_tester_check_rtcp(marie,pauline);
	end_call(marie,pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	ortp_exit();
}

static void call_with_ipv6(void){
	_call_with_ipv6(TRUE, TRUE);
}

static void call_ipv4_to_ipv6(void){
	_call_with_ipv6(FALSE, TRUE);
}

static void call_ipv6_to_ipv4(void){
	_call_with_ipv6(TRUE, FALSE);
}

static void file_transfer_message_rcs_to_external_body_client(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneChatRoom* chat_room;
		LinphoneChatMessage* message;
		LinphoneChatMessageCbs *cbs;
		LinphoneContent* content;
		FILE *file_to_send = NULL;
		size_t file_size;
		char *send_filepath = bc_tester_res("images/nowebcamCIF.jpg");
		char *receive_filepath = bc_tester_file("receive_file.dump");
		LinphoneCoreManager* marie = linphone_core_manager_new2( "marie_rc", FALSE);
		LinphoneCoreManager* pauline = linphone_core_manager_new2( "pauline_rc", FALSE);
		// This is done to prevent register to be sent before the custom header is set
		linphone_core_set_network_reachable(marie->lc, FALSE);
		linphone_core_set_network_reachable(pauline->lc, FALSE);

		linphone_proxy_config_set_custom_header(linphone_core_get_default_proxy_config(marie->lc), "Accept", "application/sdp");
		linphone_core_set_network_reachable(marie->lc, TRUE);
		linphone_core_manager_start(marie, TRUE);


		linphone_proxy_config_set_custom_header(linphone_core_get_default_proxy_config(pauline->lc), "Accept", "application/sdp, text/plain, application/vnd.gsma.rcs-ft-http+xml");
		linphone_core_set_network_reachable(pauline->lc, TRUE);
		linphone_core_manager_start(pauline, TRUE);

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);

		file_to_send = fopen(send_filepath, "rb");
		fseek(file_to_send, 0, SEEK_END);
		file_size = ftell(file_to_send);
		fseek(file_to_send, 0, SEEK_SET);

		/* Globally configure an http file transfer server. */
		linphone_core_set_file_transfer_server(pauline->lc,"https://www.linphone.org:444/lft.php");

		/* create a chatroom on pauline's side */
		chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

		/* create a file transfer message */
		content = linphone_core_create_content(pauline->lc);
		linphone_content_set_type(content,"image");
		linphone_content_set_subtype(content,"jpeg");
		linphone_content_set_size(content,file_size); /*total size to be transfered*/
		linphone_content_set_name(content,"nowebcamCIF.jpg");
		message = linphone_chat_room_create_file_transfer_message(chat_room, content);
		linphone_chat_message_set_user_data(message, file_to_send);
		cbs = linphone_chat_message_get_callbacks(message);
		{
			int dummy=0;
			wait_for_until(marie->lc,pauline->lc,&dummy,1,100); /*just to have time to purge message stored in the server*/
			reset_counters(&marie->stat);
			reset_counters(&pauline->stat);
		}
		linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
		linphone_chat_message_send(message);
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,1));

		if (marie->stat.last_received_chat_message ) {
			cbs = linphone_chat_message_get_callbacks(marie->stat.last_received_chat_message);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
			linphone_chat_message_download_file(marie->stat.last_received_chat_message);
		}
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageFileTransferDone,1));

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,2, int, "%d");
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered,1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageReceivedWithFile,1, int, "%d");
		compare_files(send_filepath, receive_filepath);

		linphone_chat_message_unref(message);
		linphone_content_unref(content);
		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		ms_free(send_filepath);
		bc_free(receive_filepath);
	}
}

static void dos_module_trigger(void) {
	LinphoneChatRoom *chat_room;
	int dummy = 0;
	const char* passmsg = "This one should pass through";
	LinphoneChatMessage * chat_msg = NULL;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	int message_rate = 20;
	int flood_duration_ms = 8000;
	uint64_t time_begin, time_current;
	int message_sent_index = 0;
	int message_to_send_index = 0;
	
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	time_begin = bctbx_get_cur_time_ms();
	do {
		time_current = bctbx_get_cur_time_ms();
		message_to_send_index = ((time_current - time_begin) * message_rate) / 1000;
		if (message_to_send_index >  message_sent_index){
			char msg[128] = { 0 };
			snprintf(msg, sizeof(msg)-1, "Flood message number %i", message_sent_index);
			chat_msg = linphone_chat_room_create_message(chat_room, msg);
			linphone_chat_message_send(chat_msg);
			message_sent_index++;
		}else{
			wait_for_until(marie->lc, pauline->lc, &dummy, 1, 20);
		}
		time_current = bctbx_get_cur_time_ms();
	} while (time_current - time_begin < (uint64_t)flood_duration_ms);
	// At this point we should be banned for a minute
	BC_ASSERT_GREATER(message_sent_index, message_rate, int, "%d");
	BC_ASSERT_LOWER_STRICT(marie->stat.number_of_LinphoneMessageReceived, message_sent_index - message_rate, int, "%d");
	
	wait_for_until(marie->lc, pauline->lc, &dummy, 1, 65000); // Wait several seconds to ensure we are not banned anymore
	
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	chat_msg = linphone_chat_room_create_message(chat_room, passmsg);
	linphone_chat_message_send(chat_msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageReceived, 1, int, "%d");
	if (marie->stat.last_received_chat_message) {
		BC_ASSERT_NSTRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), passmsg, strlen(passmsg));
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#if HAVE_SIPP
static void test_subscribe_notify_with_sipp_publisher(void) {
	char *scen;
	FILE * sipp_out;
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_rc");
	/*just to get an identity*/
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneAddress *sip_example_org;
	const LinphoneAuthInfo	*marie_auth = linphone_core_find_auth_info(marie->lc, NULL, linphone_address_get_username(marie->identity), NULL);
	LpConfig *pauline_lp = linphone_core_get_config(pauline->lc);
	char* lf_identity=linphone_address_as_string_uri_only(marie->identity);
	LinphoneFriend *lf = linphone_core_create_friend_with_address(pauline->lc,lf_identity);

	linphone_core_set_user_agent(marie->lc, "full-presence-support", NULL);
	linphone_core_set_user_agent(pauline->lc, "full-presence-support", NULL);

	ms_free(lf_identity);

	lp_config_set_int(pauline_lp,"sip","subscribe_expires",5);

	linphone_core_add_friend(pauline->lc,lf);

	/*wait for subscribe acknowledgment*/
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,pauline->lc,&pauline->stat.number_of_NotifyPresenceReceived,1,2000));
	BC_ASSERT_EQUAL(LinphoneStatusOffline,linphone_friend_get_status(lf), int, "%d");

	scen = bc_tester_res("sipp/simple_publish.xml");

	sip_example_org = linphone_core_manager_resolve(marie, marie->identity);
	sipp_out = sip_start(scen, linphone_address_get_username(marie->identity), linphone_auth_info_get_passwd(marie_auth), sip_example_org);
	linphone_address_unref(sip_example_org);

	if (sipp_out) {
		/*wait for marie status*/
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,pauline->lc,&pauline->stat.number_of_NotifyPresenceReceived,2,3000));
		BC_ASSERT_EQUAL(LinphoneStatusOnline,linphone_friend_get_status(lf), int, "%d");
		BC_ASSERT_EQUAL(0,pclose(sipp_out),int,"%d");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

//does not work because sipp seams not able to manage 2 call  id in case file
#if 0
static void test_subscribe_notify_with_sipp_publisher_double_publish(void) {
	char *scen;
	FILE * sipp_out;
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_rc");
	/*just to get an identity*/
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneAddress *sip_example_org;

	linphone_core_set_user_agent(marie->lc, "full-presence-support", NULL);
	linphone_core_set_user_agent(pauline->lc, "full-presence-support", NULL);

	LpConfig *pauline_lp = linphone_core_get_config(pauline->lc);
	char* lf_identity=linphone_address_as_string_uri_only(marie->identity);
	LinphoneFriend *lf = linphone_core_create_friend_with_address(pauline->lc,lf_identity);
	ms_free(lf_identity);
	lp_config_set_int(pauline_lp,"sip","subscribe_expires",5);

	linphone_core_add_friend(pauline->lc,lf);

	/*wait for subscribe acknowledgment*/
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,pauline->lc,&pauline->stat.number_of_NotifyPresenceReceived,1,2000));
	BC_ASSERT_EQUAL(LinphoneStatusOffline,linphone_friend_get_status(lf), int, "%d");

	scen = bc_tester_res("sipp/double_publish_with_error.xml");

	sip_example_org = linphone_core_manager_resolve(marie, marie->identity);
	sipp_out = sip_start(scen, linphone_address_get_username(marie->identity), sip_example_org);

	if (sipp_out) {
		/*wait for marie status*/
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,pauline->lc,&pauline->stat.number_of_NotifyPresenceReceived,2,3000));
		BC_ASSERT_EQUAL(LinphoneStatusOnline,linphone_friend_get_status(lf), int, "%d");
		pclose(sipp_out);
		BC_ASSERT_EQUAL(pauline->stat.number_of_NotifyPresenceReceived,2,int, "%d");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
#endif
#endif

static void test_publish_unpublish(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneProxyConfig* proxy = linphone_core_get_default_proxy_config(marie->lc);

	setPublish(proxy, TRUE);
	wait_for(marie->lc, NULL, NULL, 0);
	setPublish(proxy, FALSE);
	wait_for(marie->lc, NULL, NULL, 0);
	linphone_core_manager_destroy(marie);
}

static void test_list_subscribe (void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( "laure_rc_udp");

	char *list =	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
					"<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\"\n"
					"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
					"<list>\n"
						"\t<entry uri=\"%s\" />\n"
						"\t<entry uri=\"%s\" />\n"
						"\t<entry uri=\"sip:+33952@toto.com;user=phone\" />\n"
					"</list>\n"
					"</resource-lists>\n";


	LinphoneEvent *lev;
	bctbx_list_t* lcs=bctbx_list_append(NULL,marie->lc);
	char * pauline_uri=linphone_address_as_string_uri_only(pauline->identity);
	char * laure_uri=linphone_address_as_string_uri_only(laure->identity);
	char * subscribe_content = ms_strdup_printf(list,pauline_uri,laure_uri);
	LinphoneContent* content = linphone_core_create_content(marie->lc);
	LinphoneAddress *list_name = linphone_address_new("sip:mescops@sip.example.org");
	int dummy=0;

	ms_free(pauline_uri);
	ms_free(laure_uri);

	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"resource-lists+xml");
	linphone_content_set_buffer(content,(const uint8_t *)subscribe_content,strlen(subscribe_content));

	lev=linphone_core_create_subscribe(marie->lc,list_name,"presence",60);

	linphone_event_add_custom_header(lev,"Supported","eventlist");
	linphone_event_add_custom_header(lev,"Accept","application/pidf+xml, application/rlmi+xml");
	linphone_event_add_custom_header(lev,"Content-Disposition", "recipient-list");
	linphone_event_add_custom_header(lev,"Require", "recipient-list-subscribe");

	linphone_event_send_subscribe(lev,content);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionActive,1,5000));

	/*make sure marie receives first notification before terminating*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,1,5000));
	/*dummy wait to avoid derred notify*/
	wait_for_list(lcs,&dummy,1,2000);
	int initial_number_of_notify = marie->stat.number_of_NotifyReceived;

	setPublish(linphone_core_get_default_proxy_config(pauline->lc), TRUE);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,initial_number_of_notify + 1,5000));

	setPublish(linphone_core_get_default_proxy_config(laure->lc), TRUE);
	/*make sure notify is not sent "immediatly but defered*/
	BC_ASSERT_FALSE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,initial_number_of_notify + 2,1000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_NotifyReceived,initial_number_of_notify + 2,5000));

	linphone_event_terminate(lev);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneSubscriptionTerminated,1,5000));

	ms_free(subscribe_content);
	linphone_address_unref(list_name);
	linphone_content_unref(content);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

#if HAVE_SIPP
static void test_subscribe_on_wrong_dialog(void) {
	char *scen;
	FILE * sipp_out;
	/*just to get an identity*/
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	const LinphoneAuthInfo	*marie_auth = linphone_core_find_auth_info(marie->lc, NULL, linphone_address_get_username(marie->identity), NULL);
	LinphoneAddress *sip_example_org;

	scen = bc_tester_res("sipp/subscribe_on_wrong_dialog.xml");
	sip_example_org = linphone_core_manager_resolve(marie, marie->identity);
	sipp_out = sip_start(scen, linphone_address_get_username(marie->identity),linphone_auth_info_get_passwd(marie_auth), sip_example_org);
	linphone_address_unref(sip_example_org);

	if (sipp_out) {
		/*wait for marie status*/
		BC_ASSERT_EQUAL(0, pclose(sipp_out),int,"%d");
	}

	linphone_core_manager_destroy(marie);
}
#endif

static void test_list_subscribe_wrong_body(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");

	LinphoneEvent *lev;
	LinphoneAddress *sub_addr = linphone_address_new("sip:rls@sip.example.com");

	lev=linphone_core_create_subscribe(marie->lc,sub_addr,"presence",60);
	linphone_event_ref(lev);
	linphone_event_add_custom_header(lev,"Supported","eventlist");
	linphone_event_add_custom_header(lev,"Accept","application/pidf+xml, application/rlmi+xml");
	linphone_event_add_custom_header(lev,"Content-Disposition", "recipient-list");
	linphone_event_add_custom_header(lev,"Require", "recipient-list-subscribe");
	linphone_event_add_custom_header(lev,"Content-type", "application/resource-lists+xml");

	linphone_event_send_subscribe(lev,NULL);

	BC_ASSERT_TRUE(wait_for_until(marie->lc,NULL,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,1,1000));
	BC_ASSERT_FALSE(wait_for_until(marie->lc,NULL,&marie->stat.number_of_LinphoneSubscriptionActive,1,2000));

	linphone_event_terminate(lev);
	linphone_event_unref(lev);
	linphone_core_manager_destroy(marie);
	linphone_address_unref(sub_addr);
}


static void redis_publish_subscribe(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = NULL;
	LinphoneAddress *marie_identity = linphone_address_ref(marie->identity);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_manager_destroy(marie);

	linphone_core_invite_address(pauline->lc, marie_identity);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 3000));

	marie2 = linphone_core_manager_new("marie2_rc");
	BC_ASSERT_TRUE(wait_for_until(marie2->lc, NULL, &marie2->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));

	linphone_call_accept(linphone_core_get_current_call(marie2->lc));
	BC_ASSERT_TRUE(wait_for_until(marie2->lc, pauline->lc, &marie2->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
	BC_ASSERT_TRUE(wait_for_until(marie2->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));

	liblinphone_tester_check_rtcp(marie2, pauline);

	linphone_call_terminate(linphone_core_get_current_call(marie2->lc));

	BC_ASSERT_TRUE(wait_for_until(marie2->lc, pauline->lc, &marie2->stat.number_of_LinphoneCallEnd, 1, 3000));
	BC_ASSERT_TRUE(wait_for_until(marie2->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1, 3000));

	linphone_address_unref(marie_identity);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie2);
}


static void tls_authentication_requested_good(LinphoneCore *lc, LinphoneAuthInfo *auth_info, LinphoneAuthMethod method) {
	if (method == LinphoneAuthTls){

		char *cert = bc_tester_res("certificates/client/cert2.pem");
		char *key = bc_tester_res("certificates/client/key2.pem");

		linphone_auth_info_set_tls_cert_path(auth_info, cert);
		linphone_auth_info_set_tls_key_path(auth_info, key);
		linphone_core_add_auth_info(lc, auth_info);
		bc_free(cert);
		bc_free(key);
	}
}

static void tls_authentication_requested_bad(LinphoneCore *lc, LinphoneAuthInfo *auth_info, LinphoneAuthMethod method) {
	if (method == LinphoneAuthTls){

		char *cert = bc_tester_res("certificates/client/cert2-signed-by-other-ca.pem");
		char *key = bc_tester_res("certificates/client/key2.pem");

		linphone_auth_info_set_tls_cert_path(auth_info, cert);
		linphone_auth_info_set_tls_key_path(auth_info, key);
		linphone_core_add_auth_info(lc, auth_info);
		bc_free(cert);
		bc_free(key);
	}
}

static void tls_authentication_provide_altname_cert(LinphoneCore *lc, LinphoneAuthInfo *auth_info, LinphoneAuthMethod method){
	if (method == LinphoneAuthTls){

		char *cert = bc_tester_res("certificates/client/cert3.pem");
		char *key = bc_tester_res("certificates/client/key3.pem");

		linphone_auth_info_set_tls_cert_path(auth_info, cert);
		linphone_auth_info_set_tls_key_path(auth_info, key);
		linphone_core_add_auth_info(lc, auth_info);
		bc_free(cert);
		bc_free(key);
	}
}

static void tls_client_auth_try_register(const char *identity, LinphoneCoreAuthenticationRequestedCb cb, bool_t good_cert, bool_t must_work){
	LinphoneCoreManager *lcm;
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	LinphoneProxyConfig *cfg;

	lcm = linphone_core_manager_new(NULL);

	linphone_core_cbs_set_authentication_requested(cbs, cb);
	linphone_core_add_callbacks(lcm->lc, cbs);
	linphone_core_cbs_unref(cbs);
	cfg = linphone_core_create_proxy_config(lcm->lc);

	linphone_proxy_config_set_server_addr(cfg, "sip:sip2.linphone.org:5063;transport=tls");
	linphone_proxy_config_enable_register(cfg, TRUE);
	linphone_proxy_config_set_identity(cfg, identity);
	linphone_core_add_proxy_config(lcm->lc, cfg);
	if (must_work){
		BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
		BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneRegistrationFailed,0, int, "%d");
		BC_ASSERT_EQUAL(lcm->stat.number_of_auth_info_requested,1, int, "%d");
	}else{
		BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationFailed, 1));
		BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneRegistrationOk,0, int, "%d");
		/*we should expect at least 2 "auth_requested": one for the TLS certificate, another one because the server rejects the REGISTER with 401,
		 with eventually MD5 + SHA256 challenge*/
		/*If the certificate isn't recognized at all, the connection will not happen and no SIP response will be received from server.*/
		if (good_cert) BC_ASSERT_GREATER(lcm->stat.number_of_auth_info_requested,2, int, "%d");

		else BC_ASSERT_EQUAL(lcm->stat.number_of_auth_info_requested,1, int, "%d");
	}

	linphone_proxy_config_unref(cfg);
	linphone_core_manager_destroy(lcm);
}

void tls_client_auth_bad_certificate_cn(void) {
	if (transport_supported(LinphoneTransportTls)) {
		/*first register to the proxy with galadrielle's identity, and authenticate by supplying galadrielle's certificate.
		* It must work.*/
		tls_client_auth_try_register("sip:galadrielle@sip.example.org", tls_authentication_requested_good, TRUE, TRUE);
		/*now do the same thing, but trying to register as "Arwen". It must fail.*/
		tls_client_auth_try_register("sip:arwen@sip.example.org", tls_authentication_requested_good, TRUE, FALSE);
	}
}

void tls_client_auth_bad_certificate(void) {
	if (transport_supported(LinphoneTransportTls)) {
		/*first register to the proxy with galadrielle's identity, and authenticate by supplying galadrielle's certificate.
		* It must work.*/
		tls_client_auth_try_register("sip:galadrielle@sip.example.org", tls_authentication_requested_bad, FALSE, FALSE);
	}
}

/*
 * This test verifies that the flexisip certificate postcheck works.
 * Here, the certificate presented for gandalf is valid and matches the SIP from. However we've set the regexp in flexisip.conf to only accept
 * certificates with subjects containing either galadrielle or sip:sip.example.org.
 */
static void tls_client_rejected_due_to_unmatched_subject(void){
	if (transport_supported(LinphoneTransportTls)) {
		tls_client_auth_try_register("sip:gandalf@sip.example.org", tls_authentication_provide_altname_cert, TRUE, FALSE);
	}
}

static void on_eof(LinphonePlayer *player){
	LinphonePlayerCbs *cbs = linphone_player_get_callbacks(player);
	LinphoneCoreManager *marie=(LinphoneCoreManager*)linphone_player_cbs_get_user_data(cbs);
	marie->stat.number_of_player_eof++;
}

void transcoder_tester(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	LinphonePlayer *player;
	char *hellopath = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	char *recordpath = bc_tester_file("record-call_with_file_player.wav");

	bool_t call_ok;
	double similar=1;
	const double threshold = 0.8;

	linphone_core_set_user_agent(marie->lc,"Transcoded Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Transcoded Linphone",NULL);

	disable_all_audio_codecs_except_one(marie->lc,"pcmu",-1);
	disable_all_audio_codecs_except_one(pauline->lc,"pcma",-1);

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
		LinphonePlayerCbs *cbs = linphone_player_get_callbacks(player);
		linphone_player_cbs_set_eof_reached(cbs, on_eof);
		linphone_player_cbs_set_user_data(cbs, marie);
		BC_ASSERT_EQUAL(linphone_player_open(player,hellopath),0, int, "%d");
		BC_ASSERT_EQUAL(linphone_player_start(player),0, int, "%d");
	}
	/* This assert should be modified to be at least as long as the WAV file */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_player_eof,1,10000));
	/*wait one second more for transmission to be fully ended (transmission time + jitter buffer)*/
	wait_for_until(pauline->lc,marie->lc,NULL,0,1000);

	end_call(marie, pauline);
	/*cannot run on iphone simulator because locks main loop beyond permitted time (should run
	on another thread) */
	BC_ASSERT_EQUAL(ms_audio_diff(hellopath,recordpath,&similar,&audio_cmp_params,NULL,NULL), 0, int, "%d");

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

static void register_without_regid(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new2("marie_rc", FALSE);
	linphone_core_manager_start(marie,TRUE);
	LinphoneProxyConfig *cfg=linphone_core_get_default_proxy_config(marie->lc);
	if(cfg) {
		const LinphoneAddress *addr = linphone_proxy_config_get_contact(cfg);
		char *addrStr = linphone_address_as_string_uri_only(addr);
		BC_ASSERT_PTR_NOT_NULL(addr);
		BC_ASSERT_PTR_NULL(strstr(addrStr, "regid"));
		if (addrStr) ms_free(addrStr);
	}
	linphone_core_manager_destroy(marie);
}

void test_removing_old_tport(void) {
	LinphoneCoreManager* marie1 = linphone_core_manager_new("marie_rc");
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie1->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs,&marie1->stat.number_of_LinphoneRegistrationOk,1,5000));

	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	const char *uuid = linphone_config_get_string(linphone_core_get_config(marie1->lc), "misc", "uuid", "0");
	lp_config_set_string(linphone_core_get_config(marie2->lc), "misc", "uuid", uuid);
	linphone_core_manager_start(marie2, TRUE);
	lcs = bctbx_list_append(lcs, marie2->lc);
	linphone_core_refresh_registers(marie2->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneRegistrationOk, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneRegistrationProgress, 2, 5000));

	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	bctbx_list_free(lcs);
}
#if 0
/* SM: I comment this test out. It doesn't unregister participants properly, which confuses subsequent tests.
 * The storage of REFER request by flexisip in late forking is no longer required in group chat "release" version.
 * It is not essential to keep testing this feature.
 */

static const char* get_laure_rc(void) {
	if (liblinphone_tester_ipv6_available()) {
		return "laure_tcp_rc";
	} else {
		return "laure_rc_udp";
	}
}

static void on_refer_received(SalOp *op, const SalAddress *refer_to) {
	Sal *sal = sal_op_get_sal(op);
	LinphoneCoreManager *receiver = (LinphoneCoreManager*)sal_get_user_pointer(sal);
	receiver->stat.number_of_LinphoneCallRefered++;

}


void resend_refer_other_devices(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new( get_laure_rc());
	LinphoneCoreManager* pauline2;
	bctbx_list_t* lcs = NULL;

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,pauline->lc);
	lcs=bctbx_list_append(lcs,laure->lc);

	/* We set Pauline's Sal callback and pass the core manager to access stats */
	Sal *pauline_sal = linphone_core_get_sal(pauline->lc);
	sal_set_user_pointer(pauline_sal, (void*)pauline);
	sal_set_call_refer_callback(pauline_sal, on_refer_received);


	char *marie_address = linphone_address_as_string(marie->identity);
	char *pauline_address = linphone_address_as_string(pauline->identity);
	char *laure_address = linphone_address_as_string(laure->identity);

	/* Then we create a refer from marie to pauline that refers to laure */
	SalOp *op = sal_create_refer_op(linphone_core_get_sal(marie->lc));
	sal_op_set_from(op, marie_address);
	sal_op_set_to(op, pauline_address);

	SalAddress *address = sal_address_new(laure_address);
	sal_address_set_param(address, "text", NULL);
	sal_op_send_refer(op, address);

	ms_free(marie_address);
	ms_free(pauline_address);
	ms_free(laure_address);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallRefered,1,5000));

	/* We create another pauline and check if it has received a refer */
	pauline2 = linphone_core_manager_new( "pauline_rc");
	lcs=bctbx_list_append(lcs,pauline2->lc);

	Sal *pauline2_sal = linphone_core_get_sal(pauline2->lc);
	sal_set_user_pointer(pauline2_sal, (void*)pauline2);
	sal_set_call_refer_callback(pauline2_sal, on_refer_received);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline2->stat.number_of_LinphoneCallRefered,1,5000));

	sal_address_unref(address);
	sal_release_op(op);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline2);
	bctbx_list_free(lcs);
}

#endif

void sequential_forking(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_create("marie_rc");

	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	/*we don't set marie "q" because it is by default at 1.0 if it is not present (RFC 4596)*/
	linphone_proxy_config_set_contact_parameters(
		linphone_core_get_default_proxy_config(marie2->lc),
		"q=0.5;");

	linphone_core_manager_start(marie2, TRUE);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_invite_address(pauline->lc,marie->identity);
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*first device from Marie should be ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	/*the second should not*/
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	LinphoneCall *call = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call)) goto end;

	/*marie accepts the call on its second device*/
	linphone_call_accept(call);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));

	/*second device should have received nothing*/
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneCallEnd, 0, int, "%d");

	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	bctbx_list_free(lcs);
}

void sequential_forking_with_timeout_for_highest_priority(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager* marie3 = linphone_core_manager_create("marie_rc");

	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	/*we don't set marie "q" because it is by default at 1.0 if it is not present (RFC 4596)*/
	linphone_proxy_config_set_contact_parameters(
		linphone_core_get_default_proxy_config(marie2->lc),
		"q=0.5;");

	linphone_proxy_config_set_contact_parameters(
		linphone_core_get_default_proxy_config(marie3->lc),
		"q=0.5;");

	linphone_core_manager_start(marie2, TRUE);
	linphone_core_manager_start(marie3, TRUE);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);
	lcs=bctbx_list_append(lcs,marie3->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	/*set first device not reachable*/
	linphone_core_set_network_reachable(marie->lc,FALSE);

	linphone_core_invite_address(pauline->lc,marie->identity);

	/*second and third devices should have received the call*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,13000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*first device should receive nothing since it is disconnected*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");


	LinphoneCall *call = linphone_core_get_current_call(marie3->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call)) goto end;

	/*marie accepts the call on her third device*/
	linphone_call_accept(call);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallStreamsRunning,1,1000));

	/*second device should stop ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));

	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,1000));

	/*first device should have received nothing*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd, 0, int, "%d");

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

void sequential_forking_with_no_response_for_highest_priority(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_create("marie_rc");

	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	/*we don't set marie "q" because it is by default at 1.0 if it is not present (RFC 4596)*/
	linphone_proxy_config_set_contact_parameters(
		linphone_core_get_default_proxy_config(marie2->lc),
		"q=0.5;");

	linphone_core_manager_start(marie2, TRUE);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	linphone_core_invite_address(pauline->lc,marie->identity);

	/*first device should receive the call*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallIncomingReceived,1,3000));
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*second device should have not received the call yet*/
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	/*we wait for the call to try the next branches*/
	wait_for_list(lcs,NULL,0,10000);

	/*then the second device should receive the call*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));

	LinphoneCall *call = linphone_core_get_current_call(marie2->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call)) goto end;

	/*marie accepts the call on her second device*/
	linphone_call_accept(call);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallStreamsRunning,1,1000));

	/*the first device should finish*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd, 1, 3000));

	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	bctbx_list_free(lcs);
}

void sequential_forking_with_insertion_of_higher_priority(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie2 = linphone_core_manager_create("marie_rc");

	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	/*we don't set marie "q" because it is by default at 1.0 if it is not present (RFC 4596)*/
	linphone_proxy_config_set_contact_parameters(
		linphone_core_get_default_proxy_config(marie2->lc),
		"q=0.5;");

	linphone_core_manager_start(marie2, TRUE);

	lcs=bctbx_list_append(lcs,marie->lc);
	lcs=bctbx_list_append(lcs,marie2->lc);

	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);

	/*set first device not reachable*/
	linphone_core_set_network_reachable(marie->lc,FALSE);

	linphone_core_invite_address(pauline->lc,marie->identity);

	/*second device should have received the call*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallIncomingReceived,1,13000));
	/*pauline should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*first device should receive nothing since it is disconnected*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	/*we create a new device*/
	LinphoneCoreManager* marie3 = linphone_core_manager_new("marie_rc");
	lcs=bctbx_list_append(lcs,marie3->lc);
	linphone_core_set_user_agent(marie3->lc,"Natted Linphone",NULL);

	/*this device should receive the call*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallIncomingReceived,1,3000));

	LinphoneCall *call = linphone_core_get_current_call(marie3->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call)) goto end;

	/*marie accepts the call on her third device*/
	linphone_call_accept(call);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallStreamsRunning,1,1000));

	/*second device should stop ringing*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie2->stat.number_of_LinphoneCallEnd,1,1000));

	linphone_call_terminate(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie3->stat.number_of_LinphoneCallEnd,1,1000));

	/*first device should have received nothing*/
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEnd, 0, int, "%d");

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie3);
	bctbx_list_free(lcs);
}

void sequential_forking_with_fallback_route(void) {
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* pauline2 = linphone_core_manager_create(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreManager* marie = linphone_core_manager_create("marie_rc");

	bctbx_list_t* lcs=bctbx_list_append(NULL,pauline->lc);

	/*we set pauline2 and marie to another test server that is configured with a fallback route*/
	linphone_proxy_config_set_server_addr(
		linphone_core_get_default_proxy_config(pauline2->lc),
		"sip:sip2.linphone.org:5071;transport=tls");

	linphone_proxy_config_set_route(
		linphone_core_get_default_proxy_config(pauline2->lc),
		"sip:sip2.linphone.org:5071;transport=tls");

	linphone_proxy_config_set_server_addr(
		linphone_core_get_default_proxy_config(marie->lc),
		"sip:sip2.linphone.org:5070;transport=tcp");

	linphone_proxy_config_set_route(
		linphone_core_get_default_proxy_config(marie->lc),
		"sip:sip2.linphone.org:5070;transport=tcp;lr");

	linphone_core_manager_start(pauline2, TRUE);
	linphone_core_manager_start(marie, TRUE);

	lcs=bctbx_list_append(lcs,pauline2->lc);
	lcs=bctbx_list_append(lcs,marie->lc);

	linphone_core_set_user_agent(pauline->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(pauline2->lc,"Natted Linphone",NULL);
	linphone_core_set_user_agent(marie->lc,"Natted Linphone",NULL);

	/*set pauline2 not reachable*/
	linphone_core_set_network_reachable(pauline2->lc,FALSE);

	/*marie invites pauline2 on the other server*/
	linphone_core_invite_address(marie->lc,pauline2->identity);

	/*the call should be routed to the first server with pauline account*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,13000));

	/*marie should hear ringback*/
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,3000));
	/*pauline2 should receive nothing since it is disconnected*/
	BC_ASSERT_EQUAL(pauline2->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");

	LinphoneCall *call = linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(call)) goto end;

	/*pauline accepts the call*/
	linphone_call_accept(call);
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallStreamsRunning,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,1000));

	linphone_call_terminate(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallEnd,1,1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallEnd,1,1000));

	/*first device should have received nothing*/
	BC_ASSERT_EQUAL(pauline2->stat.number_of_LinphoneCallEnd, 0, int, "%d");

end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

static void deal_with_jwe_auth_module(const char *jwe, bool_t invalid_jwe, bool_t invalid_oid) {
	if (!transport_supported(LinphoneTransportTls))
		return;

	// 1. Register Gandalf.
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	LinphoneCoreManager *gandalf = linphone_core_manager_new(NULL);

	// Do not use Authentication module.
	linphone_core_set_user_agent(gandalf->lc, "JweAuth Linphone", NULL);

	linphone_core_cbs_set_authentication_requested(cbs, tls_authentication_provide_altname_cert);
	linphone_core_add_callbacks(gandalf->lc, cbs);
	linphone_core_cbs_unref(cbs);

	LinphoneProxyConfig *cfg = linphone_core_create_proxy_config(gandalf->lc);

	linphone_proxy_config_set_server_addr(cfg, "sip:sip2.linphone.org:5063;transport=tls");
	linphone_proxy_config_set_route(cfg, "sip:sip2.linphone.org:5063;transport=tls");
	linphone_proxy_config_enable_register(cfg, TRUE);
	linphone_proxy_config_set_identity(cfg, "sip:gandalf@sip.example.org");
	linphone_core_add_proxy_config(gandalf->lc, cfg);

	BC_ASSERT_TRUE(wait_for(gandalf->lc, NULL, &gandalf->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(gandalf->stat.number_of_LinphoneRegistrationFailed,0, int, "%d");
	BC_ASSERT_EQUAL(gandalf->stat.number_of_auth_info_requested,1, int, "%d");

	// 2. Invite Pauline.
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");

	bctbx_list_t *lcs = bctbx_list_append(NULL, gandalf->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	LinphoneCallParams *gandalf_params = linphone_core_create_call_params(gandalf->lc, NULL);
	linphone_call_params_add_custom_header(
		gandalf_params,
		"X-token-oid",
		invalid_oid ? "sip:invalid@sip.example.org" : "sip:gandalf@sip.example.org"
	);
	linphone_call_params_add_custom_header(gandalf_params, "X-token-aud", "plic-ploc");
	linphone_call_params_add_custom_header(gandalf_params, "X-token-req_act", "DRAAAAA LES PYRAMIDES");
	linphone_call_params_add_custom_header(gandalf_params, "X-token-jwe", jwe);

	linphone_core_invite_address_with_params(gandalf->lc, pauline->identity, gandalf_params);
	linphone_call_params_unref(gandalf_params);

	int n_expected_calls = invalid_jwe || invalid_oid ? 0 : 1;
	
	if (n_expected_calls){
		BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallOutgoingRinging, 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));
	}else{
		BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallError, 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallReleased, 1, 3000));
	}

	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	if (!n_expected_calls)
		BC_ASSERT_PTR_NULL(pauline_call);
	else {
		if (pauline_call)
			linphone_call_accept(pauline_call);
		BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallConnected, 1, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 1000));
	}

	LinphoneCall *gandalf_call = linphone_core_get_current_call(gandalf->lc);
	if (n_expected_calls) {
		BC_ASSERT_PTR_NOT_NULL(gandalf_call);
		if (gandalf_call){
			linphone_call_terminate(gandalf_call);
			BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallEnd, 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, n_expected_calls, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &gandalf->stat.number_of_LinphoneCallReleased, 1, 3000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 3000));
		}
	}

	linphone_core_manager_destroy(gandalf);
	linphone_core_manager_destroy(pauline);

	linphone_proxy_config_unref(cfg);

	bctbx_list_free(lcs);
}

void use_jwe_auth_module(void) {
	static const char jwe[] = "eyJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2Iiwia2lkIjoiOFp0TTNUQjJYN2YxRTBoSDk3dzlCelNvai1fck1qXzVpQnlPVG9ZcWltOCJ9.d1iXAnX4_mZ_A4Iptxbl4uka6JDa5QbWS5rk8ETap6ytTx_Fse2R_piGgePv_1TsG36SkPGbUANlKmgYr5mNkvGvjPWKNizQF_GrT5-nuQxIEq9ciYK-ELg9vahgYcdsiGu-3xn8tA2Z0s2WjVy3iY_qy6Rsab-i5nsiyudCBUQ0BLNfW4_IFP52EfdaxtspEnF7iW3fIV8VFoMfyUE2J1gw6hAf9XPkgh-HRb_umJorT9iPU5zZU7ZpfGwEAt5G-wFrVBMmdMxx1VYVKAWInSVq9W-yDqruAWLKKAlYZYQZIcU6w30x4B8KWdNEhpV-z-3GCrFRO96BcpAhEYCnJA.jObnmdLM4EVS-odJ.jV4YRAi8fA4h4e_XBFv14wpST5-Phw5fikzQZWr-G1I5THSDlun2ef42s3ZlC_kUMgTsg1NSm-CZ9XOLtADK7Krl2hKe3Ax4P6gNBGCGgP_O50pWmkLYngz6PxHQk37nk_qw6tnpCdU-rg.RKjeuRcjSvcRuUtn3dPnbg";
	deal_with_jwe_auth_module(jwe, FALSE, FALSE);
}

void use_jwe_auth_module_with_invalid_oid(void) {
	static const char jwe[] = "eyJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2Iiwia2lkIjoiOFp0TTNUQjJYN2YxRTBoSDk3dzlCelNvai1fck1qXzVpQnlPVG9ZcWltOCJ9.W1csJX4veF4SgMAUpBL5QLOibXfH6z6AEuebB9FpfzcANAtliYIggo1mbKzZVECZj4O5M158SIU4O4A5Jk9rAeriKGKUjgDpgBJlV7aKhBRvp-fGVF52BynQ7UFDZYZoPLQh17_PSNI4hzDTigVAI0OYRGQLsSoHCNaP72Zf5hNi7iJBTqL8cmViQqGTOczFxF9vacIDAcXkPhiCSy-3Q4Jsl5JJnESUc_DHrA2b2RVde_TI0koweUPs9SrS63w4XkLvOv_MVEeIZPksZv3hNPnvaut7GrytHWbz2LcYP4Gs8MQbAxIZ-rtl0x0af2KLOfI1xOEvMYIyrg0Vu4D4EQ.7--QW4e7N43miyFp.Fuu6sFIL6tCwdLiK6m-rKF6P_kp0qlPt_gZyPHBK62_v2aQoCU9Sf3RfCOePSzE0Y9fbMt2_4eTcdleJeaOzMTcZqKdNEZKpfdaDe77tobMNx6dzvQSW-Pk6FxwaBSr7fA4uFIzThkZ2cP8.dhd48xH9hZ5OQo6SNHwCmg";
	deal_with_jwe_auth_module(jwe, FALSE, TRUE);
}

void use_jwe_auth_module_with_expired_exp(void) {
	static const char jwe[] = "eyJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2Iiwia2lkIjoiOFp0TTNUQjJYN2YxRTBoSDk3dzlCelNvai1fck1qXzVpQnlPVG9ZcWltOCJ9.PChRlYeTptnYoQDSxYRtqLCBgC3LTpsqWGKDXp-cHC_hF5vurvQfrj__KQb6mR65qBm68PmHp1KwykTAld3bUVA3ykcsGqKI7WBnpu3DLNaQ9kaluP2BvFsyhUBWDU6IqU_O8CIykTwiaSn3HSBH2MiL-lPeNfDY6ndcGXlEAOGosgXAlkFerkNsu8bm4Qkch1JIrxV6I0MhfLrNGIfPPazfYU8byVXScd0YkKlwOuaJZpUXew5nAsgb0L39vMuKomLi5ibH84X01akODOnucX6fU2f_e2MuUH4X3zgqmG9AZ8RV_Iy7irsk_VG4sS8VRftk7YqMVO8kByTkOkLwLA.DO2Navrjk-1vep94.V6n_kZt7gbYGDHnaa9q3DHK7ujkFv2Jd-9jK8xUpkH7PcG3WKTPwiLy40sFvGFr7iWnojK-tODYzakfM5t3uXWdq4iYDXv2tP_JDJt-muiE99C_07cBtL2ymrIzMp-6efzNl3YL_K-rQ9g.p3YUmwPAGO0H9e64MavQlg";
	deal_with_jwe_auth_module(jwe, TRUE, FALSE);
}

void use_jwe_auth_module_with_no_exp(void) {
	static const char jwe[] = "eyJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2Iiwia2lkIjoiOFp0TTNUQjJYN2YxRTBoSDk3dzlCelNvai1fck1qXzVpQnlPVG9ZcWltOCJ9.Hn2oL4835Tkzqb4U3_0aDqgg-pxImCi2EbuTtWwa5uQ5cVz6p6gZ7s7PTPgkfJgtQTYzaRqsywI0cLIWkofAFWUGcL43U-w7_vm3gBpO7BpUKq81kscz6-31ni7M3prxyxw7eoqhdu8Hf9QjirHMaWAw7gYpEjcAshAA559T4a8svan0wq_WGoeXYYS0cEv8UEX6Lpz41tNkvchy9Ydm4kWYXloqnnl0ARR1bMOlxhpOD--Sm2fcTopO_E24tmDjdvgGddwGhHX4qOXs3dXWKM8SPj-PLvZSy7BJ3-Jfz5T1ErEzXlUxutgnV-9K1QZGCbVT6hiF39bcPfAj-y9Emw.xJR4Ot1XCoptlHau.dcYkLAvBJ2N0PnAJ5lr3f3b4CDVJNpi6PrprVB25k4EycdlW-IiiiC97SAyBeygvK5BAybyyjotU33_MC_1163Gk1SsRGn6yjujWgYeoLBRUL6yQcIiPZus.CCaNmMMMsIX4o-dIt7HquQ";
	deal_with_jwe_auth_module(jwe, TRUE, FALSE);
}

void use_jwe_auth_module_with_invalid_attr(void) {
	static const char jwe[] = "eyJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2Iiwia2lkIjoiOFp0TTNUQjJYN2YxRTBoSDk3dzlCelNvai1fck1qXzVpQnlPVG9ZcWltOCJ9.Db81FlIVyW2N2DViRoA1pUKtT9M-0uZEHa8duBt1kRnN8gw1BpFfKvDb7j8DJeTlfIuYMhmzBH5Qr6olWWqVX9dhC2-xopJ10Zpk0o48XmCviimhrl_sSM-WfthP7FTgxXBP-3PxEIE6wblnjhesSM3Q9nImyQ4xa_5ObYLo1JP35d2iV8irnQba6YxvZqQCeoxiW7YLzGPWtWF9YqXOOALgpwyCLt4JeTrP-m6RggdwfQQqdXbksF6nvg1vK54-osBGE3jwvv6dQwFLwe-TBYug1w7H7IC-dXIPon5OESx_81aW2u-j8S5LSyOjZOLTQj_WMS1MX5CEFtYxpgMlIQ.3WWvsaJzQdqVMOXJ.xmwOL6voeKXS6lLaC8EMQGfaacs1sN9Vd33wcIRfeKFfn0nk1lCtb4IsxSFEaMvaM5rjCN-K2kHTFPNitHAXrex5zhUf9Hq_gMMx9vago63DL674SDQ_dDJiYGBqs8SKxg.zm80aJUoeiJ5MiNMq8MSUQ";
	deal_with_jwe_auth_module(jwe, TRUE, FALSE);
}

void use_jwe_auth_with_malformed_token(void) {
	static const char jwe[] = "eyJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2Iiwia2lkIjoiOFp0TTNUQjJYN2YxRTBoSDk3dzlCelNvai1fck1qXzVpQnlPVG9ZcWltOCJ9.YIkpmyFKUJaSIBXh8ABjAeymbMN21VAGr7qXFmek0l8Oh3bEGBuf5YggQWP1G55V00WI8KESxm3LJnqzf-L3FDm3S8D-dLeR7GiJgJtJqED6KKf9U86aLo6UZPmh8xIdfVqLFDUeDQQwy3zwZw-MwY_xtDn_RR2u_W5bmWL-t1-A-xTIw6TEwdjqe8X_D0CuhcPx-virV3RBUHwjSO43vsdHMqLExDXIk95CuQOcUJufZJMu0q5KmpuvDSVesf6ZcmKBEVnkIlSbgAl_Hsv51RXPUT3rFsNy0LSEIByyF-zO6u_L6jpqlt8DxKc6aefa9-4KvyaxU1K7AApYZKh2TQ.fjXkk_TeGhfakvKQ.GP8RivXqe5g4OQhvzlvJ-l2jxuRziLWFNohmFIkZoXwL2mvnEqq71GWYr6_X0V7Z3I7nkMKDwhOfBpKbjnDKK-x1BEUmOmTwaJqeX_lJlZeZkXl597jtBXN3fH5vdccRvoxVFHT0DfZcEA.Km01D704Zl-J18hQJ05dEA";
	deal_with_jwe_auth_module(jwe, TRUE, FALSE);
}

test_t flexisip_tests[] = {
	TEST_ONE_TAG("Subscribe forking", subscribe_forking, "LeaksMemory"),
	TEST_NO_TAG("Message forking", message_forking),
	TEST_NO_TAG("Message forking with unreachable recipients", message_forking_with_unreachable_recipients),
	TEST_NO_TAG("Message forking with all recipients unreachable", message_forking_with_all_recipients_unreachable),
	TEST_NO_TAG("Message forking with unreachable recipients with gruu", message_forking_with_unreachable_recipients_with_gruu),
	TEST_NO_TAG("Message expires", text_message_expires),
	TEST_NO_TAG("Call expires", text_call_expires),
	TEST_NO_TAG("Call forking", call_forking),
	TEST_NO_TAG("Call forking cancelled", call_forking_cancelled),
	TEST_NO_TAG("Call forking declined globaly", call_forking_declined_globaly),
	TEST_NO_TAG("Call forking declined localy", call_forking_declined_localy),
	TEST_NO_TAG("Call forking with urgent reply", call_forking_with_urgent_reply),
	TEST_NO_TAG("Call forking with push notification (single)", call_forking_with_push_notification_single),
	TEST_NO_TAG("Call forking with push notification with double contact", call_forking_with_push_notification_double_contact),
	TEST_NO_TAG("Call forking with push notification (multiple)", call_forking_with_push_notification_multiple),
	TEST_NO_TAG("Call forking not responded", call_forking_not_responded),
	TEST_NO_TAG("Early-media call forking", early_media_call_forking),
	TEST_NO_TAG("Call with sips", call_with_sips),
	TEST_NO_TAG("Call with sips not achievable", call_with_sips_not_achievable),
	TEST_NO_TAG("Call ipv6 to ipv6", call_with_ipv6),
	TEST_NO_TAG("Call ipv6 to ipv4", call_ipv6_to_ipv4),
	TEST_NO_TAG("Call ipv4 to ipv6", call_ipv4_to_ipv6),
#if HAVE_SIPP
	TEST_ONE_TAG("Subscribe Notify with sipp publisher", test_subscribe_notify_with_sipp_publisher, "LeaksMemory"),
	/*TEST_ONE_TAG("Subscribe Notify with sipp double publish", test_subscribe_notify_with_sipp_publisher_double_publish, "LeaksMemory"),*/
#endif
	TEST_NO_TAG("Publish/unpublish", test_publish_unpublish),
	TEST_NO_TAG("List subscribe", test_list_subscribe),
	TEST_NO_TAG("List subscribe without body", test_list_subscribe_wrong_body),
	TEST_NO_TAG("File transfer message rcs to external body client", file_transfer_message_rcs_to_external_body_client),
	TEST_NO_TAG("DoS module trigger by sending a lot of chat messages", dos_module_trigger),
#if HAVE_SIPP
	TEST_NO_TAG("Subscribe on wrong dialog", test_subscribe_on_wrong_dialog),
#endif
	TEST_ONE_TAG("Redis Publish/subscribe", redis_publish_subscribe, "Skip"),
	TEST_NO_TAG("TLS authentication - client rejected due to CN mismatch", tls_client_auth_bad_certificate_cn),
	TEST_NO_TAG("TLS authentication - client rejected due to unrecognized certificate chain", tls_client_auth_bad_certificate),
	TEST_NO_TAG("TLS authentication - client rejected due to unmatched certificate subject", tls_client_rejected_due_to_unmatched_subject),
	TEST_NO_TAG("Transcoder", transcoder_tester),
	TEST_NO_TAG("Removing old tport on flexisip for the same client", test_removing_old_tport),
	/*TEST_NO_TAG("Resend of REFER with other devices", resend_refer_other_devices),*/
	TEST_NO_TAG("Sequential forking", sequential_forking),
	TEST_NO_TAG("Sequential forking with timeout for highest priority", sequential_forking_with_timeout_for_highest_priority),
	TEST_NO_TAG("Sequential forking with no response from highest priority", sequential_forking_with_no_response_for_highest_priority),
	TEST_NO_TAG("Sequential forking with insertion of higher priority", sequential_forking_with_insertion_of_higher_priority),
	TEST_NO_TAG("Sequential forking with fallback route", sequential_forking_with_fallback_route),
	TEST_NO_TAG("Registered contact does not have regid param", register_without_regid),
	TEST_NO_TAG("Use JweAuth module", use_jwe_auth_module),
	TEST_NO_TAG("Use JweAuth module with invalid oid", use_jwe_auth_module_with_invalid_oid),
	TEST_NO_TAG("Use JweAuth module with expired exp", use_jwe_auth_module_with_expired_exp),
	TEST_NO_TAG("Use JweAuth module with no exp", use_jwe_auth_module_with_no_exp),
	TEST_NO_TAG("Use JweAuth module with invalid attr", use_jwe_auth_module_with_invalid_attr),
	TEST_NO_TAG("Use JweAuth module with malformed token", use_jwe_auth_with_malformed_token)
};

test_suite_t flexisip_test_suite = {
	"Flexisip", NULL, NULL,
	liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(flexisip_tests) / sizeof(flexisip_tests[0]), flexisip_tests
};
