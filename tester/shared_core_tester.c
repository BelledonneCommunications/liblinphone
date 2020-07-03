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

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include "bctoolbox/crypto.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/api/c-types.h"
#include "linphone/core.h"
#include "linphone/wrapper_utils.h"
#include "tester_utils.h"

#if TARGET_OS_IPHONE
#include <pthread.h>
#endif

#define TEST_GROUP_ID "test group id"

struct get_msg_args {
	LinphoneCoreManager *sender_mgr;
	LinphoneCoreManager *receiver_mgr;

	// msg
	const char *text;
	const char *call_id;

	// chat room
	LinphoneChatRoom *sender_cr;
	const char *subject;
};

static void shared_main_core_prevent_executor_core_start(void) {
#if TARGET_OS_IPHONE
	LinphoneCoreManager *main_mgr;
	LinphoneCoreManager *executor_mgr;
	main_mgr = linphone_core_manager_create_shared("marie_rc", TEST_GROUP_ID, TRUE, NULL);
	linphone_core_set_auto_iterate_enabled(main_mgr->lc, FALSE);
	executor_mgr = linphone_core_manager_create_shared("", TEST_GROUP_ID, FALSE, main_mgr);
	linphone_core_set_auto_iterate_enabled(executor_mgr->lc, FALSE);

	linphone_core_manager_start(main_mgr, TRUE);
	BC_ASSERT_TRUE(wait_for_until(main_mgr->lc, NULL, &main_mgr->stat.number_of_LinphoneGlobalOn, 1, 2000));
	BC_ASSERT_EQUAL(linphone_core_start(executor_mgr->lc), -1, int, "%d");

	linphone_core_manager_destroy(main_mgr);
	linphone_core_manager_destroy(executor_mgr);
#endif
}

void *thread_shared_main_core_stops_executor_core(void *arguments) {
#if TARGET_OS_IPHONE
	LinphoneCoreManager *executor_mgr = (LinphoneCoreManager *)arguments;
	LinphoneCoreManager *main_mgr = linphone_core_manager_create_shared("", TEST_GROUP_ID, TRUE, executor_mgr);
	linphone_core_set_auto_iterate_enabled(main_mgr->lc, FALSE);
	ms_sleep(5); // for synchro with main thread
	linphone_core_manager_start(main_mgr, TRUE);
	BC_ASSERT_TRUE(wait_for_until(main_mgr->lc, NULL, &main_mgr->stat.number_of_LinphoneGlobalOn, 1, 2000));
	linphone_core_manager_destroy(main_mgr);

	pthread_exit(NULL);
#endif
	return NULL;
}

static void shared_main_core_stops_executor_core(void) {
#if TARGET_OS_IPHONE
	LinphoneCoreManager *executor_mgr;
	executor_mgr = linphone_core_manager_create_shared("marie_rc", TEST_GROUP_ID, FALSE, NULL);
	linphone_core_set_auto_iterate_enabled(executor_mgr->lc, FALSE);

	linphone_core_manager_start(executor_mgr, TRUE);
	BC_ASSERT_TRUE(wait_for_until(executor_mgr->lc, NULL, &executor_mgr->stat.number_of_LinphoneGlobalOn, 1, 2000));

	pthread_t main_core_thread;
	if (pthread_create(&main_core_thread, NULL, &thread_shared_main_core_stops_executor_core, (void *)executor_mgr)) {
		ms_fatal("Error creating main_core_thread thread");
	}

	// linphone_core_stop() will be called by this when main core try to start
	linphone_core_get_new_message_from_callid(executor_mgr->lc, "dummy_call_id");

	BC_ASSERT_TRUE(
		wait_for_until(executor_mgr->lc, NULL, &executor_mgr->stat.number_of_LinphoneGlobalShutdown, 1, 2000));

	if (pthread_join(main_core_thread, NULL)) {
		ms_fatal("Error joining thread main_core_thread");
	}

	BC_ASSERT_TRUE(linphone_core_get_global_state(executor_mgr->lc) == LinphoneGlobalOff);
	linphone_core_manager_destroy(executor_mgr);
#endif
}

const char *shared_core_send_msg_and_get_call_id(LinphoneCoreManager *sender, LinphoneCoreManager *receiver,
												 const char *text) {
	stats initial_sender_stat = sender->stat;
	const char *content_type = "text/plain";
	LinphoneChatRoom *room = linphone_core_get_chat_room(sender->lc, receiver->identity);

	LinphoneChatMessage *msg = linphone_chat_room_create_message(room, text);
	linphone_chat_message_set_content_type(msg, content_type);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for_until(sender->lc, NULL, &sender->stat.number_of_LinphoneMessageDelivered,
								  initial_sender_stat.number_of_LinphoneMessageDelivered + 1, 30000));
	const char *call_id = linphone_chat_message_get_message_id(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_FALSE(strcmp(call_id, "") == 0);

	return ms_strdup(call_id);
}

// receiver needs to be a shared core
void shared_core_get_message_from_call_id(LinphoneCoreManager *sender_mgr, LinphoneCore *receiver, const char *text,
										  const char *call_id) {

	BC_ASSERT_PTR_NOT_NULL(call_id);
	if (call_id != NULL) {
		LinphonePushNotificationMessage *received_msg = linphone_core_get_new_message_from_callid(receiver, call_id);
		BC_ASSERT_PTR_NOT_NULL(received_msg);

		if (received_msg != NULL) {
			LinphoneProxyConfig *receiver_cfg = linphone_core_get_proxy_config_list(receiver)->data;
			const LinphoneAddress *receiver_addr = linphone_proxy_config_get_identity_address(receiver_cfg);
			const LinphoneAddress *local_addr = linphone_push_notification_message_get_local_addr(received_msg);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_username(receiver_addr), linphone_address_get_username(local_addr));
			linphone_address_unref((LinphoneAddress *)local_addr);

			LinphoneProxyConfig *sender_cfg = linphone_core_get_proxy_config_list(sender_mgr->lc)->data;
			const LinphoneAddress *sender_addr = linphone_proxy_config_get_identity_address(sender_cfg);
			const LinphoneAddress *from_addr = linphone_push_notification_message_get_from_addr(received_msg);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_username(sender_addr), linphone_address_get_username(from_addr));
			linphone_address_unref((LinphoneAddress *)from_addr);

			BC_ASSERT_STRING_EQUAL(linphone_push_notification_message_get_text_content(received_msg), text);
			linphone_push_notification_message_unref(received_msg);

			BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(receiver, sender_mgr->identity));
		}
	}
}

void *thread_shared_core_get_message_from_call_id(void *arguments) {
#if TARGET_OS_IPHONE
	struct get_msg_args *args = (struct get_msg_args *)arguments;
	LinphoneCoreManager *receiver_mgr =
	linphone_core_manager_create_shared("", TEST_GROUP_ID, FALSE, args->receiver_mgr);

	shared_core_get_message_from_call_id(args->sender_mgr, receiver_mgr->lc, args->text, args->call_id);

	linphone_core_manager_destroy(receiver_mgr);

	pthread_exit(NULL);
#endif
	return NULL;
}

// receiver needs to be a shared core
void shared_core_get_message_from_user_defaults(LinphoneCoreManager *sender_mgr, LinphoneCore *receiver, const char *call_id) {
	BC_ASSERT_PTR_NOT_NULL(call_id);
	if (call_id != NULL) {
		LinphonePushNotificationMessage *received_msg = linphone_core_get_new_message_from_callid(receiver, call_id);
		BC_ASSERT_PTR_NOT_NULL(received_msg);

		if (received_msg != NULL) {
			const LinphoneAddress *local_addr = linphone_push_notification_message_get_local_addr(received_msg);
			BC_ASSERT_STRING_EQUAL(linphone_address_as_string(local_addr), "sip:local.addr");
			linphone_address_unref((LinphoneAddress *)local_addr);

			const LinphoneAddress *from_addr = linphone_push_notification_message_get_from_addr(received_msg);
			BC_ASSERT_STRING_EQUAL(linphone_address_as_string(from_addr), "sip:from.addr");
			linphone_address_unref((LinphoneAddress *)from_addr);

			BC_ASSERT_STRING_EQUAL(linphone_push_notification_message_get_text_content(received_msg), "textContent");
			linphone_push_notification_message_unref(received_msg);
		}
	}
}

void *thread_shared_core_get_message_from_user_defaults(void *arguments) {
#if TARGET_OS_IPHONE
	struct get_msg_args *args = (struct get_msg_args *)arguments;
	LinphoneCoreManager *receiver_mgr =
	linphone_core_manager_create_shared("", TEST_GROUP_ID, FALSE, args->receiver_mgr);

	shared_core_get_message_from_user_defaults(args->sender_mgr, receiver_mgr->lc, args->call_id);

	linphone_core_manager_destroy(receiver_mgr);

	pthread_exit(NULL);
#endif
	return NULL;
}

// This test suite does not work yet, so do not test "automatic iterator".
// In addition, "automatic iterator" must be disabled for extensions.
LinphoneCoreManager *linphone_core_manager_new_without_auto_iterate(const char *rc_file) {
	LinphoneCoreManager *manager = linphone_core_manager_create2(rc_file, NULL);
	linphone_core_set_auto_iterate_enabled(manager->lc, FALSE);
	linphone_core_manager_start(manager, TRUE);
	return manager;
}

static void shared_executor_core_get_message_by_starting_a_core(void) {
#if TARGET_OS_IPHONE
	const char *text = "Bli bli bli \n blu";
	LinphoneCoreManager *sender_mgr = linphone_core_manager_new_without_auto_iterate("marie_rc");
	LinphoneCoreManager *receiver_mgr = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, FALSE, NULL);
	linphone_core_set_auto_iterate_enabled(receiver_mgr->lc, FALSE);
	linphone_core_manager_start(receiver_mgr, TRUE);

	const char *call_id = shared_core_send_msg_and_get_call_id(sender_mgr, receiver_mgr, text);
	if (call_id) {
		// This is an executor core. No other executor core is started so it can get the msg.
		shared_core_get_message_from_call_id(sender_mgr, receiver_mgr->lc, text, call_id);
		ms_free((void *)call_id);
	}
	linphone_core_manager_destroy(sender_mgr);
	linphone_core_manager_destroy(receiver_mgr);
#endif
}

static void shared_executor_core_get_message_with_user_defaults_mono_thread(void) {
#if TARGET_OS_IPHONE
	/* mono thread means that the msg in already in the user defaults when the executor core start */
	const char *text = "Bli bli bli \n blu";
	LinphoneCoreManager *sender_mgr = linphone_core_manager_new_without_auto_iterate("marie_rc");
	LinphoneCoreManager *main_mgr = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, TRUE, NULL);
	linphone_core_set_auto_iterate_enabled(main_mgr->lc, FALSE);
	linphone_core_manager_start(main_mgr, TRUE);

	const char *call_id = shared_core_send_msg_and_get_call_id(sender_mgr, main_mgr, text);
	BC_ASSERT_TRUE(wait_for_until(main_mgr->lc, sender_mgr->lc, &main_mgr->stat.number_of_LinphoneMessageReceived, 1, 30000));
	if (call_id) {
		LinphoneCoreManager *executor_mgr = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, FALSE, NULL);
		// Manually mark the msg as received as user defaults are not available in iphone simulators
		linphone_shared_core_helpers_on_msg_written_in_user_defaults(executor_mgr->lc);
		shared_core_get_message_from_user_defaults(sender_mgr, executor_mgr->lc, call_id);
		ms_free((void *)call_id);
		linphone_core_manager_destroy(executor_mgr);
	}
	linphone_core_manager_destroy(sender_mgr);
	linphone_core_manager_destroy(main_mgr);
#endif
}

static void shared_executor_core_get_message_with_user_defaults_multi_thread(void) {
#if TARGET_OS_IPHONE
	/* multi thread means that the executor core waits for the msg to be written by the main core into the user defaults */
	const char *text = "Bli bli bli \n blu";
	LinphoneCoreManager *sender_mgr = linphone_core_manager_new_without_auto_iterate("marie_rc");
	LinphoneCoreManager *main_mgr = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, TRUE, NULL);
	linphone_core_set_auto_iterate_enabled(main_mgr->lc, FALSE);
	linphone_core_manager_start(main_mgr, TRUE);

	pthread_t executor;
	struct get_msg_args *args = ms_malloc(sizeof(struct get_msg_args));
	args->sender_mgr = sender_mgr;
	args->receiver_mgr = main_mgr;
	args->call_id = "call_id";
	if (pthread_create(&executor, NULL, &thread_shared_core_get_message_from_user_defaults, (void *)args)) {
		ms_fatal("Error creating executor thread");
	}

	// make sure that executor core is waiting for the msg before we send it
	ms_sleep(1);

	shared_core_send_msg_and_get_call_id(sender_mgr, main_mgr, text);
	BC_ASSERT_TRUE(wait_for_until(main_mgr->lc, sender_mgr->lc, &main_mgr->stat.number_of_LinphoneMessageReceived, 1, 30000));

	if (pthread_join(executor, NULL)) {
		ms_fatal("Error joining thread executor");
	}

	ms_free(args);

	linphone_core_manager_destroy(sender_mgr);
	linphone_core_manager_destroy(main_mgr);
#endif
}

static void two_shared_executor_cores_get_messages(void) {
#if TARGET_OS_IPHONE
	LinphoneCoreManager *sender_mgr = linphone_core_manager_new_without_auto_iterate("marie_rc");
	LinphoneCoreManager *receiver_mgr = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, FALSE, NULL);
	linphone_core_set_auto_iterate_enabled(receiver_mgr->lc, FALSE);

	linphone_core_manager_start(receiver_mgr, TRUE);
	const char *call_id1 = shared_core_send_msg_and_get_call_id(sender_mgr, receiver_mgr, "message1");
	if (strcmp(call_id1, "") == 0)
		return;
	const char *call_id2 = shared_core_send_msg_and_get_call_id(sender_mgr, receiver_mgr, "message2");
	if (strcmp(call_id2, "") == 0)
		return;
	linphone_core_stop(receiver_mgr->lc);

	pthread_t executor1;
	struct get_msg_args *args1 = ms_malloc(sizeof(struct get_msg_args));
	args1->sender_mgr = sender_mgr;
	args1->receiver_mgr = receiver_mgr;
	args1->text = "message1";
	args1->call_id = call_id1;
	if (pthread_create(&executor1, NULL, &thread_shared_core_get_message_from_call_id, (void *)args1)) {
		ms_fatal("Error creating executor1 thread");
	}

	ms_sleep(5);

	pthread_t executor2;
	struct get_msg_args *args2 = ms_malloc(sizeof(struct get_msg_args));
	args2->sender_mgr = sender_mgr;
	args2->receiver_mgr = receiver_mgr;
	args2->text = "message2";
	args2->call_id = call_id2;
	if (pthread_create(&executor2, NULL, &thread_shared_core_get_message_from_call_id, (void *)args2)) {
		ms_fatal("Error creating executor2 thread");
	}

	if (pthread_join(executor1, NULL)) {
		ms_fatal("Error joining thread executor1");
	}
	if (pthread_join(executor2, NULL)) {
		ms_fatal("Error joining thread executor2");
	}

	ms_free(args1);
	ms_free(args2);

	if (call_id1) {
		ms_free((void *)call_id1);
	}
	if (call_id2) {
		ms_free((void *)call_id2);
	}

	linphone_core_manager_destroy(receiver_mgr);
	linphone_core_manager_destroy(sender_mgr);
#endif
}

LinphoneChatRoom *shared_core_create_chat_room(LinphoneCoreManager *sender, LinphoneCoreManager *receiver,
											   const char *subject) {
	LinphoneAddress *receiverAddr = linphone_address_new(linphone_core_get_identity(receiver->lc));
	bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, (void *)receiverAddr);

	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(sender->lc);

	// Should create a one-to-one flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	LinphoneChatRoom *senderCr = linphone_core_create_chat_room_2(sender->lc, params, subject, participantsAddresses);
	linphone_chat_room_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(senderCr);

	BC_ASSERT_TRUE(wait_for_until(sender->lc, NULL, &sender->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	LinphoneAddress *senderAddr =
		linphone_address_new(linphone_proxy_config_get_identity(linphone_core_get_default_proxy_config(sender->lc)));
	BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_room_get_local_address(senderCr), senderAddr));
	linphone_address_unref(senderAddr);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	BC_ASSERT_PTR_NOT_NULL(senderCr);
	return senderCr;
}

// receiver needs to be a shared core
void shared_core_get_new_chat_room_from_addr(LinphoneCoreManager *sender, LinphoneCoreManager *receiver,
											 LinphoneChatRoom *senderCr, const char *subject) {
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(senderCr);
	BC_ASSERT_PTR_NOT_NULL(confAddr);
	if (confAddr) {
		LinphoneChatRoom *receiverCr =
			linphone_core_get_new_chat_room_from_conf_addr(receiver->lc, linphone_address_get_username(confAddr));

		BC_ASSERT_PTR_NOT_NULL(receiverCr);
		if (receiverCr != NULL) {
			ms_message("chat room [%p] chat room params [%p]", receiverCr,
					linphone_chat_room_get_current_params(receiverCr));
			linphone_chat_room_unref(receiverCr); // linphone_core_get_push_notification_chat_room_invite takes a ref on the
												// chat room, required when called from outside sdk
			LinphoneAddress *receiverAddr = linphone_address_new(linphone_core_get_identity(receiver->lc));
			BC_ASSERT_TRUE(strcmp(linphone_chat_room_get_subject(receiverCr), subject) == 0);
			BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_room_get_local_address(receiverCr), receiverAddr));
			linphone_address_unref(receiverAddr);

			stats mgrStats = receiver->stat;
			linphone_core_delete_chat_room(receiver->lc, receiverCr);
			BC_ASSERT_TRUE(wait_for_until(receiver->lc, sender->lc, &receiver->stat.number_of_LinphoneConferenceStateDeleted,
										mgrStats.number_of_LinphoneConferenceStateDeleted + 1, 10000));
			linphone_chat_room_unref(receiverCr);
		}
	}
}

void *thread_shared_core_get_new_chat_room_from_addr(void *arguments) {
#if TARGET_OS_IPHONE
	struct get_msg_args *args = (struct get_msg_args *)arguments;
	LinphoneCoreManager *receiver_mgr =
		linphone_core_manager_create_shared("", TEST_GROUP_ID, FALSE, args->receiver_mgr);
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, receiver_mgr);
	init_core_for_conference(coresManagerList);

	shared_core_get_new_chat_room_from_addr(args->sender_mgr, receiver_mgr, args->sender_cr, args->subject);
	linphone_core_manager_destroy(receiver_mgr);

	pthread_exit(NULL);
#endif
	return NULL;
}

static void shared_executor_core_get_chat_room(void) {
#if TARGET_OS_IPHONE
	const char *subject = "new chat room";
	LinphoneCoreManager *sender = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *receiver = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, FALSE, NULL);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, sender);
	coresManagerList = bctbx_list_append(coresManagerList, receiver);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	LinphoneChatRoom *senderCr = shared_core_create_chat_room(sender, receiver, subject);
	if (senderCr) {
		shared_core_get_new_chat_room_from_addr(sender, receiver, senderCr, subject);
		linphone_core_manager_delete_chat_room(sender, senderCr, coresList);
	}

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(sender);
	linphone_core_manager_destroy(receiver);
#endif
}

static void two_shared_executor_cores_get_message_and_chat_room(void) {
#if TARGET_OS_IPHONE
	const char *subject = "new chat room";
	const char *text = "message";

	LinphoneCoreManager *sender = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *receiver = linphone_core_manager_create_shared("pauline_rc", TEST_GROUP_ID, FALSE, NULL);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, sender);
	coresManagerList = bctbx_list_append(coresManagerList, receiver);
	init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	LinphoneChatRoom *senderCr = shared_core_create_chat_room(sender, receiver, subject);
	if (senderCr == NULL)
		return;

	const char *call_id = shared_core_send_msg_and_get_call_id(sender, receiver, text);
	if (strcmp(call_id, "") == 0)
		return;

	linphone_core_set_network_reachable(receiver->lc, FALSE);
	linphone_core_stop(receiver->lc);

	pthread_t executor1;
	struct get_msg_args *args1 = ms_malloc(sizeof(struct get_msg_args));
	args1->sender_mgr = sender;
	args1->receiver_mgr = receiver;
	args1->subject = subject;
	args1->sender_cr = senderCr;
	if (pthread_create(&executor1, NULL, &thread_shared_core_get_new_chat_room_from_addr, (void *)args1)) {
		ms_fatal("Error creating executor2 thread");
	}

	ms_sleep(5);

	pthread_t executor2;
	struct get_msg_args *args2 = ms_malloc(sizeof(struct get_msg_args));
	args2->sender_mgr = sender;
	args2->receiver_mgr = receiver;
	args2->text = text;
	args2->call_id = call_id;
	if (pthread_create(&executor2, NULL, &thread_shared_core_get_message_from_call_id, (void *)args2)) {
		ms_fatal("Error creating executor1 thread");
	}

	if (pthread_join(executor1, NULL)) {
		ms_fatal("Error joining thread executor1");
	}
	if (pthread_join(executor2, NULL)) {
		ms_fatal("Error joining thread executor2");
	}

	ms_free(args1);
	ms_free(args2);

	if (call_id) {
		ms_free((void *)call_id);
	}

	linphone_core_delete_chat_room(sender->lc, senderCr);
	BC_ASSERT_TRUE(wait_for_until(sender->lc, NULL, &sender->stat.number_of_LinphoneConferenceStateDeleted, 1, 10000));

	linphone_core_manager_destroy(receiver);
	linphone_core_manager_destroy(sender);
#endif
}

test_t shared_core_tests[] = {
	TEST_NO_TAG("Executor Shared Core can't start because Main Shared Core runs", shared_main_core_prevent_executor_core_start),
	TEST_NO_TAG("Executor Shared Core stopped by Main Shared Core", shared_main_core_stops_executor_core),
	TEST_NO_TAG("Executor Shared Core get message from callId by starting a core", shared_executor_core_get_message_by_starting_a_core),
	TEST_NO_TAG("Executor Shared Core get message from callId with user defaults on one thread", shared_executor_core_get_message_with_user_defaults_mono_thread),
	TEST_NO_TAG("Executor Shared Core get message from callId with user defaults on two threads", shared_executor_core_get_message_with_user_defaults_multi_thread),
	TEST_NO_TAG("Two Executor Shared Cores get messages", two_shared_executor_cores_get_messages),
	TEST_NO_TAG("Executor Shared Core get new chat room from invite", shared_executor_core_get_chat_room),
	TEST_NO_TAG("Two Executor Shared Cores get one msg and one chat room", two_shared_executor_cores_get_message_and_chat_room)
};

void shared_core_tester_before_each(void) {
	liblinphone_tester_before_each();
	LinphoneCoreManager *mgr = linphone_core_manager_create_shared("marie_rc", TEST_GROUP_ID, TRUE, NULL);
	linphone_core_reset_shared_core_state(mgr->lc);
	linphone_core_manager_destroy(mgr);
}

test_suite_t shared_core_test_suite = {"Shared Core",
									   NULL,
									   NULL,
									   shared_core_tester_before_each,
									   liblinphone_tester_after_each,
									   sizeof(shared_core_tests) / sizeof(shared_core_tests[0]),
									   shared_core_tests};
