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
#include "shared_tester_functions.h"

static void call_with_video_added_by_both_parties(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_params, *marie_params;
	LinphoneCall *marie_call, *pauline_call;
	const LinphoneErrorInfo *ei;
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;
	
	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_set_video_device(pauline->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;
	
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_FALSE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	marie_params = linphone_core_create_call_params(marie->lc, marie_call);
	linphone_call_params_enable_video(pauline_params, TRUE);
	linphone_call_params_enable_video(marie_params, TRUE);
	
	/* both request video at the same time */
	linphone_call_update(marie_call, marie_params);
	linphone_call_update(pauline_call, pauline_params);
	
	linphone_call_params_unref(pauline_params);
	linphone_call_params_unref(marie_params);
	
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating,1,10000));
	
	/* Marie shall transition to UpdatedByRemote (Pauline has priority per RFC3261 since she is the call-id owner)*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdatedByRemote,1,10000));
	
	/* Marie will succeed with its re INVITE transaction */
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,2,10000));
	
	/* Pauline shall return to Updating state*/
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallUpdating,2,10000));
	/* And finally succeed too */
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,2,10000));
	
	ei = linphone_call_get_error_info(marie_call);
	BC_ASSERT_TRUE(linphone_error_info_get_protocol_code(ei) == 0);
	ei = linphone_call_get_error_info(pauline_call);
	BC_ASSERT_TRUE(linphone_error_info_get_protocol_code(ei) == 0);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(marie_call)));
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(linphone_call_get_current_params(pauline_call)));
	
	
	end_call(pauline, marie);
end:
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	
}


static void call_paused_by_both_parties(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCall *marie_call, *pauline_call;
	bctbx_list_t *lcs = NULL;
	bool_t call_ok;

	lcs = bctbx_list_append(lcs, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	BC_ASSERT_TRUE(call_ok=call(pauline,marie));
	if (!call_ok) goto end;
	
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_call = linphone_core_get_current_call(pauline->lc);
	/* both pause the call at the same time */
	linphone_call_pause(marie_call);
	linphone_call_pause(pauline_call);
	
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing,1,10000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused,1,10000));
	
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallUpdatedByRemote, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallPausing, 2, int, "%d");
	
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallStreamsRunning, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallStreamsRunning, 1, int, "%d");
	
	/* both resume the call at the same time */
	linphone_call_resume(marie_call);
	linphone_call_resume(pauline_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning,2,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning,2,10000));
	liblinphone_tester_check_rtcp(pauline, marie);
	
	end_call(pauline, marie);
end:
	bctbx_list_free(lcs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	
}

static test_t call_race_conditions_tests[] = {
	TEST_NO_TAG("Call with video added by both parties", call_with_video_added_by_both_parties),
	TEST_NO_TAG("Call paused by both parties", call_paused_by_both_parties)
};

test_suite_t call_race_conditions_suite = {"Call race conditions", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_race_conditions_tests) / sizeof(call_race_conditions_tests[0]), call_race_conditions_tests};
