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
#include "linphone/api/c-types.h"
#include "linphone/api/c-chat-room-params.h"
#include "tester_utils.h"
#include "linphone/wrapper_utils.h"
#include "liblinphone_tester.h"
#include "bctoolbox/crypto.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

const char * sFactoryUri = "sip:conference-factory@conf.example.org";

static void chat_room_is_composing_received (LinphoneChatRoom *cr, const LinphoneAddress *remoteAddr, bool_t isComposing) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	if (isComposing)
		manager->stat.number_of_LinphoneIsComposingActiveReceived++;
	else
		manager->stat.number_of_LinphoneIsComposingIdleReceived++;
}

static void undecryptable_message_received (LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	get_stats(linphone_chat_room_get_core(room))->number_of_LinphoneMessageUndecryptable++;
}

static void chat_room_participant_added (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participants_added++;
}

static void chat_room_participant_admin_status_changed (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_admin_statuses_changed++;
}

static void chat_room_participant_removed (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participants_removed++;
}

static void chat_room_participant_device_added (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_devices_added++;
}

static void chat_room_participant_device_removed (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_participant_devices_removed++;
}


static void chat_room_state_changed (LinphoneChatRoom *cr, LinphoneChatRoomState newState) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	const LinphoneAddress *addr = linphone_chat_room_get_conference_address(cr);

	if (addr) {
		char *addr_str = linphone_address_as_string(addr);
		ms_message("ChatRoom [%s] state changed: %d", addr_str, newState);
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
			ms_error("Invalid ChatRoom state for Chatroom [%s] EndOfEnum is used ONLY as a guard", linphone_address_as_string(addr));
			break;
	}
}

static void chat_room_security_event (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
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

static void chat_room_subject_changed (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_subject_changed++;
}

static void chat_room_conference_joined (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneChatRoomConferenceJoined++;
}

static void chat_room_message_ephemeral_started (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneChatRoomEphemeralTimerStarted++;
}

static void chat_room_message_ephemeral_deleted (LinphoneChatRoom *cr, const LinphoneEventLog *event_log) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_LinphoneChatRoomEphemeralDeleted++;
}

void core_chat_room_state_changed (LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	if (state == LinphoneChatRoomStateInstantiated) {
		LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
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
		linphone_chat_room_cbs_set_ephemeral_message_timer_started(cbs, chat_room_message_ephemeral_started);
		linphone_chat_room_cbs_set_ephemeral_message_deleted(cbs, chat_room_message_ephemeral_deleted);

		linphone_chat_room_add_callbacks(cr, cbs);
		linphone_chat_room_cbs_unref(cbs);
	}
}

void core_chat_room_subject_changed (LinphoneCore *core, LinphoneChatRoom *cr) {
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	manager->stat.number_of_core_chat_room_subject_changed++;
}

void configure_core_for_conference (LinphoneCore *core, const char* username, const LinphoneAddress *factoryAddr, bool_t server) {
	const char *identity = linphone_core_get_identity(core);
	const char *new_username;
	LinphoneAddress *addr = linphone_address_new(identity);
	if (!username) {
		new_username = linphone_address_get_username(addr);
	}
	linphone_address_set_username(addr, (username) ? username : new_username);
	char *newIdentity = linphone_address_as_string_uri_only(addr);
	linphone_address_unref(addr);
	linphone_core_set_primary_contact(core, newIdentity);
	bctbx_free(newIdentity);
	linphone_core_enable_conference_server(core, server);
	char *factoryUri = linphone_address_as_string(factoryAddr);
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(core);
	linphone_proxy_config_set_conference_factory_uri(proxy, factoryUri);
	bctbx_free(factoryUri);
}

void _configure_core_for_conference (LinphoneCoreManager *lcm, LinphoneAddress *factoryAddr) {
	configure_core_for_conference(lcm->lc, NULL, factoryAddr, FALSE);
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
		linphone_chat_message_cbs_set_ephemeral_message_timer_started(msgCbs, liblinphone_tester_chat_message_ephemeral_timer_started);
		linphone_chat_message_cbs_set_ephemeral_message_deleted(msgCbs, liblinphone_tester_chat_message_ephemeral_deleted);
	}
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

	BC_ASSERT_EQUAL(read, file_size, int, "%d");
	linphone_content_set_buffer(content, buf, file_size);
	linphone_content_set_size(content, file_size); /*total size to be transfered*/
	fclose(file_to_send);
}

void _send_file_plus_text(LinphoneChatRoom* cr, const char *sendFilepath, const char *sendFilepath2, const char *text, bool_t use_buffer) {
	LinphoneChatMessage *msg;

	LinphoneChatMessageCbs *cbs;
	LinphoneContent *content = linphone_core_create_content(linphone_chat_room_get_core(cr));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content,"video");
	linphone_content_set_subtype(content,"mkv");
	linphone_content_set_name(content,"sintel_trailer_opus_h264.mkv");
	if (use_buffer) {
		fill_content_buffer(content, sendFilepath);
	} else {
		linphone_content_set_file_path(content, sendFilepath);
	}

	msg = linphone_chat_room_create_empty_message(cr);
	linphone_chat_message_add_file_content(msg, content);
	linphone_content_unref(content);

	if (text)
		linphone_chat_message_add_utf8_text_content(msg, text);

	if (sendFilepath2) {
		LinphoneContent *content2 = linphone_core_create_content(linphone_chat_room_get_core(cr));
		belle_sip_object_set_name(BELLE_SIP_OBJECT(content2), "ahbahouaismaisbon content");
		linphone_content_set_type(content2,"audio");
		linphone_content_set_subtype(content2,"wav");
		linphone_content_set_name(content2,"ahbahouaismaisbon.wav");
		if (use_buffer) {
			fill_content_buffer(content2, sendFilepath2);
		} else {
			linphone_content_set_file_path(content2, sendFilepath2);
		}
		linphone_chat_message_add_file_content(msg, content2);
		linphone_content_unref(content2);
	}

	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
}

void _send_file(LinphoneChatRoom* cr, const char *sendFilepath, const char *sendFilepath2, bool_t use_buffer) {
	_send_file_plus_text(cr, sendFilepath, sendFilepath2, NULL, use_buffer);
}

void _receive_file_plus_text(bctbx_list_t *coresList, LinphoneCoreManager *lcm, stats *receiverStats, const char *receive_filepath, const char *sendFilepath, const char *sendFilepath2, const char *text, bool_t use_buffer) {
	if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageReceivedWithFile, receiverStats->number_of_LinphoneMessageReceivedWithFile + 1, 10000))) {
		LinphoneChatMessageCbs *cbs;
		LinphoneChatMessage *msg = lcm->stat.last_received_chat_message;
		char *downloaded_file;
		if (use_buffer) {
			downloaded_file = bc_tester_file("receive_file.dump");
		} else {
			downloaded_file = ms_strdup(receive_filepath);
		}

		if (text) {
			BC_ASSERT_TRUE(linphone_chat_message_has_text_content(msg));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(msg), text);
		}

		cbs = linphone_chat_message_get_callbacks(msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);

		BC_ASSERT_PTR_NOT_NULL(linphone_chat_message_get_external_body_url(msg));
		LinphoneContent *fileTransferContent = linphone_chat_message_get_file_transfer_information(msg);
		BC_ASSERT_TRUE(linphone_content_is_file_transfer(fileTransferContent));
		if (!use_buffer) {
			linphone_content_set_file_path(fileTransferContent, downloaded_file);
		}
		linphone_chat_message_download_content(msg, fileTransferContent);
		BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

		if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageFileTransferDone,receiverStats->number_of_LinphoneMessageFileTransferDone + 1, 20000))) {
			compare_files(sendFilepath, downloaded_file);
		}

		if (sendFilepath2) {
			remove(downloaded_file);
			LinphoneContent *fileTransferContent2 = linphone_chat_message_get_file_transfer_information(msg);
			BC_ASSERT_TRUE(linphone_content_is_file_transfer(fileTransferContent2));
			if (!use_buffer) {
				linphone_content_set_file_path(fileTransferContent2, downloaded_file);
			}
			linphone_chat_message_download_content(msg, fileTransferContent2);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

			if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageFileTransferDone,receiverStats->number_of_LinphoneMessageFileTransferDone + 2, 20000))) {
				compare_files(sendFilepath2, downloaded_file);
			}
		}
		bc_free(downloaded_file);
	}
}

void _receive_file(bctbx_list_t *coresList, LinphoneCoreManager *lcm, stats *receiverStats, const char *receive_filepath, const char *sendFilepath, const char *sendFilepath2, bool_t use_buffer) {
	_receive_file_plus_text(coresList, lcm, receiverStats, receive_filepath, sendFilepath, sendFilepath2, NULL, use_buffer);
}

// Configure list of core manager for conference and add the listener
bctbx_list_t * init_core_for_conference(bctbx_list_t *coreManagerList) {
	LinphoneAddress *factoryAddr = linphone_address_new(sFactoryUri);
	bctbx_list_for_each2(coreManagerList, (void (*)(void *, void *))_configure_core_for_conference, (void *) factoryAddr);
	linphone_address_unref(factoryAddr);

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
	linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
	bctbx_list_for_each2(coreManagerList, (void (*)(void *, void *))configure_core_for_callbacks, (void *) cbs);
	linphone_core_cbs_unref(cbs);

	bctbx_list_t *coresList = NULL;
	bctbx_list_t *item = coreManagerList;
	for (item = coreManagerList; item; item = bctbx_list_next(item))
		coresList = bctbx_list_append(coresList, ((LinphoneCoreManager *)(bctbx_list_get_data(item)))->lc);
	return coresList;
}

 void start_core_for_conference(bctbx_list_t *coreManagerList) {
	bctbx_list_for_each(coreManagerList, (void (*)(void *))_start_core);
}

static LinphoneChatRoom * check_has_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, const LinphoneAddress *confAddr, const char* subject, int participantNumber, bool_t isAdmin) {
    char *deviceIdentity = linphone_core_get_device_identity(lcm->lc);
    LinphoneAddress *localAddr = linphone_address_new(deviceIdentity);
    bctbx_free(deviceIdentity);
    LinphoneChatRoom *chatRoom = linphone_core_find_chat_room(lcm->lc, confAddr, localAddr);
    linphone_address_unref(localAddr);
    BC_ASSERT_PTR_NOT_NULL(chatRoom);
    if (chatRoom) {
        BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom), participantNumber, int, "%d");
        LinphoneParticipant *participant = linphone_chat_room_get_me(chatRoom);
        BC_ASSERT_PTR_NOT_NULL(participant);
        if (!(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesOneToOne))
            BC_ASSERT(isAdmin == linphone_participant_is_admin(participant));
        BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), subject);
    }
    return chatRoom;
}

LinphoneChatRoom * check_creation_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, const LinphoneAddress *confAddr, const char* subject, int participantNumber, bool_t isAdmin) {
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats->number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated, initialStats->number_of_LinphoneChatRoomStateCreated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined, initialStats->number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
    return check_has_chat_room_client_side(lcs, lcm, initialStats, confAddr, subject, participantNumber, isAdmin);
}

LinphoneChatRoom * create_chat_room_client_side_with_expected_number_of_participants(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, bctbx_list_t *participantsAddresses, const char* initialSubject, int expectedParticipantSize, bool_t encrypted) {
	int participantsAddressesSize = (int)bctbx_list_size(participantsAddresses);
	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(lcm->lc);

	linphone_chat_room_params_enable_encryption(params, encrypted);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, participantsAddressesSize > 1 ? TRUE : FALSE);
	LinphoneChatRoom *chatRoom = linphone_core_create_chat_room_2(lcm->lc, params, initialSubject, participantsAddresses);
	linphone_chat_room_params_unref(params);

	if (!chatRoom) return NULL;

	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateInstantiated, initialStats->number_of_LinphoneChatRoomStateInstantiated + 1, 100));
	if (encrypted) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chatRoom), LinphoneChatRoomSecurityLevelEncrypted, LinphoneChatRoomSecurityLevel, "%i");
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesEncrypted);
	}

	// Check that the chat room is correctly created on Marie's side and that the participants are added
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats->number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated, initialStats->number_of_LinphoneChatRoomStateCreated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined, initialStats->number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom),
		(expectedParticipantSize >= 0) ? expectedParticipantSize : participantsAddressesSize, int, "%d");
	LinphoneParticipant *participant = linphone_chat_room_get_me(chatRoom);
	BC_ASSERT_PTR_NOT_NULL(participant);
	if (participant)
		BC_ASSERT_TRUE(linphone_participant_is_admin(participant));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), initialSubject);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	return chatRoom;
}

LinphoneChatRoom *create_chat_room_with_params(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, bctbx_list_t *participantsAddresses, const char* initialSubject, LinphoneChatRoomParams *params) {
	int participantsAddressesSize = (int)bctbx_list_size(participantsAddresses);

	if (!params) return NULL;

	LinphoneChatRoom *chatRoom = linphone_core_create_chat_room_2(lcm->lc, params, initialSubject, participantsAddresses);

	if (!chatRoom) return NULL;

	if (linphone_chat_room_params_encryption_enabled(params))
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesEncrypted);

	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateInstantiated, initialStats->number_of_LinphoneChatRoomStateInstantiated + 1, 100));

	if (linphone_chat_room_params_get_backend(params) == LinphoneChatRoomBackendFlexisipChat) {
		// Check that the chat room is correctly created on Marie's side and that the participants are added
		BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats->number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated, initialStats->number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined, initialStats->number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom), participantsAddressesSize, int, "%d");
		LinphoneParticipant *participant = linphone_chat_room_get_me(chatRoom);
		BC_ASSERT_PTR_NOT_NULL(participant);
		if (participant)
			BC_ASSERT_TRUE(linphone_participant_is_admin(participant));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), initialSubject);
	}
	return chatRoom;
}

LinphoneChatRoom * create_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, bctbx_list_t *participantsAddresses, const char* initialSubject, bool_t encrypted) {
	return create_chat_room_client_side_with_expected_number_of_participants(lcs, lcm, initialStats, participantsAddresses, initialSubject, (int) bctbx_list_size(participantsAddresses), encrypted);
}

static void group_chat_room_params (void) {
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

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));

	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);

	//Should create	a basic chatroom
	linphone_chat_room_params_enable_encryption(params, FALSE);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);
	linphone_chat_room_params_enable_group(params, FALSE);
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Basic chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_PTR_NOT_NULL(linphone_chat_room_get_current_params(marieCr));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	//Should create	a one-to-one flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, FALSE);
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "One to one client group chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	//Should create	a group flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Group chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	//Should create	an encrypted group flexisip chat
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	linphone_chat_room_params_enable_group(params, TRUE);
	linphone_chat_room_params_enable_encryption(params, TRUE);
	//Check that enabling encryption also defines a valid encryptionBackend
	BC_ASSERT_EQUAL(linphone_chat_room_params_get_encryption_backend(params), LinphoneChatRoomEncryptionBackendLime, int, "%d");
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Encrypted group chat room subject", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesEncrypted);
		BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	//Should return NULL because params are invalid
	linphone_chat_room_params_enable_group(params, TRUE);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);  //Group + basic backend = invalid
	BC_ASSERT_FALSE(linphone_chat_room_params_is_valid(params));
	marieCr = linphone_core_create_chat_room_2(marie->lc, params, "Invalid chat room subject", NULL);
	BC_ASSERT_PTR_NULL(marieCr);

	//Should set FlexisipChat as backend if encryption is enabled.
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendBasic);
	linphone_chat_room_params_enable_encryption(params, TRUE);
	BC_ASSERT_EQUAL(linphone_chat_room_params_get_backend(params), LinphoneChatRoomBackendFlexisipChat, int, "%d");

	//Cleanup
	linphone_chat_room_params_unref(params);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
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

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;

	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc)), 2, int, "%d");

	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *secondProxy = linphone_core_get_proxy_config_list(marie->lc)->next->data;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_proxy_config_get_identity(proxy));
	LinphoneAddress *marieSecondAddr = linphone_address_new(linphone_proxy_config_get_identity(secondProxy));

	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie->lc);
	LinphoneChatRoom *marieCr = linphone_core_create_chat_room(marie->lc, params, marieAddr, "first chat room", participantsAddresses);
	BC_ASSERT_PTR_NOT_NULL(marieCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));

	const LinphoneAddress *localAddr = linphone_chat_room_get_local_address(marieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(localAddr, marieAddr));

	LinphoneChatRoom *secondMarieCr = linphone_core_create_chat_room(marie->lc, params, marieSecondAddr, "second chat room", participantsAddresses);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));

	localAddr = linphone_chat_room_get_local_address(secondMarieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(localAddr, marieSecondAddr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(marie, secondMarieCr, coresList);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	linphone_address_unref(marieAddr);
	linphone_address_unref(marieSecondAddr);
	linphone_chat_room_params_unref(params);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_creation_server (void) {
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

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Pauline tries to change the subject but is not admin so it fails
	const char *newSubject = "Let's go drink a beer";
	linphone_chat_room_set_subject(paulineCr, newSubject);
	wait_for_list(coresList, &dummy, 1, 1000);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), initialSubject);

	// Marie now changes the subject
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, newSubject, 3, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	// Pauline revokes the admin status of Marie
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, marieParticipant, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_FALSE(linphone_participant_is_admin(marieParticipant));

	// Marie tries to change the subject again but is not admin, so it is not changed
	linphone_chat_room_set_subject(marieCr, initialSubject);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Chloe begins composing a message
	BC_ASSERT_PTR_NOT_NULL(chloeCr);
	linphone_chat_room_compose(chloeCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, initialLaureStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDelivered, initialChloeStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, initialLaureStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));

	LinphoneAddress *chloeAddr;
	if (BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message)) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), chloeTextMessage);
		linphone_chat_message_unref(chloeMessage);
		chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marie->stat.last_received_chat_message)));
		linphone_address_unref(chloeAddr);
	}

	// Pauline removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(paulineCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(paulineCr, laureParticipant);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participants_removed, initialChloeStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

	// Pauline removes Marie and Chloe from the chat room
	marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	LinphoneParticipant *chloeParticipant = linphone_chat_room_find_participant(paulineCr, chloeAddr);
	linphone_address_unref(chloeAddr);
	BC_ASSERT_PTR_NOT_NULL(chloeParticipant);
	bctbx_list_t *participantsToRemove = NULL;
	initialPaulineStats = pauline->stat;
	participantsToRemove = bctbx_list_append(participantsToRemove, marieParticipant);
	participantsToRemove = bctbx_list_append(participantsToRemove, chloeParticipant);
	linphone_chat_room_remove_participants(paulineCr, participantsToRemove);
	bctbx_list_free_with_data(participantsToRemove, (bctbx_list_free_func)linphone_participant_unref);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminated, initialChloeStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 2, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 0, int, "%d");

	// Pauline leaves the chat room
	wait_for_list(coresList, &dummy, 1, 1000);
	linphone_chat_room_leave(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminationPending, initialPaulineStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(laure->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(chloe->lc), 0, int,"%i");

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_add_participant (void) {
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

	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(marie->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Pauline creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *paulineCr = create_chat_room_client_side(coresList, pauline, &initialPaulineStats, participantsAddresses, initialSubject, FALSE);
	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(paulineCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

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
	paulineCr = linphone_core_find_chat_room(pauline->lc, paulineAddr, NULL);
	linphone_address_unref(paulineAddr);

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Refused by server because group chat disabled for Chloe
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");

	// Pauline begins composing a message
	linphone_chat_room_compose(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));

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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(laure->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(chloe->lc), 0, int,"%i");

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static int im_encryption_engine_process_incoming_message_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	if (linphone_chat_message_get_content_type(msg)) {
		if (strcmp(linphone_chat_message_get_content_type(msg), "cipher/b64") == 0) {
			size_t b64Size = 0;
			unsigned char *output;
			const char * msg_str = linphone_chat_message_get_text(msg);
			bctbx_base64_decode(NULL, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
			output = (unsigned char *)ms_malloc(b64Size+1),
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

static int im_encryption_engine_process_outgoing_message_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	if (strcmp(linphone_chat_message_get_content_type(msg),"message/cpim") == 0) {
		size_t b64Size = 0;
		unsigned char *output;
		const char * msg_str = linphone_chat_message_get_text(msg);
		bctbx_base64_encode(NULL, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
		output = (unsigned char *)ms_malloc0(b64Size+1);
		bctbx_base64_encode(output, &b64Size, (unsigned char *)msg_str, strlen(msg_str));
		output[b64Size] = '\0';
		linphone_chat_message_set_text(msg,(const char*)output);
		ms_free(output);
		linphone_chat_message_set_content_type(msg, "cipher/b64");
		return 0;
	}
	return -1;
}

static void group_chat_room_message (bool_t encrypt, bool_t sal_error) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	LinphoneImEncryptionEngine *marie_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *marie_cbs = linphone_im_encryption_engine_get_callbacks(marie_imee);
	LinphoneImEncryptionEngine *pauline_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *pauline_cbs = linphone_im_encryption_engine_get_callbacks(pauline_imee);
	LinphoneImEncryptionEngine *chloe_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *chloe_cbs = linphone_im_encryption_engine_get_callbacks(chloe_imee);

	if (encrypt) {
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(marie_cbs, im_encryption_engine_process_outgoing_message_cb);
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(pauline_cbs, im_encryption_engine_process_outgoing_message_cb);
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(chloe_cbs, im_encryption_engine_process_outgoing_message_cb);

		linphone_im_encryption_engine_cbs_set_process_incoming_message(marie_cbs, im_encryption_engine_process_incoming_message_cb);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(pauline_cbs, im_encryption_engine_process_incoming_message_cb);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(chloe_cbs, im_encryption_engine_process_incoming_message_cb);

		linphone_core_set_im_encryption_engine(marie->lc, marie_imee);
		linphone_core_set_im_encryption_engine(pauline->lc, pauline_imee);
		linphone_core_set_im_encryption_engine(chloe->lc, chloe_imee);
	}

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Chloe begins composing a message
	linphone_chat_room_compose(chloeCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	linphone_chat_message_unref(chloeMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));

	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Pauline begins composing a messagewith some accents
	linphone_chat_room_compose(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingActiveReceived, initialChloeStats.number_of_LinphoneIsComposingActiveReceived + 1, 5000));
	const char *paulineTextMessage = "Héllö Dàrling";
	LinphoneChatMessage *paulineMessage = _send_message(paulineCr, paulineTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived, initialChloeStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingIdleReceived, initialChloeStats.number_of_LinphoneIsComposingIdleReceived + 1, 5000));
	marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineTextMessage);
	linphone_chat_message_unref(paulineMessage);
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(paulineAddr);

	if (sal_error) {
		sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
		LinphoneChatMessage* msg = _send_message(marieCr, "Bli bli bli");
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneRegistrationOk, initialMarieStats.number_of_LinphoneRegistrationOk + 1, 10000));

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
	group_chat_room_message(FALSE, FALSE);
}

static void group_chat_room_send_message_encrypted(void) {
	group_chat_room_message(TRUE, FALSE);
}

static void group_chat_room_send_message_with_error(void) {
	group_chat_room_message(FALSE, TRUE);
}

static void group_chat_room_invite_multi_register_account (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline1's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Pauline2's side and that the participants are added
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

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

static void group_chat_room_add_admin (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

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

static void group_chat_room_add_admin_lately_notified (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Simulate pauline has disappeared
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));

	// Make sure pauline is not notified
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 3000));
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

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



static void group_chat_room_add_admin_non_admin (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Pauline designates Laure as admin
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(paulineCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, laureParticipant, TRUE);
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipant));

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

static void group_chat_room_remove_admin (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

	// Pauline revokes the admin status of Marie
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, marieParticipant, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_FALSE(linphone_participant_is_admin(marieParticipant));

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

static void group_chat_room_admin_creator_leaves_the_room (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie leaves the room
	linphone_chat_room_leave(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(linphone_chat_room_get_me(laureCr)));

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

static void group_chat_room_change_subject (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	const char *newSubject = "New subject";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie now changes the subject
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_core_chat_room_subject_changed, initialMarieStats.number_of_core_chat_room_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_core_chat_room_subject_changed, initialPaulineStats.number_of_core_chat_room_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_core_chat_room_subject_changed, initialLaureStats.number_of_core_chat_room_subject_changed + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

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

static void group_chat_room_change_subject_non_admin (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	const char *newSubject = "New subject";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie now changes the subject
	linphone_chat_room_set_subject(paulineCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), initialSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), initialSubject);

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

static void group_chat_room_remove_participant (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

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

static void group_chat_room_send_message_with_participant_removed (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	if(!BC_ASSERT_PTR_NOT_NULL(laureParticipant))
		goto end;

	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

	// Laure try to send a message with the chat room where she was removed
	const char *laureTextMessage = "Hello";
	LinphoneChatMessage *laureMessage = _send_message(laureCr, laureTextMessage);
	linphone_chat_message_unref(laureMessage);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageDelivered, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, initialPaulineStats.number_of_LinphoneIsComposingIdleReceived, 5000));

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

static void group_chat_room_leave (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	linphone_chat_room_leave(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 5000));

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

static void group_chat_room_delete_twice (void) {
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
    participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
    participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
    stats initialMarieStats = marie->stat;
    stats initialPaulineStats = pauline->stat;
    stats initialLaureStats = laure->stat;

    // Marie creates a new group chat room
    const char *initialSubject = "Colleagues";
    LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

    const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

    // Check that the chat room is correctly created on Pauline's side and that the participants are added
    LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

    // Check that the chat room is correctly created on Laure's side and that the participants are added
    LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

    // Save db
    const char *uri = linphone_config_get_string(linphone_core_get_config(laure->lc), "storage", "uri", "");
    char *uriCopy = bc_tester_file("linphone_tester.db");
    BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopy));

    // Clean db from chat room
    linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

    // Reset db
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

static void group_chat_room_come_back_after_disconnection (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	const char *newSubject = "New subject";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	linphone_core_set_network_reachable(marie->lc, FALSE);

	wait_for_list(coresList, &dummy, 1, 1000);

	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

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

static void group_chat_room_create_room_with_disconnected_friends_base (bool_t initial_message) {
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

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	if (initial_message) {
		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "Salut");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
	}

	wait_for_list(coresList, &dummy, 1, 4000);

	// Reconnect Pauline and check that the chat room is correctly created on Pauline's side and that the participants are added
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr))
			goto end;

	// Reconnect Laure and check that the chat room is correctly created on Laure's side and that the participants are added
	linphone_core_set_network_reachable(laure->lc, TRUE);
	laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr))
		goto end;

	if (initial_message) {
		if (BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000))) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "Salut");
				linphone_chat_message_unref(msg);
			}
		}
		if (BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, 1, 5000))) {
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

static void group_chat_room_create_room_with_disconnected_friends (void) {
	group_chat_room_create_room_with_disconnected_friends_base(FALSE);
}

static void group_chat_room_create_room_with_disconnected_friends_and_initial_message (void) {
	group_chat_room_create_room_with_disconnected_friends_base(TRUE);
}

static void group_chat_room_reinvited_after_removed_base (bool_t offline_when_removed, bool_t offline_when_reinvited, bool_t restart_after_reinvited) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	char *savedLaureUuid = NULL;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	if (offline_when_removed) {
		savedLaureUuid = bctbx_strdup(linphone_config_get_string(linphone_core_get_config(laure->lc), "misc", "uuid", NULL));
		coresList = bctbx_list_remove(coresList, laure->lc);
		coresManagerList = bctbx_list_remove(coresManagerList, laure);
		linphone_core_set_network_reachable(laure->lc, FALSE);
		linphone_core_manager_stop(laure);
	}

	// Marie removes Laure from the chat room
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

	if (offline_when_removed && !offline_when_reinvited) {
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
	if (!offline_when_reinvited)
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

	wait_for_list(coresList,0, 1, 2000);
	initialLaureStats = laure->stat;

	// Marie adds Laure to the chat room
	participantsAddresses = bctbx_list_append(participantsAddresses, laureAddr);
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
	if (offline_when_reinvited) {
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreationPending, initialLaureStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreated, initialLaureStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	char *laureIdentity = linphone_core_get_device_identity(laure->lc);
	laureAddr = linphone_address_new(laureIdentity);
	bctbx_free(laureIdentity);
	LinphoneChatRoom *newLaureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureAddr);
	linphone_address_unref(laureAddr);
	if (!offline_when_removed)
		BC_ASSERT_PTR_EQUAL(newLaureCr, laureCr);
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

	if (restart_after_reinvited) {
		coresList = bctbx_list_remove(coresList, laure->lc);
		linphone_core_manager_reinit(laure);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(laure, TRUE);
		laureIdentity = linphone_core_get_device_identity(laure->lc);
		laureAddr = linphone_address_new(laureIdentity);
		newLaureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureAddr);
		linphone_address_unref(laureAddr);
		wait_for_list(coresList,0, 1, 2000);
		BC_ASSERT_FALSE(linphone_chat_room_has_been_left(newLaureCr));

		unsigned int nbLaureConferenceCreatedEventsAfterRestart = 0;
		bctbx_list_t *laureHistory = linphone_chat_room_get_history_events(newLaureCr, 0);
		for (bctbx_list_t *item = laureHistory; item; item = bctbx_list_next(item)) {
			LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
			if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceCreated)
				nbLaureConferenceCreatedEventsAfterRestart++;
		}
		bctbx_list_free_with_data(laureHistory, (bctbx_list_free_func)linphone_event_log_unref);
		BC_ASSERT_EQUAL(nbLaureConferenceCreatedEventsAfterRestart, nbLaureConferenceCreatedEventsBeforeRestart, unsigned int, "%u");
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, newLaureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_reinvited_after_removed (void) {
	group_chat_room_reinvited_after_removed_base(FALSE, FALSE, FALSE);
}

static void group_chat_room_reinvited_after_removed_2 (void) {
	group_chat_room_reinvited_after_removed_base(FALSE, FALSE, TRUE);
}

static void group_chat_room_reinvited_after_removed_while_offline (void) {
	group_chat_room_reinvited_after_removed_base(TRUE, FALSE, FALSE);
}

static void group_chat_room_reinvited_after_removed_while_offline_2 (void) {
	group_chat_room_reinvited_after_removed_base(TRUE, TRUE, FALSE);
}

static void group_chat_room_reinvited_after_removed_with_several_devices (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie2 is initially offline
	linphone_core_set_network_reachable(marie2->lc, FALSE);

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marie1Cr);

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	LinphoneChatRoom *pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie1 removes Pauline from the chat room while Pauline2 is offline
	linphone_core_set_network_reachable(pauline2->lc, FALSE);
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline1->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marie1Cr, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_remove_participant(marie1Cr, paulineParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateTerminated, initialPauline1Stats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

	// Marie2 comes online, check that pauline is not notified as still being in the chat room
	linphone_core_set_network_reachable(marie2->lc, TRUE);
	LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, TRUE);

	// Marie2 adds Pauline back to the chat room
	initialPauline1Stats = pauline1->stat;
	participantsAddresses = bctbx_list_append(participantsAddresses, paulineAddr);
	linphone_chat_room_add_participants(marie2Cr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_added, initialMarie1Stats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_participants_added, initialMarie2Stats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie1Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie2Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");
	LinphoneChatRoom *newPauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_EQUAL(newPauline1Cr, pauline1Cr);
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newPauline1Cr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newPauline1Cr), initialSubject);

	// Clean chat room from db
	linphone_core_set_network_reachable(pauline2->lc, TRUE);
	linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	linphone_core_delete_chat_room(marie2->lc, marie2Cr);
	initialPauline2Stats = pauline2->stat;
	linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateTerminated, initialPauline2Stats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
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

static void group_chat_room_notify_after_disconnection (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_PTR_NOT_NULL(marieCr);

	participantsAddresses = NULL;

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_NOT_NULL(laureCr);

	// Marie now changes the subject
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	newSubject = "Let's go drink a beer";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 2, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 2, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_NOT_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 2, 5000));

	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);

	// Test with more than one missed notify
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	newSubject = "Let's go drink a mineral water !";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 3, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 3, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 3, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant));
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipantFromPauline));

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 3, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipantFromPauline));

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

static void group_chat_room_send_refer_to_all_devices (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on second Marie's device
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Check that added Marie's device didn't change her admin status
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie1->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_get_me(marieCr);
	if(BC_ASSERT_PTR_NOT_NULL(marieParticipant))
		BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_get_me(marieCr2);
	if(BC_ASSERT_PTR_NOT_NULL(marieParticipant))
		BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	if(BC_ASSERT_PTR_NOT_NULL(marieParticipant))
		BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_find_participant(paulineCr2, marieAddr);
	if(BC_ASSERT_PTR_NOT_NULL(marieParticipant))
		BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	marieParticipant = linphone_chat_room_find_participant(laureCr, marieAddr);
	if(BC_ASSERT_PTR_NOT_NULL(marieParticipant))
		BC_ASSERT_TRUE(linphone_participant_is_admin(marieParticipant));

	linphone_address_unref(marieAddr);

	// Marie removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_participants_removed, initialMarie2Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participants_removed, initialPauline1Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participants_removed, initialPauline2Stats.number_of_participants_removed + 1, 5000));

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

static void group_chat_room_add_device (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

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
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 5000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_participants_removed, initialMarie2Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participants_removed, initialPauline1Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participants_removed, initialPauline2Stats.number_of_participants_removed + 1, 5000));

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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Only one is composing
	linphone_chat_room_compose(paulineCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, 1, 5000));

	// Laure side
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity));
	}

	// Marie side
	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity));
	}

	// Pauline side
	composing_addresses = linphone_chat_room_get_composing_addresses(paulineCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");

	wait_for_list(coresList,0, 1, 1500);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, 1, 5000));

	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");

	// multiple is composing
	linphone_chat_room_compose(paulineCr);
	linphone_chat_room_compose(marieCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, 3, 5000)); // + 2
	// Laure side
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 2, int, "%i");
	if (bctbx_list_size(composing_addresses) == 2) {
			while (composing_addresses) {
				LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
				bool_t equal = strcmp(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity)) == 0
							|| strcmp(linphone_address_get_username(addr), linphone_address_get_username(marie->identity)) == 0;
				BC_ASSERT_TRUE(equal);
				composing_addresses = bctbx_list_next(composing_addresses);
			}
	}

	// Marie side
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 2, 5000)); // + 1
	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(pauline->identity));
	}

	// Pauline side
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, 1, 2000));
	composing_addresses = linphone_chat_room_get_composing_addresses(paulineCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 1, int, "%i");
	if (bctbx_list_size(composing_addresses) == 1) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_get_username(addr), linphone_address_get_username(marie->identity));
	}

	wait_for_list(coresList,0, 1, 1500);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, 3, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, 1, 5000));

	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(paulineCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");

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

static void group_chat_room_fallback_to_basic_chat_room (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_remove_linphone_spec(pauline->lc, "groupchat");
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	LinphoneChatRoom *marieCr = linphone_core_create_client_group_chat_room(marie->lc, "Fallback", TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateInstantiated, initialMarieStats.number_of_LinphoneChatRoomStateInstantiated + 1, 100));

	// Add participants
	linphone_chat_room_add_participants(marieCr, participantsAddresses);

	// Check that the group chat room creation fails and that a fallback to a basic chat room is done
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneChatRoomStateCreationFailed, initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message)
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
	LinphoneChatRoom *paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (paulineCr)
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_creation_fails_if_invited_participants_dont_support_it (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;

	// Marie creates a new group chat room
	LinphoneChatRoom *marieCr = linphone_core_create_client_group_chat_room(marie->lc, "Hello there", FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateInstantiated, initialMarieStats.number_of_LinphoneChatRoomStateInstantiated + 1, 100));

	// Add participants
	linphone_chat_room_add_participants(marieCr, participantsAddresses);

	// Check that the group chat room creation fails
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationFailed, initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed + 1, 5000));
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

static void group_chat_room_creation_successful_if_at_least_one_invited_participant_supports_it (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, 1, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

	// Check that the chat room has not been created on Laure's side
	BC_ASSERT_EQUAL(initialLaureStats.number_of_LinphoneChatRoomStateCreated, laure->stat.number_of_LinphoneChatRoomStateCreated, int, "%d");

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_room_migrate_from_basic_chat_room (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	//Enable migration mecanism only for marie and create a basic chat room
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	linphone_core_remove_linphone_spec(marie->lc, "groupchat"); //Prevent basic chat room from migrating directly
	linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
	LinphoneChatRoom *marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & (LinphoneChatRoomCapabilitiesMigratable));

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message)
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
	LinphoneChatRoom *paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (paulineCr) {
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & (LinphoneChatRoomCapabilitiesBasic | LinphoneChatRoomCapabilitiesMigratable));
	}

	// Enable chat room migration and restart core for Marie
	_linphone_chat_room_enable_migration(marieCr, TRUE); //Will add Migratable capability to the in-db chatroom
	coresList = bctbx_list_remove(coresList, marie->lc);
	linphone_core_manager_reinit(marie);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
	linphone_core_manager_start(marie, TRUE);
	marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);

	/* // Enable chat room migration and restart core for Pauline */
	_linphone_chat_room_enable_migration(paulineCr, TRUE);
	coresList = bctbx_list_remove(coresList, pauline->lc);
	linphone_core_manager_reinit(pauline);
	tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
	linphone_core_manager_start(pauline, TRUE);

	paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));

	// Send a new message to initiate chat room migration
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (marieCr && paulineCr) {
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		linphone_core_add_linphone_spec(marie->lc, "groupchat"); //Enable migration now
		BC_ASSERT_EQUAL(linphone_chat_room_get_capabilities(marieCr), LinphoneChatRoomCapabilitiesBasic | LinphoneChatRoomCapabilitiesProxy | LinphoneChatRoomCapabilitiesMigratable | LinphoneChatRoomCapabilitiesOneToOne, int, "%d");
		msg = linphone_chat_room_create_message_from_utf8(marieCr, "Did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomConferenceJoined, initialMarieStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));

		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");

		msg = linphone_chat_room_create_message_from_utf8(marieCr, "Let's go drink a beer");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 3, int, "%d");

		msg = linphone_chat_room_create_message_from_utf8(paulineCr, "Let's go drink mineral water instead");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 4, int, "%d");

		BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(marie->lc)), 1, int, "%d");
		BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_chat_rooms(pauline->lc)), 1, int, "%d");
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(paulineAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_migrate_from_basic_to_client_fail (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_remove_linphone_spec(pauline->lc, "groupchat");
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	LinphoneChatRoom *marieCr = linphone_core_create_client_group_chat_room(marie->lc, "Fallback", TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateInstantiated, initialMarieStats.number_of_LinphoneChatRoomStateInstantiated + 1, 100));

	// Add participants
	linphone_chat_room_add_participants(marieCr, participantsAddresses);

	// Check that the group chat room creation fails and that a fallback to a basic chat room is done
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneChatRoomStateCreationFailed, initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message)
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
	LinphoneChatRoom *paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (paulineCr)
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);

	// Enable chat room migration and restart core for Marie
	_linphone_chat_room_enable_migration(marieCr, TRUE);
	linphone_chat_room_unref(marieCr);
	coresList = bctbx_list_remove(coresList, marie->lc);
	linphone_core_manager_reinit(marie);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	// Send a new message to initiate chat room migration
	linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "basic_to_client_group_chat_room_migration_timer", 5);
	linphone_core_manager_start(marie, TRUE);

	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);
	linphone_address_unref(paulineAddr);
	paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (marieCr && paulineCr) {
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		BC_ASSERT_EQUAL(linphone_chat_room_get_capabilities(marieCr), LinphoneChatRoomCapabilitiesBasic | LinphoneChatRoomCapabilitiesProxy | LinphoneChatRoomCapabilitiesMigratable | LinphoneChatRoomCapabilitiesOneToOne, int, "%d");
		msg = linphone_chat_room_create_message_from_utf8(marieCr, "Did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");

		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");

		msg = linphone_chat_room_create_message_from_utf8(marieCr, "Let's go drink a beer");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 3, int, "%d");

		msg = linphone_chat_room_create_message_from_utf8(paulineCr, "Let's go drink mineral water instead");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 4, int, "%d");

		// Activate groupchat on Pauline's side and wait for 5 seconds, the migration should now be done on next message sending
		_linphone_chat_room_enable_migration(paulineCr, TRUE);

		linphone_chat_room_unref(paulineCr);
		coresList = bctbx_list_remove(coresList, pauline->lc);
		linphone_core_manager_reinit(pauline);
		tmpCoresManagerList = bctbx_list_append(NULL, pauline);
		tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
		linphone_core_add_linphone_spec(pauline->lc, "groupchat");
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(pauline, TRUE);
		paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));

		linphone_core_set_network_reachable(pauline->lc, FALSE);
		wait_for_list(coresList, &dummy, 1, 3000);
		linphone_core_set_network_reachable(pauline->lc, TRUE);
		wait_for_list(coresList, &dummy, 1, 5000);
		msg = linphone_chat_room_create_message_from_utf8(marieCr, "And now, did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomConferenceJoined, initialMarieStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 5, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
		if (paulineCr) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
			BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesConference);
			BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 5, int, "%d");
		}
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_donot_room_migrate_from_basic_chat_room (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Create a basic chat room
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneChatRoom *marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message)
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
	LinphoneChatRoom *paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (paulineCr)
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);

	// Enable chat room migration and restart core for Marie
	coresList = bctbx_list_remove(coresList, marie->lc);
	linphone_core_manager_restart(marie, TRUE);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
	init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_append(coresList, marie->lc);

	// Send a new message to initiate chat room migration
	marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	if (marieCr) {
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		BC_ASSERT_EQUAL(linphone_chat_room_get_capabilities(marieCr), LinphoneChatRoomCapabilitiesBasic | LinphoneChatRoomCapabilitiesOneToOne, int, "%d");
		msg = linphone_chat_room_create_message_from_utf8(marieCr, "Did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");

		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");

		msg = linphone_chat_room_create_message_from_utf8(marieCr, "Let's go drink a beer");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 3, int, "%d");

		msg = linphone_chat_room_create_message_from_utf8(paulineCr, "Let's go drink mineral water instead");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 4, int, "%d");
	}

	// Clean db from chat room
	linphone_core_delete_chat_room(marie->lc, marieCr);
	linphone_core_delete_chat_room(pauline->lc, paulineCr);

	linphone_address_unref(paulineAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_send_file_with_or_without_text (bool_t with_text, bool_t two_files, bool_t use_buffer) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *sendFilepath2 = NULL;
	char *receivePaulineFilepath = bc_tester_file("receive_file_pauline.dump");
	char *receiveChloeFilepath = bc_tester_file("receive_file_chloe.dump");
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;

	/* Remove any previously downloaded file */
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Sending file
	if (with_text) {
		_send_file_plus_text(marieCr, sendFilepath, sendFilepath2, text, use_buffer);
	} else {
		_send_file(marieCr, sendFilepath, sendFilepath2, use_buffer);
	}

	wait_for_list(coresList, &dummy, 1, 10000);

	// Check that chat rooms have received the file
	if (with_text) {
		_receive_file_plus_text(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, sendFilepath2, text, use_buffer);
		_receive_file_plus_text(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, sendFilepath2, text, use_buffer);
	} else {
		_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, sendFilepath2, use_buffer);
		_receive_file(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, sendFilepath2, use_buffer);
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

static void group_chat_room_send_file (void) {
	group_chat_room_send_file_with_or_without_text(FALSE, FALSE, FALSE);
}

static void group_chat_room_send_file_2 (void) {
	group_chat_room_send_file_with_or_without_text(FALSE, FALSE, TRUE);
}

static void group_chat_room_send_file_plus_text (void) {
	group_chat_room_send_file_with_or_without_text(TRUE, FALSE, FALSE);
}

static void group_chat_room_send_two_files_plus_text (void) {
	group_chat_room_send_file_with_or_without_text(TRUE, TRUE, FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_base(bool_t secondDeviceForSender) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *marie2 = NULL;
	if (secondDeviceForSender)
		marie2 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	if (secondDeviceForSender)
		coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialMarie2Stats;
	memset(&initialMarie2Stats,0, sizeof(initialMarie2Stats));
	if (secondDeviceForSender)
		initialMarie2Stats = marie2->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	const LinphoneAddress *tmpConfAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!tmpConfAddr) {
		goto end;
	}
	LinphoneAddress *confAddr = linphone_address_clone(tmpConfAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *marie2Cr = NULL;
	if (secondDeviceForSender)
		marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, FALSE);
	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	if (secondDeviceForSender)
		linphone_core_set_network_reachable(marie2->lc,FALSE);
	// Marie deletes the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the chat room again
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	// Marie sends a new message
	textMessage = "Hey again";
	message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Check that the created address is the same as before
	const LinphoneAddress *newConfAddr = linphone_chat_room_get_conference_address(marieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, newConfAddr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	if (secondDeviceForSender) {
		linphone_core_set_network_reachable(marie2->lc,TRUE);
		linphone_core_delete_chat_room(marie2->lc, marie2Cr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateTerminated, initialMarie2Stats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	}
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(confAddr);
end:
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	if (secondDeviceForSender)
		linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_unique_one_to_one_chat_room (void) {
	group_chat_room_unique_one_to_one_chat_room_base(FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_dual_sender_device (void) {
	group_chat_room_unique_one_to_one_chat_room_base(TRUE);
}

static void group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base (bool_t with_app_restart, bool_t forward_message) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	if (forward_message) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 1, int," %i");
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

			BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDelivered,1));
			BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);

			if (marie->stat.last_received_chat_message != NULL) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int," %i");
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), textMessage);
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(marie->stat.last_received_chat_message),textMessage);
				BC_ASSERT_TRUE(linphone_chat_message_is_forward(marie->stat.last_received_chat_message));
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(marie->stat.last_received_chat_message), forwarded_from);
			}

			linphone_chat_message_unref(msg);
			ms_free(forwarded_from);
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
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int," %i");
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
	} else {
		// Marie deletes the chat room
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
		wait_for_list(coresList, 0, 1, 2000);
		BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

		// Pauline sends a new message
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;

		// Pauline sends a new message
		textMessage = "Hey you";
		message = _send_message(paulineCr, textMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), textMessage);
		linphone_chat_message_unref(message);

		// Check that the chat room has been correctly recreated on Marie's side
		marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 1, FALSE);
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int,"%i");
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_call_logs(pauline->lc));

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);

}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base (bool_t with_app_restart) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(with_app_restart, FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message(void) {
	group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base(FALSE);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_with_app_restart(void) {
	group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base(TRUE);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_2 (void) {
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
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline2Stats = pauline2->stat;

	linphone_core_set_network_reachable(marie2->lc, FALSE);
	linphone_core_set_network_reachable(pauline2->lc, FALSE);

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marieCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Marie deletes the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie sends a new message
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	// Check that the chat room has been correctly recreated on Marie's side
	marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 1, FALSE);
	if (BC_ASSERT_PTR_NOT_NULL(marieCr)){
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);
		textMessage = "Hey you";
		message = _send_message(marieCr, textMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
		linphone_chat_message_unref(message);
	}

	// Clean db from chat room
	linphone_core_set_network_reachable(marie2->lc, TRUE);
	linphone_core_set_network_reachable(pauline2->lc, TRUE);

	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 1, FALSE);
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, FALSE);

	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(marie2, marieCr2, coresList);
	linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateTerminated, initialMarie2Stats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateTerminated, initialPauline2Stats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(marie->lc), 0, int,"%i");
	BC_ASSERT_EQUAL(linphone_core_get_call_history_size(pauline->lc), 0, int,"%i");
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

static void group_chat_room_join_one_to_one_chat_room_with_a_new_device (void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie1 creates a new one-to-one chat room with Pauline
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie1Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marie1Cr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie1 sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marie1Cr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneMessageDelivered, initialMarie1Stats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Hey. How are you?";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneMessageReceived, initialMarie1Stats.number_of_LinphoneMessageReceived + 1, 5000));
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
	LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie2Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Marie2 sends a new message
	textMessage = "Fine and you?";
	message = _send_message(marie2Cr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDelivered, initialMarie2Stats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Perfect!";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived, initialMarie2Stats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie2->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Clean db from chat room
	int previousNbRegistrationOk = marie1->stat.number_of_LinphoneRegistrationOk;
	linphone_core_set_network_reachable(marie1->lc, TRUE);
	wait_for_until(marie1->lc, NULL, &marie1->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
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

static void group_chat_room_new_unique_one_to_one_chat_room_after_both_participants_left (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Pauline";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *firstConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, firstConfAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	// Both participants delete the chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	wait_for_list(coresList, 0, 1, 3000);

	// Marie re-creates a chat room with Pauline
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesOneToOne);

	LinphoneAddress *secondConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room has been correctly recreated on Marie's side
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, secondConfAddr, initialSubject, 1, FALSE);
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

static void imdn_for_group_chat_room (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	time_t initialTime = ms_time(NULL);

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageSent, 1, 1000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 5000));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed));
	bctbx_list_t *participantsThatReceivedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 2, int, "%d");
		for (bctbx_list_t *item = participantsThatReceivedChloeMessage; item; item = bctbx_list_next(item)) {
			LinphoneParticipantImdnState *state = (LinphoneParticipantImdnState *)bctbx_list_get_data(item);
			BC_ASSERT_GREATER((int)linphone_participant_imdn_state_get_state_change_time(state), (int)initialTime, int, "%d");
			BC_ASSERT_EQUAL(linphone_participant_imdn_state_get_state(state), LinphoneChatMessageStateDeliveredToUser, int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_participant_imdn_state_get_participant(state));
		}
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	// Marie marks the message as read, check that the state is not yet displayed on Chloe's side
	linphone_chat_room_mark_as_read(marieCr);
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 0, 1000));
	bctbx_list_t *participantsThatDisplayedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	participantsThatReceivedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 1, int, "%d");
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

	// Pauline also marks the message as read, check that the state is now displayed on Chloe's side
	linphone_chat_room_mark_as_read(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageSent, 0, 1000));
	participantsThatDisplayedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatDisplayedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatDisplayedChloeMessage), 2, int, "%d");
		bctbx_list_free_with_data(participantsThatDisplayedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

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

static void aggregated_imdn_for_group_chat_room_base (bool_t read_while_offline) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	const char *chloeTextMessage2 = "Long time no talk";
	const char *chloeTextMessage3 = "How are you?";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	LinphoneChatMessage *chloeMessage2 = _send_message(chloeCr, chloeTextMessage2);
	LinphoneChatMessage *chloeMessage3 = _send_message(chloeCr, chloeTextMessage3);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 3, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 3, 5000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage3);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Mark the messages as read on Marie's and Pauline's sides
	linphone_chat_room_mark_as_read(marieCr);
	if (read_while_offline) {
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		linphone_chat_room_mark_as_read(paulineCr);
		wait_for_list(coresList, 0, 1, 2000);
		linphone_core_set_network_reachable(pauline->lc, TRUE);
	} else {
		linphone_chat_room_mark_as_read(paulineCr);
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 5000));
	BC_ASSERT_EQUAL(chloe->stat.number_of_LinphoneMessageDeliveredToUser, 0, int, "%d");
	if (read_while_offline) {
		wait_for_list(coresList, 0, 1, 2000); // To prevent memory leak
	}

	linphone_chat_message_unref(chloeMessage3);
	linphone_chat_message_unref(chloeMessage2);
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

static void aggregated_imdn_for_group_chat_room (void) {
	aggregated_imdn_for_group_chat_room_base(FALSE);
}

static void aggregated_imdn_for_group_chat_room_read_while_offline (void) {
	aggregated_imdn_for_group_chat_room_base(TRUE);
}

static void imdn_sent_from_db_state (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
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
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;
	confAddr = linphone_address_clone(confAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message is not globally marked as delivered to user since Marie do not notify its delivery
	BC_ASSERT_FALSE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 5000));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDisplayed));
	bctbx_list_t *participantsThatReceivedChloeMessage = linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDeliveredToUser);
	if (BC_ASSERT_PTR_NOT_NULL(participantsThatReceivedChloeMessage)) {
		BC_ASSERT_EQUAL((int)bctbx_list_size(participantsThatReceivedChloeMessage), 2, int, "%d");
		for (bctbx_list_t *item = participantsThatReceivedChloeMessage; item; item = bctbx_list_next(item)) {
			LinphoneParticipantImdnState *state = (LinphoneParticipantImdnState *)bctbx_list_get_data(item);
			BC_ASSERT_GREATER((int)linphone_participant_imdn_state_get_state_change_time(state), (int)initialTime, int, "%d");
			BC_ASSERT_EQUAL(linphone_participant_imdn_state_get_state(state), LinphoneChatMessageStateDeliveredToUser, int, "%d");
			BC_ASSERT_PTR_NOT_NULL(linphone_participant_imdn_state_get_participant(state));
		}
		bctbx_list_free_with_data(participantsThatReceivedChloeMessage, (bctbx_list_free_func)linphone_participant_imdn_state_unref);
	}
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateDelivered));
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_participants_by_imdn_state(chloeMessage, LinphoneChatMessageStateNotDelivered));

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

static void find_one_to_one_chat_room (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;
	// Only to be used in linphone_core_find_one_to_one_chatroom(...);
	const LinphoneAddress *marieAddr = linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc));
	const LinphoneAddress *paulineAddr = linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(chloeCr)) goto end;

	// Chloe leave the chat room
	linphone_chat_room_leave(chloeCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminationPending, initialChloeStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminated, initialChloeStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));

	LinphoneChatRoom *oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NULL(oneToOneChatRoom);

	// Marie create a one to one chat room with Pauline
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieOneToOneCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieOneToOneCr);
	LinphoneChatRoom *paulineOneToOneCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, "one to one", 1, FALSE);

	// Check it's the same chat room
	oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, marieOneToOneCr);

	// Clean the db
	linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineOneToOneCr, coresList);

	// Check cleaning went well
	oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NULL(oneToOneChatRoom);

	// Create a basic chat room
	marieOneToOneCr = linphone_core_get_chat_room(marie->lc, paulineAddr);

	// Check it's the same chat room
	oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(oneToOneChatRoom);
	BC_ASSERT_PTR_EQUAL(oneToOneChatRoom, marieOneToOneCr);

end:
	// Clean db from chat room
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

static void group_chat_room_new_device_after_creation (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(pauline1Cr) && BC_ASSERT_PTR_NOT_NULL(pauline2Cr))) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	// Marie adds a new device
	marie2 = linphone_core_manager_create("marie_rc");
	stats initialMarie2Stats = marie2->stat;
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie2);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	start_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);
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

static void group_chat_room_list_subscription (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	int dummy = 0;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr1 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr1)) goto end;
	confAddr1 = linphone_chat_room_get_conference_address(marieCr1);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr1)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	paulineCr1 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr1, initialSubject, 2, FALSE);
	laureCr1 = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr1, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(paulineCr1) && BC_ASSERT_PTR_NOT_NULL(laureCr1))) goto end;
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	initialSubject = "Friends";
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	marieCr2 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr2)) goto end;
	confAddr2 = linphone_chat_room_get_conference_address(marieCr2);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr2)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	paulineCr2 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr2, initialSubject, 2, FALSE);
	laureCr2 = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr2, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(paulineCr2) && BC_ASSERT_PTR_NOT_NULL(laureCr2))) goto end;
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	initialSubject = "Bros";
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	marieCr3 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr3)) goto end;
	confAddr3 = linphone_chat_room_get_conference_address(marieCr3);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr3)) goto end;

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	paulineCr3 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr3, initialSubject, 2, FALSE);
	laureCr3 = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr3, initialSubject, 2, FALSE);
	if (!(BC_ASSERT_PTR_NOT_NULL(paulineCr3) && BC_ASSERT_PTR_NOT_NULL(laureCr3))) goto end;

	participantsAddresses = NULL;

	// Marie designates Pauline as admin in chat room 1
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr1, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr1, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 3000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant1));
	linphone_chat_room_set_participant_admin_status(marieCr3, laureParticipant3, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 3, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 3, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 3, 3000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant3));

	// Marie now changes the subject or chat room 1
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr1, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 3000));
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
	linphone_address_unref(paulineAddr);
	LinphoneParticipant *laureParticipantOfPauline1 = linphone_chat_room_find_participant(paulineCr1, laureAddr);
	LinphoneParticipant *laureParticipantOfPauline2 = linphone_chat_room_find_participant(paulineCr2, laureAddr);
	LinphoneParticipant *laureParticipantOfPauline3 = linphone_chat_room_find_participant(paulineCr3, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantOfPauline1);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantOfPauline2);
	BC_ASSERT_PTR_NOT_NULL(laureParticipantOfPauline3);
	linphone_address_unref(laureAddr);
	wait_for_list(coresList, &dummy, 1, 5000);

	// Check that Pauline receive the missing info and not more
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipantOfPauline1));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipantOfPauline3));
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipantOfPauline2));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 10000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr1), newSubject);

	// Check that Pauline can still receive info once back
	// Marie now changes the subject or chat room 1
	newSubject = "New New subject";
	linphone_chat_room_set_subject(marieCr1, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 2, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 2, 10000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr1), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr1), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr1), newSubject);
	// Marie now changes the subject or chat room 2
	newSubject = "Newer subject";
	linphone_chat_room_set_subject(marieCr2, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 3, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 3, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 3, 10000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr2), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr2), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr2), newSubject);
	// Marie now changes the subject or chat room 3
	newSubject = "Newest subject";
	linphone_chat_room_set_subject(marieCr3, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 4, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 4, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 4, 10000));
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

static void group_chat_room_complex_participant_removal_scenario (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Restart Laure core
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

	wait_for_list(coresList,0, 1, 2000);
	initialLaureStats = laure->stat;

	linphone_proxy_config_refresh_register(linphone_core_get_default_proxy_config(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk, initialLaureStats.number_of_LinphoneRegistrationOk + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	// Marie adds Laure to the chat room
	participantsAddresses = bctbx_list_append(participantsAddresses, laureAddr);
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreationPending, initialLaureStats.number_of_LinphoneChatRoomStateCreationPending + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreated, initialLaureStats.number_of_LinphoneChatRoomStateCreated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk, initialLaureStats.number_of_LinphoneRegistrationOk + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	unsigned int nbLaureConferenceCreatedEventsAfterRestart = 0;
	laureHistory = linphone_chat_room_get_history_events(newLaureCr, 0);
	for (bctbx_list_t *item = laureHistory; item; item = bctbx_list_next(item)) {
		LinphoneEventLog *event = (LinphoneEventLog *)bctbx_list_get_data(item);
		if (linphone_event_log_get_type(event) == LinphoneEventLogTypeConferenceCreated)
			nbLaureConferenceCreatedEventsAfterRestart++;
	}
	bctbx_list_free_with_data(laureHistory, (bctbx_list_free_func)linphone_event_log_unref);
	BC_ASSERT_EQUAL(nbLaureConferenceCreatedEventsAfterRestart, nbLaureConferenceCreatedEventsBeforeRestart, unsigned int, "%u");

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

static void group_chat_room_subscription_denied (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	// Simulate pauline has disconnected
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddress);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_remove_participant(marieCr, paulineParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

	// Reconnect pauline
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

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

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	magicSearch = linphone_magic_search_new(pauline->lc);
	resultList = linphone_magic_search_get_contact_list_from_filter(magicSearch, "", "");
	if (BC_ASSERT_PTR_NOT_NULL(resultList)) {
		BC_ASSERT_EQUAL(bctbx_list_size(resultList), 2, int, "%d");
		_check_friend_result_list(pauline->lc, resultList, 0, laureI, NULL);
		_check_friend_result_list(pauline->lc, resultList, 1, marieI, NULL);
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

static void group_chat_room_participant_devices_name (void) {
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
	LinphoneAddress *paulineAddress = linphone_address_ref(linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *laureAddress = linphone_address_ref(linphone_address_new(linphone_core_get_identity(laure->lc)));
	LinphoneAddress *chloeAddress = linphone_address_new(linphone_core_get_identity(chloe->lc));
	LinphoneAddress *marieDeviceAddress =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc)));
	LinphoneAddress *paulineDeviceAddress1 =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline->lc)));
	LinphoneAddress *paulineDeviceAddress2 =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline2->lc)));
	LinphoneAddress *laureDeviceAddress =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(laure->lc)));
	LinphoneAddress *chloeDeviceAddress1 =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe->lc)));
	LinphoneAddress *chloeDeviceAddress2 =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe2->lc)));
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
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPaulineStats2, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Check device's name in Marie's chat room
	LinphoneParticipant *marieParticipant = linphone_chat_room_get_me(marieCr);
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddress);
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddress);
	LinphoneParticipantDevice *marieDevice = linphone_participant_find_device(marieParticipant, marieDeviceAddress);
	LinphoneParticipantDevice *paulineDevice = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress1);
	LinphoneParticipantDevice *paulineDevice2 = linphone_participant_find_device(paulineParticipant, paulineDeviceAddress2);
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

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, 0);
	LinphoneChatRoom *chloeCr2 = check_creation_chat_room_client_side(coresList, chloe2, &initialChloeStats2, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participants_added, initialPaulineStats2.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participant_devices_added, initialPaulineStats2.number_of_participant_devices_added + 2, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 2, 5000));

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
	init_core_for_conference(coresManagerList);
	coresList = bctbx_list_append(coresList, chloe3->lc);
	linphone_core_set_user_agent(chloe3->lc, "blabla (Chloe device 3) blibli/blublu (bloblo)", NULL);
	start_core_for_conference(coresManagerList);
	LinphoneAddress *chloeDeviceAddress3 =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe3->lc)));
	LinphoneChatRoom *chloeCr3 = check_creation_chat_room_client_side(coresList, chloe3, &initialChloeStats3, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participant_devices_added, initialPaulineStats2.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participant_devices_added, initialChloeStats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe2->stat.number_of_participant_devices_added, initialChloeStats2.number_of_participant_devices_added + 1, 5000));

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

static void add_device_one_to_one_chat_room_other_left (void) {
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
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	linphone_core_manager_start(marie, TRUE);
	linphone_core_manager_start(pauline, TRUE);
	LinphoneAddress *paulineAddress = linphone_address_new(linphone_core_get_identity(pauline->lc));
	participantsAddresses = bctbx_list_append(participantsAddresses, paulineAddress);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialPaulineStats2 = pauline2->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto clean;

	confAddr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto clean;
	confAddr = linphone_address_clone(confAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto clean;

	// Marie leaves the chat room
	linphone_chat_room_leave(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminationPending, initialMarieStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));

	int pauline_number_of_participant_devices_added = pauline->stat.number_of_participant_devices_added;
	linphone_core_manager_start(pauline2, TRUE);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPaulineStats2, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr2)) goto clean;

	// Also, pauline should receive a notification that pauline2 device was added.
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, pauline_number_of_participant_devices_added + 1, 5000));

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
 * This test simulates a case where whatever the reason the server thinks a client device is part of a given chatroom, but the
 * client device has no longer this information.
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Save Laure's db before it becomes part of a group chat.
	const char *uri = linphone_config_get_string(linphone_core_get_config(laure->lc), "storage", "uri", "");
	char *uriCopy = bc_tester_file("linphone_tester.db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopy));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;

	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Save Laure's db now that it is part of the group chat
	uri = linphone_config_get_string(linphone_core_get_config(laure->lc), "storage", "uri", "");
	char *uriCopyAfter = bc_tester_file("linphone_tester2.db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopyAfter));


	// Restore old db to Laure and restart it.
	laure->database_path = uriCopy;
	coresList = bctbx_list_remove(coresList, laure->lc);
	linphone_core_manager_reinit(laure);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, laure);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(laure, TRUE);

	// Pauline sends a message, laure shall not receive it, in any way.
	if (paulineCr){
		_send_message(paulineCr, "Salut");
	}
	wait_for_list(coresList, NULL, 0, 2000);
	BC_ASSERT_TRUE(linphone_core_get_chat_rooms(laure->lc) == NULL);

	// Now restarts Laure with good db in order to clean the chatroom properly.
	// Restore old db to Laure and restart it.
	laure->database_path = uriCopyAfter;
	coresList = bctbx_list_remove(coresList, laure->lc);
	linphone_core_manager_reinit(laure);
	tmpCoresManagerList = bctbx_list_append(NULL, laure);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(laure, TRUE);

	// Clean chatroom from databases.

	if (BC_ASSERT_TRUE(linphone_core_get_chat_rooms(laure->lc) != NULL)){
		LinphoneChatRoom *laureCr = (LinphoneChatRoom*) linphone_core_get_chat_rooms(laure->lc)->data;
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


static void participant_removed_then_added (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline1
	pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1Cr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) goto end;

	// Pauline restart to make use of liste subscription :::this part (core restart) is leaking  memory
	coresList = bctbx_list_remove(coresList, pauline1->lc);
	linphone_core_manager_reinit(pauline1);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline1);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline1, TRUE);

	// Check that the chat room has correctly created on Laure's side and that the participants are added
	pauline1Cr = check_has_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);

	//wait for first notify to be received by pauline
	wait_for_list(coresList, NULL, 0, 1000);

	//Pauline leaving but keeping a ref like a Java GC can do, this is the key part of this test.
	linphone_chat_room_ref(pauline1Cr);

	linphone_core_delete_chat_room(pauline1->lc, pauline1Cr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateDeleted, initialPauline1Stats.number_of_LinphoneChatRoomStateDeleted + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 5000));


	// Marie1 adds Pauline back to the chat room
	initialPauline1Stats = pauline1->stat;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	linphone_chat_room_add_participants(marie1Cr, participantsAddresses);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_added, initialMarie1Stats.number_of_participants_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 5000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie1Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");
	newPauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(newPauline1Cr)) goto end;
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newPauline1Cr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newPauline1Cr), initialSubject);

end:
	// Clean db from chat room
	if (marie1Cr) linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	if (newPauline1Cr) linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);
	if (laureCr) linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	//now GC is cleaning old chatroom
	if (pauline1Cr) linphone_chat_room_unref(pauline1Cr);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	bctbx_list_free_with_data(participantsAddresses,(bctbx_list_free_func)linphone_address_unref);
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
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPaulineStats = pauline->stat;

	// Marie1 creates a new one-to-one chat room with Pauline
	const char *initialSubject = "Pauline";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie1Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	confAddr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;
	confAddr = linphone_address_clone(confAddr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

	initialPaulineStats.number_of_participant_devices_added = pauline->stat.number_of_participant_devices_added;
	marie2 = linphone_core_manager_create("marie_rc");
	stats initialMarie2Stats = marie2->stat;

	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, marie2);
	bctbx_list_t *newCoresList = init_core_for_conference(newCoresManagerList);
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Marie2 gets the one-to-one chat room with Pauline
	marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie2Cr)) goto end;
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marie2Cr) & LinphoneChatRoomCapabilitiesOneToOne);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));

	// Save pauline db
	const char *uri = linphone_config_get_string(linphone_core_get_config(pauline->lc), "storage", "uri", "");
	char *uriCopy = bc_tester_file("linphone_tester.db");
	BC_ASSERT_FALSE(liblinphone_tester_copy_file(uri, uriCopy));
	int initialPaulineEvent = linphone_chat_room_get_history_events_size(paulineCr);
	// Simulate an uninstall of the application on Marie's side with unregistration (It should remove the device)
	coresManagerList = bctbx_list_remove(coresManagerList, marie1);
	coresList = bctbx_list_remove(coresList, marie1->lc);
	initialPaulineStats = pauline->stat;
	linphone_core_manager_stop(marie1);
	//force flexisip to publish on marie topic
	linphone_core_refresh_registers(marie2->lc);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, 5000));

	// Reset db at pauline side
	pauline->database_path = uriCopy;
	coresList = bctbx_list_remove(coresList, pauline->lc);
	memset(&initialPaulineStats, 0, sizeof(initialPaulineStats));


	linphone_core_manager_reinit(pauline);
	//force full state
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_package_force_full_state",TRUE);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline, TRUE);

	//wait for first notify to be received by pauline
	wait_for_list(coresList, NULL, 0, 1000);

	// Marie2 gets the one-to-one chat room with Pauline
	paulineCr = check_has_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	LinphoneAddress *marieAddress = linphone_address_new(linphone_core_get_identity(marie2->lc));
	LinphoneParticipant *marieParticipant =  linphone_chat_room_find_participant(paulineCr, marieAddress);
	BC_ASSERT_EQUAL(bctbx_list_size(linphone_participant_get_devices (marieParticipant)), 1, int, "%i");

	//recheck after restart
	coresList = bctbx_list_remove(coresList, pauline->lc);
	linphone_core_manager_reinit(pauline);
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "misc", "conference_event_package_force_full_state",FALSE);
	tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline, TRUE);


	//wait for first notify to be received by pauline
	wait_for_list(coresList, NULL, 0, 1000);

	// Marie2 gets the one-to-one chat room with Pauline
	paulineCr = check_has_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
	marieParticipant =  linphone_chat_room_find_participant(paulineCr, marieAddress);
	BC_ASSERT_EQUAL(bctbx_list_size(linphone_participant_get_devices (marieParticipant)), 1, int, "%i");
	BC_ASSERT_EQUAL(linphone_chat_room_get_history_events_size(paulineCr), initialPaulineEvent, int, "%i");

	//check if we can still communicate
	// Marie1 sends a message
	const char *textMessage = "Hello";
	LinphoneChatMessage *message = _send_message(marie2Cr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDelivered, initialMarie1Stats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Hey. How are you?";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived, initialMarie1Stats.number_of_LinphoneMessageReceived + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie2->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Clean db from chat room
	linphone_core_manager_reinit(marie1);
	tmpCoresManagerList = bctbx_list_append(NULL, marie1);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(marie1,TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneChatRoomStateCreated, initialMarie1Stats.number_of_LinphoneChatRoomStateCreated + 2, 5000));
	wait_for_list(coresList, NULL, 0, 1000);
	marie1Cr = check_has_chat_room_client_side(coresList, marie1, &initialMarie1Stats, confAddr, initialSubject, 1, FALSE);

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

	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) goto end;
	confAddr = linphone_chat_room_get_conference_address(marieCr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) goto end;

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);
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

	LinphoneAddress *paulineAddress = linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline->lc)));
	paulineCr = linphone_core_find_chat_room(pauline->lc, confAddr, paulineAddress);

	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	linphone_chat_room_ref(paulineCr);

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	const char *path = "";
	// set_chat_database_path() will cause a reload of the database, with re-creation of chatrooms.
	linphone_core_set_chat_database_path(pauline->lc, path);
	linphone_chat_room_unref(paulineCr);
	paulineCr = linphone_core_find_chat_room(pauline->lc, confAddr, paulineAddress);
	linphone_address_unref(paulineAddress);
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	// Marie now changes the subject
	const char *newSubject = "Subject has been changed! :O";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
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

static void one_to_one_chat_room_send_forward_message (void) {
	group_chat_room_unique_one_to_one_chat_room_with_forward_message_recreated_from_message_base(TRUE,TRUE);
}

static void core_stop_start_with_chat_room_ref (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialPauline1Stats = pauline1->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(marie1Cr)) goto end;
	participantsAddresses = NULL;
	confAddr = linphone_chat_room_get_conference_address(marie1Cr);
	if (!BC_ASSERT_PTR_NOT_NULL(confAddr)) goto end;

	// Check that the chat room is correctly created on Pauline1
	pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 1, FALSE);
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1Cr)) goto end;

	//Pauline leaving but keeping a ref like a Java GC can do, this is the key part of this test.
	linphone_chat_room_ref(pauline1Cr);

	linphone_core_stop(pauline1->lc);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneGlobalShutdown, initialPauline1Stats.number_of_LinphoneGlobalShutdown + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneGlobalOff, initialPauline1Stats.number_of_LinphoneGlobalOff + 1, 5000));

	linphone_core_start(pauline1->lc);
	//now GC is cleaning old chatroom
	if (pauline1Cr) linphone_chat_room_unref(pauline1Cr);

	//test very early client group chatroom creation, should fail
	linphone_proxy_config_set_conference_factory_uri(linphone_core_get_default_proxy_config(pauline1->lc), sFactoryUri);
	LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(pauline1->lc);
	linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
	LinphoneChatRoom *chatRoom = linphone_core_create_chat_room_2(pauline1->lc, params, initialSubject,participantsAddresses);
	linphone_chat_room_params_unref(params);

	BC_ASSERT_PTR_NULL(chatRoom);

	coresList = bctbx_list_remove(coresList, pauline1->lc);
	linphone_core_manager_reinit(pauline1);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline1);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline1, TRUE);


	// Check that the chat room has correctly created on Laure's side and that the participants are added
	newPauline1Cr = check_has_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 1, FALSE);
	wait_for_list(coresList, NULL, 0, 1000);

end:
	// Clean db from chat room
	if (marie1Cr) linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	if (newPauline1Cr) linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);


	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	bctbx_list_free_with_data(participantsAddresses,(bctbx_list_free_func)linphone_address_unref);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(pauline1);
}

static void group_chat_room_device_unregistered (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;


	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	LinphoneProxyConfig *proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, FALSE);
	linphone_proxy_config_done(proxyConfig);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationCleared, initialMarieStats.number_of_LinphoneRegistrationCleared + 1, 5000));

	//Current expected behavior is to have participant devices removed
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_removed, initialMarieStats.number_of_participant_devices_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 3000));

	//to avoid automatique re-subscription of chatroom  while disabling network
	linphone_core_enter_background(marie->lc);
	linphone_core_enter_background(pauline->lc);
	linphone_core_enter_background(laure->lc);
	wait_for_list(coresList,NULL,1,3000);


	//to force re-re-connection to restarted flexisip
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_core_set_network_reachable(laure->lc, FALSE);

	//break here and restart Flexisip
	marie->stat.number_of_participant_devices_added = 0;
	pauline->stat.number_of_participant_devices_added=0;

	proxyConfig = linphone_core_get_default_proxy_config(laure->lc);
	linphone_proxy_config_edit(proxyConfig);
	linphone_proxy_config_enable_register(proxyConfig, TRUE);
	linphone_proxy_config_done(proxyConfig);

	linphone_core_enter_foreground(marie->lc);
	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneRegistrationOk, initialMarieStats.number_of_LinphoneRegistrationOk + 1, 10000));
	linphone_core_enter_foreground(pauline->lc);
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneRegistrationOk, initialPaulineStats.number_of_LinphoneRegistrationOk + 1, 10000));

	linphone_core_enter_foreground(laure->lc);
	linphone_core_set_network_reachable(laure->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk, initialLaureStats.number_of_LinphoneRegistrationOk + 1, 10000));


	//in case of flexisip restart
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));


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

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationCleared, initialLaureStats.number_of_LinphoneRegistrationCleared + 1, 5000));

	//Current expected behavior is to have participant devices removed
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_removed, initialMarieStats.number_of_participant_devices_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

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

static void group_chat_room_admin_creator_leaves_and_is_reinvited (void) {
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
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie leaves the room
	linphone_chat_room_leave(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 5000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(linphone_chat_room_get_me(laureCr)));

	linphone_core_delete_chat_room(marie->lc, marieCr);
	initialMarieStats = marie->stat;

	// Laure invites Marie to the chatroom
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(marie->lc)));
	linphone_chat_room_add_participants(laureCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Marie's side and that the participants are added
	marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, linphone_chat_room_get_conference_address(laureCr), initialSubject, 2, FALSE);

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

test_t group_chat_tests[] = {
	TEST_NO_TAG("Chat room params", group_chat_room_params),
	TEST_NO_TAG("Chat room with forced local identity", group_chat_room_creation_with_given_identity),
	TEST_NO_TAG("Group chat room creation server", group_chat_room_creation_server),
	TEST_ONE_TAG("Add participant", group_chat_room_add_participant, "LeaksMemory"),
	TEST_NO_TAG("Send message", group_chat_room_send_message),
	TEST_NO_TAG("Send encrypted message", group_chat_room_send_message_encrypted),
	TEST_NO_TAG("Send message with error", group_chat_room_send_message_with_error),
	TEST_NO_TAG("Send invite on a multi register account", group_chat_room_invite_multi_register_account),
	TEST_NO_TAG("Add admin", group_chat_room_add_admin),
	TEST_NO_TAG("Add admin lately notified", group_chat_room_add_admin_lately_notified),
	TEST_NO_TAG("Add admin with a non admin", group_chat_room_add_admin_non_admin),
	TEST_NO_TAG("Remove admin", group_chat_room_remove_admin),
	TEST_NO_TAG("Admin creator leaves the room", group_chat_room_admin_creator_leaves_the_room),
	TEST_NO_TAG("Change subject", group_chat_room_change_subject),
	TEST_NO_TAG("Change subject with a non admin", group_chat_room_change_subject_non_admin),
	TEST_NO_TAG("Remove participant", group_chat_room_remove_participant),
	TEST_NO_TAG("Send message with a participant removed", group_chat_room_send_message_with_participant_removed),
	TEST_NO_TAG("Leave group chat room", group_chat_room_leave),
	TEST_NO_TAG("Delete group chat room successful if it's already removed by server", group_chat_room_delete_twice),
	TEST_NO_TAG("Come back on a group chat room after a disconnection", group_chat_room_come_back_after_disconnection),
	TEST_NO_TAG("Create chat room with disconnected friends", group_chat_room_create_room_with_disconnected_friends),
	TEST_NO_TAG("Create chat room with disconnected friends and initial message", group_chat_room_create_room_with_disconnected_friends_and_initial_message),
	TEST_NO_TAG("Reinvited after removed from group chat room", group_chat_room_reinvited_after_removed),
	TEST_ONE_TAG("Reinvited after removed from group chat room 2", group_chat_room_reinvited_after_removed_2, "LeaksMemory"),
	TEST_ONE_TAG("Reinvited after removed from group chat room while offline", group_chat_room_reinvited_after_removed_while_offline, "LeaksMemory"),
	TEST_ONE_TAG("Reinvited after removed from group chat room while offline 2", group_chat_room_reinvited_after_removed_while_offline_2, "LeaksMemory"),
	TEST_ONE_TAG("Reinvited after removed from group chat room with several devices", group_chat_room_reinvited_after_removed_with_several_devices, "LeaksMemory"),
	TEST_NO_TAG("Notify after disconnection", group_chat_room_notify_after_disconnection),
	TEST_NO_TAG("Send refer to all participants devices", group_chat_room_send_refer_to_all_devices),
	TEST_NO_TAG("Admin add device and doesn't lose admin status", group_chat_room_add_device),
	TEST_NO_TAG("Send multiple is composing", multiple_is_composing_notification),
	TEST_ONE_TAG("Fallback to basic chat room", group_chat_room_fallback_to_basic_chat_room, "LeaksMemory"),
	TEST_NO_TAG("Group chat room creation fails if invited participants don't support it", group_chat_room_creation_fails_if_invited_participants_dont_support_it),
	TEST_NO_TAG("Group chat room creation successful if at least one invited participant supports it", group_chat_room_creation_successful_if_at_least_one_invited_participant_supports_it),
	TEST_ONE_TAG("Migrate basic chat room to client group chat room", group_chat_room_migrate_from_basic_chat_room, "Migration"),
	TEST_TWO_TAGS("Migrate basic chat room to client group chat room failure", group_chat_room_migrate_from_basic_to_client_fail, "LeaksMemory", "Migration"),
	TEST_ONE_TAG("Migrate basic chat room to client group chat room not needed", group_chat_donot_room_migrate_from_basic_chat_room, "Migration"),
	TEST_NO_TAG("Send file", group_chat_room_send_file),
	TEST_NO_TAG("Send file using buffer", group_chat_room_send_file_2),
	TEST_NO_TAG("Send file + text", group_chat_room_send_file_plus_text),
	TEST_NO_TAG("Send 2 files + text", group_chat_room_send_two_files_plus_text),
	TEST_NO_TAG("Unique one-to-one chatroom", group_chat_room_unique_one_to_one_chat_room),
	TEST_NO_TAG("Unique one-to-one chatroom with dual sender device", group_chat_room_unique_one_to_one_chat_room_dual_sender_device),
	TEST_NO_TAG("Unique one-to-one chatroom recreated from message", group_chat_room_unique_one_to_one_chat_room_recreated_from_message),
	TEST_ONE_TAG("Unique one-to-one chatroom recreated from message with app restart", group_chat_room_unique_one_to_one_chat_room_recreated_from_message_with_app_restart, "LeaksMemory"),
	TEST_NO_TAG("Join one-to-one chat room with a new device", group_chat_room_join_one_to_one_chat_room_with_a_new_device),
	TEST_NO_TAG("New unique one-to-one chatroom after both participants left", group_chat_room_new_unique_one_to_one_chat_room_after_both_participants_left),
	TEST_NO_TAG("Unique one-to-one chatroom re-created from the party that deleted it, with inactive devices", group_chat_room_unique_one_to_one_chat_room_recreated_from_message_2),
	TEST_ONE_TAG("Group chat room notify participant devices name", group_chat_room_participant_devices_name, "LeaksMemory"/* Core restarts */),
	TEST_NO_TAG("Add device in one to one chat room where other participant left", add_device_one_to_one_chat_room_other_left),
	TEST_NO_TAG("IMDN for group chat room", imdn_for_group_chat_room),
	TEST_NO_TAG("Aggregated IMDN for group chat room", aggregated_imdn_for_group_chat_room),
	TEST_NO_TAG("Aggregated IMDN for group chat room read while offline", aggregated_imdn_for_group_chat_room_read_while_offline),
	TEST_ONE_TAG("IMDN sent from DB state", imdn_sent_from_db_state, "LeaksMemory"),
	TEST_NO_TAG("Find one to one chat room", find_one_to_one_chat_room),
	TEST_NO_TAG("New device after group chat room creation", group_chat_room_new_device_after_creation),
	TEST_ONE_TAG("Chat room list subscription", group_chat_room_list_subscription, "LeaksMemory"),
	TEST_ONE_TAG("Complex participant removal scenario", group_chat_room_complex_participant_removal_scenario, "LeaksMemory"),
	TEST_ONE_TAG("Group chat room subscription denied", group_chat_room_subscription_denied, "LeaksMemory"),
	TEST_TWO_TAGS("Search friend result chat room participants", search_friend_chat_room_participants, "MagicSearch", "LeaksMemory"),
	TEST_ONE_TAG("Client loose context of a chatroom", group_chat_loss_of_client_context, "LeaksMemory"),
	TEST_ONE_TAG("Participant removed then added", participant_removed_then_added, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Check  if participant device are removed", group_chat_room_join_one_to_one_chat_room_with_a_new_device_not_notified, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Subscribe successfull after set chat database path", subscribe_test_after_set_chat_database_path, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Send forward message", one_to_one_chat_room_send_forward_message, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Linphone core stop/start and chatroom ref", core_stop_start_with_chat_room_ref, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Subscribe successfull after set chat database path", subscribe_test_after_set_chat_database_path, "LeaksMemory" /*due to core restart*/),
	TEST_ONE_TAG("Make sure device unregistration does not triger user to be removed from a group", group_chat_room_device_unregistered,  "LeaksMemory" /*due network up/down*/),
	TEST_NO_TAG("Admin leaves the room and is reinvited", group_chat_room_admin_creator_leaves_and_is_reinvited),
};

test_suite_t group_chat_test_suite = {
	"Group Chat",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(group_chat_tests) / sizeof(group_chat_tests[0]), group_chat_tests
};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
