/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "bctoolbox/crypto.hh"

#include "belle-sip/sipstack.h"

#include "mediastreamer2/msutils.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-call-stats.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "private.h"
#include "tester_utils.h"

#ifdef _WIN32
#define unlink _unlink
#ifndef F_OK
#define F_OK 00 /*visual studio does not define F_OK*/
#endif
#endif

typedef struct _ZrtpAlgoString ZrtpAlgoString;
struct _ZrtpAlgoString {
	const char *cipher_algo = NULL;          /**< Cipher algorithm */
	bctbx_list_t *key_agreement_algo = NULL; /**< Key agreement algorithm */
	const char *hash_algo = NULL;            /**< Hash algorithm */
	const char *auth_tag_algo = NULL;        /**< Authencation tag algorithm */
	const char *sas_algo = NULL;             /**< SAS algorithm */
};

typedef struct _ZrtpAlgoRes ZrtpAlgoRes;
struct _ZrtpAlgoRes {
	std::vector<int> cipher_algo;        /**< Cipher algorithm */
	std::vector<int> key_agreement_algo; /**< Key agreement algorithm */
	std::vector<int> hash_algo;          /**< Hash algorithm */
	std::vector<int> auth_tag_algo;      /**< Authencation tag algorithm */
	std::vector<int> sas_algo;           /**< SAS algorithm */
};

static void srtp_call_non_zero_tag(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "crypto_suite_tag_starting_value", 264);

	linphone_core_invite_address(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));
	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	liblinphone_tester_check_rtcp(marie, pauline);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	end_call(pauline, marie);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void
mgr_calling_each_other(LinphoneCoreManager *marie,
                       LinphoneCoreManager *pauline,
                       const std::function<void(LinphoneCall *marieCall, LinphoneCall *paulineCall)> &callback) {

	// Reset stats
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	linphone_core_reset_tone_manager_stats(marie->lc);
	linphone_core_reset_tone_manager_stats(pauline->lc);

	BC_ASSERT_TRUE(call(pauline, marie));
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (marie_call && pauline_call) {
		liblinphone_tester_check_rtcp(marie, pauline);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(marie), 70, int, "%i");
		LinphoneCallStats *pauline_stats = linphone_call_get_audio_stats(pauline_call);
		BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(pauline_stats) > 70);
		linphone_call_stats_unref(pauline_stats);
		pauline_stats = NULL;

		const LinphoneCallParams *params = NULL;
		params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
		LinphoneMediaEncryption pauline_encryption = linphone_call_params_get_media_encryption(params);
		params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
		LinphoneMediaEncryption marie_encryption = linphone_call_params_get_media_encryption(params);
		BC_ASSERT_EQUAL(pauline_encryption, marie_encryption, int, "%d");

		if (callback != nullptr) callback(marie_call, pauline_call);

		end_call(marie, pauline);
	}

	// Reset stats
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	linphone_core_reset_tone_manager_stats(marie->lc);
	linphone_core_reset_tone_manager_stats(pauline->lc);

	BC_ASSERT_TRUE(call(marie, pauline));

	marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);
	pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	if (marie_call && pauline_call) {
		liblinphone_tester_check_rtcp(pauline, marie);

		BC_ASSERT_GREATER(linphone_core_manager_get_max_audio_down_bw(pauline), 70, int, "%i");
		LinphoneCallStats *marie_stats = linphone_call_get_audio_stats(marie_call);
		BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(marie_stats) > 70);
		linphone_call_stats_unref(marie_stats);
		marie_stats = NULL;

		if (callback != nullptr) callback(marie_call, pauline_call);

		end_call(pauline, marie);
	}
}

/**
 * Check the given calls have stats matching the expected suite and source, send and receive channel are expected to be
 * the same, pauline and marie too optionnal stream type default to audio
 */
static bool_t srtp_check_call_stats(LinphoneCall *marieCall,
                                    LinphoneCall *paulineCall,
                                    int suite,
                                    int source,
                                    bool_t inner_encryption = FALSE,
                                    LinphoneStreamType streamType = LinphoneStreamTypeAudio) {
	LinphoneCallStats *marieStats = linphone_call_get_stats(marieCall, streamType);
	LinphoneCallStats *paulineStats = linphone_call_get_stats(paulineCall, streamType);
	auto *marieSrtpInfo = linphone_call_stats_get_srtp_info(marieStats, inner_encryption);
	auto *paulineSrtpInfo = linphone_call_stats_get_srtp_info(paulineStats, inner_encryption);
	bool_t ret = TRUE;

	// use BC_ASSERT_TRUE so we can collect the return value: true if all test pass false otherwise
	ret = ret && BC_ASSERT_TRUE(marieSrtpInfo->send_suite == suite);
	ret = ret && BC_ASSERT_TRUE(marieSrtpInfo->recv_suite == suite);
	ret = ret && BC_ASSERT_TRUE(paulineSrtpInfo->send_suite == suite);
	ret = ret && BC_ASSERT_TRUE(paulineSrtpInfo->recv_suite == suite);

	ret = ret && BC_ASSERT_TRUE(marieSrtpInfo->send_source == source);
	ret = ret && BC_ASSERT_TRUE(marieSrtpInfo->recv_source == source);
	ret = ret && BC_ASSERT_TRUE(paulineSrtpInfo->send_source == source);
	ret = ret && BC_ASSERT_TRUE(paulineSrtpInfo->recv_source == source);

	linphone_call_stats_unref(marieStats);
	linphone_call_stats_unref(paulineStats);

	return ret;
}
static void
generate_ekt(MSEKTParametersSet *ekt_params, MSEKTCipherType ekt_cipher, MSCryptoSuite crypto_suite, uint16_t spi) {
	ekt_params->ekt_cipher_type = ekt_cipher;
	ekt_params->ekt_srtp_crypto_suite = crypto_suite;
	// generate random ekt key and srtp salt. Warning: this is test code, do not use this weak RNG in actual code to
	// generate keys.
	bctoolbox::RNG::cRandomize(ekt_params->ekt_key_value, 32);
	bctoolbox::RNG::cRandomize(ekt_params->ekt_master_salt, 14);
	ekt_params->ekt_spi = spi;
	ekt_params->ekt_ttl = 0; // do not use ttl
}
static void
ekt_call(MSEKTCipherType ekt_cipher, MSCryptoSuite crypto_suite, bool unmatching_ekt = false, bool update_ekt = false) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	// perform a simple SDES call and when on going, add an ekt key
	mgr_calling_each_other(
	    marie, pauline,
	    ([marie, pauline, ekt_cipher, crypto_suite, unmatching_ekt, update_ekt](LinphoneCall *marieCall,
	                                                                            LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AEAD_AES_128_GCM,
		                                         MSSrtpKeySourceSDES)); // Default crypto suite is MS_AEAD_AES_128_GCM

		    MSEKTParametersSet ekt_params;
		    generate_ekt(&ekt_params, ekt_cipher, crypto_suite, 0x1234);

		    // Call is on, add ekt key to both parties (This might generate few srtp error message as old packets
		    // without ekt tag arrives to the receiver side which already expect EKT tags)
		    linphone_call_set_ekt(marieCall, &ekt_params);
		    if (unmatching_ekt) { // set unmatching EKT on both side: simply give different spi so the double encrypted
			                      // packets will be discarded
			    ekt_params.ekt_spi++;
		    }
		    linphone_call_set_ekt(paulineCall, &ekt_params);

		    // wait a little while to be sure some pakets with Full EKT tags reach the receiver
		    int dummy = 0;
		    wait_for_until(pauline->lc, marie->lc, &dummy, 1, 2000);

		    LinphoneCallStats *marie_stats = linphone_call_get_audio_stats(marieCall);
		    LinphoneCallStats *pauline_stats = linphone_call_get_audio_stats(paulineCall);

		    if (unmatching_ekt) { // EKT does not match so each end point sends data but cannot decrypt the incoming one
			    auto *marieSrtpInfo = linphone_call_stats_get_srtp_info(marie_stats, TRUE);
			    auto *paulineSrtpInfo = linphone_call_stats_get_srtp_info(pauline_stats, TRUE);

			    // call stats are initialiased to 0
			    // As the EKT SPI won't match, the inner encryption reception suite is never set and stays at 0
			    // So is the source
			    BC_ASSERT_TRUE(marieSrtpInfo->send_suite == crypto_suite);
			    BC_ASSERT_TRUE(marieSrtpInfo->recv_suite == 0);
			    BC_ASSERT_TRUE(paulineSrtpInfo->send_suite == crypto_suite);
			    BC_ASSERT_TRUE(paulineSrtpInfo->recv_suite == 0);

			    BC_ASSERT_TRUE(marieSrtpInfo->send_source == MSSrtpKeySourceEKT);
			    BC_ASSERT_TRUE(marieSrtpInfo->recv_source == 0);
			    BC_ASSERT_TRUE(paulineSrtpInfo->send_source == MSSrtpKeySourceEKT);
			    BC_ASSERT_TRUE(paulineSrtpInfo->recv_source == 0);

			    // Bandwidth usage tests it shall be near to 0, no new packets arrived in the last 2 seconds
			    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(marie_stats) < 5);
			    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(pauline_stats) < 5);
		    } else { // EKT matches so the stream should be double encrypted and fine
			    // Check we are now double encrypted and inner encryption comes from EKT and uses the given suite
			    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, crypto_suite, MSSrtpKeySourceEKT, TRUE));

			    // Bandwidth usage tests to be sure we actually still exchange data
			    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(marie_stats) > 70);
			    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(pauline_stats) > 70);

			    if (update_ekt) {
				    // Both parties update their EKT
				    generate_ekt(&ekt_params, ekt_cipher, crypto_suite, 0x2345);
				    linphone_call_set_ekt(marieCall, &ekt_params);
				    linphone_call_set_ekt(paulineCall, &ekt_params);
				    // wait a little while to be sure some pakets with Full EKT tags reach the receiver
				    wait_for_until(pauline->lc, marie->lc, &dummy, 1, 2000);
				    // The streams shall keep going on
				    linphone_call_stats_unref(marie_stats);
				    linphone_call_stats_unref(pauline_stats);
				    marie_stats = linphone_call_get_audio_stats(marieCall);
				    pauline_stats = linphone_call_get_audio_stats(paulineCall);
				    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(marie_stats) > 70);
				    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(pauline_stats) > 70);

				    // Marie updates her EKT but not pauline
				    generate_ekt(&ekt_params, ekt_cipher, crypto_suite, 0x3456);
				    linphone_call_set_ekt(marieCall, &ekt_params);
				    // wait a little while to be sure some pakets with Full EKT tags reach the receiver
				    wait_for_until(pauline->lc, marie->lc, &dummy, 1, 2000);
				    linphone_call_stats_unref(marie_stats);
				    linphone_call_stats_unref(pauline_stats);
				    marie_stats = linphone_call_get_audio_stats(marieCall);
				    pauline_stats = linphone_call_get_audio_stats(paulineCall);
				    // Marie still receives Pauline packets but Pauline cannot decrypt Marie's one
				    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(marie_stats) > 70);
				    BC_ASSERT_TRUE(linphone_call_stats_get_upload_bandwidth(marie_stats) > 70);
				    BC_ASSERT_TRUE(linphone_call_stats_get_download_bandwidth(pauline_stats) < 5);
			    }
		    }

		    linphone_call_stats_unref(marie_stats);
		    linphone_call_stats_unref(pauline_stats);
	    }));

	// cleaning
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void ekt_call(void) {
	ekt_call(MS_EKT_CIPHERTYPE_AESKW128, MS_AEAD_AES_128_GCM);
	ekt_call(MS_EKT_CIPHERTYPE_AESKW256, MS_AEAD_AES_256_GCM);
}

static void unmatching_ekt_call(void) {
	ekt_call(MS_EKT_CIPHERTYPE_AESKW128, MS_AEAD_AES_128_GCM, true);
	ekt_call(MS_EKT_CIPHERTYPE_AESKW256, MS_AEAD_AES_256_GCM, true);
}

static void updating_ekt_call(void) {
	ekt_call(MS_EKT_CIPHERTYPE_AESKW128, MS_AEAD_AES_128_GCM, false, true);
	ekt_call(MS_EKT_CIPHERTYPE_AESKW256, MS_AEAD_AES_256_GCM, false, true);
}

static void srtp_call(void) {
	// using call base
	call_base(LinphoneMediaEncryptionSRTP, FALSE, FALSE, LinphonePolicyNoFirewall, FALSE);

	// same test using mgr_calling_each_other so we can check during the call that the correct suite are used
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    // Default is MS_AES_128_GCM, we use SDES
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AEAD_AES_128_GCM, MSSrtpKeySourceSDES));
	    }));

	// Test differents crypto suites : AES_CM_128_HMAC_SHA1_80, AES_CM_128_HMAC_SHA1_32, AES_256_CM_HMAC_SHA1_80,
	// AES_256_CM_HMAC_SHA1_32, AEAD_AES_128_GCM, AEAD_AES_256_GCM
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80");
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_CM_128_HMAC_SHA1_80");
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_128_SHA1_80, MSSrtpKeySourceSDES));
	    }));

	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_32");
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_CM_128_HMAC_SHA1_32");
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_128_SHA1_32, MSSrtpKeySourceSDES));
	    }));

	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_256_CM_HMAC_SHA1_80");
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_256_CM_HMAC_SHA1_80");
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_256_SHA1_80, MSSrtpKeySourceSDES));
	    }));

	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_256_CM_HMAC_SHA1_32");
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_256_CM_HMAC_SHA1_32");
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_256_SHA1_32, MSSrtpKeySourceSDES));
	    }));

	linphone_core_set_srtp_crypto_suites(marie->lc, "AEAD_AES_128_GCM");
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AEAD_AES_128_GCM");
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AEAD_AES_128_GCM, MSSrtpKeySourceSDES));
	    }));

	linphone_core_set_srtp_crypto_suites(marie->lc, "AEAD_AES_256_GCM");
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AEAD_AES_256_GCM");
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AEAD_AES_256_GCM, MSSrtpKeySourceSDES));
	    }));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

/*
 *Purpose of this test is to check that even if caller and callee does not have exactly the same crypto suite
 *configured, the matching crypto suite is used.
 */
static void srtp_call_with_different_crypto_suite(void) {
	call_base_with_configfile(LinphoneMediaEncryptionSRTP, FALSE, FALSE, LinphonePolicyNoFirewall, FALSE,
	                          "laure_tcp_rc", "marie_rc");

	// same test using mgr_calling_each_other so we can check during the call that the correct suite are used
	LinphoneCoreManager *marie =
	    linphone_core_manager_new("marie_rc"); // marie_rc does not specify any srtp crypto suite, propose all
	                                           // availables, default is AES128_GCM
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_srtp_crypto_suites(pauline->lc,
	                                     "AES_256_CM_HMAC_SHA1_80"); // Force pauline to support only AES256_CM_SHA1_80

	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    // We shall use AES_256 as pauline supports only this one
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_256_SHA1_80, MSSrtpKeySourceSDES));
	    }));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void srtp_call_with_crypto_suite_parameters(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_srtp_crypto_suites(
	    marie->lc, "AES_CM_128_HMAC_SHA1_80, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 "
	               "UNENCRYPTED_SRTP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP");

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP, "
	                 "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80");

	// Marie prefers encrypted but allows unencrypted STRP streams
	// Pauline prefers unencrypted but allows encrypted STRP streams
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    LinphoneCallLog *clog = linphone_call_get_call_log(marieCall);
		    // When Marie is placing the call, we shall use AES_CM_128_HMAC_SHA1_80
		    if (linphone_call_log_get_dir(clog) == LinphoneCallOutgoing) {
			    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_128_SHA1_80, MSSrtpKeySourceSDES));
		    } else { // When Pauline is placing the call, we shall use AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP
			    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_128_SHA1_80_SRTCP_NO_CIPHER,
			                                         MSSrtpKeySourceSDES));
		    }
	    }));

	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP");
	// Marie prefers encrypted but allows unencrypted SRTP streams
	// Pauline supports unencrypted only
	mgr_calling_each_other(marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		                       BC_ASSERT_TRUE(srtp_check_call_stats(
		                           marieCall, paulineCall, MS_AES_128_SHA1_80_SRTCP_NO_CIPHER, MSSrtpKeySourceSDES));
	                       }));

	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80");
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP, "
	                 "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80");
	// Marie supports encrypted only
	// Pauline prefers unencrypted but allows encrypted STRP streams
	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_128_SHA1_80, MSSrtpKeySourceSDES));
	    }));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

// This test was added to ensure correct parsing of SDP with 2 crypto attributes
static void srtp_call_with_crypto_suite_parameters_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP");

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, FALSE);
	linphone_core_set_srtp_crypto_suites(pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP");

	LinphoneCall *call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");
	linphone_call_unref(call);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void srtp_call_with_crypto_suite_parameters_and_mandatory_encryption(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
	linphone_core_set_srtp_crypto_suites(
	    marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP UNENCRYPTED_SRTP, AES_CM_128_HMAC_SHA1_80 "
	               "UNENCRYPTED_SRTP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP");

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP, "
	                 "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP,AES_CM_128_HMAC_SHA1_80");

	LinphoneCall *call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");
	linphone_call_unref(call);

	// Marie answers with an inactive audio stream hence the call aborts
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));
	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNone, int, "%d");
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	linphone_call_unref(call);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void srtp_call_with_crypto_suite_parameters_and_mandatory_encryption_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
	linphone_core_set_srtp_crypto_suites(
	    marie->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP UNENCRYPTED_SRTP, AES_CM_128_HMAC_SHA1_80 "
	               "UNENCRYPTED_SRTP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP");

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);

	LinphoneCall *call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNotAcceptable, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallIncomingReceived, 0, int, "%d");
	linphone_call_unref(call);

	// Marie answers with an inactive audio stream hence the call aborts
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));
	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNone, int, "%d");
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	linphone_call_unref(call);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void srtp_call_with_crypto_suite_parameters_and_mandatory_encryption_3(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
	linphone_core_set_srtp_crypto_suites(marie->lc, "AES_CM_128_HMAC_SHA1_80");

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, FALSE);
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP, "
	                 "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP");

	LinphoneCall *call = linphone_core_invite_address(marie->lc, pauline->identity);
	linphone_call_ref(call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallOutgoingRinging, 1));
	linphone_call_accept(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNone, int, "%d");
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	linphone_call_unref(call);

	// Marie answers with an inactive audio stream hence the call aborts
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);
	call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));
	linphone_call_accept(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallConnected, 1));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallError, 1, 6000));
	BC_ASSERT_EQUAL(linphone_call_get_reason(call), LinphoneReasonNone, int, "%d");
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallReleased, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallReleased, 1));
	linphone_call_unref(call);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}
static void srtp_call_with_crypto_suite_parameters_and_mandatory_encryption_4(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_srtp_crypto_suites(
	    marie->lc, "AES_CM_128_HMAC_SHA1_80, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 "
	               "UNENCRYPTED_SRTP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP");
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_srtp_crypto_suites(
	    pauline->lc, "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP, "
	                 "AES_CM_128_HMAC_SHA1_80 UNENCRYPTED_SRTP UNENCRYPTED_SRTCP, AES_CM_128_HMAC_SHA1_80");
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);

	mgr_calling_each_other(
	    marie, pauline, ([](LinphoneCall *marieCall, LinphoneCall *paulineCall) {
		    BC_ASSERT_TRUE(srtp_check_call_stats(marieCall, paulineCall, MS_AES_128_SHA1_80, MSSrtpKeySourceSDES));
	    }));

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

/*
 *	In the case where Marie and Pauline do not have the same algorithms,
 *	the selected algorithm is not deterministic
 *	So we check if the selected algorithm is :
 *		- the algorithm owned by Marie (or Pauline)
 *		- or the default algorithm
 */
int zrtp_params_call2(ZrtpAlgoString marieAlgo, ZrtpAlgoString paulineAlgo, ZrtpAlgoRes res, bool_t isPQ) {
	bool_t call_ok;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");

	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP), 0, int, "%d");

	LpConfig *lpm = linphone_core_get_config(marie->lc);
	LpConfig *lpp = linphone_core_get_config(pauline->lc);

	linphone_config_set_string(lpm, "sip", "zrtp_cipher_suites", marieAlgo.cipher_algo);
	linphone_config_set_string(lpp, "sip", "zrtp_cipher_suites", paulineAlgo.cipher_algo);

	linphone_core_set_zrtp_key_agreement_suites(marie->lc, marieAlgo.key_agreement_algo);
	linphone_core_set_zrtp_key_agreement_suites(pauline->lc, paulineAlgo.key_agreement_algo);

	linphone_config_set_string(lpm, "sip", "zrtp_hash_suites", marieAlgo.hash_algo);
	linphone_config_set_string(lpp, "sip", "zrtp_hash_suites", paulineAlgo.hash_algo);

	linphone_config_set_string(lpm, "sip", "zrtp_auth_suites", marieAlgo.auth_tag_algo);
	linphone_config_set_string(lpp, "sip", "zrtp_auth_suites", paulineAlgo.auth_tag_algo);

	linphone_config_set_string(lpm, "sip", "zrtp_sas_suites", marieAlgo.sas_algo);
	linphone_config_set_string(lpp, "sip", "zrtp_sas_suites", paulineAlgo.sas_algo);

	BC_ASSERT_TRUE(call_ok = call(marie, pauline));
	if (call_ok) {
		/* Check encryption algorithms */
		LinphoneStreamType streamType = LinphoneStreamTypeAudio;

		LinphoneCall *marieCall = linphone_core_get_current_call(marie->lc);
		LinphoneCall *paulineCall = linphone_core_get_current_call(pauline->lc);

		LinphoneCallStats *marieStats = linphone_call_get_stats(marieCall, streamType);
		LinphoneCallStats *paulineStats = linphone_call_get_stats(paulineCall, streamType);

		BC_ASSERT_EQUAL(linphone_call_stats_is_zrtp_key_agreement_algo_post_quantum(marieStats), isPQ, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_stats_is_zrtp_key_agreement_algo_post_quantum(paulineStats), isPQ, int, "%d");

		const ZrtpAlgo *marieZrtpInfo = linphone_call_stats_get_zrtp_algo(marieStats);
		const ZrtpAlgo *paulineZrtpInfo = linphone_call_stats_get_zrtp_algo(paulineStats);

		if (res.cipher_algo.size() != 0) {
			if (res.cipher_algo.size() == 1) {
				BC_ASSERT_EQUAL(marieZrtpInfo->cipher_algo, res.cipher_algo.at(0), int, "%d");
			} else {
				BC_ASSERT_EQUAL(marieZrtpInfo->cipher_algo,
				                marieZrtpInfo->cipher_algo == res.cipher_algo.at(0) ? res.cipher_algo.at(0)
				                                                                    : res.cipher_algo.at(1),
				                int, "%d");
			}
			BC_ASSERT_EQUAL(marieZrtpInfo->cipher_algo, paulineZrtpInfo->cipher_algo, int, "%d");
		}
		if (res.key_agreement_algo.size() != 0) {
			if (res.key_agreement_algo.size() == 1) {
				BC_ASSERT_EQUAL(marieZrtpInfo->key_agreement_algo, res.key_agreement_algo.at(0), int, "%d");
			} else {
				BC_ASSERT_EQUAL(marieZrtpInfo->key_agreement_algo,
				                marieZrtpInfo->key_agreement_algo == res.key_agreement_algo.at(0)
				                    ? res.key_agreement_algo.at(0)
				                    : res.key_agreement_algo.at(1),
				                int, "%d");
			}
			BC_ASSERT_EQUAL(marieZrtpInfo->key_agreement_algo, paulineZrtpInfo->key_agreement_algo, int, "%d");
		}
		if (res.hash_algo.size() != 0) {
			if (res.hash_algo.size() == 1) {
				BC_ASSERT_EQUAL(marieZrtpInfo->hash_algo, res.hash_algo.at(0), int, "%d");
			} else {
				BC_ASSERT_EQUAL(marieZrtpInfo->hash_algo,
				                marieZrtpInfo->hash_algo == res.hash_algo.at(0) ? res.hash_algo.at(0)
				                                                                : res.hash_algo.at(1),
				                int, "%d");
			}
			BC_ASSERT_EQUAL(marieZrtpInfo->hash_algo, paulineZrtpInfo->hash_algo, int, "%d");
		}
		if (res.auth_tag_algo.size() != 0) {
			if (res.auth_tag_algo.size() == 1) {
				BC_ASSERT_EQUAL(marieZrtpInfo->auth_tag_algo, res.auth_tag_algo.at(0), int, "%d");
			} else {
				BC_ASSERT_EQUAL(marieZrtpInfo->auth_tag_algo,
				                marieZrtpInfo->auth_tag_algo == res.auth_tag_algo.at(0) ? res.auth_tag_algo.at(0)
				                                                                        : res.auth_tag_algo.at(1),
				                int, "%d");
			}
			BC_ASSERT_EQUAL(marieZrtpInfo->auth_tag_algo, paulineZrtpInfo->auth_tag_algo, int, "%d");
		}
		if (res.sas_algo.size() != 0) {
			if (res.sas_algo.size() == 1) {
				BC_ASSERT_EQUAL(marieZrtpInfo->sas_algo, res.sas_algo.at(0), int, "%d");
			} else {
				BC_ASSERT_EQUAL(marieZrtpInfo->sas_algo,
				                marieZrtpInfo->sas_algo == res.sas_algo.at(0) ? res.sas_algo.at(0) : res.sas_algo.at(1),
				                int, "%d");
			}
			BC_ASSERT_EQUAL(marieZrtpInfo->sas_algo, paulineZrtpInfo->sas_algo, int, "%d");
		}

		linphone_call_stats_unref(marieStats);
		linphone_call_stats_unref(paulineStats);

		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

	return 0;
}

int zrtp_params_call(ZrtpAlgoString marieAlgo, ZrtpAlgoString paulineAlgo, ZrtpAlgoRes res) {
	return zrtp_params_call2(marieAlgo, paulineAlgo, res, FALSE);
}

static void zrtp_call(void) {
	ZrtpAlgoString marieAlgo;
	ZrtpAlgoString paulineAlgo;
	ZrtpAlgoRes res;

	// Call with default params
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
}

static void zrtp_sas_call(void) {
	ZrtpAlgoString marieAlgo;
	ZrtpAlgoString paulineAlgo;
	ZrtpAlgoRes res;

	// Call where Marie and Pauline use :
	// - MS_ZRTP_SAS_B32 for their SAS algorithms
	marieAlgo.sas_algo = "MS_ZRTP_SAS_B32";
	paulineAlgo.sas_algo = "MS_ZRTP_SAS_B32";
	res.sas_algo = {MS_ZRTP_SAS_B32};

	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call where Marie and Pauline use :
	// - MS_ZRTP_SAS_B256 for their SAS algorithms
	marieAlgo.sas_algo = "MS_ZRTP_SAS_B256";
	paulineAlgo.sas_algo = "MS_ZRTP_SAS_B256";
	res.sas_algo = {MS_ZRTP_SAS_B256};

	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call where Marie uses MS_ZRTP_SAS_B256 and Pauline MS_ZRTP_SAS_B32
	// This result in using one or the other
	marieAlgo.sas_algo = "MS_ZRTP_SAS_B256";
	paulineAlgo.sas_algo = "MS_ZRTP_SAS_B32";
	res.sas_algo = {MS_ZRTP_SAS_B256, MS_ZRTP_SAS_B32};

	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
}

static void zrtp_cipher_call(void) {
	ZrtpAlgoString marieAlgo;
	ZrtpAlgoString paulineAlgo;
	ZrtpAlgoRes res;

	// Default is AES128
	marieAlgo.cipher_algo = NULL;
	paulineAlgo.cipher_algo = NULL;
	res.cipher_algo = {MS_ZRTP_CIPHER_AES1};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Using AES128
	marieAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1";
	paulineAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1";
	res.cipher_algo = {MS_ZRTP_CIPHER_AES1};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Using AES256
	marieAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES3";
	paulineAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES3";
	res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// One using AES128 and the other AES256, result can be any
	marieAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES3";
	paulineAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1";
	res.cipher_algo = {MS_ZRTP_CIPHER_AES3, MS_ZRTP_CIPHER_AES1};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
}

static void zrtp_key_agreement_call(void) {
	ZrtpAlgoString marieAlgo;
	ZrtpAlgoString paulineAlgo;
	ZrtpAlgoRes res;

	// Default is DH3k otherwise
	marieAlgo.key_agreement_algo = NULL;
	paulineAlgo.key_agreement_algo = NULL;
	res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_DH3K};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Use DH2k
	bctbx_list_t *ka_list = NULL;
	ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementDh2k));
	marieAlgo.key_agreement_algo = ka_list;
	paulineAlgo.key_agreement_algo = ka_list;
	res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_DH2K};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
	bctbx_list_free(ka_list);
	ka_list = NULL;

	// Use DH3k
	ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementDh3k));
	marieAlgo.key_agreement_algo = ka_list;
	paulineAlgo.key_agreement_algo = ka_list;
	res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_DH3K};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
	bctbx_list_free(ka_list);
	ka_list = NULL;

	if (bctbx_key_agreement_algo_list() & BCTBX_ECDH_X25519) { // Do we have ECDH
		// Use X25519
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementX255));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_X255};
		BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = NULL;

		// Use X448
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementX448));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_X448};
		// when using X448, we shall use SHA512  or SHA384 and AES256 when available
		marieAlgo.hash_algo = "MS_ZRTP_HASH_S256";
		paulineAlgo.hash_algo = "MS_ZRTP_HASH_S256";
		res.hash_algo = {MS_ZRTP_HASH_S256};
		marieAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1";
		paulineAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1";
		res.cipher_algo = {MS_ZRTP_CIPHER_AES1};
		BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

		marieAlgo.hash_algo = "MS_ZRTP_HASH_S256, MS_ZRTP_HASH_S384";
		paulineAlgo.hash_algo = "MS_ZRTP_HASH_S256, MS_ZRTP_HASH_S384";
		res.hash_algo = {MS_ZRTP_HASH_S384};
		marieAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1, MS_ZRTP_CIPHER_AES3";
		paulineAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1, MS_ZRTP_CIPHER_AES3";
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

		marieAlgo.hash_algo = "MS_ZRTP_HASH_S256, MS_ZRTP_HASH_S384, MS_ZRTP_HASH_S512";
		paulineAlgo.hash_algo = "MS_ZRTP_HASH_S256, MS_ZRTP_HASH_S384, MS_ZRTP_HASH_S512";
		res.hash_algo = {MS_ZRTP_HASH_S512};
		marieAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1, MS_ZRTP_CIPHER_AES3";
		paulineAlgo.cipher_algo = "MS_ZRTP_CIPHER_AES1, MS_ZRTP_CIPHER_AES3";
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = NULL;
	}
}

static void zrtp_post_quantum_key_agreement_call(void) {
	/* Check we retrieve correctly all available key agreement algorithms, order is defined in bzrtp/cryptoUtils.cc */
	bctbx_list_t *available_key_agreements = linphone_core_get_zrtp_available_key_agreement_list(NULL);
	bctbx_list_t *key_agreement = available_key_agreements;
	BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
	               LinphoneZrtpKeyAgreementX255);
	key_agreement = bctbx_list_next(key_agreement);
	BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
	               LinphoneZrtpKeyAgreementX448);
	key_agreement = bctbx_list_next(key_agreement);
	BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
	               LinphoneZrtpKeyAgreementDh3k);
	key_agreement = bctbx_list_next(key_agreement);
	if (ms_zrtp_is_PQ_available() == TRUE) {
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementMlk1);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementKyb1);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementHqc1);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementMlk2);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementKyb2);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementHqc2);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementMlk3);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementKyb3);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementHqc3);
		key_agreement = bctbx_list_next(key_agreement);
	}
	BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
	               LinphoneZrtpKeyAgreementDh2k);
	key_agreement = bctbx_list_next(key_agreement);
	if (ms_zrtp_is_PQ_available() == TRUE) {
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK255);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK448);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK255Mlk512);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK255Kyb512);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK255Hqc128);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK448Mlk1024);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK448Kyb1024);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK448Hqc256);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK255Kyb512Hqc128);
		key_agreement = bctbx_list_next(key_agreement);
		BC_ASSERT_TRUE((LinphoneZrtpKeyAgreement)(intptr_t)(bctbx_list_get_data(key_agreement)) ==
		               LinphoneZrtpKeyAgreementK448Kyb1024Hqc256);
		key_agreement = bctbx_list_next(key_agreement);
	}
	BC_ASSERT_PTR_NULL(key_agreement);
	bctbx_list_free(available_key_agreements);

	if (linphone_core_get_post_quantum_available() == TRUE) {
		ZrtpAlgoString marieAlgo;
		ZrtpAlgoString paulineAlgo;
		ZrtpAlgoRes res;
		BC_ASSERT_TRUE(linphone_core_get_post_quantum_available());

		// Use hybrid X25519/MLKem512
		bctbx_list_t *ka_list = nullptr;
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK255Mlk512));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K255_MLK512};
		// PQ algo should force(at config time) the use of SHA512 and AES256 even if we do not explicitely enable them
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X448/MLKem1024
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK448Mlk1024));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K448_MLK1024};
		// PQ algo should force the use of SHA512 and AES256
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X25519/Kyber512
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK255Kyb512));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K255_KYB512};
		// PQ algo should force(at config time) the use of SHA512 and AES256 even if we do not explicitely enable them
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X448/Kyber1024
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK448Kyb1024));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K448_KYB1024};
		// PQ algo should force the use of SHA512 and AES256
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X25519/HQC128
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK255Hqc128));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K255_HQC128};
		// PQ algo should force the use of SHA512 and AES256
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X448/HQC256
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK448Hqc256));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K448_HQC256};
		// PQ algo should force the use of SHA512 and AES256
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X25519/Kyber512/HQC128
		ka_list = nullptr;
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK255Kyb512Hqc128));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128};
		// PQ algo should force(at config time) the use of SHA512 and AES256 even if we do not explicitely enable them
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

		// Use hybrid X448/Kyber1024/HQC256
		ka_list = bctbx_list_append(ka_list, (void *)(intptr_t)(LinphoneZrtpKeyAgreementK448Kyb1024Hqc256));
		marieAlgo.key_agreement_algo = ka_list;
		paulineAlgo.key_agreement_algo = ka_list;
		res.key_agreement_algo = {MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256};
		// PQ algo should force the use of SHA512 and AES256
		res.cipher_algo = {MS_ZRTP_CIPHER_AES3};
		res.hash_algo = {MS_ZRTP_HASH_S512};
		BC_ASSERT_EQUAL(zrtp_params_call2(marieAlgo, paulineAlgo, res, TRUE), 0, int, "%d");
		bctbx_list_free(ka_list);
		ka_list = nullptr;

	} else {
		BC_ASSERT_FALSE(linphone_core_get_post_quantum_available());
		bctbx_warning("ZRTP post quantum key agreement test skipped as PostQuantum Crypto is disabled");
	}
}

static void zrtp_hash_call(void) {
	ZrtpAlgoString marieAlgo;
	ZrtpAlgoString paulineAlgo;
	ZrtpAlgoRes res;

	// Default is SHA256
	marieAlgo.hash_algo = NULL;
	paulineAlgo.hash_algo = NULL;
	res.hash_algo = {MS_ZRTP_HASH_S256};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call using SHA256
	marieAlgo.hash_algo = "MS_ZRTP_HASH_S256";
	paulineAlgo.hash_algo = "MS_ZRTP_HASH_S256";
	res.hash_algo = {MS_ZRTP_HASH_S256};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call using SHA384
	marieAlgo.hash_algo = "MS_ZRTP_HASH_S384";
	paulineAlgo.hash_algo = "MS_ZRTP_HASH_S384";
	res.hash_algo = {MS_ZRTP_HASH_S384};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call using SHA512
	marieAlgo.hash_algo = "MS_ZRTP_HASH_S512";
	paulineAlgo.hash_algo = "MS_ZRTP_HASH_S512";
	res.hash_algo = {MS_ZRTP_HASH_S512};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
}

static void zrtp_authtag_call(void) {
	ZrtpAlgoString marieAlgo;
	ZrtpAlgoString paulineAlgo;
	ZrtpAlgoRes res;

	// Default is GCM
	//  - this is a linphone internal default setting: SRTP crypto suite default is
	//      AEAD_AES_128_GCM, AES_CM_128_HMAC_SHA1_80, AEAD_AES_256_GCM, AES_256_CM_HMAC_SHA1_80
	//      So the default auth tag set by the audio-stream is GCM
	//  - default in bzrtp is GCM, HS32, HS80
	marieAlgo.auth_tag_algo = NULL;
	paulineAlgo.auth_tag_algo = NULL;
	res.auth_tag_algo = {MS_ZRTP_AUTHTAG_GCM};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call using GCM
	marieAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_GCM";
	paulineAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_GCM";
	res.auth_tag_algo = {MS_ZRTP_AUTHTAG_GCM};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call using HS80
	marieAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_HS80, MS_ZRTP_AUTHTAG_HS32, MS_ZRTP_AUTHTAG_GCM";
	paulineAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_HS80, MS_ZRTP_AUTHTAG_HS32, MS_ZRTP_AUTHTAG_GCM";
	res.auth_tag_algo = {MS_ZRTP_AUTHTAG_HS80};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call using HS32
	marieAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_HS32, MS_ZRTP_AUTHTAG_HS80, MS_ZRTP_AUTHTAG_GCM";
	paulineAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_HS32, MS_ZRTP_AUTHTAG_HS80, MS_ZRTP_AUTHTAG_GCM";
	res.auth_tag_algo = {MS_ZRTP_AUTHTAG_HS32};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");

	// Call with on HS32 one HS80, result can be anyone of them
	marieAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_HS32";
	paulineAlgo.auth_tag_algo = "MS_ZRTP_AUTHTAG_HS80";
	res.auth_tag_algo = {MS_ZRTP_AUTHTAG_HS80, MS_ZRTP_AUTHTAG_HS32};
	BC_ASSERT_EQUAL(zrtp_params_call(marieAlgo, paulineAlgo, res), 0, int, "%d");
}

static void dtls_srtp_call(void) {
	call_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE, LinphonePolicyNoFirewall, FALSE);
}

static void dtls_srtp_call_with_ice(void) {
	call_base(LinphoneMediaEncryptionDTLS, FALSE, FALSE, LinphonePolicyUseIce, FALSE);
}

static void dtls_srtp_call_with_ice_and_dtls_start_immediate(void) {
	call_base_with_configfile(LinphoneMediaEncryptionDTLS, FALSE, FALSE, LinphonePolicyUseIce, FALSE,
	                          "marie_dtls_srtp_immediate_rc", "pauline_dtls_srtp_immediate_rc");
}

static void dtls_srtp_call_with_media_realy(void) {
	call_base(LinphoneMediaEncryptionDTLS, FALSE, TRUE, LinphonePolicyNoFirewall, FALSE);
}

static void zrtp_silent_call(void) {
	call_base_with_configfile_play_nothing(LinphoneMediaEncryptionZRTP, FALSE, TRUE, LinphonePolicyNoFirewall, FALSE,
	                                       "marie_rc", "pauline_tcp_rc");
}

static void call_with_declined_srtp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	if (linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionSRTP)) {
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

		BC_ASSERT_TRUE(call(pauline, marie));

		end_call(marie, pauline);
	} else {
		ms_warning("not tested because srtp not available");
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_srtp_paused_and_resumed(void) {
	/*
	 * This test was made to evidence a bug due to internal usage of current_params while not yet filled by
	 * linphone_call_get_current_params(). As a result it must not use the call() function because it calls
	 * linphone_call_get_current_params().
	 */
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	const LinphoneCallParams *params;
	LinphoneCall *pauline_call;
	int marieSendMasterKey = 0;
	int paulineSendMasterKey = 0;

	if (!linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionSRTP)) goto end;
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	linphone_core_invite_address(pauline->lc, marie->identity);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1)))
		goto end;
	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_accept(linphone_core_get_current_call(marie->lc));

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1)))
		goto end;

	marieSendMasterKey = marie->stat.number_of_LinphoneCallSendMasterKeyChanged;
	paulineSendMasterKey = pauline->stat.number_of_LinphoneCallSendMasterKeyChanged;

	linphone_call_pause(pauline_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));

	linphone_call_resume(pauline_call);
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2)))
		goto end;

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallSendMasterKeyChanged, marieSendMasterKey, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCallSendMasterKeyChanged, paulineSendMasterKey, int, "%d");

	/*assert that after pause and resume, SRTP is still being used*/
	params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionSRTP, int, "%d");
	params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionSRTP, int, "%d");

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_zrtp_configured_calling_base(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	if (ms_zrtp_available()) {

		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
		if (BC_ASSERT_TRUE(call(pauline, marie))) {

			liblinphone_tester_check_rtcp(marie, pauline);

			LinphoneCall *call = linphone_core_get_current_call(marie->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),
			                LinphoneMediaEncryptionZRTP, int, "%i");

			call = linphone_core_get_current_call(pauline->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),
			                LinphoneMediaEncryptionZRTP, int, "%i");
			end_call(pauline, marie);
		}
	} else {
		ms_warning("Test skipped, ZRTP not available");
	}
}

static void call_with_zrtp_configured_calling_side(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	call_with_zrtp_configured_calling_base(marie, pauline);

	/* now set other encryptions mode for receiver(marie), we shall always fall back to caller preference: ZRTP */
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionDTLS);
	call_with_zrtp_configured_calling_base(marie, pauline);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	call_with_zrtp_configured_calling_base(marie, pauline);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionNone);

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	call_with_zrtp_configured_calling_base(marie, pauline);

	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	call_with_zrtp_configured_calling_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_with_zrtp_configured_callee_base(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	if (ms_zrtp_available()) {

		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
		if (BC_ASSERT_TRUE(call(pauline, marie))) {

			liblinphone_tester_check_rtcp(marie, pauline);

			LinphoneCall *call = linphone_core_get_current_call(marie->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),
			                LinphoneMediaEncryptionZRTP, int, "%i");

			call = linphone_core_get_current_call(pauline->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return;
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),
			                LinphoneMediaEncryptionZRTP, int, "%i");
			end_call(pauline, marie);
		}
	} else {
		ms_warning("Test skipped, ZRTP not available");
	}
}

static void call_with_zrtp_configured_callee_side(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	call_with_zrtp_configured_callee_base(marie, pauline);

	linphone_core_set_user_agent(pauline->lc, "Natted Linphone", NULL);
	linphone_core_set_user_agent(marie->lc, "Natted Linphone", NULL);
	call_with_zrtp_configured_callee_base(marie, pauline);

	linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
	linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	call_with_zrtp_configured_callee_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static bool_t quick_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2) {
	linphone_core_invite_address(m1->lc, m2->identity);
	if (!BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallIncomingReceived, 1))) return FALSE;
	linphone_call_accept(linphone_core_get_current_call(m2->lc));
	if (!BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallStreamsRunning, 1))) return FALSE;
	if (!BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallStreamsRunning, 1))) return FALSE;
	return TRUE;
}

static void call_with_encryption_mandatory(bool_t caller_has_encryption_mandatory) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallStats *marie_stats, *pauline_stats;
	/*marie doesn't support ZRTP at all*/
	// marie->lc->zrtp_not_available_simulation=1;
	linphone_core_set_zrtp_not_available_simulation(marie->lc, TRUE);

	/*pauline requests encryption to be mandatory*/
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);

	if (!caller_has_encryption_mandatory) {
		if (!BC_ASSERT_TRUE(quick_call(marie, pauline))) goto end;
	} else {
		if (!BC_ASSERT_TRUE(quick_call(pauline, marie))) goto end;
	}
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 2000);

	/*assert that no RTP packets have been sent or received by Pauline*/
	/*testing packet_sent doesn't work, because packets dropped by the transport layer are counted as if they were
	 * sent.*/
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

static void call_from_plain_rtp_to_zrtp(void) {
	call_with_encryption_mandatory(FALSE);
}

static void call_from_zrtp_to_plain_rtp(void) {
	call_with_encryption_mandatory(TRUE);
}

static void recreate_zrtpdb_when_corrupted(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP))) {
		void *db;
		char *db_file;
		const char *corrupt = "corrupt mwahahahaha";
		FILE *f;

		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);

		BC_ASSERT_TRUE(call(pauline, marie));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db);

		BC_ASSERT_TRUE(call(pauline, marie));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		// Corrupt db file
		db_file = bctbx_strdup(linphone_core_get_zrtp_secrets_file(marie->lc));
		BC_ASSERT_PTR_NOT_NULL(db_file);

		f = fopen(db_file, "wb");
		fwrite(corrupt, 1, sizeof(corrupt), f);
		fclose(f);

		// force marie's zrtp db reload: it will fail and run cacheless
		linphone_core_set_zrtp_secrets_file(marie->lc, db_file);
		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NULL(db);

		BC_ASSERT_TRUE(call(pauline, marie));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		// we run cacheless -> token is not verified on this call even if it was in previous one
		BC_ASSERT_TRUE(call(pauline, marie));
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		// Db file should be recreated after corruption
		//  force marie's zrtp db relaod
		linphone_core_set_zrtp_secrets_file(marie->lc, db_file);
		bctbx_free(db_file);
		db_file = NULL;

		BC_ASSERT_TRUE(call(pauline, marie));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db);
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_zrtp_secrets_file(marie->lc));

		BC_ASSERT_TRUE(call(pauline, marie));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This test checks the cache mismatch mechanism.
 */
static void zrtp_cache_mismatch_test(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP))) {
		void *db;
		stats pauline_stats;
		stats marie_stats;

		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);

		BC_ASSERT_TRUE(call(pauline, marie));
		// The SAS is accepted by both
		BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(linphone_core_get_current_call(pauline->lc)));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);

		db = linphone_core_get_zrtp_cache_db(marie->lc);
		BC_ASSERT_PTR_NOT_NULL(db); // Check ZRTP cache database

		// Delete Marie's ZRTP database
		delete_all_in_zrtp_table(marie->zrtp_secrets_database_path);

		pauline_stats = pauline->stat;
		marie_stats = marie->stat;
		BC_ASSERT_TRUE(call(pauline, marie));
		BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_zrtp_cache_mismatch_flag(
		    linphone_core_get_current_call(pauline->lc))); // Pauline has a cache mismatch
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEncryptedOn,
		                        pauline_stats.number_of_LinphoneCallEncryptedOn + 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEncryptedOn,
		                        marie_stats.number_of_LinphoneCallEncryptedOn + 1));
		// Both must revalidate the SAS
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
		// Cache mismatch is resolved
		BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(linphone_core_get_current_call(marie->lc)));
		BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(linphone_core_get_current_call(pauline->lc)));
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void authentication_token_verified(LinphoneCall *call, bool_t verified) {
	LinphoneCore *lc = linphone_call_get_core(call);
	stats *callstats = get_stats(lc);
	ms_message("Authentication token [%s]", verified ? "verified" : "incorrect");
	if (verified) callstats->number_of_LinphoneCallAuthenticationTokenVerified++;
	else callstats->number_of_LinphoneCallIncorrectAuthenticationTokenSelected++;
}

void linphone_call_create_cbs_authentication_token_verified(LinphoneCall *call) {
	LinphoneCallCbs *call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	BC_ASSERT_PTR_NOT_NULL(call);
	linphone_call_cbs_set_authentication_token_verified(call_cbs, authentication_token_verified);
	linphone_call_add_callbacks(call, call_cbs);
	linphone_call_cbs_unref(call_cbs);
}

void validate_sas_authentication(LinphoneCoreManager *tested_mgr,
                                 const LinphoneCoreManager *other_mgr,
                                 const LinphoneCall *tested_mgr_call,
                                 const bool correct_sas_selected,
                                 const stats &tested_mgr_stats) {
	if (correct_sas_selected) {
		BC_ASSERT_TRUE(wait_for(other_mgr->lc, tested_mgr->lc, &tested_mgr->stat.number_of_LinphoneCallEncryptedOn, 2));
		BC_ASSERT_EQUAL(tested_mgr_stats.number_of_LinphoneCallAuthenticationTokenVerified + 1,
		                tested_mgr->stat.number_of_LinphoneCallAuthenticationTokenVerified, int, "%d");
		BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(tested_mgr_call));
	} else {
		BC_ASSERT_EQUAL(tested_mgr_stats.number_of_LinphoneCallIncorrectAuthenticationTokenSelected + 1,
		                tested_mgr->stat.number_of_LinphoneCallIncorrectAuthenticationTokenSelected, int, "%d");
		BC_ASSERT_FALSE(linphone_call_get_authentication_token_verified(tested_mgr_call));
	}
}

/*
 * @brief This test involves a call between Marie and Pauline and they check the SAS using
 * linphone_call_check_authentication_token_selected.
 * @param[in] marie 						The LinphoneCoreManager of Marie
 * @param[in] pauline 						The LinphoneCoreManager of Pauline
 * @param[in] marie_correct_sas_selected 	TRUE if Marie selects the correct SAS
 * @param[in] pauline_correct_sas_selected 	TRUE if Pauline selects the correct SAS
 */
static void check_zrtp_short_code_base(LinphoneCoreManager *marie,
                                       LinphoneCoreManager *pauline,
                                       const bool_t marie_correct_sas_selected,
                                       const bool_t pauline_correct_sas_selected,
                                       const bool_t enable_latency) {
	LinphoneCall *marie_call = nullptr;
	LinphoneCall *pauline_call = nullptr;
	const char *marie_local_auth_token = nullptr;
	const char *pauline_local_auth_token = nullptr;

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);

	if (enable_latency) {
		OrtpNetworkSimulatorParams simparams = {0};
		simparams.mode = OrtpNetworkSimulatorOutbound;
		simparams.enabled = TRUE;
		simparams.latency = 50;
		linphone_core_set_network_simulator_params(marie->lc, &simparams);
		linphone_core_set_network_simulator_params(pauline->lc, &simparams);
	}

	stats marie_stats = marie->stat;
	stats pauline_stats = pauline->stat;

	BC_ASSERT_TRUE(call(pauline, marie));
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_call = linphone_core_get_current_call(pauline->lc);
	linphone_call_create_cbs_authentication_token_verified(marie_call);
	linphone_call_create_cbs_authentication_token_verified(pauline_call);
	BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(marie_call));
	BC_ASSERT_FALSE(linphone_call_get_zrtp_cache_mismatch_flag(pauline_call));

	BC_ASSERT_TRUE(BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallEncryptedOn,
	                                       marie_stats.number_of_LinphoneCallEncryptedOn + 1)));
	BC_ASSERT_TRUE(BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallEncryptedOn,
	                                       pauline_stats.number_of_LinphoneCallEncryptedOn + 1)));
	if (enable_latency) {
		// Check that latency does not cause an app notification indicating that there is no encryption because
		// receiving the authentication token takes longer.
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEncryptedOff, 0, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCallEncryptedOff, 0, int, "%d");
	}

	marie_stats = marie->stat;
	pauline_stats = pauline->stat;

	// Retrieve the local authentication tokens for both participants.
	marie_local_auth_token = linphone_call_get_local_authentication_token(marie_call);
	pauline_local_auth_token = linphone_call_get_local_authentication_token(pauline_call);

	const bctbx_list_t *it = linphone_call_get_remote_authentication_tokens(marie_call);
	if (marie_correct_sas_selected) {
		// If Marie is expected to select the correct SAS, iterate until the correct token is found.
		while (it && (strcmp(static_cast<const char *>(it->data), pauline_local_auth_token) != 0)) {
			it = it->next;
		}
	} else {
		// Otherwise, iterate until an incorrect token is found.
		while (it && (strcmp(static_cast<const char *>(it->data), pauline_local_auth_token) == 0)) {
			it = it->next;
		}
	}
	BC_ASSERT_PTR_NOT_NULL(it);
	linphone_call_check_authentication_token_selected(marie_call, static_cast<const char *>(it->data));

	if (pauline_correct_sas_selected) {
		// If Pauline is expected to select the correct SAS, iterate until the correct token is found.
		it = linphone_call_get_remote_authentication_tokens(pauline_call);
		while (it && (strcmp(static_cast<const char *>(it->data), marie_local_auth_token) != 0)) {
			it = it->next;
		}
		BC_ASSERT_PTR_NOT_NULL(it);
		linphone_call_check_authentication_token_selected(pauline_call, static_cast<const char *>(it->data));
	} else {
		// Otherwise, Pauline did not find the correct SAS, so she selects an empty token.
		linphone_call_check_authentication_token_selected(pauline_call, "");
	}

	validate_sas_authentication(marie, pauline, marie_call, marie_correct_sas_selected, marie_stats);
	validate_sas_authentication(pauline, marie, pauline_call, pauline_correct_sas_selected, pauline_stats);
}

/*
 * This test involves a call between Marie and Pauline. SAS is checked by comparing the real SAS and the SAS selected by
 * the user.
 */
static void check_correct_zrtp_short_code_test() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP))) {
		check_zrtp_short_code_base(marie, pauline, TRUE, TRUE, FALSE);
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This test simulates a call between Marie and Pauline with induced latency.
 * SAS is checked by comparing the real SAS and the SAS selected by the user.
 */
static void check_correct_zrtp_short_code_with_latency_test() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP))) {
		check_zrtp_short_code_base(marie, pauline, TRUE, TRUE, TRUE);
		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This test involves a call between Marie and Pauline. Users select an incorrect SAS.
 */
static void check_incorrect_zrtp_short_code_test() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP))) {
		check_zrtp_short_code_base(marie, pauline, FALSE, FALSE, FALSE);

		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, nullptr, 0, 2000));
		LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
		LinphoneCallStats *mstats = linphone_call_get_audio_stats(marie_call);
		LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
		LinphoneCallStats *pstats = linphone_call_get_audio_stats(pauline_call);

		// Marie and Pauline selected a wrong SAS. We verify that they neither send nor receive anything.
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(pstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_upload_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_upload_bandwidth(pstats), 0.0, float, "%f");

		end_call(marie, pauline);

		linphone_call_stats_unref(mstats);
		linphone_call_stats_unref(pstats);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This test involves a call between Marie and Pauline. Marie selects an incorrect SAS, and after that, Pauline attempts
 * to pause, resume and update the call.
 */
static void check_incorrect_zrtp_short_code_pause_resume_update_test() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionZRTP))) {
		LinphoneCallStats *pstats;
		LinphoneCallParams *params;
		stats marie_stats;
		stats pauline_stats;

		check_zrtp_short_code_base(marie, pauline, FALSE, TRUE, FALSE);

		LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
		LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);

		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, nullptr, 0, 2000));
		LinphoneCallStats *mstats = linphone_call_get_audio_stats(marie_call);
		pstats = linphone_call_get_audio_stats(pauline_call);

		// Marie selected a wrong SAS. We verify that she neither sends nor receives anything. Pauline, however, sends
		// but does not receive anything.
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(pstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_upload_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_NOT_EQUAL(linphone_call_stats_get_upload_bandwidth(pstats), 0.0, float, "%f");
		linphone_call_stats_unref(mstats);
		linphone_call_stats_unref(pstats);

		/* Pauline pauses the call */
		marie_stats = marie->stat;
		pauline_stats = pauline->stat;
		linphone_call_pause(pauline_call);
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused,
		                              pauline_stats.number_of_LinphoneCallPaused + 1, 1000));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote,
		                              marie_stats.number_of_LinphoneCallPausedByRemote + 1, 1000));

		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, nullptr, 0, 2000));
		mstats = linphone_call_get_audio_stats(marie_call);
		pstats = linphone_call_get_audio_stats(pauline_call);
		// We verify that we are still in the same state.
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(pstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_upload_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_NOT_EQUAL(linphone_call_stats_get_upload_bandwidth(pstats), 0.0, float, "%f");
		linphone_call_stats_unref(mstats);
		linphone_call_stats_unref(pstats);

		/* Pauline resumes the call */
		linphone_call_resume(pauline_call);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));

		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, nullptr, 0, 2000));
		mstats = linphone_call_get_audio_stats(marie_call);
		pstats = linphone_call_get_audio_stats(pauline_call);
		// We verify that we are still in the same state.
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(pstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_upload_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_NOT_EQUAL(linphone_call_stats_get_upload_bandwidth(pstats), 0.0, float, "%f");
		linphone_call_stats_unref(mstats);
		linphone_call_stats_unref(pstats);

		/* Pauline enables her video and updates the call */
		marie_stats = marie->stat;
		pauline_stats = pauline->stat;
		params = linphone_core_create_call_params(pauline->lc, pauline_call);
		linphone_call_params_enable_video(params, TRUE);
		BC_ASSERT_PTR_NOT_NULL(params);
		linphone_call_update(pauline_call, params);
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallUpdating,
		                              pauline_stats.number_of_LinphoneCallUpdating + 1, 1000));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallUpdatedByRemote,
		                              marie_stats.number_of_LinphoneCallUpdatedByRemote + 1, 1000));
		linphone_call_params_unref(params);

		// We verify that we are still in the same state.
		wait_for_until(marie->lc, pauline->lc, nullptr, 0, 1000);
		mstats = linphone_call_get_audio_stats(marie_call);
		pstats = linphone_call_get_audio_stats(pauline_call);
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_download_bandwidth(pstats), 0.0, float, "%f");
		BC_ASSERT_EQUAL(linphone_call_stats_get_upload_bandwidth(mstats), 0.0, float, "%f");
		BC_ASSERT_NOT_EQUAL(linphone_call_stats_get_upload_bandwidth(pstats), 0.0, float, "%f");
		linphone_call_stats_unref(mstats);
		linphone_call_stats_unref(pstats);

		// Check video
		liblinphone_tester_set_next_video_frame_decoded_cb(pauline_call);
		linphone_call_send_vfu_request(pauline_call);
		BC_ASSERT_FALSE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_IframeDecoded,
		                         marie_stats.number_of_IframeDecoded + 1));

		end_call(marie, pauline);
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

/*
 * This test verifies that when a user with a specific media encryption (mandatory or not) calls another
 * with a different mandatory media encryption, the call should be in error and the reason should be
 * 488 Not Acceptable.
 */
static void
call_declined_encryption_mandatory(LinphoneMediaEncryption enc1, LinphoneMediaEncryption enc2, bool_t mandatory) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneCall *out_call = NULL;

	if (!linphone_core_media_encryption_supported(marie->lc, enc1)) goto end;
	linphone_core_set_media_encryption(marie->lc, enc1);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);

	if (!linphone_core_media_encryption_supported(pauline->lc, enc2)) goto end;
	linphone_core_set_media_encryption(pauline->lc, enc2);
	linphone_core_set_media_encryption_mandatory(pauline->lc, mandatory);

	out_call = linphone_core_invite_address(pauline->lc, marie->identity);
	linphone_call_ref(out_call);

	/* We expect a 488 Not Acceptable */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallError, 1));
	BC_ASSERT_EQUAL(linphone_call_get_reason(out_call), LinphoneReasonNotAcceptable, int, "%d");

	linphone_call_unref(out_call);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void call_declined_encryption_mandatory_both_sides(void) {
	/* If SRTP wasn't mandatory then the call would not error, so it's a good case to test both mandatory */
	call_declined_encryption_mandatory(LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionSRTP, TRUE);
}

static void zrtp_mandatory_called_by_non_zrtp(void) {
	/* We do not try with None or SRTP as it will accept the call and then set the media to ZRTP */
	call_declined_encryption_mandatory(LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionDTLS, FALSE);
}

static void srtp_mandatory_called_by_non_srtp(void) {
	call_declined_encryption_mandatory(LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionNone, FALSE);
	call_declined_encryption_mandatory(LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionZRTP, FALSE);
	call_declined_encryption_mandatory(LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionDTLS, FALSE);
}

static void srtp_dtls_mandatory_called_by_non_srtp_dtls(void) {
	/* We do not try with SRTP as it will accept the call and then set the media to DTLS */
	call_declined_encryption_mandatory(LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionNone, FALSE);
	call_declined_encryption_mandatory(LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionZRTP, FALSE);
}

static void zrtp_mandatory_called_by_srtp(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");

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

		/*
		 * Marie is in ZRTP mandatory and Pauline in SRTP not mandatory.
		 * Declining SRTP with a 488 provokes a retry without SRTP, so the call should be in ZRTP.
		 */
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),
		                LinphoneMediaEncryptionZRTP, int, "%i");

		LinphoneCallParams *params =
		    linphone_core_create_call_params(pauline->lc, linphone_core_get_current_call(pauline->lc));
		if (!BC_ASSERT_PTR_NOT_NULL(params)) goto end;

		/* We test that a reinvite with SRTP is still not acceptable and thus do not change the encryption. */
		linphone_call_params_set_media_encryption(params, LinphoneMediaEncryptionSRTP);
		linphone_call_update(linphone_core_get_current_call(pauline->lc), params);

		wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(linphone_call_get_current_params(call)),
		                LinphoneMediaEncryptionZRTP, int, "%i");

		end_call(pauline, marie);
		linphone_call_params_unref(params);
	}

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void video_srtp_call_without_audio(void) {
	/*
	 * The purpose of this test is to ensure SRTP is still present in the SDP event if the audio stream is disabled
	 */
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCallParams *pauline_params;
	const LinphoneCallParams *params;

	LinphoneVideoActivationPolicy *vpol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(vpol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(vpol, TRUE);

	if (!linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionSRTP)) goto end;
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);

	linphone_core_set_video_activation_policy(marie->lc, vpol);
	linphone_core_enable_video_capture(marie->lc, TRUE);
	linphone_core_enable_video_display(marie->lc, TRUE);

	linphone_core_set_video_activation_policy(pauline->lc, vpol);
	linphone_core_enable_video_capture(pauline->lc, TRUE);
	linphone_core_enable_video_display(pauline->lc, TRUE);
	linphone_video_activation_policy_unref(vpol);

	pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_audio(pauline_params, FALSE);
	linphone_call_params_enable_video(pauline_params, TRUE);
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(pauline_params), LinphoneMediaEncryptionSRTP, int, "%i");
	linphone_core_invite_address_with_params(pauline->lc, marie->identity, pauline_params);
	linphone_call_params_unref(pauline_params);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1)))
		goto end;
	/*assert that SRTP is being used*/
	params = linphone_call_get_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionSRTP, int, "%d");
	params = linphone_call_get_remote_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionSRTP, int, "%d");

	linphone_core_accept_call(marie->lc, linphone_core_get_current_call(marie->lc));
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);
	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1)))
		goto end;

	/*assert that SRTP is being used*/
	params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionSRTP, int, "%d");
	params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionSRTP, int, "%d");

	end_call(pauline, marie);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static bool_t setup_dtls_srtp(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	if (!linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionDTLS)) {
		BC_FAIL("SRTP-DTLS not supported.");
		return FALSE;
	}
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionDTLS);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionDTLS);
	char *path = bc_tester_file("certificates-marie");
	linphone_core_set_user_certificates_path(marie->lc, path);
	bc_free(path);
	path = bc_tester_file("certificates-pauline");
	linphone_core_set_user_certificates_path(pauline->lc, path);
	bc_free(path);
	bctbx_mkdir(linphone_core_get_user_certificates_path(marie->lc));
	bctbx_mkdir(linphone_core_get_user_certificates_path(pauline->lc));
	return TRUE;
}

static void _dtls_srtp_audio_call_with_rtcp_mux(bool_t rtcp_mux_not_accepted) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphoneCall *pauline_call, *marie_call;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_mux", 1);
	if (!rtcp_mux_not_accepted) linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_mux", 1);

	setup_dtls_srtp(marie, pauline);
	{
		/*enable ICE on both ends*/
		enable_stun_in_mgr(marie, TRUE, TRUE, TRUE, TRUE);
		enable_stun_in_mgr(pauline, TRUE, TRUE, TRUE, TRUE);
	}

	BC_ASSERT_TRUE(call(marie, pauline));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);

	if (BC_ASSERT_PTR_NOT_NULL(pauline_call) && BC_ASSERT_PTR_NOT_NULL(marie_call)) {
		BC_ASSERT_TRUE(linphone_call_params_get_media_encryption(linphone_call_get_current_params(pauline_call)) ==
		               LinphoneMediaEncryptionDTLS);
		BC_ASSERT_TRUE(linphone_call_params_get_media_encryption(linphone_call_get_current_params(marie_call)) ==
		               LinphoneMediaEncryptionDTLS);
		liblinphone_tester_check_rtcp(marie, pauline);
	}

	end_call(marie, pauline);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void dtls_srtp_audio_call_with_rtcp_mux(void) {
	_dtls_srtp_audio_call_with_rtcp_mux(FALSE);
}

static void dtls_srtp_audio_call_with_rtcp_mux_not_accepted(void) {
	_dtls_srtp_audio_call_with_rtcp_mux(TRUE);
}

#ifdef VIDEO_ENABLED
void call_with_several_video_switches_base(const LinphoneMediaEncryption caller_encryption,
                                           const LinphoneMediaEncryption callee_encryption) {
	int dummy = 0;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	bool_t call_ok;

	if (linphone_core_media_encryption_supported(marie->lc, caller_encryption) &&
	    linphone_core_media_encryption_supported(marie->lc, callee_encryption)) {
		linphone_core_set_media_encryption(marie->lc, callee_encryption);
		linphone_core_set_media_encryption(pauline->lc, caller_encryption);

		BC_ASSERT_TRUE(call_ok = call(pauline, marie));
		if (!call_ok) goto end;

		liblinphone_tester_check_rtcp(marie, pauline);

		BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
		wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1000); /* Wait for VFU request exchanges to be finished. */
		BC_ASSERT_TRUE(remove_video(pauline, marie));
		BC_ASSERT_TRUE(request_video(pauline, marie, TRUE));
		wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1000); /* Wait for VFU request exchanges to be finished. */
		BC_ASSERT_TRUE(remove_video(pauline, marie));
		/**/
		end_call(pauline, marie);
	} else {
		ms_warning("Not tested because either callee doesn't support %s or caller doesn't support %s.",
		           linphone_media_encryption_to_string(callee_encryption),
		           linphone_media_encryption_to_string(caller_encryption));
	}
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void srtp_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionSRTP);
}

static void none_to_srtp_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionNone, LinphoneMediaEncryptionSRTP);
}

static void srtp_to_none_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionSRTP, LinphoneMediaEncryptionNone);
}

static void zrtp_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionZRTP);
}

static void none_to_zrtp_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionNone, LinphoneMediaEncryptionZRTP);
}

static void zrtp_to_none_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionZRTP, LinphoneMediaEncryptionNone);
}

static void dtls_srtp_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionDTLS);
}

static void none_to_dtls_srtp_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionNone, LinphoneMediaEncryptionDTLS);
}

static void dtls_srtp_to_none_call_with_several_video_switches(void) {
	call_with_several_video_switches_base(LinphoneMediaEncryptionDTLS, LinphoneMediaEncryptionNone);
}
#endif // VIDEO_ENABLED

static void call_accepting_all_encryptions(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "accept_any_encryption", 1);
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "accept_any_encryption", 1);

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_set_media_encryption(marie_params, LinphoneMediaEncryptionZRTP);

	LinphoneCallParams *pauline_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_set_media_encryption(pauline_params, LinphoneMediaEncryptionZRTP);
	BC_ASSERT_TRUE((call_with_params(marie, pauline, marie_params, pauline_params)));
	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);

	const LinphoneCallParams *params = NULL;
	params = linphone_call_get_current_params(linphone_core_get_current_call(pauline->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionZRTP, int, "%d");
	params = linphone_call_get_current_params(linphone_core_get_current_call(marie->lc));
	BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(params), LinphoneMediaEncryptionZRTP, int, "%d");

	end_call(pauline, marie);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void secure_call_with_replaces(LinphoneMediaEncryption encryption) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_media_encryption(marie->lc, encryption);
	linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);

	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_media_encryption(pauline->lc, encryption);
	linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);

	// Set Pauline to shared media resources so that she can have two calls without pause
	linphone_core_set_media_resource_mode(pauline->lc, LinphoneSharedMediaResources);

	linphone_core_invite_address(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 1));

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableFalse, 1);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_NetworkReachableTrue, 2);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingInit, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingProgress, 2));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallOutgoingRinging, 2));

	LinphoneCall *incoming_call = linphone_core_get_current_call(marie->lc);
	linphone_call_accept(incoming_call);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	liblinphone_tester_check_rtcp(marie, pauline);

	end_call(pauline, marie);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void srtp_call_with_replaces(void) {
	secure_call_with_replaces(LinphoneMediaEncryptionSRTP);
}

static void zrtp_call_with_replaces(void) {
	secure_call_with_replaces(LinphoneMediaEncryptionZRTP);
}

static void dtls_srtp_call_with_replaces(void) {
	secure_call_with_replaces(LinphoneMediaEncryptionDTLS);
}

static test_t call_secure_tests[] = {
    TEST_ONE_TAG("SRTP call", srtp_call, "CRYPTO"),
    TEST_NO_TAG("SRTP call with non zero crypto suite tag", srtp_call_non_zero_tag),
#ifdef VIDEO_ENABLED
    TEST_NO_TAG("SRTP call with several video switches", srtp_call_with_several_video_switches),
    TEST_NO_TAG("SRTP to none call with several video switches", srtp_to_none_call_with_several_video_switches),
    TEST_NO_TAG("None to SRTP call with several video switches", none_to_srtp_call_with_several_video_switches),
#endif // VIDEO_ENABLED
    TEST_NO_TAG("SRTP call with different crypto suite", srtp_call_with_different_crypto_suite),
    TEST_NO_TAG("SRTP call with crypto suite parameters", srtp_call_with_crypto_suite_parameters),
    TEST_NO_TAG("SRTP call with crypto suite parameters 2", srtp_call_with_crypto_suite_parameters_2),
    TEST_NO_TAG("SRTP call with crypto suite parameters and mandatory encryption",
                srtp_call_with_crypto_suite_parameters_and_mandatory_encryption),
    TEST_NO_TAG("SRTP call with crypto suite parameters and mandatory encryption 2",
                srtp_call_with_crypto_suite_parameters_and_mandatory_encryption_2),
    TEST_NO_TAG("SRTP call with crypto suite parameters and mandatory encryption 3",
                srtp_call_with_crypto_suite_parameters_and_mandatory_encryption_3),
    TEST_NO_TAG("SRTP call with crypto suite parameters and mandatory encryption 4",
                srtp_call_with_crypto_suite_parameters_and_mandatory_encryption_4),
    TEST_NO_TAG("SRTP call with Replaces", srtp_call_with_replaces),
    TEST_ONE_TAG("ZRTP call", zrtp_call, "CRYPTO"),
#ifdef VIDEO_ENABLED
    TEST_NO_TAG("ZRTP call with several video switches", zrtp_call_with_several_video_switches),
    TEST_NO_TAG("ZRTP to none call with several video switches", zrtp_to_none_call_with_several_video_switches),
    TEST_NO_TAG("None to ZRTP call with several video switches", none_to_zrtp_call_with_several_video_switches),
#endif // VIDEO_ENABLED
    TEST_NO_TAG("ZRTP silent call", zrtp_silent_call),
    TEST_NO_TAG("ZRTP SAS call", zrtp_sas_call),
    TEST_ONE_TAG("ZRTP Cipher call", zrtp_cipher_call, "CRYPTO"),
    TEST_ONE_TAG("ZRTP Key Agreement call", zrtp_key_agreement_call, "CRYPTO"),
    TEST_ONE_TAG("ZRTP Post Quantum Key Agreement call", zrtp_post_quantum_key_agreement_call, "PQCalls"),
    TEST_NO_TAG("ZRTP Hash call", zrtp_hash_call),
    TEST_ONE_TAG("ZRTP Authentication tag call", zrtp_authtag_call, "CRYPTO"),
    TEST_NO_TAG("ZRTP call with Replaces", zrtp_call_with_replaces),
    TEST_TWO_TAGS("DTLS SRTP call", dtls_srtp_call, "DTLS", "CRYPTO"),
#ifdef VIDEO_ENABLED
    TEST_ONE_TAG("DTLS SRTP call with several video switches", dtls_srtp_call_with_several_video_switches, "DTLS"),
    TEST_ONE_TAG("DTLS SRTP to none call with several video switches",
                 dtls_srtp_to_none_call_with_several_video_switches,
                 "DTLS"),
    TEST_ONE_TAG("None to DTLS SRTP call with several video switches",
                 none_to_dtls_srtp_call_with_several_video_switches,
                 "DTLS"),
#endif // VIDEO_ENABLED
};

static test_t call_secure2_tests[] = {
    TEST_ONE_TAG("DTLS SRTP call with ICE", dtls_srtp_call_with_ice, "DTLS"),
    TEST_ONE_TAG(
        "DTLS SRTP call with ICE and dtls start immediatly", dtls_srtp_call_with_ice_and_dtls_start_immediate, "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with media relay", dtls_srtp_call_with_media_realy, "DTLS"),
    TEST_ONE_TAG("DTLS SRTP call with Replaces", dtls_srtp_call_with_replaces, "DTLS"),
    TEST_NO_TAG("SRTP call with declined srtp", call_with_declined_srtp),
    TEST_NO_TAG("SRTP call paused and resumed", call_srtp_paused_and_resumed),
    TEST_NO_TAG("Call with ZRTP configured calling side only", call_with_zrtp_configured_calling_side),
    TEST_NO_TAG("Call with ZRTP configured receiver side only", call_with_zrtp_configured_callee_side),
    TEST_NO_TAG("Call from plain RTP to ZRTP mandatory should be silent", call_from_plain_rtp_to_zrtp),
    TEST_NO_TAG("Call ZRTP mandatory to plain RTP should be silent", call_from_zrtp_to_plain_rtp),
    TEST_NO_TAG("Recreate ZRTP db file when corrupted", recreate_zrtpdb_when_corrupted),
    TEST_NO_TAG("ZRTP cache mismatch", zrtp_cache_mismatch_test),
    TEST_NO_TAG("Check correct ZRTP short code", check_correct_zrtp_short_code_test),
    TEST_NO_TAG("Check correct ZRTP short code with latency", check_correct_zrtp_short_code_with_latency_test),
    TEST_NO_TAG("Check incorrect ZRTP short code", check_incorrect_zrtp_short_code_test),
    TEST_NO_TAG("Check incorrect ZRTP short code and pause/resume/update",
                check_incorrect_zrtp_short_code_pause_resume_update_test),
    TEST_NO_TAG("Call declined with mandatory encryption on both sides", call_declined_encryption_mandatory_both_sides),
    TEST_NO_TAG("ZRTP mandatory called by non ZRTP", zrtp_mandatory_called_by_non_zrtp),
    TEST_NO_TAG("SRTP mandatory called by non SRTP", srtp_mandatory_called_by_non_srtp),
    TEST_ONE_TAG("SRTP DTLS mandatory called by non SRTP DTLS", srtp_dtls_mandatory_called_by_non_srtp_dtls, "DTLS"),
    TEST_NO_TAG("ZRTP mandatory called by SRTP", zrtp_mandatory_called_by_srtp),
    TEST_NO_TAG("Video SRTP call without audio", video_srtp_call_without_audio),
    TEST_ONE_TAG("DTLS-SRTP call with rtcp-mux", dtls_srtp_audio_call_with_rtcp_mux, "DTLS"),
    TEST_ONE_TAG("DTLS-SRTP call with rtcp-mux not accepted", dtls_srtp_audio_call_with_rtcp_mux_not_accepted, "DTLS"),
    TEST_NO_TAG("Call accepting all encryptions", call_accepting_all_encryptions),
    TEST_ONE_TAG("EKT call", ekt_call, "CRYPTO"),
    TEST_NO_TAG("EKT call with unmatching keys", unmatching_ekt_call),
    TEST_NO_TAG("EKT call with EKT key update", updating_ekt_call)};

test_suite_t call_secure_test_suite = {"Secure Call",
                                       NULL,
                                       NULL,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(call_secure_tests) / sizeof(call_secure_tests[0]),
                                       call_secure_tests,
                                       0};

test_suite_t call_secure2_test_suite = {"Secure Call2",
                                        NULL,
                                        NULL,
                                        liblinphone_tester_before_each,
                                        liblinphone_tester_after_each,
                                        sizeof(call_secure2_tests) / sizeof(call_secure2_tests[0]),
                                        call_secure2_tests,
                                        0};
