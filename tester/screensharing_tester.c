/*
	liblinphone_tester - liblinphone test suite
	Copyright (C) 2016  Belledonne Communications SARL

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
#include "belle-sip/belle-sip.h"

static void call_screensharing_params(LinphoneCoreManager *coreCaller, LinphoneCoreManager *coreCallee,
									  bool_t sc_caller_enable, bool_t sc_callee_enable,
									  LinphoneMediaDirection caller_dir, LinphoneMediaDirection callee_dir,
									  LinphoneMediaDirection caller_dir_result,
									  LinphoneMediaDirection callee_dir_result) {
	LinphoneCall *caller_call, *callee_call;
	bool_t sc_call_result = FALSE;

	if (sc_caller_enable && sc_caller_enable == sc_callee_enable && caller_dir != LinphoneMediaDirectionInactive &&
		callee_dir != LinphoneMediaDirectionInactive) {
		sc_call_result = (caller_dir != callee_dir);
	}

	linphone_core_enable_screensharing(coreCaller->lc, sc_caller_enable);
	linphone_core_enable_screensharing(coreCallee->lc, sc_callee_enable);
	linphone_core_set_screensharing_role(coreCaller->lc, caller_dir);
	linphone_core_set_screensharing_role(coreCallee->lc, callee_dir);

	BC_ASSERT_TRUE(call(coreCaller, coreCallee));
	caller_call = linphone_core_get_current_call(coreCaller->lc);
	BC_ASSERT_PTR_NOT_NULL(caller_call);
	callee_call = linphone_core_get_current_call(coreCallee->lc);
	BC_ASSERT_PTR_NOT_NULL(callee_call);
	if (caller_call) {
		const LinphoneCallParams *params;

		BC_ASSERT_TRUE(
			wait_for(coreCaller->lc, coreCallee->lc, &coreCaller->stat.number_of_LinphoneCallStreamsRunning, 1));
		if (caller_dir_result == LinphoneMediaDirectionRecvOnly) {
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(caller_call->screenstream->state),
									(int)MSScreenSharingListening));
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(callee_call->screenstream->state),
									(int)MSScreenSharingConnecting));
		} else if (caller_dir_result == LinphoneMediaDirectionSendOnly) {
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(callee_call->screenstream->state),
									(int)MSScreenSharingListening));
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(caller_call->screenstream->state),
									(int)MSScreenSharingConnecting));
		}

		params = linphone_call_get_current_params(linphone_core_get_current_call(coreCaller->lc));
		BC_ASSERT_EQUAL(params->screensharing_enabled, sc_call_result, bool_t, "%d");
		BC_ASSERT_EQUAL(params->screensharing_dir, caller_dir_result, int, "%d");
		if (sc_call_result) {
			if (caller_dir_result == LinphoneMediaDirectionRecvOnly) {
				BC_ASSERT_TRUE(caller_call->screenstream->server != NULL);
				BC_ASSERT_TRUE(caller_call->screenstream->client == NULL);
			} else {
				BC_ASSERT_TRUE(caller_call->screenstream->server == NULL);
				BC_ASSERT_TRUE(caller_call->screenstream->client != NULL);
			}
		} else {
			BC_ASSERT_TRUE(caller_call->screenstream->server == NULL);
			BC_ASSERT_TRUE(caller_call->screenstream->client == NULL);
		}

		params = linphone_call_get_current_params(linphone_core_get_current_call(coreCallee->lc));
		BC_ASSERT_EQUAL(params->screensharing_enabled, sc_call_result, bool_t, "%d");
		BC_ASSERT_EQUAL(params->screensharing_dir, callee_dir_result, int, "%d");
		if (sc_call_result) {
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(callee_call->screenstream->state),
									(int)MSScreenSharingStreamRunning));
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(caller_call->screenstream->state),
									(int)MSScreenSharingStreamRunning));
			if (callee_dir_result == LinphoneMediaDirectionRecvOnly) {
				BC_ASSERT_TRUE(callee_call->screenstream->server != NULL);
				BC_ASSERT_TRUE(callee_call->screenstream->client == NULL);
			} else {
				BC_ASSERT_TRUE(callee_call->screenstream->server == NULL);
				BC_ASSERT_TRUE(callee_call->screenstream->client != NULL);
			}
		} else {
			BC_ASSERT_TRUE(callee_call->screenstream->server == NULL);
			BC_ASSERT_TRUE(callee_call->screenstream->client == NULL);
		}
	}

	end_call(coreCaller, coreCallee);
}

static void call_with_screensharing_RecvOnly_SendOnly() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionRecvOnly,
							  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionRecvOnly,
							  LinphoneMediaDirectionSendOnly);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendOnly_RecvOnly() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionSendOnly,
							  LinphoneMediaDirectionRecvOnly, LinphoneMediaDirectionSendOnly,
							  LinphoneMediaDirectionRecvOnly);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendRecv_RecvOnly() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionSendRecv,
							  LinphoneMediaDirectionRecvOnly, LinphoneMediaDirectionSendOnly,
							  LinphoneMediaDirectionRecvOnly);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendOnly_SendRecv() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionSendOnly,
							  LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionSendOnly,
							  LinphoneMediaDirectionRecvOnly);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendRecv_SendRecv() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionSendRecv,
							  LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendOnly_SendOnly() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionSendOnly,
							  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_RecvOnly_RecvOnly() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaDirectionRecvOnly,
							  LinphoneMediaDirectionRecvOnly, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, FALSE, FALSE, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing_transmitter_no_screensharing() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(pauline, marie, FALSE, TRUE, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing_receiver_no_screensharing() {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, FALSE, LinphoneMediaDirectionSendRecv,
							  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive,
							  LinphoneMediaDirectionInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

// TODO asterix issue
static void test_media_screensharing() {
	belle_sdp_media_t *lTmp;
	belle_sip_list_t *list;
	belle_sdp_media_t *l_media = belle_sdp_media_parse("m=application 13078 TCP/RDP *");
	char *l_raw_media = belle_sip_object_to_string(BELLE_SIP_OBJECT(l_media));
	// char fmt = '*';
	belle_sip_object_unref(BELLE_SIP_OBJECT(l_media));
	lTmp = belle_sdp_media_parse(l_raw_media);
	l_media = BELLE_SDP_MEDIA(belle_sip_object_clone(BELLE_SIP_OBJECT(lTmp)));
	belle_sip_object_unref(BELLE_SIP_OBJECT(lTmp));
	BC_ASSERT_STRING_EQUAL(belle_sdp_media_get_media_type(l_media), "application");
	BC_ASSERT_EQUAL(belle_sdp_media_get_media_port(l_media), 13078, int, "%d");
	BC_ASSERT_STRING_EQUAL(belle_sdp_media_get_protocol(l_media), "TCP/RDP");
	list = belle_sdp_media_get_media_formats(l_media);
	BC_ASSERT_PTR_NOT_NULL(list);
	// BC_ASSERT_STRING_EQUAL(BELLE_SIP_POINTER_TO_CHAR(list->data),"*");

	belle_sip_object_unref(BELLE_SIP_OBJECT(l_media));
	belle_sip_free(l_raw_media);
}

test_t screensharing_tests[] = {
	TEST_NO_TAG("Call with screensharing RecvOnly SendOnly", call_with_screensharing_RecvOnly_SendOnly),
	TEST_NO_TAG("Call with screensharing SendOnly RecvOnly", call_with_screensharing_SendOnly_RecvOnly),
	TEST_NO_TAG("Call with screensharing SendRecv RecvOnly", call_with_screensharing_SendRecv_RecvOnly),
	TEST_NO_TAG("Call with screensharing SendOnly SendRecv", call_with_screensharing_SendOnly_SendRecv),
	TEST_NO_TAG("Call with screensharing SendRecv SendRecv", call_with_screensharing_SendRecv_SendRecv),
	TEST_NO_TAG("Call with screensharing SendOnly SendOnly", call_with_screensharing_SendOnly_SendOnly),
	TEST_NO_TAG("Call with screensharing RecvOnly RecvOnly", call_with_screensharing_RecvOnly_RecvOnly),
	TEST_NO_TAG("Call without screensharing", call_without_screensharing),
	TEST_NO_TAG("Call without screensharing, transmitter without screensharing",
				call_without_screensharing_transmitter_no_screensharing),
	TEST_NO_TAG("Call without screensharing, receiver without screensharing",
				call_without_screensharing_receiver_no_screensharing),
	TEST_NO_TAG("m=application", test_media_screensharing)};

test_suite_t screensharing_test_suite = {
	"Screensharing", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(screensharing_tests) / sizeof(screensharing_tests[0]), screensharing_tests};
