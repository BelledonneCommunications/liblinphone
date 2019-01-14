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


#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"

static FILE * log_file = NULL;

static uint32_t nb_chatrooms = 1;
static uint32_t nb_participants = 100;
static uint32_t nb_instance_participants = 20;
static uint32_t nb_messages = 100;
//The start user identity index (u_$start_identity@sip.exemple.org)
static uint32_t start_identity = 0;

static bool_t enable_limex3dh = FALSE;

#ifdef __ANDROID__

#include <android/log.h>
#include <jni.h>
#define CALLBACK_BUFFER_SIZE  1024

static JNIEnv *current_env = NULL;
static jobject current_obj = 0;
jobject system_context = 0; // Application context
static const char* LogDomain = "groupchat_benchmark";

int main(int argc, char** argv);

void groupchat_benchmark_android_log_handler(int prio, const char *fmt, va_list args) {
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

static void groupchat_benchmark_android_ortp_log_handler(const char *domain, OrtpLogLevel lev, const char *fmt, va_list args) {
	int prio;
	switch(lev){
		case ORTP_DEBUG:	prio = ANDROID_LOG_DEBUG;	break;
		case ORTP_MESSAGE:	prio = ANDROID_LOG_INFO;	break;
		case ORTP_WARNING:	prio = ANDROID_LOG_WARN;	break;
		case ORTP_ERROR:	prio = ANDROID_LOG_ERROR;	break;
		case ORTP_FATAL:	prio = ANDROID_LOG_FATAL;	break;
		default:			prio = ANDROID_LOG_DEFAULT;	break;
	}
	groupchat_benchmark_android_log_handler(prio, fmt, args);
}

static void groupchat_benchmark_android_bctbx_log_handler(const char *domain, BctbxLogLevel lev, const char *fmt, va_list args) {
	int prio;
	switch(lev){
		case BCTBX_LOG_DEBUG:	prio = ANDROID_LOG_DEBUG;	break;
		case BCTBX_LOG_MESSAGE:	prio = ANDROID_LOG_INFO;	break;
		case BCTBX_LOG_WARNING:	prio = ANDROID_LOG_WARN;	break;
		case BCTBX_LOG_ERROR:	prio = ANDROID_LOG_ERROR;	break;
		case BCTBX_LOG_FATAL:	prio = ANDROID_LOG_FATAL;	break;
		default:			prio = ANDROID_LOG_DEFAULT;	break;
	}
	groupchat_benchmark_android_log_handler(prio, fmt, args);
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

int groupchat_benchmark_tester_set_log_file(const char *filename) {
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

int silent_arg_func(const char *arg) {
	linphone_core_set_log_level(ORTP_FATAL);
	return 0;
}

int verbose_arg_func(const char *arg) {
	linphone_core_set_log_level(ORTP_MESSAGE);
	return 0;
}

int logfile_arg_func(const char *arg) {
	if (groupchat_benchmark_tester_set_log_file(arg) < 0) return -2;
	return 0;
}

//Returns the participants lists
bctbx_list_t *create_participants_addresses(uint32_t participants) {
	uint32_t i;
	char *strAddr;
	LinphoneAddress *addr;
	bctbx_list_t *addresses = NULL;

	if (start_identity != 0) {
		//Only create addresses handled by this instance
		participants = nb_instance_participants;
	}
	for (i = start_identity; i < participants + start_identity; ++i) {
		strAddr = bctbx_strdup_printf("sip:u_%d@sip.example.org", i);
		addr = linphone_address_new(strAddr);
		addresses = bctbx_list_append(addresses, addr);
		bctbx_free(strAddr);
	}
	return addresses;
}

bctbx_list_t *create_conference_cores(bctbx_list_t *participantsAddresses) {
	LinphoneAddress	*addr;
	LinphoneAuthInfo *ai;
	LinphoneCoreManager *mgr;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *it;
	uint32_t i = 0;

	if (enable_limex3dh) {
		mgr = linphone_core_manager_create("groupchat_lime_x3dh_rc");
	} else {
		mgr = linphone_core_manager_create("groupchat_rc");
	}
	//Enable imdn
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(mgr->lc));
	coresManagerList = bctbx_list_append(coresManagerList, mgr);
	for (it = participantsAddresses; it && i < nb_instance_participants; it = bctbx_list_next(it), ++i) {
		LinphoneProxyConfig* proxy_config = linphone_core_create_proxy_config(mgr->lc);
		addr = (LinphoneAddress *) bctbx_list_get_data(it);

		linphone_proxy_config_set_identity_address(proxy_config, addr);
		linphone_proxy_config_set_server_addr(proxy_config, "sip:sip.example.org;transport=tcp");
		linphone_proxy_config_enable_register(proxy_config, TRUE);
		linphone_proxy_config_enable_lime_x3dh(proxy_config, enable_limex3dh);

		BC_ASSERT_EQUAL(linphone_core_add_proxy_config(mgr->lc, proxy_config), 0, int, "%d");
		if (i == 0) {
			linphone_core_set_default_proxy_config(mgr->lc, proxy_config);
		}

		ai = linphone_auth_info_new(linphone_address_get_username(addr),
					    NULL,
					    "secret",
					    NULL,
					    NULL,
					    "sip.example.org");
		linphone_core_add_auth_info(mgr->lc, ai);
		linphone_auth_info_unref(ai);
		linphone_proxy_config_unref(proxy_config);
	}
	return coresManagerList;
}

//Subscribes to	other instances core identities	presence and wait for them to be online
void wait_participants(LinphoneCoreManager *mgr, bctbx_list_t* coresList, bctbx_list_t *participantsAddresses) {
	uint32_t nb_instances =	nb_participants / nb_instance_participants;
	uint32_t i;
	stats stats = mgr->stat;

	for (i = 1; i < nb_instances; ++i) {
		LinphoneAddress *addr = bctbx_list_nth_data(participantsAddresses, i * nb_instance_participants);
		char *identity = linphone_address_as_string_uri_only(addr);
		LinphoneFriend* friend = linphone_core_create_friend_with_address(mgr->lc, identity);
		linphone_friend_edit(friend);
		linphone_friend_enable_subscribes(friend, TRUE);
		linphone_friend_done(friend);

		linphone_core_add_friend(mgr->lc, friend);
		linphone_friend_unref(friend);
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphonePresenceActivityOnline, stats.number_of_LinphonePresenceActivityOnline + nb_instances - 1, nb_participants * 2000));
	linphone_friend_list_enable_subscriptions(linphone_core_get_default_friend_list(mgr->lc), FALSE);
}

//Wait for chatroom creator to be online before waiting	for chatroom invites
void wait_creator(LinphoneCoreManager *mgr, bctbx_list_t* coresList) {
	uint32_t tmpStartId = start_identity;
	start_identity = 0;
	bctbx_list_t *participants = create_participants_addresses(nb_instance_participants), *it;
	start_identity = tmpStartId;
	stats stats = mgr->stat;

	for (it = participants;	it; it = it->next) {
		LinphoneAddress *addr = bctbx_list_get_data(it);
		char *identity = linphone_address_as_string_uri_only(addr);
		LinphoneFriend* friend = linphone_core_create_friend_with_address(mgr->lc, identity);
		linphone_friend_edit(friend);
		linphone_friend_enable_subscribes(friend, TRUE);
		linphone_friend_done(friend);

		linphone_core_add_friend(mgr->lc, friend);
		linphone_friend_unref(friend);
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyPresenceReceived, stats.number_of_NotifyPresenceReceived + nb_instance_participants, nb_instance_participants * 10000));
	linphone_friend_list_enable_subscriptions(linphone_core_get_default_friend_list(mgr->lc), FALSE);
}

int create_chat_room(bctbx_list_t* coresList, LinphoneCoreManager *mgr, const char *subject, bctbx_list_t *participantsAddresses) {
	int ret = 0;
	stats initialStats = mgr->stat;
	LinphoneChatRoom *chatRoom = linphone_core_create_client_group_chat_room_2(mgr->lc, subject, FALSE, enable_limex3dh);
	belle_sip_object_ref(chatRoom);

	if (!chatRoom) return -1;

	ret &= BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateInstantiated, initialStats.number_of_LinphoneChatRoomStateInstantiated + 1, 2000));

	if (enable_limex3dh) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesEncrypted);
	}

	//Remove chatroom creator from invited participants
	linphone_chat_room_add_participants(chatRoom, participantsAddresses->next);

	ret &= BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats.number_of_LinphoneChatRoomStateCreationPending + nb_instance_participants, nb_participants * 2000));
	ret &= BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated, initialStats.number_of_LinphoneChatRoomStateCreated + nb_instance_participants, nb_participants * 2000));

	if (linphone_chat_room_get_nb_participants(chatRoom) != (int) nb_participants - 1) {
		ret = 1;
	}

	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), subject);

	linphone_chat_room_unref(chatRoom);

	return ret;
}

//Wait for chatrooms instances to be created for all proxy configs identities
int wait_chat_room_creation(bctbx_list_t *coresList, LinphoneCoreManager *mgr, const char *subject) {
	int ret = 0;
	stats initialStats = mgr->stat;

	ret &= BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats.number_of_LinphoneChatRoomStateCreationPending + nb_instance_participants, nb_participants * 2000));

	//Wait as long as needed to join the chat room
	ret &= BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated, initialStats.number_of_LinphoneChatRoomStateCreated + nb_instance_participants, nb_participants * 2000));

	return ret;
}

int get_or_create_chat_rooms(LinphoneCoreManager *mgr, bctbx_list_t *coresList, bctbx_list_t *participantsAddresses) {
	int ret = 0;
	uint32_t i;

	if (start_identity == 0) {
		//Wait for other instances participants to be online
		wait_participants(mgr, coresList, participantsAddresses);
	} else {
		//Wait for chatroom creator to be online
		wait_creator(mgr, coresList);
	}
	for (i = 0; i < nb_chatrooms; ++i) {
		char *subject =	bctbx_strdup_printf("Chat room %d subject", i);

		if (start_identity == 0) {
			//The instance handling u_0 identity is the creator of all chatrooms
			ret &= create_chat_room(coresList, mgr, subject, participantsAddresses);
		} else {
			ret &= wait_chat_room_creation(coresList, mgr, subject);
		}
		bctbx_free(subject);
	}
	return ret;
}

void send_messages(LinphoneCoreManager *mgr, bctbx_list_t *coresList, uint32_t messages) {
	const bctbx_list_t *coreChatRooms = linphone_core_get_chat_rooms(mgr->lc);
	uint32_t i;
	const bctbx_list_t *it;
	bctbx_list_t *messagesList;
	const LinphoneAddress *coreAddr = linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(mgr->lc));
	stats stats = mgr->stat;

	for (it = coreChatRooms; it; it = it->next) {
		chatRoom = it->data;

		if (!linphone_address_weak_equal(coreAddr, linphone_chat_room_get_local_address(it->data))) {
			//Only send messages from default identity
			continue;
		}
		linphone_chat_room_compose(it->data);

		const char *localCrAddr = linphone_address_as_string(linphone_chat_room_get_local_address(it->data));

		messagesList = NULL;

		char *message =	bctbx_strdup_printf("Hi! I'm %s", localCrAddr);

		for (i = 0; i < messages; ++i) {
			messagesList = bctbx_list_append(messagesList, _send_message(it->data, message));
			ms_sleep(1);
		}

		bctbx_free(message);

		wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageDelivered, stats.number_of_LinphoneMessageDelivered + messages, messages * 1000);

		bctbx_list_free_with_data(messagesList, (bctbx_list_free_func) belle_sip_object_unref);
	}
}

void groupchat_benchmark(void) {
	bctbx_list_t *coresList = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;

	liblinphonetester_no_account_creator = TRUE; //Explicitly disable automatic account creator

	participantsAddresses = create_participants_addresses(nb_participants);

	coresManagerList = create_conference_cores(participantsAddresses);

	coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	if (get_or_create_chat_rooms(coresManagerList->data, coresList, participantsAddresses) != 0) {
		ms_fatal("Failed to create all chatrooms!");
	} else {
		send_messages(coresManagerList->data, coresList, nb_messages);
	}

	//cleanup
	const bctbx_list_t *coreChatRooms = linphone_core_get_chat_rooms(((LinphoneCoreManager*) coresManagerList->data)->lc);
	const bctbx_list_t *it;
	for (it = coreChatRooms; it; it = it->next) {
		linphone_core_manager_delete_chat_room(coresManagerList->data, it->data, coresList);
	}
	bctbx_list_free(coresList);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func) linphone_address_unref);
	linphone_core_manager_destroy(coresManagerList->data);
	bctbx_list_free(coresManagerList);
}

int check_params(void) {
	if (nb_participants < 2) {
		bctbx_fatal("There must be at least 2 participants to create chat rooms!");
		return -1;
	}
	if (nb_chatrooms < 1) {
		bctbx_fatal("There must be at least 1 chat rooms!");
		return -1;
	}
	return 0;
}

void groupchat_benchmark_init(void(*ftester_printf)(int level, const char *fmt, va_list args)) {
	bctbx_init_logger(FALSE);
	if (! log_file) {
#if defined(__ANDROID__)
		linphone_core_set_log_handler(groupchat_benchmark_android_ortp_log_handler);
		bctbx_set_log_handler(groupchat_benchmark_android_bctbx_log_handler);
#endif
	}
	if (ftester_printf == NULL) ftester_printf = log_handler;
	bc_tester_set_silent_func(silent_arg_func);
	bc_tester_set_verbose_func(verbose_arg_func);
	bc_tester_set_logfile_func(logfile_arg_func);
	bc_tester_init(ftester_printf, ORTP_MESSAGE, ORTP_ERROR, "rcfiles");
}

void groupchat_benchmark_uninit(void) {
	bc_tester_uninit();
	bctbx_uninit_logger();
}

#if !TARGET_OS_IPHONE && !(defined(LINPHONE_WINDOWS_PHONE) || defined(LINPHONE_WINDOWS_UNIVERSAL))

//See https://wiki.linphone.org/xwiki/bin/view/Engineering/Benchmark%20Flexisip%20-%20ChatRooms/ for more details
static const char* groupchat_benchmark_helper =
	"\t\t\t--chat-rooms <nb_chat_rooms> (Number of chat rooms to create)\n"
	"\t\t\t--participants <nb_participants> (Total number of participants)\n"
	"\t\t\t--instance-participants <participants> (Number of participants handled by this instance)\n"
	"\t\t\t--start-identity <index> (Index of the first identity of participants, between 0 and <participants>)\n"
	"\t\t\t--messages <nb_messages> (Number of messages this instance will send to each chatroom)\n"
	"\t\t\t--lime (Enable lime x3dh encrypted chat rooms)\n"
	"\t\t\t--domain <test sip domain>\n"
	"\t\t\t--auth-domain <test auth domain>\n"
	"\t\t\t--dns-hosts </etc/hosts -like file to used to override DNS names (default: tester_hosts)>\n"
	"\t\t\t--keep-recorded-files\n"
	"\t\t\t--disable-leak-detector\n"
	"\t\t\t--disable-tls-support\n"
	"\t\t\t--no-ipv6 (turn off IPv6 in LinphoneCore, tests requiring IPv6 will be skipped)\n"
	"\t\t\t--show-account-manager-logs (show temporary test account creation logs)\n"
	"\t\t\t--no-account-creator (use file database flexisip for account creation)\n"
	;

int main (int argc, char *argv[])
{
	int i;
	int ret;

	groupchat_benchmark_init(NULL);
	linphone_core_set_log_level(ORTP_ERROR);

	test_t setup_tests = TEST_NO_TAG("Group chat benchmark", groupchat_benchmark);
	test_suite_t test_suite = {"Group Chat Benchmark", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each, 1, &setup_tests};
	bc_tester_add_suite(&test_suite);

	for(i = 1; i < argc; ++i) {
		if (strcmp(argv[i],"--chat-rooms")==0){
			CHECK_ARG("--chat-rooms", ++i, argc);
			nb_chatrooms=atoi(argv[i]);
		} else if (strcmp(argv[i],"--participants")==0){
			CHECK_ARG("--participants", ++i, argc);
			nb_participants=atoi(argv[i]);
		} else if (strcmp(argv[i],"--instance-participants")==0){
			CHECK_ARG("--instance-participants", ++i, argc);
			nb_instance_participants=atoi(argv[i]);
		} else if (strcmp(argv[i],"--start-identity")==0){
			CHECK_ARG("--start-identity", ++i, argc);
			start_identity=atoi(argv[i]);
		} else if (strcmp(argv[i],"--messages")==0){
			CHECK_ARG("--messages", ++i, argc);
			nb_messages=atoi(argv[i]);
		} else if (strcmp(argv[i],"--lime")==0){
			enable_limex3dh = TRUE;
		} else if (strcmp(argv[i],"--domain")==0){
			CHECK_ARG("--domain", ++i, argc);
			test_domain=argv[i];
		} else if (strcmp(argv[i],"--auth-domain")==0){
			CHECK_ARG("--auth-domain", ++i, argc);
			auth_domain=argv[i];
		} else if (strcmp(argv[i],"--dns-hosts")==0){
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
				bc_tester_helper(argv[0], groupchat_benchmark_helper);
			}
			return bret;
		}
	}

	if (check_params() != 0) {
		return -1;
	}
	ret = bc_tester_start(argv[0]);
	groupchat_benchmark_uninit();
	return ret;
}


#endif
