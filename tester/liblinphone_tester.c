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

#include "bctoolbox/defs.h"

#include "linphone/core.h"
#include "linphone/logging.h"

#include "tester_utils.h"

#include "liblinphone_tester.h"

static FILE *log_file = NULL;

static const char *liblinphone_helper =
    "\t\t\t--dns-server <hostname or ip address> (specify the DNS server of the flexisip-tester infrastructure used "
    "for the test)\n"
    "\t\t\t--keep-recorded-files\n"
    "\t\t\t--disable-leak-detector\n"
    "\t\t\t--disable-tls-support\n"
    "\t\t\t--no-ipv6 (turn off IPv6 in LinphoneCore, tests requiring IPv6 will be skipped)\n"
    "\t\t\t--ipv6-probing-address IPv6 addr used to probe connectivity, default is 2a01:e00::2)\n"
    "\t\t\t--show-account-manager-logs (show temporary test account creation logs)\n"
    "\t\t\t--no-account-creator (use file database flexisip for account creation)\n"
    "\t\t\t--file-transfer-server-url <url> - override the default "
    "https://transfer.example.org:9444/http-file-transfer-server/hft.php\n"
    "\t\t\t--domain <test sip domain>	(deprecated)\n"
    "\t\t\t--auth-domain <test auth domain>	(deprecated)\n"
    "\t\t\t--dns-hosts </etc/hosts -like file to used to override DNS names or 'none' for no overriding (default: "
    "tester_hosts)> (deprecated)\n"
    "\t\t\t--max-failed  max number of failed tests until program exit with return code 1. Current default is 2\n"
    "\t\t\t--max-cpucount max number of cpu declared at mediastremaer2 level Current default is 2";

typedef struct _MireData {
	MSVideoSize vsize;
	MSPicture pict;
	int index;
	uint64_t starttime;
	float fps;
	mblk_t *pic;
	bool_t keep_fps;
} MireData;

/*
 * Returns the list of ip address for the supplied host name using libc's dns resolver.
 * They are returned as a bctx_list_t of char*, to be freed with bctbx_list_free_with_data(list, bctbx_free).
 */
bctbx_list_t *liblinphone_tester_resolve_name_to_ip_address(const char *name) {
	struct addrinfo *ai, *ai_it;
	struct addrinfo hints;
	bctbx_list_t *ret = NULL;
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	err = getaddrinfo(name, NULL, &hints, &ai);
	if (err != 0) {
		return NULL;
	}
	for (ai_it = ai; ai_it != NULL; ai_it = ai_it->ai_next) {
		char ipaddress[NI_MAXHOST] = {0};
		err = getnameinfo(ai_it->ai_addr, (socklen_t)ai_it->ai_addrlen, ipaddress, sizeof(ipaddress), NULL, 0,
		                  NI_NUMERICHOST | NI_NUMERICSERV);
		if (err != 0) {
			ms_error("liblinphone_tester_resolve_name_to_ip_address(): getnameinfo() error : %s", gai_strerror(err));
			continue;
		}
		ret = bctbx_list_append(ret, bctbx_strdup(ipaddress));
	}
	freeaddrinfo(ai);
	return ret;
}

bctbx_list_t *liblinphone_tester_remove_v6_addr(bctbx_list_t *l) {
	bctbx_list_t *it;
	for (it = l; it != NULL;) {
		char *ip = (char *)l->data;
		if (strchr(ip, ':')) {
			l = bctbx_list_erase_link(l, it);
			bctbx_free(ip);
			it = l;
		} else it = it->next;
	}
	return l;
}

static int liblinphone_tester_start(int argc, char *argv[]) {
	int i;
	int ret;
	int liblinphone_max_failed_tests_threshold =
	    2; /* Please adjust this threshold as long as the full tester becomes more and more reliable. Also update
	          liblinphone_helper value for documentation*/

#ifdef __linux__
	/* Hack to tell mediastreamer2 alsa plugin to not detect direct driver interface ('sysdefault' card), because
	 * it makes ioctls to the driver that hang the system a few seconds on some platforms (observed on Mac+Parallels).
	 * This doesn't prevent alsa to be used during tests, it will be the case with 'default' card.
	 */
	setenv("MS_ALSA_USE_HW", "0", 0);
#endif
	liblinphone_tester_init(NULL);
	linphone_core_set_log_level(ORTP_ERROR);

#ifdef HAVE_CONFIG_H
	// If the tester is not installed we configure it, so it can be launched without installing
	if (!liblinphone_tester_is_executable_installed(argv[0], "rcfiles/marie_rc")) {
		bc_tester_set_resource_dir_prefix(LIBLINPHONE_LOCAL_RESOURCE_LOCATION);
		printf("Resource dir set to %s\n", LIBLINPHONE_LOCAL_RESOURCE_LOCATION);

		liblinphone_tester_add_grammar_loader_path(VCARD_LOCAL_GRAMMAR_LOCATION);
		liblinphone_tester_add_grammar_loader_path(SDP_LOCAL_GRAMMAR_LOCATION);
		liblinphone_tester_add_grammar_loader_path(SIP_LOCAL_GRAMMAR_LOCATION);
		liblinphone_tester_add_grammar_loader_path(LIBLINPHONE_LOCAL_GRAMMARS_LOCATION);

#ifdef HAVE_SOCI
		liblinphone_tester_add_soci_search_path(SOCI_LOCAL_PLUGINS_LOCATION);
#endif
	}
#endif

	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--domain") == 0) {
			CHECK_ARG("--domain", ++i, argc);
			test_domain = argv[i];
		} else if (strcmp(argv[i], "--auth-domain") == 0) {
			CHECK_ARG("--auth-domain", ++i, argc);
			auth_domain = argv[i];
		} else if (strcmp(argv[i], "--dns-hosts") == 0) {
			CHECK_ARG("--dns-hosts", ++i, argc);
			userhostsfile = argv[i];
			flexisip_tester_dns_server = NULL; /* host file is provided, do not use dns server.*/
		} else if (strcmp(argv[i], "--file-transfer-server-url") == 0) {
			CHECK_ARG("--file-transfer-server-url", ++i, argc);
			file_transfer_url = argv[i];
		} else if (strcmp(argv[i], "--dns-server") == 0) {
			CHECK_ARG("--dns-server", ++i, argc);
			flexisip_tester_dns_server = argv[i];
		} else if (strcmp(argv[i], "--keep-recorded-files") == 0) {
			liblinphone_tester_keep_recorded_files(TRUE);
		} else if (strcmp(argv[i], "--disable-leak-detector") == 0) {
			liblinphone_tester_disable_leak_detector(TRUE);
		} else if (strcmp(argv[i], "--disable-tls-support") == 0) {
			liblinphone_tester_tls_support_disabled = TRUE;
		} else if (strcmp(argv[i], "--no-ipv6") == 0) {
			liblinphonetester_ipv6 = FALSE;
		} else if (strcmp(argv[i], "--ipv6-probing-address") == 0) {
			CHECK_ARG("--ipv6-probing-address", ++i, argc);
			liblinphone_tester_ipv6_probing_address = argv[i];
		} else if (strcmp(argv[i], "--show-account-manager-logs") == 0) {
			liblinphonetester_show_account_manager_logs = TRUE;
		} else if (strcmp(argv[i], "--no-account-creator") == 0) {
			liblinphonetester_no_account_creator = TRUE;
		} else if (strcmp(argv[i], "--max-failed") == 0) {
			CHECK_ARG("--max-failed", ++i, argc);
			liblinphone_max_failed_tests_threshold = atoi(argv[i]);
		} else if (strcmp(argv[i], "--max-cpucount") == 0) {
			CHECK_ARG("--max-cpucount", ++i, argc);
			liblinphone_tester_max_cpu_count = atoi(argv[i]);
		} else {
			int bret = bc_tester_parse_args(argc, argv, i);
			if (bret > 0) {
				i += bret - 1;
				continue;
			} else if (bret < 0) {
				bc_tester_helper(argv[0], liblinphone_helper);
			}
			return bret;
		}
	}

	bc_tester_set_max_failed_tests_threshold(liblinphone_max_failed_tests_threshold);

	if (flexisip_tester_dns_server != NULL) {
		/*
		 * We have to remove ipv6 addresses because flexisip-tester internally uses a dnsmasq configuration that does
		 * not listen on ipv6.
		 */
		flexisip_tester_dns_ip_addresses = liblinphone_tester_remove_v6_addr(
		    liblinphone_tester_resolve_name_to_ip_address(flexisip_tester_dns_server));
		if (flexisip_tester_dns_ip_addresses == NULL) {
			ms_error("Cannot resolve the flexisip-tester's dns server name '%s'.", flexisip_tester_dns_server);
			return -1;
		}
	}
	ret = bc_tester_start(argv[0]);
	if (flexisip_tester_dns_ip_addresses) {
		bctbx_list_free_with_data(flexisip_tester_dns_ip_addresses, bctbx_free);
		flexisip_tester_dns_ip_addresses = NULL;
	}
	return ret;
}

static void liblinphone_tester_stop(void) {
	liblinphone_tester_uninit();
}

#ifdef __ANDROID__

#include <android/log.h>
#include <jni.h>
#define CALLBACK_BUFFER_SIZE 1024

static JNIEnv *current_env = NULL;
static jobject current_obj = 0;
jobject system_context = 0; // Application context
static const char *LogDomain = "liblinphone_tester";

int main(int argc, char **argv);

static jstring get_jstring_from_char(JNIEnv *env, const char *cString) {
	int len;
	jmethodID constructorString;
	jbyteArray bytesArray = NULL;
	jstring javaString = NULL;
	jclass classString = (*env)->FindClass(env, "java/lang/String");
	if (classString == NULL) {
		bctbx_error("Cannot find java.lang.String class.\n");
		goto error;
	}

	constructorString = (*env)->GetMethodID(env, classString, "<init>", "([BLjava/lang/String;)V");
	if (constructorString == NULL) {
		bctbx_error("Cannot find String <init> method.\n");
		goto error;
	}

	len = (int)strlen(cString);
	bytesArray = (*env)->NewByteArray(env, len);

	if (bytesArray) {
		(*env)->SetByteArrayRegion(env, bytesArray, 0, len, (jbyte *)cString);
		jstring UTF8 = (*env)->NewStringUTF(env, "UTF8");
		javaString = (jstring)(*env)->NewObject(env, classString, constructorString, bytesArray, UTF8);
		(*env)->DeleteLocalRef(env, bytesArray);
		(*env)->DeleteLocalRef(env, UTF8);
	}

error:
	if (classString) (*env)->DeleteLocalRef(env, classString);

	return javaString;
}

void liblinphone_android_log_handler(int prio, const char *fmt, va_list args) {
	char str[4096];
	char *current;
	char *next;

	vsnprintf(str, sizeof(str) - 1, fmt, args);
	str[sizeof(str) - 1] = '\0';
	if (strlen(str) < 512) {
		__android_log_write(prio, LogDomain, str);
	} else {
		current = str;
		while ((next = strchr(current, '\n')) != NULL) {
			*next = '\0';
			__android_log_write(prio, LogDomain, current);
			current = next + 1;
		}
		__android_log_write(prio, LogDomain, current);
	}
}

static void liblinphone_android_ortp_log_handler(BCTBX_UNUSED(const char *domain),
                                                 OrtpLogLevel lev,
                                                 const char *fmt,
                                                 va_list args) {
	int prio;
	switch (lev) {
		case ORTP_DEBUG:
			prio = ANDROID_LOG_DEBUG;
			break;
		case ORTP_MESSAGE:
			prio = ANDROID_LOG_INFO;
			break;
		case ORTP_WARNING:
			prio = ANDROID_LOG_WARN;
			break;
		case ORTP_ERROR:
			prio = ANDROID_LOG_ERROR;
			break;
		case ORTP_FATAL:
			prio = ANDROID_LOG_FATAL;
			break;
		default:
			prio = ANDROID_LOG_DEFAULT;
			break;
	}
	liblinphone_android_log_handler(prio, fmt, args);
}

static void liblinphone_android_bctbx_log_handler(BCTBX_UNUSED(const char *domain),
                                                  BctbxLogLevel lev,
                                                  const char *fmt,
                                                  va_list args) {
	int prio;
	switch (lev) {
		case BCTBX_LOG_DEBUG:
			prio = ANDROID_LOG_DEBUG;
			break;
		case BCTBX_LOG_MESSAGE:
			prio = ANDROID_LOG_INFO;
			break;
		case BCTBX_LOG_WARNING:
			prio = ANDROID_LOG_WARN;
			break;
		case BCTBX_LOG_ERROR:
			prio = ANDROID_LOG_ERROR;
			break;
		case BCTBX_LOG_FATAL:
			prio = ANDROID_LOG_FATAL;
			break;
		default:
			prio = ANDROID_LOG_DEFAULT;
			break;
	}
	liblinphone_android_log_handler(prio, fmt, args);
}

void bcunit_android_trace_handler(int level, const char *fmt, va_list args) {
	char buffer[CALLBACK_BUFFER_SIZE];
	jstring javaString;
	jclass cls;
	jmethodID method;
	jint javaLevel = level;
	JNIEnv *env = current_env;
	if (env == NULL) return;
	vsnprintf(buffer, CALLBACK_BUFFER_SIZE, fmt, args);
	javaString = (*env)->NewStringUTF(env, buffer);
	cls = (*env)->GetObjectClass(env, current_obj);
	method = (*env)->GetMethodID(env, cls, "printLog", "(ILjava/lang/String;)V");
	(*env)->CallVoidMethod(env, current_obj, method, javaLevel, javaString);
	(*env)->DeleteLocalRef(env, javaString);
	(*env)->DeleteLocalRef(env, cls);
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_setApplicationContext(JNIEnv *env,
                                                                             BCTBX_UNUSED(jclass obj),
                                                                             jobject context) {
	system_context = (jobject)(*env)->NewGlobalRef(env, context);
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_removeApplicationContext(JNIEnv *env, BCTBX_UNUSED(jclass obj)) {
	if (system_context) {
		(*env)->DeleteGlobalRef(env, system_context);
		system_context = 0;
	}
}

JNIEXPORT jint JNICALL Java_org_linphone_tester_Tester_run(JNIEnv *env, jobject obj, jobjectArray stringArray) {
	int i, ret;
	int argc = (*env)->GetArrayLength(env, stringArray);
	char **argv = (char **)malloc(sizeof(char *) * argc);

	for (i = 0; i < argc; i++) {
		jstring string = (jstring)(*env)->GetObjectArrayElement(env, stringArray, i);
		const char *rawString = (const char *)(*env)->GetStringUTFChars(env, string, 0);
		argv[i] = strdup(rawString);
		(*env)->ReleaseStringUTFChars(env, string, rawString);
	}
	current_env = env;
	current_obj = obj;
	bc_set_trace_handler(bcunit_android_trace_handler);
	ret = main(argc, argv);
	current_env = NULL;
	bc_set_trace_handler(NULL);
	for (i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
	return ret;
}

JNIEXPORT jstring JNICALL Java_org_linphone_tester_Tester_run2(JNIEnv *env, jobject obj, jobjectArray stringArray) {
	int i, ret;
	int argc = (*env)->GetArrayLength(env, stringArray);
	char **argv = (char **)malloc(sizeof(char *) * argc);

	for (i = 0; i < argc; i++) {
		jstring string = (jstring)(*env)->GetObjectArrayElement(env, stringArray, i);
		const char *rawString = (const char *)(*env)->GetStringUTFChars(env, string, 0);
		argv[i] = strdup(rawString);
		(*env)->ReleaseStringUTFChars(env, string, rawString);
	}
	current_env = env;
	current_obj = obj;
	bc_set_trace_handler(bcunit_android_trace_handler);
	ret = liblinphone_tester_start(argc, argv);

	jstring failedAsserts = NULL;
	if (ret != 0) {
		char *failed_asserts = bc_tester_get_failed_asserts();
		failedAsserts = get_jstring_from_char(env, failed_asserts);
		free(failed_asserts);
	}

	liblinphone_tester_stop();
	current_env = NULL;
	bc_set_trace_handler(NULL);
	for (i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
	return failedAsserts;
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_keepAccounts(BCTBX_UNUSED(JNIEnv *env),
                                                                    BCTBX_UNUSED(jclass c),
                                                                    jboolean keep) {
	liblinphone_tester_keep_accounts((int)keep);
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_clearAccounts(BCTBX_UNUSED(JNIEnv *env),
                                                                     BCTBX_UNUSED(jclass c)) {
	liblinphone_tester_clear_accounts();
}
#endif /* __ANDROID__ */

static void log_handler(int lev, const char *fmt, va_list args) {
#ifdef _WIN32
	vfprintf(lev == ORTP_ERROR ? stderr : stdout, fmt, args);
	fprintf(lev == ORTP_ERROR ? stderr : stdout, "\n");
#else
	va_list cap;
	va_copy(cap, args);
#ifdef __ANDROID__
	/* IMPORTANT: needed by liblinphone tester to retrieve suite list...*/
	bcunit_android_trace_handler(lev == ORTP_ERROR, fmt, cap);
#else
	/* Otherwise, we must use stdio to avoid log formatting (for autocompletion etc.) */
	vfprintf(lev == ORTP_ERROR ? stderr : stdout, fmt, cap);
	fprintf(lev == ORTP_ERROR ? stderr : stdout, "\n");
#endif
	va_end(cap);
#endif
	bctbx_logv(BCTBX_LOG_DOMAIN, lev, fmt, args);
}

int silent_arg_func(BCTBX_UNUSED(const char *arg)) {
	linphone_core_set_log_level(ORTP_FATAL);
	return 0;
}

int verbose_arg_func(BCTBX_UNUSED(const char *arg)) {
#ifdef DEBUG_LOGS
	linphone_core_set_log_level(ORTP_DEBUG);
#else
	linphone_core_set_log_level(ORTP_MESSAGE);
#endif
	return 0;
}

int logfile_arg_func(const char *arg) {
	if (liblinphone_tester_set_log_file(arg) < 0) return -2;
	return 0;
}

// Set the specific average time if it was not set by the suite.
void liblinphone_tester_add_suite_with_default_time(test_suite_t *suite, int average_time) {
	if (suite->average_time == 0) suite->average_time = average_time;
	bc_tester_add_suite(suite);
}

void liblinphone_tester_add_suites(void) {
	liblinphone_tester_add_suite_with_default_time(&setup_test_suite, 282);
	liblinphone_tester_add_suite_with_default_time(&log_file_test_suite, 5);
	liblinphone_tester_add_suite_with_default_time(&register_test_suite, 267);
#ifdef HAVE_ADVANCED_IM
	liblinphone_tester_add_suite_with_default_time(&group_chat_test_suite, 230);
	liblinphone_tester_add_suite_with_default_time(&group_chat2_test_suite, 402);
	liblinphone_tester_add_suite_with_default_time(&group_chat3_test_suite, 166);
	liblinphone_tester_add_suite_with_default_time(&group_chat4_test_suite, 285);
	liblinphone_tester_add_suite_with_default_time(&cpim_test_suite, 3);
	liblinphone_tester_add_suite_with_default_time(&ics_test_suite, 28);
#ifdef HAVE_LIME_X3DH
	liblinphone_tester_add_suite_with_default_time(&lime_db_test_suite, 500);
	liblinphone_tester_add_suite_with_default_time(&secure_group_chat_test_suite, 393);
	liblinphone_tester_add_suite_with_default_time(&secure_group_chat2_test_suite, 416);
	liblinphone_tester_add_suite_with_default_time(&secure_group_chat_exhume_test_suite, 169);
	liblinphone_tester_add_suite_with_default_time(&secure_message_test_suite, 482);
	liblinphone_tester_add_suite_with_default_time(&secure_message2_test_suite, 278);
	if (liblinphone_tester_is_lime_PQ_available()) {
		liblinphone_tester_add_suite_with_default_time(&secure_group_chat_migration_test_suite, 130);
		liblinphone_tester_add_suite_with_default_time(&secure_group_chat_multialgos_test_suite, 72);
	}
	liblinphone_tester_add_suite_with_default_time(&lime_server_auth_test_suite, 125);
	liblinphone_tester_add_suite_with_default_time(&ephemeral_group_chat_test_suite, 514);
	liblinphone_tester_add_suite_with_default_time(&ephemeral_group_chat_basic_test_suite, 189);
#endif // HAVE_LIME_X3DH
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_conference_edition, 150);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_scheduled_conference_basic, 540);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_scheduled_conference_advanced, 574);
	liblinphone_tester_add_suite_with_default_time(
	    &local_conference_test_suite_scheduled_conference_audio_only_participant, 581);
	liblinphone_tester_add_suite_with_default_time(
	    &local_conference_test_suite_scheduled_conference_with_screen_sharing, 581);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_scheduled_conference_with_chat, 600);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_scheduled_ice_conference, 563);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_impromptu_conference, 384);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_encrypted_impromptu_conference, 150);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_impromptu_mismatch_conference, 210);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_chat_basic, 481);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_chat_advanced, 300);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_chat_error, 261);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_chat_imdn, 315);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_ephemeral_chat, 281);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_secure_chat, 441);
	liblinphone_tester_add_suite_with_default_time(&local_conference_test_suite_transferred_conference_basic, 100);
#endif // HAVE_ADVANCED_IM
	liblinphone_tester_add_suite_with_default_time(&tunnel_test_suite, 0);
	liblinphone_tester_add_suite_with_default_time(&offeranswer_test_suite, 221);
	liblinphone_tester_add_suite_with_default_time(&call_test_suite, 777);
	liblinphone_tester_add_suite_with_default_time(&call2_test_suite, 480);
	bc_tester_add_suite(&call_not_established_test_suite);
	liblinphone_tester_add_suite_with_default_time(&push_incoming_call_test_suite, 65);
	liblinphone_tester_add_suite_with_default_time(&call_recovery_test_suite, 283);
	liblinphone_tester_add_suite_with_default_time(&call_with_ice_test_suite, 494);
	liblinphone_tester_add_suite_with_default_time(&call_secure_test_suite, 747);
	liblinphone_tester_add_suite_with_default_time(&call_secure2_test_suite, 527);
	liblinphone_tester_add_suite_with_default_time(&capability_negotiation_test_suite, 378);
	liblinphone_tester_add_suite_with_default_time(&capability_negotiation_parameters_test_suite, 236);
	liblinphone_tester_add_suite_with_default_time(&capability_negotiation_no_sdp_test_suite, 266);
	liblinphone_tester_add_suite_with_default_time(&srtp_capability_negotiation_basic_test_suite, 495);
	liblinphone_tester_add_suite_with_default_time(&srtp_capability_negotiation_test_suite, 395);
	liblinphone_tester_add_suite_with_default_time(&zrtp_capability_negotiation_basic_test_suite, 468);
	liblinphone_tester_add_suite_with_default_time(&zrtp_capability_negotiation_test_suite, 331);
	liblinphone_tester_add_suite_with_default_time(&dtls_srtp_capability_negotiation_basic_test_suite, 301);
	liblinphone_tester_add_suite_with_default_time(&dtls_srtp_capability_negotiation_test_suite, 173);
	liblinphone_tester_add_suite_with_default_time(&ice_capability_negotiation_test_suite, 10);
	liblinphone_tester_add_suite_with_default_time(&srtp_ice_capability_negotiation_test_suite, 116);
	liblinphone_tester_add_suite_with_default_time(&zrtp_ice_capability_negotiation_test_suite, 95);
	liblinphone_tester_add_suite_with_default_time(&dtls_srtp_ice_capability_negotiation_test_suite, 101);
#ifdef VIDEO_ENABLED
	liblinphone_tester_add_suite_with_default_time(&video_test_suite, 19);
	liblinphone_tester_add_suite_with_default_time(&call_video_test_suite, 620);
	liblinphone_tester_add_suite_with_default_time(&call_video_msogl_test_suite,
	                                               577); // Conditionals are defined in suite
	liblinphone_tester_add_suite_with_default_time(&call_video_advanced_scenarios_test_suite, 168);
	liblinphone_tester_add_suite_with_default_time(&call_video_quality_test_suite, 455);
	liblinphone_tester_add_suite_with_default_time(&alerts_test_suite, 90);
	liblinphone_tester_add_suite_with_default_time(&call_flexfec_suite, 280);
#endif // ifdef VIDEO_ENABLED
	liblinphone_tester_add_suite_with_default_time(&audio_bypass_suite, 11);
	liblinphone_tester_add_suite_with_default_time(&audio_routes_test_suite, 349);
	liblinphone_tester_add_suite_with_default_time(&audio_quality_test_suite, 293);
	liblinphone_tester_add_suite_with_default_time(&audio_video_conference_basic_test_suite, 336);
	liblinphone_tester_add_suite_with_default_time(&audio_video_conference_basic2_test_suite, 199);
	liblinphone_tester_add_suite_with_default_time(&audio_conference_test_suite, 302);
	liblinphone_tester_add_suite_with_default_time(&audio_conference_local_participant_test_suite, 105);
	liblinphone_tester_add_suite_with_default_time(&audio_conference_remote_participant_test_suite, 126);
	liblinphone_tester_add_suite_with_default_time(&video_conference_test_suite, 291);
	liblinphone_tester_add_suite_with_default_time(&video_conference_layout_test_suite, 259);
	liblinphone_tester_add_suite_with_default_time(&ice_conference_test_suite, 163);
	liblinphone_tester_add_suite_with_default_time(&multi_call_test_suite, 73);
	liblinphone_tester_add_suite_with_default_time(&message_test_suite, 521);
	// liblinphone_tester_add_suite_with_default_time(&lime_message_test_suite, 27);
	liblinphone_tester_add_suite_with_default_time(&rtt_message_test_suite, 95);
#ifdef HAVE_BAUDOT
	liblinphone_tester_add_suite_with_default_time(&baudot_message_test_suite, 60);
#endif /* HAVE_BAUDOT */
	liblinphone_tester_add_suite_with_default_time(&session_timers_test_suite, 110);
	liblinphone_tester_add_suite_with_default_time(&presence_test_suite, 77);
	liblinphone_tester_add_suite_with_default_time(&presence_server_test_suite, 339);
	// liblinphone_tester_add_suite_with_default_time(&account_creator_xmlrpc_test_suite, 140);
	liblinphone_tester_add_suite_with_default_time(&account_manager_services_test_suite, 0);
	liblinphone_tester_add_suite_with_default_time(&account_creator_local_test_suite, 3);
#ifdef HAVE_FLEXIAPI
	liblinphone_tester_add_suite_with_default_time(&flexiapiclient_suite, 4);
	liblinphone_tester_add_suite_with_default_time(&account_creator_flexiapi_test_suite, 20);
#endif
	liblinphone_tester_add_suite_with_default_time(&stun_test_suite, 259);
	liblinphone_tester_add_suite_with_default_time(&event_test_suite, 70);
#ifdef HAVE_ADVANCED_IM
	liblinphone_tester_add_suite_with_default_time(&conference_event_test_suite, 32);
#endif
	liblinphone_tester_add_suite_with_default_time(&contents_test_suite, 0);
	liblinphone_tester_add_suite_with_default_time(&flexisip_test_suite, 495);
	liblinphone_tester_add_suite_with_default_time(&remote_provisioning_test_suite, 11);
	liblinphone_tester_add_suite_with_default_time(&quality_reporting_test_suite, 71);
	liblinphone_tester_add_suite_with_default_time(&log_collection_test_suite, 5);
	liblinphone_tester_add_suite_with_default_time(&player_test_suite, 81);
	liblinphone_tester_add_suite_with_default_time(&recorder_test_suite, 33);
	liblinphone_tester_add_suite_with_default_time(&multipart_test_suite, 20);
#ifdef HAVE_EKT_SERVER_PLUGIN
	liblinphone_tester_add_suite_with_default_time(
	    &local_conference_test_suite_end_to_end_encryption_scheduled_conference, 516);
	liblinphone_tester_add_suite_with_default_time(
	    &local_conference_test_suite_end_to_end_encryption_scheduled_conference_audio_only_participant, 337);
	liblinphone_tester_add_suite_with_default_time(
	    &local_conference_test_suite_end_to_end_encryption_impromptu_conference, 155);
	liblinphone_tester_add_suite_with_default_time(
	    &local_conference_test_suite_end_to_end_encryption_scheduled_conference_with_chat, 155);
#endif // HAVE_EKT_SERVER_PLUGIN
	liblinphone_tester_add_suite_with_default_time(&clonable_object_test_suite, 0);
#ifdef HAVE_DB_STORAGE
	liblinphone_tester_add_suite_with_default_time(&main_db_test_suite, 25);
	liblinphone_tester_add_suite_with_default_time(&conference_info_tester, 2);
#endif
	liblinphone_tester_add_suite_with_default_time(&property_container_test_suite, 0);
	liblinphone_tester_add_suite_with_default_time(&multicast_call_test_suite, 83);
	liblinphone_tester_add_suite_with_default_time(&proxy_config_test_suite, 20);
	liblinphone_tester_add_suite_with_default_time(&account_test_suite, 17);
#if HAVE_SIPP
	liblinphone_tester_add_suite_with_default_time(&complex_sip_call_test_suite, 0);
#endif
#ifdef VCARD_ENABLED
	liblinphone_tester_add_suite_with_default_time(&vcard_test_suite, 210);
#endif
	liblinphone_tester_add_suite_with_default_time(&utils_test_suite, 0);
	liblinphone_tester_add_suite_with_default_time(&call_with_rtp_bundle_test_suite, 148);
	liblinphone_tester_add_suite_with_default_time(&shared_core_test_suite, 22);
	liblinphone_tester_add_suite_with_default_time(&vfs_encryption_test_suite, 57);
	liblinphone_tester_add_suite_with_default_time(&external_domain_test_suite, 165);
	liblinphone_tester_add_suite_with_default_time(&potential_configuration_graph_test_suite, 0);
	liblinphone_tester_add_suite_with_default_time(&call_race_conditions_suite, 20);
#ifdef CXX_WRAPPER_ENABLED
	liblinphone_tester_add_suite_with_default_time(&wrapper_cpp_test_suite, 8);
#endif
	liblinphone_tester_add_suite_with_default_time(&mwi_test_suite, 0);
	bc_tester_add_suite(&refer_test_suite);
	bc_tester_add_suite(&bearer_auth_test_suite);
	bc_tester_add_suite(&call_twisted_cases_suite);
	bc_tester_add_suite(&http_client_test_suite);
	bc_tester_add_suite(&turn_server_test_suite);
}

void liblinphone_tester_init(void (*ftester_printf)(int level, const char *fmt, va_list args)) {
	bctbx_init_logger(FALSE);
	if (!log_file) {
#if defined(__ANDROID__)
		linphone_core_set_log_handler(liblinphone_android_ortp_log_handler);
		bctbx_set_log_handler(liblinphone_android_bctbx_log_handler);
#endif
	}

	if (ftester_printf == NULL) ftester_printf = log_handler;
	bc_tester_set_silent_func(silent_arg_func);
	bc_tester_set_verbose_func(verbose_arg_func);
	bc_tester_set_logfile_func(logfile_arg_func);
	bc_tester_init(ftester_printf, ORTP_MESSAGE, ORTP_ERROR, "rcfiles");
	liblinphone_tester_add_suites();
	bc_tester_set_max_parallel_suites(32); /* empiricaly defined as sustainable for our lab 12 threads machine.*/
	bc_tester_set_global_timeout(30 * 60); /* 30 mn max */
}

int liblinphone_tester_set_log_file(const char *filename) {
	if (log_file) {
		fclose(log_file);
	}
	log_file = fopen(filename, "w");
	if (!log_file) {
		ms_error("Cannot open file [%s] for writing logs because [%s]", filename, strerror(errno));
		return -1;
	}
	ms_message("Redirecting traces to file [%s]", filename);
	linphone_core_set_log_file(log_file);
	return 0;
}

// if defunct : Set fps to 0 and keep it on updates. if false : remove fps protection.
void liblinphone_tester_simulate_mire_defunct(MSFilter *filter, bool_t defunct, float fps) {
	if (BC_ASSERT_PTR_NOT_NULL(filter) && BC_ASSERT_PTR_NOT_NULL(filter->data)) {
		if (defunct) {
			ms_filter_call_method(filter, MS_FILTER_SET_FPS, &fps);
		}
		((MireData *)filter->data)->keep_fps = defunct;
	}
}

#if !TARGET_OS_IPHONE && !(defined(LINPHONE_WINDOWS_PHONE) || defined(LINPHONE_WINDOWS_UNIVERSAL))
#if defined(__APPLE__)
int apple_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int ret = liblinphone_tester_start(argc, argv);
	liblinphone_tester_stop();
	return ret;
}
#endif

float liblinphone_tester_get_cpu_bogomips(void) {
	float ret = 0;
#if defined(__linux__)
	char buffer[256] = {0};
	FILE *f = fopen("/proc/cpuinfo", "r");
	if (!f) return ret;
	while (fgets(buffer, sizeof(buffer) - 1, f)) {
		const char *bogomips = "bogomips";
		if (strncasecmp(buffer, bogomips, strlen(bogomips)) == 0) {
			char *semicolon = strchr(buffer, ':');
			if (semicolon) {
				ret = (float)atof(semicolon + 1);
				break;
			}
		}
	}
	fclose(f);
#endif
	return ret;
}

int liblinphone_tester_audio_device_name_match(const LinphoneAudioDevice *audio_device, const char *name) {
	return strcmp(linphone_audio_device_get_device_name(audio_device), name);
}

int liblinphone_tester_audio_device_match(const LinphoneAudioDevice *a, LinphoneAudioDevice *b) {
	return strcmp(linphone_audio_device_get_id(a), linphone_audio_device_get_id(b));
}

bctbx_list_t *liblinphone_tester_find_changing_devices(bctbx_list_t *a, bctbx_list_t *b, bool_t *is_new) {
	bctbx_list_t *devices_changed = NULL;
	bctbx_list_t *device_it = a;
	bctbx_list_t *dev_found = NULL;
	if (!a && !b) return NULL;
	// Check for disconnected device
	while (device_it) {
		LinphoneAudioDevice *device = (LinphoneAudioDevice *)bctbx_list_get_data(device_it);
		dev_found = bctbx_list_find_custom(b, (bctbx_compare_func)liblinphone_tester_audio_device_match, device);
		device_it = bctbx_list_next(device_it);
		if (!dev_found) {
			devices_changed = bctbx_list_append(devices_changed, device);
			linphone_audio_device_ref(device);
			*is_new = FALSE;
		}
	}
	// Check for connected device
	if (!devices_changed) {
		device_it = b;
		while (device_it) {
			LinphoneAudioDevice *device = (LinphoneAudioDevice *)bctbx_list_get_data(device_it);
			dev_found = bctbx_list_find_custom(a, (bctbx_compare_func)liblinphone_tester_audio_device_match, device);
			device_it = bctbx_list_next(device_it);
			if (!dev_found) {
				devices_changed = bctbx_list_append(devices_changed, device);
				linphone_audio_device_ref(device);
				*is_new = TRUE;
			}
		}
	}

	return devices_changed;
}

int liblinphone_tester_sound_detection(LinphoneCoreManager *a,
                                       LinphoneCoreManager *b,
                                       int timeout_ms,
                                       const char *log_tag) {
	int have_sound_count = 0;
	const float silence_threshold = -20.f;

	LinphoneCall *calls[2] = {linphone_core_get_current_call(a->lc), linphone_core_get_current_call(b->lc)};
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while (have_sound_count < 2 &&
	       !liblinphone_tester_clock_elapsed(
	           &start, timeout_ms)) { // We want to avoid potential sound spikes while disconnection.
		float record_levels[2] = {linphone_call_get_record_volume(calls[0]), linphone_call_get_record_volume(calls[1])};
		float playback_levels[2] = {linphone_call_get_play_volume(calls[0]), linphone_call_get_play_volume(calls[1])};
		bool_t have_sounds[2] = {record_levels[1] > silence_threshold && playback_levels[0] > silence_threshold,
		                         record_levels[0] > silence_threshold && playback_levels[1] > silence_threshold};

		// Note: Sounds are send from Pauline to Marie without passing by the capture device (send from file)
		// The test must check for Marie => Pauline, because the sound comes directly from the capture device.
		// At this point, playback device cannot be tested without complex code (device monitoring).
		if (have_sounds[0] && have_sounds[1]) ++have_sound_count;
		else have_sound_count = 0;
		if (log_tag)
			ms_message("%s Record => Playback levels: %f => %f, %f => %f", log_tag, record_levels[0],
			           playback_levels[1], record_levels[1], playback_levels[0]);
		wait_for_until(a->lc, b->lc, NULL, 0, 100);
	}
	return have_sound_count >= 2 ? 0 : -1;
}
