/*
	liblinphone_tester - liblinphone test suite
	Copyright (C) 2015  Belledonne Communications SARL

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
#include "liblinphone_tester.h"
#include "linphone/lpconfig.h"
#include "tester_utils.h"


#if HAVE_SIPP

#include <netdb.h>

void check_rtcp(LinphoneCall *call) {
	MSTimeSpec ts;
	LinphoneCallStats *audio_stats, *video_stats;

	linphone_call_ref(call);
	liblinphone_tester_clock_start(&ts);

	do {
		audio_stats = linphone_call_get_audio_stats(call);
		video_stats = linphone_call_get_video_stats(call);
		if (linphone_call_stats_get_round_trip_delay(audio_stats) > 0.0 && (!linphone_call_log_video_enabled(linphone_call_get_call_log(call)) || linphone_call_stats_get_round_trip_delay(video_stats) > 0.0)) {
			break;
		}
		linphone_call_stats_unref(audio_stats);
		if (video_stats) linphone_call_stats_unref(video_stats);
		wait_for_until(linphone_call_get_core(call), NULL, NULL, 0, 20); /*just to sleep while iterating*/
	} while (!liblinphone_tester_clock_elapsed(&ts, 15000));

	audio_stats = linphone_call_get_audio_stats(call);
	BC_ASSERT_GREATER(linphone_call_stats_get_round_trip_delay(audio_stats), 0.0, float, "%f");
	if (linphone_call_log_video_enabled(linphone_call_get_call_log(call))) {
		video_stats = linphone_call_get_video_stats(call);
		BC_ASSERT_GREATER(linphone_call_stats_get_round_trip_delay(video_stats), 0.0, float, "%f");
		linphone_call_stats_unref(video_stats);
	}
	linphone_call_stats_unref(audio_stats);

	linphone_call_unref(call);
}

FILE *sip_start(const char *senario, const char* dest_username, const char *passwd, LinphoneAddress* dest_addres) {
	char *dest;
	char *command;
	FILE *file;
	char local_ip[64];
	if (linphone_address_get_port(dest_addres)>0)
		dest = ms_strdup_printf("%s:%i",linphone_address_get_domain(dest_addres),linphone_address_get_port(dest_addres));
	else
		dest = ms_strdup_printf("%s",linphone_address_get_domain(dest_addres));
	
	linphone_core_get_local_ip_for(AF_INET, linphone_address_get_domain(dest_addres), local_ip);
	//until errors logs are handled correctly and stop breaks output, they will be DISABLED
	command = ms_strdup_printf(SIPP_COMMAND" -sf %s -s %s %s -i %s -trace_err -trace_msg -rtp_echo -m 1 -d 1000 -ap %s 2>/dev/null",senario
								,dest_username
								,dest
							    ,local_ip
								,(passwd?passwd:"none"));

	ms_message("Starting sipp command [%s]",command);
	file = popen(command, "r");
	ms_free(command);
	ms_free(dest);
	return file;
}


static FILE *sip_start_recv(const char *senario) {
	char *command;
	FILE *file;

	//until errors logs are handled correctly and stop breaks output, they will be DISABLED
	command = ms_strdup_printf(SIPP_COMMAND" -sf %s -trace_err -trace_msg -rtp_echo -m 1 -d 1000 2>/dev/null",senario);

	ms_message("Starting sipp command [%s]",command);
	file = popen(command, "r");
	ms_free(command);
	return file;
}

static void dest_server_server_resolved(void *data, belle_sip_resolver_results_t *results) {
	belle_sip_object_ref(results);
	*(belle_sip_resolver_results_t **)data = results;
}

LinphoneAddress * linphone_core_manager_resolve(LinphoneCoreManager *mgr, const LinphoneAddress *source) {
	char ipstring [INET6_ADDRSTRLEN];
	belle_sip_resolver_results_t *results = NULL;
	const struct addrinfo *ai = NULL;
	LinphoneAddress *dest;
	int err;
	int port = linphone_address_get_port(source);

	sal_resolve_a(
		linphone_core_get_sal(mgr->lc),
		linphone_address_get_domain(source),
		linphone_address_get_port(source),
		AF_INET,
		dest_server_server_resolved,
		&results
	);
	wait_for_until(mgr->lc, mgr->lc, (int*)&results, 1,2000);

	ai = belle_sip_resolver_results_get_addrinfos(results);
	err = bctbx_getnameinfo((struct sockaddr*)ai->ai_addr, ai->ai_addrlen, ipstring, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
	if (err != 0)
		ms_error("linphone_core_manager_resolve(): getnameinfo error %s", gai_strerror(err));
	dest = linphone_address_new(NULL);
	linphone_address_set_domain(dest, ipstring);
	if (port > 0)
		linphone_address_set_port(dest, port);

	return dest;
}


static void sip_update_within_icoming_reinvite_with_no_sdp(void) {
	LinphoneCoreManager *mgr;
	char *identity_char;
	char *scen;
	FILE * sipp_out;

	/*currently we use direct connection because sipp do not properly set ACK request uri*/
	mgr= linphone_core_manager_new2( "empty_rc", FALSE);
	mgr->identity= linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,"marie");
	identity_char=linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);
	linphone_core_iterate(mgr->lc);
	scen = bc_tester_res("sipp/sip_update_within_icoming_reinvite_with_no_sdp.xml");
	sipp_out = sip_start(scen, linphone_address_get_username(mgr->identity),NULL, mgr->identity);

	if (sipp_out) {
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
		if (linphone_core_get_current_call(mgr->lc)) {
			linphone_call_accept(linphone_core_get_current_call(mgr->lc));
			BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2));
			BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallEnd, 1));
		}
		pclose(sipp_out);
	}
	linphone_core_manager_destroy(mgr);
}

static void call_with_audio_mline_before_video_in_sdp(void) {
	LinphoneCoreManager *mgr;
	char *identity_char;
	char *scen;
	FILE * sipp_out;
	LinphoneCall *call = NULL;

	/*currently we use direct connection because sipp do not properly set ACK request uri*/
	mgr= linphone_core_manager_new2( "empty_rc", FALSE);
	mgr->identity = linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,"marie");
	identity_char = linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);

	linphone_core_iterate(mgr->lc);

	scen = bc_tester_res("sipp/call_with_audio_mline_before_video_in_sdp.xml");
	
	sipp_out = sip_start(scen, linphone_address_get_username(mgr->identity), NULL,  mgr->identity);

	if (sipp_out) {
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
		call = linphone_core_get_current_call(mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_accept(call);
			BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1));
			BC_ASSERT_EQUAL(_linphone_call_get_main_audio_stream_index(call), 0, int, "%d");
			BC_ASSERT_EQUAL(_linphone_call_get_main_video_stream_index(call), 1, int, "%d");
			BC_ASSERT_TRUE(_linphone_call_get_main_text_stream_index(call) > 1);
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(call)));

			check_rtcp(call);
		}

		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallEnd, 1));
		pclose(sipp_out);
	}
	linphone_core_manager_destroy(mgr);
}

static void call_with_video_mline_before_audio_in_sdp(void) {
	LinphoneCoreManager *mgr;
	char *identity_char;
	char *scen;
	FILE * sipp_out;
	LinphoneCall *call = NULL;

	/*currently we use direct connection because sipp do not properly set ACK request uri*/
	mgr= linphone_core_manager_new2( "empty_rc", FALSE);
	mgr->identity = linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,"marie");
	identity_char = linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);

	linphone_core_iterate(mgr->lc);

	scen = bc_tester_res("sipp/call_with_video_mline_before_audio_in_sdp.xml");

	sipp_out = sip_start(scen, linphone_address_get_username(mgr->identity), NULL, mgr->identity);

	if (sipp_out) {
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
		call = linphone_core_get_current_call(mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_accept(call);
			BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1));
			BC_ASSERT_EQUAL(_linphone_call_get_main_audio_stream_index(call), 1, int, "%d");
			BC_ASSERT_EQUAL(_linphone_call_get_main_video_stream_index(call), 0, int, "%d");
			BC_ASSERT_TRUE(_linphone_call_get_main_text_stream_index(call) > 1);
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(call)));

			check_rtcp(call);
		}

		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallEnd, 1));
		pclose(sipp_out);
	}
	linphone_core_manager_destroy(mgr);
}

static void call_with_multiple_audio_mline_in_sdp(void) {
	LinphoneCoreManager *mgr;
	char *identity_char;
	char *scen;
	FILE * sipp_out;
	LinphoneCall *call = NULL;

	/*currently we use direct connection because sipp do not properly set ACK request uri*/
	mgr= linphone_core_manager_new2( "empty_rc", FALSE);
	mgr->identity = linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,"marie");
	identity_char = linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);

	linphone_core_iterate(mgr->lc);

	scen = bc_tester_res("sipp/call_with_multiple_audio_mline_in_sdp.xml");

	sipp_out = sip_start(scen, linphone_address_get_username(mgr->identity), NULL, mgr->identity);

	if (sipp_out) {
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
		call = linphone_core_get_current_call(mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_accept(call);
			BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1));
			BC_ASSERT_EQUAL(_linphone_call_get_main_audio_stream_index(call), 0, int, "%d");
			BC_ASSERT_EQUAL(_linphone_call_get_main_video_stream_index(call), 2, int, "%d");
			BC_ASSERT_TRUE(_linphone_call_get_main_text_stream_index(call) > 2);
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(call)));

			check_rtcp(call);
		}

		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallEnd, 1));
		pclose(sipp_out);
	}
	linphone_core_manager_destroy(mgr);
}

static void call_with_multiple_video_mline_in_sdp(void) {
	LinphoneCoreManager *mgr;
	char *identity_char;
	char *scen;
	FILE * sipp_out;
	LinphoneCall *call = NULL;

	/*currently we use direct connection because sipp do not properly set ACK request uri*/
	mgr= linphone_core_manager_new2( "empty_rc", FALSE);
	mgr->identity = linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,"marie");
	identity_char = linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);

	linphone_core_iterate(mgr->lc);

	scen = bc_tester_res("sipp/call_with_multiple_video_mline_in_sdp.xml");

	sipp_out = sip_start(scen, linphone_address_get_username(mgr->identity), NULL, mgr->identity);

	if (sipp_out) {
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1));
		call = linphone_core_get_current_call(mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_accept(call);
			BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1));
			BC_ASSERT_EQUAL(_linphone_call_get_main_audio_stream_index(call), 0, int, "%d");
			BC_ASSERT_EQUAL(_linphone_call_get_main_video_stream_index(call), 1, int, "%d");
			BC_ASSERT_TRUE(_linphone_call_get_main_text_stream_index(call) > 3);
			BC_ASSERT_TRUE(linphone_call_log_video_enabled(linphone_call_get_call_log(call)));

			check_rtcp(call);
		}

		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallEnd, 1));
		pclose(sipp_out);
	}
	linphone_core_manager_destroy(mgr);
}


static void call_invite_200ok_without_contact_header(void) {
	LinphoneCoreManager *mgr;
	char *identity_char;
	char *scen;
	FILE * sipp_out;
	LinphoneCall *call = NULL;

	/*currently we use direct connection because sipp do not properly set ACK request uri*/
	mgr= linphone_core_manager_new2("empty_rc", FALSE);
	mgr->identity = linphone_core_get_primary_contact_parsed(mgr->lc);
	linphone_address_set_username(mgr->identity,"marie");
	identity_char = linphone_address_as_string(mgr->identity);
	linphone_core_set_primary_contact(mgr->lc,identity_char);

	linphone_core_iterate(mgr->lc);

	scen = bc_tester_res("sipp/call_invite_200ok_without_contact_header.xml");

	sipp_out = sip_start_recv(scen);

	if (sipp_out) {
		call = linphone_core_invite(mgr->lc, "sipp@127.0.0.1");
		BC_ASSERT_PTR_NOT_NULL(call);
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallOutgoingInit, 1));
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1));
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallOutgoingRinging, 1));
		/*assert that the call never gets connected nor terminated*/
		BC_ASSERT_FALSE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallConnected, 1));
		BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneCallEnd, 0, int, "%d");
		BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneCallError, 0, int, "%d");
		linphone_call_terminate(call);
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallEnd, 1));
		BC_ASSERT_TRUE(wait_for(mgr->lc, mgr->lc, &mgr->stat.number_of_LinphoneCallReleased, 1));
		pclose(sipp_out);
	}
	linphone_core_manager_destroy(mgr);
}


static test_t tests[] = {
	TEST_NO_TAG("SIP UPDATE within incoming reinvite without sdp", sip_update_within_icoming_reinvite_with_no_sdp),
	TEST_NO_TAG("Call with audio mline before video in sdp", call_with_audio_mline_before_video_in_sdp),
	TEST_NO_TAG("Call with video mline before audio in sdp", call_with_video_mline_before_audio_in_sdp),
	TEST_NO_TAG("Call with multiple audio mline in sdp", call_with_multiple_audio_mline_in_sdp),
	TEST_NO_TAG("Call with multiple video mline in sdp", call_with_multiple_video_mline_in_sdp),
	TEST_NO_TAG("Call invite 200ok without contact header", call_invite_200ok_without_contact_header)
};
#endif

static bool_t previous_liblinphonetester_ipv6;
static void before_each(void) {
	previous_liblinphonetester_ipv6=liblinphonetester_ipv6;
	liblinphonetester_ipv6=FALSE; /*sipp  do not support ipv6 and remote port*/
	liblinphone_tester_before_each();
}
static void after_each(void) {
	liblinphonetester_ipv6=previous_liblinphonetester_ipv6;
	liblinphone_tester_after_each();
}

test_suite_t complex_sip_call_test_suite = {
	"Complex SIP Case",
	NULL,
	NULL,
	before_each,
	after_each,
#if HAVE_SIPP
	sizeof(tests) / sizeof(tests[0]),
	tests
#else
	0,
	NULL
#endif
};
