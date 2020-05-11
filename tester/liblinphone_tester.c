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


#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

static FILE * log_file = NULL;

static const char* liblinphone_helper =
		"\t\t\t--domain <test sip domain>\n"
		"\t\t\t--auth-domain <test auth domain>\n"
		"\t\t\t--dns-hosts </etc/hosts -like file to used to override DNS names or 'none' for no overriding (default: tester_hosts)>\n"
		"\t\t\t--keep-recorded-files\n"
		"\t\t\t--disable-leak-detector\n"
		"\t\t\t--disable-tls-support\n"
		"\t\t\t--no-ipv6 (turn off IPv6 in LinphoneCore, tests requiring IPv6 will be skipped)\n"
		"\t\t\t--show-account-manager-logs (show temporary test account creation logs)\n"
		"\t\t\t--no-account-creator (use file database flexisip for account creation)\n"
		;

static int liblinphone_tester_start(int argc, char *argv[]) {
	int i;
	int ret;

#ifdef __linux
	/* Hack to tell mediastreamer2 alsa plugin to not detect direct driver interface ('sysdefault' card), because
	 * it makes ioctls to the driver that hang the system a few seconds on some platforms (observed on Mac+Parallels).
	 * This doesn't prevent alsa to be used during tests, it will be the case with 'default' card.
	 */
	setenv("MS_ALSA_USE_HW", "0", 0);
#endif
	liblinphone_tester_init(NULL);
	linphone_core_set_log_level(ORTP_ERROR);

	for(i = 1; i < argc; ++i) {
		if (strcmp(argv[i],"--domain")==0){
			CHECK_ARG("--domain", ++i, argc);
			test_domain=argv[i];
		} else if (strcmp(argv[i],"--auth-domain")==0){
			CHECK_ARG("--auth-domain", ++i, argc);
			auth_domain=argv[i];
		}else if (strcmp(argv[i],"--dns-hosts")==0){
			CHECK_ARG("--dns-hosts", ++i, argc);
			userhostsfile=argv[i];
		} else if (strcmp(argv[i],"--keep-recorded-files")==0){
			liblinphone_tester_keep_recorded_files(TRUE);
		} else if (strcmp(argv[i],"--disable-leak-detector")==0){
			liblinphone_tester_disable_leak_detector(TRUE);
		} else if (strcmp(argv[i],"--disable-tls-support")==0){
			liblinphone_tester_tls_support_disabled = TRUE;
		} else if (strcmp(argv[i],"--no-ipv6")==0){
			liblinphonetester_ipv6 = FALSE;
		} else if (strcmp(argv[i],"--show-account-manager-logs")==0){
			liblinphonetester_show_account_manager_logs=TRUE;
		} else if (strcmp(argv[i],"--no-account-creator")==0){
			liblinphonetester_no_account_creator=TRUE;
		} else {
			int bret = bc_tester_parse_args(argc, argv, i);
			if (bret>0) {
				i += bret - 1;
				continue;
			} else if (bret<0) {
				bc_tester_helper(argv[0], liblinphone_helper);
			}
			return bret;
		}
	}

	ret = bc_tester_start(argv[0]);
	return ret;
}

static void liblinphone_tester_stop(void) {
	liblinphone_tester_uninit();
}

#ifdef __ANDROID__

#include <android/log.h>
#include <jni.h>
#define CALLBACK_BUFFER_SIZE  1024

static JNIEnv *current_env = NULL;
static jobject current_obj = 0;
jobject system_context = 0; // Application context
static const char* LogDomain = "liblinphone_tester";

int main(int argc, char** argv);

static jstring get_jstring_from_char(JNIEnv *env, const char* cString) {
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

static void liblinphone_android_ortp_log_handler(const char *domain, OrtpLogLevel lev, const char *fmt, va_list args) {
	int prio;
	switch(lev){
		case ORTP_DEBUG:	prio = ANDROID_LOG_DEBUG;	break;
		case ORTP_MESSAGE:	prio = ANDROID_LOG_INFO;	break;
		case ORTP_WARNING:	prio = ANDROID_LOG_WARN;	break;
		case ORTP_ERROR:	prio = ANDROID_LOG_ERROR;	break;
		case ORTP_FATAL:	prio = ANDROID_LOG_FATAL;	break;
		default:			prio = ANDROID_LOG_DEFAULT;	break;
	}
	liblinphone_android_log_handler(prio, fmt, args);
}

static void liblinphone_android_bctbx_log_handler(const char *domain, BctbxLogLevel lev, const char *fmt, va_list args) {
	int prio;
	switch(lev){
		case BCTBX_LOG_DEBUG:	prio = ANDROID_LOG_DEBUG;	break;
		case BCTBX_LOG_MESSAGE:	prio = ANDROID_LOG_INFO;	break;
		case BCTBX_LOG_WARNING:	prio = ANDROID_LOG_WARN;	break;
		case BCTBX_LOG_ERROR:	prio = ANDROID_LOG_ERROR;	break;
		case BCTBX_LOG_FATAL:	prio = ANDROID_LOG_FATAL;	break;
		default:			prio = ANDROID_LOG_DEFAULT;	break;
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
	if(env == NULL) return;
	vsnprintf(buffer, CALLBACK_BUFFER_SIZE, fmt, args);
	javaString = (*env)->NewStringUTF(env, buffer);
	cls = (*env)->GetObjectClass(env, current_obj);
	method = (*env)->GetMethodID(env, cls, "printLog", "(ILjava/lang/String;)V");
	(*env)->CallVoidMethod(env, current_obj, method, javaLevel, javaString);
	(*env)->DeleteLocalRef(env,javaString);
	(*env)->DeleteLocalRef(env,cls);
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_setApplicationContext(JNIEnv *env, jclass obj, jobject context) {
    system_context = (jobject)(*env)->NewGlobalRef(env, context);
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_removeApplicationContext(JNIEnv *env, jclass obj) {
    if (system_context) {
        (*env)->DeleteGlobalRef(env, system_context);
        system_context = 0;
    }
}

JNIEXPORT jint JNICALL Java_org_linphone_tester_Tester_run(JNIEnv *env, jobject obj, jobjectArray stringArray) {
	int i, ret;
	int argc = (*env)->GetArrayLength(env, stringArray);
	char **argv = (char**) malloc(sizeof(char*) * argc);

	for (i=0; i<argc; i++) {
		jstring string = (jstring) (*env)->GetObjectArrayElement(env, stringArray, i);
		const char *rawString = (const char *) (*env)->GetStringUTFChars(env, string, 0);
		argv[i] = strdup(rawString);
		(*env)->ReleaseStringUTFChars(env, string, rawString);
	}
	current_env = env;
	current_obj = obj;
	bc_set_trace_handler(bcunit_android_trace_handler);
	ret = main(argc, argv);
	current_env = NULL;
	bc_set_trace_handler(NULL);
	for (i=0; i<argc; i++) {
		free(argv[i]);
	}
	free(argv);
	return ret;
}

JNIEXPORT jstring JNICALL Java_org_linphone_tester_Tester_run2(JNIEnv *env, jobject obj, jobjectArray stringArray) {
	int i, ret;
	int argc = (*env)->GetArrayLength(env, stringArray);
	char **argv = (char**) malloc(sizeof(char*) * argc);

	for (i=0; i<argc; i++) {
		jstring string = (jstring) (*env)->GetObjectArrayElement(env, stringArray, i);
		const char *rawString = (const char *) (*env)->GetStringUTFChars(env, string, 0);
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
	for (i=0; i<argc; i++) {
		free(argv[i]);
	}
	free(argv);
	return failedAsserts;
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_keepAccounts(JNIEnv *env, jclass c, jboolean keep) {
	liblinphone_tester_keep_accounts((int)keep);
}

JNIEXPORT void JNICALL Java_org_linphone_tester_Tester_clearAccounts(JNIEnv *env, jclass c) {
	liblinphone_tester_clear_accounts();
}
#endif /* __ANDROID__ */

static void log_handler(int lev, const char *fmt, va_list args) {
#ifdef _WIN32
	vfprintf(lev == ORTP_ERROR ? stderr : stdout, fmt, args);
	fprintf(lev == ORTP_ERROR ? stderr : stdout, "\n");
#else
	va_list cap;
	va_copy(cap,args);
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

int silent_arg_func(const char *arg) {
	linphone_core_set_log_level(ORTP_FATAL);
	return 0;
}

int verbose_arg_func(const char *arg) {
	linphone_core_set_log_level(ORTP_MESSAGE);
	return 0;
}

int logfile_arg_func(const char *arg) {
	if (liblinphone_tester_set_log_file(arg) < 0) return -2;
	return 0;
}

void liblinphone_tester_add_suites() {
	bc_tester_add_suite(&setup_test_suite);
	bc_tester_add_suite(&register_test_suite);
#ifdef HAVE_ADVANCED_IM
	bc_tester_add_suite(&group_chat_test_suite);
	bc_tester_add_suite(&secure_group_chat_test_suite);
	bc_tester_add_suite(&local_conference_test_suite);
#endif
	bc_tester_add_suite(&tunnel_test_suite);
	bc_tester_add_suite(&offeranswer_test_suite);
	bc_tester_add_suite(&call_test_suite);
	bc_tester_add_suite(&call_recovery_test_suite);
	bc_tester_add_suite(&call_with_ice_test_suite);
	bc_tester_add_suite(&call_secure_test_suite);
#ifdef VIDEO_ENABLED
	bc_tester_add_suite(&call_video_test_suite);
#endif // ifdef VIDEO_ENABLED
	bc_tester_add_suite(&audio_bypass_suite);
	bc_tester_add_suite(&multi_call_test_suite);
	bc_tester_add_suite(&message_test_suite);
	bc_tester_add_suite(&session_timers_test_suite);
	bc_tester_add_suite(&presence_test_suite);
	bc_tester_add_suite(&presence_server_test_suite);
	bc_tester_add_suite(&account_creator_test_suite);
	bc_tester_add_suite(&stun_test_suite);
	bc_tester_add_suite(&event_test_suite);
#ifdef HAVE_ADVANCED_IM
	bc_tester_add_suite(&conference_event_test_suite);
#endif
	bc_tester_add_suite(&contents_test_suite);
	bc_tester_add_suite(&flexisip_test_suite);
	bc_tester_add_suite(&remote_provisioning_test_suite);
	bc_tester_add_suite(&quality_reporting_test_suite);
	bc_tester_add_suite(&log_collection_test_suite);
	bc_tester_add_suite(&player_test_suite);
	bc_tester_add_suite(&dtmf_test_suite);
	bc_tester_add_suite(&cpim_test_suite);
	bc_tester_add_suite(&multipart_test_suite);
	bc_tester_add_suite(&clonable_object_test_suite);
#ifdef HAVE_DB_STORAGE
	bc_tester_add_suite(&main_db_test_suite);
#endif
	bc_tester_add_suite(&property_container_test_suite);
#ifdef VIDEO_ENABLED
	bc_tester_add_suite(&video_test_suite);
	bc_tester_add_suite(&call_video_quality_test_suite);
#endif // ifdef VIDEO_ENABLED
	bc_tester_add_suite(&multicast_call_test_suite);
	bc_tester_add_suite(&proxy_config_test_suite);
#if HAVE_SIPP
	bc_tester_add_suite(&complex_sip_call_test_suite);
#endif
#ifdef VCARD_ENABLED
	bc_tester_add_suite(&vcard_test_suite);
#endif
	bc_tester_add_suite(&utils_test_suite);
	bc_tester_add_suite(&call_with_rtp_bundle_test_suite);
}

void liblinphone_tester_init(void(*ftester_printf)(int level, const char *fmt, va_list args)) {
	bctbx_init_logger(FALSE);
	if (! log_file) {
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


#if !TARGET_OS_IPHONE && !(defined(LINPHONE_WINDOWS_PHONE) || defined(LINPHONE_WINDOWS_UNIVERSAL))

int main (int argc, char *argv[])
{
	int ret = liblinphone_tester_start(argc, argv);
	liblinphone_tester_stop();
	return ret;
}

#endif
