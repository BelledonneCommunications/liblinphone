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

#ifdef _WIN32
#define unlink _unlink
#ifndef F_OK
#define F_OK 00 /*visual studio does not define F_OK*/
#endif
#endif

// prototype definition for call_recording()
#ifdef __ANDROID__
#ifdef HAVE_OPENH264
extern void libmsopenh264_init(MSFactory *factory);
#endif
#endif


void simple_push_call_base(bool_t push, bool_t canceled, bool_t decline, bool_t push_early) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	//const LinphoneAddress *from;
	LinphoneCall *pauline_call;
	LinphoneCall *marie_call;
	bool_t did_receive_call;
	
	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");
	
	if (push) {
		linphone_core_enter_background(pauline->lc);
	}

	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	BC_ASSERT_PTR_NOT_NULL(marie_call=linphone_core_invite_address(marie->lc,pauline->identity));

	stats initial_caller=marie->stat;
	stats initial_callee=pauline->stat;

	const char *callid = linphone_call_log_get_call_id(linphone_call_get_call_log(marie_call));
	if (push) {
		// simulate pushkit
		linphone_core_start_process_remote_notification(pauline, callid);
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallPushIncomingReceived, 1, int, "%d");
		pauline_call = linphone_core_get_current_call(pauline->lc);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		if (push_early) {
			if (canceled) {
				linphone_core_terminate_all_calls(marie->lc);

				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1, 20000));
				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1, 20000));
				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1, 20000));
				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallReleased,1, 20000));
				goto end;
			} else if (decline) {
				linphone_core_terminate_all_calls(pauline->lc);
				//test ios simulator needs more time, 3s plus for connectng the network
				did_receive_call = wait_for_until(pauline->lc
							,marie->lc
							,&pauline->stat.number_of_LinphoneCallIncomingReceived
							,initial_callee.number_of_LinphoneCallIncomingReceived+1, 12000);
				BC_ASSERT_EQUAL(did_receive_call, 1, int, "%d");

				if (!did_receive_call) return;

				BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallEnd,1));
				BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallEnd,1));
				BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallReleased,1));
				BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallReleased,1));

				goto end;
			} else {
				goto ok;
			}
		}
	}


	BC_ASSERT_PTR_NULL(linphone_call_get_remote_params(marie_call)); /*assert that remote params are NULL when no response is received yet*/

	//test ios simulator needs more time, 3s plus for connectng the network
	did_receive_call = wait_for_until(pauline->lc
				,marie->lc
				,&pauline->stat.number_of_LinphoneCallIncomingReceived
				,initial_callee.number_of_LinphoneCallIncomingReceived+1, 12000);
	BC_ASSERT_EQUAL(did_receive_call, 1, int, "%d");

	if (!did_receive_call) return;

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreFirstCallStarted, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_NOT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	if (canceled) {
		end_call(marie, pauline);
		goto end;
	} else if (decline) {
		end_call(pauline, marie);
		goto end;
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);

ok:
	linphone_call_accept(pauline_call);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallConnected,initial_callee.number_of_LinphoneCallConnected+1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallConnected,initial_caller.number_of_LinphoneCallConnected+1));

	liblinphone_tester_check_rtcp(marie, pauline);
	end_call(marie, pauline);

end:
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_accept_call(void) {
	simple_push_call_base(FALSE, FALSE, FALSE, FALSE);
}

static void push_accept_call(void) {
	simple_push_call_base(TRUE, FALSE, FALSE, FALSE);
}

static void push_early_accept_call(void) {
	simple_push_call_base(TRUE, FALSE, FALSE, TRUE);
}

static void simple_canceled_call(void) {
	simple_push_call_base(FALSE, TRUE, FALSE, FALSE);
}

static void push_canceled_call(void) {
	simple_push_call_base(TRUE, TRUE, FALSE, FALSE);
}

static void push_early_canceled_call(void) {
	simple_push_call_base(TRUE, TRUE, FALSE, TRUE);
}

static void simple_decline_call(void) {
	simple_push_call_base(FALSE, FALSE, TRUE, FALSE);
}

static void push_decline_call(void) {
	simple_push_call_base(TRUE, FALSE, TRUE, FALSE);
}

static void push_early_decline_call(void) {
	simple_push_call_base(TRUE, FALSE, TRUE, TRUE);
}

test_t push_incoming_call_tests[] = {
	TEST_NO_TAG("Simple accept call", simple_accept_call),
	TEST_NO_TAG("Push accept call", push_accept_call),
	TEST_NO_TAG("Push early accept call", push_early_accept_call),
	TEST_NO_TAG("Simple canceled call", simple_canceled_call),
	TEST_NO_TAG("Push canceled call", push_canceled_call),
	TEST_NO_TAG("Push early canceled call", push_early_canceled_call),
	TEST_NO_TAG("Simple decline call", simple_decline_call),
	TEST_NO_TAG("Push decline call", push_decline_call),
	TEST_NO_TAG("Push early decline call", push_early_decline_call),
};

test_suite_t push_incoming_call_test_suite = {"Push Incoming Call", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(push_incoming_call_tests) / sizeof(push_incoming_call_tests[0]), push_incoming_call_tests};

