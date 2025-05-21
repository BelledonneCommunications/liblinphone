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

#include <stdio.h>
#include <stdlib.h>

#include <bctoolbox/defs.h>
#include <bctoolbox/tester.h>
#include <bctoolbox/vfs.h>

#include "belle-sip/sipstack.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-chat-message-reaction.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-conference-cbs.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-friend.h"
#include "linphone/api/c-nat-policy.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/api/c-participant-imdn-state.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "linphone/api/c-search-result.h"
#include "linphone/core.h"
#include "linphone/logging.h"
#include "logging-private.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
#if defined(__MINGW32__) || !defined(WINAPI_FAMILY_PARTITION) || !defined(WINAPI_PARTITION_DESKTOP)
#define LINPHONE_WINDOWS_DESKTOP 1
#elif defined(WINAPI_FAMILY_PARTITION)
// See bctoolbox/include/port.h for WINAPI_PARTITION checker
#if defined(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define LINPHONE_WINDOWS_DESKTOP 1
#elif defined(WINAPI_PARTITION_PC_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PC_APP)
#define LINPHONE_WINDOWS_DESKTOP 1
#define LINPHONE_WINDOWS_UNIVERSAL 1
#define LINPHONE_WINDOWS_UWP 1
#elif defined(WINAPI_PARTITION_PHONE_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#define LINPHONE_WINDOWS_PHONE 1
#elif defined(WINAPI_PARTITION_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define LINPHONE_WINDOWS_UNIVERSAL 1
#endif
#endif
#endif
#ifdef _MSC_VER
#if (_MSC_VER >= 1900)
#define LINPHONE_MSC_VER_GREATER_19
#endif
#endif

// #define SKIP_PULSEAUDIO 1

#if _WIN32
#define unlink _unlink
#endif

#ifdef __ANDROID__
extern jobject system_context;
#else
void *system_context = 0;
#endif

static char *liblinphone_tester_empty_rc_path = NULL;
static int liblinphone_tester_keep_accounts_flag = 0;
static bool_t liblinphone_tester_keep_record_files = FALSE;
static bool_t liblinphone_tester_leak_detector_disabled = FALSE;
bool_t liblinphone_tester_keep_uuid = FALSE;
bool_t liblinphone_tester_tls_support_disabled = FALSE;
int manager_count = 0;
int leaked_objects_count = 0;
const MSAudioDiffParams audio_cmp_params = {10, 200};

/* Default test server infrastructure. You may change to sandbox infrastructure to test changes to the infrastructure
 * first. */
const char *flexisip_tester_dns_server = "fs-test-9.linphone.org";
// const char *flexisip_tester_dns_server = "fs-test-sandbox-2.linphone.org";

bctbx_list_t *flexisip_tester_dns_ip_addresses = NULL;
const char *ccmp_server_url = "http://sip.example.org:3333/xml/";
const char *test_domain = "sipopen.example.org";
const char *auth_domain = "sip.example.org";
const char *test_username = "liblinphone_tester";
const char *test_sha_username = "liblinphone_sha_tester";
const char *pure_sha256_user = "pure_sha256_user";
const char *test_password = "secret";
const char *test_route = "sip2.linphone.org";
const char *userhostsfile = "tester_hosts";
const char *file_transfer_url = "https://transfer.example.org:9444/flexisip-http-file-transfer-server/hft.php";
const char *file_transfer_url_tls_client_auth =
    "https://transfer.example.org:9445/flexisip-http-file-transfer-server/hft.php";
const char *file_transfer_url_digest_auth =
    "https://transfer.example.org:9446/flexisip-http-file-transfer-server/hft.php";
// Get proxy on the server using digest auth on sip.example.org
const char *file_transfer_get_proxy =
    "https://transfer.example.org:9446/flexisip-http-file-transfer-server/download.php";
const char *file_transfer_url_digest_auth_any_domain =
    "https://transfer.example.org:9447/flexisip-http-file-transfer-server/hft.php";
const char *file_transfer_url_small_files =
    "https://transfer.example.org:9448/flexisip-http-file-transfer-server/hft.php";
const char *file_transfer_url_digest_auth_external_domain =
    "https://transfer.external-domain.org:9544/flexisip-http-file-transfer-server/hft.php";
// Get proxy on the server using digest auth on sip.external-domain.org
const char *file_transfer_get_proxy_external_domain =
    "https://transfer.external-domain.org:9544/flexisip-http-file-transfer-server/download.php";
// These lime server authenticate user using Digest auth only on sip.example.org domain
const char *lime_server_url = "https://lime.wildcard1.linphone.org:8443/lime-server/lime-server.php";
// These lime server authenticate user using Digest auth only on any auth domain (providing the flexisip base can
// authenticate the user)
const char *lime_server_any_domain_url = "https://lime.wildcard1.linphone.org:8442/lime-server/lime-server.php";
// These lime server authenticate user using TLS auth only
const char *lime_server_tlsauth_req_url = "https://lime.wildcard1.linphone.org:8543/lime-server/lime-server.php";
// These lime server authenticate user using optionnal TLS auth, falling back on digest auth if client did not provide a
// client certificate Foreign domain external-domain.org is served on port 8643: lime_server_tlsauth_opt_url
const char *lime_server_tlsauth_opt_url = "https://lime.wildcard1.linphone.org:8544/lime-server/lime-server.php";
// Lime server using TLS and digest auth on external-domain - foreign domain is served by server on port 8544 :
const char *lime_server_external_url = "https://lime.external-domain.org:8643/lime-server/lime-server.php";
// Lime server enforcing both TLS and digest auth on sip.example.org
const char *lime_server_dual_auth_url = "https://lime.wildcard1.linphone.org:8545/lime-server/lime-server.php";
// Lime server enforcing both TLS and digest auth on external-domain
const char *lime_server_external_dual_auth_url = "https://lime.external-domain.org:8644/lime-server/lime-server.php";
bool_t liblinphonetester_ipv6 = TRUE;
const char *liblinphone_tester_ipv6_probing_address = "2a01:e00::2";
bool_t liblinphonetester_show_account_manager_logs = FALSE;
bool_t liblinphonetester_no_account_creator = FALSE;
unsigned int liblinphone_tester_max_cpu_count = 2;

const int liblinphone_tester_sip_timeout = 10000; // in ms, use this value for default SIP operation timeout
const int x3dhServer_creationTimeout = 20000;     // in ms, use this value for default lime user creation timeout
                                                  //
int liblinphonetester_transport_timeout = 9000; /*milliseconds. it is set to such low value to workaround a problem with
            our Freebox v6 when connecting to Ipv6 addresses. It was found that the freebox sometimes block SYN-ACK
            packets, which prevents connection to be successful. Thanks to the timeout, it will fallback to IPv4*/
char *message_external_body_url = NULL;
static const char *notify_content = "<somexml2>blabla</somexml2>";

const char *liblinphone_tester_mire_id = "Mire: Mire (synthetic moving picture)";
const char *liblinphone_tester_static_image_id = "StaticImage: Static picture";

static void network_reachable(LinphoneCore *lc, bool_t reachable) {
	stats *counters;
	ms_message("Network reachable [%s]", reachable ? "TRUE" : "FALSE");
	counters = get_stats(lc);
	if (reachable) counters->number_of_NetworkReachableTrue++;
	else counters->number_of_NetworkReachableFalse++;
}
void liblinphone_tester_clock_start(MSTimeSpec *start) {
	ms_get_cur_time(start);
}

bool_t liblinphone_tester_clock_elapsed(const MSTimeSpec *start, int value_ms) {
	MSTimeSpec current;
	ms_get_cur_time(&current);
	if ((((current.tv_sec - start->tv_sec) * 1000LL) + ((current.tv_nsec - start->tv_nsec) / 1000000LL)) >= value_ms)
		return TRUE;
	return FALSE;
}

LinphoneAddress *create_linphone_address(const char *domain) {
	return create_linphone_address_for_algo(domain, NULL);
}

LinphoneAddress *create_linphone_address_for_algo(const char *domain, const char *username) {
	LinphoneAddress *addr = linphone_address_new(NULL);
	if (!BC_ASSERT_PTR_NOT_NULL(addr)) return NULL;
	/* For clients who support different algorithms, their usernames must be differnet for having diffrent forms of
	 * password */
	if (username) linphone_address_set_username(addr, username);
	else linphone_address_set_username(addr, test_username);
	if (username) BC_ASSERT_STRING_EQUAL(username, linphone_address_get_username(addr));
	else BC_ASSERT_STRING_EQUAL(test_username, linphone_address_get_username(addr));
	if (!domain) domain = test_route;
	linphone_address_set_domain(addr, domain);
	BC_ASSERT_STRING_EQUAL(domain, linphone_address_get_domain(addr));
	linphone_address_set_display_name(addr, NULL);
	linphone_address_set_display_name(addr, "Mr Tester");
	BC_ASSERT_STRING_EQUAL("Mr Tester", linphone_address_get_display_name(addr));
	return addr;
}

static void
auth_info_requested(LinphoneCore *lc, const char *realm, const char *username, BCTBX_UNUSED(const char *domain)) {
	stats *counters;
	ms_message("Auth info requested (deprecated callback) for user id [%s] at realm [%s]\n", username, realm);
	counters = get_stats(lc);
	counters->number_of_auth_info_requested++;
}

static void authentication_info_requested(LinphoneCore *lc,
                                          BCTBX_UNUSED(LinphoneAuthInfo *info),
                                          BCTBX_UNUSED(LinphoneAuthMethod method)) {
	stats *counters;
	counters = get_stats(lc);
	counters->number_of_authentication_info_requested++;
}

void reset_counters(stats *counters) {
	if (counters->last_received_chat_message) linphone_chat_message_unref(counters->last_received_chat_message);
	if (counters->last_fail_to_decrypt_received_chat_message)
		linphone_chat_message_unref(counters->last_fail_to_decrypt_received_chat_message);
	if (counters->last_received_info_message) linphone_info_message_unref(counters->last_received_info_message);
	if (counters->dtmf_list_received) bctbx_free(counters->dtmf_list_received);

	memset(counters, 0, sizeof(stats));
}

static void setup_dns(LinphoneCore *lc, const char *path) {
	belle_sip_stack_set_dns_engine(
	    sal_get_stack_impl(linphone_core_get_sal(lc)),
	    BELLE_SIP_DNS_DNS_C); // Make sure we are not using Apple DNS Service during liblinphone tests
	if (flexisip_tester_dns_ip_addresses) {
		linphone_core_set_dns_servers(lc, flexisip_tester_dns_ip_addresses);
	} else if (strcmp(userhostsfile, "none") != 0) {
		char *dnsuserhostspath =
		    strchr(userhostsfile, '/') ? ms_strdup(userhostsfile) : ms_strdup_printf("%s/%s", path, userhostsfile);
		sal_set_dns_user_hosts_file(linphone_core_get_sal(lc), dnsuserhostspath);
		ms_free(dnsuserhostspath);
	} else {
		bctbx_warning("No dns-hosts file and no flexisip-tester dns server used.");
	}
}

void configure_lc(LinphoneCore *lc, const char *path, void *user_data) {
	linphone_core_enable_ipv6(lc, liblinphonetester_ipv6);
	linphone_core_set_sip_transport_timeout(lc, liblinphonetester_transport_timeout);
	linphone_core_set_user_data(lc, user_data);

	sal_enable_test_features(linphone_core_get_sal(lc), TRUE);
	setup_dns(lc, path);
}

LinphoneCore *configure_lc_from(LinphoneCoreCbs *cbs, const char *path, LinphoneConfig *config, void *user_data) {
	LinphoneCore *lc;
	char *ringpath = NULL;
	char *ringbackpath = NULL;
	char *rootcapath = NULL;
	char *nowebcampath = NULL;

	// setup dynamic-path assets
	ringpath = ms_strdup_printf("%s/sounds/oldphone.wav", path);
	ringbackpath = ms_strdup_printf("%s/sounds/ringback.wav", path);
	nowebcampath = ms_strdup_printf("%s/images/nowebcamCIF.jpg", path);
	rootcapath = ms_strdup_printf("%s/certificates/cn/cafile.pem", path);

	if (config) {
		linphone_config_set_string(config, "sound", "remote_ring", ringbackpath);
		linphone_config_set_string(config, "sound", "local_ring", ringpath);
		linphone_config_set_string(config, "sip", "root_ca", rootcapath);
		/* Disable the use of fsync() by sqlite3, which degrades performance for no benefit in the context of tests. */
		linphone_config_set_int(config, "misc", "sqlite3_synchronous", 0);

		LinphoneCoreManager *mgr = (LinphoneCoreManager *)user_data;
		if (mgr && mgr->app_group_id) {
			lc = linphone_factory_create_shared_core_with_config(linphone_factory_get(), config, system_context,
			                                                     mgr->app_group_id, mgr->main_core);
		} else {
			lc = linphone_factory_create_core_with_config_3(linphone_factory_get(), config, system_context);
		}
	} else {
		lc = linphone_factory_create_core_3(linphone_factory_get(), NULL, liblinphone_tester_get_empty_rc(),
		                                    system_context);
		linphone_core_set_ring(lc, ringpath);
		linphone_core_set_ringback(lc, ringbackpath);
		linphone_core_set_root_ca(lc, rootcapath);
	}
	if (cbs) linphone_core_add_callbacks(lc, cbs);
#ifdef VIDEO_ENABLED
	linphone_core_set_static_picture(lc, nowebcampath);
#endif
	configure_lc(lc, path, user_data);

	ms_free(ringpath);
	ms_free(ringbackpath);
	ms_free(nowebcampath);
	ms_free(rootcapath);
	return lc;
}

bool_t wait_for_until_interval(LinphoneCore *lc_1, LinphoneCore *lc_2, int *counter, int min, int max, int timeout) {
	bctbx_list_t *lcs = NULL;
	bool_t result;
	if (lc_1) lcs = bctbx_list_append(lcs, lc_1);
	if (lc_2) lcs = bctbx_list_append(lcs, lc_2);
	result = wait_for_list_interval(lcs, counter, min, max, timeout);
	bctbx_list_free(lcs);
	return result;
}

bool_t wait_for_until(LinphoneCore *lc_1, LinphoneCore *lc_2, const int *counter, int value, int timeout) {
	bctbx_list_t *lcs = NULL;
	bool_t result;
	if (lc_1) lcs = bctbx_list_append(lcs, lc_1);
	if (lc_2) lcs = bctbx_list_append(lcs, lc_2);
	result = wait_for_list(lcs, counter, value, timeout);
	bctbx_list_free(lcs);
	return result;
}

bool_t wait_for_until_for_uint64(
    LinphoneCore *lc_1, LinphoneCore *lc_2, const uint64_t *counter, uint64_t value, int timeout) {
	bctbx_list_t *lcs = NULL;
	bool_t result;
	if (lc_1) lcs = bctbx_list_append(lcs, lc_1);
	if (lc_2) lcs = bctbx_list_append(lcs, lc_2);
	result = wait_for_list_for_uint64(lcs, counter, value, timeout);
	bctbx_list_free(lcs);
	return result;
}

bool_t wait_for(LinphoneCore *lc_1, LinphoneCore *lc_2, int *counter, int value) {
	return wait_for_until(lc_1, lc_2, counter, value, liblinphone_tester_sip_timeout);
}

bool_t wait_for_list_interval(bctbx_list_t *lcs, int *counter, int min, int max, int timeout_ms) {
	bctbx_list_t *iterator;
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while ((counter == NULL || *counter < min || *counter > max) &&
	       !liblinphone_tester_clock_elapsed(&start, timeout_ms)) {
		for (iterator = lcs; iterator != NULL; iterator = iterator->next) {
			linphone_core_iterate((LinphoneCore *)(iterator->data));
		}
		bc_tester_process_events();
#if !defined(LINPHONE_WINDOWS_UWP) && defined(LINPHONE_WINDOWS_DESKTOP)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, 1)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#endif
		ms_usleep(20000);
	}
	if (counter && (*counter < min || *counter > max)) return FALSE;
	else return TRUE;
}

bool_t wait_for_list(bctbx_list_t *lcs, const int *counter, int value, int timeout_ms) {
	bctbx_list_t *iterator;
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while ((counter == NULL || *counter < value) && !liblinphone_tester_clock_elapsed(&start, timeout_ms)) {
		for (iterator = lcs; iterator != NULL; iterator = iterator->next) {
			linphone_core_iterate((LinphoneCore *)(iterator->data));
		}
#ifdef LINPHONE_WINDOWS_UWP
		{
			bc_tester_process_events();
		}
#elif defined(LINPHONE_WINDOWS_DESKTOP)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, 1)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#endif
		ms_usleep(20000);
	}
	if (counter && *counter < value) return FALSE;
	else return TRUE;
}

bool_t wait_for_list_for_uint64(bctbx_list_t *lcs, const uint64_t *counter, uint64_t value, int timeout_ms) {
	bctbx_list_t *iterator;
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while ((counter == NULL || *counter < value) && !liblinphone_tester_clock_elapsed(&start, timeout_ms)) {
		for (iterator = lcs; iterator != NULL; iterator = iterator->next) {
			linphone_core_iterate((LinphoneCore *)(iterator->data));
		}
#ifdef LINPHONE_WINDOWS_UWP
		{
			bc_tester_process_events();
		}
#elif defined(LINPHONE_WINDOWS_DESKTOP)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, 1)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#endif
		ms_usleep(20000);
	}
	if (counter && *counter < value) return FALSE;
	else return TRUE;
}

static void enable_codec(LinphoneCore *lc, const char *type, int rate) {
	bctbx_list_t *codecs = bctbx_list_copy(linphone_core_get_audio_codecs(lc));
	bctbx_list_t *codecs_it;
	PayloadType *pt;
	for (codecs_it = codecs; codecs_it != NULL; codecs_it = codecs_it->next) {
		linphone_core_enable_payload_type(lc, (PayloadType *)codecs_it->data, 0);
	}
	if ((pt = linphone_core_find_payload_type(lc, type, rate, 1))) {
		linphone_core_enable_payload_type(lc, pt, TRUE);
	}
	bctbx_list_free(codecs);
}

stats *get_stats(LinphoneCore *lc) {
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(lc);
	return &manager->stat;
}

LinphoneCoreManager *get_manager(LinphoneCore *lc) {
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(lc);
	return manager;
}

bool_t transport_supported(LinphoneTransportType transport) {
	if ((transport == LinphoneTransportDtls || transport == LinphoneTransportTls) &&
	    liblinphone_tester_tls_support_disabled == TRUE) {
		return FALSE;
	} else {
		Sal *sal = sal_init(NULL);
		bool_t supported = sal_transport_available(sal, (SalTransport)transport);
		if (!supported)
			ms_message("TLS transport not supported, falling back to TCP if possible otherwise skipping test.");
		sal_uninit(sal);
		return supported;
	}
}

#ifdef SKIP_PULSEAUDIO
static void avoid_pulseaudio_hack(LinphoneCoreManager *mgr) {
	bctbx_list_t *cards = linphone_core_get_sound_devices_list(mgr->lc);
	bctbx_list_t *it;
	bool_t capture_set = FALSE, playback_set = FALSE;
	bool_t pulseaudio_found = FALSE;
	for (it = cards; it != NULL; it = it->next) {
		const char *card_id = (const char *)it->data;
		if (strstr(card_id, "PulseAudio") != NULL) {
			pulseaudio_found = TRUE;
			continue;
		}
		if (!capture_set && linphone_core_sound_device_can_capture(mgr->lc, card_id)) {
			capture_set = TRUE;
			linphone_core_set_capture_device(mgr->lc, card_id);
		}
		if (!playback_set && linphone_core_sound_device_can_playback(mgr->lc, card_id)) {
			playback_set = TRUE;
			linphone_core_set_playback_device(mgr->lc, card_id);
			linphone_core_set_ringer_device(mgr->lc, card_id);
		}
		if (playback_set && capture_set) {
			if (pulseaudio_found)
				ms_warning("PulseAudio is not used in liblinphone_tester because of internal random crashes or hangs.");
			break;
		}
	}
	if (!playback_set || !capture_set) {
		ms_error("Could not find soundcard other than pulseaudio to use during tests. Some tests may crash or hang.");
	}
	bctbx_list_free(cards);
}
#endif

void linphone_core_manager_setup_dns(LinphoneCoreManager *mgr) {
	setup_dns(mgr->lc, bc_tester_get_resource_dir_prefix());
}

LinphoneCore *linphone_core_manager_configure_lc(LinphoneCoreManager *mgr) {
	LinphoneCore *lc;
	char *filepath =
	    mgr->rc_path ? bctbx_strdup_printf("%s/%s", bc_tester_get_resource_dir_prefix(), mgr->rc_path) : NULL;
	if (filepath && bctbx_file_exist(filepath) != 0) {
		ms_fatal("Could not find file %s in path %s, did you configured resources directory correctly?", mgr->rc_path,
		         bc_tester_get_resource_dir_prefix());
	}

	if (mgr->rc_path) {
		if (mgr->rc_local == NULL) {
			char random_id[8];
			belle_sip_random_token(random_id, sizeof random_id);
			char *bn = bctbx_basename(mgr->rc_path);
			mgr->rc_local = bctbx_strdup_printf("%s/%s_%s", bc_tester_get_writable_dir_prefix(), bn, random_id);
			bctbx_free(bn);
		}
		bctbx_vfs_file_t *in = bctbx_file_open(bctbx_vfs_get_default(), filepath, "r");
		bctbx_vfs_file_t *out = bctbx_file_open2(bctbx_vfs_get_default(), mgr->rc_local, O_WRONLY | O_CREAT | O_TRUNC);
		size_t in_size = (size_t)bctbx_file_size(in);
		uint8_t *buf = bctbx_malloc(in_size);
		bctbx_file_read(in, buf, in_size, 0);
		bctbx_file_write(out, buf, in_size, 0);
		bctbx_file_close(in);
		bctbx_file_close(out);
		bctbx_free(buf);
	}

	LinphoneConfig *config = linphone_factory_create_config_with_factory(linphone_factory_get(), mgr->rc_local, NULL);
	linphone_config_set_string(config, "auth_info_0", "userid", "");

	linphone_config_set_string(config, "storage", "backend", "sqlite3");
	linphone_config_set_string(config, "storage", "uri", mgr->database_path);
	linphone_config_set_string(config, "storage", "zrtp_secrets_db_uri", mgr->zrtp_secrets_database_path);
	linphone_config_set_string(config, "lime", "x3dh_db_path", mgr->lime_database_path);
	linphone_config_set_bool(config, "misc", "auto_iterate", FALSE);
	lc = configure_lc_from(mgr->cbs, bc_tester_get_resource_dir_prefix(), config, mgr);

	linphone_config_unref(config);
	if (filepath) bctbx_free(filepath);
	return lc;
}

void linphone_core_manager_configure(LinphoneCoreManager *mgr) {
	LinphoneImNotifPolicy *im_notif_policy;
	char *hellopath = bc_tester_res("sounds/hello8000.wav");
	char *filepath =
	    mgr->rc_path ? bctbx_strdup_printf("%s/%s", bc_tester_get_resource_dir_prefix(), mgr->rc_path) : NULL;
	if (filepath && bctbx_file_exist(filepath) != 0) {
		ms_fatal("Could not find file %s in path %s, did you configured resources directory correctly?", mgr->rc_path,
		         bc_tester_get_resource_dir_prefix());
	}

	mgr->lc = linphone_core_manager_configure_lc(mgr);

	linphone_core_manager_check_accounts(mgr);
	im_notif_policy = linphone_core_get_im_notif_policy(mgr->lc);
	if (im_notif_policy != NULL) {
		/* The IM notification policy can be NULL at this point in case of remote provisioning. */
		linphone_im_notif_policy_clear(im_notif_policy);
		linphone_im_notif_policy_set_send_is_composing(im_notif_policy, TRUE);
		linphone_im_notif_policy_set_recv_is_composing(im_notif_policy, TRUE);
	}

#if TARGET_OS_IPHONE
	linphone_core_set_ringer_device(mgr->lc, "AQ: Audio Queue Device");
	linphone_core_set_ringback(mgr->lc, NULL);
#elif __QNX__
	linphone_core_set_playback_device(mgr->lc, "QSA: voice");
#elif defined(SKIP_PULSEAUDIO)
	{
		/* Special trick for linux. Pulseaudio has random hangs, deadlocks or abort while executing test suites.
		 * It never happens in the linphone app.
		 * So far we could not identify something bad in our pulseaudio usage. As a workaround, we disable pulseaudio
		 * for the tests.*/
		avoid_pulseaudio_hack(mgr);
	}
#endif

#ifdef VIDEO_ENABLED
	{
		MSWebCam *cam;

		cam = ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(mgr->lc)),
		                                 liblinphone_tester_mire_id);

// Usefull especially for encoders not supporting qcif
#ifdef __ANDROID__
		MSVideoSize vsize = MS_VIDEO_SIZE_CIF;
		linphone_core_set_preferred_video_size(mgr->lc, vsize);
#endif

		if (cam == NULL) {
			MSWebCamDesc *desc = ms_mire_webcam_desc_get();
			if (desc) {
				cam = ms_web_cam_new(desc);
				ms_web_cam_manager_add_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(mgr->lc)), cam);
			}
		}
	}
#endif

	linphone_core_set_play_file(mgr->lc, hellopath); /*is also used when in pause*/
	ms_free(hellopath);

	if (manager_count >= 2) {
		char *recordpath = ms_strdup_printf("%s/record_for_lc_%p.wav", bc_tester_get_writable_dir_prefix(), mgr->lc);
		ms_message("Manager for '%s' using files", mgr->rc_path ? mgr->rc_path : "--");
		linphone_core_set_use_files(mgr->lc, TRUE);
		linphone_core_set_record_file(mgr->lc, recordpath);
		ms_free(recordpath);
	}

	linphone_core_set_user_certificates_path(mgr->lc, bc_tester_get_writable_dir_prefix());
	/*for now, we need the periodical updates facility to compute bandwidth measurements correctly during tests*/
	linphone_core_enable_send_call_stats_periodical_updates(mgr->lc, TRUE);
	// limit cpu core to 2 to facilitate parallel runs.
	ms_factory_set_cpu_count(linphone_core_get_ms_factory(mgr->lc), liblinphone_tester_max_cpu_count);

	// clean
	if (filepath) bctbx_free(filepath);
}

static void generate_random_database_path(LinphoneCoreManager *mgr) {
	char random_id[32];
	belle_sip_random_token(random_id, sizeof random_id);
	char *database_path_format = bctbx_strdup_printf("linphone_%s.db", random_id);
	mgr->database_path = bc_tester_file(database_path_format);
	bctbx_free(database_path_format);
	database_path_format = bctbx_strdup_printf("lime_%s.db", random_id);
	mgr->lime_database_path = bc_tester_file(database_path_format);
	bctbx_free(database_path_format);
	char *zrtp_secrets_database_path_format = bctbx_strdup_printf("zrtp-secrets-%s.db", random_id);
	mgr->zrtp_secrets_database_path = bc_tester_file(zrtp_secrets_database_path_format);
	bctbx_free(zrtp_secrets_database_path_format);
}

static void participant_device_state_changed(LinphoneParticipantDevice *device,
                                             const LinphoneParticipantDeviceState state) {
	LinphoneCore *core = linphone_participant_device_get_core(device);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	switch (state) {
		case LinphoneParticipantDeviceStateJoining:
			manager->stat.number_of_participant_devices_pending++;
			break;
		case LinphoneParticipantDeviceStateScheduledForJoining:
			manager->stat.number_of_participant_devices_scheduled_for_joining++;
			break;
		case LinphoneParticipantDeviceStatePresent:
			manager->stat.number_of_participant_devices_present++;
			break;
		case LinphoneParticipantDeviceStateOnHold:
			manager->stat.number_of_participant_devices_on_hold++;
			break;
		case LinphoneParticipantDeviceStateRequestingToJoin:
			manager->stat.number_of_participant_devices_requesting_to_join++;
			break;
		case LinphoneParticipantDeviceStateAlerting:
			manager->stat.number_of_participant_devices_alerting++;
			break;
		case LinphoneParticipantDeviceStateScheduledForLeaving:
			manager->stat.number_of_participant_devices_scheduled_for_leaving++;
			break;
		case LinphoneParticipantDeviceStateLeaving:
			manager->stat.number_of_participant_devices_leaving++;
			break;
		case LinphoneParticipantDeviceStateLeft:
			manager->stat.number_of_participant_devices_left++;
			break;
		case LinphoneParticipantDeviceStateMutedByFocus:
			manager->stat.number_of_participant_devices_muted_by_focus++;
			break;
	}
}

static void conference_state_changed(LinphoneConference *conference, LinphoneConferenceState newState) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	const LinphoneAddress *address = linphone_conference_get_conference_address(conference);
	char *address_str = NULL;

	if (address) {
		address_str = linphone_address_as_string(address);
	} else {
		address_str = ms_strdup("sip:");
	}
	char *newStateStr = linphone_conference_state_to_string(newState);
	ms_message("Conference %p [%s] state changed: %s", conference, address_str, newStateStr);
	ms_free(newStateStr);

	if ((newState != LinphoneConferenceStateNone) && (newState != LinphoneConferenceStateInstantiated) &&
	    (newState != LinphoneConferenceStateCreationPending) && !linphone_core_conference_server_enabled(core)) {
		LinphoneParticipant *me = linphone_conference_get_me(conference);
		BC_ASSERT_PTR_NOT_NULL(me);
	}

	switch (newState) {
		case LinphoneConferenceStateNone:
			break;
		case LinphoneConferenceStateInstantiated:
			manager->stat.number_of_LinphoneConferenceStateInstantiated++;
			break;
		case LinphoneConferenceStateCreationPending:
			manager->stat.number_of_LinphoneConferenceStateCreationPending++;
			break;
		case LinphoneConferenceStateCreated:
			manager->stat.number_of_LinphoneConferenceStateCreated++;
			break;
		case LinphoneConferenceStateCreationFailed:
			manager->stat.number_of_LinphoneConferenceStateCreationFailed++;
			break;
		case LinphoneConferenceStateTerminationPending:
			manager->stat.number_of_LinphoneConferenceStateTerminationPending++;
			break;
		case LinphoneConferenceStateTerminated:
			manager->stat.number_of_LinphoneConferenceStateTerminated++;
			break;
		case LinphoneConferenceStateTerminationFailed:
			manager->stat.number_of_LinphoneConferenceStateTerminationFailed++;
			break;
		case LinphoneConferenceStateDeleted:
			manager->stat.number_of_LinphoneConferenceStateDeleted++;
			break;
	}

	if (address_str) {
		bctbx_free(address_str);
	}
}

static void conference_participant_device_state_changed(LinphoneConference *conference,
                                                        BCTBX_UNUSED(const LinphoneParticipantDevice *device),
                                                        const LinphoneParticipantDeviceState state) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	switch (state) {
		case LinphoneParticipantDeviceStateJoining:
			manager->stat.number_of_conference_participant_devices_pending++;
			break;
		case LinphoneParticipantDeviceStateScheduledForJoining:
			manager->stat.number_of_conference_participant_devices_scheduled_for_joining++;
			break;
		case LinphoneParticipantDeviceStatePresent:
			manager->stat.number_of_conference_participant_devices_present++;
			break;
		case LinphoneParticipantDeviceStateOnHold:
			manager->stat.number_of_conference_participant_devices_on_hold++;
			break;
		case LinphoneParticipantDeviceStateRequestingToJoin:
			manager->stat.number_of_conference_participant_devices_requesting_to_join++;
			break;
		case LinphoneParticipantDeviceStateAlerting:
			manager->stat.number_of_conference_participant_devices_alerting++;
			break;
		case LinphoneParticipantDeviceStateScheduledForLeaving:
			manager->stat.number_of_conference_participant_devices_scheduled_for_leaving++;
			break;
		case LinphoneParticipantDeviceStateLeaving:
			manager->stat.number_of_conference_participant_devices_leaving++;
			break;
		case LinphoneParticipantDeviceStateLeft:
			manager->stat.number_of_conference_participant_devices_left++;
			break;
		case LinphoneParticipantDeviceStateMutedByFocus:
			manager->stat.number_of_conference_participant_devices_muted_by_focus++;
			break;
	}
}

static void conference_available_media_changed(LinphoneConference *conference) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_available_media_changed++;
}

static void
conference_participant_device_media_capability_changed(LinphoneConference *conference,
                                                       BCTBX_UNUSED(const LinphoneParticipantDevice *device)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_devices_media_capability_changed++;
}

static void conference_participant_role_changed(LinphoneConference *conference,
                                                BCTBX_UNUSED(const LinphoneParticipant *participant)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_role_changed++;
}

static void conference_participant_admin_status_changed(LinphoneConference *conference,
                                                        BCTBX_UNUSED(const LinphoneParticipant *participant)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_admin_statuses_changed++;
}

static void conference_subject_changed(LinphoneConference *conference, BCTBX_UNUSED(const char *subject)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_subject_changed++;
}

static void
conference_active_speaker_participant_device_changed(LinphoneConference *conference,
                                                     BCTBX_UNUSED(const LinphoneParticipantDevice *device)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_active_speaker_participant_device_changed++;
}

static void conference_allowed_participant_list_changed(LinphoneConference *conference) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_allowed_participant_list_changed++;
}

static void conference_participant_added(LinphoneConference *conference,
                                         BCTBX_UNUSED(LinphoneParticipant *participant)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participants_added++;
}
static void conference_participant_removed(LinphoneConference *conference,
                                           BCTBX_UNUSED(const LinphoneParticipant *participant)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participants_removed++;
}
static void conference_participant_device_added(LinphoneConference *conference,
                                                LinphoneParticipantDevice *participant_device) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_devices_added++;

	LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
	linphone_participant_device_cbs_set_state_changed(cbs, participant_device_state_changed);
	linphone_participant_device_add_callbacks(participant_device, cbs);
	linphone_participant_device_cbs_unref(cbs);
}
static void conference_participant_device_removed(LinphoneConference *conference,
                                                  BCTBX_UNUSED(const LinphoneParticipantDevice *participant_device)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_devices_removed++;
}
static void conference_participant_device_joining_request(LinphoneConference *conference,
                                                          BCTBX_UNUSED(LinphoneParticipantDevice *participant_device)) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_devices_joining_request++;
}
static void conference_full_state_received(LinphoneConference *conference) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_conference_full_state_received++;

	bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
	for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
		LinphoneParticipantDevice *participant_device = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
		LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
		linphone_participant_device_cbs_set_state_changed(cbs, participant_device_state_changed);
		linphone_participant_device_add_callbacks(participant_device, cbs);
		linphone_participant_device_cbs_unref(cbs);
	}
	if (devices) {
		bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
	}

	// linphone_conference_get_participant_device_list() doesn't add the me participant if it is not in the conférence.
	// This might happen when the client call is in state Updating
	if (!linphone_conference_is_in(conference)) {
		devices = linphone_participant_get_devices(linphone_conference_get_me(conference));
		for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
			LinphoneParticipantDevice *participant_device = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
			LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
			linphone_participant_device_cbs_set_state_changed(cbs, participant_device_state_changed);
			linphone_participant_device_add_callbacks(participant_device, cbs);
			linphone_participant_device_cbs_unref(cbs);
		}
		if (devices) {
			bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
		}
	}
}

static void conference_participant_device_screen_sharing_changed(
    LinphoneConference *conference, BCTBX_UNUSED(const LinphoneParticipantDevice *participant_device), bool_t enabled) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	if (enabled) {
		manager->stat.number_of_participant_devices_screen_sharing_enabled++;
	} else {
		manager->stat.number_of_participant_devices_screen_sharing_disabled++;
	}
}

static void create_conference_cb(LinphoneConference *conference) {
	LinphoneConferenceCbs *cbs = linphone_factory_create_conference_cbs(linphone_factory_get());
	linphone_conference_cbs_set_state_changed(cbs, conference_state_changed);
	linphone_conference_cbs_set_available_media_changed(cbs, conference_available_media_changed);
	linphone_conference_cbs_set_subject_changed(cbs, conference_subject_changed);
	linphone_conference_cbs_set_participant_role_changed(cbs, conference_participant_role_changed);
	linphone_conference_cbs_set_participant_admin_status_changed(cbs, conference_participant_admin_status_changed);
	linphone_conference_cbs_set_participant_device_media_capability_changed(
	    cbs, conference_participant_device_media_capability_changed);
	linphone_conference_cbs_set_participant_device_state_changed(cbs, conference_participant_device_state_changed);
	linphone_conference_cbs_set_participant_added(cbs, conference_participant_added);
	linphone_conference_cbs_set_participant_device_added(cbs, conference_participant_device_added);
	linphone_conference_cbs_set_participant_removed(cbs, conference_participant_removed);
	linphone_conference_cbs_set_participant_device_removed(cbs, conference_participant_device_removed);
	linphone_conference_cbs_set_participant_device_joining_request(cbs, conference_participant_device_joining_request);
	linphone_conference_cbs_set_participant_device_screen_sharing_changed(
	    cbs, conference_participant_device_screen_sharing_changed);
	linphone_conference_cbs_set_full_state_received(cbs, conference_full_state_received);
	linphone_conference_cbs_set_allowed_participant_list_changed(cbs, conference_allowed_participant_list_changed);
	linphone_conference_cbs_set_active_speaker_participant_device(cbs,
	                                                              conference_active_speaker_participant_device_changed);
	linphone_conference_add_callbacks(conference, cbs);
	linphone_conference_cbs_unref(cbs);
}

void core_conference_state_changed(BCTBX_UNUSED(LinphoneCore *core),
                                   LinphoneConference *conference,
                                   LinphoneConferenceState state) {
	if (state == LinphoneConferenceStateInstantiated) {
		create_conference_cb(conference);
	}
}

void default_account_changed(LinphoneCore *core, LinphoneAccount *account) {
	BC_ASSERT_PTR_EQUAL(linphone_core_get_default_account(core), account);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneDefaultAccountChanged++;
}

void account_added(LinphoneCore *core, LinphoneAccount *account) {
	BC_ASSERT_PTR_NOT_NULL(account);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneAccountAdded++;
}

void account_removed(LinphoneCore *core, LinphoneAccount *account) {
	BC_ASSERT_PTR_NOT_NULL(account);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneAccountRemoved++;
}

LinphoneStatus add_participant_to_local_conference_through_invite(bctbx_list_t *lcs,
                                                                  LinphoneCoreManager *conf_mgr,
                                                                  bctbx_list_t *participants,
                                                                  const LinphoneCallParams *params) {

	stats conf_initial_stats = conf_mgr->stat;
	stats *participants_initial_stats = NULL;
	bool_t *existing_call = NULL;
	bctbx_list_t *participant_addresses = NULL;
	int counter = 1;

	LinphoneConference *local_conference = linphone_core_get_conference(conf_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(local_conference);
	const LinphoneAddress *local_conference_address = linphone_conference_get_conference_address(local_conference);

	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		participant_addresses = bctbx_list_append(participant_addresses, m->identity);

		// Allocate memory
		existing_call = (bool_t *)realloc(existing_call, counter * sizeof(bool_t));
		participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
		// Append element
		participants_initial_stats[counter - 1] = m->stat;
		LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
		existing_call[counter - 1] = (conf_call != NULL);
		// Increment counter
		counter++;

		LinphoneAddress *participant_uri = linphone_address_new(linphone_core_get_identity(m->lc));
		LinphoneConference *participant_conference =
		    linphone_core_search_conference(m->lc, NULL, participant_uri, local_conference_address, NULL);
		linphone_address_unref(participant_uri);
		BC_ASSERT_PTR_NULL(participant_conference);
	}

	LinphoneStatus status = linphone_conference_invite_participants(local_conference, participant_addresses, params);

	if (participants != NULL) {
		int idx = 0;
		int new_call_cnt = 0;
		for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(itm);
			if (existing_call[idx] == FALSE) {
				new_call_cnt++;
				BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallOutgoingProgress,
				                             conf_initial_stats.number_of_LinphoneCallOutgoingProgress + new_call_cnt,
				                             2000));
				if (linphone_core_is_network_reachable(m->lc)) {
					BC_ASSERT_TRUE(
					    wait_for_list(lcs, &m->stat.number_of_LinphoneCallIncomingReceived,
					                  (participants_initial_stats[idx].number_of_LinphoneCallIncomingReceived + 1),
					                  liblinphone_tester_sip_timeout));
				}
			}
			idx++;
		}
	}

	if (participants_initial_stats) {
		ms_free(participants_initial_stats);
	}

	if (existing_call) {
		ms_free(existing_call);
	}

	if (participant_addresses) {
		bctbx_list_free(participant_addresses);
	}

	return status;
}

static void check_participant_media_direction(const LinphoneMediaDirection local,
                                              const LinphoneMediaDirection remote,
                                              bool_t enabled) {
	if (enabled) {
		switch (local) {
			case LinphoneMediaDirectionSendOnly:
			case LinphoneMediaDirectionRecvOnly:
			case LinphoneMediaDirectionInactive:
			case LinphoneMediaDirectionSendRecv:
				BC_ASSERT_EQUAL(remote, local, int, "%0d");
				break;
			case LinphoneMediaDirectionInvalid:
				BC_ASSERT_EQUAL(remote, LinphoneMediaDirectionInactive, int, "%0d");
				break;
		}
	} else {
		switch (local) {
			case LinphoneMediaDirectionSendRecv:
			case LinphoneMediaDirectionRecvOnly:
			case LinphoneMediaDirectionSendOnly:
				BC_ASSERT_EQUAL(remote, LinphoneMediaDirectionRecvOnly, int, "%0d");
				break;
			case LinphoneMediaDirectionInactive:
			case LinphoneMediaDirectionInvalid:
				BC_ASSERT_EQUAL(remote, LinphoneMediaDirectionInactive, int, "%0d");
				break;
		}
	}
}

void check_conference_medias(LinphoneConference *local_conference, LinphoneConference *remote_conference) {
	BC_ASSERT_PTR_NOT_NULL(local_conference);
	BC_ASSERT_PTR_NOT_NULL(remote_conference);
	if (local_conference && remote_conference) {
		BC_ASSERT_TRUE(check_conference_ssrc(local_conference, remote_conference));
		bctbx_list_t *local_conference_participants = linphone_conference_get_participant_list(local_conference);

		LinphoneParticipant *remote_conference_me = linphone_conference_get_me(remote_conference);
		const LinphoneAddress *remote_me_address = linphone_participant_get_address(remote_conference_me);
		for (bctbx_list_t *itp = local_conference_participants; itp; itp = bctbx_list_next(itp)) {
			LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
			const LinphoneAddress *p_address = linphone_participant_get_address(p);
			bctbx_list_t *local_devices = NULL;
			if (linphone_address_equal(p_address, remote_me_address)) {
				const bool_t remote_is_in = linphone_conference_is_in(remote_conference);
				const LinphoneConferenceParams *remote_params =
				    linphone_conference_get_current_params(remote_conference);
				LinphoneMediaDirection audio_direction = LinphoneMediaDirectionInactive;
				LinphoneMediaDirection video_direction = LinphoneMediaDirectionInactive;
				LinphoneMediaDirection text_direction = LinphoneMediaDirectionInactive;

				local_devices = linphone_participant_get_devices(p);
				for (bctbx_list_t *itd = local_devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if ((remote_is_in == FALSE) ||
					    ((!!linphone_conference_params_audio_enabled(remote_params)) == FALSE)) {
						audio_direction = LinphoneMediaDirectionInactive;
					} else if (_linphone_participant_device_get_audio_enabled(d) == TRUE) {
						audio_direction = LinphoneMediaDirectionSendRecv;
					} else {
						audio_direction = LinphoneMediaDirectionRecvOnly;
					}

					if ((remote_is_in == FALSE) ||
					    ((!!linphone_conference_params_video_enabled(remote_params)) == FALSE)) {
						video_direction = LinphoneMediaDirectionInactive;
					} else if (_linphone_participant_device_get_video_enabled(d) == TRUE) {
						video_direction = LinphoneMediaDirectionSendRecv;
					} else {
						video_direction = LinphoneMediaDirectionInactive;
					}

					if ((remote_is_in == FALSE) ||
					    (!!linphone_conference_params_chat_enabled(remote_params) == FALSE)) {
						text_direction = LinphoneMediaDirectionInactive;
					} else if (_linphone_participant_device_get_real_time_text_enabled(d) == TRUE) {
						text_direction = LinphoneMediaDirectionSendRecv;
					} else {
						text_direction = LinphoneMediaDirectionRecvOnly;
					}

					BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeAudio),
					                audio_direction, int, "%0d");
					BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo),
					                video_direction, int, "%0d");
					BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeText),
					                text_direction, int, "%0d");

					BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeAudio));
					BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
					               (video_direction == LinphoneMediaDirectionSendRecv));
				}
			} else {
				LinphoneParticipant *remote_participant =
				    linphone_conference_find_participant(remote_conference, p_address);
				BC_ASSERT_PTR_NOT_NULL(remote_participant);
				if (remote_participant) {
					local_devices = linphone_participant_get_devices(p);
					for (bctbx_list_t *itd = local_devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						LinphoneParticipantDevice *remote_device = linphone_participant_find_device(
						    remote_participant, linphone_participant_device_get_address(d));
						BC_ASSERT_PTR_NOT_NULL(remote_device);
						if (remote_device) {
							check_participant_media_direction(
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeAudio),
							    linphone_participant_device_get_stream_capability(remote_device,
							                                                      LinphoneStreamTypeAudio),
							    _linphone_participant_device_get_audio_enabled(d));
							check_participant_media_direction(
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo),
							    linphone_participant_device_get_stream_capability(remote_device,
							                                                      LinphoneStreamTypeVideo),
							    _linphone_participant_device_get_video_enabled(d));
							check_participant_media_direction(
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeText),
							    linphone_participant_device_get_stream_capability(remote_device,
							                                                      LinphoneStreamTypeText),
							    _linphone_participant_device_get_real_time_text_enabled(d));

							BC_ASSERT_TRUE(
							    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeAudio));
							BC_ASSERT_TRUE(
							    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
							    (linphone_participant_device_get_stream_capability(
							         remote_device, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
						}
					}
				}
			}
			if (local_devices) {
				bctbx_list_free_with_data(local_devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		bctbx_list_free_with_data(local_conference_participants, (void (*)(void *))linphone_participant_unref);

		const LinphoneConferenceParams *local_params = linphone_conference_get_current_params(local_conference);
		const LinphoneConferenceParams *remote_params = linphone_conference_get_current_params(remote_conference);
		BC_ASSERT_EQUAL(linphone_conference_params_audio_enabled(local_params),
		                linphone_conference_params_audio_enabled(remote_params), int, "%0d");
		BC_ASSERT_EQUAL(linphone_conference_params_video_enabled(local_params),
		                linphone_conference_params_video_enabled(remote_params), int, "%0d");
		BC_ASSERT_EQUAL(linphone_conference_params_chat_enabled(local_params),
		                linphone_conference_params_chat_enabled(remote_params), int, "%0d");
	}
}

static void check_participant_added_to_conference(bctbx_list_t *lcs,
                                                  LinphoneCoreManager *conf_mgr,
                                                  stats conf_initial_stats,
                                                  bctbx_list_t *new_participants,
                                                  stats *new_participant_initial_stats,
                                                  bool_t *is_call_paused,
                                                  bctbx_list_t *participants,
                                                  stats *participant_initial_stats,
                                                  LinphoneConference *conference) {

	const int no_new_participants = (int)bctbx_list_size(new_participants);
	int no_participants_without_event_log = 0;

	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             conf_initial_stats.number_of_LinphoneCallStreamsRunning + no_new_participants,
	                             liblinphone_tester_sip_timeout));

	//  Check that me has focus attribute set to true
	BC_ASSERT_PTR_NOT_NULL(conference);
	if (conference) {
		LinphoneParticipant *me = linphone_conference_get_me(conference);
		BC_ASSERT_PTR_NOT_NULL(me);
		BC_ASSERT_TRUE(linphone_participant_is_focus(me));
	}

	int idx = 0;
	bool_t conf_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(conf_mgr->lc), "misc", "conference_event_log_enabled", TRUE);
	for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
		    new_participant_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1 + (is_call_paused[idx]) ? 1 : 0,
		    liblinphone_tester_sip_timeout));

		// Remote conference creation
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateCreationPending,
		                  new_participant_initial_stats[idx].number_of_LinphoneConferenceStateCreationPending + 1,
		                  liblinphone_tester_sip_timeout));

		bool_t p_event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
		if (p_event_log_enabled) {
			// Check subscriptions
			BC_ASSERT_TRUE(
			    wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                  (new_participant_initial_stats[idx].number_of_LinphoneSubscriptionOutgoingProgress + 1),
			                  liblinphone_tester_sip_timeout));
			if (conf_event_log_enabled) {
				BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,
				                             (conf_initial_stats.number_of_LinphoneSubscriptionIncomingReceived + idx -
				                              no_participants_without_event_log + 1),
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionActive,
				                             (conf_initial_stats.number_of_LinphoneSubscriptionActive + idx + 1),
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionActive,
				                  new_participant_initial_stats[idx].number_of_LinphoneSubscriptionActive + 1,
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateCreated,
				                  new_participant_initial_stats[idx].number_of_LinphoneConferenceStateCreated + 1,
				                  liblinphone_tester_sip_timeout));
			}

		} else {
			no_participants_without_event_log++;
		}

		if (p_event_log_enabled && conf_event_log_enabled) {
			// Number of subscription errors should not change as they the participant should received a notification
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneSubscriptionError,
			                new_participant_initial_stats[idx].number_of_LinphoneSubscriptionError, int, "%0d");
		}

		// Number of subscription terminated should not change as they the participant should received a notification
		BC_ASSERT_EQUAL(m->stat.number_of_LinphoneSubscriptionTerminated,
		                new_participant_initial_stats[idx].number_of_LinphoneSubscriptionTerminated, int, "%d");
		idx++;
	}

	int idx2 = 0;
	for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
		LinphoneCoreManager *m2 = (LinphoneCoreManager *)bctbx_list_get_data(itm);
		bool_t p2_event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m2->lc), "misc", "conference_event_log_enabled", TRUE);
		if (p2_event_log_enabled && conf_event_log_enabled) {
			if (bctbx_list_find(new_participants, m2)) {
				// Notify full state
				BC_ASSERT_TRUE(wait_for_list(lcs, &m2->stat.number_of_NotifyFullStateReceived,
				                             (participant_initial_stats[idx2].number_of_NotifyFullStateReceived + 1),
				                             liblinphone_tester_sip_timeout));
			} else {
				// Participant added
				// Participant device added
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m2->stat.number_of_participants_added,
				                  (participant_initial_stats[idx2].number_of_participants_added + no_new_participants),
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    lcs, &m2->stat.number_of_participant_devices_added,
				    (participant_initial_stats[idx2].number_of_participant_devices_added + no_new_participants),
				    liblinphone_tester_sip_timeout));
			}
		}
		idx2++;
	}

	int expected_subscriptions = 0;
	if (conference) {
		expected_subscriptions = linphone_conference_get_participant_count(conference);
		for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(itm);
			bool_t event_log_enabled =
			    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
			// If events logs are not enabled, subscribes are not sent
			if (!event_log_enabled) {
				expected_subscriptions--;
			}

			LinphoneCall *part_to_conf_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(part_to_conf_call);
		}
	}

	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &conf_mgr->subscription_received, expected_subscriptions, liblinphone_tester_sip_timeout));
	if (conf_event_log_enabled) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionActive, expected_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionTerminated,
		                conf_initial_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");
	} else {
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionTerminated,
		                             expected_subscriptions, liblinphone_tester_sip_timeout));
		BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionActive, 0, int, "%d");
	}

	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionError,
	                conf_initial_stats.number_of_LinphoneSubscriptionError, int, "%0d");

	// Add a short wait to ensure that all NOTIFYs are replied
	wait_for_list(lcs, NULL, 0, 1000);

	if (!conference) {
		conference = linphone_core_get_conference(conf_mgr->lc);
	}
	BC_ASSERT_PTR_NOT_NULL(conference);
	if (conference) {
		const LinphoneAddress *local_conference_address = linphone_conference_get_conference_address(conference);
		idx = 0;
		for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			bool_t p_event_log_enabled =
			    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
			if (p_event_log_enabled && conf_event_log_enabled) {
				LinphoneAddress *m_uri = linphone_address_new(linphone_core_get_identity(m->lc));
				LinphoneConference *remote_conference =
				    linphone_core_search_conference(m->lc, NULL, m_uri, local_conference_address, NULL);
				BC_ASSERT_PTR_NOT_NULL(remote_conference);
				linphone_address_unref(m_uri);
				int no_participants =
				    (int)bctbx_list_size(participants) + 1 - (linphone_conference_is_in(conference) ? 1 : 2);
				int counter = 0;
				do {
					counter++;
					wait_for_list(lcs, NULL, 0, 100);
				} while ((counter < 10) &&
				         (linphone_conference_get_participant_count(remote_conference) < no_participants));
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(remote_conference), no_participants, int,
				                "%d");
				BC_ASSERT_PTR_NOT_NULL(remote_conference);
				if (remote_conference) {
					check_conference_medias(conference, remote_conference);
				}
			}
			idx++;
		}
	}
}

void check_nb_streams(LinphoneCoreManager *m1,
                      LinphoneCoreManager *m2,
                      const int nb_audio_streams,
                      const int nb_video_streams,
                      const int nb_text_streams) {
	LinphoneCall *m1_to_m2_call = linphone_core_get_call_by_remote_address2(m1->lc, m2->identity);
	BC_ASSERT_PTR_NOT_NULL(m1_to_m2_call);
	if (m1_to_m2_call) {
		_linphone_call_check_nb_streams(m1_to_m2_call, nb_audio_streams, nb_video_streams, nb_text_streams);
	}
	LinphoneCall *m2_to_m1_call = linphone_core_get_call_by_remote_address2(m2->lc, m1->identity);
	BC_ASSERT_PTR_NOT_NULL(m2_to_m1_call);
	if (m2_to_m1_call) {
		_linphone_call_check_nb_streams(m2_to_m1_call, nb_audio_streams, nb_video_streams, nb_text_streams);
	}
}

static void wait_for_conference_stable_state(bctbx_list_t *lcs,
                                             LinphoneCoreManager *conf_mgr,
                                             size_t no_parts,
                                             bctbx_list_t *new_participants,
                                             const LinphoneAddress *conference_address) {

	bool_t c_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(conf_mgr->lc), "misc", "conference_event_log_enabled", TRUE);
	for (bctbx_list_t *itParticipant = new_participants; itParticipant;
	     itParticipant = bctbx_list_next(itParticipant)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(itParticipant);
		bool_t p_event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
		if (c_event_log_enabled && p_event_log_enabled) {
			LinphoneConference *conference =
			    linphone_core_search_conference(m->lc, NULL, NULL, conference_address, NULL);
			int part_counter = 0;
			bctbx_list_t *participant_device_list = NULL;
			do {
				if (participant_device_list) {
					bctbx_list_free_with_data(participant_device_list,
					                          (void (*)(void *))linphone_participant_device_unref);
				}
				participant_device_list = linphone_conference_get_participant_device_list(conference);
				part_counter++;
				wait_for_list(lcs, NULL, 0, 100);
			} while ((part_counter < 100) && (bctbx_list_size(participant_device_list) != (size_t)no_parts));
			BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), no_parts, size_t, "%0zu");

			for (bctbx_list_t *itDevice = participant_device_list; itDevice; itDevice = bctbx_list_next(itDevice)) {
				LinphoneParticipantDevice *device = (LinphoneParticipantDevice *)bctbx_list_get_data(itDevice);
				LinphoneParticipantDeviceState device_state = LinphoneParticipantDeviceStateJoining;
				part_counter = 0;
				do {
					device_state = linphone_participant_device_get_state(device);
					part_counter++;
					wait_for_list(lcs, NULL, 0, 100);
				} while ((part_counter < 100) && (device_state != LinphoneParticipantDeviceStatePresent) &&
				         (device_state != LinphoneParticipantDeviceStateOnHold));
				BC_ASSERT_TRUE((device_state == LinphoneParticipantDeviceStatePresent) ||
				               (device_state == LinphoneParticipantDeviceStateOnHold));
			}

			if (participant_device_list) {
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
			}
		}
	}
}

LinphoneStatus add_calls_to_remote_conference(bctbx_list_t *lcs,
                                              LinphoneCoreManager *focus_mgr,
                                              LinphoneCoreManager *conf_mgr,
                                              bctbx_list_t *new_participants,
                                              LinphoneConference *conference,
                                              bool_t one_by_one) {

	stats conf_initial_stats = conf_mgr->stat;
	stats focus_initial_stats = focus_mgr->stat;

	LinphoneCall *conf_to_focus_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, focus_mgr->identity);

	int counter = 1;
	int counter2 = 1;
	stats *participants_initial_stats = NULL;
	stats *new_participants_initial_stats = NULL;
	bctbx_list_t *participants = NULL;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager *m = get_manager(c);
		if ((m != focus_mgr) && (m != conf_mgr)) {
			// Allocate memory
			participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
			// Append element
			participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			participants = bctbx_list_append(participants, m);
		}
		if (bctbx_list_find(new_participants, m)) {
			// Allocate memory
			new_participants_initial_stats = (stats *)realloc(new_participants_initial_stats, counter2 * sizeof(stats));
			// Append element
			new_participants_initial_stats[counter2 - 1] = m->stat;
			// Increment counter
			counter2++;
		}
	}

	LinphoneConference *focus_conference =
	    (conf_to_focus_call) ? linphone_call_get_conference(conf_to_focus_call) : NULL;
	int init_parts_count = (focus_conference) ? linphone_conference_get_participant_count(focus_conference) : 0;
	bool_t focus_conference_not_existing = (focus_conference) ? FALSE : TRUE;
	LinphoneConference *admin_conference = conference ? conference : linphone_core_get_conference(conf_mgr->lc);

	size_t initial_device_count = 0;
	if (focus_conference) {
		bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(focus_conference);
		initial_device_count =
		    bctbx_list_size(participant_device_list) + (!!!linphone_conference_is_in(focus_conference) ? 1 : 0);
		if (participant_device_list) {
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
		}
	}

	if (!one_by_one) {
		for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
			BC_ASSERT_PTR_NOT_NULL(conf_call);
			if (admin_conference) {
				const LinphoneAddress *conference_address =
				    linphone_conference_get_conference_address(admin_conference);
				char *conference_address_str =
				    (conference_address) ? linphone_address_as_string(conference_address) : ms_strdup("sip:");
				ms_message("%s is adding %s to conference %p (address %s)", linphone_core_get_identity(conf_mgr->lc),
				           linphone_core_get_identity(m->lc), admin_conference, conference_address_str);
				linphone_conference_add_participant(admin_conference, conf_call);
				ms_free(conference_address_str);
			} else {
				ms_message("%s is adding %s to a conference held by the core", linphone_core_get_identity(conf_mgr->lc),
				           linphone_core_get_identity(m->lc));
				linphone_core_add_to_conference(conf_mgr->lc, conf_call);
			}
		}
	}

	// If focus conference is not created yet, then 2 call will be established when the 1st participant is added
	counter = 0;
	int update_counter = init_parts_count;

	for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
		counter++;
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		if (one_by_one) {
			LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
			BC_ASSERT_PTR_NOT_NULL(conf_call);
			if (admin_conference) {
				const LinphoneAddress *conference_address =
				    linphone_conference_get_conference_address(admin_conference);
				char *conference_address_str =
				    conference_address ? linphone_address_as_string(conference_address) : ms_strdup("sip:");
				ms_message("%s is adding %s to a conference %s", linphone_core_get_identity(conf_mgr->lc),
				           linphone_core_get_identity(m->lc), conference_address_str);
				linphone_conference_add_participant(admin_conference, conf_call);
				ms_free(conference_address_str);
			} else {
				ms_message("%s is adding %s to a conference held by the core", linphone_core_get_identity(conf_mgr->lc),
				           linphone_core_get_identity(m->lc));
				linphone_core_add_to_conference(conf_mgr->lc, conf_call);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &m->stat.number_of_LinphoneConferenceStateCreationPending,
		    new_participants_initial_stats[counter - 1].number_of_LinphoneConferenceStateCreationPending + 1,
		    liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateCreated,
		                  new_participants_initial_stats[counter - 1].number_of_LinphoneConferenceStateCreated + 1,
		                  liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &m->stat.number_of_LinphoneSubscriptionOutgoingProgress,
		    (new_participants_initial_stats[counter - 1].number_of_LinphoneSubscriptionOutgoingProgress + 1),
		    liblinphone_tester_sip_timeout));
		//		BC_ASSERT_TRUE(wait_for_list(lcs,&focus_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,(focus_initial_stats.number_of_LinphoneSubscriptionIncomingReceived
		//+ counter),1000));

		BC_ASSERT_TRUE(
		    wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionActive,
		                  new_participants_initial_stats[counter - 1].number_of_LinphoneSubscriptionActive + 1,
		                  liblinphone_tester_sip_timeout));
		//		BC_ASSERT_TRUE(wait_for_list(lcs,&focus_mgr->stat.number_of_LinphoneSubscriptionActive,(focus_initial_stats.number_of_LinphoneSubscriptionActive
		//+ counter),1000));

		BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
		                             (focus_initial_stats.number_of_LinphoneCallStreamsRunning + counter +
		                              ((focus_conference_not_existing) ? 1 : 0)),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneTransferCallConnected,
		                             conf_initial_stats.number_of_LinphoneTransferCallConnected + counter,
		                             liblinphone_tester_sip_timeout));

		// End of call between conference and participant
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallEnd,
		                             conf_initial_stats.number_of_LinphoneCallEnd + counter,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
		                             new_participants_initial_stats[counter - 1].number_of_LinphoneCallEnd + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallReleased,
		                             conf_initial_stats.number_of_LinphoneCallReleased + counter,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
		                             new_participants_initial_stats[counter - 1].number_of_LinphoneCallReleased + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_NotifyReceived,
		                             (new_participants_initial_stats[counter - 1].number_of_NotifyReceived + 1),
		                             liblinphone_tester_sip_timeout));
		// Local conference
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus_mgr->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		if (focus_call) {
			BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(focus_call));
			BC_ASSERT_TRUE(linphone_call_is_in_conference(focus_call));
		}

		// Remote  conference
		LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, focus_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(participant_call);
		if (participant_call) {
			BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
			BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
		}

		LinphoneCall *actual_conf_to_focus_call =
		    linphone_core_get_call_by_remote_address2(conf_mgr->lc, focus_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(actual_conf_to_focus_call);
		if (actual_conf_to_focus_call) {
			focus_conference = linphone_call_get_conference(actual_conf_to_focus_call);
			BC_ASSERT_PTR_NOT_NULL(focus_conference);
			if (focus_conference) {
				const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(focus_conference);
				if (!!linphone_conference_params_video_enabled(conf_params) == TRUE) {
					BC_ASSERT_TRUE(wait_for_list(
					    lcs, &focus_mgr->stat.number_of_LinphoneCallUpdatedByRemote,
					    (focus_initial_stats.number_of_LinphoneCallUpdatedByRemote + update_counter), 20000));
					BC_ASSERT_TRUE(wait_for_list(
					    lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
					    (focus_initial_stats.number_of_LinphoneCallStreamsRunning + counter + update_counter), 20000));
				}
			}
		}

		update_counter += (update_counter + 1);
	}

	BC_ASSERT_PTR_NOT_NULL(focus_conference);
	if (focus_conference) {
		LinphoneCall *conf_to_focus_call = linphone_conference_get_call(focus_conference);
		BC_ASSERT_PTR_NOT_NULL(conf_to_focus_call);
		if (conf_to_focus_call) {
			BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                             (focus_initial_stats.number_of_LinphoneCallStreamsRunning +
			                              2 * ((int)(bctbx_list_size(new_participants)))),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &conf_mgr->stat.number_of_participants_added,
			    (conf_initial_stats.number_of_participants_added + (int)(bctbx_list_size(new_participants))),
			    liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &conf_mgr->stat.number_of_participant_devices_added,
			    (conf_initial_stats.number_of_participant_devices_added + (int)(bctbx_list_size(new_participants))),
			    liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &focus_mgr->stat.number_of_participants_added,
			    (focus_initial_stats.number_of_participants_added + (int)(bctbx_list_size(new_participants))),
			    liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    lcs, &focus_mgr->stat.number_of_participant_devices_added,
			    (focus_initial_stats.number_of_participant_devices_added + (int)(bctbx_list_size(new_participants))),
			    liblinphone_tester_sip_timeout));
		}

		const LinphoneAddress *local_conference_address = linphone_conference_get_conference_address(focus_conference);

		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneAddress *m_uri = linphone_address_new(linphone_core_get_identity(m->lc));
			LinphoneConference *remote_conference =
			    linphone_core_search_conference(m->lc, NULL, m_uri, local_conference_address, NULL);
			linphone_address_unref(m_uri);

			int part_counter = 0;
			do {
				part_counter++;
				wait_for_list(lcs, NULL, 0, 100);
			} while ((part_counter < 100) &&
			         (remote_conference && (linphone_conference_get_participant_count(remote_conference) <
			                                (int)(bctbx_list_size(participants)))));
		}

		int wait_counter = 0;
		do {
			wait_counter++;
			wait_for_list(lcs, NULL, 0, 100);
		} while ((wait_counter < 100) &&
		         (linphone_conference_get_participant_count(focus_conference) < (int)(bctbx_list_size(participants))));

		const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(focus_conference);
		if (!!linphone_conference_params_video_enabled(conf_params) == TRUE) {
			bool_t request_streams = FALSE;
			for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
				LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
				LinphoneAddress *m_uri = linphone_address_new(linphone_core_get_identity(m->lc));
				LinphoneParticipant *m_participant = linphone_conference_find_participant(focus_conference, m_uri);
				BC_ASSERT_PTR_NOT_NULL(m_participant);
				linphone_address_unref(m_uri);
				if (m_participant) {
					bctbx_list_t *devices = linphone_participant_get_devices(m_participant);
					for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
						BC_ASSERT_PTR_NOT_NULL(d);
						if (d) {
							request_streams |=
							    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						}
					}
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}

			int idx = 0;
			int part_updates = (int)bctbx_list_size(new_participants);
			for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
				counter = init_parts_count + (int)part_updates;
				LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
				LinphoneAddress *m_uri = linphone_address_new(linphone_core_get_identity(m->lc));
				LinphoneParticipant *m_participant = linphone_conference_find_participant(focus_conference, m_uri);
				BC_ASSERT_PTR_NOT_NULL(m_participant);
				linphone_address_unref(m_uri);
				if (m_participant) {
					bctbx_list_t *devices = linphone_participant_get_devices(m_participant);
					for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
						BC_ASSERT_PTR_NOT_NULL(d);
						LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(
						    focus_mgr->lc, linphone_participant_device_get_address(d));
						BC_ASSERT_PTR_NOT_NULL(participant_call);
						if (participant_call && request_streams) {
							const LinphoneCallParams *participant_call_params =
							    linphone_call_get_current_params(participant_call);
							bool_t call_video_enabled = linphone_call_params_video_enabled(participant_call_params);
							if (call_video_enabled) {
								BC_ASSERT_TRUE(
								    wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdating,
								                  (participants_initial_stats[idx].number_of_LinphoneCallUpdating + 1),
								                  liblinphone_tester_sip_timeout));
								BC_ASSERT_TRUE(wait_for_list(
								    lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
								    (participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning +
								     (bctbx_list_find(new_participants, m) ? 2 : 1)),
								    liblinphone_tester_sip_timeout));
							} else if (bctbx_list_find(new_participants, m)) {
								BC_ASSERT_TRUE(wait_for_list(
								    lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
								    (participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1),
								    liblinphone_tester_sip_timeout));
							}
						}
					}
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}

				part_updates--;
				idx++;
			}
			BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                             (focus_initial_stats.number_of_LinphoneCallStreamsRunning + counter +
			                              ((int)bctbx_list_size(new_participants))),
			                             liblinphone_tester_sip_timeout));
		}
	}

	ms_free(participants_initial_stats);
	ms_free(new_participants_initial_stats);
	bctbx_list_free(participants);

	// Remote  conference
	if (conf_to_focus_call == NULL) {
		// Asserts to verify that call between focus and confernece manager is correctly set up
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
		                             (conf_initial_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),
		                             liblinphone_tester_sip_timeout));
		//		BC_ASSERT_TRUE(wait_for_list(lcs,&focus_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,(focus_initial_stats.number_of_LinphoneSubscriptionIncomingReceived
		//+ counter + 1),1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateCreationPending,
		                             focus_initial_stats.number_of_LinphoneConferenceStateCreationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &focus_mgr->stat.number_of_LinphoneConferenceStateCreated,
		                             focus_initial_stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionActive,
		                             conf_initial_stats.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		//		BC_ASSERT_TRUE(wait_for_list(lcs,&focus_mgr->stat.number_of_LinphoneSubscriptionActive,(focus_initial_stats.number_of_LinphoneSubscriptionActive
		//+ counter + 1),1000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
		                             (conf_initial_stats.number_of_LinphoneCallStreamsRunning + 1),
		                             liblinphone_tester_sip_timeout));
		conf_to_focus_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, focus_mgr->identity);
	}
	BC_ASSERT_PTR_NOT_NULL(conf_to_focus_call);
	if (conf_to_focus_call) {
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(conf_to_focus_call));
		BC_ASSERT_FALSE(linphone_call_is_in_conference(conf_to_focus_call));
	}

	// Local conference
	LinphoneCall *focus_to_conf_call = linphone_core_get_call_by_remote_address2(focus_mgr->lc, conf_mgr->identity);
	BC_ASSERT_PTR_NOT_NULL(focus_to_conf_call);
	if (focus_to_conf_call) {
		BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(focus_to_conf_call));
		BC_ASSERT_TRUE(linphone_call_is_in_conference(focus_to_conf_call));
	}

	size_t no_parts = initial_device_count + bctbx_list_size(new_participants) + ((initial_device_count == 0) ? 1 : 0);
	wait_for_conference_stable_state(lcs, conf_mgr, no_parts, new_participants,
	                                 linphone_conference_get_conference_address(focus_conference));

	return 0;
}

LinphoneStatus add_calls_to_local_conference(bctbx_list_t *lcs,
                                             LinphoneCoreManager *conf_mgr,
                                             LinphoneConference *conference,
                                             bctbx_list_t *new_participants,
                                             bool_t one_by_one) {

	stats conf_initial_stats = conf_mgr->stat;

	stats *participants_initial_stats = NULL;
	bctbx_list_t *participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager *m = get_manager(c);
		if (m != conf_mgr) {
			// Allocate memory
			participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
			// Append element
			participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			participants = bctbx_list_append(participants, m);
		}
	}

	counter = 1;
	stats *new_participants_initial_stats = NULL;
	for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		// Allocate memory
		new_participants_initial_stats = (stats *)realloc(new_participants_initial_stats, counter * sizeof(stats));
		// Append element
		new_participants_initial_stats[counter - 1] = m->stat;
		LinphoneCall *call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		// Increment counter
		counter++;
	}

	bool_t *call_paused = NULL;

	counter = 1;
	int resumed = 1;
	int updated = 1;

	LinphoneConference *conference_used = conference ? conference : linphone_core_get_conference(conf_mgr->lc);
	size_t initial_device_count = 0;
	if (conference_used) {
		bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(conference_used);
		initial_device_count =
		    bctbx_list_size(participant_device_list) - (linphone_conference_is_in(conference_used) ? 1 : 0);
		if (participant_device_list) {
			bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
		}
	}

	bool_t c_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(conf_mgr->lc), "misc", "conference_event_log_enabled", TRUE);

	if (one_by_one) {
		for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			stats initial_stats = m->stat;
			LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
			BC_ASSERT_PTR_NOT_NULL(conf_call);
			call_paused = (bool_t *)realloc(call_paused, counter * sizeof(bool_t));
			call_paused[counter - 1] = FALSE;
			if (conf_call) {
				bool_t is_call_paused = (linphone_call_get_state(conf_call) == LinphoneCallStatePaused);
				LinphoneCall *current_call = linphone_core_get_current_call(conf_mgr->lc);
				call_paused[counter - 1] = is_call_paused;
				if (conference) {
					linphone_conference_add_participant(conference, conf_call);
					conference_used = conference;
				} else {
					linphone_core_add_to_conference(conf_mgr->lc, conf_call);
					conference_used = linphone_core_get_conference(conf_mgr->lc);
				}
				if (current_call && current_call != conf_call &&
				    ((conference && linphone_conference_is_in(conference)) ||
				     (!conference && linphone_core_is_in_conference(conf_mgr->lc)))) {
					ms_message("There is a another running call that needed to be pause in order to enter the "
					           "participant to the conference.");
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPaused,
					                             conf_initial_stats.number_of_LinphoneCallPaused + 1,
					                             liblinphone_tester_sip_timeout));
				}
				if (is_call_paused) {
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallResuming,
					                             conf_initial_stats.number_of_LinphoneCallResuming + resumed, 2000));
					resumed++;
				} else {
					BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdatedByRemote,
					                             (initial_stats.number_of_LinphoneCallUpdatedByRemote + 1),
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallUpdating,
					                             conf_initial_stats.number_of_LinphoneCallUpdating + updated,
					                             liblinphone_tester_sip_timeout));

					bool_t p_event_log_enabled = linphone_config_get_bool(linphone_core_get_config(m->lc), "misc",
					                                                      "conference_event_log_enabled", TRUE);
					if (p_event_log_enabled && c_event_log_enabled) {
						BC_ASSERT_TRUE(
						    wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallUpdatedByRemote,
						                  (conf_initial_stats.number_of_LinphoneCallUpdatedByRemote + updated),
						                  liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdating,
						                             initial_stats.number_of_LinphoneCallUpdating + 1,
						                             liblinphone_tester_sip_timeout));
					}
					updated++;
				}
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
				                             initial_stats.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
				                             conf_initial_stats.number_of_LinphoneCallStreamsRunning + counter,
				                             liblinphone_tester_sip_timeout));

				// Local conference
				BC_ASSERT_TRUE(linphone_call_is_in_conference(conf_call));
			}

			// Remote  conference
			LinphoneCall *participant_call = linphone_core_get_current_call(m->lc);
			BC_ASSERT_PTR_NOT_NULL(participant_call);
			if (participant_call) {
				LinphoneConference *pconf = linphone_call_get_conference(participant_call);
				BC_ASSERT_PTR_NOT_NULL(pconf);
				BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
				if (pconf) {
					LinphoneParticipant *participant = linphone_conference_get_me(pconf);
					BC_ASSERT_FALSE(linphone_participant_is_focus(participant));
				}
			}

			counter++;

			// Wait a little bit - slow down test
			wait_for_list(lcs, NULL, 0, 2000);
		}
	} else {
		bctbx_list_t *calls = NULL;
		for (bctbx_list_t *it = new_participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
			LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
			BC_ASSERT_PTR_NOT_NULL(conf_call);
			call_paused = (bool_t *)realloc(call_paused, counter * sizeof(bool_t));
			call_paused[counter - 1] = FALSE;
			if (conf_call) {
				bool_t is_call_paused = (linphone_call_get_state(conf_call) == LinphoneCallStatePaused);
				call_paused[counter - 1] = is_call_paused;
				calls = bctbx_list_append(calls, conf_call);
			}
			counter++;
		}

		if (conference) {
			conference_used = conference;
		} else if (linphone_core_get_conference(conf_mgr->lc)) {
			conference_used = linphone_core_get_conference(conf_mgr->lc);
		} else {
			LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(conf_mgr->lc, NULL);
			conference_used = linphone_core_create_conference_with_params(conf_mgr->lc, conf_params);
			linphone_conference_params_unref(conf_params);
			linphone_conference_unref(conference_used); /*actually linphone_core_create_conference_with_params() takes a
			                                               ref for lc->conf_ctx */
		}
		BC_ASSERT_PTR_NOT_NULL(conference_used);
		linphone_conference_add_participants(conference_used, calls);
		bctbx_list_free(calls);
		BC_ASSERT_TRUE(wait_for_list(
		    lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
		    conf_initial_stats.number_of_LinphoneCallStreamsRunning + (int)bctbx_list_size(new_participants), 50000));
	}

	check_participant_added_to_conference(lcs, conf_mgr, conf_initial_stats, new_participants,
	                                      new_participants_initial_stats, call_paused, participants,
	                                      participants_initial_stats, conference_used);

	// Add local participant device is needed
	size_t no_parts =
	    initial_device_count + bctbx_list_size(new_participants) + (linphone_conference_is_in(conference_used) ? 1 : 0);
	wait_for_conference_stable_state(lcs, conf_mgr, no_parts, new_participants,
	                                 linphone_conference_get_conference_address(conference_used));

	ms_free(call_paused);
	ms_free(participants_initial_stats);
	ms_free(new_participants_initial_stats);
	bctbx_list_free(participants);

	return 0;
}

static LinphoneStatus check_participant_removal(bctbx_list_t *lcs,
                                                LinphoneCoreManager *conf_mgr,
                                                LinphoneCoreManager *participant_mgr,
                                                bctbx_list_t *participants,
                                                int participant_size,
                                                stats conf_initial_stats,
                                                stats participant_initial_stats,
                                                stats *participants_initial_stats,
                                                LinphoneCallState *participants_initial_state,
                                                bool_t local_participant_is_in,
                                                LinphoneConference *conference) {
	BC_ASSERT_PTR_NOT_NULL(conference);
	// Wait for conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (participant_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateTerminated,
	                             (participant_initial_stats.number_of_LinphoneConferenceStateTerminated + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateDeleted,
	                             (participant_initial_stats.number_of_LinphoneConferenceStateDeleted + 1),
	                             liblinphone_tester_sip_timeout));

	bool_t conf_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(conf_mgr->lc), "misc", "conference_event_log_enabled", TRUE);
	if (conf_event_log_enabled) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionTerminated,
		                             conf_initial_stats.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
	}
	bool_t participant_event_log_enabled = linphone_config_get_bool(linphone_core_get_config(participant_mgr->lc),
	                                                                "misc", "conference_event_log_enabled", TRUE);
	if (participant_event_log_enabled) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneSubscriptionTerminated,
		                             participant_initial_stats.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
	}

	LinphoneAddress *local_conference_address = NULL;
	bool_t one_participant_conference_enabled = FALSE;
	bool_t conf_video_enabled = FALSE;
	if (conference) {
		local_conference_address = linphone_address_clone(linphone_conference_get_conference_address(conference));
		const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(conference);
		one_participant_conference_enabled = linphone_conference_params_one_participant_conference_enabled(conf_params);
	}
	int expected_no_participants = 0;
	if (((participant_size == 2) && !one_participant_conference_enabled) || (participant_size == 1)) {
		expected_no_participants = 0;

		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             (conf_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneConferenceStateTerminated,
		                             (conf_initial_stats.number_of_LinphoneConferenceStateTerminated + 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneConferenceStateDeleted,
		                             (conf_initial_stats.number_of_LinphoneConferenceStateDeleted + 1),
		                             liblinphone_tester_sip_timeout));

		conference = linphone_core_search_conference(conf_mgr->lc, NULL, local_conference_address,
		                                             local_conference_address, NULL);
		BC_ASSERT_PTR_NULL(conference);
	} else {
		expected_no_participants = (participant_size - 1);

		if (conference) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), expected_no_participants, int, "%d");
			BC_ASSERT_EQUAL(linphone_conference_is_in(conference), local_participant_is_in, int, "%d");

			const LinphoneConferenceParams *conf_params = linphone_conference_get_current_params(conference);
			conf_video_enabled = (!!linphone_conference_params_video_enabled(conf_params));
		}
	}

	// Other participants call should not have their state modified
	if ((participants != NULL) && (one_participant_conference_enabled || (participant_size > 1))) {
		int idx = 0;
		for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(itm);

			// If removing last participant, then its call is kicked out of conference
			// - Remote conference is deleted
			// - parameters are updated
			if (expected_no_participants == 0) {
				if (local_participant_is_in) {
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallUpdating,
					                             (conf_initial_stats.number_of_LinphoneCallUpdating + 1),
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdatedByRemote,
					                  (participants_initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1),
					                  liblinphone_tester_sip_timeout));
					if (participants_initial_state[idx] == LinphoneCallStreamsRunning) {
						BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
						                             (conf_initial_stats.number_of_LinphoneCallStreamsRunning + 1),
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(
						    wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
						                  (participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1),
						                  liblinphone_tester_sip_timeout));
					} else if (participants_initial_state[idx] == LinphoneCallPaused) {
						BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPausedByRemote,
						                             (conf_initial_stats.number_of_LinphoneCallPausedByRemote + 1),
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallPaused,
						                             (participants_initial_stats[idx].number_of_LinphoneCallPaused + 1),
						                             liblinphone_tester_sip_timeout));
					} else if (participants_initial_state[idx] == LinphoneCallPausedByRemote) {
						BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPaused,
						                             (conf_initial_stats.number_of_LinphoneCallPaused + 1),
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(
						    wait_for_list(lcs, &m->stat.number_of_LinphoneCallPausedByRemote,
						                  (participants_initial_stats[idx].number_of_LinphoneCallPausedByRemote + 1),
						                  liblinphone_tester_sip_timeout));
					} else {
						char str[200];
						sprintf(str, "%s - Unhandled participant call remove from conference going to state %s\n",
						        __func__, linphone_call_state_to_string(participants_initial_state[idx]));
						BC_FAIL(str);
					}
				} else {
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPausing,
					                             (conf_initial_stats.number_of_LinphoneCallPausing + 1),
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(lcs, &m->stat.number_of_LinphoneCallPausedByRemote,
					                  (participants_initial_stats[idx].number_of_LinphoneCallPausedByRemote + 1),
					                  liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPaused,
					                             (conf_initial_stats.number_of_LinphoneCallPaused + 1),
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                  (conf_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneConferenceStateTerminated,
				                             (conf_initial_stats.number_of_LinphoneConferenceStateTerminated + 1),
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneConferenceStateDeleted,
				                             (conf_initial_stats.number_of_LinphoneConferenceStateDeleted + 1),
				                             liblinphone_tester_sip_timeout));

				LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
				BC_ASSERT_PTR_NOT_NULL(conf_call);

				if (conf_call) {
					BC_ASSERT_PTR_NULL(linphone_call_get_conference(conf_call));
					BC_ASSERT_FALSE(linphone_call_is_in_conference(conf_call));
				}

				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);

				if (participant_call) {
					BC_ASSERT_PTR_NULL(linphone_call_get_conference(participant_call));
					BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
				}

				bool_t conf_event_log_enabled = linphone_config_get_bool(linphone_core_get_config(conf_mgr->lc), "misc",
				                                                         "conference_event_log_enabled", TRUE);
				if (conf_event_log_enabled) {
					BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionTerminated,
					                             conf_initial_stats.number_of_LinphoneSubscriptionTerminated + 2,
					                             liblinphone_tester_sip_timeout));
				}
				bool_t m_event_log_enabled = linphone_config_get_bool(linphone_core_get_config(m->lc), "misc",
				                                                      "conference_event_log_enabled", TRUE);
				if (m_event_log_enabled) {
					BC_ASSERT_TRUE(
					    wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionTerminated,
					                  participants_initial_stats[idx].number_of_LinphoneSubscriptionTerminated + 1,
					                  liblinphone_tester_sip_timeout));
				}
			} else {

				// Wait for notify of participant device deleted and participant deleted
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_participants_removed,
				                             (participants_initial_stats[idx].number_of_participants_removed + 1),
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m->stat.number_of_participant_devices_removed,
				                  (participants_initial_stats[idx].number_of_participant_devices_removed + 1),
				                  liblinphone_tester_sip_timeout));

				LinphoneAddress *participant_uri = linphone_address_new(linphone_core_get_identity(m->lc));
				LinphoneConference *participant_conference =
				    linphone_core_search_conference(m->lc, NULL, participant_uri, local_conference_address, NULL);
				linphone_address_unref(participant_uri);
				BC_ASSERT_PTR_NOT_NULL(participant_conference);

				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					const LinphoneCallParams *participant_call_params =
					    linphone_call_get_current_params(participant_call);
					bool_t call_video_enabled = linphone_call_params_video_enabled(participant_call_params);
					if ((conf_video_enabled == TRUE) && (call_video_enabled == TRUE)) {
						BC_ASSERT_TRUE(
						    wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
						                  (participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1),
						                  liblinphone_tester_sip_timeout));
					}
				}
			}
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallEnd,
			                participants_initial_stats[idx].number_of_LinphoneCallEnd, int, "%0d");
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallReleased,
			                participants_initial_stats[idx].number_of_LinphoneCallReleased, int, "%0d");
			idx++;
		}
	}

	linphone_address_unref(local_conference_address);

	return 0;
}

LinphoneStatus remove_participant_from_local_conference(bctbx_list_t *lcs,
                                                        LinphoneCoreManager *conf_mgr,
                                                        LinphoneCoreManager *participant_mgr,
                                                        LinphoneConference *conf) {

	stats conf_initial_stats = conf_mgr->stat;
	stats participant_initial_stats = participant_mgr->stat;

	LinphoneConference *local_conference = (conf ? conf : linphone_core_get_conference(conf_mgr->lc));
	BC_ASSERT_PTR_NOT_NULL(local_conference);
	if (local_conference) {
		local_conference = linphone_conference_ref(local_conference);
		int participant_size = linphone_conference_get_participant_count(local_conference);
		bool_t local_participant_is_in = linphone_conference_is_in(local_conference);

		LinphoneAddress *local_conference_address =
		    linphone_address_clone(linphone_conference_get_conference_address(local_conference));

		char *local_conference_address_str = linphone_address_as_string(local_conference_address);
		ms_message("Trying to remove %s from conference %s\n", linphone_core_get_identity(participant_mgr->lc),
		           local_conference_address_str);
		bctbx_free(local_conference_address_str);

		LinphoneAddress *participant_uri = linphone_address_new(linphone_core_get_identity(participant_mgr->lc));
		LinphoneConference *participant_conference =
		    linphone_core_search_conference(participant_mgr->lc, NULL, participant_uri, local_conference_address, NULL);
		linphone_address_unref(participant_uri);
		BC_ASSERT_PTR_NOT_NULL(participant_conference);

		stats *participants_initial_stats = NULL;
		LinphoneCallState *participants_initial_state = NULL;
		bctbx_list_t *participants = NULL;
		int counter = 1;
		for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
			LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
			LinphoneCoreManager *m = get_manager(c);
			if ((m != participant_mgr) && (m != conf_mgr)) {
				// Allocate memory
				participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
				participants_initial_state =
				    (LinphoneCallState *)realloc(participants_initial_state, counter * sizeof(LinphoneCallState));
				// Append element
				participants_initial_stats[counter - 1] = m->stat;

				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					participants_initial_state[counter - 1] = linphone_call_get_state(participant_call);
				} else {
					participants_initial_state[counter - 1] = LinphoneCallStateIdle;
				}

				// Increment counter
				counter++;
				participants = bctbx_list_append(participants, m);
			}
		}

		LinphoneCall *conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, participant_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		bool_t is_conf_call_paused =
		    (conf_call) ? (linphone_call_get_state(conf_call) == LinphoneCallStatePaused) : FALSE;

		LinphoneCall *participant_call =
		    linphone_core_get_call_by_remote_address2(participant_mgr->lc, conf_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(participant_call);

		LinphoneParticipant *deleted_participant =
		    linphone_conference_find_participant(local_conference, participant_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(deleted_participant);
		bool_t preserve_session =
		    (deleted_participant) ? linphone_participant_preserve_session(deleted_participant) : FALSE;

		if (conf_call) {
			linphone_conference_remove_participant_3(local_conference, conf_call);
		}

		// If the conference already put the call in pause, then the participant call is already in the state
		// PausedByRemote If the participant call was already paused, then it stays in the paused state while accepting
		// the update
		if (preserve_session) {
			// If the conference already put the call in pause, then it will not be put in pause again
			// Calls are paused or updated when removing a participant
			BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPausing,
			                             (conf_initial_stats.number_of_LinphoneCallPausing + 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPaused,
			                             (conf_initial_stats.number_of_LinphoneCallPaused + 1),
			                             liblinphone_tester_sip_timeout));

			if (is_conf_call_paused) {
				BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallUpdatedByRemote,
				                             (participant_initial_stats.number_of_LinphoneCallUpdatedByRemote + 1),
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallPausedByRemote,
				                             (participant_initial_stats.number_of_LinphoneCallPausedByRemote + 1),
				                             liblinphone_tester_sip_timeout));
			}
		} else {
			BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallEnd,
			                             (participant_initial_stats.number_of_LinphoneCallEnd + 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallReleased,
			                             (participant_initial_stats.number_of_LinphoneCallReleased + 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallEnd,
			                             (conf_initial_stats.number_of_LinphoneCallEnd + 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallReleased,
			                             (conf_initial_stats.number_of_LinphoneCallReleased + 1),
			                             liblinphone_tester_sip_timeout));
		}

		check_participant_removal(lcs, conf_mgr, participant_mgr, participants, participant_size, conf_initial_stats,
		                          participant_initial_stats, participants_initial_stats, participants_initial_state,
		                          local_participant_is_in, local_conference);

		participant_uri = linphone_address_new(linphone_core_get_identity(participant_mgr->lc));
		participant_conference =
		    linphone_core_search_conference(participant_mgr->lc, NULL, participant_uri, local_conference_address, NULL);
		linphone_address_unref(participant_uri);
		linphone_address_unref(local_conference_address);
		BC_ASSERT_PTR_NULL(participant_conference);

		ms_free(participants_initial_stats);
		ms_free(participants_initial_state);
		bctbx_list_free(participants);
		linphone_conference_unref(local_conference);
	}

	return 0;
}

static void finish_terminate_local_conference(bctbx_list_t *lcs,
                                              stats *lcm_stats,
                                              bool_t *call_is_in_conference,
                                              LinphoneCoreManager *conf_mgr,
                                              unsigned int no_participants,
                                              bool_t core_held_conference) {
	int idx = 0;
	bool_t conf_event_log_enabled =
	    linphone_config_get_bool(linphone_core_get_config(conf_mgr->lc), "misc", "conference_event_log_enabled", TRUE);

	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore *c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager *m = get_manager(c);

		unsigned int no_calls = 0;
		if (m == conf_mgr) {
			no_calls = no_participants;
		} else {
			no_calls = 1;
		}

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
		                             lcm_stats[idx].number_of_LinphoneCallEnd + no_calls, 30000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted,
		                             lcm_stats[idx].number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		bool_t event_log_enabled =
		    linphone_config_get_bool(linphone_core_get_config(m->lc), "misc", "conference_event_log_enabled", TRUE);
		// In case of a re-registration, the number of active subscriptions on the local conference side accounts the
		// numbers of subscriptions before and after the re-registration
		if ((m != conf_mgr) && (m->stat.number_of_LinphoneRegistrationOk == 1) && conf_event_log_enabled &&
		    event_log_enabled) {
			// If the participant left the conference, then his subscription to the conference was terminated
			if (call_is_in_conference[idx]) {
				BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneSubscriptionTerminated,
				                             lcm_stats[idx].number_of_LinphoneSubscriptionTerminated + 1,
				                             liblinphone_tester_sip_timeout));
			} else {
				BC_ASSERT_EQUAL(m->stat.number_of_LinphoneSubscriptionTerminated,
				                lcm_stats[idx].number_of_LinphoneSubscriptionActive, int, "%0d");
			}
		}

		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
		                             lcm_stats[idx].number_of_LinphoneCallReleased + no_calls,
		                             liblinphone_tester_sip_timeout));

		if ((m != conf_mgr) || ((m == conf_mgr) && (core_held_conference == TRUE))) {
			LinphoneConference *conference = linphone_core_get_conference(c);
			BC_ASSERT_PTR_NULL(conference);
		}

		if (m == conf_mgr) {
			for (bctbx_list_t *it2 = lcs; it2; it2 = bctbx_list_next(it2)) {
				LinphoneCore *c2 = (LinphoneCore *)bctbx_list_get_data(it2);
				LinphoneCoreManager *m2 = get_manager(c2);
				if (m2 != conf_mgr) {
					LinphoneCall *conference_call =
					    linphone_core_get_call_by_remote_address2(conf_mgr->lc, m2->identity);
					BC_ASSERT_PTR_NULL(conference_call);
				}
			}
		} else {
			LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
			BC_ASSERT_PTR_NULL(participant_call);
		}

		idx++;
	}
}

static void finish_terminate_remote_conference(bctbx_list_t *lcs,
                                               stats *lcm_stats,
                                               bool_t *call_is_in_conference,
                                               BCTBX_UNUSED(LinphoneCoreManager *conf_mgr),
                                               LinphoneCoreManager *focus_mgr,
                                               unsigned int no_participants,
                                               bool_t core_held_conference) {
	finish_terminate_local_conference(lcs, lcm_stats, call_is_in_conference, focus_mgr, no_participants + 1,
	                                  core_held_conference);
}

bctbx_list_t *terminate_participant_call(bctbx_list_t *participants,
                                         LinphoneCoreManager *conf_mgr,
                                         LinphoneCoreManager *participant_mgr) {

	LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(participant_mgr->lc, conf_mgr->identity);
	BC_ASSERT_PTR_NOT_NULL(participant_call);

	LinphoneConference *local_conference = linphone_conference_ref(linphone_core_get_conference(conf_mgr->lc));
	BC_ASSERT_PTR_NOT_NULL(local_conference);
	LinphoneAddress *local_conference_address =
	    local_conference ? linphone_address_clone(linphone_conference_get_conference_address(local_conference)) : NULL;

	if (participant_call) {
		LinphoneAddress *participant_uri = linphone_address_new(linphone_core_get_identity(participant_mgr->lc));
		LinphoneConference *participant_conference =
		    linphone_core_search_conference(participant_mgr->lc, NULL, participant_uri, local_conference_address, NULL);
		linphone_address_unref(participant_uri);
		BC_ASSERT_PTR_NOT_NULL(participant_conference);

		stats initial_participant_stats = participant_mgr->stat;
		stats initial_conf_stats = conf_mgr->stat;
		stats *initial_other_participants_stats = NULL;
		LinphoneCallState *initial_other_participants_state = NULL;

		bctbx_list_t *lcs = NULL;
		lcs = bctbx_list_append(lcs, conf_mgr->lc);

		int counter = 1;
		for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

			if (m != participant_mgr) {
				// Allocate memory
				initial_other_participants_stats =
				    (stats *)realloc(initial_other_participants_stats, counter * sizeof(stats));
				initial_other_participants_state =
				    (LinphoneCallState *)realloc(initial_other_participants_state, counter * sizeof(LinphoneCallState));

				// Append element
				initial_other_participants_stats[counter - 1] = m->stat;

				LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					initial_other_participants_state[counter - 1] = linphone_call_get_state(participant_call);
				} else {
					initial_other_participants_state[counter - 1] = LinphoneCallStateIdle;
				}

				// Increment counter
				counter++;
			}

			lcs = bctbx_list_append(lcs, m->lc);
		}

		int participant_size = -1;
		bool_t local_participant_is_in = TRUE;
		if (local_conference) {
			participant_size = linphone_conference_get_participant_count(local_conference);
			local_participant_is_in = linphone_conference_is_in(local_conference);
		}

		linphone_core_terminate_call(participant_mgr->lc, participant_call);
		participants = bctbx_list_remove(participants, participant_mgr);
		BC_ASSERT_TRUE(wait_for(conf_mgr->lc, participant_mgr->lc, &participant_mgr->stat.number_of_LinphoneCallEnd,
		                        (initial_participant_stats.number_of_LinphoneCallEnd + 1)));
		BC_ASSERT_TRUE(wait_for(conf_mgr->lc, participant_mgr->lc, &conf_mgr->stat.number_of_LinphoneCallEnd,
		                        (initial_conf_stats.number_of_LinphoneCallEnd + 1)));
		BC_ASSERT_TRUE(wait_for(conf_mgr->lc, participant_mgr->lc,
		                        &participant_mgr->stat.number_of_LinphoneCallReleased,
		                        (initial_participant_stats.number_of_LinphoneCallReleased + 1)));
		BC_ASSERT_TRUE(wait_for(conf_mgr->lc, participant_mgr->lc, &conf_mgr->stat.number_of_LinphoneCallReleased,
		                        (initial_conf_stats.number_of_LinphoneCallReleased + 1)));

		check_participant_removal(lcs, conf_mgr, participant_mgr, participants, participant_size, initial_conf_stats,
		                          initial_participant_stats, initial_other_participants_stats,
		                          initial_other_participants_state, local_participant_is_in, local_conference);

		bctbx_list_free(lcs);
		if (initial_other_participants_stats) {
			ms_free(initial_other_participants_stats);
		}
		if (initial_other_participants_state) {
			ms_free(initial_other_participants_state);
		}

		participant_uri = linphone_address_new(linphone_core_get_identity(participant_mgr->lc));
		participant_conference =
		    linphone_core_search_conference(participant_mgr->lc, NULL, participant_uri, local_conference_address, NULL);
		linphone_address_unref(participant_uri);
		BC_ASSERT_PTR_NULL(participant_conference);
	}

	linphone_address_unref(local_conference_address);
	linphone_conference_unref(local_conference);

	return participants;
}

LinphoneStatus terminate_conference(bctbx_list_t *participants,
                                    LinphoneCoreManager *conf_mgr,
                                    LinphoneConference *conference,
                                    LinphoneCoreManager *focus_mgr,
                                    bool_t participants_exit_conference) {

	bctbx_list_t *lcs = NULL;
	stats *lcm_stats = NULL;
	bool_t *call_is_in_conference = NULL;

	LinphoneCall *conf_call = NULL;
	LinphoneCall *participant_call = NULL;

	int no_participant_left_conference = 0;

	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);

		// Allocate memory
		lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));
		call_is_in_conference = (bool_t *)realloc(call_is_in_conference, counter * sizeof(bool_t));

		lcs = bctbx_list_append(lcs, m->lc);

		if (focus_mgr) {
			conf_call = linphone_core_get_call_by_remote_address2(focus_mgr->lc, m->identity);
			participant_call = linphone_core_get_call_by_remote_address2(m->lc, focus_mgr->identity);
		} else {
			conf_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, m->identity);
			participant_call = linphone_core_get_call_by_remote_address2(m->lc, conf_mgr->identity);
		}
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		BC_ASSERT_PTR_NOT_NULL(participant_call);

		// Append element
		lcm_stats[counter - 1] = m->stat;
		if (conf_call) {
			call_is_in_conference[counter - 1] = linphone_call_is_in_conference(conf_call);
			if (!linphone_call_is_in_conference(conf_call)) no_participant_left_conference++;
		} else {
			call_is_in_conference[counter - 1] = FALSE;
		}

		// Increment counter
		counter++;
	}

	lcs = bctbx_list_append(lcs, conf_mgr->lc);
	lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));
	lcm_stats[counter - 1] = conf_mgr->stat;
	call_is_in_conference = (bool_t *)realloc(call_is_in_conference, counter * sizeof(bool_t));

	if (focus_mgr) {
		conf_call = linphone_core_get_call_by_remote_address2(focus_mgr->lc, conf_mgr->identity);
		participant_call = linphone_core_get_call_by_remote_address2(conf_mgr->lc, focus_mgr->identity);
		BC_ASSERT_PTR_NOT_NULL(conf_call);
		BC_ASSERT_PTR_NOT_NULL(participant_call);
		if (conf_call) {
			call_is_in_conference[counter - 1] = linphone_call_is_in_conference(conf_call);
			if (!linphone_call_is_in_conference(conf_call)) no_participant_left_conference++;
		} else {
			call_is_in_conference[counter - 1] = FALSE;
		}
		counter++;
		lcs = bctbx_list_append(lcs, focus_mgr->lc);
		lcm_stats = (stats *)realloc(lcm_stats, counter * sizeof(stats));
		lcm_stats[counter - 1] = focus_mgr->stat;
		call_is_in_conference = (bool_t *)realloc(call_is_in_conference, counter * sizeof(bool_t));
		call_is_in_conference[counter - 1] = FALSE;
	} else {
		call_is_in_conference[counter - 1] = FALSE;
	}

	LinphoneConference *conferenceToDestroy = conference;
	if (!conferenceToDestroy) {
		conferenceToDestroy = linphone_core_get_conference(conf_mgr->lc);
	}
	BC_ASSERT_PTR_NOT_NULL(conferenceToDestroy);
	if (conferenceToDestroy) {
		unsigned int no_participants =
		    linphone_conference_get_participant_count(conferenceToDestroy) + no_participant_left_conference;

		if (conferenceToDestroy) {
			bool_t core_held_conference = (conferenceToDestroy == linphone_core_get_conference(conf_mgr->lc));
			const LinphoneAddress *conference_address = linphone_conference_get_conference_address(conferenceToDestroy);
			BC_ASSERT_PTR_NOT_NULL(conference_address);
			if (conference_address) {
				char *conference_address_str = linphone_address_as_string(conference_address);
				if (participants_exit_conference) {
					for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
						LinphoneCoreManager *m = (LinphoneCoreManager *)bctbx_list_get_data(it);
						LinphoneCall *participant_call =
						    linphone_core_get_call_by_remote_address2(m->lc, conference_address);
						BC_ASSERT_PTR_NOT_NULL(participant_call);
						if (participant_call) {
							ms_message("%s terminates its call to conference %s", linphone_core_get_identity(m->lc),
							           conference_address_str);
							linphone_call_terminate(participant_call);
						}
					}
				} else {
					ms_message("%s terminates conference %s", linphone_core_get_identity(conf_mgr->lc),
					           conference_address_str);
					linphone_conference_terminate(conferenceToDestroy);
				}
				ms_free(conference_address_str);

				if (focus_mgr) {
					finish_terminate_remote_conference(lcs, lcm_stats, call_is_in_conference, conf_mgr, focus_mgr,
					                                   no_participants, core_held_conference);
				} else {
					finish_terminate_local_conference(lcs, lcm_stats, call_is_in_conference, conf_mgr, no_participants,
					                                  core_held_conference);
				}
			}
		}
	}

	ms_free(call_is_in_conference);
	ms_free(lcm_stats);
	bctbx_list_free(lcs);

	return 0;
}

static void call_created(LinphoneCore *lc, BCTBX_UNUSED(LinphoneCall *call)) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCallCreated++;
}

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
void linphone_core_manager_init2(LinphoneCoreManager *mgr, BCTBX_UNUSED(const char *rc_file), const char *phone_alias) {
	mgr->number_of_bcunit_error_at_creation = bc_get_number_of_failures();
	mgr->cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_registration_state_changed(mgr->cbs, registration_state_changed);
	linphone_core_cbs_set_auth_info_requested(mgr->cbs, auth_info_requested);
	linphone_core_cbs_set_authentication_requested(mgr->cbs, authentication_info_requested);
	linphone_core_cbs_set_call_created(mgr->cbs, call_created);
	linphone_core_cbs_set_call_state_changed(mgr->cbs, call_state_changed);
	linphone_core_cbs_set_message_received(mgr->cbs, message_received);
	linphone_core_cbs_set_message_received_unable_decrypt(mgr->cbs, message_received_fail_to_decrypt);
	linphone_core_cbs_set_new_message_reaction(mgr->cbs, reaction_received);
	linphone_core_cbs_set_reaction_removed(mgr->cbs, reaction_removed);
	linphone_core_cbs_set_messages_received(mgr->cbs, messages_received);
	linphone_core_cbs_set_is_composing_received(mgr->cbs, is_composing_received);
	linphone_core_cbs_set_new_subscription_requested(mgr->cbs, new_subscription_requested);
	linphone_core_cbs_set_notify_presence_received(mgr->cbs, notify_presence_received);
	linphone_core_cbs_set_notify_presence_received_for_uri_or_tel(mgr->cbs, notify_presence_received_for_uri_or_tel);
	linphone_core_cbs_set_transfer_state_changed(mgr->cbs, linphone_transfer_state_changed);
	linphone_core_cbs_set_info_received(mgr->cbs, info_message_received);
	linphone_core_cbs_set_subscription_state_changed(mgr->cbs, linphone_subscription_state_change);
	linphone_core_cbs_set_publish_state_changed(mgr->cbs, linphone_publish_state_changed);
	linphone_core_cbs_set_notify_received(mgr->cbs, linphone_notify_received);
	linphone_core_cbs_set_subscribe_received(mgr->cbs, linphone_subscribe_received);
	linphone_core_cbs_set_publish_received(mgr->cbs, linphone_publish_received);
	linphone_core_cbs_set_configuring_status(mgr->cbs, linphone_configuration_status);
	linphone_core_cbs_set_call_encryption_changed(mgr->cbs, linphone_call_encryption_changed);
	linphone_core_cbs_set_call_send_master_key_changed(mgr->cbs, linphone_call_send_master_key_changed);
	linphone_core_cbs_set_call_receive_master_key_changed(mgr->cbs, linphone_call_receive_master_key_changed);
	linphone_core_cbs_set_call_goclear_ack_sent(mgr->cbs, linphone_call_goclear_ack_sent);
	linphone_core_cbs_set_network_reachable(mgr->cbs, network_reachable);
	linphone_core_cbs_set_dtmf_received(mgr->cbs, dtmf_received);
	linphone_core_cbs_set_call_stats_updated(mgr->cbs, call_stats_updated);
	linphone_core_cbs_set_global_state_changed(mgr->cbs, global_state_changed);
	linphone_core_cbs_set_remaining_number_of_file_transfer_changed(
	    mgr->cbs, liblinphone_tester_remaining_number_of_file_transfer_changed);
	linphone_core_cbs_set_message_sent(mgr->cbs, liblinphone_tester_chat_room_msg_sent);
	linphone_core_cbs_set_first_call_started(mgr->cbs, first_call_started);
	linphone_core_cbs_set_last_call_ended(mgr->cbs, last_call_ended);
	linphone_core_cbs_set_audio_device_changed(mgr->cbs, audio_device_changed);
	linphone_core_cbs_set_audio_devices_list_updated(mgr->cbs, audio_devices_list_updated);
	linphone_core_cbs_set_imee_user_registration(mgr->cbs, liblinphone_tester_x3dh_user_created);
	linphone_core_cbs_set_conference_state_changed(mgr->cbs, core_conference_state_changed);
	linphone_core_cbs_set_default_account_changed(mgr->cbs, default_account_changed);
	linphone_core_cbs_set_account_added(mgr->cbs, account_added);
	linphone_core_cbs_set_account_removed(mgr->cbs, account_removed);

	mgr->phone_alias = phone_alias ? ms_strdup(phone_alias) : NULL;

	reset_counters(&mgr->stat);

	manager_count++;
}

void linphone_core_manager_init_with_db(LinphoneCoreManager *mgr,
                                        const char *rc_file,
                                        const char *phone_alias,
                                        const char *linphone_db,
                                        const char *lime_db,
                                        const char *zrtp_secrets_db) {
	linphone_core_manager_init2(mgr, rc_file, phone_alias);
	if (rc_file) mgr->rc_path = ms_strdup_printf("rcfiles/%s", rc_file);
	if (linphone_db == NULL && lime_db == NULL) {
		generate_random_database_path(mgr);
	} else {
		mgr->database_path = bctbx_strdup(linphone_db);
		mgr->lime_database_path = bctbx_strdup(lime_db);
		mgr->zrtp_secrets_database_path = ms_strdup(zrtp_secrets_db);
	}
	linphone_core_manager_configure(mgr);
}

void linphone_core_manager_init(LinphoneCoreManager *mgr, const char *rc_file, const char *phone_alias) {
	linphone_core_manager_init_with_db(mgr, rc_file, phone_alias, NULL, NULL, NULL);
}

void linphone_core_manager_init_shared(LinphoneCoreManager *mgr,
                                       const char *rc_file,
                                       const char *phone_alias,
                                       LinphoneCoreManager *mgr_to_copy) {
	linphone_core_manager_init2(mgr, rc_file, phone_alias);

	if (mgr_to_copy == NULL) {
		if (rc_file) mgr->rc_path = ms_strdup_printf("rcfiles/%s", rc_file);
		generate_random_database_path(mgr);
	} else {
		mgr->rc_path = ms_strdup(mgr_to_copy->rc_path);
		mgr->database_path = ms_strdup(mgr_to_copy->database_path);
		mgr->lime_database_path = ms_strdup(mgr_to_copy->lime_database_path);
		mgr->zrtp_secrets_database_path = ms_strdup(mgr_to_copy->zrtp_secrets_database_path);
	}
	linphone_core_manager_configure(mgr);
}
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif

static const char *end_of_uuid(const char *uuid) {
	size_t size = strlen(uuid);
	if (size > 4) size -= 4;
	else size = 0;
	return uuid + size;
}

void linphone_core_manager_start(LinphoneCoreManager *mgr, bool_t check_for_proxies) {
	LinphoneProxyConfig *proxy;
	int proxy_count;

	if (linphone_core_start(mgr->lc) == -1) {
		ms_error("Core [%p] failed to start", mgr->lc);
	}
	proxy = linphone_core_get_default_proxy_config(mgr->lc);
	if (proxy) {
		const char *label = linphone_core_get_label(mgr->lc);
		if (label && strchr(label, ';') == NULL) {
			/* suffix the uuid, that is known from linphone_core_start() was done.*/
			char *full_label = bctbx_strdup_printf(
			    "%s;%s", label,
			    end_of_uuid(linphone_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", "unknown")));
			linphone_core_set_label(mgr->lc, full_label);
			bctbx_free(full_label);
		}
	}

	/*BC_ASSERT_EQUAL(bctbx_list_size(linphone_core_get_proxy_config_list(lc)),proxy_count, int, "%d");*/
	int old_registration_failed = mgr->stat.number_of_LinphoneRegistrationFailed;
	int old_registration_ok = mgr->stat.number_of_LinphoneRegistrationOk;
	if (check_for_proxies) { /**/
		proxy_count = (int)bctbx_list_size(linphone_core_get_proxy_config_list(mgr->lc));
	} else {
		proxy_count = 0;
		/*this is to prevent registration to go on*/
		linphone_core_set_network_reachable(mgr->lc, FALSE);
	}

	if (proxy_count) {
		const bctbx_list_t *accounts = linphone_core_get_account_list(mgr->lc);
		stats mgr_stats = mgr->stat;
		int lime_enabled = 0;
		for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
			LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
			if (linphone_account_lime_enabled(account)) {
				lime_enabled++;
			}
		}
		if (lime_enabled != 0 && !mgr->skip_lime_user_creation_asserts) {
			// Wait for lime users to be created on X3DH server
			if (mgr->lime_failure) {
				BC_ASSERT_TRUE(wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_X3dhUserCreationFailure,
				                              mgr_stats.number_of_X3dhUserCreationFailure + 1,
				                              x3dhServer_creationTimeout));
			} else {
				BC_ASSERT_TRUE(wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_X3dhUserCreationSuccess,
				                              mgr_stats.number_of_X3dhUserCreationSuccess + lime_enabled,
				                              x3dhServer_creationTimeout));
			}
			if (lime_enabled == 1) {
				// At most the number of accounts that don't have lime enabled were able to register
				// This assert is reliable only if there is one account, since the process of
				// lime user creation followed by registration happens in parallel for the two accounts,
				// not sequentially.
				BC_ASSERT_LOWER(mgr->stat.number_of_LinphoneRegistrationOk,
				                old_registration_ok + (proxy_count - lime_enabled), int, "%d");
			}
		}
#define REGISTER_TIMEOUT 20 /* seconds */
		int success = 0;
		if (mgr->registration_failure) {
			wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationFailed,
			               old_registration_failed + proxy_count, (REGISTER_TIMEOUT * 1000 * proxy_count));
		} else {
			success = wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationOk,
			                         old_registration_ok + proxy_count, (REGISTER_TIMEOUT * 1000 * proxy_count));
		}
		if (!success) {
			ms_error("Did not register after %d seconds for %d proxies", REGISTER_TIMEOUT, proxy_count);
		}
	}
	if (mgr->registration_failure) {
		BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneRegistrationFailed, old_registration_failed + proxy_count, int,
		                "%d");
	} else {
		BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneRegistrationOk, old_registration_ok + proxy_count, int, "%d");
	}
	enable_codec(mgr->lc, "PCMU", 8000);

	if (proxy) {
		if (mgr->identity) {
			linphone_address_unref(mgr->identity);
		}
		mgr->identity = linphone_address_clone(linphone_proxy_config_get_identity_address(proxy));
		linphone_address_clean(mgr->identity);
	}

	linphone_core_manager_wait_for_stun_resolution(mgr);
	if (!check_for_proxies) {
		/*now that stun server resolution is done, we can start registering*/
		linphone_core_set_network_reachable(mgr->lc, TRUE);
	} else
		wait_for_until(mgr->lc, NULL, NULL, 0,
		               20); // process events that are in main queue (fix: DNS timeout when loading linphone database)
}

LinphoneCoreManager *linphone_core_manager_create2(const char *rc_file, const char *phone_alias) {
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(manager, rc_file, phone_alias);
	return manager;
}

LinphoneCoreManager *linphone_core_manager_create(const char *rc_file) {
	return linphone_core_manager_create2(rc_file, NULL);
}

LinphoneCoreManager *linphone_core_manager_new4(
    const char *rc_file, int check_for_proxies, const char *phone_alias, const char *contact_params, int expires) {
	/* This function is for testing purposes. */
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);

	linphone_core_manager_init(manager, rc_file, phone_alias);
	LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(manager->lc);
	linphone_proxy_config_edit(config);
	linphone_proxy_config_set_contact_parameters(config, contact_params);
	linphone_proxy_config_set_expires(config, expires);
	linphone_proxy_config_done(config);
	linphone_core_manager_start(manager, check_for_proxies);
	return manager;
}

LinphoneCoreManager *
linphone_core_manager_new3(const char *rc_file, bool_t check_for_proxies, const char *phone_alias) {
	LinphoneCoreManager *manager = linphone_core_manager_create2(rc_file, phone_alias);
	linphone_core_manager_start(manager, check_for_proxies);
	return manager;
}

LinphoneCoreManager *linphone_core_manager_new_with_proxies_check(const char *rc_file, bool_t check_for_proxies) {
	return linphone_core_manager_new3(rc_file, check_for_proxies, NULL);
}

LinphoneCoreManager *linphone_core_manager_new(const char *rc_file) {
	return linphone_core_manager_new_with_proxies_check(rc_file, TRUE);
}

void linphone_core_start_process_remote_notification(LinphoneCoreManager *mgr, const char *callid) {
	LinphoneCall *incomingCall = linphone_core_get_call_by_callid(mgr->lc, callid);
	if (!incomingCall) {
		incomingCall = linphone_call_new_incoming_with_callid(mgr->lc, callid);
		linphone_call_start_basic_incoming_notification(incomingCall);
		linphone_call_start_push_incoming_notification(incomingCall);
		linphone_core_process_push_notification(mgr->lc, callid);
	}
}

/* same as new but insert the rc_local in the core manager before the init and provide path to db files */
LinphoneCoreManager *linphone_core_manager_create_local(const char *rc_factory,
                                                        const char *rc_local,
                                                        const char *linphone_db,
                                                        const char *lime_db,
                                                        const char *zrtp_secrets_db) {
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);
	if (manager->rc_local) {
		bctbx_free(manager->rc_local);
		manager->rc_local = NULL;
	}
	manager->rc_local = bctbx_strdup(rc_local);
	linphone_core_manager_init_with_db(manager, rc_factory, NULL, linphone_db, lime_db, zrtp_secrets_db);
	return manager;
}

LinphoneCoreManager *linphone_core_manager_new_local(const char *rc_factory,
                                                     const char *rc_local,
                                                     const char *linphone_db,
                                                     const char *lime_db,
                                                     const char *zrtp_secrets_db) {
	LinphoneCoreManager *manager =
	    linphone_core_manager_create_local(rc_factory, rc_local, linphone_db, lime_db, zrtp_secrets_db);
	linphone_core_manager_start(manager, TRUE);

	return manager;
}

/**
 * Create a LinphoneCoreManager that holds a shared Core.
 * mgr_to_copy is used to create a second LinphoneCoreManager with the same identity.
 * If mgr_to_copy has a value, rc_file parameter is ignored.
 */
LinphoneCoreManager *linphone_core_manager_create_shared(const char *rc_file,
                                                         const char *app_group_id,
                                                         bool_t main_core,
                                                         LinphoneCoreManager *mgr_to_copy) {
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);
	manager->app_group_id = ms_strdup(app_group_id);
	manager->main_core = main_core;
	linphone_core_manager_init_shared(manager, rc_file, NULL, mgr_to_copy);
	return manager;
}

void linphone_core_manager_skip_lime_user_creation_asserts(LinphoneCoreManager *mgr, bool_t value) {
	mgr->skip_lime_user_creation_asserts = value;
}

void linphone_core_manager_expect_lime_failure(LinphoneCoreManager *mgr, bool_t value) {
	mgr->lime_failure = value;
}

static void check_orphan_nat_policy_section(LinphoneCoreManager *mgr) {
	bctbx_list_t *l = linphone_config_get_sections_names_list(linphone_core_get_config(mgr->lc));
	bctbx_list_t *elem;
	int nat_policy_count = 0;
	int proxy_count = 0;

	for (elem = l; elem != NULL; elem = elem->next) {
		const char *name = (const char *)elem->data;
		if (strstr(name, "nat_policy_") == name) {
			nat_policy_count++;
		} else if (strstr(name, "proxy_") == name) {
			proxy_count++;
		}
	}
	/* There can't be more nat_policy_ section than proxy_? section + 1 */
	BC_ASSERT_LOWER(nat_policy_count, proxy_count + 1, int, "%i");
	bctbx_list_free(l);
}

void linphone_core_manager_stop(LinphoneCoreManager *mgr) {
	if (mgr->lc) {
		int previousNbRegistrationCleared = mgr->stat.number_of_LinphoneRegistrationCleared;
		const char *record_file = linphone_core_get_record_file(mgr->lc);
		size_t accountsCount = bctbx_list_size(linphone_core_get_account_list(mgr->lc));
		if (!liblinphone_tester_keep_record_files && record_file && bctbx_file_exist(record_file) == 0) {
			if ((bc_get_number_of_failures() - mgr->number_of_bcunit_error_at_creation) > 0) {
				ms_error("Test has failed, keeping recorded file [%s]", record_file);
			} else {
				unlink(record_file);
			}
		}

		linphone_core_stop(mgr->lc);
		if (accountsCount > 0) {
			wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationCleared,
			               previousNbRegistrationCleared + 1, 2000);
		}
		check_orphan_nat_policy_section(mgr);
		linphone_core_unref(mgr->lc);
		mgr->lc = NULL;
	}
}

void linphone_core_manager_uninit_after_stop_async(LinphoneCoreManager *mgr) {
	if (mgr->lc) {
		const char *record_file = linphone_core_get_record_file(mgr->lc);
		if (!liblinphone_tester_keep_record_files && record_file && bctbx_file_exist(record_file) == 0) {
			if ((bc_get_number_of_failures() - mgr->number_of_bcunit_error_at_creation) > 0) {
				ms_error("Test has failed, keeping recorded file [%s]", record_file);
			} else {
				unlink(record_file);
			}
		}
		linphone_core_unref(mgr->lc);
		mgr->lc = NULL;
	}
}

void linphone_core_manager_reinit(LinphoneCoreManager *mgr) {
	char *uuid = NULL;
	if (mgr->lc) {
		if (linphone_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", NULL))
			uuid = bctbx_strdup(linphone_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", NULL));
		linphone_core_set_network_reachable(mgr->lc, FALSE); // to avoid unregister
		linphone_core_stop(mgr->lc);
		linphone_core_unref(mgr->lc);
		mgr->lc = NULL;
	}
	linphone_core_manager_configure(mgr);
	reset_counters(&mgr->stat);
	// Make sure gruu is preserved
	linphone_config_set_string(linphone_core_get_config(mgr->lc), "misc", "uuid", uuid);
	if (uuid) bctbx_free(uuid);
}

void linphone_core_manager_restart(LinphoneCoreManager *mgr, bool_t check_for_proxies) {
	linphone_core_manager_reinit(mgr);
	linphone_core_manager_start(mgr, check_for_proxies);
}

void linphone_core_manager_uninit2(LinphoneCoreManager *mgr, bool_t unlinkDb, bool_t unlinkRc) {
	if (mgr->phone_alias) {
		ms_free(mgr->phone_alias);
	}
	if (mgr->identity) {
		linphone_address_unref(mgr->identity);
	}
	if (mgr->rc_path) bctbx_free(mgr->rc_path);

	if (mgr->database_path) {
		if (unlinkDb == TRUE) {
			unlink(mgr->database_path);
		}
		bc_free(mgr->database_path);
	}
	if (mgr->lime_database_path) {
		if (unlinkDb == TRUE) {
			unlink(mgr->lime_database_path);
		}
		bc_free(mgr->lime_database_path);
	}
	if (mgr->zrtp_secrets_database_path) {
		if (unlinkDb == TRUE) {
			unlink(mgr->zrtp_secrets_database_path);
		}
		bc_free(mgr->zrtp_secrets_database_path);
	}
	if (mgr->app_group_id) bctbx_free(mgr->app_group_id);

	if (mgr->cbs) linphone_core_cbs_unref(mgr->cbs);

	if (mgr->rc_local) {
		if (unlinkRc == TRUE) {
			unlink(mgr->rc_local);
		}
		bctbx_free(mgr->rc_local);
		mgr->rc_local = NULL;
	}

	reset_counters(&mgr->stat);

	manager_count--;
}
void linphone_core_manager_uninit(LinphoneCoreManager *mgr) {
	linphone_core_manager_uninit2(mgr, TRUE, TRUE);
}

void linphone_core_manager_wait_for_stun_resolution(BCTBX_UNUSED(LinphoneCoreManager *mgr)) {
	/* This function is no longer needed, the core is able to perform stun server resolution asynchronously before at
	 * call setup.*/
}

void linphone_core_manager_uninit3(LinphoneCoreManager *mgr) {
	if (mgr->lc && linphone_core_get_global_state(mgr->lc) != LinphoneGlobalOff &&
	    !linphone_core_is_network_reachable(mgr->lc)) {
		int previousNbRegistrationOk = mgr->stat.number_of_LinphoneRegistrationOk;
		linphone_core_set_network_reachable(mgr->lc, TRUE);
		wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
	}
	linphone_core_manager_stop(mgr);
	linphone_core_manager_uninit(mgr);
}

void linphone_core_manager_destroy(LinphoneCoreManager *mgr) {
	linphone_core_manager_uninit3(mgr);
	ms_free(mgr);
}

void linphone_core_manager_destroy_after_stop_async(LinphoneCoreManager *mgr) {
	if (mgr->lc && !linphone_core_is_network_reachable(mgr->lc)) {
		int previousNbRegistrationOk = mgr->stat.number_of_LinphoneRegistrationOk;
		linphone_core_set_network_reachable(mgr->lc, TRUE);
		wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
	}
	linphone_core_manager_uninit_after_stop_async(mgr);
	linphone_core_manager_uninit(mgr);
	ms_free(mgr);
}

void linphone_core_manager_delete_chat_room(LinphoneCoreManager *mgr, LinphoneChatRoom *cr, bctbx_list_t *coresList) {
	if (cr) {
		const LinphoneAddress *cr_conference_address = linphone_chat_room_get_conference_address(cr);
		char *cr_conference_address_str =
		    cr_conference_address ? linphone_address_as_string(cr_conference_address) : ms_strdup("sip:");
		ms_message("Core %s is trying to delete chat room %p (address %s)", linphone_core_get_identity(mgr->lc), cr,
		           cr_conference_address_str);
		ms_free(cr_conference_address_str);
		stats mgrStats = mgr->stat;
		linphone_core_delete_chat_room(mgr->lc, cr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateDeleted,
		                             mgrStats.number_of_LinphoneChatRoomStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));
	}
}

int liblinphone_tester_ipv6_available(void) {
	if (liblinphonetester_ipv6) {
		struct addrinfo *ai =
		    bctbx_ip_address_to_addrinfo(AF_INET6, SOCK_STREAM, liblinphone_tester_ipv6_probing_address, 53);
		if (ai) {
			struct sockaddr_storage ss;
			struct addrinfo src;
			socklen_t slen = sizeof(ss);
			char localip[128];
			int port = 0;
			belle_sip_get_src_addr_for(ai->ai_addr, (socklen_t)ai->ai_addrlen, (struct sockaddr *)&ss, &slen, 4444);
			src.ai_addr = (struct sockaddr *)&ss;
			src.ai_addrlen = slen;
			bctbx_addrinfo_to_ip_address(&src, localip, sizeof(localip), &port);
			freeaddrinfo(ai);
			return strcmp(localip, "::1") != 0;
		}
	}
	return FALSE;
}

int liblinphone_tester_ipv4_available(void) {
	struct addrinfo *ai = bctbx_ip_address_to_addrinfo(AF_INET, SOCK_STREAM, "212.27.40.240", 53);
	if (ai) {
		struct sockaddr_storage ss;
		struct addrinfo src;
		socklen_t slen = sizeof(ss);
		char localip[128];
		int port = 0;
		belle_sip_get_src_addr_for(ai->ai_addr, (socklen_t)ai->ai_addrlen, (struct sockaddr *)&ss, &slen, 4444);
		src.ai_addr = (struct sockaddr *)&ss;
		src.ai_addrlen = slen;
		bctbx_addrinfo_to_ip_address(&src, localip, sizeof(localip), &port);
		freeaddrinfo(ai);
		return strcmp(localip, "127.0.0.1") != 0;
	}
	return FALSE;
}

char *liblinphone_tester_make_unique_file_path(const char *name, const char *extension) {
	char token[10] = {0};
	char *filename;
	char *path;
	char *ret;

	belle_sip_random_token(token, sizeof(token));

	filename = bctbx_strdup_printf("%s-%s.%s", name, token, extension);
	path = bc_tester_file(filename);
	ret = bctbx_strdup(path);
	bctbx_free(filename);
	bc_free(path);
	return ret;
}

void liblinphone_tester_keep_accounts(int keep) {
	liblinphone_tester_keep_accounts_flag = keep;
}

void liblinphone_tester_keep_recorded_files(int keep) {
	liblinphone_tester_keep_record_files = keep;
}

void liblinphone_tester_disable_leak_detector(int disabled) {
	liblinphone_tester_leak_detector_disabled = disabled;
}

void liblinphone_tester_clear_accounts(void) {
	account_manager_destroy();
}

static int linphone_core_manager_get_max_audio_bw_base(const int array[], int array_size) {
	int i, result = 0;
	for (i = 0; i < array_size; i++) {
		result = MAX(result, array[i]);
	}
	return result;
}

static int linphone_core_manager_get_mean_audio_bw_base(const int array[], int array_size) {
	int i, result = 0;
	for (i = 0; i < array_size; i++) {
		result += array[i];
	}
	return result / array_size;
}

int linphone_core_manager_get_max_audio_down_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_max_audio_bw_base(mgr->stat.audio_download_bandwidth,
	                                                   sizeof(mgr->stat.audio_download_bandwidth) / sizeof(int));
}
int linphone_core_manager_get_max_audio_up_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_max_audio_bw_base(mgr->stat.audio_upload_bandwidth,
	                                                   sizeof(mgr->stat.audio_upload_bandwidth) / sizeof(int));
}

int linphone_core_manager_get_mean_audio_down_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_mean_audio_bw_base(mgr->stat.audio_download_bandwidth,
	                                                    sizeof(mgr->stat.audio_download_bandwidth) / sizeof(int));
}
int linphone_core_manager_get_mean_audio_up_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_mean_audio_bw_base(mgr->stat.audio_upload_bandwidth,
	                                                    sizeof(mgr->stat.audio_upload_bandwidth) / sizeof(int));
}

void liblinphone_tester_before_each(void) {
	const char *mediastreamer2_plugin_dir = MEDIASTREAMER2_LOCAL_PLUGINS_LOCATION;
	if (bctbx_file_exist(mediastreamer2_plugin_dir) == 0) {
		linphone_factory_set_msplugins_dir(linphone_factory_get(), mediastreamer2_plugin_dir);
	}
	const char *liblinphone_plugin_dir = LIBLINPHONE_LOCAL_PLUGINS_LOCATION;
	if (bctbx_file_exist(liblinphone_plugin_dir) == 0) {
		linphone_factory_set_liblinphone_plugins_dir(linphone_factory_get(), liblinphone_plugin_dir);
	}

	if (!liblinphone_tester_leak_detector_disabled) {
		belle_sip_object_enable_leak_detector(TRUE);
		leaked_objects_count = belle_sip_object_get_object_count();
	}
}

static char *all_leaks_buffer = NULL;

void liblinphone_tester_after_each(void) {
	linphone_factory_clean();
	if (!liblinphone_tester_leak_detector_disabled) {
		int leaked_objects = belle_sip_object_get_object_count() - leaked_objects_count;
		if (leaked_objects > 0) {
			char *format = ms_strdup_printf("%d object%s leaked in suite [%s] test [%s], please fix that!",
			                                leaked_objects, leaked_objects > 1 ? "s were" : " was",
			                                bc_tester_current_suite_name(), bc_tester_current_test_name());
			belle_sip_object_dump_active_objects();
			belle_sip_object_flush_active_objects();
			bc_tester_printf(ORTP_MESSAGE, "%s", format);
			ms_error("%s", format);

			all_leaks_buffer = ms_strcat_printf(all_leaks_buffer, "\n%s", format);
			ms_free(format);
		}

		// prevent any future leaks
		{
			const char **tags = bc_tester_current_test_tags();
			int leaks_expected = (tags && ((tags[0] && !strcmp(tags[0], "LeaksMemory")) ||
			                               (tags[1] && !strcmp(tags[1], "LeaksMemory"))));
			// if the test is NOT marked as leaking memory and it actually is, we should make it fail
			if (!leaks_expected && leaked_objects > 0) {
				BC_FAIL("This test is leaking memory!");
				// and reciprocally
			} else if (leaks_expected && leaked_objects == 0) {
				BC_FAIL("This test is not leaking anymore, please remove LeaksMemory tag!");
			}
		}
	}

	if (manager_count != 0) {
		ms_fatal("%d Linphone core managers are still alive!", manager_count);
	}
}

void liblinphone_tester_uninit(void) {
	// show all leaks that happened during the test
	if (all_leaks_buffer) {
		bc_tester_printf(ORTP_MESSAGE, all_leaks_buffer);
		ms_free(all_leaks_buffer);
		all_leaks_buffer = NULL;
	}
	bc_tester_uninit();
	bctbx_uninit_logger();
}

void compare_files(const char *path1, const char *path2) {
	uint8_t *buf1;
	uint8_t *buf2;

	bctbx_vfs_file_t *f1 = bctbx_file_open(bctbx_vfs_get_default(), path1, "r");
	bctbx_vfs_file_t *f2 = bctbx_file_open(bctbx_vfs_get_default(), path2, "r");
	BC_ASSERT_PTR_NOT_NULL(f1);
	BC_ASSERT_PTR_NOT_NULL(f2);
	if (f1 == NULL || f2 == NULL) return;

	ssize_t s1 = bctbx_file_size(f1);
	ssize_t s2 = bctbx_file_size(f2);

	BC_ASSERT_EQUAL((long)s2, (long)s1, long, "%ld");
	BC_ASSERT_TRUE(s1 != BCTBX_VFS_ERROR);
	if (s1 != s2 || s1 == BCTBX_VFS_ERROR) return;
	ssize_t fileSize = (ssize_t)s1;

	buf1 = bctbx_malloc(fileSize);
	buf2 = bctbx_malloc(fileSize);

	BC_ASSERT_TRUE(bctbx_file_read(f1, buf1, (size_t)fileSize, 0) == fileSize);
	BC_ASSERT_TRUE(bctbx_file_read(f2, buf2, (size_t)fileSize, 0) == fileSize);

	bctbx_file_close(f1);
	bctbx_file_close(f2);

	if (buf1 && buf2) {
		BC_ASSERT_EQUAL(memcmp(buf1, buf2, (size_t)fileSize), 0, int, "%d");
	}

	if (buf1) ms_free(buf1);
	if (buf2) ms_free(buf2);
}

void registration_state_changed(struct _LinphoneCore *lc,
                                LinphoneProxyConfig *cfg,
                                LinphoneRegistrationState cstate,
                                BCTBX_UNUSED(const char *message)) {
	stats *counters;
	ms_message("New registration state %s for user id [%s] at proxy [%s]\n",
	           linphone_registration_state_to_string(cstate), linphone_proxy_config_get_identity(cfg),
	           linphone_proxy_config_get_addr(cfg));
	counters = get_stats(lc);
	switch (cstate) {
		case LinphoneRegistrationNone:
			counters->number_of_LinphoneRegistrationNone++;
			break;
		case LinphoneRegistrationProgress:
			counters->number_of_LinphoneRegistrationProgress++;
			break;
		case LinphoneRegistrationRefreshing:
			counters->number_of_LinphoneRegistrationRefreshing++;
			break;
		case LinphoneRegistrationOk:
			counters->number_of_LinphoneRegistrationOk++;
			break;
		case LinphoneRegistrationCleared:
			counters->number_of_LinphoneRegistrationCleared++;
			break;
		case LinphoneRegistrationFailed:
			counters->number_of_LinphoneRegistrationFailed++;
			break;
		default:
			BC_FAIL("unexpected event");
			break;
	}
}

void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, BCTBX_UNUSED(const char *msg)) {
	stats *counters = get_stats(lc);

	if (linphone_call_is_op_configured(call)) {
		LinphoneCallLog *calllog = linphone_call_get_call_log(call);
		char *to = linphone_address_as_string(linphone_call_log_get_to_address(calllog));
		char *from = linphone_address_as_string(linphone_call_log_get_from_address(calllog));

		ms_message(" %s call %p from [%s] to [%s], new state is [%s]",
		           linphone_call_log_get_dir(calllog) == LinphoneCallIncoming ? "Incoming" : "Outgoing", call, from, to,
		           linphone_call_state_to_string(cstate));

		ms_free(to);
		ms_free(from);
	}

	switch (cstate) {
		case LinphoneCallIncomingReceived:
			counters->number_of_LinphoneCallIncomingReceived++;
			break;
		case LinphoneCallPushIncomingReceived:
			counters->number_of_LinphoneCallPushIncomingReceived++;
			break;
		case LinphoneCallOutgoingInit:
			counters->number_of_LinphoneCallOutgoingInit++;
			break;
		case LinphoneCallOutgoingProgress:
			counters->number_of_LinphoneCallOutgoingProgress++;
			break;
		case LinphoneCallOutgoingRinging:
			counters->number_of_LinphoneCallOutgoingRinging++;
			break;
		case LinphoneCallOutgoingEarlyMedia:
			counters->number_of_LinphoneCallOutgoingEarlyMedia++;
			break;
		case LinphoneCallConnected:
			counters->number_of_LinphoneCallConnected++;
			break;
		case LinphoneCallStreamsRunning:
			counters->number_of_LinphoneCallStreamsRunning++;
			break;
		case LinphoneCallPausing:
			counters->number_of_LinphoneCallPausing++;
			break;
		case LinphoneCallPaused:
			counters->number_of_LinphoneCallPaused++;
			break;
		case LinphoneCallResuming:
			counters->number_of_LinphoneCallResuming++;
			break;
		case LinphoneCallRefered:
			counters->number_of_LinphoneCallRefered++;
			break;
		case LinphoneCallError:
			counters->number_of_LinphoneCallError++;
			break;
		case LinphoneCallEnd:
			counters->number_of_LinphoneCallEnd++;
			break;
		case LinphoneCallPausedByRemote:
			counters->number_of_LinphoneCallPausedByRemote++;
			break;
		case LinphoneCallUpdatedByRemote:
			counters->number_of_LinphoneCallUpdatedByRemote++;
			break;
		case LinphoneCallIncomingEarlyMedia:
			counters->number_of_LinphoneCallIncomingEarlyMedia++;
			break;
		case LinphoneCallUpdating:
			counters->number_of_LinphoneCallUpdating++;
			break;
		case LinphoneCallReleased:
			counters->number_of_LinphoneCallReleased++;
			break;
		case LinphoneCallEarlyUpdating:
			counters->number_of_LinphoneCallEarlyUpdating++;
			break;
		case LinphoneCallEarlyUpdatedByRemote:
			counters->number_of_LinphoneCallEarlyUpdatedByRemote++;
			break;
		default:
			BC_FAIL("unexpected event");
			break;
	}
}

void messages_received(LinphoneCore *lc, BCTBX_UNUSED(LinphoneChatRoom *room), const bctbx_list_t *messages) {
	stats *counters;
	counters = get_stats(lc);
	int count = (int)bctbx_list_size(messages);
	counters->number_of_LinphoneAggregatedMessagesReceived += count;
	ms_message("Received %0d aggregated messages", count);
}

void message_received(LinphoneCore *lc, BCTBX_UNUSED(LinphoneChatRoom *room), LinphoneChatMessage *msg) {
	char *from = linphone_address_as_string(linphone_chat_message_get_from_address(msg));
	stats *counters;
	const char *text = linphone_chat_message_get_text(msg);
	const char *external_body_url = linphone_chat_message_get_external_body_url(msg);
	ms_message("Message from [%s] is [%s] , external URL [%s]", from ? from : "", text ? text : "",
	           external_body_url ? external_body_url : "");
	ms_free(from);
	counters = get_stats(lc);
	counters->number_of_LinphoneMessageReceived++;
	if (counters->last_received_chat_message) {
		linphone_chat_message_unref(counters->last_received_chat_message);
	}

	counters->last_received_chat_message = linphone_chat_message_ref(msg);
	LinphoneContent *content = linphone_chat_message_get_file_transfer_information(msg);
	if (content) counters->number_of_LinphoneMessageReceivedWithFile++;
	else if (linphone_chat_message_get_external_body_url(msg)) {
		counters->number_of_LinphoneMessageExtBodyReceived++;
		if (message_external_body_url) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_external_body_url(msg), message_external_body_url);
			message_external_body_url = NULL;
		}
	}

	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_new_message_reaction(cbs, liblinphone_tester_chat_message_reaction_received);

	if (linphone_config_get_bool(linphone_core_get_config(lc), "net", "bad_net", 0)) {
		sal_set_send_error(linphone_core_get_sal(lc), 1500);
	}
}

void message_received_fail_to_decrypt(LinphoneCore *lc,
                                      BCTBX_UNUSED(LinphoneChatRoom *room),
                                      LinphoneChatMessage *msg) {
	char *from = linphone_address_as_string(linphone_chat_message_get_from_address(msg));
	stats *counters;
	ms_message("Failed to decrypt message from [%s]", from ? from : "");
	ms_free(from);
	counters = get_stats(lc);
	counters->number_of_LinphoneMessageReceivedFailedToDecrypt++;
	if (counters->last_fail_to_decrypt_received_chat_message) {
		linphone_chat_message_unref(counters->last_fail_to_decrypt_received_chat_message);
	}
	counters->last_fail_to_decrypt_received_chat_message = linphone_chat_message_ref(msg);
}

void reaction_received(LinphoneCore *lc,
                       BCTBX_UNUSED(LinphoneChatRoom *room),
                       LinphoneChatMessage *msg,
                       const LinphoneChatMessageReaction *reaction) {
	const LinphoneAddress *address = linphone_chat_message_reaction_get_from_address(reaction);
	const char *body = linphone_chat_message_reaction_get_body(reaction);
	char *from = linphone_address_as_string(address);
	char *from2 = linphone_address_as_string(linphone_chat_message_get_from_address(msg));
	const char *text = linphone_chat_message_get_text(msg);
	ms_message("Reaction [%s] from [%s] for message [%s] sent by [%s]", body, from, text, from2);
	ms_free(from);
	ms_free(from2);

	stats *counters = get_stats(lc);
	counters->number_of_LinphoneReactionSentOrReceived++;
}

void reaction_removed(LinphoneCore *lc,
                      BCTBX_UNUSED(LinphoneChatRoom *room),
                      LinphoneChatMessage *msg,
                      const LinphoneAddress *address) {
	char *from = linphone_address_as_string(address);
	const char *text = linphone_chat_message_get_text(msg);
	ms_message("Reaction sent by [%s] for message [%s] has been removed", from, text);
	bctbx_free(from);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneReactionRemoved++;
}

void is_composing_received(LinphoneCore *lc, LinphoneChatRoom *room) {
	stats *counters = get_stats(lc);
	if (linphone_chat_room_is_remote_composing(room)) {
		counters->number_of_LinphoneIsComposingActiveReceived++;
	} else {
		counters->number_of_LinphoneIsComposingIdleReceived++;
	}
}

void new_subscription_requested(LinphoneCore *lc, LinphoneFriend *lf, const char *url) {
	stats *counters;
	const LinphoneAddress *addr = linphone_friend_get_address(lf);
	struct addrinfo *ai;
	const char *domain;
	char *ipaddr;

	if (addr != NULL) {
		char *from = linphone_address_as_string(addr);
		ms_message("New subscription request from [%s] url [%s]", from, url);
		ms_free(from);
	}
	counters = get_stats(lc);
	counters->number_of_NewSubscriptionRequest++;

	domain = linphone_address_get_domain(addr);
	if (domain[0] == '[') {
		ipaddr = ms_strdup(domain + 1);
		ipaddr[strlen(ipaddr)] = '\0';
	} else ipaddr = ms_strdup(domain);
	ai = bctbx_ip_address_to_addrinfo(strchr(domain, ':') != NULL ? AF_INET6 : AF_INET, SOCK_DGRAM, ipaddr, 4444);
	ms_free(ipaddr);
	if (ai) { /* this SUBSCRIBE comes from friend without registered SIP account, don't attempt to subscribe, it will
		         fail*/
		ms_message("Disabling subscription because friend has numeric host.");
		linphone_friend_enable_subscribes(lf, FALSE);
		bctbx_freeaddrinfo(ai);
	} else linphone_friend_enable_subscribes(lf, TRUE);

	linphone_core_add_friend(lc, lf); /*accept subscription*/
}

void notify_friend_presence_received(LinphoneFriend *lf) {
	ms_message("Presence received for friend [%p (%s)]", lf, linphone_friend_get_name(lf));
	LinphoneCore *lc = linphone_friend_get_core(lf);
	stats *counters = get_stats(lc);
	counters->number_of_NotifyFriendPresenceReceived++;
}

void notify_presence_received(LinphoneCore *lc, LinphoneFriend *lf) {
	stats *counters;
	unsigned int i;
	const LinphoneAddress *addr = linphone_friend_get_address(lf);
	if (addr != NULL) {
		char *from = linphone_address_as_string(addr);
		ms_message("New Notify request from [%s] ", from);
		ms_free(from);
	}
	counters = get_stats(lc);
	counters->number_of_NotifyPresenceReceived++;
	counters->last_received_presence = linphone_friend_get_presence_model(lf);
	if (linphone_presence_model_get_basic_status(counters->last_received_presence) == LinphonePresenceBasicStatusOpen) {
		counters->number_of_LinphonePresenceBasicStatusOpen++;
	} else if (linphone_presence_model_get_basic_status(counters->last_received_presence) ==
	           LinphonePresenceBasicStatusClosed) {
		counters->number_of_LinphonePresenceBasicStatusClosed++;
	} else {
		ms_error("Unexpected basic status [%i]",
		         linphone_presence_model_get_basic_status(counters->last_received_presence));
	}

	LinphoneConsolidatedPresence consolidated_presence = linphone_friend_get_consolidated_presence(lf);
	if (consolidated_presence == LinphoneConsolidatedPresenceOnline) {
		counters->number_of_LinphoneConsolidatedPresenceOnline++;
	} else if (consolidated_presence == LinphoneConsolidatedPresenceBusy) {
		counters->number_of_LinphoneConsolidatedPresenceBusy++;
	} else if (consolidated_presence == LinphoneConsolidatedPresenceDoNotDisturb) {
		counters->number_of_LinphoneConsolidatedPresenceDoNotDisturb++;
	} else if (consolidated_presence == LinphoneConsolidatedPresenceOffline) {
		counters->number_of_LinphoneConsolidatedPresenceOffline++;
	}

	if (counters->last_received_presence &&
	    linphone_presence_model_get_nb_activities(counters->last_received_presence) > 0) {
		for (i = 0; counters->last_received_presence &&
		            i < linphone_presence_model_get_nb_activities(counters->last_received_presence);
		     i++) {
			LinphonePresenceActivity *activity =
			    linphone_presence_model_get_nth_activity(counters->last_received_presence, i);
			switch (linphone_presence_activity_get_type(activity)) {
				case LinphonePresenceActivityAppointment:
					counters->number_of_LinphonePresenceActivityAppointment++;
					break;
				case LinphonePresenceActivityAway:
					counters->number_of_LinphonePresenceActivityAway++;
					break;
				case LinphonePresenceActivityBreakfast:
					counters->number_of_LinphonePresenceActivityBreakfast++;
					break;
				case LinphonePresenceActivityBusy:
					counters->number_of_LinphonePresenceActivityBusy++;
					break;
				case LinphonePresenceActivityDinner:
					counters->number_of_LinphonePresenceActivityDinner++;
					break;
				case LinphonePresenceActivityHoliday:
					counters->number_of_LinphonePresenceActivityHoliday++;
					break;
				case LinphonePresenceActivityInTransit:
					counters->number_of_LinphonePresenceActivityInTransit++;
					break;
				case LinphonePresenceActivityLookingForWork:
					counters->number_of_LinphonePresenceActivityLookingForWork++;
					break;
				case LinphonePresenceActivityLunch:
					counters->number_of_LinphonePresenceActivityLunch++;
					break;
				case LinphonePresenceActivityMeal:
					counters->number_of_LinphonePresenceActivityMeal++;
					break;
				case LinphonePresenceActivityMeeting:
					counters->number_of_LinphonePresenceActivityMeeting++;
					break;
				case LinphonePresenceActivityOnThePhone:
					counters->number_of_LinphonePresenceActivityOnThePhone++;
					break;
				case LinphonePresenceActivityOther:
					counters->number_of_LinphonePresenceActivityOther++;
					break;
				case LinphonePresenceActivityPerformance:
					counters->number_of_LinphonePresenceActivityPerformance++;
					break;
				case LinphonePresenceActivityPermanentAbsence:
					counters->number_of_LinphonePresenceActivityPermanentAbsence++;
					break;
				case LinphonePresenceActivityPlaying:
					counters->number_of_LinphonePresenceActivityPlaying++;
					break;
				case LinphonePresenceActivityPresentation:
					counters->number_of_LinphonePresenceActivityPresentation++;
					break;
				case LinphonePresenceActivityShopping:
					counters->number_of_LinphonePresenceActivityShopping++;
					break;
				case LinphonePresenceActivitySleeping:
					counters->number_of_LinphonePresenceActivitySleeping++;
					break;
				case LinphonePresenceActivitySpectator:
					counters->number_of_LinphonePresenceActivitySpectator++;
					break;
				case LinphonePresenceActivitySteering:
					counters->number_of_LinphonePresenceActivitySteering++;
					break;
				case LinphonePresenceActivityTravel:
					counters->number_of_LinphonePresenceActivityTravel++;
					break;
				case LinphonePresenceActivityTV:
					counters->number_of_LinphonePresenceActivityTV++;
					break;
				case LinphonePresenceActivityUnknown:
					counters->number_of_LinphonePresenceActivityUnknown++;
					break;
				case LinphonePresenceActivityVacation:
					counters->number_of_LinphonePresenceActivityVacation++;
					break;
				case LinphonePresenceActivityWorking:
					counters->number_of_LinphonePresenceActivityWorking++;
					break;
				case LinphonePresenceActivityWorship:
					counters->number_of_LinphonePresenceActivityWorship++;
					break;
			}
		}
	} else {
		if (linphone_presence_model_get_basic_status(counters->last_received_presence) ==
		    LinphonePresenceBasicStatusOpen)
			counters->number_of_LinphonePresenceActivityOnline++;
		else counters->number_of_LinphonePresenceActivityOffline++;
	}
}

void notify_presence_received_for_uri_or_tel(LinphoneCore *lc,
                                             BCTBX_UNUSED(LinphoneFriend *lf),
                                             const char *uri_or_tel,
                                             BCTBX_UNUSED(const LinphonePresenceModel *presence)) {
	stats *counters = get_stats(lc);
	ms_message("Presence notification for URI or phone number [%s]", uri_or_tel);
	counters->number_of_NotifyPresenceReceivedForUriOrTel++;
}

void _check_friend_result_list(
    LinphoneCore *lc, const bctbx_list_t *resultList, const unsigned int index, const char *uri, const char *phone) {
	_check_friend_result_list_2(lc, resultList, index, uri, phone, NULL, LinphoneMagicSearchSourceAll);
}

void _check_friend_result_list_2(LinphoneCore *lc,
                                 const bctbx_list_t *resultList,
                                 const unsigned int index,
                                 const char *uri,
                                 const char *phone,
                                 const char *name,
                                 int expected_flags) {
	bool assertError = true;
	if (index >= (unsigned int)bctbx_list_size(resultList)) {
		ms_error("Attempt to access result to an outbound index");
		return;
	}
	const LinphoneSearchResult *sr = bctbx_list_nth_data(resultList, index);
	if (expected_flags != LinphoneMagicSearchSourceAll) {
		int source_flags = linphone_search_result_get_source_flags(sr);
		BC_ASSERT_EQUAL(source_flags, expected_flags, int, "%d");
	}

	const LinphoneFriend *lf = linphone_search_result_get_friend(sr);
	const LinphoneAddress *search_result_address = linphone_search_result_get_address(sr);
	if (lf || search_result_address) {
		const LinphoneAddress *la = (search_result_address) ? search_result_address : linphone_friend_get_address(lf);
		if (uri) { // Check on address
			if (la) {
				char *fa = linphone_address_as_string_uri_only(la);
				bool ldap_available = !!linphone_core_ldap_available(lc);
				if (ldap_available) {
					int result = strcasecmp(fa, uri);
					BC_ASSERT_TRUE(result == 0);
					if (result != 0) {
						ms_error("Expected [%s], got [%s]", uri, fa);
					}
				} else {
					BC_ASSERT_STRING_EQUAL(fa, uri);
				}
				free(fa);
				assertError = false;
			} else if (phone) {
				const LinphonePresenceModel *presence = linphone_friend_get_presence_model_for_uri_or_tel(lf, phone);
				if (BC_ASSERT_PTR_NOT_NULL(presence)) {
					char *contact = linphone_presence_model_get_contact(presence);
					BC_ASSERT_STRING_EQUAL(contact, uri);
					free(contact);
					assertError = false;
				}
			}
		} else if (phone) { // Check on phone number
			BC_ASSERT_STRING_EQUAL(linphone_search_result_get_phone_number(sr), phone);
			assertError = false;
		}
		if (name) {
			const char *display_name = linphone_address_get_display_name(la);
			BC_ASSERT_STRING_EQUAL(display_name, name);
			assertError = false;
		}
	} else {
		const bctbx_list_t *callLog = linphone_core_get_call_logs(lc);
		const bctbx_list_t *f;
		for (f = callLog; f != NULL; f = bctbx_list_next(f)) {
			LinphoneCallLog *log = (LinphoneCallLog *)(f->data);
			const LinphoneAddress *addr = (linphone_call_log_get_dir(log) == LinphoneCallIncoming)
			                                  ? linphone_call_log_get_from_address(log)
			                                  : linphone_call_log_get_to_address(log);
			if (addr) {
				char *addrUri = linphone_address_as_string_uri_only(addr);
				if (addrUri && strcmp(addrUri, uri) == 0) {
					bctbx_free(addrUri);
					return;
				}
				if (addrUri) bctbx_free(addrUri);
			}
		}
	}
	if (assertError) {
		BC_ASSERT(FALSE);
		ms_error("Address NULL and Presence NULL");
	}
}

void _check_friend_result_list_3(LinphoneCore *lc,
                                 const bctbx_list_t *resultList,
                                 const unsigned int index,
                                 const char *uri,
                                 const char *phone,
                                 const char *name) {
	_check_friend_result_list_2(lc, resultList, index, uri, phone, name, LinphoneMagicSearchSourceAll);
}

void linphone_transfer_state_changed(LinphoneCore *lc, LinphoneCall *transferred, LinphoneCallState new_call_state) {
	LinphoneCallLog *clog = linphone_call_get_call_log(transferred);
	char *to = linphone_address_as_string(linphone_call_log_get_to_address(clog));
	char *from = linphone_address_as_string(linphone_call_log_get_from_address(clog));
	stats *counters;
	ms_message("Transferred call from [%s] to [%s], new state is [%s]", from, to,
	           linphone_call_state_to_string(new_call_state));
	ms_free(to);
	ms_free(from);

	counters = get_stats(lc);
	switch (new_call_state) {
		case LinphoneCallOutgoingInit:
			counters->number_of_LinphoneTransferCallOutgoingInit++;
			break;
		case LinphoneCallOutgoingProgress:
			counters->number_of_LinphoneTransferCallOutgoingProgress++;
			break;
		case LinphoneCallOutgoingRinging:
			counters->number_of_LinphoneTransferCallOutgoingRinging++;
			break;
		case LinphoneCallOutgoingEarlyMedia:
			counters->number_of_LinphoneTransferCallOutgoingEarlyMedia++;
			break;
		case LinphoneCallConnected:
			counters->number_of_LinphoneTransferCallConnected++;
			break;
		case LinphoneCallStreamsRunning:
			counters->number_of_LinphoneTransferCallStreamsRunning++;
			break;
		case LinphoneCallError:
			counters->number_of_LinphoneTransferCallError++;
			break;
		default:
			BC_FAIL("unexpected event");
			break;
	}
}

void info_message_received(LinphoneCore *lc, BCTBX_UNUSED(LinphoneCall *call), const LinphoneInfoMessage *msg) {
	stats *counters = get_stats(lc);

	if (counters->last_received_info_message) {
		linphone_info_message_unref(counters->last_received_info_message);
	}
	counters->last_received_info_message = linphone_info_message_copy(msg);
	counters->number_of_InfoReceived++;
}

void linphone_subscription_state_change(LinphoneCore *lc, LinphoneEvent *lev, LinphoneSubscriptionState state) {
	stats *counters = get_stats(lc);
	LinphoneCoreManager *mgr = get_manager(lc);
	LinphoneContent *content;
	const LinphoneAddress *from_addr = linphone_event_get_from(lev);
	char *from = linphone_address_as_string(from_addr);
	const LinphoneAddress *to_addr = linphone_event_get_to(lev);
	char *to = linphone_address_as_string(to_addr);
	content = linphone_core_create_content(lc);
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "somexml2");
	linphone_content_set_buffer(content, (const uint8_t *)notify_content, strlen(notify_content));

	ms_message("Subscription state [%s] from [%s] to [%s]", linphone_subscription_state_to_string(state), from, to);
	ms_free(from);
	ms_free(to);

	switch (state) {
		case LinphoneSubscriptionNone:
			break;
		case LinphoneSubscriptionIncomingReceived:
			counters->number_of_LinphoneSubscriptionIncomingReceived++;
			mgr->lev = lev;
			break;
		case LinphoneSubscriptionOutgoingProgress:
			counters->number_of_LinphoneSubscriptionOutgoingProgress++;
			break;
		case LinphoneSubscriptionPending:
			counters->number_of_LinphoneSubscriptionPending++;
			break;
		case LinphoneSubscriptionActive:
			counters->number_of_LinphoneSubscriptionActive++;
			if (mgr->subscribe_policy == AcceptSubscription) {
				if (linphone_event_get_subscription_dir(lev) == LinphoneSubscriptionIncoming) {
					mgr->lev = lev;
					if (strcmp(linphone_event_get_name(lev), "conference") == 0 ||
					    strcmp(linphone_event_get_name(lev), "ekt") == 0 ||
					    strcmp(linphone_event_get_name(lev), "doingnothing") == 0) {
						// TODO : Get LocalConfEventHandler and call handler->subscribeReceived(lev)
					} else {
						linphone_event_notify(lev, content);
					}
				}
			}
			break;
		case LinphoneSubscriptionTerminated:
			counters->number_of_LinphoneSubscriptionTerminated++;
			if (lev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphoneSubscriptionError:
			counters->number_of_LinphoneSubscriptionError++;
			if (lev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphoneSubscriptionExpiring:
			counters->number_of_LinphoneSubscriptionExpiring++;
			if (lev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
	}
	linphone_content_unref(content);
}

void linphone_publish_state_changed(LinphoneCore *lc, LinphoneEvent *ev, LinphonePublishState state) {
	stats *counters = get_stats(lc);
	LinphoneCoreManager *mgr = get_manager(lc);
	const LinphoneAddress *from_addr = linphone_event_get_from(ev);
	char *from = linphone_address_as_string(from_addr);
	const LinphoneAddress *to_addr = linphone_event_get_to(ev);
	char *to = linphone_address_as_string(to_addr);
	ms_message("Publish state [%s] from [%s]", linphone_publish_state_to_string(state), from);
	ms_free(from);
	ms_free(to);

	switch (state) {
		case LinphonePublishOutgoingProgress:
			counters->number_of_LinphonePublishOutgoingProgress++;
			break;
		case LinphonePublishIncomingReceived:
			counters->number_of_LinphonePublishIncomingReceived++;
			break;
		case LinphonePublishRefreshing:
			counters->number_of_LinphonePublishRefreshing++;
			break;
		case LinphonePublishOk:
			/*make sure custom header access API is working*/
			BC_ASSERT_PTR_NOT_NULL(linphone_event_get_custom_header(ev, "From"));
			counters->number_of_LinphonePublishOk++;
			break;
		case LinphonePublishError:
			counters->number_of_LinphonePublishError++;
			if (ev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphonePublishExpiring:
			counters->number_of_LinphonePublishExpiring++;
			if (ev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphonePublishCleared:
			counters->number_of_LinphonePublishCleared++;
			if (ev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphonePublishTerminating:
			counters->number_of_LinphonePublishTerminating++;
			break;
		default:
			break;
	}
}

void linphone_notify_sent(LinphoneCore *lc, LinphoneEvent *lev, const LinphoneContent *content) {
	LinphoneCoreManager *mgr;
	mgr = get_manager(lc);
	if (strcmp(linphone_event_get_name(lev), "ekt") == 0) mgr->stat.number_of_NotifyEktSent++;
	else mgr->stat.number_of_NotifySent++;

	linphone_event_set_user_data(lev, (void *)linphone_content_copy(content));
}

void linphone_notify_received(LinphoneCore *lc,
                              LinphoneEvent *lev,
                              const char *eventname,
                              const LinphoneContent *content) {
	LinphoneCoreManager *mgr = get_manager(lc);
	const char *ua = linphone_event_get_custom_header(lev, "User-Agent");
	if (content) {
		const char *text = linphone_content_get_utf8_text(content);
		if (!linphone_content_is_multipart(content) &&
		    (!ua || !strstr(ua, "flexisip"))) { /*disable check for full presence server support*/
			/*hack to disable content checking for list notify */
			BC_ASSERT_STRING_EQUAL(text, notify_content);
		}
	}
	if (strcmp(eventname, "ekt") == 0) mgr->stat.number_of_NotifyEktReceived++;
	else mgr->stat.number_of_NotifyReceived++;
}

void linphone_subscribe_received(LinphoneCore *lc,
                                 LinphoneEvent *lev,
                                 const char *eventname,
                                 BCTBX_UNUSED(const LinphoneContent *content)) {
	if (strcmp(eventname, "ekt") != 0) {
		LinphoneCoreManager *mgr = get_manager(lc);
		switch (mgr->subscribe_policy) {
			case AcceptSubscription:
				linphone_event_accept_subscription(lev);
				break;
			case DenySubscription:
				linphone_event_deny_subscription(lev, LinphoneReasonDeclined);
				break;
			case RetainSubscription:
				linphone_event_ref(lev);
				break;
			case DoNothingWithSubscription:
				break;
		}
	}
}

void linphone_publish_received(LinphoneCore *lc,
                               LinphoneEvent *lev,
                               const char *eventname,
                               BCTBX_UNUSED(const LinphoneContent *content)) {
	if (strcmp(eventname, "ekt") != 0) {
		LinphoneCoreManager *mgr = get_manager(lc);
		switch (mgr->publish_policy) {
			case AcceptPublish:
				linphone_event_accept_publish(lev);
				break;
			case DenyPublish:
				linphone_event_deny_publish(lev, LinphoneReasonDeclined);
				break;
			case DoNothingWithPublish:
				break;
		}
	}
}

void linphone_configuration_status(LinphoneCore *lc, LinphoneConfiguringState status, const char *message) {
	stats *counters;
	ms_message("Configuring state = %i with message %s", status, message ? message : "");

	counters = get_stats(lc);
	if (status == LinphoneConfiguringSkipped) {
		counters->number_of_LinphoneConfiguringSkipped++;
	} else if (status == LinphoneConfiguringFailed) {
		counters->number_of_LinphoneConfiguringFailed++;
	} else if (status == LinphoneConfiguringSuccessful) {
		counters->number_of_LinphoneConfiguringSuccessful++;
	}
}

void linphone_call_goclear_ack_sent(LinphoneCore *lc, LinphoneCall *call) {
	stats *counters;
	counters = get_stats(lc);
	counters->number_of_LinphoneCallGoClearAckSent++;
	linphone_call_confirm_go_clear(call);
}

static void security_level_downgraded(LinphoneCall *call) {
	LinphoneCore *lc = linphone_call_get_core(call);
	stats *corestats = get_stats(lc);
	corestats->number_of_LinphoneCallSecurityLevelDowngraded++;
}

void linphone_call_create_cbs_security_level_downgraded(LinphoneCall *call) {
	LinphoneCallCbs *call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
	BC_ASSERT_PTR_NOT_NULL(call);
	linphone_call_cbs_set_security_level_downgraded(call_cbs, security_level_downgraded);
	linphone_call_add_callbacks(call, call_cbs);
	linphone_call_cbs_unref(call_cbs);
}

void linphone_call_encryption_changed(LinphoneCore *lc,
                                      LinphoneCall *call,
                                      bool_t on,
                                      BCTBX_UNUSED(const char *authentication_token)) {
	LinphoneCallLog *calllog = linphone_call_get_call_log(call);
	char *to = linphone_address_as_string(linphone_call_log_get_to_address(calllog));
	char *from = linphone_address_as_string(linphone_call_log_get_from_address(calllog));
	stats *counters;
	ms_message(" %s call from [%s] to [%s], is now [%s]",
	           linphone_call_log_get_dir(calllog) == LinphoneCallIncoming ? "Incoming" : "Outgoing", from, to,
	           (on ? "encrypted" : "unencrypted"));
	ms_free(to);
	ms_free(from);
	counters = get_stats(lc);
	if (on) counters->number_of_LinphoneCallEncryptedOn++;
	else counters->number_of_LinphoneCallEncryptedOff++;
}

void linphone_call_send_master_key_changed(LinphoneCore *lc,
                                           BCTBX_UNUSED(LinphoneCall *call),
                                           BCTBX_UNUSED(const char *master_key)) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCallSendMasterKeyChanged++;
}

void linphone_call_receive_master_key_changed(LinphoneCore *lc,
                                              BCTBX_UNUSED(LinphoneCall *call),
                                              BCTBX_UNUSED(const char *master_key)) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCallReceiveMasterKeyChanged++;
}

void dtmf_received(LinphoneCore *lc, BCTBX_UNUSED(LinphoneCall *call), int dtmf) {
	stats *counters = get_stats(lc);
	char **dst = &counters->dtmf_list_received;
	*dst = *dst ? ms_strcat_printf(*dst, "%c", dtmf) : ms_strdup_printf("%c", dtmf);
	counters->dtmf_count++;
}

static bool_t rtcp_is_type(const mblk_t *m, rtcp_type_t type) {
	const rtcp_common_header_t *ch = rtcp_get_common_header(m);
	return (ch != NULL && rtcp_common_header_get_packet_type(ch) == type);
}

static void rtcp_received(stats *counters, mblk_t *packet) {
	RtcpParserContext rtcpParser;

	const mblk_t *rtcpMessage = rtcp_parser_context_init(&rtcpParser, packet);
	do {
		if (rtcp_is_type(rtcpMessage, RTCP_RTPFB)) {
			if (rtcp_RTPFB_get_type(rtcpMessage) == RTCP_RTPFB_TMMBR) {
				counters->number_of_tmmbr_received++;
				counters->last_tmmbr_value_received = (int)rtcp_RTPFB_tmmbr_get_max_bitrate(rtcpMessage);
			}
		} else if (rtcp_is_type(rtcpMessage, RTCP_PSFB)) {
			if (rtcp_PSFB_get_type(rtcpMessage) == RTCP_PSFB_AFB && rtcp_PSFB_is_goog_remb(rtcpMessage)) {
				// Use the same counters as TMMBR
				counters->number_of_tmmbr_received++;
				counters->last_tmmbr_value_received = (int)rtcp_PSFB_goog_remb_get_max_bitrate(rtcpMessage);
			}
		}
	} while ((rtcpMessage = rtcp_parser_context_next_packet(&rtcpParser)) != NULL);
	rtcp_parser_context_uninit(&rtcpParser);
}

void call_stats_updated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *lstats) {
	const int updated = _linphone_call_stats_get_updated(lstats);
	stats *counters = get_stats(lc);

	counters->number_of_LinphoneCallStatsUpdated++;
	if (updated & LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE) {
		counters->number_of_rtcp_received++;
		if (_linphone_call_stats_rtcp_received_via_mux(lstats)) {
			counters->number_of_rtcp_received_via_mux++;
		}
		rtcp_received(counters, _linphone_call_stats_get_received_rtcp(lstats));
		BC_ASSERT_TRUE(_linphone_call_stats_has_received_rtcp(lstats));
	}
	if (updated & LINPHONE_CALL_STATS_SENT_RTCP_UPDATE) {
		counters->number_of_rtcp_sent++;
		BC_ASSERT_TRUE(_linphone_call_stats_has_sent_rtcp(lstats));
	}
	if (updated & LINPHONE_CALL_STATS_PERIODICAL_UPDATE) {
		const int tab_size = sizeof counters->audio_download_bandwidth / sizeof(int);

		LinphoneCallStats *call_stats = NULL;
		int index;

		int type = linphone_call_stats_get_type(lstats);
		if (type != LINPHONE_CALL_STATS_AUDIO && type != LINPHONE_CALL_STATS_VIDEO)
			return; // Avoid out of bounds if type is TEXT.

		index = (counters->current_bandwidth_index[type]++) % tab_size;
		if (type == LINPHONE_CALL_STATS_AUDIO) {
			call_stats = linphone_call_get_audio_stats(call);
			if (call_stats) {
				counters->audio_download_bandwidth[index] = (int)linphone_call_stats_get_download_bandwidth(call_stats);
				counters->audio_upload_bandwidth[index] = (int)linphone_call_stats_get_upload_bandwidth(call_stats);
			}
		} else {
			call_stats = linphone_call_get_video_stats(call);
			if (call_stats) {
				counters->video_download_bandwidth[index] = (int)linphone_call_stats_get_download_bandwidth(call_stats);
				counters->video_upload_bandwidth[index] = (int)linphone_call_stats_get_upload_bandwidth(call_stats);
			}
		}
		if (call_stats) {
			linphone_call_stats_unref(call_stats);
		}
	}
}

bool_t liblinphone_tester_chat_message_msg_update_stats(stats *counters, LinphoneChatMessageState state) {
	switch (state) {
		case LinphoneChatMessageStateIdle:
			break;
		case LinphoneChatMessageStateDelivered:
			counters->number_of_LinphoneMessageDelivered++;
			break;
		case LinphoneChatMessageStateNotDelivered:
			counters->number_of_LinphoneMessageNotDelivered++;
			break;
		case LinphoneChatMessageStatePendingDelivery:
			counters->number_of_LinphoneMessagePendingDelivery++;
			break;
		case LinphoneChatMessageStateInProgress:
			counters->number_of_LinphoneMessageInProgress++;
			break;
		case LinphoneChatMessageStateFileTransferError:
			counters->number_of_LinphoneMessageFileTransferError++;
			break;
		case LinphoneChatMessageStateFileTransferDone:
			counters->number_of_LinphoneMessageFileTransferDone++;
			break;
		case LinphoneChatMessageStateDeliveredToUser:
			counters->number_of_LinphoneMessageDeliveredToUser++;
			break;
		case LinphoneChatMessageStateDisplayed:
			counters->number_of_LinphoneMessageDisplayed++;
			break;
		case LinphoneChatMessageStateFileTransferCancelling:
			counters->number_of_LinphoneMessageFileTransferCancelling++;
			break;
		case LinphoneChatMessageStateFileTransferInProgress:
			counters->number_of_LinphoneMessageFileTransferInProgress++;
			break;
	}
	return FALSE;
}

void liblinphone_tester_chat_message_msg_state_changed(LinphoneChatMessage *msg, LinphoneChatMessageState state) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	if (liblinphone_tester_chat_message_msg_update_stats(counters, state))
		ms_error("Unexpected state [%s] for msg [%p]", linphone_chat_message_state_to_string(state), msg);
	else ms_message("New state [%s] for msg [%p]", linphone_chat_message_state_to_string(state), msg);

	if (linphone_chat_message_is_outgoing(msg) && state == LinphoneChatMessageStateDelivered) {
		LinphoneChatRoom *room = linphone_chat_message_get_chat_room(msg);
		BC_ASSERT_PTR_NOT_NULL(room);
		if (room && linphone_chat_room_get_capabilities(room) & LinphoneChatRoomCapabilitiesConference) {
			bctbx_list_t *participants = linphone_chat_room_get_participants(room);
			if (participants) {
				bctbx_list_t *participants_in_delivered_state =
				    linphone_chat_message_get_participants_by_imdn_state(msg, state);
				BC_ASSERT_PTR_NOT_NULL(participants_in_delivered_state);
				bctbx_list_t *participants_in_displayed_state =
				    linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateDisplayed);
				bctbx_list_t *participants_in_delivered_to_user_state =
				    linphone_chat_message_get_participants_by_imdn_state(msg, LinphoneChatMessageStateDeliveredToUser);

				size_t delivered_or_displayed = 0;
				delivered_or_displayed +=
				    ((participants_in_delivered_state) ? bctbx_list_size(participants_in_delivered_state) : 0);
				delivered_or_displayed += ((participants_in_delivered_to_user_state)
				                               ? bctbx_list_size(participants_in_delivered_to_user_state)
				                               : 0);
				delivered_or_displayed +=
				    ((participants_in_displayed_state) ? bctbx_list_size(participants_in_displayed_state) : 0);

				BC_ASSERT_EQUAL(delivered_or_displayed, bctbx_list_size(participants), size_t, "%0zu");

				if (participants_in_delivered_state) {
					bctbx_list_free_with_data(participants_in_delivered_state,
					                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
				}

				if (participants_in_delivered_to_user_state) {
					bctbx_list_free_with_data(participants_in_delivered_to_user_state,
					                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
				}

				if (participants_in_displayed_state) {
					bctbx_list_free_with_data(participants_in_displayed_state,
					                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
				}

				bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);
			}
		}
	}
}

void liblinphone_tester_chat_message_file_transfer_terminated(LinphoneChatMessage *msg,
                                                              BCTBX_UNUSED(LinphoneContent *content)) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageFileTransferTerminated++;
}

bctbx_list_t *liblinphone_tester_get_messages_and_states(
    LinphoneChatRoom *cr, int *messageCount, stats *stats) { // Return all LinphoneChatMessage and count states
	bctbx_list_t *messages = linphone_chat_room_get_history(cr, 0);
	*messageCount = 0;
	for (bctbx_list_t *elem = messages; elem != NULL; elem = bctbx_list_next(elem)) {
		++(*messageCount);
		liblinphone_tester_chat_message_msg_update_stats(
		    stats, linphone_chat_message_get_state((LinphoneChatMessage *)elem->data));
	}
	return messages;
}

void liblinphone_tester_remaining_number_of_file_transfer_changed(LinphoneCore *lc,
                                                                  BCTBX_UNUSED(unsigned int download_count),
                                                                  BCTBX_UNUSED(unsigned int upload_count)) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneRemainingNumberOfFileTransferChanged++;
}

void liblinphone_tester_chat_room_msg_sent(LinphoneCore *lc,
                                           BCTBX_UNUSED(LinphoneChatRoom *room),
                                           BCTBX_UNUSED(LinphoneChatMessage *msg)) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageSent++;
}

void liblinphone_tester_x3dh_user_created(LinphoneCore *lc,
                                          const bool_t status,
                                          BCTBX_UNUSED(const char *userId),
                                          BCTBX_UNUSED(const char *info)) {
	stats *counters = get_stats(lc);
	if (status == TRUE) {
		counters->number_of_X3dhUserCreationSuccess++;
	} else {
		counters->number_of_X3dhUserCreationFailure++;
	}
}

void liblinphone_tester_chat_message_ephemeral_timer_started(LinphoneChatMessage *msg) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageEphemeralTimerStarted++;
}

void liblinphone_tester_chat_message_ephemeral_deleted(LinphoneChatMessage *msg) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageEphemeralDeleted++;
}

char *random_filename(char *prefix, char *extension) {
	char filename[255];
	char random_part[10];
	belle_sip_random_token(random_part, sizeof(random_part) - 1);
	if (extension) {
		snprintf(filename, sizeof(filename), "%s_%s.%s", prefix, random_part, extension);
	} else {
		snprintf(filename, sizeof(filename), "%s_%s", prefix, random_part);
	}
	return ms_strdup(filename);
}

char *random_filepath(char *prefix, char *extension) {
	char *filename = random_filename(prefix, extension);
	char *filepath = bc_tester_file(filename);
	bc_free(filename);
	return filepath;
}

/*
 * function called when the file transfer is initiated. file content should be feed into object LinphoneContent
 * */
LinphoneBuffer *
tester_file_transfer_send(LinphoneChatMessage *msg, LinphoneContent *content, size_t offset, size_t size) {
	LinphoneBuffer *lb;
	size_t file_size;
	size_t size_to_send;
	uint8_t *buf;
	FILE *file_to_send = linphone_content_get_user_data(content);

	// If a file path is set, we should NOT call the on_send callback !
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));
	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

	BC_ASSERT_PTR_NOT_NULL(file_to_send);
	if (file_to_send == NULL) {
		return NULL;
	}
	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, (long)offset, SEEK_SET);
	size_to_send = MIN(size, file_size - offset);
	buf = ms_malloc(size_to_send);
	if (fread(buf, sizeof(uint8_t), size_to_send, file_to_send) != size_to_send) {
		// reaching end of file, close it
		fclose(file_to_send);
		linphone_content_set_user_data(content, NULL);
	}
	lb = linphone_buffer_new_from_data(buf, size_to_send);
	ms_free(buf);
	return lb;
}

void tester_file_transfer_send_2(
    LinphoneChatMessage *msg, LinphoneContent *content, size_t offset, size_t size, LinphoneBuffer *lb) {
	size_t file_size;
	size_t size_to_send;
	uint8_t *buf;
	FILE *file_to_send = linphone_content_get_user_data(content);

	// If a file path is set, we should NOT call the on_send callback !
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));
	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

	BC_ASSERT_PTR_NOT_NULL(file_to_send);
	if (file_to_send == NULL) {
		return;
	}

	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, (long)offset, SEEK_SET);
	size_to_send = MIN(size, file_size - offset);
	buf = ms_malloc(size_to_send);
	if (fread(buf, sizeof(uint8_t), size_to_send, file_to_send) != size_to_send) {
		// reaching end of file, close it
		fclose(file_to_send);
		linphone_content_set_user_data(content, NULL);
	}
	linphone_buffer_set_content(lb, buf, size_to_send);
	ms_free(buf);
}

/**
 * function invoked to report file transfer progress.
 * */

void file_transfer_progress_indication_base(LinphoneChatMessage *msg,
                                            LinphoneContent *content,
                                            size_t offset,
                                            size_t total,
                                            bool_t reset_num_of_file_transfer) {
	const LinphoneAddress *from_address = linphone_chat_message_get_from_address(msg);
	const LinphoneAddress *to_address = linphone_chat_message_get_to_address(msg);
	int progress = (int)((offset * 100) / total);
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	char *address = linphone_address_as_string(linphone_chat_message_is_outgoing(msg) ? to_address : from_address);
	BC_ASSERT_LOWER(progress, 100, int, "%i");
	BC_ASSERT_GREATER(progress, 0, int, "%i");
	if (progress > 100 || progress < 0) {
		bctbx_error("Unexpected progress value. offset = [%zu], total=[%zu]", offset, total);
	}

	if (reset_num_of_file_transfer && progress == 0) {
		counters->number_of_LinphoneFileTransfer = 0;
	}
	counters->number_of_LinphoneFileTransfer++;

	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");
	bctbx_message("File transfer name [%s] [%d%%] %s of type [%s/%s] %s [%s] \n", linphone_content_get_name(content),
	              progress, linphone_chat_message_is_outgoing(msg) ? "sent" : "received",
	              linphone_content_get_type(content), linphone_content_get_subtype(content),
	              linphone_chat_message_is_outgoing(msg) ? "to" : "from", address);
	counters->progress_of_LinphoneFileTransfer = progress;
	if (progress == 100) {
		counters->number_of_LinphoneFileTransferDownloadSuccessful++;
	}
	free(address);
}
void file_transfer_progress_indication(LinphoneChatMessage *msg,
                                       LinphoneContent *content,
                                       size_t offset,
                                       size_t total) {
	file_transfer_progress_indication_base(msg, content, offset, total, TRUE);
}

void file_transfer_progress_indication_2(LinphoneChatMessage *msg,
                                         LinphoneContent *content,
                                         size_t offset,
                                         size_t total) {
	file_transfer_progress_indication_base(msg, content, offset, total, FALSE);
}

/**
 * function invoked when a file transfer is received.
 * */
void file_transfer_received(LinphoneChatMessage *msg, LinphoneContent *content, const LinphoneBuffer *buffer) {
	FILE *file = NULL;

	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

	if (!linphone_content_get_user_data(content)) {
		// If a file path is set, we should NOT call the on_recv callback !
		BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));
		char *receive_file = random_filepath("receive_file", "dump");
		/*first chunk, creating file*/
		file = fopen(receive_file, "wb");
		linphone_content_set_user_data(content, (void *)file); /*store fd for next chunks*/
		linphone_chat_message_set_file_transfer_filepath(msg, receive_file);
		bc_free(receive_file);
	}

	file = (FILE *)linphone_content_get_user_data(content);
	BC_ASSERT_PTR_NOT_NULL(file);
	if (linphone_buffer_is_empty(buffer)) { /* tranfer complete */
		struct stat st;
		linphone_content_set_user_data(content, NULL);
		fclose(file);
		BC_ASSERT_TRUE(stat(linphone_chat_message_get_file_transfer_filepath(msg), &st) == 0);
		BC_ASSERT_EQUAL((int)linphone_content_get_file_size(content), (int)st.st_size, int, "%i");

	} else { /* store content on a file*/
		if (fwrite(linphone_buffer_get_content(buffer), linphone_buffer_get_size(buffer), 1, file) == 0) {
			ms_error("file_transfer_received(): write() failed: %s", strerror(errno));
		}
	}
}

void check_reactions(LinphoneChatMessage *message,
                     size_t expected_reactions_count,
                     const bctbx_list_t *expected_reactions,
                     const bctbx_list_t *expected_reactions_from) {
	bctbx_list_t *reactions = linphone_chat_message_get_reactions(message);
	bctbx_list_t *reactions_it = reactions;
	bctbx_list_t *expected_reactions_it = (bctbx_list_t *)expected_reactions;
	bctbx_list_t *expected_reactions_from_it = (bctbx_list_t *)expected_reactions_from;
	BC_ASSERT_PTR_NOT_NULL(reactions);

	if (reactions_it) {
		size_t count = bctbx_list_size(reactions);
		BC_ASSERT_EQUAL(count, expected_reactions_count, size_t, "%zu");
		for (size_t i = 0; i < count; i++) {
			const LinphoneChatMessageReaction *reaction =
			    (const LinphoneChatMessageReaction *)bctbx_list_get_data(reactions_it);
			reactions_it = bctbx_list_next(reactions_it);

			const char *expected_reaction = (const char *)bctbx_list_get_data(expected_reactions_it);
			expected_reactions_it = bctbx_list_next(expected_reactions_it);

			const char *expected_reaction_from = (const char *)bctbx_list_get_data(expected_reactions_from_it);
			expected_reactions_from_it = bctbx_list_next(expected_reactions_from_it);

			const char *reaction_body = linphone_chat_message_reaction_get_body(reaction);
			BC_ASSERT_STRING_EQUAL(reaction_body, expected_reaction);

			const char *reaction_call_id = linphone_chat_message_reaction_get_call_id(reaction);
			BC_ASSERT_PTR_NOT_NULL(reaction_call_id);

			const LinphoneAddress *from = linphone_chat_message_reaction_get_from_address(reaction);
			char *from_address = linphone_address_as_string_uri_only(from);
			BC_ASSERT_STRING_EQUAL(from_address, expected_reaction_from);
			bctbx_free(from_address);
		}
	}
	bctbx_list_free_with_data(reactions, (bctbx_list_free_func)linphone_chat_message_reaction_unref);
}

void liblinphone_tester_chat_message_reaction_received(LinphoneChatMessage *msg,
                                                       const LinphoneChatMessageReaction *reaction) {
	BC_ASSERT_PTR_NOT_NULL(msg);
	BC_ASSERT_PTR_NOT_NULL(reaction);

	const LinphoneAddress *address = linphone_chat_message_reaction_get_from_address(reaction);
	BC_ASSERT_PTR_NOT_NULL(address);
	const char *body = linphone_chat_message_reaction_get_body(reaction);
	BC_ASSERT_STRING_EQUAL(body, "👍");

	bctbx_list_t *expected_reaction = bctbx_list_append(NULL, "👍");
	bctbx_list_t *expected_reaction_from =
	    bctbx_list_append(NULL, ms_strdup(linphone_address_as_string_uri_only(address)));
	check_reactions(msg, 1, expected_reaction, expected_reaction_from);
	bctbx_list_free(expected_reaction);
	bctbx_list_free_with_data(expected_reaction_from, (bctbx_list_free_func)ms_free);
}

void global_state_changed(LinphoneCore *lc, LinphoneGlobalState gstate, BCTBX_UNUSED(const char *message)) {
	stats *counters = get_stats(lc);
	switch (gstate) {
		case LinphoneGlobalOn: {
			counters->number_of_LinphoneGlobalOn++;
			bctbx_list_t *infos = linphone_core_get_conference_information_list(lc);
			for (bctbx_list_t *it = infos; it; it = bctbx_list_next(it)) {
				LinphoneConferenceInfo *info = (LinphoneConferenceInfo *)it->data;
				const LinphoneAddress *uri = linphone_conference_info_get_uri(info);
				BC_ASSERT_PTR_NOT_NULL(uri);
				LinphoneConference *conference = linphone_core_search_conference_2(lc, uri);
				if (conference) {
					create_conference_cb(conference);
				}
			}
			if (infos) {
				bctbx_list_free_with_data(infos, (bctbx_list_free_func)linphone_conference_info_unref);
			}
			const bctbx_list_t *chat_rooms = linphone_core_get_chat_rooms(lc);
			for (const bctbx_list_t *it = chat_rooms; it; it = bctbx_list_next(it)) {
				LinphoneChatRoom *chat_room = (LinphoneChatRoom *)it->data;
				LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
				setup_chat_room_callbacks(cbs);
				linphone_chat_room_add_callbacks(chat_room, cbs);
				linphone_chat_room_cbs_unref(cbs);
			}
		} break;
		case LinphoneGlobalReady:
			counters->number_of_LinphoneGlobalReady++;
			break;
		case LinphoneGlobalOff:
			counters->number_of_LinphoneGlobalOff++;
			break;
		case LinphoneGlobalStartup:
			counters->number_of_LinphoneGlobalStartup++;
			break;
		case LinphoneGlobalShutdown:
			counters->number_of_LinphoneGlobalShutdown++;
			break;
		case LinphoneGlobalConfiguring:
			counters->number_of_LinphoneGlobalConfiguring++;
			break;
	}
}

void first_call_started(LinphoneCore *lc) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreFirstCallStarted++;
}

void last_call_ended(LinphoneCore *lc) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreLastCallEnded++;
}

void audio_device_changed(LinphoneCore *lc, BCTBX_UNUSED(LinphoneAudioDevice *device)) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreAudioDeviceChanged++;
}

void audio_devices_list_updated(LinphoneCore *lc) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreAudioDevicesListUpdated++;
}

void setup_sdp_handling(const LinphoneCallTestParams *params, LinphoneCoreManager *mgr) {
	if (params->sdp_removal) {
		sal_default_set_sdp_handling(linphone_core_get_sal(mgr->lc), SalOpSDPSimulateRemove);
	} else if (params->sdp_simulate_error) {
		sal_default_set_sdp_handling(linphone_core_get_sal(mgr->lc), SalOpSDPSimulateError);
	}
}

bool_t search_matching_srtp_suite(LinphoneCoreManager *caller_mgr, LinphoneCoreManager *callee_mgr) {
	const MSCryptoSuite *callee_suites = linphone_core_get_srtp_crypto_suites_array(callee_mgr->lc);
	const MSCryptoSuite *caller_suites = linphone_core_get_srtp_crypto_suites_array(caller_mgr->lc);
	bool_t crypto_suite_found = FALSE;
	if (caller_suites && callee_suites) {
		for (size_t i = 0; (callee_suites != NULL) && (callee_suites[i] != MS_CRYPTO_SUITE_INVALID); i++) {
			for (size_t j = 0; (caller_suites != NULL) && (caller_suites[j] != MS_CRYPTO_SUITE_INVALID); j++) {
				crypto_suite_found |= (callee_suites[i] == caller_suites[j]);
			}
		}
	}
	return crypto_suite_found;
}

void check_stream_encryption(LinphoneCall *call) {
	const LinphoneCallParams *call_params = linphone_call_get_current_params(call);
	if (!linphone_call_params_rtp_bundle_enabled(call_params)) {
		const LinphoneMediaEncryption enc = linphone_call_params_get_media_encryption(call_params);
		MediaStream *astream = linphone_call_get_stream(call, LinphoneStreamTypeAudio);
		if (astream && audio_stream_started((AudioStream *)astream)) {
			if (enc == LinphoneMediaEncryptionNone) {
				BC_ASSERT_FALSE(media_stream_secured(astream));
			} else if (enc == LinphoneMediaEncryptionSRTP) {
				BC_ASSERT_TRUE(media_stream_secured(astream) == is_srtp_secured(call, LinphoneStreamTypeAudio));
			} else {
				BC_ASSERT_TRUE(media_stream_secured(astream));
			}
		}
#ifdef VIDEO_ENABLED
		MediaStream *vstream = linphone_call_get_stream(call, LinphoneStreamTypeVideo);
		if (vstream && video_stream_started((VideoStream *)vstream)) {
			if (enc == LinphoneMediaEncryptionNone) {
				BC_ASSERT_FALSE(media_stream_secured(vstream));
			} else if (enc == LinphoneMediaEncryptionSRTP) {
				BC_ASSERT_TRUE(media_stream_secured(vstream) == is_srtp_secured(call, LinphoneStreamTypeVideo));
			} else {
				BC_ASSERT_TRUE(media_stream_secured(vstream));
			}
		}
#endif
	}
}

/*
 * CAUTION this function is error prone. you should not use it anymore in new tests.
 * Creating callee call params before the call is actually received is not the good way
 * to use the Liblinphone API. Indeed, call params used for receiving calls shall be created by
 *linphone_core_create_call_params() by passing the call object for which params are to be created. This function should
 *be used only in test case where the programmer exactly knows the caller params, and then can deduce how callee params
 *will be set by linphone_core_create_call_params(). This function was developped at a time where the use of the API
 *about incoming params was not yet clarified. Tests relying on this function are then not testing the correct way to
 *use the api (through linphone_core_create_call_params()), and so it is not a so good idea to build new tests based on
 *this function.
 **/
bool_t call_with_params2(LinphoneCoreManager *caller_mgr,
                         LinphoneCoreManager *callee_mgr,
                         const LinphoneCallTestParams *caller_test_params,
                         const LinphoneCallTestParams *callee_test_params,
                         bool_t build_callee_params) {
	int retry = 0;
	stats initial_caller = caller_mgr->stat;
	stats initial_callee = callee_mgr->stat;
	bool_t result = FALSE;
	const LinphoneCallParams *caller_params = caller_test_params->base;
	const LinphoneCallParams *callee_params = callee_test_params->base;
	bool_t did_receive_call;
	LinphoneCall *callee_call = NULL;
	LinphoneCall *caller_call = NULL;
	bool_t callee_should_ring = TRUE;

	if (linphone_core_is_in_conference(callee_mgr->lc) || linphone_core_get_current_call(callee_mgr->lc) != NULL) {
		callee_should_ring = FALSE;
	}

	const LinphoneCoreToneManagerStats *callee_stats = linphone_core_get_tone_manager_stats(callee_mgr->lc);
	const LinphoneCoreToneManagerStats *caller_stats = linphone_core_get_tone_manager_stats(caller_mgr->lc);
	const LinphoneCoreToneManagerStats initial_callee_stats = *callee_stats;

	/* TODO: This should be handled correctly inside the liblinphone library but meanwhile handle this here. */
	linphone_core_manager_wait_for_stun_resolution(caller_mgr);
	linphone_core_manager_wait_for_stun_resolution(callee_mgr);

	setup_sdp_handling(caller_test_params, caller_mgr);
	setup_sdp_handling(callee_test_params, callee_mgr);

	if (!caller_params) {
		BC_ASSERT_PTR_NOT_NULL((caller_call = linphone_core_invite_address(caller_mgr->lc, callee_mgr->identity)));
	} else {
		BC_ASSERT_PTR_NOT_NULL((caller_call = linphone_core_invite_address_with_params(
		                            caller_mgr->lc, callee_mgr->identity, caller_params)));
	}

	BC_ASSERT_PTR_NULL(linphone_call_get_remote_params(
	    caller_call)); /*assert that remote params are NULL when no response is received yet*/
	// test ios simulator needs more time, 3s plus for connecting the network
	did_receive_call =
	    wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallIncomingReceived,
	                   initial_callee.number_of_LinphoneCallIncomingReceived + 1, 12000);
	BC_ASSERT_EQUAL(did_receive_call, !callee_test_params->sdp_simulate_error, int, "%d");

	sal_default_set_sdp_handling(linphone_core_get_sal(caller_mgr->lc), SalOpSDPNormal);
	sal_default_set_sdp_handling(linphone_core_get_sal(callee_mgr->lc), SalOpSDPNormal);

	if (!did_receive_call) return 0;

	int calls_nb = linphone_core_get_calls_nb(callee_mgr->lc);
	if (calls_nb <= 1) BC_ASSERT_TRUE(linphone_core_is_incoming_invite_pending(callee_mgr->lc));
	BC_ASSERT_GREATER(caller_mgr->stat.number_of_LinphoneCallOutgoingProgress,
	                  initial_caller.number_of_LinphoneCallOutgoingProgress, int, "%d");

	while (caller_mgr->stat.number_of_LinphoneCallOutgoingRinging !=
	           (initial_caller.number_of_LinphoneCallOutgoingRinging + 1) &&
	       caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia !=
	           (initial_caller.number_of_LinphoneCallOutgoingEarlyMedia + 1) &&
	       retry++ < 100) {
		linphone_core_iterate(caller_mgr->lc);
		linphone_core_iterate(callee_mgr->lc);
		ms_usleep(20000);
	}

	BC_ASSERT_TRUE((caller_mgr->stat.number_of_LinphoneCallOutgoingRinging ==
	                initial_caller.number_of_LinphoneCallOutgoingRinging + 1) ||
	               (caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia ==
	                initial_caller.number_of_LinphoneCallOutgoingEarlyMedia + 1));

	if (calls_nb == 1) {
		/*only relevant if one call, otherwise, not always set*/
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call_remote_address(callee_mgr->lc));
	}
	LinphoneAddress *callee_from = NULL;
	if (caller_params) {
		const char *callee_from_str = linphone_call_params_get_from_header(caller_params);
		if (callee_from_str) {
			callee_from = linphone_address_new(callee_from_str);
		}
	}
	if (!callee_from) {
		callee_from = linphone_address_clone(caller_mgr->identity);
	}

	callee_call = linphone_core_get_call_by_remote_address2(callee_mgr->lc, callee_from);

	if (!linphone_core_get_current_call(caller_mgr->lc) ||
	    (!callee_call && !linphone_core_get_current_call(callee_mgr->lc)) /*for privacy case*/) {
		return 0;
	} else if (caller_mgr->identity) {
		linphone_address_set_port(callee_from, 0); /*remove port because port is never present in from header*/

		if (linphone_call_params_get_privacy(linphone_call_get_current_params(
		        linphone_core_get_current_call(caller_mgr->lc))) == LinphonePrivacyNone) {
			/*don't check in case of p asserted id*/
			if (!linphone_config_get_int(linphone_core_get_config(callee_mgr->lc), "sip",
			                             "call_logs_use_asserted_id_instead_of_from", 0)) {
				BC_ASSERT_PTR_NOT_NULL(callee_call);
				if (callee_call) {
					BC_ASSERT_TRUE(
					    linphone_address_weak_equal(callee_from, linphone_call_get_remote_address(callee_call)));
				}
			}
		} else {
			BC_ASSERT_FALSE(linphone_address_weak_equal(
			    callee_from, linphone_call_get_remote_address(linphone_core_get_current_call(callee_mgr->lc))));
		}
	}
	linphone_address_unref(callee_from);

	if (callee_stats->number_of_startRingbackTone == callee_stats->number_of_stopRingbackTone) {
		if (callee_should_ring) {
			BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, (int *)&callee_stats->number_of_startRingtone,
			                        callee_mgr->stat.number_of_LinphoneCallIncomingReceived));
		}
	} else {
		// in this case, the call is currently in RingbackTone so the Ringtone should not start
		callee_should_ring = FALSE;
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, (int *)&callee_stats->number_of_startRingtone,
		                        callee_mgr->stat.number_of_LinphoneCallIncomingReceived - 1));
	}
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, (int *)&caller_stats->number_of_startRingbackTone,
	                        caller_mgr->stat.number_of_LinphoneCallOutgoingRinging));

	// Local call parameters are available after moving to OutgoingRinging
	if (!caller_params && caller_call) {
		caller_params = linphone_call_get_params(caller_call);
	}

	bool_t caller_capability_negotiations_enabled = linphone_call_params_capability_negotiations_enabled(caller_params);
	bool_t caller_capability_negotiation_reinvite_enabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(caller_params);
	const LinphoneMediaEncryption caller_local_enc = linphone_call_params_get_media_encryption(caller_params);
	const bool_t caller_mand_enc = linphone_call_params_mandatory_media_encryption_enabled(caller_params);
	bctbx_list_t *caller_supported_encs = linphone_call_params_get_supported_encryptions(caller_params);

	if (callee_params) {
		linphone_call_accept_with_params(callee_call, callee_params);
	} else if (build_callee_params) {
		LinphoneCallParams *default_params = linphone_core_create_call_params(callee_mgr->lc, callee_call);
		ms_message("Created default call params with video=%i", linphone_call_params_video_enabled(default_params));
		linphone_call_accept_with_params(callee_call, default_params);
		linphone_call_params_unref(default_params);
		callee_params = linphone_call_get_params(callee_call);
	} else if (callee_call) {
		callee_params = linphone_call_get_params(callee_call);
		linphone_call_accept(callee_call);
	} else {
		LinphoneCall *callee_mgr_current_call = linphone_core_get_current_call(callee_mgr->lc);
		callee_params = linphone_call_get_params(callee_mgr_current_call);
		linphone_call_accept(callee_mgr_current_call);
	}

	bool_t callee_capability_negotiations_enabled = linphone_call_params_capability_negotiations_enabled(callee_params);
	bool_t callee_capability_negotiation_reinvite_enabled =
	    linphone_call_params_capability_negotiation_reinvite_enabled(callee_params);
	const LinphoneMediaEncryption callee_local_enc = linphone_call_params_get_media_encryption(callee_params);
	const bool_t callee_mand_enc = linphone_call_params_mandatory_media_encryption_enabled(callee_params);

	LinphoneMediaEncryption matched_enc = LinphoneMediaEncryptionNone;

	if (caller_mand_enc) {
		matched_enc = caller_local_enc;
	} else if (callee_mand_enc) {
		matched_enc = callee_local_enc;
	} else if (caller_capability_negotiations_enabled && callee_capability_negotiations_enabled) {
		bool_t enc_check_result = FALSE;
		// Find first encryption listed in the list of supported encryptions of the caller that is supported by the
		// callee
		for (bctbx_list_t *enc = caller_supported_encs; enc != NULL; enc = enc->next) {
			matched_enc = (LinphoneMediaEncryption)((intptr_t)(bctbx_list_get_data(enc)));
			enc_check_result |= (linphone_call_params_is_media_encryption_supported(callee_params, matched_enc) ||
			                     (callee_local_enc == matched_enc));
			if (enc_check_result) {
				break;
			}
		}
		if (enc_check_result && (matched_enc == LinphoneMediaEncryptionSRTP)) {
			enc_check_result = search_matching_srtp_suite(caller_mgr, callee_mgr);
		}

		if (!enc_check_result) {
			if (caller_local_enc == callee_local_enc) {
				matched_enc = caller_local_enc;
			} else {
				BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallError,
				                        (initial_caller.number_of_LinphoneCallError + 1)));
				BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc,
				                        &caller_mgr->stat.number_of_LinphoneCallReleased,
				                        (initial_caller.number_of_LinphoneCallReleased + 1)));
				BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc,
				                        &callee_mgr->stat.number_of_LinphoneCallReleased,
				                        (initial_callee.number_of_LinphoneCallReleased + 1)));
				return FALSE;
			}
		}
	} else {
		if ((callee_local_enc != LinphoneMediaEncryptionNone) || (caller_local_enc != LinphoneMediaEncryptionNone)) {
			if ((callee_local_enc == LinphoneMediaEncryptionZRTP) ||
			    (caller_local_enc == LinphoneMediaEncryptionZRTP)) {
				matched_enc = LinphoneMediaEncryptionZRTP;
			} else if ((callee_local_enc == LinphoneMediaEncryptionDTLS) ||
			           (caller_local_enc == LinphoneMediaEncryptionDTLS)) {
				matched_enc = LinphoneMediaEncryptionDTLS;
			} else {
				matched_enc = LinphoneMediaEncryptionSRTP;
			}
		}
	}

	if (caller_supported_encs) {
		bctbx_list_free(caller_supported_encs);
	}

	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallConnected,
	                        initial_callee.number_of_LinphoneCallConnected + 1));
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallConnected,
	                        initial_caller.number_of_LinphoneCallConnected + 1));

	result = wait_for_until(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_caller.number_of_LinphoneCallStreamsRunning + 1, 2000) &&
	         wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                        initial_callee.number_of_LinphoneCallStreamsRunning + 1, 2000);

	BC_ASSERT_GREATER(callee_stats->number_of_startNamedTone,
	                  callee_mgr->stat.number_of_LinphoneCallEnd + callee_mgr->stat.number_of_LinphoneCallError, int,
	                  "%d");

	/* The ringtone, if it has started, must have stopped. */
	BC_ASSERT_EQUAL(callee_stats->number_of_startRingtone - initial_callee_stats.number_of_startRingtone,
	                callee_stats->number_of_stopRingtone - initial_callee_stats.number_of_stopRingtone, int, "%d");
	BC_ASSERT_EQUAL(caller_stats->number_of_stopRingbackTone, caller_mgr->stat.number_of_LinphoneCallOutgoingRinging,
	                int, "%d");

	if (result != 0) {
		if ((matched_enc == LinphoneMediaEncryptionDTLS) || (matched_enc == LinphoneMediaEncryptionZRTP)) {
			BC_ASSERT_TRUE(
			    wait_for_until(callee_mgr->lc, caller_mgr->lc, &caller_mgr->stat.number_of_LinphoneCallEncryptedOn,
			                   initial_caller.number_of_LinphoneCallEncryptedOn + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_until(callee_mgr->lc, caller_mgr->lc, &callee_mgr->stat.number_of_LinphoneCallEncryptedOn,
			                   initial_callee.number_of_LinphoneCallEncryptedOn + 1, liblinphone_tester_sip_timeout));
		}

		caller_call = linphone_core_get_current_call(caller_mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(caller_call);
		const LinphoneCallParams *caller_call_param = linphone_call_get_current_params(caller_call);
		const LinphoneMediaEncryption caller_enc = linphone_call_params_get_media_encryption(caller_call_param);
		check_lime_ik(caller_mgr, caller_call);

		callee_call = linphone_core_get_current_call(callee_mgr->lc);
		BC_ASSERT_PTR_NOT_NULL(callee_call);
		const LinphoneCallParams *callee_call_param = linphone_call_get_current_params(callee_call);
		const LinphoneMediaEncryption callee_enc = linphone_call_params_get_media_encryption(callee_call_param);
		check_lime_ik(callee_mgr, callee_call);

		// Ensure that encryption on both sides is the same
		BC_ASSERT_EQUAL(caller_enc, matched_enc, int, "%d");
		BC_ASSERT_EQUAL(callee_enc, matched_enc, int, "%d");
		BC_ASSERT_EQUAL(caller_enc, callee_enc, int, "%d");

		if (matched_enc == LinphoneMediaEncryptionZRTP) {
			const char *callee_token = linphone_call_get_authentication_token(callee_call);
			const char *caller_token = linphone_call_get_authentication_token(caller_call);
			BC_ASSERT_PTR_NOT_NULL(callee_token);
			BC_ASSERT_PTR_NOT_NULL(caller_token);
			if (caller_token && callee_token) {
				BC_ASSERT_STRING_EQUAL(callee_token, caller_token);
				BC_ASSERT_TRUE(strlen(callee_token) > 0);
				BC_ASSERT_TRUE(strlen(caller_token) > 0);

				const char *caller_local_token = linphone_call_get_local_authentication_token(caller_call);
				const char *callee_local_token = linphone_call_get_local_authentication_token(callee_call);
				LinphoneCallStats *stats = linphone_call_get_stats(caller_call, LinphoneStreamTypeAudio);
				const char *sas_algo = linphone_call_stats_get_zrtp_sas_algo(stats);
				char *auth_token;
				if (strcmp(sas_algo, "Base32") == 0) {
					auth_token = (char *)bctbx_malloc(strlen(caller_local_token) + strlen(callee_local_token) + 1);
					strcat(strcpy(auth_token, callee_local_token), caller_local_token);
				} else {
					auth_token = (char *)bctbx_malloc(strlen(caller_local_token) + 1 + strlen(callee_local_token) + 1);
					strcat(strcat(strcpy(auth_token, callee_local_token), ":"), caller_local_token);
				}
				BC_ASSERT_STRING_EQUAL(auth_token, caller_token);
				bctbx_free(auth_token);
				linphone_call_stats_unref(stats);

				bool_t auth_token_found = FALSE;
				for (const bctbx_list_t *it = linphone_call_get_remote_authentication_tokens(caller_call); it;
				     it = it->next) {
					if (strcmp(callee_local_token, it->data) == 0) auth_token_found = TRUE;
				}
				BC_ASSERT_TRUE(auth_token_found);
				auth_token_found = FALSE;
				for (const bctbx_list_t *it = linphone_call_get_remote_authentication_tokens(callee_call); it;
				     it = it->next) {
					if (strcmp(caller_local_token, it->data) == 0) auth_token_found = TRUE;
				}
				BC_ASSERT_TRUE(auth_token_found);
			}
		}

		const bool_t calleeSendIceReInvite = linphone_config_get_int(linphone_core_get_config(callee_mgr->lc), "sip",
		                                                             "update_call_when_ice_completed", TRUE);
		const bool_t callerSendIceReInvite = linphone_config_get_int(linphone_core_get_config(caller_mgr->lc), "sip",
		                                                             "update_call_when_ice_completed", TRUE);

		const bool_t calleeSendIceReInviteWithDtls = linphone_config_get_int(
		    linphone_core_get_config(callee_mgr->lc), "sip", "update_call_when_ice_completed_with_dtls", FALSE);
		const bool_t callerSendIceReInviteWithDtls = linphone_config_get_int(
		    linphone_core_get_config(caller_mgr->lc), "sip", "update_call_when_ice_completed_with_dtls", FALSE);

		LinphoneNatPolicy *caller_policy = get_nat_policy_for_call(caller_mgr, caller_call);
		LinphoneNatPolicy *callee_policy = get_nat_policy_for_call(callee_mgr, callee_call);
		bool_t capability_negotiation_reinvite_enabled = linphone_core_sdp_200_ack_enabled(caller_mgr->lc)
		                                                     ? callee_capability_negotiation_reinvite_enabled
		                                                     : caller_capability_negotiation_reinvite_enabled;

		/*wait ice and/or capability negotiation re-invite*/
		// If caller sets mandatory encryption, potential configurations are not added to the SDP as there is no choice
		// to be made
		if (!caller_mand_enc && caller_capability_negotiations_enabled && callee_capability_negotiations_enabled &&
		    (caller_local_enc != caller_enc) && capability_negotiation_reinvite_enabled) {
			// Capability negotiation re-invite
			BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc,
			                        &caller_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                        initial_caller.number_of_LinphoneCallStreamsRunning + 2));
			BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc,
			                        &callee_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                        initial_callee.number_of_LinphoneCallStreamsRunning + 2));
		} else if (linphone_nat_policy_ice_enabled(caller_policy) && linphone_nat_policy_ice_enabled(callee_policy) &&
		           calleeSendIceReInvite && callerSendIceReInvite &&
		           ((caller_enc != LinphoneMediaEncryptionDTLS) ||
		            (calleeSendIceReInviteWithDtls && callerSendIceReInviteWithDtls))) {
			BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc,
			                        &caller_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                        initial_caller.number_of_LinphoneCallStreamsRunning + 2));
			BC_ASSERT_TRUE(wait_for(callee_mgr->lc, caller_mgr->lc,
			                        &callee_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                        initial_callee.number_of_LinphoneCallStreamsRunning + 2));
		} else if (linphone_nat_policy_ice_enabled(caller_policy)) {
			/* check no ice re-invite received*/
			BC_ASSERT_FALSE(wait_for_until(callee_mgr->lc, caller_mgr->lc,
			                               &caller_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                               initial_caller.number_of_LinphoneCallStreamsRunning + 2, 2000));
			BC_ASSERT_FALSE(wait_for_until(callee_mgr->lc, caller_mgr->lc,
			                               &callee_mgr->stat.number_of_LinphoneCallStreamsRunning,
			                               initial_callee.number_of_LinphoneCallStreamsRunning + 2, 2000));
		}

		check_stream_encryption(caller_call);
		check_stream_encryption(callee_call);

		if (caller_enc == LinphoneMediaEncryptionDTLS) {
			LinphoneCall *call = linphone_core_get_current_call(caller_mgr->lc);
			if (!BC_ASSERT_PTR_NOT_NULL(call)) return FALSE;
			AudioStream *astream = (AudioStream *)linphone_call_get_stream(call, LinphoneStreamTypeAudio);
#ifdef VIDEO_ENABLED
			VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
#endif
			if (astream) BC_ASSERT_TRUE(ms_media_stream_sessions_get_encryption_mandatory(&astream->ms.sessions));
#ifdef VIDEO_ENABLED
			if (vstream && video_stream_started(vstream))
				BC_ASSERT_TRUE(ms_media_stream_sessions_get_encryption_mandatory(&vstream->ms.sessions));
#endif
		}
	}
	return result;
}

/*
 * CAUTION this function is error prone. you should not use it anymore in new tests.
 * Creating callee call params before the call is actually received is not the good way
 * to use the Liblinphone API. Indeed, call params used for receiving calls shall be created by
 *linphone_core_create_call_params() by passing the call object for which params are to be created. This function should
 *be used only in test case where the programmer exactly knows the caller params, and then can deduce how callee params
 *will be set by linphone_core_create_call_params(). This function was developped at a time where the use of the API
 *about incoming params was not yet clarified. Tests relying on this function are then not testing the correct way to
 *use the api (through linphone_core_create_call_params()), and so it is not a so good idea to build new tests based on
 *this function.
 **/
bool_t call_with_params(LinphoneCoreManager *caller_mgr,
                        LinphoneCoreManager *callee_mgr,
                        const LinphoneCallParams *caller_params,
                        const LinphoneCallParams *callee_params) {
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params = {0};
	caller_test_params.base = (LinphoneCallParams *)caller_params;
	callee_test_params.base = (LinphoneCallParams *)callee_params;
	return call_with_params2(caller_mgr, callee_mgr, &caller_test_params, &callee_test_params, FALSE);
}

/*
 * CAUTION this function is error prone. you should not use it anymore in new tests.
 * Creating callee call params before the call is actually received is not the good way
 * to use the Liblinphone API. Indeed, call params used for receiving calls shall be created by
 *linphone_core_create_call_params() by passing the call object for which params are to be created. This function should
 *be used only in test case where the programmer exactly knows the caller params, and then can deduce how callee params
 *will be set by linphone_core_create_call_params(). This function was developped at a time where the use of the API
 *about incoming params was not yet clarified. Tests relying on this function are then not testing the correct way to
 *use the api (through linphone_core_create_call_params()), and so it is not a so good idea to build new tests based on
 *this function.
 **/
bool_t call_with_test_params(LinphoneCoreManager *caller_mgr,
                             LinphoneCoreManager *callee_mgr,
                             const LinphoneCallTestParams *caller_test_params,
                             const LinphoneCallTestParams *callee_test_params) {
	return call_with_params2(caller_mgr, callee_mgr, caller_test_params, callee_test_params, FALSE);
}

bool_t call_with_caller_params(LinphoneCoreManager *caller_mgr,
                               LinphoneCoreManager *callee_mgr,
                               const LinphoneCallParams *params) {
	return call_with_params(caller_mgr, callee_mgr, params, NULL);
}

bool_t call(LinphoneCoreManager *caller_mgr, LinphoneCoreManager *callee_mgr) {
	return call_with_params(caller_mgr, callee_mgr, NULL, NULL);
}

void end_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2) {
	int previous_count_1 = m1->stat.number_of_LinphoneCallEnd;
	int previous_count_2 = m2->stat.number_of_LinphoneCallEnd;
	linphone_core_terminate_all_calls(m1->lc);
	BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallEnd, previous_count_1 + 1));
	BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallEnd, previous_count_2 + 1));
	BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m1->stat.number_of_LinphoneCallReleased, previous_count_1 + 1));
	BC_ASSERT_TRUE(wait_for(m1->lc, m2->lc, &m2->stat.number_of_LinphoneCallReleased, previous_count_2 + 1));
}

static void linphone_conference_server_call_state_changed(LinphoneCore *lc,
                                                          LinphoneCall *call,
                                                          LinphoneCallState cstate,
                                                          BCTBX_UNUSED(const char *msg)) {
	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(lc);
	LinphoneConferenceServer *conf_srv = (LinphoneConferenceServer *)linphone_core_cbs_get_user_data(cbs);
	LinphoneConference *conference = linphone_core_get_conference(lc);

	switch (cstate) {
		case LinphoneCallStreamsRunning:
			if (linphone_call_get_conference(call) == NULL) {
				if (conference == NULL) {
					LinphoneConferenceParams *params = linphone_conference_params_new(lc);
					// When local participant is disabled, the conference is not attached to the core (lc->conf_ctx)
					linphone_conference_params_enable_one_participant_conference(params, TRUE);
					linphone_conference_params_set_subject(params, _linphone_call_get_subject(call));
					linphone_conference_params_enable_video(
					    params, linphone_call_params_video_enabled(linphone_call_get_current_params(call)));
					conference = linphone_core_create_conference_with_params(lc, params);
					linphone_conference_params_unref(params);
					linphone_conference_unref(conference); /*actually linphone_core_create_conference_with_params()
					                                          takes a ref for lc->conf_ctx */
					linphone_conference_add_participant(conference, call);
				}
			}
			if ((linphone_call_get_conference(call) || conference) && (conf_srv->first_call == NULL)) {
				conf_srv->first_call = call;
			}
			if (conference) {
				// local participant should never be in
				linphone_conference_leave(conference);
			}
			break;
		case LinphoneCallEnd:
			if (call == conf_srv->first_call) {
				// Terminate all calls instead of terminating the conference because the latter is static, hence it will
				// never enter the Terminated and Deleted states
				linphone_core_terminate_all_calls(lc);
				conf_srv->first_call = NULL;
			}
			break;

		default:
			break;
	}
}

void linphone_conference_server_refer_received(LinphoneCore *core,
                                               const LinphoneAddress *refer_to_addr,
                                               BCTBX_UNUSED(const LinphoneHeaders *custom_headers),
                                               BCTBX_UNUSED(const LinphoneContent *content)) {
	if (refer_to_addr == NULL) return;

	LinphoneAddress *addr = linphone_address_clone(refer_to_addr);
	char method[20];
	strncpy(method, linphone_address_get_method_param(addr), sizeof(method) - 1);
	method[sizeof(method) - 1] = '\0';
	if (strcmp(method, "BYE") == 0) {
		linphone_address_clean(addr);
		char *uri = linphone_address_as_string_uri_only(addr);
		LinphoneCall *call = linphone_core_find_call_from_uri(core, uri);
		if (call) linphone_call_terminate(call);
		ms_free(uri);
	}
	linphone_address_unref(addr);
}

static void linphone_conference_server_registration_state_changed(LinphoneCore *core,
                                                                  LinphoneProxyConfig *cfg,
                                                                  LinphoneRegistrationState cstate,
                                                                  BCTBX_UNUSED(const char *message)) {
	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(core);
	LinphoneConferenceServer *m = (LinphoneConferenceServer *)linphone_core_cbs_get_user_data(cbs);
	if (cfg == linphone_core_get_default_proxy_config(core)) {
		m->reg_state = cstate;
	}
}

static void linphone_subscribe_received_internal(LinphoneCore *lc,
                                                 BCTBX_UNUSED(LinphoneEvent *lev),
                                                 BCTBX_UNUSED(const char *eventname),
                                                 BCTBX_UNUSED(const LinphoneContent *content)) {
	((LinphoneCoreManager *)linphone_core_get_user_data(lc))->subscription_received++;
}

static void linphone_notify_received_internal(LinphoneCore *lc,
                                              BCTBX_UNUSED(LinphoneEvent *lev),
                                              const char *eventname,
                                              const LinphoneContent *content) {
	LinphoneCoreManager *mgr = get_manager(lc);
	if (content) {
		const char *text = linphone_content_get_utf8_text(content);
		if (linphone_conference_type_is_full_state(text)) {
			mgr->stat.number_of_NotifyFullStateReceived++;
		}
	}
	if (strcmp(eventname, "ekt") == 0) mgr->stat.number_of_NotifyEktReceived++;
	else mgr->stat.number_of_NotifyReceived++;
}

void on_player_eof(LinphonePlayer *player) {
	LinphonePlayerCbs *cbs = linphone_player_get_current_callbacks(player);
	LinphoneCoreManager *marie = (LinphoneCoreManager *)linphone_player_cbs_get_user_data(cbs);
	marie->stat.number_of_player_eof++;
}

LinphoneConferenceServer *linphone_conference_server_new(const char *rc_file, bool_t do_registration) {
	LinphoneConferenceServer *conf_srv = (LinphoneConferenceServer *)ms_new0(LinphoneConferenceServer, 1);
	LinphoneCoreManager *lm = (LinphoneCoreManager *)conf_srv;
	conf_srv->cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_subscription_state_changed(conf_srv->cbs, linphone_subscription_state_change);
	linphone_core_cbs_set_subscribe_received(conf_srv->cbs, linphone_subscribe_received_internal);
	linphone_core_cbs_set_notify_received(conf_srv->cbs, linphone_notify_received_internal);
	linphone_core_cbs_set_call_state_changed(conf_srv->cbs, linphone_conference_server_call_state_changed);
	linphone_core_cbs_set_refer_received(conf_srv->cbs, linphone_conference_server_refer_received);
	linphone_core_cbs_set_registration_state_changed(conf_srv->cbs,
	                                                 linphone_conference_server_registration_state_changed);
	linphone_core_cbs_set_user_data(conf_srv->cbs, conf_srv);
	conf_srv->reg_state = LinphoneRegistrationNone;
	linphone_core_manager_init(lm, rc_file, NULL);
	LinphoneAccount *account = linphone_core_get_default_account(lm->lc);
	LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(account));
	linphone_account_params_enable_register(account_params, do_registration);
	linphone_account_params_set_conference_factory_address(
	    account_params, linphone_account_params_get_identity_address(account_params));
	linphone_account_set_params(account, account_params);
	linphone_account_params_unref(account_params);
	linphone_core_add_callbacks(lm->lc, conf_srv->cbs);
	setup_mgr_for_conference(lm, NULL);
	linphone_core_manager_start(lm, do_registration);
	return conf_srv;
}

static void configure_core_for_conference_callbacks(LinphoneCoreManager *lcm, LinphoneCoreCbs *cbs) {
	_linphone_core_add_callbacks(lcm->lc, cbs, TRUE);
	linphone_core_set_user_data(lcm->lc, lcm);
}

void setup_mgr_for_conference(LinphoneCoreManager *mgr, const char *conference_version) {
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	// Add subscribe and notify received here as when a participant is added, we must wait for its notify is the call
	// goes to StreamRunning
	linphone_core_cbs_set_subscription_state_changed(cbs, linphone_subscription_state_change);
	linphone_core_cbs_set_subscribe_received(cbs, linphone_subscribe_received_internal);
	linphone_core_cbs_set_notify_received(cbs, linphone_notify_received_internal);
	configure_core_for_conference_callbacks(mgr, cbs);
	linphone_core_cbs_unref(cbs);

	linphone_core_set_user_data(mgr->lc, mgr);

	mgr->subscription_received = 0;

	configure_core_for_conference(mgr->lc, NULL, NULL, FALSE);

	if (conference_version) {
		char *spec = bctbx_strdup_printf("conference/%s", conference_version);
		linphone_core_remove_linphone_spec(mgr->lc, "conference");
		linphone_core_add_linphone_spec(mgr->lc, spec);
		bctbx_free(spec);
	}
}

LinphoneCoreManager *
create_mgr_for_conference(const char *rc_file, bool_t check_for_proxies, const char *conference_version) {
	LinphoneCoreManager *mgr = linphone_core_manager_new_with_proxies_check(rc_file, check_for_proxies);

	setup_mgr_for_conference(mgr, conference_version);

	return mgr;
}

void destroy_mgr_in_conference(LinphoneCoreManager *mgr) {
	linphone_core_manager_destroy(mgr);
}

void linphone_conference_server_destroy(LinphoneConferenceServer *conf_srv) {
	linphone_core_cbs_unref(conf_srv->cbs);
	destroy_mgr_in_conference((LinphoneCoreManager *)conf_srv);
}

const char *liblinphone_tester_get_empty_rc(void) {
	if (liblinphone_tester_empty_rc_path == NULL) {
		liblinphone_tester_empty_rc_path = bc_tester_res("rcfiles/empty_rc");
	}
	return liblinphone_tester_empty_rc_path;
}

/*
 * Copy file "from" to file "to".
 * Destination file is truncated if existing.
 * Return 0 on success, positive value on error.
 */
int liblinphone_tester_copy_file(const char *from, const char *to) {
	FILE *in, *out;
	char buf[256];
	size_t n;

	/* Open "from" file for reading */
	in = fopen(from, "rb");
	if (in == NULL) {
		ms_error("Can't open %s for reading: %s\n", from, strerror(errno));
		return 1;
	}

	/* Open "to" file for writing (will truncate existing files) */
	out = fopen(to, "wb");
	if (out == NULL) {
		ms_error("Can't open %s for writing: %s\n", to, strerror(errno));
		fclose(in);
		return 2;
	}

	/* Copy data from "in" to "out" */
	while ((n = fread(buf, sizeof(char), sizeof(buf), in)) > 0) {
		if (!fwrite(buf, 1, n, out)) {
			ms_error("Could not write in %s: %s\n", to, strerror(errno));
			fclose(in);
			fclose(out);
			return 3;
		}
	}

	fclose(in);
	fclose(out);

	return 0;
}

/*
 * Read a file and set its content in a buffer
 * caller must then free the buffer
 * return size read
 */
size_t liblinphone_tester_load_text_file_in_buffer(const char *filePath, char **buffer) {
	FILE *fp = fopen(filePath, "r");
	if (fp == NULL) {
		ms_error("Can't open %s for reading: %s\n", filePath, strerror(errno));
		return 0;
	}

	/* get the size to read */
	if (fseek(fp, 0L, SEEK_END) == 0) {
		long bufsize = ftell(fp);
		*buffer = bctbx_malloc(sizeof(char) * (bufsize + 1)); // +1 to add a '\0'
		/* rewind */
		fseek(fp, 0L, SEEK_SET);
		/* read */
		size_t readSize = fread(*buffer, sizeof(char), bufsize, fp);
		if (ferror(fp) != 0) {
			bctbx_free(*buffer);
			fclose(fp);
			return 0;
		} else {
			(*buffer)[readSize++] = '\0';
			fclose(fp);
			return readSize;
		}
	}
	fclose(fp);
	return 0;
}

static const int flowControlIntervalMs = 5000;
static const int flowControlThresholdMs = 40;

static int dummy_set_sample_rate(BCTBX_UNUSED(MSFilter *obj), BCTBX_UNUSED(void *data)) {
	return 0;
}

static int dummy_get_sample_rate(BCTBX_UNUSED(MSFilter *obj), void *data) {
	int *n = (int *)data;
	*n = 44100;
	return 0;
}

static int dummy_set_nchannels(BCTBX_UNUSED(MSFilter *obj), BCTBX_UNUSED(void *data)) {
	return 0;
}

static int dummy_get_nchannels(BCTBX_UNUSED(MSFilter *obj), void *data) {
	int *n = (int *)data;
	*n = 1;
	return 0;
}

static MSFilterMethod dummy_snd_card_methods[] = {{MS_FILTER_SET_SAMPLE_RATE, dummy_set_sample_rate},
                                                  {MS_FILTER_GET_SAMPLE_RATE, dummy_get_sample_rate},
                                                  {MS_FILTER_SET_NCHANNELS, dummy_set_nchannels},
                                                  {MS_FILTER_GET_NCHANNELS, dummy_get_nchannels},
                                                  {0, NULL}};

struct _DummyOutputContext {
	MSFlowControlledBufferizer buffer;
	int samplerate;
	int nchannels;
	ms_mutex_t mutex;
};

typedef struct _DummyOutputContext DummyOutputContext;

static void dummy_snd_write_init(MSFilter *obj) {
	DummyOutputContext *octx = (DummyOutputContext *)ms_new0(DummyOutputContext, 1);
	octx->samplerate = 44100;
	ms_flow_controlled_bufferizer_init(&octx->buffer, obj, octx->samplerate, 1);
	ms_mutex_init(&octx->mutex, NULL);

	octx->nchannels = 1;

	obj->data = octx;
}

static void dummy_snd_write_uninit(MSFilter *obj) {
	DummyOutputContext *octx = (DummyOutputContext *)obj->data;
	ms_flow_controlled_bufferizer_uninit(&octx->buffer);
	ms_mutex_destroy(&octx->mutex);
	free(octx);
}

static void dummy_snd_write_process(MSFilter *obj) {

	DummyOutputContext *octx = (DummyOutputContext *)obj->data;

	ms_mutex_lock(&octx->mutex);
	// Retrieve data and put them in the filter bugffer ready to be played
	ms_flow_controlled_bufferizer_put_from_queue(&octx->buffer, obj->inputs[0]);
	ms_mutex_unlock(&octx->mutex);
}

MSFilterDesc dummy_filter_write_desc = {MS_FILTER_PLUGIN_ID,
                                        "DummyPlayer",
                                        "dummy player",
                                        MS_FILTER_OTHER,
                                        NULL,
                                        1,
                                        0,
                                        dummy_snd_write_init,
                                        NULL,
                                        dummy_snd_write_process,
                                        NULL,
                                        dummy_snd_write_uninit,
                                        dummy_snd_card_methods};

static MSFilter *dummy_snd_card_create_writer(MSSndCard *card) {
	MSFilter *f = ms_factory_create_filter_from_desc(ms_snd_card_get_factory(card), &dummy_filter_write_desc);
	DummyOutputContext *octx = (DummyOutputContext *)(f->data);
	ms_flow_controlled_bufferizer_set_samplerate(&octx->buffer, octx->samplerate);
	ms_flow_controlled_bufferizer_set_nchannels(&octx->buffer, octx->nchannels);
	ms_flow_controlled_bufferizer_set_max_size_ms(&octx->buffer, flowControlThresholdMs);
	ms_flow_controlled_bufferizer_set_flow_control_interval_ms(&octx->buffer, flowControlIntervalMs);
	return f;
}

struct _DummyInputContext {
	queue_t q;
	MSFlowControlledBufferizer buffer;
	int samplerate;
	int nchannels;
	ms_mutex_t mutex;
};

typedef struct _DummyInputContext DummyInputContext;

static void dummy_snd_read_init(MSFilter *obj) {
	DummyInputContext *ictx = (DummyInputContext *)ms_new0(DummyInputContext, 1);
	ictx->samplerate = 44100;
	ms_flow_controlled_bufferizer_init(&ictx->buffer, obj, ictx->samplerate, 1);
	ms_mutex_init(&ictx->mutex, NULL);
	qinit(&ictx->q);

	ictx->nchannels = 1;

	obj->data = ictx;
}

static void dummy_snd_read_uninit(MSFilter *obj) {
	DummyInputContext *ictx = (DummyInputContext *)obj->data;

	flushq(&ictx->q, 0);
	ms_flow_controlled_bufferizer_uninit(&ictx->buffer);
	ms_mutex_destroy(&ictx->mutex);

	free(ictx);
}

static void dummy_snd_read_process(MSFilter *obj) {

	DummyInputContext *ictx = (DummyInputContext *)obj->data;

	mblk_t *m;

	ms_mutex_lock(&ictx->mutex);
	// Retrieve data and put them in the filter output queue
	while ((m = getq(&ictx->q)) != NULL) {
		ms_queue_put(obj->outputs[0], m);
	}
	ms_mutex_unlock(&ictx->mutex);
}

MSFilterDesc dummy_filter_read_desc = {MS_FILTER_PLUGIN_ID,
                                       "DummyRecorder",
                                       "dummy recorder",
                                       MS_FILTER_OTHER,
                                       NULL,
                                       0,
                                       1,
                                       dummy_snd_read_init,
                                       NULL,
                                       dummy_snd_read_process,
                                       NULL,
                                       dummy_snd_read_uninit,
                                       dummy_snd_card_methods};

static MSFilter *dummy_snd_card_create_reader(MSSndCard *card) {
	MSFilter *f = ms_factory_create_filter_from_desc(ms_snd_card_get_factory(card), &dummy_filter_read_desc);

	DummyInputContext *ictx = (DummyInputContext *)(f->data);
	ms_flow_controlled_bufferizer_set_samplerate(&ictx->buffer, ictx->samplerate);
	ms_flow_controlled_bufferizer_set_nchannels(&ictx->buffer, ictx->nchannels);
	ms_flow_controlled_bufferizer_set_max_size_ms(&ictx->buffer, flowControlThresholdMs);
	ms_flow_controlled_bufferizer_set_flow_control_interval_ms(&ictx->buffer, flowControlIntervalMs);

	return f;
}

static void dummy_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy_test_snd_card_desc = {"dummyTest",
                                          dummy_test_snd_card_detect,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          dummy_snd_card_create_reader,
                                          dummy_snd_card_create_writer,
                                          NULL};

static MSSndCard *create_dummy_test_snd_card(void) {
	MSSndCard *sndcard;
	sndcard = ms_snd_card_new(&dummy_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK | MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy_test_snd_card());
}

static void dummy2_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy2_test_snd_card_desc = {"dummyTest2",
                                           dummy2_test_snd_card_detect,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           dummy_snd_card_create_reader,
                                           dummy_snd_card_create_writer,
                                           NULL};

static MSSndCard *create_dummy2_test_snd_card(void) {
	MSSndCard *sndcard;
	sndcard = ms_snd_card_new(&dummy2_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY2_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK | MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy2_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy2_test_snd_card());
}

static void dummy3_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy3_test_snd_card_desc = {"dummyTest3",
                                           dummy3_test_snd_card_detect,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           dummy_snd_card_create_reader,
                                           dummy_snd_card_create_writer,
                                           NULL};

static MSSndCard *create_dummy3_test_snd_card(void) {
	MSSndCard *sndcard;
	sndcard = ms_snd_card_new(&dummy3_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY3_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK | MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy3_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy3_test_snd_card());
}

static void dummy_playback_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy_playback_test_snd_card_desc = {
    "dummyPlaybackTest",          dummy_playback_test_snd_card_detect, NULL, NULL, NULL, NULL, NULL, NULL,
    dummy_snd_card_create_reader, dummy_snd_card_create_writer,        NULL};

static MSSndCard *create_dummy_playback_test_snd_card(void) {
	MSSndCard *sndcard;
	sndcard = ms_snd_card_new(&dummy_playback_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY_PLAYBACK_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy_playback_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy_playback_test_snd_card());
}

static void dummy_capture_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy_capture_test_snd_card_desc = {
    "dummyCaptureTest",           dummy_capture_test_snd_card_detect, NULL, NULL, NULL, NULL, NULL, NULL,
    dummy_snd_card_create_reader, dummy_snd_card_create_writer,       NULL};

static MSSndCard *create_dummy_capture_test_snd_card(void) {
	MSSndCard *sndcard;
	sndcard = ms_snd_card_new(&dummy_capture_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY_CAPTURE_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy_capture_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy_capture_test_snd_card());
}

const char *limeAlgoEnum2String(const LinphoneTesterLimeAlgo curveId) {
	switch (curveId) {
		case C448:
			return ("c448");
		case C25519:
			return ("c25519");
		case C25519K512:
			return ("c25519k512");
		case C25519MLK512:
			return ("c25519mlk512");
		case C448MLK1024:
			return ("c448mlk1024");
		default:
			return ("unset");
	}
}
/**
 * set the curve and lime server url to use for this test
 * This will crash whatever settings are in the linphonerc for x3dh server
 *
 * @param[in]	curveId		The curveId to use as base for lime
 * @param[in]	manager		The core manager
 * @param[in]	tls_auth_server	True if we must connect to a server trying to authenticate users with client
 * certificate. Other server will use digest auth to authenticate clients
 * @param[in]	req		True: when tls_auth_server is true, connect to a server requesting client authentication using
 * certificates
 * @param[in]	in_account	True: the server URL setting is set in the default account(this is the default behavior)-
 * False: setting in the [lime] section at core level (legacy behavior, do not use)
 *
 */
static void set_lime_server_and_curve_tls(const LinphoneTesterLimeAlgo curveId,
                                          LinphoneCoreManager *manager,
                                          bool_t tls_auth_server,
                                          bool_t req,
                                          bool_t in_account) {
	const char *server = NULL;
	char algo[16];
	switch (curveId) {
		case C25519:
		case C448:
		case C25519K512:
		case C25519MLK512:
		case C448MLK1024:
			sprintf(algo, "%s", limeAlgoEnum2String(curveId));
			break;
		case UNSET: // explicitely disable lime
			sprintf(algo, "%s", "unset");
			linphone_config_set_string(linphone_core_get_config(manager->lc), "lime", "enabled", FALSE);
			linphone_core_set_lime_x3dh_server_url(manager->lc, NULL);
			linphone_config_set_string(linphone_core_get_config(manager->lc), "lime", "curve", "unset");
			return;
		default:
			BC_FAIL("Unknown lime curve setting");
			return;
	}
	// changing the url will restart the encryption engine allowing to also use the changed curve config
	if (tls_auth_server == TRUE) {
		if (req == TRUE) {
			server = lime_server_tlsauth_req_url;
		} else {
			server = lime_server_tlsauth_opt_url;
		}
	} else {
		server = lime_server_url;
	}

	if (in_account) { // This is the way to set the lime server url: in the accounts
		const bctbx_list_t *accountList = linphone_core_get_account_list(manager->lc);
		while (accountList != NULL) {
			LinphoneAccount *account = (LinphoneAccount *)(accountList->data);
			const LinphoneAccountParams *account_params = linphone_account_get_params(account);
			LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
			linphone_account_params_set_lime_algo(new_account_params, algo);
			linphone_account_params_set_lime_server_url(new_account_params, server);
			linphone_account_set_params(account, new_account_params);
			linphone_account_params_unref(new_account_params);
			accountList = accountList->next;
		}
	}
	// this is legacy behavior: Set the lime server url in the core [lime] setting
	// we must set it too as it is used to populate the legacy Ik attribute still in use for testing
	linphone_config_set_string(linphone_core_get_config(manager->lc), "lime", "curve", algo);
	linphone_core_set_lime_x3dh_server_url(manager->lc, server);
}

void set_lime_server_and_curve(const LinphoneTesterLimeAlgo curveId, LinphoneCoreManager *manager) {
	set_lime_server_and_curve_tls(curveId, manager, FALSE, FALSE, TRUE);
}

void legacy_set_lime_server_and_curve(const LinphoneTesterLimeAlgo curveId, LinphoneCoreManager *manager) {
	set_lime_server_and_curve_tls(curveId, manager, FALSE, FALSE, FALSE);
}

void set_lime_server_and_curve_list_tls(const LinphoneTesterLimeAlgo curveId,
                                        bctbx_list_t *managerList,
                                        bool_t tls_auth_server,
                                        bool_t req) {
	bctbx_list_t *item = managerList;
	for (item = managerList; item; item = bctbx_list_next(item)) {
		set_lime_server_and_curve_tls(curveId, (LinphoneCoreManager *)(bctbx_list_get_data(item)), tls_auth_server, req,
		                              TRUE);
	}
}

void set_lime_server_and_curve_list(const LinphoneTesterLimeAlgo curveId, bctbx_list_t *managerList) {
	set_lime_server_and_curve_list_tls(curveId, managerList, FALSE, FALSE);
}

LinphoneNatPolicy *get_nat_policy_for_call(LinphoneCoreManager *mgr, LinphoneCall *call) {
	const LinphoneCallParams *call_params = linphone_call_get_params(call);
	const LinphoneAccount *account = linphone_call_params_get_account(call_params);
	const LinphoneAccountParams *account_params = linphone_account_get_params(account);
	LinphoneNatPolicy *account_nat_policy = linphone_account_params_get_nat_policy(account_params);
	LinphoneNatPolicy *core_nat_policy = linphone_core_get_nat_policy(mgr->lc);
	return (account_nat_policy) ? account_nat_policy : core_nat_policy;
}

void enable_stun_in_mgr(LinphoneCoreManager *mgr,
                        const bool_t account_enable_stun,
                        const bool_t account_enable_ice,
                        const bool_t core_enable_stun,
                        const bool_t core_enable_ice) {
	const bctbx_list_t *accounts = linphone_core_get_account_list(mgr->lc);
	for (const bctbx_list_t *account_it = accounts; account_it != NULL; account_it = account_it->next) {
		LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(account_it));
		enable_stun_in_account(mgr, account, account_enable_stun, account_enable_ice);
	}
	enable_stun_in_core(mgr, core_enable_stun, core_enable_ice);
	linphone_core_manager_wait_for_stun_resolution(mgr);
}

void enable_stun_in_account(LinphoneCoreManager *mgr,
                            LinphoneAccount *account,
                            const bool_t enable_stun,
                            const bool_t enable_ice) {
	LinphoneCore *lc = mgr->lc;
	LinphoneNatPolicy *nat_policy = NULL;
	LinphoneNatPolicy *core_nat_policy = linphone_core_get_nat_policy(lc);
	const LinphoneAccountParams *account_params = linphone_account_get_params(account);
	LinphoneNatPolicy *account_nat_policy = linphone_account_params_get_nat_policy(account_params);
	char *stun_server = NULL;
	char *stun_server_username = NULL;

	if (account_nat_policy != NULL) {
		nat_policy = linphone_nat_policy_clone(account_nat_policy);
	} else if (core_nat_policy != NULL) {
		nat_policy = linphone_nat_policy_clone(core_nat_policy);
	}

	if (nat_policy) {
		stun_server = ms_strdup(linphone_nat_policy_get_stun_server(nat_policy));
		stun_server_username = ms_strdup(linphone_nat_policy_get_stun_server_username(nat_policy));
		linphone_nat_policy_clear(nat_policy);
	} else {
		nat_policy = linphone_core_create_nat_policy(lc);
		stun_server = ms_strdup(linphone_core_get_stun_server(lc));
	}

	linphone_nat_policy_enable_stun(nat_policy, enable_stun);
	linphone_nat_policy_enable_ice(nat_policy, enable_ice);

	if (stun_server_username != NULL) {
		linphone_nat_policy_set_stun_server_username(nat_policy, stun_server_username);
		ms_free(stun_server_username);
	}
	if (stun_server != NULL) {
		linphone_nat_policy_set_stun_server(nat_policy, stun_server);
		ms_free(stun_server);
	}

	LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
	linphone_account_params_set_nat_policy(new_account_params, nat_policy);
	linphone_account_set_params(account, new_account_params);
	linphone_account_params_unref(new_account_params);
	linphone_nat_policy_unref(nat_policy);
}

void enable_stun_in_core(LinphoneCoreManager *mgr, const bool_t enable_stun, const bool_t enable_ice) {
	LinphoneCore *lc = mgr->lc;
	LinphoneNatPolicy *nat_policy = linphone_core_get_nat_policy(lc);
	char *stun_server = NULL;
	char *stun_server_username = NULL;

	if (nat_policy != NULL) {
		nat_policy = linphone_nat_policy_ref(nat_policy);
		stun_server = ms_strdup(linphone_nat_policy_get_stun_server(nat_policy));
		stun_server_username = ms_strdup(linphone_nat_policy_get_stun_server_username(nat_policy));
		linphone_nat_policy_clear(nat_policy);
	} else {
		nat_policy = linphone_core_create_nat_policy(lc);
		stun_server = ms_strdup(linphone_core_get_stun_server(lc));
	}

	linphone_nat_policy_enable_stun(nat_policy, enable_stun);

	if (enable_ice) {
		linphone_nat_policy_enable_ice(nat_policy, TRUE);
	}

	if (stun_server_username != NULL) {
		linphone_nat_policy_set_stun_server_username(nat_policy, stun_server_username);
		ms_free(stun_server_username);
	}
	if (stun_server != NULL) {
		linphone_nat_policy_set_stun_server(nat_policy, stun_server);
		ms_free(stun_server);
	}
	linphone_core_set_nat_policy(lc, nat_policy);
	linphone_nat_policy_unref(nat_policy);
}

int find_matching_participant_info(const LinphoneParticipantInfo *info1, const LinphoneParticipantInfo *info2) {
	return !linphone_address_weak_equal(linphone_participant_info_get_address(info1),
	                                    linphone_participant_info_get_address(info2));
}

LinphoneParticipantInfo *add_participant_info_to_list(bctbx_list_t **participants_info,
                                                      const LinphoneAddress *address,
                                                      LinphoneParticipantRole role,
                                                      int sequence) {
	LinphoneParticipantInfo *ret = NULL;
	LinphoneParticipantInfo *participant_info = linphone_participant_info_new(address);
	linphone_participant_info_set_role(participant_info, role);
	linphone_participant_info_set_sequence_number(participant_info, sequence);
	const bctbx_list_t *participant_info_it = bctbx_list_find_custom(
	    *participants_info, (int (*)(const void *, const void *))find_matching_participant_info, participant_info);
	if (participant_info_it) {
		ret = (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info_it);
	} else {
		ret = linphone_participant_info_ref(participant_info);
		*participants_info = bctbx_list_append(*participants_info, ret);
	}
	linphone_participant_info_unref(participant_info);
	return ret;
}

void conference_scheduler_state_changed(LinphoneConferenceScheduler *scheduler,
                                        LinphoneConferenceSchedulerState state) {
	stats *stat = get_stats(linphone_conference_scheduler_get_core(scheduler));
	switch (state) {
		case LinphoneConferenceSchedulerStateIdle:
			stat->number_of_ConferenceSchedulerStateIdle++;
			break;
		case LinphoneConferenceSchedulerStateAllocationPending:
			stat->number_of_ConferenceSchedulerStateAllocationPending++;
			break;
		case LinphoneConferenceSchedulerStateReady:
			stat->number_of_ConferenceSchedulerStateReady++;
			break;
		case LinphoneConferenceSchedulerStateError:
			stat->number_of_ConferenceSchedulerStateError++;
			break;
		case LinphoneConferenceSchedulerStateUpdating:
			stat->number_of_ConferenceSchedulerStateUpdating++;
			break;
	}
}

void conference_scheduler_invitations_sent(LinphoneConferenceScheduler *scheduler,
                                           BCTBX_UNUSED(const bctbx_list_t *failed_addresses)) {
	stats *stat = get_stats(linphone_conference_scheduler_get_core(scheduler));
	stat->number_of_ConferenceSchedulerInvitationsSent++;
}

void check_conference_info_in_db(LinphoneCoreManager *mgr,
                                 const char *uid,
                                 LinphoneAddress *confAddr,
                                 LinphoneAddress *organizer,
                                 bctbx_list_t *participantList,
                                 long long start_time,
                                 int duration, // in minutes
                                 const char *subject,
                                 const char *description,
                                 unsigned int sequence,
                                 LinphoneConferenceInfoState state,
                                 LinphoneConferenceSecurityLevel security_level,
                                 bool_t skip_participant_info,
                                 bool_t audio_enabled,
                                 bool_t video_enabled,
                                 bool_t chat_enabled) {
	LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
	if (BC_ASSERT_PTR_NOT_NULL(info)) {
		bool_t is_conference_server = linphone_core_conference_server_enabled(mgr->lc);
		// DB conference scheduler sets the description on the server as well whereas CCMP and SIP conference scheduler
		// do not set it. Hence copy the description from the retrieved info and verifiy that it is the same as the one
		// passed as argument should not be NULL
		const char *actual_description = NULL;
		if (!!is_conference_server) {
			actual_description = linphone_conference_info_get_description(info);
			if (actual_description) {
				BC_ASSERT_STRING_EQUAL(description, actual_description);
			}
		} else {
			actual_description = description;
		}
		check_conference_info_members(info, uid, confAddr, organizer, participantList, start_time, duration, subject,
		                              actual_description, sequence, state, security_level, skip_participant_info,
		                              audio_enabled, video_enabled, chat_enabled);
		linphone_conference_info_unref(info);
	}
}

void check_conference_info_against_db(LinphoneCoreManager *mgr,
                                      LinphoneAddress *confAddr,
                                      const LinphoneConferenceInfo *info1,
                                      bool_t skip_participant_info) {
	LinphoneConferenceInfo *info2 = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
	if (BC_ASSERT_PTR_NOT_NULL(info2)) {
		compare_conference_infos(info1, info2, skip_participant_info);
		linphone_conference_info_unref(info2);
	}
}

void check_conference_info_members(const LinphoneConferenceInfo *info,
                                   const char *uid,
                                   LinphoneAddress *confAddr,
                                   LinphoneAddress *organizer,
                                   bctbx_list_t *participantList,
                                   long long start_time,
                                   int duration, // in minutes
                                   const char *subject,
                                   const char *description,
                                   unsigned int sequence,
                                   LinphoneConferenceInfoState state,
                                   LinphoneConferenceSecurityLevel security_level,
                                   bool_t skip_participant_info,
                                   bool_t audio_enabled,
                                   bool_t video_enabled,
                                   bool_t chat_enabled) {
	LinphoneConferenceInfo *info2 = linphone_conference_info_new();
	linphone_conference_info_set_ics_uid(info2, uid);
	linphone_conference_info_set_uri(info2, confAddr);
	linphone_conference_info_set_organizer(info2, organizer);
	linphone_conference_info_set_participant_infos(info2, participantList);
	linphone_conference_info_set_date_time(info2, start_time);
	linphone_conference_info_set_duration(info2, duration);
	linphone_conference_info_set_subject(info2, subject);
	linphone_conference_info_set_description(info2, description);
	linphone_conference_info_set_ics_sequence(info2, sequence);
	linphone_conference_info_set_state(info2, state);
	linphone_conference_info_set_security_level(info2, security_level);
	linphone_conference_info_set_capability(info2, LinphoneStreamTypeAudio, audio_enabled);
	linphone_conference_info_set_capability(info2, LinphoneStreamTypeVideo, video_enabled);
	linphone_conference_info_set_capability(info2, LinphoneStreamTypeText, chat_enabled);
	compare_conference_infos(info, info2, skip_participant_info);
	linphone_conference_info_unref(info2);
}

void compare_conference_infos(const LinphoneConferenceInfo *info1,
                              const LinphoneConferenceInfo *info2,
                              bool_t skip_participant_info) {
	BC_ASSERT_PTR_NOT_NULL(info1);
	BC_ASSERT_PTR_NOT_NULL(info2);
	if (info1 && info2) {
		BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_organizer(info1),
		                                           linphone_conference_info_get_organizer(info2)));

		BC_ASSERT_TRUE(
		    linphone_address_equal(linphone_conference_info_get_uri(info1), linphone_conference_info_get_uri(info2)));

		const bctbx_list_t *info1_participants = linphone_conference_info_get_participant_infos(info1);
		const bctbx_list_t *info2_participants = linphone_conference_info_get_participant_infos(info2);
		BC_ASSERT_EQUAL(bctbx_list_size(info1_participants), bctbx_list_size(info2_participants), size_t, "%zu");
		for (const bctbx_list_t *it = info1_participants; it; it = bctbx_list_next(it)) {
			const LinphoneParticipantInfo *participant_info1 = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
			const bctbx_list_t *participant_info2_it = bctbx_list_find_custom(
			    info2_participants, (int (*)(const void *, const void *))find_matching_participant_info,
			    participant_info1);
			BC_ASSERT_PTR_NOT_NULL(participant_info2_it);
			if (participant_info2_it) {
				const LinphoneParticipantInfo *participant_info2 =
				    (LinphoneParticipantInfo *)bctbx_list_get_data(participant_info2_it);
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_info_get_address(participant_info1),
				                                           linphone_participant_info_get_address(participant_info2)));
				if (!skip_participant_info) {
					BC_ASSERT_EQUAL(linphone_participant_info_get_role(participant_info1),
					                linphone_participant_info_get_role(participant_info2), int, "%0d");
					BC_ASSERT_EQUAL(linphone_participant_info_get_sequence_number(participant_info1),
					                linphone_participant_info_get_sequence_number(participant_info2), int, "%0d");
				}
			}
		}

		BC_ASSERT_EQUAL((int)linphone_conference_info_get_security_level(info1),
		                (int)linphone_conference_info_get_security_level(info2), int, "%0d");

		time_t start_time1 = linphone_conference_info_get_date_time(info1);
		time_t start_time2 = linphone_conference_info_get_date_time(info2);

		if ((start_time1 > 0) && (start_time2 > 0)) {
			BC_ASSERT_EQUAL((long long)start_time1, (long long)start_time2, long long, "%lld");
		} else {
			if ((start_time1 != 0) && (start_time1 != -1)) {
				BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(info1), 0, long long,
				                         "%lld");
			}
			if ((start_time2 != 0) && (start_time2 != -1)) {
				BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(info2), 0, long long,
				                         "%lld");
			}
		}

		const int duration1_m = linphone_conference_info_get_duration(info1);
		const int duration2_m = linphone_conference_info_get_duration(info2);
		BC_ASSERT_EQUAL(duration1_m, duration2_m, int, "%d");

		const char *subject1 = linphone_conference_info_get_subject(info1);
		const char *subject2 = linphone_conference_info_get_subject(info2);
		if (subject1 && subject2) {
			BC_ASSERT_STRING_EQUAL(subject1, subject2);
		} else {
			BC_ASSERT_PTR_NULL(subject1);
			BC_ASSERT_PTR_NULL(subject2);
		}

		const char *description1 = linphone_conference_info_get_description(info1);
		const char *description2 = linphone_conference_info_get_description(info2);
		// Dial out conferences do not have a description set. It may happen that the reference conference information
		// has one as it is built by the tester
		if (duration1_m > 0) {
			if (description1 && description2) {
				BC_ASSERT_STRING_EQUAL(description1, description2);
			} else {
				BC_ASSERT_PTR_NULL(description1);
				BC_ASSERT_PTR_NULL(description2);
			}
		}

		const unsigned int ics_sequence1 = linphone_conference_info_get_ics_sequence(info1);
		const unsigned int ics_sequence2 = linphone_conference_info_get_ics_sequence(info2);
		BC_ASSERT_EQUAL(ics_sequence1, ics_sequence2, int, "%d");
		BC_ASSERT_EQUAL((int)linphone_conference_info_get_state(info1), (int)linphone_conference_info_get_state(info2),
		                int, "%d");

		const char *uid1 = linphone_conference_info_get_ics_uid(info1);
		const char *uid2 = linphone_conference_info_get_ics_uid(info2);
		if (uid1 && uid2) {
			BC_ASSERT_STRING_EQUAL(uid1, uid2);
		}

		const bool_t audio_enabled1 = linphone_conference_info_get_capability(info1, LinphoneStreamTypeAudio);
		const bool_t audio_enabled2 = linphone_conference_info_get_capability(info2, LinphoneStreamTypeAudio);
		BC_ASSERT_EQUAL(audio_enabled1, audio_enabled2, int, "%d");

		const bool_t video_enabled1 = linphone_conference_info_get_capability(info1, LinphoneStreamTypeVideo);
		const bool_t video_enabled2 = linphone_conference_info_get_capability(info2, LinphoneStreamTypeVideo);
		BC_ASSERT_EQUAL(video_enabled1, video_enabled2, int, "%d");

		const bool_t chat_enabled1 = linphone_conference_info_get_capability(info1, LinphoneStreamTypeText);
		const bool_t chat_enabled2 = linphone_conference_info_get_capability(info2, LinphoneStreamTypeText);
		BC_ASSERT_EQUAL(chat_enabled1, chat_enabled2, int, "%d");
	}
}
