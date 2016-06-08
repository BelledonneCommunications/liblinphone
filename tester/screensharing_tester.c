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
									  bool_t sc_caller_enable, bool_t sc_callee_enable, LinphoneMediaRole caller_role,
									  LinphoneMediaRole callee_role, LinphoneMediaRole caller_role_result,
									  LinphoneMediaRole callee_role_result) {
	LinphoneCall *caller_call, *callee_call;
	bool_t sc_call_result = FALSE;

	if (sc_caller_enable && sc_caller_enable == sc_callee_enable && caller_role != LinphoneMediaRoleInactive &&
		callee_role != LinphoneMediaRoleInactive) {
		sc_call_result = (caller_role != callee_role);
	}

	linphone_core_enable_screensharing(coreCaller->lc, sc_caller_enable);
	linphone_core_enable_screensharing(coreCallee->lc, sc_callee_enable);
	linphone_core_set_screensharing_role(coreCaller->lc, caller_role);
	linphone_core_set_screensharing_role(coreCallee->lc, callee_role);

	BC_ASSERT_TRUE(call(coreCaller, coreCallee));
	caller_call = linphone_core_get_current_call(coreCaller->lc);
	BC_ASSERT_PTR_NOT_NULL(caller_call);
	callee_call = linphone_core_get_current_call(coreCallee->lc);
	BC_ASSERT_PTR_NOT_NULL(callee_call);
	if (caller_call) {
		LinphoneCallParams *new_params;
		const LinphoneCallParams *paramsCaller;
		const LinphoneCallParams *paramsCallee;

		BC_ASSERT_TRUE(
			wait_for(coreCaller->lc, coreCallee->lc, &coreCaller->stat.number_of_LinphoneCallStreamsRunning, 1));

		if (caller_role_result == LinphoneMediaRoleServer) {
			paramsCaller = linphone_call_get_current_params(caller_call);
			new_params = linphone_call_params_copy(paramsCaller);
			linphone_call_params_enable_screensharing(new_params,TRUE,TRUE);
			linphone_core_update_call(coreCaller->lc, caller_call, new_params);
			linphone_call_params_destroy(new_params);
			linphone_call_params_enable_screensharing(callee_call->params,TRUE,TRUE);
			BC_ASSERT_TRUE(
				wait_for(coreCaller->lc, coreCallee->lc, &coreCallee->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		} else if (callee_role_result == LinphoneMediaRoleServer) {
			paramsCallee = linphone_call_get_current_params(callee_call);
			new_params = linphone_call_params_copy(paramsCallee);
			linphone_call_params_enable_screensharing(new_params,TRUE,TRUE);
			linphone_core_update_call(coreCallee->lc, callee_call, new_params);
			linphone_call_params_destroy(new_params);
			linphone_call_params_enable_screensharing(caller_call->params,TRUE,TRUE);
			BC_ASSERT_TRUE(
				wait_for(coreCaller->lc, coreCallee->lc, &coreCaller->stat.number_of_LinphoneCallUpdatedByRemote, 1));
		}

		if (caller_role_result == LinphoneMediaRoleClient) {
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(caller_call->screenstream->state),
									(int)MSScreenSharingListening));
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(callee_call->screenstream->state),
									(int)MSScreenSharingConnecting));
		} else if (caller_role_result == LinphoneMediaRoleServer) {
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(callee_call->screenstream->state),
									(int)MSScreenSharingListening));
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(caller_call->screenstream->state),
									(int)MSScreenSharingConnecting));
		}

		paramsCaller = linphone_call_get_current_params(caller_call);
		BC_ASSERT_EQUAL(paramsCaller->screensharing_enabled, sc_call_result, bool_t, "%d");
		BC_ASSERT_EQUAL(paramsCaller->screensharing_role, caller_role_result, int, "%d");
		if (sc_call_result) {
			if (caller_role_result == LinphoneMediaRoleServer) {
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

		paramsCallee = linphone_call_get_current_params(callee_call);
		BC_ASSERT_EQUAL(paramsCallee->screensharing_enabled, sc_call_result, bool_t, "%d");
		BC_ASSERT_EQUAL(paramsCallee->screensharing_role, callee_role_result, int, "%d");
		if (sc_call_result) {
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(callee_call->screenstream->state),
									(int)MSScreenSharingStreamRunning));
			BC_ASSERT_TRUE(wait_for(coreCaller->lc, coreCallee->lc, (int *)&(caller_call->screenstream->state),
									(int)MSScreenSharingStreamRunning));
			if (callee_role_result == LinphoneMediaRoleClient) {
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

static void call_with_screensharing_Client_Server(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleClient, LinphoneMediaRoleServer,
							  LinphoneMediaRoleClient, LinphoneMediaRoleServer);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_Server_Client(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleServer, LinphoneMediaRoleClient,
							  LinphoneMediaRoleServer, LinphoneMediaRoleClient);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_ServerClient_Client(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleServerClient, LinphoneMediaRoleClient,
							  LinphoneMediaRoleServer, LinphoneMediaRoleClient);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_ServerClient_Server(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleServerClient, LinphoneMediaRoleServer,
							  LinphoneMediaRoleClient, LinphoneMediaRoleServer);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_Server_ServerClient(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleServer, LinphoneMediaRoleServerClient,
							  LinphoneMediaRoleServer, LinphoneMediaRoleClient);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_Client_ServerClient(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleClient, LinphoneMediaRoleServerClient,
							  LinphoneMediaRoleClient, LinphoneMediaRoleServer);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_ServerClient_ServerClient(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleServerClient, LinphoneMediaRoleServerClient,
							  LinphoneMediaRoleInactive, LinphoneMediaRoleInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_Server_Server(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleServer, LinphoneMediaRoleServer,
							  LinphoneMediaRoleInactive, LinphoneMediaRoleInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_Client_Client(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, TRUE, LinphoneMediaRoleClient, LinphoneMediaRoleClient,
							  LinphoneMediaRoleInactive, LinphoneMediaRoleInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, FALSE, FALSE, LinphoneMediaRoleInactive, LinphoneMediaRoleInactive,
							  LinphoneMediaRoleInactive, LinphoneMediaRoleInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing_transmitter_no_screensharing(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(pauline, marie, FALSE, TRUE, LinphoneMediaRoleInactive, LinphoneMediaRoleServerClient,
							  LinphoneMediaRoleInactive, LinphoneMediaRoleInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing_receiver_no_screensharing(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	call_screensharing_params(marie, pauline, TRUE, FALSE, LinphoneMediaRoleServerClient, LinphoneMediaRoleInactive,
							  LinphoneMediaRoleInactive, LinphoneMediaRoleInactive);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

// TODO asterix issue
static void test_media_screensharing(void) {
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
	TEST_ONE_TAG("Call with screensharing Client Server", call_with_screensharing_Client_Server, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing Server Client", call_with_screensharing_Server_Client, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing ServerClient Client", call_with_screensharing_ServerClient_Client, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing ServerClient Server", call_with_screensharing_ServerClient_Server, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing Server ServerClient", call_with_screensharing_Server_ServerClient, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing Client ServerClient", call_with_screensharing_Client_ServerClient, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing ServerClient ServerClient", call_with_screensharing_ServerClient_ServerClient, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing Server Server", call_with_screensharing_Server_Server, "Screensharing"),
	TEST_ONE_TAG("Call with screensharing Client Client", call_with_screensharing_Client_Client, "Screensharing"),
	TEST_ONE_TAG("Call without screensharing", call_without_screensharing, "Screensharing"),
	TEST_ONE_TAG("Call without screensharing, transmitter without screensharing",
				call_without_screensharing_transmitter_no_screensharing, "Screensharing"),
	TEST_ONE_TAG("Call without screensharing, receiver without screensharing",
				call_without_screensharing_receiver_no_screensharing, "Screensharing"),
	TEST_ONE_TAG("m=application", test_media_screensharing, "Screensharing")};

test_suite_t screensharing_test_suite = {
	"Screensharing", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(screensharing_tests) / sizeof(screensharing_tests[0]), screensharing_tests};
