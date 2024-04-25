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

#include "bctoolbox/tester.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

#include "ortp/telephonyevents.h"

void send_dtmf_base(LinphoneCoreManager **pmarie,
                    LinphoneCoreManager **ppauline,
                    bool_t use_rfc2833,
                    bool_t use_sipinfo,
                    char dtmf,
                    char *dtmf_seq,
                    bool_t use_opus) {
	char *expected = NULL;
	int dtmf_count_prev;
	LinphoneCoreManager *marie = *pmarie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = *ppauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCall *marie_call = NULL;

	if (use_opus) {
		// if (!ms_filter_codec_supported("opus")) {
		if (!ms_factory_codec_supported(linphone_core_get_ms_factory(marie->lc), "opus") &&
		    !ms_factory_codec_supported(linphone_core_get_ms_factory(pauline->lc), "opus")) {

			ms_warning("Opus not supported, skipping test.");
			return;
		}
		disable_all_audio_codecs_except_one(marie->lc, "opus", 48000);
		disable_all_audio_codecs_except_one(pauline->lc, "opus", 48000);
	}

	linphone_core_set_use_rfc2833_for_dtmf(marie->lc, use_rfc2833);
	linphone_core_set_use_info_for_dtmf(marie->lc, use_sipinfo);
	linphone_core_set_use_rfc2833_for_dtmf(pauline->lc, use_rfc2833);
	linphone_core_set_use_info_for_dtmf(pauline->lc, use_sipinfo);

	BC_ASSERT_TRUE(call(pauline, marie));

	marie_call = linphone_core_get_current_call(marie->lc);

	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (!marie_call) return;

	if (dtmf != '\0') {
		dtmf_count_prev = pauline->stat.dtmf_count;
		linphone_call_send_dtmf(marie_call, dtmf);

		/*wait for the DTMF to be received from pauline*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.dtmf_count, dtmf_count_prev + 1, 10000));
		expected = ms_strdup_printf("%c", dtmf);
	}

	if (dtmf_seq != NULL) {
		int dtmf_delay_ms = linphone_config_get_int(linphone_core_get_config(linphone_call_get_core(marie_call)), "net",
		                                            "dtmf_delay_ms", 200);
		dtmf_count_prev = pauline->stat.dtmf_count;
		linphone_call_send_dtmfs(marie_call, dtmf_seq);

		/*wait for the DTMF sequence to be received from pauline*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.dtmf_count,
		                              (int)(dtmf_count_prev + strlen(dtmf_seq)),
		                              (int)(10000 + dtmf_delay_ms * strlen(dtmf_seq))));
		expected = (dtmf != '\0') ? ms_strdup_printf("%c%s", dtmf, dtmf_seq) : ms_strdup(dtmf_seq);
	}

	if (expected != NULL) {
		BC_ASSERT_PTR_NOT_NULL(pauline->stat.dtmf_list_received);
		if (pauline->stat.dtmf_list_received) {
			BC_ASSERT_STRING_EQUAL(pauline->stat.dtmf_list_received, expected);
		}
		ms_free(expected);
	} else {
		BC_ASSERT_PTR_NULL(pauline->stat.dtmf_list_received);
	}
}

void send_dtmf_cleanup(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	if (marie_call) {
		BC_ASSERT_PTR_NULL(_linphone_call_get_dtmf_timer(marie_call));
		BC_ASSERT_FALSE(_linphone_call_has_dtmf_sequence(marie_call));

		/*just to sleep*/
		linphone_core_terminate_all_calls(pauline->lc);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	}

	linphone_core_play_dtmf(marie->lc, '#', 900);
	/*wait a few time to ensure that DTMF is sent*/
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 500);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void send_dtmf_rfc2833(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, TRUE, FALSE, '1', NULL, FALSE);
	send_dtmf_cleanup(marie, pauline);
}

static void send_dtmfs_sequence_rfc2833_with_hardcoded_payload_type(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCall *marie_call = NULL;
	int dtmf_count_prev;

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "telephone_event_pt", 104);

	linphone_core_set_use_rfc2833_for_dtmf(marie->lc, TRUE);
	linphone_core_set_use_rfc2833_for_dtmf(pauline->lc, TRUE);

	if (BC_ASSERT_TRUE(call(pauline, marie))) {
		MediaStream *as;
		char *expected;
		char dtmf = '2';
		marie_call = linphone_core_get_current_call(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(marie_call);

		dtmf_count_prev = pauline->stat.dtmf_count;
		linphone_call_send_dtmf(marie_call, dtmf);

		/*wait for the DTMF to be received from pauline*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.dtmf_count, dtmf_count_prev + 1, 10000));
		expected = ms_strdup_printf("%c", dtmf);
		BC_ASSERT_PTR_NOT_NULL(pauline->stat.dtmf_list_received);
		if (pauline->stat.dtmf_list_received) {
			BC_ASSERT_STRING_EQUAL(pauline->stat.dtmf_list_received, expected);
		}
		ms_free(expected);
		as = linphone_call_get_stream(marie_call, 0);
		BC_ASSERT_EQUAL(rtp_session_telephone_events_supported(as->sessions.rtp_session), 104, int, "%i");
		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void send_dtmfs_sequence_rfc2833_with_different_numbering(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCall *pauline_call = NULL;
	int dtmf_count_prev;

	linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "telephone_event_pt", 104);
	linphone_core_set_answer_with_own_numbering_policy(marie->lc, TRUE);

	linphone_core_set_use_rfc2833_for_dtmf(marie->lc, TRUE);
	linphone_core_set_use_rfc2833_for_dtmf(pauline->lc, TRUE);

	if (BC_ASSERT_TRUE(call(pauline, marie))) {
		MediaStream *as;
		char *expected;
		char dtmf = '2';
		pauline_call = linphone_core_get_current_call(pauline->lc);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		dtmf_count_prev = marie->stat.dtmf_count;
		linphone_call_send_dtmf(pauline_call, dtmf);

		as = linphone_call_get_stream(pauline_call, 0);
		BC_ASSERT_EQUAL(as->sessions.rtp_session->tev_send_pt, 104, int, "%i");

		/*wait for the DTMF to be received by marie*/
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.dtmf_count, dtmf_count_prev + 1, 10000));
		expected = ms_strdup_printf("%c", dtmf);
		BC_ASSERT_PTR_NOT_NULL(marie->stat.dtmf_list_received);
		if (marie->stat.dtmf_list_received) {
			BC_ASSERT_STRING_EQUAL(marie->stat.dtmf_list_received, expected);
		}
		ms_free(expected);

		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void send_dtmf_sip_info(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, FALSE, TRUE, '#', NULL, FALSE);
	send_dtmf_cleanup(marie, pauline);
}

void linphone_call_send_dtmf__char__rfc2833_and_sip_info_enabled(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, TRUE, TRUE, '5', NULL, FALSE);
	send_dtmf_cleanup(marie, pauline);
}

static void send_dtmfs_sequence_rfc2833(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, TRUE, FALSE, '\0', "1230#", FALSE);
	send_dtmf_cleanup(marie, pauline);
}

static void send_dtmfs_sequence_sip_info(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, FALSE, TRUE, '\0', "1230#", FALSE);
	send_dtmf_cleanup(marie, pauline);
}

void linphone_call_send_dtmf__sequence__rfc2833_and_sip_info_enabled(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, TRUE, TRUE, '\0', "3125A", FALSE);
	send_dtmf_cleanup(marie, pauline);
}

static void send_dtmfs_sequence_call_state_changed(void) {
	LinphoneCoreManager *marie, *pauline;
	LinphoneCall *marie_call = NULL;
	send_dtmf_base(&marie, &pauline, FALSE, TRUE, '\0', NULL, FALSE);

	marie_call = linphone_core_get_current_call(marie->lc);
	if (marie_call) {
		/*very long DTMF(around 4 sec to be sent)*/
		linphone_call_send_dtmfs(marie_call, "123456789123456789");
		/*just after, change call state, and expect DTMF to be canceled*/
		linphone_call_pause(marie_call);
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPausing, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallPaused, 1));

		/*wait a few time to ensure that no DTMF are received*/
		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

		BC_ASSERT_PTR_NULL(pauline->stat.dtmf_list_received);
	}
	end_call(marie, pauline);
	send_dtmf_cleanup(marie, pauline);
}

static void send_dtmf_rfc2833_opus(void) {
	LinphoneCoreManager *marie, *pauline;
	send_dtmf_base(&marie, &pauline, TRUE, FALSE, '1', NULL, TRUE);
	send_dtmf_cleanup(marie, pauline);
}

test_t dtmf_tests[10] = {
    TEST_NO_TAG("Send DTMF using RFC2833", send_dtmf_rfc2833),
    TEST_NO_TAG("Send DTMF using SIP INFO", send_dtmf_sip_info),
    TEST_NO_TAG_AUTO_NAMED(linphone_call_send_dtmf__char__rfc2833_and_sip_info_enabled),
    TEST_NO_TAG("Send DTMF sequence using RFC2833", send_dtmfs_sequence_rfc2833),
    TEST_NO_TAG("Send DTMF sequence using RFC2833 with hardcoded payload type",
                send_dtmfs_sequence_rfc2833_with_hardcoded_payload_type),
    TEST_NO_TAG("Send DTMF sequence using RFC2833 and different payload type numbering",
                send_dtmfs_sequence_rfc2833_with_different_numbering),
    TEST_NO_TAG("Send DTMF sequence using SIP INFO", send_dtmfs_sequence_sip_info),
    TEST_NO_TAG_AUTO_NAMED(linphone_call_send_dtmf__sequence__rfc2833_and_sip_info_enabled),
    TEST_NO_TAG("DTMF sequence canceled if call state changed", send_dtmfs_sequence_call_state_changed),
    TEST_NO_TAG("Send DTMF using RFC2833 using Opus", send_dtmf_rfc2833_opus)};
