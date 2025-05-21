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

#include "bctoolbox/crypto.h"
#include "bctoolbox/defs.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-event-log.h"
#include "linphone/api/c-friend.h"
#include "linphone/api/c-magic-search.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/api/c-participant-imdn-state.h"
#include "linphone/api/c-participant.h"
#include "linphone/api/c-types.h"
#include "linphone/chat.h"
#include "linphone/core.h"
#include "linphone/wrapper_utils.h"
#include "tester_utils.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

const char *sFactoryUri = "sip:conference-factory@conf.example.org";

static bool_t wait_for_chat_room_participants(bctbx_list_t *lcs, LinphoneChatRoom *chat, int value, int timeout_ms) {
	bctbx_list_t *iterator;
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while ((linphone_chat_room_get_nb_participants(chat) < value) &&
	       !liblinphone_tester_clock_elapsed(&start, timeout_ms)) {
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
	if (linphone_chat_room_get_nb_participants(chat) < value) return FALSE;
	else return TRUE;
}

void chat_room_session_state_changed(LinphoneChatRoom *cr, LinphoneCallState cstate, BCTBX_UNUSED(const char *msg)) {
	LinphoneCore *lc = linphone_chat_room_get_core(cr);
	stats *counters = get_stats(lc);

	switch (cstate) {
		case LinphoneCallIncomingReceived:
			counters->number_of_LinphoneChatRoomSessionIncomingReceived++;
			break;
		case LinphoneCallPushIncomingReceived:
			counters->number_of_LinphoneChatRoomSessionPushIncomingReceived++;
			break;
		case LinphoneCallOutgoingInit:
			counters->number_of_LinphoneChatRoomSessionOutgoingInit++;
			break;
		case LinphoneCallOutgoingProgress:
			counters->number_of_LinphoneChatRoomSessionOutgoingProgress++;
			break;
		case LinphoneCallOutgoingRinging:
			counters->number_of_LinphoneChatRoomSessionOutgoingRinging++;
			break;
		case LinphoneCallOutgoingEarlyMedia:
			counters->number_of_LinphoneChatRoomSessionOutgoingEarlyMedia++;
			break;
		case LinphoneCallConnected:
			counters->number_of_LinphoneChatRoomSessionConnected++;
			break;
		case LinphoneCallStreamsRunning:
			counters->number_of_LinphoneChatRoomSessionStreamsRunning++;
			break;
		case LinphoneCallPausing:
			counters->number_of_LinphoneChatRoomSessionPausing++;
			break;
		case LinphoneCallPaused:
			counters->number_of_LinphoneChatRoomSessionPaused++;
			break;
		case LinphoneCallResuming:
			counters->number_of_LinphoneChatRoomSessionResuming++;
			break;
		case LinphoneCallRefered:
			counters->number_of_LinphoneChatRoomSessionRefered++;
			break;
		case LinphoneCallError:
			counters->number_of_LinphoneChatRoomSessionError++;
			break;
		case LinphoneCallEnd:
			counters->number_of_LinphoneChatRoomSessionEnd++;
			break;
		case LinphoneCallPausedByRemote:
			counters->number_of_LinphoneChatRoomSessionPausedByRemote++;
			break;
		case LinphoneCallUpdatedByRemote:
			counters->number_of_LinphoneChatRoomSessionUpdatedByRemote++;
			break;
		case LinphoneCallIncomingEarlyMedia:
			counters->number_of_LinphoneChatRoomSessionIncomingEarlyMedia++;
			break;
		case LinphoneCallUpdating:
			counters->number_of_LinphoneChatRoomSessionUpdating++;
			break;
		case LinphoneCallReleased:
			counters->number_of_LinphoneChatRoomSessionReleased++;
			break;
		case LinphoneCallEarlyUpdating:
			counters->number_of_LinphoneChatRoomSessionEarlyUpdating++;
			break;
		case LinphoneCallEarlyUpdatedByRemote:
			counters->number_of_LinphoneChatRoomSessionEarlyUpdatedByRemote++;
			break;
		default:
			BC_FAIL("unexpected event");
			break;
	}
}

static void chat_room_is_composing_received(LinphoneChatRoom *cr,
                                            BCTBX_UNUSED(const LinphoneAddress *remoteAddr),
                                            bool_t isComposing) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	if (isComposing) manager->stat.number_of_LinphoneIsComposingActiveReceived++;
	else manager->stat.number_of_LinphoneIsComposingIdleReceived++;
}

static void undecryptable_message_received(LinphoneChatRoom *room, BCTBX_UNUSED(LinphoneChatMessage *msg)) {
	get_stats(linphone_chat_room_get_core(room))->number_of_LinphoneMessageUndecryptable++;
}

static void chat_room_participant_added(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_chat_room_participants_added++;
}

static void chat_room_participant_admin_status_changed(LinphoneChatRoom *cr,
                                                       BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_chat_room_participant_admin_statuses_changed++;
}

static void chat_room_participant_removed(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_chat_room_participants_removed++;
}

static void chat_room_participant_device_added(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_chat_room_participant_devices_added++;
}

static void chat_room_participant_device_removed(LinphoneChatRoom *cr,
                                                 BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_chat_room_participant_devices_removed++;
}

static void chat_room_state_changed(LinphoneChatRoom *cr, LinphoneChatRoomState newState) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	const LinphoneAddress *addr = linphone_chat_room_get_conference_address(cr);
	if (addr) {
		char *addr_str = linphone_address_as_string(addr);
		ms_message("ChatRoom [%s] state changed: %s", addr_str, linphone_chat_room_state_to_string(newState));
		bctbx_free(addr_str);
	}
	switch (newState) {
		case LinphoneChatRoomStateNone:
			break;
		case LinphoneChatRoomStateInstantiated:
			manager->stat.number_of_LinphoneChatRoomStateInstantiated++;
			break;
		case LinphoneChatRoomStateCreationPending:
			manager->stat.number_of_LinphoneChatRoomStateCreationPending++;
			break;
		case LinphoneChatRoomStateCreated:
			manager->stat.number_of_LinphoneChatRoomStateCreated++;
			break;
		case LinphoneChatRoomStateCreationFailed:
			manager->stat.number_of_LinphoneChatRoomStateCreationFailed++;
			break;
		case LinphoneChatRoomStateTerminationPending:
			manager->stat.number_of_LinphoneChatRoomStateTerminationPending++;
			break;
		case LinphoneChatRoomStateTerminated:
			manager->stat.number_of_LinphoneChatRoomStateTerminated++;
			break;
		case LinphoneChatRoomStateTerminationFailed:
			manager->stat.number_of_LinphoneChatRoomStateTerminationFailed++;
			break;
		case LinphoneChatRoomStateDeleted:
			manager->stat.number_of_LinphoneChatRoomStateDeleted++;
			break;
		default:
			ms_error("Invalid ChatRoom state for Chatroom [%s] EndOfEnum is used ONLY as a guard",
			         linphone_address_as_string(addr));
			break;
	}
}

static void chat_room_security_event(LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);

	switch (linphone_event_log_get_security_event_type(event_log)) {
		case LinphoneSecurityEventTypeNone:
			break;
		case LinphoneSecurityEventTypeSecurityLevelDowngraded:
			manager->stat.number_of_SecurityLevelDowngraded++;
			break;
		case LinphoneSecurityEventTypeParticipantMaxDeviceCountExceeded:
			manager->stat.number_of_ParticipantMaxDeviceCountExceeded++;
			break;
		case LinphoneSecurityEventTypeEncryptionIdentityKeyChanged:
			manager->stat.number_of_EncryptionIdentityKeyChanged++;
			break;
		case LinphoneSecurityEventTypeManInTheMiddleDetected:
			manager->stat.number_of_ManInTheMiddleDetected++;
			break;
	}
}

static void chat_room_subject_changed(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_chat_room_subject_changed++;
}

static void chat_room_conference_joined(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneChatRoomConferenceJoined++;
}

static void chat_room_message_ephemeral(LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	switch (linphone_event_log_get_type(event_log)) {
		case LinphoneEventLogTypeConferenceEphemeralMessageLifetimeChanged:
			manager->stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged++;
			break;
		case LinphoneEventLogTypeConferenceEphemeralMessageEnabled:
			manager->stat.number_of_LinphoneChatRoomEphemeralMessageEnabled++;
			break;
		case LinphoneEventLogTypeConferenceEphemeralMessageDisabled:
			manager->stat.number_of_LinphoneChatRoomEphemeralMessageDisabled++;
			break;
		default:
			break;
	}
}

static void chat_room_message_ephemeral_started(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneChatRoomEphemeralTimerStarted++;
}

static void chat_room_message_ephemeral_deleted(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *event_log)) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneChatRoomEphemeralDeleted++;
}

void setup_chat_room_callbacks(LinphoneChatRoomCbs *cbs) {
	linphone_chat_room_cbs_set_session_state_changed(cbs, chat_room_session_state_changed);
	linphone_chat_room_cbs_set_is_composing_received(cbs, chat_room_is_composing_received);
	linphone_chat_room_cbs_set_participant_added(cbs, chat_room_participant_added);
	linphone_chat_room_cbs_set_participant_admin_status_changed(cbs, chat_room_participant_admin_status_changed);
	linphone_chat_room_cbs_set_participant_removed(cbs, chat_room_participant_removed);
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_cbs_set_security_event(cbs, chat_room_security_event);
	linphone_chat_room_cbs_set_subject_changed(cbs, chat_room_subject_changed);
	linphone_chat_room_cbs_set_participant_device_added(cbs, chat_room_participant_device_added);
	linphone_chat_room_cbs_set_participant_device_removed(cbs, chat_room_participant_device_removed);
	linphone_chat_room_cbs_set_undecryptable_message_received(cbs, undecryptable_message_received);
	linphone_chat_room_cbs_set_conference_joined(cbs, chat_room_conference_joined);
	linphone_chat_room_cbs_set_ephemeral_event(cbs, chat_room_message_ephemeral);
	linphone_chat_room_cbs_set_ephemeral_message_timer_started(cbs, chat_room_message_ephemeral_started);
	linphone_chat_room_cbs_set_ephemeral_message_deleted(cbs, chat_room_message_ephemeral_deleted);
}

void core_chat_room_state_changed(BCTBX_UNUSED(LinphoneCore *core), LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	const LinphoneConferenceParams *chat_params = linphone_chat_room_get_current_params(cr);
	bool hasAudio = !!linphone_conference_params_audio_enabled(chat_params) ||
	                !!linphone_conference_params_video_enabled(chat_params);
	// When a chatroom is instantied as part of a conference, the first state it will be notifed is the Created state as
	// the core waits for the full state to arrive before creating it. In fact, it may happen that a core wishes to
	// create a conference with chat capabilities but the server doesn't supports it. The client would end up having a
	// dangling tchat and needs to destroy it
	if ((!hasAudio && (state == LinphoneChatRoomStateInstantiated)) ||
	    (hasAudio && (state == LinphoneChatRoomStateCreated))) {
		LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
		setup_chat_room_callbacks(cbs);
		linphone_chat_room_add_callbacks(cr, cbs);
		linphone_chat_room_cbs_unref(cbs);
	}
}

void core_chat_room_subject_changed(LinphoneCore *core, BCTBX_UNUSED(LinphoneChatRoom *cr)) {
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_core_chat_room_subject_changed++;
}

void configure_core_for_conference(LinphoneCore *core,
                                   const char *username,
                                   const LinphoneAddress *factoryAddr,
                                   bool_t server) {
	const char *identity = linphone_core_get_identity(core);
	LinphoneAddress *addr = linphone_address_new(identity);
	linphone_address_set_username(
	    addr, (username) ? username : linphone_address_get_username(addr)); // What's the point if !username?
	char *newIdentity = linphone_address_as_string_uri_only(addr);
	linphone_address_unref(addr);
	linphone_core_set_primary_contact(core, newIdentity);
	bctbx_free(newIdentity);
	linphone_core_enable_conference_server(core, server);
	linphone_core_enable_rtp_bundle(core, TRUE);
	LinphoneAccount *account = linphone_core_get_default_account(core);
	const LinphoneAccountParams *account_params = linphone_account_get_params(account);
	LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
	linphone_account_params_enable_rtp_bundle(new_account_params, TRUE);
	linphone_account_set_params(account, new_account_params);
	linphone_account_params_unref(new_account_params);
	BC_ASSERT_TRUE(linphone_account_params_rtp_bundle_enabled(linphone_account_get_params(account)));
	if (factoryAddr) {
		char *factoryUri = linphone_address_as_string(factoryAddr);
		LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(core);
		linphone_proxy_config_edit(proxy);
		linphone_proxy_config_set_conference_factory_uri(proxy, factoryUri);
		linphone_proxy_config_done(proxy);
		bctbx_free(factoryUri);
	}
}

void _configure_core_for_conference(LinphoneCoreManager *lcm, const LinphoneAddress *factoryAddr) {
	configure_core_for_conference(lcm->lc, NULL, factoryAddr, FALSE);
}

void _configure_core_for_audio_video_conference(LinphoneCoreManager *lcm, const LinphoneAddress *factoryAddr) {
	if (factoryAddr) {
		LinphoneAccount *account = linphone_core_get_default_account(lcm->lc);
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		linphone_account_params_set_audio_video_conference_factory_address(params, factoryAddr);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
	}
}

void configure_core_for_callbacks(LinphoneCoreManager *lcm, LinphoneCoreCbs *cbs) {
	// Remove is-composing callback from the core, we use our own on the chat room
	linphone_core_cbs_set_is_composing_received(lcm->cbs, NULL);
	linphone_core_add_callbacks(lcm->lc, cbs);
	linphone_core_set_user_data(lcm->lc, lcm);
}

void _start_core(LinphoneCoreManager *lcm) {
	linphone_core_manager_start(lcm, TRUE);
}

LinphoneChatMessage *_send_message_ephemeral(LinphoneChatRoom *chatRoom, const char *message, bool_t isEphemeral) {
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(chatRoom, message);
	LinphoneChatMessageCbs *msgCbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(msgCbs, liblinphone_tester_chat_message_msg_state_changed);
	if (isEphemeral) {
		linphone_chat_message_cbs_set_ephemeral_message_timer_started(
		    msgCbs, liblinphone_tester_chat_message_ephemeral_timer_started);
		linphone_chat_message_cbs_set_ephemeral_message_deleted(msgCbs,
		                                                        liblinphone_tester_chat_message_ephemeral_deleted);
	}
	ms_message("Chat room %p sends %smessage %s", chatRoom, (isEphemeral ? "ephemeral " : ""), message);
	linphone_chat_message_send(msg);
	return msg;
}

LinphoneChatMessage *_send_message(LinphoneChatRoom *chatRoom, const char *message) {
	return _send_message_ephemeral(chatRoom, message, FALSE);
}

static void fill_content_buffer(LinphoneContent *content, const char *sendFilePath) {
	FILE *file_to_send = NULL;
	size_t file_size;

	file_to_send = fopen(sendFilePath, "rb");
	BC_ASSERT_PTR_NOT_NULL(file_to_send);

	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, 0, SEEK_SET);

	uint8_t *buf = ms_malloc(file_size);
	size_t read = fread(buf, sizeof(uint8_t), file_size, file_to_send);

	BC_ASSERT_EQUAL((int)read, (int)file_size, int, "%d");
	linphone_content_set_buffer(content, buf, file_size);
	ms_free(buf);
	linphone_content_set_size(content, file_size); /*total size to be transferred*/
	fclose(file_to_send);
}

void _send_file_plus_text(
    LinphoneChatRoom *cr, const char *sendFilepath, const char *sendFilepath2, const char *text, bool_t use_buffer) {
	LinphoneChatMessage *msg;

	LinphoneChatMessageCbs *cbs;
	LinphoneContent *content = linphone_core_create_content(linphone_chat_room_get_core(cr));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content, "video");
	linphone_content_set_subtype(content, "mkv");
	linphone_content_set_name(content, "sintel_trailer_opus_h264.mkv");
	if (use_buffer) {
		fill_content_buffer(content, sendFilepath);
	} else {
		linphone_content_set_file_path(content, sendFilepath);
	}

	msg = linphone_chat_room_create_empty_message(cr);
	linphone_chat_message_add_file_content(msg, content);
	BC_ASSERT_PTR_NULL(linphone_content_get_related_chat_message_id(content));
	linphone_content_unref(content);

	if (text) linphone_chat_message_add_utf8_text_content(msg, text);

	if (sendFilepath2) {
		LinphoneContent *content2 = linphone_core_create_content(linphone_chat_room_get_core(cr));
		belle_sip_object_set_name(BELLE_SIP_OBJECT(content2), "ahbahouaismaisbon content");
		linphone_content_set_type(content2, "audio");
		linphone_content_set_subtype(content2, "wav");
		linphone_content_set_name(content2, "ahbahouaismaisbon.wav");
		if (use_buffer) {
			fill_content_buffer(content2, sendFilepath2);
		} else {
			linphone_content_set_file_path(content2, sendFilepath2);
		}
		linphone_chat_message_add_file_content(msg, content2);
		BC_ASSERT_PTR_NULL(linphone_content_get_related_chat_message_id(content2));
		linphone_content_unref(content2);
	}

	cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
	linphone_chat_message_cbs_set_file_transfer_send_chunk(cbs, tester_file_transfer_send_2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);

	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication_2);
	linphone_chat_message_add_callbacks(msg, cbs);
	linphone_chat_message_cbs_unref(cbs);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
}

void _send_file(LinphoneChatRoom *cr, const char *sendFilepath, const char *sendFilepath2, bool_t use_buffer) {
	_send_file_plus_text(cr, sendFilepath, sendFilepath2, NULL, use_buffer);
}

void _receive_file_plus_text(bctbx_list_t *coresList,
                             LinphoneCoreManager *lcm,
                             stats *receiverStats,
                             const char *receive_filepath,
                             const char *sendFilepath,
                             const char *sendFilepath2,
                             const char *text,
                             bool_t use_buffer) {
	if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageReceivedWithFile,
	                                 receiverStats->number_of_LinphoneMessageReceivedWithFile + 1,
	                                 liblinphone_tester_sip_timeout))) {
		LinphoneChatMessageCbs *cbs;
		LinphoneChatMessage *msg = lcm->stat.last_received_chat_message;
		const char *downloaded_file = NULL;
		if (!use_buffer) {
			downloaded_file = receive_filepath;
		}

		if (text) {
			BC_ASSERT_TRUE(linphone_chat_message_has_text_content(msg));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(msg), text);
		}

		cbs = linphone_chat_message_get_callbacks(msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication_2);

		BC_ASSERT_PTR_NOT_NULL(linphone_chat_message_get_external_body_url(msg));
		LinphoneContent *fileTransferContent = linphone_chat_message_get_file_transfer_information(msg);

		// Take a ref. Indeed during the next wait_for_list(), the file may be downloaded entirely,
		// which drops the Content and replaces it with a FileContent internally.
		// Then the subsequent linphone_chat_message_download_content() on this fileTransferContent reference
		// cause a crash.
		// FIXME: the API shall not be so error-prone.
		linphone_content_ref(fileTransferContent);
		BC_ASSERT_TRUE(linphone_content_is_file_transfer(fileTransferContent));
		if (!use_buffer) {
			linphone_content_set_file_path(fileTransferContent, downloaded_file);
		}

		BC_ASSERT_STRING_EQUAL(linphone_content_get_related_chat_message_id(fileTransferContent),
		                       linphone_chat_message_get_message_id(msg));

		linphone_chat_message_download_content(msg, fileTransferContent);
		BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int,
		                "%d");
		// Cancel the download shortly
		BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.progress_of_LinphoneFileTransfer, 5, 5000));

		char *downloaded_file_temp = bctbx_concat(downloaded_file, ".copy", NULL);
		remove(downloaded_file_temp);
		liblinphone_tester_copy_file(downloaded_file, downloaded_file_temp);
		linphone_chat_message_cancel_file_transfer(msg);
		liblinphone_tester_copy_file(downloaded_file_temp, downloaded_file);
		remove(downloaded_file_temp);
		bctbx_free(downloaded_file_temp);
		linphone_chat_message_download_content(msg, fileTransferContent);
		linphone_content_unref(fileTransferContent);

		if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageFileTransferDone,
		                                 receiverStats->number_of_LinphoneMessageFileTransferDone + 1,
		                                 liblinphone_tester_sip_timeout))) {
			if (use_buffer) {
				// file_transfer_received function store file name into file_transfer_filepath
				downloaded_file = bctbx_strdup(linphone_chat_message_get_file_transfer_filepath(msg));
			}

			compare_files(sendFilepath, downloaded_file);
		}

		if (sendFilepath2) {
			remove(downloaded_file);
			LinphoneContent *fileTransferContent2 = linphone_chat_message_get_file_transfer_information(msg);
			BC_ASSERT_TRUE(linphone_content_is_file_transfer(fileTransferContent2));
			if (!use_buffer) {
				linphone_content_set_file_path(fileTransferContent2, downloaded_file);
			}
			BC_ASSERT_STRING_EQUAL(linphone_content_get_related_chat_message_id(fileTransferContent2),
			                       linphone_chat_message_get_message_id(msg));
			linphone_chat_message_download_content(msg, fileTransferContent2);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int,
			                "%d");

			if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageFileTransferDone,
			                                 receiverStats->number_of_LinphoneMessageFileTransferDone + 2,
			                                 liblinphone_tester_sip_timeout))) {
				if (use_buffer) {
					// file_transfer_received function store file name into file_transfer_filepath
					downloaded_file = bctbx_strdup(linphone_chat_message_get_file_transfer_filepath(msg));
				}
				compare_files(sendFilepath2, downloaded_file);
			}
		}
		if (use_buffer && downloaded_file) bctbx_free((char *)downloaded_file);
	}
}

void _receive_file(bctbx_list_t *coresList,
                   LinphoneCoreManager *lcm,
                   stats *receiverStats,
                   const char *receive_filepath,
                   const char *sendFilepath,
                   const char *sendFilepath2,
                   bool_t use_buffer) {
	_receive_file_plus_text(coresList, lcm, receiverStats, receive_filepath, sendFilepath, sendFilepath2, NULL,
	                        use_buffer);
}

// Configure list of core manager for conference and add the listener
static bctbx_list_t *_init_core_for_conference_with_factory_uri(bctbx_list_t *coreManagerList,
                                                                const char *factoryUri,
                                                                const char *groupchat_version) {
	LinphoneAddress *factoryAddr = linphone_address_new(factoryUri);
	bctbx_list_for_each2(coreManagerList, (void (*)(void *, void *))_configure_core_for_conference,
	                     (void *)factoryAddr);
	linphone_address_unref(factoryAddr);

	if (groupchat_version) {
		bctbx_list_t *it;
		char *spec = bctbx_strdup_printf("groupchat/%s", groupchat_version);
		for (it = coreManagerList; it != NULL; it = it->next) {
			LinphoneCoreManager *mgr = (LinphoneCoreManager *)it->data;
			linphone_core_remove_linphone_spec(mgr->lc, "groupchat");
			linphone_core_add_linphone_spec(mgr->lc, spec);
		}
		bctbx_free(spec);
	}

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
	linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
	bctbx_list_for_each2(coreManagerList, (void (*)(void *, void *))configure_core_for_callbacks, (void *)cbs);
	linphone_core_cbs_unref(cbs);

	bctbx_list_t *coresList = NULL;
	bctbx_list_t *item = coreManagerList;
	for (item = coreManagerList; item; item = bctbx_list_next(item))
		coresList = bctbx_list_append(coresList, ((LinphoneCoreManager *)(bctbx_list_get_data(item)))->lc);
	return coresList;
}

bctbx_list_t *init_core_for_conference_with_factory_uri(bctbx_list_t *coreManagerList, const char *factoryUri) {
	return _init_core_for_conference_with_factory_uri(coreManagerList, factoryUri, NULL);
}

bctbx_list_t *init_core_for_conference(bctbx_list_t *coreManagerList) {
	return init_core_for_conference_with_factory_uri(coreManagerList, sFactoryUri);
}

bctbx_list_t *init_core_for_conference_with_groupchat_version(bctbx_list_t *coreManagerList,
                                                              const char *groupchat_version) {
	return _init_core_for_conference_with_factory_uri(coreManagerList, sFactoryUri, groupchat_version);
}

void start_core_for_conference(bctbx_list_t *coreManagerList) {
	bctbx_list_for_each(coreManagerList, (void (*)(void *))_start_core);
}

static LinphoneChatRoom *check_has_chat_room_client_side(bctbx_list_t *lcs,
                                                         LinphoneCoreManager *lcm,
                                                         BCTBX_UNUSED(stats *initialStats),
                                                         const LinphoneAddress *confAddr,
                                                         const char *subject,
                                                         int participantNumber,
                                                         bool_t isAdmin) {
	char *deviceIdentity = linphone_core_get_device_identity(lcm->lc);
	LinphoneAddress *localAddr = linphone_address_new(deviceIdentity);
	bctbx_free(deviceIdentity);
	LinphoneChatRoom *chatRoom = linphone_core_find_chat_room(lcm->lc, confAddr, localAddr);
	linphone_address_unref(localAddr);
	BC_ASSERT_PTR_NOT_NULL(chatRoom);
	if (chatRoom) {
		BC_ASSERT_TRUE(
		    wait_for_chat_room_participants(lcs, chatRoom, participantNumber, liblinphone_tester_sip_timeout));
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom), participantNumber, int, "%d");
		LinphoneParticipant *participant = linphone_chat_room_get_me(chatRoom);
		BC_ASSERT_PTR_NOT_NULL(participant);
		if (!(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesOneToOne))
			BC_ASSERT_TRUE(isAdmin == linphone_participant_is_admin(participant));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), subject);

		if (linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesBasic) {
			BC_ASSERT_FALSE(linphone_chat_room_can_handle_participants(chatRoom));
		} else {
			BC_ASSERT_TRUE(linphone_chat_room_can_handle_participants(chatRoom));
		}
	}
	return chatRoom;
}

LinphoneChatRoom *check_creation_chat_room_client_side(bctbx_list_t *lcs,
                                                       LinphoneCoreManager *lcm,
                                                       stats *initialStats,
                                                       const LinphoneAddress *confAddr,
                                                       const char *subject,
                                                       int participantNumber,
                                                       bool_t isAdmin) {
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending,
	                             initialStats->number_of_LinphoneChatRoomStateCreationPending + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialStats->number_of_LinphoneChatRoomStateCreated + 1,
	                             liblinphone_tester_sip_timeout));
	if (linphone_core_is_network_reachable(lcm->lc)) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined,
		                             initialStats->number_of_LinphoneChatRoomConferenceJoined + 1,
		                             liblinphone_tester_sip_timeout));
	}
	return check_has_chat_room_client_side(lcs, lcm, initialStats, confAddr, subject, participantNumber, isAdmin);
}

void check_create_chat_room_client_side(bctbx_list_t *lcs,
                                        LinphoneCoreManager *lcm,
                                        LinphoneChatRoom *chatRoom,
                                        stats *initialStats,
                                        bctbx_list_t *participantsAddresses,
                                        const char *initialSubject,
                                        int expectedParticipantSize) {

	BC_ASSERT_PTR_NOT_NULL(chatRoom);
	if (!chatRoom) return;

	const LinphoneChatRoomParams *params = linphone_chat_room_get_current_params(chatRoom);
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateInstantiated,
	                             initialStats->number_of_LinphoneChatRoomStateInstantiated + 1, 100));
	BC_ASSERT_PTR_NOT_NULL(params);
	if (linphone_chat_room_params_encryption_enabled(params)) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chatRoom), LinphoneChatRoomSecurityLevelEncrypted,
		                LinphoneChatRoomSecurityLevel, "%i");
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesEncrypted);
	}

	LinphoneParticipant *participant = linphone_chat_room_get_me(chatRoom);
	BC_ASSERT_PTR_NOT_NULL(participant);

	if (expectedParticipantSize == 0) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationFailed,
		                             initialStats->number_of_LinphoneChatRoomStateCreationFailed + 1,
		                             liblinphone_tester_sip_timeout));
		if (participant) BC_ASSERT_FALSE(linphone_participant_is_admin(participant));
	} else {
		if (linphone_core_is_network_reachable(lcm->lc)) {
			// Check that the chat room is correctly created on Marie's side and that the participants are added
			BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending,
			                             initialStats->number_of_LinphoneChatRoomStateCreationPending + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated,
			                             initialStats->number_of_LinphoneChatRoomStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined,
			                             initialStats->number_of_LinphoneChatRoomConferenceJoined + 1,
			                             2 * liblinphone_tester_sip_timeout));

			// FIXME: Small hack to handle situation where the core resubscribes to the chat room
			wait_for_list(lcs, NULL, 0, 1000);

			BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom),
			                (expectedParticipantSize >= 0) ? expectedParticipantSize
			                                               : ((int)bctbx_list_size(participantsAddresses)),
			                int, "%d");

			if (participant) BC_ASSERT_TRUE(linphone_participant_is_admin(participant));
		}
	}
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), initialSubject);

	if (linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesBasic) {
		BC_ASSERT_FALSE(linphone_chat_room_can_handle_participants(chatRoom));
	} else {
		BC_ASSERT_TRUE(linphone_chat_room_can_handle_participants(chatRoom));
	}

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
}

LinphoneChatRoom *create_chat_room_client_side_with_params(bctbx_list_t *lcs,
                                                           LinphoneCoreManager *lcm,
                                                           stats *initialStats,
                                                           bctbx_list_t *participantsAddresses,
                                                           const char *initialSubject,
                                                           LinphoneChatRoomParams *params) {
	LinphoneChatRoom *chatRoom =
	    linphone_core_create_chat_room_2(lcm->lc, params, initialSubject, participantsAddresses);

	check_create_chat_room_client_side(lcs, lcm, chatRoom, initialStats, participantsAddresses, initialSubject,
	                                   (int)bctbx_list_size(participantsAddresses));

	linphone_chat_room_unref(chatRoom);

	return chatRoom;
}

LinphoneChatRoom *
create_chat_room_client_side_with_expected_number_of_participants(bctbx_list_t *lcs,
                                                                  LinphoneCoreManager *lcm,
                                                                  stats *initialStats,
                                                                  bctbx_list_t *participantsAddresses,
                                                                  const char *initialSubject,
                                                                  int expectedParticipantSize,
                                                                  bool_t encrypted,
                                                                  LinphoneChatRoomEphemeralMode mode) {
	LinphoneConferenceParams *params = linphone_conference_params_new(lcm->lc);
	linphone_conference_params_enable_chat(params, TRUE);
	linphone_conference_params_set_security_level(params, encrypted ? LinphoneConferenceSecurityLevelEndToEnd
	                                                                : LinphoneConferenceSecurityLevelNone);
	int participantsAddressesSize = (int)bctbx_list_size(participantsAddresses);
	linphone_conference_params_enable_group(params, participantsAddressesSize > 1 ? TRUE : FALSE);
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
	linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_params_set_ephemeral_mode(chat_params, mode);
	LinphoneChatRoom *chatRoom =
	    linphone_core_create_chat_room_2(lcm->lc, params, initialSubject, participantsAddresses);
	linphone_chat_room_params_unref(params);

	if (!chatRoom) return NULL;

	check_create_chat_room_client_side(lcs, lcm, chatRoom, initialStats, participantsAddresses, initialSubject,
	                                   expectedParticipantSize);

	linphone_chat_room_unref(chatRoom);

	return chatRoom;
}

LinphoneChatRoom *create_chat_room_with_params(bctbx_list_t *lcs,
                                               LinphoneCoreManager *lcm,
                                               stats *initialStats,
                                               bctbx_list_t *participantsAddresses,
                                               const char *initialSubject,
                                               LinphoneChatRoomParams *params) {
	int participantsAddressesSize = (int)bctbx_list_size(participantsAddresses);

	if (!params) return NULL;

	LinphoneChatRoom *chatRoom =
	    linphone_core_create_chat_room_2(lcm->lc, params, initialSubject, participantsAddresses);

	check_create_chat_room_client_side(lcs, lcm, chatRoom, initialStats, participantsAddresses, initialSubject,
	                                   participantsAddressesSize);

	linphone_chat_room_unref(chatRoom);

	return chatRoom;
}

LinphoneChatRoom *create_chat_room_client_side(bctbx_list_t *lcs,
                                               LinphoneCoreManager *lcm,
                                               stats *initialStats,
                                               bctbx_list_t *participantsAddresses,
                                               const char *initialSubject,
                                               bool_t encrypted,
                                               LinphoneChatRoomEphemeralMode mode) {
	return create_chat_room_client_side_with_expected_number_of_participants(
	    lcs, lcm, initialStats, participantsAddresses, initialSubject, (int)bctbx_list_size(participantsAddresses),
	    encrypted, mode);
}

static void group_chat_room_params(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = bctbx_list_append(NULL, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *participantsAddresses = NULL;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	LinphoneChatRoom *marieCr = NULL;
	start_core_for_conference(coresManagerList);

	const LinphoneAddress *marieAddr =
	    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc));
	const LinphoneAddress *paulineAddr =
	    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc));

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);

	// Should create	a basic chatroom
	linphone_chat_room_params_enable_encryption(params, FALSE);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);
	linphone_chat_room_params_enable_group(params, FALSE);
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Basic chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_PTR_NOT_NULL(linphone_chat_room_get_current_params(marieCr));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		linphone_chat_room_unref(marieCr);

		BC_ASSERT_PTR_NULL(linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr));
		BC_ASSERT_PTR_NULL(linphone_core_find_one_to_one_chat_room(pauline->lc, paulineAddr, marieAddr));
	}
	// Should create	a one-to-one flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, FALSE);
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "One to one client group chat room subject",
	                                           participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		linphone_chat_room_unref(marieCr);

		BC_ASSERT_PTR_NULL(linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr));
		BC_ASSERT_PTR_NULL(linphone_core_find_one_to_one_chat_room(pauline->lc, paulineAddr, marieAddr));
	}

	// Should create	a group flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Group chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		linphone_chat_room_unref(marieCr);
	}

	// Should create	an encrypted group flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	linphone_chat_room_params_enable_encryption(params, TRUE);
	// Check that enabling encryption also defines a valid encryptionBackend
	BC_ASSERT_EQUAL(linphone_chat_room_params_get_encryption_backend(params), LinphoneChatRoomEncryptionBackendLime,
	                int, "%d");
	marieCr =
	    linphone_core_create_chat_room_2(marie->lc, params, "Encrypted group chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesEncrypted);
		BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		linphone_chat_room_unref(marieCr);
	}

	// Should return NULL because params are invalid
	linphone_chat_room_params_enable_group(params, TRUE);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic); // Group + basic backend = invalid
	BC_ASSERT_FALSE(linphone_chat_room_params_is_valid(params));
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Invalid chat room subject", NULL);
	BC_ASSERT_PTR_NULL(marieCr);
	if (marieCr) linphone_chat_room_unref(marieCr);

	// Should set FlexisipChat as backend if encryption is enabled.
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);
	linphone_chat_room_params_enable_encryption(params, TRUE);
	BC_ASSERT_EQUAL(linphone_chat_room_params_get_backend(params), LinphoneChatRoomBackendFlexisipChat, int, "%d");

	// Cleanup
	linphone_chat_room_params_unref(params);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_creation_core_restart(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	int dummy = 0;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	LinphoneAddress *confAddr = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_proxy_config_get_identity(proxy));
	const char *initialSubject = "Colleagues";

	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);
	linphone_chat_room_params_enable_encryption(params, FALSE);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	LinphoneChatRoom *marieCr =
	    linphone_core_create_chat_room_2(marie->lc, params, initialSubject, participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (!marieCr) goto end;

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), initialSubject);
	// Chat room is no longer caching participants
	BC_ASSERT_EQUAL(bctbx_list_size(linphone_chat_room_get_participants(marieCr)), 0, size_t, "%zu");

	const LinphoneAddress *localAddr = linphone_chat_room_get_local_address(marieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(localAddr, marieAddr));

	confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// now with simulate foregound/backgroud switch to get a remote event handler list instead of a simple remote
	// event handler
	linphone_core_enter_background(marie->lc);
	linphone_config_set_bool(linphone_core_get_config(marie->lc), "misc", "conference_event_package_force_full_state",
	                         TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Restart Marie
	char *uuid = NULL;
	if (linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "uuid", NULL)) {
		uuid = bctbx_strdup(linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "uuid", NULL));
	}

	ms_message("Restart %s's core", linphone_core_get_identity(marie->lc));
	initialMarieStats = marie->stat;
	coresList = bctbx_list_remove(coresList, marie->lc);
	linphone_core_manager_reinit(marie);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
	bctbx_list_free(init_core_for_conference(tmpCoresManagerList));
	bctbx_list_free(tmpCoresManagerList);
	// Make sure gruu is preserved
	linphone_config_set_string(linphone_core_get_config(marie->lc), "misc", "uuid", uuid);
	linphone_core_manager_start(marie, TRUE);
	coresList = bctbx_list_append(coresList, marie->lc);

	if (uuid) {
		bctbx_free(uuid);
	}

	char *marieDeviceIdentity = linphone_core_get_device_identity(marie->lc);
	LinphoneAddress *marieLocalAddr = linphone_address_new(marieDeviceIdentity);
	bctbx_free(marieDeviceIdentity);
	marieCr = linphone_core_search_chat_room(marie->lc, NULL, marieLocalAddr, confAddr, NULL);
	linphone_address_unref(marieLocalAddr);
	BC_ASSERT_PTR_NOT_NULL(marieCr);

	// Clean db from chat room
	if (marieCr) {
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(laureCr);

end:
	if (marieAddr) linphone_address_unref(marieAddr);
	if (confAddr) linphone_address_unref(confAddr);
	linphone_chat_room_params_unref(params);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_creation_with_given_identity(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dual_proxy_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;

	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc)), 2, int, "%d");

	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *secondProxy = linphone_core_get_proxy_config_list(marie->lc)->next->data;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_proxy_config_get_identity(proxy));
	LinphoneAddress *marieSecondAddr = linphone_address_new(linphone_proxy_config_get_identity(secondProxy));

	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);
	LinphoneChatRoom *marieCr =
	    linphone_core_create_chat_room(marie->lc, params, marieAddr, "first chat room", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1,
	                             liblinphone_tester_sip_timeout));

	const LinphoneAddress *localAddr = linphone_chat_room_get_local_address(marieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(localAddr, marieAddr));

	LinphoneChatRoom *secondMarieCr =
	    linphone_core_create_chat_room(marie->lc, params, marieSecondAddr, "second chat room", participantsAddresses);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1,
	                             liblinphone_tester_sip_timeout));

	localAddr = linphone_chat_room_get_local_address(secondMarieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(localAddr, marieSecondAddr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(marie, secondMarieCr, coresList);
	if (marieCr) linphone_chat_room_unref(marieCr);
	if (secondMarieCr) linphone_chat_room_unref(secondMarieCr);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	linphone_address_unref(marieAddr);
	linphone_address_unref(marieSecondAddr);
	linphone_chat_room_params_unref(params);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_creation_server(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Pauline tries to change the subject but is not admin so it fails
	const char *newSubject = "Let's go drink a beer";
	linphone_chat_room_set_subject(paulineCr, newSubject);
	wait_for_list(coresList, &dummy, 1, 1000);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), initialSubject);

	// Marie now changes the subject
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	paulineParticipant = linphone_participant_ref(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));
	linphone_participant_unref(paulineParticipant);

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, newSubject, 3, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
	                             initialMarieStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
	                             initialPaulineStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_added,
	                             initialLaureStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	// Pauline revokes the admin status of Marie
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	marieParticipant = linphone_participant_ref(marieParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, marieParticipant, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(linphone_participant_is_admin(marieParticipant));

	// Marie tries to change the subject again but is not admin, so it is not changed
	linphone_chat_room_set_subject(marieCr, initialSubject);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Chloe begins composing a message
	BC_ASSERT_PTR_NOT_NULL(chloeCr);
	if (chloeCr) {
		linphone_chat_room_compose(chloeCr);
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived,
	                             initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived,
	                             initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived,
	                             initialLaureStats.number_of_LinphoneIsComposingActiveReceived + 1,
	                             liblinphone_tester_sip_timeout));
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDelivered,
	                             initialChloeStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
	                             initialLaureStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived,
	                             initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived,
	                             initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived,
	                             initialLaureStats.number_of_LinphoneIsComposingIdleReceived + 1,
	                             liblinphone_tester_sip_timeout));

	LinphoneAddress *chloeAddr;
	if (BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message)) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message),
		                       chloeTextMessage);
		linphone_chat_message_unref(chloeMessage);
		chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(
		    chloeAddr, linphone_chat_message_get_from_address(marie->stat.last_received_chat_message)));
		linphone_address_unref(chloeAddr);
	}

	// Pauline removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(paulineCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	laureParticipant = linphone_participant_ref(laureParticipant);
	linphone_chat_room_remove_participant(paulineCr, laureParticipant);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_chat_room_participants_removed,
	                             initialChloeStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	linphone_participant_unref(laureParticipant);

	// Pauline removes Marie and Chloe from the chat room
	marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	LinphoneParticipant *chloeParticipant = linphone_chat_room_find_participant(paulineCr, chloeAddr);
	linphone_address_unref(chloeAddr);
	BC_ASSERT_PTR_NOT_NULL(chloeParticipant);
	chloeParticipant = linphone_participant_ref(chloeParticipant);
	bctbx_list_t *participantsToRemove = NULL;
	initialPaulineStats = pauline->stat;
	participantsToRemove = bctbx_list_append(participantsToRemove, marieParticipant);
	participantsToRemove = bctbx_list_append(participantsToRemove, chloeParticipant);
	linphone_chat_room_remove_participants(paulineCr, participantsToRemove);
	bctbx_list_free_with_data(participantsToRemove, (bctbx_list_free_func)linphone_participant_unref);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialChloeStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 0, int, "%d");

	// Pauline leaves the chat room
	wait_for_list(coresList, &dummy, 1, 1000);
	linphone_chat_room_leave(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminationPending,
	                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(laure->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(chloe->lc), 0, int, "%i");

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_creation_server_network_down(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	linphone_chat_room_params_enable_encryption(params, FALSE);
	LinphoneChatRoom *marieCr =
	    linphone_core_create_chat_room_2(marie->lc, params, initialSubject, participantsAddresses);
	linphone_chat_room_params_unref(params);

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, pauline->lc, &(marie->stat.number_of_LinphoneChatRoomStateInstantiated), 1, 100));
	// Check that the chat room is correctly created on Marie's side and that the participants are added
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
	                              &(marie->stat.number_of_LinphoneChatRoomSessionOutgoingProgress), 1,
	                              liblinphone_tester_sip_timeout));

	ms_message("%s turns off network", linphone_core_get_identity(marie->lc));
	linphone_core_set_network_reachable(marie->lc, FALSE);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &(marie->stat.number_of_LinphoneChatRoomStateCreationFailed),
	                              1, 2 * liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &(marie->stat.number_of_LinphoneChatRoomSessionError), 1,
	                              liblinphone_tester_sip_timeout));

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (marieCr) linphone_chat_room_unref(marieCr);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_creation_server_network_down_up(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	int dummy = 0;
	stats initialPaulineStats = pauline->stat;

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	linphone_chat_room_params_enable_encryption(params, FALSE);
	LinphoneChatRoom *marieCr =
	    linphone_core_create_chat_room_2(marie->lc, params, initialSubject, participantsAddresses);
	linphone_chat_room_params_unref(params);

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, pauline->lc, &(marie->stat.number_of_LinphoneChatRoomStateInstantiated), 1, 100));
	// Check that the chat room is correctly created on Marie's side and that the participants are added
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &(marie->stat.number_of_LinphoneChatRoomStateCreationPending),
	                              1, liblinphone_tester_sip_timeout));

	linphone_core_set_network_reachable(marie->lc, FALSE);
	BC_ASSERT_FALSE(wait_for_until(marie->lc, pauline->lc, &dummy, 1, 5000));
	linphone_core_set_network_reachable(marie->lc, TRUE);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &(marie->stat.number_of_LinphoneChatRoomStateCreated), 1,
	                              2 * liblinphone_tester_sip_timeout));
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	if (marieCr) linphone_chat_room_unref(marieCr);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_add_participant(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_remove_linphone_spec(chloe->lc, "groupchat");

	linphone_core_set_user_agent(marie->lc, "liblinphone-tester/tester_version (Marie MacOSX) SDK",
	                             "(sdk_version) (sdk_branch)");
	linphone_core_set_user_agent(laure->lc, "liblinphone-tester/tester_version (Pixel 6 Pro) SDK",
	                             "(sdk_version) (sdk_branch)");
	linphone_core_set_user_agent(chloe->lc, "liblinphone-tester/tester_version (Windows 11 Pro de Chloe) SDK",
	                             "(sdk_version) (sdk_branch)");

	LinphoneAddress *marie_identity = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *laure_identity = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneAddress *chloe_identity = linphone_address_new(linphone_core_get_identity(chloe->lc));

	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(marie_identity));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laure_identity));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Pauline creates friends
	LinphoneFriend *marie_friend = linphone_core_create_friend(pauline->lc);
	linphone_friend_set_name(marie_friend, "Marie");
	linphone_friend_add_address(marie_friend, marie_identity);
	linphone_core_add_friend(pauline->lc, marie_friend);

	LinphoneFriend *laure_friend = linphone_core_create_friend(pauline->lc);
	linphone_friend_set_name(laure_friend, "Laure");
	linphone_friend_add_address(laure_friend, laure_identity);
	linphone_core_add_friend(pauline->lc, laure_friend);

	LinphoneFriend *chloe_friend = linphone_core_create_friend(pauline->lc);
	linphone_friend_set_name(chloe_friend, "Chloe");
	linphone_friend_add_address(chloe_friend, chloe_identity);
	linphone_core_add_friend(pauline->lc, chloe_friend);

	bctbx_list_t *marie_devices = linphone_friend_get_devices(marie_friend);
	BC_ASSERT_PTR_NULL(marie_devices);
	bctbx_list_t *laure_devices = linphone_friend_get_devices(laure_friend);
	BC_ASSERT_PTR_NULL(laure_devices);
	bctbx_list_t *chloe_devices = linphone_friend_get_devices(chloe_friend);
	BC_ASSERT_PTR_NULL(chloe_devices);

	// Pauline creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *paulineCr =
	    create_chat_room_client_side(coresList, pauline, &initialPaulineStats, participantsAddresses, initialSubject,
	                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(paulineCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *marieCr =
	    check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	linphone_friend_unref(marie_friend);
	linphone_friend_unref(laure_friend);
	linphone_friend_unref(chloe_friend);

	// To simulate dialog removal for Pauline
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	LinphoneAddress *paulineAddr = linphone_address_clone(linphone_chat_room_get_peer_address(paulineCr));
	coresList = bctbx_list_remove(coresList, pauline->lc);
	linphone_core_manager_reinit(pauline);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline, TRUE);
	paulineCr = linphone_core_search_chat_room(pauline->lc, NULL, NULL, paulineAddr, NULL);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	linphone_address_unref(paulineAddr);

	marie_friend = linphone_core_find_friend(pauline->lc, marie_identity);
	BC_ASSERT_PTR_NOT_NULL(marie_friend);
	laure_friend = linphone_core_find_friend(pauline->lc, laure_identity);
	BC_ASSERT_PTR_NOT_NULL(laure_friend);
	chloe_friend = linphone_core_find_friend(pauline->lc, chloe_identity);
	BC_ASSERT_PTR_NOT_NULL(chloe_friend);

	if (marie_friend) {
		marie_devices = linphone_friend_get_devices(marie_friend);
		BC_ASSERT_PTR_NOT_NULL(marie_devices);
	}
	if (laure_friend) {
		laure_devices = linphone_friend_get_devices(laure_friend);
		BC_ASSERT_PTR_NOT_NULL(laure_devices);
	}

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(chloe_identity));
	if (paulineCr) {
		linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	}
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Refused by server because group chat disabled for Chloe
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
	                              initialMarieStats.number_of_chat_room_participants_added + 1,
	                              liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
	                              initialPaulineStats.number_of_chat_room_participants_added + 1,
	                              liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_added,
	                              initialLaureStats.number_of_chat_room_participants_added + 1,
	                              liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");

	chloe_devices = linphone_friend_get_devices(chloe_friend);
	BC_ASSERT_PTR_NULL(chloe_devices);

	// Try to search for a participant that is not in the chatroom
	LinphoneAddress *fakeAddr = linphone_address_new("sip:toto@sip.example.org");
	LinphoneParticipant *fakeParticipant = linphone_chat_room_find_participant(marieCr, fakeAddr);
	linphone_address_unref(fakeAddr);
	BC_ASSERT_PTR_NULL(fakeParticipant);

	// Pauline begins composing a message
	linphone_chat_room_compose(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived,
	                             initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived,
	                             initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1,
	                             liblinphone_tester_sip_timeout));

	// Now, Chloe is upgrading to group chat client
	linphone_core_set_network_reachable(chloe->lc, FALSE);
	coresList = bctbx_list_remove(coresList, chloe->lc);
	linphone_core_manager_reinit(chloe);
	tmpCoresManagerList = bctbx_list_append(NULL, chloe);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(chloe, TRUE);

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(chloe_identity));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
	                             initialMarieStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
	                             initialPaulineStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_added,
	                             initialLaureStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	chloe_devices = linphone_friend_get_devices(chloe_friend);
	BC_ASSERT_PTR_NOT_NULL(chloe_devices);

	linphone_address_unref(marie_identity);
	linphone_address_unref(laure_identity);
	linphone_address_unref(chloe_identity);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(laure->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(chloe->lc), 0, int, "%i");

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static int im_encryption_engine_process_incoming_message_cb(BCTBX_UNUSED(LinphoneImEncryptionEngine *engine),
                                                            BCTBX_UNUSED(LinphoneChatRoom *room),
                                                            LinphoneChatMessage *msg) {
	if (linphone_chat_message_get_content_type(msg)) {
		if (strcmp(linphone_chat_message_get_content_type(msg), "cipher/b64") == 0) {
			size_t b64Size = 0;
			unsigned char *output;
			const char *msg_str = linphone_chat_message_get_text(msg);
			bctbx_base64_decode(NULL, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
			output = (unsigned char *)ms_malloc(b64Size + 1),
			bctbx_base64_decode(output, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
			output[b64Size] = '\0';
			linphone_chat_message_set_text(msg, (char *)output);
			ms_free(output);
			linphone_chat_message_set_content_type(msg, "message/cpim");
			return 0;
		} else if (strcmp(linphone_chat_message_get_content_type(msg), "application/im-iscomposing+xml") == 0) {
			return -1; // Not encrypted, nothing to do
		} else {
			return 488; // Not acceptable
		}
	}
	return 500;
}

static int im_encryption_engine_process_outgoing_message_cb(BCTBX_UNUSED(LinphoneImEncryptionEngine *engine),
                                                            BCTBX_UNUSED(LinphoneChatRoom *room),
                                                            LinphoneChatMessage *msg) {
	if (strcmp(linphone_chat_message_get_content_type(msg), "message/cpim") == 0) {
		size_t b64Size = 0;
		unsigned char *output;
		const char *msg_str = linphone_chat_message_get_text(msg);
		bctbx_base64_encode(NULL, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
		output = (unsigned char *)ms_malloc0(b64Size + 1);
		bctbx_base64_encode(output, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
		output[b64Size] = '\0';
		linphone_chat_message_set_text(msg, (const char *)output);
		ms_free(output);
		linphone_chat_message_set_content_type(msg, "cipher/b64");
		return 0;
	}
	return -1;
}

static void group_chat_room_message(bool_t encrypt, bool_t sal_error, bool_t im_encryption_mandatory) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	LinphoneImEncryptionEngine *marie_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *marie_cbs = linphone_im_encryption_engine_get_callbacks(marie_imee);
	LinphoneImEncryptionEngine *pauline_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *pauline_cbs = linphone_im_encryption_engine_get_callbacks(pauline_imee);
	LinphoneImEncryptionEngine *chloe_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *chloe_cbs = linphone_im_encryption_engine_get_callbacks(chloe_imee);

	if (im_encryption_mandatory) {
		LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);
		LinphoneAccountParams *pauline_params =
		    linphone_account_params_clone(linphone_account_get_params(pauline_account));
		linphone_account_params_set_instant_messaging_encryption_mandatory(pauline_params, true);
		linphone_account_set_params(pauline_account, pauline_params);
		linphone_account_params_unref(pauline_params);

		LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
		LinphoneAccountParams *marie_params = linphone_account_params_clone(linphone_account_get_params(marie_account));
		linphone_account_params_set_instant_messaging_encryption_mandatory(marie_params, true);
		linphone_account_set_params(marie_account, marie_params);
		linphone_account_params_unref(marie_params);

		LinphoneAccount *chloe_account = linphone_core_get_default_account(chloe->lc);
		LinphoneAccountParams *chloe_params = linphone_account_params_clone(linphone_account_get_params(chloe_account));
		linphone_account_params_set_instant_messaging_encryption_mandatory(chloe_params, true);
		linphone_account_set_params(chloe_account, chloe_params);
		linphone_account_params_unref(chloe_params);
	}

	if (encrypt) {
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(
		    marie_cbs, im_encryption_engine_process_outgoing_message_cb);
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(
		    pauline_cbs, im_encryption_engine_process_outgoing_message_cb);
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(
		    chloe_cbs, im_encryption_engine_process_outgoing_message_cb);

		linphone_im_encryption_engine_cbs_set_process_incoming_message(
		    marie_cbs, im_encryption_engine_process_incoming_message_cb);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(
		    pauline_cbs, im_encryption_engine_process_incoming_message_cb);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(
		    chloe_cbs, im_encryption_engine_process_incoming_message_cb);

		linphone_core_set_im_encryption_engine(marie->lc, marie_imee);
		linphone_core_set_im_encryption_engine(pauline->lc, pauline_imee);
		linphone_core_set_im_encryption_engine(chloe->lc, chloe_imee);
	}

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Chloe begins composing a message
	linphone_chat_room_compose(chloeCr);
	if ((im_encryption_mandatory && encrypt) || !im_encryption_mandatory) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived,
		                             initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived,
		                             initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1,
		                             liblinphone_tester_sip_timeout));
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived,
		                              initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived,
		                              initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	}

	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	const char *messageId = linphone_chat_message_get_message_id(chloeMessage);
	LinphoneChatMessage *marieLastMsg = NULL;

	if ((im_encryption_mandatory && encrypt) || !im_encryption_mandatory) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
		                             initialMarieStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
		                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived,
		                             initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived,
		                             initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1,
		                             liblinphone_tester_sip_timeout));
		marieLastMsg = marie->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg)) goto end;

		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
		LinphoneChatMessage *foundMessage = linphone_chat_room_find_message(chloeCr, messageId);
		BC_ASSERT_PTR_NOT_NULL(foundMessage);
		LinphoneEventLog *foundEventLog = linphone_chat_room_find_event_log(chloeCr, messageId);
		BC_ASSERT_PTR_NOT_NULL(foundEventLog);
		BC_ASSERT_PTR_NOT_NULL(linphone_chat_message_get_text(foundMessage));
		linphone_chat_message_unref(foundMessage);
		linphone_event_log_unref(foundEventLog);

		LinphoneEventLog *event =
		    linphone_chat_room_search_chat_message_by_text(marieCr, "Hello", NULL, LinphoneSearchDirectionUp);
		if (BC_ASSERT_PTR_NOT_NULL(event)) {
			LinphoneChatMessage *chat_from_search = linphone_event_log_get_chat_message(event);
			if (BC_ASSERT_PTR_NOT_NULL(chat_from_search)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(chat_from_search), "Hello");
			}
			linphone_event_log_unref(event);
		}

		LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
		linphone_address_unref(chloeAddr);
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
		                              initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
		                              initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived,
		                              initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived,
		                              initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	}
	linphone_chat_message_unref(chloeMessage); // Unref here because of messageId using

	// Pauline begins composing a messagewith some accents
	linphone_chat_room_compose(paulineCr);
	if ((im_encryption_mandatory && encrypt) || !im_encryption_mandatory) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived,
		                             initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingActiveReceived,
		                             initialChloeStats.number_of_LinphoneIsComposingActiveReceived + 1,
		                             liblinphone_tester_sip_timeout));
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived,
		                              initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingActiveReceived,
		                              initialChloeStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	}
	const char *paulineTextMessage = "Héllö Dàrling";
	LinphoneChatMessage *paulineMessage = _send_message(paulineCr, paulineTextMessage);

	if ((im_encryption_mandatory && encrypt) || !im_encryption_mandatory) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
		                             initialMarieStats.number_of_LinphoneMessageReceived + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
		                             initialChloeStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived,
		                             initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingIdleReceived,
		                             initialChloeStats.number_of_LinphoneIsComposingIdleReceived + 1,
		                             liblinphone_tester_sip_timeout));

		marieLastMsg = marie->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg)) goto end;
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), paulineTextMessage);

		LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
		linphone_address_unref(paulineAddr);
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
		                              initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
		                              initialChloeStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived,
		                              initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingIdleReceived,
		                              initialChloeStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	}
	linphone_chat_message_unref(paulineMessage);

	if (sal_error) {
		sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
		LinphoneChatMessage *msg = _send_message(marieCr, "Bli bli bli");
		char *message_id = ms_strdup(linphone_chat_message_get_message_id(msg));
		BC_ASSERT_STRING_NOT_EQUAL(message_id, "");

		wait_for_list(coresList, NULL, 0, 1000);

		sal_set_send_error(linphone_core_get_sal(marie->lc), 0);
		linphone_chat_message_send(msg);
		const char *message_id_2 = linphone_chat_message_get_message_id(msg);
		BC_ASSERT_STRING_NOT_EQUAL(message_id_2, "");
		BC_ASSERT_STRING_EQUAL(message_id, message_id_2);
		ms_free(message_id);

		wait_for_list(coresList, NULL, 0, 1000);

		linphone_core_refresh_registers(marie->lc);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneRegistrationOk,
		                             initialMarieStats.number_of_LinphoneRegistrationOk + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
	}

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_im_encryption_engine_unref(marie_imee);
	linphone_im_encryption_engine_unref(pauline_imee);
	linphone_im_encryption_engine_unref(chloe_imee);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_send_message(void) {
	group_chat_room_message(FALSE, FALSE, FALSE);
}

static void group_chat_room_send_message_im_encryption_mandatory(void) {
	group_chat_room_message(FALSE, FALSE, TRUE);
}

static void group_chat_room_send_message_encrypted(void) {
	group_chat_room_message(TRUE, FALSE, FALSE);
}

static void group_chat_room_send_message_encrypted_im_encryption_mandatory(void) {
	group_chat_room_message(TRUE, FALSE, TRUE);
}

static void group_chat_room_send_message_with_error(void) {
	group_chat_room_message(FALSE, TRUE, FALSE);
}

static void group_chat_room_invite_multi_register_account(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline1's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Pauline2's side and that the participants are added
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats,
	                                                                    confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline1, paulineCr, coresList);
	linphone_core_delete_chat_room(pauline2->lc, paulineCr2);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_add_admin(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_add_admin_lately_notified(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Simulate pauline has disappeared
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));

	// Make sure pauline is not notified
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                              initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                              3000));
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_add_admin_non_admin(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;

	// Pauline designates Laure as admin
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(paulineCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, laureParticipant, TRUE);
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipant));
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_remove_admin(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

	// Pauline revokes the admin status of Marie
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, marieParticipant, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(linphone_participant_is_admin(marieParticipant));

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_admin_creator_leaves_the_room(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie leaves the room
	linphone_chat_room_leave(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_removed,
	                             initialLaureStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	/* FIXME: the number of admin status changed shoud be 1. But the conference server notifies first that marie looses
	 * its admin status, before being deleted from the chatroom. This is indeed useless to notify this. Once fixed in
	 * the conference server, the counter shall be set back to +1. */
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(linphone_chat_room_get_me(laureCr)));

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_change_subject(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	const char *newSubject = "New subject";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie now changes the subject
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_core_chat_room_subject_changed,
	                             initialMarieStats.number_of_core_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_core_chat_room_subject_changed,
	                             initialPaulineStats.number_of_core_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_core_chat_room_subject_changed,
	                             initialLaureStats.number_of_core_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
end:
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_change_subject_non_admin(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	const char *newSubject = "New subject";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie now changes the subject
	linphone_chat_room_set_subject(paulineCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), initialSubject);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
end:
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_remove_participant_base(bool_t restart) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");

	if (restart == TRUE) {
		// Restart core for Marie
		coresList = bctbx_list_remove(coresList, marie->lc);
		linphone_core_manager_restart(marie, TRUE);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
		bctbx_list_t *tmpInitList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpInitList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_append(coresList, marie->lc);

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr = linphone_address_clone(
		    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc)));
		marieCr = linphone_core_search_chat_room(marie->lc, NULL, marieDeviceAddr, confAddr, NULL);
		linphone_address_unref(marieDeviceAddr);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
	}

	// Check that participant removed event has been stored
	// Passing 0 as second argument because all events must be retrived
	int nbMarieParticipantRemoved = 0;
	bctbx_list_t *marieHistory = linphone_chat_room_get_history_2(
	    marieCr, 0, LinphoneChatRoomHistoryFilterChatMessage | LinphoneChatRoomHistoryFilterInfoNoDevice);
	for (bctbx_list_t *item = marieHistory; item; item = bctbx_list_next(item)) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
		if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceParticipantRemoved) {
			nbMarieParticipantRemoved++;
			const LinphoneAddress *removedParticipantAddress = linphone_event_log_get_participant_address(event);
			BC_ASSERT_PTR_NOT_NULL(removedParticipantAddress);
		}
	}
	bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_event_log_unref);
	BC_ASSERT_EQUAL(nbMarieParticipantRemoved, 1, unsigned int, "%u");

	linphone_address_unref(confAddr);

	// Clean db from chat room
	linphone_core_delete_chat_room(marie->lc, marieCr);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
end:
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_remove_participant(void) {
	group_chat_room_remove_participant_base(FALSE);
}

static void group_chat_room_remove_participant_and_restart(void) {
	group_chat_room_remove_participant_base(TRUE);
}

static void group_chat_room_send_message_with_participant_removed(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	if (!BC_ASSERT_PTR_NOT_NULL(laureParticipant)) goto end;

	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	// Laure try to send a message with the chat room where she was removed
	const char *laureTextMessage = "Hello";
	LinphoneChatMessage *laureMessage = _send_message(laureCr, laureTextMessage);
	linphone_chat_message_unref(laureMessage);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageDelivered, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageReceived, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived,
	                             initialMarieStats.number_of_LinphoneIsComposingIdleReceived,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived,
	                             initialPaulineStats.number_of_LinphoneIsComposingIdleReceived,
	                             liblinphone_tester_sip_timeout));

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_leave(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(laureCr));

	linphone_chat_room_leave(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_removed,
	                             initialLaureStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_chat_room_is_read_only(paulineCr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_delete_twice(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	// Save db
	const char *uri = linphone_config_get_string(linphone_core_get_config(laure->lc), "storage", "uri", "");
	char *uriCopy = random_filepath("linphone_tester", "db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopy));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	// Reset db
	if (laure->database_path) bc_free(laure->database_path);
	laure->database_path = uriCopy;
	coresList = bctbx_list_remove(coresList, laure->lc);
	linphone_core_manager_reinit(laure);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(laure, TRUE);

	// Check that the chat room has correctly created on Laure's side and that the participants are added
	laureCr = check_has_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
end:
	// Clean db from chat room again
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_come_back_after_disconnection(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	const char *newSubject = "New subject";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;
	linphone_core_set_network_reachable(marie->lc, FALSE);

	wait_for_list(coresList, &dummy, 1, 1000);

	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_create_room_with_disconnected_friends_base(bool_t initial_message) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	LinphoneChatRoom *paulineCr = NULL;
	LinphoneChatRoom *laureCr = NULL;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	wait_for_list(coresList, &dummy, 1, 2000);

	// Disconnect pauline and laure
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, FALSE);

	wait_for_list(coresList, &dummy, 1, 2000);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	if (initial_message) {
		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "Salut");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
	}

	wait_for_list(coresList, &dummy, 1, 4000);

	// Reconnect Pauline and check that the chat room is correctly created on Pauline's side and that the participants
	// are added
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Reconnect Laure and check that the chat room is correctly created on Laure's side and that the participants are
	// added
	linphone_core_set_network_reachable(laure->lc, TRUE);
	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	if (initial_message) {
		if (BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1,
		                                 liblinphone_tester_sip_timeout))) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "Salut");
				linphone_chat_message_unref(msg);
			}
		}
		if (BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, 1,
		                                 liblinphone_tester_sip_timeout))) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(laureCr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "Salut");
				linphone_chat_message_unref(msg);
			}
		}
	}

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_create_room_with_disconnected_friends(void) {
	group_chat_room_create_room_with_disconnected_friends_base(FALSE);
}

static void group_chat_room_create_room_with_disconnected_friends_and_initial_message(void) {
	group_chat_room_create_room_with_disconnected_friends_base(TRUE);
}

static void group_chat_room_reinvited_after_removed_base(bool_t offline_when_removed,
                                                         bool_t offline_when_reinvited,
                                                         bool_t restart_after_reinvited) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");

	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	const char *marieIdentity = linphone_core_get_identity(marie->lc);
	const char *paulineIdentity = linphone_core_get_identity(pauline->lc);
	const char *laureIdentity = linphone_core_get_identity(laure->lc);

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(paulineIdentity));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(laureIdentity));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	char *savedLaureUuid = NULL;
	char *laureParticipantAddress = NULL;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);
	char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *newLaureCr = NULL;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;

	LinphoneAddress *laureAddr = linphone_address_new(laureIdentity);
	laureParticipantAddress = linphone_address_as_string(laureAddr);
	if (offline_when_removed) {
		savedLaureUuid =
		    bctbx_strdup(linphone_config_get_string(linphone_core_get_config(laure->lc), "misc", "uuid", NULL));
		coresList = bctbx_list_remove(coresList, laure->lc);
		coresManagerList = bctbx_list_remove(coresManagerList, laure);
		linphone_core_set_network_reachable(laure->lc, FALSE);
		linphone_core_manager_stop(laure);
	}

	// Marie removes Laure from the chat room
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	ms_message("%s removes %s to chatroom %s", marieIdentity, laureParticipantAddress, conference_address_str);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	if (offline_when_removed && !offline_when_reinvited) {
		wait_for_list(coresList, 0, 1, 20000); /* see large comment below, also applicable for BYE */
		linphone_core_manager_configure(laure);
		linphone_config_set_string(linphone_core_get_config(laure->lc), "misc", "uuid", savedLaureUuid);
		bctbx_free(savedLaureUuid);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		linphone_core_manager_start(laure, TRUE);
		laureIdentity = linphone_core_get_identity(laure->lc);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		coresManagerList = bctbx_list_append(coresManagerList, laure);
	}
	if (!offline_when_reinvited) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
		                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
	}

	wait_for_list(coresList, 0, 1, 2000);
	initialLaureStats = laure->stat;

	// Marie adds Laure to the chat room
	ms_message("%s adds %s to chatroom %s", marieIdentity, laureParticipantAddress, conference_address_str);
	participantsAddresses = bctbx_list_append(participantsAddresses, laureAddr);
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
	                             initialMarieStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
	                             initialPaulineStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	if (offline_when_reinvited) {
		/*
		 * Hack: while we were offline the server sent an INVITE, that will get a 408 timeout after 20 seconds.
		 * During this time, the REGISTER from Laure won't trigger a new INVITE.
		 * FIXME: it should be handled at server side.
		 * However, since the case isn't very real-world probable, we can workaround it in the test by waiting this
		 * INVITE to timeout, and then REGISTER.
		 * */
		wait_for_list(coresList, 0, 1, 20000);
		linphone_core_manager_configure(laure);
		linphone_config_set_string(linphone_core_get_config(laure->lc), "misc", "uuid", savedLaureUuid);
		bctbx_free(savedLaureUuid);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		linphone_core_manager_start(laure, TRUE);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		coresManagerList = bctbx_list_append(coresManagerList, laure);
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateCreated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	char *laureDeviceIdentity = linphone_core_get_device_identity(laure->lc);
	laureAddr = linphone_address_new(laureDeviceIdentity);
	bctbx_free(laureDeviceIdentity);
	newLaureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureAddr);
	linphone_address_unref(laureAddr);
	if (!offline_when_removed) BC_ASSERT_PTR_EQUAL(newLaureCr, laureCr);
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newLaureCr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newLaureCr), initialSubject);
	BC_ASSERT_FALSE(linphone_chat_room_has_been_left(newLaureCr));

	unsigned int nbLaureConferenceCreatedEventsBeforeRestart = 0;
	bctbx_list_t *laureHistory = linphone_chat_room_get_history_events(newLaureCr, 0);
	for (bctbx_list_t *item = laureHistory; item; item = bctbx_list_next(item)) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
		if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceCreated)
			nbLaureConferenceCreatedEventsBeforeRestart++;
	}
	bctbx_list_free_with_data(laureHistory, (bctbx_list_free_func)linphone_event_log_unref);
	// Restarting Laure's core doesn't generate a second ConferenceCreated event
	BC_ASSERT_EQUAL(nbLaureConferenceCreatedEventsBeforeRestart, 1, unsigned int, "%u");

	if (restart_after_reinvited) {
		ms_message("%s restarts its core", laureIdentity);
		coresList = bctbx_list_remove(coresList, laure->lc);
		linphone_core_manager_reinit(laure);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(laure, TRUE);
		laureDeviceIdentity = linphone_core_get_device_identity(laure->lc);
		laureAddr = linphone_address_new(laureDeviceIdentity);
		bctbx_free(laureDeviceIdentity);
		newLaureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureAddr);
		linphone_address_unref(laureAddr);
		wait_for_list(coresList, 0, 1, 2000);
		BC_ASSERT_FALSE(linphone_chat_room_has_been_left(newLaureCr));

		unsigned int nbLaureConferenceCreatedEventsAfterRestart = 0;
		bctbx_list_t *laureHistory = linphone_chat_room_get_history_events(newLaureCr, 0);
		for (bctbx_list_t *item = laureHistory; item; item = bctbx_list_next(item)) {
			LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
			if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceCreated)
				nbLaureConferenceCreatedEventsAfterRestart++;
		}
		bctbx_list_free_with_data(laureHistory, (bctbx_list_free_func)linphone_event_log_unref);
		BC_ASSERT_EQUAL(nbLaureConferenceCreatedEventsAfterRestart, nbLaureConferenceCreatedEventsBeforeRestart + 1,
		                unsigned int, "%u");
	}
end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, newLaureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	ms_free(conference_address_str);
	if (laureParticipantAddress) ms_free(laureParticipantAddress);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_reinvited_after_removed(void) {
	group_chat_room_reinvited_after_removed_base(FALSE, FALSE, FALSE);
}

static void group_chat_room_reinvited_after_removed_2(void) {
	group_chat_room_reinvited_after_removed_base(FALSE, FALSE, TRUE);
}

static void group_chat_room_reinvited_after_removed_while_offline(void) {
	group_chat_room_reinvited_after_removed_base(TRUE, FALSE, FALSE);
}

static void group_chat_room_reinvited_after_removed_while_offline_2(void) {
	group_chat_room_reinvited_after_removed_base(TRUE, TRUE, FALSE);
}

static void group_chat_room_reinvited_after_removed_with_several_devices(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie2 is initially offline
	linphone_core_set_network_reachable(marie2->lc, FALSE);

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marie1Cr =
	    create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject,
	                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	LinphoneChatRoom *marie2Cr = NULL;
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marie1Cr);

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are
	// added
	LinphoneChatRoom *pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats,
	                                                                    confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats,
	                                                                    confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *newPauline1Cr = NULL;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr) || !BC_ASSERT_PTR_NOT_NULL(pauline1Cr) ||
	    !BC_ASSERT_PTR_NOT_NULL(pauline2Cr) || !BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;

	// Marie1 removes Pauline from the chat room while Pauline2 is offline
	linphone_core_set_network_reachable(pauline2->lc, FALSE);
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline1->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marie1Cr, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_remove_participant(marie1Cr, paulineParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_chat_room_participants_removed,
	                             initialMarie1Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_removed,
	                             initialLaureStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPauline1Stats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	// Marie2 comes online, check that pauline is not notified as still being in the chat room
	linphone_core_set_network_reachable(marie2->lc, TRUE);
	marie2Cr =
	    check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, TRUE);

	// Marie2 adds Pauline back to the chat room
	initialPauline1Stats = pauline1->stat;
	participantsAddresses = bctbx_list_append(participantsAddresses, paulineAddr);
	linphone_chat_room_add_participants(marie2Cr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_chat_room_participants_added,
	                             initialMarie1Stats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_chat_room_participants_added,
	                             initialMarie2Stats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_added,
	                             initialLaureStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie1Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie2Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");
	newPauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr,
	                                                     initialSubject, 2, FALSE);
	BC_ASSERT_PTR_EQUAL(newPauline1Cr, pauline1Cr);
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newPauline1Cr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newPauline1Cr), initialSubject);
end:
	// Clean chat room from db
	linphone_core_set_network_reachable(pauline2->lc, TRUE);
	linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	linphone_core_delete_chat_room(marie2->lc, marie2Cr);
	initialPauline2Stats = pauline2->stat;
	linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPauline2Stats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	linphone_core_delete_chat_room(pauline2->lc, pauline2Cr);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_notify_after_disconnection(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_PTR_NOT_NULL(marieCr);

	participantsAddresses = NULL;

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(laureCr);

	// Marie now changes the subject
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	newSubject = "Let's go drink a beer";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                              initialPaulineStats.number_of_chat_room_subject_changed + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_NOT_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 2,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);

	// Test with more than one missed notify
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	newSubject = "Let's go drink a mineral water !";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                              initialPaulineStats.number_of_chat_room_subject_changed + 3, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_NOT_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	// Marie designates Laure as admin
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	LinphoneParticipant *laureParticipantFromPauline = linphone_chat_room_find_participant(paulineCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantFromPauline);
	linphone_chat_room_set_participant_admin_status(marieCr, laureParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                              initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                              3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant));
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipantFromPauline));

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipantFromPauline));

	// Test with a participant being removed
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Laure leaves the room
	linphone_chat_room_leave(laureCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                              initialPaulineStats.number_of_chat_room_participants_removed + 1, 3000));

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_notify_after_core_restart(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_PTR_NOT_NULL(marieCr);

	participantsAddresses = NULL;

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(laureCr);

	// Now paulines stops it's Core
	coresList = bctbx_list_remove(coresList, pauline->lc);
	// Make sure gruu is preserved
	char *uuid = bctbx_strdup(linphone_config_get_string(linphone_core_get_config(pauline->lc), "misc", "uuid", NULL));
	linphone_core_set_network_reachable(pauline->lc, FALSE); // to avoid unregister
	linphone_core_manager_stop(pauline);

	// Marie changes the subject
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	// Laure leaves the room
	linphone_chat_room_leave(laureCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	// Pauline comes back
	linphone_core_manager_reinit(pauline);
	coresList = bctbx_list_append(coresList, pauline->lc);

	// Make sure gruu is restored
	linphone_config_set_string(linphone_core_get_config(pauline->lc), "misc", "uuid", uuid);

	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	bctbx_list_t *tmpInitList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpInitList);
	bctbx_list_free(tmpCoresManagerList);

	// Paulines starts it's Core again
	linphone_core_manager_start(pauline, TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1, 3000));

	char *deviceIdentity = linphone_core_get_device_identity(pauline->lc);
	LinphoneAddress *localAddr = linphone_address_new(deviceIdentity);
	bctbx_free(deviceIdentity);
	paulineCr = linphone_core_find_chat_room(pauline->lc, confAddr, localAddr);
	linphone_address_unref(localAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_free(uuid);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_send_refer_to_all_devices(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject,
	                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on second Marie's device
	LinphoneChatRoom *marieCr2 =
	    check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats,
	                                                                   confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats,
	                                                                    confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Check that added Marie's device didn't change her admin status
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie1->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_get_me(marieCr);
	if (BC_ASSERT_PTR_NOT_NULL(marieParticipant)) BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_get_me(marieCr2);
	if (BC_ASSERT_PTR_NOT_NULL(marieParticipant)) BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	if (BC_ASSERT_PTR_NOT_NULL(marieParticipant)) BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_find_participant(paulineCr2, marieAddr);
	if (BC_ASSERT_PTR_NOT_NULL(marieParticipant)) BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_find_participant(laureCr, marieAddr);
	if (BC_ASSERT_PTR_NOT_NULL(marieParticipant)) BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	linphone_address_unref(marieAddr);

	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_chat_room_participants_removed,
	                             initialMarie1Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_chat_room_participants_removed,
	                             initialMarie2Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_chat_room_participants_removed,
	                             initialPauline1Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_chat_room_participants_removed,
	                             initialPauline2Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr2), 1, int, "%d");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marieCr, coresList);
	linphone_core_delete_chat_room(marie2->lc, marieCr2);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline1, paulineCr, coresList);
	linphone_core_delete_chat_room(pauline2->lc, paulineCr2);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_add_device(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject,
	                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats,
	                                                                   confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats,
	                                                                    confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie adds a new device
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	LinphoneAddress *factoryAddr = linphone_address_new(sFactoryUri);
	_configure_core_for_conference(marie2, factoryAddr);
	linphone_address_unref(factoryAddr);
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
	configure_core_for_callbacks(marie2, cbs);
	linphone_core_cbs_unref(cbs);
	coresList = bctbx_list_append(coresList, marie2->lc);
	stats initialMarie2Stats = marie2->stat;
	_start_core(marie2);
	// Check that the chat room is correctly created on second Marie's device
	LinphoneChatRoom *marieCr2 =
	    check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_chat_room_participant_devices_added,
	                             initialMarie1Stats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_chat_room_participant_devices_added,
	                             initialMarie1Stats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_chat_room_participant_devices_added,
	                             initialMarie1Stats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_devices_added,
	                             initialMarie1Stats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));

	// Check that adding Marie's device didn't change her admin status
	LinphoneParticipant *marieParticipant = linphone_chat_room_get_me(marieCr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));
	marieParticipant = linphone_chat_room_get_me(marieCr2);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie1->lc));
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));
	marieParticipant = linphone_chat_room_find_participant(paulineCr2, marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));
	marieParticipant = linphone_chat_room_find_participant(laureCr, marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));
	linphone_address_unref(marieAddr);

	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_chat_room_participants_removed,
	                             initialMarie1Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_chat_room_participants_removed,
	                             initialMarie2Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_chat_room_participants_removed,
	                             initialPauline1Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_chat_room_participants_removed,
	                             initialPauline2Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr2), 1, int, "%d");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marieCr, coresList);
	linphone_core_delete_chat_room(marie2->lc, marieCr2);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline1, paulineCr, coresList);
	linphone_core_delete_chat_room(pauline2->lc, paulineCr2);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void multiple_is_composing_notification(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	const bctbx_list_t *composing_addresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Only one is composing
	linphone_chat_room_compose(paulineCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, 1,
	                             liblinphone_tester_sip_timeout));

	// Laure side
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity));
	}

	// Marie side
	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity));
	}

	// Pauline side
	composing_addresses = linphone_chat_room_get_composing_addresses(paulineCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");

	wait_for_list(coresList, 0, 1, 1500);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, 1,
	                             liblinphone_tester_sip_timeout));

	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");

	// multiple is composing
	linphone_chat_room_compose(paulineCr);
	linphone_chat_room_compose(marieCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, 3,
	                             liblinphone_tester_sip_timeout)); // + 2
	// Laure side
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 2, int, "%i");
	if (bctbx_list_size(composing_addresses) == 2) {
		while (composing_addresses) {
			LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
			bool_t equal =
			    strcmp(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity)) == 0 ||
			    strcmp(linphone_address_get_username(addr), linphone_address_get_username(marie->identity)) == 0;
			BC_ASSERT_TRUE(equal);
			composing_addresses = bctbx_list_next(composing_addresses);
		}
	}

	// Marie side
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 2,
	                             liblinphone_tester_sip_timeout)); // + 1
	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity));
	}

	// Pauline side
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, 1, 2000));
	composing_addresses = linphone_chat_room_get_composing_addresses(paulineCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(marie->identity));
	}

	wait_for_list(coresList, 0, 1, 1500);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, 1,
	                             liblinphone_tester_sip_timeout));

	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(paulineCr);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_creation_fails_if_invited_participants_dont_support_it(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_remove_linphone_spec(pauline->lc, "groupchat");
	linphone_core_remove_linphone_spec(laure->lc, "groupchat");
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;

	// Marie creates a new group chat room
	LinphoneChatRoom *marieCr = linphone_core_create_client_group_chat_room(marie->lc, "Hello there", FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateInstantiated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateInstantiated + 1, 100));

	// Add participants
	linphone_chat_room_add_participants(marieCr, participantsAddresses);

	// Check that the group chat room creation fails
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationFailed,
	                             initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed + 1,
	                             liblinphone_tester_sip_timeout));
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_EQUAL(linphone_chat_room_get_state(marieCr), LinphoneChatRoomStateCreationFailed, int, "%d");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_creation_successful_if_at_least_one_invited_participant_supports_it(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_remove_linphone_spec(laure->lc, "groupchat");
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
	    coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, 1, FALSE,
	    LinphoneChatRoomEphemeralModeDeviceManaged);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);

	// Check that the chat room has not been created on Laure's side
	BC_ASSERT_EQUAL(initialLaureStats.number_of_LinphoneChatRoomStateCreated,
	                laure->stat.number_of_LinphoneChatRoomStateCreated, int, "%d");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_send_file_with_or_without_text(bool_t with_text, bool_t two_files, bool_t use_buffer) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *sendFilepath2 = NULL;
	char *receivePaulineFilepath = random_filepath("receive_file_pauline", "dump");
	char *receiveChloeFilepath = random_filepath("receive_file_chloe", "dump");
	const char *text = "Hello Group !";

	if (two_files) {
		sendFilepath2 = bc_tester_res("sounds/ahbahouaismaisbon.wav");
	}

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;

	/* Remove any previously downloaded file */
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Sending file
	if (with_text) {
		_send_file_plus_text(marieCr, sendFilepath, sendFilepath2, text, use_buffer);
	} else {
		_send_file(marieCr, sendFilepath, sendFilepath2, use_buffer);
	}

	// Check that chat rooms have received the file
	if (with_text) {
		_receive_file_plus_text(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath,
		                        sendFilepath2, text, use_buffer);
		_receive_file_plus_text(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, sendFilepath2,
		                        text, use_buffer);
	} else {
		_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, sendFilepath2,
		              use_buffer);
		_receive_file(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, sendFilepath2,
		              use_buffer);
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);
	bc_free(sendFilepath);
	if (sendFilepath2) bc_free(sendFilepath2);
	bc_free(receivePaulineFilepath);
	bc_free(receiveChloeFilepath);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_send_file(void) {
	group_chat_room_send_file_with_or_without_text(FALSE, FALSE, FALSE);
}

static void group_chat_room_send_file_2(void) {
	group_chat_room_send_file_with_or_without_text(FALSE, FALSE, TRUE);
}

static void group_chat_room_send_file_plus_text(void) {
	group_chat_room_send_file_with_or_without_text(TRUE, FALSE, FALSE);
}

static void group_chat_room_send_two_files_plus_text(void) {
	group_chat_room_send_file_with_or_without_text(TRUE, TRUE, FALSE);
}

static void group_chat_room_send_multipart_custom_content_types(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);

	// Add our custom content types
	linphone_core_add_content_type_support(marie->lc, "application/vnd.3gpp.mcptt-info+xml");
	linphone_core_add_content_type_support(marie->lc, "application/vnd.3gpp.mcptt-location-info+xml");
	linphone_core_add_content_type_support(pauline->lc, "application/vnd.3gpp.mcptt-info+xml");
	linphone_core_add_content_type_support(pauline->lc, "application/vnd.3gpp.mcptt-location-info+xml");

	const char *data1 =
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?><mcpttinfo xmlns=\"urn:3gpp:ns:mcpttInfo:1.0\" "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><mcptt-Params><mcptt-request-uri "
	    "type=\"Normal\"><mcpttURI>sip:test@sip.example.org</mcpttURI></mcptt-request-uri></mcptt-Params></mcpttinfo>";
	const char *data2 =
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?><location-info "
	    "xmlns=\"urn:3gpp:ns:mcpttILocationInfo:1.0\"><Configuration><NonEmergencyLocationInformation><ServingEcgi/"
	    "><NeighbouringEcgi/><MbmsSaId/><MbsfnArea/><GeographicalCoordinate/><minimumIntervalLength>10</"
	    "minimumIntervalLength></NonEmergencyLocationInformation><TriggeringCriteria><CellChange><AnyCellChange "
	    "TriggerId=\"AnyCellChange\"/></CellChange><TrackingAreaChange><AnyTrackingAreaChange "
	    "TriggerId=\"AnyTrackingAreaChange\"/></TrackingAreaChange><PlmnChange><AnyPlmnChange "
	    "TriggerId=\"AnyPlmnChange\"/></PlmnChange><PeriodicReport "
	    "TriggerId=\"Periodic\">20</PeriodicReport><GeographicalAreaChange><AnyAreaChange "
	    "TriggerId=\"177db491c22\"></AnyAreaChange></GeographicalAreaChange></TriggeringCriteria></Configuration></"
	    "location-info>";

	LinphoneChatMessage *msg;
	msg = linphone_chat_room_create_empty_message(marieCr);

	LinphoneContent *content = linphone_factory_create_content(linphone_factory_get());
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "vnd.3gpp.mcptt-info+xml");
	linphone_content_set_utf8_text(content, data1);
	linphone_chat_message_add_content(msg, content);
	linphone_content_unref(content);

	content = linphone_factory_create_content(linphone_factory_get());
	linphone_content_set_type(content, "application");
	linphone_content_set_subtype(content, "vnd.3gpp.mcptt-location-info+xml");
	linphone_content_set_utf8_text(content, data2);
	linphone_chat_message_add_content(msg, content);
	linphone_content_unref(content);

	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
	linphone_chat_message_unref(msg);

	if (pauline->stat.last_received_chat_message) {
		const bctbx_list_t *contents = linphone_chat_message_get_contents(pauline->stat.last_received_chat_message);
		BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 2, int, "%d");

		LinphoneContent *content1 = bctbx_list_get_data(contents);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content1), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content1), "vnd.3gpp.mcptt-info+xml");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(content1), data1);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_related_chat_message_id(content1),
		                       linphone_chat_message_get_message_id(pauline->stat.last_received_chat_message));

		LinphoneContent *content2 = bctbx_list_nth_data(contents, 1);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content2), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content2), "vnd.3gpp.mcptt-location-info+xml");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(content2), data2);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_related_chat_message_id(content2),
		                       linphone_chat_message_get_message_id(pauline->stat.last_received_chat_message));
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_unique_one_to_one_chat_room_base(bool_t secondDeviceForSender) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *marie2 = NULL;
	if (secondDeviceForSender) marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	if (secondDeviceForSender) coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	/* This test was constructed according to 1.0 specification of groupchat.*/
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialMarie2Stats;
	memset(&initialMarie2Stats, 0, sizeof(initialMarie2Stats));
	if (secondDeviceForSender) initialMarie2Stats = marie2->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	const LinphoneAddress *tmpConfAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!tmpConfAddr) {
		goto end;
	}
	LinphoneAddress *confAddr = linphone_address_clone(tmpConfAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *marie2Cr = NULL;
	if (secondDeviceForSender)
		marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr,
		                                                initialSubject, 1, FALSE);

	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(paulineCr), 0, int, "%d");

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
	                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(paulineCr), 1, int, "%d");

	textMessage = "Hello again";
	message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
	                             initialMarieStats.number_of_LinphoneMessageDelivered + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(paulineCr), 2, int, "%d");
	LinphoneChatMessage *paulineMessage = pauline->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(paulineMessage);
	if (paulineMessage) {
		linphone_chat_message_mark_as_read(paulineMessage);
	}
	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(paulineCr), 1, int, "%d");

	if (secondDeviceForSender) linphone_core_set_network_reachable(marie2->lc, FALSE);
	// Marie deletes the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_chat_room_participants_removed,
	                initialPaulineStats.number_of_chat_room_participants_removed, int, "%d");

	// Marie creates the chat room again
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);

	// Marie sends a new message
	textMessage = "Hey again";
	message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
	                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Check that the created address is the same as before
	const LinphoneAddress *newConfAddr = linphone_chat_room_get_conference_address(marieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, newConfAddr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (secondDeviceForSender) {
		linphone_core_set_network_reachable(marie2->lc, TRUE);
		linphone_core_delete_chat_room(marie2->lc, marie2Cr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateTerminated,
		                             initialMarie2Stats.number_of_LinphoneChatRoomStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
	}
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(confAddr);
end:
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	if (secondDeviceForSender) linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_unique_one_to_one_chat_room(void) {
	group_chat_room_unique_one_to_one_chat_room_base(FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_dual_sender_device(void) {
	group_chat_room_unique_one_to_one_chat_room_base(TRUE);
}

static void group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(
    bool_t with_app_restart, bool_t forward_message, bool_t reply_message) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	/* This test checks the behavior of 1.0 groupchat protocol version, where one-to-one chatroom were supposed to be
	 * unique. */
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	char *messageId, *secondMessageId = NULL;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	const bctbx_list_t *contents = linphone_chat_message_get_contents(message);
	BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int, "%d");
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
	                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	messageId = bctbx_strdup(linphone_chat_message_get_message_id(message));
	linphone_chat_message_unref(message);

	if (forward_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, " %i");
		if (linphone_chat_room_get_history_size(paulineCr) > 0) {
			bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 1);
			LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)(history->data);

			LinphoneChatMessage *msg = linphone_chat_room_create_forward_message(paulineCr, recv_msg);
			const LinphoneAddress *forwarded_from_address = linphone_chat_message_get_from_address(recv_msg);
			char *forwarded_from = linphone_address_as_string_uri_only(forwarded_from_address);

			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_send(msg);

			BC_ASSERT_TRUE(linphone_chat_message_is_forward(msg));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(msg), forwarded_from);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);

			if (marie->stat.last_received_chat_message != NULL) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, " %i");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message),
				                       textMessage);
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(marie->stat.last_received_chat_message),
				                       textMessage);
				BC_ASSERT_TRUE(linphone_chat_message_is_forward(marie->stat.last_received_chat_message));
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(marie->stat.last_received_chat_message),
				                       forwarded_from);
			}

			linphone_chat_message_unref(msg);
			ms_free(forwarded_from);
		}
	} else if (reply_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, " %i");
		if (linphone_chat_room_get_history_size(paulineCr) > 0) {
			bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 1);
			LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)(history->data);

			LinphoneChatMessage *msg = linphone_chat_room_create_reply_message(paulineCr, recv_msg);

			BC_ASSERT_TRUE(linphone_chat_message_is_reply(msg));
			const char *replyId = linphone_chat_message_get_reply_message_id(msg);
			BC_ASSERT_STRING_EQUAL(replyId, messageId);

			linphone_chat_message_add_utf8_text_content(msg, "Quite funny!");
			BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(msg),
			                                           linphone_chat_message_get_from_address(recv_msg)));

			LinphoneChatMessage *orig = linphone_chat_message_get_reply_message(msg);
			BC_ASSERT_PTR_NOT_NULL(orig);
			BC_ASSERT_PTR_EQUAL(orig, recv_msg);
			if (orig) {
				const char *reply = linphone_chat_message_get_utf8_text(orig);
				BC_ASSERT_STRING_EQUAL(reply, "Hello");
				linphone_chat_message_unref(orig);
			}

			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_send(msg);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);

			if (marie->stat.last_received_chat_message != NULL) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, " %i");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message),
				                       "Quite funny!");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(marie->stat.last_received_chat_message),
				                       "Quite funny!");
				BC_ASSERT_TRUE(linphone_chat_message_is_reply(marie->stat.last_received_chat_message));
				BC_ASSERT_STRING_EQUAL(
				    linphone_chat_message_get_reply_message_id(marie->stat.last_received_chat_message), messageId);
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(msg),
				                                           linphone_chat_room_get_local_address(marieCr)));
			}

			secondMessageId = bctbx_strdup(linphone_chat_message_get_message_id(msg));
			linphone_chat_message_unref(msg);
		}
	}

	if (with_app_restart) {
		// To simulate dialog removal
		LinphoneAddress *marieAddr = linphone_address_clone(linphone_chat_room_get_peer_address(marieCr));
		linphone_core_set_network_reachable(marie->lc, FALSE);
		coresList = bctbx_list_remove(coresList, marie->lc);
		linphone_core_manager_reinit(marie);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(marie, TRUE);
		marieCr = linphone_core_get_chat_room(marie->lc, marieAddr);
		linphone_address_unref(marieAddr);
	}

	if (forward_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, " %i");
		if (linphone_chat_room_get_history_size(marieCr) > 1) {
			LinphoneChatMessage *recv_msg = linphone_chat_room_get_last_message_in_history(marieCr);
			BC_ASSERT_TRUE(linphone_chat_message_is_forward(recv_msg));

			// for marie, forward message by anonymous
			LinphoneChatMessage *msgFromMarie = linphone_chat_room_create_forward_message(marieCr, recv_msg);
			linphone_chat_message_send(msgFromMarie);

			BC_ASSERT_TRUE(linphone_chat_message_is_forward(msgFromMarie));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(msgFromMarie), "Anonymous");
			linphone_chat_message_unref(msgFromMarie);

			linphone_chat_message_unref(recv_msg);
		}
	} else if (reply_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, " %i");
		if (linphone_chat_room_get_history_size(marieCr) > 1) {
			LinphoneChatMessage *recv_msg = linphone_chat_room_get_last_message_in_history(marieCr);
			const char *body = linphone_chat_message_get_utf8_text(recv_msg);
			BC_ASSERT_STRING_EQUAL(body, "Quite funny!");

			BC_ASSERT_TRUE(linphone_chat_message_is_reply(recv_msg));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_reply_message_id(recv_msg), messageId);
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_message_id(recv_msg), secondMessageId);

			LinphoneChatMessage *orig = linphone_chat_message_get_reply_message(recv_msg);
			BC_ASSERT_PTR_NOT_NULL(orig);
			if (orig) {
				const char *reply = linphone_chat_message_get_utf8_text(orig);
				BC_ASSERT_STRING_EQUAL(reply, "Hello");
				linphone_chat_message_unref(orig);
			}

			LinphoneChatMessage *msgFromMarie = linphone_chat_room_create_reply_message(marieCr, recv_msg);
			BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(recv_msg),
			                                           linphone_chat_message_get_from_address(msgFromMarie)));

			BC_ASSERT_TRUE(linphone_chat_message_is_reply(msgFromMarie));

			linphone_chat_message_add_utf8_text_content(msgFromMarie, "Still laughing!");

			linphone_chat_message_send(msgFromMarie);

			linphone_chat_message_unref(msgFromMarie);
			linphone_chat_message_unref(recv_msg);
		}
	} else {
		// Marie deletes the chat room
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		wait_for_list(coresList, 0, 1, 2000);
		BC_ASSERT_EQUAL(pauline->stat.number_of_chat_room_participants_removed,
		                initialPaulineStats.number_of_chat_room_participants_removed, int, "%d");

		// Pauline sends a new message
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;

		// Pauline sends a new message
		textMessage = "Hey you";
		message = _send_message(paulineCr, textMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered,
		                             initialPaulineStats.number_of_LinphoneMessageDelivered + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
		                             initialMarieStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), textMessage);
		linphone_chat_message_unref(message);

		// Check that the chat room has been correctly recreated on Marie's side
		marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject,
		                                               1, FALSE);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);
	}

	if (messageId) bc_free(messageId);
	if (secondMessageId) bc_free(secondMessageId);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int, "%i");
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(pauline->lc));

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base(bool_t with_app_restart) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(with_app_restart,
	                                                                                             FALSE, FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message(void) {
	group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base(FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_with_app_restart(void) {
	group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base(TRUE);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	// Create a second device for Marie, but it is inactive after registration in this test
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	// Create a second device for Pauline, but again inactivate it after registration
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline2Stats = pauline2->stat;

	linphone_core_set_network_reachable(marie2->lc, FALSE);
	linphone_core_set_network_reachable(pauline2->lc, FALSE);

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
	                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Marie deletes the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_chat_room_participants_removed,
	                initialPaulineStats.number_of_chat_room_participants_removed, int, "%d");

	// Marie sends a new message
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);

	// Check that the chat room has been correctly recreated on Marie's side
	marieCr =
	    check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 1, FALSE);
	if (BC_ASSERT_PTR_NOT_NULL(marieCr)) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);
		textMessage = "Hey you";
		message = _send_message(marieCr, textMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
		                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
		                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
		linphone_chat_message_unref(message);
	}

	// Clean db from chat room
	linphone_core_set_network_reachable(marie2->lc, TRUE);
	linphone_core_set_network_reachable(pauline2->lc, TRUE);

	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats,
	                                                                    confAddr, initialSubject, 1, FALSE);
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr,
	                                                                  initialSubject, 1, FALSE);

	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(marie2, marieCr2, coresList);
	linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialMarie2Stats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPauline2Stats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int, "%i");
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(pauline->lc));

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline2);
}

static void group_chat_room_join_one_to_one_chat_room_with_a_new_device(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	bctbx_list_t *participantsAddresses =
	    bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie1 creates a new one-to-one chat room with Pauline
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marie1Cr =
	    create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject,
	                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie1Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marie1Cr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie1 sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marie1Cr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneMessageDelivered,
	                             initialMarie1Stats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Hey. How are you?";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered,
	                             initialPaulineStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneMessageReceived,
	                             initialMarie1Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie1->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Simulate an uninstall of the application on Marie's side
	linphone_core_set_network_reachable(marie1->lc, FALSE);
	coresManagerList = bctbx_list_remove(coresManagerList, marie1);
	coresList = bctbx_list_remove(coresList, marie1->lc);
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	stats initialMarie2Stats = marie2->stat;
	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, marie2);
	bctbx_list_t *newCoresList = init_core_for_conference(newCoresManagerList);
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Marie2 gets the one-to-one chat room with Pauline
	LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr,
	                                                                  initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie2Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie2 sends a new message
	textMessage = "Fine and you?";
	message = _send_message(marie2Cr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDelivered,
	                             initialMarie2Stats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Perfect!";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered,
	                             initialPaulineStats.number_of_LinphoneMessageDelivered + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                             initialMarie2Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie2->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Clean db from chat room
	int previousNbRegistrationOk = marie1->stat.number_of_LinphoneRegistrationOk;
	linphone_core_set_network_reachable(marie1->lc, TRUE);
	wait_for_until(marie1->lc, NULL, &marie1->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1,
	               2000);
	linphone_core_manager_delete_chat_room(marie2, marie2Cr, coresList);
	linphone_core_delete_chat_room(marie1->lc, marie1Cr);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_new_unique_one_to_one_chat_room_after_both_participants_left(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	;
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *firstConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   firstConfAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Both participants delete the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	wait_for_list(coresList, 0, 1, 3000);

	// Marie re-creates a chat room with Pauline
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *secondConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room has been correctly recreated on Marie's side
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, secondConfAddr,
	                                                 initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	/* firstconfAddr and secondConfAddr must differ */
	BC_ASSERT_FALSE(linphone_address_equal(firstConfAddr, secondConfAddr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(firstConfAddr);
	linphone_address_unref(secondConfAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void imdn_for_group_chat_room(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	LinphoneChatRoom *marieCr = NULL, *marie2Cr = NULL, *paulineCr = NULL, *chloeCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	stats initialMarie2Stats = marie2->stat;
	time_t initialTime = ms_time(NULL);

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie2->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	marie2Cr =
	    check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie2Cr)) goto end;

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                             initialMarie2Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageSent, 1, 1000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg)) goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed));
	bctbx_list_t *participantsThatReceivedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 2, int, "%d");
		for (bctbx_list_t *item = participantsThatReceivedChloeMessage; item; item = bctbx_list_next(item)) {
			LinphoneParticipantImdnState *state = (LinphoneParticipantImdnState *)bctbx_list_get_data(item);
			BC_ASSERT_GREATER((int)linphone_participant_imdn_state_get_state_change_time(state), (int)initialTime, int,
			                  "%d");
			BC_ASSERT_EQUAL(linphone_participant_imdn_state_get_state(state), LinphoneChatMessageStateDeliveredToUser,
			                int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_participant_imdn_state_get_participant(state));
		}
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	// Marie marks the message as read, check that the state is not yet displayed on Chloe's side
	linphone_chat_room_mark_as_read(marieCr);
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
	                              initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 0, 1000));
	bctbx_list_t *participantsThatDisplayedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	participantsThatReceivedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	// Marie2 should have received marie's Display IMDN, causing it's own chatroom to be marked as read as well
	// automatically
	BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(marie2Cr), 0, int, "%d");

	// Pauline also marks the message as read, check that the state is now displayed on Chloe's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
	                             initialChloeStats.number_of_LinphoneMessageDisplayed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 0, 1000));
	participantsThatDisplayedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 2, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	linphone_chat_message_unref(chloeMessage);

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	if (marie2Cr) linphone_core_manager_delete_chat_room(marie2, marie2Cr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

void group_chat_with_imdn_sent_only_to_sender_base(bool_t add_participant,
                                                   bool_t enable_lime,
                                                   bool_t participant_goes_offline) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *michelle = linphone_core_manager_create("michelle_rc");
	LinphoneCoreManager *berthe = linphone_core_manager_create("berthe_rc");
	LinphoneChatRoom *marieCr = NULL, *laureCr = NULL, *paulineCr = NULL, *chloeCr = NULL, *michelleCr = NULL,
	                 *bertheCr = NULL, *marie2Cr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, michelle);
	coresManagerList = bctbx_list_append(coresManagerList, berthe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	if (enable_lime) {
		set_lime_server_and_curve_list(C25519, coresManagerList);
	}
	start_core_for_conference(coresManagerList);
	int imdn_to_everybody_threshold = (!!add_participant) ? 5 : 4;
	for (const bctbx_list_t *coreManagerIt = coresManagerList; coreManagerIt != NULL;
	     coreManagerIt = bctbx_list_next(coreManagerIt)) {
		// For testing purposes, show empty chatroom as one-to-one chatroom with only IMDN messages are considered as
		// empty. This will allow the test to verify their existence
		LinphoneCoreManager *coreManager = (LinphoneCoreManager *)bctbx_list_get_data(coreManagerIt);
		linphone_config_set_int(linphone_core_get_config(coreManager->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_core_set_imdn_to_everybody_threshold(coreManager->lc, imdn_to_everybody_threshold);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(coreManager->lc));
		if (enable_lime) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &coreManager->stat.number_of_X3dhUserCreationSuccess, 1,
			                             x3dhServer_creationTimeout));
		}
	}
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(michelle->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	stats initialLaureStats = laure->stat;
	stats initialMichelleStats = michelle->stat;
	stats initialBertheStats = berthe->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       enable_lime, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 4, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 4, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 4, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	michelleCr = check_creation_chat_room_client_side(coresList, michelle, &initialMichelleStats, confAddr,
	                                                  initialSubject, 4, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(michelleCr)) goto end;

	marie2Cr =
	    check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 4, TRUE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie2Cr)) goto end;

	if (participant_goes_offline) {
		linphone_core_set_network_reachable(pauline->lc, FALSE);
	}

	// Chloe begins composing a message
	const char *marieTextMessage = "My first message";
	LinphoneChatMessage *marieMessage = _send_message(marieCr, marieTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
	                             initialChloeStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
	                             initialLaureStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                             initialMarie2Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	if (!!!participant_goes_offline) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
		                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageReceived,
	                             initialMichelleStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 1000));

	if (!!!participant_goes_offline) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialMarie2Stats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
	}
	if (!!!participant_goes_offline && !!add_participant) {
		// The threshold is expected to be 5 therefore everybody receives the IMDN as there are only 4 participants
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialLaureStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialMichelleStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialPaulineStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
	} else {
		// The threshold is expected to be 4 therefore only the sender receives the IMDM
		BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
		                              initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDeliveredToUser,
		                              initialLaureStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDeliveredToUser,
		                              initialMichelleStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser,
		                              initialPaulineStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
	}

	if (participant_goes_offline) {
		linphone_core_set_network_reachable(pauline->lc, TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
		                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialMarie2Stats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             liblinphone_tester_sip_timeout));

		if (!!add_participant) {
			// The threshold is expected to be 5 therefore everybody receives the IMDN as there are only 4 participants
			BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
			                             initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDeliveredToUser,
			                             initialLaureStats.number_of_LinphoneMessageDeliveredToUser + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDeliveredToUser,
			                             initialMichelleStats.number_of_LinphoneMessageDeliveredToUser + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser,
			                             initialPaulineStats.number_of_LinphoneMessageDeliveredToUser + 1,
			                             liblinphone_tester_sip_timeout));
		}
	}

	LinphoneChatMessageState expected_participant_state =
	    !!add_participant ? LinphoneChatMessageStateDeliveredToUser : LinphoneChatMessageStateDelivered;
	for (const bctbx_list_t *coreIt = coresList; coreIt != NULL; coreIt = bctbx_list_next(coreIt)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(coreIt);
		LinphoneChatRoom *cr = linphone_core_search_chat_room(core, NULL, NULL, confAddr, NULL);
		if (core == berthe->lc) {
			BC_ASSERT_PTR_NULL(cr);
		} else {
			BC_ASSERT_PTR_NOT_NULL(cr);
		}
		size_t coreCrNo = bctbx_list_size(linphone_core_get_chat_rooms(core));
		size_t expectedCrNo = (!!add_participant) ? 1 : 2;
		if ((core == marie->lc) || (core == marie2->lc)) {
			expectedCrNo = (!!add_participant) ? 1 : 5;
		} else if (core == berthe->lc) {
			expectedCrNo = 0;
		}
		BC_ASSERT_EQUAL(coreCrNo, expectedCrNo, size_t, "%0zu");
		if (cr) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), marieTextMessage);
				LinphoneChatMessageState expected_state = expected_participant_state;
				if ((core == marie->lc) || (core == marie2->lc)) {
					expected_state = LinphoneChatMessageStateDeliveredToUser;
				}
				BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_state, int, "%0d");
				linphone_chat_message_unref(msg);
			}
		}
	}

	if (!!add_participant) {
		char *conference_addr_str = linphone_address_as_string(confAddr);
		ms_message("%s adds %s to chat room %s", linphone_core_get_identity(marie->lc),
		           linphone_core_get_identity(berthe->lc), conference_addr_str);
		bctbx_free(conference_addr_str);
		linphone_chat_room_add_participant(marieCr, berthe->identity);
		bertheCr = check_creation_chat_room_client_side(coresList, berthe, &initialBertheStats, confAddr,
		                                                initialSubject, 5, FALSE);
		if (!BC_ASSERT_PTR_NOT_NULL(bertheCr)) goto end;

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
		                             initialMarieStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_chat_room_participants_added,
		                             initialMarie2Stats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
		                             initialPaulineStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_added,
		                             initialLaureStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle->stat.number_of_chat_room_participants_added,
		                             initialMichelleStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_chat_room_participants_added,
		                             initialChloeStats.number_of_chat_room_participants_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe->stat.number_of_LinphoneChatRoomStateCreationPending,
		                             initialBertheStats.number_of_LinphoneChatRoomStateCreationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe->stat.number_of_LinphoneChatRoomStateCreated,
		                             initialBertheStats.number_of_LinphoneChatRoomStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe->stat.number_of_LinphoneChatRoomConferenceJoined,
		                             initialBertheStats.number_of_LinphoneChatRoomConferenceJoined + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie2Cr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(bertheCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chloeCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 5, int, "%d");
	}

	linphone_chat_room_mark_as_read(laureCr);
	linphone_chat_room_mark_as_read(michelleCr);
	linphone_chat_room_mark_as_read(paulineCr);
	linphone_chat_room_mark_as_read(chloeCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarie2Stats.number_of_LinphoneMessageDisplayed + 1,
	                             liblinphone_tester_sip_timeout));
	if (!!add_participant) {
		// The threshold is expected to be 5 therefore everybody receives the IMDN as there are only 4 participants that
		// received the message
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
		                             initialChloeStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDisplayed,
		                             initialLaureStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDisplayed,
		                             initialMichelleStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDisplayed,
		                             initialPaulineStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
	} else {
		// The threshold is expected to be 4 therefore only the sender receives the IMDM
		BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
		                              initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDisplayed,
		                              initialLaureStats.number_of_LinphoneMessageDisplayed + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDisplayed,
		                              initialMichelleStats.number_of_LinphoneMessageDisplayed + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDisplayed,
		                              initialPaulineStats.number_of_LinphoneMessageDisplayed + 1, 1000));
	}

	expected_participant_state =
	    !!add_participant ? LinphoneChatMessageStateDisplayed : LinphoneChatMessageStateDelivered;
	for (const bctbx_list_t *coreIt = coresList; coreIt != NULL; coreIt = bctbx_list_next(coreIt)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(coreIt);
		LinphoneChatRoom *cr = linphone_core_search_chat_room(core, NULL, NULL, confAddr, NULL);
		if (!!!add_participant && (core == berthe->lc)) {
			BC_ASSERT_PTR_NULL(cr);
		} else {
			BC_ASSERT_PTR_NOT_NULL(cr);
		}
		size_t coreCrNo = bctbx_list_size(linphone_core_get_chat_rooms(core));
		size_t expectedCrNo = (!!add_participant) ? 1 : 2;
		if ((core == marie->lc) || (core == marie2->lc)) {
			expectedCrNo = (!!add_participant) ? 1 : 5;
		} else if (core == berthe->lc) {
			expectedCrNo = (!!add_participant) ? 1 : 0;
		}
		BC_ASSERT_EQUAL(coreCrNo, expectedCrNo, size_t, "%0zu");
		if (cr) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
			if (core == berthe->lc) {
				BC_ASSERT_PTR_NULL(msg);
			} else {
				BC_ASSERT_PTR_NOT_NULL(msg);
			}
			if (msg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), marieTextMessage);
				LinphoneChatMessageState expected_state = ((core == marie->lc) || (core == marie2->lc))
				                                              ? LinphoneChatMessageStateDisplayed
				                                              : expected_participant_state;
				BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_state, int, "%0d");
				linphone_chat_message_unref(msg);
			}
		}
	}
	linphone_chat_message_unref(marieMessage);

	initialMarieStats = marie->stat;
	initialMarie2Stats = marie2->stat;
	initialPaulineStats = pauline->stat;
	initialChloeStats = chloe->stat;
	initialLaureStats = laure->stat;
	initialMichelleStats = michelle->stat;
	initialBertheStats = berthe->stat;

	const char *marieTextMessage2 = "My second message";
	LinphoneChatMessage *marieMessage2 = _send_message(marieCr, marieTextMessage2);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
	                             initialChloeStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
	                             initialLaureStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                             initialMarie2Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageReceived,
	                             initialMichelleStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 1000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialMarie2Stats.number_of_LinphoneMessageDeliveredToUser + 1,
	                             liblinphone_tester_sip_timeout));

	// The chat room has gone over the threshold therefore no one receives the IMDN
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
	                              initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDeliveredToUser,
	                              initialLaureStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDeliveredToUser,
	                              initialMichelleStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser,
	                              initialPaulineStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));

	if (!!add_participant) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe->stat.number_of_LinphoneMessageReceived,
		                             initialBertheStats.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &berthe->stat.number_of_LinphoneMessageDeliveredToUser,
		                              initialBertheStats.number_of_LinphoneMessageDeliveredToUser + 1, 1000));
	}

	for (const bctbx_list_t *coreIt = coresList; coreIt != NULL; coreIt = bctbx_list_next(coreIt)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(coreIt);
		LinphoneChatRoom *cr = linphone_core_search_chat_room(core, NULL, NULL, confAddr, NULL);
		if (!!!add_participant && (core == berthe->lc)) {
			BC_ASSERT_PTR_NULL(cr);
		} else {
			BC_ASSERT_PTR_NOT_NULL(cr);
		}
		size_t coreCrNo = bctbx_list_size(linphone_core_get_chat_rooms(core));
		size_t expectedCrNo = 2;
		if ((core == marie->lc) || (core == marie2->lc)) {
			expectedCrNo = (!!add_participant) ? 6 : 5;
		} else if (core == berthe->lc) {
			expectedCrNo = (!!add_participant) ? 2 : 0;
		}
		BC_ASSERT_EQUAL(coreCrNo, expectedCrNo, size_t, "%0zu");
		if (cr) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), marieTextMessage2);
				LinphoneChatMessageState expected_state = LinphoneChatMessageStateDelivered;
				if ((core == marie->lc) || (core == marie2->lc)) {
					expected_state = LinphoneChatMessageStateDeliveredToUser;
				}
				BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_state, int, "%0d");
				linphone_chat_message_unref(msg);
			}
		}
	}

	linphone_chat_room_mark_as_read(laureCr);
	linphone_chat_room_mark_as_read(michelleCr);
	linphone_chat_room_mark_as_read(paulineCr);
	linphone_chat_room_mark_as_read(chloeCr);

	if (!!add_participant) {
		linphone_chat_room_mark_as_read(bertheCr);
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDisplayed,
	                             initialMarie2Stats.number_of_LinphoneMessageDisplayed + 1,
	                             liblinphone_tester_sip_timeout));

	// The chat room has gone over the threshold therefore no one receives the IMDN
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
	                              initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageDisplayed,
	                              initialLaureStats.number_of_LinphoneMessageDisplayed + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &michelle->stat.number_of_LinphoneMessageDisplayed,
	                              initialMichelleStats.number_of_LinphoneMessageDisplayed + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDisplayed,
	                              initialPaulineStats.number_of_LinphoneMessageDisplayed + 1, 1000));

	if (!!add_participant) {
		BC_ASSERT_FALSE(wait_for_list(coresList, &berthe->stat.number_of_LinphoneMessageDisplayed,
		                              initialBertheStats.number_of_LinphoneMessageDisplayed + 1, 1000));
	}

	for (const bctbx_list_t *coreIt = coresList; coreIt != NULL; coreIt = bctbx_list_next(coreIt)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(coreIt);
		LinphoneChatRoom *cr = linphone_core_search_chat_room(core, NULL, NULL, confAddr, NULL);
		if (!!!add_participant && (core == berthe->lc)) {
			BC_ASSERT_PTR_NULL(cr);
		} else {
			BC_ASSERT_PTR_NOT_NULL(cr);
		}
		size_t coreCrNo = bctbx_list_size(linphone_core_get_chat_rooms(core));
		size_t expectedCrNo = 2;
		if ((core == marie->lc) || (core == marie2->lc)) {
			expectedCrNo = (!!add_participant) ? 6 : 5;
		} else if (core == berthe->lc) {
			expectedCrNo = (!!add_participant) ? 2 : 0;
		}
		BC_ASSERT_EQUAL(coreCrNo, expectedCrNo, size_t, "%0zu");
		if (cr) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), marieTextMessage2);
				LinphoneChatMessageState expected_state = LinphoneChatMessageStateDelivered;
				if ((core == marie->lc) || (core == marie2->lc)) {
					expected_state = LinphoneChatMessageStateDisplayed;
				}
				BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_state, int, "%0d");
				linphone_chat_message_unref(msg);
			}
		}
		// Verify that chatrooms where IMDNs are sent are hidden
		linphone_config_set_int(linphone_core_get_config(core), "misc", "hide_empty_chat_rooms", 1);

		size_t prunedCoreCrNo = bctbx_list_size(linphone_core_get_chat_rooms(core));
		BC_ASSERT_EQUAL(prunedCoreCrNo, ((!!!add_participant) && (core == berthe->lc) ? 0 : 1), size_t, "%0zu");
	}
	linphone_chat_message_unref(marieMessage2);

end:

	for (const bctbx_list_t *coreIt = coresList; coreIt != NULL; coreIt = bctbx_list_next(coreIt)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(coreIt);

		// Show empty chat rooms in order to properly delete them
		linphone_config_set_int(linphone_core_get_config(core), "misc", "hide_empty_chat_rooms", 0);

		LinphoneCoreManager *mgr = get_manager(core);
		const bctbx_list_t *chatRooms = linphone_core_get_chat_rooms(core);
		for (const bctbx_list_t *chatRoomIt = chatRooms; chatRoomIt != NULL; chatRoomIt = bctbx_list_next(chatRoomIt)) {
			LinphoneChatRoom *chatRoom = (LinphoneChatRoom *)bctbx_list_get_data(chatRoomIt);
			const LinphoneAddress *chatroom_address = linphone_chat_room_get_conference_address(chatRoom);
			char *chatroom_address_string = linphone_address_as_string(chatroom_address);
			ms_message("Checking chatroom %s of manager %s", chatroom_address_string,
			           linphone_core_get_identity(mgr->lc));
			ms_free(chatroom_address_string);
			LinphoneChatRoomSecurityLevel security_level = LinphoneChatRoomSecurityLevelClearText;
			LinphoneChatRoomBackend backend = LinphoneChatRoomBackendFlexisipChat;
			LinphoneChatRoomEncryptionBackend encryption_backend = LinphoneChatRoomEncryptionBackendNone;
			if (!!enable_lime) {
				encryption_backend = LinphoneChatRoomEncryptionBackendLime;
				security_level = LinphoneChatRoomSecurityLevelEncrypted;
			}
			BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chatRoom), security_level,
			                LinphoneChatRoomSecurityLevel, "%i");
			const LinphoneChatRoomParams *params = linphone_chat_room_get_current_params(chatRoom);
			BC_ASSERT_PTR_NOT_NULL(params);
			if (params) {
				BC_ASSERT_EQUAL(linphone_chat_room_params_get_backend(params), backend, LinphoneChatRoomBackend, "%i");
				BC_ASSERT_EQUAL(!!linphone_chat_room_params_encryption_enabled(params), !!enable_lime, int, "%0d");
				BC_ASSERT_EQUAL(linphone_chat_room_params_get_encryption_backend(params), encryption_backend,
				                LinphoneChatRoomEncryptionBackend, "%i");
			}
			linphone_core_manager_delete_chat_room(mgr, chatRoom, coresList);
		}
	}

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
	linphone_core_manager_destroy(michelle);
	linphone_core_manager_destroy(berthe);
}

static void group_chat_with_imdn_sent_only_to_sender(void) {
	group_chat_with_imdn_sent_only_to_sender_base(FALSE, FALSE, FALSE);
}

static void group_chat_with_imdn_sent_only_to_sender_with_participant_offline(void) {
	group_chat_with_imdn_sent_only_to_sender_base(FALSE, FALSE, TRUE);
}

static void group_chat_with_imdn_sent_only_to_sender_after_going_over_threshold(void) {
	group_chat_with_imdn_sent_only_to_sender_base(TRUE, FALSE, FALSE);
}

static void imdn_updated_for_group_chat_room_with_one_participant_offline(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *chloeCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// pauline offline
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                              initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                              liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageSent, 1, 1000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg)) goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message has been delivered to Marie
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDelivered,
	                             initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1,
	                             liblinphone_tester_sip_timeout));
	bctbx_list_t *participantsThatReceivedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	// Check that the message has been sent to Pauline
	bctbx_list_t *participantsThatSentChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatSentChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatSentChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatSentChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));
	// Marie marks the message as read
	linphone_chat_room_mark_as_read(marieCr);
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
	                              initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 0, 1000));
	// Check that the message has beed displayed by Marie
	bctbx_list_t *participantsThatDisplayedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	// Check that the message has been sent to Pauline
	participantsThatSentChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatSentChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatSentChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatSentChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	// Check that the message state is Delivered
	BC_ASSERT_TRUE(linphone_chat_message_get_state(chloeMessage) == LinphoneChatMessageStateDelivered);

	// set appData will trigger update db
	linphone_chat_message_set_appdata(chloeMessage, "");
	// Check that the message has beed displayed by Marie
	participantsThatDisplayedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	// Check that the message has been sent to Pauline
	participantsThatSentChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatSentChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatSentChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatSentChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	// Check that the message state is Delivered
	BC_ASSERT_TRUE(linphone_chat_message_get_state(chloeMessage) == LinphoneChatMessageStateDelivered);

	linphone_chat_message_unref(chloeMessage);

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void aggregated_imdn_for_group_chat_room_base(bool_t read_while_offline) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneCoreManager *chloe2 = linphone_core_manager_create("chloe_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *chloeCr = NULL, *chloe2Cr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	coresManagerList = bctbx_list_append(coresManagerList, chloe2);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	stats initialChloe2Stats = chloe2->stat;
	bctbx_list_t *chloeMessages = NULL, *chloe2Messages = NULL, *chloe2DisplayedMessages = NULL;
	stats chloeMessagesStats = {0}, chloe2MessagesStats = {0}, chloe2DisplayedMessagesStats = {0};
	int messageCount = 0;

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe2->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;
	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;
	// Check that the chat room is correctly created on Chloe2's side and that the participants are added
	chloe2Cr = check_creation_chat_room_client_side(coresList, chloe2, &initialChloe2Stats, confAddr, initialSubject, 2,
	                                                FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloe2Cr)) goto end;

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	const char *chloeTextMessage2 = "Long time no talk";
	const char *chloeTextMessage3 = "How are you?";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	LinphoneChatMessage *chloeMessage2 = _send_message(chloeCr, chloeTextMessage2);
	LinphoneChatMessage *chloeMessage3 = _send_message(chloeCr, chloeTextMessage3);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageReceived + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe2->stat.number_of_LinphoneMessageReceived,
	                             initialChloe2Stats.number_of_LinphoneMessageReceived + 3,
	                             liblinphone_tester_sip_timeout));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg)) goto end;
	LinphoneChatMessage *chloe2LastMsg = chloe2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(chloe2LastMsg)) goto end;

	BC_ASSERT_TRUE(linphone_chat_message_get_state(marieLastMsg) == LinphoneChatMessageStateDelivered);
	BC_ASSERT_TRUE(linphone_chat_message_get_state(chloe2LastMsg) == LinphoneChatMessageStateDelivered);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage3);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(chloe2LastMsg), chloeTextMessage3);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Mark as read on Chloe and Chloe2 should not impact the message status
	linphone_chat_room_mark_as_read(chloeCr);
	linphone_chat_room_mark_as_read(chloe2Cr);
	// Mark the messages as read on Marie's sides
	linphone_chat_room_mark_as_read(marieCr);
	wait_for_list(coresList, 0, 1, 2000);

	// Get Chloe2 messages and use them to set callbacks. Check callback status and messages status
	chloe2Messages = liblinphone_tester_get_messages_and_states(
	    chloe2Cr, &messageCount, &chloe2MessagesStats); // Do not unref chloe2Messages to keep callbacks
	if (!BC_ASSERT_PTR_NOT_NULL(chloe2Messages)) goto end;
	for (bctbx_list_t *elem = chloe2Messages; elem != NULL; elem = bctbx_list_next(elem))
		linphone_chat_message_cbs_set_msg_state_changed(
		    linphone_chat_message_get_callbacks((LinphoneChatMessage *)elem->data),
		    liblinphone_tester_chat_message_msg_state_changed);

	// All participants didn't read messages yet (only Marie) : number_of_LinphoneMessageDisplayed == displayedCount
	BC_ASSERT_EQUAL(chloe->stat.number_of_LinphoneMessageDisplayed,
	                initialChloeStats.number_of_LinphoneMessageDisplayed, int, "%d");
	BC_ASSERT_EQUAL(chloe2->stat.number_of_LinphoneMessageDisplayed,
	                initialChloe2Stats.number_of_LinphoneMessageDisplayed, int, "%d");
	BC_ASSERT_EQUAL(chloe2MessagesStats.number_of_LinphoneMessageDisplayed, 0, int, "%d");
	BC_ASSERT_EQUAL(messageCount, 3, int, "%d");

	// Mark the messages as read on Pauline's sides
	if (read_while_offline) {
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		linphone_chat_room_mark_as_read(paulineCr);
		wait_for_list(coresList, 0, 1, 2000);
		linphone_core_set_network_reachable(pauline->lc, TRUE);
	} else {
		linphone_chat_room_mark_as_read(paulineCr);
	}

	// Messages have been read
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed,
	                             initialChloeStats.number_of_LinphoneMessageDisplayed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe2->stat.number_of_LinphoneMessageDisplayed,
	                             initialChloe2Stats.number_of_LinphoneMessageDisplayed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(chloe->stat.number_of_LinphoneMessageDeliveredToUser, 3, int, "%d"); // 3 for sending to chloe2

	if (read_while_offline) {
		wait_for_list(coresList, 0, 1, 2000); // To prevent memory leak
	}
	wait_for_list(coresList, 0, 1, 2000);

	// Test internal data on Chloe2 and check consistency between callback and messages
	chloe2DisplayedMessages =
	    liblinphone_tester_get_messages_and_states(chloe2Cr, &messageCount, &chloe2DisplayedMessagesStats);
	BC_ASSERT_EQUAL(chloe2DisplayedMessagesStats.number_of_LinphoneMessageDisplayed, 3, int,
	                "%d"); // All messages have been read
	BC_ASSERT_EQUAL(messageCount, 3, int, "%d");
	// Test internal data on Chloe
	chloeMessages = liblinphone_tester_get_messages_and_states(chloeCr, &messageCount, &chloeMessagesStats);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeMessages)) goto end;
	BC_ASSERT_EQUAL(chloeMessagesStats.number_of_LinphoneMessageDisplayed, 3, int, "%d"); // All messages have been read
	BC_ASSERT_EQUAL(messageCount, 3, int, "%d");

	linphone_chat_message_unref(chloeMessage3);
	linphone_chat_message_unref(chloeMessage2);
	linphone_chat_message_unref(chloeMessage);

end:

	if (chloeMessages) bctbx_list_free_with_data(chloeMessages, (bctbx_list_free_func)linphone_chat_message_unref);
	if (chloe2Messages) bctbx_list_free_with_data(chloe2Messages, (bctbx_list_free_func)linphone_chat_message_unref);
	if (chloe2DisplayedMessages)
		bctbx_list_free_with_data(chloe2DisplayedMessages, (bctbx_list_free_func)linphone_chat_message_unref);

	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	if (chloe2Cr) linphone_core_manager_delete_chat_room(chloe2, chloe2Cr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
	linphone_core_manager_destroy(chloe2);
}

static void aggregated_imdn_for_group_chat_room(void) {
	aggregated_imdn_for_group_chat_room_base(FALSE);
}

static void aggregated_imdn_for_group_chat_room_read_while_offline(void) {
	aggregated_imdn_for_group_chat_room_base(TRUE);
}

static void imdn_sent_from_db_state(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *chloeCr = NULL;
	LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	time_t initialTime = ms_time(NULL);

	// Enable IMDN except for Marie
	linphone_im_notif_policy_clear(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;
	confAddr = linphone_address_clone(confAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
	                             initialMarieStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg)) goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message is not globally marked as delivered to user since Marie do not notify its delivery
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
	                              initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));

	// Restart Marie's core with IMDN enabled so that delivery notification is sent when chat room is loaded from DB
	coresList = bctbx_list_remove(coresList, marie->lc);
	linphone_core_manager_reinit(marie);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_core_manager_start(marie, TRUE);
	char *marieIdentity = linphone_core_get_device_identity(marie->lc);
	LinphoneAddress *marieAddr = linphone_address_new(marieIdentity);
	bctbx_free(marieIdentity);
	marieCr = linphone_core_find_chat_room(marie->lc, confAddr, marieAddr);
	linphone_address_unref(marieAddr);

	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed));
	bctbx_list_t *participantsThatReceivedChloeMessage =
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 2, int, "%d");
		for (bctbx_list_t *item = participantsThatReceivedChloeMessage; item; item = bctbx_list_next(item)) {
			LinphoneParticipantImdnState *state = (LinphoneParticipantImdnState *)bctbx_list_get_data(item);
			BC_ASSERT_GREATER((int)linphone_participant_imdn_state_get_state_change_time(state), (int)initialTime, int,
			                  "%d");
			BC_ASSERT_EQUAL(linphone_participant_imdn_state_get_state(state), LinphoneChatMessageStateDeliveredToUser,
			                int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_participant_imdn_state_get_participant(state));
		}
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage,
		                          (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(
	    linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	linphone_chat_message_unref(chloeMessage);

end:
	if (confAddr) linphone_address_unref(confAddr);

	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void basic_chat_room_with_cpim_base(bool_t use_gruu, bool_t enable_imdn) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	if (!use_gruu) {
		linphone_core_remove_supported_tag(marie->lc, "gruu");
	}
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	if (enable_imdn) {
		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	}

	linphone_core_set_chat_messages_aggregation_enabled(marie->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "chat_messages_aggregation_delay", 10);

	// Enable CPIM
	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	const LinphoneAccountParams *marie_account_params = linphone_account_get_params(marie_account);
	LinphoneAccountParams *cloned_marie_account_params = linphone_account_params_clone(marie_account_params);
	linphone_account_params_enable_cpim_in_basic_chat_room(cloned_marie_account_params, TRUE);
	linphone_account_set_params(marie_account, cloned_marie_account_params);
	linphone_account_params_unref(cloned_marie_account_params);

	LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);
	const LinphoneAccountParams *pauline_account_params = linphone_account_get_params(pauline_account);
	LinphoneAccountParams *cloned_pauline_account_params = linphone_account_params_clone(pauline_account_params);
	linphone_account_params_enable_cpim_in_basic_chat_room(cloned_pauline_account_params, TRUE);
	linphone_account_set_params(pauline_account, cloned_pauline_account_params);
	linphone_account_params_unref(cloned_pauline_account_params);

	LinphoneAddress *pauline_contact_address = linphone_account_get_contact_address(pauline_account);

	bctbx_list_t *participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, pauline_contact_address);
	LinphoneConferenceParams *marie_params = linphone_core_create_conference_params(marie->lc);
	linphone_conference_params_enable_chat(marie_params, TRUE);
	linphone_conference_params_enable_group(marie_params, FALSE);
	LinphoneChatParams *marie_chat_params = linphone_conference_params_get_chat_params(marie_params);
	linphone_chat_params_set_backend(marie_chat_params, LinphoneChatRoomBackendBasic);
	LinphoneChatRoom *marieCr = linphone_core_create_chat_room_7(marie->lc, marie_params, participantsAddresses);
	bctbx_list_free(participantsAddresses);
	participantsAddresses = NULL;
	linphone_chat_room_params_unref(marie_params);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 1,
	                             liblinphone_tester_sip_timeout));

	// Marie sends a message in a basic chatroom
	const char *marie_text = "This is a basic chat room";
	LinphoneChatMessage *marie_msg = NULL;
	if (marieCr) {
		const LinphoneAddress *peer_address = linphone_chat_room_get_peer_address(marieCr);
		BC_ASSERT_TRUE(linphone_address_has_uri_param(peer_address, "gr"));
		linphone_chat_room_compose(marieCr);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, 1));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 2000));
		marie_msg = _send_message(marieCr, marie_text);
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1,
		                             liblinphone_tester_sip_timeout));
		if (enable_imdn) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1,
			                             liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, 1,
			                             liblinphone_tester_sip_timeout));
		}
	}

	LinphoneConferenceParams *pauline_params = linphone_core_create_conference_params(pauline->lc);
	linphone_conference_params_enable_chat(pauline_params, TRUE);
	linphone_conference_params_enable_group(pauline_params, FALSE);
	LinphoneChatParams *pauline_chat_params = linphone_conference_params_get_chat_params(pauline_params);
	linphone_chat_params_set_backend(pauline_chat_params, LinphoneChatRoomBackendBasic);
	const LinphoneAddress *pauline_identity_address =
	    linphone_account_params_get_identity_address(linphone_account_get_params(pauline_account));
	LinphoneChatRoom *paulineCr =
	    linphone_core_search_chat_room_2(pauline->lc, pauline_params, pauline_identity_address, NULL, NULL);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	linphone_chat_room_params_unref(pauline_params);

	const char *pauline_text = "Yes Madam";
	LinphoneChatMessage *pauline_msg = NULL;
	if (paulineCr) {
		linphone_chat_room_compose(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, 2, 2000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, 2000));
		if (enable_imdn) {
			linphone_chat_room_mark_as_read(paulineCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDisplayed, 1,
			                             liblinphone_tester_sip_timeout));
		}
		if (marie_msg) {
			linphone_chat_message_unref(marie_msg);
		}
		LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
		if (BC_ASSERT_PTR_NOT_NULL(msg)) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), marie_text);
			linphone_chat_message_unref(msg);
		}
		const LinphoneAddress *local_address = linphone_chat_room_get_local_address(paulineCr);
		BC_ASSERT_TRUE(linphone_address_has_uri_param(local_address, "gr"));
		pauline_msg = _send_message(paulineCr, pauline_text);
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2, 2000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneAggregatedMessagesReceived, 1,
		                             liblinphone_tester_sip_timeout));
		if (enable_imdn) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1,
			                             liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, 1,
			                             liblinphone_tester_sip_timeout));
		}
	}

	LinphoneChatMessageState expected_msg_state =
	    (enable_imdn) ? LinphoneChatMessageStateDisplayed : LinphoneChatMessageStateDelivered;
	if (marieCr) {
		if (enable_imdn) {
			linphone_chat_room_mark_as_read(marieCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDisplayed, 2,
			                             liblinphone_tester_sip_timeout));
			// As Marie enables chat message aggregation, the message state changed is not called
			BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, 2, 1000));
		}
		if (pauline_msg) {
			linphone_chat_message_unref(pauline_msg);
		}

		LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(marieCr);
		if (BC_ASSERT_PTR_NOT_NULL(msg)) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), pauline_text);
			linphone_chat_message_unref(msg);
		}
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");
		bctbx_list_t *marie_history = linphone_chat_room_get_history(marieCr, 3);
		for (bctbx_list_t *item = marie_history; item; item = bctbx_list_next(item)) {
			LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_msg_state, int, "%d");
		}
		bctbx_list_free_with_data(marie_history, (bctbx_list_free_func)linphone_chat_message_unref);
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		linphone_chat_room_unref(marieCr);
	}
	if (paulineCr) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");
		bctbx_list_t *pauline_history = linphone_chat_room_get_history(paulineCr, 3);
		for (bctbx_list_t *item = pauline_history; item; item = bctbx_list_next(item)) {
			LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_msg_state, int, "%d");
		}
		bctbx_list_free_with_data(pauline_history, (bctbx_list_free_func)linphone_chat_message_unref);
		linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	}

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void basic_chat_room_with_cpim_gruu(void) {
	basic_chat_room_with_cpim_base(TRUE, FALSE);
}

static void basic_chat_room_with_cpim_gruu_imdn(void) {
	basic_chat_room_with_cpim_base(TRUE, TRUE);
}

static void basic_chat_room_with_cpim_without_gruu(void) {
	basic_chat_room_with_cpim_base(FALSE, FALSE);
}

static void find_one_to_one_chat_room(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *chloeCr = NULL, *marieOneToOneCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	// Only to be used in linphone_core_find_one_to_one_chatroom(...);
	const LinphoneAddress *marieAddr =
	    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc));
	const LinphoneAddress *paulineAddr =
	    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc));
	LinphoneChatMessage *basicMessage = NULL;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// Chloe leave the chat room
	linphone_chat_room_leave(chloeCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminationPending,
	                             initialChloeStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialChloeStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	LinphoneChatRoom *oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NULL(oneToOneChatRoom);

	// Marie create a one to one chat room with Pauline
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses,
	                                               "one to one", FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieOneToOneCr);
	LinphoneChatRoom *paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                           confAddr, "one to one", 1, FALSE);

	// Marie creates a basic chat room with Pauline
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);
	linphone_chat_room_params_enable_group(params, FALSE);
	LinphoneChatRoom *basicCR = linphone_core_create_chat_room_6(marie->lc, params, marieAddr, participantsAddresses);
	linphone_chat_room_params_unref(params);

	// Check it returns the One-To-One chat room (flexisip based)
	oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, marieOneToOneCr);

	// Check it returns the Basic chat room
	oneToOneChatRoom = linphone_core_search_chat_room(marie->lc, NULL, marieAddr, paulineAddr, NULL);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, basicCR);

	// Check it returns the Basic chat room
	params = linphone_core_create_default_chat_room_params(marie->lc);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);
	oneToOneChatRoom = linphone_core_search_chat_room(marie->lc, params, marieAddr, NULL, participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, basicCR);

	// Check it returns the One-To-One chat room (flexisip based)
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	oneToOneChatRoom = linphone_core_search_chat_room(marie->lc, params, marieAddr, NULL, participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, marieOneToOneCr);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	linphone_chat_room_params_unref(params);

	// Clean the db
	linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);
	marieOneToOneCr = NULL;
	paulineOneToOneCr = NULL;

	// Check cleaning went well
	oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, basicCR);
	if (basicCR) linphone_chat_room_unref(basicCR);

end:
	// Clean db from chat room
	if (basicMessage) linphone_chat_message_unref(basicMessage);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void exhume_one_to_one_chat_room_1(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses,
	                                               "one to one", FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);

	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr,
	                                                         "one to one", 1, FALSE);

	LinphoneChatMessage *message =
	    linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "Do. Or do not. There is no try.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, liblinphone_tester_sip_timeout));
	linphone_chat_message_unref(message);

	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

	if (marieOneToOneCr) {
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneChatRoomStateTerminated, 1,
		                              liblinphone_tester_sip_timeout));
		marieOneToOneCr = NULL;
		/* The chatroom from Pauline is expected to terminate as well */
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
		                              1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

		bctbx_list_t *participants = linphone_chat_room_get_participants(paulineOneToOneCr);
		BC_ASSERT_EQUAL((int)bctbx_list_size(participants), 1, int, "%d");
		bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);

		LinphoneChatMessage *exhume_message =
		    linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "No. I am your father.");
		linphone_chat_message_send(exhume_message);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(exhume_message);

		exhumedConfAddr =
		    linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(paulineOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
		BC_ASSERT_EQUAL(pauline_messages, 2, int, "%d");

		if (exhumedConfAddr) {
			const char *conf_id = linphone_address_get_uri_param(confAddr, "conf-id");
			BC_ASSERT_PTR_NOT_NULL(conf_id);
			const char *exhumed_conf_id = linphone_address_get_uri_param(exhumedConfAddr, "conf-id");
			BC_ASSERT_PTR_NOT_NULL(exhumed_conf_id);
			if (!exhumed_conf_id || !conf_id) goto end;
			BC_ASSERT_STRING_NOT_EQUAL(conf_id, exhumed_conf_id);
			marieOneToOneCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats,
			                                                       exhumedConfAddr, "one to one", 1, FALSE);
			BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr);
			if (marieOneToOneCr) {
				int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
				BC_ASSERT_EQUAL(marie_messages, 1, int, "%d");

				LinphoneChatMessage *exhume_answer_message =
				    linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "Nooooooooooooo !");
				linphone_chat_message_send(exhume_answer_message);
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1,
				                             liblinphone_tester_sip_timeout));
				linphone_chat_message_unref(exhume_answer_message);
			}
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int, "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int, "%d");

		if (marieOneToOneCr) {
			BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
		}
		if (paulineOneToOneCr) {
			BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));
		}
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void exhume_one_to_one_chat_room_2(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses,
	                                               "one to one", FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);

	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr,
	                                                         "one to one", 1, FALSE);

	LinphoneChatMessage *message =
	    linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "Help me, Obi-Wan Kenobi. You’re my only hope.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, liblinphone_tester_sip_timeout));
	linphone_chat_message_unref(message);

	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

	if (marieOneToOneCr) {
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneChatRoomStateTerminated, 1,
		                              liblinphone_tester_sip_timeout));
		marieOneToOneCr = NULL;
		/* The chatroom from Pauline is expected to terminate as well */
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
		                              1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

		participantsAddresses = NULL;
		initialMarieStats = marie->stat;
		participantsAddresses =
		    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
		marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses,
		                                               "one to one", FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2, 5000);
		if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, 2,
		                             liblinphone_tester_sip_timeout));

		exhumedConfAddr =
		    linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		if (exhumedConfAddr) {
			const char *conf_id = linphone_address_get_uri_param(confAddr, "conf-id");
			BC_ASSERT_PTR_NOT_NULL(conf_id);
			const char *exhumed_conf_id = linphone_address_get_uri_param(exhumedConfAddr, "conf-id");
			BC_ASSERT_PTR_NOT_NULL(exhumed_conf_id);
			if (!exhumed_conf_id || !conf_id) goto end;
			BC_ASSERT_STRING_NOT_EQUAL(conf_id, exhumed_conf_id);
			LinphoneAddress *paulineNewConfAddr =
			    linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(paulineOneToOneCr));
			BC_ASSERT_TRUE(linphone_address_weak_equal(exhumedConfAddr, paulineNewConfAddr));
			if (paulineNewConfAddr) linphone_address_unref(paulineNewConfAddr);

			LinphoneChatMessage *exhume_message =
			    linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "I find your lack of faith disturbing.");
			linphone_chat_message_send(exhume_message);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1,
			                             liblinphone_tester_sip_timeout));
			linphone_chat_message_unref(exhume_message);

			int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 2, int, "%d");

			int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 1, int, "%d");

			LinphoneChatRoom *paulineOneToOneCr2 = check_creation_chat_room_client_side(
			    coresList, pauline, &initialPaulineStats, exhumedConfAddr, "one to one", 1, FALSE);
			BC_ASSERT_PTR_NOT_NULL(paulineOneToOneCr2);
			if (paulineOneToOneCr2) {
				int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr2);
				BC_ASSERT_EQUAL(pauline_messages, 2, int, "%d");

				LinphoneChatMessage *exhume_answer_message = linphone_chat_room_create_message_from_utf8(
				    paulineOneToOneCr2, "Your focus determines your reality.");
				linphone_chat_message_send(exhume_answer_message);
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2,
				                             liblinphone_tester_sip_timeout));
				linphone_chat_message_unref(exhume_answer_message);
			}
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int, "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int, "%d");

		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void linphone_tester_chat_room_exhumed(LinphoneCore *core, BCTBX_UNUSED(LinphoneChatRoom *room)) {
	stats *counters;
	counters = get_stats(core);
	counters->number_of_LinphoneChatRoomExhumed++;
}

static void exhume_one_to_one_chat_room_3_base(bool_t core_restart) {
	// WARNING: some waits are supposed to timeout in this test: the BC_ASSER_FALSE(wait_for...), do not increase the
	// value or it triggers a timeout on the server side
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = NULL;
	LinphoneChatRoom *marieOneToOneCr = NULL, *pauline2OneToOneCr = NULL, *paulineOneToOneCr = NULL;

	int marie_messages = 0, pauline_messages = 0, pauline2_messages = 0;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	char *conference_address_str = NULL;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	const char *initialSubject = "one to one";
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses,
	                                               initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);

	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	BC_ASSERT_PTR_NOT_NULL(confAddr);
	conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr,
	                                                         initialSubject, 1, FALSE);

	LinphoneChatMessage *message =
	    linphone_chat_room_create_message_from_utf8(paulineOneToOneCr, "Hasta la vista, baby.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, liblinphone_tester_sip_timeout));
	linphone_chat_message_unref(message);

	marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
	BC_ASSERT_EQUAL(marie_messages, 1, int, "%d");

	pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
	BC_ASSERT_EQUAL(pauline_messages, 1, int, "%d");

	// Pauline goes offline
	int dummy = 0;
	wait_for_list(coresList, &dummy, 1, 5000); // Ensures 200 OK of the NOTIFY has been ACK'ed
	ms_message("%s goes offline", linphone_core_get_identity(pauline->lc));
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);

	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

	if (marieOneToOneCr) {

		pauline2 = linphone_core_manager_create("pauline_rc");
		stats initialPauline2Stats = pauline2->stat;
		coresManagerList = bctbx_list_append(coresManagerList, pauline2);

		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline2);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		start_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);

		pauline2OneToOneCr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr,
		                                                          initialSubject, 1, FALSE);
		BC_ASSERT_PTR_NOT_NULL(pauline2OneToOneCr);
		if (!pauline2OneToOneCr) goto end;
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, 2, 5000));

		LinphoneChatMessage *offline_message =
		    linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "I'll be back.");
		linphone_chat_message_send(offline_message);
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
		linphone_chat_message_unref(offline_message);

		// The other device using Pauline's account received the message as she was online
		pauline2_messages = linphone_chat_room_get_history_size(pauline2OneToOneCr);
		BC_ASSERT_EQUAL(pauline2_messages, 1, int, "%d");

		marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
		BC_ASSERT_EQUAL(marie_messages, 2, int, "%d");

		pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
		BC_ASSERT_EQUAL(pauline_messages, 1, int, "%d");

		ms_message("%s is deleting chat room %s", linphone_core_get_identity(marie->lc), conference_address_str);
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneChatRoomStateTerminated, 1,
		                              liblinphone_tester_sip_timeout));
		marieOneToOneCr = NULL;
		/* The chatroom from Pauline won't be terminated as it is offline */
		BC_ASSERT_FALSE(
		    wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, 1, 5000));

		participantsAddresses = NULL;
		initialMarieStats = marie->stat;
		participantsAddresses =
		    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
		ms_message("%s is exhuming chat room %s", linphone_core_get_identity(marie->lc), conference_address_str);
		marieOneToOneCr =
		    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
		                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2,
		               liblinphone_tester_sip_timeout);
		if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 2, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, 2, 5000));

		exhumedConfAddr =
		    linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		if (exhumedConfAddr) {
			LinphoneChatMessage *exhume_message =
			    linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "I'm back.");
			linphone_chat_message_send(exhume_message);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 2, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, 2,
			                             liblinphone_tester_sip_timeout));

			marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 1, int, "%d");

			pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 1, int, "%d");

			pauline2_messages = linphone_chat_room_get_history_size(pauline2OneToOneCr);
			BC_ASSERT_EQUAL(pauline2_messages, 2, int, "%d");

			linphone_chat_message_unref(exhume_message);

			if (core_restart) {
				ms_message("%s is restarting its core", linphone_core_get_identity(pauline->lc));
				coresList = bctbx_list_remove(coresList, pauline->lc);
				linphone_core_manager_reinit(pauline);
				// Make sure conference factory URI is preserved
				LinphoneProxyConfig *lpc = linphone_core_get_default_proxy_config(pauline->lc);
				linphone_proxy_config_edit(lpc);
				linphone_proxy_config_set_conference_factory_uri(lpc, sFactoryUri);
				linphone_proxy_config_done(lpc);
				bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
				bctbx_list_t *tmpInitList = init_core_for_conference(tmpCoresManagerList);
				bctbx_list_free(tmpInitList);
				bctbx_list_free(tmpCoresManagerList);
				LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
				linphone_core_cbs_set_chat_room_exhumed(cbs, linphone_tester_chat_room_exhumed);
				linphone_core_add_callbacks(pauline->lc, cbs);
				linphone_core_manager_start(pauline, TRUE);
				coresList = bctbx_list_append(coresList, pauline->lc);

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomExhumed, 1,
				                             liblinphone_tester_sip_timeout));
				paulineOneToOneCr = linphone_core_get_chat_room(pauline->lc, exhumedConfAddr);
				BC_ASSERT_EQUAL((int)linphone_chat_room_get_previouses_conference_ids_count(paulineOneToOneCr), 1, int,
				                "%d");
				linphone_core_cbs_unref(cbs);

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_FALSE(
				    wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, 1, 5000));

				BC_ASSERT_EQUAL((int)linphone_chat_room_get_previouses_conference_ids_count(paulineOneToOneCr), 0, int,
				                "%d");
			} else {
				// Pauline goes back online
				ms_message("%s comes back online", linphone_core_get_identity(pauline->lc));
				linphone_core_set_network_reachable(pauline->lc, TRUE);
				BC_ASSERT_FALSE(
				    wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 2, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_FALSE(
				    wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, 1, 5000));
			}

			pauline2_messages = linphone_chat_room_get_history_size(pauline2OneToOneCr);
			BC_ASSERT_EQUAL(pauline2_messages, 2, int, "%d");

			pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 3, int, "%d");

			marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 1, int, "%d");

			LinphoneChatMessage *post_exhume_message =
			    linphone_chat_room_create_message_from_utf8(marieOneToOneCr, "Sarah Connor ?");
			linphone_chat_message_send(post_exhume_message);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, 3,
			                             liblinphone_tester_sip_timeout));
			linphone_chat_message_unref(post_exhume_message);

			pauline2_messages = linphone_chat_room_get_history_size(pauline2OneToOneCr);
			BC_ASSERT_EQUAL(pauline2_messages, 3, int, "%d");

			pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
			BC_ASSERT_EQUAL(pauline_messages, 4, int, "%d");

			marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
			BC_ASSERT_EQUAL(marie_messages, 2, int, "%d");
		}

		linphone_config_set_int(linphone_core_get_config(pauline2->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline2->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *pauline2_chat_rooms = linphone_core_get_chat_rooms(pauline2->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline2_chat_rooms), 1, int, "%d");

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int, "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int, "%d");

		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);
	if (conference_address_str) ms_free(conference_address_str);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(pauline2);
}

static void exhume_one_to_one_chat_room_3(void) {
	exhume_one_to_one_chat_room_3_base(FALSE);
}

static void exhume_one_to_one_chat_room_3_core_restart(void) {
	exhume_one_to_one_chat_room_3_base(TRUE);
}

static void exhume_one_to_one_chat_room_4(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneChatRoom *marieOneToOneCr = NULL, *paulineOneToOneCr = NULL;
	LinphoneAddress *confAddr = NULL, *exhumedConfAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses,
	                                               "one to one", FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);

	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieOneToOneCr));
	paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr,
	                                                         "one to one", 1, FALSE);

	LinphoneChatMessage *message = linphone_chat_room_create_message_from_utf8(
	    paulineOneToOneCr,
	    "There is only one Lord of the Ring, only one who can bend it to his will. And he does not share power.");
	linphone_chat_message_send(message);
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 1, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 1, liblinphone_tester_sip_timeout));
	linphone_chat_message_unref(message);

	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
	BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

	if (marieOneToOneCr) {
		linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneChatRoomStateTerminated, 1,
		                              liblinphone_tester_sip_timeout));
		/* The chatroom from Pauline is expected to terminate as well */
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
		                              1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));

		bctbx_list_t *participants = linphone_chat_room_get_participants(paulineOneToOneCr);
		BC_ASSERT_EQUAL((int)bctbx_list_size(participants), 1, int, "%d");
		bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);

		// Marie goes offline
		int dummy = 0;
		linphone_core_set_network_reachable(marie->lc, FALSE);
		wait_for_list(coresList, &dummy, 1, 2000);

		LinphoneChatMessage *exhume_message = linphone_chat_room_create_message_from_utf8(
		    paulineOneToOneCr, "I am Gandalf the White. And I come back to you now... at the turn of the tide.");
		linphone_chat_message_send(exhume_message);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                              liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2,
		                              liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(exhume_message);

		exhumedConfAddr =
		    linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(paulineOneToOneCr));
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);

		int pauline_messages = linphone_chat_room_get_history_size(paulineOneToOneCr);
		BC_ASSERT_EQUAL(pauline_messages, 2, int, "%d");

		// Marie goes back online
		linphone_core_set_network_reachable(marie->lc, TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomConferenceJoined, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, 2,
		                             liblinphone_tester_sip_timeout));

		if (exhumedConfAddr) {
			const char *conf_id = linphone_address_get_uri_param(confAddr, "conf-id");
			BC_ASSERT_PTR_NOT_NULL(conf_id);
			const char *exhumed_conf_id = linphone_address_get_uri_param(exhumedConfAddr, "conf-id");
			BC_ASSERT_PTR_NOT_NULL(exhumed_conf_id);
			if (!exhumed_conf_id || !conf_id) goto end;
			BC_ASSERT_STRING_NOT_EQUAL(conf_id, exhumed_conf_id);
			marieOneToOneCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats,
			                                                       exhumedConfAddr, "one to one", 1, FALSE);
			BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr);
			if (marieOneToOneCr) {
				int marie_messages = linphone_chat_room_get_history_size(marieOneToOneCr);
				BC_ASSERT_EQUAL(marie_messages, 1, int, "%d");

				LinphoneChatMessage *exhume_answer_message = linphone_chat_room_create_message_from_utf8(
				    marieOneToOneCr, "In my experience there is no such thing as luck.");
				linphone_chat_message_send(exhume_answer_message);
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1,
				                             liblinphone_tester_sip_timeout));
				linphone_chat_message_unref(exhume_answer_message);
			}
		}

		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *pauline_chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_chat_rooms), 1, int, "%d");

		linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "hide_chat_rooms_from_removed_proxies",
		                        0);
		const bctbx_list_t *marie_chat_rooms = linphone_core_get_chat_rooms(marie->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marie_chat_rooms), 1, int, "%d");

		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(marieOneToOneCr));
		BC_ASSERT_FALSE(linphone_chat_room_is_read_only(paulineOneToOneCr));
	}

end:
	if (confAddr) linphone_address_unref(confAddr);
	if (exhumedConfAddr) linphone_address_unref(exhumedConfAddr);
	if (marieOneToOneCr) linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	if (paulineOneToOneCr) linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_new_device_after_creation(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *marie2 = NULL;
	LinphoneChatRoom *marie1Cr = NULL, *marie2Cr = NULL, *pauline1Cr = NULL, *pauline2Cr = NULL, *laureCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses,
	                                        initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are
	// added
	pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr,
	                                                  initialSubject, 2, FALSE);
	pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr,
	                                                  initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(pauline1Cr) && BC_ASSERT_PTR_NOT_NULL(pauline2Cr))) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	// Marie adds a new device
	marie2 = linphone_core_manager_create("marie_rc");
	stats initialMarie2Stats = marie2->stat;
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie2);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	start_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	marie2Cr =
	    check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie2Cr)) goto end;

end:
	// Clean db from chat room
	if (marie1Cr) linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	if (marie2Cr) linphone_core_delete_chat_room(marie2->lc, marie2Cr);
	if (pauline1Cr) linphone_core_manager_delete_chat_room(pauline1, pauline1Cr, coresList);
	if (pauline2Cr) linphone_core_delete_chat_room(pauline2->lc, pauline2Cr);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	if (marie2) linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_list_subscription(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneChatRoom *marieCr1 = NULL, *marieCr2 = NULL, *marieCr3 = NULL;
	LinphoneChatRoom *paulineCr1 = NULL, *paulineCr2 = NULL, *paulineCr3 = NULL;
	LinphoneChatRoom *laureCr1 = NULL, *laureCr2 = NULL, *laureCr3 = NULL;
	const LinphoneAddress *confAddr1 = NULL, *confAddr2 = NULL, *confAddr3 = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	int dummy = 0;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr1 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                        FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr1)) goto end;
	confAddr1 = linphone_chat_room_get_conference_address(marieCr1);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr1)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are
	// added
	paulineCr1 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr1,
	                                                  initialSubject, 2, FALSE);
	laureCr1 =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr1, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(paulineCr1) && BC_ASSERT_PTR_NOT_NULL(laureCr1))) goto end;
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	initialSubject = "Friends";
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	marieCr2 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                        FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr2)) goto end;
	confAddr2 = linphone_chat_room_get_conference_address(marieCr2);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr2)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are
	// added
	paulineCr2 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr2,
	                                                  initialSubject, 2, FALSE);
	laureCr2 =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr2, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(paulineCr2) && BC_ASSERT_PTR_NOT_NULL(laureCr2))) goto end;
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	initialSubject = "Bros";
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	marieCr3 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                        FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr3)) goto end;
	confAddr3 = linphone_chat_room_get_conference_address(marieCr3);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr3)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are
	// added
	paulineCr3 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr3,
	                                                  initialSubject, 2, FALSE);
	laureCr3 =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr3, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(paulineCr3) && BC_ASSERT_PTR_NOT_NULL(laureCr3))) goto end;

	participantsAddresses = NULL;

	// Marie designates Pauline as admin in chat room 1
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr1, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr1, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

	// Pauline's device goes off
	paulineAddr = linphone_address_clone(linphone_chat_room_get_local_address(paulineCr1));
	coresList = bctbx_list_remove(coresList, pauline->lc);
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie designates Laure as admin in chat rooms 1 & 3
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant1 = linphone_chat_room_find_participant(marieCr1, laureAddr);
	LinphoneParticipant *laureParticipant2 = linphone_chat_room_find_participant(marieCr2, laureAddr);
	LinphoneParticipant *laureParticipant3 = linphone_chat_room_find_participant(marieCr3, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant1);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant2);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant3);
	linphone_chat_room_set_participant_admin_status(marieCr1, laureParticipant1, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                              initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                              3000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant1));
	linphone_chat_room_set_participant_admin_status(marieCr3, laureParticipant3, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialMarieStats.number_of_chat_room_participant_admin_statuses_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                              initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 3,
	                              3000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant3));

	// Marie now changes the subject or chat room 1
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr1, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                              initialPaulineStats.number_of_chat_room_subject_changed + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr1), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr1), newSubject);

	// Pauline is back
	linphone_core_manager_reinit(pauline);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline, TRUE);
	paulineCr1 = linphone_core_find_chat_room(pauline->lc, confAddr1, paulineAddr);
	paulineCr2 = linphone_core_find_chat_room(pauline->lc, confAddr2, paulineAddr);
	paulineCr3 = linphone_core_find_chat_room(pauline->lc, confAddr3, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineCr1);
	BC_ASSERT_PTR_NOT_NULL(paulineCr2);
	BC_ASSERT_PTR_NOT_NULL(paulineCr3);
	wait_for_list(coresList, &dummy, 1, 5000);

	// Check that Pauline receive the missing info and not more
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	linphone_address_unref(paulineAddr);
	LinphoneParticipant *laureParticipantOfPauline1 = linphone_chat_room_find_participant(paulineCr1, laureAddr);
	LinphoneParticipant *laureParticipantOfPauline2 = linphone_chat_room_find_participant(paulineCr2, laureAddr);
	LinphoneParticipant *laureParticipantOfPauline3 = linphone_chat_room_find_participant(paulineCr3, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantOfPauline1);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantOfPauline2);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantOfPauline3);
	linphone_address_unref(laureAddr);
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipantOfPauline1));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipantOfPauline3));
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipantOfPauline2));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr1), newSubject);

	// Check that Pauline can still receive info once back
	// Marie now changes the subject or chat room 1
	newSubject = "New New subject";
	linphone_chat_room_set_subject(marieCr1, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr1), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr1), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr1), newSubject);
	// Marie now changes the subject or chat room 2
	newSubject = "Newer subject";
	linphone_chat_room_set_subject(marieCr2, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 3,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr2), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr2), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr2), newSubject);
	// Marie now changes the subject or chat room 3
	newSubject = "Newest subject";
	linphone_chat_room_set_subject(marieCr3, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 4,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 4,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 4,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr3), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr3), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr3), newSubject);

end:
	// Clean db from chat room
	if (marieCr1) linphone_core_manager_delete_chat_room(marie, marieCr1, coresList);
	if (paulineCr1) linphone_core_manager_delete_chat_room(pauline, paulineCr1, coresList);
	if (laureCr1) linphone_core_manager_delete_chat_room(laure, laureCr1, coresList);
	if (marieCr2) linphone_core_manager_delete_chat_room(marie, marieCr2, coresList);
	if (paulineCr2) linphone_core_manager_delete_chat_room(pauline, paulineCr2, coresList);
	if (laureCr2) linphone_core_manager_delete_chat_room(laure, laureCr2, coresList);
	if (marieCr3) linphone_core_manager_delete_chat_room(marie, marieCr3, coresList);
	if (paulineCr3) linphone_core_manager_delete_chat_room(pauline, paulineCr3, coresList);
	if (laureCr3) linphone_core_manager_delete_chat_room(laure, laureCr3, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_complex_participant_removal_scenario(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *newLaureCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	LinphoneAddress *laureDeviceAddress;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Restart Laure core
	ms_message("%s restarts core", linphone_core_get_identity(laure->lc));
	linphone_core_set_network_reachable(laure->lc, FALSE);
	LinphoneAddress *laureAddr = linphone_address_clone(laure->identity);
	coresList = bctbx_list_remove(coresList, laure->lc);
	linphone_core_manager_reinit(laure);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(laure, TRUE);
	initialLaureStats = laure->stat;

	// Marie removes Laure from the chat room
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	wait_for_list(coresList, 0, 1, 2000);
	initialLaureStats = laure->stat;

	linphone_proxy_config_refresh_register(linphone_core_get_default_proxy_config(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk,
	                             initialLaureStats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                              initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	// Marie adds Laure to the chat room
	char *conference_addr_str = linphone_address_as_string(confAddr);
	ms_message("%s adds %s to chat room %s", linphone_core_get_identity(marie->lc),
	           linphone_core_get_identity(laure->lc), conference_addr_str);
	bctbx_free(conference_addr_str);
	participantsAddresses = bctbx_list_append(participantsAddresses, laureAddr);
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
	                             initialMarieStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
	                             initialPaulineStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreationPending,
	                             initialLaureStats.number_of_LinphoneChatRoomStateCreationPending + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialLaureStats.number_of_LinphoneChatRoomStateCreated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined,
	                             initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	char *laureIdentity = linphone_core_get_device_identity(laure->lc);
	laureDeviceAddress = linphone_address_new(laureIdentity);
	bctbx_free(laureIdentity);
	newLaureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureDeviceAddress);
	if (!BC_ASSERT_PTR_NOT_NULL(newLaureCr)) goto end;
	linphone_address_unref(laureDeviceAddress);
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newLaureCr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newLaureCr), initialSubject);
	BC_ASSERT_FALSE(linphone_chat_room_has_been_left(newLaureCr));

	unsigned int nbLaureConferenceCreatedEventsBeforeRestart = 0;
	bctbx_list_t *laureHistory = linphone_chat_room_get_history_events(newLaureCr, 0);
	for (bctbx_list_t *item = laureHistory; item; item = bctbx_list_next(item)) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
		if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceCreated)
			nbLaureConferenceCreatedEventsBeforeRestart++;
	}
	bctbx_list_free_with_data(laureHistory, (bctbx_list_free_func)linphone_event_log_unref);
	BC_ASSERT_EQUAL(nbLaureConferenceCreatedEventsBeforeRestart, 2, unsigned int, "%u");

	initialLaureStats = laure->stat;
	linphone_proxy_config_refresh_register(linphone_core_get_default_proxy_config(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk,
	                             initialLaureStats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined,
	                              initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated,
	                              initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	unsigned int nbLaureConferenceCreatedEventsAfterRestart = 0;
	laureHistory = linphone_chat_room_get_history_events(newLaureCr, 0);
	for (bctbx_list_t *item = laureHistory; item; item = bctbx_list_next(item)) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
		if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceCreated)
			nbLaureConferenceCreatedEventsAfterRestart++;
	}
	bctbx_list_free_with_data(laureHistory, (bctbx_list_free_func)linphone_event_log_unref);
	BC_ASSERT_EQUAL(nbLaureConferenceCreatedEventsAfterRestart, nbLaureConferenceCreatedEventsBeforeRestart,
	                unsigned int, "%u");

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (newLaureCr) linphone_core_manager_delete_chat_room(laure, newLaureCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_subscription_denied(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *laureCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	LinphoneAddress *paulineAddress = linphone_address_new(linphone_core_get_identity(pauline->lc));
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddress));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	// Simulate pauline has disconnected
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddress);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_remove_participant(marieCr, paulineParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                             initialMarieStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_removed,
	                             initialLaureStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	// Reconnect pauline
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(paulineAddress);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void search_friend_chat_room_participants(void) {
	LinphoneMagicSearch *magicSearch = NULL;
	bctbx_list_t *resultList = NULL;

	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *laureCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	LinphoneProxyConfig *laurePC = linphone_core_get_default_proxy_config(laure->lc);
	LinphoneProxyConfig *mariePC = linphone_core_get_default_proxy_config(marie->lc);
	const LinphoneAddress *laureIdentity = linphone_proxy_config_get_identity_address(laurePC);
	const LinphoneAddress *marieIdentity = linphone_proxy_config_get_identity_address(mariePC);
	char *laureI = linphone_address_as_string_uri_only(laureIdentity);
	char *marieI = linphone_address_as_string_uri_only(marieIdentity);
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	magicSearch = linphone_magic_search_new(pauline->lc);
	resultList = linphone_magic_search_get_contacts_list(magicSearch, "", "", LinphoneMagicSearchSourceAll,
	                                                     LinphoneMagicSearchAggregationNone);
	if (BC_ASSERT_PTR_NOT_NULL(resultList)) {
		BC_ASSERT_EQUAL(bctbx_list_size(resultList), 2, size_t, "%zu");
		_check_friend_result_list_2(pauline->lc, resultList, 0, laureI, NULL, NULL, LinphoneMagicSearchSourceChatRooms);
		_check_friend_result_list_2(pauline->lc, resultList, 1, marieI, NULL, NULL, LinphoneMagicSearchSourceChatRooms);
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_magic_search_unref);
	}

	ms_free(laureI);
	ms_free(marieI);
	linphone_magic_search_reset_search_cache(magicSearch);
	linphone_magic_search_unref(magicSearch);

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_participant_devices_name(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	LinphoneCoreManager *chloe2 = linphone_core_manager_create("chloe_rc");
	LinphoneCoreManager *chloe3 = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	coresManagerList = bctbx_list_append(coresManagerList, chloe2);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_set_user_agent(marie->lc, "blabla (Marie device) blibli/blublu (bloblo)", NULL);
	linphone_core_set_user_agent(pauline->lc, "blabla (Pauline device 1) blibli/blublu (bloblo)", NULL);
	linphone_core_set_user_agent(pauline2->lc, "blabla (Pauline device 2) blibli/blublu (bloblo)", NULL);
	linphone_core_set_user_agent(laure->lc, "(Laure device ((toto) tata) (titi)) (bloblo)", NULL);
	linphone_core_set_user_agent(chloe->lc, "blabla (Chloe device 1) blibli/blublu (bloblo)", NULL);
	linphone_core_set_user_agent(chloe2->lc, "blabla (Chloe device 2) blibli/blublu (bloblo)", NULL);
	start_core_for_conference(coresManagerList);
	LinphoneAddress *marieAddress = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddress =
	    linphone_address_ref(linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *laureAddress = linphone_address_ref(linphone_address_new(linphone_core_get_identity(laure->lc)));
	LinphoneAddress *chloeAddress = linphone_address_new(linphone_core_get_identity(chloe->lc));
	LinphoneAddress *marieDeviceAddress =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc)));
	LinphoneAddress *paulineDeviceAddress1 =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline->lc)));
	LinphoneAddress *paulineDeviceAddress2 =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline2->lc)));
	LinphoneAddress *laureDeviceAddress =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(laure->lc)));
	LinphoneAddress *chloeDeviceAddress1 =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe->lc)));
	LinphoneAddress *chloeDeviceAddress2 =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe2->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, paulineAddress);
	participantsAddresses = bctbx_list_append(participantsAddresses, laureAddress);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialPaulineStats2 = pauline2->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;
	stats initialChloeStats2 = chloe2->stat;
	stats initialChloeStats3 = chloe3->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPaulineStats2,
	                                                                    confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Check device's name in Marie's chat room
	LinphoneParticipant *marieParticipant = linphone_chat_room_get_me(marieCr);
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddress);
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddress);
	LinphoneParticipantDevice *marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	LinphoneParticipantDevice *paulineDevice =
	    linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	LinphoneParticipantDevice *paulineDevice2 =
	    linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	LinphoneParticipantDevice *laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");

	// Check device's name in Pauline1's chat room
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddress);
	paulineParticipant = linphone_chat_room_get_me(paulineCr);
	laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");

	// Check device's name in Pauline2's chat room
	marieParticipant = linphone_chat_room_find_participant(paulineCr2, marieAddress);
	paulineParticipant = linphone_chat_room_get_me(paulineCr2);
	laureParticipant = linphone_chat_room_find_participant(paulineCr2, laureAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");

	// Check device's name in Laure's chat room
	marieParticipant = linphone_chat_room_find_participant(laureCr, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(laureCr, paulineAddress);
	laureParticipant = linphone_chat_room_get_me(laureCr);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");

	// Reset stats
	marie->stat = initialMarieStats;
	pauline->stat = initialPaulineStats;
	pauline2->stat = initialPaulineStats2;
	laure->stat = initialLaureStats;
	chloe->stat = initialChloeStats;
	chloe2->stat = initialChloeStats2;
	chloe3->stat = initialChloeStats3;

	// Marie adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, chloeAddress);
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free(participantsAddresses);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, 0);
	LinphoneChatRoom *chloeCr2 =
	    check_creation_chat_room_client_side(coresList, chloe2, &initialChloeStats2, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_added,
	                             initialMarieStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_devices_added,
	                             initialMarieStats.number_of_chat_room_participant_devices_added + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_added,
	                             initialPaulineStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_added,
	                             initialPaulineStats.number_of_chat_room_participant_devices_added + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_chat_room_participants_added,
	                             initialPaulineStats2.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_chat_room_participant_devices_added,
	                             initialPaulineStats2.number_of_chat_room_participant_devices_added + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_added,
	                             initialLaureStats.number_of_chat_room_participants_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_devices_added,
	                             initialLaureStats.number_of_chat_room_participant_devices_added + 2,
	                             liblinphone_tester_sip_timeout));

	// Check device's name in Chloe's chat room
	LinphoneParticipant *chloeParticipant = linphone_chat_room_get_me(chloeCr);
	marieParticipant = linphone_chat_room_find_participant(chloeCr, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(chloeCr, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(chloeCr, laureAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	LinphoneParticipantDevice *chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	LinphoneParticipantDevice *chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");

	// Check device's name in Chloe2's chat room
	chloeParticipant = linphone_chat_room_get_me(chloeCr2);
	marieParticipant = linphone_chat_room_find_participant(chloeCr2, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(chloeCr2, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(chloeCr2, laureAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");

	// Check Chloe's devices name in other chat rooms
	// Marie
	chloeParticipant = linphone_chat_room_find_participant(marieCr, chloeAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");

	// Pauline
	chloeParticipant = linphone_chat_room_find_participant(paulineCr, chloeAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");

	// Pauline 2
	chloeParticipant = linphone_chat_room_find_participant(paulineCr2, chloeAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");

	// Laure
	chloeParticipant = linphone_chat_room_find_participant(laureCr, chloeAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");

	// Reset stats
	marie->stat = initialMarieStats;
	pauline->stat = initialPaulineStats;
	pauline2->stat = initialPaulineStats2;
	laure->stat = initialLaureStats;
	chloe->stat = initialChloeStats;
	chloe2->stat = initialChloeStats2;
	chloe3->stat = initialChloeStats3;

	// Chloe adds a new device
	coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, chloe3);
	bctbx_list_t *tmpInitList = init_core_for_conference(coresManagerList);
	bctbx_list_free(tmpInitList);
	coresList = bctbx_list_append(coresList, chloe3->lc);
	linphone_core_set_user_agent(chloe3->lc, "blabla (Chloe device 3) blibli/blublu (bloblo)", NULL);
	start_core_for_conference(coresManagerList);
	LinphoneAddress *chloeDeviceAddress3 =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe3->lc)));
	LinphoneChatRoom *chloeCr3 =
	    check_creation_chat_room_client_side(coresList, chloe3, &initialChloeStats3, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_devices_added,
	                             initialMarieStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_added,
	                             initialPaulineStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_chat_room_participant_devices_added,
	                             initialPaulineStats2.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_devices_added,
	                             initialLaureStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_chat_room_participant_devices_added,
	                             initialChloeStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe2->stat.number_of_chat_room_participant_devices_added,
	                             initialChloeStats2.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));

	// Check device's name in Chloe3's chat room
	chloeParticipant = linphone_chat_room_get_me(chloeCr3);
	marieParticipant = linphone_chat_room_find_participant(chloeCr3, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(chloeCr3, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(chloeCr3, laureAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	LinphoneParticipantDevice *chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Check Chloe3's device name in other chat rooms
	// Marie
	chloeParticipant = linphone_chat_room_find_participant(marieCr, chloeAddress);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Pauline
	chloeParticipant = linphone_chat_room_find_participant(paulineCr, chloeAddress);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Pauline 2
	chloeParticipant = linphone_chat_room_find_participant(paulineCr2, chloeAddress);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Laure
	chloeParticipant = linphone_chat_room_find_participant(laureCr, chloeAddress);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Chloe
	chloeParticipant = linphone_chat_room_get_me(chloeCr);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Chloe2
	chloeParticipant = linphone_chat_room_get_me(chloeCr2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Still works after restart
	linphone_core_manager_restart(marie, TRUE);
	linphone_core_manager_restart(pauline, TRUE);
	linphone_core_manager_restart(pauline2, TRUE);
	linphone_core_manager_restart(laure, TRUE);
	linphone_core_manager_restart(chloe, TRUE);
	linphone_core_manager_restart(chloe2, TRUE);
	linphone_core_manager_restart(chloe3, TRUE);

	bctbx_list_free(coresList);
	coresList = NULL;
	coresList = bctbx_list_append(coresList, marie->lc);
	coresList = bctbx_list_append(coresList, pauline->lc);
	coresList = bctbx_list_append(coresList, pauline2->lc);
	coresList = bctbx_list_append(coresList, laure->lc);
	coresList = bctbx_list_append(coresList, chloe->lc);
	coresList = bctbx_list_append(coresList, chloe2->lc);
	coresList = bctbx_list_append(coresList, chloe3->lc);

	// Marie
	marieCr = linphone_core_find_chat_room(marie->lc, confAddr, marieDeviceAddress);
	LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(marieCr, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	marieParticipant = linphone_chat_room_get_me(marieCr);
	paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddress);
	chloeParticipant = linphone_chat_room_find_participant(marieCr, chloeAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Pauline1
	paulineCr = linphone_core_find_chat_room(pauline->lc, confAddr, paulineDeviceAddress1);
	cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(paulineCr, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddress);
	paulineParticipant = linphone_chat_room_get_me(paulineCr);
	laureParticipant = linphone_chat_room_find_participant(paulineCr, laureAddress);
	chloeParticipant = linphone_chat_room_find_participant(paulineCr, chloeAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Pauline2
	paulineCr2 = linphone_core_find_chat_room(pauline2->lc, confAddr, paulineDeviceAddress2);
	cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(paulineCr2, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(paulineCr2);
	marieParticipant = linphone_chat_room_find_participant(paulineCr2, marieAddress);
	paulineParticipant = linphone_chat_room_get_me(paulineCr2);
	laureParticipant = linphone_chat_room_find_participant(paulineCr2, laureAddress);
	chloeParticipant = linphone_chat_room_find_participant(paulineCr2, chloeAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Laure
	laureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureDeviceAddress);
	cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(laureCr, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(laureCr);
	marieParticipant = linphone_chat_room_find_participant(laureCr, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(laureCr, paulineAddress);
	laureParticipant = linphone_chat_room_get_me(laureCr);
	chloeParticipant = linphone_chat_room_find_participant(laureCr, chloeAddress);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Chloe1
	chloeCr = linphone_core_find_chat_room(chloe->lc, confAddr, chloeDeviceAddress1);
	cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(chloeCr, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(chloeCr);
	marieParticipant = linphone_chat_room_find_participant(chloeCr, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(chloeCr, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(chloeCr, laureAddress);
	chloeParticipant = linphone_chat_room_get_me(chloeCr);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Chloe2
	chloeCr2 = linphone_core_find_chat_room(chloe2->lc, confAddr, chloeDeviceAddress2);
	cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(chloeCr2, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(chloeCr2);
	marieParticipant = linphone_chat_room_find_participant(chloeCr2, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(chloeCr2, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(chloeCr2, laureAddress);
	chloeParticipant = linphone_chat_room_get_me(chloeCr2);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	// Chloe3
	chloeCr3 = linphone_core_find_chat_room(chloe3->lc, confAddr, chloeDeviceAddress3);
	cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
	linphone_chat_room_cbs_set_state_changed(cbs, chat_room_state_changed);
	linphone_chat_room_add_callbacks(chloeCr3, cbs);
	linphone_chat_room_cbs_unref(cbs);
	BC_ASSERT_PTR_NOT_NULL(chloeCr3);
	marieParticipant = linphone_chat_room_find_participant(chloeCr3, marieAddress);
	paulineParticipant = linphone_chat_room_find_participant(chloeCr3, paulineAddress);
	laureParticipant = linphone_chat_room_find_participant(chloeCr3, laureAddress);
	chloeParticipant = linphone_chat_room_get_me(chloeCr3);
	marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
	laureDevice = linphone_participant_find_device(laureParticipant, laureDeviceAddress);
	chloeDevice = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress1);
	chloeDevice2 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress2);
	chloeDevice3 = linphone_participant_find_device(chloeParticipant, chloeDeviceAddress3);
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(marieDevice), "Marie device");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice), "Pauline device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(paulineDevice2), "Pauline device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device ((toto) tata) (titi)");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice), "Chloe device 1");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice2), "Chloe device 2");
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(chloeDevice3), "Chloe device 3");

	linphone_address_unref(marieAddress);
	linphone_address_unref(paulineAddress);
	linphone_address_unref(laureAddress);
	linphone_address_unref(chloeAddress);
	linphone_address_unref(confAddr);
	linphone_address_unref(marieDeviceAddress);
	linphone_address_unref(paulineDeviceAddress1);
	linphone_address_unref(paulineDeviceAddress2);
	linphone_address_unref(laureDeviceAddress);
	linphone_address_unref(chloeDeviceAddress1);
	linphone_address_unref(chloeDeviceAddress2);
	linphone_address_unref(chloeDeviceAddress3);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(chloe2, chloeCr2, coresList);
	linphone_core_manager_delete_chat_room(chloe3, chloeCr3, coresList);
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
	linphone_core_manager_destroy(chloe2);
	linphone_core_manager_destroy(chloe3);
}

static void add_device_one_to_one_chat_room_other_left(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_rc");
	LinphoneAddress *confAddr = NULL;
	LinphoneChatRoom *paulineCr = NULL, *paulineCr2 = NULL, *marieCr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	LinphoneAddress *paulineAddress = linphone_address_new(linphone_core_get_identity(pauline->lc));
	participantsAddresses = bctbx_list_append(participantsAddresses, paulineAddress);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialPaulineStats2 = pauline2->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto clean;

	confAddr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto clean;
	confAddr = linphone_address_clone(confAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto clean;

	// Marie leaves the chat room
	linphone_chat_room_leave(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminationPending,
	                             initialMarieStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));

	int pauline_number_of_chat_room_participant_devices_added =
	    pauline->stat.number_of_chat_room_participant_devices_added;
	linphone_core_manager_start(pauline2, TRUE);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPaulineStats2, confAddr,
	                                                  initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr2)) goto clean;

	// Also, pauline should receive a notification that pauline2 device was added.
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_added,
	                             pauline_number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));

	wait_for_list(coresList, NULL, 0, 1000);

clean:
	// Clean db from chat room
	if (pauline && paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	if (pauline2 && paulineCr) linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	if (marie && marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);

	if (confAddr) linphone_address_unref(confAddr);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(pauline2);
}

/*
 * This test simulates a case where whatever the reason the server thinks a client device is part of a given chatroom,
 *but the client device has no longer this information.
 **/
static void group_chat_loss_of_client_context(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Save Laure's db before it becomes part of a group chat.
	const char *uri = linphone_config_get_string(linphone_core_get_config(laure->lc), "storage", "uri", "");
	char *uriCopy = random_filepath("linphone_tester", "db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopy));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Save Laure's db now that it is part of the group chat
	uri = linphone_config_get_string(linphone_core_get_config(laure->lc), "storage", "uri", "");
	char *uriCopyAfter = random_filepath("linphone_tester2", "db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopyAfter));

	// Restore old db to Laure and restart it.
	if (laure->database_path) bc_free(laure->database_path);
	laure->database_path = uriCopy;
	coresList = bctbx_list_remove(coresList, laure->lc);
	linphone_core_manager_reinit(laure);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(laure, TRUE);

	// Pauline sends a message, laure shall not receive it, in any way.
	if (paulineCr) {
		_send_message(paulineCr, "Salut");
	}
	wait_for_list(coresList, NULL, 0, 2000);
	BC_ASSERT_TRUE(linphone_core_get_chat_rooms(laure->lc) == NULL);

	// Now restarts Laure with good db in order to clean the chatroom properly.
	// Restore old db to Laure and restart it.
	if (laure->database_path) bc_free(laure->database_path);
	laure->database_path = uriCopyAfter;
	coresList = bctbx_list_remove(coresList, laure->lc);
	linphone_core_manager_reinit(laure);
	tmpCoresManagerList = bctbx_list_append(NULL, laure);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(laure, TRUE);

	// Clean chatroom from databases.

	if (BC_ASSERT_TRUE(linphone_core_get_chat_rooms(laure->lc) != NULL)) {
		LinphoneChatRoom *laureCr = (LinphoneChatRoom *)linphone_core_get_chat_rooms(laure->lc)->data;
		linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	}

end:
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void participant_removed_then_added(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneChatRoom *marie1Cr = NULL, *pauline1Cr = NULL, *newPauline1Cr = NULL, *laureCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses,
	                                        initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline1
	pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr,
	                                                  initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1Cr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	// Pauline restart to make use of list subscription ::this part (core restart) is leaking  memory
	coresList = bctbx_list_remove(coresList, pauline1->lc);
	linphone_core_manager_reinit(pauline1);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline1);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline1, TRUE);

	// Check that the chat room has correctly created on Pauline's side and that the participants are added
	pauline1Cr =
	    check_has_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);

	// wait for first notify to be received by pauline
	wait_for_list(coresList, NULL, 0, 1000);

	// Pauline leaving but keeping a ref like a Java GC can do, this is the key part of this test.
	linphone_chat_room_ref(pauline1Cr);

	linphone_core_delete_chat_room(pauline1->lc, pauline1Cr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateDeleted,
	                             initialPauline1Stats.number_of_LinphoneChatRoomStateDeleted + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_chat_room_participants_removed,
	                             initialMarie1Stats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_removed,
	                             initialLaureStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie1Cr), 1, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 1, int, "%d");

	size_t paulineCrNo = bctbx_list_size(linphone_core_get_chat_rooms(pauline1->lc));
	BC_ASSERT_EQUAL(paulineCrNo, 0, size_t, "%0zu");

	// Send a message when Pauline is left to test ordering between events and messages.
	wait_for_list(coresList, NULL, 0, 1500); // Wait enough time to make a difference on time_t
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marie1Cr, textMessage);
	wait_for_list(coresList, NULL, 0, 1500); // Wait enough time to make a difference on time_t
	linphone_chat_message_unref(message);

	// Marie1 adds Pauline back to the chat room
	initialMarie1Stats = marie1->stat;
	initialLaureStats = laure->stat;
	initialPauline1Stats = pauline1->stat;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	linphone_chat_room_add_participants(marie1Cr, participantsAddresses);
	BC_ASSERT_TRUE(wait_for_chat_room_participants(coresList, marie1Cr, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_chat_room_participants(coresList, laureCr, 2, liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie1Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");
	newPauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr,
	                                                     initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(newPauline1Cr)) goto end;
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newPauline1Cr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newPauline1Cr), initialSubject);

	// Check timestamps ordering in history
	bctbx_list_t *history = linphone_chat_room_get_history_range_near(
	    marie1Cr, 3, 0, NULL,
	    LinphoneChatRoomHistoryFilterChatMessage | LinphoneChatRoomHistoryFilterInfoNoDevice); // Leave + Message + Join
	bctbx_list_t *itHistory = history;
	int step = 0;
	time_t lastTime = 0;
	while (itHistory) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(itHistory);
		switch (step) {
			case 0:
				BC_ASSERT_EQUAL((int)linphone_event_log_get_type(event),
				                LinphoneEventLogTypeConferenceParticipantRemoved, int, "%d");
				lastTime = linphone_event_log_get_creation_time(event); // Main ordering (Leave + Message + Join)
				break;
			case 1: {
				BC_ASSERT_EQUAL((int)linphone_event_log_get_type(event), LinphoneEventLogTypeConferenceChatMessage, int,
				                "%d");
				LinphoneChatMessage *message = linphone_event_log_get_chat_message(event);
				time_t messageTime = linphone_chat_message_get_time(message);
				BC_ASSERT_TRUE(lastTime < messageTime); // Main ordering (Leave + Message + Join)
				BC_ASSERT_TRUE(messageTime >= linphone_event_log_get_creation_time(
				                                  event)); // A send message timestamp should be after its creation.
				lastTime = messageTime;
			} break;
			case 2:
				BC_ASSERT_EQUAL((int)linphone_event_log_get_type(event), LinphoneEventLogTypeConferenceParticipantAdded,
				                int, "%d");
				BC_ASSERT_TRUE(lastTime <
				               linphone_event_log_get_creation_time(event)); // Main ordering (Leave + Message + Join)
				lastTime = linphone_event_log_get_creation_time(event);
				break;
			default: {
			}
		}
		++step;
		itHistory = bctbx_list_next(itHistory);
	}
	bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_event_log_unref);

	size_t marieCrNo = bctbx_list_size(linphone_core_get_chat_rooms(marie1->lc));
	BC_ASSERT_EQUAL(marieCrNo, 1, size_t, "%0zu");

	size_t laureCrNo = bctbx_list_size(linphone_core_get_chat_rooms(laure->lc));
	BC_ASSERT_EQUAL(laureCrNo, 1, size_t, "%0zu");

	paulineCrNo = bctbx_list_size(linphone_core_get_chat_rooms(pauline1->lc));
	BC_ASSERT_EQUAL(paulineCrNo, 1, size_t, "%0zu");

end:
	// Clean db from chat room
	if (marie1Cr) linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	if (newPauline1Cr) linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	// now GC is cleaning old chatroom
	if (pauline1Cr) linphone_chat_room_unref(pauline1Cr);

	marieCrNo = bctbx_list_size(linphone_core_get_chat_rooms(marie1->lc));
	BC_ASSERT_EQUAL(marieCrNo, 0, size_t, "%0zu");
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneChatRoomStateDeleted, 1,
	                             liblinphone_tester_sip_timeout));

	laureCrNo = bctbx_list_size(linphone_core_get_chat_rooms(laure->lc));
	BC_ASSERT_EQUAL(laureCrNo, 0, size_t, "%0zu");
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateDeleted, 1,
	                             liblinphone_tester_sip_timeout));

	paulineCrNo = bctbx_list_size(linphone_core_get_chat_rooms(pauline1->lc));
	BC_ASSERT_EQUAL(paulineCrNo, 0, size_t, "%0zu");
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateDeleted, 2,
	                             liblinphone_tester_sip_timeout));

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_join_one_to_one_chat_room_with_a_new_device_not_notified(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *marie2 = NULL;
	LinphoneChatRoom *marie1Cr = NULL, *marie2Cr = NULL, *paulineCr = NULL;
	LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference_with_groupchat_version(coresManagerList, "1.0");
	start_core_for_conference(coresManagerList);
	bctbx_list_t *participantsAddresses =
	    bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie1 creates a new one-to-one chat room with Pauline
	const char *initialSubject = "Pauline";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses,
	                                        initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie1Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	confAddr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;
	confAddr = linphone_address_clone(confAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	initialPaulineStats.number_of_chat_room_participant_devices_added =
	    pauline->stat.number_of_chat_room_participant_devices_added;
	marie2 = linphone_core_manager_create("marie_rc");
	stats initialMarie2Stats = marie2->stat;

	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, marie2);
	bctbx_list_t *newCoresList = init_core_for_conference_with_groupchat_version(newCoresManagerList, "1.0");
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Marie2 gets the one-to-one chat room with Pauline
	marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1,
	                                                FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie2Cr)) goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie2Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_added,
	                             initialPaulineStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));

	// Save pauline db
	const char *uri = linphone_config_get_string(linphone_core_get_config(pauline->lc), "storage", "uri", "");
	char *uriCopy = random_filepath("linphone_tester", "db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopy));
	int initialPaulineEvent = linphone_chat_room_get_history_events_size(paulineCr);
	// Simulate an uninstall of the application on Marie's side with unregistration (It should remove the device)

	// Save the number of chat rooms at the moment of stopping the core
	const int marie1_no_cr = (int)bctbx_list_size(linphone_core_get_chat_rooms(marie1->lc));
	coresManagerList = bctbx_list_remove(coresManagerList, marie1);
	coresList = bctbx_list_remove(coresList, marie1->lc);
	initialPaulineStats = pauline->stat;
	linphone_core_manager_stop(marie1);
	// force flexisip to publish on marie topic
	linphone_core_refresh_registers(marie2->lc);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_removed,
	                             initialPaulineStats.number_of_chat_room_participant_devices_removed + 1,
	                             liblinphone_tester_sip_timeout));

	LinphoneAddress *marieAddress = linphone_address_new(linphone_core_get_identity(marie2->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddress);
	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_participant_get_devices(marieParticipant)), 1, int, "%i");

	// Reset db at pauline side
	if (pauline->database_path) bc_free(pauline->database_path);
	pauline->database_path = uriCopy;
	coresList = bctbx_list_remove(coresList, pauline->lc);
	memset(&initialPaulineStats, 0, sizeof(initialPaulineStats));

	linphone_core_manager_reinit(pauline);
	// force full state
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_package_force_full_state",
	                         TRUE);
	setup_mgr_for_conference(pauline, NULL);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	bctbx_list_t *tmpCoresList = init_core_for_conference_with_groupchat_version(tmpCoresManagerList, "1.0");
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	initialPaulineStats = pauline->stat;
	linphone_core_manager_start(pauline, TRUE);

	// wait for first notify to be received by pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_NotifyFullStateReceived,
	                             initialPaulineStats.number_of_NotifyFullStateReceived + 1,
	                             liblinphone_tester_sip_timeout));

	// Marie2 gets the one-to-one chat room with Pauline
	paulineCr =
	    check_has_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	marieAddress = linphone_address_new(linphone_core_get_identity(marie2->lc));
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddress);
	bctbx_list_t *marieDevices = linphone_participant_get_devices(marieParticipant);
	BC_ASSERT_EQUAL((int)bctbx_list_size(marieDevices), 1, int, "%i");
	bctbx_list_free(marieDevices);

	// recheck after restart
	coresList = bctbx_list_remove(coresList, pauline->lc);
	linphone_core_manager_reinit(pauline);
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_package_force_full_state",
	                         FALSE);
	setup_mgr_for_conference(pauline, NULL);
	tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	tmpCoresList = init_core_for_conference_with_groupchat_version(tmpCoresManagerList, "1.0");
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	initialPaulineStats = pauline->stat;
	linphone_core_manager_start(pauline, TRUE);

	// wait for the subscription to be sent and gone into the active state
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneSubscriptionActive,
	                             initialPaulineStats.number_of_LinphoneSubscriptionActive + 1,
	                             liblinphone_tester_sip_timeout));

	// Marie2 gets the one-to-one chat room with Pauline
	paulineCr =
	    check_has_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddress);
	marieDevices = linphone_participant_get_devices(marieParticipant);
	BC_ASSERT_EQUAL((int)bctbx_list_size(marieDevices), 1, int, "%i");
	bctbx_list_free(marieDevices);
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_events_size(paulineCr), initialPaulineEvent, int, "%i");

	// check if we can still communicate
	//  Marie1 sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marie2Cr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDelivered,
	                             initialMarie1Stats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Hey. How are you?";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered,
	                             initialPaulineStats.number_of_LinphoneMessageDelivered + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                             initialMarie2Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie2->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Clean db from chat room
	linphone_core_manager_reinit(marie1);
	tmpCoresManagerList = bctbx_list_append(NULL, marie1);
	tmpCoresList = init_core_for_conference_with_groupchat_version(tmpCoresManagerList, "1.0");
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(marie1, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneChatRoomStateCreated,
	                             initialMarie1Stats.number_of_LinphoneChatRoomStateCreated + marie1_no_cr,
	                             liblinphone_tester_sip_timeout));
	wait_for_list(coresList, NULL, 0, 1000);
	marie1Cr =
	    check_has_chat_room_client_side(coresList, marie1, &initialMarie1Stats, confAddr, initialSubject, 1, FALSE);

end:
	if (marie1Cr) linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	if (marie2Cr) linphone_core_manager_delete_chat_room(marie2, marie2Cr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	if (confAddr) linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	if (marie2) linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
}

static void subscribe_test_after_set_chat_database_path(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneChatRoom *marieCr = NULL, *paulineCr = NULL, *laureCr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);

	start_core_for_conference(coresManagerList);

	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                       FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject,
	                                                 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	linphone_core_manager_reinit(pauline);
	coresList = NULL;
	coresList = bctbx_list_append(coresList, marie->lc);
	coresList = bctbx_list_append(coresList, pauline->lc);
	coresList = bctbx_list_append(coresList, laure->lc);

	LinphoneAddress *factoryAddr = linphone_address_new(sFactoryUri);
	_configure_core_for_conference(pauline, factoryAddr);
	linphone_address_unref(factoryAddr);
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
	configure_core_for_callbacks(pauline, cbs);
	linphone_core_cbs_unref(cbs);
	linphone_core_manager_start(pauline, TRUE);

	LinphoneAddress *paulineAddress =
	    linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline->lc)));
	paulineCr = linphone_core_find_chat_room(pauline->lc, confAddr, paulineAddress);

	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	linphone_chat_room_ref(paulineCr);

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	const char *path = "";
	// set_chat_database_path() will cause a reload of the database, with re-creation of chatrooms.
	linphone_core_set_chat_database_path(pauline->lc, path);
	if (paulineCr) linphone_chat_room_unref(paulineCr);
	paulineCr = linphone_core_find_chat_room(pauline->lc, confAddr, paulineAddress);
	linphone_address_unref(paulineAddress);
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	// Marie now changes the subject
	const char *newSubject = "Subject has been changed! :O";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_subject_changed,
	                             initialMarieStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_subject_changed,
	                             initialPaulineStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_subject_changed,
	                             initialLaureStats.number_of_chat_room_subject_changed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

end:
	// Clean db from chat room
	if (marieCr) linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	if (paulineCr) linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void one_to_one_chat_room_send_forward_message(void) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(FALSE, TRUE, FALSE);
}

static void one_to_one_chat_room_send_forward_message_with_restart(void) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE, TRUE, FALSE);
}

static void one_to_one_chat_room_reply_forward_message(void) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(FALSE, FALSE, TRUE);
}

static void one_to_one_chat_room_reply_forward_message_with_restart(void) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE, FALSE, TRUE);
}

static void core_stop_start_with_chat_room_ref(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_rc");
	LinphoneChatRoom *marie1Cr = NULL, *pauline1Cr = NULL, *newPauline1Cr = NULL;
	const LinphoneAddress *confAddr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses,
	                                        initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline1
	pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr,
	                                                  initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1Cr)) goto end;

	// Pauline leaving but keeping a ref like a Java GC can do, this is the key part of this test.
	linphone_chat_room_ref(pauline1Cr);

	linphone_core_stop(pauline1->lc);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneGlobalShutdown,
	                             initialPauline1Stats.number_of_LinphoneGlobalShutdown + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneGlobalOff,
	                             initialPauline1Stats.number_of_LinphoneGlobalOff + 1, liblinphone_tester_sip_timeout));

	linphone_core_start(pauline1->lc);
	// now GC is cleaning old chatroom
	if (pauline1Cr) linphone_chat_room_unref(pauline1Cr);

	// test very early client group chatroom creation, should fail
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(pauline1->lc);
	linphone_proxy_config_edit(proxy);
	linphone_proxy_config_set_conference_factory_uri(proxy, sFactoryUri);
	linphone_proxy_config_done(proxy);
	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(pauline1->lc);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	LinphoneChatRoom *chatRoom =
	    linphone_core_create_chat_room_2(pauline1->lc, params, initialSubject, participantsAddresses);
	linphone_chat_room_params_unref(params);

	BC_ASSERT_PTR_NOT_NULL(chatRoom);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateCreationFailed,
	                             initialPauline1Stats.number_of_LinphoneChatRoomStateCreationFailed + 1,
	                             liblinphone_tester_sip_timeout));
	if (chatRoom) linphone_chat_room_unref(chatRoom);

	coresList = bctbx_list_remove(coresList, pauline1->lc);
	linphone_core_manager_reinit(pauline1);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline1);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline1, TRUE);

	// Check that the chat room has correctly created on Laure's side and that the participants are added
	newPauline1Cr =
	    check_has_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 1, FALSE);
	wait_for_list(coresList, NULL, 0, 1000);

end:
	// Clean db from chat room
	if (marie1Cr) linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	if (newPauline1Cr) linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(pauline1);
}

static void group_chat_room_device_unregistered(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	LinphoneProxyConfig *proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, FALSE);
	linphone_proxy_config_done(proxyConfig);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationCleared,
	                             initialMarieStats.number_of_LinphoneRegistrationCleared + 1,
	                             liblinphone_tester_sip_timeout));

	// Current expected behavior is to have participant devices removed
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_devices_removed,
	                             initialMarieStats.number_of_chat_room_participant_devices_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_removed,
	                             initialPaulineStats.number_of_chat_room_participant_devices_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                              initialMarieStats.number_of_chat_room_participants_removed + 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                              initialPaulineStats.number_of_chat_room_participants_removed + 1, 3000));

	// to avoid automatic re-subscription of chatroom  while disabling network
	ms_message("%s enters background", linphone_core_get_identity(marie->lc));
	linphone_core_enter_background(marie->lc);
	ms_message("%s enters background", linphone_core_get_identity(pauline->lc));
	linphone_core_enter_background(pauline->lc);
	ms_message("%s enters background", linphone_core_get_identity(laure->lc));
	linphone_core_enter_background(laure->lc);
	wait_for_list(coresList, NULL, 1, 3000);
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateDeleted,
	                              initialMarieStats.number_of_LinphoneChatRoomStateDeleted + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateDeleted,
	                              initialPaulineStats.number_of_LinphoneChatRoomStateDeleted + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateDeleted,
	                              initialLaureStats.number_of_LinphoneChatRoomStateDeleted + 1, 1000));

	// to force re-re-connection to restarted flexisip
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, FALSE);

	// break here and restart Flexisip
	marie->stat.number_of_chat_room_participant_devices_added = 0;
	pauline->stat.number_of_chat_room_participant_devices_added = 0;

	proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, TRUE);
	linphone_proxy_config_done(proxyConfig);

	ms_message("%s enters foreground", linphone_core_get_identity(marie->lc));
	linphone_core_enter_foreground(marie->lc);
	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneRegistrationOk,
	                             initialMarieStats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));

	ms_message("%s enters foreground", linphone_core_get_identity(pauline->lc));
	linphone_core_enter_foreground(pauline->lc);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneRegistrationOk,
	                             initialPaulineStats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));

	ms_message("%s enters foreground", linphone_core_get_identity(laure->lc));
	linphone_core_enter_foreground(laure->lc);
	linphone_core_set_network_reachable(laure->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk,
	                             initialLaureStats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));

	// in case of flexisip restart
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_devices_added,
	                             initialMarieStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_added,
	                             initialPaulineStats.number_of_chat_room_participant_devices_added + 1,
	                             liblinphone_tester_sip_timeout));

	// Laure adds a new device without group chat enabled
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;
	LinphoneCoreManager *laure2 = linphone_core_manager_new("laure_tcp_rc");
	coresManagerList = bctbx_list_append(coresManagerList, laure2);

	proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, FALSE);
	linphone_proxy_config_done(proxyConfig);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationCleared,
	                             initialLaureStats.number_of_LinphoneRegistrationCleared + 1,
	                             liblinphone_tester_sip_timeout));

	// Current expected behavior is to have participant devices removed
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participant_devices_removed,
	                             initialMarieStats.number_of_chat_room_participant_devices_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_devices_removed,
	                             initialPaulineStats.number_of_chat_room_participant_devices_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_chat_room_participants_removed,
	                              initialMarieStats.number_of_chat_room_participants_removed + 1,
	                              liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                              initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                              liblinphone_tester_sip_timeout));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(laure2);
}

static void group_chat_room_admin_creator_leaves_and_is_reinvited(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie leaves the room
	linphone_chat_room_leave(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated,
	                             initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participants_removed,
	                             initialPaulineStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participants_removed,
	                             initialLaureStats.number_of_chat_room_participants_removed + 1,
	                             liblinphone_tester_sip_timeout));

	/* FIXME: the number of admin status changed shoud be 1. But the conference server notifies first that marie looses
	 * its admin status, before being deleted from the chatroom. This is indeed useless to notify this. Once fixed in
	 * the conference server, the counter shall be set back to +1. */
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialPaulineStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_chat_room_participant_admin_statuses_changed,
	                             initialLaureStats.number_of_chat_room_participant_admin_statuses_changed + 2,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(linphone_participant_is_admin(linphone_chat_room_get_me(laureCr)));

	linphone_core_delete_chat_room(marie->lc, marieCr);
	initialMarieStats = marie->stat;

	// Laure invites Marie to the chatroom
	participantsAddresses = NULL;
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(marie->lc)));
	linphone_chat_room_add_participants(laureCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Marie's side and that the participants are added
	marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats,
	                                               linphone_chat_room_get_conference_address(laureCr), initialSubject,
	                                               2, FALSE);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_forward_file_transfer_message_url(const char *file_transfer_server_url) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *receivePaulineFilepath = random_filepath("receive_file_pauline", "dump");
	char *receiveMarieFilepath = random_filepath("receive_file_marie", "dump");

	/* Remove any previously downloaded file */
	remove(receivePaulineFilepath);
	remove(receiveMarieFilepath);

	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	linphone_core_set_file_transfer_server(marie->lc, file_transfer_server_url);
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_server_url);

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneConferenceParams *conference_params = linphone_core_create_conference_params(marie->lc);
	linphone_conference_params_enable_chat(conference_params, TRUE);
	linphone_conference_params_enable_group(conference_params, FALSE);
	linphone_conference_params_set_subject(conference_params, initialSubject);
	linphone_conference_params_set_security_level(conference_params, LinphoneConferenceSecurityLevelNone);
	LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(conference_params);
	linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_params_set_ephemeral_mode(chat_params, LinphoneChatRoomEphemeralModeDeviceManaged);
	// Try search with the actual chat room subject
	BC_ASSERT_PTR_NOT_NULL(linphone_core_search_chat_room_2(marie->lc, conference_params, marie->identity, confAddr,
	                                                        participantsAddresses));

	linphone_conference_params_set_subject(conference_params, "Default one to one subject");
	// Try search with a different chat room subject. As it is a one-to-one, the subject comparison should be ignored
	BC_ASSERT_PTR_NOT_NULL(linphone_core_search_chat_room_2(marie->lc, conference_params, marie->identity, confAddr,
	                                                        participantsAddresses));
	linphone_conference_params_unref(conference_params);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	_send_file(marieCr, sendFilepath, NULL, FALSE);

	// Check pauline got it
	// Note: we must not use the buffer reception(last param to FALSE) or our message transfer won't work, probably some
	// fix to be done in the callback
	_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, NULL, FALSE);

	// Retrieve message from Pauline chatroom history
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int, " %i");
	if (linphone_chat_room_get_history_size(paulineCr) > 0) {
		bctbx_list_t *history = linphone_chat_room_get_history(paulineCr, 1);
		LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)(history->data);

		// Forward it to Marie
		LinphoneChatMessage *msg = linphone_chat_room_create_forward_message(paulineCr, recv_msg);
		const LinphoneAddress *forwarded_from_address = linphone_chat_message_get_from_address(recv_msg);
		char *forwarded_from = linphone_address_as_string_uri_only(forwarded_from_address);

		bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);

		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_send(msg);

		BC_ASSERT_TRUE(linphone_chat_message_is_forward(msg));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(msg), forwarded_from);

		// Check Marie received it and that the file is still the same
		_receive_file(coresList, marie, &initialMarieStats, receiveMarieFilepath, sendFilepath, NULL, FALSE);

		linphone_chat_message_unref(msg);
		ms_free(forwarded_from);
	} else {
		BC_FAIL("Could not get message to forward from history");
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int, "%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int, "%i");
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(pauline->lc));

	ms_free(sendFilepath);
	ms_free(receivePaulineFilepath);
	ms_free(receiveMarieFilepath);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_forward_file_transfer_message(void) {
	group_chat_forward_file_transfer_message_url(file_transfer_url);
}

static void group_chat_forward_file_transfer_message_digest_auth_server(void) {
	group_chat_forward_file_transfer_message_url(file_transfer_url_digest_auth);
}

static void group_chat_forward_file_transfer_message_digest_auth_server_encryptedFS(void) {
	uint8_t evfs_key[32] = {0xaa, 0x55, 0xFF, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde,
	                        0xf0, 0x11, 0x22, 0x33, 0x44, 0x5a, 0xa5, 0x5F, 0xaF, 0x52, 0xa4,
	                        0xa6, 0x58, 0xaa, 0x5c, 0xae, 0x50, 0xa1, 0x52, 0xa3, 0x54};
	linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, evfs_key,
	                                    32);

	group_chat_forward_file_transfer_message_url(file_transfer_url_digest_auth);

	linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_UNSET, NULL, 0);
}

static void group_chat_room_message_sync_between_devices_with_same_identity(void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");

	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marie1Cr =
	    create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject,
	                                 FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
	BC_ASSERT_PTR_NOT_NULL(marie1Cr);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marie1Cr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);

	// Check that the chat room is correctly created on Marie's side and that the participants are added
	LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr,
	                                                                  initialSubject, 1, FALSE);
	BC_ASSERT_PTR_NOT_NULL(marie2Cr);

	// Marie2 won't send 200OK to flexisip when receiving it's own outgoing message from marie1
	sal_set_send_error(linphone_core_get_sal(marie2->lc), -1); // to trash 200ok without generating error

	const char *marie1TextMessage = "Hum.";
	LinphoneChatMessage *marie1Message = _send_message(marie1Cr, marie1TextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                             initialMarie2Stats.number_of_LinphoneMessageReceived + 1,
	                             liblinphone_tester_sip_timeout));
	linphone_chat_message_unref(marie1Message);

	wait_for_list(coresList, NULL, 0, 1000);

	// Marie2 will now receive the message again but this time it will acknowledge it
	sal_set_send_error(linphone_core_get_sal(marie2->lc), 0);
	linphone_core_refresh_registers(marie2->lc);

	// Check that the duplicate is detected and we are not notified of it, not it is stored in DB
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived,
	                              initialMarie2Stats.number_of_LinphoneMessageReceived + 2,
	                              liblinphone_tester_sip_timeout));
	bctbx_list_t *messages = linphone_chat_room_get_history(marie2Cr, 0);
	size_t count = (bctbx_list_size(messages));
	BC_ASSERT_EQUAL(count, 1, size_t, "%zu");
	bctbx_list_free_with_data(messages, (bctbx_list_free_func)linphone_chat_message_unref);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	linphone_core_manager_delete_chat_room(marie2, marie2Cr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);

	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
}

test_t group_chat_tests[] = {
    TEST_NO_TAG("Chat room params", group_chat_room_params),
    TEST_ONE_TAG("Core restarts as soon as chat room is created", group_chat_room_creation_core_restart, "LeaksMemory"),
    TEST_NO_TAG("Chat room with forced local identity", group_chat_room_creation_with_given_identity),
    TEST_NO_TAG("Group chat room creation server", group_chat_room_creation_server),
    TEST_NO_TAG("Network down while creating group chat room", group_chat_room_creation_server_network_down),
    TEST_NO_TAG("Network down/up while creating group chat room", group_chat_room_creation_server_network_down_up),
    TEST_ONE_TAG("Add participant", group_chat_room_add_participant, "LeaksMemory"),
    TEST_NO_TAG("Send message", group_chat_room_send_message),
    TEST_NO_TAG("Send message IM encryption mandatory", group_chat_room_send_message_im_encryption_mandatory),
    TEST_NO_TAG("Send encrypted message", group_chat_room_send_message_encrypted),
    TEST_NO_TAG("Send encrypted message IM encryption mandatory",
                group_chat_room_send_message_encrypted_im_encryption_mandatory),
    TEST_NO_TAG("Send message with error", group_chat_room_send_message_with_error),
    TEST_NO_TAG("Send invite on a multi register account", group_chat_room_invite_multi_register_account),
    TEST_NO_TAG("Send message check sync between devices",
                group_chat_room_message_sync_between_devices_with_same_identity),
    TEST_NO_TAG("Add admin", group_chat_room_add_admin),
    TEST_NO_TAG("Add admin lately notified", group_chat_room_add_admin_lately_notified),
    TEST_NO_TAG("Add admin with a non admin", group_chat_room_add_admin_non_admin),
    TEST_NO_TAG("Remove admin", group_chat_room_remove_admin),
    TEST_NO_TAG("Admin creator leaves the room", group_chat_room_admin_creator_leaves_the_room),
    TEST_NO_TAG("Change subject", group_chat_room_change_subject),
    TEST_NO_TAG("Change subject with a non admin", group_chat_room_change_subject_non_admin),
    TEST_NO_TAG("Remove participant", group_chat_room_remove_participant),
    TEST_ONE_TAG("Remove participant and restart",
                 group_chat_room_remove_participant_and_restart,
                 "LeaksMemory"), // Due to core restart
    TEST_NO_TAG("Send message with a participant removed", group_chat_room_send_message_with_participant_removed),
    TEST_NO_TAG("Leave group chat room", group_chat_room_leave),
    TEST_NO_TAG("Delete group chat room successful if it's already removed by server", group_chat_room_delete_twice),
    TEST_NO_TAG("Come back on a group chat room after a disconnection", group_chat_room_come_back_after_disconnection),
    TEST_NO_TAG("Create chat room with disconnected friends", group_chat_room_create_room_with_disconnected_friends),
    TEST_NO_TAG("Create chat room with disconnected friends and initial message",
                group_chat_room_create_room_with_disconnected_friends_and_initial_message),
    TEST_NO_TAG("Reinvited after removed from group chat room", group_chat_room_reinvited_after_removed)};

test_t group_chat2_tests[] = {
    TEST_NO_TAG("Send file + text", group_chat_room_send_file_plus_text),
    TEST_NO_TAG("Send 2 files + text", group_chat_room_send_two_files_plus_text),
    TEST_NO_TAG("Send multipart message with custom content types",
                group_chat_room_send_multipart_custom_content_types),
    TEST_NO_TAG("Unique one-to-one chatroom", group_chat_room_unique_one_to_one_chat_room),
    TEST_NO_TAG("Unique one-to-one chatroom with dual sender device",
                group_chat_room_unique_one_to_one_chat_room_dual_sender_device),
    TEST_NO_TAG("Unique one-to-one chatroom recreated from message",
                group_chat_room_unique_one_to_one_chat_room_recreated_from_message),
    TEST_ONE_TAG("Unique one-to-one chatroom recreated from message with app restart",
                 group_chat_room_unique_one_to_one_chat_room_recreated_from_message_with_app_restart,
                 "LeaksMemory"),
    TEST_NO_TAG("Join one-to-one chat room with a new device",
                group_chat_room_join_one_to_one_chat_room_with_a_new_device),
    TEST_NO_TAG("New unique one-to-one chatroom after both participants left",
                group_chat_room_new_unique_one_to_one_chat_room_after_both_participants_left),
    TEST_NO_TAG("Unique one-to-one chatroom re-created from the party that deleted it, with inactive devices",
                group_chat_room_unique_one_to_one_chat_room_recreated_from_message_2),
    TEST_ONE_TAG("Group chat room notify participant devices name",
                 group_chat_room_participant_devices_name,
                 "LeaksMemory" /* Core restarts */),
    TEST_NO_TAG("Add device in one-to-one chat room where other participant left",
                add_device_one_to_one_chat_room_other_left),
    TEST_NO_TAG("IMDN for group chat room", imdn_for_group_chat_room),
    TEST_NO_TAG("Aggregated IMDN for group chat room", aggregated_imdn_for_group_chat_room),
    TEST_NO_TAG("Aggregated IMDN for group chat room read while offline",
                aggregated_imdn_for_group_chat_room_read_while_offline),
    TEST_ONE_TAG("IMDN sent from DB state", imdn_sent_from_db_state, "LeaksMemory"),
    TEST_NO_TAG("IMDN updated for group chat room with one participant offline",
                imdn_updated_for_group_chat_room_with_one_participant_offline),
    TEST_NO_TAG("Basic chat room with CPIM and GRUU", basic_chat_room_with_cpim_gruu),
    TEST_NO_TAG("Basic chat room with CPIM, GRUU and IMDN", basic_chat_room_with_cpim_gruu_imdn),
    TEST_NO_TAG("Basic chat room with CPIM and without GRUU", basic_chat_room_with_cpim_without_gruu),
    TEST_NO_TAG("Find one-to-one chat room", find_one_to_one_chat_room),
    TEST_NO_TAG("Exhumed one-to-one chat room 1", exhume_one_to_one_chat_room_1),
    TEST_NO_TAG("Exhumed one-to-one chat room 2", exhume_one_to_one_chat_room_2),
    TEST_NO_TAG("Exhumed one-to-one chat room 3", exhume_one_to_one_chat_room_3),
    TEST_ONE_TAG("Exhumed one-to-one chat room 3 with core restart",
                 exhume_one_to_one_chat_room_3_core_restart,
                 "LeaksMemory" /*due to core restart*/),
    TEST_NO_TAG("Exhumed one-to-one chat room 4", exhume_one_to_one_chat_room_4),
    TEST_NO_TAG("New device after group chat room creation", group_chat_room_new_device_after_creation),
    TEST_ONE_TAG("Chat room list subscription", group_chat_room_list_subscription, "LeaksMemory"),
    TEST_ONE_TAG(
        "Complex participant removal scenario", group_chat_room_complex_participant_removal_scenario, "LeaksMemory"),
    TEST_NO_TAG("Group chat room subscription denied", group_chat_room_subscription_denied),
    TEST_ONE_TAG("Search friend result chat room participants", search_friend_chat_room_participants, "MagicSearch"),
    TEST_ONE_TAG("Client loose context of a chatroom", group_chat_loss_of_client_context, "LeaksMemory")};

test_t group_chat3_tests[] = {
    TEST_ONE_TAG(
        "Participant removed then added", participant_removed_then_added, "LeaksMemory" /*due to core restart*/),
    TEST_ONE_TAG("Check if participant device are removed",
                 group_chat_room_join_one_to_one_chat_room_with_a_new_device_not_notified,
                 "LeaksMemory" /*due to core restart*/),
    TEST_ONE_TAG("Subscribe successful after set chat database path",
                 subscribe_test_after_set_chat_database_path,
                 "LeaksMemory" /*due to core restart*/),
    TEST_NO_TAG("Send forward message", one_to_one_chat_room_send_forward_message),
    TEST_ONE_TAG("Send forward message with restart",
                 one_to_one_chat_room_send_forward_message_with_restart,
                 "LeaksMemory" /*due to core restart*/),
    TEST_NO_TAG("Send reply message", one_to_one_chat_room_reply_forward_message),
    TEST_ONE_TAG("Send reply message with core restart",
                 one_to_one_chat_room_reply_forward_message_with_restart,
                 "LeaksMemory" /*due to core restart*/),
    TEST_ONE_TAG("Linphone core stop/start and chatroom ref",
                 core_stop_start_with_chat_room_ref,
                 "LeaksMemory" /*due to core restart*/),
    TEST_ONE_TAG("Subscribe successful after set chat database path",
                 subscribe_test_after_set_chat_database_path,
                 "LeaksMemory" /*due to core restart*/),
    TEST_NO_TAG("Make sure device unregistration does not trigger user to be removed from a group",
                group_chat_room_device_unregistered),
    TEST_NO_TAG("Admin leaves the room and is reinvited", group_chat_room_admin_creator_leaves_and_is_reinvited),
    TEST_NO_TAG("Forward file transfer message", group_chat_forward_file_transfer_message),
    TEST_NO_TAG("Forward file transfer message using digest auth on server",
                group_chat_forward_file_transfer_message_digest_auth_server),
    TEST_NO_TAG("Forward file transfer message using digest auth on server and encrypted FS",
                group_chat_forward_file_transfer_message_digest_auth_server_encryptedFS),
};

test_t group_chat4_tests[] = {
    TEST_ONE_TAG(
        "Reinvited after removed from group chat room 2", group_chat_room_reinvited_after_removed_2, "LeaksMemory"),
    TEST_ONE_TAG("Reinvited after removed from group chat room while offline",
                 group_chat_room_reinvited_after_removed_while_offline,
                 "LeaksMemory"),
    TEST_ONE_TAG("Reinvited after removed from group chat room while offline 2",
                 group_chat_room_reinvited_after_removed_while_offline_2,
                 "LeaksMemory"),
    TEST_NO_TAG("Reinvited after removed from group chat room with several devices",
                group_chat_room_reinvited_after_removed_with_several_devices),
    TEST_NO_TAG("Notify after disconnection", group_chat_room_notify_after_disconnection),
    TEST_ONE_TAG("Notify after core restart",
                 group_chat_room_notify_after_core_restart,
                 "LeaksMemory"), /* due to Core restart */
    TEST_NO_TAG("Send refer to all participants devices", group_chat_room_send_refer_to_all_devices),
    TEST_NO_TAG("Admin add device and doesn't lose admin status", group_chat_room_add_device),
    TEST_NO_TAG("Send multiple is composing", multiple_is_composing_notification),
    TEST_NO_TAG("Group chat room creation fails if invited participants don't support it",
                group_chat_room_creation_fails_if_invited_participants_dont_support_it),
    TEST_NO_TAG("Group chat room creation successful if at least one invited participant supports it",
                group_chat_room_creation_successful_if_at_least_one_invited_participant_supports_it),
    TEST_NO_TAG("Send file", group_chat_room_send_file),
    TEST_NO_TAG("Group chat with IMDN sent only to sender", group_chat_with_imdn_sent_only_to_sender),
    TEST_NO_TAG("Group chat with IMDN sent only to sender with participant offline",
                group_chat_with_imdn_sent_only_to_sender_with_participant_offline),
    TEST_NO_TAG("Group chat with IMDN sent only to sender after going over threshold",
                group_chat_with_imdn_sent_only_to_sender_after_going_over_threshold),
    TEST_NO_TAG("Send file using buffer", group_chat_room_send_file_2)};

test_suite_t group_chat_test_suite = {"Group Chat",
                                      NULL,
                                      NULL,
                                      liblinphone_tester_before_each,
                                      liblinphone_tester_after_each,
                                      sizeof(group_chat_tests) / sizeof(group_chat_tests[0]),
                                      group_chat_tests,
                                      0};

test_suite_t group_chat2_test_suite = {"Group Chat2",
                                       NULL,
                                       NULL,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(group_chat2_tests) / sizeof(group_chat2_tests[0]),
                                       group_chat2_tests,
                                       0};

test_suite_t group_chat3_test_suite = {"Group Chat3",
                                       NULL,
                                       NULL,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(group_chat3_tests) / sizeof(group_chat3_tests[0]),
                                       group_chat3_tests,
                                       0};

test_suite_t group_chat4_test_suite = {"Group Chat4",
                                       NULL,
                                       NULL,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(group_chat4_tests) / sizeof(group_chat4_tests[0]),
                                       group_chat4_tests,
                                       0};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
