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

static void srtp_call(void) {
	call_base(LinphoneMediaEncryptionSRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

/*
 *Purpose of this test is to check that even if caller and callee does not have exactly the same crypto suite configured, the matching crypto suite is used.
 */
static void srtp_call_with_different_crypto_suite(void) {
	call_base_with_configfile(LinphoneMediaEncryptionSRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "laure_tcp_rc", "marie_rc");
}

static void zrtp_call(void) {
	call_base(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void zrtp_sas_call(void) {
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_b256_rc", "pauline_zrtp_b256_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_b256_rc", "pauline_tcp_rc");
}

static void zrtp_cipher_call(void) {
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_srtpsuite_aes256_rc", "pauline_zrtp_srtpsuite_aes256_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_aes256_rc", "pauline_zrtp_aes256_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_aes256_rc", "pauline_tcp_rc");
}

static void zrtp_key_agreement_call(void) {
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_ecdh255_rc", "pauline_zrtp_ecdh255_rc");
	call_base_with_configfile(LinphoneMediaEncryptionZRTP,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE, "marie_zrtp_ecdh448_rc", "pauline_zrtp_ecdh448_rc");
}

static void dtls_srtp_call(void) {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,FALSE,LinphonePolicyNoFirewall,FALSE);
}

static void dtls_srtp_call_with_media_realy(void) {
	call_base(LinphoneMediaEncryptionDTLS,FALSE,TRUE,LinphonePolicyNoFirewall,FALSE);
}

static void zrtp_silent_call(void) {
	call_base_with_configfile_play_nothing(LinphoneMediaEncryptionZRTP,FALSE,TRUE,LinphonePolicyNoFirewall,FALSE,  "marie_rc", "pauline_tcp_rc");
}

static void call_with_declined_srtp(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionSRTP)) {
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionSRTP);

		BC_ASSERT_TRUE(call(pauline,marie));

		end_call(marie, pauline);
	} else {
		ms_warning ("not tested because srtp not available");
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_srtp_paused_and_resumed(void) {
	/*
	 * This test was made to evidence a bug due to internal usage of current_params while not yet filled by linphone_call_get_current_params().
	 * As a result it must not use the call() function because it calls linphone_call_get_current_params().
	 */
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const LinphoneCallParams *params;
	LinphoneCall *pauline_call;

	if (!linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionSRTP)) goto end;
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionSRTP);

	linphone_core_invite_address(pauline->lc, marie->identity);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallIncomingReceived,1))) goto end;
	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(linphone_core_get_current_call(marie->lc));

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,1))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,1))) goto end;

	linphone_call_pause(pauline_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallPaused,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallPausedByRemote,1));

	linphone_call_resume(pauline_call);
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneCallStreamsRunning,2))) goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneCallStreamsRunning,2))) goto end;

	/*assert that after pause and resume, SRTP is still being used*/
	params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params) , LinphoneMediaEncryptionSRTP, int, "%d");
	params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params) , LinphoneMediaEncryptionSRTP, int, "%d");

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_zrtp_configured_calling_base(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	if (ms_zrtp_available()) {

		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
		if (BC_ASSERT_TRUE(call(pauline,marie))){

			liblinphone_tester_check_rtcp(marie,pauline);

			LinphoneCall *call = linphone_core_get_current_call(marie->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))
						, LinphoneMediaEncryptionZRTP, int, "%i");

			call = linphone_core_get_current_call(pauline->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))
						, LinphoneMediaEncryptionZRTP, int, "%i");
			end_call(pauline, marie);
		}
	} else {
		ms_warning("Test skipped, ZRTP not available");
	}
}

static void call_with_zrtp_configured_calling_side(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	call_with_zrtp_configured_calling_base(marie,pauline);

	/* now set other encryptions mode for receiver(marie), we shall always fall back to caller preference: ZRTP */
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionDTLS);
	call_with_zrtp_configured_calling_base(marie,pauline);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	call_with_zrtp_configured_calling_base(marie,pauline);


	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionNone);

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	call_with_zrtp_configured_calling_base(marie,pauline);

	linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
	call_with_zrtp_configured_calling_base(marie,pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_zrtp_configured_callee_base(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	if (ms_zrtp_available()) {

		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
		if (BC_ASSERT_TRUE(call(pauline,marie))){

			liblinphone_tester_check_rtcp(marie,pauline);

			LinphoneCall *call = linphone_core_get_current_call(marie->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))
						, LinphoneMediaEncryptionZRTP, int, "%i");

			call = linphone_core_get_current_call(pauline->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))
						, LinphoneMediaEncryptionZRTP, int, "%i");
			end_call(pauline, marie);
		}
	} else {
		ms_warning("Test skipped, ZRTP not available");
	}
}

static void call_with_zrtp_configured_callee_side(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	call_with_zrtp_configured_callee_base(marie,pauline);

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	call_with_zrtp_configured_callee_base(marie,pauline);

	linphone_core_set_firewall_policy(marie->lc,LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc,LinphonePolicyUseIce);
	call_with_zrtp_configured_callee_base(marie,pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static bool_t quick_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2){
	linphone_core_invite_address(m1->lc, m2->identity);
	if (!BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallIncomingReceived, 1)))
		return FALSE;
	linphone_call_accept(linphone_core_get_current_call(m2->lc));
	if (!BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallStreamsRunning, 1)))
		return FALSE;
	if (!BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallStreamsRunning, 1)))
		return FALSE;
	return TRUE;
}

static void call_with_encryption_mandatory(bool_t caller_has_encryption_mandatory){
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallStats *marie_stats, *pauline_stats;
	/*marie doesn't support ZRTP at all*/
	// marie->lc->zrtp_not_available_simulation=1;
	linphone_core_set_zrtp_not_available_simulation(marie->lc, TRUE);

	/*pauline requests encryption to be mandatory*/
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);

	if (!caller_has_encryption_mandatory){
		if (!BC_ASSERT_TRUE(quick_call(marie, pauline))) goto end;
	}else{
		if (!BC_ASSERT_TRUE(quick_call(pauline, marie))) goto end;
	}
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 2000);

	/*assert that no RTP packets have been sent or received by Pauline*/
	/*testing packet_sent doesn't work, because packets dropped by the transport layer are counted as if they were sent.*/
#if 0
	BC_ASSERT_EQUAL(linphone_call_get_audio_stats(linphone_core_get_current_call(pauline->lc))->rtp_stats.packet_sent, 0, int, "%i");
#endif
	/*however we can trust packet_recv from the other party instead */
	marie_stats = linphone_call_get_audio_stats(linphone_core_get_current_call(marie->lc));
	pauline_stats = linphone_call_get_audio_stats(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL((int)linphone_call_stats_get_rtp_stats(marie_stats)->packet_recv, 0, int, "%i");
	BC_ASSERT_EQUAL((int)linphone_call_stats_get_rtp_stats(pauline_stats)->packet_recv, 0, int, "%i");
	linphone_call_stats_unref(marie_stats);
	linphone_call_stats_unref(pauline_stats);
	end_call(marie, pauline);

	end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_from_plain_rtp_to_zrtp(void){
	call_with_encryption_mandatory(FALSE);
}

static void call_from_zrtp_to_plain_rtp(void){
	call_with_encryption_mandatory(TRUE);
}

static void recreate_zrtpdb_when_corrupted(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc,LinphoneMediaEncryptionZRTP))) {
		void *db;
		const char* db_file;
		const char *filepath;
		const char *filepath2;
		const char *corrupt = "corrupt mwahahahaha";
		FILE *f;

		remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
		filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
		remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
		filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
		linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
		linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
		linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
		linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

		BC_ASSERT_TRUE(call(pauline,marie));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db);

		BC_ASSERT_TRUE(call(pauline,marie));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		//Corrupt db file
		db_file = linphone_core_get_zrtp_secrets_file(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db_file);

		f = fopen(db_file, "wb");
		fwrite(corrupt, 1, sizeof(corrupt), f);
		fclose(f);

		//Simulate relaunch of linphone core marie
		linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NULL(db);

		BC_ASSERT_TRUE(call(pauline,marie));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		BC_ASSERT_TRUE(call(pauline,marie));
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		//Db file should be recreated after corruption
		//Simulate relaunch of linphone core marie
		linphone_core_set_zrtp_secrets_file(marie->lc, filepath);

		BC_ASSERT_TRUE(call(pauline,marie));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db);
		db_file = linphone_core_get_zrtp_secrets_file(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db_file);

		BC_ASSERT_TRUE(call(pauline,marie));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_declined_encryption_mandatory(LinphoneMediaEncryption enc1, bool_t mandatory1, LinphoneMediaEncryption enc2, bool_t mandatory2) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_rc");

	if (!linphone_core_media_encryption_supported(marie->lc, enc1)) goto end;
	linphone_core_set_media_encryption(marie->lc, enc1);
	linphone_core_set_media_encryption_mandatory(marie->lc, mandatory1);

	if (!linphone_core_media_encryption_supported(pauline->lc, enc2)) goto end;
	linphone_core_set_media_encryption(pauline->lc, enc2);
	linphone_core_set_media_encryption_mandatory(pauline->lc, mandatory2);

	LinphoneCall* out_call = linphone_core_invite_address(pauline->lc,marie->identity);
	linphone_call_ref(out_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallError, 1));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");
	
	linphone_call_unref(out_call);
	
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_declined_encryption_mandatory_both_sides(void) {
	/* It is important to have SRTP second to test that there is no reinvite */
	call_declined_encryption_mandatory(LinphoneMediaEncryptionZRTP, TRUE, LinphoneMediaEncryptionSRTP, TRUE);
}

static void zrtp_mandatory_called_by_non_zrtp(void) {
	/* We do not try with None as it will accept the call and then set the media to ZRTP or SRTP since it will do a reinvite */
	call_declined_encryption_mandatory(LinphoneMediaEncryptionZRTP, TRUE, LinphoneMediaEncryptionDTLS, FALSE);
}

static void srtp_mandatory_called_by_non_srtp(void) {
	call_declined_encryption_mandatory(LinphoneMediaEncryptionSRTP, TRUE, LinphoneMediaEncryptionNone, FALSE);
	call_declined_encryption_mandatory(LinphoneMediaEncryptionSRTP, TRUE, LinphoneMediaEncryptionZRTP, FALSE);
	call_declined_encryption_mandatory(LinphoneMediaEncryptionSRTP, TRUE, LinphoneMediaEncryptionDTLS, FALSE);
}

static void srtp_dtls_mandatory_called_by_non_srtp_dtls(void) {
	/* We do not try with SRTP since it will do a reinvite */
	call_declined_encryption_mandatory(LinphoneMediaEncryptionDTLS, TRUE, LinphoneMediaEncryptionNone, FALSE);
	call_declined_encryption_mandatory(LinphoneMediaEncryptionDTLS, TRUE, LinphoneMediaEncryptionZRTP, FALSE);
}

static void zrtp_mandatory_called_by_srtp(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_rc");

	if (!linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP)) goto end;
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);

	if (!linphone_core_media_encryption_supported(pauline->lc, LinphoneMediaEncryptionSRTP)) goto end;
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	if (BC_ASSERT_TRUE(quick_call(pauline, marie))) {
		LinphoneCall *call = linphone_core_get_current_call(marie->lc);
		if (!BC_ASSERT_PTR_NOT_NULL(call)) goto end;

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEncryptedOn, 1));

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))
					, LinphoneMediaEncryptionZRTP, int, "%i");

		LinphoneCallParams *params = linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		if (!BC_ASSERT_PTR_NOT_NULL(params)) goto end;

		linphone_call_params_set_media_encryption(params, LinphoneMediaEncryptionSRTP);
		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))
					, LinphoneMediaEncryptionZRTP, int, "%i");

		end_call(pauline, marie);
		linphone_call_params_unref(params);
	}
	
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t call_secure_tests[] = {
	TEST_NO_TAG("SRTP call", srtp_call),
	TEST_NO_TAG("SRTP call with different crypto suite", srtp_call_with_different_crypto_suite),
	TEST_NO_TAG("ZRTP call", zrtp_call),
	TEST_NO_TAG("ZRTP silent call", zrtp_silent_call),
	TEST_NO_TAG("ZRTP SAS call", zrtp_sas_call),
	TEST_NO_TAG("ZRTP Cipher call", zrtp_cipher_call),
	TEST_NO_TAG("ZRTP Key Agreement call", zrtp_key_agreement_call),
	TEST_ONE_TAG("DTLS SRTP call", dtls_srtp_call, "DTLS"),
	TEST_ONE_TAG("DTLS SRTP call with media relay", dtls_srtp_call_with_media_realy, "DTLS"),
	TEST_NO_TAG("SRTP call with declined srtp", call_with_declined_srtp),
	TEST_NO_TAG("SRTP call paused and resumed", call_srtp_paused_and_resumed),
	TEST_NO_TAG("Call with ZRTP configured calling side only", call_with_zrtp_configured_calling_side),
	TEST_NO_TAG("Call with ZRTP configured receiver side only", call_with_zrtp_configured_callee_side),
	TEST_NO_TAG("Call from plain RTP to ZRTP mandatory should be silent", call_from_plain_rtp_to_zrtp),
	TEST_NO_TAG("Call ZRTP mandatory to plain RTP should be silent", call_from_zrtp_to_plain_rtp),
	TEST_NO_TAG("Recreate ZRTP db file when corrupted", recreate_zrtpdb_when_corrupted),
	TEST_NO_TAG("Call declined with mandatory encryption on both sides", call_declined_encryption_mandatory_both_sides),
	TEST_NO_TAG("ZRTP mandatory called by non ZRTP", zrtp_mandatory_called_by_non_zrtp),
	TEST_NO_TAG("SRTP mandatory called by non SRTP", srtp_mandatory_called_by_non_srtp),
	TEST_NO_TAG("SRTP DTLS mandatory called by non SRTP DTLS", srtp_dtls_mandatory_called_by_non_srtp_dtls),
	TEST_NO_TAG("ZRTP mandatory called by SRTP", zrtp_mandatory_called_by_srtp),
};

test_suite_t call_secure_test_suite = {"Secure Call", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(call_secure_tests) / sizeof(call_secure_tests[0]), call_secure_tests};