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

static void call_screensharing_params(LinphoneCoreManager *call1, LinphoneCoreManager *call2, bool_t sc_call1,
									  bool_t sc_call2, LinphoneMediaDirection role_call1,
									  LinphoneMediaDirection role_call2, LinphoneMediaDirection role_result1,
									  LinphoneMediaDirection role_result2) {
	LinphoneCall *user_call1, *user_call2;
	bool_t sc_result = FALSE;

	if (sc_call1 && sc_call1 == sc_call2 && role_call1 != LinphoneMediaDirectionInactive &&
		role_call2 != LinphoneMediaDirectionInactive) {
		if (role_call1 == role_call2 && role_call1 != LinphoneMediaDirectionSendRecv)
			sc_result = FALSE;
		else
			sc_result = TRUE;
	}

	linphone_core_enable_screensharing(call1->lc, sc_call1);
	linphone_core_enable_screensharing(call2->lc, sc_call2);
	linphone_core_set_screensharing_role(call1->lc, role_call1);
	linphone_core_set_screensharing_role(call2->lc, role_call2);

	BC_ASSERT_TRUE(call(call1, call2));
	user_call1 = linphone_core_get_current_call(call1->lc);
	BC_ASSERT_PTR_NOT_NULL(user_call1);
	user_call2 = linphone_core_get_current_call(call2->lc);
	BC_ASSERT_PTR_NOT_NULL(user_call2);
	if (user_call1) {
		const LinphoneCallParams *params;

		BC_ASSERT_TRUE(wait_for(call1->lc, call2->lc, &call1->stat.number_of_LinphoneCallStreamsRunning, 1));

		params = linphone_call_get_current_params(linphone_core_get_current_call(call1->lc));
		BC_ASSERT_EQUAL(params->screensharing_enabled, sc_result, bool_t, "%d");
		BC_ASSERT_EQUAL(params->screensharing_dir, role_result1, int, "%d");
		if (sc_result) {
			if (role_result1 == LinphoneMediaDirectionRecvOnly) {
				BC_ASSERT_TRUE(user_call1->screenstream->server != NULL);
				BC_ASSERT_TRUE(user_call1->screenstream->client == NULL);
			} else {
				BC_ASSERT_TRUE(user_call1->screenstream->server == NULL);
				// BC_ASSERT_TRUE(user_call1->screenstream->client!=NULL);
			}
		} else {
			BC_ASSERT_TRUE(user_call1->screenstream->server == NULL);
			BC_ASSERT_TRUE(user_call1->screenstream->client == NULL);
		}

		params = linphone_call_get_current_params(linphone_core_get_current_call(call2->lc));
		BC_ASSERT_EQUAL(params->screensharing_enabled, sc_result, bool_t, "%d");
		BC_ASSERT_EQUAL(params->screensharing_dir, role_result2, int, "%d");
		if (sc_result) {
			if (role_result2 == LinphoneMediaDirectionRecvOnly) {
				BC_ASSERT_TRUE(user_call2->screenstream->server != NULL);
				BC_ASSERT_TRUE(user_call2->screenstream->client == NULL);
			} else {
				BC_ASSERT_TRUE(user_call2->screenstream->server == NULL);
				// BC_ASSERT_TRUE(user_call2->screenstream->client!=NULL);
			}
		} else {
			BC_ASSERT_TRUE(user_call2->screenstream->server == NULL);
			BC_ASSERT_TRUE(user_call2->screenstream->client == NULL);
		}
	}

	end_call(call1,call2);
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
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, TRUE,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionRecvOnly,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionRecvOnly
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendRecv_RecvOnly() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, TRUE,
				  LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionRecvOnly,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionRecvOnly
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendOnly_SendRecv() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, TRUE,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionSendRecv,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionRecvOnly
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendRecv_SendRecv() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, TRUE,
				  LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionSendRecv,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionRecvOnly
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_SendOnly_SendOnly() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, TRUE,
				  LinphoneMediaDirectionSendOnly, LinphoneMediaDirectionSendOnly,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_screensharing_RecvOnly_RecvOnly() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, TRUE,
				  LinphoneMediaDirectionRecvOnly, LinphoneMediaDirectionRecvOnly,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, FALSE, FALSE,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing_transmitter_no_screensharing() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(pauline, marie, FALSE, TRUE,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionSendRecv,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_without_screensharing_receiver_no_screensharing() {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_rc");
	
	call_screensharing_params(marie, pauline, TRUE, FALSE,
				  LinphoneMediaDirectionSendRecv, LinphoneMediaDirectionInactive,
				  LinphoneMediaDirectionInactive, LinphoneMediaDirectionInactive
				);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void test_media_screensharing() {
	belle_sdp_media_t* lTmp;
	belle_sip_list_t* list;
	belle_sdp_media_t* l_media = belle_sdp_media_parse("m=application 13078 TCP/RDP *");
	char* l_raw_media = belle_sip_object_to_string(BELLE_SIP_OBJECT(l_media));
	//char fmt = '*';
	belle_sip_object_unref(BELLE_SIP_OBJECT(l_media));
	lTmp = belle_sdp_media_parse(l_raw_media);
	l_media = BELLE_SDP_MEDIA(belle_sip_object_clone(BELLE_SIP_OBJECT(lTmp)));
	belle_sip_object_unref(BELLE_SIP_OBJECT(lTmp));
	BC_ASSERT_STRING_EQUAL(belle_sdp_media_get_media_type(l_media), "application");
	BC_ASSERT_EQUAL(belle_sdp_media_get_media_port(l_media), 13078, int, "%d");
	BC_ASSERT_STRING_EQUAL(belle_sdp_media_get_protocol(l_media), "TCP/RDP");
	list = belle_sdp_media_get_media_formats(l_media);
	BC_ASSERT_PTR_NOT_NULL(list);
	//BC_ASSERT_STRING_EQUAL(BELLE_SIP_POINTER_TO_CHAR(list->data),"*");
	
	belle_sip_object_unref(BELLE_SIP_OBJECT(l_media));
	belle_sip_free(l_raw_media);
}

test_t screensharing_tests[] = {
	TEST_NO_TAG("Call with screensharing RecvOnly SendOnly", call_with_screensharing_RecvOnly_SendOnly),
	TEST_NO_TAG("Call with screensharing SendOnly RecvOnly", call_with_screensharing_SendOnly_RecvOnly),
	TEST_NO_TAG("Call with screensharing SendRecv RecvOnly",call_with_screensharing_SendRecv_RecvOnly),
	TEST_NO_TAG("Call with screensharing SendOnly SendRecv",call_with_screensharing_SendOnly_SendRecv),
	TEST_NO_TAG("Call with screensharing SendRecv SendRecv",call_with_screensharing_SendRecv_SendRecv),
	TEST_NO_TAG("Call with screensharing SendOnly SendOnly",call_with_screensharing_SendOnly_SendOnly),
	TEST_NO_TAG("Call with screensharing RecvOnly RecvOnly",call_with_screensharing_RecvOnly_RecvOnly),
	TEST_NO_TAG("Call without screensharing", call_without_screensharing),
	TEST_NO_TAG("Call without screensharing, transmitter without screensharing",call_without_screensharing_transmitter_no_screensharing),
	TEST_NO_TAG("Call without screensharing, receiver without screensharing",call_without_screensharing_receiver_no_screensharing),
	TEST_NO_TAG("m=application", test_media_screensharing)
};

test_suite_t screensharing_test_suite = {"Screensharing", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
										sizeof(screensharing_tests) / sizeof(screensharing_tests[0]), screensharing_tests};
