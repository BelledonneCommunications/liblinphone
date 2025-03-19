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

#include <sys/stat.h>
#include <sys/types.h>

#include "belle-sip/sipstack.h"

#include "mediastreamer2/msutils.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-call-log.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

#ifdef _WIN32
#define unlink _unlink
#endif

/*
 * With IPV6, Flexisip automatically switches to TCP, so it's no more possible to really have Laure configured with UDP
 * Anyway for IPV4, it's still a good opportunity to test UDP.
 */
static const char *get_laure_rc(void) {
	if (liblinphone_tester_ipv6_available()) {
		return "laure_tcp_rc";
	} else {
		return "laure_rc_udp";
	}
}

static void call_waiting_indication_with_param(bool_t enable_caller_privacy) {
	bctbx_list_t *iterator;
	bctbx_list_t *lcs;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *pauline_called_by_laure = NULL;
	LinphoneCoreManager *marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_rc", NULL);
	linphone_core_remove_supported_tag(marie->lc, "gruu");
	linphone_core_manager_start(marie, TRUE);
	LinphoneCoreManager *pauline = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(pauline, "pauline_tcp_rc", NULL);
	linphone_core_remove_supported_tag(pauline->lc, "gruu");
	linphone_core_manager_start(pauline, TRUE);
	LinphoneCoreManager *laure = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(laure, get_laure_rc(), NULL);
	linphone_core_remove_supported_tag(laure->lc, "gruu");
	linphone_core_manager_start(laure, TRUE);
	LinphoneCallParams *laure_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);

	if (enable_caller_privacy) linphone_call_params_set_privacy(marie_params, LinphonePrivacyId);

	lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	linphone_call_params_unref(marie_params);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);

	if (enable_caller_privacy) linphone_call_params_set_privacy(laure_params, LinphonePrivacyId);

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(laure->lc, pauline->identity, laure_params));
	linphone_call_params_unref(laure_params);

	BC_ASSERT_TRUE(wait_for(laure->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 2));

	BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneCallOutgoingProgress, 1, int, "%d");

	BC_ASSERT_TRUE(wait_for(laure->lc, pauline->lc, &laure->stat.number_of_LinphoneCallOutgoingRinging, 1));

	bctbx_list_t *calls = bctbx_list_copy(linphone_core_get_calls(pauline->lc));
	for (iterator = calls; iterator; iterator = bctbx_list_next(iterator)) {
		LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(iterator);
		if (call != pauline_called_by_marie) {
			/*fine, this is the call waiting*/
			pauline_called_by_laure = call;
			linphone_call_accept(pauline_called_by_laure);
		}
	}
	bctbx_list_free(calls);

	BC_ASSERT_TRUE(wait_for(laure->lc, pauline->lc, &laure->stat.number_of_LinphoneCallConnected, 1));

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));

	if (pauline_called_by_laure && enable_caller_privacy)
		BC_ASSERT_EQUAL(linphone_call_params_get_privacy(linphone_call_get_current_params(pauline_called_by_laure)),
		                LinphonePrivacyId, int, "%d");
	/*wait a bit for ACK to be sent*/
	wait_for_list(lcs, NULL, 0, 1000);
	linphone_core_terminate_all_calls(pauline->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}
static void call_waiting_indication(void) {
	call_waiting_indication_with_param(FALSE);
}

static void call_waiting_indication_with_privacy(void) {
	call_waiting_indication_with_param(TRUE);
}

static void second_call_rejection(bool_t second_without_audio) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCall *pauline_call;
	LinphoneCallParams *params;
	LinphoneCall *marie_call;

	/*start a call to pauline*/
	linphone_core_invite_address(marie->lc, pauline->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));

	/*attempt to send a second call while the first one is not answered.
	 * It must be rejected by the core, since the audio resources are already engaged for the first call*/
	params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_audio(params, !second_without_audio);
	marie_call = linphone_core_invite_with_params(marie->lc, "sip:laure_non_existent@sip.example.org", params);

	linphone_call_params_unref(params);

	if (second_without_audio) {
		BC_ASSERT_PTR_NOT_NULL(marie_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));

	} else {
		BC_ASSERT_PTR_NULL(marie_call);
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (pauline_call) {
		linphone_call_accept(pauline_call);
	}
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void second_call_rejected_if_first_one_in_progress(void) {
	second_call_rejection(FALSE);
}

static void second_call_allowed_if_not_using_audio(void) {
	second_call_rejection(TRUE);
}

static void incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallState state) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	bctbx_list_t *lcs;
	LinphoneCallParams *laure_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);

	lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	if (state == LinphoneCallOutgoingRinging || state == LinphoneCallOutgoingEarlyMedia) {
		BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc, pauline->identity, marie_params));

		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));

		if (state == LinphoneCallOutgoingEarlyMedia)
			linphone_call_accept_early_media(linphone_core_get_current_call(pauline->lc));

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallOutgoingProgress, 1, int, "%d");
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc,
		                        state == LinphoneCallOutgoingEarlyMedia
		                            ? &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia
		                            : &marie->stat.number_of_LinphoneCallOutgoingRinging,
		                        1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc,
		                        (int *)&linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingbackTone,
		                        1));
	} else if (state == LinphoneCallOutgoingProgress) {
		BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address(marie->lc, pauline->identity));
	} else {
		ms_error("Unsupported state");
		return;
	}

	BC_ASSERT_TRUE(call_with_caller_params(laure, marie, laure_params));

	if (state == LinphoneCallOutgoingRinging) {
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingtone, 0, int, "%d");
	} else if (state == LinphoneCallOutgoingProgress || state == LinphoneCallOutgoingEarlyMedia) {
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingtone, 1, int, "%d");
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));

	linphone_core_terminate_all_calls(marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));

	linphone_call_params_unref(laure_params);
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}
static void incoming_call_accepted_when_outgoing_call_in_progress(void) {
	incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallOutgoingProgress);
}
static void incoming_call_accepted_when_outgoing_call_in_outgoing_ringing(void) {
	incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallOutgoingRinging);
}
static void incoming_call_accepted_when_outgoing_call_in_outgoing_ringing_early_media(void) {
	incoming_call_accepted_when_outgoing_call_in_state(LinphoneCallOutgoingEarlyMedia);
}

static void _simple_call_transfer(bool_t transferee_is_default_account, bool_t pause_before_transfer) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dual_proxy_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());

	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_calling_pauline;
	LinphoneCall *marie_calling_laure;

	char *laure_identity = linphone_address_as_string(laure->identity);
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	char *marie_identity = linphone_address_as_string(marie->identity);

	if (pause_before_transfer) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "pause_before_transfer", 1);
	}

	// Marie calls Pauline
	BC_ASSERT_TRUE(call(marie, pauline));
	marie_calling_pauline = linphone_core_get_current_call(marie->lc);
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_calling_pauline)) goto end;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);

	if (!transferee_is_default_account) {
		// Set Marie's second account (marie2) as default
		const bctbx_list_t *accounts = linphone_core_get_account_list(marie->lc);
		BC_ASSERT_EQUAL(bctbx_list_size(accounts), 2, size_t, "%zu");
		bctbx_list_t *it = (bctbx_list_t *)accounts;
		it = bctbx_list_next(it);
		LinphoneAccount *secondAccount = (LinphoneAccount *)it->data;
		linphone_core_set_default_account(marie->lc, secondAccount);
	}

	// Pauline transfers call from Marie to Laure
	linphone_call_transfer(pauline_called_by_marie, laure_identity);
	bctbx_free(laure_identity);
	if (pause_before_transfer) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote, 1, 10000));
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallRefered, 1, 2000));
	if (!pause_before_transfer) {
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallPausing, 0, int, "%i");
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallPaused, 0, int, "%i");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallPausedByRemote, 0, int, "%i");
	}

	// marie pausing pauline
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, 1, 2000));
	if (!pause_before_transfer) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1, 2000));
	} else {
		// Pauline's call is already in Paused state, it won't transition to PausedByRemote.
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 1, 2000));
	// marie calling laure
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));

	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_transfer_target_call(marie_calling_pauline));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneTransferCallOutgoingInit, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneTransferCallOutgoingProgress, 1, 2000));

	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_call)) goto end;
	linphone_call_accept(laure_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));

	marie_calling_laure = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(marie_calling_laure)) goto end;
	BC_ASSERT_PTR_EQUAL(linphone_call_get_transferer_call(marie_calling_laure), marie_calling_pauline);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneTransferCallConnected, 1, 2000));

	char *remote_address_str = linphone_call_get_remote_address_as_string(laure_call);
	BC_ASSERT_STRING_EQUAL(remote_address_str, marie_identity);
	ms_free(remote_address_str);

	// terminate marie to pauline call
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 1, 2000));

	end_call(marie, laure);
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 2000));

end:
	bctbx_free(marie_identity);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}

static void simple_call_transfer(void) {
	_simple_call_transfer(TRUE, FALSE);
}

static void simple_call_transfer_from_non_default_account(void) {
	/* this test is run in force_name_addr mode (ie enclose all URIs within < > within SIP headers)
	 * to make sure it has no incidence on the good execution of call transfers.
	 */
	belle_sip_stack_force_name_addr(TRUE);
	_simple_call_transfer(FALSE, FALSE);
	belle_sip_stack_force_name_addr(FALSE);
}

static void simple_call_transfer_with_pause_before(void) {
	_simple_call_transfer(TRUE, TRUE);
}

static void unattended_call_transfer(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *laure_call;
	const LinphoneAddress *referred_by;

	char *laure_identity = linphone_address_as_string(laure->identity);
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_called_by_marie = linphone_core_get_current_call(marie->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;

	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	reset_counters(&laure->stat);

	linphone_call_transfer(pauline_called_by_marie, laure_identity);
	bctbx_free(laure_identity);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallRefered, 1, 3000));

	// marie ends the call
	linphone_call_terminate(pauline_called_by_marie);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 3000));

	// Pauline starts the transfer
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1, 3000));
	laure_call = linphone_core_get_current_call(laure->lc);
	if (!BC_ASSERT_PTR_NOT_NULL(laure_call)) goto end;
	linphone_call_accept(laure_call);
	referred_by = linphone_call_get_referred_by_address(laure_call);
	if (BC_ASSERT_PTR_NOT_NULL(referred_by)) {
		BC_ASSERT_TRUE(linphone_address_weak_equal(referred_by, marie->identity));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 3000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 1, 3000));

	end_call(laure, pauline);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}

static void unattended_call_transfer_with_error(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCall *pauline_called_by_marie;
	bool_t call_ok = TRUE;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);

	lcs = bctbx_list_append(lcs, pauline->lc);

	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (call_ok) {
		pauline_called_by_marie = linphone_core_get_current_call(marie->lc);

		if (!BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie)) goto end;

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);

		linphone_call_transfer(pauline_called_by_marie, "unknown_user");
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallRefered, 1, 2000));

		// Pauline starts the transfer
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1, 2000));
		// and immediately get an error
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallError, 1, 2000));

		// the error must be reported back to marie
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallError, 1, 2000));

		// and pauline should resume the call automatically
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallResuming, 1, 2000));

		// and call should be resumed
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));

		end_call(marie, pauline);
	}
end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

void linphone_refer_requested(LinphoneCall *call, const LinphoneAddress *address) {
	LinphoneCore *lc = linphone_call_get_core(call);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCallReferRequested++;
	char *refer_to = linphone_address_as_string_uri_only(address);
	ms_message("A REFER request has been initiated to transfer the call to %s", refer_to);
	ms_free(refer_to);
}

void linphone_call_create_cbs_refer_requested(LinphoneCall *call) {
	LinphoneCallCbs *call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	BC_ASSERT_PTR_NOT_NULL(call);
	linphone_call_cbs_set_refer_requested(call_cbs, linphone_refer_requested);
	linphone_call_add_callbacks(call, call_cbs);
	linphone_call_cbs_unref(call_cbs);
}

static void call_transfer_existing_call(bool_t outgoing_call,
                                        bool_t auto_answer_replacing_calls,
                                        bool_t security_level_downgraded) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *laure_called_by_marie;
	LinphoneCall *pauline_call_laure = NULL;
	LinphoneCall *laure_called_by_pauline = NULL;
	LinphoneCall *lcall;
	bool_t call_ok = TRUE;
	const bctbx_list_t *calls;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	if (!auto_answer_replacing_calls)
		linphone_config_set_int(linphone_core_get_config(laure->lc), "sip", "auto_answer_replacing_calls", 0);

	if (security_level_downgraded) {
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "auto_accept_refer", 0);
		BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");

		LpConfig *lpm = linphone_core_get_config(marie->lc);
		LpConfig *lpp = linphone_core_get_config(pauline->lc);
		LpConfig *lpl = linphone_core_get_config(laure->lc);

		linphone_config_set_string(lpm, "sip", "zrtp_key_agreements_suites", "MS_ZRTP_KEY_AGREEMENT_X255");
		linphone_config_set_string(lpp, "sip", "zrtp_key_agreements_suites", "MS_ZRTP_KEY_AGREEMENT_X255");
		linphone_config_set_string(lpl, "sip", "zrtp_key_agreements_suites", "MS_ZRTP_KEY_AGREEMENT_DH3K");
	}

	// marie call pauline
	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (call_ok) {
		marie_call_pauline = linphone_core_get_current_call(marie->lc);
		pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
		linphone_call_create_cbs_security_level_downgraded(marie_call_pauline);
		linphone_call_create_cbs_security_level_downgraded(pauline_called_by_marie);
		linphone_call_create_cbs_refer_requested(marie_call_pauline);
		linphone_call_create_cbs_refer_requested(pauline_called_by_marie);

		// marie pause pauline
		if (!BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie))) {
			goto end;
		}

		if (outgoing_call) {
			// marie call laure
			if (!BC_ASSERT_TRUE(call(marie, laure))) {
				end_call(marie, pauline);
				goto end;
			}
		} else {
			// laure call marie
			if (!BC_ASSERT_TRUE(call(laure, marie))) {
				end_call(marie, pauline);
				goto end;
			}
		}

		marie_call_laure = linphone_core_get_current_call(marie->lc);
		laure_called_by_marie = linphone_core_get_current_call(laure->lc);

		if (!BC_ASSERT_PTR_NOT_NULL(marie_call_laure)) goto end;
		if (!BC_ASSERT_PTR_NOT_NULL(laure_called_by_marie)) goto end;

		linphone_call_create_cbs_security_level_downgraded(marie_call_laure);
		linphone_call_create_cbs_security_level_downgraded(laure_called_by_marie);

		// marie pause laure
		BC_ASSERT_TRUE(pause_call_1(marie, marie_call_laure, laure, laure_called_by_marie));

		reset_counters(&marie->stat);
		reset_counters(&pauline->stat);
		reset_counters(&laure->stat);

		linphone_call_transfer_to_another(marie_call_pauline, marie_call_laure);
		if (security_level_downgraded) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReferRequested, 1, 2000));
			linphone_call_accept_transfer(pauline_called_by_marie);
		}
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallRefered, 1, 2000));

		pauline_call_laure = linphone_core_get_current_call(pauline->lc);
		laure_called_by_pauline = linphone_core_get_current_call(laure->lc);
		linphone_call_create_cbs_security_level_downgraded(pauline_call_laure);
		linphone_call_create_cbs_security_level_downgraded(laure_called_by_pauline);

		// pauline pausing marie
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing, 1, 4000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused, 1, 4000));
		// pauline calling laure
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingInit, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingProgress, 1, 2000));

		// laure accept call
		if (!auto_answer_replacing_calls) {
			for (calls = linphone_core_get_calls(laure->lc); calls != NULL; calls = calls->next) {
				lcall = (LinphoneCall *)calls->data;
				if (linphone_call_get_state(lcall) == LinphoneCallIncomingReceived) {
					BC_ASSERT_PTR_EQUAL(linphone_call_get_replaced_call(lcall), laure_called_by_marie);
					linphone_call_accept(lcall);
					break;
				}
			}
		}

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallConnected, 1, 2000));

		if (security_level_downgraded) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallSecurityLevelDowngraded, 1, 2000));
		} else {
			BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallSecurityLevelDowngraded, 0, int, "%d");
		}

		// terminate marie to pauline/laure call
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, 2000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 2000));

		end_call(pauline, laure);
	}
end:
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

static void call_transfer_existing_call_outgoing_call(void) {
	call_transfer_existing_call(TRUE, TRUE, FALSE);
}

static void call_transfer_existing_call_outgoing_call_no_auto_answer(void) {
	call_transfer_existing_call(TRUE, FALSE, FALSE);
}

static void call_transfer_existing_call_incoming_call(void) {
	call_transfer_existing_call(FALSE, TRUE, FALSE);
}

static void call_transfer_existing_call_outgoing_call_no_auto_answer_security_level_downgraded(void) {
	call_transfer_existing_call(TRUE, FALSE, TRUE);
}

static void call_transfer_existing_call_incoming_call_no_auto_answer_security_level_downgraded(void) {
	call_transfer_existing_call(FALSE, FALSE, TRUE);
}

static void call_transfer_existing_ringing_call(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *marie_call_pauline;
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_call_laure;
	LinphoneCall *lcall;
	LinphoneCall *laure_call;
	bool_t call_ok = TRUE;
	const bctbx_list_t *calls;
	stats initial_marie_stats;
	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	// marie calls pauline
	BC_ASSERT_TRUE((call_ok = call(marie, pauline)));
	if (call_ok) {
		marie_call_pauline = linphone_core_get_current_call(marie->lc);
		pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
		// marie pauses pauline
		if (!BC_ASSERT_TRUE(pause_call_1(marie, marie_call_pauline, pauline, pauline_called_by_marie))) goto end;

		initial_marie_stats = marie->stat;
		BC_ASSERT_PTR_NOT_NULL((marie_call_laure = linphone_core_invite_address(marie->lc, laure->identity)));
		if (!marie_call_laure) goto end;
		BC_ASSERT_TRUE(wait_for(marie->lc, laure->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging,
		                        initial_marie_stats.number_of_LinphoneCallOutgoingRinging + 1));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
		laure_call = linphone_core_get_current_call(laure->lc);
		if (!BC_ASSERT_PTR_NOT_NULL(laure_call)) goto end;

		linphone_call_transfer_to_another(marie_call_pauline, marie_call_laure);
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallRefered, 1, 10000));

		// pauline pausing marie
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausing, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPaused, 1, 10000));
		// pauline calling laure
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingInit, 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallOutgoingProgress, 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 2, 10000));
		// laure accepts the new (replacing) call
		for (calls = linphone_core_get_calls(laure->lc); calls != NULL; calls = calls->next) {
			lcall = (LinphoneCall *)calls->data;
			if (lcall == laure_call) continue;
			if (linphone_call_get_state(lcall) == LinphoneCallIncomingReceived) {
				linphone_call_accept(lcall);
				break;
			}
		}
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallConnected, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneTransferCallConnected, 1, 10000));

		// terminate marie to pauline/laure call
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));

		end_call(pauline, laure);
	}

end:
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

void do_not_stop_ringing_when_declining_one_of_two_incoming_calls(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *pauline_called_by_laure;
	LinphoneCallParams *laure_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	bctbx_list_t *core_list = NULL;

	// Do not allow Pauline to use files as the goal of the test is to test audio routes on ring stream
	linphone_core_set_use_files(pauline->lc, FALSE);

	core_list = bctbx_list_append(core_list, marie->lc);
	core_list = bctbx_list_append(core_list, pauline->lc);
	core_list = bctbx_list_append(core_list, laure->lc);
	// Laure call Pauline
	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(laure->lc, pauline->identity, laure_params));
	linphone_call_params_unref(laure_params);

	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	pauline_called_by_laure = linphone_core_get_current_call(pauline->lc);
	const LinphoneCoreToneManagerStats *paulineToneManagerStats = linphone_core_get_tone_manager_stats(pauline->lc);
	BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_startRingtone, 1, int, "%d");
	// Marie call Pauline
	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc, pauline->identity, marie_params));
	linphone_call_params_unref(marie_params);

	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallIncomingReceived, 2, 10000));
	pauline_called_by_marie = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_startRingtone, 1, int, "%d");
	BC_ASSERT_TRUE(linphone_ringtoneplayer_is_started(linphone_core_get_ringtoneplayer(pauline->lc)));

	// Pauline decline Laure
	linphone_call_decline(pauline_called_by_laure, LinphoneReasonDeclined);
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));

	// check that ringtone player restart
	BC_ASSERT_TRUE(wait_for_list(core_list, (int *)&paulineToneManagerStats->number_of_stopRingtone, 1, 2000));
	BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_stopRingtone, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for_list(core_list, (int *)&paulineToneManagerStats->number_of_startRingtone, 2, 2000));

	BC_ASSERT_TRUE(linphone_ringtoneplayer_is_started(linphone_core_get_ringtoneplayer(pauline->lc)));
	linphone_call_terminate(pauline_called_by_marie);
	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallEnd, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallReleased, 2, 10000));
	BC_ASSERT_EQUAL(paulineToneManagerStats->number_of_stopRingtone, 2, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(core_list);
}

void stop_ringing_when_accepting_call_while_holding_another(bool_t activate_ice) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *pauline_called_by_marie;
	LinphoneCall *marie_called_by_laure;
	LinphoneCall *laure_call = NULL;
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *laure_params = linphone_core_create_call_params(laure->lc, NULL);
	bctbx_list_t *core_list = NULL;

	// Do not allow Pauline to use files as the goal of the test is to test audio routes on ring stream
	linphone_core_set_use_files(marie->lc, FALSE);

	core_list = bctbx_list_append(core_list, marie->lc);
	core_list = bctbx_list_append(core_list, pauline->lc);
	core_list = bctbx_list_append(core_list, laure->lc);

	// Enable ICE
	enable_stun_in_mgr(marie, TRUE, activate_ice, TRUE, TRUE);
	enable_stun_in_mgr(pauline, TRUE, activate_ice, TRUE, TRUE);
	enable_stun_in_mgr(laure, TRUE, activate_ice, TRUE, TRUE);

	// Marie calls Pauline
	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc, pauline->identity, marie_params));
	linphone_call_params_unref(marie_params);

	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1, 10000));
	pauline_called_by_marie = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(pauline->lc)->number_of_startRingtone, 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(marie->lc)->number_of_startRingbackTone, 1, int, "%d");

	// Laure calls Marie
	laure_call = linphone_core_invite_address_with_params(laure->lc, marie->identity, laure_params);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	LinphoneCallLog *laure_call_log = linphone_call_get_call_log(laure_call);
	linphone_call_params_unref(laure_params);

	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));

	const char *laure_call_id = linphone_call_log_get_call_id(laure_call_log);
	marie_called_by_laure = linphone_core_get_call_by_callid(marie->lc, laure_call_id);
	// marie_called_by_laure=linphone_core_get_current_call(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &laure->stat.number_of_LinphoneCallOutgoingRinging, 1, 10000));

	const LinphoneCoreToneManagerStats *marieToneStats = linphone_core_get_tone_manager_stats(marie->lc);

	wait_for_list(core_list, 0, 1, 2000); // Let some time ringing

	// Pauline accept Marie
	linphone_call_accept(pauline_called_by_marie);
	BC_ASSERT_TRUE(wait_for_list(core_list, (int *)&marieToneStats->number_of_stopRingbackTone, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));

	/* the call from laure should ring as a waiting tone */
	BC_ASSERT_TRUE(wait_for_list(core_list, (int *)&marieToneStats->number_of_startNamedTone, 1, 5000));

	// Marie accept Laure
	linphone_call_accept(marie_called_by_laure);

	BC_ASSERT_TRUE(wait_for_list(core_list, (int *)&marieToneStats->number_of_stopTone, 1, 5000)); // End of named tone

	linphone_call_terminate(marie_called_by_laure);
	linphone_call_terminate(pauline_called_by_marie);

	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &marie->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(core_list, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(core_list);
}

void stop_ringing_when_accepting_call_while_holding_another_with_ice(void) {
	stop_ringing_when_accepting_call_while_holding_another(TRUE);
}

void stop_ringing_when_accepting_call_while_holding_another_without_ice(void) {
	stop_ringing_when_accepting_call_while_holding_another(FALSE);
}

void no_auto_answer_on_fake_call_with_replaces_header(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie1->lc);
	lcs = bctbx_list_append(lcs, marie2->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);

	LinphoneCallParams *params = linphone_core_create_call_params(marie1->lc, NULL);
	linphone_call_params_set_privacy(params, LinphonePrivacyId);
	LinphoneCall *call1 = linphone_core_invite_address_with_params(marie1->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(call1);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallOutgoingRinging, 1, 3000));

	reset_counters(&marie1->stat);
	reset_counters(&marie2->stat);
	reset_counters(&pauline->stat);

	const char *callId = linphone_call_log_get_call_id(linphone_call_get_call_log(call1));
	BC_ASSERT_PTR_NOT_NULL(callId);
	SalOp *op = linphone_call_get_op_as_sal_op(call1);
	const char *fromTag = sal_call_get_local_tag(op);
	const char *toTag = sal_call_get_remote_tag(op);
	BC_ASSERT_PTR_NOT_NULL(fromTag);
	BC_ASSERT_PTR_NOT_NULL(toTag);

	params = linphone_core_create_call_params(marie2->lc, NULL);
	char *headerValue = bctbx_strdup_printf("%s;from-tag=%s;to-tag=%s", callId, fromTag, toTag);
	linphone_call_params_add_custom_header(params, "Replaces", headerValue);
	bctbx_free(headerValue);
	LinphoneCall *call2 = linphone_core_invite_address_with_params(marie2->lc, pauline->identity, params);
	linphone_call_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(call2);
	BC_ASSERT_EQUAL(marie2->stat.number_of_LinphoneCallOutgoingProgress, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallOutgoingRinging, 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallConnected, 1, 2000));

	linphone_call_terminate(call1);
	linphone_call_terminate(call2);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie1->stat.number_of_LinphoneCallReleased, 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie2->stat.number_of_LinphoneCallReleased, 1, 3000));

	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
	bctbx_list_free(lcs);
}

static void call_accepted_while_another_one_is_updating(bool_t update_from_callee) {
	int call_ring_timeout = 10000000;

	// Core that is called by other cores
	// ICE is enabled
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_inc_timeout(marie->lc, call_ring_timeout);

	bctbx_list_t *participants = NULL;
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, call_ring_timeout);
	participants = bctbx_list_append(participants, pauline);

	LinphoneCoreManager *chloe = linphone_core_manager_new("chloe_rc");
	linphone_core_set_inc_timeout(chloe->lc, call_ring_timeout);
	participants = bctbx_list_append(participants, chloe);

	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	linphone_core_set_inc_timeout(laure->lc, call_ring_timeout);
	participants = bctbx_list_append(participants, laure);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_core_set_video_activation_policy(pauline->lc, pol);
	linphone_core_set_video_activation_policy(marie->lc, pol);
	linphone_core_set_video_activation_policy(laure->lc, pol);
	linphone_core_set_video_activation_policy(chloe->lc, pol);
	linphone_video_activation_policy_unref(pol);

	linphone_core_set_video_device(marie->lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;

		linphone_core_set_video_device(c, liblinphone_tester_mire_id);
		linphone_core_enable_video_capture(c, TRUE);
		linphone_core_enable_video_display(c, TRUE);
	}

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, marie->lc);

	initiate_calls(participants, marie);

	unsigned int no_callers = (unsigned int)bctbx_list_size(participants);

	unsigned int idx = 0;

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore *c = m->lc;
		lcs = bctbx_list_append(lcs, c);

		LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(marie_call);

		// Take call - ringing ends
		linphone_call_accept(marie_call);

		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, (idx + 1), 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, (idx + 1), 5000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, idx, 5000));

		// Send update only on the first call taken
		if (it == participants) {
			LinphoneCall *call_to_update = NULL;
			if (update_from_callee) {
				call_to_update = linphone_core_get_call_by_remote_address2(marie->lc, m->identity);
			} else {
				call_to_update = linphone_core_get_current_call(c);
			}
			BC_ASSERT_PTR_NOT_NULL(call_to_update);
			if (call_to_update) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(c, call_to_update);
				linphone_call_params_enable_video(new_params, TRUE);
				linphone_call_update(call_to_update, new_params);
				linphone_call_params_unref(new_params);
			}
		}

		idx++;
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, no_callers, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, no_callers, 5000));

	LinphoneCoreManager *phead = (LinphoneCoreManager *)bctbx_list_get_data(participants);

	if (update_from_callee) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallUpdatedByRemote, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdating, 1, 5000));
	} else {
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallUpdatedByRemote, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallUpdating, 1, 5000));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &phead->stat.number_of_LinphoneCallPausedByRemote, 1, 5000));

	// Only one call is not paused
	unsigned int no_call_paused = no_callers - 1;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, no_call_paused, 5000));

	LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie->lc, phead->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	const LinphoneCallParams *marie_params = linphone_call_get_params(marie_call);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(marie_params));

	LinphoneCall *phead_call = linphone_core_get_current_call(phead->lc);
	BC_ASSERT_PTR_NOT_NULL(phead_call);
	const LinphoneCallParams *phead_params = linphone_call_get_params(phead_call);
	BC_ASSERT_TRUE(linphone_call_params_video_enabled(phead_params));

	LinphoneCall *pcall = NULL;
	unsigned int no_paused_by_remote = 0;
	LinphoneCoreManager *pm = NULL;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCall *call = linphone_core_get_current_call(m->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			no_paused_by_remote += (linphone_call_get_state(call) == LinphoneCallPausedByRemote) ? 1 : 0;
			if (linphone_call_get_state(call) == LinphoneCallStreamsRunning) {
				pcall = call;
				pm = m;
			}
		}
	}

	BC_ASSERT_PTR_NOT_NULL(pcall);
	if (pcall) {
		char *p_remote_address = linphone_call_get_remote_address_as_string(pcall);
		char *marie_identity = linphone_address_as_string(marie->identity);
		BC_ASSERT_EQUAL(strcmp(p_remote_address, marie_identity), 0, int, "%d");
		ms_free(p_remote_address);
		ms_free(marie_identity);
	}
	BC_ASSERT_EQUAL(no_paused_by_remote, no_call_paused, int, "%d");

	LinphoneCall *hcall = NULL;
	int no_active_calls_stream_running = 0;
	bctbx_list_t *calls = bctbx_list_copy(linphone_core_get_calls(marie->lc));
	for (bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
		LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(it);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			if (linphone_call_get_state(call) == LinphoneCallStreamsRunning) {
				no_active_calls_stream_running += 1;
				hcall = call;
			}
		}
	}
	bctbx_list_free(calls);

	BC_ASSERT_PTR_NOT_NULL(hcall);
	BC_ASSERT_PTR_NOT_NULL(pm);
	if (hcall && pm) {
		char *h_remote_address = linphone_call_get_remote_address_as_string(hcall);
		char *pm_address = linphone_address_as_string(pm->identity);
		BC_ASSERT_EQUAL(strcmp(h_remote_address, pm_address), 0, int, "%d");
		ms_free(h_remote_address);
		ms_free(pm_address);
	}
	BC_ASSERT_EQUAL(no_active_calls_stream_running, 1, int, "%d");

	linphone_core_terminate_all_calls(marie->lc);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, no_callers, 10000));

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &chloe->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, no_callers, 10000));

	bctbx_list_free(participants);
	bctbx_list_free(lcs);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void call_accepted_while_callee_is_updating_another_one(void) {
	call_accepted_while_another_one_is_updating(TRUE);
}

#if 0
static void call_accepted_while_caller_is_updating_to_same_callee(void) {
	call_accepted_while_another_one_is_updating(FALSE);
}

static void call_with_ice_negotiations_ending_while_accepting_call_base(bool_t back_to_back_accept) {
	LinphoneMediaEncryption mode = LinphoneMediaEncryptionNone;

	// Core that is called by other cores
	// ICE is enabled
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	linphone_core_set_inc_timeout(marie->lc, 10000);
	if (linphone_core_media_encryption_supported(marie->lc,mode)) {
		linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(marie->lc,mode);
	}

	bctbx_list_t* ice_participants=NULL;
	// ICE is enabled
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	linphone_core_set_inc_timeout(pauline->lc, 10000);
	if (linphone_core_media_encryption_supported(pauline->lc,mode)) {
		linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(pauline->lc,mode);
	}
	ice_participants=bctbx_list_append(ice_participants,pauline);

	// ICE is enabled
	LinphoneCoreManager* chloe = linphone_core_manager_new( "chloe_rc");
	linphone_core_set_inc_timeout(chloe->lc, 10000);
	if (linphone_core_media_encryption_supported(chloe->lc,mode)) {
		linphone_core_set_firewall_policy(chloe->lc,LinphonePolicyUseIce);
		linphone_core_set_media_encryption(chloe->lc,mode);
	}
	ice_participants=bctbx_list_append(ice_participants,chloe);

	bctbx_list_t* non_ice_participants=NULL;
	// ICE is disabled
	LinphoneCoreManager* laure = linphone_core_manager_new( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp");
	linphone_core_set_inc_timeout(laure->lc, 10000);
	non_ice_participants=bctbx_list_append(non_ice_participants,laure);

	// ICE is disabled
	LinphoneCoreManager* michelle = linphone_core_manager_new( "michelle_rc_udp");
	linphone_core_set_inc_timeout(michelle->lc, 10000);
	non_ice_participants=bctbx_list_append(non_ice_participants,michelle);

	bctbx_list_t* lcs = NULL;
	lcs=bctbx_list_append(lcs,marie->lc);

	initiate_calls(non_ice_participants, marie);
	initiate_calls(ice_participants, marie);

	unsigned int no_ice_callers = (unsigned int)bctbx_list_size(ice_participants);
	unsigned int no_non_ice_callers = (unsigned int)bctbx_list_size(non_ice_participants);
	unsigned int no_calls = no_ice_callers + no_non_ice_callers;

	bctbx_list_t* participants = NULL;
	participants=bctbx_list_append(participants,laure);
	participants=bctbx_list_append(participants,pauline);
	participants=bctbx_list_append(participants,michelle);

	bool_t callee_uses_ice = (linphone_core_get_firewall_policy(marie->lc) == LinphonePolicyUseIce);
	unsigned int no_calls_paused_by_remote = 0;
	LinphoneCoreManager * pm = NULL;

	if (back_to_back_accept) {
		lcs=bctbx_list_append(lcs,chloe->lc);
		const LinphoneAddress *chloe_uri = chloe->identity;
		LinphoneCall * marie_call = linphone_core_get_call_by_remote_address2(marie->lc, chloe_uri);
		BC_ASSERT_PTR_NOT_NULL(marie_call);

		// Take call - ringing ends
		linphone_call_accept(marie_call);

		BC_ASSERT_TRUE(wait_for_until(chloe->lc,marie->lc, &chloe->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_until(chloe->lc,marie->lc, &chloe->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		if (chloe && callee_uses_ice && linphone_core_get_firewall_policy(chloe->lc) == LinphonePolicyUseIce) {
			no_calls_paused_by_remote++;
			BC_ASSERT_TRUE(wait_for_until(chloe->lc,marie->lc, &chloe->stat.number_of_LinphoneCallUpdating,1,5000));
		}
		pm = chloe;

		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			const LinphoneAddress *caller_uri = m->identity;
			LinphoneCall * marie_call = linphone_core_get_call_by_remote_address2(marie->lc, caller_uri);
			BC_ASSERT_PTR_NOT_NULL(marie_call);

			// Take call - ringing ends
			linphone_call_accept(marie_call);
		}
	} else {
		participants=bctbx_list_prepend(participants,chloe);
	}

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);
		if (!back_to_back_accept) {
			const LinphoneAddress *caller_uri = m->identity;
			LinphoneCall * marie_call = linphone_core_get_call_by_remote_address2(marie->lc, caller_uri);
			BC_ASSERT_PTR_NOT_NULL(marie_call);

			// Take call - ringing ends
			linphone_call_accept(marie_call);
		}

		if (pm) {
			BC_ASSERT_TRUE(wait_for_until(pm->lc,marie->lc, &marie->stat.number_of_LinphoneCallPaused, no_calls_paused_by_remote, 5000));
			BC_ASSERT_TRUE(wait_for_until(pm->lc,marie->lc, &pm->stat.number_of_LinphoneCallPausedByRemote,1,5000));
		}

		BC_ASSERT_TRUE(wait_for_until(m->lc,marie->lc, &m->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_until(m->lc,marie->lc, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		if (m && callee_uses_ice && linphone_core_get_firewall_policy(m->lc) == LinphonePolicyUseIce) {
			no_calls_paused_by_remote++;
			BC_ASSERT_TRUE(wait_for_until(m->lc,marie->lc, &m->stat.number_of_LinphoneCallUpdating,1,5000));
		}
		pm = m;
	}

	if (pm && callee_uses_ice && linphone_core_get_firewall_policy(pm->lc) == LinphonePolicyUseIce) {
		BC_ASSERT_TRUE(wait_for_until(pm->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote, no_calls_paused_by_remote, 5000));
	}
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallConnected, no_calls, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, no_calls, 5000));

	for (bctbx_list_t *it = ice_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning, 2, 5000));
		BC_ASSERT_TRUE(check_ice(m,marie,LinphoneIceStateHostConnection));
		BC_ASSERT_TRUE(check_ice(marie,m,LinphoneIceStateHostConnection));
	}
	// Calls whose core enables ICE send a reINVITE hence they go twice to stream running
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, (2*no_ice_callers) +  no_non_ice_callers, 5000));

	// Only one call is not paused
	unsigned int no_call_paused = no_ice_callers + no_non_ice_callers - 1;
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, no_call_paused, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, no_call_paused, 5000));

	unsigned int no_paused_by_remote = 0;
	for (bctbx_list_t *it = ice_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		no_paused_by_remote += m->stat.number_of_LinphoneCallPausedByRemote;
	}

	for (bctbx_list_t *it = non_ice_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		no_paused_by_remote += m->stat.number_of_LinphoneCallPausedByRemote;
	}

	BC_ASSERT_EQUAL(no_paused_by_remote,no_call_paused, int, "%d");

	int no_active_calls_stream_running = 0;
	bctbx_list_t *calls = bctbx_list_copy(linphone_core_get_calls(marie->lc));
	for (bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
		LinphoneCall *call = (LinphoneCall *)bctbx_list_get_data(it);
		no_active_calls_stream_running += (linphone_call_get_state(call) == LinphoneCallStreamsRunning) ? 1 : 0;
	}
	bctbx_list_free(calls);

	BC_ASSERT_EQUAL(no_active_calls_stream_running,1, int, "%d");

	bctbx_list_free(participants);
	bctbx_list_free(non_ice_participants);
	bctbx_list_free(ice_participants);
	bctbx_list_free(lcs);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(michelle);
	linphone_core_manager_destroy(chloe);
}

static void call_with_ice_negotiations_ending_while_accepting_call(void) {
	call_with_ice_negotiations_ending_while_accepting_call_base(FALSE);
}

static void call_with_ice_negotiations_ending_while_accepting_call_back_to_back(void) {
	call_with_ice_negotiations_ending_while_accepting_call_base(TRUE);
}
#endif

/* Workflow:
 * Marie-Pauline-Laure : Accounts
 * - Pauline call Marie
 * - Pauline pauses Marie with inactive stream
 * - Laure call Marie
 * - Laure pauses Marie
 * - Pauline resumes
 * - Marie resumes Pauline
 * - Pauline pauses Marie with inactive stream
 * - Pauline resumes
 * */
void resuming_inactive_stream(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	LinphoneCall *pauline_call_marie, *laure_call_marie;
	LinphoneCall *marie_called_by_pauline;
	LinphoneCallParams *pauline_call_marie_params;

	bctbx_list_t *lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	// Pauline call Marie. Prepare early media to have meta-data debug for checking stream state (inactive/sendrecv)
	pauline_call_marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_early_media_sending(pauline_call_marie_params, TRUE);
	pauline_call_marie =
	    linphone_core_invite_address_with_params(pauline->lc, marie->identity, pauline_call_marie_params);
	linphone_call_params_unref(pauline_call_marie_params);
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 2000));
	linphone_call_accept_early_media(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 2000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 2000));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallOutgoingProgress, 1, int, "%d");

	if (pauline_call_marie != NULL) {
		marie_called_by_pauline = linphone_core_get_current_call(marie->lc);
		pauline_call_marie = linphone_core_get_current_call(pauline->lc);
		// Marie accept call
		linphone_call_accept(marie_called_by_pauline);
		if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 2000)))
			goto end; // Marie is in call with Pauline
		if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 2000))) goto end;
		// Pauline set inactive stream
		pauline_call_marie_params = linphone_core_create_call_params(pauline->lc, NULL);
		linphone_call_params_set_audio_direction(pauline_call_marie_params, LinphoneMediaDirectionInactive);
		linphone_call_update(pauline_call_marie, pauline_call_marie_params);
		linphone_call_params_unref(pauline_call_marie_params);
		if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 2000)))
			goto end; // By setting the current stream to inactive, Pauline has been entered in Running state
		if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1)))
			goto end; // Marie detect the inactive stream as a Paused from remote
			          // Laure call Marie
		if (BC_ASSERT_TRUE(call(laure, marie))) {
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausing, 1, 2000)))
				goto end; // Marie do pause when accepting the new call
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 1, 2000)))
				goto end; // Marie is paused on Pauline inactive call
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote, 1, 2000)))
				goto end; // Pauline receive the paused state from Marie
			laure_call_marie = linphone_core_get_current_call(laure->lc);
			// Laure pauses Marie
			linphone_call_pause(laure_call_marie);
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallPaused, 1, 2000)))
				goto end; // Laure do pauses
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote, 2, 2000)))
				goto end; // Marie get paused
				          // Pauline set sendrecv stream
			pauline_call_marie_params = linphone_core_create_call_params(pauline->lc, NULL);
			linphone_call_params_set_audio_direction(pauline_call_marie_params, LinphoneMediaDirectionSendRecv);
			linphone_call_update(pauline_call_marie, pauline_call_marie_params);
			linphone_call_params_unref(pauline_call_marie_params);
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallPausedByRemote, 2, 2000)))
				goto end; // When reactivating the call by stream, Pauline get the paused state from Marie. That allows
				          // Marie to resume
				          // Marie resumes Pauline
			linphone_call_resume(marie_called_by_pauline);
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 2, 2000)))
				goto end; // Marie pauses Laure call
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallResuming, 1, 2000)))
				goto end; // Marie resumes
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 3, 2000)))
				goto end; // Marie open the stream
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 3, 2000)))
				goto end; // Pauline run the stream as it is unpaused from Marie and in SendRecv state
				          // Pauline set inactive
			pauline_call_marie_params = linphone_core_create_call_params(pauline->lc, NULL);
			linphone_call_params_set_audio_direction(pauline_call_marie_params, LinphoneMediaDirectionInactive);
			linphone_call_update(pauline_call_marie, pauline_call_marie_params);
			linphone_call_params_unref(pauline_call_marie_params);
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPausedByRemote, 3, 2000)))
				goto end; // Marie has been paused from Inactive stream
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 4, 2000)))
				goto end; // By setting the stream to inactive, Pauline has been entered in Running state
				          // Pauline set sendrecv stream
			pauline_call_marie_params = linphone_core_create_call_params(pauline->lc, NULL);
			linphone_call_params_set_audio_direction(pauline_call_marie_params, LinphoneMediaDirectionSendRecv);
			linphone_call_update(pauline_call_marie, pauline_call_marie_params);
			linphone_call_params_unref(pauline_call_marie_params);
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallStreamsRunning, 4, 2000)))
				goto end; // comes from : Pauline call, Laure Call, 2 resumes
			if (!BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallStreamsRunning, 5, 2000)))
				goto end; // comes from  : first call, 2 inactives, 2 sendrecv

			wait_for_list(lcs, NULL, 5, 200);
			end_call(laure, marie);
		}
		end_call(pauline, marie);
	}
end:
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
	bctbx_list_free(lcs);
}

static void second_call_with_early_media(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreManager *laure = linphone_core_manager_new(get_laure_rc());
	bctbx_list_t *lcs;
	LinphoneCallParams *laure_params = linphone_core_create_call_params(laure->lc, NULL);
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);

	lcs = bctbx_list_append(NULL, marie->lc);
	lcs = bctbx_list_append(lcs, pauline->lc);
	lcs = bctbx_list_append(lcs, laure->lc);

	/* Marie makes a first call to pauline, who accepts it. */
	if (!BC_ASSERT_TRUE(call(marie, pauline))) {
		goto end;
	}
	/* Marie pauses this call and makes a second call to laure, who answers with early-media first */
	linphone_call_pause(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallPaused, 1, 10000));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_invite_address_with_params(marie->lc, laure->identity, marie_params));

	if (BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingReceived, 1, 10000))) {

		linphone_call_accept_early_media(linphone_core_get_current_call(laure->lc));
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallIncomingEarlyMedia, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1, 10000));

		/* The OutgoingEarlyMedia state should stop the "waiting" tone that was started for the first call being paused.
		 */
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, (int *)&linphone_core_get_tone_manager_stats(marie->lc)->number_of_stopTone, 1, 5000));
	}

	linphone_core_terminate_all_calls(marie->lc);

	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallEnd, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallEnd, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneCallReleased, 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneCallReleased, 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneCallReleased, 1, 10000));

end:
	linphone_call_params_unref(laure_params);
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	bctbx_list_free(lcs);
}

static void on_notify_response(LinphoneEvent *ev) {
	LinphoneEventCbs *cbs = linphone_event_get_current_callbacks(ev);
	int *number_of_notify_response = (int *)linphone_event_cbs_get_user_data(cbs);
	(*number_of_notify_response)++;
}

static void call_transfer_for_b2bua(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneAddress *referTo = linphone_address_new("sip:inexistant@anonymous.invalid");
	LinphoneCall *pauline_call;
	LinphoneEvent *notify_event = NULL;
	LinphoneEventCbs *event_cbs = NULL;
	int number_of_notify_response = 0;

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "auto_accept_refer", FALSE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "terminate_call_upon_transfer_completion",
	                        FALSE);

	if (!BC_ASSERT_TRUE(call(marie, pauline))) {
		goto end;
	}
	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_transfer_to(linphone_core_get_current_call(marie->lc), referTo);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallRefered, 1));
	/* do not execute the transfer but manually create the NOTIFY request to notify the progress of the transfer call */
	linphone_call_pause(pauline_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));
	/* make sure Pauline does not attempt to create the tranfer call */
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallOutgoingInit, 0, int, "%i");

	notify_event = linphone_call_create_notify(pauline_call, "refer");
	event_cbs = linphone_factory_create_event_cbs(linphone_factory_get());
	linphone_event_cbs_set_notify_response(event_cbs, on_notify_response);
	linphone_event_cbs_set_user_data(event_cbs, &number_of_notify_response);

	if (BC_ASSERT_PTR_NOT_NULL(notify_event)) {
		LinphoneContent *content = linphone_factory_create_content(linphone_factory_get());

		linphone_event_add_callbacks(notify_event, event_cbs);

		linphone_content_set_type(content, "message");
		linphone_content_set_subtype(content, "sipfrag");
		linphone_content_set_utf8_text(content, "SIP/2.0 100 Trying\r\n");
		linphone_event_notify(notify_event, content);
		linphone_content_unref(content);
		BC_ASSERT_TRUE(
		    wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneTransferCallOutgoingProgress, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &number_of_notify_response, 1));

		content = linphone_factory_create_content(linphone_factory_get());
		linphone_content_set_type(content, "message");
		linphone_content_set_subtype(content, "sipfrag");
		linphone_content_set_utf8_text(content, "SIP/2.0 200 Ringing\r\n");
		linphone_event_notify(notify_event, content);
		linphone_content_unref(content);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneTransferCallConnected, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &number_of_notify_response, 2));
	}
	/* Marie shall not automatically terminate the call, per the configuration key set at the beginning of the test */
	BC_ASSERT_FALSE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1, 1000));
	end_call(marie, pauline);

end:
	if (notify_event) {
		linphone_event_terminate(notify_event);
		linphone_event_unref(notify_event);
	}
	if (event_cbs) linphone_event_cbs_unref(event_cbs);
	linphone_address_unref(referTo);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static test_t multi_call_tests[] = {
    TEST_NO_TAG("Call waiting indication", call_waiting_indication),
    TEST_NO_TAG("Call waiting indication with privacy", call_waiting_indication_with_privacy),
    TEST_NO_TAG("Second call rejected if first one in progress", second_call_rejected_if_first_one_in_progress),
    TEST_NO_TAG("Second call allowed if not using audio", second_call_allowed_if_not_using_audio),
    TEST_NO_TAG("Incoming call accepted when outgoing call in progress",
                incoming_call_accepted_when_outgoing_call_in_progress),
    TEST_NO_TAG("Incoming call accepted when outgoing call in outgoing ringing",
                incoming_call_accepted_when_outgoing_call_in_outgoing_ringing),
    TEST_NO_TAG("Incoming call accepted when outgoing call in outgoing ringing early media",
                incoming_call_accepted_when_outgoing_call_in_outgoing_ringing_early_media),
    TEST_NO_TAG("Call accepted while callee is updating another one",
                call_accepted_while_callee_is_updating_another_one),
    //	TEST_NO_TAG("Call accepted while caller is updating to same callee",
    // call_accepted_while_caller_is_updating_to_same_callee), 	TEST_ONE_TAG("Call with ICE negotiations ending while
    // accepting call", call_with_ice_negotiations_ending_while_accepting_call, "ICE"), 	TEST_ONE_TAG("Call with ICE
    // negotiations ending while accepting call back to back",
    // call_with_ice_negotiations_ending_while_accepting_call_back_to_back, "ICE"),
    TEST_NO_TAG("Simple call transfer", simple_call_transfer),
    TEST_NO_TAG("Simple call transfer from non default account", simple_call_transfer_from_non_default_account),
    TEST_NO_TAG("Simple call transfer with pause before", simple_call_transfer_with_pause_before),
    TEST_NO_TAG("Unattended call transfer", unattended_call_transfer),
    TEST_NO_TAG("Unattended call transfer with error", unattended_call_transfer_with_error),
    TEST_NO_TAG("Call transfer existing outgoing call", call_transfer_existing_call_outgoing_call),
    TEST_NO_TAG("Call transfer existing outgoing call without auto answer of replacing call",
                call_transfer_existing_call_outgoing_call_no_auto_answer),
    TEST_NO_TAG("Call transfer existing incoming call", call_transfer_existing_call_incoming_call),
    TEST_NO_TAG("Call transfer existing outgoing call without auto answer of replacing call security level downgraded",
                call_transfer_existing_call_outgoing_call_no_auto_answer_security_level_downgraded),
    TEST_NO_TAG("Call transfer existing incoming call without auto answer of replacing call security level downgraded",
                call_transfer_existing_call_incoming_call_no_auto_answer_security_level_downgraded),
    TEST_NO_TAG("Call transfer existing ringing call", call_transfer_existing_ringing_call),
    TEST_NO_TAG("Do not stop ringing when declining one of two incoming calls",
                do_not_stop_ringing_when_declining_one_of_two_incoming_calls),
    TEST_NO_TAG("No auto answer on fake call with Replaces header", no_auto_answer_on_fake_call_with_replaces_header),
    TEST_NO_TAG("Resuming on inactive stream", resuming_inactive_stream),
    TEST_NO_TAG("Stop ringing when accepting call while holding another with ICE",
                stop_ringing_when_accepting_call_while_holding_another_with_ice),
    TEST_NO_TAG("Stop ringing when accepting call while holding another without ICE",
                stop_ringing_when_accepting_call_while_holding_another_without_ice),
    TEST_NO_TAG("Stop paused tone when second call is accepted with early media", second_call_with_early_media),
    TEST_NO_TAG("Call transfer for back to back user agent", call_transfer_for_b2bua)};

test_suite_t multi_call_test_suite = {"Multi call",
                                      NULL,
                                      NULL,
                                      liblinphone_tester_before_each,
                                      liblinphone_tester_after_each,
                                      sizeof(multi_call_tests) / sizeof(multi_call_tests[0]),
                                      multi_call_tests,
                                      0};
