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

#include "liblinphone_tester.h"
#include "linphone/core.h"
#include "tester_utils.h"

static void session_timer_disabled(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	// Session Timers is disabled both sides
	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_params, "Session-Expires"));
	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_params, "Min-SE"));

	const char *marie_value_require = linphone_call_params_get_custom_header(marie_params, "Require");
	BC_ASSERT_PTR_NULL(marie_value_require);

	const char *marie_value_supported = linphone_call_params_get_custom_header(marie_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_supported)) {
		BC_ASSERT_PTR_NULL(strstr(marie_value_supported, "timer"));
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(pauline_params, "Session-Expires"));
	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(pauline_params, "Min-SE"));

	const char *value_require = linphone_call_params_get_custom_header(pauline_params, "Require");
	BC_ASSERT_PTR_NULL(value_require);

	const char *value_supported = linphone_call_params_get_custom_header(pauline_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(value_supported)) {
		BC_ASSERT_PTR_NULL(strstr(value_supported, "timer"));
	}

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_too_small(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 6); // Client value bellow the server Min-SE
	linphone_core_set_session_expires_refresher_value(marie->lc, LinphoneSessionExpiresRefresherUAC);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 10);
	linphone_core_set_session_expires_min_value(pauline->lc, 10);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call); // Keep the reference, we'll clean it later

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Pauline 422
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "10;refresher=uac");
	}

	const char *value_require = linphone_call_params_get_custom_header(pauline_params, "Require");
	if (BC_ASSERT_PTR_NOT_NULL(value_require)) {
		BC_ASSERT_STRING_EQUAL(value_require, "timer");
	}

	const char *value_supported = linphone_call_params_get_custom_header(pauline_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(value_supported)) {
		BC_ASSERT_PTR_NOT_NULL(strstr(value_supported, "timer"));
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Marie UPDATE
	const LinphoneCallParams *marie_update_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update_value_session_expires, "10;refresher=uac");
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_update_params, "Min-SE"));

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));

	// Marie re-UPDATE
	const LinphoneCallParams *marie_update2_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update2_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update2_value_session_expires, "10;refresher=uac");
	}

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_refresher_invite(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 8);
	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 8);
	linphone_core_set_enable_sip_update(pauline->lc, FALSE);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Pauline 200 OK, without UPDATED support
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_allow = linphone_call_params_get_custom_header(pauline_params, "Allow");
	if (BC_ASSERT_PTR_NOT_NULL(value_allow)) {
		BC_ASSERT_PTR_NULL(strstr(value_allow, "UPDATED"));
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));

	// Pauline INVITE
	const LinphoneCallParams *pauline_invite_params = linphone_call_get_remote_params(out_call);

	const char *pauline_invite_value_session_expires =
	    linphone_call_params_get_custom_header(pauline_invite_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(pauline_invite_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(pauline_invite_value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));

	// Pauline re-INVITE
	const LinphoneCallParams *pauline_invite2_params = linphone_call_get_remote_params(out_call);

	const char *pauline_invite2_value_session_expires =
	    linphone_call_params_get_custom_header(pauline_invite2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(pauline_invite2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(pauline_invite2_value_session_expires, "8;refresher=uac");
	}

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_disabled_client(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, FALSE); // Disabled for Marie
	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 8);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	BC_ASSERT_PTR_NULL(marie_value_session_expires);

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	BC_ASSERT_PTR_NULL(value_session_expires);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_disabled_server(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 8);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, FALSE);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "8");
	}

	const char *marie_value_supported = linphone_call_params_get_custom_header(marie_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_supported)) {
		BC_ASSERT_PTR_NOT_NULL(strstr(marie_value_supported, "timer"));
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_params, "Min-SE"));

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	BC_ASSERT_PTR_NULL(value_session_expires);

	const char *value_supported = linphone_call_params_get_custom_header(pauline_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(value_supported)) {
		BC_ASSERT_PTR_NULL(strstr(value_supported, "timer"));
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Marie UPDATE
	const LinphoneCallParams *marie_update_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update_value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_update_params, "Min-SE"));

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));

	// Marie re-UPDATE
	const LinphoneCallParams *marie_update2_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update2_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update2_value_session_expires, "8;refresher=uac");
	}

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_refresher_none_uas(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 8);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 8);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "8");
	}

	const char *marie_value_supported = linphone_call_params_get_custom_header(marie_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_supported)) {
		BC_ASSERT_PTR_NOT_NULL(strstr(marie_value_supported, "timer"));
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_params, "Min-SE"));

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "8;refresher=uas");
	}

	const char *value_require = linphone_call_params_get_custom_header(pauline_params, "Require");
	if (BC_ASSERT_PTR_NOT_NULL(value_require)) {
		BC_ASSERT_STRING_EQUAL(value_require, "timer");
	}

	const char *value_supported = linphone_call_params_get_custom_header(pauline_params, "Supported");
	if (BC_ASSERT_PTR_NOT_NULL(value_supported)) {
		BC_ASSERT_PTR_NOT_NULL(strstr(value_supported, "timer"));
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Pauline UPDATE
	const LinphoneCallParams *pauline_update_params = linphone_call_get_remote_params(out_call);

	const char *pauline_update_value_session_expires =
	    linphone_call_params_get_custom_header(pauline_update_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(pauline_update_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(pauline_update_value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));

	// Pauline re-UPDATE
	const LinphoneCallParams *pauline_update2_params = linphone_call_get_remote_params(out_call);

	const char *pauline_update2_value_session_expires =
	    linphone_call_params_get_custom_header(pauline_update2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(pauline_update2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(pauline_update2_value_session_expires, "8;refresher=uac");
	}

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_call_unref(out_call);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_refresher_uac_uac(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_create("marie_session_timers_rc");

	BC_ASSERT_TRUE(linphone_core_get_session_expires_value(marie->lc) == 8);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_min_value(marie->lc) == 5);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_refresher_value(marie->lc) ==
	               (int)LinphoneSessionExpiresRefresherUAC);
	linphone_core_manager_start(marie, TRUE);

	pauline = linphone_core_manager_create("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 6);
	linphone_core_manager_start(pauline, TRUE);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "8;refresher=uac");
	}

	const char *marie_value_session_expires_min_se = linphone_call_params_get_custom_header(marie_params, "Min-SE");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires_min_se)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires_min_se, "5");
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "6;refresher=uac");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Marie UPDATE
	const LinphoneCallParams *marie_update_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update_value_session_expires, "6;refresher=uac");
	}

	const char *marie_update_value_cseq = linphone_call_params_get_custom_header(marie_update_params, "CSeq");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update_value_cseq)) {
		BC_ASSERT_STRING_EQUAL(marie_update_value_cseq, "22 UPDATE");
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_update_params, "Min-SE"));

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));

	// Marie re-UPDATE
	const LinphoneCallParams *marie_update2_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update2_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update2_value_session_expires, "6;refresher=uac");
	}

	const char *marie_update2_value_cseq = linphone_call_params_get_custom_header(marie_update2_params, "CSeq");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update2_value_cseq)) {
		BC_ASSERT_STRING_EQUAL(marie_update2_value_cseq, "23 UPDATE");
	}

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_smaller(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_create("marie_session_timers_rc");

	linphone_core_set_session_expires_value(marie->lc, 5); // Client value smaller but above the server Min-SE
	BC_ASSERT_TRUE(linphone_core_get_session_expires_value(marie->lc) == 5);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_min_value(marie->lc) == 5);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_refresher_value(marie->lc) ==
	               (int)LinphoneSessionExpiresRefresherUAC);
	linphone_core_manager_start(marie, TRUE);

	pauline = linphone_core_manager_create("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 6);
	linphone_core_set_session_expires_min_value(pauline->lc, 3);
	linphone_core_manager_start(pauline, TRUE);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "5;refresher=uac");
	}

	const char *marie_value_session_expires_min_se = linphone_call_params_get_custom_header(marie_params, "Min-SE");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires_min_se)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires_min_se, "5");
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "5;refresher=uac");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	// UPDATE
	LinphoneCallParams *params;
	int dummy = 0;
	params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));

	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMU", 8000, 1),
	                                  FALSE); /*disable PCMU*/
	linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMU", 8000, 1),
	                                  FALSE); /*disable PCMU*/
	linphone_core_enable_payload_type(marie->lc, linphone_core_find_payload_type(marie->lc, "PCMA", 8000, 1),
	                                  TRUE); /*enable PCMA*/
	linphone_core_enable_payload_type(pauline->lc, linphone_core_find_payload_type(pauline->lc, "PCMA", 8000, 1),
	                                  TRUE); /*enable PCMA*/
	linphone_call_update(linphone_core_get_current_call(pauline->lc), params);
	linphone_call_params_unref(params);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_STRING_EQUAL(linphone_payload_type_get_mime_type(linphone_call_params_get_used_audio_payload_type(
	                           linphone_call_get_current_params(linphone_core_get_current_call(marie->lc)))),
	                       "PCMA");
	BC_ASSERT_STRING_EQUAL(linphone_payload_type_get_mime_type(linphone_call_params_get_used_audio_payload_type(
	                           linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc)))),
	                       "PCMA");
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 5000);

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_larger(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_create("marie_session_timers_rc");

	linphone_core_set_session_expires_value(marie->lc, 5); // Client value larger that the server SE and Min-SE
	linphone_core_set_session_expires_min_value(marie->lc, 0);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_value(marie->lc) == 5);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_min_value(marie->lc) == 0);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_refresher_value(marie->lc) ==
	               (int)LinphoneSessionExpiresRefresherUAC);
	linphone_core_manager_start(marie, TRUE);

	pauline = linphone_core_manager_create("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 4);
	linphone_core_set_session_expires_min_value(pauline->lc, 2);
	linphone_core_manager_start(pauline, TRUE);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "5;refresher=uac");
	}

	const char *marie_value_session_expires_min_se = linphone_call_params_get_custom_header(marie_params, "Min-SE");
	BC_ASSERT_PTR_NULL(marie_value_session_expires_min_se);

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "4;refresher=uac");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_refresher_uas_uas(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 8);
	linphone_core_set_session_expires_refresher_value(marie->lc, LinphoneSessionExpiresRefresherUAS);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 8);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "8;refresher=uas");
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "8;refresher=uas");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Pauline UPDATE
	const LinphoneCallParams *pauline_update_params = linphone_call_get_remote_params(out_call);

	const char *pauline_update_value_session_expires =
	    linphone_call_params_get_custom_header(pauline_update_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(pauline_update_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(pauline_update_value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(pauline_update_params, "Min-SE"));

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 3));

	// Pauline re-UPDATE
	const LinphoneCallParams *pauline_update2_params = linphone_call_get_remote_params(out_call);

	const char *pauline_update2_value_session_expires =
	    linphone_call_params_get_custom_header(pauline_update2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(pauline_update2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(pauline_update2_value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(pauline_update2_params, "Min-SE"));

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_refresher_none_uac(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 8);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 8);
	linphone_core_set_session_expires_refresher_value(pauline->lc, LinphoneSessionExpiresRefresherUAC);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_value_session_expires = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires, "8");
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Marie UPDATE
	const LinphoneCallParams *marie_update_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update_value_session_expires, "8;refresher=uac");
	}

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_update_params, "Min-SE"));

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallUpdatedByRemote, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3));

	// Marie re-UPDATE
	const LinphoneCallParams *marie_update2_params = linphone_call_get_remote_params(pauline_call);

	const char *marie_update2_value_session_expires =
	    linphone_call_params_get_custom_header(marie_update2_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(marie_update2_value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(marie_update2_value_session_expires, "8;refresher=uac");
	}

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_no_se(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_create("marie_session_timers_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_min_value(marie->lc, 6);
	linphone_core_set_session_expires_value(marie->lc, 0); // We clear the SE header for Marie

	BC_ASSERT_TRUE(linphone_core_get_session_expires_value(marie->lc) == 0);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_min_value(marie->lc) == 6);
	BC_ASSERT_TRUE(linphone_core_get_session_expires_refresher_value(marie->lc) ==
	               (int)LinphoneSessionExpiresRefresherUAC);
	linphone_core_manager_start(marie, TRUE);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 4);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie INVITE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	BC_ASSERT_PTR_NULL(linphone_call_params_get_custom_header(marie_params, "Session-Expires"));

	const char *marie_value_session_expires_min_se = linphone_call_params_get_custom_header(marie_params, "Min-SE");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires_min_se)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires_min_se, "6");
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "6;refresher=uas");
	}

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_interval_ok_no_se_smaller(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_session_timers_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_min_value(marie->lc, 3);
	linphone_core_set_session_expires_value(marie->lc, 0); // We clear the SE header for Marie

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 5);
	linphone_core_set_session_expires_min_value(pauline->lc, 4);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Marie RE INVITE with a new Min-SE
	const LinphoneCallParams *marie_params = linphone_call_get_remote_params(pauline_call);

	const char *value_session_expires_se = linphone_call_params_get_custom_header(marie_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires_se)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires_se, "4;refresher=uac");
	}

	const char *marie_value_session_expires_min_se = linphone_call_params_get_custom_header(marie_params, "Min-SE");
	if (BC_ASSERT_PTR_NOT_NULL(marie_value_session_expires_min_se)) {
		BC_ASSERT_STRING_EQUAL(marie_value_session_expires_min_se, "4");
	}

	// Pauline 200 OK
	const LinphoneCallParams *pauline_params = linphone_call_get_remote_params(out_call);

	const char *value_session_expires = linphone_call_params_get_custom_header(pauline_params, "Session-Expires");
	if (BC_ASSERT_PTR_NOT_NULL(value_session_expires)) {
		BC_ASSERT_STRING_EQUAL(value_session_expires, "4;refresher=uac");
	}

	linphone_call_terminate(out_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_cancel_timer(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 4);
	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 4);

	LinphoneCall *out_call = linphone_core_invite_address(marie->lc, pauline->identity);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));

	// Wait for the 200 OK
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	linphone_call_terminate(out_call);

	// Wait more to see if any more UPDATE are sent, if not the state is set to released
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 10000);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void session_timer_update_during_paused_call(void) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *marie_call;

	marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_session_expires_enabled(marie->lc, TRUE);
	linphone_core_set_session_expires_value(marie->lc, 8);

	pauline = linphone_core_manager_new("pauline_rc");
	linphone_core_set_session_expires_enabled(pauline->lc, TRUE);
	linphone_core_set_session_expires_value(pauline->lc, 8);
	linphone_core_set_session_expires_refresher_value(pauline->lc, LinphoneSessionExpiresRefresherUAC);

	if (BC_ASSERT_TRUE(call(marie, pauline))) {
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));

		marie_call = linphone_core_get_current_call(marie->lc);
		linphone_call_pause(marie_call);

		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 1, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1, 10000));

		// Wait for an update
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallUpdating, 1, 10000));

		// The call should still be in pause
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 2, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPausedByRemote, 2, 10000));

		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t session_timers_tests[] = {
    TEST_ONE_TAG("Session timer disabled", session_timer_disabled, "Session Timer"),
    TEST_ONE_TAG("Session timer disabled client", session_timer_disabled_client, "Session Timer"),
    TEST_ONE_TAG("Session timer disabled server", session_timer_disabled_server, "Session Timer"),
    TEST_ONE_TAG("Session timer invite 422", session_timer_interval_too_small, "Session Timer"),
    TEST_ONE_TAG("Session timer invite OK, smaller", session_timer_interval_smaller, "Session Timer"),
    TEST_ONE_TAG("Session timer invite OK, larger", session_timer_interval_larger, "Session Timer"),
    TEST_ONE_TAG("Session timer invite OK, no SE", session_timer_interval_ok_no_se, "Session Timer"),
    TEST_ONE_TAG("Session timer invite 422, no SE, smaller", session_timer_interval_ok_no_se_smaller, "Session Timer"),

    // https://tools.ietf.org/html/rfc4028#page-15
    TEST_ONE_TAG(
        "Session timer INVITE (UPDATE not supported)", session_timer_interval_ok_refresher_invite, "Session Timer"),
    TEST_ONE_TAG(
        "Session timer invite OK, c = none, s = auto", session_timer_interval_ok_refresher_none_uas, "Session Timer"),
    TEST_ONE_TAG(
        "Session timer invite OK, c = uac, s = auto", session_timer_interval_ok_refresher_uac_uac, "Session Timer"),
    TEST_ONE_TAG(
        "Session timer invite OK, c = uas, s = auto", session_timer_interval_ok_refresher_uas_uas, "Session Timer"),
    TEST_ONE_TAG(
        "Session timer invite OK, c = none, s = uac", session_timer_interval_ok_refresher_none_uac, "Session Timer"),
    TEST_ONE_TAG("Session timer cancel OK", session_timer_cancel_timer, "Session Timer"),

    TEST_ONE_TAG("Session timer update during a paused call", session_timer_update_during_paused_call, "Session Timer"),
};

test_suite_t session_timers_test_suite = {"Session Timers",
                                          NULL,
                                          NULL,
                                          liblinphone_tester_before_each,
                                          liblinphone_tester_after_each,
                                          sizeof(session_timers_tests) / sizeof(session_timers_tests[0]),
                                          session_timers_tests,
                                          0};
